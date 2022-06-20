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
 * FilePath: lwipopts.h
 * Date: 2022-04-02 16:43:32
 * LastEditTime: 2022-04-19 21:27:57
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver Who      Date        Changes
 * ----- ------     --------    --------------------------------------
 */

#ifndef __LWIPOPTS_H_
#define __LWIPOPTS_H_

#ifndef PROCESSOR_LITTLE_ENDIAN
#define PROCESSOR_LITTLE_ENDIAN
#endif

#define SYS_LIGHTWEIGHT_PROT 1

#define NO_SYS 1
#define LWIP_SOCKET 0
#define LWIP_COMPAT_SOCKETS 0
#define LWIP_NETCONN 0

#define NO_SYS_NO_TIMERS 0

#define LWIP_TCP_KEEPALIVE 0

#define MEM_ALIGNMENT 64
#define MEM_SIZE 131072
#define MEMP_NUM_PBUF 16
#define MEMP_NUM_UDP_PCB 4
#define MEMP_NUM_TCP_PCB 32
#define MEMP_NUM_TCP_PCB_LISTEN 8
#define MEMP_NUM_TCP_SEG 256
#define MEMP_NUM_SYS_TIMEOUT 8
#define MEMP_NUM_NETBUF 8
#define MEMP_NUM_NETCONN 16
#define MEMP_NUM_TCPIP_MSG_API 16
#define MEMP_NUM_TCPIP_MSG_INPKT 64

#define MEMP_NUM_SYS_TIMEOUT 8
#define PBUF_POOL_SIZE 2048
#define PBUF_POOL_BUFSIZE 1700
#define PBUF_LINK_HLEN 16

#define ARP_TABLE_SIZE 10
#define ARP_QUEUEING 1

#define ICMP_TTL 255

#define IP_OPTIONS 0
#define IP_FORWARD 0
#define IP_REASSEMBLY 1
#define IP_FRAG 1
#define IP_REASS_MAX_PBUFS 128
#define IP_FRAG_MAX_MTU 1500
#define IP_DEFAULT_TTL 255
#define LWIP_CHKSUM_ALGORITHM 3

#define LWIP_UDP 1
#define UDP_TTL 255

#define LWIP_TCP 1
#define TCP_MSS 1460
#define TCP_SND_BUF 8192
#define TCP_WND 2048
#define TCP_TTL 255
#define TCP_MAXRTX 12
#define TCP_SYNMAXRTX 4
#define TCP_QUEUE_OOSEQ 1
#define TCP_SND_QUEUELEN   16 * TCP_SND_BUF/TCP_MSS
#define CHECKSUM_GEN_TCP 	0
#define CHECKSUM_GEN_UDP 	0
#define CHECKSUM_GEN_IP  	0
#define CHECKSUM_CHECK_TCP  0
#define CHECKSUM_CHECK_UDP  0
#define CHECKSUM_CHECK_IP 	0
#define LWIP_FULL_CSUM_OFFLOAD_RX  1
#define LWIP_FULL_CSUM_OFFLOAD_TX  1

#define MEMP_SEPARATE_POOLS 1
#define MEMP_NUM_FRAG_PBUF 256
#define IP_OPTIONS_ALLOWED 0
#define TCP_OVERSIZE TCP_MSS

#define LWIP_DHCP 0
#define DHCP_DOES_ARP_CHECK 0

#define CONFIG_LINKSPEED_AUTODETECT 1

/*
#define LWIP_IPV4 1
#define LWIP_IPV6 0

#define LWIP_ARP 1
#define ARP_TABLE_SIZE 10
#define ARP_QUEUEING 1
*/
#define LWIP_DEBUG


#endif
