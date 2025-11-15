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
 * FilePath: fmhu_intr.c
 * Date: 2022-12-29 16:42:08
 * LastEditTime: 2022-12-29 16:42:08
 * Description:  This file is for intr mode
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 0.0.1 liushengming 2022/12/29 create file
 */

#include "fmhu_hw.h"
#include "fmhu.h"
#include "fdebug.h"


#define FMHU_INTR_DEBUG_TAG "FMHU_INTR"
#define FMHU_INTR_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FMHU_INTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMHU_INTR_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FMHU_INTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMHU_INTR_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FMHU_INTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define FMHU_INTR_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FMHU_INTR_DEBUG_TAG, format, ##__VA_ARGS__)

/**
 * @name: FMhuGetChanIntrMask
 * @msg: get channel interrupt status
 * @param {FScmiMhu *instance_p} instance pointer
 * @return {u32} channel interrupt status
 */
u32 FMhuGetChanIntrMask(FScmiMhu *instance_p)
{
    u32 status = 0;

    status = FMHU_READ_INT_MASK(instance_p->mhu.config.irq_addr);

    return status;
}

/**
 * @name: FMhuSetChanIntrMask
 * @msg: set channel interrupt status
 * @param {FScmiMhu *instance_p} instance pointer
 * @param {u32} status channel interrupt status
 * @return {void} 
 */
void FMhuSetChanIntrMask(FScmiMhu *instance_p, u32 status)
{
    FMHU_WRITE_INT_MASK(instance_p->mhu.config.irq_addr, status);
    return;
}

/**
 * @name: FMhuIntrHandler
 * @msg: interrupt handler
 * @param {FScmiMhu *instance_p} instance pointer
 * @return {void} 
 */
void FMhuIntrHandler(FScmiMhu *instance_p)
{
    FASSERT(instance_p != NULL);
    u32 status = 0;

    status = FMHU_READ_SE_STAT(instance_p->mhu.config.base_addr);

    if (!status)
    {
        /* code */
        FMHU_INTR_ERROR("FMhuIntrHandler: status is 0x%x\n", status);
        return;
    }

    /*receive data*/
    FMHU_INTR_ERROR("FMhuIntrHandler: receive data.");
    FMHU_WRITE_SE_CLR(instance_p->mhu.config.base_addr, status);
    return;
}