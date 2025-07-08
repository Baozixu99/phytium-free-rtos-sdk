/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fatfs_examples.h
 * Date: 2022-06-17 10:42:40
 * LastEditTime: 2022-06-17 10:42:40
 * Description:  This file is for the fatfs test example function declarations.
 *
 * Modify History:
 *  Ver   Who         Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/7    init commit
 */

#ifndef FATFS_EXAMPLES_H
#define FATFS_EXAMPLES_H

#include "ftypes.h"
#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum
{
    FFREERTOS_FATFS_RAM_DISK =   0U,
    FFREERTOS_FATFS_TF_CARD =    1U,
    FFREERTOS_FATFS_EMMC_CARD =  2U,
    FFREERTOS_FATFS_USB_DISK =   3U,
    FFREERTOS_FATFS_SATA_DISK =   4U,
    FFREERTOS_FATFS_SATA_PCIE_DISK =   5U,
    FFREERTOS_DISK_TYPE_NUM,
};

/* fatfs run */
BaseType_t FFreeRTOSFatfsTest(void);

void FatfsExampleEntry(void);
#ifdef __cplusplus
}
#endif

#endif // !