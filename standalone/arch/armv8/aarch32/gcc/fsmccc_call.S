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
 * FilePath: smccc-call.S
 * Date: 2022-02-10 14:53:41
 * LastEditTime: 2022-02-17 17:28:10
 * Description:  This file is for initiating SMC call
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe     2021/7/3     first release
 */


/*******************************************************************************
*
* FSmcCall - initiate SMC call
*
* This routine initiates SMC call which traps the processor into Monitor Mode.
* The ARM SMC Call Convetion defines that up to eight registers can be exchanged
* during an SMC call. The input parameter contains eight INT32 valeus which are
* to be passed in the SMC call; similarily the output parameter also contains
* eight INT32 values which are returned from the SMC call.
*
* \NOMANUAL
*
* RETURNS: OK
*
* void FSmcCall
*     (
*     FSmc_Data_t * input,  /@ r0 - input register values @/
*     FSmc_Data_t * output  /@ r1 - output register values @/
*     )
*/

.arm
.align 4     
.globl FSmcCall   
FSmcCall:
    STMDB   sp!, {r0-r7}        /* save clobbered registers to stack */
    ldr     r12, [sp, #(4 * 0)] /* get 1st argument (ptr to input struct) */
    ldmia   r12, {r0-r7}        /* save input argument to r0-r7 */
    smc     #0
    ldr	    r12, [sp, #(4 * 1)] /* get 2th argument (ptr to output result) */
    stmia   r12, {r0-r7}        /* get output argument from r0-r7 */
    ldmfd   sp!, {r0-r7}        /* restore clobbered registers from stack */
    bx	    lr
.size FSmcCall, .- FSmcCall
