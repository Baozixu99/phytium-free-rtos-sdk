/*
 * Copyright (C) 2021, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fi2c_sinit.c
 * Date: 2021-11-01 14:53:42
 * LastEditTime: 2022-02-18 08:36:52
 * Description:  This file is for implementation of driver's static initialization functionality
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhugengyu 2021/11/1  first commit
 */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fi2c.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/
#if defined(FI2C_NUM)
extern const FI2cConfig FI2C_CONFIG_TBL[FI2C_NUM];
/************************** Function Prototypes ******************************/
/**
 * @name: FI2cLookupConfig
 * @msg: 获取I2C驱动的默认配置参数
 * @return {const FI2cConfig*} 驱动默认参数
 * @param {u32} instance_id, 当前控制的I2C控制器实例号
 */
const FI2cConfig *FI2cLookupConfig(u32 instance_id)
{
    const FI2cConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)FI2C_NUM; index++)
    {
        if (FI2C_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FI2C_CONFIG_TBL[index];
            break;
        }
    }

    return (const FI2cConfig *)ptr;
}
#endif