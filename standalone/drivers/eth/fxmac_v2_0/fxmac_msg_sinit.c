/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fxmac_msg_sinit.c
 * Date: 2024-10-28 14:46:52
 * LastEditTime: 2024-10-28 14:46:52
 * Description:  This file is for the implementation of the xmac msg driver's static
 * initialization functionality.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/28  first release
 */

#include "fxmac_msg.h"
#include "fparameters.h"

extern const FXmacMsgConfig fxmac_msg_cfg_tbl[FXMAC_MSG_NUM];

const FXmacMsgConfig *FXmacMsgLookupConfig(u32 instance_id)
{
    const FXmacMsgConfig *cfg_ptr = NULL;
    u32 index;
    for (index = 0; index < (u32)FXMAC_MSG_NUM; index++)
    {
        if (fxmac_msg_cfg_tbl[index].instance_id == instance_id)
        {
            cfg_ptr = &fxmac_msg_cfg_tbl[index];
            break;
        }
    }

    return (const FXmacMsgConfig *)cfg_ptr;
}
