/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
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
 * Date: 2022-04-02 16:43:32
 * LastEditTime: 2022-04-19 21:27:57
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver Who      Date        Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/netif.h"
#include "netif/etharp.h"
#include "lwip/sys.h"
#include "lwip_port.h"
#include "f_printk.h"
#include "fxmac.h"
#include "arch/cc.h"

#define MAX_FRAME_SIZE_JUMBO (FXMAC_MTU_JUMBO + FXMAC_HDR_SIZE + FXMAC_TRL_SIZE)

err_t 	ethernetif_init(struct netif *netif);
void 	ethernetif_input(struct netif *netif);



#ifdef __cplusplus
}
#endif

#endif /* __ETHERNETIF_H__ */
