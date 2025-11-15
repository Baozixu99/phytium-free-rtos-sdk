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
 * Date: 2024-06-24 14:53:42
 * LastEditTime: 2024-06-24 08:25:29
 * Description:  This file is for io-pad function
 *
 * Modify History:
 *  Ver   Who        Date       Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangzq    2024/06/24     init commit
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
 * @name: FIOPadSetUartMux
 * @msg: set iopad mux for uart
 * @return {*}
 * @param {u32} uart_id, instance id of uart
 */
void FIOPadSetUartMux(u32 uart_id)
{
    FASSERT(uart_id < FUART_MSG_NUM);
    switch (uart_id)
    {
        case FUART0_MSG_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AE5_REG0_OFFSET, FIOPAD_FUNC0); /*rx*/
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AG5_REG0_OFFSET, FIOPAD_FUNC0); /*tx*/
            break;
        case FUART1_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_J13_REG0_OFFSET, FIOPAD_FUNC0); /*rx*/
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_L13_REG0_OFFSET, FIOPAD_FUNC0); /*tx*/
            break;
        case FUART2_MSG_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_BK24_REG0_OFFSET, FIOPAD_FUNC2); /*rx*/
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_BL25_REG0_OFFSET, FIOPAD_FUNC2); /*tx*/
            break;
        default:
            break;
    }
}

void FIOPadSetSpimMux(u32 spim_id)
{
    if (FSPI0_MSG_ID == spim_id)
    {
        FBOARD_IO_INFO("%d-%d-%d-%d", FIOPadGetFunc(&iopad_ctrl, FIOPAD_N3_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_N1_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_M2_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_K2_REG0_OFFSET));
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_N3_REG0_OFFSET, FIOPAD_FUNC0); /* sclk */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_N1_REG0_OFFSET, FIOPAD_FUNC0); /* txd */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_M2_REG0_OFFSET, FIOPAD_FUNC0); /* rxd */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_K2_REG0_OFFSET, FIOPAD_FUNC0); /* csn0 */
        FBOARD_IO_INFO("%d-%d-%d-%d", FIOPadGetFunc(&iopad_ctrl, FIOPAD_N3_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_N1_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_M2_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_K2_REG0_OFFSET));
    }
    else if (FSPI1_MSG_ID == spim_id)
    {
        FBOARD_IO_INFO("%d-%d-%d-%d", FIOPadGetFunc(&iopad_ctrl, FIOPAD_BK20_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_BL21_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_BK22_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_BL23_REG0_OFFSET));
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_BK20_REG0_OFFSET, FIOPAD_FUNC1); /* sclk */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_BL21_REG0_OFFSET, FIOPAD_FUNC1); /* txd */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_BK22_REG0_OFFSET, FIOPAD_FUNC1); /* rxd */
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_BL23_REG0_OFFSET, FIOPAD_FUNC1); /* csn0 */
        FBOARD_IO_INFO("%d-%d-%d-%d", FIOPadGetFunc(&iopad_ctrl, FIOPAD_BK20_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_BL21_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_BK22_REG0_OFFSET),
                       FIOPadGetFunc(&iopad_ctrl, FIOPAD_BL23_REG0_OFFSET));
    }
    else
    {
        FBOARD_IO_ERROR("spim id %d is not support", spim_id);
    }
}

/*****************************************************************************/
void FIOPadSetQspiMux(u32 qspi_id, u32 cs_id)
{

    if (qspi_id == FQSPI0_ID)
    {
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_A5_REG0_OFFSET, FIOPAD_FUNC0);
    }
    if (cs_id == FQSPI_CS_1)
    {
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_C3_REG0_OFFSET, FIOPAD_FUNC0);
    }
    else if (cs_id == FQSPI_CS_2)
    {
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_B8_REG0_OFFSET, FIOPAD_FUNC1);
    }
    else if (cs_id == FQSPI_CS_3)
    {
        FIOPadSetFunc(&iopad_ctrl, FIOPAD_E1_REG0_OFFSET, FIOPAD_FUNC1);
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
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_V8_REG0_OFFSET, FIOPAD_FUNC1); /* PWM0_OUT: func 1 */
            }
            if (pwm_channel == 1)
            {
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_M8_REG0_OFFSET, FIOPAD_FUNC1); /* PWM1_OUT: func 1 */
            }
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
    if (FGPIO_CTRL_0 == gpio_id)
    {
        switch (pin_id)
        {
            case 0: /* gpio 0-0*/
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_B8_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 1: /* gpio 0-1 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_E1_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 4: /* gpio 0-4 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_A9_REG0_OFFSET, FIOPAD_FUNC4);
                break;
            case 5: /* gpio 0-5 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_C9_REG0_OFFSET, FIOPAD_FUNC4);
                break;
            case 6: /* gpio 0-6 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AD6_REG0_OFFSET, FIOPAD_FUNC4);
                break;
            case 7: /* gpio 0-7 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AK6_REG0_OFFSET, FIOPAD_FUNC4);
                break;
            case 8: /* gpio 0-8 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_Y8_REG0_OFFSET, FIOPAD_FUNC4);
                break;
            case 9: /* gpio 0-9 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AJ5_REG0_OFFSET, FIOPAD_FUNC4);
                break;
            case 11: /* gpio 0-11 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_E7_REG0_OFFSET, FIOPAD_FUNC4);
                break;
            case 13: /* gpio 0-13 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AR5_REG0_OFFSET, FIOPAD_FUNC4);
                break;
            case 14: /* gpio 0-14 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AT6_REG0_OFFSET, FIOPAD_FUNC4);
                break;
            default:
                break;
        }
    }
    else if (FGPIO_CTRL_1 == gpio_id)
    {
        switch (pin_id)
        {
            case 0: /* gpio 1-0 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AE3_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 1: /* gpio 1-1 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AF2_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 2: /* gpio 1-2 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AH2_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 3: /* gpio 1-3 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AJ1_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 4: /* gpio 1-4 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_AK2_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 5: /* gpio 1-5 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BM12_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 6: /* gpio 1-6 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BM10_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 7: /* gpio 1-7 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BK12_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 8: /* gpio 1-8 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BL11_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 9: /* gpio 1-9 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BK20_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 10: /* gpio 1-10 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BL21_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 11: /* gpio 1-11 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BK22_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 12: /* gpio 1-12 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BL23_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 13: /* gpio 1-13 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BK24_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 14: /* gpio 1-14 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_BL25_REG0_OFFSET, FIOPAD_FUNC0);
                break;
            case 15: /* gpio 1-15 */
                FIOPadSetFunc(&iopad_ctrl, FIOPAD_T2_REG0_OFFSET, FIOPAD_FUNC4);
                break;
            default:
                break;
        }
    }
}

void FIOPadSetI2CMux(u32 i2c_id)
{
    FASSERT(i2c_id < FI2C_MSG_NUM);
    switch (i2c_id)
    {
        case FI2C0_MSG_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_A9_REG0_OFFSET, FIOPAD_FUNC1);  /* clk */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_F10_REG0_OFFSET, FIOPAD_FUNC1); /* data */
            break;
        case FI2C1_MSG_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AE3_REG0_OFFSET, FIOPAD_FUNC2); /* clk */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_AF2_REG0_OFFSET, FIOPAD_FUNC2); /* data */
            break;
        case FI2C2_MSG_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_N3_REG0_OFFSET, FIOPAD_FUNC1); /* alert */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_E7_REG0_OFFSET, FIOPAD_FUNC0); /* clk */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_F8_REG0_OFFSET, FIOPAD_FUNC0); /* data */
            break;
        case FI2C3_MSG_ID:
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_BM10_REG0_OFFSET, FIOPAD_FUNC2); /* alert*/
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_BK12_REG0_OFFSET, FIOPAD_FUNC2); /* clk */
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_BL11_REG0_OFFSET, FIOPAD_FUNC2); /* data */
            break;
        default:
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
            FIOPadSetFunc(&iopad_ctrl, FIOPAD_T8_REG0_OFFSET, FIOPAD_FUNC1);
            break;
        default:
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
