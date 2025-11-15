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
 * FilePath: fi2s_msg_sinit.c
 * Created Date: 2025-05-14 15:00:06
 * Last Modified: 2025-05-27 16:28:10
 * Description:  This file is for
 *
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */
/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fi2s_msg.h"
#include "fparameters.h"
/************************** Variable Definitions *****************************/
extern const FI2sMsgConfig FI2S_MSG_CONFIG_TBL[FI2S_MSG_NUM];
/************************** Function Prototypes ******************************/
/**
 * @name: FI2sMsgLookupConfig
 * @msg: 获取I2S驱动的默认配置参数
 * @return {const FI2sMsgConfig*} 驱动默认参数
 * @param {u32} instance_id, 当前控制的I2S控制器实例号
 */
const FI2sMsgConfig *FI2sMsgLookupConfig(u32 instance_id)
{
    const FI2sMsgConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FI2S_MSG_NUM; index++)
    {
        if (FI2S_MSG_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FI2S_MSG_CONFIG_TBL[index];
            break;
        }
    }

    return (const FI2sMsgConfig *)ptr;
}
