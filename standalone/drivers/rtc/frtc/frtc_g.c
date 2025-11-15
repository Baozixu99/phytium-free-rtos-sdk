/*
 * Copyright (C) 2021, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: frtc_g.c
 * Date: 2021-08-25 14:53:42
 * LastEditTime: 2021-08-26 09:01:26
 * Description:  This file is for rtc static configuration
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   Wangxiaodong   2021/8/26 first commit
 */

#include "fparameters.h"
#include "frtc.h"

/* default configs of rtc ctrl */
const FRtcConfig FRtcConfigTbl =
{
    .control_base_addr = RTC_CONTROL_BASE,
    .instance_name = "RTC"
};

