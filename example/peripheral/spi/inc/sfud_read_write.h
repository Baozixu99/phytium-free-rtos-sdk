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