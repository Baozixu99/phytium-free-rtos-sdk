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
 * FilePath: fspim_msg_sinit.c
 * Date: 2024-10-14 11:27:42
 * LastEditTime: 2024-10-24 17:46:03
 * Description:  This file is for providing spi msg config init definition.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2024/10/24   first release
 */

#include "ftypes.h"
#include "fparameters.h"
#include "fspim_msg.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern const FSpiMsgConfig FSPI_MSG_CONFIG_TBL[FSPI_MSG_NUM];

/************************** Function Prototypes ******************************/
/**
 * @name: FSpiMsgLookupConfig
 * @msg: 获取FSpiMsgCtrl实例的默认配置
 * @return {const FSpiMsgConfig *} FSPIM实例的默认配置
 * @param {u32} instance_id, SPI控制器ID
 */
const FSpiMsgConfig *FSpiMsgLookupConfig(u32 instance_id)
{
    const FSpiMsgConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FSPI_MSG_NUM; index++)
    {
        if (FSPI_MSG_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FSPI_MSG_CONFIG_TBL[index];
        }
    }

    return (const FSpiMsgConfig *)ptr;
}
