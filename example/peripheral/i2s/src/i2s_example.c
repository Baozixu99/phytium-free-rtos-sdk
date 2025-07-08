/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: i2s_example.h
 * Date: 2024-02-29 13:22:40
 * LastEditTime: 2024-02-29 15:40:40
 * Description:  This file is for task create function
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   Wangzq      2024/02/29  first commit
 * 2.0   Wangzq      2024/4/22   add no letter shell mode, adapt to auto-test system
 */

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "fdebug.h"
#include "fio_mux.h"
#include "fmemory_pool.h"
#include "ferror_code.h"
#include "fcache.h"

#include "fdevice.h"
#include "fi2s_os.h"
#include "fddma_os.h"
#include "fddma.h"
#include "fddma_bdl.h"
#include "fi2s.h"
#include "fi2s_hw.h"
/************************** Constant Definitions *****************************/
#include "fes8336.h"
static FEs8336Controller fes8336 =
{
    .fes8336_device.name = "es8336",
    .dev_type = DEV_TYPE_MIO,
    .controller_id = FMIO14_ID,
};
static const u32 ddma_ctrl_id = FDDMA2_I2S_ID;
static const u32 i2s_ctrl_id = FI2S0_ID;

static FFreeRTOSI2s *os_i2s = NULL;
static FFreeRTOSDdma *ddma = NULL;
static FFreeRTOSDdmaConfig ddma_config;
static FDdmaChanIndex rx_chan_id = FDDMA_CHAN_1;
static FDdmaChanIndex tx_chan_id = FDDMA_CHAN_0;
static EventGroupHandle_t chan_evt = NULL;
static FFreeRTOSRequest rx_request = {0};
static FFreeRTOSRequest tx_request = {0};

static FMemp memp;
static u8 memp_buf[SZ_4M] = {0};

static boolean is_running = FALSE;
static QueueHandle_t xQueue = NULL;
#define CHAN_REQ_DONE(chan)    (0x1 << chan) /* if signal, chan req finished */
#define I2S_TEST_TASK_PRIORITY  3
/**************************** Type Definitions *******************************/
enum
{
    I2S_TRANS_TEST_SUCCESS = 0,
    I2S_TRANS_TEST_UNKNOWN = 1,
    I2S_INIT_ERROR   = 2,
    I2S_RECEIVE_ERROR = 3,
    I2S_PLAYBACK_ERROR = 4,
};

/************************** user config *****************************/
static FDdmaBdlDescConfig bdl_desc_config[100];/*bdl_desc_config []根据TX_RX_BUF_LEN设置，随buffer数更改*/
static FDdmaBdlDesc *bdl_desc_list_g;
#define  per_buffer  16384/*录制时每个buffer的大小，单位字节*/
#define TX_RX_BUF_LEN         100* 16384/*录制时总buffer的大小，一共10个buff,单位字节*/

static u32 data_buf[TX_RX_BUF_LEN] __attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT))) = {0};
/***************** Macros (Inline Functions) Definitions *********************/
#define FI2S_DEBUG_TAG "FI2S-TRANS"
#define FI2S_ERROR(format, ...) FT_DEBUG_PRINT_E(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_WARN(format, ...)  FT_DEBUG_PRINT_W(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_INFO(format, ...)  FT_DEBUG_PRINT_I(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2S_DEBUG(format, ...) FT_DEBUG_PRINT_D(FI2S_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
static void FFreeRTOSI2sDeinit(void)
{
    printf("Exiting...\r\n");
    /* deinit iomux */
    FIOMuxDeInit();
    FError ret = TRUE;
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
        ret = FFreeRTOSDdmaDeinit(ddma);
        if (FT_SUCCESS != ret)
        {
            FI2S_ERROR("deinit ddma failed.");
        }
        ddma = NULL;
    }
    if (os_i2s)
    {
        ret = FFreeRTOSI2SDeinit(os_i2s);
        if (FT_SUCCESS != ret)
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
    is_running = FALSE;
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

static FError I2sWaitTransferDone(void)
{
    boolean ret = TRUE;
    EventBits_t ev;
    static u32 wait_timeout = pdMS_TO_TICKS(100000UL);/*等待超时时间，单位ms，请确保等待超时时间大于录制时间*/
    u32 wait_bits = CHAN_REQ_DONE(rx_chan_id) | CHAN_REQ_DONE(tx_chan_id);
    ev = xEventGroupWaitBits(chan_evt, wait_bits, pdTRUE, pdFALSE, wait_timeout);
    if ((ev & wait_bits) != 0U)
    {
        ret = FT_SUCCESS;
        FI2S_INFO("transfer success *********\r\n");
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
    }
    return ret;
}

/*I2s playback function*/
static FError I2sPlayback(void)
{
    FError ret = FT_SUCCESS;

    ret = FDeviceOpen(&fes8336.fes8336_device, FDEVICE_FLAG_RDWR);
    if (FT_SUCCESS != ret)
    {
        printf("ES8336 dev open failed.\r\n");
        return ret;
    }

    ret = FFreeRTOSDdmaBDLStartChannel(ddma, tx_chan_id);
    if (ret != FT_SUCCESS)
    {
        FI2S_ERROR("Ddma set config failed.");
        goto exit;
    }
exit:
    return ret;
}

/*I2s receive function*/
static FError I2sReceive(void)
{
    FError ret = FT_SUCCESS;
    
    ret = FDeviceOpen(&fes8336.fes8336_device, FDEVICE_FLAG_RDWR);
    if (FT_SUCCESS != ret)
    {
        printf("ES8336 dev open failed.\r\n");
        return ret;
    }

    FFreeRTOSDdmaBDLStartChannel(ddma, rx_chan_id);
    if (ret != FT_SUCCESS)
    {
        FI2S_ERROR("Ddma set config failed.");
        goto exit;
    }
exit:
    return ret;
}

static FError DdmaInit(void)
{
    FError ret = FT_SUCCESS;
    /*初始化ddma*/
    ddma = FFreeRTOSDdmaInit(FDDMA2_I2S_ID, &ddma_config); /* init DDMA */
    if (ddma == NULL)
    {
        FI2S_ERROR("Init i2s failed.");
        return FI2S_ERR_CONFIG_SET_FAILED;
        goto exit;
    }
    /*ddma bdl配置*/

    fsize_t bdl_num = TX_RX_BUF_LEN / per_buffer;
    ret = FMempInit(&memp, memp_buf, memp_buf + sizeof(memp_buf));
    FCacheDCacheFlushRange((uintptr)data_buf, TX_RX_BUF_LEN);

    for (u32 chan = FDDMA_CHAN_0; chan < FDDMA_NUM_OF_CHAN; chan++) /* 清除中断 */
    {
        FDdmaClearChanIrq(ddma->ctrl.config.base_addr, chan, ddma->ctrl.config.caps);
        u32 status = FDdmaReadReg(ddma->ctrl.config.base_addr, FDDMA_STA_OFFSET);
    }
    /* set BDL descriptors */
    for (fsize_t loop = 0; loop < bdl_num; loop++)
    {
        bdl_desc_config[loop].current_desc_num = loop;
        bdl_desc_config[loop].src_addr = (uintptr)(data_buf + per_buffer * loop);
        bdl_desc_config[loop].trans_length = per_buffer;
        bdl_desc_config[loop].ioc = FALSE;
    }
    bdl_desc_config[bdl_num - 1].ioc = TRUE;
    bdl_desc_list_g = FMempMallocAlign(&memp, bdl_num * sizeof(FDdmaBdlDesc), FDDMA_BDL_ADDR_ALIGMENT);
    memset(bdl_desc_list_g, 0, bdl_num * sizeof(FDdmaBdlDesc));
    /* set BDL descriptor list with descriptor configs */
    for (fsize_t loop = 0; loop <  bdl_num; loop++)
    {
        FDdmaBDLSetDesc(bdl_desc_list_g, &bdl_desc_config[loop]);
    }
    rx_request.slave_id = 0U;
    rx_request.mem_addr = (uintptr)data_buf;
    rx_request.dev_addr = os_i2s->i2s_ctrl.config.base_addr + FI2S_RXDMA;
    rx_request.trans_len = TX_RX_BUF_LEN;
    rx_request.is_rx = TRUE;
    rx_request.req_done_handler = DdmaI2sAckDMADone;
    rx_request.req_done_args = NULL;
    rx_request.first_desc_addr = (uintptr)bdl_desc_list_g;
    rx_request.valid_desc_num = bdl_num;
    ret = FFreeRTOSDdmaSetupBDLChannel(ddma, rx_chan_id, &rx_request);

    if (ret != FT_SUCCESS)
    {
        FI2S_ERROR("Ddma set config failed.");
        goto exit;
    }
    tx_request.slave_id = 0U;
    tx_request.mem_addr = (uintptr)data_buf;
    tx_request.dev_addr = os_i2s->i2s_ctrl.config.base_addr + FI2S_TXDMA;
    tx_request.trans_len = TX_RX_BUF_LEN;
    tx_request.is_rx = FALSE;
    tx_request.req_done_handler = DdmaI2sAckDMADone;
    tx_request.req_done_args = NULL;
    tx_request.first_desc_addr = (uintptr)bdl_desc_list_g;
    tx_request.valid_desc_num = bdl_num;
    ret = FFreeRTOSDdmaSetupBDLChannel(ddma, tx_chan_id, &tx_request);

    printf("ddma id = %d, tx_chan_id = %d, rx_chan_id = %d\r\n",FDDMA2_I2S_ID, tx_chan_id, rx_chan_id);
    printf("bdl_num = %d, per_buffer = %d, TX_RX_BUF_LEN = %d\r\n",bdl_num, per_buffer, TX_RX_BUF_LEN);
    printf("the buffer address is %p\r\n", data_buf);
    if (ret != FT_SUCCESS)
    {
        FI2S_ERROR("Ddma set config failed.");
        goto exit;
    }
exit:
    return ret;
}

/*I2s trans init function*/
static FError I2sInit(void)
{
    FError ret = FT_SUCCESS;
    u32 volumel = 0x1;
    u32 word_length = AUDIO_PCM_STREAM_WORD_LENGTH_16; /* 16-bits word length */
    /*先初始化es8336*/
    FIOMuxInit();
    FIOPadSetI2sMux();

    ret = FEs8336DevRegister(&fes8336.fes8336_device); 
    if (FT_SUCCESS != ret)
    {
        printf("ES8336 dev register failed.\r\n");
        return ret;
    }

    ret = FDeviceInit(&fes8336.fes8336_device);
    if (FT_SUCCESS != ret)
    {
        printf("ES8336 dev init failed.\r\n");
        return ret;
    }

    ret = FDeviceOpen(&fes8336.fes8336_device, FDEVICE_FLAG_RDWR);
    if (FT_SUCCESS != ret)
    {
        printf("ES8336 dev open failed.\r\n");
        return ret;
    }

    ret = FDeviceControl(&fes8336.fes8336_device, FES8336_SET_FORMAT, &word_length); /* 设置ES8336工作模式 */
    if (FT_SUCCESS != ret)
    {
        printf("Set the ES8336 word length failed.\r\n");
        return ret;
    }
    
    ret = FDeviceControl(&fes8336.fes8336_device, FES8336_SET_VOLUMEL, &volumel); /* 设置ES8336工作模式 */
    if (FT_SUCCESS != ret)
    {
        printf("Set the ES8336 volumel failed.\r\n");
        return ret;
    }

    os_i2s = FFreeRTOSI2sInit(i2s_ctrl_id);
    if (os_i2s == NULL)
    {
        FI2S_ERROR("Init i2s failed.");
        return FI2S_ERR_CONFIG_SET_FAILED;
        goto exit;
    }
    os_i2s->i2s_ctrl.data_config.sample_rate = FI2S_SAMPLE_RATE_CD;
    os_i2s->i2s_ctrl.data_config.word_length = FI2S_WORLD_LENGTH_16;
    /*设置i2s模块*/
    FFreeRTOSSetupI2S(os_i2s);
exit:
    return ret;
}

static void FFreeRTOSI2sTransTask(void)
{
    int task_res = I2S_TRANS_TEST_SUCCESS;
    FError ret = FT_SUCCESS;

    ret = I2sInit();
    if (ret != FT_SUCCESS)
    {
        FI2S_ERROR("I2sInitFunction failed.");
        task_res = I2S_INIT_ERROR;
        goto task_exit;
    }
    ret = DdmaInit();
    if (ret != FT_SUCCESS)
    {
        FI2S_ERROR("I2sInitFunction failed.");
        task_res = I2S_INIT_ERROR;
        goto task_exit;
    }
    ret = I2sReceive();
    if (ret != FT_SUCCESS)
    {
        FI2S_ERROR("I2sReceive failed.");
        task_res = I2S_RECEIVE_ERROR;
        goto task_exit;
    }
    I2sWaitTransferDone();/*等待接收完成，之后开始发送*/
    if (ret != FT_SUCCESS)
    {
        FI2S_ERROR("I2sWaitRxDone failed.");
        goto task_exit;
    }

    ret = I2sPlayback();/*开始发送*/
    if (ret != FT_SUCCESS)
    {
        FI2S_ERROR("I2sPlayback failed.");
        task_res = I2S_PLAYBACK_ERROR;
        goto task_exit;
    }
    I2sWaitTransferDone();/*等待发送完成*/

task_exit:
    xQueueSend(xQueue, &task_res, 0);
    vTaskDelete(NULL);
}

BaseType_t FFreeRTOSRunI2sExample(void)
{
    BaseType_t xReturn = pdPASS; /*  pdPASS */
    int task_res = I2S_TRANS_TEST_UNKNOWN;
    FASSERT_MSG(NULL == chan_evt, "Event group exists.");
    FASSERT_MSG((chan_evt = xEventGroupCreate()) != NULL, "Create event group failed.");
    xQueue = xQueueCreate(1, sizeof(int)); /* create queue for task communication */
    if (is_running)
    {
        FI2S_ERROR("The task is running.");
        return pdPASS;
    }

    is_running = TRUE;


    if (xQueue == NULL)
    {
        FI2S_ERROR("xQueue create failed.");
        goto exit;
    }
    xReturn = xTaskCreate((TaskFunction_t)FFreeRTOSI2sTransTask,         /* 任务入口函数 */
                          (const char *)"FFreeRTOSI2sTransTask",         /* 任务名字 */
                          4096,                         /* 任务栈大小 */
                          NULL,                                   /* 任务入口函数参数 */
                          (UBaseType_t)I2S_TEST_TASK_PRIORITY, /* 任务优先级 */
                          NULL);                                  /* 任务句柄 */
    if (xReturn == pdFAIL)
    {
        FI2S_ERROR("xTaskCreate I2s trans task failed.");
        goto exit;
    }
    xReturn = xQueueReceive(xQueue, &task_res, portMAX_DELAY);
    if (xReturn == pdFAIL)
    {
        FI2S_ERROR("xQueue receive timeout.");
        goto exit;
    }

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }
    FFreeRTOSI2sDeinit();
    if (task_res != I2S_TRANS_TEST_SUCCESS)
    {
        printf("%s@%d: I2s trans example [failure], task_res = %d\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: I2s trans example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}
