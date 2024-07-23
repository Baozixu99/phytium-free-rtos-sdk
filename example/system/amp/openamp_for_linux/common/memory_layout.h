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
 * Last Modified: 2024-07-22 17:02:12
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0     liusm   2024-05-17        first version
 */

#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H



#if defined __cplusplus
extern "C" {
#endif

/*slave core0*/
/* 与linux共享的内存 */
#define SLAVE00_SOURCE_TABLE_ADDR        0xc0000000 /*与linux协商好的地址*/
#define SLAVE00_KICK_IO_ADDR             0xc0224000

/* MEM = |tx vring|rx vring|share buffer| */
#define SLAVE00_SHARE_MEM_ADDR           0xffffffff /*全F表示等待linux分配*/
#define SLAVE00_SHARE_MEM_SIZE           0x100000 /*共享内存大小*/
#define SLAVE00_VRING_SIZE               0x8000UL
#define SLAVE00_VRING_NUM                0x100
#define SLAVE00_TX_VRING_ADDR            0xffffffff /*全F表示等待linux分配*/
#define SLAVE00_RX_VRING_ADDR            0xffffffff /*全F表示等待linux分配*/

#define SLAVE00_SOURCE_TABLE_ATTRIBUTE   (MT_NORMAL|MT_P_RW_U_RW)
#define SLAVE00_SHARE_BUFFER_ATTRIBUTE   (MT_NORMAL|MT_P_RW_U_RW)

#if defined __cplusplus
}
#endif

#endif /* MEMORY_LAYOUT_H */
