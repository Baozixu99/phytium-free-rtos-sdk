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
 * FilePath: fxmac_sinit.c
 * Date: 2022-04-06 14:46:52
 * LastEditTime: 2022-04-06 14:46:58
 * Description:  This file is for the implementation of the xmac driver's static
 * initialization functionality.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/06/16    first release
 */

#include "fxmac.h"
#include "fparameters.h"

extern const FXmacConfig fxmac_cfg_tbl[FXMAC_NUM];

const FXmacConfig *FXmacLookupConfig(u32 instance_id)
{
    const FXmacConfig *cfg_ptr = NULL;
    u32 index;
    for (index = 0; index < (u32)FXMAC_NUM; index++)
    {
        if (fxmac_cfg_tbl[index].instance_id == instance_id)
        {
            cfg_ptr = &fxmac_cfg_tbl[index];
            break;
        }
    }

    return (const FXmacConfig *)cfg_ptr;
}
