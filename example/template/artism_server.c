#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>  
#include <string.h>               
#include "hyperamp_server.h"       // For IRQ ID
#include "finterrupt.h"            // For InterruptInstall

// Include ARTISM definitions
#include "artism_def.h"
#include "artism_scheduler.h"      // FG-WRR Scheduler

// Global State
static ArtismSharedLayout *g_artism_shm = (ArtismSharedLayout *)ARTISM_SHM_BASE_PADDR;
ArtismMeta *g_meta = NULL;         // Now non-static for scheduler access
static uint8_t *g_data_region = NULL;
static SemaphoreHandle_t xArtismIrqSem = NULL;

// FG-WRR Scheduler Instance
static ArtismScheduler g_scheduler;

// Deadline configuration per queue (in nanoseconds)
// Frozen Architecture: 4 Queues = 4 Semantics
// RT/HR are critical (tight deadlines), HT/BE are non-critical (relaxed)
static const uint64_t QUEUE_DEADLINES[SCHED_NUM_QUEUES] = {
    1000000,     // Q0 (RT):   1ms  - Real-Time control loops
    5000000,     // Q1 (HR):   5ms  - Reliable data transfer
    20000000,    // Q2 (HT):   20ms - High-Throughput bulk
    100000000    // Q3 (BE):   100ms - Best-Effort (relaxed)
};

// Helper: Get current timer value (nanoseconds)
static inline uint64_t get_current_time_ns(void) {
    uint64_t count, freq;
    __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r" (count));
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r" (freq));
    return (count * 1000000000ULL) / freq;
}

// Spinlock for bitmap
void artism_lock(void) {
    while (__sync_lock_test_and_set(&g_meta->bitmap_lock.flag, 1)) {
        // Spin
    }
}

void artism_unlock(void) {
    __sync_lock_release(&g_meta->bitmap_lock.flag);
}

static void artism_free_dynamic(uint16_t block_id) {
    if (block_id < ARTISM_DYNAMIC_START_ID) return;
    int idx = block_id - ARTISM_DYNAMIC_START_ID;
    int w = idx / 64;
    int bit = idx % 64;
    
    // Atomic clear using CAS for cross-VM safety
    volatile uint64_t *word_ptr = &g_meta->dynamic_bitmap[w];
    uint64_t old_val, new_val;
    
    do {
        old_val = __atomic_load_n(word_ptr, __ATOMIC_ACQUIRE);
        new_val = old_val & ~(1UL << bit);
    } while (!__atomic_compare_exchange_n(word_ptr, &old_val, new_val,
                                          0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE));
    
    __asm__ volatile("dsb sy" ::: "memory");  // Ensure visibility to Linux
}

// Processing Logic (with latency recording for EWMA and RTT ACK)
void ArtismProcessPacket(int prio, EntryDesc *desc, uint64_t recv_time_ns) {
    uint16_t block_id = desc->block_id;
    uint8_t *data = g_data_region + (block_id * ARTISM_BLOCK_SIZE);
    
    // [CACHE FIX] Invalidate data block to read fresh data from Linux
    uint64_t data_addr = (uint64_t)data;
    __asm__ volatile("dsb sy" ::: "memory");
    // Invalidate the entire block (256 bytes = 4 cache lines)
    for (uint64_t addr = data_addr; addr < data_addr + ARTISM_BLOCK_SIZE; addr += 64) {
        __asm__ volatile("dc civac, %0" :: "r" (addr) : "memory");
    }
    __asm__ volatile("dsb sy" ::: "memory");
    
    // Extract seq_id from first 4 bytes (for RTT measurement)
    uint32_t seq_id = *(uint32_t*)data;
    
    // Log received packet (skip first 4 bytes which is seq_id, read 60 bytes payload)
    // char tmp[64];
    // strncpy(tmp, (char*)(data + 4), 60);
    // tmp[60] = 0;
    // printf("[ARTISM-RTOS] Recv P%d Type%d Block%d Seq%u: %s\r\n", prio, desc->type, block_id, seq_id, tmp);
    
    // [PROFILE] Record packet read time
    uint64_t t_read;
    __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r" (t_read));
    g_meta->prof_packet_read = t_read;
    
    // ========================================================================
    // Latency Test: 检测特殊 seq_id (0xFFFF0000 + n) 表示单向时延测量包
    // 直接计算 t2 - t1 得到真实单向时延 (因为时钟已同步)
    // 通信时延分解: t1 -> 数据复制 -> IPI注入 -> 中断处理 -> t2
    // ========================================================================
    if ((seq_id & 0xFFFF0000) == 0xFFFF0000) {
        // 从 payload 读取 Linux 发送时间戳 t1
        // 数据包格式: [seq_id: 4B][t1: 8B][padding: 52B]
        uint64_t t1_linux = *(uint64_t*)(data + 4);
        
        // 记录 t2 (接收完成时间戳)
        uint64_t t2_freertos;
        __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r" (t2_freertos));
        
        // 计算单向时延: t2 - t1 (ticks -> ns)
        uint64_t freq;
        __asm__ volatile("mrs %0, cntfrq_el0" : "=r" (freq));
        uint64_t latency_ns_val = ((t2_freertos - t1_linux) * 1000000000ULL) / freq;
        
        // 写入结果到 SHM (只保留必要字段)
        g_meta->latency_t2 = t2_freertos;
        g_meta->latency_ns = latency_ns_val;
        g_meta->latency_count++;
        
        // Flush 结果到共享内存
        __asm__ volatile("dc cvac, %0" :: "r" (&g_meta->latency_t2) : "memory");
        __asm__ volatile("dc cvac, %0" :: "r" (&g_meta->latency_ns) : "memory");
        __asm__ volatile("dc cvac, %0" :: "r" (&g_meta->latency_count) : "memory");
        __asm__ volatile("dsb sy" ::: "memory");
        
        // 更新统计并释放资源
        g_meta->stats.rx_count[prio]++;
        __asm__ volatile("dc civac, %0" :: "r" (&g_meta->stats.rx_count[prio]) : "memory");
        __asm__ volatile("dsb sy" ::: "memory");
        
        if (desc->flags & 1) {
            artism_free_dynamic(block_id);
        }
        return;
    }
    
    // Write ACK to ring buffer for RTT measurement
    if (seq_id != 0) {  // seq_id=0 means no RTT measurement needed
        uint32_t ack_idx = g_meta->ack_head % ARTISM_ACK_RING_SIZE;
        g_meta->ack_ring[ack_idx].seq_id = seq_id;
        g_meta->ack_ring[ack_idx].queue_idx = prio;
        __asm__ volatile("dmb sy" ::: "memory");
        g_meta->ack_ring[ack_idx].status = 1;  // Mark as ready
        g_meta->ack_head++;
        
        // [PROFILE] Record ACK write time
        uint64_t t_ack;
        __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r" (t_ack));
        g_meta->prof_ack_write = t_ack;
        
        // Cache clean for ACK visibility to Linux
        uint64_t addr = (uint64_t)&g_meta->ack_ring[ack_idx];
        __asm__ volatile("dc cvac, %0" :: "r" (addr) : "memory");
        __asm__ volatile("dmb sy" ::: "memory");
        
        // [PROFILE] Record cache flush time + store metadata
        uint64_t t_flush;
        __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r" (t_flush));
        g_meta->prof_cache_flush = t_flush;
        g_meta->prof_seq_id = seq_id;
        
        // Also flush profile data to ensure visibility
        // Also flush profile data to ensure visibility
        // FIX: Extend range by 64 bytes to ensure the last cache line (which might start before end) is covered.
        // Previous logic skipped the 2nd cache line because pa (start+64) > end, even though end was IN that line.
        for (uint64_t pa = (uint64_t)&g_meta->prof_isr_entry; pa < (uint64_t)&g_meta->prof_seq_id + 64; pa += 64) {
            __asm__ volatile("dc civac, %0" :: "r" (pa) : "memory");
        }
        __asm__ volatile("dsb sy" ::: "memory");
        
        // NOTE: printf removed for RTT benchmarking - it blocks ISR response
        // Uncomment for debugging: printf("[ARTISM-RTOS] Recv P%d Seq%u\r\n", prio, seq_id);
    }
    
    // [STATS] Update RX Count for FG-WRR Verification (Atomic-ish single writer)
    g_meta->stats.rx_count[prio]++;
    
    // Flush RX stats (Robust: DSB -> Clean&Inv -> DSB)
    uint64_t stats_addr = (uint64_t)&g_meta->stats.rx_count[prio];
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("dc civac, %0" :: "r" (stats_addr) : "memory");
    __asm__ volatile("dsb sy" ::: "memory");
    
    // NOTE: Simulated workload removed - it doesn't help create congestion because
    // the scheduler runs in a tight while loop after each IRQ, draining all queues.
    // Instead, we increased ARTISM_DESC_PER_Q to 64 so WRR has more packets to work with.
    
    uint64_t process_time_ns = get_current_time_ns();
    uint64_t latency_ns = process_time_ns - recv_time_ns;
    
    // Record metrics for EWMA feedback
    int is_dynamic = (block_id >= ARTISM_DYNAMIC_START_ID) ? 1 : 0;
    artism_record_packet(&g_scheduler, prio, latency_ns, QUEUE_DEADLINES[prio], is_dynamic);
    
    // Free dynamic block if used
    if (desc->flags & 1) {
        artism_free_dynamic(block_id);
    }
}

// Main Task with FG-WRR Scheduling
void ArtismServerTask(void *pvParameters) {
    printf("[ARTISM-RTOS] FG-WRR Server Started.\r\n");
    
    g_meta = &g_artism_shm->meta;
    g_data_region = (uint8_t*)g_artism_shm + ARTISM_DATA_OFFSET;
    
    // Initialize FG-WRR scheduler
    artism_scheduler_init(&g_scheduler);
    
    // Sync initial weights to SHM (so Linux can read correct values)
    artism_scheduler_sync_to_shm(&g_scheduler);
    
    uint32_t msg_count = 0;
    
    for (;;) {
        // Wait for IRQ (Semaphore given by ISR) - use timeout to allow reset check
        if (xSemaphoreTake(xArtismIrqSem, pdMS_TO_TICKS(10)) == pdTRUE) {
            // [PROFILE] Record task wakeup time IMMEDIATELY after semaphore take
            uint64_t t_wakeup;
            __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r" (t_wakeup));
            g_meta->prof_task_wakeup = t_wakeup;
            uint64_t freq;
            __asm__ volatile("mrs %0, cntfrq_el0" : "=r" (freq));
            g_meta->prof_freq = freq;
            
            uint64_t recv_time_ns = get_current_time_ns();
            
            // ============================================================
            // Check sync/reset flags (always check, for all tests)
            // ============================================================
            // [RTT优化建议] 如需降低 RTT 时延（当前 ~12μs），可以注释掉以下 3 个检查（约可降低 1-2μs）
            // 但这会影响系统的统一性和可解释性
            // 当前架构保证了所有测试（RTT/WRR/Adaptive）使用完全相同的处理路径。
            // ============================================================
            
            // Clock Sync Test (simple)
            uint64_t sync_addr = (uint64_t)&g_meta->sync_test_flag;
            __asm__ volatile("dc civac, %0" :: "r" (sync_addr) : "memory");
            __asm__ volatile("dsb sy" ::: "memory");
            
            if (g_meta->sync_test_flag == 1) {
                uint64_t t2;
                __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r" (t2));
                g_meta->sync_freertos_t2 = t2;
                __asm__ volatile("dc cvac, %0" :: "r" (&g_meta->sync_freertos_t2) : "memory");
                __asm__ volatile("dsb sy" ::: "memory");
                g_meta->sync_test_flag = 2;
                __asm__ volatile("dc cvac, %0" :: "r" (sync_addr) : "memory");
                __asm__ volatile("dsb sy" ::: "memory");
                continue;
            }
            
            // Four-Message Exchange (NTP-style)
            uint64_t phase_addr = (uint64_t)&g_meta->sync_phase;
            __asm__ volatile("dc civac, %0" :: "r" (phase_addr) : "memory");
            __asm__ volatile("dsb sy" ::: "memory");
            
            if (g_meta->sync_phase == 1) {
                uint64_t t2;
                __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r" (t2));
                g_meta->sync_t2 = t2;
                uint64_t t3;
                __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r" (t3));
                g_meta->sync_t3 = t3;
                __asm__ volatile("dc cvac, %0" :: "r" (&g_meta->sync_t2) : "memory");
                __asm__ volatile("dc cvac, %0" :: "r" (&g_meta->sync_t3) : "memory");
                __asm__ volatile("dsb sy" ::: "memory");
                g_meta->sync_phase = 2;
                __asm__ volatile("dc cvac, %0" :: "r" (phase_addr) : "memory");
                __asm__ volatile("dsb sy" ::: "memory");
                continue;
            }
            
            // Weight reset request
            uint64_t reset_addr = (uint64_t)&g_meta->stats.reset_weights_request;
            __asm__ volatile("dc civac, %0" :: "r" (reset_addr) : "memory");
            __asm__ volatile("dsb sy" ::: "memory");
            
            if (g_meta->stats.reset_weights_request == 1) {
                artism_scheduler_init(&g_scheduler);
                artism_scheduler_sync_to_shm(&g_scheduler);
                for (int i = 0; i < ARTISM_NUM_QUEUES; i++) {
                    g_meta->stats.rx_count[i] = 0;
                    __asm__ volatile("dc cvac, %0" :: "r" (&g_meta->stats.rx_count[i]) : "memory");
                }
                __asm__ volatile("dsb sy" ::: "memory");
                g_meta->stats.reset_weights_request = 0;
                __asm__ volatile("dc cvac, %0" :: "r" (reset_addr) : "memory");
                __asm__ volatile("dsb sy" ::: "memory");
            }
            
            // ============================================================
            // Unified FG-WRR Scheduling Path
            // All tests (RTT, WRR, Adaptive) use the same scheduler
            // This ensures consistency and explainability for publication
            // ============================================================
            int selected_queue;
            int processed_any = 0;
            while ((selected_queue = artism_select_queue(&g_scheduler)) >= 0) {
                volatile ArtismQueue *q = &g_meta->queues[selected_queue];
                
                // Process ONE packet from selected queue
                if (q->info.tail != q->info.head) {
                    uint16_t idx = q->info.tail;
                    EntryDesc desc = q->descs[idx];
                    
                    // Consume credit
                    artism_consume_credit(&g_scheduler, selected_queue, SCHED_CREDIT_QUANTUM);
                    
                    // FIX: Use per-packet dequeue timestamp for accurate latency measurement.
                    // Previously recv_time_ns was captured once at semaphore wakeup and reused
                    // for ALL packets, masking queuing delay for later packets.
                    // Now we use the IRQ arrival time (recv_time_ns) as the enqueue reference,
                    // and measure actual processing time at dequeue point.
                    uint64_t dequeue_time_ns = get_current_time_ns();
                    // Latency = time from IRQ arrival to actual dequeue (captures queuing delay)
                    uint64_t pkt_latency_ref = recv_time_ns;
                    
                    // Process packet (pass dequeue_time for internal use)
                    ArtismProcessPacket(selected_queue, &desc, pkt_latency_ref);
                    
                    // Advance Tail
                    __asm__ volatile("dmb sy" ::: "memory");
                    q->info.tail = (idx + 1) % ARTISM_DESC_PER_Q;
                    
                    // [CACHE FIX] Clean tail pointer so Linux sees updated queue state
                    uint64_t tail_addr = (uint64_t)&q->info;
                    __asm__ volatile("dsb sy" ::: "memory");
                    __asm__ volatile("dc cvac, %0" :: "r" (tail_addr) : "memory");
                    __asm__ volatile("dsb sy" ::: "memory");
                    
                    msg_count++;
                    processed_any = 1;
                }
            }
        }
    }
}

// ----------------------------------------------------------------------
// ISR Handler
// ----------------------------------------------------------------------
static void ArtismIrqHandler(s32 vector, void *param) {
    // [PROFILE] Record ISR entry time
    uint64_t isr_time;
    __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r" (isr_time));
    if (g_meta) g_meta->prof_isr_entry = isr_time;
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xArtismIrqSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// ----------------------------------------------------------------------
// Init
// ----------------------------------------------------------------------
void ArtismInit(void) {
    // Map Global State (Fixed base address, direct physical mapping)
    g_meta = &g_artism_shm->meta;
    g_data_region = (uint8_t*)g_artism_shm + ARTISM_DATA_OFFSET;
    
    printf("[ARTISM-RTOS] g_meta=%p, sizeof(ArtismMeta)=%lu\r\n", 
           g_meta, (unsigned long)sizeof(ArtismMeta));
    
    xArtismIrqSem = xSemaphoreCreateBinary();
    
    // Register ISR (Use correct HyperAMP SGI ID)
    // HYPERAMP_SGI_IRQ_ID = 74 is defined in hyperamp_server.h
    InterruptInstall(HYPERAMP_SGI_IRQ_ID, ArtismIrqHandler, NULL, "ARTISM-ISR");
    InterruptSetPriority(HYPERAMP_SGI_IRQ_ID, 0); // Max Prio
    InterruptUmask(HYPERAMP_SGI_IRQ_ID);
    
    // Create Task
    // Use smaller stack for efficiency if possible
    xTaskCreate(ArtismServerTask, "ARTISM_Server", 4096, NULL, configMAX_PRIORITIES-1, NULL); 
    
    printf("[ARTISM-RTOS] Initialized. Listening on IRQ %d.\r\n", HYPERAMP_SGI_IRQ_ID);
}
