#include "artism_scheduler.h"
#include "artism_def.h"
#include <stdio.h>
#include <string.h>

// External reference to shared memory meta for queue data check
extern ArtismMeta *g_meta;

// ============================================================================
// Initialization
// ============================================================================
void artism_scheduler_init(ArtismScheduler *sched) {
    memset(sched, 0, sizeof(ArtismScheduler));
    
    // Get timer frequency (cached)
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r" (sched->timer_freq));
    
    for (int i = 0; i < SCHED_NUM_QUEUES; i++) {
        sched->queues[i].weight = DEFAULT_WEIGHTS[i];
        sched->queues[i].min_weight = DEFAULT_MIN_WEIGHTS[i];
        sched->queues[i].init_weight = DEFAULT_WEIGHTS[i];
        sched->queues[i].alpha_shift = DEFAULT_ALPHA_SHIFTS[i];
        sched->queues[i].is_critical = DEFAULT_CRITICAL[i];
        sched->queues[i].bucket_max = DEFAULT_WEIGHTS[i] * 8;
        sched->queues[i].credit = 0; // Start empty for faster steady-state
        sched->queues[i].cooldown = 0;
        
        // Initialize metrics
        sched->metrics[i].weight_margin = DEFAULT_WEIGHTS[i] - DEFAULT_MIN_WEIGHTS[i];
    }
    
    printf("[SCHED] FG-WRR Initialized. Total Weight=%d, Timer Freq=%lu Hz\r\n",
           SCHED_WEIGHT_TOTAL, sched->timer_freq);
}

// ============================================================================
// Credit Management
// ============================================================================
void artism_replenish_credits(ArtismScheduler *sched) {
    for (int i = 0; i < SCHED_NUM_QUEUES; i++) {
        sched->queues[i].credit += sched->queues[i].weight;
        if (sched->queues[i].credit > (int32_t)sched->queues[i].bucket_max) {
            sched->queues[i].credit = sched->queues[i].bucket_max;
        }
    }
    sched->global_tick++;
}

void artism_consume_credit(ArtismScheduler *sched, int queue_idx, int packet_size) {
    if (queue_idx >= 0 && queue_idx < SCHED_NUM_QUEUES) {
        sched->queues[queue_idx].credit -= packet_size;
    }
}

// ============================================================================
// Queue Selection (Core FG-WRR Logic)
// ============================================================================
int artism_select_queue(ArtismScheduler *sched) {
    // --------------------------------------------------------
    // Phase 1: Try to find a queue with Credit > 0 AND Data
    // --------------------------------------------------------
    for (int count = 0; count < SCHED_NUM_QUEUES; count++) {
        int idx = (sched->next_queue_idx + count) % SCHED_NUM_QUEUES;
        
        volatile ArtismQueue *q = &g_meta->queues[idx];
        if (sched->queues[idx].credit > 0 && q->info.head != q->info.tail) {
            sched->next_queue_idx = idx; // Stay on this queue or move next?
            // DRR typically exhausts quantum, but for interleaved smoothness, 
            // we process one packet and let next call potentially pick same or next.
            // Let's implement byte-fairness: we stay if large credit, but RR is better for latency.
            // Point to next for NEXT call to ensure fairness
            sched->next_queue_idx = (idx + 1) % SCHED_NUM_QUEUES;
            return idx;
        }
    }
    
    // --------------------------------------------------------
    // Phase 2: All active queues are out of credit? 
    // Check if ANY queue has data.
    // --------------------------------------------------------
    int pending_data = 0;
    for (int i = 0; i < SCHED_NUM_QUEUES; i++) {
        volatile ArtismQueue *q = &g_meta->queues[i];
        if (q->info.head != q->info.tail) {
            pending_data = 1;
            break;
        }
    }
    
    if (!pending_data) {
        return -1; // No data at all, sleep
    }
    
    // --------------------------------------------------------
    // Phase 3: Data exists (congestion/busy), but no credits.
    // Start a new "Round" -> Replenish.
    // --------------------------------------------------------
    artism_replenish_credits(sched);
    
    // --------------------------------------------------------
    // Phase 4: Retry selection after replenish
    // --------------------------------------------------------
    for (int count = 0; count < SCHED_NUM_QUEUES; count++) {
        int idx = (sched->next_queue_idx + count) % SCHED_NUM_QUEUES;
        
        volatile ArtismQueue *q = &g_meta->queues[idx];
        if (sched->queues[idx].credit > 0 && q->info.head != q->info.tail) {
             sched->next_queue_idx = (idx + 1) % SCHED_NUM_QUEUES;
             return idx;
        }
    }
    
    // --------------------------------------------------------
    // Phase 5: Work-Conserving Fallback 
    // (Should rarely happen unless weights are huge or packets huge)
    // Just pick the first one with data (Priority Order or RR?)
    // --------------------------------------------------------
    for (int i = 0; i < SCHED_NUM_QUEUES; i++) {
        volatile ArtismQueue *q = &g_meta->queues[i];
        if (q->info.head != q->info.tail) {
            return i; 
        }
    }
    
    return -1;
}

// ============================================================================
// Packet Metrics Recording
// ============================================================================
void artism_record_packet(ArtismScheduler *sched, int queue_idx, 
                          uint64_t latency_ns, uint64_t deadline_ns,
                          int is_dynamic) {
    if (queue_idx < 0 || queue_idx >= SCHED_NUM_QUEUES) return;
    
    QueueMetrics *m = &sched->metrics[queue_idx];
    m->total_packets++;
    m->sum_latency_ns += latency_ns;
    
    if (latency_ns > deadline_ns) {
        m->violated_packets++;
    }
    
    if (is_dynamic) {
        m->dynamic_blocks++;
    }
    
    // Trigger EWMA update after window size
    if (m->total_packets >= SCHED_EWMA_WINDOW) {
        artism_update_ewma(sched);
    }
}

// ============================================================================
// EWMA Update
// ============================================================================
void artism_update_ewma(ArtismScheduler *sched) {
    for (int i = 0; i < SCHED_NUM_QUEUES; i++) {
        QueueMetrics *m = &sched->metrics[i];
        QueueState *q = &sched->queues[i];
        
        if (m->total_packets == 0) continue;
        
        // Calculate current DVR (Q16.16 fixed-point)
        uint64_t dvr_current = ((uint64_t)m->violated_packets << 16) / m->total_packets;
        
        // EWMA: new = α * current + (1-α) * old
        // α = 1 >> alpha_shift
        uint64_t alpha_term = dvr_current >> q->alpha_shift;
        uint64_t decay_term = m->ewma_dvr - (m->ewma_dvr >> q->alpha_shift);
        m->ewma_dvr = alpha_term + decay_term;
        
        // Update weight margin for fair redistribution
        m->weight_margin = q->weight - q->min_weight;
        
        // Reset window
        m->total_packets = 0;
        m->violated_packets = 0;
        m->sum_latency_ns = 0;
    }
    
    // Attempt weight adjustment
    artism_adjust_weights(sched);
}

// ============================================================================
// Fair Weight Redistribution (User Feedback Applied)
// ============================================================================
void artism_adjust_weights(ArtismScheduler *sched) {
    int boost_requests[SCHED_NUM_QUEUES] = {0};
    int total_boost_needed = 0;
    int donor_capacity[SCHED_NUM_QUEUES] = {0};
    int total_donor_capacity = 0;
    
    // Phase 1: Identify queues needing boost (DVR exceeds high threshold)
    for (int i = 0; i < SCHED_NUM_QUEUES; i++) {
        QueueState *q = &sched->queues[i];
        QueueMetrics *m = &sched->metrics[i];
        
        // Skip if in cooldown
        if (q->cooldown > 0) {
            q->cooldown--;
            continue;
        }
        
        // Check if needs boost (DVR > 0.9 * deadline threshold)
        if (m->ewma_dvr > SCHED_DVR_HIGH_THRESH && q->is_critical) {
            boost_requests[i] = SCHED_WEIGHT_STEP;
            total_boost_needed += SCHED_WEIGHT_STEP;
        }
        // Check if can release boost (DVR < 0.7 * deadline threshold)
        else if (m->ewma_dvr < SCHED_DVR_LOW_THRESH && 
                 q->weight > q->init_weight) {
            // This queue was boosted before but now stable - can give back
            boost_requests[i] = -SCHED_WEIGHT_STEP;
            total_boost_needed -= SCHED_WEIGHT_STEP; // Net effect
        }
    }
    
    if (total_boost_needed <= 0) return; // No boost needed
    
    // Phase 2: Identify donors (exclude queues requesting boost)
    // First, calculate total available donor capacity
    for (int i = SCHED_NUM_QUEUES - 1; i >= 0; i--) {
        QueueState *q = &sched->queues[i];
        if (q->cooldown > 0) continue;
        
        // Skip if this queue is requesting a boost (can't donate to itself)
        if (boost_requests[i] > 0) continue;
        
        // Donor must have weight > min_weight
        if (q->weight > q->min_weight) {
            donor_capacity[i] = q->weight - q->min_weight;
            total_donor_capacity += donor_capacity[i];
        }
    }
    
    if (total_donor_capacity < total_boost_needed) {
        // Not enough donors - partial boost only
        printf("[SCHED] Warning: Insufficient donor capacity (%d < %d)\r\n",
               total_donor_capacity, total_boost_needed);
    }
    
    // Phase 3: Apply fair redistribution (proportional to donor capacity)
    int remaining_boost = total_boost_needed;
    
    for (int i = SCHED_NUM_QUEUES - 1; i >= 0 && remaining_boost > 0; i--) {
        if (donor_capacity[i] == 0) continue;
        
        // Calculate this donor's fair share
        int contribution = (remaining_boost * donor_capacity[i]) / total_donor_capacity;
        contribution = (contribution < donor_capacity[i]) ? contribution : donor_capacity[i];
        contribution = (contribution < remaining_boost) ? contribution : remaining_boost;
        
        if (contribution > 0) {
            sched->queues[i].weight -= contribution;
            sched->queues[i].cooldown = SCHED_COOLDOWN_CYCLES;
            remaining_boost -= contribution;
            
            // [RTT BENCHMARK] Printf disabled
            // printf("[SCHED] Q%d donated %d weight (now=%d)\r\n", 
            //        i, contribution, sched->queues[i].weight);
        }
    }
    
    // Phase 4: Apply boost to requesting queues
    for (int i = 0; i < SCHED_NUM_QUEUES; i++) {
        if (boost_requests[i] > 0) {
            int actual_boost = (total_boost_needed - remaining_boost > 0) ?
                               boost_requests[i] : 0;
            
            sched->queues[i].weight += actual_boost;
            sched->queues[i].cooldown = SCHED_COOLDOWN_CYCLES;
            sched->queues[i].bucket_max = sched->queues[i].weight * 8;
            
            // [RTT BENCHMARK] Printf disabled
            // printf("[SCHED] Q%d boosted by %d (now=%d)\r\n",
            //        i, actual_boost, sched->queues[i].weight);
        }
    }
    
    sched->adjust_count++;
}

// ============================================================================
// Debug Functions
// ============================================================================
uint16_t artism_get_weight(ArtismScheduler *sched, int queue_idx) {
    if (queue_idx >= 0 && queue_idx < SCHED_NUM_QUEUES) {
        return sched->queues[queue_idx].weight;
    }
    return 0;
}

void artism_print_state(ArtismScheduler *sched) {
    if (!g_meta) return;
    
    char *buf = g_meta->debug_buffer;
    int offset = 0;
    
    offset += snprintf(buf + offset, 2048 - offset, "\r\n=== FG-WRR Scheduler State ===\r\n");
    offset += snprintf(buf + offset, 2048 - offset, "Global Tick: %lu, Adjustments: %lu\r\n", 
           (unsigned long)sched->global_tick, (unsigned long)sched->adjust_count);
    offset += snprintf(buf + offset, 2048 - offset, "Queue | Weight | Credit | DVR(Q16) | Cooldown\r\n");
    offset += snprintf(buf + offset, 2048 - offset, "------+--------+--------+----------+---------\r\n");
    
    for (int i = 0; i < SCHED_NUM_QUEUES; i++) {
        offset += snprintf(buf + offset, 2048 - offset, "  Q%d  |  %3d   | %5d  |  %5lu   |    %d\r\n",
               i, sched->queues[i].weight, sched->queues[i].credit,
               (unsigned long)(sched->metrics[i].ewma_dvr >> 10), // Show as 0-63
               sched->queues[i].cooldown);
    }
    offset += snprintf(buf + offset, 2048 - offset, "==============================\r\n");
    
    // Ensure data is visible to Linux (Cache Clean)
    // 1. Memory Barrier
    __asm__ volatile("dmb sy" ::: "memory");
    
    // 2. Cache Clean by Virtual Address (DC CVAC)
    // We need to clean the cache lines covering the buffer (2048 bytes)
    // Assuming 64-byte cache line size
    uint64_t start = (uint64_t)buf;
    uint64_t end = start + 2048;
    for (uint64_t addr = start; addr < end; addr += 64) {
        __asm__ volatile("dc cvac, %0" :: "r" (addr) : "memory");
    }
    __asm__ volatile("dmb sy" ::: "memory");
}
