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
 * FilePath: fsmcc.h
 * Created Date: 2023-06-16 11:30:49
 * Last Modified: 2023-06-16 15:47:29
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0      huanghe   2023-06-16        first init
 */
#ifndef ARCH_ARMV8_AARCH32_SMCC_H
#define ARCH_ARMV8_AARCH32_SMCC_H


#ifdef __cplusplus
extern "C"
{
#endif
#include "ftypes.h"

struct FSmcccRes
{
    unsigned long a0;
    unsigned long a1;
    unsigned long a2;
    unsigned long a3;
};


void FSmcccHvcCall(unsigned long arg0, unsigned long arg1, unsigned long arg2,
                   unsigned long arg3, unsigned long arg4, unsigned long arg5,
                   unsigned long arg6, unsigned long arg7, struct FSmcccRes *res);


void FSmcccSmcCall(unsigned long arg0, unsigned long arg1, unsigned long arg2,
                   unsigned long arg3, unsigned long arg4, unsigned long arg5,
                   unsigned long arg6, unsigned long arg7, struct FSmcccRes *res);

void FSmcccSmcGetSocIdCall(struct FSmcccRes *res);

#ifdef __cplusplus
}
#endif

#endif // !FT_SMC_H
