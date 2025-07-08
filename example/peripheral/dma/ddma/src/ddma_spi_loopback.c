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
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: ddma_spi_loopback.c
 * Date: 2022-07-20 09:24:39
 * LastEditTime: 2024-04-19 09:24:39
 * Description:  This file is for DDMA task implementations.
 *
 * Modify History:
 *  Ver    Who          Date         Changes
 * -----  ------       --------     --------------------------------------
 *  1.0   zhugengyu    2022/7/27    init commit
 *  1.1   liqiaozhong  2023/11/10   synchronous update with standalone sdk
 *  2.0   liyilun      2024/4/19    add no letter shell mode, adapt to auto test system
 */
/***************************** Include Files *********************************/
#include <string.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "fcache.h"
#include "fassert.h"
#include "fdebug.h"
#include "fkernel.h"
#include "fspim_os.h"
#include "fddma_os.h"
#include "fspim_hw.h"
/************************** Constant Definitions *****************************/
#define TIMER_OUT               (pdMS_TO_TICKS(50000UL))
#define WAIT_DMA_DONE_TICKS     (pdMS_TO_TICKS(10000UL))
#define TX_RX_BUF_LEN           128
#define LOOPBACK_TIMES          3
/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FFreeRTOSSpim *spim = NULL;
static FFreeRTOSDdma *ddma = NULL;

static QueueHandle_t xQueue = NULL;
/* spim test id */
static u32 spi_instance_id = 0U;
static FDdmaChanIndex rx_chan_id = FDDMA_CHAN_0;
static FDdmaChanIndex tx_chan_id = FDDMA_CHAN_1;
static FFreeRTOSRequest rx_request = {0};
static FFreeRTOSRequest tx_request = {0};
static u8 rx_buf[TX_RX_BUF_LEN] __attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT))) = {0};
static u8 tx_buf[TX_RX_BUF_LEN] __attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT))) = {0};

static TaskHandle_t ddma_trans_task;

static const u32 spim_rx_slave_id[FSPI_NUM] =
{
    [FSPI0_ID] = FDDMA0_SPIM0_RX_SLAVE_ID,
    [FSPI1_ID] = FDDMA0_SPIM1_RX_SLAVE_ID,
    [FSPI2_ID] = FDDMA0_SPIM2_RX_SLAVE_ID,
    [FSPI3_ID] = FDDMA0_SPIM3_RX_SLAVE_ID
};
static const u32 spim_tx_slave_id[FSPI_NUM] =
{
    [FSPI0_ID] = FDDMA0_SPIM0_TX_SLAVE_ID,
    [FSPI1_ID] = FDDMA0_SPIM1_TX_SLAVE_ID,
    [FSPI2_ID] = FDDMA0_SPIM2_TX_SLAVE_ID,
    [FSPI3_ID] = FDDMA0_SPIM3_TX_SLAVE_ID
};
/***************** Macros (Inline Functions) Definitions *********************/
#define FDDMA_DEBUG_TAG "DDMA-LP"
#define FDDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_WARN(format, ...)    FT_DEBUG_PRINT_W(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FDDMA_DEBUG_TAG, format, ##__VA_ARGS__)

static void DdmaSpiLoopbackAckDMADone(FDdmaChanIrq *const chan_irq_info_p, void *arg)
{
    FASSERT(chan_irq_info_p);

    BaseType_t xhigher_priority_task_woken = pdFALSE;

    FDDMA_INFO("Ack chan-%d %s done for DDMA.", chan_irq_info_p->channel_id,
               (chan_irq_info_p->channel_id == rx_chan_id) ? "RX" : "TX");
    xTaskNotifyFromISR(ddma_trans_task, BIT(chan_irq_info_p->channel_id), eSetBits, &xhigher_priority_task_woken);
    portYIELD_FROM_ISR(xhigher_priority_task_woken);

    return;
}


static void SpimDdmaInit(FFreeRTOSSpimConifg spim_config, u32 trans_len)
{
    const u32 ddma_id = FDDMA0_ID; /* spi use DDMA-0 only */
    FError err = FT_SUCCESS;
    uintptr spi_base;
    FFreeRTOSDdmaConfig ddma_config;
    spim = FFreeRTOSSpimInit(spi_instance_id, &spim_config); /* init SPIM */
    FASSERT_MSG(spim, "Init SPIM failed.");
    ddma = FFreeRTOSDdmaInit(ddma_id, &ddma_config); /* init DDMA */
    FASSERT_MSG(ddma, "Init DDMA failed.");
    spi_base = spim->ctrl.config.base_addr;
    rx_request.slave_id = spim_rx_slave_id[spi_instance_id];
    rx_request.mem_addr = (uintptr)(void *)rx_buf;
    rx_request.dev_addr = spi_base + FSPIM_DR_OFFSET;
    rx_request.trans_len = trans_len;
    rx_request.is_rx = TRUE;
    rx_request.req_done_handler = DdmaSpiLoopbackAckDMADone;
    rx_request.req_done_args = NULL;
    err = FFreeRTOSDdmaSetupChannel(ddma, rx_chan_id, &rx_request);
    FASSERT_MSG(FT_SUCCESS == err, "Init RX channel failed.");

    tx_request.slave_id = spim_tx_slave_id[spi_instance_id];
    tx_request.mem_addr = (uintptr)(void *)tx_buf;
    tx_request.dev_addr = spi_base + FSPIM_DR_OFFSET;
    tx_request.trans_len = trans_len;
    tx_request.is_rx = FALSE;
    tx_request.req_done_handler = DdmaSpiLoopbackAckDMADone;
    tx_request.req_done_args = NULL;
    err = FFreeRTOSDdmaSetupChannel(ddma, tx_chan_id, &tx_request);
    FASSERT_MSG(FT_SUCCESS == err, "Init TX channel failed.");

}


static void SpimDdmaDeinit(void)
{
    FError err = FT_SUCCESS;
    FDDMA_INFO("Deiniting.....\r\n");
    if (spim)
    {
        err = FFreeRTOSSpimDeInit(spim);
        if(err != FT_SUCCESS)
        {
            FDDMA_ERROR("Spim deinit fail.");
        }
        spim = NULL;
    }
    if (ddma)
    {
        if (FT_SUCCESS != FFreeRTOSDdmaRevokeChannel(ddma, rx_chan_id))
        {
            FDDMA_ERROR("Delete RX channel failed.");
        }

        if (FT_SUCCESS != FFreeRTOSDdmaRevokeChannel(ddma, tx_chan_id))
        {
            FDDMA_ERROR("Delete TX channel failed.");
        }

        err = FFreeRTOSDdmaDeinit(ddma);
        if(err != FT_SUCCESS)
        {
            FDDMA_ERROR("Ddma Deinit fail.");
        }
        ddma = NULL;
    }


    FDDMA_INFO("Deinit success!\r\n");
}

static boolean DdmaSpiLoopbackWaitDmaEnd(void)
{
    boolean ret = TRUE;
    u32 ulNotifiedValue;
    u32 wait_bits = BIT(rx_chan_id) | BIT(tx_chan_id);

    xTaskNotifyWait(pdFALSE, 0xffffffff, &ulNotifiedValue, WAIT_DMA_DONE_TICKS);
    if ((ulNotifiedValue & wait_bits) == wait_bits)
    {
        FDDMA_INFO("DDMA transfer success.");
    }
    else
    {
        if ((ulNotifiedValue & BIT(tx_chan_id)) == 0U)
        {
            FDDMA_ERROR("TX timeout.");
        }

        if ((ulNotifiedValue & BIT(rx_chan_id)) == 0U)
        {
            FDDMA_ERROR("RX timeout.");
        }

        if (ulNotifiedValue & wait_bits == 0U)
        {
            FDDMA_ERROR("Both TX and RX timeout.");
        }

        ret = FALSE;
    }

    return ret;
    
    
}


static int DdmaSpiLoopbackSend(u32 trans_len)
{
    u32 loop;
    FDDMA_INFO("Waiting send data...");
    static FFreeRTOSSpiMessage spi_msg;
    memset(tx_buf, 0, trans_len);
    memset(rx_buf, 0, trans_len);

    for (loop = 0; loop < trans_len; loop += 4)
    {
        tx_buf[loop] = loop;
    }


    printf("Before loopback ..... \r\n");
    printf("TX buf ===> \r\n");
    FtDumpHexByte((u8 *)tx_buf, trans_len);
    printf("RX buf <=== \r\n");
    FtDumpHexByte((u8 *)rx_buf, trans_len);

    spi_msg.rx_buf = rx_buf;
    spi_msg.rx_len = trans_len;
    spi_msg.tx_buf = tx_buf;
    spi_msg.tx_len = trans_len;

    if ((FFREERTOS_DDMA_OK != FFreeRTOSDdmaStartChannel(ddma, rx_chan_id)) ||
        (FFREERTOS_DDMA_OK != FFreeRTOSDdmaStartChannel(ddma, tx_chan_id)))
    {
        FDDMA_ERROR("Start DDMA channel failed.");
        return FFREERTOS_DDMA_TRANSFER_FAIL;
    }

    /* setup spi transfer only for the first time */
    if (FFREERTOS_DDMA_OK != FFreeRTOSSpimTransfer(spim, &spi_msg))
    {
        FDDMA_ERROR("Start SPI transfer failed.");
        return FFREERTOS_DDMA_TRANSFER_FAIL;
    }
    return FFREERTOS_DDMA_OK;
}

static int DdmaSpiLoopbackCheckRecvData(u32 trans_len)
{
    FDDMA_INFO("Waiting for recv data...");

    if ((FFREERTOS_DDMA_OK != FFreeRTOSDdmaStopChannel(ddma, tx_chan_id)) ||
        (FFREERTOS_DDMA_OK != FFreeRTOSDdmaStopChannel(ddma, rx_chan_id)))
    {
        FDDMA_ERROR("Stop DDMA transfer failed.");
        return FFREERTOS_DDMA_TRANSFER_FAIL;
    }
    printf("After loopback ..... \r\n");
    printf("TX buf ===> \r\n");
    FtDumpHexByte(tx_buf, trans_len);
    printf("RX buf <=== \r\n");
    FtDumpHexByte(rx_buf, trans_len);

    /* compare if loopback success */
    if (0 == memcmp(rx_buf, tx_buf, trans_len))
    {
        printf("Loopback transfer success.\r\n");
        return FFREERTOS_DDMA_OK;
    }
    else
    {
        FDDMA_ERROR("RX data != TX data, loopback transfer failed.");
        return FFREERTOS_DDMA_TRANSFER_FAIL;
    }
}

static void DdmaTask(void *pvParameters)
{   
    ddma_trans_task = xTaskGetCurrentTaskHandle();

    int i=0;
    int ret = FFREERTOS_DDMA_OK;
    uint32_t notify_result;
    uint32_t* trans_len_addr = (uint32_t*)pvParameters;

    u32 trans_len = *trans_len_addr;
    FASSERT_MSG((trans_len <= TX_RX_BUF_LEN) && (trans_len % 4 == 0), "Trans_len is wrong.");

    /*set test mode */
    FFreeRTOSSpimConifg spim_config;
    spim_config.spi_mode = FFREERTOS_SPIM_MODE_0;
    spim_config.en_dma = TRUE;
    spim_config.inner_loopback = TRUE;

    /* test loop */
    for(i = 0; i < LOOPBACK_TIMES; i++)
    {
        SpimDdmaInit(spim_config, trans_len);
        ret = DdmaSpiLoopbackSend(trans_len);
        if(ret != FFREERTOS_DDMA_OK)
        {
            goto task_ret;
        }
        
        vTaskDelay(100); /* insure RX is done */
        if(!DdmaSpiLoopbackWaitDmaEnd())
        {
            ret = FFREERTOS_DDMA_TRANSFER_FAIL;
            goto task_ret;
        }

        /* check recv data is right*/
        ret = DdmaSpiLoopbackCheckRecvData(trans_len);
        SpimDdmaDeinit();
        if(ret != FFREERTOS_DDMA_OK)
        {
            goto task_ret;
        } 
    }

task_ret:
    SpimDdmaDeinit();
    xQueueSend(xQueue, &ret, 0);
    vTaskDelete(NULL);
}

/* Parameter bytes indicates bytes of transfered data, and can be set by cmd.
 * bytes default 32. bytes must be a multiple of 4, and <= 128. 
 * spi_id: valid spi controller id for board.
 */
BaseType_t FFreeRTOSRunDDMASpiLoopback(u32 spi_id, u32 bytes)
{
    BaseType_t ret = pdPASS;
    int task_ret = 0;
    spi_instance_id = spi_id;
    xQueue = xQueueCreate(1,sizeof(int));

    ret = xTaskCreate((TaskFunction_t)DdmaTask,  /* task entry */
                    (const char *)"DdmaTask",/* task name */
                    4096,  /* task stack size in words */
                    (void *)&bytes, /* task params */
                    (UBaseType_t)configMAX_PRIORITIES - 1,  /* task priority */
                    NULL); /* task handler */
    
    ret = xQueueReceive(xQueue, &task_ret, TIMER_OUT);
    FASSERT_MSG(pdPASS == ret, "xQueue Receive failed.\r\n");

    vQueueDelete(xQueue);
    if(task_ret != FFREERTOS_DDMA_OK)
    {
        printf("%s@%d: Ddma spi loopback example [failure].\r\n", __func__, __LINE__);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Ddma spi loopback example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}