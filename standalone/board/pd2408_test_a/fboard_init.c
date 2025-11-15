/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * @FilePath: fboard_init.c
 * @Date: 2024-06-24 19:50:35
 * @LastEditTime: 2024-06-24 19:50:35
 * @Description:  This file is for io and status init
 * 
 * @Modify History: 
 *  Ver   Who    Date           Changes
 * ----- ------  --------       --------------------------------------
 * 1.0   wangzq   2024/06/24      first release
 */
#include "strto.h"
#include "fboard_port.h"
#include "fboard_init.h"
#include "fio_mux.h"
#include "fparameters.h"

int FBoardStatusInit(void *para)
{
    FIOPadSetUartMux(FUART1_ID);

    return 0;
}
