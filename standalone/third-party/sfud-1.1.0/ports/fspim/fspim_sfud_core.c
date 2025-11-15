/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fspim_sfud_core.c
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-03-13 09:01:56
 * Description:  This file is for providing sfud func based on spi.
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu    2021/12/10   first commit
 * 1.0.1 wangxiaodong 2022/12/1    parameter naming change
 * 1.0.2 huangjin     2023/12/26   cancel the use of the FGPIO_VERSION_1 and FGPIO_VERSION_2 macros
 * 2.0   liyilun      2024/2/20    add ddma transfer way
 * 2.0.1 liqiaozhong  2024/3/11    optimize DDMA-SFUD tranfer api
 */

#include "fspim_sfud_core.h"
#include "fparameters.h"
#include "sfud_def.h"
#if defined(CONFIG_SFUD_TRANS_MODE_INTERRUPT) || defined(CONFIG_SFUD_TRANS_MODE_DDMA)
#include "finterrupt.h"
#include "fcpu_info.h"
#endif
#include "fspim_hw.h"
#include "sdkconfig.h"
#include "fio_mux.h"

#ifndef SDK_CONFIG_H__
#warning "Please include sdkconfig.h"
#endif
#if defined(CONFIG_D2000_TEST_BOARD) || defined(CONFIG_FT2004_DSK_BOARD)
#include "fgpio.h"
#endif


#ifdef CONFIG_SFUD_TRANS_MODE_DDMA
#include "fddma.h"
#include "fddma_hw.h"
#endif

/* ../port/sfup_port.c */
extern void sfud_log_debug(const char *file, const long line, const char *format, ...);
extern void sfud_log_info(const char *format, ...);

#define SFUD_SPI_CS_ON  TRUE
#define SFUD_SPI_CS_OFF FALSE
#define SFUD_TX_BUF_LEN 256


#if defined CONFIG_E2000Q_DEMO_BOARD || defined CONFIG_E2000D_DEMO_BOARD
#define SFUD_CONTROLLER_ID SFUD_FSPIM2_INDEX /* E2000 Default Use SPI2 id  */

#define TX_SLAVE_ID        FDDMA0_SPIM2_TX_SLAVE_ID
#define RX_SLAVE_ID        FDDMA0_SPIM2_RX_SLAVE_ID

#else
#define SFUD_CONTROLLER_ID SFUD_FSPIM0_INDEX /* Default Use SPI0 id */

#define TX_SLAVE_ID        FDDMA0_SPIM0_TX_SLAVE_ID
#define RX_SLAVE_ID        FDDMA0_SPIM0_RX_SLAVE_ID

#endif

#ifdef CONFIG_SFUD_TRANS_MODE_DDMA
#define DDMA_CONTROLLER_ID FDDMA0_ID
#define FDDMA_TX_MAX_LEN \
    300 /* 目前SFUD实现方案中单次最大需要spi写入到flash的数据量为（256 + 5）字节 */
#define FDDMA_RX_MAX_LEN \
    512 /* 目前SFUD实现方案中单次最大需要spi从flash读取的数据量上限为flash自身容量，这里为了减少控制器负载，人为定为512字节 */
#define TX_CHAN_ID 0
#define RX_CHAN_ID 1

static FDdma ddma_instance;
static FDdmaConfig ddma_config;
static FDdmaChanConfig tx_chan_config;
static FDdmaChanConfig rx_chan_config;

static volatile boolean rx_dma_done = FALSE;
static volatile boolean tx_dma_done = FALSE;
#endif

typedef struct
{
    FSpim spim;
#if defined(CONFIG_D2000_TEST_BOARD) || defined(CONFIG_FT2004_DSK_BOARD)
    FGpio gpio;
#endif
} FSpimCore;

static u32 device_select_mask; /* 每一位用于指示那个设备被选择，如0x3 ,则 fspim0 ，fspim1 被选择 */
static FSpimCore fspim[FSPI_NUM] = {0};

#if defined(CONFIG_D2000_TEST_BOARD) || defined(CONFIG_FT2004_DSK_BOARD)
/* 使用GPIO引脚控制片选信号 */
static u32 cs_pin_id = FGPIO_ID(FGPIO_CTRL_1, FGPIO_PORT_A, FGPIO_PIN_5);

static int SfudSpiPortSetupCs(FSpimCore *core_p)
{
    FGpioConfig input_cfg = *FGpioLookupConfig(cs_pin_id);
    FGpio *gpio_p = &core_p->gpio;

    (void)FGpioCfgInitialize(gpio_p, &input_cfg);
    FGpioSetDirection(gpio_p, FGPIO_DIR_OUTPUT);

    return SFUD_SUCCESS;
}

static void SfudSpiPortCsOnOff(FSpimCore *core_p, boolean on)
{
    FGpio *gpio = &core_p->gpio;
    if (on)
    {
        FGpioSetOutputValue(gpio, FGPIO_PIN_LOW);
    }
    else
    {
        FGpioSetOutputValue(gpio, FGPIO_PIN_HIGH);
    }
}

#else
/* 使用FSpimSetChipSelection控制片选信号 */
static int SfudSpiPortSetupCs(FSpimCore *core_p)
{
    return SFUD_SUCCESS;
}

static void SfudSpiPortCsOnOff(FSpimCore *core_p, boolean on)
{
    FSpim *spim_p = &core_p->spim;
    FSpimSetChipSelection(spim_p, on);
}
#endif

#ifdef CONFIG_SFUD_TRANS_MODE_POLL_FIFO
static sfud_err SfudSpiPortPollFifoTransfer(FSpim *spim_p, const uint8_t *write_buf,
                                            size_t write_size, uint8_t *read_buf, size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;

    SFUD_DEBUG("spim@%p beg+++++++++++++++++++++++++++++++++++++++++++++++++++", spim_p);
    SFUD_DEBUG("++++  Write %d Bytes @%p: 0x%x, 0x%x, 0x%x", write_size, write_buf,
               ((NULL != write_buf) && (write_size > 0)) ? write_buf[0] : 0xff,
               ((NULL != write_buf) && (write_size > 1)) ? write_buf[1] : 0xff,
               ((NULL != write_buf) && (write_size > 2)) ? write_buf[2] : 0xff);
    SFUD_DEBUG("++++  Read %d Bytes @%p: 0x%x, 0x%x, 0x%x", read_size, read_buf,
               ((NULL != read_buf) && (read_size > 0)) ? read_buf[0] : 0xff,
               ((NULL != read_buf) && (read_size > 1)) ? read_buf[1] : 0xff,
               ((NULL != read_buf) && (read_size > 2)) ? read_buf[2] : 0xff);
    SFUD_DEBUG("spim@%p end+++++++++++++++++++++++++++++++++++++++++++++++++++", spim_p);
    if (write_size && read_size)
    {
        result = FSpimTransferPollFifo(spim_p, write_buf, NULL, write_size);
        if (SFUD_SUCCESS != result)
        {
            goto err_ret;
        }

        result = FSpimTransferPollFifo(spim_p, NULL, read_buf, read_size);
    }
    else if (write_size)
    {
        result = FSpimTransferPollFifo(spim_p, write_buf, NULL, write_size);
    }
    else if (read_size)
    {
        result = FSpimTransferPollFifo(spim_p, NULL, read_buf, read_size);
    }

err_ret:
    return result;
}

#endif

#ifdef CONFIG_SFUD_TRANS_MODE_INTERRUPT
boolean sfud_spi_rx_done = FALSE;
static void SfudSpiRxDoneHandler(void *instance_p, void *param)
{
    FASSERT(instance_p && param);
    FSpim *spim_p = (FSpim *)instance_p;
    boolean *done_flag = (boolean *)param;

    *done_flag = TRUE;
    return;
}

static FError SfudSpiSetupInterrupt(FSpim *instance_p)
{
    FASSERT(instance_p);
    FSpimConfig *config_p = &instance_p->config;
    uintptr base_addr = config_p->base_addr;
    u32 cpu_id;

    if (FT_COMPONENT_IS_READY != instance_p->is_ready)
    {
        SFUD_DEBUG("device is already initialized!!!");
        return FSPIM_ERR_NOT_READY;
    }

    GetCpuId(&cpu_id);
    SFUD_DEBUG("cpu_id is cpu_id %d", cpu_id);
    InterruptSetTargetCpus(config_p->irq_num, cpu_id);

    InterruptSetPriority(config_p->irq_num, config_p->irq_prority);

    /* register intr callback */
    InterruptInstall(config_p->irq_num, FSpimInterruptHandler, instance_p, NULL);

    /* enable tx fifo overflow / rx overflow / rx full */
    FSpimMaskIrq(base_addr, FSPIM_IMR_ALL_BITS);

    /* enable irq */
    InterruptUmask(config_p->irq_num);

    return FSPIM_SUCCESS;
}

static int SfudSpiPortWaitRxDone(int timeout)
{
    while (TRUE != sfud_spi_rx_done)
    {
        FDriverMdelay(10);
        if (0 >= --timeout)
        {
            break;
        }
    }

    if (0 >= timeout)
    {
        SFUD_DEBUG("wait rx timeout \r\n");
        return -1;
    }

    return 0;
}

static sfud_err SfudSpiPortInterruptRx(FSpim *spim_p, uint8_t *read_buf, size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;

    sfud_spi_rx_done = FALSE;
    if (FSPIM_SUCCESS != FSpimTransferByInterrupt(spim_p, NULL, read_buf, read_size))
    {
        SFUD_ERROR("interrupt read by fifo failed !!!");
        return SFUD_ERR_READ;
    }

    if (0 != SfudSpiPortWaitRxDone(50000000))
    {
        SFUD_ERROR("wait timeout 500 seconds used up !!!");
        return SFUD_ERR_TIMEOUT;
    }

    return result;
}

static sfud_err SfudSpiPortInterruptTx(FSpim *spim_p, const uint8_t *write_buf, size_t write_size)
{
    sfud_err result = SFUD_SUCCESS;

    sfud_spi_rx_done = FALSE;
    if (FSPIM_SUCCESS != FSpimTransferByInterrupt(spim_p, write_buf, NULL, write_size))
    {
        SFUD_ERROR("interrupt write by fifo failed !!!");
        return SFUD_ERR_WRITE;
    }

    /* timeout 10 us * 50000000 = 500s */
    if (0 != SfudSpiPortWaitRxDone(50000000))
    {
        SFUD_ERROR("wait timeout 500 seconds used up !!!");
        return SFUD_ERR_TIMEOUT;
    }

    return result;
}

static sfud_err SfudSpiPortInterruptTransfer(FSpim *spim_p, const uint8_t *write_buf,
                                             size_t write_size, uint8_t *read_buf, size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    SFUD_DEBUG("spim@%p beg+++++++++++++++++++++++++++++++++++++++++++++++++++", spim_p);
    SFUD_DEBUG("++++  Write %d Bytes @%p: 0x%x, 0x%x, 0x%x", write_size, write_buf,
               ((NULL != write_buf) && (write_size > 0)) ? write_buf[0] : 0xff,
               ((NULL != write_buf) && (write_size > 1)) ? write_buf[1] : 0xff,
               ((NULL != write_buf) && (write_size > 2)) ? write_buf[2] : 0xff);
    SFUD_DEBUG("++++  Read %d Bytes @%p: 0x%x, 0x%x, 0x%x", read_size, read_buf,
               ((NULL != read_buf) && (read_size > 0)) ? read_buf[0] : 0xff,
               ((NULL != read_buf) && (read_size > 1)) ? read_buf[1] : 0xff,
               ((NULL != read_buf) && (read_size > 2)) ? read_buf[2] : 0xff);
    SFUD_DEBUG("spim@%p end+++++++++++++++++++++++++++++++++++++++++++++++++++", spim_p);

    if (write_size && read_size)
    {
        result = SfudSpiPortInterruptTx(spim_p, write_buf, write_size);
        if (SFUD_SUCCESS != result)
        {
            goto err_ret;
        }

        result = SfudSpiPortInterruptRx(spim_p, read_buf, read_size);
    }
    else if (write_size)
    {
        result = SfudSpiPortInterruptTx(spim_p, write_buf, write_size);
    }
    else if (read_size)
    {
        result = SfudSpiPortInterruptRx(spim_p, read_buf, read_size);
    }

err_ret:
    return result;
}
#endif

#ifdef CONFIG_SFUD_TRANS_MODE_DDMA
static void FU8buff2U32buff(u32 *buff1, const u8 *buff2, u32 size)
{
    FASSERT(buff1 && buff2);

    for (size_t i = 0; i < size; i++)
    {
        buff1[i] = buff2[i];
    }

    return;
}

static void FU32buff2U8buff(u8 *buff1, u32 *buff2, u32 size)
{
    FASSERT(buff1 && buff2);

    for (size_t i = 0; i < size; i++)
    {
        buff1[i] = buff2[i];
    }

    return;
}

static void FDdmaSpimTxDMADone()
{
    tx_dma_done = TRUE;
    return;
}

static void FDdmaSpimRxDMADone()
{
    rx_dma_done = TRUE;
    return;
}

static FError FDdmaCtrlInit()
{
    /* controller init */
    FError ret = FT_SUCCESS;
    ddma_config = *FDdmaLookupConfig(DDMA_CONTROLLER_ID);
    ret = FDdmaCfgInitialize(&ddma_instance, &ddma_config);
    if (FDDMA_SUCCESS != ret)
    {
        SFUD_ERROR("Ddma-%d init error: configuration failed.", DDMA_CONTROLLER_ID);
        return ret;
    }

    /* irq set */
    FDdmaRegisterChanEvtHandler(&ddma_instance, TX_CHAN_ID, FDDMA_CHAN_EVT_REQ_DONE,
                                FDdmaSpimTxDMADone, NULL);
    FDdmaRegisterChanEvtHandler(&ddma_instance, RX_CHAN_ID, FDDMA_CHAN_EVT_REQ_DONE,
                                FDdmaSpimRxDMADone, NULL);

    u32 cpu_id = 0;
    GetCpuId(&cpu_id);
    InterruptSetTargetCpus(ddma_instance.config.irq_num, cpu_id);
    InterruptSetPriority(ddma_instance.config.irq_num, ddma_instance.config.irq_prority);
    InterruptInstall(ddma_instance.config.irq_num, FDdmaIrqHandler, &ddma_instance, NULL); /* register intr callback */

    return ret;
}


static FError FDdmaChanReset(FSpim *spim_p, u32 *tx_buff, u32 tx_size, u32 *rx_buff, u32 rx_size)
{
    /* irq flag reset */
    FError ret = FT_SUCCESS;
    tx_dma_done = FALSE;
    rx_dma_done = FALSE;

    /* disable DDMA irq in gic */
    InterruptMask(ddma_instance.config.irq_num);

    FDdmaStop(&ddma_instance);

    /* tx and rx channel deconfigure */
    ret = FDdmaChanDeactive(&ddma_instance, TX_CHAN_ID);
    ret = FDdmaChanDeconfigure(&ddma_instance, TX_CHAN_ID);
    if (ret)
    {
        SFUD_ERROR("TX channel FDdmaChanDeconfigure failed: 0x%x", ret);
        return FDDMA_ERR_IS_USED;
    }

    ret = FDdmaChanDeactive(&ddma_instance, RX_CHAN_ID);
    ret = FDdmaChanDeconfigure(&ddma_instance, RX_CHAN_ID);
    if (ret)
    {
        SFUD_ERROR("RX channel FDdmaChanDeconfigure failed: 0x%x", ret);
        return FDDMA_ERR_IS_USED;
    }

    /* tx channel reset */
    tx_chan_config.slave_id = TX_SLAVE_ID;
    tx_chan_config.ddr_addr = (uintptr)tx_buff;
    tx_chan_config.dev_addr = spim_p->config.base_addr + FSPIM_DR_OFFSET;
    tx_chan_config.req_mode = FDDMA_CHAN_REQ_TX;
    tx_chan_config.timeout = 0xffff;
    tx_chan_config.trans_len = tx_size * 4;

    ret = FDdmaChanConfigure(&ddma_instance, TX_CHAN_ID, &tx_chan_config);
    if (FDDMA_SUCCESS != ret)
    {
        SFUD_ERROR("Ddma init error: tx FDdmaChanConfigure failed: 0x%x.", ret);
        return ret;
    }

    /* rx channel reset */
    rx_chan_config.slave_id = RX_SLAVE_ID;
    rx_chan_config.ddr_addr = (uintptr)rx_buff;
    rx_chan_config.dev_addr = spim_p->config.base_addr + FSPIM_DR_OFFSET;
    rx_chan_config.req_mode = FDDMA_CHAN_REQ_RX;
    rx_chan_config.timeout = 0xffff;
    rx_chan_config.trans_len = rx_size * 4;

    ret = FDdmaChanConfigure(&ddma_instance, RX_CHAN_ID, &rx_chan_config);
    if (FDDMA_SUCCESS != ret)
    {
        SFUD_ERROR("Ddma init error: rx FDdmaChanConfigure failed: 0x%x.", ret);
        return ret;
    }

    InterruptUmask(ddma_instance.config.irq_num); /* enable DDMA irq */

    if (FDdmaIsChanRunning(ddma_instance.config.base_addr, TX_CHAN_ID) ||
        FDdmaIsChanRunning(ddma_instance.config.base_addr, RX_CHAN_ID))
    {
        SFUD_ERROR("RX or TX channel is already running!");
        return FDDMA_ERR_IS_USED;
    }

    /* enable tx and rx channel and ready for use */
    FDdmaChanActive(&ddma_instance, TX_CHAN_ID);
    FDdmaChanActive(&ddma_instance, RX_CHAN_ID);

    return ret;
}

static u32 temp_tx[FDDMA_TX_MAX_LEN] __attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT)));
static u32 temp_rx[FDDMA_RX_MAX_LEN] __attribute__((aligned(FDDMA_DDR_ADDR_ALIGMENT)));

static FError SfudSpiPortDdmaTransfer(FSpim *spim_p, const uint8_t *write_buf,
                                      size_t write_size, uint8_t *read_buf, size_t read_size)
{

    if (write_size > FDDMA_TX_MAX_LEN || read_size > FDDMA_RX_MAX_LEN)
    {
        SFUD_ERROR("TX or RX single tranfer size error.");
        return FDDMA_ERR_INVALID_INPUT;
    }

    SFUD_DEBUG("spim@%p beg+++++++++++++++++++++++++++++++++++++++++++++++++++", spim_p);
    SFUD_DEBUG("++++  Write %d Bytes @%p: 0x%x, 0x%x, 0x%x", write_size, write_buf,
               ((NULL != write_buf) && (write_size > 0)) ? write_buf[0] : 0xff,
               ((NULL != write_buf) && (write_size > 1)) ? write_buf[1] : 0xff,
               ((NULL != write_buf) && (write_size > 2)) ? write_buf[2] : 0xff);
    SFUD_DEBUG("++++  Read %d Bytes @%p: 0x%x, 0x%x, 0x%x", read_size, read_buf,
               ((NULL != read_buf) && (read_size > 0)) ? read_buf[0] : 0xff,
               ((NULL != read_buf) && (read_size > 1)) ? read_buf[1] : 0xff,
               ((NULL != read_buf) && (read_size > 2)) ? read_buf[2] : 0xff);
    SFUD_DEBUG("spim@%p end+++++++++++++++++++++++++++++++++++++++++++++++++++", spim_p);

    sfud_err result = SFUD_SUCCESS;
    int timeout = 1000;
    size_t total_size = write_size + read_size;
    memset(temp_tx, 0x0, sizeof(temp_tx));
    memset(temp_rx, 0x0, sizeof(temp_rx));

    if (write_size)
    {
        FU8buff2U32buff(temp_tx, write_buf, write_size);
    }

    FDdmaChanReset(spim_p, temp_tx, total_size, temp_rx, total_size);
    result = FSpimTransferDMA(spim_p);
    if (result != FT_SUCCESS)
    {
        SFUD_ERROR("Dma transfer failed.");
    }
    FDdmaStart(&ddma_instance);
    /* wait for DDMA tx and rx done */
    while (!tx_dma_done || !rx_dma_done)
    {
        if (--timeout <= 0)
        {
            SFUD_DEBUG("DDMA wait irq timeout, tx_dma_done: %s, rx_dma_done: %s",
                       (tx_dma_done ? "TRUE" : "FALSE"), (rx_dma_done ? "TRUE" : "FALSE"));
            break;
        }
        FDriverMdelay(1);
    }

    if (read_size)
    {
        FU32buff2U8buff(read_buf, (temp_rx + write_size), read_size);
    }

    return result;
}

#endif

static sfud_err FspiWriteRead(const sfud_spi *spi, const uint8_t *write_buf,
                              size_t write_size, uint8_t *read_buf, size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    FSpimCore *spi_p = (FSpimCore *)spi->user_data;
    FSpim *spim_p = &spi_p->spim;
    if (write_size)
    {
        SFUD_ASSERT(write_buf);
    }
    if (read_size)
    {
        SFUD_ASSERT(read_buf);
    }

    /**
     * add your spi write and read code
     */
    SfudSpiPortCsOnOff(spi_p, SFUD_SPI_CS_ON);

#ifdef CONFIG_SFUD_TRANS_MODE_POLL_FIFO
    result = SfudSpiPortPollFifoTransfer(spim_p, write_buf, write_size, read_buf, read_size);
#endif

#ifdef CONFIG_SFUD_TRANS_MODE_INTERRUPT
    result = SfudSpiPortInterruptTransfer(spim_p, write_buf, write_size, read_buf, read_size);
#endif

#ifdef CONFIG_SFUD_TRANS_MODE_DDMA
    result = SfudSpiPortDdmaTransfer(spim_p, write_buf, write_size, read_buf, read_size);
#endif

    SfudSpiPortCsOnOff(spi_p, SFUD_SPI_CS_OFF);


    return result;
}

sfud_err FSpimProbe(sfud_flash *flash)
{

    sfud_spi *spi_p = &flash->spi;
    sfud_err result = SFUD_SUCCESS;
    FSpim *spim_p;
    u32 spim_id = FSPI0_ID;

    if (!memcmp(FSPIM0_SFUD_NAME, spi_p->name, strlen(FSPIM0_SFUD_NAME)))
    {
        spim_id = FSPI0_ID;
    }
    else if (!memcmp(FSPIM1_SFUD_NAME, spi_p->name, strlen(FSPIM1_SFUD_NAME)))
    {
        spim_id = FSPI1_ID;
    }

#if defined(CONFIG_TARGET_PE220X)
    else if (!memcmp(FSPIM2_SFUD_NAME, spi_p->name, strlen(FSPIM2_SFUD_NAME)))
    {
        spim_id = FSPI2_ID;
    }
    else if (!memcmp(FSPIM3_SFUD_NAME, spi_p->name, strlen(FSPIM3_SFUD_NAME)))
    {
        spim_id = FSPI3_ID;
    }
#endif

    else
    {
        return SFUD_ERR_NOT_FOUND;
    }

    spim_p = &fspim[spim_id].spim;
    FSpimConfig input_cfg = *FSpimLookupConfig(spim_id);

    FIOMuxInit();
    FIOPadSetSpimMux(spim_id);

    memset(&fspim[spim_id], 0, sizeof(fspim[spim_id]));
    if (0 != SfudSpiPortSetupCs(&fspim[spim_id]))
    {
        SFUD_DEBUG("init gpio cs failed");
        result = SFUD_ERR_INIT_FAILED;
        return result;
    }

    input_cfg.slave_dev_id = FSPIM_SLAVE_DEV_0;
    input_cfg.cpha = FSPIM_CPHA_1_EDGE;
    input_cfg.cpol = FSPIM_CPOL_LOW;
    input_cfg.n_bytes = FSPIM_1_BYTE; /* sfud only support 1 bytes read/write */
    input_cfg.sclk_hz = FSPI_DEFAULT_SCLK;


#ifdef CONFIG_SFUD_TRANS_MODE_DDMA
    input_cfg.en_dma = TRUE;
    FDdmaCtrlInit();
#endif

    if (FSPIM_SUCCESS != FSpimCfgInitialize(spim_p, &input_cfg))
    {
        SFUD_DEBUG("init spi failed");
        result = SFUD_ERR_INIT_FAILED;
        return result;
    }

#ifdef CONFIG_SFUD_TRANS_MODE_INTERRUPT
    if (FSPIM_SUCCESS != SfudSpiSetupInterrupt(spim_p))
    {
        SFUD_DEBUG("init spi interrupt failed");
        result = SFUD_ERR_INIT_FAILED;
        return result;
    }

    FSpimRegisterInterruptHandler(spim_p, FSPIM_INTR_EVT_RX_DONE, SfudSpiRxDoneHandler, &sfud_spi_rx_done);
#endif

    flash->spi.wr = FspiWriteRead;
    flash->spi.user_data = &fspim[spim_id];

    device_select_mask |= 0x1;

    return result;
}
