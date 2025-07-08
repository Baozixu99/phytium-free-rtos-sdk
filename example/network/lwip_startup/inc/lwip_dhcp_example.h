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
 * FilePath: lwip_dhcp_example.h
 * Created Date: 2023-11-21 15:59:57
 * Last Modified: 2024-04-26 15:52:15
 * Description:  This file is for lwip dhcp example function definition.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong   2023/12/26          first release
 *  2.0   liuzhihong   2024/4/26   add no letter shell mode, adapt to auto-test system
 */
#ifndef  LWIP_DHCP_EXAMPLE_H
#define  LWIP_DHCP_EXAMPLE_H

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
/* entry function for lwip dhcp example */
int FFreeRTOSLwipDhcpTaskCreate(void);
void LwipDhcpTestDeinit(void);

#ifdef __cplusplus
}
#endif

#endif