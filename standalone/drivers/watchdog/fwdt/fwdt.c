/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fwdt.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-07-15 17:05:09
 * Description:  This file is for wdt ctrl function implementation.
 * Users can operate as a single stage watchdog or a two stages watchdog.
 * In the single stage mode, when the timeout is reached, your system will
 * be reset by WS1. The first signal (WS0) is ignored.
 * In the two stages mode, when the timeout is reached, the first signal (WS0)
 * will trigger panic. If the system is getting into trouble and cannot be reset
 * by panic or restart properly by the kdump kernel(if supported), then the
 * second stage (as long as the first stage) will be reached, system will be
 * reset by WS1. This function can help administrator to backup the system
 * context info by panic console output or kdump.
 *
 * GWDT:
 * two stages mode:
 * |--------WOR-------WS0--------WOR-------WS1
 * |----timeout-----(panic)----timeout-----reset
 *
 * single stage mode:
 * |------WOR-----WS0(ignored)-----WOR------WS1
 * |--------------timeout-------------------reset
 *
 * Note: Since this watchdog timer has two stages, and each stage is determined
 * by WOR, in the single stage mode, the timeout is (WOR * 2); in the two
 * stages mode, the timeout is WOR.
 * This driver use two stages mode, when WS0=1, it can Raise the timeout interrupt.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   Wangxiaodong   2021/8/25   init
 * 1.1   Wangxiaodong   2021/11/5   restruct
 * 1.2   Wangxiaodong   2022/7/20   add some functions
 */

#include <string.h>
#include "fparameters.h"
#include "ftypes.h"
#include "ferror_code.h"
#include "fdrivers_port.h"
#include "fwdt.h"
#include "fwdt_hw.h"

#define FWDT_DEBUG_TAG          "FWDT"
#define FWDT_ERROR(format, ...) FT_DEBUG_PRINT_E(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_WARN(format, ...)  FT_DEBUG_PRINT_W(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_INFO(format, ...)  FT_DEBUG_PRINT_I(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)
#define FWDT_DEBUG(format, ...) FT_DEBUG_PRINT_D(FWDT_DEBUG_TAG, format, ##__VA_ARGS__)

/**
 * @name: FWdtCfgInitialize
 * @msg:  Initializes a specific instance such that it is ready to be used.
 * @param {FWdtCtrl} *pctrl, instance of FWDT controller
 * @param {FWdtConfig} *input_config_p, Configuration parameters of FWdt
 * @return err code information, FWDT_SUCCESS indicates success，others indicates failed
 */
FError FWdtCfgInitialize(FWdtCtrl *pctrl, const FWdtConfig *input_config_p)
{
    FASSERT(pctrl && input_config_p);

    FError ret = FWDT_SUCCESS;
    /*
    * If the device is started, disallow the initialize and return a Status
    * indicating it is started.  This allows the user to de-initialize the device
    * and reinitialize, but prevents a user from inadvertently
    * initializing.
    */
    if (FT_COMPONENT_IS_READY == pctrl->is_ready)
    {
        FWDT_WARN("Device is already initialized!!!");
    }

    /*Set default values and configuration data */
    FWdtDeInitialize(pctrl);

    pctrl->config = *input_config_p;

    pctrl->is_ready = FT_COMPONENT_IS_READY;

    return ret;
}

/**
 * @name: FWdtDeInitialize
 * @msg: DeInitialization function for the device instance
 * @param {FWdtCtrl} *pctrl, instance of FWDT controller
 * @return {*}
 */
void FWdtDeInitialize(FWdtCtrl *pctrl)
{
    FASSERT(pctrl);

    pctrl->is_ready = 0;
    memset(pctrl, 0, sizeof(*pctrl));

    return;
}

/**
 * @name: FWdtSetTimeout
 * @msg: Set Timeout Value, the first time it will raise a signal, which is typically
 *  wired to an interrupt; If this watchdog remains un-refreshed, it will raise a
 *  second signal which can be used to interrupt higher-privileged software
 *  or cause a PE reset.
 * @param {WdtCtrl} *pctrl, instance of FWDT controller.
 * @param {u32} timeout, represent in generic timer tick.
 * @return {FError} err code information, FWDT_SUCCESS indicates success, others indicates failed.
 */
FError FWdtSetTimeout(FWdtCtrl *pctrl, u32 timeout)
{
    FASSERT(pctrl != NULL);
    if (pctrl->is_ready != FT_COMPONENT_IS_READY)
    {
        FWDT_ERROR("Device is not ready!!!");
        return FWDT_NOT_READY;
    }

    uintptr base_addr = pctrl->config.control_base_addr;
    FWDT_WRITE_REG32(base_addr, FWDT_GWDT_WOR, timeout);

    return FWDT_SUCCESS;
}

/**
 * @name: FWdtRefresh
 * @msg: Refresh watchdog
 * @param {WdtCtrl} *pctrl, instance of FWDT controller.
 * @return {FError} err code information, FWDT_SUCCESS indicates success, others indicates failed.
 */
FError FWdtRefresh(FWdtCtrl *pctrl)
{
    FASSERT(pctrl != NULL);
    if (pctrl->is_ready != FT_COMPONENT_IS_READY)
    {
        FWDT_ERROR("Device is not ready!!!");
        return FWDT_NOT_READY;
    }
    uintptr base_addr = pctrl->config.refresh_base_addr;
    FWDT_WRITE_REG32(base_addr, FWDT_GWDT_WRR, 0);
    return FWDT_SUCCESS;
}

/**
 * @name: FWdtStart
 * @msg: Start watchdog
 * @param {WdtCtrl} *pctrl, instance of FWDT controller
 * @return {FError} err code information, FWDT_SUCCESS indicates success, others indicates failed.
 */
FError FWdtStart(FWdtCtrl *pctrl)
{
    FASSERT(pctrl != NULL);
    if (pctrl->is_ready != FT_COMPONENT_IS_READY)
    {
        FWDT_ERROR("Device is not ready!!!");
        return FWDT_NOT_READY;
    }

    uintptr base_addr = pctrl->config.control_base_addr;
    FWDT_WRITE_REG32(base_addr, FWDT_GWDT_WCS, FWDT_GWDT_WCS_WDT_EN);

    return FWDT_SUCCESS;
}

/**
 * @name: FWdtStop
 * @msg: Stop watchdog
 * @param {WdtCtrl} *pctrl, instance of FWDT controller
 * @return {FError} err code information, FWDT_SUCCESS indicates success, others indicates failed.
 */
FError FWdtStop(FWdtCtrl *pctrl)
{
    FASSERT(pctrl != NULL);
    uintptr base_addr = pctrl->config.control_base_addr;
    FWDT_WRITE_REG32(base_addr, FWDT_GWDT_WCS, 0);
    return FWDT_SUCCESS;
}

/**
 * @name: FWdtReadFWdtReadWIIDR
 * @msg:  Read wdt iidr register value.
 * @param {FWdtCtrl} *pctrl, instance of FWDT controller
 * @param {FWdtIdentifier} *wdt_identify, wdt identifier struct.
 * @return {FError} err code information, FWDT_SUCCESS indicates success, others indicates failed.
 */
FError FWdtReadFWdtReadWIIDR(FWdtCtrl *pctrl, FWdtIdentifier *wdt_identify)
{
    FASSERT(pctrl != NULL);
    FASSERT(wdt_identify != NULL);

    if (pctrl->is_ready != FT_COMPONENT_IS_READY)
    {
        FWDT_ERROR("Device is not ready!!!");
        return FWDT_NOT_READY;
    }

    u32 reg_val = 0;
    uintptr base_addr = pctrl->config.refresh_base_addr;
    reg_val = FWDT_READ_REG32(base_addr, FWDT_GWDT_W_IIR);

    wdt_identify->version = (u16)((reg_val & FWDT_VERSION_MASK) >> 16);
    wdt_identify->continuation_code = (u8)((reg_val & FWDT_CONTINUATION_CODE_MASK) >> 8);
    wdt_identify->identity_code = (u8)((reg_val & FWDT_IDENTIFY_CODE_MASK));

    return FWDT_SUCCESS;
}