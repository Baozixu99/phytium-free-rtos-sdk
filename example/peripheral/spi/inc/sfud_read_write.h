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
 * FilePath: sfud_read_write.h
 * Date: 2022-07-18 16:43:35
 * LastEditTime: 2022-07-18 16:43:35
 * Description:  This file is for providing some sfud apis.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  zhugengyu  2022/8/26    first commit
 */
#ifndef  SFUD_READ_WRITE_H
#define  SFUD_READ_WRITE_H

#include "ftypes.h"
#include "FreeRTOS.h"
#include "sdkconfig.h"
#ifdef __cplusplus
extern "C"
{
#endif
#if defined(CONFIG_E2000Q_DEMO_BOARD) || defined(CONFIG_E2000D_DEMO_BOARD)
#define SFUD_FLASH_INDEX  SFUD_FSPIM2_INDEX
#else
#define SFUD_FLASH_INDEX  SFUD_FSPIM0_INDEX
#endif
/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

BaseType_t FFreeRTOSSfudWriteThenRead(void);

#ifdef __cplusplus
}
#endif

#endif