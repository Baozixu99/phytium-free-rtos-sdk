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
 * FilePath: fi2s_os.h
 * Created Date: 2024-02-29 10:49:34
 * Last Modified: 2024-03-07 10:09:31
 * Description:  This file is for providing function related definitions of i2s driver
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *   1.0     Wangzq   2024/03/07        init
 */


#ifndef FI2S_OS_H
#define FI2S_OS_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "fi2s.h"
#include "ftypes.h"
#include "fparameters.h"
#include "event_groups.h"

#ifdef __cplusplus
extern "C"
{
#endif
/************************** Constant Definitions *****************************/
#define FFREERTOS_I2S_SUCCESS                   FT_SUCCESS
/*Error code depend on OS standard*/
#define FREERTOS_I2S_TASK_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2s, 0x1)
#define FREERTOS_I2S_MESG_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2s, 0x2)
#define FREERTOS_I2S_MEMY_ERROR                 FT_CODE_ERR(ErrModPort, ErrBspI2s, 0x3)

typedef struct
{
    FI2s i2s_ctrl;
    u32 work_mode;     /*the work mode of i2s*/
    SemaphoreHandle_t i2s_semaphore; /* i2s read and write semaphore for resource sharing */
    EventGroupHandle_t trx_event;   /* i2s TX/RX completion event */
} FFreeRTOSI2s;

/*init the i2s and return the i2s instance*/
FFreeRTOSI2s *FFreeRTOSI2sInit(u32 id);

/*deinit the i2s */
FError FFreeRTOSI2SDeinit(FFreeRTOSI2s *os_i2s_p);

#ifdef __cplusplus
}
#endif

#endif
