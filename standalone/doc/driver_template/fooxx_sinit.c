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
 * FilePath: fooxx_sinit.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:25:15
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */



/* - This file contains the implementation of driver's static initialization functionality.
- 驱动静态初始化  */

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"
#include "fooxx.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern const FooxxConfig FOOXX_CONFIG_TBL[NUM];

/************************** Function Prototypes ******************************/
const FooxxConfig *FooxxLookupConfig(u32 instance_id)
{
    const FooxxConfig *ptr = NULL;
    u32 index;

    for (index = 0; index < (u32)NUM; index++)
    {
        if (FOOXX_CONFIG_TBL[index].instance_id == instance_id)
        {
            ptr = &FOOXX_CONFIG_TBL[index];
            break;
        }
    }

    return (const FooxxConfig *)ptr;
}