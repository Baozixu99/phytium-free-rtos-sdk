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
 * FilePath: fi2c_msg_os.c
 * Date: 2025-04-17 10:43:29
 * LastEditTime: 2025-04-17 10:43:29
 * Description:  This file is for required function implementations of i2c v2 driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 zhangyan 2025/04/17  first commit
 */

#include <stdio.h>
#include <string.h>
#include "fi2c_msg_os.h"
#include "finterrupt.h"
#include "ftypes.h"
#include "sdkconfig.h"
#include "fcpu_info.h"
#include "fparameters.h"
#include "fdebug.h"
#include "fio_mux.h"
#include "fi2c_msg.h"
#include "fi2c_msg_master.h"
#include "fi2c_msg_slave.h"
#include "fi2c_msg_hw.h"
#include "fassert.h"

static FFreeRTOSI2cMsg os_i2c[FI2C_MSG_NUM] = {0};
#define I2C_TIMEOUT_MS 1000
/**
 * @name: FI2cOsSetupInterrupt
 * @msg: 设置i2c中断
 * @return {*}
 * @param {FI2cMsgCtrl} *pctrl
 */
static void FI2cOsSetupInterrupt(FI2cMsgCtrl *pctrl)
{
    FASSERT(pctrl);
    FI2cMsgConfig *pconfig = &pctrl->config;
    u32 cpu_id;
    FError err = FREERTOS_I2C_MSG_SUCCESS;

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(pconfig->irq_num, cpu_id);
    InterruptSetPriority(pconfig->irq_num, pconfig->irq_prority);
    InterruptInstall(pconfig->irq_num, (IrqHandler)FI2cMsgMasterRegfileIsr, pctrl, NULL); /* register intr callback */
    InterruptUmask(pconfig->irq_num);

    return ;
}

static void FI2cMsgOsResetInterrupt(FI2cMsgCtrl *pctrl)
{
    FASSERT(pctrl);
    /* disable irq */
    InterruptMask(pctrl->config.irq_num);
}

/**
 * @name: FFreeRTOSI2cMsgInit
 * @msg: init FreeRTOS i2c instance
 * @return {FFreeRTOSI2cMsg *}pointer to os i2c instance
 * @param {u32} instance_id,i2c instance_id
 */
FFreeRTOSI2cMsg *FFreeRTOSI2cMsgInit(u32 instance_id, u32 speed_mode)
{
    FASSERT((os_i2c[instance_id].wr_semaphore = xSemaphoreCreateMutex()) != NULL);
    FASSERT((os_i2c[instance_id].trx_event = xEventGroupCreate()) != NULL);

    FError err = FREERTOS_I2C_MSG_SUCCESS;
    FI2cMsgConfig i2c_config;

    FASSERT(instance_id < FI2C_MSG_NUM);

    if (FT_COMPONENT_IS_READY == os_i2c[instance_id].i2c_device.is_ready)
    {
        vPrintf("I2c device %d is already initialized.\r\n", instance_id);
        return NULL;
    }

    i2c_config = *FI2cMsgLookupConfig(instance_id);
    err = FI2cMsgCfgInitialize(&os_i2c[instance_id].i2c_device, &i2c_config);

    if (err != FREERTOS_I2C_MSG_SUCCESS)
    {
        vPrintf("FI2cMsgCfgInitialize failed, ret = %d", err);
        return NULL;
    }

    os_i2c[instance_id].i2c_device.speed_mode = speed_mode;
    os_i2c[instance_id].i2c_device.timeout_ms  = I2C_TIMEOUT_MS ;
    os_i2c[instance_id].i2c_device.clk_clock_frequency = FI2C_CLK_FREQ_HZ;

    FI2cMsgSetBusSpeed(&os_i2c[instance_id].i2c_device, speed_mode, TRUE);

    FI2cOsSetupInterrupt(&os_i2c[instance_id].i2c_device);

    err = FI2cMsgMasterVirtProbe(&os_i2c[instance_id].i2c_device);
    if (err != FREERTOS_I2C_MSG_SUCCESS)
    {
        vPrintf("FI2cMsgMasterVirtProbe failed, ret = %d", err);
        return NULL;
    }

    return (&os_i2c[instance_id]);
}

/**
 * @name: FFreeRTOSI2cMsgDeinit
 * @msg: deinit FreeRTOS i2c msg instance, include deinit i2c and delete mutex semaphore
 * @return {*}无
 * @param {FFreeRTOSI2cMsg} *os_i2c_p,pointer to os i2c instance
 */
void FFreeRTOSI2cMsgDeinit(FFreeRTOSI2cMsg *os_i2c_p)
{
    FASSERT(os_i2c_p);
    FASSERT(os_i2c_p->wr_semaphore != NULL);

    /* 避免没有关闭中断，存在触发 */
    FI2cMsgOsResetInterrupt(&os_i2c_p->i2c_device);
    FI2cMsgDeInitialize(&os_i2c_p->i2c_device);

    FASSERT_MSG(NULL != os_i2c_p->wr_semaphore, "Semaphore not exists!!!");
    vSemaphoreDelete(os_i2c_p->wr_semaphore);
    os_i2c_p->wr_semaphore = NULL;

    FASSERT_MSG(NULL != os_i2c_p->trx_event, "Event group not exists!!!");
    vEventGroupDelete(os_i2c_p->trx_event);
    os_i2c_p->trx_event = NULL;
}

/**
 * @name: FFreeRTOSI2cMsgTransfer
 * @msg: tranfer i2c mesage
 * @return {*}
 * @param {u32} instance_id
 * @param {FFreeRTOSI2cMsgMessage} *message
 * @param {u8} mode
 */
FError FFreeRTOSI2cMsgTransfer(FFreeRTOSI2cMsg *os_i2c_p, FFreeRTOSI2cMsgMessage *message)
{
    FASSERT(os_i2c_p);
    FASSERT(message);
    FASSERT(os_i2c_p->wr_semaphore != NULL);
    FASSERT(message->mode < FI2C_READ_DATA_MODE_NUM);

    if (pdFALSE == xSemaphoreTake(os_i2c_p->wr_semaphore, portMAX_DELAY))
    {
        vPrintf("I2c xSemaphoreTake failed.\r\n");
        return FREERTOS_I2C_MSG_MESG_ERROR;
    }

    FError ret = FREERTOS_I2C_MSG_SUCCESS;
    
    if (message->mode == FI2C_WRITE_DATA)
    {
        struct FI2cMsg msg = 
        {
            .addr = message->slave_addr,
            .flags = 0,
            .len = message->buf_length + message->mem_byte_len,
            .buf = pvPortMalloc(msg.len)
        };

        FASSERT_MSG(msg.buf, "I2C msg write buf NULL!");

        memcpy(msg.buf, &message->mem_addr,  message->mem_byte_len);
        memcpy(msg.buf + message->mem_byte_len, message->buf, message->buf_length);
        FI2cMsgMasterVirtXfer(&os_i2c_p->i2c_device, &msg, 1);
        
        vPortFree(msg.buf);
    }
    else if (message->mode == FI2C_READ_DATA)
    {
        /*When performing a read operation, first write to the input memaddr, and then read*/
        struct FI2cMsg msg[2];
        msg[0].addr = message->slave_addr;
        msg[0].flags = FI2C_MSG_M_WR;
        msg[0].len = message->mem_byte_len;
        memcpy(msg[0].buf, &message->mem_addr, message->mem_byte_len);

        msg[1].addr = message->slave_addr;
        msg[1].flags = FI2C_MSG_M_RD;
        msg[1].len = message->buf_length;
        msg[1].buf = message->buf;
        FI2cMsgMasterVirtXfer(&os_i2c_p->i2c_device, msg, 2);
    }

    /* Enable next transfer. Current one is finished */
    if (pdFALSE == xSemaphoreGive(os_i2c_p->wr_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        vPrintf("FFreeRTOSI2cMsgTransfer function xSemaphoreGive failed.\r\n");
        return FREERTOS_I2C_MSG_MESG_ERROR;
    }

    if (ret != FREERTOS_I2C_MSG_SUCCESS)
    {
        vPrintf("FFreeRTOSI2cMsgTransfer error,id:%d.\r\n", os_i2c_p->i2c_device.config.instance_id);
        return FREERTOS_I2C_MSG_TASK_ERROR;
    }
    return ret;
}