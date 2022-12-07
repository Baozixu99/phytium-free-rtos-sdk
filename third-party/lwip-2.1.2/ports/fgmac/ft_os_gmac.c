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
 * FilePath: ft_os_gmac.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-25 09:16:57
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */


#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <string.h>
#include <stdio.h>
#include "ft_os_gmac.h"
#include "fgmac.h"
#include "fassert.h"
#include "fio.h"
#include "fassert.h"
#include "finterrupt.h"
#include "list.h"
#include "fcpu_info.h"
#include "fdebug.h"

#define OS_MAC_DEBUG_TAG "OS_MAC"

#define OS_MAC_DEBUG_D(format, ...) FT_DEBUG_PRINT_D(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define OS_MAC_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define OS_MAC_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define OS_MAC_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)

#define FT_OS_GMACOBJECT_READLY 0x58

static boolean rx_data_flag = FALSE;
static FGmacConfig gmac_config;

static void EthLinkPhyStatusChecker(void *param)
{
    FASSERT(param != NULL);
    FGmac *instance_p = (FGmac *)param;
    uintptr base_addr = instance_p->config.base_addr;

    u32 status = FGMAC_READ_REG32(base_addr, FGMAC_MAC_PHY_STATUS);

    if (FGMAC_RGSMIIIS_LNKSTS_UP == (FGMAC_RGSMIIIS_LNKSTS & status))
    {
        FGmacPhyCfgInitialize(instance_p);
        OS_MAC_DEBUG_I("link is up");
    }
    else
    {
        OS_MAC_DEBUG_I("link is down");
    }
    
    return;
}

static void EthLinkDmaErrChecker(void *param)
{
    FASSERT(param != NULL);
    FGmac *instance_p = (FGmac *)param;
    uintptr base_addr = instance_p->config.base_addr;

    u32 reg_val = FGMAC_READ_REG32(base_addr, FGMAC_DMA_INTR_OFFSET);
    u32 status  = FGMAC_READ_REG32(base_addr, FGMAC_DMA_STATUS_OFFSET);

    if ((FGMAC_DMA_STATUS_TPS & status) && (FGMAC_DMA_INTR_ENA_TSE & reg_val))
    {
        OS_MAC_DEBUG_E("Transmit process stopped");
    }

    if ((FGMAC_DMA_STATUS_TU & status) && (FGMAC_DMA_INTR_ENA_TUE & reg_val))
    {
        OS_MAC_DEBUG_E("Transmit Buffer Unavailable");
    }

    if ((FGMAC_DMA_STATUS_TJT & status) && (FGMAC_DMA_INTR_ENA_THE & reg_val))
    {
        OS_MAC_DEBUG_E("Transmit Jabber Timeout");
    }

    if ((FGMAC_DMA_STATUS_OVF & status) && (FGMAC_DMA_INTR_ENA_OVE & reg_val))
    {
        OS_MAC_DEBUG_E("Receive Overflow");
    }

    if ((FGMAC_DMA_STATUS_UNF & status) && (FGMAC_DMA_INTR_ENA_UNE & reg_val))
    {
        OS_MAC_DEBUG_E("Transmit Underflow");
    }

    if ((FGMAC_DMA_STATUS_RU & status) && (FGMAC_DMA_INTR_ENA_RUE & reg_val))
    {
        OS_MAC_DEBUG_E("Receive Buffer Unavailable");
    }

    if ((FGMAC_DMA_STATUS_RPS & status) && (FGMAC_DMA_INTR_ENA_RSE & reg_val))
    {
        OS_MAC_DEBUG_E("Receive Process Stopped");
    }

    if ((FGMAC_DMA_STATUS_RWT & status) && (FGMAC_DMA_INTR_ENA_RWE & reg_val))
    {
        OS_MAC_DEBUG_E("Receive Watchdog Timeout");
    }

    if ((FGMAC_DMA_STATUS_ETI & status) && (FGMAC_DMA_INTR_ENA_ETE & reg_val))
    {
        OS_MAC_DEBUG_E("Early Transmit Interrupt");
    }

    if ((FGMAC_DMA_STATUS_FBI & status) && (FGMAC_DMA_INTR_ENA_FBE & reg_val))
    {
        OS_MAC_DEBUG_E("Fatal Bus Error");
    }

    return;
}

void EthLinkStatusChecker(void *param)
{
    FASSERT(param);
    FGmac *instance_p = (FGmac *)param;
    uintptr base_addr = instance_p->config.base_addr;
    u32 status = FGMAC_READ_REG32(base_addr, FGMAC_MAC_PHY_STATUS);
    u32 speed_status, duplex_status;
    u32 speed, duplex;

    /* Check the link status */
    if (FGMAC_RGSMIIIS_LNKSTS_UP == (FGMAC_RGSMIIIS_LNKSTS & status))
    {
        speed_status = FGMAC_RGSMIIIS_SPEED & status;
        duplex_status = FGMAC_RGSMIIIS_LNKMODE & status;

        if (FGMAC_RGSMIIIS_SPEED_125MHZ == speed_status)
            speed = FGMAC_PHY_SPEED_1000;
        else if (FGMAC_RGSMIIIS_SPEED_25MHZ == speed_status)
            speed = FGMAC_PHY_SPEED_100;
        else
            speed = FGMAC_PHY_SPEED_10;

        if (FGMAC_RGSMIIIS_LNKMODE_HALF == duplex_status)
            duplex = FGMAC_PHY_MODE_HALFDUPLEX;
        else
            duplex = FGMAC_PHY_MODE_FULLDUPLEX;

        OS_MAC_DEBUG_I("link is up --- %d/%s", 
                   speed, (FGMAC_PHY_MODE_FULLDUPLEX == duplex) ? "full" : "half");
    }
    else
    {
        OS_MAC_DEBUG_I("link is down ---");
    }
}

static void EthLinkRecvDoneCallback(void *param)
{
    FASSERT(param);
    FGmac *instance_p = (FGmac *)param;

    if (TRUE == rx_data_flag)
    {
        OS_MAC_DEBUG_W("last rx data has not been handled !!!");
    }

    rx_data_flag = TRUE;
    OS_MAC_DEBUG_I("recv data");
    return;
}

static void EthLinkTransDoneCallback(void *param)
{
    FASSERT(param);
    FGmac *instance_p = (FGmac *)param;

    FGmacResumeDmaSend(instance_p->config.base_addr);
    OS_MAC_DEBUG_I("resume trans");
    return;
}

int FtOsGmacSetupInterrupt(FGmac *instance_p)
{
    LWIP_ASSERT("instance_p != NULL", (instance_p != NULL));
    FGmacConfig *config_p = &instance_p->config;
    uintptr base_addr = config_p->base_addr;
    u32 irq_num = config_p->irq_num;
    u32 cpu_id;

    u32 irq_priority = (config_p->irq_prority);
    
    if(irq_priority < (configMAX_API_CALL_INTERRUPT_PRIORITY))
    {
        OS_MAC_DEBUG_E("gmac irq priority is not applicable, you need set value below configMAX_API_CALL_INTERRUPT_PRIORITY!!!");
        FASSERT(0);
    }

    /* gic initialize */
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(irq_num, cpu_id);

    /* disable all gmac & dma intr */
    FGmacSetInterruptMask(instance_p, FGMAC_CTRL_INTR, FGMAC_ISR_MASK_ALL_BITS);
    FGmacSetInterruptMask(instance_p, FGMAC_DMA_INTR, FGMAC_DMA_INTR_ENA_ALL_MASK);

    InterruptSetPriority(irq_num, irq_priority);
    InterruptInstall(irq_num, FGmacInterruptHandler, instance_p, "GMAC-IRQ");

    /* register intr callback */
    FGmacRegisterEvtHandler(instance_p, FGMAC_PHY_STATUS_EVT, EthLinkPhyStatusChecker);
    FGmacRegisterEvtHandler(instance_p, FGMAC_DMA_ERR_EVT, EthLinkDmaErrChecker);
    FGmacRegisterEvtHandler(instance_p, FGMAC_LINK_STATUS_EVT, EthLinkStatusChecker);
    FGmacRegisterEvtHandler(instance_p, FGMAC_TX_COMPLETE_EVT, EthLinkTransDoneCallback);

    /* umask intr */
    InterruptUmask(irq_num);

    /* enable some interrupts */
    FGmacSetInterruptUmask(instance_p, FGMAC_CTRL_INTR, FGMAC_ISR_MASK_RSIM);
    FGmacSetInterruptUmask(instance_p, FGMAC_DMA_INTR, 
							FGMAC_DMA_INTR_ENA_NIE | FGMAC_DMA_INTR_ENA_RIE | FGMAC_DMA_INTR_ENA_AIE);

    OS_MAC_DEBUG_I("gmac setup done");
    return 0;
}


/**
 * @name: Ft_Os_GmacMem_Create
 * @msg:  Initialize the Gmac TX/Rx Describe Memory 。
 * @param {*}
 * @return {*}
 */

static void FtOsGmacMemCreate(FtOsGmac *os_gmac, netif_config *netif_config_p)
{
    FASSERT(os_gmac != NULL);
    
    os_gmac->rx_buffer = netif_config_p->rx_buf;
    if (os_gmac->rx_buffer == NULL)
    {
        OS_MAC_DEBUG_E("Rxbuffer Malloc is error ");
        FASSERT(0);
    }

    os_gmac->tx_buffer = netif_config_p->tx_buf;
    if (os_gmac->tx_buffer == NULL)
    {
        OS_MAC_DEBUG_E("Txbuffer Malloc is error ");
        FASSERT(0);
    }

    os_gmac->gmac.tx_desc = (FGmacDmaDesc *)netif_config_p->tx_desc;
    if (os_gmac->gmac.tx_desc == NULL)
    {
        OS_MAC_DEBUG_E("tx_desc Malloc is error ");
        FASSERT(0);
    }

    os_gmac->gmac.rx_desc = (FGmacDmaDesc *)netif_config_p->rx_desc;
    if (os_gmac->gmac.rx_desc == NULL)
    {
        OS_MAC_DEBUG_E("rx_desc Malloc is error ");
        FASSERT(0);
    }

#define ROUND_UP(x, align) (((long)(x) + ((long)align - 1)) & \
                            ~((long)align - 1))

    os_gmac->gmac.rx_desc = (FGmacDmaDesc *)ROUND_UP(os_gmac->gmac.rx_desc, 128);
}

static void FtOsGmacMemFree(FtOsGmac *os_gmac)
{
    FASSERT(os_gmac != NULL);
    if (os_gmac->rx_buffer)
    {
        vPortFree(os_gmac->rx_buffer);
    }

    if (os_gmac->tx_buffer)
    {
        vPortFree(os_gmac->tx_buffer);
    }

    if (os_gmac->gmac.rx_desc)
    {
        vPortFree((void *)(os_gmac->gmac.rx_desc));
    }

    if (os_gmac->gmac.tx_desc)
    {
        vPortFree((void *)(os_gmac->gmac.tx_desc));
    }
}

/**
 * @name: Ft_Os_Gmac Object_Init
 * @msg:
 * @param {FtOsGmac} *Os_Gmac
 * @param {FtOsGmacConfig} *config
 * @return {*}
 */
void FtOsGmacObjectInit(FtOsGmac *os_gmac, FtOsGmacConfig *config)
{
    FASSERT(os_gmac != NULL);
    FASSERT(os_gmac->is_ready != FT_OS_GMACOBJECT_READLY);
    memset(os_gmac, 0, sizeof(FtOsGmac));
    os_gmac->config = *config;
    os_gmac->is_ready = FT_OS_GMACOBJECT_READLY;
}

/**
 * @name: Ft_Os_Gmac_Init
 * @msg:
 * @param {Ft_Os_Gmac} *Os_Gmac
 * @param {u32} InstanceId
 * @return {*}
 */

FError FtOsGmacInit(FtOsGmac *os_gmac, netif_config *netif_config_p)
{
    FASSERT(os_gmac != NULL);
    FASSERT(os_gmac->is_ready == FT_OS_GMACOBJECT_READLY);
    const FGmacConfig *def_config_p = NULL;
    FGmac *gmac;
    FError ret = FGMAC_SUCCESS;
    gmac = &os_gmac->gmac;
    
    FtOsGmacMemFree(os_gmac);
    
    memset(gmac, 0U, sizeof(FGmac));
	def_config_p = FGmacLookupConfig(os_gmac->config.gmac_instance);
	if(NULL == def_config_p)
    {
        OS_MAC_DEBUG_E("gmac lookup cfg failed ");
        return FGMAC_ERR_FAILED;
    }
    
	gmac_config = *def_config_p;
	gmac_config.speed = FGMAC_PHY_SPEED_1000;
    gmac_config.mdc_clk_hz = FGMAC_PHY_MII_ADDR_CR_250_300MHZ;
    gmac_config.en_auto_negtiation = TRUE;
    gmac_config.duplex_mode = FGMAC_PHY_MODE_FULLDUPLEX;

#ifdef CONFIG_GMAC_IRQ_PRIORITY
    gmac_config.irq_prority = CONFIG_GMAC_IRQ_PRIORITY;
#else
    gmac_config.irq_prority = IRQ_PRIORITY_VALUE_12;
#endif

    /* initialize gmac */
	ret = FGmacCfgInitialize(gmac, &gmac_config);
    if(FGMAC_SUCCESS != ret)
    {
        OS_MAC_DEBUG_E("gmac cfg init failed ");
        os_gmac->is_ready = 0;
        return FGMAC_ERR_FAILED;
    }

    FtOsGmacMemCreate(os_gmac, netif_config_p);

    /* initialize phy */
    ret = FGmacPhyCfgInitialize(gmac);
    if(FGMAC_SUCCESS != ret)
    {
        OS_MAC_DEBUG_E("gmac phy cfg init failed ");
        return FGMAC_ERR_PHY_AUTO_FAILED;
    }

    return ret;
}

/**
 * @name: Ft_Os_Gmac_Start
 * @msg:
 * @param {Ft_Os_Gmac} *Os_Gmac
 * @return {*}
 */

void FtOsGmacStart(FtOsGmac *os_gmac)
{
    FASSERT(os_gmac != NULL);
    FASSERT((os_gmac->is_ready == FT_OS_GMACOBJECT_READLY));

    FGmac *gmac;
    gmac = &os_gmac->gmac;
    u32 ret = FT_SUCCESS;
    
    /* Initialize Rx Description list : ring Mode */
    ret = FGmacSetupRxDescRing(gmac, (FGmacDmaDesc *)(gmac->rx_desc), os_gmac->rx_buffer, FGMAC_MAX_PACKET_SIZE, GMAC_RX_DESCNUM);
    if (FT_SUCCESS != ret)
    {
        OS_MAC_DEBUG_E("gmac setup rx return err code %d\r\n", ret);
        FASSERT(FT_SUCCESS == ret);
    }

    /* Initialize Tx Description list : ring Mode */
    ret = FGmacSetupTxDescRing(gmac, (FGmacDmaDesc *)(gmac->tx_desc), os_gmac->tx_buffer, FGMAC_MAX_PACKET_SIZE, GMAC_TX_DESCNUM);
    if (FT_SUCCESS != ret)
    {
        OS_MAC_DEBUG_E("gmac setup tx return err code %d\r\n", ret);
        FASSERT(FT_SUCCESS == ret);
    }

    /* enable gmac */
    FGmacStartTrans(gmac);

    /* Gmac interrupt init */
    FtOsGmacSetupInterrupt(gmac);

    return;
}

void FtOsGmacStop(FtOsGmac *os_gmac)
{
    FGmac *gmac;

    FASSERT(os_gmac != NULL);
    FASSERT((os_gmac->is_ready == FT_OS_GMACOBJECT_READLY));
    gmac = &os_gmac->gmac;

    /* disable all gmac & dma intr */
    FGmacSetInterruptMask(gmac, FGMAC_CTRL_INTR, FGMAC_ISR_MASK_ALL_BITS);
    FGmacSetInterruptMask(gmac, FGMAC_DMA_INTR, FGMAC_DMA_INTR_ENA_ALL_MASK);

    /* umask intr */
    InterruptMask(gmac->config.irq_num);

    FGmacStopTrans(gmac);
}
