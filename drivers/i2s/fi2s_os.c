/*
 * Copyright : (C) 2024 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: fi2s_os.c
 * Created Date: 2024-02-29 10:49:34
 * Last Modified: 2024-05-14 09:16:29
 * Description:  This file is for i2s driver
 *
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0     Wangzq       2024/03/07        init
 */

#include <stdio.h>
#include <string.h>
#include "finterrupt.h"
#include "ftypes.h"
#include "sdkconfig.h"
#include "fcpu_info.h"
#include "fparameters.h"
#include "fparameters_comm.h"
#include "fdebug.h"
#include "fassert.h"
#include "ferror_code.h"

#include "fi2s_os.h"
/************************** Variable Definitions *****************************/
static FFreeRTOSI2s i2s;
/***************** Macros (Inline Functions) Definitions *********************/
#define FI2S_DEBUG_TAG "FI2S-OS"
#define FI2S_ERROR(format, ...) FT_DEBUG_PRINT_E(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_WARN(format, ...)  FT_DEBUG_PRINT_W(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_INFO(format, ...)  FT_DEBUG_PRINT_I(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_DEBUG(format, ...) FT_DEBUG_PRINT_D(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
/**
 * @name: FFreeRTOSI2SSetupInterrupt
 * @msg: 设置I2s中断
 * @param {FI2s} *ctrl
 * @return Null
 */
static void FFreeRTOSI2SSetupInterrupt(FI2s *ctrl)
{
    FASSERT(ctrl);
    FI2sConfig *config = &ctrl->config;
    u32 cpu_id;
    FError err = FFREERTOS_I2S_SUCCESS;

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(config->irq_num, cpu_id);
    /* umask i2s irq */
    InterruptSetPriority(config->irq_num, config->irq_prority);
    /* enable irq */
    InterruptUmask(config->irq_num);
}

/**
 * @name: FFreeRTOSI2s *FFreeRTOSI2sInit
 * @msg: 初始化i2s控制器
 * @param {u32} id, i2s id
 * @return {FFreeRTOSI2s} *instance
 */
FFreeRTOSI2s *FFreeRTOSI2sInit(u32 id)
{
    FASSERT_MSG(id < FI2S_NUM, "Invalid i2s id.");
    FFreeRTOSI2s *instance = &i2s;
    FI2s *ctrl = &instance->i2s_ctrl;
    FI2sConfig config;
    FError err = FFREERTOS_I2S_SUCCESS;

    if (FT_COMPONENT_IS_READY == ctrl->is_ready)
    {
        FI2S_ERROR("i2s-%d already init.", id);
        return instance;
    }
    /* no scheduler during init */
    taskENTER_CRITICAL();
    config = *FI2sLookupConfig(id);
    err = FI2sCfgInitialize(ctrl, &config);
    if (FFREERTOS_I2S_SUCCESS != err)
    {
        FI2S_ERROR("Init i2s-%d failed, err: 0x%x!!!", id, err);
        goto err_exit;
    }
    FFreeRTOSI2SSetupInterrupt(ctrl);
err_exit:
    taskEXIT_CRITICAL(); /* allow schedule after init */
    return (FFREERTOS_I2S_SUCCESS == err) ? instance : NULL; /* exit with NULL if failed */
}

/**
 * @name: FFreeRTOSSetupI2S
 * @msg:  set the i2s controller to work
 * @param {FFreeRTOSI2s} *os_i2s_p, the instance of i2s
 * @return Null
 */
FError FFreeRTOSSetupI2S(FFreeRTOSI2s *os_i2s_p)
{
    FASSERT(os_i2s_p);
    FI2s *ctrl = &os_i2s_p->i2s_ctrl;
    FError err = FT_SUCCESS;
    err =  FI2sClkOutDiv(ctrl, FI2S_SAMPLE_RATE_CD); /* 默认16-bits采集 */
    if (FT_SUCCESS != err)
    {
        FI2S_ERROR("Set i2s failed, err: 0x%x!!!", err);
        return err;
    }
    FI2sSetHwconfig(ctrl);
    FI2sTxRxEnable(ctrl, TRUE);
    return err;
}

/**
 * @name: FFreeRTOSI2SDeinit
 * @msg: 去初始化i2s控制器
 * @param {FFreeRTOSI2s} *os_i2s_p, the instance of i2s
 * @return Null
 */
FError FFreeRTOSI2SDeinit(FFreeRTOSI2s *os_i2s_p)
{
    FASSERT(os_i2s_p);

    FI2sStopWork(&os_i2s_p->i2s_ctrl);

    FI2sDeInitialize(&os_i2s_p->i2s_ctrl);

    return FT_SUCCESS;
}