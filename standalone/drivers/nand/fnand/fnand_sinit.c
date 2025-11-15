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
 * FilePath: fnand_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:56:56
 * Description:  This file contains the implementation of the fnand driver's static
 * initialization functionality.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/05/10    first release
 */


#include "fnand.h"
#include "fparameters.h"

extern FNandConfig FNandConfigTable[FNAND_NUM];

/**
 * @name: FNandLookupConfig
 * @msg:  This function looks up the device configuration based on the unique device ID.
 * @param {u32} instance_id contains the ID of the device
 * @return {FNandConfig *} - A pointer to the configuration found . - NULL if the specified device ID is not found
 */
FNandConfig *FNandLookupConfig(u32 instance_id)
{
    FNandConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FNAND_NUM; index++)
    {
        if (FNandConfigTable[index].instance_id == instance_id)
        {
            ptr = &FNandConfigTable[index];
            break;
        }
    }

    return (FNandConfig *)ptr;
}