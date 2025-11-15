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
 * FilePath: fi2c_msg_master.h
 * Date: 2024-10-1 14:53:42
 * LastEditTime: 2024-10-10 08:36:14
 * Description:  This file is for msg i2c driver header file.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangzq      2024/10/08  first commit
 */
#ifndef FI2C_MSG_MASTER_H
#define FI2C_MSG_MASTER_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fmsg_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

FError FI2cMsgMasterVirtParseData(FI2cMsgCtrl *const instance, FI2cMsgInfo *shmem_msg, int index);

FError FI2cMsgMasterVirtProbe(FI2cMsgCtrl *const instance);

FError FI2cMsgMasterVirtXfer(FI2cMsgCtrl *const instance, struct FI2cMsg *msgs, u32 num);

int FI2cMsgMasterHandle(FI2cMsgCtrl *const instance);

int FI2cMsgMasterTotalFrameCnt(FI2cMsgCtrl *instance);
#ifdef __cplusplus
}
#endif

#endif
