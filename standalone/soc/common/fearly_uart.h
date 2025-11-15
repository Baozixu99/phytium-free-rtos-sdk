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
 * FilePath: fearly_uart.h
 * Date: 2022-02-11 13:33:28
 * LastEditTime: 2022-02-17 18:00:16
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   rtos       2022/6/25    init commit
 * 1.1   zhangyan   2023/7/11    reconstruct
 */
#ifndef COMMON_FEARLY_UART_H
#define COMMON_FEARLY_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fio.h"
#include "fparameters.h"
#include "sdkconfig.h"

/**************************** Type Definitions *******************************/

/************************** Constant Definitions *****************************/
#if defined(CONFIG_DEFAULT_DEBUG_PRINT_UART2)
#define EARLY_UART_CTRL_ID FUART2_ID
#elif defined(CONFIG_DEFAULT_DEBUG_PRINT_UART0)
#define EARLY_UART_CTRL_ID FUART0_ID
#else
#define EARLY_UART_CTRL_ID FUART1_ID
#endif

#define STDOUT_BASEADDRESS
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
char GetByte(void);
typedef void (*FPrintfCall)(s8 c);
extern FPrintfCall printf_call;
void FInitializePrintf(FPrintfCall func); /*注意，当定义printf输出接口时，请确保输出接口准备就绪，例如此时应确保FPl011Block对应的接口已可以使用*/
#ifdef __cplusplus
}
#endif

#endif