/*
 * @ : Copyright (c) 2020 Phytium Information Technology, Inc. 
 * The right to copy, distribute, modify, or otherwise make use 
 * of this software may be licensed only pursuant to the terms
 * of an applicable Phytium license agreement.
 * @Date: 2021-04-07 09:53:07
 * @LastEditTime: 2021-04-07 15:40:12
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef FT_OS_UART_H
#define FT_OS_UART_H

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include "ft_error_code.h"
#include "pl011_uart.h"
#include "ft_types.h"

#define FREERTOS_UART_SEM_ERROR FT_CODE_ERR(ErrModPort, 0, 0x1)
#define FREERTOS_UART_EVENT_ERROR FT_CODE_ERR(ErrModPort, 0, 0x2)
#define FREERTOS_UART_FIFO_ERROR FT_CODE_ERR(ErrModPort, 0, 0x3)
#define FREERTOS_UART_RECV_ERROR FT_CODE_ERR(ErrModPort, 0, 0x4)
#define FREERTOS_UART_INVAILD_PARAM FT_CODE_ERR(ErrModPort, 0, 0x5)

#define UART0_ID FT_UART0_ID
#define UART1_ID FT_UART1_ID
#define UART2_ID FT_UART2_ID
#define UART3_ID FT_UART3_ID

/*!
* @cond RTOS_PRIVATE
* @name UART FreeRTOS handler
*
* These are the only valid states for txEvent and rxEvent 
*/
/*@{*/
/*! @brief Event flag - transfer complete. */
#define RTOS_UART_COMPLETE 0x1
/*! @brief Event flag - hardware buffer overrun. */
#define RTOS_UART_HARDWARE_BUFFER_OVERRUN 0x2
/*！ @brief Event flag Receive is error */
#define RTOS_UART_RECV_ERROR 0x4

/*@}*/

typedef struct
{
    u32 uart_instance; /* select uart global object */
    u32 isr_priority;  /* irq Priority */
} FtFreertosUartConfig;

typedef struct
{
    // Ft_Uart UartObj; /* Uart Object */
    Pl011 bsp_uart;
    SemaphoreHandle_t rx_semaphore; /*!< RX semaphore for resource sharing */
    SemaphoreHandle_t tx_semaphore; /*!< TX semaphore for resource sharing */
    EventGroupHandle_t rx_event;    /*!< RX completion event */
    EventGroupHandle_t tx_event;    /*!< TX completion event */
} FtFreertosUart;

void FtFreertosUartInit(FtFreertosUart *uart_p, FtFreertosUartConfig *config_p);
s32 FtFreertosUartBlcokingSend(FtFreertosUart *uart_p, u8 *buffer, u32 length);
s32 FtFreertosUartReceiveBuffer(FtFreertosUart *uart_p, u8 *buffer, u32 length, u32 *received_length);

#endif // !
