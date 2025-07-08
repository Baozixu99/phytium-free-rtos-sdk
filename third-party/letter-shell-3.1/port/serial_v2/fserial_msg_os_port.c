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
 * FilePath: fserial_msg_os_port.c
 * Date: 2025-04-24 21:42:27
 * LastEditTime: 2025-05-13 21:42:27
 * Description:  This file is for letter shell port to serial_v2
 *
 * Modify History:
 *  Ver   Who           Date         Changes
 * ----- ------       --------    --------------------------------------
 * 1.0   liyilun       2025/5/13   first release
 */


#include <string.h>
#include "fassert.h"
#include "fparameters.h"
#include "shell_port.h"
#include "finterrupt.h"
#include "fuart_msg_os.h"
#include "fio_mux.h"
#include "fsleep.h"

FtFreertosUartMsg os_uart_msg;
extern Shell shell_object;
static char data[64];


#ifdef CONFIG_DEFAULT_LETTER_SHELL_USE_UART1
    #define LETTER_SHELL_UART_ID    UART1_ID
#endif

#ifdef CONFIG_DEFAULT_LETTER_SHELL_USE_UART0
    #define LETTER_SHELL_UART_ID    UART0_ID
#endif

#ifdef CONFIG_DEFAULT_LETTER_SHELL_USE_UART2
    #define LETTER_SHELL_UART_ID    UART2_ID
#endif

#ifdef CONFIG_LETTER_SHELL_UART_MSG_INTR_MODE

/**
 * @brief 用户shell写
 *
 * @param data 数据
 */
void LSUserShellWrite(char data)
{
    u32 wait_times = 100;
    while(0 == FUartMsgSend(&os_uart_msg.bsp_uart_msg, &data, 1))
    {
        fsleep_microsec(20);
        wait_times--;
        if(wait_times == 0)
        {
            printf("error\r\n");
            break;
        }
    }

}

/**
 * @brief 用户shell读
 *
 * @param data 数据
 * @return char 状态
 */
signed char LSUserShellRead(char *data)
{
    u32 received_length;
    FtFreertosUartMsgReceiveBuffer(&os_uart_msg, data, 1, &received_length);
    return 0;
}

void LSSerialMsgConfig(void)
{
        FtFreertosUartMsgConfig config =
    {
        .uart_msg_instance = LETTER_SHELL_UART_ID, /* select uart global object */
        .isr_priority = IRQ_PRIORITY_VALUE_13,  /* irq Priority */
        .isr_event_mask = RTOS_RX_INTR_EVENT_MASK | RTOS_TX_INTR_EVENT_MASK,  /* irq event mask */
        .uart_baudrate = 115200
    };
    FtFreertosUartMsgInit(&os_uart_msg, &config);
}

void LSSerialMsgWaitLoop(void)
{
    u32 recive_length = 0;
    u32 i = 0;

    while (TRUE)
    {
        FtFreertosUartMsgReceiveBuffer(&os_uart_msg, data, sizeof(data), &recive_length);
        for (i = 0; i < recive_length; i++)
        {
            shellHandler(&shell_object, data[i]);
        }
    }
    vTaskDelete(NULL);    
}
#elif defined CONFIG_LETTER_SHELL_UART_MSG_POLLED_MODE

#define LS_UART_WAIT_LOOP_DELAY 5
    /**
 * @brief 用户shell写
 *
 * @param data 数据
 */
void LSUserShellWrite(char data)
{
    FUartMsgBlockSend(&os_uart_msg.bsp_uart_msg, (u8 *)&data, 1);
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
    length = FUartMsgReceive(&os_uart_msg.bsp_uart_msg,(u8 *)data,1);
    return (length > 0)? 1:0;
}

void LSSerialMsgConfig(void)
{
    s32 ret = FT_SUCCESS;
    u8 modem = 0;
    const FUartMsgConfig * config_p;
    FUartMsgConfig config_value;
    memset(&os_uart_msg.bsp_uart_msg, 0, sizeof(&os_uart_msg.bsp_uart_msg));
    config_p = FUartMsgLookupConfig(LETTER_SHELL_UART_ID) ;
    memcpy(&config_value, config_p, sizeof(FUartMsgConfig)); 

    /*init iomux*/
    FIOMuxInit();
    FIOPadSetUartMux(LETTER_SHELL_UART_ID);

    /* 初始化PL011 */
    ret = FUartMsgCfgInitialize(&os_uart_msg.bsp_uart_msg, &config_value);
    FASSERT(FT_SUCCESS == ret);
    modem = FUartMsgSetStartUp(&os_uart_msg.bsp_uart_msg);
}


void LSSerialMsgWaitLoop(void)
{
    u32 recive_length = 0;
    u32 i = 0;

    while (TRUE)
    {
        FtFreertosUartMsgReceiveBuffer(&os_uart_msg, data, sizeof(data), &recive_length);
        for (i = 0; i < recive_length; i++)
        {
            shellHandler(&shell_object, data[i]);
        }
        vTaskDelay(LS_UART_WAIT_LOOP_DELAY);
    }
    vTaskDelete(NULL);  
}
#endif