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
 * FilePath: fiopad_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 09:01:10
 * Description:  This files is for getting default configuration of specific IOPad instance_id
 *
 * Modify History:
 *  Ver   Who           Date        Changes
 * -----  ------        --------    --------------------------------------
 * 1.0   zhangyan   2023/7/3    first release
 */

#include "fparameters.h"
#include "fassert.h"
#include "fiopad.h"

extern FIOPadConfig FIOPadConfigTbl[FIOPAD_NUM];

/**
 * @name: FIOPadLookupConfig
 * @msg: get iopad configs by id
 * @return {*}
 * @param {u32} instanceId, id of iopad ctrl
 */
const FIOPadConfig *FIOPadLookupConfig(u32 instance_id)
{
    FASSERT(instance_id < FIOPAD_NUM);
    const FIOPadConfig *pconfig = NULL;
    u32 index;

    for (index = 0; index < (u32)FIOPAD_NUM; index++)
    {
        if (FIOPadConfigTbl[index].instance_id == instance_id)
        {
            pconfig = &FIOPadConfigTbl[index];
            break;
        }
    }

    return (const FIOPadConfig *)pconfig;
}