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
 * FilePath: fpcie_sinit.c
 * Created Date: 2023-08-01 09:08:08
 * Last Modified: 2023-08-07 08:58:42
 * Description:  This file is for pcie ecam configuration table
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2023/08/06        first release
 */


#include "fpcie_ecam.h"
#include "fparameters.h"

extern FPcieEcamConfig FPcieEcamConfigTable[FPCIE_ECAM_INSTANCE_NUM];


FPcieEcamConfig *FPcieEcamLookupConfig(u32 instance_id)
{
    FPcieEcamConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FPCIE_ECAM_INSTANCE_NUM; index++)
    {
        if (FPcieEcamConfigTable[index].instance_id == instance_id)
        {
            ptr = &FPcieEcamConfigTable[index];
            break;
        }
    }

    return (FPcieEcamConfig *)ptr;
}
