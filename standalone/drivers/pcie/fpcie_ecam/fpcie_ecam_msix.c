/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fpcie_ecam_msi.c
 * Created Date: 2024-07-30 17:17:57
 * Last Modified: 2025-07-25 10:45:06
 * Description:  This file is for msi-x driver
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2025-07-15   first version
 */

#include "fpcie_ecam_msix.h"
#include "fpcie_ecam_common.h"
#include "fgic_its.h"
#include "fsleep.h"
#include "finterrupt.h"

#define FPCIE_MSIX_DEBUG_TAG "FPCIE_MSIX"
#define FPCIE_MSIX_DEBUG_D(format, ...) \
    FT_DEBUG_PRINT_D(FPCIE_MSIX_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_MSIX_DEBUG_I(format, ...) \
    FT_DEBUG_PRINT_I(FPCIE_MSIX_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_MSIX_DEBUG_W(format, ...) \
    FT_DEBUG_PRINT_W(FPCIE_MSIX_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_MSIX_DEBUG_E(format, ...) \
    FT_DEBUG_PRINT_E(FPCIE_MSIX_DEBUG_TAG, format, ##__VA_ARGS__)

#define FPCI_DEVID(bus, dev, fn) \
    ((((bus)&0xff) << 8) | (((dev)&0x1f) << 3) | ((fn)&0x07))

/**
 * @name: FPcieEcamGetMsixTableSize
 * @msg: Get the MSI-X interrupt table size of a specified PCIe device
 * @param instance_p: Pointer to the PCIe ECAM configuration space instance
 * @param bus: PCIe bus number
 * @param device: PCIe device number
 * @param function: PCIe function number
 * @param base: Base address offset of the MSI-X capability structure in configuration space
 * @return: Number of MSI-X table entries (actual size = TSIZE field value + 1)
 * @note: 
 */
u32 FPcieEcamGetMsixTableSize(FPcieEcam *instance_p, u8 bus, u8 device, u8 function, u32 base)
{
    u32 mcr;
    FPcieEcamReadConfigSpaceEx(instance_p, bus, device, function, base + FPCIE_MSIX_MCR,
                               &mcr, PCI_SIZE_16);
    return ((mcr & FPCIE_MSIX_MCR_TSIZE) >> FPCIE_MSIX_MCR_TSIZE_SHIFT) + 1;
}


/**
 * @name: FPcieEcamGetMsixTableEntry
 * @msg: Get the physical addresses of MSI-X table entries for a specified PCIe device
 * @param instance_p: Pointer to the PCIe ECAM configuration space instance
 * @param bus: PCIe bus number
 * @param device: PCIe device number
 * @param function: PCIe function number
 * @param base: Base address offset of the MSI-X capability structure in configuration space
 * @param msix: Output parameter, pointer to structure storing MSI-X entry address information
 * @param n_vector: Number of MSI-X entries to retrieve
 * @return: Operation status code (FT_SUCCESS indicates success, others are error codes)
 * @note: 
 */
FError FPcieEcamGetMsixTableEntry(FPcieEcam *instance_p, u8 bus, u8 device, u8 function,
                                  u32 base, FPcieMsix *msix, u16 n_vector)
{
    u32 table_offset;
    u8 table_bir; /* bar index */
    struct FPcieBarInfo bar = {0};
    FError ret;


    FPcieEcamReadConfigSpace(instance_p, bus, device, function, base + FPCIE_MSIX_TR, &table_offset);
    table_bir = table_offset & FPCIE_MSIX_TR_BIR;
    table_offset &= FPCIE_MSIX_TR_OFFSET;
    FPCIE_MSIX_DEBUG_I("table_offset = %08x, table_bir = %02x\n", table_offset, table_bir);
    /* get bar info */
    ret = FPcieEcamGetBar(instance_p, bus, device, function, table_bir, &bar);
    if (ret != FT_SUCCESS)
    {
        FPCIE_MSIX_DEBUG_E("Failed to get bar info for device %02x:%02x.%01x\n", bus, device, function);
        return ret;
    }

    msix->msix_vector_num = n_vector;
    for (size_t i = 0; i < n_vector; i++)
    {
        /* code */
        msix->msix[i] = (struct FPcieMsixVectorTable *)(bar.phys_addr + table_offset +
                                                        (i * FPCIE_MSIX_TABLE_ENTRY_SIZE));
        FPCIE_MSIX_DEBUG_I("msix->msix[%d] = %p\n", i, msix->msix[i]);
    }

    return FT_SUCCESS;
}

/**
 * @name: FPcieEcamMsixDeviceSetup
 * @msg: Initialize MSI-X interrupt vectors for a PCIe device and map them to the interrupt controller
 * @param msix: Pointer to structure storing MSI-X vector information
 * @return: Number of successfully configured vectors (0 indicates failure)
 * @note:
 *  1. Allocates GIC ITS resources for the device
 *  2. Sets up mapping between device events and interrupt IDs
 */
u16 FPcieEcamMsixDeviceSetup(FPcieMsix *msix)
{
    u32 device_id;
    u16 i;
    int ret;
    FGic *gic_instance;
    gic_instance = (FGic *)InterruptGetInstance();
    FASSERT_MSG(gic_instance, "Failed to get interrupt instance\n");
    FASSERT_MSG(msix, "Invalid msix pointer\n");

    if (!msix->msix_vector_num)
    {
        FPCIE_MSIX_DEBUG_E("No msix to setup\n");
        return 0;
    }

    device_id = FPCI_DEVID(msix->bus, msix->device, msix->function);
    FPCIE_MSIX_DEBUG_I("device_id = %08x\n", device_id);
    FPCIE_MSIX_DEBUG_I("msix_vector_num is %d \r\n", msix->msix_vector_num);
    ret = FGicItsDeviceIdInit(gic_instance, device_id, msix->msix_vector_num);
    if (ret)
    {
        FPCIE_MSIX_DEBUG_E("Failed to initialize device id %08x\n", device_id);
        return 0;
    }

    msix->device_id = device_id;
    for (i = 0; i < msix->msix_vector_num; i++)
    {
        msix->vector[i].irq = FGicItsLpiIdAlloc(gic_instance);
        if (!msix->vector[i].irq)
        {
            FASSERT_MSG(0, "Failed to allocate interrupt id for device id %08x\n", device_id)
            return 0;
        }
        msix->vector[i].address = FGicItsGetTransAddr(gic_instance); /* Get message address register */
        msix->vector[i].eventid = i;
        FPCIE_MSIX_DEBUG_I("msix->msi[%d].irq = %d, msix->msi[%d].address = %08x, "
                           "msix->msi[%d].eventid = %d\n",
                           i, msix->vector[i].irq, i, msix->vector[i].address, i,
                           msix->vector[i].eventid);
        ret = FGicItsMapIntId(gic_instance, device_id, i, msix->vector[i].irq);
        if (ret)
        {
            FASSERT_MSG(0, "Failed to map interrupt id %d for device id %08x\n", i, device_id);
            return 0;
        }
    }

    return msix->msix_vector_num;
}


/**
 * @name: FPcieEcamMsixEnable
 * @msg: Enable MSI-X interrupt capability for a PCIe device
 * @param instance_p: Pointer to the PCIe ECAM instance
 * @param msix: Pointer to structure storing MSI-X vector information
 * @return: Operation status code (FT_SUCCESS indicates success)
 * @note:
 *  1. Programs MSI-X table entries with address and data values
 *  2. Sets the enable bit in the MSI-X control register
 *  3. Verifies successful enabling
 */
FError FPcieEcamMsixEnable(FPcieEcam *instance_p, FPcieMsix *msix)
{
    u32 mcr;
    FError ret = FT_SUCCESS;
    FASSERT_MSG(msix, "Invalid msix pointer\n");
    FASSERT_MSG(instance_p, "Invalid instance pointer\n");
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    for (size_t i = 0; i < msix->msix_vector_num; i++)
    {
        u32 map = msix->vector[i].address;
        u32 mdr = msix->vector[i].eventid;

        FPCIE_MSIX_DEBUG_I("msix->msix[%d]->msg_addr = %p, msix->msix[%d]->msg_up_addr "
                           "= %p, msix->msix[%d]->msg_data = %p, "
                           "msix->msix[%d]->vector_ctrl = %p\n",
                           i, &msix->msix[i]->msg_addr, i, &msix->msix[i]->msg_up_addr,
                           i, &msix->msix[i]->msg_data, i, &msix->msix[i]->vector_ctrl);
        FPCIE_MSIX_DEBUG_I("msix->msi[%d].address = %08x, msix->msi[%d].eventid = %d\n",
                           i, msix->vector[i].address, i, msix->vector[i].eventid);
        FPCIE_WRITEECAM_REG((uintptr)&msix->msix[i]->msg_addr, 0, map);
        FPCIE_WRITEECAM_REG((uintptr)&msix->msix[i]->msg_up_addr, 0, 0);
        FPCIE_WRITEECAM_REG((uintptr)&msix->msix[i]->msg_data, 0, mdr);
        FPCIE_WRITEECAM_REG((uintptr)&msix->msix[i]->vector_ctrl, 0, 0);


        FPCIE_MSIX_DEBUG_I("read:msix->msix[]->msg_addr = %08x, "
                           "msix->msix[]->msg_up_addr = %08x, msix->msix[]->msg_data = "
                           "%08x, msix->msix[]->vector_ctrl = %08x\n",
                           FPCIE_READECAM_REG((uintptr)&msix->msix[i]->msg_addr, 0),
                           FPCIE_READECAM_REG((uintptr)&msix->msix[i]->msg_up_addr, 0),
                           FPCIE_READECAM_REG((uintptr)&msix->msix[i]->msg_data, 0),
                           FPCIE_READECAM_REG((uintptr)&msix->msix[i]->vector_ctrl, 0));
    }
    FPCIE_MSIX_DEBUG_I("enable MSI-X for device %02x:%02x.%01x, msix->base is %p \r\n ",
                       msix->bus, msix->device, msix->function, msix->base);
    ret = FPcieEcamReadConfigSpace(instance_p, msix->bus, msix->device, msix->function,
                                   msix->base + FPCIE_MSIX_MCR, &mcr);
    if (ret != FT_SUCCESS)
    {
        FPCIE_MSIX_DEBUG_E("Failed to read MSIX MCR for device %02x:%02x.%01x\n",
                           msix->bus, msix->device, msix->function);
        return ret;
    }
    FPCIE_MSIX_DEBUG_I("before mcr is %08x\n", mcr);
    mcr |= FPCIE_MSIX_MCR_EN;

    ret = FPcieEcamWriteConfigSpace(instance_p, msix->bus, msix->device, msix->function,
                                    msix->base + FPCIE_MSIX_MCR, mcr);
    if (ret != FT_SUCCESS)
    {
        FPCIE_MSIX_DEBUG_E("Failed to write MSIX MCR for device %02x:%02x.%01x\n",
                           msix->bus, msix->device, msix->function);
        return ret;
    }

    ret = FPcieEcamReadConfigSpace(instance_p, msix->bus, msix->device, msix->function,
                                   msix->base + FPCIE_MSIX_MCR, &mcr);
    if (ret != FT_SUCCESS)
    {
        FPCIE_MSIX_DEBUG_E("Failed to read MSIX MCR for device %02x:%02x.%01x\n",
                           msix->bus, msix->device, msix->function);
        return ret;
    }
    FPCIE_MSIX_DEBUG_I("mcr is %08x\n", mcr);
    return ret;
}

/**
 * @name: FPcieEcamMsixDisable
 * @msg: Disable MSI-X interrupt capability for a PCIe device
 * @param instance_p: Pointer to the PCIe ECAM instance
 * @param msix: Pointer to structure storing MSI-X vector information
 * @return: Operation status code (FT_SUCCESS indicates success)
 * @note:
 *  1. Clears the enable bit in the MSI-X control register
 *  2. Resets all MSI-X table entry registers
 */
FError FPcieEcamMsixDisable(FPcieEcam *instance_p, FPcieMsix *msix)
{
    u32 mcr;
    FError ret = FT_SUCCESS;
    FASSERT_MSG(msix, "Invalid msix pointer\n");
    FASSERT_MSG(instance_p, "Invalid instance pointer\n");
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);


    ret = FPcieEcamReadConfigSpace(instance_p, msix->bus, msix->device, msix->function,
                                   msix->base + FPCIE_MSIX_MCR, &mcr);
    if (ret != FT_SUCCESS)
    {
        FPCIE_MSIX_DEBUG_E("Failed to read MSIX MCR for device %02x:%02x.%01x\n",
                           msix->bus, msix->device, msix->function);
        return ret;
    }
    FPCIE_MSIX_DEBUG_I("before mcr is %08x\n", mcr);
    mcr = 0;

    ret = FPcieEcamWriteConfigSpace(instance_p, msix->bus, msix->device, msix->function,
                                    msix->base + FPCIE_MSIX_MCR, mcr);
    if (ret != FT_SUCCESS)
    {
        FPCIE_MSIX_DEBUG_E("Failed to write MSIX MCR for device %02x:%02x.%01x\n",
                           msix->bus, msix->device, msix->function);
        return ret;
    }

    for (fsize_t i = 0; i < msix->msix_vector_num; i++)
    {
        FPCIE_WRITEECAM_REG((uintptr)&msix->msix[i]->msg_addr, 0, 0);
        FPCIE_WRITEECAM_REG((uintptr)&msix->msix[i]->msg_up_addr, 0, 0);
        FPCIE_WRITEECAM_REG((uintptr)&msix->msix[i]->msg_data, 0, 0);
        FPCIE_WRITEECAM_REG((uintptr)&msix->msix[i]->vector_ctrl, 0, 0);
    }

    return ret;
}

/**
 * @name: FPcieEcamMsixAllocate
 * @msg: Allocate MSI-X interrupt vector resources for a PCIe device
 * @param instance_p: Pointer to the PCIe ECAM instance
 * @param bus: PCIe bus number
 * @param device: PCIe device number
 * @param function: PCIe function number
 * @param msix: Pointer to structure storing MSI-X vector information
 * @param n_vector: Number of vectors to allocate
 * @return: Actual number of vectors allocated (0 indicates failure)
 * @note:
 *  1. Full allocation workflow: capability detection → table size query → 
 *     entry retrieval → interrupt controller setup
 */
u16 FPcieEcamMsixAllocate(FPcieEcam *instance_p, u8 bus, u8 device, u8 function,
                          FPcieMsix *msix, u16 n_vector)
{
    u32 base;

    base = FPcieEcamGetCapability(instance_p, bus, device, function, FPCI_CAP_ID_MSIX);
    FPCIE_MSIX_DEBUG_I("base = %08x\n", base);
    if (base == 0)
    {
        FASSERT_MSG(0, "Failed to get MSIX capability for device %02x:%02x.%01x\n", bus,
                    device, function);
        return 0;
    }
    msix->base = base;
    msix->msix_vector_max_num = FPcieEcamGetMsixTableSize(instance_p, bus, device, function, base);

    if (FPcieEcamGetMsixTableEntry(instance_p, bus, device, function, base, msix, n_vector) != FT_SUCCESS)
    {
        FPCIE_MSIX_DEBUG_E("Failed to get MSIX table entry for device %02x:%02x.%01x\n",
                           bus, device, function);
        return 0;
    }

    msix->ecam_instance = instance_p;
    return FPcieEcamMsixDeviceSetup(msix);
}
