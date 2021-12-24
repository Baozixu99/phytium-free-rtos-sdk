/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0.
 *
 * @Date: 2021-04-07 09:53:07
 * @LastEditTime: 2021-04-20 09:51:46
 * @Description:  This files is for
 *
 * @Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef FT_OS_GMAC_H
#define FT_OS_GMAC_H

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>

#include "fgmac.h"
#include "fgmac_hw.h"
#include "fgmac_phy.h"
#include "parameters.h"
#include "lwip/netif.h"

#define FT_OS_GMAC0_ID GMAC_INSTANCE_0
#define FT_OS_GMAC1_ID GMAC_INSTANCE_1

#define FT_NETIF_LINKUP 0x1U
#define FT_NETIF_DOWN 0x2U

/** @defgroup ENET_Buffers_setting
  * @{
  */

#define GMAC_RX_DESCNUM     1024U
#define GMAC_TX_DESCNUM     1024U

#define GMAC_MTU            1500U

/* Common PHY Registers (AR8035) */
#define PHY_INTERRUPT_ENABLE_OFFSET ((u16)0x12)
#define PHY_INTERRUPT_ENABLE_LINK_FAIL 0x00000800U  /* Link fail interrupt, 0  Interrupt disable , 1 Interrupt enable */

struct Ipv4Address
{
    u8 Ip_Address[4];
    u8 Netmask_Address[4];
    u8 GateWay_Address[4];
};

struct GmacThread
{
    const char *thread_name;
    u16 stack_depth; /* The number of words the stack */
    u32 priority;   /* Defines the priority at which the task will execute. */
    TaskHandle_t thread_handle;
};

typedef struct
{
    u32 gmac_instance; /* select Gmac global object */
    u32 isr_priority;  /* irq Priority */
    struct Ipv4Address address;
    /* Gmac input thread */
    struct GmacThread mac_input_thread;
} FtOsGmacConfig;

typedef struct
{
    FGmac gmac;
    struct netif netif_object;
    FtOsGmacConfig config;
    u8 *rx_buffer; /* Buffer for RxDesc */
    u8 *tx_buffer; /* Buffer for TxDesc */
    u8 is_ready;   /* Ft_Os_Gmac Object first need Init use Ft_Os_GmacObjec_Init */
    SemaphoreHandle_t s_semaphore;    /*   Semaphore to signal incoming packets */
    EventGroupHandle_t s_status_event; /* Event Group to show netif's status ,follow FT_NETIF_XX*/
} FtOsGmac;
void FtOsGmacObjectInit(FtOsGmac *os_gmac, FtOsGmacConfig *config);
void FtOsGmacInit(FtOsGmac *os_gmac, FGmacPhy *phy_p);
void FtOsGmacStart(FtOsGmac *os_gmac);
void FtOsGmacStop(FtOsGmac *os_gmac);
int FtOsGmacSetupInterrupt(FGmac *instance_p);

#endif // ! FT_OS_GMAC_H
