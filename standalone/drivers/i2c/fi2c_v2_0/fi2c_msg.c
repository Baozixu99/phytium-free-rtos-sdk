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
 * FilePath: fi2c_msg_.c
 * Date: 2024-10-1 14:53:42
 * LastEditTime: 2024-10-10 08:36:14
 * Description:  This file is for I2C msg configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangzq     2024/10/1  first commit
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "fio.h"
#include "ferror_code.h"
#include "ftypes.h"
#include "fdrivers_port.h"
#include "fi2c_msg.h"
#include "fi2c_msg_common.h"
#include "fi2c_msg_hw.h"
#include "fassert.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FI2C_MSG_DEBUG_TAG "I2C-MSG"
#define FI2C_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI2C_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FI2C_MSG_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
// clang-format off
static const FI2cMsgSpeedModeInfo i2c_msg_speed_cfg[FI2C_SPEED_MODE_MAX] = {
    [FI2C_STANDARD_SPEED] =
    {
        4000,
        4700,
        1000,
        300,
    },
    [FI2C_FAST_SPEED] =
    {
        600,
        1300,
        300,
        300,
    },
    [FI2C_HIGH_SPEED] = {
        260,
        500,
        60,
        160,
    }
}; /* i2c 速率配置时序参数，此参数为i2c 协议建议值，具体配置参数可调 */
// clang-format on
static u32 FI2cMsgSclHcnt(u32 ic_clk, u32 tSYMBOL, u32 tf, int cond, int offset)
{
    if (cond)
    {
        return DIV_ROUND_UP(ic_clk / 1000 * tSYMBOL, NANO_TO_KILO) - 8 + offset;
    }
    else
    {
        return DIV_ROUND_UP(ic_clk / 1000 * (tSYMBOL + tf), NANO_TO_KILO) - 3 + offset;
    }
}

static u32 FI2cMsgSclLcnt(u32 ic_clk, u32 tLOW, u32 tf, int offset)
{
    return DIV_ROUND_UP(ic_clk / 1000 * (tLOW + tf), NANO_TO_KILO) - 1 + offset;
}

void FI2cMsgSetBusSpeed(FI2cMsgCtrl *instance_p, u32 speed_mode, boolean auto_calc)
{
    FI2cMsgBusSpeedInfo config;
    u32 sda_falling_time = 300;
    u32 scl_falling_time = 300;

    config.speed_mode = speed_mode;
    config.scl_hcnt = FI2cMsgSclHcnt(instance_p->clk_clock_frequency,
                                     i2c_msg_speed_cfg[speed_mode].min_scl_hightime_ns,
                                     sda_falling_time, 0, 0);
    config.scl_lcnt = FI2cMsgSclLcnt(instance_p->clk_clock_frequency,
                                     i2c_msg_speed_cfg[speed_mode].min_scl_lowtime_ns,
                                     scl_falling_time, 0);
    config.sda_hold = 3;

    FI2cMsgInfo i2c_mng_msg;
    memset(&i2c_mng_msg, 0, sizeof(i2c_mng_msg));

    i2c_mng_msg.cmd_type = FMSG_CMD_SET;
    i2c_mng_msg.cmd_subid = FI2C_MSG_CMD_SET_SPEED;
    config.calc_en = auto_calc;

    FI2cMsgCommonSetCmd(instance_p, &i2c_mng_msg, FMSG_CMD_SET, FI2C_MSG_CMD_SET_SPEED);
    FI2cMsgMemcpy(&i2c_mng_msg.para[0], &config, sizeof(FI2cMsgBusSpeedInfo));
    instance_p->total_shmem_len = FI2C_MSG_CMDDATA_SIZE;
    FI2cMsgVirtSendMsg(instance_p, &i2c_mng_msg, TRUE);
}

/**
 * @name: FI2cDeInitialize
 * @msg: 完成I2C驱动实例去使能，清零实例数据
 * @return {*}
 * @param {FI2c} *instance_p
 */
void FI2cMsgDeInitialize(FI2cMsgCtrl *instance_p)
{
    FASSERT(instance_p);
    instance_p->is_ready = 0;

    memset(instance_p, 0, sizeof(*instance_p));
}

/**
 * @name: FI2cMsgCfgInitialize
 * @msg: 完成I2C驱动实例的初始化，使之可以使用
 * @param {FI2cMsgCtrl} *instance_p I2C驱动实例数据
 * @param {FI2cMsgConfig} *cofig_p I2C驱动配置数据
 * @return SUCCESS if initialization was successful
 *         
 */
FError FI2cMsgCfgInitialize(FI2cMsgCtrl *instance_p, const FI2cMsgConfig *input_config_p)
{
    FASSERT(instance_p && input_config_p);

    FError ret = FI2C_MSG_SUCCESS;
    FDevMsg *msg = &(instance_p->config.msg);

    /*
     * If the device is started, disallow the initialize and return a Status
     * indicating it is started.  This allows the user to de-initialize the device
     * and reinitialize, but prevents a user from inadvertently
     * initializing.
     */
    if (FT_COMPONENT_IS_READY == instance_p->is_ready)
    {
        FI2C_MSG_ERROR("Device is already initialized!!!");
        return FI2C_MSG_NOT_INIT;
    }

    /*
     * Set default values and configuration data, including setting the
     * callback handlers to stubs  so the system will not crash should the
     * application not assign its own callbacks.
     */
    FI2cMsgDeInitialize(instance_p);
    if (&instance_p->config != input_config_p)
    {
        instance_p->config = *input_config_p;
    }

    instance_p->mng.tx_ring_cnt = (FI2C_MSG_READ_REG32(msg->regfile, FMSG_MEM_RING_SIZE) &
                                   FI2C_MSG_REGFILE_TX_RING_MASK) >>
                                  FI2C_MSG_REGFILE_TX_RING_OFFSET;
    if (!instance_p->mng.tx_ring_cnt || (instance_p->mng.tx_ring_cnt > FI2C_MSG_MAX_RING))
    {
        FI2C_MSG_ERROR("failed set tx ring cnt:%d\n", instance_p->mng.tx_ring_cnt);
        return FI2C_MSG_PARA_ERR;
    }

    instance_p->tx_shmem_addr = (void *)msg->shmem;
    instance_p->rx_shmem_addr = (void *)msg->shmem + instance_p->mng.tx_ring_cnt * sizeof(FI2cMsgInfo);
    instance_p->i2c_msg = (FI2cMsgInfo *)instance_p->tx_shmem_addr;

    FI2C_MSG_INFO("tx_shmem: 0x%x,rx_shmem: 0x%x, tx_ring_cnt: %d", instance_p->tx_shmem_addr,
                  instance_p->rx_shmem_addr, instance_p->mng.tx_ring_cnt);
    instance_p->is_ready = FT_COMPONENT_IS_READY;
    return ret;
}
