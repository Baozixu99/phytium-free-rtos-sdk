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
 * FilePath: fgdma_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:25:15
 * Description:  This file is for gdma static init
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2021/11/5    init commit
 * 1.1   zhugengyu  2022/5/16    modify according to tech manual.
 */

/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fparameters.h"

#include "fgdma.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
extern const FGdmaConfig fgdma_cfg_tbl[FGDMA_INSTANCE_NUM];
/************************** Function Prototypes ******************************/
/**
 * @name: FGdmaLookupConfig
 * @msg: 获取GDMA控制器默认配置
 * @return {const FGdmaConfig *} 控制器默认配置
 * @param {u32} instance_id, GDMA控制器ID
 */
const FGdmaConfig *FGdmaLookupConfig(u32 instance_id)
{
    const FGdmaConfig *ptr = NULL;

    for (int index = 0; index < (u32)FGDMA_INSTANCE_NUM; index++)
    {
        if (fgdma_cfg_tbl[index].instance_id == instance_id)
        {
            ptr = &fgdma_cfg_tbl[index];
            break;
        }
    }

    return (const FGdmaConfig *)ptr;
}