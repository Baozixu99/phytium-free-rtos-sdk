/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * @FilePath: fio_mux.c
 * @Date: 2023-08-01 17:07:29
 * @LastEditTime: 2023-08-01 17:07:29
 * @Description:  This file is for 
 * 
 * @Modify History: 
 *  Ver   Who    Date           Changes
 * ----- ------  --------       --------------------------------------
 * 1.0   liusm   2023/08/01      first release
 */
#include "fio_mux.h"
#include "fioctrl_hw.h"
#include "fioctrl.h"
#include "fparameters.h"

#include "fassert.h"
#include "fboard_port.h"

FIOCtrl ioctrl;
#define FBOARD_IO_DEBUG_TAG "FBOARD_IO-CFG"
#define FBOARD_IO_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FBOARD_IO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FBOARD_IO_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FBOARD_IO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FBOARD_IO_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FBOARD_IO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FBOARD_IO_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FBOARD_IO_DEBUG_TAG, format, ##__VA_ARGS__)
/*****************************************************************************/

/**
 * @name: FIOPadSetSpimMux
 * @msg: set iopad mux for spim cs
 * @return {*}
 * @param {u32} spim_id, instance id of spi
 */
void FIOPadSetSpimMux(u32 spim_id)
{
    if (spim_id == FSPI0_ID)
    {
        FIOCtrlSetFunc(&ioctrl, FIOCTRL_SPI0_CSN0_PAD, FIOCTRL_FUNC1); /* work as gpio */
    }
}

void FIOPadSetQspiMux(u32 qspi_id, u32 cs_id)
{
    /* QspiMux has been set in uboot */
}

void FIOPadSetGpioMux(u32 gpio_id, u32 port_id, u32 pin_id)
{
    if (gpio_id == FGPIO_CTRL_1)
    {
        if (port_id == FGPIO_PORT_A)
        {
            switch (pin_id)
            {
                case 3: /* gpio 1-a-3 */
                    FIOCtrlSetFunc(&ioctrl, FIOCTRL_LPC_LAD0_PAD, FIOCTRL_FUNC1);
                    break;
                case 4: /* gpio 1-a-4 */
                    FIOCtrlSetFunc(&ioctrl, FIOCTRL_LPC_LAD1_PAD, FIOCTRL_FUNC1);
                    break;
                case 5: /* gpio 1-a-5 */
                    FIOCtrlSetFunc(&ioctrl, FIOCTRL_SPI0_CSN0_PAD, FIOCTRL_FUNC1);
                    break;
                case 6: /* gpio 1-a-6 */
                    FIOCtrlSetFunc(&ioctrl, FIOCTRL_SPI0_SCK_PAD, FIOCTRL_FUNC1);
                    break;
                case 7: /* gpio 1-a-7 */
                    FIOCtrlSetFunc(&ioctrl, FIOCTRL_SPI0_SO_PAD, FIOCTRL_FUNC1);
                    break;
                default:
                    break;
            }
        }
        else if (port_id == FGPIO_PORT_B)
        {
            switch (pin_id)
            {
                case 0: /* gpio 1-b-0 */
                    FIOCtrlSetFunc(&ioctrl, FIOCTRL_SPI0_SI_PAD, FIOCTRL_FUNC2);
                    break;
                default:
                    break;
            }
        }
    }
}

void FIOPadSetI2cMux(u32 i2c_id)
{
    switch (i2c_id)
    {
        case FI2C0_ID:
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_I2C0_SCL_PAD, FIOCTRL_FUNC0);
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_I2C0_SDA_PAD, FIOCTRL_FUNC0);
            break;
        case FI2C1_ID:
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_ALL_PLL_LOCK_PAD, FIOCTRL_FUNC2);
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_CRU_CLK_OBV_PAD, FIOCTRL_FUNC2);
            break;
        case FI2C2_ID:
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_SWDO_SWJ_PAD, FIOCTRL_FUNC2);
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_TDO_SWJ_IN_PAD, FIOCTRL_FUNC2);
            break;
        case FI2C3_ID:
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_HDT_MB_DONE_STATE_PAD, FIOCTRL_FUNC2);
            FIOCtrlSetFunc(&ioctrl, FIOCTRL_HDT_MB_FAIL_STATE_PAD, FIOCTRL_FUNC2);
            break;
        default:
            FASSERT(0);
            break;
    }
}

void FIOPadSetCanMux(u32 can_id)
{
    if (can_id == FCAN0_ID)
    {
        FIOCtrlSetFunc(&ioctrl, FIOCTRL_TJTAG_TDI_PAD, FIOCTRL_FUNC1); /* can0-tx: func 1 */
        FIOCtrlSetFunc(&ioctrl, FIOCTRL_SWDITMS_SWJ_PAD, FIOCTRL_FUNC1); /* can0-rx: func 1 */
    }
    else if (can_id == FCAN1_ID)
    {
        FIOCtrlSetFunc(&ioctrl, FIOCTRL_NTRST_SWJ_PAD, FIOCTRL_FUNC1); /* can1-tx: func 1 */
        FIOCtrlSetFunc(&ioctrl, FIOCTRL_SWDO_SWJ_PAD, FIOCTRL_FUNC1); /* can1-rx: func 1 */
    }
    else
    {
        FBOARD_IO_DEBUG("No preconfiguration for CAN-%d iopad mux info.\r\n", can_id);
    }
}

_WEAK void FIOMuxInit(void)
{
    FIOCtrlCfgInitialize(&ioctrl, FIOCtrlLookupConfig(FIOCTRL0_ID));
}

_WEAK void FIOMuxDeInit(void)
{
    FIOCtrlDeInitialize(&ioctrl);
}
