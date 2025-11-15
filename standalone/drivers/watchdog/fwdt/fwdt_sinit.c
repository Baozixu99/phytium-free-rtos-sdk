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
 * FilePath: fwdt_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-25 11:45:05
 * Description:  This file is for wdt static variables implementation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/4/25   init commit
 */


/***************************** Include Files *********************************/

#include "fwdt.h"
#include "fparameters.h"

#define FWDT_DEBUG_TAG          "WDT_SINIT"
#define FWDT_ERROR(format, ...) FT_DEBUG_PRINT_E(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_WARN(format, ...)  FT_DEBUG_PRINT_W(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_INFO(format, ...)  FT_DEBUG_PRINT_I(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_DEBUG(format, ...) FT_DEBUG_PRINT_D(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)


extern FWdtConfig FWdtConfigTbl[FWDT_NUM];
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

/**
 * @name: FWdtLookupConfig
 * @msg: get wdt configs by id
 * @param {u32} instance_id, id of wdt ctrl
 * @return {FWdtConfig *}, wdt config address
 */
const FWdtConfig *FWdtLookupConfig(u32 instance_id)
{
    const FWdtConfig *pconfig = NULL;
    u32 index;

    if (instance_id >= FWDT_NUM)
    {
        FWDT_ERROR("wdt id is not invalid.");
        return NULL;
    }

    for (index = 0; index < (u32)FWDT_NUM; index++)
    {
        if (FWdtConfigTbl[index].instance_id == instance_id)
        {
            pconfig = &FWdtConfigTbl[index];
            break;
        }
    }
    return (const FWdtConfig *)pconfig;
}
