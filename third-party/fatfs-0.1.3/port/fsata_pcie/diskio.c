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
#include "fpcie.h"
#include "fpcie_common.h"
#include "fsata.h"
#include "fsata_hw.h"

#define PORT_NUM    0  /* sata link port 1 */

#define FSATA_DEBUG_TAG "FSATA-PCIE-DISKIO"
#define FSATA_ERROR(format, ...)   FT_DEBUG_PRINT_E(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_WARN(format, ...)    FT_DEBUG_PRINT_W(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_INFO(format, ...)    FT_DEBUG_PRINT_I(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSATA_DEBUG(format, ...)   FT_DEBUG_PRINT_D(FSATA_DEBUG_TAG, format, ##__VA_ARGS__)

/* 64位需要预留给内存池更大的空间 */
static u8 mem[50000] __attribute__((aligned(1024))) = {0};

#define PCI_CLASS_STORAGE_SATA_AHCI	0x010601

#define SATA_PORT_MAX_NUM	4
#define SATA_HOST_MAX_NUM   16

static u32 port_mem_count = 0;

static FSataCtrl sata_device[SATA_HOST_MAX_NUM];//最多支持16个ahci控制器，可以自行定义个数
static s32 sata_host_count = 0;

static boolean sata_ok = FALSE;

static FPcie pcie_obj = {0};
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS status = STA_NOINIT;

	if (FT_COMPONENT_IS_READY == sata_device[PORT_NUM].is_ready)
		status &= ~STA_NOINIT; /* 假设Sata处于插入状态 */

	return status;
}

static void FSataPcieIrqHandler(void *param)
{
    FSataIrqHandler(0, param);
}

static void PCieIntxInit(FPcie* instance_p)
{
	InterruptSetPriority(FT_PCI_INTA_IRQ_NUM, 0);	
    
    InterruptInstall(FT_PCI_INTA_IRQ_NUM, (IrqHandler)FPcieIntxIrqHandler, instance_p, "pcieInta");
    InterruptUmask(FT_PCI_INTA_IRQ_NUM);			
    InterruptSetPriority(FT_PCI_INTB_IRQ_NUM, 0);
    InterruptInstall(FT_PCI_INTB_IRQ_NUM, (IrqHandler)FPcieIntxIrqHandler, instance_p, "pcieIntB");
    InterruptUmask(FT_PCI_INTB_IRQ_NUM);
    InterruptSetPriority(FT_PCI_INTC_IRQ_NUM, 0);
    InterruptInstall(FT_PCI_INTC_IRQ_NUM, (IrqHandler)FPcieIntxIrqHandler, instance_p, "pcieIntC");
    InterruptUmask(FT_PCI_INTC_IRQ_NUM);
    InterruptSetPriority(FT_PCI_INTD_IRQ_NUM, 0);
    InterruptInstall(FT_PCI_INTD_IRQ_NUM, (IrqHandler)FPcieIntxIrqHandler, instance_p, "pcieIntD");
    InterruptUmask(FT_PCI_INTD_IRQ_NUM);
}

static void FPcieInit()
{	
	/* 第一步初始化pcie_obj这个实例，初始化mem，io资源成员 */
    FPcieCfgInitialize(&pcie_obj, FPcieLookupConfig(FT_PCIE0_ID));
    FSATA_DEBUG("\n");
	FSATA_DEBUG("	PCI:\n");
	FSATA_DEBUG("	B:D:F			VID:PID			parent_BDF			class_code\n");
    FPcieScanBus(&pcie_obj, 0, 0xffffffff);
	PCieIntxInit(&pcie_obj);
}

static uintptr_t SataPcieIrqInstall(FSataCtrl* ahci_ctl, u32 bdf)
{
	int ret = FT_SUCCESS;
	
	FPcieIntxFun intx_fun;
    intx_fun.IntxCallBack = FSataPcieIrqHandler;
	intx_fun.args = ahci_ctl;
	intx_fun.bdf = bdf;

    ret = FPcieIntxRegiterIrqHandler(&pcie_obj, bdf, &intx_fun);
    if(ret != FT_SUCCESS)
    {
        return ret;
    }

	return 0;
}

static int FSataInit(void)
{
	int ret;
	s32 i;
	u32	j;
	u32 bdf;
	u32 class;
	u16 pci_command;
	const u32 class_code = PCI_CLASS_STORAGE_SATA_AHCI;
	u8	port_num_max = SATA_PORT_MAX_NUM;
	uintptr bar_addr = 0;
	u16 vid, did;
    u8 id = 0;

    const FSataConfig *config_p = NULL;
    FSataCtrl *instance_p;
    FError status = FSATA_SUCCESS;

	FPcie *pcie = &pcie_obj;

	if (sata_ok == TRUE)
    {
        FSATA_WARN("sata already init\r\n");
        return 0;
    }

	for(i = 0; i < SATA_HOST_MAX_NUM; i++)
    {
		instance_p = &(sata_device[i]);
		memset(instance_p, 0, sizeof(*instance_p));
        config_p = FSataLookupConfig(id, FSATA_TYPE_CONTROLLER);
        status = FSataCfgInitialize(&sata_device[sata_host_count], config_p);
        if (FSATA_SUCCESS != status)
        {
            FSATA_ERROR("init sata failed, status: 0x%x", status);
            continue;
        }
	}

	/* find xhci host from pcie instance */
	for(i = 0; i < pcie->scaned_bdf_count; i++)
	{
		bdf = pcie->scaned_bdf_array[i];
		FPcieEcamReadConfig32bit(pcie->config.ecam, bdf, FPCI_CLASS_REVISION, &class) ;
        class = (class) >> 8 ;

		if(class == class_code)
		{
			FPcieEcamReadConfig16bit(pcie->config.ecam, bdf, FPCIE_VENDOR_REG, &vid) ;
			FPcieEcamReadConfig16bit(pcie->config.ecam, bdf, FPCIE_DEVICE_ID_REG, &did);

			FSATA_DEBUG("AHCI-PCI HOST found !!!, b.d.f = %x.%x.%x\n", FPCIE_BUS(bdf), FPCIE_DEV(bdf), FPCIE_FUNC(bdf));
			
			FPcieEcamReadConfig32bit(pcie->config.ecam, bdf, FPCIE_BASE_ADDRESS_5, (u32*)&bar_addr);
			FSATA_DEBUG("FSataPcieIntrInstall BarAddress %p \r\n", bar_addr);

			if (0x0 == bar_addr)
            {
                ret = -1;
                FSATA_DEBUG("Bar address: 0x%lx", bar_addr);
                break;
            }
			FPcieEcamReadConfig16bit(pcie->config.ecam, bdf, FPCIE_COMMAND_REG, &pci_command);			
			pci_command |= FPCIE_COMMAND_MASTER;
			FPcieEcamWriteConfig16bit(pcie->config.ecam, bdf, FPCIE_COMMAND_REG, pci_command);

			SataPcieIrqInstall(&(sata_device[sata_host_count]) ,bdf);
			sata_device[sata_host_count].config.base_addr = bar_addr;
			FSATA_DEBUG("sata_device[%d].config.base_addr = 0x%x\n", sata_host_count, sata_device[sata_host_count].config.base_addr);
            sata_device[sata_host_count].is_ready = FT_COMPONENT_IS_READY;
			sata_host_count++;
		}
	}

	if(ret == -1)
	{		
		return ret;
	}

	FSATA_DEBUG("scaned %d ahci host\n", sata_host_count);

	for(i = 0; i < sata_host_count; i++)
	{
		instance_p = &(sata_device[i]);

	    /* Initialization */
		status = FSataAhciInit(instance_p);
	    if (FSATA_SUCCESS != status)
	    {
	        FSataCfgDeInitialize(instance_p);
	        FSATA_ERROR("FSataAhciRestart sata failed, ret: 0x%x", status);
			continue;
	    }

		for (j = 0; j < instance_p->n_ports; j++)
		{
			u32 port_map = instance_p->port_map;
			if (!(port_map & (1 << j)))
				continue;
			ret = FSataAhciPortStart(instance_p, j, (uintptr)mem + 1024*3*port_mem_count);
			port_mem_count++;
		    if (FSATA_SUCCESS != ret)
		    {
		        FSATA_ERROR("FSataAhciPortStart port %d failed, ret: 0x%x", j, ret);
				continue;
		    }
			
		    status = FSataAhciReadInfo(instance_p, j);
		    if (FSATA_SUCCESS != status)
		    {
		        FSataCfgDeInitialize(instance_p);
		        FSATA_ERROR("FSataAhciReadInfo failed, ret: 0x%x", status);
				continue;
		    }
			FSATA_DEBUG("\n");
		}

	}
    FSATA_DEBUG("Formatting sata device......\r\n");
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
    static u8 flag = 0;
    if (flag == 0)
    {
        FPcieInit();
        flag = 1;
    }
    
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

	err = FSataReadWrite(&sata_device[PORT_NUM], PORT_NUM, sector, count, io_buf, 0);

	if (FSATA_SUCCESS != err)
	{
		FSATA_ERROR("read pcie sata sector [%d-%d] failed: 0x%x", sector, sector + count, err);
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

	err = FSataReadWrite(&sata_device[PORT_NUM], PORT_NUM, sector, count, (u8 *)io_buf, 1);

	if (FSATA_SUCCESS != err)
	{
		FSATA_ERROR("write pcie sata sector [%d-%d] failed: 0x%x", sector, sector + count, err);
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
			*((DWORD *)buff) = sata_device[PORT_NUM].port[PORT_NUM].dev_info.lba512;
			res = RES_OK; /* 最多使用1000个sector */
			break;
		/* 返回磁盘扇区大小, 只用于f_mkfs() */
		case GET_SECTOR_SIZE:
			res = RES_PARERR;
			break;
		/* 每个扇区有多少个字节 */			
		case GET_BLOCK_SIZE:
			*((DWORD *)buff) = sata_device[PORT_NUM].port[PORT_NUM].dev_info.blksz;
			res = RES_OK;
			break;
		case CTRL_TRIM:
			res = RES_PARERR;
			break;
		}

	FSATA_INFO("cmd %d, buff: %p", cmd, *((DWORD*) buff));
	return res;
}











