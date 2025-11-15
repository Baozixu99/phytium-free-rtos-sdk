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
 * FilePath: fpcie_ecam_msi.h
 * Created Date: 2024-07-30 17:59:56
 * Last Modified: 2025-07-15 11:43:46
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2024-08-09   first version
 */


#ifndef FPCIE_ECAM_MSI_H
#define FPCIE_ECAM_MSI_H

#include "ftypes.h"
#include "fpcie_ecam.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FPCIE_MSI_MAX_MESSAGE_NUM   32
/*
 * The first word of the MSI capability is shared with the
 * capability ID and list link.  The high 16 bits are the MCR.
 */

#define FPCIE_MSI_MCR               0U

#define FPCIE_MSI_MCR_EN            0x00010000U /* enable MSI */
#define FPCIE_MSI_MCR_MMC           0x000E0000U /* Multi Messages Capable mask */
#define FPCIE_MSI_MCR_MMC_SHIFT     17
#define FPCIE_MSI_MCR_MME           0x00700000U /* mask of # of enabled IRQs */
#define FPCIE_MSI_MCR_MME_SHIFT     20
#define FPCIE_MSI_MCR_64            0x00800000U /* 64-bit MSI */

/*
 * The MAP follows the MCR. If FPCIE_MSI_MCR_64, then the MAP
 * is two words long. The MDR follows immediately after the MAP.
 */

#define FPCIE_MSI_MAP0              1U
#define FPCIE_MSI_MAP1_64           2U
#define FPCIE_MSI_MDR_32            2U
#define FPCIE_MSI_MDR_64            3U

#define FPCIE_MSIX_MCR              0U

#define FPCIE_MSIX_MCR_EN           0x80000000U /* Enable MSI-X */
#define FPCIE_MSIX_MCR_FMASK        0x40000000U /* Function Mask */
#define FPCIE_MSIX_MCR_TSIZE        0x07FF0000U /* Table size mask */
#define FPCIE_MSIX_MCR_TSIZE_SHIFT  16
#define FPCIE_MSIR_TABLE_ENTRY_SIZE 16

#define FPCIE_MSIX_TR               1U
#define FPCIE_MSIX_TR_BIR           0x00000007U /* Table BIR mask */
#define FPCIE_MSIX_TR_OFFSET        0xFFFFFFF8U /* Offset mask */

#define FPCIE_MSIX_PBA              2U
#define FPCIE_MSIX_PBA_BIR          0x00000007U /* PBA BIR mask */
#define FPCIE_MSIX_PBA_OFFSET       0xFFFFFFF8U /* Offset mask */

#define PCIE_VTBL_MA                0U  /* Msg Address offset */
#define PCIE_VTBL_MUA               4U  /* Msg Upper Address offset */
#define PCIE_VTBL_MD                8U  /* Msg Data offset */
#define PCIE_VTBL_VCTRL             12U /* Vector control offset */


struct FPcieMsiVectorGeneric
{
    u32 irq;
    u32 address;
    u16 eventid;
};

typedef struct
{
    u8 bus;
    u8 device;
    u8 function;
    u32 device_id;
    u8 msi_num;
    struct FPcieMsiVectorGeneric msi[FPCIE_MSI_MAX_MESSAGE_NUM];
} FPcieMsiVector;

u8 FPcieEcamVectorsAllocate(FPcieEcam *instance_p, u8 bus, u8 device, u8 function,
                            FPcieMsiVector *vectors);
u8 FPcieEcamMsiEnable(FPcieEcam *instance_p, FPcieMsiVector *vectors);
u8 FPcieEcamMsiDisable(FPcieEcam *instance_p, FPcieMsiVector *vectors);
void FPcieEcamVectorsFree(FPcieEcam *instance_p, FPcieMsiVector *vectors);
int FPcieEcamMsiDeviceSetup(FPcieMsiVector *vectors, u8 count);

/* msix */
u32 FPcieEcamGetMsiXBase(FPcieEcam *instance_p, u8 bus, u8 device, u8 function);

#ifdef __cplusplus
}
#endif


#endif