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
 * @Date: 2023-07-28 16:20:15
 * @LastEditTime: 2023-07-28 16:20:15
 * @Description:  This file is for io mux
 * 
 * @Modify History: 
 *  Ver   Who        Date       Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liusm    2023/7/28     init commit
 * 1.1   zhugengyu 2023/10/23   add sd mux
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
    if (FSPI0_ID == spim_id)
    {
        FBOARD_IO_INFO("%d-%d-%d-%d", FIOPadGetFunc(&iopad_ctrl, FIOPAD_W55_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_W53_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_U55_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_U53_REG0_OFFSET));
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_W55_REG0_OFFSET, FIOPAD_FUNC2); /* sclk */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_W53_REG0_OFFSET, FIOPAD_FUNC2); /* txd */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_U55_REG0_OFFSET, FIOPAD_FUNC2); /* rxd */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_U53_REG0_OFFSET, FIOPAD_FUNC2); /* csn0 */
        FBOARD_IO_INFO("%d-%d-%d-%d", FIOPadGetFunc(&iopad_ctrl, FIOPAD_W55_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_W53_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_U55_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_U53_REG0_OFFSET));
    }
    else
    {
        FBOARD_IO_DEBUG("No preconfiguration for SPI-%d iopad mux info.\r\n", spim_id);
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
    if (FGPIO_CTRL_0 == gpio_id)
    {
        switch (pin_id)
        {
            case 0: /* gpio 0-0 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AN59_REG0_OFFSET, FIOPAD_FUNC5);
                break;
            case 5: /* gpio 0-5 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_N43_REG0_OFFSET, FIOPAD_FUNC6);
                break;
            case 14: /* gpio 0-14 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AJ59_REG0_OFFSET, FIOPAD_FUNC5);
                break;
            default:
                FBOARD_IO_DEBUG("No preconfiguration for GPIO-%d pin-%d iopad mux "
                                "info.\r\n",
                                gpio_id, pin_id);
                break;
        }
    }
    else if (FGPIO_CTRL_1 == gpio_id)
    {
        switch (pin_id)
        {
            case 2: /* gpio 1-2 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AE59_REG0_OFFSET, FIOPAD_FUNC5);
                break;
            case 9: /* gpio 1-9 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_E35_REG0_OFFSET, FIOPAD_FUNC6);
                break;
            case 10: /* gpio 1-10 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_G35_REG0_OFFSET, FIOPAD_FUNC6);
                break;
            case 11: /* gpio 1-11 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BA49_REG0_OFFSET, FIOPAD_FUNC5);
                break;
            case 12: /* gpio 1-12 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AW55_REG0_OFFSET, FIOPAD_FUNC5);
                break;
            default:
                FBOARD_IO_DEBUG("No preconfiguration for GPIO-%d pin-%d iopad mux "
                                "info.\r\n",
                                gpio_id, pin_id);
                break;
        }
    }
    else if (FGPIO_CTRL_2 == gpio_id)
    {
        switch (pin_id)
        {
            case 10: /* gpio 2-10 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_A37_REG0_OFFSET, FIOPAD_FUNC6);
                break;
            default:
                FBOARD_IO_DEBUG("No preconfiguration for GPIO-%d pin-%d iopad mux "
                                "info.\r\n",
                                gpio_id, pin_id);
                break;
        }
    }
    else if (FGPIO_CTRL_3 == gpio_id)
    {
        switch (pin_id)
        {
            case 1: /* gpio 3-1 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_C49_REG0_OFFSET, FIOPAD_FUNC6);
                break;
            case 2: /* gpio 3-2 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_A51_REG0_OFFSET, FIOPAD_FUNC6);
                break;
            default:
                FBOARD_IO_DEBUG("No preconfiguration for GPIO-%d pin-%d iopad mux "
                                "info.\r\n",
                                gpio_id, pin_id);
                break;
        }
    }
    else if (FGPIO_CTRL_4 == gpio_id)
    {
        switch (pin_id)
        {

            case 11: /* gpio 4-11 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC49_REG0_OFFSET, FIOPAD_FUNC6);
                break;
            case 12: /* gpio 4-12 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AE47_REG0_OFFSET, FIOPAD_FUNC6);
                break;
            case 13: /* gpio 4-13 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AA47_REG0_OFFSET, FIOPAD_FUNC6);
                break;
            default:
                FBOARD_IO_DEBUG("No preconfiguration for GPIO-%d pin-%d iopad mux "
                                "info.\r\n",
                                gpio_id, pin_id);
                break;
        }
    }
    else
    {
        FBOARD_IO_DEBUG("No preconfiguration for GPIO-%d iopad mux info.\r\n", gpio_id);
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
    if (can_id == FCAN0_ID)
    {
        /* can0 */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_A41_REG0_OFFSET, FIOPAD_FUNC0); /* can0-tx: func 0 */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_A43_REG0_OFFSET, FIOPAD_FUNC0); /* can0-rx: func 0 */
    }
    else if (can_id == FCAN1_ID)
    {
        /* can1 */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_A45_REG0_OFFSET, FIOPAD_FUNC0); /* can1-tx: func 0 */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_C45_REG0_OFFSET, FIOPAD_FUNC0); /* can1-rx: func 0 */
    }
    else
    {
        FBOARD_IO_DEBUG("No preconfiguration for CAN-%d iopad mux info.\r\n", can_id);
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
    else
    {
        FBOARD_IO_DEBUG("No preconfiguration for QSPI-%d iopad mux info.\r\n", qspi_id);
    }

    if (cs_id == FQSPI_CS_0)
    {
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_AR55_REG0_OFFSET, FIOPAD_FUNC0);
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
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AL59_REG0_OFFSET, FIOPAD_FUNC1); /* PWM0_OUT: func 1 */
            }
            if (pwm_channel == 1)
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AJ57_REG0_OFFSET, FIOPAD_FUNC1); /* PWM1_OUT: func 1 */
            }
            break;

        case FPWM1_ID:
            if (pwm_channel == 0)
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AG57_REG0_OFFSET, FIOPAD_FUNC1); /* PWM2_OUT: func 1 */
            }
            if (pwm_channel == 1)
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AC59_REG0_OFFSET, FIOPAD_FUNC1); /* PWM3_OUT: func 1 */
            }
            break;

        default:
            FBOARD_IO_DEBUG("No preconfiguration for PWM-%d iopad mux info.\r\n", pwm_id);
            break;
    }
}

/**
 * @name: FIOPadSetMioMux
 * @msg: set iopad mux for mio
 * @return {*}
 * @param {u32} mio_id, instance id of i2c
 */
void FIOPadSetMioMux(u32 mio_id)
{
    switch (mio_id)
    {
        case FMIO0_ID:
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_A41_REG0_OFFSET, FIOPAD_FUNC5); /* scl */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_A43_REG0_OFFSET, FIOPAD_FUNC5); /* sda */
            }
            break;
        case FMIO1_ID:
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_A45_REG0_OFFSET, FIOPAD_FUNC5); /* scl */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_C45_REG0_OFFSET, FIOPAD_FUNC5); /* sda */
            }
            break;
        case FMIO2_ID:
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_A47_REG0_OFFSET, FIOPAD_FUNC5); /* scl */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_A49_REG0_OFFSET, FIOPAD_FUNC5); /* sda */
            }
            break;
        case FMIO10_ID:
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_C49_REG0_OFFSET, FIOPAD_FUNC5); /* scl */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_A51_REG0_OFFSET, FIOPAD_FUNC5); /* sda */
            }
            break;
        default:
            FBOARD_IO_DEBUG("No preconfiguration for MIO-%d iopad mux info.\r\n", mio_id);
            break;
    }
}

/**
 * @name: FIOPadSetTachoMux
 * @msg: set iopad mux for pwm_in
 * @return {*}
 * @param {u32} pwm_in_id, instance id of tacho
 */
void FIOPadSetTachoMux(u32 pwm_in_id)
{
    switch (pwm_in_id)
    {
        case FTACHO1_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AJ59_REG0_OFFSET, FIOPAD_FUNC1);
            break;
        case FTACHO3_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AE59_REG0_OFFSET, FIOPAD_FUNC1);
            break;
        default:
            FBOARD_IO_DEBUG("No preconfiguration for pwm-in-%d iopad mux info.\r\n", pwm_in_id);
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
    switch (uart_id)
    {
        case FUART1_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AW51_REG0_OFFSET, FIOPAD_FUNC0);
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AU51_REG0_OFFSET, FIOPAD_FUNC0);
            break;
        case FUART2_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_A47_REG0_OFFSET, FIOPAD_FUNC0);
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_A49_REG0_OFFSET, FIOPAD_FUNC0);
            break;
        default:
            FBOARD_IO_DEBUG("No preconfiguration for UART-%d iopad mux info.\r\n", uart_id);
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
    /* set mux for int pin, card_int_n_0 card_int_n_1 */
    switch (sd_id)
    {
        case FSDIF0_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AN49_REG0_OFFSET, FIOPAD_FUNC0);
            break;
        case FSDIF1_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_N45_REG0_OFFSET, FIOPAD_FUNC5);
            break;
    }
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
