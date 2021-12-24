/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0.
 *
 * @Date: 2021-04-07 09:53:07
 * @LastEditTime: 2021-04-07 15:00:19
 * @Description:  This files is for freertos gmac ports
 *
 * @Modify History:
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
#include "ft_assert.h"
#include "ft_io.h"
#include "ft_assert.h"
#include "gicv3.h"
#include "interrupt.h"
#include "list.h"
#include "ft_debug.h"

#define OS_MAC_DEBUG_TAG "OS_MAC"

#define OS_MAC_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define OS_MAC_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)
#define OS_MAC_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(OS_MAC_DEBUG_TAG, format, ##__VA_ARGS__)

#define FT_OS_GMACOBJECT_READLY 0x58

static boolean rx_data_flag = FALSE;

static void EthLinkPhyStatusChecker(void *param)
{
    FT_ASSERTVOID(param != NULL);
    FGmac *instance_p = (FGmac *)param;
    uintptr base_addr = instance_p->config.base_addr;

    u32 phy_status = FGMAC_READ_REG32(base_addr, FGMAC_MAC_PHY_STATUS);

    if (FGMAC_RGSMIIIS_LNKSTS_UP == (FGMAC_RGSMIIIS_LNKSTS & phy_status))
    {
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
    FT_ASSERTVOID(param != NULL);
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
    FT_ASSERTVOID(param);
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
        OS_MAC_DEBUG_I("link is down");
    }

    (void)speed;
    (void)duplex;
}

static void EthLinkRecvDoneCallback(void *param)
{
    FT_ASSERTVOID(param);
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
    FT_ASSERTVOID(param);
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
    u32 irq_priority = (0x8 << 4) + 4 * 16;

    /* disable all gmac & dma intr */
    FGmacSetInterruptMask(instance_p, FGMAC_CTRL_INTR, FGMAC_ISR_MASK_ALL_BITS, FALSE);
    FGmacSetInterruptMask(instance_p, FGMAC_DMA_INTR, FGMAC_DMA_INTR_ENA_ALL_MASK, FALSE);

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
    FGmacSetInterruptMask(instance_p, FGMAC_CTRL_INTR, FGMAC_ISR_MASK_RSIM, TRUE);
    FGmacSetInterruptMask(instance_p, FGMAC_DMA_INTR,  FGMAC_DMA_INTR_ENA_NIE | FGMAC_DMA_INTR_ENA_RIE\
                    /*FGMAC_DMA_INTR_ENA_ALL_MASK*/, TRUE);

    OS_MAC_DEBUG_I("gmac setup done");
    return 0;
}


/**
 * @name: Ft_Os_GmacMem_Create
 * @msg:  Initialize the Gmac TX/Rx Describe Memory 。
 * @param {*}
 * @return {*}
 */

static u8 rx_buf[GMAC_RX_DESCNUM * GMAC_MAX_PACKET_SIZE] __attribute__((aligned(32)));
static u8 tx_buf[GMAC_TX_DESCNUM * GMAC_MAX_PACKET_SIZE] __attribute__((aligned(32)));
static u8 tx_desc[GMAC_TX_DESCNUM * sizeof(FGmacDmaDesc)] __attribute__((aligned(128)));
static u8 rx_desc[GMAC_RX_DESCNUM * sizeof(FGmacDmaDesc) + 128] __attribute__((aligned(128)));

static void FtOsGmacMemCreate(FtOsGmac *os_gmac)
{
    FT_ASSERTVOID(os_gmac != NULL);
    os_gmac->rx_buffer = rx_buf;
    if (os_gmac->rx_buffer == NULL)
    {
        OS_MAC_DEBUG_E("Rxbuffer Malloc is error ");
        FT_ASSERTVOIDALWAYS();
    }

    os_gmac->tx_buffer = tx_buf;
    if (os_gmac->tx_buffer == NULL)
    {
        OS_MAC_DEBUG_E("Txbuffer Malloc is error ");
        FT_ASSERTVOIDALWAYS();
    }

    os_gmac->gmac.tx_desc = (FGmacDmaDesc *)tx_desc;
    if (os_gmac->gmac.tx_desc == NULL)
    {
        OS_MAC_DEBUG_E("tx_desc Malloc is error ");
        FT_ASSERTVOIDALWAYS();
    }

    os_gmac->gmac.rx_desc = (FGmacDmaDesc *)rx_desc;
    if (os_gmac->gmac.rx_desc == NULL)
    {
        OS_MAC_DEBUG_E("rx_desc Malloc is error ");
        FT_ASSERTVOIDALWAYS();
    }

#define ROUND_UP(x, align) (((long)(x) + ((long)align - 1)) & \
                            ~((long)align - 1))

    os_gmac->gmac.rx_desc = (FGmacDmaDesc *)ROUND_UP(os_gmac->gmac.rx_desc, 128);
}

static void FtOsGmacMemFree(FtOsGmac *os_gmac)
{
    FT_ASSERTVOID(os_gmac != NULL);
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
    FT_ASSERTVOID(os_gmac != NULL);
    FT_ASSERTVOID(os_gmac->is_ready != FT_OS_GMACOBJECT_READLY);
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

void FtOsGmacInit(FtOsGmac *os_gmac, FGmacPhy *phy_p)
{
    
    FT_ASSERTVOID(os_gmac != NULL);
    FT_ASSERTVOID(phy_p != NULL);
    FT_ASSERTVOID((os_gmac->is_ready == FT_OS_GMACOBJECT_READLY));

    FGmac *gmac;
    gmac = &os_gmac->gmac;
    FtOsGmacMemFree(os_gmac);

    FT_ASSERTVOID(FGmacCfgInitialize(gmac, FGmacLookupConfig(gmac->config.instance_id)) == FT_SUCCESS);
    
    FtOsGmacMemCreate(os_gmac);

    /* initialize phy */
    memset(phy_p, 0U, sizeof(FGmacPhy));
    FGmacPhyLookupConfig(gmac->config.instance_id, phy_p);
    FT_ASSERTVOID(FGmacPhyCfgInitialize(phy_p) == FT_SUCCESS);

    /* Create a binary semaphore used for informing ethernetif of frame reception */
    FT_ASSERTVOID((os_gmac->s_semaphore = xSemaphoreCreateBinary()) != NULL);
    /* Create a event group used for ethernetif of status change */
    FT_ASSERTVOID((os_gmac->s_status_event = xEventGroupCreate()) != NULL);
}

/**
 * @name: Ft_Os_Gmac_Start
 * @msg:
 * @param {Ft_Os_Gmac} *Os_Gmac
 * @return {*}
 */

void FtOsGmacStart(FtOsGmac *os_gmac)
{
    FT_ASSERTVOID(os_gmac != NULL);
    FT_ASSERTVOID((os_gmac->is_ready == FT_OS_GMACOBJECT_READLY));

    FGmac *gmac;
    gmac = &os_gmac->gmac;
    u32 ret = FT_SUCCESS;

    if (FT_SUCCESS != ret)
    {
        OS_MAC_DEBUG_E("gmac return err code %d\r\n", ret);
        FT_ASSERTVOID(FT_SUCCESS == ret);
    }

    /* Initialize Rx Description list : ring Mode */
    FGmacSetupRxDescRing(gmac, (FGmacDmaDesc *)(gmac->rx_desc), os_gmac->rx_buffer, GMAC_MAX_PACKET_SIZE, GMAC_RX_DESCNUM);

    /* Initialize Tx Description list : ring Mode */
    FGmacSetupTxDescRing(gmac, (FGmacDmaDesc *)(gmac->tx_desc), os_gmac->tx_buffer, GMAC_MAX_PACKET_SIZE, GMAC_TX_DESCNUM);

    /* enable gmac */
    FGmacStartTrans(gmac);

    /* Gmac interrupt init */
    FtOsGmacSetupInterrupt(gmac);

    return;
}

void FtOsGmacStop(FtOsGmac *os_gmac)
{
    FGmac *gmac;

    FT_ASSERTVOID(os_gmac != NULL);
    FT_ASSERTVOID((os_gmac->is_ready == FT_OS_GMACOBJECT_READLY));
    gmac = &os_gmac->gmac;
}
