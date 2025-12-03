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
 * FilePath: hyperamp_server.h
 * Description: HyperAMP Server for FreeRTOS - Cross-VM communication using shared memory
 */

#ifndef HYPERAMP_SERVER_H
#define HYPERAMP_SERVER_H

#include "ftypes.h"
#include "FreeRTOS.h"  /* 需要 BaseType_t 定义 */

#ifdef __cplusplus
extern "C" {
#endif

/* 共享内存物理地址定义 (与 hvisor 配置一致) */
#define SHM_PADDR_DATA      0xDE000000UL  /* 数据缓冲区 */
#define SHM_SIZE_DATA       0x00400000UL  /* 4MB */
#define SHM_PADDR_ROOT_Q    0xDE400000UL  /* Root Linux 队列 */
#define SHM_PADDR_FREERTOS_Q 0xDE410000UL /* FreeRTOS 队列 (对应 seL4 队列地址) */
#define SHM_PAGE_SIZE       0x1000UL      /* 4KB */

/* HyperAMP SGI 中断号 */
#define HYPERAMP_SGI_IRQ_ID    74         /* hvisor 发送的软中断 */

/* 消息队列初始化标记 */
#define MSG_QUEUE_MARK_BUSY     (0xFFFFFFFFU) /* 负责处理该消息队列的核心正在处理（16位） */
#define INIT_MARK_INITIALIZED  (0xEEEEEEEEU)
#define MSG_QUEUE_MARK_IDLE    (0xBBBBBBBBU)

/* 消息处理状态 - 与 Linux 端保持一致 */
#define MSG_DEAL_STATE_NO      (0)  /* 消息还未被处理 */
#define MSG_DEAL_STATE_YES     (1)  /* 消息已被处理 */

/* 服务处理结果 - 与 Linux 端保持一致 */
#define MSG_SERVICE_RET_NONE       (0)  /* 消息还未被处理 */
#define MSG_SERVICE_RET_SUCCESS    (1)  /* 服务正确响应 */
#define MSG_SERVICE_RET_FAIL       (2)  /* 服务未曾正确服务,或参数错误等 */
#define MSG_SERVICE_RET_NOT_EXITS  (3)  /* 请求的服务不存在 */
#define MSG_SERVICE_RET_WAIT       (4)  /* 被引入用户态,等待处理 */

/* 服务 ID 定义 */
#define SERVICE_ECHO_ID        (66U)
#define SERVICE_ENCRYPT_ID     (1U)
#define SERVICE_DECRYPT_ID     (2U)

/* 消息标志结构 - 必须与 Linux 端完全一致! */
typedef struct {
    u16 deal_state : 1;      /* 1位: 消息是否被处理 */
    u16 service_result : 2;  /* 2位: 消息对应的服务是否被正确服务 */
} MsgFlag;

/* 消息结构 - 必须与 Linux 端字段顺序和类型完全一致! */
typedef struct {
    MsgFlag flag;        /* 消息标志 (2字节) */
    u16 service_id;      /* 服务 ID (2字节) */
    u32 offset;          /* 数据偏移 (4字节) */
    u32 length;          /* 数据长度 (4字节) */
} __attribute__((aligned(4))) Msg;  /*4字节对齐，总共12字节 */

/* 添加 cur_idx 字段! */
typedef struct MsgEntry {
    Msg msg;             /* 消息实体 (12字节) */
    u16 cur_idx;         /* 当前消息的索引 (2字节) - 之前被错误删除! */
    u16 nxt_idx;         /* 下一个消息的索引 (2字节) */
} __attribute__((aligned(4))) MsgEntry;  /* 4字节对齐，总共16字节 */

/* AMP 消息队列结构 - 必须与Linux端完全一致! */
typedef struct {
    volatile u32 working_mark;    /* 工作标记 (4字节) */
    u16 buf_size;                 /* 缓冲区大小 (2字节) */
    volatile u16 empty_h;         /* 空闲队列头 (2字节) */
    volatile u16 wait_h;          /* 等待队列头 (2字节) */
    volatile u16 proc_ing_h;      /* 处理中队列头 (2字节) */
    MsgEntry entries[0];          /* 柔性数组 - 实际存放的消息 */
} __attribute__((aligned(4))) AmpMsgQueue;  /* 队列头共12字节，4字节对齐 */

/**
 * @brief 初始化 HyperAMP 服务端
 * 
 * 初始化共享内存映射和消息队列
 * 
 * @return 0 成功, -1 失败
 */
int HyperAmpServerInit(void);

/**
 * @brief 创建 HyperAMP 服务端任务
 * 
 * 创建一个 FreeRTOS 任务来处理来自 Root Linux 的消息
 * 
 * @return pdPASS 成功, pdFAIL 失败
 */
BaseType_t HyperAmpServerCreateTask(void);

/**
 * @brief 获取共享内存状态
 * 
 * 打印当前共享内存和消息队列的状态信息
 */
void HyperAmpServerGetStatus(void);

/**
 * @brief 停止 HyperAMP 服务端
 * 
 * 停止消息处理任务
 */
void HyperAmpServerStop(void);

#ifdef __cplusplus
}
#endif

#endif /* HYPERAMP_SERVER_H */
