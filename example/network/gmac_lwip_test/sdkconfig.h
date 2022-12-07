#ifndef SDK_CONFIG_H__
#define SDK_CONFIG_H__

/* Project Configuration */

#define CONFIG_TARGET_NAME "d2000_freertos_a64"
#define CONFIG_LWIP_IPV4_TEST
/* CONFIG_LWIP_IPV4_DHCP_TEST is not set */
/* CONFIG_LWIP_IPV6_TEST is not set */
#define CONFIG_GMAC_RX_DESCNUM 16
#define CONFIG_GMAC_TX_DESCNUM 16
#define CONFIG_GMAC_IRQ_PRIORITY 12
/* end of Project Configuration */

/* Standalone Setting */

#define CONFIG_USE_FREERTOS

/* Arch Configuration */

/* CONFIG_TARGET_ARMV8_AARCH32 is not set */
#define CONFIG_TARGET_ARMV8_AARCH64
#define CONFIG_USE_CACHE
#define CONFIG_USE_L3CACHE
#define CONFIG_USE_MMU
#define CONFIG_USE_SYS_TICK
/* CONFIG_MMU_DEBUG_PRINTS is not set */
/* end of Arch Configuration */

/* Board Configuration */

/* CONFIG_TARGET_F2000_4 is not set */
#define CONFIG_TARGET_D2000
/* CONFIG_TARGET_E2000Q is not set */
/* CONFIG_TARGET_E2000D is not set */
/* CONFIG_TARGET_E2000S is not set */
#define CONFIG_DEFAULT_DEBUG_PRINT_UART1
/* CONFIG_DEFAULT_DEBUG_PRINT_UART0 is not set */
/* CONFIG_DEFAULT_DEBUG_PRINT_UART2 is not set */
/* end of Board Configuration */

/* Components Configuration */

/* CONFIG_USE_SPI is not set */
/* CONFIG_USE_QSPI is not set */
#define CONFIG_USE_GIC
#define CONFIG_ENABLE_GICV3
#define CONFIG_USE_SERIAL

/* Usart Configuration */

#define CONFIG_ENABLE_Pl011_UART
/* end of Usart Configuration */
/* CONFIG_USE_GPIO is not set */
#define CONFIG_USE_ETH

/* Eth Configuration */

/* CONFIG_ENABLE_FXMAC is not set */
#define CONFIG_ENABLE_FGMAC
#define CONFIG_FGMAC_PHY_COMMON
/* CONFIG_FGMAC_PHY_AR803X is not set */
/* end of Eth Configuration */
/* CONFIG_USE_CAN is not set */
/* CONFIG_USE_I2C is not set */
/* CONFIG_USE_TIMER is not set */
/* CONFIG_USE_MIO is not set */
/* CONFIG_USE_SDMMC is not set */
/* CONFIG_USE_PCIE is not set */
/* CONFIG_USE_WDT is not set */
/* CONFIG_USE_DMA is not set */
/* CONFIG_USE_NAND is not set */
/* CONFIG_USE_RTC is not set */
/* CONFIG_USE_SATA is not set */
/* CONFIG_USE_USB is not set */
/* CONFIG_USE_ADC is not set */
/* CONFIG_USE_PWM is not set */
/* CONFIG_USE_IPC is not set */
/* end of Components Configuration */
#define CONFIG_USE_NEW_LIBC
/* end of Standalone Setting */

/* Building Option */

/* CONFIG_LOG_VERBOS is not set */
/* CONFIG_LOG_DEBUG is not set */
#define CONFIG_LOG_INFO
/* CONFIG_LOG_WARN is not set */
/* CONFIG_LOG_ERROR is not set */
/* CONFIG_LOG_NONE is not set */
#define CONFIG_USE_DEFAULT_INTERRUPT_CONFIG
#define CONFIG_INTERRUPT_ROLE_MASTER
/* CONFIG_INTERRUPT_ROLE_SLAVE is not set */
/* CONFIG_LOG_EXTRA_INFO is not set */
/* CONFIG_BOOTUP_DEBUG_PRINTS is not set */

/* Linker Options */

/* CONFIG_AARCH32_RAM_LD is not set */
#define CONFIG_AARCH64_RAM_LD
/* CONFIG_USER_DEFINED_LD is not set */
#define CONFIG_LINK_SCRIPT_ROM
#define CONFIG_ROM_START_UP_ADDR 0x80100000
#define CONFIG_ROM_SIZE_MB 1
#define CONFIG_LINK_SCRIPT_RAM
#define CONFIG_RAM_START_UP_ADDR 0x81000000
#define CONFIG_RAM_SIZE_MB 64
#define CONFIG_HEAP_SIZE 1
#define CONFIG_STACK_SIZE 0x100000
#define CONFIG_FPU_STACK_SIZE 0x1000
/* end of Linker Options */

/* Compiler Options */

/* Cross-Compiler Setting */

#define CONFIG_GCC_OPTIMIZE_LEVEL 0
/* CONFIG_USE_EXT_COMPILER is not set */
/* CONFIG_USE_KLIN_SYS is not set */
/* end of Cross-Compiler Setting */
/* CONFIG_OUTPUT_BINARY is not set */
/* end of Compiler Options */
/* end of Building Option */

/* Component Configuration */

/* Freertos Uart Drivers */

#define CONFIG_FREERTOS_USE_UART
/* end of Freertos Uart Drivers */

/* Freertos Pwm Drivers */

/* CONFIG_FREERTOS_USE_PWM is not set */
/* end of Freertos Pwm Drivers */

/* Freertos Qspi Drivers */

/* CONFIG_FREERTOS_USE_QSPI is not set */
/* end of Freertos Qspi Drivers */

/* Freertos Wdt Drivers */

/* CONFIG_FREERTOS_USE_WDT is not set */
/* end of Freertos Wdt Drivers */

/* Freertos Eth Drivers */

/* CONFIG_FREERTOS_USE_XMAC is not set */
/* end of Freertos Eth Drivers */

/* Freertos Gpio Drivers */

/* CONFIG_FREERTOS_USE_GPIO is not set */
/* end of Freertos Gpio Drivers */

/* Freertos Spim Drivers */

/* CONFIG_FREERTOS_USE_FSPIM is not set */
/* end of Freertos Spim Drivers */

/* Freertos DMA Drivers */

/* CONFIG_FREERTOS_USE_FDDMA is not set */
/* CONFIG_FREERTOS_USE_FGDMA is not set */
/* end of Freertos DMA Drivers */

/* Freertos Adc Drivers */

/* CONFIG_FREERTOS_USE_ADC is not set */
/* end of Freertos Adc Drivers */

/* Freertos Can Drivers */

/* CONFIG_FREERTOS_USE_CAN is not set */
/* end of Freertos Can Drivers */

/* Freertos I2c Drivers */

/* CONFIG_FREERTOS_USE_I2C is not set */
/* end of Freertos I2c Drivers */

/* Freertos Mio Drivers */

/* CONFIG_FREERTOS_USE_MIO is not set */
/* end of Freertos Mio Drivers */

/* Freertos Timer Drivers */

/* CONFIG_FREERTOS_USE_TIMER is not set */
/* end of Freertos Timer Drivers */
/* end of Component Configuration */

/* Third-Party Configuration */

#define CONFIG_USE_LWIP

/* LWIP Configuration */

/* LWIP Port Configuration */

#define CONFIG_LWIP_FGMAC
/* CONFIG_LWIP_FXMAC is not set */
/* end of LWIP Port Configuration */
#define CONFIG_LWIP_LOCAL_HOSTNAME "phytium"

/* memory configuration */

#define CONFIG_LWIP_USE_MEM_POOL
/* CONFIG_LWIP_USE_MEM_HEAP is not set */
#define CONFIG_MEMP_NUM_PBUF 64
#define CONFIG_MEM_ALIGNMENT 64
/* end of memory configuration */

/* NETWORK_INTERFACE_OPTIONS */

/* CONFIG_LWIP_NETIF_API is not set */
/* CONFIG_LWIP_NETIF_STATUS_CALLBACK is not set */
/* end of NETWORK_INTERFACE_OPTIONS */

/* LOOPIF */

#define CONFIG_LWIP_NETIF_LOOPBACK
#define CONFIG_LWIP_LOOPBACK_MAX_PBUFS 8
/* end of LOOPIF */

/* SLIPIF */

/* CONFIG_LWIP_SLIP_SUPPORT is not set */
/* end of SLIPIF */

/* Pbuf options */

#define CONFIG_PBUF_POOL_BUFSIZE 2
/* end of Pbuf options */

/* Internal Memory Pool Sizes */

#define CONFIG_PBUF_POOL_SIZE 1
/* end of Internal Memory Pool Sizes */
#define CONFIG_LWIP_MAX_SOCKETS 10

/* LWIP RAW API */

#define CONFIG_LWIP_MAX_RAW_PCBS 16
/* end of LWIP RAW API */

/* TCP */

#define CONFIG_LWIP_MAX_ACTIVE_TCP 16
#define CONFIG_LWIP_MAX_LISTENING_TCP 16
#define CONFIG_LWIP_TCP_HIGH_SPEED_RETRANSMISSION
#define CONFIG_LWIP_TCP_MAXRTX 12
#define CONFIG_LWIP_TCP_SYNMAXRTX 12
#define CONFIG_LWIP_TCP_MSS 1440
#define CONFIG_LWIP_TCP_TMR_INTERVAL 250
#define CONFIG_LWIP_TCP_MSL 60000
#define CONFIG_LWIP_TCP_SND_BUF_DEFAULT 5744
#define CONFIG_LWIP_TCP_WND_DEFAULT 5744
#define CONFIG_LWIP_TCP_RECVMBOX_SIZE 6
#define CONFIG_LWIP_TCP_QUEUE_OOSEQ
/* CONFIG_LWIP_TCP_SACK_OUT is not set */
#define CONFIG_LWIP_TCP_OVERSIZE_MSS
/* CONFIG_LWIP_TCP_OVERSIZE_QUARTER_MSS is not set */
/* CONFIG_LWIP_TCP_OVERSIZE_DISABLE is not set */
/* end of TCP */

/* UDP */

#define CONFIG_LWIP_MAX_UDP_PCBS 16
#define CONFIG_LWIP_UDP_RECVMBOX_SIZE 6
/* CONFIG_LWIP_NETBUF_RECVINFO is not set */
/* end of UDP */

/* IPv4 */

/* CONFIG_USE_IPV4_ONLY is not set */
/* CONFIG_LWIP_IP4_REASSEMBLY is not set */
#define CONFIG_LWIP_IP4_FRAG
/* CONFIG_LWIP_IP_FORWARD is not set */
#define CONFIG_IP_REASS_MAX_PBUFS 16
/* end of IPv4 */

/* ICMP */

#define CONFIG_LWIP_ICMP
/* CONFIG_LWIP_MULTICAST_PING is not set */
/* CONFIG_LWIP_BROADCAST_PING is not set */
/* end of ICMP */

/* DHCP */

/* CONFIG_LWIP_DHCP_ENABLE is not set */
#define CONFIG_LWIP_DHCP_DOES_ARP_CHECK
/* CONFIG_LWIP_DHCP_GET_NTP_SRV is not set */
/* CONFIG_LWIP_DHCP_DISABLE_CLIENT_ID is not set */
/* CONFIG_LWIP_DHCP_RESTORE_LAST_IP is not set */
#define CONFIG_LWIP_DHCP_OPTIONS_LEN 68
#define CONFIG_LWIP_DHCP_DISABLE_VENDOR_CLASS_ID
/* end of DHCP */

/* AUTOIP */

/* CONFIG_LWIP_AUTOIP is not set */
/* end of AUTOIP */

/* DNS */

#define CONFIG_LWIP_DNS_SUPPORT_MDNS_QUERIES
/* end of DNS */

/* TCP options */

#define CONFIG_LWIP_TCP_RTO_TIME 1500
/* end of TCP options */
/* CONFIG_LWIP_TCPIP_CORE_LOCKING is not set */

/* socket */

/* CONFIG_LWIP_SO_LINGER is not set */
#define CONFIG_LWIP_SO_REUSE
#define CONFIG_LWIP_SO_REUSE_RXTOALL
/* end of socket */
/* CONFIG_LWIP_STATS is not set */

/* PPP */

/* CONFIG_LWIP_PPP_SUPPORT is not set */
#define CONFIG_LWIP_IPV6_MEMP_NUM_ND6_QUEUE 3
#define CONFIG_LWIP_IPV6_ND6_NUM_NEIGHBORS 5
/* end of PPP */

/* Checksums */

/* CONFIG_LWIP_CHECKSUM_CHECK_IP is not set */
/* CONFIG_LWIP_CHECKSUM_CHECK_UDP is not set */
#define CONFIG_LWIP_CHECKSUM_CHECK_ICMP
/* end of Checksums */

/* ipv6 */

#define CONFIG_LWIP_IPV6
/* CONFIG_LWIP_IPV6_AUTOCONFIG is not set */
#define CONFIG_LWIP_IPV6_NUM_ADDRESSES 3
/* CONFIG_LWIP_IPV6_FORWARD is not set */
#define CONFIG_LWIP_IP6_FRAG
/* CONFIG_LWIP_IP6_REASSEMBLY is not set */
/* end of ipv6 */
/* CONFIG_LWIP_DEBUG is not set */
/* end of LWIP Configuration */
#define CONFIG_USE_BACKTRACE
/* CONFIG_USE_FATFS is not set */
/* CONFIG_USE_FATFS_0_1_4 is not set */
/* CONFIG_USE_SFUD is not set */
/* CONFIG_USE_SPIFFS is not set */
/* CONFIG_USE_AMP is not set */
#define CONFIG_USE_LETTER_SHELL

/* Letter Shell Configuration */

#define CONFIG_LS_PL011_UART
#define CONFIG_DEFAULT_LETTER_SHELL_USE_UART1
/* CONFIG_DEFAULT_LETTER_SHELL_USE_UART0 is not set */
/* CONFIG_DEFAULT_LETTER_SHELL_USE_UART2 is not set */
/* end of Letter Shell Configuration */
#define CONFIG_USE_TLSF
/* CONFIG_USE_SDMMC_CMD is not set */
/* CONFIG_USE_CHERRY_USB is not set */
/* end of Third-Party Configuration */

/* Kernel Configuration */

#define CONFIG_FREERTOS_OPTIMIZED_SCHEDULER
#define CONFIG_FREERTOS_HZ 1000
#define CONFIG_FREERTOS_MAX_PRIORITIES 32
#define CONFIG_FREERTOS_KERNEL_INTERRUPT_PRIORITIES 13
#define CONFIG_FREERTOS_MAX_API_CALL_INTERRUPT_PRIORITIES 11
#define CONFIG_FREERTOS_THREAD_LOCAL_STORAGE_POINTERS 1
#define CONFIG_FREERTOS_MINIMAL_TASK_STACKSIZE 1024
#define CONFIG_FREERTOS_MAX_TASK_NAME_LEN 32
#define CONFIG_FREERTOS_TIMER_TASK_PRIORITY 1
#define CONFIG_FREERTOS_TIMER_TASK_STACK_DEPTH 2048
#define CONFIG_FREERTOS_TIMER_QUEUE_LENGTH 10
#define CONFIG_FREERTOS_QUEUE_REGISTRY_SIZE 0
#define CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
#define CONFIG_FREERTOS_USE_TRACE_FACILITY
#define CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
/* CONFIG_FREERTOS_USE_TICKLESS_IDLE is not set */
#define CONFIG_FREERTOS_TOTAL_HEAP_SIZE 10240
/* end of Kernel Configuration */

#endif
