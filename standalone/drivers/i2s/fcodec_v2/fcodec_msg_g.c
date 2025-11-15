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
 * FilePath: fcodec_msg_g.c
 * Created Date: 2025-05-19 11:10:20
 * Last Modified: 2025-05-21 14:09:51
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

/*  - This file contains a configuration table that specifies the configuration
- 驱动全局变量定义，包括静态配置参数 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fcodec_msg.h"
#include "fparameters.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/**
 * @name: FCODEC_MSG_CONFIG_TBL
 * @msg: CODEC驱动的默认配置参数
 */

const FCodecMsgConfig FCodec_MSG_CONFIG_TBL[FCODEC_MSG_NUM] =
{
[FCODEC0_MSG_ID] =
    {
        .instance_id = FCODEC0_MSG_ID,
        .msg =
        {
            .shmem = FCODEC0_MSG_SHARE_MEM_ADDR,
            .regfile = FCODEC0_MSG_REGFILE_ADDR ,
        }
    },
};



