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
 *
 * FilePath: fi2c_msg_os.h
 * Date: 2025-04-17 10:43:29
 * LastEditTime: 2025-04-17 10:43:29
 * Description:  This file is for required function implementations of i2c v2 driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 zhangyan 2025/04/17  first commit
 */

#ifndef FI2C_MSG_OS_H
#define FI2C_MSG_OS_H
/***************************** Include Files *********************************/
#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include "ferror_code.h"
#include "ftypes.h"
#include "fi2c_msg.h"
#include "fi2c_msg_master.h"
#include "fi2c_msg_slave.h"
#include "fi2c_msg_hw.h"
/************************** Constant Definitions *****************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*Error code from standalone i2c driver*/
#define FREERTOS_I2C_MSG_SUCCESS                    FI2C_MSG_SUCCESS
#define FREERTOS_I2C_MSG_TIMEOUT                    FI2C_MSG_TIMEOUT

/*Error code depend on OS standard*/
#define FREERTOS_I2C_MSG_TASK_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2c, 0x1)
#define FREERTOS_I2C_MSG_MESG_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2c, 0x2)
#define FREERTOS_I2C_MSG_TIME_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2c, 0x3)
#define FREERTOS_I2C_MSG_MEMY_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2c, 0x4)

/*!
* @cond RTOS_PRIVATE
* @name I2C FreeRTOS handler
*
* These are the only valid states for txEvent and rxEvent
*/

/************************** Variable Definitions *****************************/
/**
 * iic message structure
 */
typedef struct
{
    void *buf;              /* i2c read or write buffer */
    size_t buf_length;      /* i2c read or write buffer length */
    u32 slave_addr;         /* i2c slave addr,you can change slave_addr to send different device in the bus*/
    u32 mem_addr;           /* i2c slave address offset to read or write */
    u8 mem_byte_len;        /* sizeof slave address */
    volatile u8 mode;       /* transport mode */
} FFreeRTOSI2cMsgMessage;

typedef struct
{
    FI2cMsgCtrl i2c_device;
    SemaphoreHandle_t wr_semaphore; /* i2c read and write semaphore for resource sharing */
    EventGroupHandle_t trx_event;   /* i2c TX/RX completion event */
} FFreeRTOSI2cMsg;

enum /*选择操作I2C的方式*/
{
    FI2C_READ_DATA,
    FI2C_WRITE_DATA,

    FI2C_READ_DATA_MODE_NUM
};

/************************** Function Prototypes ******************************/
/* init FreeRTOS i2c instance */
FFreeRTOSI2cMsg *FFreeRTOSI2cMsgInit(u32 instance_id, u32 speed_rate);

/* deinit FreeRTOS i2c instance */
void FFreeRTOSI2cMsgDeinit(FFreeRTOSI2cMsg *os_i2c_p);

/* tranfer i2c mesage */
FError FFreeRTOSI2cMsgTransfer(FFreeRTOSI2cMsg *os_i2c_p, FFreeRTOSI2cMsgMessage *message);

#ifdef __cplusplus
}
#endif

#endif