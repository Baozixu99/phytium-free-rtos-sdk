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
 * Last Modified: 2025-08-05 14:27:39
 * Description:  This file is for for msi-x driver
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe     2025-07-15   first version
 */


#ifndef FPCIE_ECAM_MSIX_H
#define FPCIE_ECAM_MSIX_H

#include "ftypes.h"
#include "fpcie_ecam.h"
#include "fpcie_ecam_msi.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FPCIE_MSIX_MAX_MESSAGE_NUM  2048


#define FPCIE_MSIX_MCR              0U
#define FPCIE_MSIX_MCR_EN           0x80000000U /* Enable MSI-X */
#define FPCIE_MSIX_MCR_FMASK        0x40000000U /* Function Mask */
#define FPCIE_MSIX_MCR_TSIZE        0x07FF0000U /* Table size mask */
#define FPCIE_MSIX_MCR_TSIZE_SHIFT  16
#define FPCIE_MSIX_TABLE_ENTRY_SIZE 16

#define FPCIE_MSIX_TR               1U
#define FPCIE_MSIX_TR_BIR           0x00000007U /* Table BIR mask */
#define FPCIE_MSIX_TR_OFFSET        0xFFFFFFF8U /* Offset mask */
#define FPCIE_MSIX_PBA              2U
#define FPCIE_MSIX_PBA_BIR          0x00000007U /* PBA BIR mask */
#define FPCIE_MSIX_PBA_OFFSET       0xFFFFFFF8U /* Offset mask */

struct FPcieMsixVectorTable
{
    u32 msg_addr;
    u32 msg_up_addr;
    u32 msg_data;
    u32 vector_ctrl;
} __packed;

struct FPcieMsixVector
{
    u32 irq;
    u32 address;
    u16 eventid;
};


typedef struct
{
    FPcieEcam *ecam_instance;
    u8 bus;
    u8 device;
    u8 function;
    u32 base;
    u32 device_id;
    u16 msix_vector_max_num;
    u16 msix_vector_num;
    struct FPcieMsixVectorTable *msix[FPCIE_MSIX_MAX_MESSAGE_NUM];
    struct FPcieMsixVector vector[FPCIE_MSIX_MAX_MESSAGE_NUM];
} FPcieMsix;

u16 FPcieEcamMsixAllocate(FPcieEcam *instance_p, u8 bus, u8 device, u8 function,
                          FPcieMsix *msix, u16 n_vector);
FError FPcieEcamMsixEnable(FPcieEcam *instance_p, FPcieMsix *msix);
u8 FPcieEcamMsixDisableMsi(FPcieEcam *instance_p, FPcieMsix *msix);
u16 FPcieEcamMsixDeviceSetup(FPcieMsix *msix);
FError FPcieEcamMsixDisable(FPcieEcam *instance_p, FPcieMsix *msix);

#ifdef __cplusplus
}
#endif


#endif