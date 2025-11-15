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
 * FilePath: port.h
 * Date: 2022-09-29 18:07:34
 * LastEditTime: 2022-09-29 18:07:34
 * Description:  This file is for modbus variable types and critical section
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming 2022/09/29    first commit
 */

#ifndef _PORT_H
#define _PORT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "ftypes.h"
#include "faarch.h"

#if defined(__aarch64__)

#define ENTER_CRITICAL_SECTION()                    \
    __asm volatile("MSR DAIFSET, #2" ::: "memory"); \
    __asm volatile("DSB SY");                       \
    __asm volatile("ISB SY");

#define EXIT_CRITICAL_SECTION()                     \
    __asm volatile("MSR DAIFCLR, #2" ::: "memory"); \
    __asm volatile("DSB SY");                       \
    __asm volatile("ISB SY");

#else

#define ENTER_CRITICAL_SECTION() \
    do                           \
    {                            \
        u32 state;               \
        state = MFCPSR();        \
        MTCPSR(state | 0xc0);    \
    } while (0);

#define EXIT_CRITICAL_SECTION()   \
    do                            \
    {                             \
        MTCPSR(MFCPSR() & ~0xc0); \
    } while (0);
#endif

typedef unsigned int BOOL;

typedef unsigned char UCHAR;
typedef char CHAR;

typedef uint16_t USHORT;
typedef int16_t SHORT;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* ----------------------- Function prototypes ------------------------------*/

#ifdef __cplusplus
}
#endif

#endif