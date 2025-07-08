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
 * FilePath: memory_layout.h
 * Created Date: 2024-04-29 14:22:47
 * Last Modified: 2025-06-20 16:21:01
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     liusm   2024-05-17        first version
 */

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H



#ifdef __cplusplus
extern "C"
{
#endif

/*slave core0*/
#define SLAVE00_IMAGE_NUM                0          /*镜像位置与amp_config.json中编译排序对应*/
#define SLAVE00_SOURCE_TABLE_ADDR        0xc0000000
#define SLAVE00_SOURCE_TABLE_SIZE        SLAVE00_KICK_IO_ADDR - SLAVE00_SOURCE_TABLE_ADDR
#define SLAVE00_KICK_IO_ADDR             0xc0020000
#define SLAVE00_KICK_IO_SIZE             SLAVE00_SHARE_MEM_ADDR - SLAVE00_KICK_IO_ADDR
/* MEM = |tx vring|rx vring|share buffer| */
#define SLAVE00_SHARE_MEM_ADDR           0xc1000000
#define SLAVE00_SHARE_MEM_SIZE           0x1000000
#define SLAVE00_VRING_SIZE               (SLAVE00_SHARE_MEM_SIZE >> 2)
#define SLAVE00_VRING_NUM                0x100
#define SLAVE00_TX_VRING_ADDR            0xc1000000
#define SLAVE00_RX_VRING_ADDR            (SLAVE00_TX_VRING_ADDR + SLAVE00_VRING_SIZE/2)

#define SLAVE00_SOURCE_TABLE_ATTRIBUTE   (MT_NORMAL|MT_P_RW_U_NA)
#define SLAVE00_SHARE_BUFFER_ATTRIBUTE   (MT_NORMAL|MT_P_RW_U_NA)

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_LAYOUT_H */
