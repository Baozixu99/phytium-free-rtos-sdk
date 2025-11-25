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
#include "hyperamp_server.h"
#include "fcache.h"  /* 用于缓存操作 */

/* 共享内存虚拟地址指针 */
static volatile char *g_data_vaddr = NULL;
static volatile AmpMsgQueue *g_root_q_vaddr = NULL;
static volatile AmpMsgQueue *g_freertos_q_vaddr = NULL;

/* 服务端运行状态 */
static volatile int g_server_running = 0;
static int g_message_count = 0;

/* FreeRTOS 任务句柄 */
static TaskHandle_t g_server_task_handle = NULL;

/* 缓存同步函数 - 确保共享内存的缓存一致性 */
static void CacheSyncSharedMemory(void)
{
    if (!g_data_vaddr || !g_root_q_vaddr || !g_freertos_q_vaddr) {
        return;
    }
    
    /* ARM64 缓存管理：数据同步屏障 */
    __asm__ volatile("dsb sy" ::: "memory");
    
    /* 清理并失效数据缓冲区的缓存 (按 64 字节缓存行) */
    unsigned long data_start = (unsigned long)g_data_vaddr;
    for (unsigned long addr = data_start; addr < data_start + SHM_SIZE_DATA; addr += 64) {
        __asm__ volatile("dc civac, %0" :: "r"(addr) : "memory");
    }
    
    /* 清理 Root 队列缓存 */
    unsigned long root_q_start = (unsigned long)g_root_q_vaddr;
    for (unsigned long addr = root_q_start; addr < root_q_start + SHM_PAGE_SIZE; addr += 64) {
        __asm__ volatile("dc civac, %0" :: "r"(addr) : "memory");
    }
    
    /* 清理 FreeRTOS 队列缓存 */
    unsigned long freertos_q_start = (unsigned long)g_freertos_q_vaddr;
    for (unsigned long addr = freertos_q_start; addr < freertos_q_start + SHM_PAGE_SIZE; addr += 64) {
        __asm__ volatile("dc civac, %0" :: "r"(addr) : "memory");
    }
    
    /* 最终的内存屏障 */
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("isb" ::: "memory");
}

/* 精确的消息缓存同步 - 只同步单个消息相关的缓存行 */
static void CacheSyncMessage(volatile Msg *msg)
{
    if (!msg) {
        return;
    }
    
    /* 同步消息结构本身 */
    unsigned long msg_addr = (unsigned long)msg;
    unsigned long msg_end = msg_addr + sizeof(Msg);
    
    /* 第一步: DSB 确保所有写入完成 */
    __asm__ volatile("dsb sy" ::: "memory");
    
    /* 第二步: 清理缓存到主内存 (DC CVAC) */
    for (unsigned long addr = msg_addr; addr < msg_end; addr += 64) {
        __asm__ volatile("dc cvac, %0" :: "r"(addr) : "memory");
    }
    
    /* 第三步: DSB 确保清理操作完成 */
    __asm__ volatile("dsb sy" ::: "memory");
    
    /* 第四步: 使缓存行失效,强制其他核重新从主内存读取 (DC CIVAC) */
    for (unsigned long addr = msg_addr; addr < msg_end; addr += 64) {
        __asm__ volatile("dc civac, %0" :: "r"(addr) : "memory");
    }
    
    /* 第五步: 最终的屏障确保所有操作完成 */
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("isb" ::: "memory");
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
        g_root_q_vaddr->buf_size = 16;  /* 支持 16 个消息 */
        g_root_q_vaddr->empty_h = 0;
        g_root_q_vaddr->wait_h = 0;
        g_root_q_vaddr->proc_ing_h = 0;
        
        /* 强制缓存同步 */
        CacheSyncSharedMemory();
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
    g_freertos_q_vaddr->buf_size = 16;  /* 支持 16 个消息 */
    g_freertos_q_vaddr->empty_h = 0;
    g_freertos_q_vaddr->wait_h = 0;
    g_freertos_q_vaddr->proc_ing_h = 0;
    
    /* 强制缓存同步 */
    CacheSyncSharedMemory();
    printf("[HyperAMP] FreeRTOS queue initialized (mark=0x%x)\r\n", 
           INIT_MARK_INITIALIZED);
}

/* 处理来自 Root Linux 的消息 */
static int ProcessRootLinuxMessage(void)
{
    if (!g_root_q_vaddr || !g_data_vaddr) {
        return 0;
    }
    
    /* 强制缓存同步，确保看到最新数据 */
    CacheSyncSharedMemory();
    
    /* 获取当前处理队列头 */
    u16 current_proc_head = g_root_q_vaddr->proc_ing_h;
    
    /* 检查队列头是否有效 */
    if (current_proc_head >= g_root_q_vaddr->buf_size) {
        return 0;
    }
    
    /* 计算消息实体的起始地址 */
    volatile MsgEntry *msg_entries = (volatile MsgEntry *)((char *)g_root_q_vaddr + sizeof(AmpMsgQueue));
    volatile MsgEntry *msg_entry = &msg_entries[current_proc_head];
    volatile Msg *msg = &msg_entry->msg;
    
    /* 检查是否为有效消息 */
    if (msg->length == 0 || msg->length >= SHM_SIZE_DATA || 
        msg->offset >= SHM_SIZE_DATA || msg->flag.deal_state == MSG_DEAL_STATE_YES) {
        return 0;
    }
    
    /* 找到有效消息 */
    g_message_count++;
    printf("\r\n[HyperAMP] *** MESSAGE #%d FROM ROOT LINUX ***\r\n", g_message_count);
    printf("[HyperAMP]   Index: %u, Service ID: %u\r\n", current_proc_head, msg->service_id);
    printf("[HyperAMP]   Offset: 0x%x, Length: %u\r\n", msg->offset, msg->length);
    
    /* 获取数据指针 */
    volatile char *data_ptr = g_data_vaddr + msg->offset;
    
    /* 显示接收到的数据 */
    printf("[HyperAMP]   Data: [");
    int display_len = (msg->length > 64) ? 64 : msg->length;
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
            if (ServiceEncrypt((char *)data_ptr, msg->length, SHM_SIZE_DATA - msg->offset) == 0) {
                printf("[HyperAMP]   Encryption completed\r\n");
                data_modified = 1;
            } else {
                printf("[HyperAMP]   Encryption failed\r\n");
                service_result = MSG_SERVICE_RET_FAIL;
            }
            break;
            
        case SERVICE_DECRYPT_ID:
            printf("[HyperAMP]   Service: DECRYPTION\r\n");
            if (ServiceDecrypt((char *)data_ptr, msg->length, SHM_SIZE_DATA - msg->offset) == 0) {
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
        printf("[HyperAMP]   Result: [");
        for (int i = 0; i < display_len; i++) {
            char c = data_ptr[i];
            if (c >= 32 && c <= 126) {
                printf("%c", c);
            } else {
                printf("\\x%02x", (unsigned char)c);
            }
        }
        if (msg->length > 64) printf("...");
        printf("]\r\n");
    }
    
    /* 标记消息已处理 */
    msg->flag.deal_state = MSG_DEAL_STATE_YES;
    msg->flag.service_result = service_result;
    
    /* 额外的编译器屏障,防止重排序 */
    __asm__ volatile("" ::: "memory");
    
    /* !!!关键!!! 多次同步以确保可见性 */
    for (int sync_count = 0; sync_count < 3; sync_count++) {
        CacheSyncMessage((volatile Msg *)msg);
    }
    
    /* 同步整个消息实体(包括 nxt_idx) */
    unsigned long entry_addr = (unsigned long)msg_entry;
    unsigned long entry_end = entry_addr + sizeof(MsgEntry);
    __asm__ volatile("dsb sy" ::: "memory");
    for (unsigned long addr = entry_addr; addr < entry_end; addr += 64) {
        __asm__ volatile("dc cvac, %0" :: "r"(addr) : "memory");
    }
    __asm__ volatile("dsb sy" ::: "memory");
    for (unsigned long addr = entry_addr; addr < entry_end; addr += 64) {
        __asm__ volatile("dc civac, %0" :: "r"(addr) : "memory");
    }
    __asm__ volatile("dsb sy" ::: "memory");
    
    /* 打印调试信息 */
    printf("[HyperAMP]   DEBUG: deal_state set to %u, service_result set to %u\r\n",
           msg->flag.deal_state, msg->flag.service_result);
    printf("[HyperAMP]   DEBUG: msg address=%p, entry address=%p\r\n",
           (void*)msg, (void*)msg_entry);
    
    /* 再次读取确认 */
    printf("[HyperAMP]   DEBUG: Verify read back: deal_state=%u\r\n",
           msg->flag.deal_state);
    
    /* 更新队列头 */
    u16 new_head;
    if (msg_entry->nxt_idx < g_root_q_vaddr->buf_size) {
        new_head = msg_entry->nxt_idx;
    } else {
        new_head = (current_proc_head + 1) % g_root_q_vaddr->buf_size;
    }
    
    g_root_q_vaddr->proc_ing_h = new_head;
    g_root_q_vaddr->working_mark = MSG_QUEUE_MARK_IDLE;
    
    /* 再次强制缓存同步整个队列区域 */
    CacheSyncSharedMemory();
    
    printf("[HyperAMP]   *** Message processed successfully! ***\r\n");
    
    return 1;
}

/* HyperAMP 服务端任务 */
static void HyperAmpServerTask(void *pvParameters)
{
    printf("\r\n[HyperAMP] === Server Task Started ===\r\n");
    printf("[HyperAMP] Waiting for messages from Root Linux...\r\n");
    
    /* 初始化队列 */
    InitRootLinuxQueue();
    InitFreeRTOSQueue();
    
    int idle_count = 0;
    int status_report_interval = 10000;  /* 每 10000 次空闲检查报告一次状态 */
    
    /* 主消息处理循环 */
    while (g_server_running) {
        /* 尝试处理消息 */
        int processed = ProcessRootLinuxMessage();
        
        if (processed) {
            /* 处理了消息，重置空闲计数 */
            idle_count = 0;
        } else {
            /* 没有消息，增加空闲计数 */
            idle_count++;
            
            /* 定期显示监控状态 */
            if (idle_count >= status_report_interval) {
                printf("[HyperAMP] Monitoring... (processed: %d messages)\r\n", g_message_count);
                idle_count = 0;
            }
            
            /* 短暂延时，避免过度占用 CPU */
            vTaskDelay(pdMS_TO_TICKS(1));  /* 1ms 延时 */
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
    
    /* 写入测试标记 */
    g_data_vaddr[0] = 'F';
    g_data_vaddr[1] = 'R';
    g_data_vaddr[2] = 'E';
    g_data_vaddr[3] = 'E';
    g_data_vaddr[4] = 'R';
    g_data_vaddr[5] = 'T';
    g_data_vaddr[6] = 'O';
    g_data_vaddr[7] = 'S';
    g_data_vaddr[8] = '\0';
    
    /* 强制缓存同步 */
    CacheSyncSharedMemory();
    
    printf("[HyperAMP] Memory test: wrote '%s'\r\n", (const char *)g_data_vaddr);
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
    
    /* 等待任务退出 */
    while (g_server_task_handle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    printf("[HyperAMP] Server stopped\r\n");
}
