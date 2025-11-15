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
 * @FilePath: fio_mux.h
 * @Date: 2024-06-25 11:29:44
 * @LastEditTime: 2024-06-25 11:29:44
 * @Description:  This file is for io-pad function definition
 * 
 * @Modify History: 
 *  Ver   Who       Date       Changes
 * ----- ------  --------      --------------------------------------
 * 1.0   wangzq    2024/06/25    init commit
 */

#ifndef FIO_MUX_H
#define FIO_MUX_H

#include "fboard_port.h"
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

void FIOPadSetQspiMux(u32 qspi_id, u32 cs_id);
void FIOPadSetUartMux(u32 uart_id);
void FIOPadSetSpimMux(u32 spim_id);
void FIOPadSetI2CMux(u32 i2c_id);
void FIOPadSetGpioMux(u32 gpio_id, u32 pin_id);
void FIOPadSetTachoMux(u32 tacho_id);
void FIOPadSetPwmMux(u32 pwm_id, u32 pwm_channel);
_WEAK void FIOMuxInit(void);
_WEAK void FIOMuxDeInit(void);

#ifdef __cplusplus
}
#endif

#endif
