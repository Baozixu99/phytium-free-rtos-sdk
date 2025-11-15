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
 * FilePath: fqspi_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 09:01:10
 * Description:  This files is for getting default configuration of specific qspi instance_id
 *
 * Modify History:
 *  Ver   Who           Date        Changes
 * -----  ------        --------    --------------------------------------
 * 1.0   wangxiaodong  2022/3/29  first release
 * 1.1   wangxiaodong  2022/9/9   improve functions
 */

#include "fparameters.h"
#include "fassert.h"
#include "fqspi.h"

extern FQspiConfig FQspiConfigTbl[FQSPI_NUM];

const FQspiConfig *FQspiLookupConfig(u32 instance_id)
{
    FASSERT(instance_id < FQSPI_NUM);
    const FQspiConfig *pconfig = NULL;
    u32 index;

    for (index = 0; index < (u32)FQSPI_NUM; index++)
    {
        if (FQspiConfigTbl[index].instance_id == instance_id)
        {
            pconfig = &FQspiConfigTbl[index];
            break;
        }
    }

    return (const FQspiConfig *)pconfig;
}