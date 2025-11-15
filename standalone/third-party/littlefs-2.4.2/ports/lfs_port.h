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
 * FilePath: lfs_port.h
 * Date: 2022-04-06 16:00:32
 * LastEditTime: 2022-04-06 16:00:32
 * Description:  This file is for little fs general port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/4/7     init commit
 */
#ifndef LITTLE_FS_PORT_H
#define LITTLE_FS_PORT_H


/***************************** Include Files *********************************/
#include "ftypes.h"
#include "lfs.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
enum
{
    FLFS_PORT_OK = 0,

    FLFS_PORT_ALREADY_INITED,
    FLFS_PORT_MEMP_ALREADY_INIT,
    FLFS_PORT_MEMP_INIT_FAILED,
    FLFS_PORT_FILEBD_INIT_FAILED,
    FLFS_PORT_FSPIM_INIT_FAILED,
    FLFS_PORT_IO_ERR,
};

typedef enum
{
    FLFS_PORT_TO_FSPIM = 0,
    FLFS_PORT_TO_DRY_RUN_IN_RAM,
    FLFS_PORT_TO_DRY_RUN_IN_FILE
} FLfsPortType;

/**************************** Type Definitions *******************************/
typedef struct lfs lfs_t;
typedef struct
{
    lfs_t lfs;
    u32 lfs_ready;
} FLfs;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
int FLfsInitialize(FLfs *const instance, FLfsPortType type);
void FLfsDeInitialize(FLfs *const instance);
const struct lfs_config *FLfsGetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif