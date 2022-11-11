/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * FilePath: fsdio_os.c
 * Date: 2022-07-25 09:14:40
 * LastEditTime: 2022-07-25 09:14:40
 * Description:  This files is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/7/27   init commit
 */

/***************************** Include Files *********************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "finterrupt.h"
#include "fparameters.h"
#include "fdebug.h"
#include "fassert.h"
#include "fsleep.h"
#include "fcpu_info.h"
#include "fiopad.h"

#include "fsdio_os.h"
#include "fsdio_hw.h"
#include "sdmmc_cmd.h"
#include "sdmmc_defs.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct
{
    sdmmc_card_t    card; /* instance of card in sdmmc */
    sdmmc_host_t    host; /* instance of host in sdmmc */
} FSdioOSHost; /* instance to support sdmmc commands */

/************************** Variable Definitions *****************************/
static FFreeRTOSSdio sdio[FSDIO_HOST_INSTANCE_NUM];
static FSdioOSHost sdmmc_host[FSDIO_HOST_INSTANCE_NUM];

/***************** Macros (Inline Functions) Definitions *********************/
#define FSDIO_DEBUG_TAG "FSDIO-OS"
#define FSDIO_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_WARN(format, ...)    FT_DEBUG_PRINT_W(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_INFO(format, ...)    FT_DEBUG_PRINT_I(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIO_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSDIO_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static inline FError FSdioOsTakeSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "locker not exists");
    if (pdFALSE == xSemaphoreTake(locker, portMAX_DELAY))
    {
        FSDIO_ERROR("failed to give locker !!!");
        return FFREERTOS_SDIO_SEMA_ERR;
    }

    return FFREERTOS_SDIO_OK;
}

static inline void FSdioOsGiveSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "locker not exists");
    if (pdFALSE == xSemaphoreGive(locker))
    {
        FSDIO_ERROR("failed to give locker !!!");
    }

    return;
}

static void FSdioOSSetupInterrupt(FSdio *ctrl)
{
    uintptr base_addr = ctrl->config.base_addr;
    u32 cpu_id = 0;

    GetCpuId(&cpu_id);
    FSDIO_INFO("cpu_id is cpu_id %d", cpu_id);
    InterruptSetTargetCpus(ctrl->config.irq_num, cpu_id);
    InterruptSetPriority(ctrl->config.irq_num, FFREERTOS_SDIO_IRQ_PRIORITY);

    /* register intr callback */
    InterruptInstall(ctrl->config.irq_num, 
                     FSdioInterruptHandler, 
                     ctrl, 
                     NULL);

    /* enable sdio irq */
    InterruptUmask(ctrl->config.irq_num);

    FSDIO_INFO("sdio interrupt setup done !!!");
    return;    
}

/* init sdmmc host */
static sdmmc_err_t FSdioOsHostInit(int slot)
{
    FASSERT((slot >= 0) && (slot < FSDIO_HOST_INSTANCE_NUM));
    return SDMMC_OK;
}

/* sdmmc host set bus width */
static sdmmc_err_t FSdioOsHostSetBusWidth(int slot, size_t width)
{
    FASSERT((slot >= 0) && (slot < FSDIO_HOST_INSTANCE_NUM));
    FFreeRTOSSdio *instance = &sdio[slot];
    FSdio *ctrl = &instance->ctrl;
    uintptr base_addr = ctrl->config.base_addr;
    FSDIO_INFO("set bus width as %d", width);
    FSdioSetBusWidth(base_addr, width);
    return SDMMC_OK;
}

/* sdmmc host get bus width */
static size_t FSdioOsHostGetBusWidth(int slot)
{
    FASSERT((slot >= 0) && (slot < FSDIO_HOST_INSTANCE_NUM));
    FFreeRTOSSdio *instance = &sdio[slot];
    FSdio *ctrl = &instance->ctrl;
    uintptr base_addr = ctrl->config.base_addr;
    size_t bus_width = (size_t)FSdioGetBusWidth(base_addr);
    FSDIO_INFO("get bus width as %d", bus_width);
    return bus_width;
}

/* sdmmc host set ddr mode */
static sdmmc_err_t FSdioOsHostSetDDRMode(int slot, bool ddr_enabled)
{
    FASSERT((slot >= 0) && (slot < FSDIO_HOST_INSTANCE_NUM));
    FSDIO_ERROR("sdio_host_set_bus_ddr_mode not supported !!!");
    return SDMMC_OK;
}

/* sdmmc host change card clock freq */
static sdmmc_err_t FSdioOsHostSetCardClock(int slot, u32 freq_khz)
{
    FASSERT((slot >= 0) && (slot < FSDIO_HOST_INSTANCE_NUM));
    FFreeRTOSSdio *instance = &sdio[slot];
    FSdio *ctrl = &instance->ctrl;
    FSDIO_INFO("set clk rate as %dKHz", freq_khz);
    FError err = FSdioSetClkFreq(ctrl, freq_khz * 1000);
    return (FSDIO_SUCCESS == err) ? SDMMC_OK : SDMMC_FAIL;
}

/* convert command info from sdmmc_command_t to FSdioCmdData */
static void FSdioOsConvertCmdInfo(sdmmc_command_t *cmdinfo, FSdioCmdData *cmd_data)
{
    if (MMC_GO_IDLE_STATE == cmdinfo->opcode)
    {
        cmd_data->flag |= FSDIO_CMD_FLAG_NEED_INIT;
    }

    if (SCF_RSP_CRC & cmdinfo->flags)
    {
        cmd_data->flag |= FSDIO_CMD_FLAG_NEED_RESP_CRC;
    }

    if (SCF_RSP_PRESENT & cmdinfo->flags)
    {
        cmd_data->flag |= FSDIO_CMD_FLAG_EXP_RESP;

        if (SCF_RSP_136 & cmdinfo->flags)
        {
            cmd_data->flag |= FSDIO_CMD_FLAG_EXP_LONG_RESP;
        }
    }

    if (cmdinfo->data)
    {
        FASSERT_MSG(cmd_data->data_p, "data buffer shall be assigned");
        cmd_data->flag |= FSDIO_CMD_FLAG_EXP_DATA;

        if (SCF_CMD_READ & cmdinfo->flags)
        {
            cmd_data->flag |= FSDIO_CMD_FLAG_READ_DATA;
        }
        else
        {
            cmd_data->flag |= FSDIO_CMD_FLAG_WRITE_DATA;
        }

        cmd_data->data_p->buf = cmdinfo->data;
        cmd_data->data_p->blksz = cmdinfo->blklen;
        cmd_data->data_p->datalen = cmdinfo->datalen;
        FSDIO_INFO("buf@%p, blksz: %d, datalen: %d", 
                    cmd_data->data_p->buf,
                    cmd_data->data_p->blksz,
                    cmd_data->data_p->datalen);
    }

    cmd_data->cmdidx = cmdinfo->opcode;
    cmd_data->cmdarg = cmdinfo->arg;

    return;
}

/* send signal sdio error from interrupt */
static void FSdioOsHandleError(FSdio *const ctrl, void *args)
{
    FASSERT(ctrl);
    FSDIO_ERROR("sdio error occur ...");

    FFreeRTOSSdio *instance = (FFreeRTOSSdio *)args;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;

    FASSERT(instance->evt);
    x_result = xEventGroupSetBitsFromISR(instance->evt, FFREERTOS_SDIO_ERROR_OCCURRED,
                                         &xhigher_priority_task_woken);
}

/* send signal sdio command done from interrupt */
static void FSdioOsAckCmdDone(FSdio *const ctrl, void *args)
{
    FASSERT(args);
    FFreeRTOSSdio *instance = (FFreeRTOSSdio *)args;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;

    FSDIO_DEBUG("ack cmd and data trans done");
    FASSERT(instance->evt);
    x_result = xEventGroupSetBitsFromISR(instance->evt, FFREERTOS_SDIO_CMD_TRANS_DONE,
                                         &xhigher_priority_task_woken);

    return;
}

/* send signal sdio data over from interrupt */
static void FSdioOsAckDataDone(FSdio *const ctrl, void *args)
{
    FASSERT(args);
    FFreeRTOSSdio *instance = (FFreeRTOSSdio *)args;
    BaseType_t xhigher_priority_task_woken = pdFALSE;
    BaseType_t x_result = pdFALSE;

    FSDIO_DEBUG("ack cmd and data trans done");
    FASSERT(instance->evt);
    x_result = xEventGroupSetBitsFromISR(instance->evt, FFREERTOS_SDIO_DAT_TRANS_DONE,
                                         &xhigher_priority_task_woken);

    return;
}

/* wait sdio command done / data over / error occurred from task */
static FError FSdioOsWaitCmdDataDone(FFreeRTOSSdio *const instance)
{
    const TickType_t wait_delay = pdMS_TO_TICKS(FFREERTOS_SDIO_CMD_TIMEOUT); /* wait for 5000 ms */
    EventBits_t ev;
    FError err = FFREERTOS_SDIO_OK;
    u32 evt_bits = FFREERTOS_SDIO_CMD_TRANS_DONE; /* command done */
    u32 err_bits = FFREERTOS_SDIO_ERROR_OCCURRED; /* error occurred */
    FSdioCmdData *cmd_data = &instance->cmd_data;
    FSdio *ctrl = &instance->ctrl;

    if (cmd_data->data_p)
    {
        evt_bits |= FFREERTOS_SDIO_DAT_TRANS_DONE;  /* need to wait data over */
    }

    /* block task to wait finish signal */
    FASSERT_MSG(instance->evt, "evt not exists");
    ev = xEventGroupWaitBits(instance->evt, evt_bits,
                             pdTRUE, pdTRUE, wait_delay); /* wait for cmd/data done */
    if (ev & evt_bits == evt_bits)
    { 
        FSDIO_DEBUG("trans done");
        err = FSdioGetCmdResponse(ctrl, cmd_data); /* check for response data */
    }
    else if (ev & err_bits)
    {
        /* restart controller in interrupt is not a good idea */
        FSdioDumpRegister(ctrl->config.base_addr);
        FSDIO_WARN("restart controller from error state !!!");
        err = FSdioRestart(ctrl); /* update clock otherwise next command will also fail */       
    }
    else 
    {
        FSDIO_ERROR("trans timeout, ev: 0x%x != 0x%x", ev, evt_bits);
        err = FFREERTOS_SDIO_EVT_ERR;
    }

    return err;
}

/* sdmmc do cmd and data transaction */
static sdmmc_err_t FSdioOsHostDoTransaction(int slot, sdmmc_command_t *cmdinfo)
{
    FASSERT((slot >= 0) && (slot < FSDIO_HOST_INSTANCE_NUM));
    FFreeRTOSSdio *instance = &sdio[slot];
    FSdio *ctrl = &instance->ctrl;
    sdmmc_err_t ret = SDMMC_OK;
    FSdioCmdData *cmd_data = &instance->cmd_data;
    FSdioData *trans_data = &instance->trans_data;
    FError err = FSDIO_SUCCESS;

    /* use static structure attached at instance to forward command & data */
    memset(cmd_data, 0, sizeof(*cmd_data));
    if (cmdinfo->data)
    {
        memset(trans_data, 0, sizeof(*trans_data));
        cmd_data->data_p = trans_data;
    }

    FSdioOsConvertCmdInfo(cmdinfo, cmd_data); /* prepare command data */

    if (instance->config.en_dma) /* DMA mode */
    { 
        err = FSdioDMATransfer(ctrl, cmd_data);
        if (FSDIO_SUCCESS != err)
        {
            ret = SDMMC_FAIL;
            goto err_exit;
        }
    }
    else /* non-DMA, PIO mode */
    {
        err = FSdioPIOTransfer(ctrl, cmd_data);
        if (FSDIO_SUCCESS != err)
        {
            ret = SDMMC_FAIL;
            goto err_exit;
        }
    }

    /* wait command / data over signal */
    err = FSdioOsWaitCmdDataDone(instance);
    if (FSDIO_SUCCESS != err)
    {
        ret = SDMMC_FAIL;
        goto err_exit;
    }

    /* success, copy response out */
    if (SCF_RSP_PRESENT & cmdinfo->flags)
    {
        cmdinfo->response[0] = cmd_data->response[0];

        if (SCF_RSP_136 & cmdinfo->flags)
        {
            cmdinfo->response[1] = cmd_data->response[1];
            cmdinfo->response[2] = cmd_data->response[2];
            cmdinfo->response[3] = cmd_data->response[3];
        }
    }

err_exit:
    return ret;
}

/* sdmmc host deinit */
static sdmmc_err_t FSdioOsHostDeinit(void)
{
    sdmmc_err_t err = SDMMC_OK;
    return err;    
}

static sdmmc_err_t FSdioOsHostEnableIOInt(int slot)
{
    FASSERT((slot >= 0) && (slot < FSDIO_HOST_INSTANCE_NUM));
    FSDIO_ERROR("sdio_host_io_int_enable not supported !!!");
    return SDMMC_OK;
}

static sdmmc_err_t FSdioOsHostWaitIOInt(int slot, TickType_t timeout_ticks)
{
    FASSERT((slot >= 0) && (slot < FSDIO_HOST_INSTANCE_NUM));
    FSDIO_ERROR("sdio_host_io_int_wait not supported !!!");
    return SDMMC_OK;
}

/* setup sdmmc host */
static FError FSdioOsSetupHost(FFreeRTOSSdio *const instance)
{
    FSdio *ctrl = &instance->ctrl;
    int slot = (int)ctrl->config.instance_id;
    FSdioOSHost *const sdmmc = &sdmmc_host[slot];
    sdmmc_host_t *host_p = &sdmmc->host; /* host */
    sdmmc_card_t *card_p = &sdmmc->card; /* card */
    sdmmc_err_t sd_err = SDMMC_OK;
    static boolean sdmmc_inited = FALSE;

    if (TRUE != sdmmc_inited)
    {
        /* init sdmmc, which is shared by all instance, and init only one time */
        sd_err = sdmmc_port_init();
        if (SDMMC_OK != sd_err)
            goto err_exit;

        sdmmc_inited = TRUE;
    }

    instance->host_data = sdmmc;

    /* set sdmmc host parameters */
    host_p->slot = slot;
    host_p->flags |=  SDMMC_HOST_FLAG_4BIT; /* bus width = 4 bit */
    host_p->flags &= ~SDMMC_HOST_FLAG_DDR; /* not support DDR mode */
    host_p->flags &= ~SDMMC_HOST_FLAG_SPI; /* not support SPI mode */
    host_p->max_freq_khz = SDMMC_FREQ_50M; /* max freq 50MHz */
    host_p->io_voltage = 3.3f; /* 3.3v */
    host_p->command_timeout_ms = FFREERTOS_SDIO_CMD_TIMEOUT; /* 5000ms timeout */

    FSDIO_INFO("slot: %d, flags: 0x%x, freq: %dkHz", 
                host_p->slot, host_p->flags, host_p->max_freq_khz);

    /* register host ops */
    host_p->init = FSdioOsHostInit;
    host_p->set_bus_width = FSdioOsHostSetBusWidth;
    host_p->get_bus_width = FSdioOsHostGetBusWidth;
    host_p->set_bus_ddr_mode = FSdioOsHostSetDDRMode;
    host_p->set_card_clk = FSdioOsHostSetCardClock;
    host_p->do_transaction = FSdioOsHostDoTransaction;
    host_p->deinit = FSdioOsHostDeinit;
    host_p->io_int_enable = FSdioOsHostEnableIOInt;
    host_p->io_int_wait = FSdioOsHostWaitIOInt;

    /* set clock as 400kHz to probe card */
    sd_err = FSdioOsHostSetCardClock(slot, SDMMC_FREQ_PROBING);
    if (SDMMC_OK != sd_err)
    {
        FSDIO_ERROR("sdio ctrl setup 400kHz clock failed");
        goto err_exit;   
    }

    /* probe eMMC or tf card */
    if (FFREERTOS_SDIO_MEDIUM_EMMC == instance->config.medium_type)
    {
        sd_err = sdmmc_init_emmc(host_p, card_p);
    }
    else if (FFREERTOS_SDIO_MEDIUM_TF == instance->config.medium_type)
    {
        sd_err = sdmmc_init_tf_card(host_p, card_p);
    }

    if (SDMMC_OK == sd_err)
    {
        sdmmc_card_print_info(stdout, card_p); /* print card info is success */
    }
    else
    {
        FSDIO_ERROR("card init failed: 0x%x", sd_err);
    }

err_exit:
    return (SDMMC_OK != sd_err) ? FFREERTOS_SDIO_HOST_ERR : FFREERTOS_SDIO_OK;
}

/**
 * @name: FFreeRTOSSdioInit
 * @msg: init and get sdio instance
 * @return {FFreeRTOSSdio *} NULL if failed
 * @param {u32} id, instance id of sdio
 * @param {FFreeRTOSSdioConifg} *input_config, input config of sdio
 */
FFreeRTOSSdio *FFreeRTOSSdioInit(u32 id, const FFreeRTOSSdioConifg *input_config)
{
    FASSERT(input_config);
    FASSERT_MSG(id < FSDIO_HOST_INSTANCE_NUM, "invalid sdio id");
    FFreeRTOSSdio *instance = &sdio[id];
    FSdio *ctrl = &instance->ctrl;
    FSdioConfig sdio_config;
    FError err = FFREERTOS_SDIO_OK;
    sdmmc_err_t sd_err = SDMMC_OK;

    if (FT_COMPONENT_IS_READY == ctrl->is_ready)
    {
        FSDIO_ERROR("sdio-%d already init", id);
        return instance;
    }

    taskENTER_CRITICAL(); /* no scheduler during init */

    instance->config = *input_config;
    sdio_config = *FSdioLookupConfig(id);
    sdio_config.trans_mode = (instance->config.en_dma) ? FSDIO_IDMA_TRANS_MODE : FSDIO_PIO_TRANS_MODE;
    if (FFREERTOS_SDIO_MEDIUM_EMMC == instance->config.medium_type)
        sdio_config.non_removable = TRUE;
    else
        sdio_config.non_removable = FALSE;

    err = FSdioCfgInitialize(ctrl, &sdio_config);
    if (FSDIO_SUCCESS != err)
    {
        FSDIO_ERROR("init sdio-%d failed, err: 0x%x !!!", id, err);
        taskEXIT_CRITICAL(); /* allow schedule after init */
        return NULL;     
    }

    if (instance->config.en_dma)
    {
        /* setup dma list */
        err = FSdioSetIDMAList(ctrl, instance->rw_desc, FFREERTOS_SDIO_MAX_TRANS_BLOCK);
        if (FSDIO_SUCCESS != err)
        {
            FSDIO_ERROR("set sdio-%d dma list, err: 0x%x !!!", id, err);
            taskEXIT_CRITICAL(); /* allow schedule after init */
            return NULL;
        }
    }

    FSdioOSSetupInterrupt(ctrl);

    if (instance->config.card_detect_handler)
    {
        FSdioRegisterEvtHandler(ctrl, FSDIO_EVT_CARD_DETECTED, 
                                instance->config.card_detect_handler, 
                                instance->config.card_detect_args);
    }

    /* restart controller if error */
    FSdioRegisterEvtHandler(ctrl, FSDIO_EVT_ERR_OCCURE, FSdioOsHandleError, instance);        

    /* transfer may end up with cmd over or cmd and data over */
    FSdioRegisterEvtHandler(ctrl, FSDIO_EVT_CMD_DONE, FSdioOsAckCmdDone, instance);
    FSdioRegisterEvtHandler(ctrl, FSDIO_EVT_DATA_DONE, FSdioOsAckDataDone, instance);       

    FASSERT_MSG(NULL == instance->locker, "locker exists !!!");
    FASSERT_MSG((instance->locker = xSemaphoreCreateMutex()) != NULL, "create mutex failed !!!");

    FASSERT_MSG(NULL == instance->evt, "event group exists !!!");
    FASSERT_MSG((instance->evt = xEventGroupCreate()) != NULL, "create event group failed !!!");

    taskEXIT_CRITICAL(); /* allow schedule after init */

    /* take locker to protect instead disable scheduler, 
        since card probe is time-consuming */
    err = FSdioOsTakeSema(instance->locker);
    if (FFREERTOS_SDIO_OK != err)
        return NULL;

    /* including card probe process, which could be time consuming */
    err = FSdioOsSetupHost(instance);

err_exit:
    FSdioOsGiveSema(instance->locker);
    return (FFREERTOS_SDIO_OK == err) ? instance : NULL; /* exit with NULL if failed */
}

/**
 * @name: FFreeRTOSSdioDeInit
 * @msg: deinit sdio instance
 * @return {FError} FFREERTOS_SDIO_OK if success
 * @param {FFreeRTOSSdio} *instance, freertos sdio instance
 */
FError FFreeRTOSSdioDeInit(FFreeRTOSSdio *const instance)
{
    FASSERT(instance);
    FSdio *ctrl = &instance->ctrl;
    FError err = FFREERTOS_SDIO_OK;

    /* no scheduler during deinit */
    taskENTER_CRITICAL(); 

    /* disable sdio irq */
    InterruptMask(ctrl->config.irq_num);

    FSdioDeInitialize(ctrl);

    instance->host_data = NULL;

    FASSERT_MSG(NULL != instance->locker, "locker not exists !!!");
    vSemaphoreDelete(instance->locker);
    instance->locker = NULL;

    FASSERT_MSG(NULL != instance->evt, "event group not exists !!!");
    vEventGroupDelete(instance->evt);
    instance->evt = NULL;

    taskEXIT_CRITICAL(); /* allow schedule after deinit */

    return err;
}

/**
 * @name: FFreeRTOSSdioTransfer
 * @msg: start sdio transfer and wait transfer done in this function
 * @return {FError} FFREERTOS_SDIO_OK if transfer success
 * @param {FFreeRTOSSdio} *instance, freertos sdio instance
 * @param {FFreeRTOSSdioMessage} *message, sdio transfer message
 */
FError FFreeRTOSSdioTransfer(FFreeRTOSSdio *const instance, const FFreeRTOSSdioMessage *message)
{
    FASSERT(instance);
    FSdio *ctrl = &instance->ctrl;
    sdmmc_err_t sd_err = SDMMC_OK;
    FError err = FFREERTOS_SDIO_OK;
    FSdioOSHost *sdmmc = (FSdioOSHost *)instance->host_data;

    err = FSdioOsTakeSema(instance->locker);
    if (FFREERTOS_SDIO_OK != err)
        return err;

    FASSERT_MSG(message->buf_len >= message->block_num * FSDIO_DEFAULT_BLOCK_SZ, "buffer not enough !!!");
    if (FFREERTOS_SDIO_TRANS_READ == message->trans_type)
    {
        sd_err = sdmmc_read_sectors(&sdmmc->card, (void *)message->buf,
                                    message->start_block, message->block_num);
        if (SDMMC_OK != sd_err)
        {
            err = FFREERTOS_SDIO_READ_ERR;
        }
    }
    else
    {
        sd_err = sdmmc_write_sectors(&sdmmc->card, (const void *)message->buf, 
                                     message->start_block, message->block_num);
        if (SDMMC_OK != sd_err)
        {
            err = FFREERTOS_SDIO_WRITE_ERR;
        }
    }

err_exit:
    FSdioOsGiveSema(instance->locker);
    return err;
}