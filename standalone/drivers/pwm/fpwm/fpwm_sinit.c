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
 * FilePath: fpwm_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-04-25 11:45:05
 * Description:  This file is for pwm static variables implementation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/4/25   init commit
 */


/***************************** Include Files *********************************/

#include "fpwm.h"
#include "fparameters.h"
#include "fassert.h"

extern FPwmConfig FPwmConfigTbl[FPWM_NUM];

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/

/**
 * @name: FPwmLookupConfig
 * @msg: get pwm configs by id
 * @return {*}
 * @param {u32} instanceId, id of pwm ctrl
 */
const FPwmConfig *FPwmLookupConfig(u32 instance_id)
{
    const FPwmConfig *pconfig = NULL;
    FASSERT(instance_id < FPWM_NUM);

    u32 index = 0;

    for (index = 0; index < (u32)FPWM_NUM; index++)
    {
        if (FPwmConfigTbl[index].instance_id == instance_id)
        {
            pconfig = &FPwmConfigTbl[index];
            break;
        }
    }

    return (FPwmConfig *)pconfig;
}