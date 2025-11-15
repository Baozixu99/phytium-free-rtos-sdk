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
 * FilePath: fgpio_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:25:29
 * Description:  This files is for GPIO static variables implementation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/3/1     init commit
 * 2.0   zhugengyu  2022/7/1     support e2000
 * 3.0   zhugengyu  2024/5/7     modify interface to use gpio by pin
 */


/***************************** Include Files *********************************/
#include "fparameters.h"

#include "fgpio_hw.h"
#include "fgpio.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
extern FGpioConfig fgpio_cfg_tbl[FGPIO_NUM];

/*****************************************************************************/
/**
 * @name: FGpioLookupConfig
 * @msg: 获取GPIO引脚的默认配置
 * @return {const FGpioConfig *} GPIO引脚的参数和默认配置
 * @param {u32} gpio_id, GPIO引脚号, 0 ~ FGPIO_NUM，可以通过宏 FGPIO_ID 获取编码
 */
const FGpioConfig *FGpioLookupConfig(u32 gpio_id)
{
    FGpioConfig *ptr = NULL;
    u32 index;
    FASSERT_MSG((gpio_id < FGPIO_NUM), "Instance_id is invalid.");

    for (index = 0; index < FGPIO_NUM; index++) /* find configs of GPIO */
    {
        if (fgpio_cfg_tbl[index].id == gpio_id)
        {
            ptr = &fgpio_cfg_tbl[index];
            break;
        }
    }

    return ptr;
}
