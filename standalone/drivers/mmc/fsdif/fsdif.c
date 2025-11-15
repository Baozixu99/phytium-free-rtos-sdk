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
 *
 *
 * FilePath: fsdif.c
 * Date: 2022-05-26 16:27:54
 * LastEditTime: 2022-05-26 16:27:54
 * Description:  This file is for sdif user function implementation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/2    init
 * 1.1   zhugengyu  2022/6/6     modify according to tech manual.
 * 2.0   zhugengyu  2023/9/16    rename as sdif, support SD 3.0 and rework clock timing
 */

/***************************** Include Files *********************************/

#include "fdrivers_port.h"
#include "fsdif_hw.h"
#include "fsdif.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FSDIF_DEBUG_TAG "FSDIF"
#define FSDIF_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_WARN(format, ...) FT_DEBUG_PRINT_W(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_INFO(format, ...) FT_DEBUG_PRINT_I(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
static FError FSdifReset(FSdif *const instance_p);
static FError FSdifUpdateExternalClk(uintptr base_addr, u32 uhs_reg_val);

/*****************************************************************************/
/**
 * @name: FSdifCfgInitialize
 * @msg: initialization SDIF controller instance
 * @return {FError} FSDIF_SUCCESS if initialization success, otherwise failed
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {FSdifConfig} *input_config_p, SDIF controller configure
 * @note get into card-detect mode after initialization, bus width = 1, card freq = 400kHz
 */
FError FSdifCfgInitialize(FSdif *const instance_p, const FSdifConfig *input_config_p)
{
    FASSERT(instance_p && input_config_p);
    FError ret = FSDIF_SUCCESS;

    if (FT_COMPONENT_IS_READY == instance_p->is_ready)
    {
        FSDIF_WARN("Device is already initialized!!!");
    }

    if (&instance_p->config != input_config_p)
    {
        instance_p->config = *input_config_p;
    }

    ret = FSdifReset(instance_p); /* reset the device */

    if (FSDIF_SUCCESS == ret)
    {
        instance_p->is_ready = FT_COMPONENT_IS_READY;
        FSDIF_INFO("Device initialize success !!!");
    }

    return ret;
}

/**
 * @name: FSdifDeInitialize
 * @msg: deinitialization SDIF controller instance
 * @return {NONE}
 * @param {FSdif} *instance_p, SDIF controller instance
 */
void FSdifDeInitialize(FSdif *const instance_p)
{
    FASSERT(instance_p);
    uintptr base_addr = instance_p->config.base_addr;

    FSdifSetPower(base_addr, FALSE);       /* 关闭电源 */
    FSdifSetClock(base_addr, FALSE);       /* 关闭卡时钟 */
    FSdifSetClockSrc(base_addr, FALSE);    /* 关闭外部时钟 */
    FSdifSetVoltage1_8V(base_addr, FALSE); /* 恢复为3.3v默认电压 */

    FSdifSetInterruptMask(instance_p, FSDIF_GENERAL_INTR, FSDIF_INT_ALL_BITS, FALSE); /* 关闭控制器中断位 */
    FSdifSetInterruptMask(instance_p, FSDIF_IDMA_INTR, FSDIF_DMAC_INT_ENA_ALL, FALSE); /* 关闭DMA中断位 */

    FSdifClearRawStatus(base_addr); /* 清除中断状态 */
    FSdifClearDMAStatus(base_addr);

    /* enable card detect interrupt */
    if (FALSE == instance_p->config.non_removable)
    {
        FSDIF_SET_BIT(base_addr, FSDIF_INT_MASK_OFFSET, FSDIF_INT_CD_BIT);
    }

    instance_p->is_ready = 0;
}

/**
 * @name: FSdifSetClkFreqByDict
 * @msg: Set the Card clock freq with fixed timing
 * @return {FError} FSDIF_SUCCESS if set clock success, otherwise failed
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {boolean} is_ddr, TRUE if DDR mode, FALSE if SDR mode
 * @param {FSdifTiming} *target_timing, Timing parameters for card clock
 * @param {u32} input_clk_hz, Card clock freqency in Hz
 */
FError FSdifSetClkFreqByDict(FSdif *const instance_p, boolean is_ddr,
                             const FSdifTiming *target_timing, u32 input_clk_hz)
{
    FASSERT(instance_p);
    uintptr base_addr = instance_p->config.base_addr;
    u32 mci_cmd_bits = FSDIF_CMD_UPD_CLK;
    FError ret = FSDIF_SUCCESS;
    u32 cmd_reg;
    u32 cur_cmd_index;

    cmd_reg = FSDIF_READ_REG(base_addr, FSDIF_CMD_OFFSET);
    cur_cmd_index = FSDIF_CMD_INDX_GET(cmd_reg);

    FSDIF_INFO("Set clk as %ld.", input_clk_hz);
    if (FSDIF_SWITCH_VOLTAGE == cur_cmd_index)
    {
        mci_cmd_bits |= FSDIF_CMD_VOLT_SWITCH;
    }

    if (input_clk_hz > 0)
    {
        /* update pad delay */
        if (target_timing->pad_delay)
        {
            target_timing->pad_delay(instance_p->config.instance_id);
        }

        /* update clock source setting */
        ret = FSdifUpdateExternalClk(base_addr, target_timing->clk_src);
        if (FSDIF_SUCCESS != ret)
        {
            FSDIF_ERROR("update clock source failed !!!");
            return ret;
        }

        FSdifSetClock(base_addr, FALSE);

        /* update clock for clock source */
        if (FSDIF_SWITCH_VOLTAGE == cur_cmd_index)
        {
            ret = FSdifSendPrivateCmd11(base_addr, mci_cmd_bits | cmd_reg);
        }
        else
        {
            ret = FSdifSendPrivateCmd(base_addr, mci_cmd_bits, 0);
        }

        if (FSDIF_SUCCESS != ret)
        {
            FSDIF_ERROR("update ext clock failed !!!");
            return ret;
        }

        /* set clock divider */
        FSDIF_WRITE_REG(base_addr, FSDIF_CLKDIV_OFFSET, target_timing->clk_div);

        FSDIF_WRITE_REG(base_addr, FSDIF_ENABLE_SHIFT_OFFSET, target_timing->shift);

        FSDIF_INFO("clk_src: 0x%x clk_div: 0x%x, shift: 0x%x",
                   FSDIF_READ_REG(base_addr, FSDIF_CLK_SRC_OFFSET),
                   FSDIF_READ_REG(base_addr, FSDIF_CLKDIV_OFFSET),
                   FSDIF_READ_REG(base_addr, FSDIF_ENABLE_SHIFT_OFFSET));

        if (is_ddr)
        {
            FSDIF_SET_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_CRC_SERIAL_DATA);
            FSDIF_SET_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_DRV_SHIFT_EN);
        }
        else
        {
            FSDIF_CLR_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_CRC_SERIAL_DATA);
            FSDIF_CLR_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_DRV_SHIFT_EN);
        }

        FSDIF_WRITE_REG(base_addr, FSDIF_FULL_CLK_DIV_OFFSET, target_timing->clk_divider);

        FSdifSetClock(base_addr, TRUE);

        /* update clock for clock divider */
        if (FSDIF_SWITCH_VOLTAGE == cur_cmd_index)
        {
            ret = FSdifSendPrivateCmd11(base_addr, mci_cmd_bits | cmd_reg);
        }
        else
        {
            ret = FSdifSendPrivateCmd(base_addr, mci_cmd_bits, 0);
        }
    }
    else
    {
        /* close bus clock in case target clock is 0 */
        FSdifSetClock(base_addr, FALSE);

        if (FSDIF_SWITCH_VOLTAGE == cur_cmd_index)
        {
            ret = FSdifSendPrivateCmd11(base_addr, mci_cmd_bits | cmd_reg);
        }
        else
        {
            ret = FSdifSendPrivateCmd(base_addr, mci_cmd_bits, 0);
        }

        if (FSDIF_SUCCESS != ret)
        {
            FSDIF_ERROR("update ext clock failed !!!");
            return ret;
        }

        FSdifSetClock(base_addr, TRUE);
    }

    FSDIF_INFO("Update clock freq done.");
    return ret;
}

/**
 * @name: FSdifSetClkFreqByCalc
 * @msg: Set the Card clock freq with caclcuated timing
 * @return {FError} FSDIF_SUCCESS if set clock success, otherwise failed
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {boolean} is_ddr, TRUE if DDR mode, FALSE if SDR mode
 * @param {u32} input_clk_hz, Card clock freqency in Hz
 */
FError FSdifSetClkFreqByCalc(FSdif *const instance_p, boolean is_ddr, u32 input_clk_hz)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    uintptr base_addr = instance_p->config.base_addr;
    u32 div = 0xff, drv = 0, sample = 0;
    unsigned long clk_rate;
    u32 mci_cmd_bits = FSDIF_CMD_UPD_CLK;
    u32 cmd_reg;
    u32 cur_cmd_index;
    u32 first_uhs_div, tmp_ext_reg;
    FError ret = FSDIF_SUCCESS;

    cmd_reg = FSDIF_READ_REG(base_addr, FSDIF_CMD_OFFSET);
    cur_cmd_index = FSDIF_CMD_INDX_GET(cmd_reg);

    if (cur_cmd_index == FSDIF_SWITCH_VOLTAGE)
    {
        mci_cmd_bits |= FSDIF_CMD_VOLT_SWITCH;
    }

    if (input_clk_hz)
    {
        if (input_clk_hz == instance_p->cur_clock)
        {
            return ret;
        }

        FSDIF_INFO("will change clock, src_clk_rate: %d, input_clk_rate: %d",
                   instance_p->config.src_clk_rate, input_clk_hz);

        if (input_clk_hz >= FSDIF_CLK_SPEED_25_MHZ)
        {
            tmp_ext_reg = 0x102;
        }
        else if (input_clk_hz == FSDIF_CLK_SPEED_400KHZ)
        {
            tmp_ext_reg = 0x502;
        }
        else
        {
            tmp_ext_reg = 0x302;
        }

        ret = FSdifUpdateExternalClk(base_addr, tmp_ext_reg);
        if (FSDIF_SUCCESS != ret)
        {
            FSDIF_ERROR("update external clock failed");
            return ret;
        }

        FSdifSetClock(base_addr, FALSE);

        if (cur_cmd_index == FSDIF_SWITCH_VOLTAGE)
        {
            ret = FSdifSendPrivateCmd11(base_addr, mci_cmd_bits | cmd_reg);
        }
        else
        {
            ret = FSdifSendPrivateCmd(base_addr, mci_cmd_bits, 0);
        }

        if (FSDIF_SUCCESS != ret)
        {
            FSDIF_ERROR("update clock failed\n");
            return ret;
        }

        clk_rate = instance_p->config.src_clk_rate;
        first_uhs_div = 1 + ((tmp_ext_reg >> 8) & 0xFF);
        div = clk_rate / (2 * first_uhs_div * input_clk_hz);

        if (div > 2)
        {
            sample = div / 2 + 1;
            drv = sample - 1;
            FSDIF_WRITE_REG(base_addr, FSDIF_CLKDIV_OFFSET,
                            (sample << 16) | (drv << 8) | (div & 0xff));
        }
        else if (div == 2)
        {
            drv = 0;
            sample = 1;
            FSDIF_WRITE_REG(base_addr, FSDIF_CLKDIV_OFFSET,
                            (drv << 8) | (sample << 16) | (div & 0xff));
        }

        if (!(FSDIF_READ_REG(base_addr, FSDIF_CLKDIV_OFFSET) & 0xff00) && (is_ddr))
        {
            FSDIF_SET_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_CRC_SERIAL_DATA);
            FSDIF_SET_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_DRV_SHIFT_EN);
        }
        else
        {
            FSDIF_CLR_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_CRC_SERIAL_DATA);
            FSDIF_CLR_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_DRV_SHIFT_EN);
        }

        if (div >= 2)
        {
            FSDIF_WRITE_REG(base_addr, FSDIF_FULL_CLK_DIV_OFFSET, ((2 * (div & 0xff)) & 0xffff));
        }

        FSDIF_INFO("UHS_REG_EXT ext: %x, CLKDIV: %x MCI_CLK_DIVIDER %x %x",
                   FSDIF_READ_REG(base_addr, FSDIF_CLK_SRC_OFFSET),
                   FSDIF_READ_REG(base_addr, FSDIF_CLKDIV_OFFSET),
                   FSDIF_READ_REG(base_addr, FSDIF_FULL_CLK_DIV_OFFSET),
                   ((2 * (div & 0xff)) & 0xffff));

        if (cur_cmd_index == FSDIF_SWITCH_VOLTAGE)
        {
            FDriverMdelay(40); /* sleep 40ms for voltage switch */
        }

        FSdifSetClock(base_addr, TRUE);

        if (cur_cmd_index == FSDIF_SWITCH_VOLTAGE)
        {
            ret = FSdifSendPrivateCmd11(base_addr, mci_cmd_bits | cmd_reg);
        }
        else
        {
            ret = FSdifSendPrivateCmd(base_addr, mci_cmd_bits, 0);
        }

        if (FSDIF_SUCCESS != ret)
        {
            FSDIF_ERROR("update clock failed\n");
            return ret;
        }

        instance_p->cur_clock = input_clk_hz;
    }
    else
    {
        instance_p->cur_clock = 0U;
        FSdifSetClock(base_addr, FALSE);

        if (cur_cmd_index == FSDIF_SWITCH_VOLTAGE)
        {
            ret = FSdifSendPrivateCmd11(base_addr, mci_cmd_bits | cmd_reg);
        }
        else
        {
            ret = FSdifSendPrivateCmd(base_addr, mci_cmd_bits, 0);
        }

        if (FSDIF_SUCCESS != ret)
        {
            FSDIF_ERROR("update clock failed\n");
            return ret;
        }

        FSdifSetClockSrc(base_addr, FALSE);
    }

    return ret;
}

/**
 * @name: FSdifUpdateExternalClk
 * @msg: update uhs clock value and wait clock ready
 * @return {FError}
 * @param {uintptr} base_addr
 * @param {u32} uhs_reg_val
 */
static FError FSdifUpdateExternalClk(uintptr base_addr, u32 uhs_reg_val)
{
    u32 reg_val;
    int retries = FSDIF_TIMEOUT;
    FSDIF_WRITE_REG(base_addr, FSDIF_CLK_SRC_OFFSET, 0U);
    FSDIF_WRITE_REG(base_addr, FSDIF_CLK_SRC_OFFSET, uhs_reg_val);

    do
    {
        reg_val = FSDIF_READ_REG(base_addr, FSDIF_CKSTS_OFFSET);
        if (--retries <= 0)
        {
            break;
        }
    } while (!(reg_val & FSDIF_CLK_READY));

    return (retries <= 0) ? FSDIF_ERR_TIMEOUT : FSDIF_SUCCESS;
}

/**
 * @name: FSdifResetCtrl
 * @msg: Reset fifo/DMA in cntrl register
 * @return {FError} FSDIF_SUCCESS if reset success
 * @param {uintptr} base_addr, base address of SDIF controller
 * @param {u32} reset_bits, bits to be reset
 */
FError FSdifResetCtrl(uintptr base_addr, u32 reset_bits)
{
    u32 reg_val;
    int retries = FSDIF_TIMEOUT;
    FSDIF_SET_BIT(base_addr, FSDIF_CNTRL_OFFSET, reset_bits);

    do
    {
        reg_val = FSDIF_READ_REG(base_addr, FSDIF_CNTRL_OFFSET);
        if (--retries <= 0)
        {
            break;
        }
    } while (reset_bits & reg_val);

    if (retries <= 0)
    {
        FSDIF_ERROR("Reset failed, bits = 0x%x", reset_bits);
        return FSDIF_ERR_TIMEOUT;
    }

    /* update clock after reset */
    if (FSDIF_SUCCESS != FSdifSendPrivateCmd(base_addr, FSDIF_CMD_UPD_CLK, 0U))
    {
        FSDIF_ERROR("Update clock failed !!!");
        return FSDIF_ERR_CMD_TIMEOUT;
    }

    /* for fifo reset, need to check if fifo empty */
    if (reset_bits & FSDIF_CNTRL_FIFO_RESET)
    {
        retries = FSDIF_TIMEOUT;

        do
        {
            reg_val = FSDIF_READ_REG(base_addr, FSDIF_STATUS_OFFSET);
        } while (!(FSDIF_STATUS_FIFO_EMPTY & reg_val));

        if (retries <= 0)
        {
            FSDIF_ERROR("Fifo not empty !!!");
            return FSDIF_ERR_TIMEOUT;
        }
    }

    return FSDIF_SUCCESS;
}

/**
 * @name: FSdifResetBusyCard
 * @msg: reset controller from card busy state
 * @return {FError} FSDIF_SUCCESS if reset success
 * @param {uintptr} base_addr, base address of controller
 */
FError FSdifResetBusyCard(uintptr base_addr)
{
    u32 reg_val;
    int retries = FSDIF_TIMEOUT;
    FSDIF_SET_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_CONTROLLER_RESET);

    do
    {
        FSDIF_SET_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_CONTROLLER_RESET);
        reg_val = FSDIF_READ_REG(base_addr, FSDIF_STATUS_OFFSET);
        if (--retries <= 0)
        {
            break;
        }
    } while (reg_val & FSDIF_STATUS_DATA_BUSY);

    return (retries <= 0) ? FSDIF_ERR_BUSY : FSDIF_SUCCESS;
}

/**
 * @name: FSdifRestartClk
 * @msg: restart controller clock from error status
 * @return {FError} FSDIF_SUCCESS if reset success
 * @param {uintptr} base_addr, base address of controller
 */
FError FSdifRestartClk(uintptr base_addr)
{
    u32 clk_div, uhs;
    int retries = FSDIF_TIMEOUT;
    u32 reg_val;
    FError ret = FSDIF_SUCCESS;

    /* wait command finish if previous command is in error state */
    do
    {
        reg_val = FSDIF_READ_REG(base_addr, FSDIF_CMD_OFFSET);
        if (--retries <= 0)
        {
            break;
        }
    } while (reg_val & FSDIF_CMD_START);

    if (retries <= 0)
    {
        return FSDIF_ERR_TIMEOUT;
    }

    /* update clock */
    FSdifSetClock(base_addr, FALSE);

    clk_div = FSDIF_READ_REG(base_addr, FSDIF_CLKDIV_OFFSET);
    uhs = FSDIF_READ_REG(base_addr, FSDIF_CLK_SRC_OFFSET);

    ret = FSdifUpdateExternalClk(base_addr, uhs);
    if (FSDIF_SUCCESS != ret)
    {
        return ret;
    }

    FSDIF_WRITE_REG(base_addr, FSDIF_CLKDIV_OFFSET, clk_div);

    FSdifSetClock(base_addr, TRUE);

    ret = FSdifSendPrivateCmd(base_addr, FSDIF_CMD_UPD_CLK, 0U);

    return ret;
}

/**
 * @name: FSdifReset
 * @msg: Reset SDIF controller instance
 * @return {FError} FSDIF_SUCCESS if reset success
 * @param {FSdif} *instance_p, SDIF controller instance
 */
static FError FSdifReset(FSdif *const instance_p)
{
    FASSERT(instance_p);
    uintptr base_addr = instance_p->config.base_addr;
    u32 reg_val;
    FError ret = FSDIF_SUCCESS;

    /* set fifo */
    reg_val = FSDIF_FIFOTH(FSDIF_FIFOTH_DMA_TRANS_8, FSDIF_RX_WMARK, FSDIF_TX_WMARK);
    FSDIF_WRITE_REG(base_addr, FSDIF_FIFOTH_OFFSET, reg_val);

    /* set card threshold */
    reg_val = FSDIF_CARD_THRCTL_THRESHOLD(FSDIF_FIFO_DEPTH_8) | FSDIF_CARD_THRCTL_CARDRD;
    FSDIF_WRITE_REG(base_addr, FSDIF_CARD_THRCTL_OFFSET, reg_val);

    /* disable clock and update ext clk */
    FSdifSetClock(base_addr, FALSE);

    /* update ext clk */
    reg_val = FSDIF_UHS_REG(0U, 0U, 0x5U) | FSDIF_UHS_EXT_CLK_ENA;
    FASSERT_MSG(0x502 == reg_val, "invalid uhs config");
    ret = FSdifUpdateExternalClk(base_addr, reg_val);
    if (FSDIF_SUCCESS != ret)
    {
        FSDIF_ERROR("Update extern clock failed !!!");
        return ret;
    }

    /* power on */
    FSdifSetPower(base_addr, TRUE);
    FSdifSetClock(base_addr, TRUE);
    FSdifSetClockSrc(base_addr, TRUE);

    /* set voltage as 3.3v */
    FSdifSetVoltage1_8V(base_addr, FALSE);

    /* set bus width as 1 */
    FSdifSetBusWidth(base_addr, 1);

    /* off DDR mode */
    FSdifSetDDRMode(base_addr, FALSE);
    FSdifSeteMMCDDR(base_addr, FALSE);

    /* reset controller and card */
    ret = FSdifResetCtrl(base_addr, FSDIF_CNTRL_FIFO_RESET | FSDIF_CNTRL_DMA_RESET);
    if (FSDIF_SUCCESS != ret)
    {
        FSDIF_ERROR("Reset controller failed !!!");
        return ret;
    }

    /* reset card for no-removeable media, e.g. eMMC */
    if (TRUE == instance_p->config.non_removable)
    {
        FSDIF_SET_BIT(base_addr, FSDIF_CARD_RESET_OFFSET, FSDIF_CARD_RESET_ENABLE);
    }
    else
    {
        FSDIF_CLR_BIT(base_addr, FSDIF_CARD_RESET_OFFSET, FSDIF_CARD_RESET_ENABLE);
    }

    /* clear interrupt status */
    FSDIF_WRITE_REG(base_addr, FSDIF_INT_MASK_OFFSET, 0U);
    reg_val = FSDIF_READ_REG(base_addr, FSDIF_RAW_INTS_OFFSET);
    FSDIF_WRITE_REG(base_addr, FSDIF_RAW_INTS_OFFSET, reg_val);

    FSDIF_WRITE_REG(base_addr, FSDIF_DMAC_INT_EN_OFFSET, 0U);
    reg_val = FSDIF_READ_REG(base_addr, FSDIF_DMAC_STATUS_OFFSET);
    FSDIF_WRITE_REG(base_addr, FSDIF_DMAC_STATUS_OFFSET, reg_val);

    /* enable card detect interrupt */
    if (FALSE == instance_p->config.non_removable)
    {
        FSDIF_SET_BIT(base_addr, FSDIF_INT_MASK_OFFSET, FSDIF_INT_CD_BIT);
    }

    /* enable controller and internal DMA */
    FSDIF_SET_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_INT_ENABLE | FSDIF_CNTRL_USE_INTERNAL_DMAC);

    /* set data and resp timeout */
    FSDIF_WRITE_REG(base_addr, FSDIF_TMOUT_OFFSET,
                    FSDIF_TIMEOUT_DATA(FSDIF_MAX_DATA_TIMEOUT, FSDIF_MAX_RESP_TIMEOUT));

    /* reset descriptors and dma */
    if (FSDIF_IDMA_TRANS_MODE == instance_p->config.trans_mode)
    {
        FSdifSetDescriptor(base_addr, (uintptr)NULL); /* set decriptor list as NULL */
        FSdifResetIDMA(base_addr);
    }

    FSDIF_INFO("Reset hardware done !!!");
    return ret;
}

/**
 * @name: FSdifRestart
 * @msg: reset controller from error state
 * @return {FError} FSDIF_SUCCESS if restart success
 * @param {FSdif} *instance_p, instance of controller
 */
FError FSdifRestart(FSdif *const instance_p)
{
    FASSERT(instance_p);
    uintptr base_addr = instance_p->config.base_addr;
    FError ret = FSDIF_SUCCESS;
    uint32_t reg_val;

    if (FT_COMPONENT_IS_READY != instance_p->is_ready)
    {
        FSDIF_ERROR("Device is not yet initialized!!!");
        return FSDIF_ERR_NOT_INIT;
    }

    /* set fifo */
    reg_val = FSDIF_FIFOTH(FSDIF_FIFOTH_DMA_TRANS_8, FSDIF_RX_WMARK, FSDIF_TX_WMARK);
    FSDIF_WRITE_REG(base_addr, FSDIF_FIFOTH_OFFSET, reg_val);

    /* set card threshold */
    reg_val = FSDIF_CARD_THRCTL_THRESHOLD(FSDIF_FIFO_DEPTH_8) | FSDIF_CARD_THRCTL_CARDRD;
    FSDIF_WRITE_REG(base_addr, FSDIF_CARD_THRCTL_OFFSET, reg_val);

    /* disable clock and update ext clk */
    FSdifSetClock(base_addr, FALSE);

    /* update ext clk */
    reg_val = FSDIF_UHS_REG(0U, 0U, 0x5U) | FSDIF_UHS_EXT_CLK_ENA;
    FASSERT_MSG(0x502 == reg_val, "invalid uhs config");
    ret = FSdifUpdateExternalClk(base_addr, reg_val);
    if (FSDIF_SUCCESS != ret)
    {
        FSDIF_ERROR("Update extern clock failed !!!");
        return ret;
    }

    /* power on */
    FSdifSetPower(base_addr, TRUE);
    FSdifSetClock(base_addr, TRUE);
    FSdifSetClockSrc(base_addr, TRUE);

    /* reset controller */
    ret = FSdifResetCtrl(base_addr, FSDIF_CNTRL_FIFO_RESET | FSDIF_CNTRL_DMA_RESET);
    if (FSDIF_SUCCESS != ret)
    {
        return ret;
    }

    /* reset card for no-removeable media, e.g. eMMC */
    if (TRUE == instance_p->config.non_removable)
    {
        FSDIF_SET_BIT(base_addr, FSDIF_CARD_RESET_OFFSET, FSDIF_CARD_RESET_ENABLE);
    }
    else
    {
        FSDIF_CLR_BIT(base_addr, FSDIF_CARD_RESET_OFFSET, FSDIF_CARD_RESET_ENABLE);
    }

    /* clear interrupt status */
    FSDIF_WRITE_REG(base_addr, FSDIF_INT_MASK_OFFSET, 0U);
    reg_val = FSDIF_READ_REG(base_addr, FSDIF_RAW_INTS_OFFSET);
    FSDIF_WRITE_REG(base_addr, FSDIF_RAW_INTS_OFFSET, reg_val);

    FSDIF_WRITE_REG(base_addr, FSDIF_DMAC_INT_EN_OFFSET, 0U);
    reg_val = FSDIF_READ_REG(base_addr, FSDIF_DMAC_STATUS_OFFSET);
    FSDIF_WRITE_REG(base_addr, FSDIF_DMAC_STATUS_OFFSET, reg_val);

    /* enable card detect interrupt */
    if (FALSE == instance_p->config.non_removable)
    {
        FSDIF_SET_BIT(base_addr, FSDIF_INT_MASK_OFFSET, FSDIF_INT_CD_BIT);
    }

    /* enable controller and internal DMA */
    FSDIF_SET_BIT(base_addr, FSDIF_CNTRL_OFFSET, FSDIF_CNTRL_INT_ENABLE | FSDIF_CNTRL_USE_INTERNAL_DMAC);

    /* set data and resp timeout */
    FSDIF_WRITE_REG(base_addr, FSDIF_TMOUT_OFFSET,
                    FSDIF_TIMEOUT_DATA(FSDIF_MAX_DATA_TIMEOUT, FSDIF_MAX_RESP_TIMEOUT));

    /* reset descriptors and dma */
    if (FSDIF_IDMA_TRANS_MODE == instance_p->config.trans_mode)
    {
        FSdifSetDescriptor(base_addr, (uintptr)NULL); /* set decriptor list as NULL */
        FSdifResetIDMA(base_addr);
    }

    FSDIF_INFO("Restart hardware done !!!");

    return ret;
}

void FSdifRegisterRelaxHandler(FSdif *const instance_p, FSdifRelaxHandler relax_handler)
{
    FASSERT(instance_p);
    instance_p->relax_handler = relax_handler;
}

/**
 * @name: FSdifCheckCardExists
 * @msg: Check if card is in present
 * @return {boolean} TRUE if card present, FALSE if not present
 * @param {FSdif} *instance_p, SDIF controller instance
 */
boolean FSdifCheckCardExists(FSdif *const instance_p)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    if (!instance_p->config.non_removable)
    {
        return FSdifCheckIfCardExists(instance_p->config.base_addr);
    }

    return TRUE;
}

/**
 * @name: FSdifCheckCardBusy
 * @msg: Check if card is busy transferring data
 * @return {boolean} TRUE if card busy
 * @param {FSdif} *instance_p, SDIF controller instance
 */
boolean FSdifCheckCardBusy(FSdif *const instance_p)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    return FSdifCheckIfCardBusy(instance_p->config.base_addr);
}

/**
 * @name: FSdifSetCardPower
 * @msg: Check card power on/off
 * @return {NONE}
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {boolean} power_on, TRUE if power on, FALSE if power off
 */
void FSdifSetCardPower(FSdif *const instance_p, boolean power_on)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    FSdifSetPower(instance_p->config.base_addr, power_on);
}

/**
 * @name: FSdifSetCardBusWidth
 * @msg: Set card bus width
 * @return {NONE}
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {uint32_t} bus_width, buswidth, e.g. 1, 4, 8
 */
void FSdifSetCardBusWidth(FSdif *const instance_p, uint32_t bus_width)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    FSdifSetBusWidth(instance_p->config.base_addr, bus_width);
}

/**
 * @name: FSdifSetCardDDRMode
 * @msg: Set card DDR/SDR mode
 * @return {NONE}
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {boolean} enable, TRUE if DDR, FALSE if SDR
 */
void FSdifSetCardDDRMode(FSdif *const instance_p, boolean enable)
{
    FASSERT(instance_p);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    FSdifSetDDRMode(instance_p->config.base_addr, enable);
}


/**
 * @name: FSdifPollWaitTransferEnd
 * @msg: Wait DMA/PIO transfer finished by poll
 * @return {FError} FSDIF_SUCCESS if wait success, otherwise wait failed
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {FSdifCmdData} *cmd_data_p, contents of transfer command and data
 */
FError FSdifPollWaitTransferEnd(FSdif *const instance_p, FSdifCmdData *const cmd_data_p)
{
    FASSERT(instance_p);
    FASSERT(cmd_data_p);
    FError ret = FSDIF_SUCCESS;
    u32 int_status;
    u32 dmac_status;
    int delay;
    uintptr base_addr = instance_p->config.base_addr;
    const boolean read = !(cmd_data_p->rawcmd & FSDIF_CMD_DAT_WRITE);
    boolean wait_data = (cmd_data_p->data_p) ? TRUE : FALSE;

    if (FT_COMPONENT_IS_READY != instance_p->is_ready)
    {
        FSDIF_ERROR("Device is not yet initialized!!!");
        return FSDIF_ERR_NOT_INIT;
    }

    /* wait command done or data timeout */
    delay = FSDIF_TIMEOUT;
    do
    {
        int_status = FSdifGetRawStatus(base_addr);
        dmac_status = FSdifGetDMAStatus(base_addr);

        if (delay % 1000 == 0)
        {
            FSDIF_DEBUG("int_status = 0x%x", int_status);
        }

        if ((wait_data) && (int_status & FSDIF_INT_DTO_BIT) && (int_status & FSDIF_INT_CMD_BIT)) /* handle data done */
        {
            FSDIF_DEBUG("Cmd and data over !!!");
            break;
        }
        else if ((!wait_data) && (int_status & FSDIF_INT_CMD_BIT)) /* handle cmd done */
        {
            FSDIF_DEBUG("Cmd done !!!");
            break;
        }
        else if ((int_status & FSDIF_INTS_CMD_ERR_MASK) || (dmac_status & FSDIF_DMAC_ERR_MASK))
        {
            FSDIF_ERROR("Cmd finished with error !!!");
            ret = FSDIF_ERR_INVALID_STATE;
            break;
        }

        if (instance_p->relax_handler)
        {
            instance_p->relax_handler();
        }
    } while (--delay > 0);

    /* clear status to ack data done */
    FSdifClearRawStatus(base_addr);

    if (delay <= 0)
    {
        FSDIF_ERROR("Wait %s done timeout, raw ints: 0x%x.", wait_data ? "data" : "cmd", int_status);
        return FSDIF_ERR_CMD_TIMEOUT;
    }

    if (ret)
    {
        return ret;
    }

    if (NULL != cmd_data_p->data_p) /* wait data transfer done or timeout */
    {
        /* invalidate cache of transfer buffer */
        if (read)
        {
            FSDIF_DATA_BARRIER();
        }
    }

    return ret;
}
