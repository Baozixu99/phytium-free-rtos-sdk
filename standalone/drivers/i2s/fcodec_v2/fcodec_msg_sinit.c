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
 * FilePath: fcodec_msg_sinit.c
 * Created Date: 2025-05-19 11:14:45
 * Last Modified: 2025-05-21 14:10:20
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fcodec_msg.h"
#include "fparameters.h"
/************************** Variable Definitions *****************************/
extern const FCodecMsgConfig FCodec_MSG_CONFIG_TBL[FCODEC_MSG_NUM];
/************************** Function Prototypes ******************************/
/**
 * @name: FCodecMsgLookupConfig
 * @msg: 获取Codec驱动的默认配置参数
 * @return {const FCodecMsgConfig*} 驱动默认参数
 * @param {u32} instance_id, 当前控制的Codec控制器实例号
 */
const FCodecMsgConfig *FCodecMsgLookupConfig(u32 instance_id)
{
    const FCodecMsgConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FCODEC_MSG_NUM; index++)
    {
        if (FCodec_MSG_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FCodec_MSG_CONFIG_TBL[index];
            break;
        }
    }

    return (const FCodecMsgConfig *)ptr;
}
