#ifndef _ARTISM_SCHEDULER_H_
#define _ARTISM_SCHEDULER_H_

#include <stdint.h>

// ============================================================================
// ARTISM Two-Layer Scheduler Configuration (Frozen Architecture)
// ============================================================================
// Layer 1: Criticality-First (RT/HR have absolute priority)
// Layer 2: WRR for Low-Criticality (HT/BE share remaining bandwidth)
// ============================================================================
#define SCHED_NUM_QUEUES        4           // 4 Semantic Queues
#define SCHED_CREDIT_QUANTUM    10          // Credit consumed per packet
#define SCHED_WEIGHT_TOTAL      1000        // Sum of all weights

// Queue Indices (Must match artism_def.h)
#define SCHED_Q_RT              0
#define SCHED_Q_HR              1
#define SCHED_Q_HT              2
#define SCHED_Q_BE              3

// Adaptive Feedback Configuration
#define SCHED_COOLDOWN_CYCLES   10          // Lock period after weight adjustment
#define SCHED_EWMA_WINDOW       20          // Packets per EWMA update window
#define SCHED_DVR_HIGH_THRESH   58982       // 0.9 * 65536 (Q16.16)
#define SCHED_DVR_LOW_THRESH    45875       // 0.7 * 65536 (Q16.16)
#define SCHED_WEIGHT_STEP       50          // Weight adjustment step
#define SCHED_MAX_WEIGHT        600         // Maximum weight per queue

// ============================================================================
// RTT-Based Latency Measurement (Cross-VM Clock Sync Solution)
// ============================================================================
// Instead of comparing timestamps across VMs, we embed send_tick in the packet
// and FreeRTOS echoes it back. Linux calculates RTT/2 as one-way latency.
// This requires a simple ACK mechanism.

typedef struct {
    uint64_t send_tick;         // Linux sends timestamp (embedded in packet)
    uint64_t recv_tick;         // FreeRTOS records when it received
    uint64_t ack_tick;          // FreeRTOS sends back (if ACK enabled)
} LatencyProbe;

// ============================================================================
// Per-Queue State
// ============================================================================
typedef struct {
    uint16_t weight;            // Current weight
    uint16_t min_weight;        // Minimum (anti-starvation guarantee)
    uint16_t init_weight;       // Initial weight (for reference)
    int32_t  credit;            // Current credit bucket (can go negative)
    uint16_t bucket_max;        // Max credit = 8 × weight
    uint8_t  alpha_shift;       // EWMA: α = 1 >> alpha_shift (3=0.125, 4=0.0625)
    uint8_t  cooldown;          // Remaining cooldown cycles
    uint8_t  is_critical;       // 1 = High-criticality, 0 = Low-criticality
} QueueState;

// ============================================================================
// Per-Queue Metrics (for EWMA feedback)
// ============================================================================
typedef struct {
    uint32_t total_packets;     // Packets in current window
    uint32_t violated_packets;  // Packets exceeding deadline
    uint64_t ewma_dvr;          // EWMA delay violation rate (Q16.16)
    uint64_t sum_latency_ns;    // Sum of latencies for average calculation
    uint16_t dynamic_blocks;    // Current dynamic pool usage
    uint16_t weight_margin;     // (weight - min_weight), for donor selection
} QueueMetrics;

// ============================================================================
// Global Scheduler State
// ============================================================================
typedef struct {
    QueueState   queues[SCHED_NUM_QUEUES];
    QueueMetrics metrics[SCHED_NUM_QUEUES];
    uint32_t     global_tick;   // Incremented each scheduling round
    uint32_t     adjust_count;  // Total weight adjustments made
    uint32_t     next_queue_idx; // Round Robin state
    uint64_t     timer_freq;    // ARM timer frequency (cached)
    
    // Debug: Selection phase counters
    uint32_t     select_phase1[SCHED_NUM_QUEUES];  // Selected in Phase 1
    uint32_t     select_phase4[SCHED_NUM_QUEUES];  // Selected in Phase 4
    uint32_t     select_phase5[SCHED_NUM_QUEUES];  // Selected in Phase 5 (fallback)
    uint32_t     replenish_count;                  // Total replenish calls
} ArtismScheduler;

// ============================================================================
// Default Weight Configuration (4 Queues - Frozen Architecture)
// ============================================================================
// Queue | Semantic  | Weight | Min | α_shift | Critical | Resource Policy
// ------+-----------+--------+-----+---------+----------+----------------
//   0   | RT        |   400  | 200 |    2    |    1     | Overwrite (Static Only)
//   1   | HR        |   300  | 150 |    3    |    1     | Block (Static Only)
//   2   | HT        |   200  | 100 |    3    |    0     | Borrow (Static + Dynamic)
//   3   | BE        |   100  |  50 |    2    |    0     | Drop (Static + Dynamic)
// Total: 1000
// Note: RT/HR (Critical) use Layer 1 absolute priority, HT/BE use Layer 2 WRR
// ============================================================================

static const uint16_t DEFAULT_WEIGHTS[SCHED_NUM_QUEUES] = {400, 300, 200, 100};
static const uint16_t DEFAULT_MIN_WEIGHTS[SCHED_NUM_QUEUES] = {200, 150, 100, 50};
static const uint8_t  DEFAULT_ALPHA_SHIFTS[SCHED_NUM_QUEUES] = {2, 3, 3, 2};
static const uint8_t  DEFAULT_CRITICAL[SCHED_NUM_QUEUES] = {1, 1, 0, 0};

// ============================================================================
// Function Prototypes
// ============================================================================

// Initialize scheduler with default weights
void artism_scheduler_init(ArtismScheduler *sched);

// Sync initial weights to SHM (call after g_meta is valid)
void artism_scheduler_sync_to_shm(ArtismScheduler *sched);

// Select next queue to service (returns queue index, or -1 if all empty)
// Two-Layer: Layer 1 checks RT/HR first, Layer 2 uses WRR for HT/BE
// Complexity: O(4), Expected time: <200ns
int artism_select_queue(ArtismScheduler *sched);

// Replenish credits for all queues (called each scheduling tick)
void artism_replenish_credits(ArtismScheduler *sched);

// Consume credit after sending a packet
void artism_consume_credit(ArtismScheduler *sched, int queue_idx, int packet_size);

// Record packet metrics (called after processing each packet)
// latency_ns should be calculated using RTT/2 or local timer
void artism_record_packet(ArtismScheduler *sched, int queue_idx, 
                          uint64_t latency_ns, uint64_t deadline_ns, 
                          int is_dynamic);

// Update EWMA and adjust weights (called periodically)
void artism_update_ewma(ArtismScheduler *sched);

// Fair weight redistribution (conservation constraint)
// User feedback: prioritize queues with weight_margin > 0 as donors
void artism_adjust_weights(ArtismScheduler *sched);

// Get current queue weight (for debugging)
uint16_t artism_get_weight(ArtismScheduler *sched, int queue_idx);

// Print scheduler state (for debugging)
void artism_print_state(ArtismScheduler *sched);

#endif // _ARTISM_SCHEDULER_H_
