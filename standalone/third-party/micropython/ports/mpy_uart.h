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
 * FilePath: mpy_uart.h
 * Date: 2023-12-07 14:53:41
 * LastEditTime: 2023-12-07 17:36:39
 * Description:  This file is for defining the function
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/12/07   Modify the format and establish the version
 */

#ifndef MP_UART_H
#define MP_UART_H

#include "ftypes.h"
#ifdef __cplusplus
extern "C"
{
#endif

int mp_hal_stdin_rx_chr(void);

void mp_hal_stdout_tx_strn(char *str, mp_uint_t len);

void MpySerialConfig(void);

#ifdef __cplusplus
}
#endif

#endif