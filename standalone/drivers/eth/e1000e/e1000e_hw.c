/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: e1000e_hw.c
 * Date: 2025-01-02 14:46:52
 * LastEditTime: 2025-01-02 14:46:58
 * Description:  This file is for manipulation of hardware registers .
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2025/01/02    first release
 */

#include "fassert.h"
#include "fdrivers_port.h"
#include "e1000e.h"
#include "e1000e_hw.h"
#include "e1000e_phy.h"
#include "faarch.h"

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FE1000E_DEBUG_TAG "FE1000E-HW"
#define FE1000E_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FE1000E_DEBUG_TAG, format, ##__VA_ARGS__)

static const char *speed_info[] = {"10", "100", "1000", "1000"};

/************************** Function Prototypes ******************************/

/**
 * @name: FE1000ESoftwareReset
 * @msg: 触发FE1000E控制器软件复位
 * @return {*}
 * @param {uintptr} uintptr, e1000e控制器地址
 * @param {int} timeout 等待复位完成的状态检测周期数目
 */
FError FE1000ESoftwareReset(uintptr addr)
{
    uintptr base_addr = addr;
    uint32_t reg;
    uint32_t val;

    /* 全局重置 */
    FE1000E_WRITEREG32(base_addr, E1000_CTRL, CTRL_RST);
    FDriverUdelay(20000);

    /* CTRL */
    reg = FE1000E_READREG32(base_addr, E1000_CTRL);
    reg &= ~(CTRL_LRST | CTRL_FRCSPD);
    reg |= CTRL_SLU;
    FE1000E_WRITEREG32(base_addr, E1000_CTRL, reg);

    /* power up again in case the previous user turned it off */
    FE1000EPhyWrite(base_addr, PHY_CTRL, FE1000EPhyRead(base_addr, PHY_CTRL) & ~PHYC_POWER_DOWN);

    FE1000E_INFO("Waiting for link...");
    while (!(FE1000E_READREG32(base_addr, E1000_STATUS) & STATUS_LU))
    {
        BARRIER();
    }
    FE1000E_INFO(" ok");

    /* 读取 phy ，确定link的速度 */
    val = FE1000E_READREG32(base_addr, E1000_STATUS) & STATUS_SPEED;
    val >>= STATUS_SPEEDSHFT;
    FE1000E_INFO("Link speed: %s Mb/s", speed_info[val]);

    return FE1000E_SUCCESS;
}
