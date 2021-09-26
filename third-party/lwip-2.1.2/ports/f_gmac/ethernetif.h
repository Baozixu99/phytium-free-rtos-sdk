/*
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-08-17 10:15:46
 * @LastEditTime: 2021-08-25 19:23:49
 * @Description:  This files is for lwip ports
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */
#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);
void ethernetif_input(void const *argument);
void ethernetif_notify_conn_changed(struct netif *netif);

#endif