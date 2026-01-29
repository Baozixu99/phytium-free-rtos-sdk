#ifndef _ARTISM_SCHEDULER_H_
#define _ARTISM_SCHEDULER_H_

#include <stdint.h>

// ============================================================================
// FG-WRR Scheduler Configuration
// ============================================================================
#define SCHED_NUM_QUEUES        8
#define SCHED_CREDIT_QUANTUM    10      // Small quantum for fine-grained byte fairness
                                        // Weight 400 = 40 packets (if quantum=10)
                                        // Weight 140 = 14 packets 
#define SCHED_WEIGHT_TOTAL      1600    // Sum of all weights (conserved)
#define SCHED_MAX_DYN_BLOCKS    64      // Per-queue dynamic block limit
#define SCHED_COOLDOWN_CYCLES   10      // Lock period after weight adjustment
#define SCHED_EWMA_WINDOW       10      // Packets per EWMA update window (Must be < 16 for Q0 static limit)

// Hysteresis thresholds (Q16.16 fixed-point, 0.9 and 0.7)
#define SCHED_DVR_HIGH_THRESH   58982   // 0.9 * 65536
#define SCHED_DVR_LOW_THRESH    45875   // 0.7 * 65536

// Weight adjustment step (Increased to 40 for visibility and 10x scaling match)
#define SCHED_WEIGHT_STEP       40

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
} ArtismScheduler;

// ============================================================================
// Default Weight Configuration
// Based on discussion: High-crit uses smaller α (more stable),
// Low-crit uses larger α (faster response)
// ============================================================================
// Queue | Type      | Weight | Min | α_shift | Critical
// ------+-----------+--------+-----+---------+----------
//   0   | RT        |   40   | 20  |    4    |    1
//   1   | RT/HR     |   30   | 15  |    4    |    1
//   2   | HR        |   24   | 12  |    4    |    1
//   3   | HR/HT     |   18   |  8  |    3    |    0
//   4   | HT        |   14   |  6  |    3    |    0
//   5   | HT        |   12   |  6  |    3    |    0
//   6   | HT/BE     |    8   |  4  |    2    |    0
//   7   | BE        |   14   |  4  |    2    |    0
// Total: 160

// Total: 1600
static const uint16_t DEFAULT_WEIGHTS[SCHED_NUM_QUEUES] = {400, 300, 240, 180, 140, 120, 80, 140};
static const uint16_t DEFAULT_MIN_WEIGHTS[SCHED_NUM_QUEUES] = {200, 150, 120, 80, 60, 60, 40, 40};
static const uint8_t  DEFAULT_ALPHA_SHIFTS[SCHED_NUM_QUEUES] = {0, 4, 4, 3, 3, 3, 2, 2};
static const uint8_t  DEFAULT_CRITICAL[SCHED_NUM_QUEUES] = {1, 1, 1, 0, 0, 0, 0, 0};

// ============================================================================
// Function Prototypes
// ============================================================================

// Initialize scheduler with default weights
void artism_scheduler_init(ArtismScheduler *sched);

// Select next queue to service (returns queue index, or -1 if all empty)
// Complexity: O(8), Expected time: <200ns
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
