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
 * FilePath: lfs_testbd_port.h
 * Date: 2022-04-07 08:41:31
 * LastEditTime: 2022-04-07 08:41:31
 * Description:  This file is for little fs test bd port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/4/7     init commit
 */
#ifndef LITTLE_FS_DRY_RUN_PORT_H
#define LITTLE_FS_DRY_RUN_PORT_H


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
    FLFS_DRY_RUN_PORT_OK = 0,
    FLFS_DRY_RUN_PORT_INIT_FAILED,
};
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
int FLfsTestBDInitialize(FLfs *const instance, const char *lfs_testbd_path);
void FLfsTestBDDeInitialize(FLfs *const instance);
const struct lfs_config *FLfsTestBDGetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif