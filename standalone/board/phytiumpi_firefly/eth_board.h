/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: eth_board.h
 * Created Date: 2023-09-27 10:01:22
 * Last Modified: 2023-10-11 14:58:10
 * Description:  This file is for recording the mac config on the firefly board.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   liuzhihong   2023/10/8         first release
 */
#ifndef ETH_BOARD_H
#define ETH_BOARD_H


#include "fparameters.h"
#include "lwip_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAC_NUM                 2

/* 网卡接口与实际控制器的对应关系 */
#define MAC_NUM0                0
#define MAC_NUM0_CONTROLLER     FXMAC0_ID
#define MAC_NUM0_LWIP_PORT_TYPE LWIP_PORT_TYPE_XMAC
#define MAC_NUM0_MII_INTERFACE  LWIP_PORT_INTERFACE_SGMII

#define MAC_NUM0_IP             "192.168.4.70"
#define MAC_NUM0_ADDR                      \
    {                                      \
        0x98, 0x0e, 0x24, 0x00, 0x11, 0x50 \
    }
#define MAC_NUM0_IP_ADDR0       192 // IP 地址第一段
#define MAC_NUM0_IP_ADDR1       168 // IP 地址第二段
#define MAC_NUM0_IP_ADDR2       4   // IP 地址第三段
#define MAC_NUM0_IP_ADDR3       70  // IP 地址第四段

#define MAC_NUM1                1
#define MAC_NUM1_CONTROLLER     FXMAC1_ID
#define MAC_NUM1_LWIP_PORT_TYPE LWIP_PORT_TYPE_XMAC
#define MAC_NUM1_MII_INTERFACE  LWIP_PORT_INTERFACE_SGMII

#define MAC_NUM1_IP             "192.168.4.71"
#define MAC_NUM1_ADDR                      \
    {                                      \
        0x98, 0x0e, 0x24, 0x00, 0x11, 0x51 \
    }
#define MAC_NUM1_IP_ADDR0 192 // IP 地址第一段
#define MAC_NUM1_IP_ADDR1 168 // IP 地址第二段
#define MAC_NUM1_IP_ADDR2 4   // IP 地址第三段
#define MAC_NUM1_IP_ADDR3 71  // IP 地址第四段

#define UDP_LOCAL_PORT    5011
#define UDP_REMOTE_PORT   5012

#define TCP_CLIENT_PORT   5006
#define TCP_SERVER_PORT   5006

#ifdef __cplusplus
}
#endif

#endif