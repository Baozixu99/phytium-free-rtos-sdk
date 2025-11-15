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
 * FilePath: fi2c_msg_common.h
 * Date: 2024-10-1 14:53:42
 * LastEditTime: 2024-10-10 08:36:14
 * Description:  This file is for msg i2c common header file.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangzq      2024/10/08  first commit
 */
#ifndef FI2C_MSG_COMMON_H
#define FI2C_MSG_COMMON_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fmsg_common.h"
#include "fi2c_msg.h"
#include "fi2c_msg_hw.h"

#ifdef __cplusplus
extern "C"
{
#endif

void FI2cMsgVirtDisable(FI2cMsgCtrl *const instance);

void FI2cMsgCommonSetCmd(FI2cMsgCtrl *const instance, FI2cMsgInfo *i2c_virt_msg, u8 cmd, u8 sub_cmd);

FError FI2cMsgVirtCheckResult(FI2cMsgCtrl *const instance);

FError FI2cMsgVirtCheckStatus(FI2cMsgCtrl *const instance, FI2cMsgInfo *msg);

void FI2cMsgVirtSetCmdArray(FI2cMsgCtrl *const instance, u8 sub_cmd, u8 *data, u32 len);

void FI2cMsgVirtSetCmd8(FI2cMsgCtrl *const instance, u8 sub_cmd, u8 data);

void FI2cMsgVirtSetCmd16(FI2cMsgCtrl *const instance, u8 sub_cmd, u16 data);

void FI2cMsgVirtSetCmd32(FI2cMsgCtrl *const instance, u8 sub_cmd, u32 data);

void FI2cMsgVirtSetModuleEn(FI2cMsgCtrl *const instance, u8 data);

void FI2cMsgVirtSetSdaHold(FI2cMsgCtrl *const instance, u32 data);

void FI2cMsgVirtSetIntTl(FI2cMsgCtrl *const instance, u8 tx_threshold, u8 rx_threshold);

void FI2cMsgVirtSetDefaultCfg(FI2cMsgCtrl *const instance, FI2cMsgDefaultCfg *buf);

void FI2cMsgNotifySlave(FI2cMsgCtrl *const instance, boolean need_check);

void *FI2cMsgMemcpy(void *dest, const void *src, u32 len);

void FI2cMsgVirtSendMsg(FI2cMsgCtrl *const instance, FI2cMsgInfo *set_msg, boolean complete_flag);
#ifdef __cplusplus
}
#endif

#endif
