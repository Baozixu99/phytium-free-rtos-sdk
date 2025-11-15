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
* FilePath: facpi.h
* Date: 2025-04-10 16:20:52
* LastEditTime: 2025-04-10 16:20:52
* Description:  This file is for ACPI user interface definition
*
* Modify History:
*  Ver   Who        Date         Changes
* ----- ------     --------    --------------------------------------
* 1.0   zhugengyu  2025/7/30    init commit
*/

#ifndef FACPI_H
#define FACPI_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fdrivers_port.h"
#ifndef __PHYTIUM_ACPICA__
#define __PHYTIUM_ACPICA__
#endif
/* include ACPICA library */
#include "acpica/include/acpi.h"
#include "acpica/include/accommon.h"
#include "acpica/include/amlresrc.h"

/************************** Constant Definitions *****************************/

/* ACPI driver error code */
#define FACPI_SUCCESS                   FT_SUCCESS
#define FACPI_ERR_RSDP_INVALID          FT_MAKE_ERRCODE(ErrModBsp, ErrAcpi, 1)
#define FACPI_ERR_ACPICA_INIT_FAILED    FT_MAKE_ERRCODE(ErrModBsp, ErrAcpi, 2)
#define FACPI_ERR_CPPC_INIT_FAILED      FT_MAKE_ERRCODE(ErrModBsp, ErrAcpi, 3)
#define FACPI_ERR_PCC_INIT_FAILED       FT_MAKE_ERRCODE(ErrModBsp, ErrAcpi, 4)
#define FACPI_ERR_CPPC_GET_FREQ_FAILED  FT_MAKE_ERRCODE(ErrModBsp, ErrAcpi, 5)
#define FACPI_ERR_CPPC_SET_FREQ_FAILED  FT_MAKE_ERRCODE(ErrModBsp, ErrAcpi, 6)
#define FACPI_ERR_CPPC_SET_FREQ_INVALID FT_MAKE_ERRCODE(ErrModBsp, ErrAcpi, 7)
#define FACPI_ERR_TZ_INIT_FAILED        FT_MAKE_ERRCODE(ErrModBsp, ErrAcpi, 8)

#define PCC_READ                        BIT(0)
#define PCC_WRITE                       BIT(1)

/**************************** Type Definitions *******************************/
typedef struct _FAcpi FAcpi;
typedef struct _FAcpiDevNode FAcpiDevNode;
typedef struct _FAcpiWalkContext FAcpiWalkContext;

typedef struct
{
    uintptr_t rsdp_root;
    void *rsdp_virt;
} FAcpiConfig;

typedef struct
{
    FAcpiDevNode *ad;
    ACPI_HANDLE handle;
    ACPI_OBJECT *cpc;
    FAcpi *acpi;
    u32 ncpc;
    ACPI_INTEGER max_target;
    ACPI_INTEGER min_target;
} FAcpiCppc;

typedef struct
{
    ACPI_GENERIC_ADDRESS reg_addr;
    u64 reg_preserve;
    u64 reg_set;
} FAcpiPccRegister;

typedef struct
{
    u8 ss_id;
    u8 ss_type;
    void *ss_data;
    size_t ss_len;
    FAcpiPccRegister ss_doorbell_reg;
    u32 ss_latency;
    u32 ss_turnaround;
} FAcpiPccSubspace;

typedef struct
{
    ACPI_TABLE_PCCT *pcct;
    FAcpiPccSubspace ss[25]; /* according to CPCPackageElement */
    u32 nss;
} FAcpiPcc;

typedef struct
{
    ACPI_HANDLE res_handle;
    ACPI_INTEGER res_level;
    ACPI_INTEGER res_order;
    ACPI_HANDLE res_ref[5];
    char res_name[5];
} FAcpiPowerResource;

#define ATZ_NLEVELS     10
#define ATZ_ZEROC       2732
#define ATZ_ZONE_EXPIRE 9000

typedef struct
{
    ACPI_BUFFER al[ATZ_NLEVELS];
    uint32_t ac[ATZ_NLEVELS];
    uint32_t crt;
    uint32_t hot;
    uint32_t rtv;
    uint32_t psv;
    uint32_t tc1;
    uint32_t tc2;
    uint32_t tmp;
    uint32_t prevtmp;
    uint32_t tzp;
    uint32_t fanmin;
    uint32_t fanmax;
    uint32_t fancurrent;
} FAcpiTzZone;

typedef struct
{
    FAcpiDevNode *node;
    FAcpiTzZone zone;
    int zone_expire;
    boolean first;
    boolean have_fan;
    FAcpi *acpi;
} FAcpiTz;

typedef struct _FAcpiDevNode
{
    FAcpi *acpi;
    ACPI_NOTIFY_HANDLER ad_notify; /* Device notify */
    ACPI_DEVICE_INFO *ad_devinfo;  /* Device info */
    ACPI_HANDLE ad_handle;         /* Device handle */
    char ad_name[5];               /* Device name */
    uint32_t ad_flags;             /* Device flags */
    uint32_t ad_type;              /* Device type */
    int ad_state;                  /* Device power state */
} FAcpiDevNode;

typedef struct _FAcpiWalkContext
{
    FAcpi *acpi;
    FAcpiDevNode *parent;
} FAcpiWalkContext;

typedef struct _FAcpi
{
    FAcpiConfig config;
    ACPI_HANDLE acpi_scopes[4];
    ACPI_TABLE_HEADER *madt_header;
    ACPI_TABLE_HEADER *gtdt_header;
    FAcpiWalkContext dev_walk_context;
#define FACPI_MAX_DEV_NODE_NUM 256U
    FAcpiDevNode dev_nodes[FACPI_MAX_DEV_NODE_NUM];
    u32 max_dev_node;
#define FACPI_MAX_PWR_RES_NUM 20U
    FAcpiPowerResource res_head[FACPI_MAX_PWR_RES_NUM];
    u32 max_pwr_res;
    FAcpiCppc cppc;
    FAcpiPcc pcc;
    FAcpiTz tz;
    u32 is_ready;
} FAcpi;
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
FError FAcpiCfgInitialize(FAcpi *acpi, const FAcpiConfig *config);

void FAcpiDeInitialize(FAcpi *acpi);

FError FAcpiCppcInitialize(FAcpi *acpi, FAcpiCppc *cppc, u32 cpu_id);

void FAcpiCppcDeInitialize(FAcpiCppc *cppc);

FError FAcpiCppcGetCpuFreq(FAcpiCppc *cppc, u32 *current_cpu_freq);

FError FAcpiCppcSetCpuFreq(FAcpiCppc *cppc, u32 desired_freq);

FError FAcpiTzInitialize(FAcpi *acpi, FAcpiTz *tz);

void FAcpiTzDeInitialize(FAcpiTz *tz);

FError FAcpiTzGetTemperature(FAcpiTz *tz, u32 *temperature);

#ifdef __cplusplus
}
#endif

#endif /* FACPI_H */