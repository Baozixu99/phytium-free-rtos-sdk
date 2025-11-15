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
 * FilePath: facpi_acpica_impl.c
 * Date: 2025-04-10 16:20:52
 * LastEditTime: 2025-04-10 16:20:52
 * Description:  This file is for acpica library implmentation
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2025/7/30    init commit
 */

#include <acpi.h>
#include <accommon.h>
#include <acapps.h>
#include <aslcompiler.h>

#include "fparameters.h"
#include "fcompiler.h"
#include "fassert.h"
#include "fmmu.h"
#include "fio.h"
#include "fsleep.h"
#include "fgeneric_timer.h"
#include "fmemory_pool.h"
#include "finterrupt.h"
#include "fdebug.h"

#define ACPILIB_TAG           "ACPI-OS"
#define LOG_ERR(format, ...)  FT_DEBUG_PRINT_E(ACPILIB_TAG, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) FT_DEBUG_PRINT_W(ACPILIB_TAG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) FT_DEBUG_PRINT_I(ACPILIB_TAG, format, ##__VA_ARGS__)
#define LOG_DBG(format, ...)  FT_DEBUG_PRINT_D(ACPILIB_TAG, format, ##__VA_ARGS__)

#define ACPICA_MEM_BUF_SIZE   SZ_1M
#define RSDP_PAGE_START(addr) ((addr) & (0xFFFF0000U))
#define RSDP_PAGE_SIZE(addr, size) \
    (((addr)-RSDP_PAGE_START(addr) + (size)) & (0xFFFF0000U))

/* Global variables use from acpica lib. */
BOOLEAN AslGbl_DoTemplates = FALSE;      /* disable ACPI table template file */
BOOLEAN AslGbl_VerboseTemplates = FALSE; /* disable ACPI table template file with details */

char AslGbl_MsgBuffer[ASL_MSG_BUFFER_SIZE]; /* ASL compiler gloabal message buffer */
static BOOLEAN EnDbgPrint = FALSE;          /* enable/disable debug print */

static uint8_t acpica_mem_buf[ACPICA_MEM_BUF_SIZE] __attribute__((aligned(8))) = {0};
static FMemp acpica_mem_pool;

extern const uintptr_t rsdp_addr;

/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadable
 *
 * PARAMETERS:  Pointer             - Area to be verified
 *              Length              - Size of area
 *
 * RETURN:      TRUE if readable for entire Length
 *
 * DESCRIPTION: Verify that a pointer is valid for reading
 *
 *****************************************************************************/

BOOLEAN
AcpiOsReadable(void *Pointer, ACPI_SIZE Length)
{
    return (TRUE); /* Do not check Pointer readability */
}

/******************************************************************************
 *
 * FUNCTION:    AcpiEnableDbgPrint
 *
 * PARAMETERS:  en, 	            - Enable/Disable debug print
 *
 * RETURN:      None
 *
 * DESCRIPTION: Formatted output
 *
 *****************************************************************************/

void AcpiEnableDbgPrint(bool Enable)
{
    if (Enable)
    {
        EnDbgPrint = TRUE;
    }
    else
    {
        EnDbgPrint = FALSE;
    }
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsPrintf
 *
 * PARAMETERS:  Fmt, ...            - Standard printf format
 *
 * RETURN:      None
 *
 * DESCRIPTION: Formatted output
 *
 *****************************************************************************/

void ACPI_INTERNAL_VAR_XFACE AcpiOsPrintf(const char *Fmt, ...)
{
    va_list args;

    va_start(args, Fmt);

    if (EnDbgPrint)
    {
        vprintf(Fmt, args);
    }

    va_end(args);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetLine
 *
 * PARAMETERS:  Buffer              - Where to return the command line
 *              BufferLength        - Maximum Length of Buffer
 *              BytesRead           - Where the actual byte count is returned
 *
 * RETURN:      Status and actual bytes read
 *
 * DESCRIPTION: Formatted input with argument list pointer
 *
 *****************************************************************************/

WEAK ACPI_STATUS AcpiOsGetLine(char *Buffer, UINT32 BufferLength, UINT32 *BytesRead)
{
    printf("AcpiOsGetLine called but no input support");
    return (AE_NOT_EXIST);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsAllocate
 *
 * PARAMETERS:  Size                - Amount to allocate, in bytes
 *
 * RETURN:      Pointer to the new allocation. Null on error.
 *
 * DESCRIPTION: Allocate memory. Algorithm is dependent on the OS.
 *
 *****************************************************************************/

void *AcpiOsAllocate(ACPI_SIZE Size)
{
    return (FMempMalloc(&acpica_mem_pool, Size));
}

#ifdef USE_NATIVE_ALLOCATE_ZEROED
/******************************************************************************
 *
 * FUNCTION:    AcpiOsAllocateZeroed
 *
 * PARAMETERS:  Size                - Amount to allocate, in bytes
 *
 * RETURN:      Pointer to the new allocation. Null on error.
 *
 * DESCRIPTION: Allocate and zero memory. Algorithm is dependent on the OS.
 *
 *****************************************************************************/

void *AcpiOsAllocateZeroed(ACPI_SIZE Size)
{
    void *mem;

    mem = AcpiOsAllocate(Size);

    if (mem)
    {
        memset(mem, 0, Size);
    }

    return (mem);
}

void *AcpiOsCalloc(UINT32 Num, ACPI_SIZE Size)
{
    return AcpiOsAllocateZeroed(Num * Size);
}
#endif

/******************************************************************************
 *
 * FUNCTION:    AcpiOsFree
 *
 * PARAMETERS:  Mem                 - Pointer to previously allocated memory
 *
 * RETURN:      None.
 *
 * DESCRIPTION: Free memory allocated via AcpiOsAllocate
 *
 *****************************************************************************/

void AcpiOsFree(void *Mem)
{
    FMempFree(&acpica_mem_pool, Mem);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadMemory
 *
 * PARAMETERS:  Address             - Physical Memory Address to read
 *              Value               - Where Value is placed
 *              Width               - Number of bits (8,16,32, or 64)
 *
 * RETURN:      Value read from physical memory Address. Always returned
 *              as a 64-bit integer, regardless of the read Width.
 *
 * DESCRIPTION: Read data from a physical memory Address
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 *Value, UINT32 Width)
{
    switch (Width)
    {
        case 8:

            *((UINT8 *)Value) = FtIn8(Address);
            break;

        case 16:

            *((UINT16 *)Value) = FtIn16(Address);
            break;

        case 32:

            *((UINT32 *)Value) = FtIn32(Address);
            break;
        case 64:

            *((UINT64 *)Value) = FtIn64(Address);
            break;
        default:

            return (AE_BAD_PARAMETER);
    }

    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsWriteMemory
 *
 * PARAMETERS:  Address             - Physical Memory Address to write
 *              Value               - Value to write
 *              Width               - Number of bits (8,16,32, or 64)
 *
 * RETURN:      None
 *
 * DESCRIPTION: Write data to a physical memory Address
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width)
{
    switch (Width)
    {
        case 8:

            FtOut8(Address, (u8)Value);
            break;

        case 16:

            FtOut16(Address, (u16)Value);
            break;

        case 32:

            FtOut32(Address, (u32)Value);
            break;

        case 64:

            FtOut64(Address, (u64)Value);
            break;

        default:

            return (AE_BAD_PARAMETER);
    }

    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadPort
 *
 * PARAMETERS:  Address             - Address of I/O port/register to read
 *              Value               - Where Value is placed
 *              Width               - Number of bits
 *
 * RETURN:      Value read from port
 *
 * DESCRIPTION: Read data from an I/O port or register
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32 *Value, UINT32 Width)
{
    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsWritePort
 *
 * PARAMETERS:  Address             - Address of I/O port/register to write
 *              Value               - Value to write
 *              Width               - Number of bits
 *
 * RETURN:      None
 *
 * DESCRIPTION: Write data to an I/O port or register
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width)
{
    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsWritePciConfiguration
 *
 * PARAMETERS:  PciId               - Seg/Bus/Dev
 *              Register            - Device Register
 *              Value               - Value to be written
 *              Width               - Number of bits
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Write data to PCI configuration space
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsWritePciConfiguration(ACPI_PCI_ID *PciId, UINT32 Register, UINT64 Value, UINT32 Width)
{
    /* ARM64TODO: Add pci support */
    return (AE_SUPPORT);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsReadPciConfiguration
 *
 * PARAMETERS:  PciId               - Seg/Bus/Dev
 *              Register            - Device Register
 *              Value               - Buffer Where Value is placed
 *              Width               - Number of bits
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Read data from PCI configuration space
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsReadPciConfiguration(ACPI_PCI_ID *PciId, UINT32 Register, UINT64 *Value, UINT32 Width)
{
    /* ARM64TODO: Add pci support */
    return (AE_SUPPORT);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsRedirectOutput
 *
 * PARAMETERS:  Destination         - An open file handle/pointer
 *
 * RETURN:      None
 *
 * DESCRIPTION: Causes redirect of AcpiOsPrintf and AcpiOsVprintf
 *
 *****************************************************************************/

void AcpiOsRedirectOutput(void *Destination)
{
}

void AcpiOsVprintf(const char *Format, va_list Args)
{
    if (EnDbgPrint)
    {
        vprintf(Format, Args);
    }
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsPredefinedOverride
 *
 * PARAMETERS:  InitVal             - Initial Value of the predefined object
 *              NewVal              - The new Value for the object
 *
 * RETURN:      Status, pointer to Value. Null pointer returned if not
 *              overriding.
 *
 * DESCRIPTION: Allow the OS to override predefined names
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES *InitVal, ACPI_STRING *NewVal)
{

    if (!InitVal || !NewVal)
    {
        return (AE_BAD_PARAMETER);
    }

    *NewVal = NULL;

    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsTableOverride
 *
 * PARAMETERS:  ExistingTable       - Header of current table (probably firmware)
 *              NewTable            - Where an entire new table is returned.
 *
 * RETURN:      Status, pointer to new table. Null pointer returned if no
 *              table is available to override
 *
 * DESCRIPTION: Return a different version of a table if one is available
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsTableOverride(ACPI_TABLE_HEADER *ExistingTable, ACPI_TABLE_HEADER **NewTable)
{

    if (!ExistingTable || !NewTable)
    {
        return (AE_BAD_PARAMETER);
    }

    *NewTable = NULL;

    return (AE_NO_ACPI_TABLES);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetRootPointer
 *
 * PARAMETERS:  None
 *
 * RETURN:      RSDP physical Address
 *
 * DESCRIPTION: Gets the root pointer (RSDP)
 *
 *****************************************************************************/

ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer(void)
{
    FASSERT(rsdp_addr != 0U);
    return (ACPI_PHYSICAL_ADDRESS)rsdp_addr;
}

#ifndef ACPI_USE_NATIVE_MEMORY_MAPPING
/******************************************************************************
 *
 * FUNCTION:    AcpiOsMapMemory
 *
 * PARAMETERS:  Where               - Physical Address of memory to be mapped
 *              Length              - How much memory to map
 *
 * RETURN:      Pointer to mapped memory. Null on error.
 *
 * DESCRIPTION: Map physical memory into caller's Address space
 *
 *****************************************************************************/

void *AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS Where, ACPI_SIZE Length)
{
    void *VirtlAdd = (void *)Where;
    return (VirtlAdd);
}
#endif

/******************************************************************************
 *
 * FUNCTION:    AcpiOsUnmapMemory
 *
 * PARAMETERS:  Where               - Logical Address of memory to be unmapped
 *              Length              - How much memory to unmap
 *
 * RETURN:      None.
 *
 * DESCRIPTION: Delete a previously created mapping. Where and Length must
 *              correspond to a previous mapping exactly.
 *
 *****************************************************************************/

void AcpiOsUnmapMemory(void *Where, ACPI_SIZE Length)
{
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsPhysicalTableOverride
 *
 * PARAMETERS:  ExistingTable       - Header of current table (probably firmware)
 *              NewAddress          - Where new table Address is returned
 *                                    (Physical Address)
 *              NewTableLength      - Where new table Length is returned
 *
 * RETURN:      Status, Address/Length of new table. Null pointer returned
 *              if no table is available to override.
 *
 * DESCRIPTION: Returns AE_SUPPORT, function not used in user space.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER *ExistingTable,
                            ACPI_PHYSICAL_ADDRESS *NewAddress, UINT32 *NewTableLength)
{
    return (AE_SUPPORT);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsInitialize
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Init this OSL
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsInitialize(void)
{
    uintptr_t acpi_page_start_addr = RSDP_PAGE_START(rsdp_addr);
    uintptr_t acpi_page_size = RSDP_PAGE_SIZE(rsdp_addr, 0x10000);

    FMmuMap(acpi_page_start_addr, acpi_page_start_addr, acpi_page_size,
            MT_NORMAL | MT_P_RO_U_RX | MT_NS);
    printf("mapping <0x%lx ~ 0x%lx> where rsdp_addr=0x%lx\n", acpi_page_start_addr,
           acpi_page_start_addr + acpi_page_size, rsdp_addr);

    if (FT_COMPONENT_IS_READY != acpica_mem_pool.is_ready)
    {
        memset(&acpica_mem_pool, 0, sizeof(acpica_mem_pool));

        FError result = FMempInit(&acpica_mem_pool, acpica_mem_buf,
                                  acpica_mem_buf + ACPICA_MEM_BUF_SIZE); /* init memory pool */
        FASSERT(result == FMEMP_SUCCESS);
    }

    LOG_INFO("%s", __func__);
    return (AE_OK);
}

ACPI_STATUS
AcpiOsTerminate(void)
{
    uintptr_t acpi_page_start_addr = RSDP_PAGE_START(rsdp_addr);
    uintptr_t acpi_page_size = RSDP_PAGE_SIZE(rsdp_addr, 0x10000);

    FMmuUnMap(acpi_page_start_addr, acpi_page_size);
    printf("unmapping <0x%lx ~ 0x%lx> where rsdp_addr=0x%lx\n", acpi_page_start_addr,
           acpi_page_start_addr + acpi_page_size, rsdp_addr);

    FASSERT(FT_COMPONENT_IS_READY == acpica_mem_pool.is_ready);
    FMempDeinit(&acpica_mem_pool);
    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsStall
 *
 * PARAMETERS:  Microseconds        - Time to stall
 *
 * RETURN:      None. Blocks until stall is completed.
 *
 * DESCRIPTION: Sleep at microsecond granularity
 *
 *****************************************************************************/

void AcpiOsStall(UINT32 Microseconds)
{
    fsleep_microsec((u32)Microseconds);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsSleep
 *
 * PARAMETERS:  Milliseconds        - Time to sleep
 *
 * RETURN:      None. Blocks until sleep is completed.
 *
 * DESCRIPTION: Sleep at millisecond granularity
 *
 *****************************************************************************/

void AcpiOsSleep(UINT64 Milliseconds)
{
    fsleep_millisec((u32)Milliseconds);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsEnterSleep
 *
 * PARAMETERS:  SleepState          - Which sleep state to enter
 *              RegaValue           - Register A Value
 *              RegbValue           - Register B Value
 *
 * RETURN:      Status
 *
 * DESCRIPTION: A hook before writing sleep registers to enter the sleep
 *              state. Return AE_CTRL_SKIP to skip further sleep register
 *              writes.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsEnterSleep(UINT8 SleepState, UINT32 RegaValue, UINT32 RegbValue)
{
    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsGetTimer
 *
 * PARAMETERS:  None
 *
 * RETURN:      Current ticks in 100-nanosecond units
 *
 * DESCRIPTION: Get the Value of a system timer
 *
 ******************************************************************************/

UINT64
AcpiOsGetTimer(void)
{
    return GenericTimerRead(GENERIC_TIMER_ID0);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsInstallInterruptHandler
 *
 * PARAMETERS:  InterruptNumber     - Level handler should respond to.
 *              ServiceRoutine      - Address of the ACPI interrupt handler
 *              Context             - User context
 *
 * RETURN:      Handle to the newly installed handler.
 *
 * DESCRIPTION: Install an interrupt handler. Used to install the ACPI
 *              OS-independent handler.
 *
 *****************************************************************************/

UINT32
AcpiOsInstallInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine, void *Context)
{
    /* ARM64TODO: Add pci support */
    return (AE_SUPPORT);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsRemoveInterruptHandler
 *
 * PARAMETERS:  Handle              - Returned when handler was installed
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Uninstalls an interrupt handler.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine)
{
    /* ARM64TODO: Add pci support */
    return (AE_SUPPORT);
}

/******************************************************************************
 *
 * FUNCTION:    AcpiOsSignal
 *
 * PARAMETERS:  Function            - ACPICA signal function code
 *              Info                - Pointer to function-dependent structure
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Miscellaneous functions. Example implementation only.
 *
 *****************************************************************************/

ACPI_STATUS
AcpiOsSignal(UINT32 Function, void *Info)
{
    ACPI_SIGNAL_FATAL_INFO *fatal;

    switch (Function)
    {
        case ACPI_SIGNAL_FATAL:
            fatal = (ACPI_SIGNAL_FATAL_INFO *)Info;
            printf("ACPI fatal signal, type 0x%x code 0x%x argument 0x%x", fatal->Type,
                   fatal->Code, fatal->Argument);
            break;

        case ACPI_SIGNAL_BREAKPOINT:
            printf("ACPI_SIGNAL_BREAKPOINT");
            break;

        default:
            return (AE_BAD_PARAMETER);
    }

    return (AE_OK);
}

/******************************************************************************
 *
 * FUNCTION:    Spinlock/Semaphore interfaces
 *
 * DESCRIPTION: Map these interfaces to semaphore interfaces
 *
 *****************************************************************************/

#ifdef ACPI_SINGLE_THREADED
ACPI_STATUS
AcpiOsCreateLock(ACPI_SPINLOCK *OutHandle)
{
    return (AE_OK);
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle)
{
}

ACPI_CPU_FLAGS
AcpiOsAcquireLock(ACPI_SPINLOCK Handle)
{
    return (0);
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags)
{
}

ACPI_STATUS
AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_HANDLE *OutHandle)
{
    *OutHandle = (ACPI_HANDLE)1;
    return (AE_OK);
}

ACPI_STATUS
AcpiOsDeleteSemaphore(ACPI_HANDLE Handle)
{
    return (AE_OK);
}

ACPI_STATUS
AcpiOsWaitSemaphore(ACPI_HANDLE Handle, UINT32 Units, UINT16 Timeout)
{
    return (AE_OK);
}

ACPI_STATUS
AcpiOsSignalSemaphore(ACPI_HANDLE Handle, UINT32 Units)
{
    return (AE_OK);
}

ACPI_THREAD_ID
AcpiOsGetThreadId(void)
{
    return (1);
}

ACPI_STATUS
AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void *Context)
{
    Function(Context);
    return (AE_OK);
}
#endif