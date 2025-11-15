/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fpcie_c_sinit.c
 * Created Date: 2023-08-02 09:40:22
 * Last Modified: 2023-08-02 13:25:16
 * Description:  This file is for pciec configuration table
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2023/08/06        first release
 */


#include "fpciec.h"
#include "fparameters.h"

extern FPcieCConfig FPcieCConfigTable[FPCIEC_INSTANCE_NUM];

/**
 * @name: FPcieCLookupConfig
 * @msg: Looks up and retrieves the configuration structure for a specific instance ID.
 * @param {u32} instance_id - The instance ID for which to retrieve the configuration.
 * @return {FPcieCConfig *} - Returns a pointer to the configuration structure, or NULL if not found.
 */
FPcieCConfig *FPcieCLookupConfig(u32 instance_id)
{
    FPcieCConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FPCIEC_INSTANCE_NUM; index++)
    {
        if (FPcieCConfigTable[index].instance_id == instance_id)
        {
            ptr = &FPcieCConfigTable[index];
            break;
        }
    }

    return (FPcieCConfig *)ptr;
}
