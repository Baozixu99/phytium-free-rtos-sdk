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
 * FilePath: ethernetif.h
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:04:37
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include "lwip/err.h"
#include "lwip/netif.h"
#include "fgmac.h"
#include "parameters.h"
#include "sdkconfig.h"

#define GMAC_RX_DESCNUM     CONFIG_GMAC_RX_DESCNUM
#define GMAC_TX_DESCNUM     CONFIG_GMAC_TX_DESCNUM

typedef struct  {
    struct eth_addr *ethaddr;
    FGmac *ethctrl;
}ethernetif;

typedef struct 
{
  FGmac gctrl;
  ethernetif netifctrl;
  /* align buf and descriptor by 128 */
  u8 tx_buf[GMAC_TX_DESCNUM * GMAC_MAX_PACKET_SIZE] __aligned(GMAC_DMA_MIN_ALIGN);
  u8 rx_buf[GMAC_RX_DESCNUM * GMAC_MAX_PACKET_SIZE] __aligned(GMAC_DMA_MIN_ALIGN);
  u8 tx_desc[GMAC_TX_DESCNUM * sizeof(FGmacDmaDesc)] __aligned(GMAC_DMA_MIN_ALIGN);
  u8 rx_desc[GMAC_RX_DESCNUM * sizeof(FGmacDmaDesc) + 128] __aligned(GMAC_DMA_MIN_ALIGN);
}netif_config;

err_t ethernetif_init(struct netif *netif);
void ethernetif_input(void const *argument);

#endif