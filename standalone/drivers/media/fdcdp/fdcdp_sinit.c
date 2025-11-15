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
 * FilePath: fdc_stinit.c
 * Date: 2022-09-05 23:18:44
 * LastEditTime: 2022/12/20 13:10:44
 * Description:  This file is for geting the dc config
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2022/12/20  Modify the format and establish the version
 */
#include "fparameters.h"
#include "fdc.h"
#include "fdp.h"

extern const FDpConfig fdp_config[FDP_INSTANCE_NUM];
extern const FDcConfig fdc_config[FDC_INSTANCE_NUM];

const FDcConfig *FDcLookupConfig(u32 instance_id)
{
    const FDcConfig *cfg_ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FDC_INSTANCE_NUM; index++)
    {
        if (fdc_config[index].instance_id == instance_id)
        {
            cfg_ptr = &fdc_config[index];
            break;
        }
    }

    return (const FDcConfig *)cfg_ptr;
}

const FDpConfig *FDpLookupConfig(u32 instance_id)
{
    const FDpConfig *cfg_ptr = NULL;
    u32 index;
    for (index = 0; index < (u32)FDP_INSTANCE_NUM; index++)
    {
        if (fdp_config[index].instance_id == instance_id)
        {
            cfg_ptr = &fdp_config[index];
            break;
        }
    }

    return (const FDpConfig *)cfg_ptr;
}
