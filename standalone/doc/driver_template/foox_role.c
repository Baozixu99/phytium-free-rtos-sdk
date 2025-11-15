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
 * FilePath: foox_role.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:25:03
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */


/*
    - 一些驱动模块，直接操作硬件的I/O接口，无法实现有意义的操作，此时需要针对中间件或者用户使用习惯设计此模块 （i2c,nand,eth）
    - 部分场景适用, 分角色的 I/O 操作
    - 此模块的函数原型，在fooxx.h 中声明一次，方便用户或者中间件层调用

*/


/***************************** Include Files *********************************/

#include "fio.h"
#include "foox_hw.h"
#include "fooxx.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

