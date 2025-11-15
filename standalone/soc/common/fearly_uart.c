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
 * FilePath: early_uart.c
 * Date: 2022-02-11 13:33:28
 * LastEditTime: 2022-02-17 17:59:26
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   rtos       2022/6/25    init commit
 * 1.1   zhangyan   2023/7/11    modify
 */

/***************************** Include Files *********************************/
#include "fkernel.h"
#include "fio.h"
#include "fparameters.h"
#include "fearly_uart.h"
#include "sdkconfig.h"

#if defined(CONFIG_USE_SERIAL_V2) && defined(CONFIG_DEFAULT_DEBUG_PRINT_UART2)

#include "fuart_msg.h"
#include "fio_mux.h"
#include "fsleep.h"
#include "fassert.h"

static FUartMsg early_uart;
FPrintfCall printf_call = NULL;

static void OutByte(s8 byte)
{
    FUartMsgBlockSend(&early_uart, (u8 *)&byte, 1);
}

char GetByte(void)
{
    return (char)(FUartMsgBlockReceive(&early_uart));
}

void FInitializePrintf(FPrintfCall func)
{
    printf_call = func;
}

void FEarlyUartProbe(void)
{
    FUartMsgConfig config;
    FInitializePrintf(&OutByte);
    config = *FUartMsgLookupConfig(EARLY_UART_CTRL_ID);

    FIOMuxInit();
    FIOPadSetUartMux(EARLY_UART_CTRL_ID);

    FUartMsgCfgInitialize(&early_uart, &config);
    FUartMsgSetStartUp(&early_uart);
    return;
}

#else

#include "fpl011.h"
/**************************** Type Definitions *******************************/
static FPl011 early_uart;
FPrintfCall printf_call = NULL;

static void OutByte(s8 byte)
{
    FPl011BlockSend(&early_uart, (u8 *)&byte, 1);
}

char GetByte(void)
{
    return (char)(FPl011BlockReceive(&early_uart));
}

void FInitializePrintf(FPrintfCall func)
{
    printf_call = func;
}

void FEarlyUartProbe(void)
{
    FPl011Config config;
    FInitializePrintf(&OutByte);
    config = *FPl011LookupConfig(EARLY_UART_CTRL_ID);

#ifdef CONFIG_UART_INIT_NOT_CLR_INTR
    config.init_flg = FPL011_INIT_NEED_NOT_CLEAN_ISR_MASK; /* bit 0 is 1: not need clean isr mask. */
#endif
    FPl011CfgInitialize(&early_uart, &config);
    FPl011SetOptions(&early_uart, FPL011_OPTION_UARTEN | FPL011_OPTION_RXEN |
                                      FPL011_OPTION_TXEN | FPL011_OPTION_FIFOEN);
    return;
}

#endif