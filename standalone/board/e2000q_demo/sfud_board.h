/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fgic_v3.h
 * Date: 2023-10-7 09:30:29
 * LastEditTime: 2023-10-7 09:30:29
 * Description: This file is for detailed description of the device and driver.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0   liyilun   2023/03/21         first release
 */
#ifndef  SFUD_BOARD_H
#define  SFUD_BOARD_H

#include "fparameters.h"


#define FSPIM0_SFUD_NAME "FSPIM0"
#define FSPIM1_SFUD_NAME "FSPIM1"
#define FSPIM2_SFUD_NAME "FSPIM2"
#define FSPIM3_SFUD_NAME "FSPIM3"
#define FQSPI0_SFUD_NAME "FQSPI0"

enum {
    SFUD_FSPIM2_INDEX = 0,
    SFUD_FQSPI0_INDEX = 1,
    SFUD_DEVICE_INDEX
};

#define SFUD_FLASH_DEVICE_TABLE_BOARD                                               	\
{                                                                                   \
        [SFUD_FSPIM2_INDEX] = {.name = "SPI2-FLASH", .spi.name = FSPIM2_SFUD_NAME}, \
        [SFUD_FQSPI0_INDEX] = {.name = "QSPI0-FLASH", .spi.name = FQSPI0_SFUD_NAME} \
}

#endif 
