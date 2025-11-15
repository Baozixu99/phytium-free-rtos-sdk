/*
 * Copyright (C) 2021, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fiopad_sinit.c
 * Date: 2021-04-29 10:21:53
 * LastEditTime: 2022-02-18 08:29:20
 * Description:  This files is for the ioctrl static init functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2023/7/30    first release
 */


#include "fparameters.h"
#include "fassert.h"
#include "fioctrl.h"

extern FIOCtrlConfig FIOCtrlConfigTbl[FIOCTRL_NUM];

/**
 * @name: FIOCtrlLookupConfig
 * @msg: get ioctrl configs by id
 * @return {*}
 * @param {u32} instanceId, id of iopad ctrl
 */
const FIOCtrlConfig *FIOCtrlLookupConfig(u32 instance_id)
{
    FASSERT(instance_id < FIOCTRL_NUM);
    const FIOCtrlConfig *pconfig = NULL;
    u32 index;

    for (index = 0; index < (u32)FIOCTRL_NUM; index++)
    {
        if (FIOCtrlConfigTbl[index].instance_id == instance_id)
        {
            pconfig = &FIOCtrlConfigTbl[index];
            break;
        }
    }

    return (const FIOCtrlConfig *)pconfig;
}