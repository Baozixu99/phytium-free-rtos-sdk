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
 * FilePath: mphalport.h
 * Date: 2023-12-07 14:53:41
 * LastEditTime: 2023-12-07 17:36:39
 * Description:  This file is for defining the function
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/12/07   Modify the format and establish the version
 */

#ifndef MP_HALPORT_H
#define MP_HALPORT_H

#include "ftypes.h"
#ifdef __cplusplus
extern "C"
{
#endif


static inline mp_uint_t mp_hal_ticks_ms(void)
{
    return 0;
}
static inline void mp_hal_set_interrupt_char(char c)
{
}


#ifdef __cplusplus
}
#endif

#endif