/*
 * 
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
 * 
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 * 
 *      https://opensource.org/licenses/BSD-3-Clause
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * 
 * FilePath: nvme_disk.h
 * Created Date: 2025-07-16 11:16:18
 * Last Modified: 2025-07-22 14:17:37
 * Description:  This file is for 
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0    huanghe	2025-07-23		first version
 */



#ifndef NVME_DISH_H
#define NVME_DISH_H

#include "dlist.h"

/**
 * @brief Possible Cmd Codes for disk_ioctl()
 */

/** Get the number of sectors in the disk  */
#define DISK_IOCTL_GET_SECTOR_COUNT		1
/** Get the size of a disk SECTOR in bytes */
#define DISK_IOCTL_GET_SECTOR_SIZE		2
/** reserved. It used to be DISK_IOCTL_GET_DISK_SIZE */
#define DISK_IOCTL_RESERVED			3
/** How many  sectors constitute a FLASH Erase block */
#define DISK_IOCTL_GET_ERASE_BLOCK_SZ		4
/** Commit any cached read/writes to disk */
#define DISK_IOCTL_CTRL_SYNC			5
/** Initialize the disk. This IOCTL must be issued before the disk can be
 * used for I/O. It is reference counted, so only the first successful
 * invocation of this macro on an uninitialized disk will initialize the IO
 * device
 */
#define DISK_IOCTL_CTRL_INIT			6
/** Deinitialize the disk. This IOCTL can be used to de-initialize the disk,
 * enabling it to be removed from the system if the disk is hot-pluggable.
 * Disk usage is reference counted, so for a given disk the
 * `DISK_IOCTL_CTRL_DEINIT` IOCTL must be issued as many times as the
 * `DISK_IOCTL_CTRL_INIT` IOCTL was issued in order to de-initialize it.
 *
 * This macro optionally accepts a pointer to a boolean as the `buf` parameter,
 * which if true indicates the disk should be forcibly stopped, ignoring all
 * reference counts. The disk driver must report success if a forced stop is
 * requested, but this operation is inherently unsafe.
 */
#define DISK_IOCTL_CTRL_DEINIT			7

/**
 * @brief Possible return bitmasks for disk_status()
 */

/** Disk status okay */
#define DISK_STATUS_OK			0x00
/** Disk status uninitialized */
#define DISK_STATUS_UNINIT		0x01
/** Disk status no media */
#define DISK_STATUS_NOMEDIA		0x02
/** Disk status write protected */
#define DISK_STATUS_WR_PROTECT		0x04


/**
 * @brief Disk info
 */
struct disk_info {
	/** Internally used list node */
	sys_dnode_t node;
	/** Disk name */
	const char *name;
	/** Device associated to this disk */
	/** Internally used disk reference count */
	uint16_t refcnt;

	void *ns ;
};

int nvme_disk_init(struct disk_info *disk,void *ns) ;
int nvme_disk_read(struct disk_info *disk,
			  uint8_t *data_buf,
			  uint32_t start_sector,
			  uint32_t num_sector) ;

int nvme_disk_status(struct disk_info *disk) ;
int nvme_disk_flush(struct disk_info *disk) ;
int nvme_disk_ioctl(struct disk_info *disk, uint8_t cmd, void *buff) ;
int nvme_disk_write(struct disk_info *disk,
			   const uint8_t *data_buf,
			   uint32_t start_sector,
			   uint32_t num_sector) ;


#endif // !