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
 * FilePath: spim_spiffs_example.h
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2022-06-17 10:42:40
 * Description:  This file is for the spim_spiffs example function declarations.
 * 
 * Modify History: 
 *  Ver     Who      Date         Changes
 * -----   ------  --------   --------------------------------------
 * 1.0 liqiaozhong 2022/11/2  first commit
 * 
 */

#ifndef SPIM_SPIFFS_EXAMPLE_H
#define SPIM_SPIFFS_EXAMPLE_H

/* spim spiffs read and write test */
int FFreeRTOSSpimSpiffsRunWR(void);

void SpimSpiffsExampleEntry(void);

#endif // !