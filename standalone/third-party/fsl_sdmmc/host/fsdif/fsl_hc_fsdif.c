/*
 * Copyright (c) 2023 Phytium Information Technology, Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_sdmmc_common.h"
#include "fsl_sdmmc.h"
#include "fparameters.h"
#include "fsdif_hw.h"
#include "fsdif.h"
#include "fsdif_timing.h"

#include "finterrupt.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FSDIF_COMMAND_TIMEOUT       5000U /* 5 seconds timeout in sd spec. */
#define FSDIF_TRANS_ERR_EVENTS      (SDMMC_OSA_EVENT_TRANSFER_CMD_FAIL | \
                                     SDMMC_OSA_EVENT_TRANSFER_DATA_FAIL | \
                                     SDMMC_OSA_EVENT_CARD_REMOVED)

static const char* TAG = "SDMMC:FSdif";

typedef struct _fsdiohost_dev_
{
    sdmmchost_t *instance;
    FSdif hc;
    FSdifConfig hc_cfg;
    FSdifCmdData cmd_pkg;
    FSdifData dat_pkg;
    FSdifIDmaDesc *rw_desc;
    uint32_t desc_num;
    sdmmc_osa_event_t hc_evt;
} fsdifhost_dev_t;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static void FSDIFHOST_Relax(void)
{
    SDMMC_OSADelayUs(10);
}

static void FSDIFHOST_SetCardBusWidth(sdmmchost_t *host, uint32_t dataBusWidth)
{
    if (host->currBusWidth == dataBusWidth)
    {
        return;
    }

    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;

    switch (dataBusWidth)
    {
        case kSDMMC_BusWdith1Bit:
            FSdifSetCardBusWidth(&dev->hc, 1);
            host->currBusWidth = dataBusWidth;
            break;
        case kSDMMC_BusWdith4Bit:
            FSdifSetCardBusWidth(&dev->hc, 4);
            host->currBusWidth = dataBusWidth;
            break;
        case kSDMMC_BusWdith8Bit:
            FSdifSetCardBusWidth(&dev->hc, 8);
            host->currBusWidth = dataBusWidth;
            break;
        default:
            break;
    }

    return;
}

static void FSDIFHOST_SendCardActive(sdmmchost_t *host)
{

}

static uint32_t FSDIFHOST_SetCardClock(sdmmchost_t *host, uint32_t targetClock)
{
    if (host->currClockFreq == targetClock)
    {
        return targetClock;
    }

    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;
    (void)dev;

    if (host->config.prefixedTiming)
    {
        /* use fixed timing when data transfer failed with CRC error somehow */
#if defined(CONFIG_TARGET_PE2202) || defined(CONFIG_TARGET_PE2204) || \
    defined(CONFIG_TARGET_PD2308)
        FSdifTiming timing;
        memset(&timing, 0U, sizeof(timing));
        if (FSDIF_SUCCESS != FSdifGetTimingSetting(targetClock, dev->hc.config.non_removable, &timing))
        {
            SDMMC_LOGE(TAG, "Failed to find timing for clock-%d", targetClock);
            return 0;
        }
    
        if (FSDIF_SUCCESS != FSdifSetClkFreqByDict(&dev->hc, FALSE, &timing, targetClock)) 
        {
            SDMMC_LOGE(TAG, "Failed to update clock");
        }
        else
        {
            SDMMC_LOGD(TAG, "BUS CLOCK: %d", targetClock);
            host->currClockFreq = targetClock;
        }
#else
        SDMMC_LOGE(TAG, "Prefix timing not supported !!!");
        assert(0);
#endif
    }
    else
    {
        boolean is_ddr = FALSE;

        if (host->config.cardType == kSDMMCHOST_CARD_TYPE_EMMC)
        {
#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
            mmc_card_t *card = host->card;
            mmc_high_speed_timing_t timing = card->busTiming;
            if ((kMMC_HighSpeed400Timing == timing) ||
                (kMMC_EnhanceHighSpeed400Timing == timing))
            {
                is_ddr = TRUE;
            }
#endif
        }
        else if ((kSDMMCHOST_CARD_TYPE_STANDARD_SD == host->config.cardType) ||
                (kSDMMCHOST_CARD_TYPE_MICRO_SD == host->config.cardType))
        {
#if defined(CONFIG_FSL_SDMMC_ENABLE_SD)        
            sd_card_t *card = host->card;
            sd_timing_mode_t timing = card->currentTiming;
    
            if (kSD_TimingDDR50Mode == timing)
            {
                is_ddr = TRUE;
            }
#endif
        }
    
        if (FSDIF_SUCCESS != FSdifSetClkFreqByCalc(&dev->hc, is_ddr, targetClock))
        {
            SDMMC_LOGE(TAG, "Failed to update clock");
            return host->currClockFreq;
        }
        else
        {
            SDMMC_LOGD(TAG, "BUS CLOCK: %d", targetClock);
            host->currClockFreq = targetClock;
        }
    }
    
    return host->currClockFreq;
}

static bool FSDIFHOST_IsCardBusy(sdmmchost_t *host)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;

    return FSdifCheckCardBusy(&dev->hc) ? true : false;
}

static uint32_t FSDIFHOST_GetSignalLineStatus(sdmmchost_t *host, uint32_t signalLine)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;

    return FSdifCheckCardBusy(&dev->hc) ? false : true;
}

static void FSDIFHOST_EnableCardInt(sdmmchost_t *host, bool enable)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;
    if (kSDMMCHOST_CARD_TYPE_SDIO == host->config.cardType)
    {
        FSdifSetInterruptMask(&dev->hc, FSDIF_GENERAL_INTR, FSDIF_INT_SDIO_BIT, enable);
    }
}

static void FSDIFHOST_EnableDDRMode(sdmmchost_t *host, bool enable, uint32_t nibblePos)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;

    FSdifSetCardDDRMode(&dev->hc, enable);
}

static void FSDIFHOST_EnableHS400Mode(sdmmchost_t *host, bool enable)
{

}

static void FSDIFHOST_EnableStrobeDll(sdmmchost_t *host, bool enable)
{

}

static status_t FSDIFHOST_StartBoot(sdmmchost_t *host,
                                    sdmmchost_boot_config_t *hostConfig,
                                    sdmmchost_cmd_t *cmd,
                                    uint8_t *buffer)
{
    SDMMC_LOGE(TAG, "%s not implmented", __func__);
    return kStatus_Fail;
}

static status_t FSDIFHOST_ReadBootData(sdmmchost_t *host, 
                                sdmmchost_boot_config_t *hostConfig, 
                                uint8_t *buffer)
{
    SDMMC_LOGE(TAG, "%s not implmented", __func__);
    return kStatus_Fail;
}

static void FSDIFHOST_EnableBoot(sdmmchost_t *host, bool enable)
{
    SDMMC_LOGE(TAG, "%s not implmented", __func__);
    return;
}

static status_t FSDIFHOST_CardIntInit(sdmmchost_t *host, void *sdioInt)
{
    return kStatus_Success;    
}

static void FSDIFHOST_ForceClockOn(sdmmchost_t *host, bool enable)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;
    uintptr base_addr = dev->hc_cfg.base_addr;

    FSdifSetClock(base_addr, ((enable) ? TRUE : FALSE));
}

static void FSDIFHOST_SwitchToVoltage(sdmmchost_t *host, uint32_t voltage)
{
    if (host->currVoltage == voltage)
    {
        return;
    }

    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;
    uintptr base_addr = dev->hc_cfg.base_addr;

    switch (voltage)
    {
    case kSDMMC_OperationVoltageNone:
        break;
    case kSDMMC_OperationVoltage300V:
    case kSDMMC_OperationVoltage330V:
        FSdifSetVoltage1_8V(base_addr, FALSE);
        host->currVoltage = voltage;
        SDMMC_LOGD(TAG, "Switch to 3.3v");
        break;
    case kSDMMC_OperationVoltage180V:
        FSdifSetVoltage1_8V(base_addr, TRUE);
        host->currVoltage = voltage;
        SDMMC_LOGD(TAG, "Switch to 1.8v");
        break;
    default:
        SDMMC_LOGE(TAG, "Invalid target voltage !!!");
        break;
    }

    return;
}

static status_t FSDIFHOST_CardDetectInit(sdmmchost_t *host, void *cd)
{
    return kStatus_Success;
}

static uint32_t FSDIFHOST_CardDetectStatus(sdmmchost_t *host)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;

    return FSdifCheckCardExists(&dev->hc) ? kSD_Inserted : kSD_Removed;
}

static status_t FSDIFHOST_PollingCardDetectStatus(sdmmchost_t *host, uint32_t waitCardStatus, uint32_t timeout)
{
    assert(host != NULL);
    assert(host->cd != NULL);    

    sd_detect_card_t *cd = host->cd;
    bool isCardInserted  = false;
    uint32_t retry_time = 100;
    
    /* Wait card inserted. */
    do
    {
        isCardInserted = (FSDIFHOST_CardDetectStatus(host) == (uint32_t)kSD_Inserted);
        if ((waitCardStatus == (uint32_t)kSD_Inserted) && isCardInserted)
        {
            SDMMC_OSADelay(cd->cdDebounce_ms);
            if (FSDIFHOST_CardDetectStatus(host) == (uint32_t)kSD_Inserted)
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

static void FSDIFHOST_SetCardPower(sdmmchost_t *host, bool enable)
{

}

static uint32_t FSDIFHOST_PerpareRawCommand(sdmmchost_t *host, sdmmchost_transfer_t *in_trans, FSdifCmdData *out_trans)
{
    sdmmchost_cmd_t *in_cmd = in_trans->command;
    sdmmchost_data_t *in_data = in_trans->data;
    FSdifCmdData *out_cmd = out_trans;
    FSdifData *out_data = out_trans->data_p;
    assert(in_cmd);
    assert(out_cmd);
    uint32_t opcode = in_cmd->index;
    u32 raw_cmd = FSDIF_CMD_INDX_SET(opcode);

    if (kSDMMC_GoIdleState == in_cmd->index)
    {
        raw_cmd |= FSDIF_CMD_INIT;
    }

    if ((kSDMMC_GoInactiveState == in_cmd->index) ||
        ((kSDIO_RWIODirect == in_cmd->index) && 
        ((in_cmd->argument >> 9) & 0x1FFFF) == kSDIO_RegIOAbort))
    {
        raw_cmd |= FSDIF_CMD_STOP_ABORT;
    }

    if (kCARD_ResponseTypeNone != in_cmd->responseType)
    {
        raw_cmd |= FSDIF_CMD_RESP_EXP;

        if (kCARD_ResponseTypeR2 == in_cmd->responseType)
        {
            /* need 136 bits long response */
            raw_cmd |= FSDIF_CMD_RESP_LONG;
        }

        if ((kCARD_ResponseTypeR3 != in_cmd->responseType) &&
            (kCARD_ResponseTypeR4 != in_cmd->responseType))
        {
            /* most cmds need CRC */
            raw_cmd |= FSDIF_CMD_RESP_CRC;
        }
    }

    if (kSD_VoltageSwitch == in_cmd->index)
    {
        /* CMD11 need switch voltage */
        raw_cmd |= FSDIF_CMD_VOLT_SWITCH;
    }

    if (in_data)
    {
        assert(out_data);
        raw_cmd |= FSDIF_CMD_DAT_EXP;

        if (in_data->rxData)
        {
            out_data->buf = (void *)in_data->rxData;
            out_data->buf_dma = (uintptr)in_data->rxData;
        }
        else if (in_data->txData)
        {
            raw_cmd |= FSDIF_CMD_DAT_WRITE;
            out_data->buf = (void *)in_data->txData;
            out_data->buf_dma = (uintptr)in_data->txData;
        }
        else
        {
            assert(0);
        }

        out_data->blksz = in_data->blockSize;
        out_data->blkcnt = in_data->blockCount;
        out_data->datalen = in_data->blockSize * in_data->blockCount;
        SDMMC_LOGD(TAG, "buf@%p, blksz: %d, datalen: %ld", 
                    out_data->buf, 
                    out_data->blksz, out_data->datalen);

    }

    out_cmd->cmdidx = in_cmd->index;
    out_cmd->cmdarg = in_cmd->argument;

    return raw_cmd;
}

static status_t FSDIFHOST_PreCommand(sdmmchost_t *host, sdmmchost_transfer_t *content)
{
    if ((kSDMMC_ReadMultipleBlock == content->command->index) || 
        (kSDMMC_WriteMultipleBlock == content->command->index)/*&& (content->data)*/)
    {
        /* it's a quirk that some host controller need to send CMD12 before mulit-block read/write */
        u32 block_count = content->data->blockCount;
        if (block_count > 1U)
        {
            return SDMMC_SetBlockCount(host, block_count);
        }
    }

    return kStatus_Success;
}

static status_t FSDIFHOST_TransferFunction_Poll(sdmmchost_t *host, sdmmchost_transfer_t *content)
{
    assert(content);
    status_t status = kStatus_Success;
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;
    FSdifCmdData *cmd_data = &(dev->cmd_pkg);
    FSdifData *trans_data = &(dev->dat_pkg);

    status = FSDIFHOST_PreCommand(host, content);
    if (kStatus_Success != status)
    {
        return status; 
    }

    memset(cmd_data, 0U, sizeof(*cmd_data));

    if (content->data)
    {
        memset(trans_data, 0, sizeof(*trans_data));
        cmd_data->data_p = trans_data;
    }
    else
    {
        cmd_data->data_p = NULL;
    }

    cmd_data->rawcmd = FSDIFHOST_PerpareRawCommand(host, content, cmd_data);

    if (host->config.enableDMA)
    {
        if (cmd_data->data_p)
        {
            if (FSDIF_SUCCESS != FSdifSetupDMADescriptor(&dev->hc, cmd_data->data_p))
            {
                status = kStatus_NoData;
                return status;            
            }
        }

        if (FSDIF_SUCCESS != FSdifDMATransfer(&dev->hc, cmd_data)) 
        {
            status = kStatus_NoData;
            return status; 
        }
    }
    else
    {
        if (FSDIF_SUCCESS != FSdifPIOTransfer(&dev->hc, cmd_data)) 
        {
            status = kStatus_NoData;
            return status; 
        }  
    }

    if (FSDIF_SUCCESS != FSdifPollWaitTransferEnd(&dev->hc,cmd_data))
    {
        status = kStatus_NoData;
        return status;
    }

    if (FSDIF_SUCCESS != FSdifGetCmdResponse(&dev->hc, cmd_data))
    {
        SDMMC_LOGE(TAG, "Transfer cmd and data failed !!!");
        status = kStatus_Timeout;
        return status;
    }

    /* check if any error events */
    uint32_t err_events = SDMMC_OSA_EVENT_TRANSFER_CMD_FAIL | SDMMC_OSA_EVENT_TRANSFER_DATA_FAIL |
                          SDMMC_OSA_EVENT_CARD_REMOVED;
    uint32_t events = 0;
    (void)SDMMC_OSAEventWait(&dev->hc_evt, err_events, 0, &events, SDMMC_OSA_EVENT_FLAG_OR);
    if (events)
    {
        SDMMC_LOGE(TAG, "Finish command with error 0x%x!!!", events);
        status = kStatus_Timeout;
        FSdifDumpRegister(dev->hc.config.base_addr);
        (void)SDMMC_OSAEventClear(&dev->hc_evt, events);
        return status;
    }

    if (cmd_data->rawcmd & FSDIF_CMD_RESP_LONG)
    {
        content->command->response[0] = cmd_data->response[0];
        content->command->response[1] = cmd_data->response[1];
        content->command->response[2] = cmd_data->response[2];
        content->command->response[3] = cmd_data->response[3];
    }
    else if (cmd_data->rawcmd & FSDIF_CMD_RESP_EXP)
    {
        content->command->response[0] = cmd_data->response[0];
        content->command->response[1] = 0U;
        content->command->response[2] = 0U;
        content->command->response[3] = 0U;
    }

    if (content->command->response[0U] & content->command->responseErrorFlags)
    {
        status = kStatus_Fail;
    }

    return status; 
}

static status_t FSDIFHOST_TransferFunction_Irq(sdmmchost_t *host, sdmmchost_transfer_t *content)
{
    assert(content);
    status_t status = kStatus_Success;
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;
    FSdifCmdData *cmd_data = &(dev->cmd_pkg);
    FSdifData *trans_data = &(dev->dat_pkg);
    int32_t timeout = FSDIF_COMMAND_TIMEOUT;

    status = FSDIFHOST_PreCommand(host, content);
    if (kStatus_Success != status)
    {
        return status; 
    }

    memset(cmd_data, 0U, sizeof(*cmd_data));

    if (content->data)
    {
        memset(trans_data, 0, sizeof(*trans_data));
        cmd_data->data_p = trans_data;

        if (!(host->config.enableDMA) && (content->data->rxData))
        {
            /* for PIO mode, we just read data after finish transfer,
               therefore no need to check if data done */
        }
    }
    else
    {
        cmd_data->data_p = NULL;
    }

    cmd_data->rawcmd = FSDIFHOST_PerpareRawCommand(host, content, cmd_data);

    if (host->config.enableDMA)
    {
        if (cmd_data->data_p)
        {
            if (FSDIF_SUCCESS != FSdifSetupDMADescriptor(&dev->hc, cmd_data->data_p))
            {
                status = kStatus_NoData;
                return status;            
            }
        }

        if (FSDIF_SUCCESS != FSdifDMATransfer(&dev->hc, cmd_data)) 
        {
            status = kStatus_NoData;
            return status; 
        }
    }
    else
    {
        if (FSDIF_SUCCESS != FSdifPIOTransfer(&dev->hc, cmd_data)) 
        {
            status = kStatus_NoData;
            return status; 
        }     
    }

    uint32_t complete_events = (cmd_data->data_p) ? 
                         (SDMMC_OSA_EVENT_TRANSFER_CMD_SUCCESS | SDMMC_OSA_EVENT_TRANSFER_DATA_SUCCESS) : 
                         SDMMC_OSA_EVENT_TRANSFER_CMD_SUCCESS;
    uint32_t events = 0;
    if ((SDMMC_OSAEventWait(&dev->hc_evt, complete_events, timeout, &events, SDMMC_OSA_EVENT_FLAG_AND) != kStatus_Success) ||
        (events != complete_events))
    {
        SDMMC_LOGE(TAG, "Wait command done timeout !!!");
        status = kStatus_Timeout;
        FSdifDumpRegister(dev->hc.config.base_addr);
        (void)SDMMC_OSAEventClear(&dev->hc_evt, events);
        return status;
    }
    
    (void)SDMMC_OSAEventClear(&dev->hc_evt, events);

    /* check if any error events */
    uint32_t err_events = FSDIF_TRANS_ERR_EVENTS;
    events = 0;
    (void)SDMMC_OSAEventWait(&dev->hc_evt, err_events, 0, &events, SDMMC_OSA_EVENT_FLAG_OR);
    if (events)
    {
        SDMMC_LOGE(TAG, "Finish command with error 0x%x!!!", events);
        status = kStatus_Timeout;
        FSdifDumpRegister(dev->hc.config.base_addr);
        (void)SDMMC_OSAEventClear(&dev->hc_evt, events);
        return status;        
    }

    /* in IRQ mode, read PIO data after recv DTO flag */
    if (FSDIF_SUCCESS != FSdifGetCmdResponse(&dev->hc, cmd_data))
    {
        SDMMC_LOGE(TAG, "Transfer cmd and data failed !!!");
        status = kStatus_Timeout;
        return status;
    }

    if (cmd_data->rawcmd & FSDIF_CMD_RESP_LONG)
    {
        content->command->response[0] = cmd_data->response[0];
        content->command->response[1] = cmd_data->response[1];
        content->command->response[2] = cmd_data->response[2];
        content->command->response[3] = cmd_data->response[3];
    }
    else if (cmd_data->rawcmd & FSDIF_CMD_RESP_EXP)
    {
        content->command->response[0] = cmd_data->response[0];
        content->command->response[1] = 0U;
        content->command->response[2] = 0U;
        content->command->response[3] = 0U;
    }

    if (content->command->response[0U] & content->command->responseErrorFlags)
    {
        status = kStatus_Fail;
    }

    return status;
}

static status_t FSDIFHOST_ExecuteTuning(sdmmchost_t *host, uint32_t tuningCmd, uint32_t *revBuf, uint32_t blockSize)
{
    return kStatus_Success;
}

static void FSDIFHOST_ConvertDataToLittleEndian(sdmmchost_t *host, uint32_t *data, uint32_t wordSize, uint32_t format)
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

static void FSDIFHOST_SetupIrq(FSdif *ctrl_p)
{
    u32 cpu_id = 0;

    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(ctrl_p->config.irq_num, cpu_id);
    InterruptSetPriority(ctrl_p->config.irq_num, 0xc);

    /* register intr callback */
    InterruptInstall(ctrl_p->config.irq_num, 
                     FSdifInterruptHandler, 
                     ctrl_p, 
                     NULL);

    /* enable sdio irq */
    InterruptUmask(ctrl_p->config.irq_num);

    SDMMC_LOGD(TAG, "Sdio interrupt setup done !!!");
    return;
}

static void FSDIFHOST_RevokeIrq(FSdif *ctrl_p)
{
    /* disable sdio irq */
    InterruptMask(ctrl_p->config.irq_num);
}

static void FSDIFHOST_CardDetected(FSdif *const instance_p, void *args, u32 status, u32 dmac_status)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)args;
    SDMMC_LOGD(TAG, "Card detected !!!");
    (void)SDMMC_OSAEventSet(&dev->hc_evt, SDMMC_OSA_EVENT_CARD_INSERTED);
}

static void FSDIFHOST_CmdDone(FSdif *const instance_p, void *args, u32 status, u32 dmac_status)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)args;
    SDMMC_LOGD(TAG, "CMD done !!!");
    (void)SDMMC_OSAEventSet(&dev->hc_evt, SDMMC_OSA_EVENT_TRANSFER_CMD_SUCCESS);
}

static void FSDIFHOST_DataDone(FSdif *const instance_p, void *args, u32 status, u32 dmac_status)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)args;
    SDMMC_LOGD(TAG, "Data done !!!");

    uint32_t check_status = status & (FSDIF_INT_DTO_BIT | FSDIF_INT_RCRC_BIT |
                                        FSDIF_INT_DCRC_BIT | FSDIF_INT_RE_BIT |
                                        FSDIF_INT_DRTO_BIT | FSDIF_INT_EBE_BIT |
                                        FSDIF_INT_SBE_BCI_BIT | FSDIF_INT_RTO_BIT);    
	uint32_t check_dmac =  dmac_status & (FSDIF_DMAC_STATUS_AIS | FSDIF_DMAC_STATUS_DU);

    if (NULL == dev->cmd_pkg.data_p) 
    {
        (void)SDMMC_OSAEventSet(&dev->hc_evt, SDMMC_OSA_EVENT_TRANSFER_DATA_SUCCESS);
    } 
    else if (check_status | check_dmac) 
    {
        if (check_status & FSDIF_INT_DTO_BIT) 
        {
            (void)SDMMC_OSAEventSet(&dev->hc_evt, SDMMC_OSA_EVENT_TRANSFER_DATA_SUCCESS);
        } 
        else 
        {
            SDMMC_LOGE(TAG, "Xfer data error, status: 0x%x, dmac status: 0x%x", 
                       check_status, check_dmac);
        }
    }
}

static void FSDIFHOST_ErrorOccur(FSdif *const instance_p, void *args, u32 status, u32 dmac_status)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)args;
    SDMMC_LOGE(TAG, "Error occur !!!");
    SDMMC_LOGE(TAG, "Status: 0x%x, dmac status: 0x%x.", status, dmac_status);

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

    FSdifDumpRegister(instance_p->config.base_addr);

    if ((status & FSDIF_INT_RE_BIT) || (status & FSDIF_INT_RTO_BIT))
        (void)SDMMC_OSAEventSet(&dev->hc_evt, SDMMC_OSA_EVENT_TRANSFER_CMD_FAIL);

    if ((dmac_status & FSDIF_DMAC_STATUS_DU) || (status & FSDIF_INT_DCRC_BIT) || (status & FSDIF_INT_RCRC_BIT))
        (void)SDMMC_OSAEventSet(&dev->hc_evt, SDMMC_OSA_EVENT_TRANSFER_DATA_FAIL);
}

static void FSDIFHOST_SdioInterrupt(FSdif *const instance_p, void *args, u32 status, u32 dmac_status)
{
#if defined(CONFIG_FSL_SDMMC_ENABLE_SDIO)
    assert(args);
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)args;
    assert(dev->instance);
    sdmmchost_t *host = dev->instance;
    assert(host->card);
    sdio_card_t *card = (sdio_card_t *)host->card;

    if ((card->usrParam.sdioInt) && (card->usrParam.sdioInt->cardInterrupt))
    {
        card->usrParam.sdioInt->cardInterrupt(card->usrParam.sdioInt->userData);
    }
#endif
}

static status_t FSDIFHOST_Reset(sdmmchost_t *host)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;

    if (FSDIF_SUCCESS != FSdifRestart(&dev->hc))
    {
        return kStatus_Fail;
    }
    else
    {
        return kStatus_Success;
    }
}
static status_t FSDIFHOST_DoInit(sdmmchost_t *host)
{
    status_t ret = kStatus_Success;
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;
    dev->hc_cfg = *FSdifLookupConfig(host->config.hostId);

    if ((kSDMMCHOST_CARD_TYPE_STANDARD_SD == host->config.cardType) ||
        (kSDMMCHOST_CARD_TYPE_MICRO_SD == host->config.cardType) ||
        (kSDMMCHOST_CARD_TYPE_SDIO == host->config.cardType))
    {
        dev->hc_cfg.non_removable = FALSE;
    }
    else if (kSDMMCHOST_CARD_TYPE_EMMC == host->config.cardType)
    {
        dev->hc_cfg.non_removable = TRUE;
    }

    if (host->config.enableDMA)
    {
        dev->hc_cfg.trans_mode = FSDIF_IDMA_TRANS_MODE;
    }
    else
    {
        dev->hc_cfg.trans_mode = FSDIF_PIO_TRANS_MODE;
    }

    if (kStatus_Success != SDMMC_OSAEventCreate(&dev->hc_evt))
    {
        return kStatus_Fail;
    }

    memset(&dev->hc, 0, sizeof(dev->hc));
    if (FSDIF_SUCCESS != FSdifCfgInitialize(&dev->hc, &dev->hc_cfg)) 
    {
        SDMMC_LOGE(TAG, "Sdio ctrl init failed.");
        ret = kStatus_Fail;
        return ret;
    }

    FSdifRegisterRelaxHandler(&dev->hc, FSDIFHOST_Relax);

    if (host->config.enableIrq)
    {
        FSDIFHOST_SetupIrq(&dev->hc);
        FSdifRegisterEvtHandler(&dev->hc, FSDIF_EVT_CARD_DETECTED, FSDIFHOST_CardDetected, (void *)dev);
        FSdifRegisterEvtHandler(&dev->hc, FSDIF_EVT_ERR_OCCURE, FSDIFHOST_ErrorOccur, (void *)dev);
        FSdifRegisterEvtHandler(&dev->hc, FSDIF_EVT_CMD_DONE, FSDIFHOST_CmdDone, (void *)dev);
        FSdifRegisterEvtHandler(&dev->hc, FSDIF_EVT_DATA_DONE, FSDIFHOST_DataDone, (void *)dev);
        FSdifRegisterEvtHandler(&dev->hc, FSDIF_EVT_SDIO_IRQ, FSDIFHOST_SdioInterrupt, (void *)dev);
    }

    if (host->config.enableDMA)
    {
        if (FSDIF_SUCCESS != FSdifSetIDMAList(&dev->hc, dev->rw_desc, (uintptr_t)dev->rw_desc, dev->desc_num)) 
        {
            SDMMC_LOGE(TAG, "Sdio ctrl setup DMA failed.");
            ret = kStatus_Fail;
            return ret;           
        }
    }

    return ret;
}

static void FSDIFHOST_Deinit(sdmmchost_t *host)
{
    fsdifhost_dev_t *dev = (fsdifhost_dev_t *)host->dev;

    FSDIFHOST_RevokeIrq(&dev->hc);
    FSdifDeInitialize(&dev->hc);
    SDMMC_OSAEventDestroy(&dev->hc_evt);

    SDMMC_OSAMemoryFree(dev->rw_desc);
    
    memset(&dev->hc, 0, sizeof(dev->hc));

    SDMMC_OSAMemoryFree(dev);

    SDMMC_LOG("Sdif host deinited !!!");
    return;
}

static const sdmmchost_ops_t sdio_ops = 
{
    .deinit = FSDIFHOST_Deinit,
    .reset = FSDIFHOST_Reset,

    .switchToVoltage = FSDIFHOST_SwitchToVoltage,
    .executeTuning = FSDIFHOST_ExecuteTuning,
    .enableDDRMode = FSDIFHOST_EnableDDRMode,
    .enableHS400Mode = FSDIFHOST_EnableHS400Mode,
    .enableStrobeDll = FSDIFHOST_EnableStrobeDll,
    .getSignalLineStatus = FSDIFHOST_GetSignalLineStatus,
    .convertDataToLittleEndian = FSDIFHOST_ConvertDataToLittleEndian,

    .cardDetectInit = FSDIFHOST_CardDetectInit,
    .cardSetPower = FSDIFHOST_SetCardPower,
    .cardEnableInt = FSDIFHOST_EnableCardInt,
    .cardIntInit = FSDIFHOST_CardIntInit,
    .cardSetBusWidth = FSDIFHOST_SetCardBusWidth,
    .cardPollingDetectStatus = FSDIFHOST_PollingCardDetectStatus,
    .cardDetectStatus = FSDIFHOST_CardDetectStatus,
    .cardSendActive = FSDIFHOST_SendCardActive,
    .cardSetClock = FSDIFHOST_SetCardClock,
    .cardForceClockOn = FSDIFHOST_ForceClockOn,
    .cardIsBusy = FSDIFHOST_IsCardBusy,

    .transferFunction = NULL,

    .startBoot = FSDIFHOST_StartBoot,
    .readBootData = FSDIFHOST_ReadBootData,
    .enableBoot = FSDIFHOST_EnableBoot,
};


status_t FSDIFHOST_Init(sdmmchost_t *host)
{
    assert(host);
    /* num of descriptor and sg list item need to finish the max trans blocks */
    size_t num_of_desc = host->config.maxTransSize / host->config.defBlockSize;

    /* find the space for dev instance */
    fsdifhost_dev_t *dev = SDMMC_OSAMemoryAllocate(sizeof(fsdifhost_dev_t));
    if (NULL == dev)
    {
        return kStatus_OutOfRange;
    }

    memset(dev, 0U, sizeof(*dev));
    dev->instance = host;

    /* find aligned space for dma descriptor buffer */
    dev->rw_desc = SDMMC_OSAMemoryAlignedAllocate(sizeof(FSdifIDmaDesc) * num_of_desc, 
                                                  host->config.defBlockSize);
    SDMMC_LOGD(TAG, "rw_desc = %p", dev->rw_desc);
    if (NULL == dev->rw_desc)
    {
        return kStatus_OutOfRange;
    }
    
    memset(dev->rw_desc, 0U, sizeof(FSdifIDmaDesc) * num_of_desc);
    dev->desc_num = num_of_desc;

    host->ops = sdio_ops;
    host->dev = dev;

    if (host->config.enableIrq)
    {
        host->ops.transferFunction = FSDIFHOST_TransferFunction_Irq;
    }
    else
    {
        host->ops.transferFunction = FSDIFHOST_TransferFunction_Poll;
    }

    return FSDIFHOST_DoInit(host); 
}

#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
status_t FSDIFHOST_MMCConfig(sdmmc_mmc_t *sdmmc, sdmmchost_config_t *config)
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
#if defined(CONFIG_TARGET_PD2408)
                        (uint32_t)kSDMMCHOST_SupportHS400 |
                        (uint32_t)kSDMMCHOST_SupportDDRMode |
#endif
                        (uint32_t)kSDMMCHOST_SupportAutoCmd12;
    host->maxBlockCount = host->config.maxTransSize / host->config.defBlockSize;
    host->maxBlockSize  = SDMMCHOST_SUPPORT_MAX_BLOCK_LENGTH;
    host->sourceClock_Hz = FSDIF_CLK_FREQ_HZ; /* 1.2GHz */

    return kStatus_Success;
}
#endif

#if defined(CONFIG_FSL_SDMMC_ENABLE_SD)
status_t FSDIFHOST_SDConfig(sdmmc_sd_t *sdmmc, sdmmchost_config_t *config)
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
status_t FSDIFHOST_SDIOConfig(sdmmc_sdio_t *sdmmc, sdmmchost_config_t *config)
{
    assert(sdmmc);
    assert(config);
    sdmmchost_t *host = &sdmmc->host;
    sdio_card_t *card = &sdmmc->card;
    sd_detect_card_t *card_cd = &sdmmc->cardDetect;
    sdio_card_int_t *card_int = &sdmmc->sdioInt;

    card_cd->type = kSD_DetectCardByHostCD;
    card_cd->cdDebounce_ms = 10;

    card->usrParam.pwr = NULL;
    card->usrParam.powerOnDelayMS = 0U;
    card->usrParam.powerOffDelayMS = 0U;

    card->usrParam.ioStrength = NULL;
    card->usrParam.maxFreq = config->cardClock;
    card->usrParam.capability = 0U;
    card->usrParam.ioVoltage = NULL;

    card->noInternalAlign = FALSE;

    card_int->cardInterrupt = config->sdioCardInt;
    card_int->userData = config->sdioCardIntArg;
    card->usrParam.sdioInt = card_int;

    host->capability |= (uint32_t)kSDMMCHOST_SupportVoltage3v3 | 
                        (uint32_t)kSDMMCHOST_SupportHighSpeed |
                        (uint32_t)kSDMMCHOST_Support4BitDataWidth;
    host->maxBlockCount = host->config.maxTransSize / host->config.defBlockSize;
    host->maxBlockSize  = SDMMCHOST_SUPPORT_MAX_BLOCK_LENGTH;
    host->sourceClock_Hz = FSDIF_CLK_FREQ_HZ; /* 1.2GHz */

    return kStatus_Success;    
}
#endif