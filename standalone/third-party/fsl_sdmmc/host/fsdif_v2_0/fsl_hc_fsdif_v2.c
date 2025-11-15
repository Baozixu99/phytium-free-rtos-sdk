/*
* Copyright (c) 2025 Phytium Information Technology, Inc.
* All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "fsl_common.h"
#include "fsl_sdmmc_common.h"
#include "fsl_sdmmc.h"
#include "fparameters.h"
#include "fsdif_msg_hw.h"
#include "fsdif_msg.h"

#include "finterrupt.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

static const char* TAG = "SDMMC:FSdifMsgCtrl:v2";

typedef struct _fsdifv2_host_dev_
{
    sdmmchost_t *instance;
    FSdifMsgCtrl hc;
    FSdifMsgConfig hc_cfg;
    FSdifMsgCommand cmd;
    FSdifMsgData data;
    FSdifMsgRequest req;
    FSdifMsgIDmaDesc *rw_desc;
    uint32_t desc_num;
    sdmmc_osa_event_t hc_evt;
    boolean first_inited;
} fsdifv2_host_dev_t;


#define FSDIF_COMMAND_TIMEOUT       5000U /* 5 seconds timeout in sd spec. */
#define FSDIF_TRANS_ERR_EVENTS      (SDMMC_OSA_EVENT_TRANSFER_CMD_FAIL | \
    SDMMC_OSA_EVENT_TRANSFER_DATA_FAIL | \
    SDMMC_OSA_EVENT_CARD_REMOVED)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void FSDIFV2_PrepareInitData(FSdifMsgDataInit *msg_data_init, sdmmchost_t *sdmmc);
status_t FSDIFV2_PrepareInitVolt(FSdifMsgCtrl *const instance, sdmmchost_t *sdmmc);
status_t FSDIFV2_PrepareInitIOS(FSdifMsgCtrl *const instance, sdmmchost_t *sdmmc);
void FSDIFV2_PrepareTimingData(FSdifMsgDataSetIos *msg_set_ios, sdmmchost_t *sdmmc);
void FSDIFV2_PrepareVoltageData(FSdifMsgDataSwitchVolt *msg_volt, uint32_t voltage);
void FSDIFV2_PrepareBusWidthData(FSdifMsgDataSetIos *msg_set_ios, uint32_t dataBusWidth);
void FSDIFV2_PrepareClockData(FSdifMsgDataSetIos *msg_set_ios, uint32_t targetClock);
void FSDIFV2_PrepareCommandTrasnfer(FSdifMsgDataStartCmd *msg_cmd, sdmmchost_transfer_t *content, sdmmchost_t *sdmmc);
void FSDIFV2_PrepareDataTransfer(FSdifMsgDataStartData *msg_data, sdmmchost_transfer_t *content, sdmmchost_t *sdmmc);

FError FSdifMsgHostResetHW(FSdifMsgCtrl *const instance);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static void FSDIFV2_HOST_RevokeIrq(FSdifMsgCtrl *ctrl_p)
{
    /* disable sdio irq */
    InterruptMask(ctrl_p->config.irq_num);
}

static void FSDIFV2_HOST_Deinit(sdmmchost_t *host)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;

    FSDIFV2_HOST_RevokeIrq(&dev->hc);
    FSdifMsgDeInitialize(&dev->hc);
    SDMMC_OSAEventDestroy(&dev->hc_evt);

    SDMMC_OSAMemoryFree(dev->rw_desc);

    memset(&dev->hc, 0, sizeof(dev->hc));
    SDMMC_OSAMemoryFree(dev);

    SDMMC_LOG("Sdif v2 host deinited !!!");
    return;
}

static status_t FSDIFV2_HOST_Reset(sdmmchost_t *host)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;
    if (FSDIF_SUCCESS != FSdifMsgHostResetHW(&dev->hc))
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

static void FSDIFV2_HOST_SwitchToVoltage(sdmmchost_t *host, uint32_t voltage)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;
    FSdifMsgDataSwitchVolt *cur_volt = &(dev->hc.cur_volt);
    FSdifMsgDataSwitchVolt target_volt;
    FError err;

    if (host->currVoltage == voltage)
    {
        return;
    }

    memcpy(&target_volt, cur_volt, sizeof(*cur_volt));
    FSDIFV2_PrepareVoltageData(&target_volt, voltage); 

    SDMMC_LOGD(TAG, "Set card voltage to %d", voltage);
    err = FSdifMsgSetVoltage(&dev->hc, &target_volt);
    if (FSDIF_SUCCESS != err)
    {
        SDMMC_LOGE(TAG, "Switch voltage from %d to %d failed 0x%x !!!", 
            cur_volt->signal_voltage, target_volt.signal_voltage, err);        
        return;
    }

    SDMMC_LOGI(TAG, "Switch voltage %d success", target_volt.signal_voltage);
    host->currVoltage = voltage;
    return;
}

static status_t FSDIFV2_HOST_ExecuteTuning(sdmmchost_t *host, uint32_t tuningCmd, uint32_t *revBuf, uint32_t blockSize)
{
    return kStatus_Success;
}

static void FSDIFV2_HOST_EnableDDRMode(sdmmchost_t *host, bool enable, uint32_t nibblePos)
{
    if (!(host->capability & kSDMMCHOST_SupportDDRMode))
    {
        return;
    }

    return;
}

static void FSDIFV2_HOST_EnableHS400Mode(sdmmchost_t *host, bool enable)
{
    if (!(host->capability & kSDMMCHOST_SupportHS400))
    {
        return;
    }

#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;
    FSdifMsgDataSetIos *cur_ios = &(dev->hc.cur_ios);
    FSdifMsgDataSetIos target_ios;
    FError err;

    if (!enable)
    {
        assert(0);
    }

    memcpy(&target_ios, cur_ios, sizeof(*cur_ios));
    FSDIFV2_PrepareTimingData(&target_ios, host);

    err = FSdifMsgSetIos(&dev->hc, &target_ios);
    if (FSDIF_SUCCESS != err)
    {
        SDMMC_LOGE(TAG, "Set HS400 failed !!!"); 
        return;
    }

    SDMMC_LOGD(TAG, "Set HS400 success");
#endif
    return;
}

static void FSDIFV2_HOST_EnableStrobeDll(sdmmchost_t *host, bool enable)
{
    return;
}

static uint32_t FSDIFV2_HOST_GetSignalLineStatus(sdmmchost_t *host, uint32_t signalLine)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;
    return FSdifMsgCheckifCardBusy(&dev->hc) ? false : true;
}

static void FSDIFV2_HOST_ConvertDataToLittleEndian(sdmmchost_t *host, uint32_t *data, uint32_t wordSize, uint32_t format)
{
    uint32_t temp = 0U;

    if (((uint32_t)host->config.endianMode == (uint32_t)kSDMMCHOST_EndianModeLittle) &&
        (format == kSDMMC_DataPacketFormatMSBFirst))
    {
        for (uint32_t i = 0U; i < wordSize; i++)
        {
            temp    = data[i];
            data[i] = SWAP_WORD_BYTE_SEQUENCE(temp);
        }
    }
    else if ((uint32_t)host->config.endianMode == (uint32_t)kSDMMCHOST_EndianModeHalfWordBig)
    {
        for (uint32_t i = 0U; i < wordSize; i++)
        {
            temp    = data[i];
            data[i] = SWAP_HALF_WROD_BYTE_SEQUENCE(temp);
        }
    }
    else if (((uint32_t)host->config.endianMode == (uint32_t)kSDMMCHOST_EndianModeBig) &&
             (format == kSDMMC_DataPacketFormatLSBFirst))
    {
        for (uint32_t i = 0U; i < wordSize; i++)
        {
            temp    = data[i];
            data[i] = SWAP_WORD_BYTE_SEQUENCE(temp);
        }
    }
    else
    {
        /* nothing to do */
    }
}

static status_t FSDIFV2_HOST_CardDetectInit(sdmmchost_t *host, void *cd)
{
    return kStatus_Success;
}

static void FSDIFV2_HOST_SetCardPower(sdmmchost_t *host, bool enable)
{
    return;    
}

static void FSDIFV2_HOST_EnableCardInt(sdmmchost_t *host, bool enable)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;
    if (kSDMMCHOST_CARD_TYPE_SDIO == host->config.cardType)
    {
        FSdifMsgEnableSdioIrq(&dev->hc, enable);
    }
}

static status_t FSDIFV2_HOST_CardIntInit(sdmmchost_t *host, void *sdioInt)
{
    return kStatus_Success;    
}

static void FSDIFV2_HOST_SetCardBusWidth(sdmmchost_t *host, uint32_t dataBusWidth)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;
    FSdifMsgDataSetIos *cur_ios = &(dev->hc.cur_ios);
    FSdifMsgDataSetIos target_ios;
    FError err;

    if (host->currBusWidth == dataBusWidth)
    {
        return;
    }

    memcpy(&target_ios, cur_ios, sizeof(*cur_ios));
    FSDIFV2_PrepareBusWidthData(&target_ios, dataBusWidth);
    FSDIFV2_PrepareTimingData(&target_ios, host);

    err = FSdifMsgSetIos(&dev->hc, &target_ios);
    if (FSDIF_SUCCESS != err)
    {
        SDMMC_LOGE(TAG, "Set card bus width failed !!!"); 
        return;
    }

    SDMMC_LOGD(TAG, "Set card bus width %d", dataBusWidth);
    host->currBusWidth = dataBusWidth;
    return;      
}

static uint32_t FSDIFV2_HOST_CardDetectStatus(sdmmchost_t *host)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;

    return FSdifMsgCheckifCardExists(&dev->hc) ? kSD_Inserted : kSD_Removed;
}

static status_t FSDIFV2_HOST_PollingCardDetectStatus(sdmmchost_t *host, uint32_t waitCardStatus, uint32_t timeout)
{
    assert(host != NULL);
    assert(host->cd != NULL);    

    sd_detect_card_t *cd = host->cd;
    bool isCardInserted  = false;
    uint32_t retry_time = 100;
    
    /* Wait card inserted. */
    do
    {
        isCardInserted = (FSDIFV2_HOST_CardDetectStatus(host) == (uint32_t)kSD_Inserted);
        if ((waitCardStatus == (uint32_t)kSD_Inserted) && isCardInserted)
        {
            SDMMC_OSADelay(cd->cdDebounce_ms);
            if (FSDIFV2_HOST_CardDetectStatus(host) == (uint32_t)kSD_Inserted)
            {
                break;
            }
        }

        if ((isCardInserted == false) && (waitCardStatus == (uint32_t)kSD_Removed))
        {
            break;
        }
        if(--retry_time == 0)
        {
            break;
        }
    } while (true);
        
    if (retry_time == 0)
    {
        SDMMC_LOGE(TAG, "Wait card insert timeout !!!");
        return kStatus_Timeout;
    }

    return kStatus_Success; 
}

static void FSDIFV2_HOST_SendCardActive(sdmmchost_t *host)
{

}

static uint32_t FSDIFV2_HOST_SetCardClock(sdmmchost_t *host, uint32_t targetClock)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;
    FSdifMsgDataSetIos *cur_ios = &(dev->hc.cur_ios);
    FSdifMsgDataSetIos target_ios;
    FError err;

    if ((host->currClockFreq == targetClock))
    {
        return targetClock;
    }

    memcpy(&target_ios, cur_ios, sizeof(*cur_ios));
    FSDIFV2_PrepareClockData(&target_ios, targetClock);
    FSDIFV2_PrepareTimingData(&target_ios, host);

    SDMMC_LOGD(TAG, "Set card clock to %d", targetClock);
    err = FSdifMsgSetIos(&dev->hc, &target_ios);
    if (FSDIF_SUCCESS == err)
    {
        host->currClockFreq = targetClock;
    }
    else
    {
        SDMMC_LOGE(TAG, "Set clock to %d failed", targetClock);
    }

    return host->currClockFreq;     
}

static void FSDIFV2_HOST_ForceClockOn(sdmmchost_t *host, bool enable)
{

}

static bool FSDIFV2_HOST_IsCardBusy(sdmmchost_t *host)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;
    return FSdifMsgCheckifCardBusy(&dev->hc) ? true : false;
}

static status_t FSDIFV2_HOST_PreCommand(sdmmchost_t *host, sdmmchost_transfer_t *content)
{
    status_t err = kStatus_Success;

    if ((host->config.cardType == kSDMMCHOST_CARD_TYPE_STANDARD_SD) ||
        (host->config.cardType == kSDMMCHOST_CARD_TYPE_MICRO_SD))
    {
#if defined(CONFIG_FSL_SDMMC_ENABLE_SD)
        if ((kSDMMC_ReadMultipleBlock == content->command->index) || 
            (kSDMMC_WriteMultipleBlock == content->command->index))
        {
            u32 block_count = content->data->blockCount;
            if (block_count > 1U)
            {
                err = SDMMC_SetBlockCount(host, block_count);
            }
        }
#endif
    }

    return err;
}

static status_t FSDIFV2_HOST_TransferFunction_Irq(sdmmchost_t *host, sdmmchost_transfer_t *content)
{
    assert(content);
    status_t status = kStatus_Success;
    FError err;
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;
    FSdifMsgCommand *command = &(dev->cmd);
    FSdifMsgData *trans_data = &(dev->data);
    FSdifMsgRequest *request = &(dev->req);
    int32_t timeout = FSDIF_COMMAND_TIMEOUT;
    uint32_t complete_events = 0U;
    uint32_t err_events = FSDIF_TRANS_ERR_EVENTS;

    status = FSDIFV2_HOST_PreCommand(host, content);
    if (kStatus_NoData == status)
    {
        return kStatus_Success; /* skip command with success */
    }
    else if (kStatus_Success != status)
    {
        return status; 
    }

    memset(request, 0U, sizeof(*request));

    if (content->data)
    {
        memset(trans_data, 0U, sizeof(*trans_data));
        trans_data->buf = (void *)((content->data->rxData) ? (content->data->rxData) : (content->data->txData));
        trans_data->buf_dma = (uintptr)trans_data->buf;
        request->data = trans_data;
        complete_events |= SDMMC_OSA_EVENT_TRANSFER_DATA_SUCCESS;

        FSDIFV2_PrepareDataTransfer(&trans_data->datainfo, content, host);

        err = FSdifMsgSetupDMADescriptor(&(dev->hc), trans_data);
        if (FSDIF_SUCCESS != err)
        {
            SDMMC_LOGE(TAG, "SDIF setup DMA failed, err = 0x%x", err);
            status = kStatus_NoData;
            return status;
        }        
    }

    memset(command, 0U, sizeof(*command));
    request->command = command;
    complete_events |= SDMMC_OSA_EVENT_TRANSFER_CMD_SUCCESS;

    FSDIFV2_PrepareCommandTrasnfer(&command->cmdinfo, content, host);

    err = FSdifMsgDMATransfer(&(dev->hc), request);
    if (FSDIF_SUCCESS != err)
    {
        SDMMC_LOGE(TAG, "SDIF request failed, err = 0x%x", err);
        status = kStatus_NoData;
        return status;
    }

    SDMMC_LOGD(TAG, "Wait command event 0x%x", complete_events);

    uint32_t events = 0;
    if ((SDMMC_OSAEventWait(&dev->hc_evt, complete_events, timeout, &events, SDMMC_OSA_EVENT_FLAG_AND) != kStatus_Success) ||
        (events != complete_events))
    {
        SDMMC_LOGE(TAG, "Wait command done timeout !!!");
        status = kStatus_Timeout;
        (void)SDMMC_OSAEventClear(&dev->hc_evt, events);
        return status;
    }

    (void)SDMMC_OSAEventClear(&dev->hc_evt, events);

    events = 0;
    (void)SDMMC_OSAEventWait(&dev->hc_evt, err_events, 0, &events, SDMMC_OSA_EVENT_FLAG_OR);
    if (events)
    {
        SDMMC_LOGE(TAG, "Finish command with error 0x%x!!!", events);
        status = kStatus_Timeout;
        (void)SDMMC_OSAEventClear(&dev->hc_evt, events);
        return status;        
    }

    if (kCARD_ResponseTypeR2 == content->command->responseType)
    {
        content->command->response[3] = command->response[0];
        content->command->response[2] = command->response[1];
        content->command->response[1] = command->response[2];
        content->command->response[0] = command->response[3];
    }
    else
    {
        content->command->response[3] = 0;
        content->command->response[2] = 0;
        content->command->response[1] = 0;
        content->command->response[0] = command->response[0];
    }

    if (content->data)
    {
#if defined(CONFIG_LOG_DEBUG) || defined(CONFIG_LOG_VERBOS)
        if (kMMC_SendExtendedCsd == content->command->index)
        {
            FtDumpHexWord(content->data->rxData, content->data->blockSize * content->data->blockCount);
        }
#endif
    }

    SDMMC_LOGD(TAG, "============[%s-%d] end ============", "CMD", content->command->index);

    if (content->command->response[0U] & content->command->responseErrorFlags)
    {
        status = kStatus_Fail;
    }

    return status;
}

static status_t FSDIFV2_HOST_StartBoot(sdmmchost_t *host,
                                        sdmmchost_boot_config_t *hostConfig,
                                        sdmmchost_cmd_t *cmd,
                                        uint8_t *buffer)
{
    SDMMC_LOGE(TAG, "%s not implmented", __func__);
    return kStatus_Fail;
}

static status_t FSDIFV2_HOST_ReadBootData(sdmmchost_t *host, 
                                           sdmmchost_boot_config_t *hostConfig, 
                                           uint8_t *buffer)
{
    SDMMC_LOGE(TAG, "%s not implmented", __func__);
    return kStatus_Fail;
}

static void FSDIFV2_HOST_EnableBoot(sdmmchost_t *host, bool enable)
{
    SDMMC_LOGE(TAG, "%s not implmented", __func__);
    return;
}

static const sdmmchost_ops_t sdif_v2_ops = 
{
    .deinit = FSDIFV2_HOST_Deinit,
    .reset = FSDIFV2_HOST_Reset,

    .switchToVoltage = FSDIFV2_HOST_SwitchToVoltage,
    .executeTuning = FSDIFV2_HOST_ExecuteTuning,
    .enableDDRMode = FSDIFV2_HOST_EnableDDRMode,
    .enableHS400Mode = FSDIFV2_HOST_EnableHS400Mode,
    .enableStrobeDll = FSDIFV2_HOST_EnableStrobeDll,
    .getSignalLineStatus = FSDIFV2_HOST_GetSignalLineStatus,
    .convertDataToLittleEndian = FSDIFV2_HOST_ConvertDataToLittleEndian,

    .cardDetectInit = FSDIFV2_HOST_CardDetectInit,
    .cardSetPower = FSDIFV2_HOST_SetCardPower,
    .cardEnableInt = FSDIFV2_HOST_EnableCardInt,
    .cardIntInit = FSDIFV2_HOST_CardIntInit,
    .cardSetBusWidth = FSDIFV2_HOST_SetCardBusWidth,
    .cardPollingDetectStatus = FSDIFV2_HOST_PollingCardDetectStatus,
    .cardDetectStatus = FSDIFV2_HOST_CardDetectStatus,
    .cardSendActive = FSDIFV2_HOST_SendCardActive,
    .cardSetClock = FSDIFV2_HOST_SetCardClock,
    .cardForceClockOn = FSDIFV2_HOST_ForceClockOn,
    .cardIsBusy = FSDIFV2_HOST_IsCardBusy,

    .transferFunction = FSDIFV2_HOST_TransferFunction_Irq,

    .startBoot = FSDIFV2_HOST_StartBoot,
    .readBootData = FSDIFV2_HOST_ReadBootData,
    .enableBoot = FSDIFV2_HOST_EnableBoot, 
};

static void FSDIFV2_HOST_SetupIrq(FSdifMsgCtrl *ctrl_p)
{
    u32 cpu_id = 0;

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(ctrl_p->config.irq_num, cpu_id);
    InterruptSetPriority(ctrl_p->config.irq_num, 0xc);

    /* register intr callback */
    InterruptInstall(ctrl_p->config.irq_num, 
                     FSdifMsgInterruptHandler, 
                     ctrl_p, 
                     NULL);

    /* enable sdio irq */
    InterruptUmask(ctrl_p->config.irq_num);

    SDMMC_LOGD(TAG, "Sdif V2 interrupt setup done !!!");
    return;
}

static void FSDIFV2_HOST_CardDetected(FSdifMsgCtrl *const instance_p, void *args, void *data)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)args;
    SDMMC_LOGD(TAG, "Card detected !!!");
    (void)SDMMC_OSAEventSet(&dev->hc_evt, SDMMC_OSA_EVENT_CARD_INSERTED);
}

static void FSDIFV2_HOST_CmdDone(FSdifMsgCtrl *const instance_p, void *args, void *data)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)args;
    SDMMC_LOGD(TAG, "CMD done !!!");
    (void)SDMMC_OSAEventSet(&dev->hc_evt, SDMMC_OSA_EVENT_TRANSFER_CMD_SUCCESS);
}

static void FSDIFV2_HOST_DataDone(FSdifMsgCtrl *const instance_p, void *args, void *data)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)args;
    SDMMC_LOGD(TAG, "Data done !!!");
    (void)SDMMC_OSAEventSet(&dev->hc_evt, SDMMC_OSA_EVENT_TRANSFER_DATA_SUCCESS);
}

static void FSDIFV2_HOST_SDIOIrq(FSdifMsgCtrl *const instance_p, void *args, void *data)
{

}

static void FSDIFV2_HOST_ErrorOccur(FSdifMsgCtrl *const instance_p, void *args, void *data)
{
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)args;
    FSdifMsgDataErrIrq *err_data = (FSdifMsgDataErrIrq *)data;
    if (err_data)
    {
        u32 status = err_data->raw_ints;
        u32 dmac_status = err_data->dmac_status;

        if (status & FSDIF_INT_RE_BIT)
            SDMMC_LOGE(TAG, "Response err. 0x%x", FSDIF_INT_RE_BIT);

        if (status & FSDIF_INT_RTO_BIT)
            SDMMC_LOGE(TAG, "Response timeout. 0x%x", FSDIF_INT_RTO_BIT);

        if (dmac_status & FSDIF_DMAC_STATUS_DU)
            SDMMC_LOGE(TAG, "Descriptor un-readable. 0x%x", FSDIF_DMAC_STATUS_DU);

        if (status & FSDIF_INT_DCRC_BIT)
            SDMMC_LOGE(TAG, "Data CRC error. 0x%x", FSDIF_INT_DCRC_BIT);

        if (status & FSDIF_INT_RCRC_BIT)
            SDMMC_LOGE(TAG, "Data CRC error. 0x%x", FSDIF_INT_RCRC_BIT);

        if ((status & FSDIF_INT_RE_BIT) || (status & FSDIF_INT_RTO_BIT))
            (void)SDMMC_OSAEventSet(&dev->hc_evt, SDMMC_OSA_EVENT_TRANSFER_CMD_FAIL);

        if ((dmac_status & FSDIF_DMAC_STATUS_DU) || (status & FSDIF_INT_DCRC_BIT) || (status & FSDIF_INT_RCRC_BIT))
            (void)SDMMC_OSAEventSet(&dev->hc_evt, SDMMC_OSA_EVENT_TRANSFER_DATA_FAIL);        
    }
}

static status_t FSDIFV2_HOST_DoInit(sdmmchost_t *host)
{
    status_t ret = kStatus_Success;
    fsdifv2_host_dev_t *dev = (fsdifv2_host_dev_t *)host->dev;
    dev->hc_cfg = *FSdifMsgLookupConfig(host->config.hostId);
    
    if (!(host->config.enableDMA) || !(host->config.enableIrq))
    {
        SDMMC_LOGE(TAG, "Sdif v2 only works in DMA IRQ mode.");
        return kStatus_Fail;
    }

    if ((kSDMMCHOST_CARD_TYPE_STANDARD_SD == host->config.cardType) ||
        (kSDMMCHOST_CARD_TYPE_MICRO_SD == host->config.cardType) ||
        (kSDMMCHOST_CARD_TYPE_SDIO == host->config.cardType))
    {
#if defined(CONFIG_FSL_SDMMC_ENABLE_SD)
        dev->hc_cfg.non_removable = FALSE;
#else
        return kStatus_Fail;
#endif
    }
    else if (kSDMMCHOST_CARD_TYPE_EMMC == host->config.cardType)
    {
#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
        dev->hc_cfg.non_removable = TRUE;
#else
        return kStatus_Fail;
#endif
    }

    FSDIFV2_PrepareInitData(&(dev->hc_cfg.init), host);

    if (kStatus_Success != SDMMC_OSAEventCreate(&dev->hc_evt))
    {
        return kStatus_Fail;
    }

    memset(&dev->hc, 0, sizeof(dev->hc));
    if (FSDIF_SUCCESS != FSdifMsgCfgInitialize(&dev->hc, &dev->hc_cfg)) 
    {
        SDMMC_LOGE(TAG, "Sdif v2 ctrl init failed.");
        ret = kStatus_Fail;
        return ret;
    }

    FSDIFV2_HOST_SetupIrq(&dev->hc);
    FSdifMsgRegisterEvtHandler(&dev->hc, FSDIF_EVT_CARD_DETECTED, FSDIFV2_HOST_CardDetected, (void *)dev);
    FSdifMsgRegisterEvtHandler(&dev->hc, FSDIF_EVT_ERR_OCCURE, FSDIFV2_HOST_ErrorOccur, (void *)dev);
    FSdifMsgRegisterEvtHandler(&dev->hc, FSDIF_EVT_CMD_DONE, FSDIFV2_HOST_CmdDone, (void *)dev);
    FSdifMsgRegisterEvtHandler(&dev->hc, FSDIF_EVT_DATA_DONE, FSDIFV2_HOST_DataDone, (void *)dev);

    if (kSDMMCHOST_CARD_TYPE_SDIO == host->config.cardType)
    {
        FSdifMsgRegisterEvtHandler(&dev->hc, FSDIF_EVT_SDIO_IRQ, FSDIFV2_HOST_SDIOIrq, (void *)dev);
    }

    if (FSDIF_SUCCESS != FSdifMsgSetIDMAList(&dev->hc, dev->rw_desc, (uintptr_t)dev->rw_desc, dev->desc_num)) 
    {
        SDMMC_LOGE(TAG, "Sdif v2 ctrl setup DMA failed.");
        ret = kStatus_Fail;
        return ret;           
    }

    ret = FSDIFV2_PrepareInitIOS(&dev->hc, host);
    if (kStatus_Success == ret)
    {
        ret = FSDIFV2_PrepareInitVolt(&dev->hc, host);
    }

    return ret;
}

status_t FSDIFV2_HOST_Init(sdmmchost_t *host)
{
    assert(host);
    /* num of descriptor and sg list item need to finish the max trans blocks */
    size_t num_of_desc = host->config.maxTransSize / host->config.defBlockSize;

    /* find the space for dev instance */
    fsdifv2_host_dev_t *dev = SDMMC_OSAMemoryAllocate(sizeof(fsdifv2_host_dev_t));
    if (NULL == dev)
    {
        return kStatus_OutOfRange;
    }

    memset(dev, 0U, sizeof(*dev));
    dev->instance = host;

    /* find aligned space for dma descriptor buffer */
    dev->rw_desc = SDMMC_OSAMemoryAlignedAllocate(sizeof(FSdifMsgIDmaDesc) * num_of_desc, 
                                                  host->config.defBlockSize);
    SDMMC_LOGD(TAG, "rw_desc = %p", dev->rw_desc);
    if (NULL == dev->rw_desc)
    {
        return kStatus_OutOfRange;
    }

    memset(dev->rw_desc, 0U, sizeof(FSdifMsgIDmaDesc) * num_of_desc);
    dev->desc_num = num_of_desc;

    host->ops = sdif_v2_ops;
    host->dev = dev;

    return FSDIFV2_HOST_DoInit(host);
}

#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
status_t FSDIFV2_HOST_MMCConfig(sdmmc_mmc_t *sdmmc, sdmmchost_config_t *config)
{
    assert(sdmmc);
    assert(config);
    sdmmchost_t *host = &sdmmc->host;
    mmc_card_t *card = &sdmmc->card;

    card->usrParam.ioStrength = NULL;
    card->usrParam.maxFreq = config->cardClock;
    card->usrParam.capability = (uint32_t)kSDMMC_Support8BitWidth |
                                (uint32_t)kMMC_SupportHighSpeed26MHZFlag |
                                (uint32_t)kMMC_SupportHighSpeed52MHZFlag |
                                (uint32_t)kMMC_SupportHS200200MHZ180VFlag |
                                (uint32_t)kMMC_SupportHS400DDR200MHZ180VFlag;
    card->hostVoltageWindowVCC = (uint32_t)kMMC_VoltageWindow170to195;
    card->hostVoltageWindowVCCQ = (uint32_t)kMMC_VoltageWindow170to195;
    card->noInteralAlign = FALSE;
    card->enablePreDefinedBlockCount = TRUE;

    host->capability |= (uint32_t)kSDMMCHOST_Support8BitDataWidth |
                        (uint32_t)kSDMMCHOST_Support4BitDataWidth |
                        (uint32_t)kSDMMCHOST_SupportVoltage1v8 |
                        (uint32_t)kSDMMCHOST_SupportHighSpeed |
                        (uint32_t)kSDMMCHOST_SupportHS200 |
                        (uint32_t)kSDMMCHOST_SupportHS400 |
                        (uint32_t)kSDMMCHOST_SupportDDRMode |
                        (uint32_t)kSDMMCHOST_SupportAutoCmd12;
    host->maxBlockCount = host->config.maxTransSize / host->config.defBlockSize;
    host->maxBlockSize  = SDMMCHOST_SUPPORT_MAX_BLOCK_LENGTH;
    host->sourceClock_Hz = FSDIF_CLK_FREQ_HZ; /* 1.2GHz */

    return kStatus_Success;
}
#endif

#if defined(CONFIG_FSL_SDMMC_ENABLE_SD)
status_t FSDIFV2_HOST_SDConfig(sdmmc_sd_t *sdmmc, sdmmchost_config_t *config)
{
    assert(sdmmc);
    assert(config);
    sdmmchost_t *host = &sdmmc->host;
    sd_card_t *card = &sdmmc->card;
    sd_detect_card_t *card_cd = &sdmmc->cardDetect;
    sd_io_voltage_t *io_voltage = &sdmmc->ioVoltage;

    card_cd->type = kSD_DetectCardByHostCD;
    card_cd->cdDebounce_ms = 10;

    card->usrParam.pwr = NULL;
    card->usrParam.powerOnDelayMS = 0U;
    card->usrParam.powerOffDelayMS = 0U;
    card->usrParam.ioStrength = NULL;
    card->usrParam.capability = (uint32_t)kSDMMCHOST_SupportSuspendResume |
                                (uint32_t)kSDMMCHOST_Support4BitDataWidth |
                                (uint32_t)kSDMMCHOST_SupportDetectCardByData3 |
                                (uint32_t)kSDMMCHOST_SupportDetectCardByCD |
                                (uint32_t)kSDMMCHOST_SupportAutoCmd12 |
                                (uint32_t)kSDMMCHOST_SupportDriverTypeC |
                                (uint32_t)kSDMMCHOST_SupportSetCurrent;
    card->noInteralAlign = FALSE;

    if (host->config.isUHSCard)
    {
        card->usrParam.ioVoltage = io_voltage; /* need to switch voltage to 1.8v */
        card->usrParam.ioVoltage->type = kSD_IOVoltageCtrlByHost;
        card->usrParam.ioVoltage->func = NULL;

        host->capability |= (uint32_t)kSDMMCHOST_SupportVoltage3v3 |
                            (uint32_t)kSDMMCHOST_SupportHighSpeed |
                            (uint32_t)kSDMMCHOST_SupportVoltage1v8 |
                            (uint32_t)kSDMMCHOST_SupportSDR50 | 
                            (uint32_t)kSDMMCHOST_SupportSDR104;
    }
    else
    {
        card->usrParam.ioVoltage = NULL; /* no need to switch voltage */
        host->capability |= (uint32_t)kSDMMCHOST_SupportVoltage3v3;

        if (host->config.cardClock >= SD_CLOCK_50MHZ)
        {
            host->capability |= (uint32_t)kSDMMCHOST_SupportHighSpeed;
        }
    }

    card->usrParam.maxFreq = (uint32_t)host->config.cardClock;

    host->maxBlockCount = host->config.maxTransSize / host->config.defBlockSize;
    host->maxBlockSize  = SDMMCHOST_SUPPORT_MAX_BLOCK_LENGTH;
    host->sourceClock_Hz = FSDIF_CLK_FREQ_HZ; /* 1.2GHz */

    return kStatus_Success;
}
#endif

#if defined(CONFIG_FSL_SDMMC_ENABLE_SDIO)
status_t FSDIFV2_HOST_SDIOConfig(sdmmc_sdio_t *sdmmc, sdmmchost_config_t *config)
{
    assert(0);
    return kStatus_Success;
}
#endif