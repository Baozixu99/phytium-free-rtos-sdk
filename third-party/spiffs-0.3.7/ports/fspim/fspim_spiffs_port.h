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
 * FilePath: fspim_spiffs_port.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:52
 * Description:  This files is for providing spiffs api based on spi.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liqiaozhong   2022/4/15    first commit, support Spiffs
 */

#ifndef  FSPIM_SPIFFS_PORT_H
#define  FSPIM_SPIFFS_PORT_H

#ifdef __cplusplus
extern "C"
{
#endif
/***************************** Include Files *********************************/
#include "ftypes.h"

#include "spiffs_port.h"
/************************** Constant Definitions *****************************/
enum
{
    FSPIFFS_SPIM_PORT_OK = 0,
    FSPIFFS_SPIM_PORT_SFUD_INIT_FAILED,
    FSPIFFS_SPIM_PORT_SFUD_NOT_READY,
    FSPIFFS_SPIM_PORT_SFUD_IO_ERROR,
    FSPIFFS_SPIM_PORT_ALREADY_INITED,
};

/* 根据SPIFFS的技术手册，最优页尺寸可以参考公式
    ~~~   Logical Page Size = Logical Block Size / 256   ~~~
 */
#define FSPIFFS_LOG_PAGE_SIZE           256 /* size of logic page */
#define FSPIFFS_LOG_BLOCK_SIZE          (FSPIFFS_LOG_PAGE_SIZE * 256)
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int FSpiffsSpimInitialize(FSpiffs *const instance);
void FSpiffsSpimDeInitialize(FSpiffs *const instance);
const spiffs_config *FSpiffsSpimGetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif