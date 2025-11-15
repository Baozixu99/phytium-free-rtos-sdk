/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * 
 * FilePath: fmsg_common.h
 * Date: 2024-10-08 14:53:42
 * LastEditTime: 2024-10-08 14:53:42
 * Description:  This file is for msg common driver header file.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangzq      2024/10/08  first commit
 */
#ifndef FMSG_COMMON_H
#define FMSG_COMMON_H

/***************************** Include Files *********************************/

#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FMSG_MAX_RING_ITEMS                20
#define FMSG_MAX_RX_RING_ITEMS             10
/************************** Constant Definitions *****************************/
#define FMSG_TX_HEAD                       0x0  /*tx 首指针寄存器*/
#define FMSG_TX_TAIL                       0x4  /*tx 尾指针寄存器*/
#define FMSG_RX_HEAD                       0x8  /*rx 首指针寄存器*/
#define FMSG_RX_TAIL                       0xC  /*rx 尾指针寄存器*/
#define FMSG_RESERVED_HEAD_0               0x10 /*备用首指针寄存器0*/
#define FMSG_RESERVED_TAIL_0               0x14 /*备用尾指针寄存器0*/
#define FMSG_RESERVED_HEAD_1               0x18 /*备用首指针寄存器1*/
#define FMSG_RESERVED_TAIL_1               0x1C /*备用尾指针寄存器1*/
#define FMSG_M2S_INT_MASK                  0x20 /*中断屏蔽寄存器*/
#define FMSG_M2S_INT_STATE                 0x24 /*中断状态寄存器*/
#define FMSG_S2M_INT_MASK                  0x28 /*中断屏蔽寄存器*/
#define FMSG_S2M_INT_STATE                 0x2C /*中断状态寄存器*/
#define FMSG_HARDWARE_PASS_THROUGH_0       0x30 /*硬件通道0*/
#define FMSG_HARDWARE_PASS_THROUGH_1       0x34 /*硬件通道1*/
#define FMSG_HARDWARE_PASS_THROUGH_2       0x38 /*硬件通道2*/
#define FMSG_HARDWARE_PASS_THROUGH_3       0x3c /*硬件通道3*/
#define FMSG_HARDWARE_PASS_THROUGH_4       0x40 /*硬件通道4*/
#define FMSG_HARDWARE_PASS_THROUGH_5       0x44 /*硬件通道5*/
#define FMSG_MEM_RING_SIZE                 0x48 /*mem 和 ring  size 配置寄存器*/
#define FMSG_CUSTOM_REG_0                  0x4C /*自定义寄存器0*/
#define FMSG_CUSTOM_REG_1                  0x50 /*自定义寄存器1*/
#define FMSG_CUSTOM_REG_2                  0x54 /*自定义寄存器2*/
#define FMSG_CUSTOM_REG_3                  0x58 /*自定义寄存器3*/
#define FMSG_AHB_ERROR_REG                 0x5C /*ahb错误寄存器*/
#define FMSG_S2M_INT_CLEAN                 0x74 /*清除中断*/
/*only for slave core*/
#define FMSG_SLAVE_HARDWARE_PASS_THROUGH_0 0x300 /*SLAVE硬件通道0*/
#define FMSG_SLAVE_HARDWARE_PASS_THROUGH_1 0x304 /*SLAVE硬件通道1*/
#define FMSG_SLAVE_HARDWARE_PASS_THROUGH_2 0x308 /*SLAVE硬件通道2*/
#define FMSG_SLAVE_FUZZY_PASS_THROUGH_REG  0x30C /*SLAVE模糊通道*/
/*only for gmac*/
#define FMSG_TAIL_PTR_Q0_REG               0x100
#define FMSG_TAIL_PTR_Q1_REG               0x104
#define FMSG_INT_ENABLE_Q0_REG             0x140
#define FMSG_INT_ENABLE_Q1_REG             0x144
#define FMSG_INT_DISENABLE_Q0_REG          0x180
#define FMSG_INT_DISENABLE_Q1_REG          0x184
#define FMSG_INT_MASK_Q0_REG               0x1C0
#define FMSG_INT_MASK_Q1_REG               0x1C4
#define FMSG_INT_STATUS_Q0_REG             0x200
#define FMSG_INT_STATUS_Q1_REG             0x204
#define FMSG_NETWORK_STATUS_REG            0x240
#define FMSG_PCS_AN_LP_REG                 0x244
#define FMSG_USX_STATUS_REG                0x248
#define FMSG_TSU_TIMER_INCR_SUB_NESC_REG   0x24C
#define FMSG_TSU_TIMER_INCR_REG            0x250
#define FMSG_TSU_TIMER_MSB_SEC_REG         0x254
#define FMSG_TSU_TIMER_SEC_REG             0x258
#define FMSG_TSU_TIMER_NSEC_REG            0x25C
#define FMSG_TSU_TIMER_ADJUST_REG          0x260
#define FMSG_MDIO_REG                      0x264

/** @name FMSG_TX_TAIL 0x04 Register
 */
#define FMSG_TX_TAIL_INT_BIT               BIT(16)

/** @name FMSG_M2S_INT_MASK 0x2c Register
 */
#define FMSG_TX_HEAD_INT_BIT               BIT(0)
#define FMSG_RX_TAIL_INT_BIT               BIT(1)

typedef enum
{
    FMSG_MSG_CMD_DEFAULT = 0, /*复位，主要针对低速设备提供一组默认值*/
    FMSG_CMD_SET,             /*初始化或配置或启动等*/
    FMSG_CMD_GET,             /*获取功能，寄存器外设状态等*/
    FMSG_CMD_DATA, /*低速接口数据收发，master core主动轮询，异步数据中断*/
    FMSG_CMD_REPORT, /*上报状态主动发起的任何命令，除异步数据中断*/
    FMSG_CMD_PROTOCOL, /*从slave获取协议相关内容，sw_ver , hw_ver , attributes*/
} FMsgCmdType;

typedef struct
{
    uintptr regfile; /*regfile信息*/
    uintptr shmem;
} FDevMsg;


#ifdef __cplusplus
}
#endif

#endif
