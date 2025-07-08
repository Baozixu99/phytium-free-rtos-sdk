/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: lwip_iperf_example.h
 * Created Date: 2023-10-16 15:16:18
 * Last Modified: 2023-10-23 16:46:19
 * Description:  This file is for lwip iperf server example function definition.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong  2023/10/16          first release
 *  2.0   liuzhihong  2024/04/28   add no letter shell mode, adapt to auto-test system
 */

#ifndef  LWIP_IPERF_SERVER_EXAMPLE_H
#define  LWIP_IPERF_SERVER_EXAMPLE_H

/***************************** Include Files *********************************/
#include "ftypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* entry function for lwip iperf server example */
int FFreeRTOSLwipIperfServerTaskCreate(void);
void LwipIperfServerDeinit(void);

#ifdef __cplusplus
}
#endif

#endif