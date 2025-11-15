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
 * FilePath: sfud_board.h
 * Created Date: 2024-03-21:00:53
 * Last Modified: 2025-06-04 15:02:32
 * Description:  This file is for recording the sfud spim config on the e2000q-demo board.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liyilun   2023/03/21         first release
 */

#ifndef  SFUD_BOARD_H
#define  SFUD_BOARD_H

#define FSPIM0_SFUD_NAME "FSPIM0"
#define FSPIM1_SFUD_NAME "FSPIM1"

#define FQSPI0_SFUD_NAME "FQSPI0"

enum 
{
    SFUD_FSPIM0_INDEX = 0,
    SFUD_FSPIM1_INDEX = 1,
    SFUD_FQSPI0_INDEX,
    SFUD_DEVICE_INDEX
};

#define SFUD_FLASH_DEVICE_TABLE_BOARD                                                 \
{                                                                               \
    [SFUD_FSPIM0_INDEX] = {.name = "SPI0-FLASH", .spi.name = FSPIM0_SFUD_NAME}, \
    [SFUD_FSPIM1_INDEX] = {.name = "SPI1-FLASH", .spi.name = FSPIM1_SFUD_NAME}, \
    [SFUD_FQSPI0_INDEX] = {.name = "QSPI0-FLASH", .spi.name = FQSPI0_SFUD_NAME} \
}

#endif 
