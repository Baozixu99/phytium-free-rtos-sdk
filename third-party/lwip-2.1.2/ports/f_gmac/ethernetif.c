/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include <string.h>
#include "../include/lwip/opt.h"
#include "../include/lwip/mem.h"
#include "../include/lwip/memp.h"
#include "../include/lwip/timeouts.h"
#include "../include/netif/ethernet.h"
#include "../include/lwip/etharp.h"
#include "../include/lwip/debug.h"
#include "ethernetif.h"
#include "sdkconfig.h"
#include "parameters.h"

#include "ft_os_gmac.h"
#include "fgmac.h"
#include "fgmac_hw.h"
#include "fgmac_phy.h"

#include "ft_assert.h"
#include "interrupt.h"

#ifndef SDK_CONFIG_H__
	#error "Please include sdkconfig.h first"
#endif

#ifndef CONFIG_USE_SYS_TICK
	#error "Please enable system tick by CONFIG_USE_SYS_TICK first"
#endif
#include "generic_timer.h"
#include "fgmac.h"
#include "ft_debug.h"

/* The time to block waiting for input. */
#define TIME_WAITING_FOR_INPUT (portMAX_DELAY)

/* Stack size of the interface thread */
#define INTERFACE_THREAD_STACK_SIZE (350)

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'

#define ETHNETIF_DEBUG_TAG "ETHNETIF"

#define ETHNETIF_DEBUG_I(format, ...) FT_DEBUG_PRINT_I(ETHNETIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define ETHNETIF_DEBUG_E(format, ...) FT_DEBUG_PRINT_E(ETHNETIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define ETHNETIF_DEBUG_W(format, ...) FT_DEBUG_PRINT_W(ETHNETIF_DEBUG_TAG, format, ##__VA_ARGS__)

extern FError FGmacWritePhyReg(FGmacPhy *instance_p, u16 phy_reg, u32 phy_reg_val);
extern FError FGmacReadPhyReg(FGmacPhy *instance_p, u16 phy_reg, u32 *phy_reg_val_p);

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
    struct eth_addr *ethaddr;
    FGmac *ethctrl;
};

static FGmac gctrl; /* huge size ctrl block */
static FGmacPhy phy;

static struct ethernetif netifctrl;
/* align buf and descriptor by 128 */
static u8 tx_buf[GMAC_TX_DESCNUM * GMAC_MAX_PACKET_SIZE] __aligned(GMAC_DMA_MIN_ALIGN);
static u8 rx_buf[GMAC_RX_DESCNUM * GMAC_MAX_PACKET_SIZE] __aligned(GMAC_DMA_MIN_ALIGN);
static FGmacDmaDesc tx_desc[GMAC_TX_DESCNUM] __aligned(GMAC_DMA_MIN_ALIGN);
static FGmacDmaDesc rx_desc[GMAC_RX_DESCNUM] __aligned(GMAC_DMA_MIN_ALIGN);

/**
 * @name: eth_ctrl_init
 * @msg: config gmac and initialization
 * @return {*}
 * @param {FGmac} *pctrl
 */
u32 eth_ctrl_init(FGmac *pctrl)
{
	LWIP_ASSERT("pctrl != NULL", (pctrl != NULL));
	u32 ret = FGMAC_SUCCESS;
	return ret;
}


void ethernet_link_thread(void *argument)
{
  EventBits_t ev;
  FtOsGmac *os_gmac_ptr = (FtOsGmac *)argument;
  struct netif *netif = &os_gmac_ptr->netif_object;
  u32 last_status = 0;
  u32 flag;
  
  for (;;)
  {
    ev = xEventGroupWaitBits(os_gmac_ptr->s_status_event,
                             FT_NETIF_LINKUP | FT_NETIF_DOWN,
                             pdTRUE, pdFALSE, portMAX_DELAY);

    if (ev & FT_NETIF_DOWN)
    {
      netif_set_link_down(netif);
      netif_set_down(netif);
      last_status = FT_NETIF_DOWN;
    }

    else if (ev & FT_NETIF_LINKUP)
    {
      flag = (last_status == FT_NETIF_LINKUP) ? 0 : 1;
      last_status = FT_NETIF_LINKUP;
	    ETHNETIF_DEBUG_I("FT_NETIF_LINKUP Linkup \r\n");
    }
    else
    {
      ETHNETIF_DEBUG_I("EventGroup is error \r\n");
      FT_ASSERTVOIDALWAYS();
    }

    if (flag)
    {
      flag = 0;
      netif_set_link_down(netif);
      netif_set_down(netif);
      ETHNETIF_DEBUG_I(" Start Linkup \r\n");
      FtOsGmacStop(os_gmac_ptr);
      FtOsGmacStart(os_gmac_ptr);
      ETHNETIF_DEBUG_I(" HardWare is ok \r\n");
      if (last_status == FT_NETIF_LINKUP)
      {
        vTaskDelay(500);
        netif_set_up(netif);
        netif_set_link_up(netif);
      }
    }
  }
}

void GmacReceiveCallBack(void *args)
{
  FtOsGmac *os_gmac_ptr;
  os_gmac_ptr = (FtOsGmac *)args;
  xSemaphoreGiveFromISR(os_gmac_ptr->s_semaphore, 0);
}


void GmacStatusCheckCallBack(void *args, u32 mac_phy_status)
{
  struct netif *netifPtr;
  FtOsGmac *Os_GmacPtr;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  BaseType_t xResult = pdFALSE;

  netifPtr = (struct netif *)args;
  Os_GmacPtr = container_of(netifPtr, FtOsGmac, netif_object);

  if (mac_phy_status & 0x8)
  {
    ETHNETIF_DEBUG_I("netif_set_link_up \r\n");
    xResult = xEventGroupSetBitsFromISR(Os_GmacPtr->s_status_event, FT_NETIF_LINKUP, &xHigherPriorityTaskWoken);
  }
  else
  {
    ETHNETIF_DEBUG_I("netif_set_link_down \r\n");
    xResult = xEventGroupSetBitsFromISR(Os_GmacPtr->s_status_event, FT_NETIF_DOWN, &xHigherPriorityTaskWoken);
  }

  if (xResult != pdFAIL)
  {
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}


/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
	u32 reg_value = 0;
	LWIP_ASSERT("netif != NULL", (netif != NULL));
	u32 ret = FGMAC_SUCCESS;
  	FGmacMacAddr mac_addr;
	FGmac *p_ctrl;
	FtOsGmac *os_gmac_ptr;
	os_gmac_ptr = container_of(netif, FtOsGmac, netif_object);

	p_ctrl = &os_gmac_ptr->gmac;
	
	/* Init Gmac */
  	FtOsGmacInit(os_gmac_ptr, &phy);

  	/* Set Receive Callback */
	FGmacRegisterEvtHandler(p_ctrl, FGMAC_RX_COMPLETE_EVT, GmacReceiveCallBack);

#if LWIP_ARP || LWIP_ETHERNET

	/* set MAC hardware address length */
	netif->hwaddr_len = ETH_HWADDR_LEN;

  /* set MAC hardware address */
	memset(mac_addr, 0, sizeof(mac_addr));
	FGmacGetMacAddr(os_gmac_ptr->gmac.config.base_addr, mac_addr);
  
	/* set MAC hardware address */
	netif->hwaddr[0] = mac_addr[0];
	netif->hwaddr[1] = mac_addr[1];
	netif->hwaddr[2] = mac_addr[2];
	netif->hwaddr[3] = mac_addr[3];
	netif->hwaddr[4] = mac_addr[4];
	netif->hwaddr[5] = mac_addr[5];

	/* maximum transfer unit */
	netif->mtu = GMAC_MTU;

/* Accept broadcast address and ARP traffic */
/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
#if LWIP_ARP
  	netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
#else
  	netif->flags |= NETIF_FLAG_BROADCAST;
#endif /* LWIP_ARP */

  	/* Create the task that handles the ETH_MAC */
	if (xTaskCreate((TaskFunction_t)ethernetif_input,
					os_gmac_ptr->config.mac_input_thread.thread_name,
					os_gmac_ptr->config.mac_input_thread.stack_depth,
					os_gmac_ptr, os_gmac_ptr->config.mac_input_thread.priority,
					&os_gmac_ptr->config.mac_input_thread.thread_handle) != pdPASS)
	{
    ETHNETIF_DEBUG_I("xTaskCreate is Error  %s\r\n", os_gmac_ptr->config.mac_input_thread.thread_name);
    FT_ASSERTVOIDALWAYS();
	}

	/* Enable MAC and DMA transmission and reception */
	FtOsGmacStart(os_gmac_ptr);
	
	/* Read Register Configuration */
	FGmacReadPhyReg(&phy, PHY_INTERRUPT_ENABLE_OFFSET, &reg_value);
	reg_value |= (PHY_INTERRUPT_ENABLE_LINK_FAIL);

	/* Enable Interrupt on change of link status */
	FGmacWritePhyReg(&phy, PHY_INTERRUPT_ENABLE_OFFSET, reg_value);

	/* Read Register Configuration */
	FGmacReadPhyReg(&phy, PHY_INTERRUPT_ENABLE_OFFSET, &reg_value);
	
#endif /* LWIP_ARP || LWIP_ETHERNET */

	LWIP_DEBUGF(NETIF_DEBUG, ("init success\n"));
	return;
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 * 
 * 发送数据的过程，用户在应用层想要通过一个网卡发送数据，那么就要将数据传入LwIP内核中，
 * 经过内核进行传输层封装、IP层封装等。简单来说，就是上层将要发送的数据层层封装，
 * 存储在pbuf数据包中，可能数据很大，需要多个pbuf才能存放得下，
 * 这时pbuf就以链表的形式存在，当发送数据的时候，就要将属于一个数据包的数据全部发送出去。
 * 此处需要注意的是，属于同一个数据包的所有数据都必须放在同一个以太网帧中发送。
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  err_t errval;
  struct pbuf *q;
  u8 *buffer = NULL;
  volatile FGmacDmaDesc *dma_tx_desc;
  u32 frame_length = 0;
  u32 buffer_offset = 0;
  u32 bytes_left_to_copy = 0;
  u32 pay_load_offset = 0;

  FGmac *gmac;
  FtOsGmac *os_gmac;
  os_gmac = (FtOsGmac *)container_of(netif, FtOsGmac, netif_object);
  gmac = &os_gmac->gmac;
  dma_tx_desc = &gmac->tx_desc[gmac->tx_ring.desc_buf_idx];
  buffer = (u8 *)(intptr)(dma_tx_desc->buf_addr);

  if (buffer == NULL)
  {
    ETHNETIF_DEBUG_I(" error buffer is 0 \r\n");
    return ERR_VAL;
  }

#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

  for (q = p; q != NULL; q = q->next)
  {
    /* Is this buffer available? If not, goto error */
    if ((dma_tx_desc->status & FGMAC_DMA_TDES0_OWN) != 0)
    {
      errval = ERR_USE;
      ETHNETIF_DEBUG_I("error errval = ERR_USE; \r\n");
      goto error;
    }

    /* Get bytes in current lwIP buffer */

    bytes_left_to_copy = q->len;
    pay_load_offset = 0;

    /* Check if the length of data to copy is bigger than Tx buffer size*/
    while ((bytes_left_to_copy + buffer_offset) > GMAC_MAX_PACKET_SIZE)
    {
      /* Copy data to Tx buffer*/
      memcpy((u8 *)((u8 *)buffer + buffer_offset), (u8 *)((u8 *)q->payload + pay_load_offset), (GMAC_MAX_PACKET_SIZE - buffer_offset));
      FGMAC_DMA_INC_DESC(gmac->tx_ring.desc_buf_idx, gmac->tx_ring.desc_max_num);
      /* Point to next descriptor */
      dma_tx_desc = &gmac->tx_desc[gmac->tx_ring.desc_buf_idx];

      /* Check if the Bufferis available */
      if ((dma_tx_desc->status & FGMAC_DMA_TDES0_OWN) != (u32)0)
      {
        errval = ERR_USE;
        ETHNETIF_DEBUG_I("Check if the Bufferis available \r\n");
        goto error;
      }

      buffer = (u8 *)(intptr)(dma_tx_desc->buf_addr);
      bytes_left_to_copy = bytes_left_to_copy - (GMAC_MAX_PACKET_SIZE - buffer_offset);
      pay_load_offset = pay_load_offset + (GMAC_MAX_PACKET_SIZE - buffer_offset);
      frame_length = frame_length + (GMAC_MAX_PACKET_SIZE - buffer_offset);
      buffer_offset = 0;

      if (buffer == NULL)
      {
        ETHNETIF_DEBUG_I(" error Buffer is 0 \r\n");
		    return ERR_VAL;
	    }
	}

    /* Copy the remaining bytes */
    memcpy((u8 *)((u8 *)buffer + buffer_offset), (u8 *)((u8 *)q->payload + pay_load_offset), bytes_left_to_copy);
    buffer_offset = buffer_offset + bytes_left_to_copy;
    frame_length = frame_length + bytes_left_to_copy;
    FGMAC_DMA_INC_DESC(gmac->tx_ring.desc_buf_idx, gmac->tx_ring.desc_max_num);
  }

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

  FGmacSendFrame(gmac, frame_length);
  
error:
  FGmacResmuDmaUnderflow(gmac->config.base_addr);
  
  return errval;
}


/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 * 
 * 该函数用于从网卡中接收一个数据包，并将数据包封装在pbuf中递交给上层
 */
static struct pbuf *low_level_input(struct netif *netif)
{
  struct pbuf *p = NULL;
  struct pbuf *q = NULL;
  u16 length = 0;
  u8 *buffer;
  volatile FGmacDmaDesc *dma_rx_desc;
  u32 buffer_offset = 0;
  u32 pay_load_offset = 0;
  u32 bytes_left_to_copy = 0;

  u32 desc_buffer_index; /* For Current Desc buffer buf position */
  FtOsGmac *os_gmac;
  FGmac *gmac;

  os_gmac = (FtOsGmac *)container_of(netif, FtOsGmac, netif_object);
  gmac = &os_gmac->gmac;

  /* get received frame */
  if (FGmacRecvFrame(gmac) != FT_SUCCESS)
  {
    return NULL;
  }

  desc_buffer_index = gmac->rx_ring.desc_buf_idx;
  length = (gmac->rx_desc[desc_buffer_index].status & FGMAC_DMA_RDES0_FRAME_LEN_MASK) >> FGMAC_DMA_RDES0_FRAME_LEN_SHIFT;
  buffer = (u8 *)(intptr)(gmac->rx_desc[desc_buffer_index].buf_addr);

  
#if ETH_PAD_SIZE
  length += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

  if (length > 0)
  {
	  /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
	  p = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);

  }

#ifdef RAW_DATA_PRINT
  dump_hex(Buffer, (u32)length);
#endif
  if (p != NULL)
  {
#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
    dma_rx_desc = &gmac->rx_desc[desc_buffer_index];
    buffer_offset = 0;
    for (q = p; q != NULL; q = q->next)
    {
      bytes_left_to_copy = q->len;
      pay_load_offset = 0;
      /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size*/
      while ((bytes_left_to_copy + buffer_offset) > GMAC_MAX_PACKET_SIZE)
      {
        /* Copy data to pbuf */
        memcpy((u8 *)((u8 *)q->payload + pay_load_offset), (u8 *)((u8 *)buffer + buffer_offset), (GMAC_MAX_PACKET_SIZE - buffer_offset));

        /* Point to next descriptor */
        FGMAC_DMA_INC_DESC(desc_buffer_index, gmac->rx_ring.desc_max_num);
        if (desc_buffer_index == gmac->rx_ring.desc_idx)
        {
          break;
        }

        dma_rx_desc = &gmac->rx_desc[desc_buffer_index];
        buffer = (u8 *)(intptr)(dma_rx_desc->buf_addr);

        bytes_left_to_copy = bytes_left_to_copy - (GMAC_MAX_PACKET_SIZE - buffer_offset);
        pay_load_offset = pay_load_offset + (GMAC_MAX_PACKET_SIZE - buffer_offset);
        buffer_offset = 0;
      }
      /* Copy remaining data in pbuf */
	  memcpy((u8 *)((u8 *)q->payload + pay_load_offset), (u8 *)((u8 *)buffer + buffer_offset), bytes_left_to_copy);
	  buffer_offset = buffer_offset + bytes_left_to_copy;
    }

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
  }else
  {
	  printf("error mallco is %d \r\n",length);
  }

  /* Release descriptors to DMA */
  /* Point to first descriptor */
  dma_rx_desc = &gmac->rx_desc[desc_buffer_index];
  /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
  for (desc_buffer_index = gmac->rx_ring.desc_buf_idx; desc_buffer_index != gmac->rx_ring.desc_idx; FGMAC_DMA_INC_DESC(desc_buffer_index, gmac->rx_ring.desc_max_num))
  {
    dma_rx_desc->status |= FGMAC_DMA_RDES0_OWN;
    dma_rx_desc = &gmac->rx_desc[desc_buffer_index];
  }

  /* Sync index */
  gmac->rx_ring.desc_buf_idx = gmac->rx_ring.desc_idx;

  FGmacResumeDmaRecv(gmac->config.base_addr);
  return p;
}


/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void ethernetif_input(void const *argument)
{
  struct pbuf *p = 0;
  FtOsGmac *os_gmac_ptr = (FtOsGmac *)argument;
  struct netif *netif = &os_gmac_ptr->netif_object;

  for (;;)
  {
	  
   if (xSemaphoreTake(os_gmac_ptr->s_semaphore, TIME_WAITING_FOR_INPUT) == pdTRUE)
    {

      do
      {
        LOCK_TCPIP_CORE();
        p = low_level_input(netif);
        if (p != NULL)
        {
          if (netif->input(p, netif) != ERR_OK)
          {
            pbuf_free(p);
          }
        }
        UNLOCK_TCPIP_CORE();
	  } while (p != NULL);
    }
  }
}


static void arp_timer(void *arg)
{
	(void)arg;
	etharp_tmr();
	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}

#if !LWIP_ARP
/**
 * This function has to be completed by user in case of ARP OFF.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if ...
 */
static err_t low_level_output_arp_off(struct netif *netif, struct pbuf *q, const ip4_addr_t *ipaddr)
{  
  	err_t errval;
	errval = ERR_OK;
    
	return errval;
}
#endif /* LWIP_ARP */ 

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 * 接收网卡的数据，接收完毕后将数据封装在pbuf中通过网卡netif的input接口递交给上层
 */
err_t ethernetif_init(struct netif *netif)
{
	LWIP_ASSERT("netif != NULL", (netif != NULL));

	memset(&netifctrl, 0, sizeof(netifctrl));

	LWIP_DEBUGF(NETIF_DEBUG, ("*******start init eth\n"));

#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
  	netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

	netif->state = &netifctrl; // 通过state将ethernetif结构传递到上层
  	netif->name[0] = IFNAME0;
  	netif->name[1] = IFNAME1;
	/* We directly use etharp_output() here to save a function call.
	* You can instead declare your own function an call etharp_output()
	* from it if you have to do some checks before sending (e.g. if link
	* is available...) */

#if LWIP_IPV4
#if LWIP_ARP || LWIP_ETHERNET
#if LWIP_ARP
	netif->output = etharp_output;
#else
  	/* The user should write ist own code in low_level_output_arp_off function */
	netif->output = low_level_output_arp_off;
#endif /* LWIP_ARP */
#endif /* LWIP_ARP || LWIP_ETHERNET */
#endif /* LWIP_IPV4 */

#if LWIP_IPV6
	netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */

	netif->linkoutput = low_level_output;
	/* initialize the hardware */
	netifctrl.ethctrl = &gctrl;
	low_level_init(netif);
	netifctrl.ethaddr = (struct eth_addr *) &(netif->hwaddr[0]);

	return ERR_OK;
}

/**
* @brief  Returns the current time in milliseconds
*         when LWIP_TIMERS == 1 and NO_SYS == 1
* @param  None
* @retval Time
*/
u32_t sys_jiffies(void)
{
	return GenericGetTick();
}

/**
* @brief  Returns the current time in milliseconds
*         when LWIP_TIMERS == 1 and NO_SYS == 1
* @param  None
* @retval Time
*/
u32_t sys_now(void)
{
	return GenericGetTick();
}

/**
 * Sends a single character to the serial device.
 *
 * @param c character to send
 * @param fd serial device handle
 *
 * @note This function will block until the character can be sent.
 */
void sio_send(u8_t c, sio_fd_t fd)
{
	(void)c;
	(void)fd;
}

/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open(u8_t devnum)
{
	sio_fd_t sd;
	(void)devnum;

	sd = 0;

	return sd;
}

/**
 * Reads from the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received - may be 0 if aborted by sio_read_abort
 *
 * @note This function will block until data can be received. The blocking
 * can be cancelled by calling sio_read_abort().
 */
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len)
{
	u32_t recved_bytes;
	(void)len;
	(void)data;
	(void)fd;

	recved_bytes = 0; 

	return recved_bytes;
}

/**
 * Tries to read from the serial device. Same as sio_read but returns
 * immediately if no data is available and never blocks.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received
 */
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len)
{
	u32_t recved_bytes;
	(void)len;
	(void)data;
	(void)fd;

	recved_bytes = 0; // dummy code

	return recved_bytes;
}