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
 * Modified from NetBSD sys/dev/acpi/acpi_cppc.c with NETBSD LICENSE
 *
 * FilePath: facpi_cppc.c
 * Date: 2025-04-10 16:20:52
 * LastEditTime: 2025-04-10 16:20:52
 * Description:  This file is for acpi cppc functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2025/7/30    init commit
 */
/***************************** Include Files *********************************/
#include "fdrivers_port.h"
#include "facpi.h"
#include "facpi_util.h"

/************************** Constant Definitions *****************************/
/* _CPC package elements */
typedef enum CPCPackageElement
{
    CPCNumEntries,
    CPCRevision,
    CPCHighestPerformance,
    CPCNominalPerformance,
    CPCLowestNonlinearPerformance,
    CPCLowestPerformance,
    CPCGuaranteedPerformanceReg,
    CPCDesiredPerformanceReg,
    CPCMinimumPerformanceReg,
    CPCMaximumPerformanceReg,
    CPCPerformanceReductionToleranceReg,
    CPCTimeWindowReg,
    CPCCounterWraparoundTime,
    CPCReferencePerformanceCounterReg,
    CPCDeliveredPerformanceCounterReg,
    CPCPerformanceLimitedReg,
    CPCCPPCEnableReg,
    CPCAutonomousSelectionEnable,
    CPCAutonomousActivityWindowReg,
    CPCEnergyPerformancePreferenceReg,
    CPCReferencePerformance,
    CPCLowestFrequency,
    CPCNominalFrequency,
} CPCPackageElement;

/* PCC command numbers */
#define CPPC_PCC_READ   0x00
#define CPPC_PCC_WRITE  0x01
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FACPI_DEBUG_TAG "FACPI"
#define FACPI_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_WARN(format, ...) FT_DEBUG_PRINT_W(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_INFO(format, ...) FT_DEBUG_PRINT_I(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_VERBOS(format, ...) \
    FT_DEBUG_PRINT_V(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/
static ACPI_STATUS cppc_parse_cpc(FAcpiCppc *);
static ACPI_STATUS cppc_cpufreq_init(FAcpiCppc *);
int cppc_cpufreq_get(FAcpiCppc *cppc, uint32_t *fq);
int cppc_cpufreq_set(FAcpiCppc *cppc, uint32_t fq);
static ACPI_STATUS cppc_read(FAcpiCppc *, CPCPackageElement, ACPI_INTEGER *);
static ACPI_STATUS cppc_write(FAcpiCppc *, CPCPackageElement, ACPI_INTEGER);

ACPI_STATUS pcc_message(FAcpiPcc *pcc, ACPI_GENERIC_ADDRESS *reg, uint32_t command,
                        int flags, ACPI_INTEGER *val);

FError FAcpiPccInitialize(FAcpi *acpi, FAcpiPcc *pcc);
/*****************************************************************************/
FError FAcpiCppcInitialize(FAcpi *acpi, FAcpiCppc *cppc, u32 cpu_id)
{
    FASSERT(acpi && cppc);
    FError ret = FACPI_SUCCESS;
    FAcpiDevNode *ad;
    ACPI_HANDLE handle;
    ACPI_STATUS rv;
    ACPI_TABLE_HEADER *hdr;

    for (u32 i = 0; i < acpi->max_dev_node; i++)
    {
        ad = &acpi->dev_nodes[i];
        if (ad->ad_devinfo->Type != ACPI_TYPE_DEVICE)
        {
            continue;
        }

        rv = AcpiGetHandle(ad->ad_handle, "_CPC", &handle);
        if (ACPI_FAILURE(rv))
        {
            continue;
        }

        if (!acpi_match_cpu_handle(cpu_id, ad->ad_handle))
        {
            continue;
        }

        cppc->ad = ad;
        cppc->handle = ad->ad_handle;
        cppc->acpi = acpi;
        break;
    }

    if (cppc->ad == NULL)
    {
        FACPI_ERROR("Failed to find CPU handle for CPU ID %u", cpu_id);
        return FACPI_ERR_CPPC_INIT_FAILED;
    }

    FACPI_INFO("Processor Performance Control (cpu-%d)", cpu_id);

    /*
	 * Load drivers that operate on System Description Tables.
	 */
    for (u32 i = 0; i < AcpiGbl_RootTableList.CurrentTableCount; ++i)
    {
        rv = AcpiGetTableByIndex(i, &hdr);
        if (ACPI_FAILURE(rv))
        {
            continue;
        }

        if (memcmp(hdr->Signature, ACPI_SIG_PCCT, ACPI_NAMESEG_SIZE) == 0)
        {
            acpi->pcc.pcct = (void *)hdr;
            ret = FAcpiPccInitialize(acpi, &acpi->pcc);
            if (ret != FACPI_SUCCESS)
            {
                return ret;
            }
        }
        AcpiPutTable(hdr);
    }

    rv = cppc_parse_cpc(cppc);
    if (ACPI_FAILURE(rv))
    {
        FACPI_ERROR("failed to parse CPC package: %s", AcpiFormatException(rv));
        return FACPI_ERR_CPPC_INIT_FAILED;
    }

    rv = cppc_cpufreq_init(cppc);
    if (ACPI_FAILURE(rv))
    {
        FACPI_ERROR("failed to init CPC freq: %s", AcpiFormatException(rv));
        return FACPI_ERR_CPPC_INIT_FAILED;
    }

    return ret;
}

void FAcpiCppcDeInitialize(FAcpiCppc *cppc)
{
}

FError FAcpiCppcGetCpuFreq(FAcpiCppc *cppc, u32 *current_cpu_freq)
{
    return (cppc_cpufreq_get(cppc, current_cpu_freq) == 0) ? FACPI_SUCCESS : FACPI_ERR_CPPC_GET_FREQ_FAILED;
}

FError FAcpiCppcSetCpuFreq(FAcpiCppc *cppc, u32 desired_freq)
{
    return (cppc_cpufreq_set(cppc, desired_freq) == 0) ? FACPI_SUCCESS : FACPI_ERR_CPPC_SET_FREQ_FAILED;
}

/*
 * cppc_parse_cpc --
 *
 *	Read and verify the contents of the _CPC package.
 */
static ACPI_STATUS cppc_parse_cpc(FAcpiCppc *cppc)
{
    ACPI_BUFFER buf;
    ACPI_STATUS rv;

    buf.Pointer = NULL;
    buf.Length = ACPI_ALLOCATE_BUFFER;
    rv = AcpiEvaluateObjectTyped(cppc->handle, "_CPC", NULL, &buf, ACPI_TYPE_PACKAGE);
    if (ACPI_FAILURE(rv))
    {
        return rv;
    }

    cppc->cpc = (ACPI_OBJECT *)buf.Pointer;
    if (cppc->cpc->Package.Count == 0)
    {
        return AE_NOT_EXIST;
    }
    if (cppc->cpc->Package.Elements[CPCNumEntries].Type != ACPI_TYPE_INTEGER)
    {
        return AE_TYPE;
    }
    cppc->ncpc = cppc->cpc->Package.Elements[CPCNumEntries].Integer.Value;

    return AE_OK;
}

static inline ACPI_INTEGER get_delta(ACPI_INTEGER t1, ACPI_INTEGER t0)
{
    if (t1 > t0 || t0 > ~(uint32_t)0)
    {
        return t1 - t0;
    }

    return (uint32_t)t1 - (uint32_t)t0;
}

/* from linux-kernel 6.6 drivers/cpufreq/cppc_cpufreq.c cppc_cpufreq_get_rate() */
int cppc_cpufreq_get(FAcpiCppc *cppc, uint32_t *fq)
{
    ACPI_INTEGER reference_perf, delta_reference, delta_delivered;
    ACPI_INTEGER t0_reference, t1_reference;
    ACPI_INTEGER t0_delivered, t1_delivered;
    ACPI_INTEGER desired;
    ACPI_STATUS rv;

    rv = cppc_read(cppc, CPCReferencePerformance, &reference_perf);
    if (ACPI_FAILURE(rv))
    {
        return EIO;
    }

    rv = cppc_read(cppc, CPCReferencePerformanceCounterReg, &t0_reference);
    if (ACPI_FAILURE(rv))
    {
        return EIO;
    }

    rv = cppc_read(cppc, CPCDeliveredPerformanceCounterReg, &t0_delivered);
    if (ACPI_FAILURE(rv))
    {
        return EIO;
    }

    FDriverUdelay(2); /* 2usec delay between sampling */

    rv = cppc_read(cppc, CPCReferencePerformanceCounterReg, &t1_reference);
    if (ACPI_FAILURE(rv))
    {
        return EIO;
    }

    rv = cppc_read(cppc, CPCDeliveredPerformanceCounterReg, &t1_delivered);
    if (ACPI_FAILURE(rv))
    {
        return EIO;
    }

    delta_reference = get_delta(t1_reference, t0_reference);
    delta_delivered = get_delta(t1_delivered, t0_delivered);

    /* Check to avoid divide-by zero and invalid delivered_perf */
    if (!delta_reference || !delta_delivered)
    {
        rv = cppc_read(cppc, CPCDesiredPerformanceReg, &desired);
        if (ACPI_FAILURE(rv))
        {
            return EIO;
        }

        *fq = (u_int)desired;
        return 0;
    }

    *fq = (reference_perf * delta_delivered) / delta_reference;
    return 0;
}

int cppc_cpufreq_set(FAcpiCppc *cppc, uint32_t fq)
{
    ACPI_INTEGER val;
    ACPI_STATUS rv;

    if (fq < cppc->min_target || fq > cppc->max_target)
    {
        return EINVAL;
    }

    rv = cppc_write(cppc, CPCDesiredPerformanceReg, fq);
    if (ACPI_FAILURE(rv))
    {
        return EIO;
    }

    return 0;
}

/*
 * cppc_cpufreq_init --
 *
 *	Create sysctl machdep.cpu.cpuN.* sysctl tree.
 */
static ACPI_STATUS cppc_cpufreq_init(FAcpiCppc *cppc)
{
    static CPCPackageElement perf_regs[4] = {CPCHighestPerformance, CPCNominalPerformance,
                                             CPCLowestNonlinearPerformance, CPCLowestPerformance};
    ACPI_INTEGER perf[4], last;
    ACPI_STATUS rv;
    int error, i, n;

    for (i = 0, n = 0; i < ARRAY_SIZE(perf_regs); i++)
    {
        rv = cppc_read(cppc, perf_regs[i], &perf[i]);
        if (ACPI_FAILURE(rv))
        {
            return rv;
        }
    }

    cppc->max_target = perf[0];
    cppc->min_target = perf[3];

    FACPI_INFO("Highest cpu freq: %d MHz", cppc->max_target);
    FACPI_INFO("Lowest cpu freq: %d MHz", cppc->min_target);

    uint32_t current_freq = 0U;
    if (cppc_cpufreq_get(cppc, &current_freq) == 0)
    {
        FACPI_INFO("Current cpu freq: %d MHz", current_freq);
    }
    else
    {
        FACPI_ERROR("Failed to get current cpu freq");
    }

    return AE_OK;
}

/*
 * cppc_read --
 *
 *	Read a value from the CPC package that contains either an integer
 *	or indirect register reference.
 */
static ACPI_STATUS cppc_read(FAcpiCppc *cppc, CPCPackageElement index, ACPI_INTEGER *val)
{
    ACPI_OBJECT *obj;
    ACPI_GENERIC_ADDRESS addr;
    ACPI_STATUS rv;

    if (index >= cppc->ncpc)
    {
        return AE_NOT_EXIST;
    }

    obj = &cppc->cpc->Package.Elements[index];
    switch (obj->Type)
    {
        case ACPI_TYPE_INTEGER:
            *val = obj->Integer.Value;
            return AE_OK;

        case ACPI_TYPE_BUFFER:
            if (obj->Buffer.Length < sizeof(AML_RESOURCE_GENERIC_REGISTER))
            {
                return AE_TYPE;
            }
            memcpy(&addr, obj->Buffer.Pointer + sizeof(AML_RESOURCE_LARGE_HEADER), sizeof(addr));
            if (addr.SpaceId == ACPI_ADR_SPACE_PLATFORM_COMM)
            {
                rv = pcc_message(&(cppc->acpi->pcc), &addr, CPPC_PCC_READ, PCC_READ, val);
            }
            else
            {
                rv = AcpiRead(val, &addr);
            }
            return rv;

        default:
            return AE_SUPPORT;
    }
}

/*
 * cppc_write --
 *
 *	Write a value based on the CPC package to the specified register.
 */
static ACPI_STATUS cppc_write(FAcpiCppc *cppc, CPCPackageElement index, ACPI_INTEGER val)
{
    ACPI_OBJECT *obj;
    ACPI_GENERIC_ADDRESS addr;
    ACPI_STATUS rv;

    if (index >= cppc->ncpc)
    {
        return AE_NOT_EXIST;
    }

    obj = &cppc->cpc->Package.Elements[index];
    if (obj->Type != ACPI_TYPE_BUFFER || obj->Buffer.Length < sizeof(AML_RESOURCE_GENERIC_REGISTER))
    {
        return AE_TYPE;
    }

    memcpy(&addr, obj->Buffer.Pointer + sizeof(AML_RESOURCE_LARGE_HEADER), sizeof(addr));
    if (addr.SpaceId == ACPI_ADR_SPACE_PLATFORM_COMM)
    {
        rv = pcc_message(&(cppc->acpi->pcc), &addr, CPPC_PCC_WRITE, PCC_WRITE, &val);
    }
    else
    {
        rv = AcpiWrite(val, &addr);
    }

    return rv;
}