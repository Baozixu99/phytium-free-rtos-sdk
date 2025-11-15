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
 * FilePath: fgeneric_timer.h
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:30:13
 * Description:  This file is for generic timer API's 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  huanghe	2021/11/13		initialization
 *  1.1  zhugengyu 2022/06/05     add tick api
 *  1.2  wangxiaodong 2023/05/29  modify api
 */


#ifndef FGENERIC_TIMER_H
#define FGENERIC_TIMER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ftypes.h"

/* Generic timer register defines. The format is: coproc, opc1, CRn, CRm, opc2 */
#define CNTFRQ       15, 0, 14, 0, 0 /*Counter-timer Frequency register*/
#define CNTP_CTL     15, 0, 14, 2, 1 /*Counter-timer Physical Timer Control register*/
#define CNTP_TVAL    15, 0, 14, 2, 0 /*Counter-timer Physical Timer TimerValue register*/
#define CNTV_CTL     15, 0, 14, 3, 1 /*Counter-timer Virtual Timer Control register*/
#define CNTV_TVAL    15, 0, 14, 3, 0 /*Counter-timer Virtual Timer TimerValue register*/

/* Generic timer 64 bit register defines. The format is: coproc, opc1, CRm */
#define CNTP_CVAL_64 15, 2, 14 /*Counter-timer Physical Timer CompareValue register*/
#define CNTPCT_64    15, 0, 14 /*Counter-timer Physical Count register*/
#define CNTV_CVAL_64 15, 3, 14 /*Counter-timer Virtual Timer CompareValue register*/
#define CNTVCT_64    15, 1, 14 /*Counter-timer Virtual Count register*/
#define CNTVOFF_64   15, 4, 14 /*Counter-timer Virtual Offset register*/

/* Set generic timer CompareValue */
void GenericTimerSetTimerCompareValue(u32 id, u64 timeout);

/* Set generic timer TimerValue */
void GenericTimerSetTimerValue(u32 id, u32 timeout);

/* Unmask generic timer interrupt */
void GenericTimerInterruptEnable(u32 id);

/* Mask generic timer interrupt */
void GenericTimerInterruptDisable(u32 id);

/* Enable generic timer */
void GenericTimerStart(u32 id);

/* Get generic timer count value */
u64 GenericTimerRead(u32 id);

/* Get generic timer frequency of the system counter */
u32 GenericTimerFrequecy(void);

/* Disable generic timer */
void GenericTimerStop(u32 id);


#ifdef __cplusplus
}
#endif

#endif