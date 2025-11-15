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
 * FilePath: fscmi_base.h
 * Date: 2022-12-31 21:38:51
 * LastEditTime: 2022-12-31 21:38:51
 * Description:  This file is for base protocol message
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2023/01/04 init
 */


#ifndef FSCMI_BASE_PROTOCOL_H
#define FSCMI_BASE_PROTOCOL_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fparameters.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FSCMI_BASE_PROTOCOL_VERSION                         0x0
#define FSCMI_BASE_PROTOCOL_ATTRIBUTES                      0x1
#define FSCMI_BASE_PROTOCOL_MESSAGE_ATTRIBUTES              0x2
#define FSCMI_BASE_PROTOCOL_DISCOVER_VENDOR                 0x3
#define FSCMI_BASE_PROTOCOL_DISCOVER_SUB_VENDOR             0x4
#define FSCMI_BASE_PROTOCOL_DISCOVER_IMPLEMENTATION_VERSION 0x5
#define FSCMI_BASE_PROTOCOL_DISCOVER_LIST_PROTOCOLS         0x6
#define FSCMI_BASE_PROTOCOL_DISCOVER_AGENT                  0x7
#define FSCMI_BASE_PROTOCOL_NOTIFY_ERRORS                   0x8
#define FSCMI_BASE_PROTOCOL_SET_DEVICE_PERMISSIONS          0x9
#define FSCMI_BASE_PROTOCOL_SET_PROTOCOL_PERMISSIONS        0xa
#define FSCMI_BASE_PROTOCOL_RESET_AGENT_CONFIGURATION       0xb

FError FScmiBaseInit(FScmi *instance_p, boolean poll_completion);

#ifdef __cplusplus
}
#endif


#endif
