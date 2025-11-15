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
 * FilePath: fsdmmc_dma.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:51:25
 * Description:  This file is dma descriptormanagement API.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/2    init
 */

#ifndef FSDMMC_DMA_H
#define FSDMMC_DMA_H
/***************************** Include Files *********************************/
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* 设置读数据DMA配置 */
void FSdmmcSetReadDMA(uintptr base_addr, uintptr card_addr, u32 blk_cnt, uintptr buf_p);

/* 设置写数据DMA配置 */
void FSdmmcSetWriteDMA(uintptr base_addr, uintptr card_addr, u32 blk_cnt, uintptr buf_p);

#ifdef __cplusplus
}
#endif

#endif
