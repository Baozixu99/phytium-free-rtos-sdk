/*
 * Copyright : (C) 2024 Phytium Information Technology, Inc.
 * All Rights Reserved.
 * 
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,
 * either version 1.0 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details.
 * 
 * 
 * FilePath: memory_layout.h
 * Created Date: 2024-04-29 14:22:47
 * Last Modified: 2024-07-22 17:01:37
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

#define SLAVE00_SOURCE_TABLE_ATTRIBUTE   (MT_NORMAL|MT_P_RW_U_RW)
#define SLAVE00_SHARE_BUFFER_ATTRIBUTE   (MT_NORMAL|MT_P_RW_U_RW)

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_LAYOUT_H */
