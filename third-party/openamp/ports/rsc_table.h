/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc.
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
 * FilePath: rsc_table.h
 * Date: 2022-02-23 11:24:12
 * LastEditTime: 2022-02-23 11:44:06
 * Description:  This file populates resource table for BM remote 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0	 huanghe	2022/03/25   first release
 */


#ifndef RSC_TABLE_H
#define RSC_TABLE_H

/***************************** Include Files *********************************/

#include <stddef.h>
#include <openamp/open_amp.h>

#if defined __cplusplus
extern "C" {
#endif

#define NO_RESOURCE_ENTRIES         8

/* Place resource table in special ELF section */
#define __section_t(S)          __attribute__((__section__(#S)))
#define __resource              __section_t(.resource_table)

#define RPMSG_IPU_C0_FEATURES        1

/* VirtIO rpmsg device id */
#define VIRTIO_ID_RPMSG_             7

/* notifyid is a unique rproc-wide notify index for this vdev */
#define VDEV_NOTIFYID				0
/* Remote supports Name Service announcement */
#define VIRTIO_RPMSG_F_NS           0

#define NUM_VRINGS                  0x02
#define VRING_ALIGN                 0x1000
#define RING_TX                     CONFIG_VRING_TX_ADDR
#define RING_RX                     CONFIG_VRING_RX_ADDR

#define NUM_TABLE_ENTRIES           1

/**************************** Type Definitions *******************************/

/* Resource table for the given remote */
struct remote_resource_table
{
    unsigned int version;
    unsigned int num;
    unsigned int reserved[2];
    unsigned int offset[NO_RESOURCE_ENTRIES];
    /* rpmsg vdev entry */
    struct fw_rsc_vdev rpmsg_vdev;
    struct fw_rsc_vdev_vring rpmsg_vring0;
    struct fw_rsc_vdev_vring rpmsg_vring1;
} __attribute__((packed, aligned(0x1000)));


#if defined __cplusplus
}
#endif

#endif // !
