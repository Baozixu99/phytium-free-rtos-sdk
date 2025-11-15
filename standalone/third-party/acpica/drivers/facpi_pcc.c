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
 * Modified from NetBSD sys/dev/acpi/acpi_pcc.c with NETBSD LICENSE
 *
 * FilePath: facpi_pcc.c
 * Date: 2025-04-10 16:20:52
 * LastEditTime: 2025-04-10 16:20:52
 * Description:  This file is for acpi pcc functions
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
/* Macros for min/max. */
#define __MIN(a, b) ((/*CONSTCOND*/ (a) <= (b)) ? (a) : (b))
#define __MAX(a, b) ((/*CONSTCOND*/ (a) > (b)) ? (a) : (b))

#define __BIT(__n)                                                        \
    (((__UINTMAX_TYPE__)(__n) >= __CHAR_BIT__ * sizeof(__UINTMAX_TYPE__)) \
         ? 0                                                              \
         : ((__UINTMAX_TYPE__)1                                           \
            << (__UINTMAX_TYPE__)((__n) & (__CHAR_BIT__ * sizeof(__UINTMAX_TYPE__) - 1))))

/* __BITS(m, n): bits m through n, m < n. */
#define __BITS(__m, __n) \
    ((__BIT(__MAX((__m), (__n)) + 1) - 1) ^ (__BIT(__MIN((__m), (__n))) - 1))

#define __LOWEST_SET_BIT(__mask)    ((((__mask)-1) & (__mask)) ^ (__mask))
#define __SHIFTOUT(__x, __mask)     (((__x) & (__mask)) / __LOWEST_SET_BIT(__mask))
#define __SHIFTIN(__x, __mask)      ((__x)*__LOWEST_SET_BIT(__mask))

#define __UNVOLATILE(a)             ((void *)(unsigned long)(volatile void *)(a))
#define PCC_MEMORY_BARRIER()        __asm __volatile("dmb ish" ::: "memory")
#define PCC_DMA_BARRIER()           __asm __volatile("dsb sy" ::: "memory")

#define PCC_SIGNATURE(space_id)     (0x50434300 | (space_id))
#define PCC_STATUS_COMMAND_COMPLETE BIT(0)
#define PCC_STATUS_COMMAND_ERROR    BIT(2)
/**************************** Type Definitions *******************************/
typedef void (*pcc_subspace_callback)(ACPI_SUBTABLE_HEADER *, uint8_t, void *);

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
static uint8_t pcc_subspace_foreach(ACPI_TABLE_PCCT *, pcc_subspace_callback, void *);
static void pcc_subspace_attach(ACPI_SUBTABLE_HEADER *, uint8_t, void *);
/*****************************************************************************/
FError FAcpiPccInitialize(FAcpi *acpi, FAcpiPcc *pcc)
{
    FASSERT(acpi && pcc);
    FError ret = FACPI_SUCCESS;
    ACPI_TABLE_PCCT *pcct = pcc->pcct;
    ;
    u8 count;

    count = pcc_subspace_foreach(pcct, NULL, NULL);
    if (count == 0)
    {
        FACPI_ERROR("No subspaces found!");
        return FACPI_ERR_PCC_INIT_FAILED;
    }

    FACPI_INFO("Platform Communications Channel, %u subspace%s", count, count == 1 ? "" : "s");

    pcc->nss = count;
    pcc_subspace_foreach(pcct, pcc_subspace_attach, pcc);

    return ret;
}

/*
 * pcc_subspace_foreach --
 *
 *	Find all subspaces defined in the PCCT table. If a 'func' callback
 *	is provided, the callback is invoked for each subspace with the
 *	table pointer, subspace ID, and 'arg' as arguments. Returns the
 *	number of subspaces found.
 */
static uint8_t pcc_subspace_foreach(ACPI_TABLE_PCCT *pcct, pcc_subspace_callback func, void *arg)
{
    ACPI_SUBTABLE_HEADER *header;
    char *ptr, *end;
    uint8_t count;

    end = (char *)pcct + pcct->Header.Length;
    ptr = (char *)pcct + sizeof(*pcct);
    count = 0;

    while (ptr < end && count < 255)
    {
        header = (ACPI_SUBTABLE_HEADER *)ptr;
        if (header->Length == 0 || ptr + header->Length > end)
        {
            break;
        }
        if (func != NULL)
        {
            func(header, count, arg);
        }
        ++count;
        ptr += header->Length;
    }

    return count;
}

/*
 * pcc_subspace_attach --
 *
 *	Allocate resources for a PCC subspace.
 */
static void pcc_subspace_attach(ACPI_SUBTABLE_HEADER *header, uint8_t id, void *arg)
{
    FAcpiPcc *pcc = arg;
    FAcpiPccSubspace *ss = &pcc->ss[id];
    ACPI_PCCT_SUBSPACE *generic = (ACPI_PCCT_SUBSPACE *)header;

    ss->ss_id = id;
    ss->ss_type = header->Type;
    ss->ss_data = AcpiOsMapMemory(generic->BaseAddress, generic->Length);
    FASSERT(ss->ss_data != NULL);
    ss->ss_len = generic->Length;
    ss->ss_doorbell_reg.reg_addr = generic->DoorbellRegister;
    ss->ss_doorbell_reg.reg_preserve = generic->PreserveMask;
    ss->ss_doorbell_reg.reg_set = generic->WriteMask;
}

/*
 * pcc_wait_command --
 *
 *	Wait for command complete to be set on a PCC subspace, with timeout.
 */
static ACPI_STATUS pcc_wait_command(FAcpiPccSubspace *ss, bool first)
{
    volatile ACPI_PCCT_SHARED_MEMORY *shmem;
    int retry;

    switch (ss->ss_type)
    {
        case ACPI_PCCT_TYPE_GENERIC_SUBSPACE:
            /*
		 * OSPM does not have to wait for command complete before
		 * sending the first command.
		 */
            if (first)
            {
                return AE_OK;
            }
            /* FALLTHROUGH */

        case ACPI_PCCT_TYPE_HW_REDUCED_SUBSPACE:
        case ACPI_PCCT_TYPE_HW_REDUCED_SUBSPACE_TYPE2:
            /*
		 * Wait for the command complete bit to be set in the status
		 * register.
		 */
            shmem = ss->ss_data;
            retry = max(1000, (int)(ss->ss_turnaround + ss->ss_latency * 100));
            while (retry > 0)
            {
                PCC_MEMORY_BARRIER();
                if ((shmem->Status & PCC_STATUS_COMMAND_COMPLETE) != 0)
                {
                    return AE_OK;
                }
                FDriverUdelay(10);
                retry -= 10;
            }
            return AE_TIME;

        default:
            return AE_NOT_IMPLEMENTED;
    }
}

/*
 * pcc_doorbell --
 *
 *	Ring the door bell by writing to the doorbell register.
 */
static ACPI_STATUS pcc_doorbell(FAcpiPccRegister *reg)
{
    ACPI_STATUS rv;
    UINT64 val;

    rv = AcpiRead(&val, &reg->reg_addr);
    if (ACPI_FAILURE(rv))
    {
        return rv;
    }
    val &= reg->reg_preserve;
    val |= reg->reg_set;
    return AcpiWrite(val, &reg->reg_addr);
}

/*
 * pcc_send_command --
 *
 *	Write a command into PCC subspace and ring the doorbell.
 */
static ACPI_STATUS pcc_send_command(FAcpiPccSubspace *ss, ACPI_GENERIC_ADDRESS *reg,
                                    uint32_t command, int flags, ACPI_INTEGER val)
{
    volatile ACPI_PCCT_SHARED_MEMORY *shmem = ss->ss_data;
    uint8_t *data = __UNVOLATILE(shmem + 1);

    FASSERT(ss->ss_type == ACPI_PCCT_TYPE_GENERIC_SUBSPACE ||
            ss->ss_type == ACPI_PCCT_TYPE_HW_REDUCED_SUBSPACE ||
            ss->ss_type == ACPI_PCCT_TYPE_HW_REDUCED_SUBSPACE_TYPE2);

    shmem->Signature = PCC_SIGNATURE(ss->ss_id);
    shmem->Command = command & 0xff;
    if ((flags & PCC_WRITE) != 0)
    {
        if (reg->BitWidth == 32U)
        {
            UINT32 tmp32, mask32, val32;
            val32 = (UINT32)val;
            mask32 = __BITS(reg->BitOffset + reg->BitWidth - 1, reg->BitOffset);
            ACPI_MOVE_32_TO_32(&tmp32, data + reg->Address);
            tmp32 &= ~mask32;
            tmp32 |= __SHIFTIN(val32, mask32);
            ACPI_MOVE_32_TO_32(data + reg->Address, &tmp32);
        }
        else if (reg->BitWidth == 64U)
        {
            UINT64 tmp, mask;
            mask = __BITS(reg->BitOffset + reg->BitWidth - 1, reg->BitOffset);
            ACPI_MOVE_64_TO_64(&tmp, data + reg->Address);
            tmp &= ~mask;
            tmp |= __SHIFTIN(val, mask);
            ACPI_MOVE_64_TO_64(data + reg->Address, &tmp);
        }
        else
        {
            FACPI_ERROR("PCC %d bit not support", reg->BitWidth);
            FASSERT(0);
        }
    }
    PCC_MEMORY_BARRIER();
    shmem->Status &= ~(PCC_STATUS_COMMAND_COMPLETE | PCC_STATUS_COMMAND_ERROR);
    PCC_DMA_BARRIER();

    return pcc_doorbell(&ss->ss_doorbell_reg);
}

/*
 * pcc_receive_response --
 *
 *	Process a command response in PCC subspace.
 */
static ACPI_STATUS pcc_receive_response(FAcpiPccSubspace *ss, ACPI_GENERIC_ADDRESS *reg,
                                        int flags, ACPI_INTEGER *val)
{
    volatile ACPI_PCCT_SHARED_MEMORY *shmem = ss->ss_data;
    const uint8_t *data = __UNVOLATILE(shmem + 1);

    FASSERT(ss->ss_type == ACPI_PCCT_TYPE_GENERIC_SUBSPACE ||
            ss->ss_type == ACPI_PCCT_TYPE_HW_REDUCED_SUBSPACE ||
            ss->ss_type == ACPI_PCCT_TYPE_HW_REDUCED_SUBSPACE_TYPE2);

    FASSERT((shmem->Status & PCC_STATUS_COMMAND_COMPLETE) != 0);

    if ((shmem->Status & PCC_STATUS_COMMAND_ERROR) != 0)
    {
        return AE_ERROR;
    }

    if ((flags & PCC_READ) != 0)
    {
        if (reg->BitWidth == 32U)
        {
            UINT32 tmp32, mask32, val32;
            mask32 = __BITS(reg->BitOffset + reg->BitWidth - 1, reg->BitOffset);
            ACPI_MOVE_32_TO_32(&tmp32, data + reg->Address);
            val32 = __SHIFTOUT(tmp32, mask32);
            *val = (UINT64)val32;
        }
        else if (reg->BitWidth == 64U)
        {
            UINT64 tmp, mask;
            mask = __BITS(reg->BitOffset + reg->BitWidth - 1, reg->BitOffset);
            ACPI_MOVE_64_TO_64(&tmp, data + reg->Address);
            *val = __SHIFTOUT(tmp, mask);
        }
        else
        {
            FACPI_ERROR("PCC %d bit not support", reg->BitWidth);
            FASSERT(0);
        }
    }

    return AE_OK;
}

/*
 * pcc_message --
 *
 *	Send or receive a command over a PCC subspace.
 */
ACPI_STATUS
pcc_message(FAcpiPcc *pcc, ACPI_GENERIC_ADDRESS *reg, uint32_t command, int flags, ACPI_INTEGER *val)
{
    FAcpiPccSubspace *ss;
    uint8_t ss_id;
    ACPI_STATUS rv;

    /* The "Access Width" in the PCC GAS is the Subspace ID */
    ss_id = reg->AccessWidth;

    if (pcc == NULL)
    {
        return AE_NOT_CONFIGURED;
    }
    if (ss_id >= pcc->nss)
    {
        return AE_NOT_FOUND;
    }

    ss = &pcc->ss[ss_id];
    switch (ss->ss_type)
    {
        case ACPI_PCCT_TYPE_GENERIC_SUBSPACE:
        case ACPI_PCCT_TYPE_HW_REDUCED_SUBSPACE:
        case ACPI_PCCT_TYPE_HW_REDUCED_SUBSPACE_TYPE2:
            break;
        default:
            return AE_NOT_IMPLEMENTED;
    }

    /* Wait for any previous commands to finish. */
    rv = pcc_wait_command(ss, true);
    if (ACPI_FAILURE(rv))
    {
        goto unlock;
    }

    /* Place command into subspace and ring the doorbell */
    rv = pcc_send_command(ss, reg, command, flags, *val);
    if (ACPI_FAILURE(rv))
    {
        goto unlock;
    }

    /* Wait for command to complete. */
    rv = pcc_wait_command(ss, false);
    if (ACPI_FAILURE(rv))
    {
        goto unlock;
    }

    /* Process the command response */
    rv = pcc_receive_response(ss, reg, flags, val);

    if (ss->ss_turnaround != 0)
    {
        FDriverUdelay(ss->ss_turnaround);
    }

unlock:
    return rv;
}