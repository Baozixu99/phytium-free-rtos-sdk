/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fserial_v2_port.c
 * Date: 2025-04-30 14:53:43
 * LastEditTime: 2025-05-13 11:47:23
 * Description:  This files is for letter shell port to uart_msg 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2025/5/13     init commit
 */


#include <string.h>
#include "fassert.h"
#include "fparameters.h"
#include "../shell_port.h"
#include "finterrupt.h"
#include "fuart_msg.h"
#include "sdkconfig.h"
#include "fio_mux.h"

static FUartMsg uart_msg;
extern Shell shell_object;

#if defined(CONFIG_DEFAULT_LETTER_SHELL_USE_UART2)
#define LETTER_SHELL_UART_ID FUART2_MSG_ID
#elif defined(CONFIG_DEFAULT_LETTER_SHELL_USE_UART0)
#define LETTER_SHELL_UART_ID FUART0_MSG_ID
#else
#define LETTER_SHELL_UART_ID FUART1_MSG_ID
#endif


/**
 * @brief 用户shell写
 *
 * @param data 数据
 */
void LSUserShellWrite(char data)
{
    FUartMsgBlockSend(&uart_msg, (u8 *)&data, 1);
}

/**
 * @brief 用户shell读
 *
 * @param data 数据
 * @return char 状态
 */
signed char LSUserShellRead(char *data)
{
    u32 length = 0;
    length = FUartMsgReceive(&uart_msg, (u8 *)data, 1);
    return (length > 0) ? 1 : 0;
}


void LSSerialConfig(void)
{
    s32 ret = FT_SUCCESS;
    const FUartMsgConfig *config_p;
    FUartMsgConfig config_value;
    memset(&uart_msg, 0, sizeof(uart_msg));
    config_p = FUartMsgLookupConfig(LETTER_SHELL_UART_ID);
    memcpy(&config_value, config_p, sizeof(FUartMsgConfig));

    /*init iomux*/
    FIOMuxInit();
    FIOPadSetUartMux(LETTER_SHELL_UART_ID);
    /* 初始化uart_msg */
    ret = FUartMsgCfgInitialize(&uart_msg, &config_value);
    FASSERT(FT_SUCCESS == ret);
    FUartMsgSetStartUp(&uart_msg);
    return;
}

/**
 * @brief 串口有数据输入时就执行相对应的命令,没有数据就阻塞在内部循环中
 *
 * @param 无
 * @return 无
 */

void LSSerialWaitLoop(void)
{
    u8 data[16] = {0};
    u32 get_length = 0, index = 0;

    while (TRUE)
    {
        get_length = FUartMsgReceive(&uart_msg, data, 16);
        for (index = 0; index < get_length; index++)
        {
            shellHandler(&shell_object, data[index]);
        }
    }
}

/**
 * @brief 串口有数据输入时就执行相对应的命令,没有数据程序会正常返回，不会等待
 *
 * @param 无
 * @return 无
 */

void LSSerialBNoWaitLoop(void)
{
    u8 data[16] = {0};
    u32 get_length = 0, index = 0;
    get_length = FUartMsgReceive(&uart_msg, data, 16);

    while (get_length)
    {
        shellHandler(&shell_object, data[index]);
        index++;
        get_length--;
    }
}