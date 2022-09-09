/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <string.h>
#include "parameters.h"
#include "ft_debug.h"
#include "interrupt.h"
#include "ff.h"
#include "diskio.h"		/* FatFs lower layer API */
#include "fsata.h"
#include "fsata_hw.h"

#define HOST_NUM    0  /* sata host */
#define PORT_NUM    0  /* sata link port */

#define FSATA_DEBUG_TAG "FSATA-CONTROLLER-DISKIO"
#define FSATA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_WARN(format, ...)    FT_DEBUG_PRINT_W(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_INFO(format, ...)    FT_DEBUG_PRINT_I(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)

/* 64位需要预留给内存池更大的空间 */
static u8 mem[50000] __attribute__((aligned(1024))) = {0};

#define SATA_PORT_MEM_SIZE 0x00000C00

static u32 port_mem_count = 0;

static FSataCtrl sata_device[FSATA_INSTANCE_NUM];//最多支持16个ahci控制器，可以自行定义个数
static s32 sata_host_count;

static boolean sata_ok = FALSE;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS status = STA_NOINIT;

	if (FT_COMPONENT_IS_READY == sata_device[HOST_NUM].is_ready)
		status &= ~STA_NOINIT; /* 假设Sata处于插入状态 */

	return status;
}

static int FSataInit(void)
{
	s32 i;
	u32	j;
    u8 id = 0;

    const FSataConfig *config_p = NULL;
    FSataCtrl *instance_p;
    FError status = FSATA_SUCCESS;
    FError ret = FSATA_SUCCESS;
    boolean host_valid = FALSE;

    if (sata_ok == TRUE)
    {
        FSATA_WARN("sata already init\r\n");
        return 0;
    }

	for(i = 0; i < FSATA_INSTANCE_NUM; i++)
    {
		instance_p = &(sata_device[i]);
		memset(instance_p, 0, sizeof(*instance_p));
	}

	/* get xhci host from fsata_g.c */
	for(id = 0;id < FSATA_INSTANCE_NUM; id++)
	{
		config_p = FSataLookupConfig(id, FSATA_TYPE_CONTROLLER);
		/* 如果有一个定义的PLATFORM AHCI HOST，则获取，否则跳过 */
		if(config_p->base_addr != 0)
        {
            /* base不为0，表示有platform ahci自定义 */
            status = FSataCfgInitialize(&sata_device[sata_host_count], config_p);
            if (FSATA_SUCCESS != status)
            {
                FSATA_ERROR("init sata failed, status: 0x%x", status);
                continue;
            }

			FSATA_DEBUG("plat ahci host[%d] base_addr = 0x%x", id, sata_device[sata_host_count].config.base_addr);
			FSATA_DEBUG("plat ahci host[%d] irq_num = %d", id, sata_device[sata_host_count].config.irq_num);
			sata_host_count++;
		}
        else
        {
			continue;
		}
	}
	FSATA_DEBUG("sata_host_count = %d\n", sata_host_count);
	for (i = 0; i < sata_host_count; i++)
	{
        host_valid = FALSE;
		instance_p = &(sata_device[i]);

	    /* init ahci controller and port */
		status = FSataAhciInit(instance_p);
	    if (FSATA_SUCCESS != status)
	    {
	        FSataCfgDeInitialize(instance_p);
	        FSATA_ERROR("FSataAhciInit sata failed, status: 0x%x", status);
			continue;
	    }

        FSATA_DEBUG("instance_p->n_ports = %d\n", instance_p->n_ports);

        for (j = 0; j < instance_p->n_ports; j++)
		{
			u32 port_map = instance_p->port_map;
			if (!(port_map & (1 << j)))
				continue;
			ret = FSataAhciPortStart(instance_p, j, (uintptr)mem + SATA_PORT_MEM_SIZE*port_mem_count);
			port_mem_count++;
		    if (FSATA_SUCCESS != ret)
		    {
		        FSATA_ERROR("FSataAhciPortStart host %d port %d failed, ret: 0x%x", i, j, ret);
				continue;
		    }
			
		    ret = FSataAhciReadInfo(instance_p, j);
		    if (FSATA_SUCCESS != ret)
		    {
		        FSataCfgDeInitialize(instance_p);
		        FSATA_ERROR("FSataAhciReadInfo %d-%d failed, ret: 0x%x", i, j, ret);
				continue;
		    }
            if(FSATA_SUCCESS == ret)
            {
                host_valid = TRUE;
            }
		}
	    
	}
	
    sata_ok = TRUE;

    return 0;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS status = STA_NOINIT;
    if (FSATA_SUCCESS == FSataInit())
	{
		status &= ~STA_NOINIT;
		FSATA_INFO("init sata driver ok");
	}
	else
	{
		FSATA_ERROR("init sata driver failed");
	}

	return status;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT status = RES_OK;
	BYTE *io_buf = buff;
	UINT err = FSATA_SUCCESS;

	err = FSataReadWrite(&sata_device[HOST_NUM], PORT_NUM, sector, count, io_buf, 0);

	if (FSATA_SUCCESS != err)
	{
		FSATA_ERROR("read sata controller sector [%d-%d] failed: 0x%x", sector, sector + count, err);
		status = RES_ERROR;
	}

	return status;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT status = RES_OK;
	const BYTE *io_buf = buff;
	UINT err = FSATA_SUCCESS;

	err = FSataReadWrite(&sata_device[HOST_NUM], PORT_NUM, sector, count, (u8 *)io_buf, 1);

	if (FSATA_SUCCESS != err)
	{
		FSATA_ERROR("write sata controller sector [%d-%d] failed: 0x%x", sector, sector + count, err);
		status = RES_ERROR;
	}
	
	return status;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_ERROR;

	switch (cmd)
	{
		/* 确保磁盘驱动器已经完成了写处理，当磁盘I/O有一个写回缓存，
		   立即刷新原扇区，只读配置下不适用此命令 */
		case CTRL_SYNC:
			res = RES_OK;
			break;
		/* 所有可用的扇区数目（逻辑寻址即LBA寻址方式） */			
		case GET_SECTOR_COUNT:
			*((DWORD *)buff) = sata_device[HOST_NUM].port[PORT_NUM].dev_info.lba512;
			res = RES_OK; 
			break;
		/* 返回磁盘扇区大小, 只用于f_mkfs() */
		case GET_SECTOR_SIZE:
			res = RES_PARERR;
			break;
		/* 每个扇区有多少个字节 */			
		case GET_BLOCK_SIZE:
			*((DWORD *)buff) = sata_device[HOST_NUM].port[PORT_NUM].dev_info.blksz;
			res = RES_OK;
			break;
		case CTRL_TRIM:
			res = RES_PARERR;
			break;
		}

	FSATA_INFO("cmd %d, buff: %p", cmd, *((DWORD*) buff));
	return res;
}











