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
 * FilePath: fgdma_os.c
 * Date: 2022-07-20 10:54:31
 * LastEditTime: 2022-07-20 10:54:31
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
#include "fsleep.h"
#include "fcpu_info.h"

#include "fgdma.h"
#include "fgdma_hw.h"

#include "fgdma_os.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/
static FFreeRTOSGdma gdma[FGDMA_INSTANCE_NUM];

/***************** Macros (Inline Functions) Definitions *********************/
#define FGDMA_DEBUG_TAG "GDMA-OS"
#define FGDMA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_WARN(format, ...)    FT_DEBUG_PRINT_W(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_INFO(format, ...)    FT_DEBUG_PRINT_I(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGDMA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FGDMA_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/
static inline FError FGdmaOsTakeSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "locker not exists");
    if (pdFALSE == xSemaphoreTake(locker, portMAX_DELAY))
    {
        FGDMA_ERROR("failed to take locker !!!");
        return FFREERTOS_GDMA_SEMA_ERR;
    }

    return FFREERTOS_GDMA_OK;
}

static inline void FGdmaOsGiveSema(SemaphoreHandle_t locker)
{
    FASSERT_MSG((NULL != locker), "locker not exists");
    if (pdFALSE == xSemaphoreGive(locker))
    {
        FGDMA_ERROR("failed to give locker !!!");
    }

    return;
}

static void FGdmaOsSetupInterrupt(FGdma *const ctrl)
{
	FASSERT(ctrl);
    FGdmaConfig *config = &ctrl->config;
    uintptr base_addr = config->base_addr;
    u32 cpu_id = 0;

    GetCpuId(&cpu_id);
    FGDMA_INFO("cpu_id is cpu_id %d", cpu_id);
    FGDMA_INFO("interrupt_id is %d", config->irq_num);
    InterruptSetTargetCpus(config->irq_num, cpu_id);

    InterruptSetPriority(config->irq_num, config->irq_prority);

    /* register intr callback */
    InterruptInstall(config->irq_num, 
                     FGdmaIrqHandler, 
                     ctrl, 
                     NULL);

    /* enable gdma irq */
    InterruptUmask(config->irq_num);

    FGDMA_INFO("gdma interrupt setup done !!!");
    return;    
}

FFreeRTOSGdma *FFreeRTOSGdmaInit(u32 id)
{
    FASSERT_MSG(id < FGDMA_INSTANCE_NUM, "invalid gdma id");
    FFreeRTOSGdma *instance = &gdma[id];
    FGdma *const ctrl = &instance->ctrl;
    FGdmaConfig config;
    FMemp *memp = &instance->memp;
    void *memp_buf_beg = (void *)instance->memp_buf;
    void *memp_buf_end = (void *)instance->memp_buf + sizeof(instance->memp_buf);
    FError err = FFREERTOS_GDMA_OK;

    if (FT_COMPONENT_IS_READY == ctrl->is_ready)
    {
        FGDMA_WARN("gdma ctrl %d already inited !!!", id);
        return instance;
    }

    /* no scheduler during init */
    taskENTER_CRITICAL(); 

    config = *FGdmaLookupConfig(id);
    config.irq_prority = FFREERTOS_GDMA_IRQ_PRIORITY;
    err = FGdmaCfgInitialize(ctrl, &config);
    if (FGDMA_SUCCESS != err)
	{
		FGDMA_ERROR("init gdma-%d failed, 0x%x", id, err);
        goto err_exit;
    }

    FGdmaOsSetupInterrupt(ctrl);
    
    err = FMempInit(memp, memp_buf_beg, memp_buf_end);
	if (FMEMP_SUCCESS != err)
	{
		FGDMA_ERROR("init memp failed, 0x%x", err);
        goto err_exit;
    }    

    FASSERT_MSG(NULL == instance->locker, "locker exists !!!");
    FASSERT_MSG((instance->locker = xSemaphoreCreateMutex()) != NULL, "create mutex failed !!!");

    /* start gdma first, then config gdma channel */
	err = FGdmaStart(ctrl);

err_exit:
    taskEXIT_CRITICAL(); /* allow schedule after init */
    return (FT_SUCCESS == err) ? instance : NULL; /* exit with NULL if failed */ 
}

FError FFreeRTOSGdmaDeInit(FFreeRTOSGdma *const instance)
{
    FASSERT(instance);
    FGdma *const ctrl = &instance->ctrl;
    FGdmaConfig *config = &ctrl->config;
    FMemp *memp = &instance->memp;
    FError err = FFREERTOS_GDMA_OK;

    if (FT_COMPONENT_IS_READY != ctrl->is_ready)
    {
        FGDMA_ERROR("gdma ctrl %d not yet init !!!", ctrl->config.instance_id);
        return FFREERTOS_GDMA_NOT_INIT;
    }

    taskENTER_CRITICAL();  /* no schedule when deinit instance */

    err = FGdmaStop(ctrl);
    FMempDeinit(memp);
	FGdmaDeInitialize(ctrl);    

    vSemaphoreDelete(instance->locker);    
    instance->locker = NULL;

    taskEXIT_CRITICAL(); /* allow schedule after deinit */
    return err; 
}

FError FFreeRTOSGdmaSetupChannel(FFreeRTOSGdma *const instance, u32 chan_id, const FFreeRTOSGdmaRequest *req)
{
    FASSERT(instance);
    FGdma *const ctrl = &instance->ctrl;
    FMemp *memp = &instance->memp;
    FError err = FFREERTOS_GDMA_OK;
    FFreeRTOSGdmaChan *chan_os = &instance->chan[chan_id];
    FFreeRTOSGdmaTranscation *trans = NULL;
    FGdmaChanConfig *chan_config = NULL;
    u32 buf_idx;

    err = FGdmaOsTakeSema(instance->locker);
    if (FFREERTOS_GDMA_OK != err)
        return err;

    chan_os->bdl_list = FMempMallocAlign(memp, sizeof(FGdmaBdlDesc) * req->valid_trans_num, FGDMA_ADDR_ALIGMENT);
    if (NULL == chan_os->bdl_list)
    {
        FGDMA_ERROR("allocate buffer failed !!!");
        err = FFREERTOS_GDMA_ALLOCATE_FAIL;
        goto err_exit;            
    }

    chan_config = &chan_os->chan.config;
    *chan_config = FGDMA_DEFAULT_BDL_CHAN_CONFIG(chan_id, chan_os->bdl_list, req->valid_trans_num);
    err = FGdmaAllocateChan(ctrl, &chan_os->chan, chan_config);
    if (FGDMA_SUCCESS != err)
    {
        FGDMA_ERROR("allocate chan failed !!!");
        goto err_exit;
    }

    FGdmaChanRegisterEvtHandler(&chan_os->chan,
                                FGDMA_CHAN_EVT_TRANS_END,
                                req->req_done_handler,
                                req->req_done_args);

    /* assign bdl for each transaction buffer */
    for (buf_idx = 0; buf_idx < req->valid_trans_num; buf_idx++)
    {
        trans = &req->trans[buf_idx];

        /* append bdl entry */
        err = FGdmaAppendBDLEntry(&chan_os->chan, (uintptr)trans->src_buf, 
                                 (uintptr)trans->dst_buf, trans->data_len);
        if (FGDMA_SUCCESS != err)
        {
            FGDMA_ERROR("setup bdl entry failed !!!");
            goto err_exit;
        }    
    }

err_exit:
    FGdmaOsGiveSema(instance->locker);
    return err;  
}

FError FFreeRTOSGdmaRevokeChannel(FFreeRTOSGdma *const instance, u32 chan_id)
{
    FASSERT(instance);
    FGdma *const ctrl = &instance->ctrl;
    FError err = FFREERTOS_GDMA_OK;
    FMemp *memp = &instance->memp;
    u32 chan_idx;
    FFreeRTOSGdmaChan *chan_os = &instance->chan[chan_id];

    err = FGdmaOsTakeSema(instance->locker);
    if (FFREERTOS_GDMA_OK != err)
        return err;

    /* free dynamic memroy allocated for bdl */
    if (chan_os->bdl_list)
    {
        FMempFree(memp, chan_os->bdl_list);
        chan_os->bdl_list = NULL;
    }    

    /* deallocate channel */
    err = FGdmaDellocateChan(&chan_os->chan);
    if (FGDMA_SUCCESS != err)
    {
        FGDMA_ERROR("dellocate chan %d failed", chan_id);
    }

err_exit:
    FGdmaOsGiveSema(instance->locker);
    return err;      
}

FError FFreeRTOSGdmaStart(FFreeRTOSGdma *const instance, u32 chan_id)
{
    FASSERT(instance);
    FGdma *const ctrl = &instance->ctrl;
    FError err = FFREERTOS_GDMA_OK;
    FFreeRTOSGdmaChan *chan_os = &instance->chan[chan_id];

    err = FGdmaOsTakeSema(instance->locker);
    if (FFREERTOS_GDMA_OK != err)
        return err;
	   
    err = FGdmaBDLTransfer(&chan_os->chan); /* start transfer of each channel */
    if (FGDMA_SUCCESS != err)
    {
        goto err_exit;
    }    

    /* you may wait memcpy end in other task */    

err_exit:
    FGdmaOsGiveSema(instance->locker);
    return err;
}