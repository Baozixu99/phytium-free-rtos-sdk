/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "wifi_bt_config.h"
#include "fsl_wifi_config.h"

#include "fkernel.h"
#include "fcache.h"
#include "fio.h"

#include "fio_mux.h"
#include "fgpio.h"

#include "fsdif_timing.h"
#include "fsl_sdmmc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef WIFI_BT_USE_M2_INTERFACE
#warning "M.2 interface is not supported on this board"
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static sdmmchost_config_t s_inst_config;

static FGpio gpio;
static FGpioConfig gpio_config;
static u32 gpio_power_index = FGPIO_ID(FGPIO_CTRL_4, FGPIO_PIN_11);

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_WIFI_BT_Enable(bool enable)
{
    if (enable)
    {
        /* Enable module */
        /* Enable power supply for SD */
    }
    else
    {
        /* Disable module */
        /* Disable power supply for SD */
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void SdioMW8801PowerUp(void)
{
    gpio_config = *FGpioLookupConfig(gpio_power_index);
    (void)FGpioCfgInitialize(&gpio, &gpio_config);
    
    FIOMuxInit();

    FIOPadSetGpioMux(gpio_config.ctrl, (u32)gpio_config.pin);

    (void)FGpioSetDirection(&gpio, FGPIO_DIR_OUTPUT);

    /* transitions from low to high for PDn pin to reset sdio card */
    FGpioSetOutputValue(&gpio, FGPIO_PIN_LOW);
    SDMMC_OSADelay(50);
    FGpioSetOutputValue(&gpio, FGPIO_PIN_HIGH);
    SDMMC_OSADelay(50);

    FGpioDeInitialize(&gpio);

    FIOMuxDeInit();

    printf("Assert PDn to power-up MW8801 \r\n");  
}

void BOARD_WIFI_BT_Config(void *host, sdio_int_t cardInt)
{
    sdmmc_sdio_t *sdio_host = (sdmmc_sdio_t *)host;
    FSdifTimingInit();
    SdioMW8801PowerUp();
    SDMMC_OSAInit();
    memset(&s_inst_config, 0, sizeof(s_inst_config));
    memset(sdio_host, 0, sizeof(*sdio_host));

    s_inst_config.hostId = FSDIF1_ID;
    s_inst_config.hostType = kSDMMCHOST_TYPE_FSDIF;
    s_inst_config.cardType = kSDMMCHOST_CARD_TYPE_SDIO;
    s_inst_config.enableDMA = TRUE;
    s_inst_config.enableIrq = TRUE;
    s_inst_config.endianMode = kSDMMCHOST_EndianModeLittle;
    s_inst_config.maxTransSize = 8 * 256U;
    s_inst_config.defBlockSize = 256U;
    s_inst_config.cardClock = SD_CLOCK_25MHZ;
    s_inst_config.isUHSCard = FALSE;
    s_inst_config.sdioCardInt = cardInt;
    s_inst_config.sdioCardIntArg = NULL;

    /*init iomux*/
    FIOMuxInit();
    FIOPadSetSdMux(s_inst_config.hostId);
    if (kStatus_Success != SDIO_CfgInitialize(sdio_host, &s_inst_config))
    {
        PRINTF("Config SDIO failed !!! \r\n");
    }

    PRINTF("%s success\r\n", __func__);
    return;
}
