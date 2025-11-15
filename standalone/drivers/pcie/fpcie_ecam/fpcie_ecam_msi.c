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
 * Last Modified: 2025-07-15 11:43:43
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2024-08-09   first version
 */

#include "fpcie_ecam.h"
#include "fpcie_ecam_common.h"
#include "fpcie_ecam_msi.h"
#include "fgic_its.h"
#include "finterrupt.h"

#define FPCIE_DEBUG_TAG "FPCIE"
#define FPCIE_DEBUG_D(format, ...) \
    FT_DEBUG_PRINT_D(FPCIE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_DEBUG_I(format, ...) \
    FT_DEBUG_PRINT_I(FPCIE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_DEBUG_W(format, ...) \
    FT_DEBUG_PRINT_W(FPCIE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPCIE_DEBUG_E(format, ...) \
    FT_DEBUG_PRINT_E(FPCIE_DEBUG_TAG, format, ##__VA_ARGS__)

#define FPCI_DEVID(bus, dev, fn) \
    ((((bus)&0xff) << 8) | (((dev)&0x1f) << 3) | ((fn)&0x07))

/**
 * @name: FPcieEcamMsiMMc
 * @msg: Reads the MSI Message Control Register (MCR) to determine the Multiple Message Capable (MMC) value.
 * @param instance_p: Pointer to the FPcieEcam instance.
 * @param bus: PCI bus number.
 * @param device: PCI device number.
 * @param function: PCI function number.
 * @param base: Base address of the MSI capability structure.
 * @return: The MMC value, which is 2^(MMC field).
 * @note: Returns 0 if there is an error reading the MCR.
 */
static u32 FPcieEcamMsiMmc(FPcieEcam *instance_p, u8 bus, u8 device, u8 function, u32 base)
{
    u32 mcr;
    FError ret;
    ret = FPcieEcamReadConfigSpace(instance_p, bus, device, function, base + FPCIE_MSI_MCR, &mcr);
    if (ret)
    {
        return 0;
    }
    FPCIE_DEBUG_I("mcr = %08x\n", mcr);
    /* Getting MMC true count: 2^(MMC field) */
    /* Get Multiple Message Capable  */
    return 1 << ((mcr & FPCIE_MSI_MCR_MMC) >> FPCIE_MSI_MCR_MMC_SHIFT);
}

/**
 * @name: FPcieEcamMsiDeviceSetup
 * @msg: Sets up MSI vectors for a given PCI device.
 * @param vectors: Pointer to an array of FPcieMsiVector structures.
 * @param count: Number of MSI vectors to setup.
 * @return: Number of successfully setup vectors, 0 if there was an error.
 * @note: Initializes the device in the GIC and allocates LPI IDs.
 */
int FPcieEcamMsiDeviceSetup(FPcieMsiVector *vectors, u8 count)
{
    u32 device_id;
    u8 i;
    int ret;
    FGic *gic_instance_p;
    gic_instance_p = (FGic *)InterruptGetInstance();
    FASSERT_MSG(gic_instance_p, "Failed to get interrupt instance\n");
    FASSERT_MSG(vectors, "Invalid vectors pointer\n");

    if (!count)
    {
        FPCIE_DEBUG_E("No vectors to setup\n");
        return 0;
    }

    device_id = FPCI_DEVID(vectors->bus, vectors->device, vectors->function);
    FPCIE_DEBUG_I("device_id = %08x\n", device_id);
    ret = FGicItsDeviceIdInit(gic_instance_p, device_id, count);
    if (ret)
    {
        FPCIE_DEBUG_E("Failed to initialize device id %08x\n", device_id);
        return 0;
    }

    vectors->device_id = device_id;
    for (i = 0; i < count; i++)
    {
        vectors->msi[i].irq = FGicItsLpiIdAlloc(gic_instance_p);
        if (!vectors->msi[i].irq)
        {
            FASSERT_MSG(0, "Failed to allocate interrupt id for device id %08x\n", device_id)
            return 0;
        }
        vectors->msi[i].address = FGicItsGetTransAddr(gic_instance_p); /* Get message address register */
        vectors->msi[i].eventid = i;
        FPCIE_DEBUG_I("vectors->msi[%d].irq = %d, vectors->msi[%d].address = %08x, "
                      "vectors->msi[%d].eventid = %d\n",
                      i, vectors->msi[i].irq, i, vectors->msi[i].address, i,
                      vectors->msi[i].eventid);
        ret = FGicItsMapIntId(gic_instance_p, device_id, i, vectors->msi[i].irq);
        if (ret)
        {
            FASSERT_MSG(0, "Failed to map interrupt id %d for device id %08x\n", i, device_id);
            return 0;
        }
    }
    vectors->msi_num = count;
    return count;
}

/**
 * @name: FPcieEcamVectorsAllocate
 * @msg: Allocates MSI vectors for a given PCI device.
 * @param instance_p: Pointer to the FPcieEcam instance.
 * @param bus: PCI bus number.
 * @param device: PCI device number.
 * @param function: PCI function number.
 * @param vectors: Pointer to an FPcieMsiVector structure to store the allocated vectors.
 * @return: Number of allocated MSI vectors, 0 if there was an error.
 * @note: Determines the MSI capability and allocates the required number of vectors.
 */
u8 FPcieEcamVectorsAllocate(FPcieEcam *instance_p, u8 bus, u8 device, u8 function,
                            FPcieMsiVector *vectors)
{
    u32 base;
    u32 req_vectors;
    u32 count = FPCIE_MSI_MAX_MESSAGE_NUM;
    FASSERT_MSG(vectors, "Invalid vectors pointer\n");
    FASSERT_MSG(instance_p, "Invalid instance pointer\n");
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);


    base = FPcieEcamGetCapability(instance_p, bus, device, function, FPCI_CAP_ID_MSI);
    FPCIE_DEBUG_I("base = %08x\n", base);
    if (base == 0)
    {
        FASSERT_MSG(0, "Failed to get MSI capability for device %02x:%02x.%01x\n", bus,
                    device, function);
        return 0;
    }

    req_vectors = FPcieEcamMsiMmc(instance_p, bus, device, function, base);

    FPCIE_DEBUG_I("req_vectors = %d, count = %d\n", req_vectors, count);

    if (count > req_vectors)
    {
        count = req_vectors;
    }

    vectors->bus = bus;
    vectors->device = device;
    vectors->function = function;

    return FPcieEcamMsiDeviceSetup(vectors, count);
}

/**
 * @name: FPcieEcamVectorsFree
 * @msg: Frees allocated MSI vectors for a given PCI device.
 * @param instance_p: Pointer to the FPcieEcam instance.
 * @param vectors: Pointer to the FPcieMsiVector structure containing the allocated vectors.
 * @return: None.
 * @note: Deinitializes the device in the GIC and frees LPI IDs.
 */
void FPcieEcamVectorsFree(FPcieEcam *instance_p, FPcieMsiVector *vectors)
{
    u32 device_id;
    FGic *gic_instance_p = NULL;
    FError ret;
    int i;
    u8 count = vectors->msi_num;
    gic_instance_p = (FGic *)InterruptGetInstance();
    FASSERT_MSG(vectors, "Invalid vectors pointer\n");
    FASSERT_MSG(instance_p, "Invalid instance pointer\n");


    device_id = FPCI_DEVID(vectors->bus, vectors->device, vectors->function);
    ret = FGicItsDeviceIdDeinit(gic_instance_p, device_id);
    if (ret)
    {
        FPCIE_DEBUG_E("Failed to deinit device id %d\n", device_id);
        return;
    }

    for (i = 0; i < count; i++)
    {
        FGicItsLpiIdFree(gic_instance_p, vectors->msi[i].irq);
    }
}

/**
 * @name: FPcieEcamMsiSetup
 * @msg: Configures the MSI capability for a given PCI device.
 * @param instance_p: Pointer to the FPcieEcam instance.
 * @param vectors: Pointer to the FPcieMsiVector structure containing the MSI vectors.
 * @param base: Base address of the MSI capability structure.
 * @return: None.
 * @note: Writes the MSI configuration space registers to enable MSI.
 */
static void FPcieEcamMsiSetup(FPcieEcam *instance_p, FPcieMsiVector *vectors, u32 base)
{
    u32 mcr; /* Message Control Register */
    u32 trans_phys, temp_phys;
    u32 mdr; /*   Message Data Register */
    u32 mme; /* Message Mask Enable Register */
    FError ret;
    u8 count = vectors->msi_num;
    trans_phys = vectors->msi[0].address;

    FPcieEcamWriteConfigSpace(instance_p, vectors->bus, vectors->device,
                              vectors->function, base + FPCIE_MSI_MAP0, trans_phys);
    FPcieEcamReadConfigSpace(instance_p, vectors->bus, vectors->device,
                             vectors->function, base + FPCIE_MSI_MAP0, &temp_phys);
    mdr = vectors->msi[0].eventid;
    ret = FPcieEcamReadConfigSpace(instance_p, vectors->bus, vectors->device,
                                   vectors->function, base + FPCIE_MSI_MCR, &mcr);
    if (ret)
    {
        FASSERT_MSG(0, "Failed to read MSI MCR register");
        return;
    }
    if (mcr & FPCIE_MSI_MCR_64)
    {
        FPcieEcamWriteConfigSpace(instance_p, vectors->bus, vectors->device,
                                  vectors->function, base + FPCIE_MSI_MAP1_64, 0);
        FPcieEcamWriteConfigSpace(instance_p, vectors->bus, vectors->device,
                                  vectors->function, base + FPCIE_MSI_MDR_64, mdr);
    }
    else
    {
        FPcieEcamWriteConfigSpace(instance_p, vectors->bus, vectors->device,
                                  vectors->function, base + FPCIE_MSI_MDR_32, mdr);
    }

    for (mme = 0; count > 1; mme++)
    {
        count >>= 1;
    }

    mcr |= mme << FPCIE_MSI_MCR_MME_SHIFT; /* massage enable */

    mcr |= FPCIE_MSI_MCR_EN;
    ret = FPcieEcamWriteConfigSpace(instance_p, vectors->bus, vectors->device,
                                    vectors->function, base + FPCIE_MSI_MCR, mcr);
    ret = FPcieEcamReadConfigSpace(instance_p, vectors->bus, vectors->device,
                                   vectors->function, base + FPCIE_MSI_MCR, &temp_phys);
}

static void FPcieEcamMsiDeinit(FPcieEcam *instance_p, FPcieMsiVector *vectors, u32 base)
{
    FPcieEcamWriteConfigSpace(instance_p, vectors->bus, vectors->device,
                              vectors->function, base + FPCIE_MSI_MCR, 0);
}


/**
 * @name: FPcieEcamMsiEnable
 * @msg: Enables MSI for a given PCI device.
 * @param instance_p: Pointer to the FPcieEcam instance.
 * @param vectors: Pointer to the FPcieMsiVector structure containing the MSI vectors.
 * @return: 0 if successful, -1 if there was an error.
 * @note: Retrieves the MSI capability base and sets up the MSI.
 */
u8 FPcieEcamMsiEnable(FPcieEcam *instance_p, FPcieMsiVector *vectors)
{
    u32 base;
    u8 i = 0;
    FASSERT_MSG(vectors, "Invalid vectors pointer\n");
    FASSERT_MSG(instance_p, "Invalid instance pointer\n");
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    base = FPcieEcamGetCapability(instance_p, vectors->bus, vectors->device,
                                  vectors->function, FPCI_CAP_ID_MSI);
    FPCIE_DEBUG_I("base = %08x\n", base);
    if (base == 0)
    {
        FPCIE_DEBUG_E("Failed to get MSI capability for device %02x:%02x.%01x\n",
                      vectors[i].bus, vectors[i].device, vectors[i].function);
        return -1;
    }

    FPcieEcamMsiSetup(instance_p, vectors, base);

    return 0;
}

u8 FPcieEcamMsiDisable(FPcieEcam *instance_p, FPcieMsiVector *vectors)
{
    u32 base;
    u8 i = 0;
    FASSERT_MSG(vectors, "Invalid vectors pointer\n");
    FASSERT_MSG(instance_p, "Invalid instance pointer\n");
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);

    base = FPcieEcamGetCapability(instance_p, vectors->bus, vectors->device,
                                  vectors->function, FPCI_CAP_ID_MSI);
    FPCIE_DEBUG_I("base = %08x\n", base);
    if (base == 0)
    {
        FPCIE_DEBUG_E("Failed to get MSI capability for device %02x:%02x.%01x\n",
                      vectors[i].bus, vectors[i].device, vectors[i].function);
        return -1;
    }

    FPcieEcamMsiDeinit(instance_p, vectors, base);

    return 0;
}