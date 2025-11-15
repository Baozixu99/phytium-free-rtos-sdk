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
 * FilePath: fnand_hw.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:56:36
 * Description:  This file contains macros that can be used to access the device.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/05/10    first release
 */

#include "fnand_hw.h"
#include "fnand.h"


void FNandEnable(uintptr_t base_address)
{
    FNAND_SETBIT(base_address, FNAND_CTRL0_OFFSET, FNAND_CTRL0_EN_MASK); /* 使能控制器 */
}


void FNandHwReset(uintptr_t base_address)
{
    FNAND_WRITEREG(base_address, FNAND_INTRMASK_OFFSET, FNAND_INTRMASK_ALL_INT_MASK); /* 屏蔽所有中断位 */
    FNAND_WRITEREG(base_address, FNAND_STATE_OFFSET, FNAND_STATE_ALL_BIT); /* 清空所有状态位 */
    FNAND_WRITEREG(base_address, FNAND_ERROR_CLEAR_OFFSET, FNAND_ERROR_ALL_CLEAR); /* 清除所有错误位 */
    FNAND_WRITEREG(base_address, FNAND_FIFO_FREE_OFFSET, FNAND_FIFO_FREE_MASK); /* 清空fifo */
    // FNAND_CLEARBIT(base_address,FNAND_CTRL0_OFFSET,FNAND_CTRL0_SPARE_SIZE_EN_MASK|FNAND_CTRL0_ECC_EN_MASK ); /* 关闭ECC 位、并且关闭spare size 有效位 */
}


void FNandHwInit(uintptr_t base_address, FNandInterMode inter_mode)
{
    // FNAND_WRITEREG(base_address, FNAND_CTRL1_OFFSET, FNAND_CTRL1_SAMPL_PHASE_MAKE(1UL)); /* 读取数据时的采样速度 */
    FNAND_SETBIT(base_address, FNAND_CTRL1_OFFSET, FNAND_CTRL1_SAMPL_PHASE_MAKE(5UL));
    FNAND_CLEARBIT(base_address, FNAND_CTRL1_OFFSET, FNAND_CTRL1_ECC_DATA_FIRST_EN_MASK);

    // FNAND_SETBIT(base_address, FNAND_CTRL1_OFFSET, FNAND_CTRL1_ECC_DATA_FIRST_EN_MASK); /* 开启先读取ECC 数据，然后读入对应数据  */
    FNAND_WRITEREG(base_address, FNAND_INTERVAL_OFFSET, FNAND_INTERVAL_TIME_MAKE(1UL)); /* 命令 、地址、数据之间的时间间隔 */
    FNAND_WRITEREG(base_address, FNAND_FIFO_LEVEL0_FULL_OFFSET,
                   FNAND_FIFO_LEVEL0_FULL_THRESHOLD_MASK & 4); /* 满阈值配置 1/2 full, default 0 */
    FNAND_WRITEREG(base_address, FNAND_FIFO_LEVEL1_EMPTY_OFFSET,
                   FNAND_FIFO_LEVEL1_EMPTY_THRESHOLD_MASK & 4); /* 空阈值配置 1/2 empty, default 0 */
    FNAND_WRITEREG(base_address, FNAND_FIFO_FREE_OFFSET, FNAND_FIFO_FREE_MASK); /* 清空fifo 操作 */
    FNAND_WRITEREG(base_address, FNAND_ERROR_CLEAR_OFFSET, FNAND_ERROR_CLEAR_DSP_ERR_CLR_MASK); /* 清除所有错误 */
    FNAND_WRITEREG(base_address, FNAND_CTRL0_OFFSET,
                   FNAND_CTRL0_ECC_CORRECT_MAKE(1UL) |
                       FNAND_CTRL0_INTER_MODE((unsigned long)(inter_mode))); /*  纠错为2位 ，使用配置的模式 */
}