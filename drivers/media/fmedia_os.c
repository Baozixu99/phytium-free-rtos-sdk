/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: fmedia_os.c
 * Date: 2022-09-15 14:20:19
 * LastEditTime: 2022-09-21 16:59:51
 * Description:  This file is for providing the media driver
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  Wangzq     2022/12/20  Modify the format and establish the version
 */
#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "ftypes.h"
#include "fdebug.h"
#include "fparameters_comm.h"
#include "fmedia_os.h"
#include "fdcdp.h"
#include "fdp_hw.h"
#include "fdp.h"

/***************** Macros (Inline Functions) Definitions *********************/

#define FMEDIA_DEBUG_TAG "FFreeRTOSMEDIA"
#define FMEDIA_ERROR(format, ...) FT_DEBUG_PRINT_E(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEDIA_INFO(format, ...) FT_DEBUG_PRINT_I(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMEDIA_DEBUG(format, ...) FT_DEBUG_PRINT_D(FMEDIA_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/**
 * @name: FFreeRTOSMediaHwInit
 * @msg:  init the media,dc and dp
 * @param  {u32} channel is the dc channel
 * @param  {FFreeRTOSMedia*} instance is the driver instance
 * @param  {u32} width is the width
 * @param  {u32} height is the height
 * @return err code information, 0 indicates success，others indicates failed
 */
FFreeRTOSMedia *FFreeRTOSMediaHwInit(FFreeRTOSMedia *instance, u32 width, u32 height)
{
    FError ret = FT_SUCCESS;
    u32 index;

    for (index = 0; index < FDCDP_INSTANCE_NUM; index ++)
    {
        ret = FDcDpInitial(&instance->dcdp_ctrl, index, width, height);
    }
    if((instance->dcdp_ctrl.is_initialized[0]) | (instance->dcdp_ctrl.is_initialized[1]))
    {
        ret = FDP_SUCCESS;
    }
    return (FT_SUCCESS == ret) ? instance : NULL; /* exit with NULL if failed */
}

