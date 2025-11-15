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
 * FilePath: fadc_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-25 11:45:05
 * Description: This file is for adc static variables implementation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/4/25   init commit
 */


/***************************** Include Files *********************************/


#include "fparameters.h"
#include "fadc.h"
#include "fassert.h"

extern FAdcConfig FAdcConfigTbl[FADC_NUM];

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

/**
 * @name: FAdcLookupConfig
 * @msg:  get default configuration of specific adc id.
 * @param {u32} instance_id, instance id of FADC controller
 * @return {FAdcConfig*} Default configuration parameters of FADC
 */
const FAdcConfig *FAdcLookupConfig(u32 instance_id)
{
    const FAdcConfig *pconfig = NULL;
    FASSERT(instance_id < FADC_NUM);

    u32 index = 0;

    for (index = 0; index < (u32)FADC_NUM; index++)
    {
        if (FAdcConfigTbl[index].instance_id == instance_id)
        {
            pconfig = &FAdcConfigTbl[index];
            break;
        }
    }

    return (FAdcConfig *)pconfig;
}
