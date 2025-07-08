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
 * FilePath: fpwm_os.h
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 16:59:58
 * Description:  This file is for providing function related definitions of pwm driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2022/08/26  first commit
 */

#ifndef FPWM_OS_H
#define FPWM_OS_H

#include <FreeRTOS.h>
#include <semphr.h>
#include "ferror_code.h"
#include "fpwm.h"
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* freertos pwm error */
#define FREERTOS_PWM_SEM_ERROR    FT_CODE_ERR(ErrModBsp, ErrBspPwm, 10)

/* freertos pwm interrupt priority */
#define FREERTOS_PWM_IRQ_PRIORITY   IRQ_PRIORITY_VALUE_12

#define FREERTOS_PWM_CTRL_ENABLE        (1) /* enable pwm channel */
#define FREERTOS_PWM_CTRL_DISABLE       (2) /* disable pwm channel */
#define FREERTOS_PWM_CTRL_SET           (3) /* set pwm channel configuration */
#define FREERTOS_PWM_CTRL_GET           (4) /* get pwm channel configuration */
#define FREERTOS_PWM_CTRL_DB_SET        (5) /* enable pwm dead band function */
#define FREERTOS_PWM_CTRL_DB_GET        (6) /* disable pwm dead band function */
#define FREERTOS_PWM_CTRL_PULSE_SET     (7) /* set pwm pulse */


typedef struct
{
    u32 channel; /* 0-1 */
    FPwmVariableConfig pwm_cfg;
    FPwmDbVariableConfig db_cfg;
} FFreeRTOSPwmConfig;

typedef struct
{
    FPwmCtrl pwm_ctrl; /* pwm object */
    xSemaphoreHandle pwm_semaphore; /*!< pwm semaphore for resource sharing */
} FFreeRTOSPwm;

/* init freertos pwm instance */
FFreeRTOSPwm *FFreeRTOSPwmInit(u32 instance_id);

/* deinit freertos pwm instance */
FError FFreeRTOSPwmDeinit(FFreeRTOSPwm *os_pwm_p);

/* pwm channel enable or disable */
FError FFreeRTOSPwmEnable(FFreeRTOSPwm *os_pwm_p, u8 channel, boolean state);

/* set pwm channel config */
FError FFreeRTOSPwmSet(FFreeRTOSPwm *os_pwm_p, u8 channel, FPwmVariableConfig *pwm_cfg_p);

/* get pwm channel config */
FError FFreeRTOSPwmGet(FFreeRTOSPwm *os_pwm_p, u8 channel, FPwmVariableConfig *pwm_cfg_p);

/* set pwm db config */
FError FFreeRTOSPwmDbSet(FFreeRTOSPwm *os_pwm_p, FPwmDbVariableConfig *db_cfg_p);

/* get pwm db config */
FError FFreeRTOSPwmDbGet(FFreeRTOSPwm *os_pwm_p, FPwmDbVariableConfig *db_cfg_p);

/* set pwm channel pulse */
FError FFreeRTOSPwmPulseSet(FFreeRTOSPwm *os_pwm_p, u8 channel, u16 pulse);

#ifdef __cplusplus
}
#endif

#endif // !
