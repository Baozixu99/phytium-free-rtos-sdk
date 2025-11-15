/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fcpumode.h
 * Date: 2024-05-21 14:53:41
 * LastEditTime: 2024-05-21 17:29:56
 * Description:  This file if for defining the CPU work mode .
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangzq     2024/5/21    first release	
 */


#ifndef FCPU_MODE_H
#define FCPU_MODE_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/


/*AARCH32 processor modes*/
#define ARM_USER_MODE   0b10000 /* USER */
#define ARM_FIQ_MODE    0b10001 /* FIQ */
#define ARM_IRQ_MODE    0b10010 /* IRQ */
#define ARM_SVC_MODE    0b10011 /* Supervisor */
#define ARM_ABT_MODE    0b10111 /* Abort */
#define ARM_UND_MODE    0b11011 /* Undef */
#define ARM_SYS_MODE    0b11111 /* System */
#define ARM_MON_MODE    0b10110 /* Monitor */
#define ARM_HYP_MODE    0b11010 /* Hyp */

#define CURRENT_PE_MODE ARM_SVC_MODE /* the default CPU Work Mode in SVC */
#define NESTED_IRQ_MODE \
    ARM_IRQ_MODE /* Nested IRQ mode,used in FInterruptNestedDisable */


#ifdef __cplusplus
}
#endif

#endif