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

static const char* TAG = "SDMMC:FSdifMsgCtrl:V2";

void FSDIFV2_PrepareInitData(FSdifMsgDataInit *msg_data_init, sdmmchost_t *host)
{
    assert(msg_data_init);
    assert(host);

    memset(msg_data_init, 0U, sizeof(*msg_data_init));

    if ((host->config.cardType == kSDMMCHOST_CARD_TYPE_STANDARD_SD) ||
        (host->config.cardType == kSDMMCHOST_CARD_TYPE_MICRO_SD))
    {
        msg_data_init->caps = FSDIF_MMC_CAP_4_BIT_DATA | FSDIF_MMC_CAP_SD_HIGHSPEED |
                              FSDIF_MMC_CAP_UHS | FSDIF_MMC_CAP_CMD23;
    }
    else if (host->config.cardType == kSDMMCHOST_CARD_TYPE_EMMC)
    {
        msg_data_init->caps = FSDIF_MMC_CAP_4_BIT_DATA | FSDIF_MMC_CAP_8_BIT_DATA |
                              FSDIF_MMC_CAP_MMC_HIGHSPEED | FSDIF_MMC_CAP_NONREMOVABLE |
                              FSDIF_MMC_CAP_1_8V_DDR | FSDIF_MMC_CAP_CMD23 | FSDIF_MMC_CAP_HW_RESET;
    }

    msg_data_init->clk_rate = host->sourceClock_Hz;
}

status_t FSDIFV2_PrepareInitVolt(FSdifMsgCtrl *const instance, sdmmchost_t *host)
{
    FSdifMsgDataSwitchVolt target_volt;

    if (kSDMMCHOST_CARD_TYPE_EMMC == host->config.cardType)
    {
        target_volt.signal_voltage = FSDIF_MMC_SIGNAL_VOLTAGE_180;
    }
    else if((kSDMMCHOST_CARD_TYPE_STANDARD_SD == host->config.cardType) ||
            (kSDMMCHOST_CARD_TYPE_MICRO_SD == host->config.cardType))
    {
        target_volt.signal_voltage = FSDIF_MMC_SIGNAL_VOLTAGE_330;
    }

    if (FSDIF_SUCCESS != FSdifMsgSetVoltage(instance, &target_volt))
    {       
        return kStatus_Fail;
    }

    switch (target_volt.signal_voltage)
    {
    case FSDIF_MMC_SIGNAL_VOLTAGE_330:
        host->currVoltage = kSDMMC_OperationVoltage330V;
        break;
    case FSDIF_MMC_SIGNAL_VOLTAGE_180:
        host->currVoltage = kSDMMC_OperationVoltage180V;
        break;
    default:
        break;
    }

    return kStatus_Success;
}

status_t FSDIFV2_PrepareInitIOS(FSdifMsgCtrl *const instance, sdmmchost_t *host)
{
    FSdifMsgDataSetIos target_ios;

    target_ios.ios_clock = 0U;
    target_ios.ios_timing = FSDIF_MMC_TIMING_LEGACY;
    target_ios.ios_bus_width = FSDIF_MMC_BUS_WIDTH_1;
    target_ios.ios_power_mode = FSDIF_MMC_POWER_UP;
    if (FSDIF_SUCCESS != FSdifMsgSetIos(instance, &target_ios))
    {
        SDMMC_LOGE(TAG, "Set init IOS failed");
        return kStatus_Fail;
    }

    instance->cur_ios.ios_power_mode = FSDIF_MMC_POWER_ON;
    host->currClockFreq = 0U;
    host->currBusWidth = kSDMMC_BusWdith1Bit;
    return kStatus_Success;
}

void FSDIFV2_PrepareTimingData(FSdifMsgDataSetIos *msg_set_ios, sdmmchost_t *host)
{
    assert(msg_set_ios);
    if ((host->config.cardType == kSDMMCHOST_CARD_TYPE_STANDARD_SD) ||
        (host->config.cardType == kSDMMCHOST_CARD_TYPE_MICRO_SD))
    {
#if defined(CONFIG_FSL_SDMMC_ENABLE_SD)
        sd_card_t *card = host->card;
        sd_timing_mode_t timing = card->currentTiming;

        if (host->currVoltage == kSDMMC_OperationVoltage330V)
        {
            if (msg_set_ios->ios_clock == SDMMC_CLOCK_400KHZ)
            {
                msg_set_ios->ios_timing = FSDIF_MMC_TIMING_LEGACY;
            }
            else
            {
                msg_set_ios->ios_timing = FSDIF_MMC_TIMING_SD_HS;
            }
        }
        else
        {
            switch (timing)
            {
                case kSD_TimingSDR12DefaultMode:
                    msg_set_ios->ios_timing = FSDIF_MMC_TIMING_UHS_SDR12;
                    break;
                case kSD_TimingSDR25HighSpeedMode:
                    msg_set_ios->ios_timing = FSDIF_MMC_TIMING_UHS_SDR25;
                    break;
                case kSD_TimingSDR50Mode:
                    msg_set_ios->ios_timing = FSDIF_MMC_TIMING_UHS_SDR50;
                    break;
                case kSD_TimingSDR104Mode:
                    msg_set_ios->ios_timing = FSDIF_MMC_TIMING_UHS_SDR104;
                    break;
                case kSD_TimingDDR50Mode:
                    msg_set_ios->ios_timing = FSDIF_MMC_TIMING_UHS_DDR50;
                    break;
                default:
                    break;
            }
        }
#else
        assert(0);
#endif
    }
    else if (host->config.cardType == kSDMMCHOST_CARD_TYPE_EMMC)
    {
#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
        mmc_card_t *card = host->card;
        mmc_high_speed_timing_t timing = card->busTiming;

        switch (timing)
        {
        case kMMC_HighSpeedTimingNone:
            msg_set_ios->ios_timing = FSDIF_MMC_TIMING_LEGACY;
            break;
        case kMMC_HighSpeedTiming:
            msg_set_ios->ios_timing = FSDIF_MMC_TIMING_MMC_HS;
            break;
        case kMMC_HighSpeed200Timing:
            msg_set_ios->ios_timing = FSDIF_MMC_TIMING_MMC_HS200;
            break;
        case kMMC_HighSpeed400Timing:
        case kMMC_EnhanceHighSpeed400Timing:
            msg_set_ios->ios_timing = FSDIF_MMC_TIMING_MMC_HS400;
            break;
        default:
            break;
        }
#else
        assert(0);
#endif
    }
}

void FSDIFV2_PrepareBusWidthData(FSdifMsgDataSetIos *msg_set_ios, uint32_t dataBusWidth)
{
    assert(msg_set_ios);

    switch (dataBusWidth)
    {
    case kSDMMC_BusWdith1Bit:
        msg_set_ios->ios_bus_width = FSDIF_MMC_BUS_WIDTH_1;
        break;
    case kSDMMC_BusWdith4Bit:
        msg_set_ios->ios_bus_width = FSDIF_MMC_BUS_WIDTH_4;
        break;
    case kSDMMC_BusWdith8Bit:
        msg_set_ios->ios_bus_width = FSDIF_MMC_BUS_WIDTH_8;
        break;
    default:
        assert(0);
        break;
    }
}

void FSDIFV2_PrepareClockData(FSdifMsgDataSetIos *msg_set_ios, uint32_t targetClock)
{
    assert(msg_set_ios);

    msg_set_ios->ios_clock = targetClock;
}

void FSDIFV2_PrepareVoltageData(FSdifMsgDataSwitchVolt *msg_volt, uint32_t voltage)
{
    assert(msg_volt);

    switch (voltage)
    {
    case kSDMMC_OperationVoltage330V:
        msg_volt->signal_voltage = FSDIF_MMC_SIGNAL_VOLTAGE_330;
        break;
    case kSDMMC_OperationVoltage180V:
        msg_volt->signal_voltage = FSDIF_MMC_SIGNAL_VOLTAGE_180;
        break;
    default:
        assert(0);
        break;
    }
}

static uint32_t FSDIFV2_PrepareRawCommand(sdmmchost_transfer_t *content)
{
    assert(content);
    assert(content->command);
    uint32_t opcode = content->command->index;
    uint32_t argument = content->command->argument;
    sdmmc_card_response_type_t resp_type = content->command->responseType;
    uint32_t raw_cmd = FSDIF_CMD_INDX_SET(opcode);

    if (kSDMMC_GoIdleState == opcode)
    {
        raw_cmd |= FSDIF_CMD_INIT;
    }

    if ((kSDMMC_GoInactiveState == opcode) ||
        ((kSDIO_RWIODirect == opcode) && 
        ((argument >> 9) & 0x1FFFF) == kSDIO_RegIOAbort))
    {
        raw_cmd |= FSDIF_CMD_STOP_ABORT;
    }

    if (kCARD_ResponseTypeNone != resp_type)
    {
        raw_cmd |= FSDIF_CMD_RESP_EXP;

        if (kCARD_ResponseTypeR2 == resp_type)
        {
            /* need 136 bits long response */
            raw_cmd |= FSDIF_CMD_RESP_LONG;
        }

        if ((kCARD_ResponseTypeR3 != resp_type) &&
            (kCARD_ResponseTypeR4 != resp_type))
        {
            /* most cmds need CRC */
            raw_cmd |= FSDIF_CMD_RESP_CRC;
        }
    }

    if (kSD_VoltageSwitch == opcode)
    {
        /* CMD11 need switch voltage */
        raw_cmd |= FSDIF_CMD_VOLT_SWITCH;
    }

    if (content->data)
    {
        raw_cmd |= FSDIF_CMD_DAT_EXP;

        if (content->data->txData)
        {
            raw_cmd |= FSDIF_CMD_DAT_WRITE;
        }
    }

    raw_cmd |= FSDIF_CMD_START;

    return raw_cmd;
}

#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
static uint32_t FSDIFV2_PrepareMMCCommandFlags(sdmmchost_transfer_t *content)
{
    uint32_t opcode = content->command->index;
    uint32_t argument = content->command->argument;
    uint32_t flags = 0U;

    switch(opcode)
    {
        case kSDMMC_GoIdleState: /* MMC_GO_IDLE_STATE 0 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_NONE | FSDIF_MMC_CMD_BC;
            break;
        case kMMC_SendOperationCondition: /* MMC_SEND_OP_COND 1 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R3 | FSDIF_MMC_CMD_BCR;
            break;
        case kSDMMC_AllSendCid: /* MMC_ALL_SEND_CID 2 */
            flags |= FSDIF_MMC_RSP_R2 | FSDIF_MMC_CMD_AC;
            break;
        case kMMC_SetRelativeAddress: /* MMC_SET_RELATIVE_ADDR 3 */
            flags |= FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_AC;
            break;
        case kSDMMC_SendCsd: /* MMC_SEND_CSD 9 */
            flags |= FSDIF_MMC_RSP_R2 | FSDIF_MMC_CMD_AC;
            break;
        case kSDMMC_SelectCard: /* MMC_SELECT_CARD 7 */
            if (argument)
            {
                flags |= FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_AC;
            }
            else
            {
                flags |= FSDIF_MMC_RSP_NONE | FSDIF_MMC_CMD_AC;
            }
            break;
        case kMMC_SendExtendedCsd: /* MMC_SEND_EXT_CSD 8 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_ADTC;
            break;
        case kMMC_Switch: /* MMC_SWITCH 6 */
            flags |= FSDIF_MMC_CMD_AC | FSDIF_MMC_RSP_SPI_R1B | FSDIF_MMC_RSP_R1B;
            break;
        case kSDMMC_SendStatus: /* MMC_SEND_STATUS 13 */
            flags |= FSDIF_MMC_RSP_SPI_R2 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_AC;
            break;
        case kSDMMC_SetBlockLength: /* MMC_SET_BLOCKLEN 16 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_AC;
            break;
        case kSDMMC_SetBlockCount: /* MMC_SET_BLOCK_COUNT 23 */
            flags |= FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_AC;
            break;
        case kSDMMC_WriteSingleBlock: /* MMC_WRITE_BLOCK 24 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_ADTC;
            break;
        case kSDMMC_WriteMultipleBlock: /* MMC_WRITE_MULTIPLE_BLOCK 25 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_ADTC;
            break;
        case kSDMMC_ReadSingleBlock: /* MMC_READ_SINGLE_BLOCK 17 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_ADTC;
            break;
        case kSDMMC_ReadMultipleBlock: /* MMC_READ_MULTIPLE_BLOCK 18 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_ADTC;
            break;
        default:
            SDMMC_LOGE(TAG, "unhandled command-%d !!!", opcode);
            assert(0);
            break;
    }

    return flags;
}
#endif

#if defined(CONFIG_FSL_SDMMC_ENABLE_SD)
static uint32_t FSDIFV2_PrepareSDCommandFlags(sdmmchost_transfer_t *content)
{
    uint32_t opcode = content->command->index;
    uint32_t argument = content->command->argument;
    uint32_t flags = 0U;

    switch(opcode)
    {
        case kSDMMC_GoIdleState: /* MMC_GO_IDLE_STATE 0 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_NONE | FSDIF_MMC_CMD_BC;
            break;
        case kSD_SendInterfaceCondition: /* SD_SEND_IF_COND 8 */
            flags |= FSDIF_MMC_RSP_SPI_R7 | FSDIF_MMC_RSP_R7 | FSDIF_MMC_CMD_BCR;
            break;
        case kSD_ApplicationSendOperationCondition: /* SD_APP_OP_COND 41 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R3 | FSDIF_MMC_CMD_BCR;
            break;
        case kSD_VoltageSwitch: /* SD_SWITCH_VOLTAGE 11 */
            flags |= FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_AC;
            break;
        case kSDMMC_AllSendCid: /* MMC_ALL_SEND_CID 2 */
            flags |= FSDIF_MMC_RSP_R2 | FSDIF_MMC_CMD_AC;
            break;
        case kSD_SendRelativeAddress: /* SD_SEND_RELATIVE_ADDR 3 */
            flags |= FSDIF_MMC_RSP_R6 | FSDIF_MMC_CMD_BCR;
            break;
        case kSDMMC_SendCsd: /* MMC_SEND_CSD 9 */
            flags |= FSDIF_MMC_RSP_R2 | FSDIF_MMC_CMD_AC;
            break;
        case kSDMMC_SelectCard: /* MMC_SELECT_CARD 7 */
            if (argument)
            {
                flags |= FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_AC;
            }
            else
            {
                flags |= FSDIF_MMC_RSP_NONE | FSDIF_MMC_CMD_AC;
            }
            break;
        case kSDMMC_ApplicationCommand: /* MMC_APP_CMD 55 */
            if (argument)
            {
                flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_AC;
            }
            else
            {
                flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_BCR;
            }
            break;
        case kSD_ApplicationSendScr: /* SD_APP_SEND_SCR 51 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_ADTC;
            break;
        case kSD_ApplicationSetBusWdith: /* SD_APP_SET_BUS_WIDTH 6 */
            flags |= FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_AC;
            break;
        case kSD_ApplicationStatus: /* SD_APP_SD_STATUS 13 */
            flags |= FSDIF_MMC_RSP_SPI_R2 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_ADTC;
            break;
        case kSDMMC_SetBlockLength: /* MMC_SET_BLOCKLEN 16 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_AC;
            break;
        case kSDMMC_SetBlockCount: /* MMC_SET_BLOCK_COUNT 23 */
            flags |= FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_AC;
            break;
        case kSDMMC_WriteSingleBlock: /* MMC_WRITE_BLOCK 24 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_ADTC;
            break;
        case kSDMMC_WriteMultipleBlock: /* MMC_WRITE_MULTIPLE_BLOCK 25 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_ADTC;
            break;
        case kSDMMC_ReadSingleBlock: /* MMC_READ_SINGLE_BLOCK 17 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_ADTC;
            break;
        case kSDMMC_ReadMultipleBlock: /* MMC_READ_MULTIPLE_BLOCK 18 */
            flags |= FSDIF_MMC_RSP_SPI_R1 | FSDIF_MMC_RSP_R1 | FSDIF_MMC_CMD_ADTC;
            break;
        default:
            SDMMC_LOGE(TAG, "unhandled command-%d !!!", opcode);
            assert(0);
            break;
    }

    return flags;
}
#endif

void FSDIFV2_PrepareCommandTrasnfer(FSdifMsgDataStartCmd *msg_cmd, sdmmchost_transfer_t *content, sdmmchost_t *host)
{
    assert(msg_cmd);
    assert(content);
    assert(content->command);

    memset(msg_cmd, 0U, sizeof(*msg_cmd));

    msg_cmd->opcode = content->command->index;
    msg_cmd->cmd_arg = content->command->argument;
    msg_cmd->raw_cmd = FSDIFV2_PrepareRawCommand(content);

    if ((host->config.cardType == kSDMMCHOST_CARD_TYPE_STANDARD_SD) ||
        (host->config.cardType == kSDMMCHOST_CARD_TYPE_MICRO_SD))
    {
#if defined(CONFIG_FSL_SDMMC_ENABLE_SD)
        msg_cmd->flags = FSDIFV2_PrepareSDCommandFlags(content);
#else
        assert(0);
#endif    
    }
    else if (host->config.cardType == kSDMMCHOST_CARD_TYPE_EMMC)
    {
#if defined(CONFIG_FSL_SDMMC_ENABLE_MMC)
        msg_cmd->flags = FSDIFV2_PrepareMMCCommandFlags(content);
#else
        assert(0);
#endif
    }

    return;
}

void FSDIFV2_PrepareDataTransfer(FSdifMsgDataStartData *msg_data, sdmmchost_transfer_t *content, sdmmchost_t *host)
{
    assert(msg_data);
    assert(content);
    assert(content->command);
    assert(content->data);

    memset(msg_data, 0U, sizeof(*msg_data));

    msg_data->cmd_arg = content->command->argument;
    msg_data->raw_cmd = FSDIFV2_PrepareRawCommand(content);
    if ((content->command->index == kSDMMC_WriteSingleBlock) ||
        (content->command->index == kSDMMC_WriteMultipleBlock))
    {
        msg_data->data_flags = FSDIF_MMC_DATA_WRITE;
    }
    else
    {
        msg_data->data_flags = FSDIF_MMC_DATA_READ;
    }

    msg_data->adtc_type = FSDIF_BLOCK_RW_ADTC;
    msg_data->adma_addr = 0U; /* we do not know the descriptor addr here */
    msg_data->mrq_data_blksz = content->data->blockSize;
    msg_data->mrq_data_blocks = content->data->blockCount;

    return;
}