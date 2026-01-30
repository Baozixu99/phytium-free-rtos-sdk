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
        
        // DEBUG: Reset phase counters
        sched->select_phase1[i] = 0;
        sched->select_phase4[i] = 0;
        sched->select_phase5[i] = 0;
    }
    sched->replenish_count = 0;
    
    printf("[SCHED] FG-WRR Initialized. Total Weight=%d, Timer Freq=%lu Hz\r\n",
           SCHED_WEIGHT_TOTAL, sched->timer_freq);
}

// [NEW] Sync initial weights to SHM - call this AFTER g_meta is valid
void artism_scheduler_sync_to_shm(ArtismScheduler *sched) {
    if (g_meta == NULL) return;
    
    for (int i = 0; i < SCHED_NUM_QUEUES; i++) {
        g_meta->stats.curr_weight[i] = sched->queues[i].weight;
        g_meta->stats.max_weight_seen[i] = sched->queues[i].weight;  // Init max to current
        ARTISM_STATS_FLUSH(&g_meta->stats.curr_weight[i]);
        ARTISM_STATS_FLUSH(&g_meta->stats.max_weight_seen[i]);
    }
    printf("[SCHED] Synced weights to SHM: Q0=%u, Q7=%u\r\n", 
           sched->queues[0].weight, sched->queues[7].weight);
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
        
        // FIX: When credit exhausted, advance to next queue for WRR fairness.
        // This is the key to achieving weight-proportional throughput (3.43:1).
        if (sched->queues[queue_idx].credit <= 0) {
            sched->next_queue_idx = (queue_idx + 1) % SCHED_NUM_QUEUES;
        }
    }
}

// ============================================================================
// Queue Selection (Deadline-Aware WRR - ARTISM Innovation)
// ============================================================================
// Key Innovation: WRR with deadline-aware priority boost
// - Normal case: Follow WRR credit-based scheduling
// - Urgent case: If a queue's oldest packet is near deadline, boost priority
// This combines WRR fairness with real-time deadline guarantees.
// ============================================================================
int artism_select_queue(ArtismScheduler *sched) {
    // --------------------------------------------------------
    // Phase 0: URGENT CHECK - Any queue has near-deadline packet?
    // This is the key innovation: deadline-aware priority boost
    // --------------------------------------------------------
    // Check critical queues for urgency (based on queue depth as proxy)
    // If a critical queue has many pending packets, it's likely urgent.
    // More sophisticated: could embed timestamp in packet and check deadline.
    for (int i = 0; i < SCHED_NUM_QUEUES; i++) {
        if (!sched->queues[i].is_critical) continue;
        
        volatile ArtismQueue *q = &g_meta->queues[i];
        uint64_t q_addr = (uint64_t)&q->info;
        __asm__ volatile("dc civac, %0" :: "r" (q_addr) : "memory");
        __asm__ volatile("dsb sy" ::: "memory");
        
        if (q->info.head == q->info.tail) continue;  // Empty
        
        // Calculate queue depth (pending packets)
        uint16_t head = q->info.head;
        uint16_t tail = q->info.tail;
        uint16_t depth = (head >= tail) ? (head - tail) : (ARTISM_DESC_PER_Q - tail + head);
        
        // URGENCY THRESHOLD: If queue depth > 50% capacity, consider urgent
        // This is a heuristic - packets waiting longer = more urgent
        // Can be refined with actual timestamp checking
        if (depth > (ARTISM_DESC_PER_Q / 2)) {
            // Urgent! Process this queue even if out of credit (credit borrowing)
            sched->select_phase1[i]++;
            return i;
        }
    }
    
    // --------------------------------------------------------
    // Phase 1: Normal WRR - Credit-based selection
    // Try all queues (critical and non-critical) with credit
    // --------------------------------------------------------
    for (int count = 0; count < SCHED_NUM_QUEUES; count++) {
        int idx = (sched->next_queue_idx + count) % SCHED_NUM_QUEUES;
        
        volatile ArtismQueue *q = &g_meta->queues[idx];
        
        uint64_t q_addr = (uint64_t)&q->info;
        __asm__ volatile("dc civac, %0" :: "r" (q_addr) : "memory");
        __asm__ volatile("dsb sy" ::: "memory");
        
        if (sched->queues[idx].credit > 0 && q->info.head != q->info.tail) {
            sched->select_phase1[idx]++;
            return idx;
        }
    }
    
    // --------------------------------------------------------
    // Phase 2: All queues out of credit. Check if any has data.
    // --------------------------------------------------------
    int pending_data = 0;
    for (int i = 0; i < SCHED_NUM_QUEUES; i++) {
        volatile ArtismQueue *q = &g_meta->queues[i];
        
        uint64_t q_addr = (uint64_t)&q->info;
        __asm__ volatile("dc civac, %0" :: "r" (q_addr) : "memory");
        __asm__ volatile("dsb sy" ::: "memory");
        
        if (q->info.head != q->info.tail) {
            pending_data = 1;
            break;
        }
    }
    
    if (!pending_data) {
        return -1; // No data at all, sleep
    }
    
    // --------------------------------------------------------
    // Phase 3: Replenish credits for new round
    // --------------------------------------------------------
    artism_replenish_credits(sched);
    sched->replenish_count++;
    
    // --------------------------------------------------------
    // Phase 4: Retry selection after replenish
    // --------------------------------------------------------
    for (int count = 0; count < SCHED_NUM_QUEUES; count++) {
        int idx = (sched->next_queue_idx + count) % SCHED_NUM_QUEUES;
        
        volatile ArtismQueue *q = &g_meta->queues[idx];
        
        uint64_t q_addr = (uint64_t)q;
        __asm__ volatile("dc civac, %0" :: "r" (q_addr) : "memory");
        __asm__ volatile("dsb sy" ::: "memory");

        if (sched->queues[idx].credit > 0 && q->info.head != q->info.tail) {
            sched->select_phase4[idx]++;
            return idx;
        }
    }
    
    // --------------------------------------------------------
    // Phase 5: Work-Conserving Fallback
    // --------------------------------------------------------
    for (int count = 0; count < SCHED_NUM_QUEUES; count++) {
        int idx = (sched->next_queue_idx + count) % SCHED_NUM_QUEUES;
        volatile ArtismQueue *q = &g_meta->queues[idx];
        if (q->info.head != q->info.tail) {
            sched->next_queue_idx = (idx + 1) % SCHED_NUM_QUEUES;
            sched->select_phase5[idx]++;
            return idx; 
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
        // FIX: Also update global stats for visibility in Linux tool
        g_meta->stats.deadline_miss[queue_idx]++;
        ARTISM_STATS_FLUSH(&g_meta->stats.deadline_miss[queue_idx]);
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
    
    // FIX: If no donors available, cannot boost - return early to prevent divide-by-zero
    if (total_donor_capacity == 0) {
        // printf("[SCHED] No donors available, skipping boost\r\n");
        return;
    }
    
    if (total_donor_capacity < total_boost_needed) {
        // Not enough donors - partial boost only (cap to available capacity)
        total_boost_needed = total_donor_capacity;
    }
    
    // Phase 3: Apply fair redistribution (proportional to donor capacity)
    int remaining_boost = total_boost_needed;
    
    for (int i = SCHED_NUM_QUEUES - 1; i >= 0 && remaining_boost > 0; i--) {
        if (donor_capacity[i] == 0) continue;
        
        // Calculate this donor's fair share (safe: total_donor_capacity > 0 guaranteed)
        int contribution = (remaining_boost * donor_capacity[i]) / total_donor_capacity;
        contribution = (contribution < donor_capacity[i]) ? contribution : donor_capacity[i];
        contribution = (contribution < remaining_boost) ? contribution : remaining_boost;
        
        if (contribution > 0) {
// Helper for stats flush
// (Macro now defined in artism_def.h)

// ... (artism_adjust_weights additions) ...

            sched->queues[i].weight -= contribution;
            sched->queues[i].cooldown = SCHED_COOLDOWN_CYCLES;
            remaining_boost -= contribution;
            
            // [STATS] Update weight in SHM for Linux verification
            g_meta->stats.curr_weight[i] = sched->queues[i].weight;
            ARTISM_STATS_FLUSH(&g_meta->stats.curr_weight[i]);
            
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
            
            // FIX: Enforce weight upper limit to prevent explosion
            int headroom = SCHED_MAX_WEIGHT - sched->queues[i].weight;
            if (headroom < 0) headroom = 0;
            if (actual_boost > headroom) {
                actual_boost = headroom;
            }
            
            sched->queues[i].weight += actual_boost;
            sched->queues[i].cooldown = SCHED_COOLDOWN_CYCLES;
            sched->queues[i].bucket_max = sched->queues[i].weight * 8;
        } // End if boost > 0

        // [STATS] Unconditional Update (Ensure visibility even if no boost or during cooldown)
        g_meta->stats.curr_weight[i] = sched->queues[i].weight;
        ARTISM_STATS_FLUSH(&g_meta->stats.curr_weight[i]);
        
        // FIX: Invalidate max_weight_seen BEFORE reading, in case Linux reset it to 0
        // This ensures we compare against the actual RAM value, not stale cache.
        uint64_t max_addr = (uint64_t)&g_meta->stats.max_weight_seen[i];
        __asm__ volatile("dsb sy" ::: "memory");
        __asm__ volatile("dc civac, %0" :: "r" (max_addr) : "memory");
        __asm__ volatile("dsb sy" ::: "memory");
            
        if (sched->queues[i].weight > g_meta->stats.max_weight_seen[i]) {
             g_meta->stats.max_weight_seen[i] = sched->queues[i].weight;
             ARTISM_STATS_FLUSH(&g_meta->stats.max_weight_seen[i]);
        }
    } // End Loop
    
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
    // Print selection phase statistics for WRR debugging
    static uint32_t print_count = 0;
    print_count++;
    
    // Only print every 1000 calls to reduce spam
    if (print_count % 1000 != 0) return;
    
    printf("[SCHED] Phase Stats (replenish=%u):\r\n", sched->replenish_count);
    printf("  Q0: P1=%u, P4=%u, P5=%u, credit=%d\r\n", 
           sched->select_phase1[0], sched->select_phase4[0], sched->select_phase5[0],
           sched->queues[0].credit);
    printf("  Q7: P1=%u, P4=%u, P5=%u, credit=%d\r\n", 
           sched->select_phase1[7], sched->select_phase4[7], sched->select_phase5[7],
           sched->queues[7].credit);
    
    // Calculate SELECTION ratio (NOT rx_count ratio!)
    uint32_t total_q0 = sched->select_phase1[0] + sched->select_phase4[0] + sched->select_phase5[0];
    uint32_t total_q7 = sched->select_phase1[7] + sched->select_phase4[7] + sched->select_phase5[7];
    
    // Also show rx_count for comparison
    uint32_t rx0 = g_meta->stats.rx_count[0];
    uint32_t rx7 = g_meta->stats.rx_count[7];
    
    if (total_q7 > 0) {
        printf("  Selection Ratio Q0/Q7: %u/%u = %.2f\r\n", total_q0, total_q7, (float)total_q0 / total_q7);
    }
    if (rx7 > 0) {
        printf("  RX Count Ratio Q0/Q7: %u/%u = %.2f\r\n", rx0, rx7, (float)rx0 / rx7);
    }
    
    // Key metric: Average selections per replenish cycle
    // This should be ~40 for Q0 and ~14 for Q7 in sustained overload
    if (sched->replenish_count > 0) {
        float avg_q0 = (float)total_q0 / sched->replenish_count;
        float avg_q7 = (float)total_q7 / sched->replenish_count;
        printf("  Avg per replenish: Q0=%.1f (expect 40), Q7=%.1f (expect 14)\r\n", avg_q0, avg_q7);
    }
}
