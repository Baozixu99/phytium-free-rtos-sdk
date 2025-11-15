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
* FilePath: facpi_util.h
* Date: 2025-04-10 16:20:52
* LastEditTime: 2025-04-10 16:20:52
* Description:  This file is for ACPI user interface definition
*
* Modify History:
*  Ver   Who        Date         Changes
* ----- ------     --------    --------------------------------------
* 1.0   zhugengyu  2025/7/30    init commit
*/

#ifndef FACPI_UTIL_H
#define FACPI_UTIL_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "facpi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
ACPI_STATUS acpi_eval_integer(ACPI_HANDLE, char *, ACPI_INTEGER *);
ACPI_STATUS acpi_eval_set_integer(ACPI_HANDLE handle, char *path, ACPI_INTEGER arg);
ACPI_STATUS acpi_eval_string(ACPI_HANDLE, char *, char **);
ACPI_STATUS acpi_eval_struct(ACPI_HANDLE, char *, ACPI_BUFFER *);
ACPI_STATUS acpi_eval_reference_handle(ACPI_OBJECT *, ACPI_HANDLE *);

ACPI_STATUS acpi_foreach_package_object(ACPI_OBJECT *,
                                        ACPI_STATUS (*)(ACPI_OBJECT *, void *), void *);
ACPI_STATUS acpi_get(ACPI_HANDLE, ACPI_BUFFER *, ACPI_STATUS (*)(ACPI_HANDLE, ACPI_BUFFER *));

FAcpiDevNode *acpi_match_node(ACPI_HANDLE handle);
void acpi_match_node_init(FAcpiDevNode *ad);

const char *acpi_name_netbsd(ACPI_HANDLE);
int acpi_match_hid(ACPI_DEVICE_INFO *, const char *const *);
int acpi_match_class(ACPI_HANDLE, uint8_t, uint8_t, uint8_t);

boolean acpi_match_cpu_handle(u32 cpu_id, ACPI_HANDLE hdl);

ACPI_STATUS acpi_dsd_integer(ACPI_HANDLE, const char *, ACPI_INTEGER *);
ACPI_STATUS acpi_dsd_string(ACPI_HANDLE, const char *, char **);
ACPI_STATUS acpi_dsd_bool(ACPI_HANDLE, const char *, bool *);

ACPI_STATUS acpi_dsm(ACPI_HANDLE, uint8_t *, ACPI_INTEGER, ACPI_INTEGER,
                     const ACPI_OBJECT *, ACPI_OBJECT **);
ACPI_STATUS acpi_dsm_typed(ACPI_HANDLE, uint8_t *, ACPI_INTEGER, ACPI_INTEGER,
                           const ACPI_OBJECT *, ACPI_OBJECT_TYPE, ACPI_OBJECT **);
ACPI_STATUS acpi_dsm_integer(ACPI_HANDLE, uint8_t *, ACPI_INTEGER, ACPI_INTEGER,
                             const ACPI_OBJECT *, ACPI_INTEGER *);
ACPI_STATUS acpi_dsm_query(ACPI_HANDLE, uint8_t *, ACPI_INTEGER, ACPI_INTEGER *);

ACPI_TABLE_HEADER *acpi_map_rsdt(FAcpi *acpi);
void acpi_unmap_rsdt(FAcpi *acpi, ACPI_TABLE_HEADER *rsdt);

ACPI_STATUS acpi_madt_map(FAcpi *acpi);
void acpi_madt_unmap(FAcpi *acpi);

ACPI_STATUS acpi_gtdt_map(FAcpi *acpi);
void acpi_gtdt_unmap(FAcpi *acpi);

ACPI_GTDT_WATCHDOG *acpi_find_wdt_table(FAcpi *acpi);

ACPI_STATUS acpi_make_devnode(ACPI_HANDLE handle, uint32_t level, void *context, void **status);

bool acpi_power_register(ACPI_HANDLE);
void acpi_power_deregister(FAcpi *, ACPI_HANDLE);
bool acpi_power_get(ACPI_HANDLE, int *);
bool acpi_power_set(ACPI_HANDLE, int);
ACPI_STATUS acpi_power_res(FAcpi *, ACPI_HANDLE, ACPI_HANDLE, bool);
void acpi_power_add(FAcpiDevNode *);

#ifdef __cplusplus
}
#endif

#endif /* FACPI_H */