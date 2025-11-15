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
 * FilePath: fqspi.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-03-28 09:00:41
 * Description:  This files is for the qspi specific functions implementations
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/3/29  first release
 * 1.1   wangxiaodong  2022/9/9   improve functions
 * 1.2   zhangyan      2022/12/7  improve functions
 */

#include <string.h>
#include "fassert.h"
#include "fqspi.h"
#include "fqspi_hw.h"
#include "fqspi_flash.h"
#include "fdrivers_port.h"

#define FQSPI_DEBUG_TAG "FQSPI"
#define FQSPI_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_WARN(format, ...) FT_DEBUG_PRINT_W(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_INFO(format, ...) FT_DEBUG_PRINT_I(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FQSPI_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FQSPI_DEBUG_TAG, format, ##__VA_ARGS__)


/**
 * @name: FQspiCfgInitialize
 * @msg:  Initializes a specific instance such that it is ready to be used.
 * @param {FQspiCtrl} *pctrl, instance of FQSPI controller
 * @param {FQspiConfig} *input_config_p, Configuration parameters of FQSPI
 * @return err code information, FQSPI_SUCCESS indicates success，others indicates failed
 */
FError FQspiCfgInitialize(FQspiCtrl *pctrl, const FQspiConfig *input_config_p)
{
    FASSERT(pctrl && input_config_p);

    FError ret = FQSPI_SUCCESS;
    /*
    * If the device is started, disallow the initialize and return a Status
    * indicating it is started.  This allows the user to de-initialize the device
    * and reinitialize, but prevents a user from inadvertently
    * initializing.
    */
    if (FT_COMPONENT_IS_READY == pctrl->is_ready)
    {
        FQSPI_WARN("Device is already initialized!!!");
    }

    /*Set default values and configuration data */
    FQspiDeInitialize(pctrl);

    pctrl->config = *input_config_p;

    pctrl->is_ready = FT_COMPONENT_IS_READY;

    return ret;
}

/**
 * @name: FQspiDeInitialize
 * @msg: DeInitialization function for the device instance
 * @param {FQspiCtrl} *pctrl, instance of FQSPI controller
 * @return {*}
 */
void FQspiDeInitialize(FQspiCtrl *pctrl)
{
    FASSERT(pctrl);

    pctrl->is_ready = 0;
    memset(pctrl, 0, sizeof(*pctrl));

    return;
}

/**
 * @name: FQspiSetCapacityAndNum
 * @msg:  Initializes the capacity and number of flash connect to specific instance.
 * @param {FQspiCtrl} *pctrl, instance of FQSPI controller
 * @return void
 */

void FQspiSetCapacityAndNum(FQspiCtrl *pctrl)
{
    FASSERT(pctrl);
    u32 reg_val = 0;
    FQspiConfig *config_p = &pctrl->config;
    uintptr base_addr = pctrl->config.base_addr;

    for (u32 index = 0; index < FQSPI_CS_NUM; index++)
    {
        switch (config_p->capacity[index])
        {
            case FQSPI_FLASH_CAP_4MB:
                pctrl->flash_size[index] = SZ_4M;
                break;
            case FQSPI_FLASH_CAP_8MB:
                pctrl->flash_size[index] = SZ_8M;
                break;
            case FQSPI_FLASH_CAP_16MB:
                pctrl->flash_size[index] = SZ_16M;
                break;
            case FQSPI_FLASH_CAP_32MB:
                pctrl->flash_size[index] = SZ_32M;
                break;
            case FQSPI_FLASH_CAP_64MB:
                pctrl->flash_size[index] = SZ_64M;
                break;
            case FQSPI_FLASH_CAP_128MB:
                pctrl->flash_size[index] = SZ_128M;
                break;
            case FQSPI_FLASH_CAP_256MB:
                pctrl->flash_size[index] = SZ_256M;
                break;
            default:
                pctrl->flash_size[index] = SZ_4M;
                break;
        }

        reg_val |= config_p->capacity[index] << (index * FQSPI_CAP_FLASH_CAP_SHIFT);
    }
    /* Write flash capacity and numbers information to qspi Capacity register */
    reg_val &= ~FQSPI_CAP_FLASH_NUM_MASK;
    reg_val |= (FQSPI_CAP_FLASH_NUM_MASK & FQSPI_CAP_FLASH_NUM(config_p->dev_num));
    /*write value to flash capacity register 0x00 */
    FQSPI_WRITE_REG32(base_addr, FQSPI_REG_CAP_OFFSET, reg_val);
}


/**
 * @name: FQspiRdCfgConfig
 * @msg:  config read config register
 * @param {FQspiCtrl} *pctrl, instance of FQSPI controller
 * @return err code information, FQSPI_SUCCESS indicates success，others indicates failed
 */
FError FQspiRdCfgConfig(FQspiCtrl *pctrl)
{
    FASSERT(pctrl);
    FError ret = FQSPI_SUCCESS;
    u32 cmd_reg = 0;
    uintptr base_addr = pctrl->config.base_addr;

    FQspiRdCfgDef rd_config = pctrl->rd_cfg;

    cmd_reg |= FQSPI_RD_CFG_CMD(rd_config.rd_cmd);
    cmd_reg |= FQSPI_RD_CFG_THROUGH(rd_config.rd_through);
    cmd_reg |= FQSPI_RD_CFG_TRANSFER(rd_config.rd_transfer);
    cmd_reg |= FQSPI_RD_CFG_ADDR_SEL(rd_config.rd_addr_sel);
    cmd_reg |= FQSPI_RD_CFG_LATENCY(rd_config.rd_latency);
    cmd_reg |= FQSPI_RD_CFG_MODE_BYTE(rd_config.mode_byte);

    if ((rd_config.mode_byte) || (rd_config.cmd_sign == 0))
    {
        cmd_reg |= FQSPI_RD_CFG_CMD_SIGN(rd_config.cmd_sign);
    }
    else
    {
        FQSPI_ERROR("rd_cfg mode_byte disable !!!");
        return FQSPI_INVAL_PARAM;
    }

    if ((rd_config.rd_latency == FQSPI_CMD_LATENCY_ENABLE) || (rd_config.dummy == 0))
    {
        rd_config.dummy = rd_config.dummy ? rd_config.dummy : 1;
        cmd_reg |= FQSPI_RD_CFG_DUMMY(rd_config.dummy);
    }
    else
    {
        FQSPI_ERROR("rd_cfg latency disable !!!");
        return FQSPI_INVAL_PARAM;
    }

    cmd_reg |= FQSPI_RD_CFG_D_BUFFER(rd_config.d_buffer);
    cmd_reg |= FQSPI_RD_CFG_SCK_SEL(rd_config.rd_sck_sel);

    FQSPI_WRITE_REG32(base_addr, FQSPI_REG_RD_CFG_OFFSET, cmd_reg);
    return ret;
}

/**
 * @name: FQspiWrCfgConfig
 * @msg:  config write config register
 * @param {FQspiCtrl} *pctrl, instance of FQSPI controller
 * @return err code information, FQSPI_SUCCESS indicates success，others indicates failed
 */
FError FQspiWrCfgConfig(FQspiCtrl *pctrl)
{
    FASSERT(pctrl);
    FError ret = FQSPI_SUCCESS;
    u32 cmd_reg = 0;
    uintptr base_addr = pctrl->config.base_addr;

    FQspiWrCfgDef wr_config = pctrl->wr_cfg;

    cmd_reg |= FQSPI_WR_CFG_CMD(wr_config.wr_cmd);
    cmd_reg |= FQSPI_WR_CFG_WAIT(wr_config.wr_wait);
    cmd_reg |= FQSPI_WR_CFG_THROUGH(wr_config.wr_through);
    cmd_reg |= FQSPI_WR_CFG_TRANSFER(wr_config.wr_transfer);
    cmd_reg |= FQSPI_WR_CFG_ADDRSEL(wr_config.wr_addr_sel);
    cmd_reg |= FQSPI_WR_CFG_MODE(wr_config.wr_mode);
    cmd_reg |= FQSPI_WR_CFG_SCK_SEL(wr_config.wr_sck_sel);

    FQSPI_WRITE_REG32(base_addr, FQSPI_REG_WR_CFG_OFFSET, cmd_reg);

    return ret;
}


/**
 * @name: FQspiCommandPortConfig
 * @msg:  config command port register
 * @param {FQspiCtrl} *pctrl, instance of FQSPI controller
 * @return err code information, FQSPI_SUCCESS indicates success，others indicates failed
 */
FError FQspiCommandPortConfig(FQspiCtrl *pctrl)
{
    FASSERT(pctrl);
    FError ret = FQSPI_SUCCESS;
    u32 cmd_reg = 0;
    uintptr base_addr = pctrl->config.base_addr;

    FQspiCommandPortDef cmd_port_config = pctrl->cmd_def;

    cmd_reg |= FQSPI_CMD_PORT_CMD_MASK & FQSPI_CMD_PORT_CMD(cmd_port_config.cmd);

    cmd_reg |= FQSPI_CMD_PORT_WAIT(cmd_port_config.wait);

    cmd_reg |= FQSPI_CMD_PORT_THROUGH(cmd_port_config.through);

    cmd_reg |= FQSPI_CMD_PORT_CS_MASK & FQSPI_CMD_PORT_CS(cmd_port_config.cs);

    cmd_reg |= FQSPI_CMD_PORT_TRANSFER(cmd_port_config.transfer);

    cmd_reg |= FQSPI_CMD_PORT_CMD_ADDR(cmd_port_config.cmd_addr);

    cmd_reg |= FQSPI_CMD_PORT_LATENCY(cmd_port_config.latency);

    cmd_reg |= FQSPI_CMD_PORT_DATA_TRANS(cmd_port_config.data_transfer);

    cmd_reg |= FQSPI_CMD_PORT_ADDR_SEL(cmd_port_config.addr_sel);

    if ((cmd_port_config.latency == FQSPI_CMD_LATENCY_ENABLE) || (cmd_port_config.dummy == 0))
    {
        cmd_port_config.dummy = cmd_port_config.dummy ? cmd_port_config.dummy : 1;
        cmd_reg |= FQSPI_CMD_PORT_DUMMY(cmd_port_config.dummy);
    }
    else
    {
        FQSPI_ERROR("cmd_port latency disable !!!");
        return FQSPI_INVAL_PARAM;
    }

    cmd_reg |= FQSPI_CMD_PORT_P_BUFFER(cmd_port_config.p_buffer);

    /* read data num */
    cmd_reg |= FQSPI_CMD_PORT_RW_NUM_MASK & FQSPI_CMD_PORT_RW_NUM(cmd_port_config.rw_num);

    cmd_reg |= FQSPI_CMD_PORT_CLK_SEL_MASK & FQSPI_CMD_PORT_CLK_SEL(cmd_port_config.sck_sel);

    FQSPI_WRITE_REG32(base_addr, FQSPI_REG_CMD_PORT_OFFSET, cmd_reg);

    return ret;
}


/**
 * @name: FQspiChannelSet
 * @msg:  config qspi cs num
 * @param {FQspiCtrl} *pctrl, instance of FQSPI controller
 * @param {u32} channel, cs number
 * @return
 */
void FQspiChannelSet(FQspiCtrl *pctrl, u32 channel)
{
    FASSERT(pctrl);
    FASSERT(channel < FQSPI_CS_NUM);
    pctrl->config.channel = channel;
}

/**
 * @name: FQspiCsTimingSet
 * @msg:  config qspi cs timing
 * @param {FQspiCtrl} *pctrl, instance of FQSPI controller
 * @param {FQspiCsTimingCfgDef} cs_timing_cfg, cs timing
 * @return err code information, FQSPI_SUCCESS indicates success，others indicates failed
 */
void FQspiCsTimingSet(FQspiCtrl *pctrl, FQspiCsTimingCfgDef *cs_timing_cfg)
{
    FASSERT(pctrl);
    u32 cmd_reg = 0;
    uintptr base_addr = pctrl->config.base_addr;

    cmd_reg |= FQSPI_FUN_SET_CS_HOLD(cs_timing_cfg->cs_hold);

    cmd_reg |= FQSPI_FUN_SET_CS_SETUP(cs_timing_cfg->cs_setup);

    cmd_reg |= FQSPI_FUN_SET_CS_DELAY(cs_timing_cfg->cs_delay);

    FQSPI_WRITE_REG32(base_addr, FQSPI_REG_CS_TIMING_SET_OFFSET, cmd_reg);
}


/**
 * @name: FQspiCycleSet
 * @msg:  Implements finer-grained integer division from 2 to 256
 * @param {FQspiCtrl} *pctrl, instance of FQSPI controller
 * @param {u8} Frequency division coefficient
 * @param {u8} enable enable or disable cycle set
 * @return
 */
void FQspiCycleSet(FQspiCtrl *pctrl, u8 clk_div, u8 enable)
{
    FASSERT(pctrl);
    uintptr base_addr = pctrl->config.base_addr;
    u32 cycle_plus = clk_div - 1;

    if (enable)
    {
        FQSPI_WRITE_REG32(base_addr, FQSPI_REG_CYCLE_OFFSET,
                          FQSPI_CYCLE_SEL_ENABLE | FQSPI_CYCLE_PLUS(cycle_plus));
    }
    else
    {
        FQSPI_CLEARBIT(base_addr, FQSPI_REG_CYCLE_OFFSET, FQSPI_CYCLE_SEL_ENABLE);
    }
}
