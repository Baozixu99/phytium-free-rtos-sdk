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
 * FilePath: fspim_lfs_port.h
 * Date: 2022-04-06 16:07:32
 * LastEditTime: 2022-04-06 16:07:33
 * Description:  This file is for little fs spim port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/4/7     init commit
 */
#ifndef LITTLE_FS_FSPIM_PORT_H
#define LITTLE_FS_FSPIM_PORT_H


/***************************** Include Files *********************************/
#include "lfs.h"
#include "lfs_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
enum
{
    FLFS_FSPIM_PORT_OK = 0,
    FLFS_FSPIM_PORT_INIT_SFUD_FAILED,
    FLFS_FSPIM_PORT_SFUD_NOT_READY,
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
int FLfsSpimInitialize(FLfs *const instance);
void FLfsSpimDeInitialize(FLfs *const instance);
const struct lfs_config *FLfsSpimGetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif