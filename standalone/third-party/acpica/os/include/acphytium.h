/*
 * Copyright : (C) 2025 Phytium Information Technology, Inc.
 * All Rights Reserved.
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
 * FilePath: acphytium.h
 * Date: 2025-04-10 16:20:52
 * LastEditTime: 2025-04-10 16:20:52
 * Description:  This file is for acpica library definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2025/7/30    init commit
 */

#ifndef __ACPHYTIUM_RTOS_H__
#define __ACPHYTIUM_RTOS_H__

#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftypes.h"

#ifdef __aarch64__
#define ACPI_MACHINE_WIDTH        64
#define COMPILER_DEPENDENT_INT64  long
#define COMPILER_DEPENDENT_UINT64 unsigned long
#else
#define ACPI_MACHINE_WIDTH        32
#define COMPILER_DEPENDENT_INT64  long long
#define COMPILER_DEPENDENT_UINT64 unsigned long long
#endif

/* 使用 ACPICA 内部实现的互斥锁 */
#define ACPI_MUTEX_TYPE 0

/* 启用 ACPI 应用程序支持 */
#define ACPI_APPLICATION

/* 启用单线程模式 */
#define ACPI_SINGLE_THREADED

/* 启用大型命名空间节点支持 */
#define ACPI_LARGE_NAMESPACE_NODE

/* 使用标准 C 头文件 */
#define ACPI_USE_STANDARD_HEADERS

/* 使用系统提供的 C 标准库 */
#define ACPI_USE_SYSTEM_CLIBRARY

/* 使用系统 C 标准库的文件接口 */
/* #define ACPI_USB_SYSTEM_CLIBRARY_FILEIO */

/* 使用自定义的 calloc */
#define USE_NATIVE_ALLOCATE_ZEROED

/* 启用调试打印 */
#define ACPI_DEBUG_OUTPUT

/* 启用 AcpiOsCreateCache 等接口 */
#define ACPI_USE_LOCAL_CACHE

/* 简化 GPE 相关接口实现 */
#define ACPI_REDUCED_HARDWARE 1

/******************************************************************************
 *
 * FUNCTION:    AcpiEnableDbgPrint
 *
 * PARAMETERS:  Enable, 	            - Enable/Disable debug print
 *
 * RETURN:      None
 *
 * DESCRIPTION: Enable/disable debug print
 *
 *****************************************************************************/

void AcpiEnableDbgPrint(bool Enable);

#endif /* __ACPHYTIUM_RTOS_H__ */