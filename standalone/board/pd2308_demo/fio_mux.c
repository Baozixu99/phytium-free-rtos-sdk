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
 * FilePath: fio_mux.c
 * Date: 2023-07-25 14:53:42
 * LastEditTime: 2023-07-25 08:25:29
 * Description:  This file is for io-pad function
 *
 * Modify History:
 *  Ver   Who        Date       Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong    2023/9/5     init commit
 * 1.1   huangjin   2023/12/22      Added FIOPadSetSdMux
 */

/***************************** Include Files *********************************/
#include "fiopad_hw.h"
#include "fiopad.h"
#include "fio_mux.h"
#include "fparameters.h"
#include "fboard_port.h"
#include "fassert.h"

#define FBOARD_IO_DEBUG_TAG "FBOARD_IO-CFG"
#define FBOARD_IO_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FBOARD_IO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FBOARD_IO_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FBOARD_IO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FBOARD_IO_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FBOARD_IO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FBOARD_IO_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FBOARD_IO_DEBUG_TAG, format, ##__VA_ARGS__)

FIOPadCtrl iopad_ctrl;

/*****************************************************************************/
/**
 * @name: FIOPadSetSpimMux
 * @msg: set iopad mux for spim
 * @return {*}
 * @param {u32} spim_id, instance id of spi
 */
void FIOPadSetSpimMux(u32 spim_id)
{
    FASSERT(spim_id < FSPI_NUM);
    switch (spim_id)
    {
        case FSPI0_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AG13_REG0_OFFSET, FIOPAD_FUNC0); /* sclk */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AG11_REG0_OFFSET, FIOPAD_FUNC0); /* txd */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AL15_REG0_OFFSET, FIOPAD_FUNC0); /* rxd */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AJ17_REG0_OFFSET, FIOPAD_FUNC0); /* csn0 */
            break;

        case FSPI1_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_W3_REG0_OFFSET, FIOPAD_FUNC1);  /* sclk */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC7_REG0_OFFSET, FIOPAD_FUNC1); /* txd */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_W1_REG0_OFFSET, FIOPAD_FUNC1);  /* rxd */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AA5_REG0_OFFSET, FIOPAD_FUNC1); /* csn0 */
            break;

        default:
            break;
    }
}

/**
 * @name: FIOPadSetGpioMux
 * @msg: set iopad mux for gpio
 * @return {*}
 * @param {u32} gpio_id, instance id of gpio
 * @param {u32} pin_id, index of pin
 */
void FIOPadSetGpioMux(u32 gpio_id, u32 pin_id)
{
    FASSERT(gpio_id < FGPIO_NUM);
    FASSERT(pin_id < FGPIO_PIN_NUM);

    if (FGPIO_CTRL_0 == gpio_id)
    {
        switch (pin_id)
        {
            case 8: /* gpio 0-8 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_CE61_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 9: /* gpio 0-9 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC19_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 10: /* gpio 0-10 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AA17_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 11: /* gpio 0-11 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_W13_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 14: /* gpio 0-14 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_CM45_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 15: /* gpio 0-15 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_CK45_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            default:
                break;
        }
    }
}


/**
 * @name: FIOPadSetCanMux
 * @msg: set iopad mux for can
 * @return {*}
 * @param {u32} can_id, instance id of can
 */
void FIOPadSetCanMux(u32 can_id)
{
    FASSERT(can_id < FCAN_NUM);
    switch (can_id)
    {
        case FCAN0_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC15_REG0_OFFSET, FIOPAD_FUNC1); /* tx */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC9_REG0_OFFSET, FIOPAD_FUNC1);  /* rx */
            break;
        case FCAN1_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC11_REG0_OFFSET, FIOPAD_FUNC1); /* tx */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AE13_REG0_OFFSET, FIOPAD_FUNC1); /* rx */
            break;
        default:
            break;
    }
}

/**
 * @name: FIOPadSetQspiMux
 * @msg: set iopad mux for qspi
 * @return {*}
 * @param {u32} qspi_id, id of qspi instance
 * @param {u32} cs_id, id of qspi cs
 */
void FIOPadSetQspiMux(u32 qspi_id, u32 cs_id)
{
    if (qspi_id == FQSPI0_ID)
    {
        /* add sck, io0-io3 iopad multiplex */
    }

    if (cs_id == FQSPI_CS_0)
    {
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_L5_REG0_OFFSET, FIOPAD_FUNC0);
    }
    else if (cs_id == FQSPI_CS_1)
    {
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_J5_REG0_OFFSET, FIOPAD_FUNC0);
    }
    else if (cs_id == FQSPI_CS_2)
    {
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_U11_REG0_OFFSET, FIOPAD_FUNC1);
    }
    else if (cs_id == FQSPI_CS_3)
    {
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_U9_REG0_OFFSET, FIOPAD_FUNC1);
    }
    else
    {
        FBOARD_IO_DEBUG("No preconfiguration for QSPI cs-%d iopad mux info.\r\n", cs_id);
    }
}

/**
 * @name: FIOPadSetPwmMux
 * @msg: set iopad mux for pwm
 * @return {*}
 * @param {u32} pwm_id, id of pwm instance
 * @param {u32} pwm_channel, channel of pwm instance
 */
void FIOPadSetPwmMux(u32 pwm_id, u32 pwm_channel)
{
    FASSERT(pwm_id < FPWM_NUM);
    FASSERT(pwm_channel < FPWM_CHANNEL_NUM);
    switch (pwm_id)
    {
        case FPWM0_ID:
            if (pwm_channel == 0)
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BP63_REG0_OFFSET, FIOPAD_FUNC1); /* PWM0_OUT: func 1 */
            }
            if (pwm_channel == 1)
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_CK47_REG0_OFFSET, FIOPAD_FUNC1); /* PWM1_OUT: func 1 */
            }
            break;

        case FPWM1_ID:
            if (pwm_channel == 0)
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC9_REG0_OFFSET, FIOPAD_FUNC3); /* PWM2_OUT: func 1 */
            }
            if (pwm_channel == 1)
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AE13_REG0_OFFSET, FIOPAD_FUNC3); /* PWM3_OUT: func 1 */
            }
            break;

        default:
            FBOARD_IO_DEBUG("No preconfiguration for PWM-%d iopad mux info.\r\n", pwm_id);
            break;
    }
}

/**
 * @name: FIOPadSetTachoMux
 * @msg: set iopad mux for pwm_in
 * @return {*}
 * @param {u32} tacho_id, instance id of tacho
 */
void FIOPadSetTachoMux(u32 tacho_id)
{
    FASSERT(tacho_id < FTACHO_NUM);
    switch (tacho_id)
    {
        case FTACHO0_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_BM63_REG0_OFFSET, FIOPAD_FUNC1);
            break;
        case FTACHO1_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_CM47_REG0_OFFSET, FIOPAD_FUNC1);
            break;
        case FTACHO2_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC15_REG0_OFFSET, FIOPAD_FUNC3);
            break;
        case FTACHO3_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC11_REG0_OFFSET, FIOPAD_FUNC3);
            break;

        default:
            break;
    }
}

void FIOPadSetI2CMux(u32 i2c_id)
{
    FASSERT(i2c_id < FI2C_NUM);
    switch (i2c_id)
    {
        case FI2C0_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AN17_REG0_OFFSET, FIOPAD_FUNC0); /* clk */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AR17_REG0_OFFSET, FIOPAD_FUNC0); /* data */
            break;
        case FI2C1_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AR15_REG0_OFFSET, FIOPAD_FUNC0); /* clk */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AN15_REG0_OFFSET, FIOPAD_FUNC0); /* data */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_W11_REG0_OFFSET, FIOPAD_FUNC1); /* alert */
            break;
        case FI2C2_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AG15_REG0_OFFSET, FIOPAD_FUNC1); /* clk */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AE21_REG0_OFFSET, FIOPAD_FUNC1); /* data */
            break;
        default:
            break;
    }
}

/**
 * @name: FIOPadSetUartMux
 * @msg: set iopad mux for uart
 * @return {*}
 * @param {u32} uart_id, instance id of uart
 */
void FIOPadSetUartMux(u32 uart_id)
{
    FASSERT(uart_id < FUART_NUM);
    switch (uart_id)
    {
        case FUART0_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AA1_REG0_OFFSET, FIOPAD_FUNC0); /*rx*/
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC1_REG0_OFFSET, FIOPAD_FUNC0); /*tx*/
            break;
        case FUART1_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_CB63_REG0_OFFSET, FIOPAD_FUNC0); /*rx*/
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_CD63_REG0_OFFSET, FIOPAD_FUNC0); /*tx*/
            break;
        case FUART2_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_BM63_REG0_OFFSET, FIOPAD_FUNC0); /*rx*/
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_BP63_REG0_OFFSET, FIOPAD_FUNC0); /*tx*/
            break;
        case FUART3_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_CM47_REG0_OFFSET, FIOPAD_FUNC0); /*rx*/
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_CK47_REG0_OFFSET, FIOPAD_FUNC0); /*tx*/
            break;
        default:
            break;
    }
}

/**
 * @name: FIOPadSetSdMux
 * @msg: set iopad mux for sd
 * @return {*}
 */
void FIOPadSetSdMux(u32 sd_id)
{
    /* set mux for int pin, card_int_n_0 */
    switch (sd_id)
    {
        case FSDIF0_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AA19_REG0_OFFSET, FIOPAD_FUNC1);
            break;
        default:
            break;
    }
}

/**
 * @name: FIOPadSetI2sMux
 * @msg: set iopad mux for i2s
 * @return {*}
 */
void FIOPadSetI2sMux(void)
{
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_AN17_REG0_OFFSET, FIOPAD_FUNC0); /* clk */
    FIOPadSetFunc(&iopad_ctrl, FIOPAD_AR17_REG0_OFFSET, FIOPAD_FUNC0); /* data */
}

/**
 * @name: FIOMuxInit
 * @msg: init io mux
 * @return {void}
 * @note: 
 */
_WEAK void FIOMuxInit(void)
{
    FIOPadCfgInitialize(&iopad_ctrl, FIOPadLookupConfig(FIOPAD0_ID));

    return;
}

/**
 * @name: FIOMuxDeInit
 * @msg: deinit io mux
 * @return {void}
 * @note: 
 */
_WEAK void FIOMuxDeInit(void)
{
    FIOPadDeInitialize(&iopad_ctrl);
}
