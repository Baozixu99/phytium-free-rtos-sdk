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
 * FilePath: fxmac_msg_common.h
 * Date: 2024-10-21 14:53:42
 * LastEditTime: 2024-10-21 14:53:42
 * Description:  This file is for msg driver header file.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/21  first commit
 */
#ifndef FXMAC_MSG_COMMON_H
#define FXMAC_MSG_COMMON_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fxmac_msg.h"


#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Function declaration *********************************/

int FXmacMsgInitRing(FXmacMsgCtrl *instance);
int FXmacMsgSendMessage(FXmacMsgCtrl *instance, u16 cmd_id, u16 cmd_subid, void *data,
                        int len, int wait);
unsigned int FXmacMsgTxRingWrap(FXmacMsgCtrl *pdata, unsigned int index);

#ifdef __cplusplus
}
#endif

#endif
