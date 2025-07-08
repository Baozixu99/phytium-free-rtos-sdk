/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: ddma_spi_loopback.h
 * Date: 2022-07-18 11:01:37
 * LastEditTime: 2022-07-18 11:01:37
 * Description:  This file is for task create function define
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 zhugengyu 2022/08/26  first commit
 */
#ifndef  DDMA_SPI_LOOPBACK_H
#define  DDMA_SPI_LOOPBACK_H

#include "ftypes.h"
#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
BaseType_t FFreeRTOSRunDDMASpiLoopback(u32 spi_id, u32 bytes);

void DdmaTasksEntry(void);

#ifdef __cplusplus
}
#endif

#endif