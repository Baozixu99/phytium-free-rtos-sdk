#ifndef _ARTISM_DEF_H_
#define _ARTISM_DEF_H_

#include <stdint.h>

// ============================================================================
// ARTISM Memory Constants (Fair Comparison to RTISM)
// ============================================================================
#define ARTISM_TOTAL_SIZE       0x10000     // 64KB Total (Same as RTISM)
#define ARTISM_BLOCK_SIZE       256         // 256 Bytes per Block
#define ARTISM_TOTAL_BLOCKS     256         // 64KB / 256B = 256 Blocks

// Layer 1: Static Reserve (50%)
#define ARTISM_STATIC_BLOCKS    128         // 32KB Total Static
#define ARTISM_NUM_QUEUES       8           // 8 Priority Queues
#define ARTISM_BLOCKS_PER_Q     16          // 128 / 8 = 16 Blocks (4KB) per Queue

// Layer 2: Dynamic Shared Pool (Remaining after static)
// 64KB total - 16KB meta = 48KB data = 192 blocks
// Static: 8Q * 16 = 128 blocks
// Dynamic: 192 - 128 = 64 blocks
#define ARTISM_DYNAMIC_BLOCKS   64          // 16KB Dynamic Shared (Corrected)
#define ARTISM_DYNAMIC_START_ID 128         // Dynamic blocks are IDs 128-191
#define ARTISM_BITMAP_WORDS     1           // 64 bits / 64 bits = 1 word

// Base Address (Partitioned from RTISM/HyperAMP region)
// We use the same base, assuming ARTISM replaces RTISM in usage
#define ARTISM_SHM_BASE_PADDR   0xDE400000UL 

// Traffic Types for Semantic Mapping
#define ARTISM_TRAFFIC_RT       0  // Real-Time (Overwrite)
#define ARTISM_TRAFFIC_HR       1  // High-Reliability (Blocking)
#define ARTISM_TRAFFIC_HT       2  // High-Throughput (Borrowing)
#define ARTISM_TRAFFIC_BE       3  // Best-Effort (Drop)

// ============================================================================
// Data Structures
// ============================================================================

// Simple Spinlock structure (Same as HyperAMP logic)
typedef struct {
    volatile uint32_t flag;
} ArtismSpinlock;

// 1. Memory Block (The actual data unit)
typedef struct {
    uint8_t data[ARTISM_BLOCK_SIZE];
} __attribute__((aligned(256), packed)) ArtismBlock;

// 2. Queue Header (Static Cyclic Buffer Metadata)
typedef struct {
    volatile uint16_t head;      // Write Index (0 - 65535, mask with 15)
    volatile uint16_t tail;      // Read Index
} __attribute__((aligned(64))) ArtismQueueHeader;

// 3. Queue Entry (Descriptor)
#define ARTISM_DESC_PER_Q   32 // Allow more descriptors than blocks to handle Dynamic bursts

typedef struct {
    volatile uint16_t block_id; // 0-255 (0xFFFF = Invalid)
    uint16_t len;               // Data length
    uint8_t  type;              // Traffic Type
    uint8_t  flags;             // Bit 0: Is_Dynamic
} EntryDesc;

typedef struct {
    ArtismQueueHeader info;
    EntryDesc descs[ARTISM_DESC_PER_Q]; // The Ring Buffer of Descriptors
} ArtismQueue;

// ============================================================================
// RTT Measurement: ACK Ring Buffer (FreeRTOS -> Linux)
// ============================================================================
#define ARTISM_ACK_RING_SIZE    64

typedef struct {
    volatile uint32_t seq_id;     // Sequence ID from probe
    volatile uint32_t queue_idx;  // Source queue
    volatile uint32_t status;     // 0=empty, 1=ready to read
} ArtismAck;

// Debug Statistics Structure (Telemetry)
// Aligned to 64 bytes to prevent false sharing and ensure atomic updates visibility
typedef struct {
    volatile uint32_t rx_count[ARTISM_NUM_QUEUES];      // Packets received successfully
    volatile uint32_t drop_count[ARTISM_NUM_QUEUES];    // Packets dropped (for BE test)
    volatile uint32_t block_count[ARTISM_NUM_QUEUES];   // Times blocked (for HR test)
    volatile uint32_t curr_weight[ARTISM_NUM_QUEUES];   // Current scheduler weight
    volatile uint32_t deadline_miss[ARTISM_NUM_QUEUES]; // Deadline violations
    volatile uint32_t max_weight_seen[ARTISM_NUM_QUEUES]; // Max weight reached (for Adaptive test)
    
    uint32_t _padding[10]; // Pad to align struct size to cache line multiple
} __attribute__((aligned(64))) ArtismDebugStats;

// 4. Global Manager (Metadata Region)
typedef struct {
    // Shared Dynamic Pool Management
    volatile uint64_t dynamic_bitmap[ARTISM_BITMAP_WORDS]; // 0=Free, 1=Used
    ArtismSpinlock bitmap_lock; // Spinlock for bitmap ops
    
    // Per-Queue Management
    ArtismQueue queues[ARTISM_NUM_QUEUES];
    
    // Debug Statistics for Verification (Telemetry)
    // Aligned to 64 bytes to prevent false sharing
    ArtismDebugStats stats;
    
    // ACK Ring Buffer for RTT Measurement
    ArtismAck ack_ring[ARTISM_ACK_RING_SIZE];
    volatile uint32_t ack_head;  // FreeRTOS writes (increments)
    volatile uint32_t ack_tail;  // Linux reads (increments)
    
    // ========================================================================
    // RTT Profiling: Timestamps at each critical point (FreeRTOS side)
    // All values are raw counter ticks from cntvct_el0 (50MHz = 20ns/tick)
    // ========================================================================
    volatile uint64_t prof_isr_entry;      // When IRQ handler fires
    volatile uint64_t prof_task_wakeup;    // When server task resumes
    volatile uint64_t prof_packet_read;    // After reading packet from SHM
    volatile uint64_t prof_ack_write;      // After writing ACK to ring
    volatile uint64_t prof_cache_flush;    // After DC CVAC cache clean
    volatile uint64_t prof_freq;           // Timer frequency for conversion
    volatile uint32_t prof_seq_id;         // Which seq this profile is for
    
} __attribute__((aligned(4096))) ArtismMeta; // 4KB aligned for Metadata

// 5. Root Structure
typedef struct {
    ArtismMeta meta;        // Metadata at start
    // Padding to 4KB or appropriate offset? 
    uint8_t reserved[0x4000 - sizeof(ArtismMeta)]; 
    
    // Data Blocks Region
    ArtismBlock blocks[0]; // Flexible array, pointing to 0x4000
} ArtismSharedLayout;

// Helper Macros
#define ARTISM_META_SIZE        0x4000      // 16KB reserved for Meta (Generous)
#define ARTISM_DATA_OFFSET      ARTISM_META_SIZE

// Function Prototypes for Server
void ArtismInit(void);

// 5. Cache Maintenance Macro (For Coherency with Linux)
// This is critical when SHM is mapped as Normal Cacheable.
// Usage: ARTISM_STATS_FLUSH(&address_to_flush);
#define ARTISM_STATS_FLUSH(addr) do { \
    uint64_t __addr = (uint64_t)(addr); \
    __asm__ volatile("dsb sy" ::: "memory"); \
    __asm__ volatile("dc civac, %0" :: "r" (__addr) : "memory"); \
    __asm__ volatile("dsb sy" ::: "memory"); \
} while(0)

#endif // _ARTISM_DEF_H_
