/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 *
 *
 * FilePath: i2s_example.h
 * Date: 2024-02-29 13:22:40
 * LastEditTime: 2024-02-29 15:40:40
 * Description:  This file is for task create function
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   Wangzq       2024/02/29  first commit
 */

#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "fcpu_info.h"
#include "fio_mux.h"
#include "fassert.h"
#include"fparameters.h"
#include"fparameters_comm.h"
#include "fmemory_pool.h"
#include "ferror_code.h"
#include "fcache.h"

#include "fi2s_os.h"
#include "fddma_os.h"
#include "fddma.h"
#include "fddma_hw.h"
#include "fddma_bdl.h"
#include "fes8336.h"
#include "fi2s.h"
#include "fi2s_hw.h"
/************************** Constant Definitions *****************************/
/* write and read task handle */
static xTaskHandle i2s_init_handle;
static xTaskHandle i2s_deinit_handle;
static xTaskHandle i2s_Trans_handle;
static xTaskHandle i2s_receive_handle;
static TimerHandle_t exit_timer = NULL;
static QueueHandle_t sync = NULL;

static FFreeRTOSI2s *os_i2s = NULL;
static FFreeRTOSDdma *ddma = NULL;
static FFreeRTOSDdmaConfig ddma_config;
static FDdmaChanIndex rx_chan_id = FDDMA_CHAN_1;
static FDdmaChanIndex tx_chan_id = FDDMA_CHAN_0;
static EventGroupHandle_t chan_evt = NULL;
static FFreeRTOSRequest rx_tx_request = {0};
static FDdmaBdlDesc *bdl_desc_list_g;

static FMemp memp;
static u8 memp_buf[SZ_4M] = {0};
static u32 trans_num  = 0; /*已经传输完成的BDL数量*/
#define CHAN_REQ_DONE(chan)    (0x1 << chan) /* if signal, chan req finished */

/************************** user config *****************************/
static FDdmaBdlDescConfig bdl_desc_config[4096];/*bdl_desc_config []根据TX_RX_BUF_LEN设置*/
static u32 per_buffer = 16384;
#define TX_RX_BUF_LEN         4096* 16384
static u32 rx_buf = 0xa0000000;
static u8 tx_buf[TX_RX_BUF_LEN] __attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT))) = {0};
static FI2sData i2s_config =
{
    .word_length = FI2S_WORLD_LENGTH_16,
    .data_length = 16,
    .sample_rate = FI2S_SAMPLE_RATE_CD,
};

/***************** Macros (Inline Functions) Definitions *********************/
#define FI2S_DEBUG_TAG "FI2S-OS"
#define FI2S_ERROR(format, ...) FT_DEBUG_PRINT_E(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_WARN(format, ...)  FT_DEBUG_PRINT_W(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_INFO(format, ...)  FT_DEBUG_PRINT_I(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_DEBUG(format, ...) FT_DEBUG_PRINT_D(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
static void FFreeRTOSI2sDeinitTask(void *args)
{
    FError err = FT_SUCCESS;

    if (i2s_init_handle) /* stop and delete init task */
    {
        vTaskDelete(i2s_init_handle);
        i2s_init_handle = NULL;
    }
    if (i2s_Trans_handle) /* stop and delete send task */
    {
        vTaskDelete(i2s_Trans_handle);
        i2s_Trans_handle = NULL;
    }
    if (i2s_receive_handle) /* stop and delete recv task */
    {
        vTaskDelete(i2s_receive_handle);
        i2s_receive_handle = NULL;
    }
    if (ddma)
    {
        if (FT_SUCCESS != FFreeRTOSDdmaRevokeChannel(ddma, rx_chan_id))
        {
            FI2S_ERROR("Delete RX channel failed.");
        }
        if (FT_SUCCESS != FFreeRTOSDdmaRevokeChannel(ddma, tx_chan_id))
        {
            FI2S_ERROR("Delete TX channel failed.");
        }
        err = FFreeRTOSDdmaDeinit(ddma);
        if (FT_SUCCESS != err)
        {
            FI2S_ERROR("deinit ddma failed.");
        }
        ddma = NULL;
    }
    if (os_i2s)
    {
        err = FFreeRTOSI2SDeinit(os_i2s);
        if (FT_SUCCESS != err)
        {
            FI2S_ERROR("deinit i2s failed.");
        }
        os_i2s = NULL;
    }
    if (chan_evt)
    {
        vEventGroupDelete(chan_evt);
        chan_evt = NULL;
    }
    if (sync)
    {
        vQueueDelete(sync);
        sync = NULL;
    }
    FMempFree(&memp, bdl_desc_list_g);
    FI2S_DEBUG("exit all task and deinit.");
}

static void DdmaI2sAckDMADone(FDdmaChanIrq *const chan_irq_info_p, void *arg)
{
    FASSERT(chan_irq_info_p);
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;

    FI2S_INFO("Ack chan-%d %s done for DDMA.\r\n", chan_irq_info_p->channel_id,
              (chan_irq_info_p->channel_id == rx_chan_id) ? "RX" : "TX");
    FASSERT_MSG(chan_evt, "RX event group not exists.");

    x_result = xEventGroupSetBitsFromISR(chan_evt,
                                         CHAN_REQ_DONE(chan_irq_info_p->channel_id),
                                         &xhigher_priority_task_woken);
    if (x_result == pdFALSE)
    {
        FI2S_ERROR("xEventGroupSetBitsFromISR() fail.");
    }
    portYIELD_FROM_ISR(xhigher_priority_task_woken);
    return;
}


static inline boolean FFreeRTOSI2sDdmaGiveSync()
{
    boolean data = TRUE;
    FASSERT_MSG((NULL != sync), "Sync not exists.");
    if (pdFALSE == xQueueSend(sync, &data, portMAX_DELAY))
    {
        FI2S_ERROR("Failed to give locker.");
        return FALSE;
    }
    return TRUE;
}

static inline void FFreeRTOSI2sDdmTakeSync()
{
    boolean data = FALSE;
    FASSERT_MSG((NULL != sync), "Sync not exists.");
    if (pdFALSE == xQueueReceive(sync, &data, portMAX_DELAY))
    {
        FI2S_ERROR("Failed to give locker.");
    }
    return;
}

static boolean FFreeRTOSI2sWaitDmaEnd(void)
{
    boolean ok = TRUE;
    EventBits_t ev;
    u32 wait_bits = CHAN_REQ_DONE(rx_chan_id) | CHAN_REQ_DONE(tx_chan_id);

    ev = xEventGroupWaitBits(chan_evt, wait_bits, pdTRUE, pdFALSE, portMAX_DELAY);
    if ((ev & wait_bits) != 0U)
    {
        FI2S_INFO("DDMA transfer success.");
        trans_num ++ ;
        if (trans_num >= TX_RX_BUF_LEN / per_buffer) /*已经传输完成的数目，传输完一个bdl产生一次中断*/
        {
            return ok ;
        }
    }
    else
    {
        if ((ev & CHAN_REQ_DONE(tx_chan_id)) == 0U)
        {
            FI2S_ERROR("TX timeout.");
        }
        if ((ev & CHAN_REQ_DONE(rx_chan_id)) == 0U)
        {
            FI2S_ERROR("RX timeout.");
        }
        if (ev & wait_bits == 0U)
        {
            FI2S_ERROR("Both TX and RX timeout.");
        }
        ok = FALSE;
    }

    return FALSE;
}

static void FFreeRTOSI2sHardWareConfig(u32 work_mode)
{
    FError err = FT_SUCCESS;

    /*设置es8336芯片*/
    FEs8336RegsProbe(); /* 寄存器默认值 */
    FEs8336Startup();
    err = FEs8336SetFormat(i2s_config.word_length); /* 设置ES8336工作模式 */
    FASSERT_MSG(FT_SUCCESS == err, "set es8336 failed.");

    /*设置i2s模块*/
    err =  FI2sClkOutDiv(&os_i2s->i2s_ctrl); /* 默认16-bits采集 */
    FASSERT_MSG(FT_SUCCESS == err, "set i2s clk failed.");
    FI2sSetHwconfig(&os_i2s->i2s_ctrl);
    FI2sTxRxEnable(&os_i2s->i2s_ctrl, TRUE); /* 模块使能 */

    /*ddma bdl配置*/
    fsize_t bdl_num = TX_RX_BUF_LEN / per_buffer;
    err = FMempInit(&memp, memp_buf, memp_buf + sizeof(memp_buf));
    FCacheDCacheFlushRange((uintptr)rx_buf, TX_RX_BUF_LEN);

    for (u32 chan = FDDMA_CHAN_0; chan < FDDMA_NUM_OF_CHAN; chan++) /* 清除中断 */
    {
        FDdmaClearChanIrq(ddma->ctrl.config.base_addr, chan, ddma->ctrl.config.caps);
        u32 status = FDdmaReadReg(ddma->ctrl.config.base_addr, FDDMA_STA_OFFSET);
    }
    /* set BDL descriptors */
    for (fsize_t loop = 0; loop < bdl_num; loop++)
    {
        bdl_desc_config[loop].current_desc_num = loop;
        bdl_desc_config[loop].src_addr = (uintptr)(rx_buf + per_buffer * loop);
        bdl_desc_config[loop].trans_length = per_buffer;
        bdl_desc_config[loop].ioc = TRUE;
    }
    FDdmaBdlDesc *bdl_desc_list = FMempMallocAlign(&memp, bdl_num * sizeof(FDdmaBdlDesc), FDDMA_BDL_ADDR_ALIGMENT);
    memset(bdl_desc_list, 0, bdl_num * sizeof(FDdmaBdlDesc));
    bdl_desc_list_g = bdl_desc_list;
    /* set BDL descriptor list with descriptor configs */
    for (fsize_t loop = 0; loop <  bdl_num; loop++)
    {
        FDdmaBDLSetDesc(bdl_desc_list, &bdl_desc_config[loop]);
    }
    if (work_mode == AUDIO_PCM_STREAM_CAPTURE)
    {
        rx_tx_request.slave_id = 0U;
        rx_tx_request.mem_addr = (uintptr)rx_buf;
        rx_tx_request.dev_addr = os_i2s->i2s_ctrl.config.base_addr + FI2S_RXDMA;
        rx_tx_request.trans_len = TX_RX_BUF_LEN;
        rx_tx_request.is_rx = TRUE;
        rx_tx_request.req_done_handler = DdmaI2sAckDMADone;
        rx_tx_request.req_done_args = NULL;
        rx_tx_request.first_desc_addr = (uintptr)bdl_desc_list;
        rx_tx_request.valid_desc_num = bdl_num;
        err = FFreeRTOSDdmaSetupBDLChannel(ddma, rx_chan_id, &rx_tx_request);
        FFreeRTOSDdmaBDLStartChannel(ddma, rx_chan_id);
    }
    else
    {
        rx_tx_request.slave_id = 0U;
        rx_tx_request.mem_addr = (uintptr)(void *)rx_buf;
        rx_tx_request.dev_addr = os_i2s->i2s_ctrl.config.base_addr + FI2S_TXDMA;
        rx_tx_request.trans_len = TX_RX_BUF_LEN;
        rx_tx_request.is_rx = FALSE;
        rx_tx_request.req_done_handler = DdmaI2sAckDMADone;
        rx_tx_request.req_done_args = NULL;
        rx_tx_request.first_desc_addr = (uintptr)bdl_desc_list;
        rx_tx_request.valid_desc_num = bdl_num;
        err = FFreeRTOSDdmaSetupBDLChannel(ddma, tx_chan_id, &rx_tx_request);
        FFreeRTOSDdmaBDLStartChannel(ddma, tx_chan_id);
    }
}

static void FFreeRTOSI2sInitTask(void *pvParameters)
{
    FError err = FT_SUCCESS;
    const u32 i2s_id = FI2S0_ID;/* 默认使用I2S-0 */
    const u32 ddma_id = FDDMA2_I2S_ID; /* I2S所绑定的DDMA默认是DDMA-2 */

    /*先初始化es8336*/
    FIOMuxInit();
    FIOPadSetI2sMux();
    err = FEs8336Init(); /* es8336初始化，I2C slave设置 */
    FASSERT_MSG(FT_SUCCESS == err, "Init es8336 failed.");

    /*初始化i2s*/
    os_i2s = FFreeRTOSI2sInit(i2s_id);
    FASSERT_MSG(os_i2s, "Init i2s failed.");

    /*初始化ddma*/
    ddma = FFreeRTOSDdmaInit(ddma_id, &ddma_config); /* init DDMA */
    FASSERT_MSG(ddma, "Init DDMA failed.");
    vTaskDelete(NULL);
}

static void FFreeRTOSI2sReceiveTask(void *pvParameters)
{
    FError err = FT_SUCCESS;
    u32 work_mode = AUDIO_PCM_STREAM_CAPTURE;

    FFreeRTOSI2sHardWareConfig(work_mode);

    for (;;)
    {
        printf("...Waiting for recv data...\r\n");
        /* block recv task until RX done */
        if (!FFreeRTOSI2sWaitDmaEnd())
        {
            continue;
        }
        if ((FFREERTOS_DDMA_OK != FFreeRTOSDdmaStopChannel(ddma, tx_chan_id)) ||
            (FFREERTOS_DDMA_OK != FFreeRTOSDdmaStopChannel(ddma, rx_chan_id)))
        {
            FI2S_ERROR("Stop DDMA transfer failed.");
            continue;
        }
        trans_num = 0;
        FFreeRTOSI2sDdmaGiveSync(); /* recv finish, give send sync and allow sending */
        FFreeRTOSDdmaStop(ddma);
        vTaskDelete(NULL);
    }
}

static void FFreeRTOSI2sSendTask(void *pvParameters)
{
    FFreeRTOSI2sDdmTakeSync();/*receiver task is over , send task take sync from recv and start work*/
    FError err = FT_SUCCESS;
    u32 work_mode = AUDIO_PCM_STREAM_PLAYBACK;

    FFreeRTOSI2sHardWareConfig(work_mode);

    for (;;)
    {
        printf("...Waiting for send data...\r\n");
        /* block send task until send done */
        if (!FFreeRTOSI2sWaitDmaEnd())
        {
            continue;
        }
        if ((FFREERTOS_DDMA_OK != FFreeRTOSDdmaStopChannel(ddma, tx_chan_id)) ||
            (FFREERTOS_DDMA_OK != FFreeRTOSDdmaStopChannel(ddma, rx_chan_id)))
        {
            FI2S_ERROR("Stop DDMA transfer failed.");
            continue;
        }
        trans_num = 0;
        FFreeRTOSDdmaStop(ddma);
        vTaskDelete(NULL);
    }
}

BaseType_t FFreeRTOSI2sInitCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    FASSERT_MSG(NULL == chan_evt, "Event group exists.");
    FASSERT_MSG((chan_evt = xEventGroupCreate()) != NULL, "Create event group failed.");
    FASSERT_MSG(NULL == sync, "Sync exists.");
    FASSERT_MSG((sync = xQueueCreate(1, sizeof(boolean))) != NULL, "Create sync failed.");

    taskENTER_CRITICAL(); /* no schedule when create task */

    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSI2sInitTask,  /* 任务入口函数 */
                          (const char *)"FFreeRTOSI2sInitTask",/* 任务名字 */
                          (uint16_t)4096,  /* 任务栈大小 */
                          (void *)NULL,/* 任务入口函数参数 */
                          (UBaseType_t)configMAX_PRIORITIES - 2,  /* 任务的优先级 */
                          (TaskHandle_t *)&i2s_init_handle); /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "I2sInitTask creation is failed.");

    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSI2sSendTask,  /* task entry */
                          (const char *)"FFreeRTOSI2sSendTask",/* task name */
                          (uint16_t)4096,  /* task stack size in words */
                          NULL, /* task params */
                          (UBaseType_t)configMAX_PRIORITIES - 4,  /* task priority */
                          (TaskHandle_t *)&i2s_Trans_handle); /* task handler */

    FASSERT_MSG(pdPASS == xReturn, "Create task failed.");

    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSI2sReceiveTask,  /* task entry */
                          (const char *)"FFreeRTOSI2sReceiveTask",/* task name */
                          (uint16_t)4096,  /* task stack size in words */
                          NULL, /* task params */
                          (UBaseType_t)configMAX_PRIORITIES - 3,  /* task priority */
                          (TaskHandle_t *)&i2s_receive_handle); /* task handler */

    FASSERT_MSG(pdPASS == xReturn, "Create task failed.");

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    return xReturn;
}

BaseType_t FFreeRTOSI2sDeInitCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */

    taskENTER_CRITICAL(); /* no schedule when create task */

    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSI2sDeinitTask,  /* task entry */
                          (const char *)"FFreeRTOSI2sDeinitTask",/* task name */
                          (uint16_t)4096,  /* task stack size in words */
                          NULL, /* task params */
                          (UBaseType_t)configMAX_PRIORITIES - 3,  /* task priority */
                          (TaskHandle_t *)&i2s_deinit_handle); /* task handler */

    FASSERT_MSG(pdPASS == xReturn, "Create task failed.");

    taskEXIT_CRITICAL(); /* allow schedule since task created */

    return xReturn;
}