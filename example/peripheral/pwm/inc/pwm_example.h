/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
 * All Rights Reserved.
 *
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
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

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
#if defined(CONFIG_E2000Q_DEMO_BOARD)||defined(CONFIG_E2000D_DEMO_BOARD)
#define PWM_TEST_ID FPWM2_ID
#define PWM_TEST_CHANNEL     FPWM_CHANNEL_1
#elif defined(CONFIG_FIREFLY_DEMO_BOARD)
#define PWM_TEST_ID FPWM4_ID
#define PWM_TEST_CHANNEL     FPWM_CHANNEL_0
#endif
/* pwm test */
BaseType_t FFreeRTOSPwmDeadBandTaskCreate();
BaseType_t FFreeRTOSPwmSingleChannelTaskCreate();
BaseType_t FFreeRTOSPwmDualChannelTaskCreate();

#ifdef __cplusplus
}
#endif

#endif // !