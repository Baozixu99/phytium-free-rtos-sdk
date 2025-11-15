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
 * FilePath: fxmac_msg_debug.c
 * Date: 2024-10-28 15:33:03
 * LastEditTime: 2024-10-28 15:33:04
 * Description:  This file is for user to debug xmac msg information.
 * 
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/28  first release
 */

#include "fxmac_msg_hw.h"
#include "fxmac_msg.h"
#include "fio.h"
#include "fdrivers_port.h"

#define FXMAC_MSG_DEBUG_TAG "FXMAC_MSG_DEBUG"
#define FXMAC_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FXMAC_MSG_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FXMAC_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
