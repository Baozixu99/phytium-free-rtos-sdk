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
 * FilePath: mpy_uart.c
 * Created Date: 2023-12-01 15:22:57
 * Last Modified: 2024-03-22 16:53:09
 * Description:  This file is for the uart of micropython
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/12/07   Modify the format and establish the version
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fpl011.h"
#include "fassert.h"
#include "fparameters.h"
#include "mpconfig.h"

#define MPY_SHELL_UART_ID FUART1_ID
static FPl011 serial;

void MpySerialConfig(void)
{
    s32 ret = FT_SUCCESS;

    const FPl011Config *config_p;
    FPl011Config config_value;
    memset(&serial, 0, sizeof(serial));
    config_p = FPl011LookupConfig(MPY_SHELL_UART_ID);
    memcpy(&config_value, config_p, sizeof(FPl011Config));
    /* 初始化PL011 */
    ret = FPl011CfgInitialize(&serial, &config_value);
    FASSERT(FT_SUCCESS == ret);
    FPl011SetOptions(&serial, FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN |
                                  FPL011_OPTION_TXEN | FPL011_OPTION_FIFOEN);
    return;
}

// Receive single character
int mp_hal_stdin_rx_chr(void)
{
    u8 data = 0;
    int length = FPl011Receive(&serial, &data, 1);
    (void)length;
    return data;
}

// Send string of given length
void mp_hal_stdout_tx_strn(char *str, mp_uint_t len)
{
    FPl011BlockSend(&serial, (u8 *)str, len);
}
