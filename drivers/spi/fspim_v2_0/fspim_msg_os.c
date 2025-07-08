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
 * FilePath: fspim_msg_os.c
 * Date: 2025-04-15 10:09:31
 * LastEditTime: 2025-04-15 10:09:31
 * Description:  This file is for providing function implementation of spi msg master driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2025-04-15   init commit
 */

/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "finterrupt.h"
#include "fparameters.h"
#include "fdebug.h"
#include "fassert.h"
#include "fsleep.h"
#include "fcpu_info.h"
#include "fio_mux.h"

#include "finterrupt.h"
#include "fspim_msg_os.h"

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FFreeRTOSSpim spim[FSPI_MSG_NUM];

/***************** Macros (Inline Functions) Definitions *********************/
#define FSPIM_DEBUG_TAG "FSPIM-MSG-OS"
#define FSPIM_ERROR(format, ...) FT_DEBUG_PRINT_E(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_WARN(format, ...) FT_DEBUG_PRINT_W(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_INFO(format, ...) FT_DEBUG_PRINT_I(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_DEBUG(format, ...) FT_DEBUG_PRINT_D(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)


static inline FError FSpimOsTakeSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");
    if (pdFALSE == xSemaphoreTake(locker, portMAX_DELAY))
    {
        FSPIM_ERROR("Failed to give locker!!!");
        return FFREERTOS_SPIM_SEMA_ERR;
    }

    return FFREERTOS_SPIM_OK;
}

static inline void FSpimOsGiveSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "Locker not exists.");
    if (pdFALSE == xSemaphoreGive(locker))
    {
        FSPIM_ERROR("Failed to give locker!!!");
    }

    return;
}


/**
 * @name: FFreeRTOSSpimMsgIntrInit
 * @msg: init spi msg interrupt
 * @return none
 * @param {FSpiMsgCtrl *} ctrl, spi msg ctrl
 * @param {FSpiMsgConfig *} config, spi msg config
 */
static void FFreeRTOSSpimMsgIntrInit(FSpiMsgCtrl *ctrl, const FSpiMsgConfig *config)
{
    FASSERT(ctrl && config);
    u32 cpu_id = 0;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(config->irq_num, cpu_id);
    InterruptSetPriority(config->irq_num, config->irq_priority);

    ctrl->cmd_completion = CMD_MSG_NOT_COMPLETION;
    InterruptInstall(config->irq_num, FSpiMsgInterruptHandler, ctrl, NULL);

    InterruptUmask(config->irq_num);
}

/**
 * @name: FFreeRTOSSpimMsgInit
 * @msg: init and get spi instance
 * @return {FFreeRTOSSpim *} return
 * @param {u32} id, spim instance id
 * @param {FFreeRTOSSpimConifg} *input_config, freertos spim config
 */
FFreeRTOSSpim *FFreeRTOSSpimMsgInit(u32 id, const FFreeRTOSSpimConifg *input_config)
{
    FASSERT(input_config);
    FASSERT_MSG(id < FSPI_MSG_NUM, "Invalid spim id.");
    FFreeRTOSSpim *instance = &spim[id];
    FSpiMsgCtrl *ctrl = &instance->ctrl;
    FSpiMsgConfig config;
    FError err = FFREERTOS_SPIM_OK;

    if (FT_COMPONENT_IS_READY == ctrl->is_ready)
    {
        FSPIM_ERROR("spi-%d already init.", id);
        return instance;
    }

    /* no scheduler during init */
    taskENTER_CRITICAL();

    instance->config = *input_config;
    config = *FSpiMsgLookupConfig(id);
    config.slave_dev_id = FSPI_MSG_SLAVE_DEV_0;

    if (FFREERTOS_SPIM_MODE_0 == instance->config.spi_mode) /* mode 0 */
    {
        config.mode = 0x00;
    }
    else if (FFREERTOS_SPIM_MODE_1 == instance->config.spi_mode) /* mode 1 */
    {
        config.mode = 0x10;
    }
    else if (FFREERTOS_SPIM_MODE_2 == instance->config.spi_mode) /* mode 2 */
    {
        config.mode = 0x01;
    }
    else if (FFREERTOS_SPIM_MODE_3 == instance->config.spi_mode) /* mode 3 */
    {
        config.mode = 0x11;
    }

    config.n_bytes = FSPIM_1_BYTE;
    config.en_dma = instance->config.en_dma;

    FIOMuxInit();
    FIOPadSetSpimMux(id);

    FFreeRTOSSpimMsgIntrInit(ctrl, &config);

    err = FSpiMsgCfgInitialize(ctrl, &config);
    if (FT_SUCCESS != err)
    {
        FSPIM_ERROR("Init spim-%d failed, err: 0x%x!!!", id, err);
        goto err_exit;
    }

    FSPIM_INFO("init spi-%d @ 0x%x", config.instance_id, config.spi_msg.shmem);

    FASSERT_MSG(NULL == instance->locker, "Locker exists!!!");
    FASSERT_MSG((instance->locker = xSemaphoreCreateMutex()) != NULL, "Create mutex failed!!!");

    FASSERT_MSG(NULL == instance->evt, "Event group exists!!!");
    FASSERT_MSG((instance->evt = xEventGroupCreate()) != NULL, "Create event group failed!!!");

    FSPIM_INFO("Init spi-%d success!!!", id);

err_exit:
    taskEXIT_CRITICAL();                          /* allow schedule after init */
    return (FT_SUCCESS == err) ? instance : NULL; /* exit with NULL if failed */
}

/**
 * @name: FFreeRTOSSpimMsgDeInit
 * @msg: deinit spi instance
 * @return {FError} return FFREERTOS_SPIM_OK
 * @param {FFreeRTOSSpim} *instance, spi instance
 */
FError FFreeRTOSSpimMsgDeInit(FFreeRTOSSpim *const instance)
{
    FASSERT(instance);
    FSpiMsgCtrl *ctrl = &instance->ctrl;
    FError err = FFREERTOS_SPIM_OK;

    if (FT_COMPONENT_IS_READY != ctrl->is_ready)
    {
        FSPIM_ERROR("ddma-%d already init.");
        return FFREERTOS_SPIM_NOT_INIT;
    }

    /* no scheduler during deinit */
    taskENTER_CRITICAL();

    FSpiMsgDeInitialize(ctrl);

    FASSERT_MSG(NULL != instance->locker, "Locker not exists!!!");
    vSemaphoreDelete(instance->locker);
    instance->locker = NULL;

    FASSERT_MSG(NULL != instance->evt, "Event group not exists!!!");
    vEventGroupDelete(instance->evt);
    instance->evt = NULL;

    taskEXIT_CRITICAL(); /* allow schedule after deinit */

    return err;
}

/* SPI数据传输主函数 */
FError FFreeRTOSSpimMsgTransfer(FFreeRTOSSpim *instance,
                                FFreeRTOSSpiMessage *msg)
{
    FError result = FFREERTOS_SPIM_OK;
    FSpiMsgCtrl *spi_msg_ctrl = &instance->ctrl;
    u8 first_data;
    /* 获取访问锁 */
    if (FSpimOsTakeSema(instance->locker) != FFREERTOS_SPIM_OK)
    {
        return FFREERTOS_SPIM_SEMA_ERR;
    }

    /* 使能片选 */
    FSpiMsgSetChipSelection(&instance->ctrl, TRUE);

    if(msg->tx_len)
    {
        result = FSpiMsgTransfer(spi_msg_ctrl, (void *)msg->tx_buf, NULL, msg->tx_len);
        if(result != FFREERTOS_SPIM_OK)
        {
            FSPIM_ERROR("FSpiMsgTransfer tx failed, err: 0x%x!!!", result);
            goto exit;
        }
    }

    if(msg->rx_len)
    {
        result = FSpiMsgTransfer(spi_msg_ctrl, NULL, (void *)msg->rx_buf, msg->rx_len);
        if(result != FFREERTOS_SPIM_OK)
        {
            FSPIM_ERROR("FSpiMsgTransfer rx failed, err: 0x%x!!!", result);
            goto exit;
        }
    }

    /* 禁用片选 */
    FSpiMsgSetChipSelection(&instance->ctrl, FALSE);

exit:
    /* 释放锁 */
    FSpimOsGiveSema(instance->locker);
    return result;
}
