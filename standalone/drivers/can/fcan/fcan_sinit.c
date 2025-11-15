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
 * FilePath: fcan_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:29:15
 * Description:  This files is for getting default configuration of specific can instance_id
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/5/26  first releases
 */


#include "fcan.h"
#include "fparameters.h"
#include "fassert.h"

extern const FCanConfig FCanConfigTbl[FCAN_NUM];

/**
 * @name: FCanLookupConfig
 * @msg:  get default configuration of specific can instance_id.
 * @param {u32} instance_id, instance id of Can controller
 * @return {FCanConfig*} Default configuration parameters of Can
 */
const FCanConfig *FCanLookupConfig(u32 instance_id)
{
    FASSERT(instance_id < FCAN_NUM);
    const FCanConfig *pconfig = NULL;
    u32 index;

    for (index = 0; index < (u32)FCAN_NUM; index++)
    {
        if (FCanConfigTbl[index].instance_id == instance_id)
        {
            pconfig = &FCanConfigTbl[index];
            break;
        }
    }
    return (const FCanConfig *)pconfig;
}
