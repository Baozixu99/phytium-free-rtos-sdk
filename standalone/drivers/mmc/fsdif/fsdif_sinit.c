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
 * FilePath: fsdif_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:48:41
 * Description:  This file is for static init
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/2    init
 * 1.1   zhugengyu  2022/6/6     modify according to tech manual.
 * 2.0   zhugengyu  2023/9/16    rename as sdif, support SD 3.0 and rework clock timing
 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fsdif.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern const FSdifConfig FSDIF_CONFIG_TBL[FSDIF_NUM];

/************************** Function Prototypes ******************************/
/**
 * @name: FSdifLookupConfig
 * @msg: Get the device instance default configure
 * @return {const FSdifConfig *} default configure
 * @param {u32} instance_id
 */
const FSdifConfig *FSdifLookupConfig(u32 instance_id)
{
    const FSdifConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FSDIF_NUM; index++)
    {
        if (FSDIF_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FSDIF_CONFIG_TBL[index];
            break;
        }
    }

    return (const FSdifConfig *)ptr;
}