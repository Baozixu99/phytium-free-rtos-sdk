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
 * FilePath: fpwm_hw.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-25 11:45:05
 * Description:  This file is for pwm register implementation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/4/15   init commit
 */

#include <stdio.h>
#include "fparameters.h"
#include "fpwm_hw.h"
#include "fassert.h"

/**
 * @name: FPwmLsdEnable
 * @msg:  enable pwm lsd syn
 * @param {uintptr} lsd_addr, base address of the lsd_pwm_syn
 * @param {u8} pwm_id, pwm id parameters of FPWM
 * @return
 */
void FPwmLsdEnable(uintptr lsd_addr, u8 pwm_id)
{
    FASSERT(pwm_id < FPWM_NUM);
    u32 reg_val = 0;

    reg_val = FPWM_READ_REG32(lsd_addr, FLSD_PWM_HADDR);

    reg_val |= BIT(pwm_id);

    FPWM_WRITE_REG32(lsd_addr, FLSD_PWM_HADDR, reg_val);
}

/**
 * @name: FPwmLsdDisable
 * @msg:  disable pwm lsd syn
 * @param {uintptr} lsd_addr, base address of the lsd_pwm_syn
 * @param {u8} pwm_id, pwm id parameters of FPWM
 * @return
 */
void FPwmLsdDisable(uintptr lsd_addr, u8 pwm_id)
{
    FASSERT(pwm_id < FPWM_NUM);
    u32 reg_val = 0;

    reg_val = FPWM_READ_REG32(lsd_addr, FLSD_PWM_HADDR);

    reg_val &= (~BIT(pwm_id));

    FPWM_WRITE_REG32(lsd_addr, FLSD_PWM_HADDR, reg_val);
}


/**
 * @name: FPwmDump
 * @msg:  dump some pwm registers value.
 * @param {FPwmCtrl} *pctrl, instance of FPWM controller
 * @return
 */
void FPwmDump(uintptr base_addr)
{
    uintptr db_base_addr = base_addr;
    uintptr pwm_base_addr = base_addr + FPWM_OFFSET;

    printf("Off[0x%x]: FPWM_DB_CTRL_OFFSET  = 0x%08x\r\n", db_base_addr + FPWM_DB_CTRL_OFFSET,
           FPWM_READ_REG32(db_base_addr, FPWM_DB_CTRL_OFFSET));
    printf("Off[0x%x]: FPWM_DB_DLY_OFFSET   = 0x%08x\r\n", db_base_addr + FPWM_DB_DLY_OFFSET,
           FPWM_READ_REG32(db_base_addr, FPWM_DB_DLY_OFFSET));
    printf("\r\n");
    printf("Off[0x%x]: FPWM_TIM_CNT_OFFSET    = 0x%08x\r\n", pwm_base_addr + FPWM_TIM_CNT_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_TIM_CNT_OFFSET));
    printf("Off[0x%x]: FPWM_TIM_CTRL_OFFSET   = 0x%08x\r\n", pwm_base_addr + FPWM_TIM_CTRL_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_TIM_CTRL_OFFSET));
    printf("Off[0x%x]: FPWM_STATE_OFFSET    = 0x%08x\r\n", pwm_base_addr + FPWM_STATE_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_STATE_OFFSET));
    printf("Off[0x%x]: FPWM_PERIOD_OFFSET = 0x%08x\r\n", pwm_base_addr + FPWM_PERIOD_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_PERIOD_OFFSET));
    printf("Off[0x%x]: FPWM_CTRL_OFFSET = 0x%08x\r\n", pwm_base_addr + FPWM_CTRL_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_CTRL_OFFSET));
    printf("Off[0x%x]: FPWM_CCR_OFFSET  = 0x%08x\r\n", pwm_base_addr + FPWM_CCR_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_CCR_OFFSET));

    pwm_base_addr = pwm_base_addr + FPWM_OFFSET;
    printf("\r\n");
    printf("Off[0x%x]: FPWM_TIM_CNT_OFFSET    = 0x%08x\r\n", pwm_base_addr + FPWM_TIM_CNT_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_TIM_CNT_OFFSET));
    printf("Off[0x%x]: FPWM_TIM_CTRL_OFFSET   = 0x%08x\r\n", pwm_base_addr + FPWM_TIM_CTRL_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_TIM_CTRL_OFFSET));
    printf("Off[0x%x]: FPWM_STATE_OFFSET    = 0x%08x\r\n", pwm_base_addr + FPWM_STATE_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_STATE_OFFSET));
    printf("Off[0x%x]: FPWM_PERIOD_OFFSET = 0x%08x\r\n", pwm_base_addr + FPWM_PERIOD_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_PERIOD_OFFSET));
    printf("Off[0x%x]: FPWM_CTRL_OFFSET = 0x%08x\r\n", pwm_base_addr + FPWM_CTRL_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_CTRL_OFFSET));
    printf("Off[0x%x]: FPWM_CCR_OFFSET  = 0x%08x\r\n", pwm_base_addr + FPWM_CCR_OFFSET,
           FPWM_READ_REG32(pwm_base_addr, FPWM_CCR_OFFSET));

#if defined(FLSD_CONFIG_BASE)
    printf("Off[0x%x]: FPWM_LSD_OFFSET  = 0x%08x\r\n", FLSD_CONFIG_BASE + FLSD_PWM_HADDR,
           FPWM_READ_REG32(FLSD_CONFIG_BASE, FLSD_PWM_HADDR));
#endif
    printf("\r\n");
}
