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
 * FilePath: fsemaphore_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:25:15
 * Description:  This file is for semaphore static init
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/5/23    init commit
 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fsemaphore.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern const FSemaConfig fsema_cfg_tbl[FSEMA_INSTANCE_NUM];

/************************** Function Prototypes ******************************/
/**
 * @name: FSemaLoopkupConfig
 * @msg: 获取Semaphore的默认配置
 * @return {const FSemaConfig *} Semaphore的默认配置
 * @param {u32} instance_id, Semaphore的实例id
 */
const FSemaConfig *FSemaLoopkupConfig(u32 instance_id)
{
    const FSemaConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FSEMA_INSTANCE_NUM; index++)
    {
        if (fsema_cfg_tbl[index].id == instance_id)
        {
            ptr = &fsema_cfg_tbl[index];
            break;
        }
    }

    return (const FSemaConfig *)ptr;
}