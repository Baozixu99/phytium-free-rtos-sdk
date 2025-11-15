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
 * FilePath: fpl011_sinit.c
 * Date: 2021-11-02 14:53:42
 * LastEditTime: 2022-02-18 09:07:00
 * Description:  This file is for uart static init
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  huanghe 2021/11/2  first commit
 * 1.1  liushengming 2022/02/18  add file head 
 */

/***************************** Include Files *********************************/

#include "fpl011.h"
#include "fparameters.h"

extern FPl011Config FPl011ConfigTable[FUART_NUM];
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
 * @name: FPl011LookupConfig
 * @msg:  Initializes a specific FPl011 instance such that it is ready to be used.
 * @param  contains the ID of the device
 * @return {FPl011Config *}  A pointer to the configuration structure or NULL if the
 *      specified device is not in the system.
 */
const FPl011Config *FPl011LookupConfig(u32 instance_id)
{
    const FPl011Config *cfg_ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FUART_NUM; index++)
    {
        if (FPl011ConfigTable[index].instance_id == instance_id)
        {
            cfg_ptr = &FPl011ConfigTable[index];
            break;
        }
    }

    return (const FPl011Config *)cfg_ptr;
}
