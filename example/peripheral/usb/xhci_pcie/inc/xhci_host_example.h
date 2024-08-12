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
 * FilePath: xhci_host_example.h
 * Date: 2022-07-19 09:26:25
 * LastEditTime: 2022-07-19 09:26:25
 * Description:  This file is for the xhci host definition.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/9/20   init commit
 * 2.0   zhugengyu  2024/7/1    support auto-run
 */
#ifndef  XHCI_HOST_EXAMPLE_H
#define  XHCI_HOST_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
BaseType_t FFreeRTOSRunXhciDisk(u32 id, const char *devname);
BaseType_t FFreeRTOSRunXhciDiskBench(u32 id, const char *devname);
BaseType_t FFreeRTOSRunXhciKeyboard(u32 id, const char *devname);
BaseType_t FFreeRTOSRunXhciMouse(u32 id, const char *devname);

#ifdef __cplusplus
}
#endif

#endif