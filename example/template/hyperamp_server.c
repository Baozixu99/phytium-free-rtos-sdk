/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * FilePath: hyperamp_server.c
 * Description: HyperAMP Server implementation for FreeRTOS
 */

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hyperamp_server.h"
#include "fcache.h"  /* 用于缓存操作 */
#include "finterrupt.h"  /* 中断管理,包含 GIC 支持 */

/* 共享内存虚拟地址指针 */
static volatile char *g_data_vaddr = NULL;
static volatile AmpMsgQueue *g_root_q_vaddr = NULL;
static volatile AmpMsgQueue *g_freertos_q_vaddr = NULL;

/* 服务端运行状态 */
static volatile int g_server_running = 0;
static int g_message_count = 0;

/* FreeRTOS 任务句柄 */
static TaskHandle_t g_server_task_handle = NULL;

/* 二值信号量,用于中断和任务之间的同步 */
static SemaphoreHandle_t g_hyperamp_sem = NULL;

/* 消息缓存失效 - 读取前使缓存失效,确保从主内存读取最新数据 */
static inline void CacheInvalidateMessage(volatile Msg *msg)
{
    unsigned long msg_addr = (unsigned long)msg;
    
    /* 使缓存行失效,强制从主内存读取 (DC IVAC) */
    __asm__ volatile("dc ivac, %0" :: "r"(msg_addr) : "memory");
    
    /* 确保失效操作完成 */
    __asm__ volatile("dsb sy" ::: "memory");
}

/* 消息缓存同步 - 写入后将消息写回主内存,确保其他核可见 */
static inline void CacheSyncMessage(volatile Msg *msg)
{
    unsigned long msg_addr = (unsigned long)msg;
    
    /* 确保写入完成 */
    __asm__ volatile("dsb sy" ::: "memory");
    
    /* 将消息缓存行写回主内存 (sizeof(Msg)=12字节,只需清理1个缓存行) */
    __asm__ volatile("dc cvac, %0" :: "r"(msg_addr) : "memory");
    
    /* 确保对其他核可见 */
    __asm__ volatile("dsb sy" ::: "memory");
}

/* XOR 加密服务 */
static int ServiceEncrypt(char *data, int data_len, int buf_size)
{
    if (!data || data_len <= 0 || buf_size <= data_len) {
        return -1;
    }
    
    /* 简单的 XOR 加密，密钥为 0x5A */
    for (int i = 0; i < data_len; i++) {
        data[i] ^= 0x5A;
    }
    
    return 0;
}

/* XOR 解密服务 (与加密相同) */
static int ServiceDecrypt(char *data, int data_len, int buf_size)
{
    return ServiceEncrypt(data, data_len, buf_size);
}

/* HyperAMP SGI 中断处理函数 */
static void HyperAmpIrqHandler(s32 vector, void *param)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    printf("[HyperAMP-IRQ] Interrupt %d triggered!\r\n", vector);
    
    /* 短暂延迟,给Linux缓存刷新时间 */
    for (volatile int i = 0; i < 1000; i++);
    
    /* 在中断中释放信号量,唤醒处理任务 */
    if (g_hyperamp_sem != NULL) {
        xSemaphoreGiveFromISR(g_hyperamp_sem, &xHigherPriorityTaskWoken);
    } else {
        printf("[HyperAMP-IRQ] ERROR: Semaphore is NULL!\r\n");
    }
    
    printf("[HyperAMP-IRQ] Semaphore released\r\n");
    
    /* 如果需要进行上下文切换 */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* 初始化 Root Linux 队列 */
static void InitRootLinuxQueue(void)
{
    if (!g_root_q_vaddr) {
        printf("[HyperAMP] Root queue not available\r\n");
        return;
    }
    
    printf("[HyperAMP] Checking Root Linux queue...\r\n");
    
    /* 如果 Root Linux 队列未初始化，帮助初始化 */
    if (g_root_q_vaddr->working_mark != INIT_MARK_INITIALIZED) {
        printf("[HyperAMP] Initializing Root Linux queue\r\n");
        g_root_q_vaddr->working_mark = MSG_QUEUE_MARK_IDLE;
        g_root_q_vaddr->buf_size = 16;
        g_root_q_vaddr->empty_h = 0;
        g_root_q_vaddr->wait_h = 0;
        g_root_q_vaddr->proc_ing_h = 0;
        printf("[HyperAMP] Root Linux queue initialized\r\n");
    } else {
        printf("[HyperAMP] Root Linux queue already initialized\r\n");
    }
}

/* 初始化 FreeRTOS 队列 */
static void InitFreeRTOSQueue(void)
{
    if (!g_freertos_q_vaddr) {
        printf("[HyperAMP] FreeRTOS queue not available\r\n");
        return;
    }
    
    printf("[HyperAMP] Initializing FreeRTOS queue...\r\n");
    
    g_freertos_q_vaddr->working_mark = INIT_MARK_INITIALIZED;
    g_freertos_q_vaddr->buf_size = 16;
    g_freertos_q_vaddr->empty_h = 0;
    g_freertos_q_vaddr->wait_h = 0;
    g_freertos_q_vaddr->proc_ing_h = 0;
    
    /* 强制将队列头写回主内存,让 Linux 能检测到初始化完成 */
    unsigned long queue_addr = (unsigned long)g_freertos_q_vaddr;
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("dc cvac, %0" :: "r"(queue_addr) : "memory");
    __asm__ volatile("dsb sy" ::: "memory");
    
    printf("[HyperAMP] FreeRTOS queue initialized (mark=0x%x)\r\n", 
           INIT_MARK_INITIALIZED);
}

/* 处理来自 Root Linux 的消息 */
static int ProcessRootLinuxMessage(void)
{
    if (!g_root_q_vaddr || !g_data_vaddr) {
        printf("[HyperAMP-DEBUG] Queue or data not available\r\n");
        return 0;
    }
    
    /* 获取当前处理队列头 */
    unsigned long queue_addr = (unsigned long)g_root_q_vaddr;
    unsigned long queue_end = queue_addr + sizeof(AmpMsgQueue);
    
    /* 刷新整个队列头结构的缓存 */
    for (unsigned long addr = queue_addr; addr < queue_end; addr += 64) {
        __asm__ volatile("dc civac, %0" :: "r"(addr) : "memory");
    }
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("isb" ::: "memory");
    
    /* 延迟,给缓存刷新时间 */
    for (volatile int i = 0; i < 100; i++);
    
    u16 current_proc_head = g_root_q_vaddr->proc_ing_h;
    
    printf("[HyperAMP-DEBUG] proc_ing_h=%u, buf_size=%u, working_mark=0x%x\r\n",
           current_proc_head, g_root_q_vaddr->buf_size, g_root_q_vaddr->working_mark);
    
    /* 检查队列头是否有效 */
    if (current_proc_head >= g_root_q_vaddr->buf_size) {
        printf("[HyperAMP-DEBUG] Invalid proc_ing_h: %u >= %u\r\n", 
               current_proc_head, g_root_q_vaddr->buf_size);
        return 0;
    }
    
    /* ✅ 正确方式: 直接通过柔性数组访问消息实体 */
    volatile MsgEntry *msg_entry = &g_root_q_vaddr->entries[current_proc_head];
    volatile Msg *msg = &msg_entry->msg;
    
    /* 读取消息前刷新消息entry缓存 */
    unsigned long entry_addr = (unsigned long)msg_entry;
    unsigned long entry_end = entry_addr + sizeof(MsgEntry);
    
    /* 按缓存行对齐刷新整个entry */
    unsigned long entry_aligned_start = entry_addr & ~0x3FUL;
    unsigned long entry_aligned_end = (entry_end + 63) & ~0x3FUL;
    for (unsigned long addr = entry_aligned_start; addr < entry_aligned_end; addr += 64) {
        __asm__ volatile("dc civac, %0" :: "r"(addr) : "memory");
    }
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("isb" ::: "memory");
    
    /* 延迟,给缓存刷新时间 */
    for (volatile int i = 0; i < 100; i++);
    
    printf("[HyperAMP-DEBUG] Message: length=%u, offset=0x%x, deal_state=%u, service_id=%u\r\n",
           msg->length, msg->offset, msg->flag.deal_state, msg->service_id);
    
    /* 检查是否为有效消息 */
    if (msg->length == 0 || msg->length >= SHM_SIZE_DATA || 
        msg->offset >= SHM_SIZE_DATA || msg->flag.deal_state == MSG_DEAL_STATE_YES) {
        printf("[HyperAMP-DEBUG] Invalid message: ");
        if (msg->length == 0) printf("length==0 ");
        if (msg->length >= SHM_SIZE_DATA) printf("length>=SHM_SIZE ");
        if (msg->offset >= SHM_SIZE_DATA) printf("offset>=SHM_SIZE ");
        if (msg->flag.deal_state == MSG_DEAL_STATE_YES) printf("already_processed ");
        printf("\r\n");
        return 0;
    }
    
    /* 找到有效消息 */
    g_message_count++;
    printf("\r\n[HyperAMP] *** MESSAGE #%d FROM ROOT LINUX ***\r\n", g_message_count);
    printf("[HyperAMP]   Index: %u, Service ID: %u\r\n", current_proc_head, msg->service_id);
    printf("[HyperAMP]   Offset: 0x%x, Length: %u\r\n", msg->offset, msg->length);
    printf("[HyperAMP]   DEBUG: flag.deal_state=%u, flag.service_result=%u\r\n", 
           msg->flag.deal_state, msg->flag.service_result);
    printf("[HyperAMP]   DEBUG: msg addr=%p, entry addr=%p, nxt_idx=%u\r\n",
           (void*)msg, (void*)msg_entry, msg_entry->nxt_idx);
    
    /* 获取数据指针 */
    volatile char *data_ptr = g_data_vaddr + msg->offset;
    
    /* 刷新数据缓冲区缓存 */
    unsigned long data_start = (unsigned long)data_ptr;
    unsigned long data_end = data_start + msg->length;
    
    /* 按缓存行对齐刷新数据 */
    unsigned long data_aligned_start = data_start & ~0x3FUL;
    unsigned long data_aligned_end = (data_end + 63) & ~0x3FUL;
    for (unsigned long addr = data_aligned_start; addr < data_aligned_end; addr += 64) {
        __asm__ volatile("dc civac, %0" :: "r"(addr) : "memory");
    }
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("isb" ::: "memory");
    
    /* 显示接收到的数据 */
    printf("[HyperAMP]   Data: [");
    int display_len = (msg->length > 64) ? 64 : msg->length;
    
    /* 检查是否包含结尾符,如果有则忽略 */
    int actual_len = msg->length;
    if (actual_len > 0 && data_ptr[actual_len - 1] == '\0') {
        actual_len--;
        printf("[HyperAMP]   WARNING: Message includes null terminator, adjusting length %u -> %u\r\n", 
               msg->length, actual_len);
    }
    
    for (int i = 0; i < display_len; i++) {
        char c = data_ptr[i];
        if (c >= 32 && c <= 126) {
            printf("%c", c);
        } else if (c == '\0') {
            break;
        } else {
            printf("\\x%02x", (unsigned char)c);
        }
    }
    if (msg->length > 64) printf("...");
    printf("]\r\n");
    
    /* 处理服务请求 */
    int service_result = MSG_SERVICE_RET_SUCCESS;
    int data_modified = 0;
    
    switch (msg->service_id) {
        case SERVICE_ENCRYPT_ID:
            printf("[HyperAMP]   Service: ENCRYPTION\r\n");
            if (ServiceEncrypt((char *)data_ptr, actual_len, SHM_SIZE_DATA - msg->offset) == 0) {
                printf("[HyperAMP]   Encryption completed\r\n");
                data_modified = 1;
            } else {
                printf("[HyperAMP]   Encryption failed\r\n");
                service_result = MSG_SERVICE_RET_FAIL;
            }
            break;
            
        case SERVICE_DECRYPT_ID:
            printf("[HyperAMP]   Service: DECRYPTION\r\n");
            if (ServiceDecrypt((char *)data_ptr, actual_len, SHM_SIZE_DATA - msg->offset) == 0) {
                printf("[HyperAMP]   Decryption completed\r\n");
                data_modified = 1;
            } else {
                printf("[HyperAMP]   Decryption failed\r\n");
                service_result = MSG_SERVICE_RET_FAIL;
            }
            break;
            
        case SERVICE_ECHO_ID:
            printf("[HyperAMP]   Service: ECHO TEST\r\n");
            break;
            
        default:
            printf("[HyperAMP]   Service: ECHO (ID: %u)\r\n", msg->service_id);
            break;
    }
    
    /* 如果数据被修改，显示处理后的结果 */
    if (data_modified) {
        /* 将修改后的数据写回主内存 */
        unsigned long data_write_start = (unsigned long)data_ptr;
        unsigned long data_write_end = data_write_start + actual_len;
        __asm__ volatile("dsb sy" ::: "memory");
        for (unsigned long addr = data_write_start; addr < data_write_end; addr += 64) {
            __asm__ volatile("dc cvac, %0" :: "r"(addr) : "memory");
        }
        __asm__ volatile("dsb sy" ::: "memory");
        
        printf("[HyperAMP]   Result: [");
        int result_display = (actual_len > 64) ? 64 : actual_len;
        for (int i = 0; i < result_display; i++) {
            char c = data_ptr[i];
            if (c >= 32 && c <= 126) {
                printf("%c", c);
            } else {
                printf("\\x%02x", (unsigned char)c);
            }
        }
        if (actual_len > 64) printf("...");
        printf("]\r\n");
    }
    
    /* 标记消息已处理并立即同步到主内存 */
    msg->flag.deal_state = MSG_DEAL_STATE_YES;
    msg->flag.service_result = service_result;
    CacheSyncMessage(msg);
    
    /* 更新队列头并写回主内存 */
    u16 new_head;
    if (msg_entry->nxt_idx < g_root_q_vaddr->buf_size) {
        new_head = msg_entry->nxt_idx;
    } else {
        new_head = (current_proc_head + 1) % g_root_q_vaddr->buf_size;
    }
    
    g_root_q_vaddr->proc_ing_h = new_head;
    g_root_q_vaddr->working_mark = MSG_QUEUE_MARK_IDLE;
    
    /* 将队列头更新写回主内存 */
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("dc cvac, %0" :: "r"(queue_addr) : "memory");
    __asm__ volatile("dsb sy" ::: "memory");
    
    printf("[HyperAMP]   *** Message processed successfully! ***\r\n");
    
    return 1;
}

/* HyperAMP 服务端任务 */
static void HyperAmpServerTask(void *pvParameters)
{
    printf("\r\n[HyperAMP] === Server Task Started (Interrupt-Driven Mode) ===\r\n");
    printf("[HyperAMP] Waiting for IRQ %d from Root Linux...\r\n", HYPERAMP_SGI_IRQ_ID);
    
    /* 初始化队列 */
    InitRootLinuxQueue();
    InitFreeRTOSQueue();
    
    printf("[HyperAMP] Task entering main loop, waiting for semaphore...\r\n");
    
    /* 主消息处理循环 - 等待中断触发 */
    while (g_server_running) {
        /* 阻塞等待信号量,直到中断触发 */
        printf("[HyperAMP] Blocking on semaphore (waiting for interrupt)...\r\n");
        if (xSemaphoreTake(g_hyperamp_sem, portMAX_DELAY) == pdTRUE) {
            printf("[HyperAMP] Semaphore acquired! Processing messages......\r\n");
            
            /* 收到中断信号,处理所有待处理的消息 */
            int processed_count = 0;
            
            /* 可能有多个消息排队,全部处理完 */
            while (ProcessRootLinuxMessage()) {
                processed_count++;
            }
            
            if (processed_count > 0) {
                printf("[HyperAMP] Processed %d message(s) in this interrupt\r\n", processed_count);
            } else {
                printf("[HyperAMP] No messages to process (spurious wakeup?)\r\n");
            }
        }
    }
    
    printf("\r\n[HyperAMP] === Server Task Stopped ===\r\n");
    printf("[HyperAMP] Total messages processed: %d\r\n", g_message_count);
    
    /* 删除任务自身 */
    g_server_task_handle = NULL;
    vTaskDelete(NULL);
}

/* 初始化 HyperAMP 服务端 */
int HyperAmpServerInit(void)
{
    printf("[HyperAMP] Initializing server...\r\n");
    
    /* 映射共享内存到虚拟地址 */
    g_data_vaddr = (volatile char *)SHM_PADDR_DATA;
    g_root_q_vaddr = (volatile AmpMsgQueue *)SHM_PADDR_ROOT_Q;
    g_freertos_q_vaddr = (volatile AmpMsgQueue *)SHM_PADDR_FREERTOS_Q;
    
    printf("[HyperAMP] Shared memory mapped:\r\n");
    printf("[HyperAMP]   Data buffer:    0x%lx (vaddr: %p)\r\n", 
           (unsigned long)SHM_PADDR_DATA, (void *)g_data_vaddr);
    printf("[HyperAMP]   Root queue:     0x%lx (vaddr: %p)\r\n", 
           (unsigned long)SHM_PADDR_ROOT_Q, (void *)g_root_q_vaddr);
    printf("[HyperAMP]   FreeRTOS queue: 0x%lx (vaddr: %p)\r\n", 
           (unsigned long)SHM_PADDR_FREERTOS_Q, (void *)g_freertos_q_vaddr);
    
    /* 测试访问共享内存 */
    printf("[HyperAMP] Testing shared memory access...\r\n");
    
    /* 清空测试区域,避免残留数据干扰 */
    memset((void *)g_data_vaddr, 0, 64);
    
    /* 创建二值信号量 */
    g_hyperamp_sem = xSemaphoreCreateBinary();
    if (g_hyperamp_sem == NULL) {
        printf("[HyperAMP] ERROR: Failed to create semaphore\r\n");
        return -1;
    }
    printf("[HyperAMP] Semaphore created\r\n");
    
    /* 注册 SGI 中断处理函数 */
    InterruptInstall(HYPERAMP_SGI_IRQ_ID, 
                     HyperAmpIrqHandler, 
                     NULL, 
                     "HyperAMP-SGI");
    printf("[HyperAMP] Interrupt handler installed for IRQ %d\r\n", HYPERAMP_SGI_IRQ_ID);
    
    /* 设置中断优先级 */
    InterruptSetPriority(HYPERAMP_SGI_IRQ_ID, 0);  /* 最高优先级 */
    
    /* 使能中断 */
    InterruptUmask(HYPERAMP_SGI_IRQ_ID);
    printf("[HyperAMP] Interrupt %d enabled\r\n", HYPERAMP_SGI_IRQ_ID);
    
    /* 验证结构体大小 */
    printf("[HyperAMP] Structure size verification:\r\n");
    printf("[HyperAMP]   sizeof(Msg) = %lu (expected: 12)\r\n", sizeof(Msg));
    printf("[HyperAMP]   sizeof(MsgEntry) = %lu (expected: 16)\r\n", sizeof(MsgEntry));
    printf("[HyperAMP]   sizeof(AmpMsgQueue) = %lu (expected: 12)\r\n", sizeof(AmpMsgQueue));
    
    /* 验证偏移量 */
    if (g_root_q_vaddr != NULL) {
        printf("[HyperAMP] Offset verification:\r\n");
        printf("[HyperAMP]   &entries[0] offset = %ld (expected: 12)\r\n",
               (char*)&g_root_q_vaddr->entries[0] - (char*)g_root_q_vaddr);
        printf("[HyperAMP]   &entries[1] offset = %ld (expected: 28)\r\n",
               (char*)&g_root_q_vaddr->entries[1] - (char*)g_root_q_vaddr);
    }
    
    printf("[HyperAMP] Initialization complete\r\n");
    
    return 0;
}

/* 创建 HyperAMP 服务端任务 */
BaseType_t HyperAmpServerCreateTask(void)
{
    if (g_server_task_handle != NULL) {
        printf("[HyperAMP] Server task already running\r\n");
        return pdPASS;
    }
    
    g_server_running = 1;
    g_message_count = 0;
    
    /* 创建服务端任务 */
    BaseType_t xReturn = xTaskCreate(
        HyperAmpServerTask,           /* 任务函数 */
        "HyperAMP_Server",             /* 任务名称 */
        4096,                          /* 栈大小 (字) */
        NULL,                          /* 任务参数 */
        configMAX_PRIORITIES - 2,      /* 任务优先级 (高优先级) */
        &g_server_task_handle          /* 任务句柄 */
    );
    
    if (xReturn == pdPASS) {
        printf("[HyperAMP] Server task created successfully\r\n");
    } else {
        printf("[HyperAMP] Failed to create server task\r\n");
        g_server_running = 0;
    }
    
    return xReturn;
}

/* 获取共享内存状态 */
void HyperAmpServerGetStatus(void)
{
    if (!g_root_q_vaddr || !g_freertos_q_vaddr) {
        printf("[HyperAMP] Shared memory not initialized\r\n");
        return;
    }
    
    printf("\r\n[HyperAMP] === Shared Memory Status ===\r\n");
    
    printf("[HyperAMP] Root Linux queue:\r\n");
    printf("[HyperAMP]   working_mark = 0x%x\r\n", g_root_q_vaddr->working_mark);
    printf("[HyperAMP]   buf_size = %u\r\n", g_root_q_vaddr->buf_size);
    printf("[HyperAMP]   empty_h = %u\r\n", g_root_q_vaddr->empty_h);
    printf("[HyperAMP]   wait_h = %u\r\n", g_root_q_vaddr->wait_h);
    printf("[HyperAMP]   proc_ing_h = %u\r\n", g_root_q_vaddr->proc_ing_h);
    
    printf("[HyperAMP] FreeRTOS queue:\r\n");
    printf("[HyperAMP]   working_mark = 0x%x\r\n", g_freertos_q_vaddr->working_mark);
    printf("[HyperAMP]   buf_size = %u\r\n", g_freertos_q_vaddr->buf_size);
    printf("[HyperAMP]   empty_h = %u\r\n", g_freertos_q_vaddr->empty_h);
    printf("[HyperAMP]   wait_h = %u\r\n", g_freertos_q_vaddr->wait_h);
    printf("[HyperAMP]   proc_ing_h = %u\r\n", g_freertos_q_vaddr->proc_ing_h);
    
    printf("[HyperAMP] Server status:\r\n");
    printf("[HyperAMP]   Running: %s\r\n", g_server_running ? "YES" : "NO");
    printf("[HyperAMP]   Messages processed: %d\r\n", g_message_count);
    printf("[HyperAMP] ===============================\r\n");
}

/* 停止 HyperAMP 服务端 */
void HyperAmpServerStop(void)
{
    if (!g_server_running) {
        printf("[HyperAMP] Server not running\r\n");
        return;
    }
    
    printf("[HyperAMP] Stopping server...\r\n");
    g_server_running = 0;
    
    /* 释放信号量,让任务退出阻塞状态 */
    if (g_hyperamp_sem != NULL) {
        xSemaphoreGive(g_hyperamp_sem);
    }
    
    /* 等待任务退出 */
    while (g_server_task_handle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    /* 禁用中断 */
    InterruptMask(HYPERAMP_SGI_IRQ_ID);
    printf("[HyperAMP] Interrupt %d disabled\r\n", HYPERAMP_SGI_IRQ_ID);
    
    /* 删除信号量 */
    if (g_hyperamp_sem != NULL) {
        vSemaphoreDelete(g_hyperamp_sem);
        g_hyperamp_sem = NULL;
        printf("[HyperAMP] Semaphore deleted\r\n");
    }
    
    printf("[HyperAMP] Server stopped\r\n");
}
