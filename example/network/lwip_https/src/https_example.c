/*
 * Copyright : (C) 2023 Phytium Information Technology, Inc.
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
 * FilePath: https_example.c
 * Created Date: 2024-9-29 11:12:04
 * Last Modified: 2024-09-29 15:53:23
 * Description:  This file is for https example function implementation.
 * 
 * Modify History:
 *  Ver      Who         Date               Changes
 * -----  ----------   --------  ---------------------------------
 *  1.0   huangjin     2024/9/29          first release
 */

#include <string.h>
#include <stdio.h>
#include "strto.h"
#include "sdkconfig.h"
#include "ftypes.h"
#include "fassert.h"
#include "fparameters.h"
#include "eth_board.h"
#ifndef SDK_CONFIG_H__
    #error "Please include sdkconfig.h first"
#endif
#include "FreeRTOS.h"

#include "lwip_port.h"
#include "lwip/ip4_addr.h"
#include "lwip/init.h"
#include "netif/ethernet.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"

/* mbedtls头文件 */
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include "tls_certificate.h"
#include "tls_net.h"

/* https server 参数 */
#define SERVER_NAME "localhost"
#define SERVER_IP   "192.168.4.50"
#define SERVER_PORT "4433"

#define GET_REQUEST "GET / HTTP/1.0\r\n\r\n"

#define ETH_NAME_PREFIX 'e'

#define CONFIG_DEFAULT_INIT(config,driver_config,instance_id,interface_type)           \
		.config.magic_code = LWIP_PORT_CONFIG_MAGIC_CODE,  \
		.config.driver_type = driver_config,		  \
		.config.mac_instance = instance_id,                          \
		.config.mii_interface = interface_type, \
		.config.autonegotiation = 1,                       \
		.config.phy_speed = LWIP_PORT_SPEED_1000M,         \
		.config.phy_duplex = LWIP_PORT_FULL_DUPLEX, \
		.config.capability = LWIP_PORT_MODE_NAIVE, 
        
#define TIMER_OUT (pdMS_TO_TICKS(5000UL))

enum
{
    HTTPS_EXAMPLE_SUCCESS = 0,
    HTTPS_EXAMPLE_UNKNOWN_STATE = 1,
    HTTPS_EXAMPLE_INIT_FAILURE = 2,
};
static QueueHandle_t xQueue = NULL;

typedef struct
{
    UserConfig lwip_mac_config;
    u32 dhcp_en;
    char*  ipaddr;
    char*  netmask; 
    char*  gw;
    unsigned char mac_address[6];
    struct netif netif;
} BoardMacConfig;


 static BoardMacConfig board_mac_config[MAC_NUM] = 
 {
    #if defined(MAC_NUM0)
            {
                CONFIG_DEFAULT_INIT(lwip_mac_config,MAC_NUM0_LWIP_PORT_TYPE,MAC_NUM0_CONTROLLER,MAC_NUM0_MII_INTERFACE)
                .dhcp_en=0,
                .ipaddr="192.168.4.10",
                .gw="192.168.4.1",
                .netmask="255.255.255.0",
                .mac_address={0x98, 0x0e, 0x24, 0x00, 0x11, 0x0},
            },
    #endif
    #if defined(MAC_NUM1) 
            {
                CONFIG_DEFAULT_INIT(lwip_mac_config,MAC_NUM1_LWIP_PORT_TYPE,MAC_NUM1_CONTROLLER,MAC_NUM1_MII_INTERFACE)
                .dhcp_en=0,
                .ipaddr="192.168.4.11",
                .gw="192.168.4.1",
                .netmask="255.255.255.0",
                .mac_address={0x98, 0x0e, 0x24, 0x00, 0x11, 0x1},
            },
    #endif
};


static void my_debug( void *ctx, int level, const char *file, int line, const char *str )
{
    ((void) level);
    fprintf( (FILE *) ctx, "%s:%04d: %s", file, line, str );
}

static void SetIP(ip_addr_t* ipaddr,ip_addr_t* gw,ip_addr_t* netmask,u32 mac_id)
{
    if(inet_aton(board_mac_config[mac_id].ipaddr,ipaddr)==0)
        printf("The addr of ipaddr is wrong\r\n");      
    if(inet_aton(board_mac_config[mac_id].gw,gw)==0)
        printf("The addr of gw is wrong\r\n");
    if(inet_aton(board_mac_config[mac_id].netmask,netmask)==0)
        printf("The addr of netmask is wrong\r\n");
}
 
static int entropy_source(void *data, uint8_t *output, size_t len, size_t *olen)
{
    uint32_t seed;

    seed = rand();

    if ( len > sizeof(seed) ) 
    {
        len = sizeof(seed);
    }
    memcpy(output, &seed, len);

    *olen = len;
    return 0;
}

void HttpsInitTask(void)
{
    int task_res = HTTPS_EXAMPLE_SUCCESS;

    /* MAC初始化 */
    for (int i = 0; i < MAC_NUM; i++)
    {
        struct netif *netif_p = NULL;
        ip_addr_t ipaddr,netmask, gw;
        board_mac_config[i].lwip_mac_config.name[0] = ETH_NAME_PREFIX;
        itoa(board_mac_config[i].lwip_mac_config.mac_instance, &(board_mac_config[i].lwip_mac_config.name[1]), 10);

        /* mac ip addr set: char* -> ip_addr_t */
        SetIP(&ipaddr,&gw,&netmask,i);

        netif_p= &board_mac_config[i].netif;
        /* Add network interface to the netif_list, and set it as default */
        if (!LwipPortAdd(netif_p, &ipaddr, &netmask, &gw, board_mac_config[i].mac_address, (UserConfig *)&board_mac_config[i]))
        {
            printf("Error adding N/W interface %d.\n\r",board_mac_config[i].lwip_mac_config.mac_instance);
            task_res = HTTPS_EXAMPLE_INIT_FAILURE;
            goto exit;
        }
        printf("LwipPortAdd mac_instance %d is over.\n\r",board_mac_config[i].lwip_mac_config.mac_instance);

        netif_set_default(netif_p);

        if (netif_is_link_up(netif_p))
        {
            /* 当netif完全配置好时，必须调用该函数 */
            netif_set_up(netif_p);
            if (board_mac_config[i].dhcp_en == 1)
            {
                LwipPortDhcpSet(netif_p, TRUE);
            }
        }
        else
        {
            /* 当netif链接关闭时，必须调用该函数 */
            netif_set_down(netif_p);
        }
    }
    printf("Network setup complete.\n");

    /* 进行Https测试 */
    printf("\n  . Start Https test!");
    int ret, len = 0;
    uint32_t flags;
    uint8_t buf[256];

    const char *pers = "tls_client";

    /* 初始化数据 */
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;   

    mbedtls_net_init(&server_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ssl_config_init(&conf);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    /* 添加熵源接口，设置熵源属性 */ 
    printf("\n  . Seeding the random number generator..."); 
    mbedtls_entropy_add_source(&entropy, 
                                entropy_source, 
                                NULL,
                                MBEDTLS_ENTROPY_MAX_GATHER, //熵源可用阈值，随机数达到阈值时熵源才被使用
                                MBEDTLS_ENTROPY_SOURCE_STRONG); //强熵源，一般是硬件真随机数生成器
    if((task_res = mbedtls_ctr_drbg_seed(&ctr_drbg, 
                                    mbedtls_entropy_func, 
                                    &entropy, 
                                    (const unsigned char *)pers, 
                                    strlen(pers))) != 0)
    {
        printf("failed\n ! mbedtls_ctr_drbg_seed returned %d\n", task_res);
        goto exit;
    }
    printf("ok\n");

    /* 初始化证书 */
    printf( "  . Loading the CA root certificate ..." );
    task_res = mbedtls_x509_crt_parse( &cacert, 
                                        (const unsigned char *) mbedtls_test_cas_pem,
                                        mbedtls_test_cas_pem_len );
    if( task_res < 0 )
    {
        printf( " failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -task_res );
        goto exit;
    }
    printf( " ok (%d skipped)\n", task_res );    

    /* 开始连接 */
    printf( "  . Connecting to tcp/%s/%s...", SERVER_IP, SERVER_PORT );
    if( ( task_res = mbedtls_net_connect( &server_fd, 
                                     SERVER_IP,
                                     SERVER_PORT, 
                                     MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        printf( " failed\n  ! mbedtls_net_connect returned %d\n\n", task_res );
        goto exit;
    }
    printf( " ok\n" );     

    /* TLS握手过程的初始化 */
    printf( "  . Setting up the SSL/TLS structure..." );
    if( ( task_res = mbedtls_ssl_config_defaults( &conf,
                                            MBEDTLS_SSL_IS_CLIENT,
                                            MBEDTLS_SSL_TRANSPORT_STREAM,
                                            MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        printf( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", task_res );
        goto exit;
    }
    printf( " ok\n" );

    /* 配置认证模式、CA证书链、随机数生成器、调试回调、设置TLS结构体、设置服务器主机名和设置TLS连接的底层W/R操作 */
    mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
    mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
    mbedtls_ssl_conf_dbg( &conf, my_debug, stdout );
    if( ( task_res = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 )
    {
        printf( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", task_res );
        goto exit;
    }
    if( ( task_res = mbedtls_ssl_set_hostname( &ssl, SERVER_NAME ) ) != 0 )
    {
        printf( " failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", task_res );
        goto exit;
    }
    mbedtls_ssl_set_bio( &ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL ); 

    /* TLS握手 */   
    printf( "  . Performing the SSL/TLS handshake..." );
    while( ( task_res = mbedtls_ssl_handshake( &ssl ) ) != 0 )
    {
        if( task_res != MBEDTLS_ERR_SSL_WANT_READ && task_res != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            printf( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -task_res );
            goto exit;
        }
    }
    printf( " ok\n" );

    /* 验证服务器证书 */
    printf( "  . Verifying peer X.509 certificate..." );
    if( ( flags = mbedtls_ssl_get_verify_result( &ssl ) ) != 0 )
    {
        char vrfy_buf[512];

        printf( " failed\n" );

        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

        printf( "%s\n", vrfy_buf );
    }
    else
    {
        printf( " ok\n" );   
    }

    /* 编写GET请求 */
    printf( "  > Write to server:" );
    len = sprintf( (char *) buf, GET_REQUEST );
    while( ( ret = mbedtls_ssl_write( &ssl, buf, len ) ) <= 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            printf( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
            goto exit;
        }
    }
    len = ret;
    printf( " %d bytes written\n\n%s", len, (char *) buf );   

    /* 读取HTTPS响应 */
    printf( "  < Read from server:" );
    do
    {
        len = sizeof( buf ) - 1;
        memset( buf, 0, sizeof( buf ) );
        ret = mbedtls_ssl_read( &ssl, buf, len );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 )
        {
            printf( "failed\n  ! mbedtls_ssl_read returned %d\n\n", ret );
            break;
        }

        if( ret == 0 )
        {
            printf( "\n\nEOF\n\n" );
            break;
        }

        len = ret;
        printf( " %d bytes read\n\n%s", len, (char *) buf );
    }
    while( 1 );

    mbedtls_ssl_close_notify( &ssl );     
 

exit:
    mbedtls_net_free( &server_fd );
    mbedtls_x509_crt_free( &cacert );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);

    xQueueSend(xQueue, &task_res, 0);
    vTaskDelete(NULL);
}

int FFreeRTOSHttpsTaskCreate(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int ret = HTTPS_EXAMPLE_UNKNOWN_STATE;

    xQueue = xQueueCreate(1, sizeof(int)); /* 创建消息队列 */
    if (xQueue == NULL)
    {
        printf("xQueue create failed.\r\n");
        goto exit;
    }
    
    xReturn = xTaskCreate((TaskFunction_t)HttpsInitTask, /* 任务入口函数 */
                      (const char *)"HttpsInitTask", /* 任务名字 */
                      4096,                 /* 任务栈大小 */
                        NULL,                   /* 任务入口函数参数 */
                      (UBaseType_t)configMAX_PRIORITIES - 1, /* 任务的优先级 */
                      NULL);                          /* 任务控制块指针 */
    if (xReturn == pdFAIL)
    {
        printf("xTaskCreate HttpsInitTask failed.\r\n");
        goto exit;
    }

    xReturn = xQueueReceive(xQueue, &ret, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        printf("xQueue receive timeout.\r\n");
        goto exit;
    }

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }

    if (ret != HTTPS_EXAMPLE_SUCCESS)
    {
        printf("%s@%d: Https example [failure], ret = %d\r\n", __func__, __LINE__, ret);
        return ret;
    }
    else
    {
        printf("%s@%d: Https example [success].\r\n", __func__, __LINE__);
        return ret;
    }
}

void HttpsTestDeinit(void)
{
    for (int i = 0; i < MAC_NUM; i++)
    {
        struct netif *netif_p = NULL;
        netif_p = &board_mac_config[i].netif;
        vPortEnterCritical();
        LwipPortStop(netif_p,board_mac_config[i].dhcp_en);
        vPortExitCritical(); 
    }
}