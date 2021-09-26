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
#include "gmac.h"
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

/**
 * @name: Ft_Os_GmacMem_Create
 * @msg:  Initialize the Gmac TX/Rx Describe Memory 。
 * @param {*}
 * @return {*}
 */

static u8 rx_buf[GMAC_RX_DESCNUM * GMAC_MAX_PACKET_SIZE] __attribute__((aligned(32)));
static u8 tx_buf[GMAC_TX_DESCNUM * GMAC_MAX_PACKET_SIZE] __attribute__((aligned(32)));
static u8 tx_desc[GMAC_TX_DESCNUM * sizeof(GmacDmaDesc)] __attribute__((aligned(128)));
static u8 rx_desc[GMAC_RX_DESCNUM * sizeof(GmacDmaDesc) + 128] __attribute__((aligned(128)));

static void FtOsGmacMemCreate(FtOsGmac *os_gmac)
{
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

    os_gmac->gmac.txDesc = (GmacDmaDesc *)tx_desc;
    if (os_gmac->gmac.txDesc == NULL)
    {
        OS_MAC_DEBUG_E("txDesc Malloc is error ");
        FT_ASSERTVOIDALWAYS();
    }

    os_gmac->gmac.rxDesc = (GmacDmaDesc *)rx_desc;
    if (os_gmac->gmac.rxDesc == NULL)
    {
        OS_MAC_DEBUG_E("rxDesc Malloc is error ");
        FT_ASSERTVOIDALWAYS();
    }

#define ROUND_UP(x, align) (((long)(x) + ((long)align - 1)) & \
                            ~((long)align - 1))

    os_gmac->gmac.rxDesc = (GmacDmaDesc *)ROUND_UP(os_gmac->gmac.rxDesc, 128);
}

static void FtOsGmacMemFree(FtOsGmac *os_gmac)
{
    if (os_gmac->rx_buffer)
    {
        vPortFree(os_gmac->rx_buffer);
    }

    if (os_gmac->tx_buffer)
    {
        vPortFree(os_gmac->tx_buffer);
    }

    if (os_gmac->gmac.rxDesc)
    {
        vPortFree((void *)(os_gmac->gmac.rxDesc));
    }

    if (os_gmac->gmac.txDesc)
    {
        vPortFree((void *)(os_gmac->gmac.txDesc));
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
void FtOsGmacInit(FtOsGmac *os_gmac)
{
    GmacCtrl *gmac;
    
    FT_ASSERTVOID(os_gmac != NULL);

    FT_ASSERTVOID((os_gmac->is_ready == FT_OS_GMACOBJECT_READLY));

    gmac = &os_gmac->gmac;
    FtOsGmacMemFree(os_gmac);
    FT_ASSERTVOID(GmacCfgInitialize(gmac, GmacLookupConfig(os_gmac->config.gmac_instance)) == FT_SUCCESS);
    FtOsGmacMemCreate(os_gmac);

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
    GmacCtrl *gmac;
    FT_ASSERTVOID(os_gmac != NULL);
    FT_ASSERTVOID((os_gmac->is_ready == FT_OS_GMACOBJECT_READLY));
    gmac = &os_gmac->gmac;
    u32 ret;

    ret = GmacHwInitialize(gmac);
    if (FT_SUCCESS != ret)
    {
        OS_MAC_DEBUG_E("gmac return err code %d\r\n", ret);
        FT_ASSERTVOID(FT_SUCCESS == ret);
    }

    /* Initialize Rx Description list : ring Mode */
    GmacDmaInitRxDescRing(gmac, (GmacDmaDesc *)(gmac->rxDesc), os_gmac->rx_buffer, GMAC_MAX_PACKET_SIZE, GMAC_RX_DESCNUM);

    /* Initialize Tx Description list : ring Mode */
    GmacDmaInitTxDescRing(gmac, (GmacDmaDesc *)(gmac->txDesc), os_gmac->tx_buffer, GMAC_MAX_PACKET_SIZE, GMAC_TX_DESCNUM);

    GmacDmaStart(gmac);

    /* Gmac interrupt init */
    GmacIntrInit(gmac);

    return;
}

void FtOsGmacStop(FtOsGmac *os_gmac)
{
    GmacCtrl *gmac;

    FT_ASSERTVOID(os_gmac != NULL);
    FT_ASSERTVOID((os_gmac->is_ready == FT_OS_GMACOBJECT_READLY));
    gmac = &os_gmac->gmac;
}
