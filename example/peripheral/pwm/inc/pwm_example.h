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
 * FilePath: pwm_example.h
 * Date: 2022-08-16 14:55:40
 * LastEditTime: 2022-08-19 11:42:40
 * Description:  This file is for pwm test example function declarations.
 *
 * Modify History:
 *  Ver   Who           Date           Changes
 * ----- ------       --------      --------------------------------------
 * 1.0  wangxiaodong  2022/8/24      first release
 */


#ifndef PWM_EXAMPLE_H
#define PWM_EXAMPLE_H
#include "FreeRTOS.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
#if defined(CONFIG_PHYTIUMPI_FIREFLY_BOARD)
#define PWM_TEST_ID FPWM1_ID
#define PWM_TEST_CHANNEL     FPWM_CHANNEL_0
#elif defined(CONFIG_PD2308_DEMO_BOARD) || defined(CONFIG_PD2408_TEST_A_BOARD) || defined(CONFIG_PD2408_TEST_B_BOARD)
#define PWM_TEST_ID FPWM0_ID
#define PWM_TEST_CHANNEL     FPWM_CHANNEL_0
#else
#error "Please select a valid board"
#endif

/* pwm test */
BaseType_t FFreeRTOSPwmDeadBandTaskCreate();
BaseType_t FFreeRTOSPwmSingleChannelTaskCreate();
BaseType_t FFreeRTOSPwmDualChannelTaskCreate();

#ifdef __cplusplus
}
#endif

#endif // !