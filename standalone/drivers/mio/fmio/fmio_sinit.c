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
 * FilePath: fmio_sinit.c
 * Date: 2022-06-20 20:33:25
 * LastEditTime: 2022-06-20 20:33:25
 * Description:  This file is for mio static initialization functionality
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming  2022/06/20  first commit
 */
#include "ftypes.h"
#include "fparameters.h"
#include "fassert.h"
#include "fmio.h"
#include "fmio_hw.h"


extern FMioConfig FMioConfigTbl[FMIO_NUM];

/***************** Macros (Inline Functions) Definitions *********************/
#define FMIO_DEBUG_TAG          "MIO"
#define FMIO_ERROR(format, ...) FT_DEBUG_PRINT_E(FMIO_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/**
 * @name: FMioLookupConfig
 * @msg: get mio configs by id
 * @param {u32} instance_id, id of mio ctrl
 * @return {FMioConfig *}, mio config address
 */
const FMioConfig *FMioLookupConfig(u32 instance_id)
{
    FASSERT(instance_id < FMIO_NUM);
    const FMioConfig *pconfig = NULL;
    u32 index;

    for (index = 0; index < (u32)FMIO_NUM; index++)
    {
        if (FMioConfigTbl[index].instance_id == instance_id)
        {
            pconfig = &FMioConfigTbl[index];
            break;
        }
    }
    return (const FMioConfig *)pconfig;
}
