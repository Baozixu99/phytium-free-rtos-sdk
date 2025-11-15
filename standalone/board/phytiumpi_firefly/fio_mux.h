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
 * @Date: 2023-07-28 16:23:49
 * @LastEditTime: 2023-07-28 16:23:50
 * @Description:  This file is for io-pad function definition
 * 
 * @Modify History: 
 *  Ver   Who       Date       Changes
 * ----- ------  --------      --------------------------------------
 * 1.0   liusm    2023/7/28    init commit
 * 1.1   zhugengyu 2023/10/23   add sd mux
 */

#ifndef FIO_MUX_H
#define FIO_MUX_H

#include "fboard_port.h"
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

void FIOPadSetSpimMux(u32 spim_id);
void FIOPadSetGpioMux(u32 gpio_id, u32 pin_id);
void FIOPadSetCanMux(u32 can_id);
void FIOPadSetQspiMux(u32 qspi_id, u32 cs_id);
void FIOPadSetPwmMux(u32 pwm_id, u32 pwm_channel);
void FIOPadSetMioMux(u32 mio_id);
void FIOPadSetTachoMux(u32 pwm_in_id);
void FIOPadSetUartMux(u32 uart_id);
void FIOPadSetSdMux(u32 sd_id);

_WEAK void FIOMuxInit(void);
_WEAK void FIOMuxDeInit(void);
#ifdef __cplusplus
}
#endif

#endif
