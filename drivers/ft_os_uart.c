/*
 * @ : Copyright (c) 2020 Phytium Information Technology, Inc. 
 * The right to copy, distribute, modify, or otherwise make use 
 * of this software may be licensed only pursuant to the terms
 * of an applicable Phytium license agreement.
 * @Date: 2021-04-07 09:53:07
 * @LastEditTime: 2021-04-07 15:40:07
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include "ft_os_uart.h"
#include "pl011_uart.h"
#include "interrupt.h"
#include "ft_types.h"
#include "ft_assert.h"
#include "sdkconfig.h"
#include <stdio.h>

#ifdef CONFIG_FREERTOS_USE_UART
/* Callback events  */

static void FtFreeRtosUartCallback(void *args, u32 event, u32 event_data)
{
    FtFreertosUart *uart_p = (FtFreertosUart *)args;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;
    if (FUART_EVENT_RECV_DATA == event || FUART_EVENT_RECV_TOUT == event)
    {
        x_result = xEventGroupSetBitsFromISR(uart_p->rx_event, RTOS_UART_COMPLETE, &xhigher_priority_task_woken);
    }
    else if (FUART_EVENT_RECV_ERROR == event)
    {
        x_result = xEventGroupSetBitsFromISR(uart_p->rx_event, RTOS_UART_RECV_ERROR, &xhigher_priority_task_woken);
    }
    else if (FUART_EVENT_SENT_DATA == event)
    {

        x_result = xEventGroupSetBitsFromISR(uart_p->tx_event, RTOS_UART_RECV_ERROR, &xhigher_priority_task_woken);
    }
    else if (FUART_EVENT_PARE_FRAME_BRKE == event)
    {
        x_result = xEventGroupSetBitsFromISR(uart_p->rx_event, RTOS_UART_RECV_ERROR, &xhigher_priority_task_woken);
    }
    else if (FUART_EVENT_RECV_ORERR == event)
    {
    }

    if (FUART_EVENT_SENT_DATA == event)
    {
    }
    else
    {
        Pl011IrqClearReciveTimeOut(&uart_p->bsp_uart);
    }

    if (x_result != pdFAIL)
    {
        portYIELD_FROM_ISR(xhigher_priority_task_woken);
    }
}

static void FtFreertosUartIrq(s32 vector, void *param)
{
    Pl011InterruptHandler(param);
}

void FtFreertosUartInit(FtFreertosUart *uart_p, FtFreertosUartConfig *config_p)
{
    Pl011 *bsp_uart_p = NULL;
    ft_error_t ret;
    FT_ASSERTVOID(uart_p != NULL);
    FT_ASSERTVOID(config_p != NULL);
    bsp_uart_p = &uart_p->bsp_uart;
    ret = Pl011CfgInitialize(bsp_uart_p, Pl011LookupConfig(config_p->uart_instance));
    FT_ASSERTVOID(FT_SUCCESS == ret);
    Pl011SetHandler(bsp_uart_p, FtFreeRtosUartCallback, uart_p);
    FT_ASSERTVOID((uart_p->rx_semaphore = xSemaphoreCreateMutex()) != NULL);
    FT_ASSERTVOID((uart_p->tx_semaphore = xSemaphoreCreateMutex()) != NULL);
    FT_ASSERTVOID((uart_p->tx_event = xEventGroupCreate()) != NULL);
    FT_ASSERTVOID((uart_p->rx_event = xEventGroupCreate()) != NULL);
    InterruptSetPriority(bsp_uart_p->config.irq_num, 0xd0);
    InterruptInstall(bsp_uart_p->config.irq_num, FtFreertosUartIrq, bsp_uart_p, "uart1");
    InterruptUmask(bsp_uart_p->config.irq_num);
    Pl011SetOptions(bsp_uart_p, FUART_OPTION_UARTEN | FUART_OPTION_RXEN | FUART_OPTION_TXEN | FUART_OPTION_FIFOEN);
}

s32 FtFreertosUartReceiveBuffer(FtFreertosUart *uart_p, u8 *buffer, u32 length, u32 *received_length)
{
    u32 get_length;
    ft_error_t ret = FT_SUCCESS;
    Pl011 *bsp_uart_p = NULL;
    EventBits_t ev;
    FT_ASSERTNONVOID(NULL != uart_p);
    FT_ASSERTNONVOID(NULL != buffer);
    bsp_uart_p = &uart_p->bsp_uart;

    /* New transfer can be performed only after current one is finished */
    if (pdFALSE == xSemaphoreTake(uart_p->rx_semaphore, portMAX_DELAY))
    {
        /* We could not take the semaphore, exit with 0 data received */
        ret = FREERTOS_UART_SEM_ERROR;
    }

    get_length = Pl011Receive(bsp_uart_p, buffer, length);
    if (get_length > 0)
    {
        *received_length = get_length;
        ret = FT_SUCCESS;
    }
    else
    {
        Pl011IrqEnableReciveTimeOut(bsp_uart_p);

        ev = xEventGroupWaitBits(uart_p->rx_event,
                                 RTOS_UART_COMPLETE | RTOS_UART_HARDWARE_BUFFER_OVERRUN | RTOS_UART_RECV_ERROR,
                                 pdTRUE, pdFALSE, portMAX_DELAY);

        if (ev & RTOS_UART_HARDWARE_BUFFER_OVERRUN)
        {
            ret = FREERTOS_UART_FIFO_ERROR;
            *received_length = 0;
        }
        else if (ev & RTOS_UART_COMPLETE)
        {
            ret = FT_SUCCESS;
            *received_length = bsp_uart_p->receive_buffer.requested_bytes - bsp_uart_p->receive_buffer.remaining_bytes;
        }
        else if (ev & RTOS_UART_RECV_ERROR)
        {
            ret = FREERTOS_UART_RECV_ERROR;
            *received_length = 0;
        }
        else
        {
            ret = FREERTOS_UART_INVAILD_PARAM;
            *received_length = 0;
        }
    }

    /* Enable next transfer. Current one is finished */
    if (pdFALSE == xSemaphoreGive(uart_p->rx_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        printf("FST_FAILURE xSemaphoreGive \r\n");
        ret = FREERTOS_UART_RECV_ERROR;
    }

    return ret;
}

s32 FtFreertosUartBlcokingSend(FtFreertosUart *uart_p, u8 *buffer, u32 length)
{
    ft_error_t ret = FT_SUCCESS;
    Pl011 *bsp_uart_p = NULL;
    EventBits_t ev;
    FT_ASSERTNONVOID(NULL == uart_p);
    FT_ASSERTNONVOID(NULL == buffer);
    bsp_uart_p = &uart_p->bsp_uart;

    if (pdFALSE == xSemaphoreTake(uart_p->tx_semaphore, portMAX_DELAY))
    {
        return FREERTOS_UART_SEM_ERROR;
    }
    Pl011Send(bsp_uart_p, buffer, length);

    ev = xEventGroupWaitBits(uart_p->tx_event, RTOS_UART_COMPLETE, pdTRUE, pdFALSE, portMAX_DELAY);
    if (!(ev & RTOS_UART_COMPLETE))
    {
        ret = FREERTOS_UART_EVENT_ERROR;
    }

    if (pdFALSE == xSemaphoreGive(uart_p->tx_semaphore))
    {
        /* We could not post the semaphore, exit with error */
        ret = FREERTOS_UART_SEM_ERROR;
    }

    return ret;
}


#endif
