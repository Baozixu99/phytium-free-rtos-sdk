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
 * FilePath: fspim_msg_intr.c
 * Date: 2024-10-14 11:27:42
 * LastEditTime: 2024-10-24 17:46:03
 * Description:  This file is for providing spi msg interrupt func.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2024/10/24   first release
 */


#include "fspim_msg.h"
#include "fspim_msg_hw.h"
#include "fassert.h"

#define FSPI_DEBUG_TAG "FSPI-MSG-INTR"
#define FSPI_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FSPI_DEBUG_TAG, format, ##__VA_ARGS__)

/**
 * @name: FSpiMsgInterruptHandler
 * @msg: SPI_MSG中断处理函数
 * @return {无}
 * @param {s32} vector，中断向量号，此处不关心此参数
 * @param {void} *param, 中断输入参数, 指向FSpiMsgCtrl的驱动控制实例
 */

void FSpiMsgInterruptHandler(s32 vector, void *param)
{
    FSpiMsgCtrl *spi_msg_ctrl = (FSpiMsgCtrl *)param;
    spi_msg_ctrl->cmd_completion = CMD_MSG_COMPLETION;


    FSpiMsgWriteRegfile(spi_msg_ctrl, FMSG_S2M_INT_STATE, 0);
    FSpiMsgWriteRegfile(spi_msg_ctrl, FSPI_MSG_REGFILE_S2M_INT_CLEAN, 0x10);
    return;
}