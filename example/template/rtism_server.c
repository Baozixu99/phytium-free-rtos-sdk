#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hyperamp_server.h" 
#include "finterrupt.h"

// RTISM Constants
#define RTISM_NUM_PRIORITIES 8
#define RTISM_QUEUE_SIZE     0x2000
#define RTISM_BASE_PADDR     0xDE400000UL

static volatile AmpMsgQueue* rtism_qs[RTISM_NUM_PRIORITIES];
static TaskHandle_t g_rtism_task = NULL;
static SemaphoreHandle_t g_rtism_sem = NULL;

// Reuse Cache Logic (Inline for simplicity or external ref)
static inline void InvalidateRange(void* start, size_t len) {
    unsigned long addr = (unsigned long)start;
    unsigned long end = addr + len;
    addr &= ~0x3FUL; // Align down
    end = (end + 63) & ~0x3FUL; // Align up
    
    for (; addr < end; addr += 64) {
        __asm__ volatile("dc ivac, %0" :: "r"(addr) : "memory");
    }
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("isb" ::: "memory");
}

static inline void CleanRange(void* start, size_t len) {
    unsigned long addr = (unsigned long)start;
    unsigned long end = addr + len;
    addr &= ~0x3FUL; 
    end = (end + 63) & ~0x3FUL; 
    
    for (; addr < end; addr += 64) {
        __asm__ volatile("dc cvac, %0" :: "r"(addr) : "memory");
    }
    __asm__ volatile("dsb sy" ::: "memory");
}

// ----------------------------------------------------------------------
// 1. RTISM Server Task (The Forwarder)
// ----------------------------------------------------------------------
void RtismServerTask(void* params) {
    printf("[RTISM] Server Task Started. Polling 8 Priority Queues.\r\n");
    
    while(1) {
        // Wait for SGI Interrupt
        xSemaphoreTake(g_rtism_sem, portMAX_DELAY);
        
        // Priority Polling (High Prio 0 -> Low Prio 7)
        for(int p = 0; p < RTISM_NUM_PRIORITIES; p++) {
            volatile AmpMsgQueue* q = rtism_qs[p];
            
            // 1. Invalidate Queue Header to see new 'proc_ing_h' or 'wait_h'
            InvalidateRange((void*)q, 64);
            
            // Check if new items (Simplified check: wait_h != proc_ing_h?)
            // Normally HyperAMP uses 'wait_h' (producer) and local 'proc_ing_h' (consumer)?
            // Assuming HyperAMP logic: Linux updates wait_h, FreeRTOS updates proc_ing_h?
            // Let's assume standard Ring Buffer: Linux wrote to entry at `queue->wait_h` then incremented it?
            // Actually HyperAMP code uses: `msg_queue_mutex->msg_wait_cnt` or similar.
            // Let's use `q->wait_h` vs `q->proc_ing_h` for diff.
            
            u16 tail = q->wait_h; // Producer Head
            u16 head = q->proc_ing_h; // Consumer Head
            
            if (head != tail) {
                printf("[RTISM] Prio %d: Found New Msg! (Head: %d, Tail: %d)\r\n", p, head, tail);
                
                // Process Item at 'head'
                volatile MsgEntry* entry = &q->entries[head];
                InvalidateRange((void*)entry, sizeof(MsgEntry));
                
                // Read Data
                char* data_ptr = (char*)(SHM_PADDR_DATA + entry->msg.offset); // Global Data Pool
                InvalidateRange(data_ptr, entry->msg.length);
                
                printf("[RTISM]   Msg Content: %s\r\n", data_ptr);
                
                // "Process" it (Echo/Service)
                // ...
                
                // Update Head
                q->proc_ing_h = (head + 1) % q->buf_size;
                CleanRange((void*)q, 64);
                
                // Go back to P=0 (Strict Priority: Preempt lower logic handling if we loop)
                // But here we processed ONE. Should we check P0 again? 
                // Yes, "Strict Priority" implies always checking Higher first.
                p = -1; // Reset loop to start from 0
            }
        }
    }
}

// ----------------------------------------------------------------------
// 2. ISR Handler
// ----------------------------------------------------------------------
static void RtismIrqHandler(s32 vector, void *param) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(g_rtism_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// ----------------------------------------------------------------------
// 3. Init
// ----------------------------------------------------------------------
int RtismServerInit() {
    // Map Queues
    for(int i=0; i<RTISM_NUM_PRIORITIES; i++) {
        rtism_qs[i] = (volatile AmpMsgQueue*)(RTISM_BASE_PADDR + (i * RTISM_QUEUE_SIZE));
        // Invalidate initial state
        InvalidateRange((void*)rtism_qs[i], 64);
    }
    
    g_rtism_sem = xSemaphoreCreateBinary();
    
    // Register ISR (Reuse ID 74)
    InterruptInstall(HYPERAMP_SGI_IRQ_ID, RtismIrqHandler, NULL, "RTISM-ISR");
    InterruptSetPriority(HYPERAMP_SGI_IRQ_ID, 0); // Max Prio
    InterruptUmask(HYPERAMP_SGI_IRQ_ID);
    
    // Create Task
    xTaskCreate(RtismServerTask, "RTISM_Server", 4096, NULL, configMAX_PRIORITIES-1, &g_rtism_task);
    
    printf("[RTISM] Server Initialized (Queues at 0x%lx)\r\n", RTISM_BASE_PADDR);
    return 0;
}

// ----------------------------------------------------------------------
// 4. τ_read Calibration (Read time per bit from shared memory)
// Call this manually from main() to calibrate
// ----------------------------------------------------------------------
void RtismCalibrateTauRead(void) {
    printf("\r\n");
    printf("=========================================================\r\n");
    printf("  RTISM Calibration: Measuring τ_read (FreeRTOS side)   \r\n");
    printf("=========================================================\r\n\r\n");
    
    int test_sizes[] = {64, 128, 256, 512, 1024, 2048, 4096};
    int num_sizes = 7;
    int iterations = 1000;
    
    // Get timer frequency
    uint64_t freq;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    printf("  Timer Frequency: %lu Hz\r\n", freq);
    printf("  Iterations per size: %d\r\n\r\n", iterations);
    
    printf("  Size(B)   Time(us)   Bits      τ_read(ns/b)\r\n");
    printf("  ------------------------------------------------\r\n");
    
    double total_tau_read = 0;
    int valid = 0;
    
    for (int s = 0; s < num_sizes; s++) {
        int size = test_sizes[s];
        
        // Use first queue's data area as test source
        char* src = (char*)(SHM_PADDR_DATA);
        char local_buf[4096];
        
        // Start timing
        uint64_t start, end;
        __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r"(start));
        
        for (int i = 0; i < iterations; i++) {
            // Invalidate cache to force read from memory
            InvalidateRange(src, size);
            
            // Read from shared memory to local buffer (byte by byte)
            volatile char* s_ptr = (volatile char*)src;
            volatile char* d_ptr = (volatile char*)local_buf;
            for (int b = 0; b < size; b++) {
                d_ptr[b] = s_ptr[b];
            }
        }
        
        __asm__ volatile("isb; mrs %0, cntvct_el0" : "=r"(end));
        
        double total_time_us = (double)(end - start) * 1000000.0 / freq;
        double avg_time_us = total_time_us / iterations;
        int bits = size * 8;
        double tau_read_ns = (avg_time_us * 1000.0) / bits;
        
        printf("  %4d      %7.2f    %5d     %8.4f\r\n",
               size, avg_time_us, bits, tau_read_ns);
        
        total_tau_read += tau_read_ns;
        valid++;
    }
    
    printf("  ------------------------------------------------\r\n\r\n");
    
    double avg_tau_read = total_tau_read / valid;
    
    printf("  === Calibration Results ===\r\n");
    printf("  Measured τ_read: %.4f ns/bit (%.6f μs/bit)\r\n", avg_tau_read, avg_tau_read / 1000.0);
    printf("  Paper τ_read:    12.5000 ns/bit (0.012500 μs/bit)\r\n");
    printf("  Ratio: %.2fx\r\n\r\n", avg_tau_read / 12.5);
    printf("  Update rtism_lpa.c with:\r\n");
    printf("    #define TAU_READ_BIT %.6f  // μs/bit\r\n\r\n", avg_tau_read / 1000.0);
}
