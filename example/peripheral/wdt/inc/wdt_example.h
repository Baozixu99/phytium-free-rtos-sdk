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
 * FilePath: wdt_example.h
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2022-06-17 10:42:40
 * Description:  This file is for wdt test example function declarations.
 *
 * Modify History:
 *  Ver      Who            Date           Changes
 * -----   ------         --------     --------------------------------------
 *  1.0   wangxiaodong    2022/8/9      first release
 *  2.0   liqiaozhong     2024/4/22     add no letter shell mode, adapt to auto-test system
 */


#ifndef WDT_EXAMPLE_H
#define WDT_EXAMPLE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* wdt feed test */
int FFreeRTOSWdtCreate(void);

#ifdef __cplusplus
}
#endif

#endif // !