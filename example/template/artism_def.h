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

// Layer 2: Dynamic Shared Pool (50%)
#define ARTISM_DYNAMIC_BLOCKS   128         // 32KB Dynamic Shared
#define ARTISM_DYNAMIC_START_ID 128         // Dynamic blocks are IDs 128-255
#define ARTISM_BITMAP_WORDS     2           // 128 bits / 64 bits = 2 words

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

// 4. Global Manager (Metadata Region)
typedef struct {
    // Shared Dynamic Pool Management
    volatile uint64_t dynamic_bitmap[ARTISM_BITMAP_WORDS]; // 0=Free, 1=Used
    ArtismSpinlock bitmap_lock; // Spinlock for bitmap ops
    
    // Per-Queue Management
    ArtismQueue queues[ARTISM_NUM_QUEUES];
    
    // Debug Buffer for FG-WRR verification
    char debug_buffer[2048];
    
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

#endif // _ARTISM_DEF_H_
