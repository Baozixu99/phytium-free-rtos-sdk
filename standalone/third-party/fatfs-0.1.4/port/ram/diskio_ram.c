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
 * FilePath: diskio_ram.c
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This file is for fatfs port to ramdisk
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/3   init commit
 */

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "fdebug.h"
#include "fkernel.h"
#include "diskio.h"
#include "ffconf.h"
#include "ff.h"
#include "fmmu.h"

#define FF_DEBUG_TAG          "DISKIO-RAM"
#define FF_ERROR(format, ...) FT_DEBUG_PRINT_E(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_INFO(format, ...)  FT_DEBUG_PRINT_I(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_DEBUG(format, ...) FT_DEBUG_PRINT_D(FF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FF_WARN(format, ...)  FT_DEBUG_PRINT_W(FF_DEBUG_TAG, format, ##__VA_ARGS__)

typedef struct
{
    void *base;
    DWORD sector_sz;
    DWORD sector_cnt;
    u32 mem_attr;
    BYTE pdrv;
} ff_ram_disk;

static ff_ram_disk ram_disk = {.base = (void *)(uintptr)CONFIG_FATFS_RAM_DISK_BASE,
                               .sector_sz = CONFIG_FATFS_RAM_DISK_SECTOR_SIZE_BYTE,
                               .sector_cnt = CONFIG_FATFS_RAM_DISK_SIZE_MB * SZ_1M /
                                             CONFIG_FATFS_RAM_DISK_SECTOR_SIZE_BYTE,
                               .mem_attr = (MT_NORMAL | MT_P_RW_U_RW | MT_NS),
                               .pdrv = FF_DRV_NOT_USED};

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

static DSTATUS ram_disk_status(BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    ff_ram_disk *disk = &ram_disk;

    if (FF_DRV_NOT_USED == disk->pdrv)
    {
        return STA_NOINIT;
    }

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

static DSTATUS ram_disk_initialize(BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

static DRESULT ram_disk_read(BYTE pdrv, /* Physical drive nmuber to identify the drive */
                             BYTE *buff,   /* Data buffer to store read data */
                             DWORD sector, /* Start sector in LBA */
                             UINT count    /* Number of sectors to read */
)
{
    ff_ram_disk *disk = &ram_disk;

    if ((FF_DRV_NOT_USED != disk->pdrv) && (sector < disk->sector_cnt))
    {
        memcpy(buff, disk->base + sector * disk->sector_sz, count * disk->sector_sz);
        return RES_OK;
    }

    return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

static DRESULT ram_disk_write(BYTE pdrv, /* Physical drive nmuber to identify the drive */
                              const BYTE *buff, /* Data to be written */
                              DWORD sector,     /* Start sector in LBA */
                              UINT count        /* Number of sectors to write */
)
{
    ff_ram_disk *disk = &ram_disk;

    if ((FF_DRV_NOT_USED != disk->pdrv) && (sector < disk->sector_cnt))
    {
        memcpy(disk->base + sector * disk->sector_sz, buff, count * disk->sector_sz);
        return RES_OK;
    }

    return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT ram_disk_ioctl(BYTE pdrv, /* Physical drive nmuber (0..) */
                       BYTE cmd,  /* Control code */
                       void *buff /* Buffer to send/receive control data */
)
{
    DRESULT res;
    ff_ram_disk *disk = &ram_disk;

    res = RES_PARERR;
    if (FF_DRV_NOT_USED == disk->pdrv)
    {
        return res;
    }

    switch (cmd)
    {
        case CTRL_SYNC: /* Nothing to do */
            res = RES_OK;
            break;

        case GET_SECTOR_COUNT: /* Get number of sectors on the drive */
            *(DWORD *)buff = disk->sector_cnt;
            res = RES_OK;
            break;

        case GET_SECTOR_SIZE: /* Get size of sector for generic read/write */
            *(WORD *)buff = disk->sector_sz;
            res = RES_OK;
            break;

        case GET_BLOCK_SIZE:
            *(DWORD *)buff = 1; /* This is not flash storage that can be erase by command, return 1 */
            res = RES_OK;
            break;
    }

    return res;
}

static const ff_diskio_driver_t ram_disk_drv = {.init = &ram_disk_initialize,
                                                .status = &ram_disk_status,
                                                .read = &ram_disk_read,
                                                .write = &ram_disk_write,
                                                .ioctl = &ram_disk_ioctl};

void ff_diskio_register_ram(BYTE pdrv)
{
    ff_ram_disk *disk = &ram_disk;
    FMmuMap((uintptr_t)ram_disk.base, (uintptr_t)ram_disk.base,
            ram_disk.sector_cnt * ram_disk.sector_sz, ram_disk.mem_attr);
    disk->pdrv = pdrv; /* assign volume for ram disk */
    ff_diskio_register(pdrv, &ram_disk_drv);

    printf("Create ram disk @[0x%p ~ 0x%p] as driver-%d\r\n", disk->base,
           disk->base + disk->sector_sz * disk->sector_cnt, disk->pdrv);
}