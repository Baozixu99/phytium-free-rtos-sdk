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
// Based on typical real-time requirements
static const uint64_t QUEUE_DEADLINES[SCHED_NUM_QUEUES] = {
    // STRESS TEST: Set Q0 deadline to 100ns to FORCE VIOLATION
    100,        // Q0 (RT):    100ns (Impossible deadline)
    2000000,    // Q1 (RT/HR): 2ms
    5000000,    // Q2 (HR):    5ms
    10000000,   // Q3 (HR/HT): 10ms
    20000000,   // Q4 (HT):    20ms
    50000000,   // Q5 (HT):    50ms
    100000000,  // Q6 (HT/BE): 100ms
    500000000   // Q7 (BE):    500ms (best-effort, relaxed)
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
    
    artism_lock();
    g_meta->dynamic_bitmap[w] &= ~(1UL << bit);
    artism_unlock();
    
    printf("[ARTISM-RTOS] Freed Dynamic Block %d\r\n", block_id);
}

// Processing Logic (with latency recording for EWMA and RTT ACK)
void ArtismProcessPacket(int prio, EntryDesc *desc, uint64_t recv_time_ns) {
    uint16_t block_id = desc->block_id;
    uint8_t *data = g_data_region + (block_id * ARTISM_BLOCK_SIZE);
    
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
        for (uint64_t pa = (uint64_t)&g_meta->prof_isr_entry; pa < (uint64_t)&g_meta->prof_seq_id + 4; pa += 64) {
            __asm__ volatile("dc cvac, %0" :: "r" (pa) : "memory");
        }
        __asm__ volatile("dmb sy" ::: "memory");
    }
    
    // Calculate latency (using local time - simplified, not RTT)
    // For accurate cross-VM latency, use RTT mechanism
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
    
    uint32_t msg_count = 0;
    
    for (;;) {
        // Wait for IRQ (Semaphore given by ISR)
        if (xSemaphoreTake(xArtismIrqSem, portMAX_DELAY) == pdTRUE) {
            // [PROFILE] Record task wakeup time + timer frequency
            uint64_t t_wakeup;
            __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r" (t_wakeup));
            g_meta->prof_task_wakeup = t_wakeup;
            uint64_t freq;
            __asm__ volatile("mrs %0, cntfrq_el0" : "=r" (freq));
            g_meta->prof_freq = freq;
            
            uint64_t recv_time_ns = get_current_time_ns();
            
            // FG-WRR: Select queue based on credit and data availability
            int selected_queue;
            while ((selected_queue = artism_select_queue(&g_scheduler)) >= 0) {
                volatile ArtismQueue *q = &g_meta->queues[selected_queue];
                
                // Process ONE packet from selected queue
                if (q->info.tail != q->info.head) {
                    uint16_t idx = q->info.tail;
                    EntryDesc desc = q->descs[idx];
                    
                    // Consume credit
                    artism_consume_credit(&g_scheduler, selected_queue, SCHED_CREDIT_QUANTUM);
                    
                    // Process packet
                    ArtismProcessPacket(selected_queue, &desc, recv_time_ns);
                    
                    // Advance Tail
                    __asm__ volatile("dmb sy" ::: "memory");
                    q->info.tail = (idx + 1) % ARTISM_DESC_PER_Q;
                    
                    msg_count++;
                }
            }
            
            // Periodic debug print
            if (msg_count % 10 == 0 && msg_count > 0) {
                artism_print_state(&g_scheduler);
            }
        }
        
        // Force print at end of batch to ensure visibility
        if (msg_count > 0) {
             artism_print_state(&g_scheduler);
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
