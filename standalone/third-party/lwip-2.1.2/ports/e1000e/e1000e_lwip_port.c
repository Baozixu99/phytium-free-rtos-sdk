/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: e1000e_lwip_port.c
 * Date: 2025-01-06 14:59:22
 * LastEditTime: 2025-01-06 14:59:22
 * Description:  This file is e1000e portable code for lwip port input,output,status check.
 * 
 * Modify History: 
 *  Ver     Who           Date                  Changes
 * -----   ------       --------     --------------------------------------
 *  1.0    huangjin     2025/01/06            first release
 */
#include "fparameters.h"
#include "fassert.h"
#include "e1000e_lwip_port.h"
#include "e1000e.h"
#include "e1000e_hw.h"
#include "fcache.h"
#include "lwip_port.h"
#include "fcpu_info.h"
#include "sys_arch.h"
#include "faarch.h"
#include "finterrupt.h"
#include "fdebug.h"
#include "../include/lwip/sys.h"
#include "eth_ieee_reg.h"

#include "fpcie_ecam.h"
#include "fpcie_ecam_common.h"
#include "fpcie_ecam_msi.h"

#define FE1000E_LWIP_PORT_E1000E_DEBUG_TAG "FE1000E_LWIP_PORT_E1000E"
#define FE1000E_LWIP_PORT_E1000E_PRINT_E(format, ...) \
    FT_DEBUG_PRINT_E(FE1000E_LWIP_PORT_E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_LWIP_PORT_E1000E_PRINT_I(format, ...) \
    FT_DEBUG_PRINT_I(FE1000E_LWIP_PORT_E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_LWIP_PORT_E1000E_PRINT_D(format, ...) \
    FT_DEBUG_PRINT_D(FE1000E_LWIP_PORT_E1000E_DEBUG_TAG, format, ##__VA_ARGS__)
#define FE1000E_LWIP_PORT_E1000E_PRINT_W(format, ...) \
    FT_DEBUG_PRINT_W(FE1000E_LWIP_PORT_E1000E_DEBUG_TAG, format, ##__VA_ARGS__)

static FE1000ELwipPort fe1000e_lwip_port_instance[FE1000E_NUM] = {
    [FE1000E0_ID] = {{{0}}},
};

#define PCIE_MSI_DEVICE_MAX_NUM 32
extern FE1000EConfig FE1000E_CONFIG_TBL[FE1000E_NUM];
FPcieMsiVector msi_vector[FE1000E_NUM][PCIE_MSI_DEVICE_MAX_NUM] = {0};
#define PCI_CLASS_STORAGE_NET_AHCI 0x020000
/*max support 16 ahci controllers*/
static FPcieEcam pcie_device;

/* queue */
void FE1000EQueueInit(PqQueue *q)
{
    FASSERT(q != NULL);
    q->head = q->tail = q->len = 0;
}

int FE1000EPqEnqueue(PqQueue *q, void *p)
{
    if (q->len == PQ_QUEUE_SIZE)
    {
        return -1;
    }

    q->data[q->head] = (uintptr)p;
    q->head = (q->head + 1) % PQ_QUEUE_SIZE;
    q->len++;

    return 0;
}

void *FE1000EPqDequeue(PqQueue *q)
{
    int ptail;

    if (q->len == 0)
    {
        return NULL;
    }

    ptail = q->tail;
    q->tail = (q->tail + 1) % PQ_QUEUE_SIZE;
    q->len--;

    return (void *)q->data[ptail];
}

int FE1000EPqQlength(PqQueue *q)
{
    return q->len;
}

int FE1000ELwipPortRxComplete(FE1000ELwipPort *instance_p)
{
    FE1000ECtrl *e1000e_p = &instance_p->instance;
    u32 temp = 0;
    u32 rx_idx = e1000e_p->rx_ring.desc_idx;

    SYS_ARCH_DECL_PROTECT(lev);
    SYS_ARCH_PROTECT(lev);

    /* 判断是否成功接收 */
    if (e1000e_p->rx[rx_idx].sta & RDESC_STA_DD)
    {
        temp = 1;
    }
    else
    {
        temp = 0;
    }


    SYS_ARCH_UNPROTECT(lev);
    return temp;
}

static FError FE1000ELwipPortConfigConvert(FE1000ELwipPort *instance_p, FE1000EConfig *e1000e_config_p)
{
    FE1000EConfig *mac_config_p;
    FE1000EPhyInterface interface = FE1000E_PHY_INTERFACE_MODE_SGMII;

    mac_config_p = FE1000ELookupConfig(instance_p->e1000e_port_config.instance_id);
    if (mac_config_p == NULL)
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_E("FE1000ELookupConfig is error , instance_id "
                                         "is %d",
                                         instance_p->e1000e_port_config.instance_id);
        return FREERTOS_E1000E_INIT_ERROR;
    }

    memcpy(e1000e_config_p, mac_config_p, sizeof(FE1000EConfig));

    switch (instance_p->e1000e_port_config.interface)
    {
        case FE1000E_LWIP_PORT_INTERFACE_SGMII:
            interface = FE1000E_PHY_INTERFACE_MODE_SGMII;
            FE1000E_LWIP_PORT_E1000E_PRINT_I("SGMII select");
            break;
        case FE1000E_LWIP_PORT_INTERFACE_RMII:
            interface = FE1000E_PHY_INTERFACE_MODE_RMII;
            FE1000E_LWIP_PORT_E1000E_PRINT_I("RMII select");
            break;
        case FE1000E_LWIP_PORT_INTERFACE_RGMII:
            FE1000E_LWIP_PORT_E1000E_PRINT_I("RGMII select");
            interface = FE1000E_PHY_INTERFACE_MODE_RGMII;
            break;
        case FE1000E_LWIP_PORT_INTERFACE_USXGMII:
            FE1000E_LWIP_PORT_E1000E_PRINT_I("USXGMII select");
            instance_p->e1000e_port_config.phy_speed = FE1000E_PHY_SPEED_10G;
            interface = FE1000E_PHY_INTERFACE_MODE_USXGMII;
            break;
        default:
            FE1000E_LWIP_PORT_E1000E_PRINT_E("update interface is error , interface is "
                                             "%d",
                                             instance_p->e1000e_port_config.instance_id);
            return FREERTOS_E1000E_INIT_ERROR;
    }
    e1000e_config_p->interface = interface;

    if (instance_p->e1000e_port_config.autonegotiation)
    {
        e1000e_config_p->auto_neg = 1;
    }
    else
    {
        e1000e_config_p->auto_neg = 0;
    }

    switch (instance_p->e1000e_port_config.phy_speed)
    {
        case FE1000E_PHY_SPEED_10M:
            e1000e_config_p->speed = FE1000E_SPEED_10;
            break;
        case FE1000E_PHY_SPEED_100M:
            e1000e_config_p->speed = FE1000E_SPEED_100;
            break;
        case FE1000E_PHY_SPEED_1000M:
            e1000e_config_p->speed = FE1000E_SPEED_1000;
            FE1000E_LWIP_PORT_E1000E_PRINT_I("select FE1000E_PHY_SPEED_1000M");
            break;
        case FE1000E_PHY_SPEED_10G:
            FE1000E_LWIP_PORT_E1000E_PRINT_I("select FE1000E_PHY_SPEED_10G");
            e1000e_config_p->speed = FE1000E_SPEED_10000;
            break;
        default:
            FE1000E_LWIP_PORT_E1000E_PRINT_E("setting speed is not valid , speed is %d",
                                             instance_p->e1000e_port_config.phy_speed);
            return FREERTOS_E1000E_INIT_ERROR;
    }

    switch (instance_p->e1000e_port_config.phy_duplex)
    {
        case FE1000E_PHY_HALF_DUPLEX:
            e1000e_config_p->duplex = 0;
            FE1000E_LWIP_PORT_E1000E_PRINT_I("select Half Duplex");
            break;
        case FE1000E_PHY_FULL_DUPLEX:
            e1000e_config_p->duplex = 1;
            FE1000E_LWIP_PORT_E1000E_PRINT_I("select Full Duplex");
            break;
    }

    return FT_SUCCESS;
}

static FError FE1000EGetIeeePhySpeed(FE1000ECtrl *instance_p)
{
    u16 temp, temp2;
    u16 control;
    u16 status;
    u32 negotiation_timeout_cnt = 0;
    FError ret;

    FE1000E_LWIP_PORT_E1000E_PRINT_I("Start phy auto negotiation.");

    control = FE1000EPhyRead(instance_p->config.base_addr, PHY_CONTROL_REG_OFFSET);
    control |= PHY_CONTROL_AUTONEGOTIATE_ENABLE;
    control |= PHY_CONTROL_AUTONEGOTIATE_RESTART;
    ret = FE1000EPhyWrite(instance_p->config.base_addr, PHY_CONTROL_REG_OFFSET, control);
    if (ret != FT_SUCCESS)
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_E("%s:%d,write PHY_CONTROL_REG_OFFSET is error",
                                         __func__, __LINE__);
        return ret;
    }

    FE1000E_LWIP_PORT_E1000E_PRINT_I("Waiting for phy to complete auto negotiation.");

    do
    {
        FDriverMdelay(50);
        status = FE1000EPhyRead(instance_p->config.base_addr, PHY_STATUS_REG_OFFSET);


        if (negotiation_timeout_cnt++ >= 0xff)
        {
            FE1000E_LWIP_PORT_E1000E_PRINT_E("Auto negotiation is error.");
            return FE1000E_ERR_PHY_AUTO_FAILED;
        }
    } while (!(status & PHY_STATUS_AUTONEGOTIATE_COMPLETE));

    FE1000E_LWIP_PORT_E1000E_PRINT_I("Auto negotiation complete.");

    temp = FE1000EPhyRead(instance_p->config.base_addr, PHY_SPECIFIC_STATUS_REG);
    FE1000E_LWIP_PORT_E1000E_PRINT_I("Temp is 0x%x", temp);

    temp2 = FE1000EPhyRead(instance_p->config.base_addr, PHY_STATUS_REG_OFFSET);
    FE1000E_LWIP_PORT_E1000E_PRINT_I("Temp2 is 0x%x", temp2);

    if (temp & (1 << 13))
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_I("Duplex is full.");
        instance_p->config.duplex = 1;
    }
    else
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_I("Duplex is half.");
        instance_p->config.duplex = 0;
    }

    if ((temp & 0xC000) == PHY_SPECIFIC_STATUS_SPEED_1000M)
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_I("Speed is 1000M.");
        instance_p->config.speed = 1000;
    }
    else if ((temp & 0xC000) == PHY_SPECIFIC_STATUS_SPEED_100M)
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_I("Speed is 100M.");
        instance_p->config.speed = 100;
    }
    else
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_I("Speed is 10M.");
        instance_p->config.speed = 10;
    }

    return FT_SUCCESS;
}

enum lwip_port_link_status FE1000EPhyReconnect(struct LwipPort *xmac_netif_p)
{
    FE1000ECtrl *e1000e_p;
    FE1000ELwipPort *instance_p;
    FASSERT(xmac_netif_p != NULL);
    FASSERT(xmac_netif_p->state != NULL);

    instance_p = (FE1000ELwipPort *)(xmac_netif_p->state);

    e1000e_p = &instance_p->instance;

    if (e1000e_p->config.interface == FE1000E_PHY_INTERFACE_MODE_SGMII)
    {
        if (e1000e_p->link_status == FE1000E_NEGOTIATION)
        {
            FE1000EIrqDisable(e1000e_p, IMS_TXDW | IMS_LSC | IMS_RXT0 | IMS_RXQ0 | IMS_TXQ0);
            FE1000EGetIeeePhySpeed(e1000e_p);
            FE1000EStart(e1000e_p);
            e1000e_p->link_status = FE1000E_LINKUP;
            FE1000EIrqEnable(e1000e_p, IMS_TXDW | IMS_LSC | IMS_RXT0 | IMS_RXQ0 | IMS_TXQ0);
        }

        switch (e1000e_p->link_status)
        {
            case FE1000E_LINKDOWN:
                return ETH_LINK_DOWN;
            case FE1000E_LINKUP:
                return ETH_LINK_UP;
            default:
                return ETH_LINK_DOWN;
        }
    }
    else
    {
        switch (e1000e_p->link_status)
        {
            case FE1000E_LINKDOWN:
                return ETH_LINK_DOWN;
            case FE1000E_LINKUP:
                return ETH_LINK_UP;
            default:
                return ETH_LINK_DOWN;
        }
    }
}

/* msi irq */
static void FE1000EPCIeMsiIrqHandler(s32 vector, void *param)
{
    FE1000EIrqHandler(0, param);
}

static void FE1000ETransDoneCallback(void *param)
{
}

static void FE1000EReceiveDoneCallBack(void *args)
{
    FE1000ELwipPort *instance_p = (FE1000ELwipPort *)args;
    instance_p->recv_flg++;
}

static void FE1000ELinkChangeCallBack(void *args)
{
    u32 ctrl;
    u32 link, link_status;

    FE1000ECtrl *e1000e_p;
    FE1000ELwipPort *instance_p;

    instance_p = (FE1000ELwipPort *)args;
    e1000e_p = &instance_p->instance;

    if (e1000e_p->config.interface == FE1000E_PHY_INTERFACE_MODE_SGMII)
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_I("e1000e_p->config.base_address is %p",
                                         e1000e_p->config.base_addr);
        ctrl = FE1000E_READREG32(e1000e_p->config.base_addr, E1000_STATUS);
        FE1000E_LWIP_PORT_E1000E_PRINT_I("E1000_STATUS is 0x%x", ctrl);
        link = (ctrl & STATUS_LU) >> STATUS_LU_SHFT;

        switch (link)
        {
            case 0:
                FE1000E_LWIP_PORT_E1000E_PRINT_I("link status is down");
                link_status = FE1000E_LINKDOWN;
                break;
            case 1:
                FE1000E_LWIP_PORT_E1000E_PRINT_I("link status is up");
                link_status = FE1000E_LINKUP;
                break;
            default:
                FE1000E_LWIP_PORT_E1000E_PRINT_E("link status is error 0x%x \r\n", link);
                return;
        }

        if (link_status == FE1000E_LINKUP)
        {
            if (link_status != e1000e_p->link_status)
            {
                e1000e_p->link_status = FE1000E_NEGOTIATION;
                FE1000E_LWIP_PORT_E1000E_PRINT_I("need NEGOTIATING");
            }
        }
        else
        {
            e1000e_p->link_status = FE1000E_LINKDOWN;
        }
    }
}

static FError FPcieInit(FPcieEcam *pcie_device)
{
    FError ret = FE1000E_SUCCESS;

    ret = FPcieEcamCfgInitialize(pcie_device, FPcieEcamLookupConfig(FPCIE_ECAM_INSTANCE0), NULL);
    if (ret != FT_SUCCESS)
    {
        return ret;
    }
    FE1000E_LWIP_PORT_E1000E_PRINT_D("\n");
    FE1000E_LWIP_PORT_E1000E_PRINT_D("	PCI:\n");
    FE1000E_LWIP_PORT_E1000E_PRINT_D("	B:D:F			VID:PID			parent_BDF	"
                                     "		class_code\n");
    ret = FPcieEcamEnumerateBus(pcie_device, 0);
    if (ret != FT_SUCCESS)
    {
        return ret;
    }

    return ret;
}

static FError FNetPcieMsiIrqInstall(FE1000ECtrl *net_device, FPcieEcam *pcie_device, u8 bus,
                                    u8 device, u8 function, FPcieMsiVector *msi_vector)
{
    FError ret = FE1000E_SUCCESS;
    u8 ret_msi = 0;

    u32 cpu_id;
    GetCpuId(&cpu_id);
    printf("bus %d device %d function %d\n", bus, device, function);
    ret_msi = FPcieEcamVectorsAllocate(pcie_device, bus, device, function, msi_vector);
    printf("start FPcieEcamVectorsAllocate is here , ret_msi = %d\r\n", ret_msi);
    FPcieEcamMsiEnable(pcie_device, msi_vector);
    /* 中断回调函数 */
    FE1000ERegisterEvtHandler(net_device, FE1000E_TX_COMPLETE_EVT, FE1000ETransDoneCallback);
    FE1000ERegisterEvtHandler(net_device, FE1000E_RX_COMPLETE_EVT, FE1000EReceiveDoneCallBack);
    FE1000ERegisterEvtHandler(net_device, FE1000E_LINK_STATUS_EVT, FE1000ELinkChangeCallBack);
    for (int i = 0; i < ret_msi; i++)
    {
        /* 注册中断服务函数 */
        InterruptSetPriority(msi_vector->msi[i].irq, 0);
        InterruptSetTargetCpus(msi_vector->msi[i].irq, cpu_id);
        InterruptInstall(msi_vector->msi[i].irq, (IrqHandler)FE1000EPCIeMsiIrqHandler,
                         net_device, "pciemsi");
        InterruptUmask(msi_vector->msi[i].irq);
    }

    return ret;
}

FError FE1000ELwipPortInit(FE1000ELwipPort *instance_p)
{
    s32 host;
    u32 pci_class = 0;
    const u32 class_code = PCI_CLASS_STORAGE_NET_AHCI;
    uintptr bar_addr = 0;
    u8 bus = 0;
    u8 device = 0;
    u8 function = 0;
    u32 config_data;
    u32 cmdstat;

    FE1000EConfig mac_config;
    FE1000ECtrl *e1000e_p;
    FError status;
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->e1000e_port_config.instance_id < FE1000E_NUM);

    e1000e_p = &instance_p->instance;
    FE1000E_LWIP_PORT_E1000E_PRINT_I("instance_id IS %d \r\n",
                                     instance_p->e1000e_port_config.instance_id);


    status = FE1000ELwipPortConfigConvert(instance_p, &mac_config);
    if (status != FT_SUCCESS)
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_E("In %s: FE1000ELwipPortControl Convert to "
                                         "FE1000EConfig Failed....\r\n",
                                         __func__);
    }

    /* pcie init */
    status = FPcieInit(&pcie_device);
    if (status != FE1000E_SUCCESS)
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_E("FPcieInit failed.");
        return FE1000E_ERR_FAILED;
    }

    /* find xhci host from pcie instance */
    for (host = 0; host < pcie_device.scans_bdf_count; host++)
    {
        bus = pcie_device.scans_bdf[host].bus;
        device = pcie_device.scans_bdf[host].device;
        function = pcie_device.scans_bdf[host].function;

        FPcieEcamReadConfigSpace(&pcie_device, bus, device, function,
                                 FPCIE_CCR_REV_CLASSID_REGS, &config_data);
        FE1000E_LWIP_PORT_E1000E_PRINT_D("FPCIE_CCR_REV_CLASSID_REGS = %x\n", config_data);
        pci_class = config_data >> 8;

        if (pci_class == class_code)
        {
            FPcieEcamReadConfigSpace(&pcie_device, bus, device, function,
                                     FPCIE_CCR_ID_REG, &config_data);

            FE1000E_LWIP_PORT_E1000E_PRINT_D("AHCI-PCI HOST found !!!, b.d.f = "
                                             "%x.%x.%x\n",
                                             bus, device, function);

            FPcieEcamReadConfigSpace(&pcie_device, bus, device, function,
                                     FPCIE_CCR_BAR_ADDR0_REGS, (u32 *)&bar_addr);
            FE1000E_LWIP_PORT_E1000E_PRINT_D("FNetPcieIntrInstall BarAddress %p", bar_addr);
            FE1000E_CONFIG_TBL[FE1000E0_ID].base_addr = bar_addr;
            e1000e_p->config.base_addr = bar_addr;
            mac_config.base_addr = bar_addr;

            if (0x0 == bar_addr)
            {
                FE1000E_LWIP_PORT_E1000E_PRINT_E("Bar address: 0x%lx", bar_addr);
                return -1;
            }

            break;
        }
    }
    if (pci_class != class_code)
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_E("class_code: 0x%06x is not pcie net card!", pci_class);
        return -1;
    }

    status = FE1000ECfgInitialize(e1000e_p, (FE1000EConfig *)&mac_config);
    if (status != FT_SUCCESS)
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_E("In %s:E1000E Configuration Failed....\r\n", __func__);
    }

    /* 设置cmd reg */
    FPcieEcamReadConfigSpace(&pcie_device, bus, device, function, FPCIE_CCR_CMD_STATUS_REGS, &cmdstat);
    cmdstat |= (FPCIE_CCR_CMD_MEMORY_ACCESS_ENABLED | FPCIE_CCR_CMD_BUS_MASTER_ENABLED);
    FPcieEcamWriteConfigSpace(&pcie_device, bus, device, function, FPCIE_CCR_CMD_STATUS_REGS, cmdstat);

    /* Initialize Rx Description list : ring Mode */
    status = FE1000ESetupRxDescRing(e1000e_p);
    if (FT_SUCCESS != status)
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_E("e1000e setup rx return err code %d\r\n", status);
        FASSERT(FT_SUCCESS == status);
    }

    /* Initialize Tx Description list : ring Mode */
    status = FE1000ESetupTxDescRing(e1000e_p);
    if (FT_SUCCESS != status)
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_E("e1000e setup tx return err code %d\r\n", status);
        FASSERT(FT_SUCCESS == status);
    }

    FNetPcieMsiIrqInstall(e1000e_p, &pcie_device, bus, device, function,
                          (FPcieMsiVector *)&msi_vector[FE1000E0_ID]);
    FE1000EIrqEnable(e1000e_p, IMS_TXDW | IMS_LSC | IMS_RXT0 | IMS_RXQ0 | IMS_TXQ0);

    /* 打印寄存器的值 */
    FE1000EDebugPrint(e1000e_p);

    return FT_SUCCESS;
}

void *FE1000ELwipPortRx(FE1000ELwipPort *instance_p)
{
    struct pbuf *p = NULL;
    u16 length = 0;
    FE1000ECtrl *e1000e_p = &instance_p->instance;
    u32 rx_idx = e1000e_p->rx_ring.desc_idx;

    /* get received frame */
    if (FE1000ERecvFrame(e1000e_p) != FT_SUCCESS)
    {
        return NULL;
    }
    else
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_D("FE1000ERecvFrame is ok!");
    }

    length = e1000e_p->rx[rx_idx].len;

#if ETH_PAD_SIZE
    length += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

    p = (struct pbuf *)e1000e_p->rxb[rx_idx];

#ifdef RAW_DATA_PRINT
    dump_hex(Buffer, (u32)length);
#endif
    if (p != NULL)
    {
#if ETH_PAD_SIZE
        pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

        /* zero copy */
        FLwipDCacheRangeInvalidate((uintptr)p->payload, length);

        /* entry queue */
        if (FE1000EPqEnqueue(&instance_p->recv_q, (void *)p) < 0)
        {
#if LINK_STATS
            lwip_stats.link.memerr++;
            lwip_stats.link.drop++;
#endif
            pbuf_free(p);
        }

        e1000e_p->rxb[rx_idx] = (uintptr)NULL;
        p = pbuf_alloc(PBUF_RAW, FE1000E_MAX_FRAME_SIZE, PBUF_POOL);
        FCacheDCacheInvalidateRange((uintptr)p->payload, FE1000E_MAX_FRAME_SIZE);
        e1000e_p->rx[rx_idx].addr = (uintptr)p->payload;
        e1000e_p->rxb[rx_idx] = (uintptr)p;

#if ETH_PAD_SIZE
        pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
    }
    else
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_E("error: pbuf alloc failed the alloc length is "
                                         "%d",
                                         length);
    }

    return p;
}

void FE1000ERecvHandler(void *arg)
{
    FE1000ELwipPort *instance_p = (FE1000ELwipPort *)arg;
    struct pbuf *p = NULL;
    u16 length = 0;
    u8 *buffer;
    FE1000ECtrl *e1000e_p = &instance_p->instance;
    u32 rx_idx = e1000e_p->rx_ring.desc_idx;

    /* get received frame */
    if (FE1000ERecvFrame(e1000e_p) != FT_SUCCESS)
    {
        return;
    }
    else
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_D("FE1000ERecvFrame is ok!");
    }

    length = e1000e_p->rx[rx_idx].len;
    buffer = (u8 *)(intptr)(e1000e_p->rx[rx_idx].addr);

#if ETH_PAD_SIZE
    length += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

    if (length > 0)
    {
        p = pbuf_alloc(PBUF_RAW, length, PBUF_RAM);
    }

#ifdef RAW_DATA_PRINT
    dump_hex(Buffer, (u32)length);
#endif
    if (p != NULL)
    {
#if ETH_PAD_SIZE
        pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

        memcpy((u8 *)((u8 *)p->payload), (u8 *)buffer, length);

#if ETH_PAD_SIZE
        pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
    }
    else
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_E("error: pbuf alloc failed the alloc length is "
                                         "%d",
                                         length);
    }

    return;
}

void *FE1000ELwipPortQueueRx(FE1000ELwipPort *instance_p)
{
    FASSERT(instance_p != NULL);
    struct pbuf *p;

    /* see if there is data to process */
    if (FE1000EPqQlength(&instance_p->recv_q) == 0)
    {
        return NULL;
    }
    /* return one packet from receive q */
    p = (struct pbuf *)FE1000EPqDequeue(&instance_p->recv_q);

    return p;
}

FError FE1000ELwipPortTx(FE1000ELwipPort *instance_p, void *tx_buf)
{
    FASSERT(instance_p != NULL);
    FASSERT(tx_buf != NULL);
    err_t errval = ERR_OK;
    struct pbuf *q;
    struct pbuf *p = tx_buf;
    FError ret;
    u32 bytes_left_to_copy = 0;
    FE1000ECtrl *e1000e_p = &instance_p->instance;

    sys_prot_t lev;
    lev = sys_arch_protect();

    for (q = p; q != NULL; q = q->next)
    {
        /* Get bytes in current lwIP buffer */
        bytes_left_to_copy = q->len;
        FE1000E_LWIP_PORT_E1000E_PRINT_D("bytes_left_to_copy = %d", bytes_left_to_copy);

        /* Copy the remaining bytes */
        // FCacheDCacheFlushRange((uintptr)q->payload, (uintptr)q->len);
        e1000e_p->txb[e1000e_p->tx_ring.desc_idx] = (uintptr)q->payload;
    }

    ret = FE1000ESendFrame(e1000e_p, bytes_left_to_copy);

    if (ret != FE1000E_SUCCESS)
    {
        errval = ERR_USE;
        FE1000E_LWIP_PORT_E1000E_PRINT_I("error errval = ERR_USE; FE1000ESendFrame");
        goto error;
    }
    else
    {
        FE1000E_LWIP_PORT_E1000E_PRINT_I("FE1000ESendFrame is ok!!!");
    }

    sys_arch_unprotect(lev);
error:
    return errval;
}

FE1000ELwipPort *FE1000ELwipPortGetInstancePointer(u32 FE1000ELwipPortInstanceID)
{
    FASSERT(FE1000ELwipPortInstanceID < FE1000E_NUM);

    FE1000ELwipPort *instance_p;
    instance_p = &fe1000e_lwip_port_instance[FE1000ELwipPortInstanceID];
    return instance_p;
}

static void FreeOnlyRxPbufs(FE1000ELwipPort *instance_p)
{
    u32 index;
    struct pbuf *p;

    for (index = 0; index < (FE1000E_RX_DESCRIPTORS); index++)
    {
        if (instance_p->instance.rxb[index] != 0)
        {
            p = (struct pbuf *)instance_p->instance.rxb[index];
            pbuf_free(p);
            instance_p->instance.rxb[index] = (uintptr)NULL;
        }
    }
}

void FE1000ELwipPortStop(FE1000ELwipPort *instance_p)
{
    u32 rx_queue_len = 0;
    struct pbuf *p;
    FASSERT(instance_p != NULL);

    /* close mac controler */
    FE1000EStop(&instance_p->instance);

    /*  */
    rx_queue_len = FE1000EPqQlength(&instance_p->recv_q);
    while (rx_queue_len)
    {
        /* return one packet from receive q */
        p = (struct pbuf *)FE1000EPqDequeue(&instance_p->recv_q);
        pbuf_free(p);
        FE1000E_LWIP_PORT_E1000E_PRINT_W("delete queue %p", p);
        rx_queue_len--;
    }

    /* free all pbuf */
    FreeOnlyRxPbufs(instance_p);
}

void FE1000ELwipPortStart(FE1000ELwipPort *instance_p)
{
    FASSERT(instance_p != NULL);

    /* start mac */
    FE1000EStart(&instance_p->instance);
}
