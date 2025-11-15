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
 * FilePath: fes8336.c
 * Date: 2023-05-23 17:14:36
 * LastEditTime: 2023-05-23 17:14:36
 * Description:  This file is for providing functions to file es8336.c
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangzongqiang    2023/05/23     init
 */
#include <string.h>
#include <stdio.h>
#include "ftypes.h"
#include "fdrivers_port.h"
#include "fes8336.h"
#include "fiopad.h"

#include "fparameters.h"
#include "fi2c.h"
#include "fi2c_hw.h"
#include "fmio_hw.h"
#include "fmio.h"

/*i2s slave address*/
#define ES8336_ADDR 0x10

/**
 * @name: FEs8336WriteByte
 * @msg:  write one byte to es8336
 * @param Null
 * @return FES8336_SUCCESS or others
 */
static FError FEs8336WriteByte(FI2c *instance_p, u32 mem_addr, u8 value)
{
    FError ret = FES8336_SUCCESS;
    ret = FI2cMasterWritePoll(instance_p, mem_addr, ONE_BYTE_OF_MEMORY, &value, 1);
    FDriverUdelay(4); //Bus Free Time Between Transmissions
    return ret;
}

/**
 * @name: FEs8336ReadByte
 * @msg:  read one byte to es8336
 * @param Null
 * @return FES8336_SUCCESS or others
 */
static FError FEs8336ReadByte(FI2c *instance_p, u32 mem_addr, u8 *value)
{
    FError ret = FES8336_SUCCESS;
    ret = FI2cMasterReadPoll(instance_p, mem_addr, ONE_BYTE_OF_MEMORY, value, 1);
    FDriverUdelay(4); //Bus Free Time Between Transmissions
    return ret;
}

/**
 * @name: FEs8336Reset
 * @msg:  reset the es8336
 * @return Null
 */
static void FEs8336Reset(FDevice *dev)
{
    FEs8336Controller *fes8336;
    fes8336 = (FEs8336Controller *)(dev);
    FI2c *instance_p = &fes8336->i2c_controller;
    u8 value = 0;
    value = FEs8336ReadByte(instance_p, ES8336_RESET_REG00, &value);
    FEs8336WriteByte(instance_p, ES8336_RESET_REG00, 0x3F);
    FDriverUdelay(100);
    FEs8336WriteByte(instance_p, ES8336_RESET_REG00, 0x03);
}
/**
 * @name: FEs8336InitRegs
 * @msg:  init the es8336 regs
 * @return Null
 */
static void FEs8336InitRegs(FDevice *dev)
{
    FEs8336Controller *fes8336;
    fes8336 = (FEs8336Controller *)(dev);
    FI2c *instance_p = &fes8336->i2c_controller;

    FEs8336WriteByte(instance_p, ES8336_RESET_REG00, 0x3f);
    FDriverUdelay(500); /*ensure reset success*/
    FEs8336WriteByte(instance_p, ES8336_RESET_REG00, 0x00);
    FEs8336WriteByte(instance_p, ES8336_SYS_VMIDSEL_REG0C, 0xFF); /*vmisel config */
    FDriverUdelay(30);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_CLKSEL_REG02, 0x08);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_ADCOSR_REG03, 0x20);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_ADCDIV1_REG04, 0x11);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_ADCDIV2_REG05, 0x00);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_DACDIV1_REG06, 0x11);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_DACDIV2_REG07, 0x00);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_CPDIV_REG08, 0x00);
    FEs8336WriteByte(instance_p, ES8336_SDP_MS_BCKDIV_REG09, 0x04);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_CLKSW_REG01, 0x7F);
    FEs8336WriteByte(instance_p, ES8336_CAL_TYPE_REG1C, 0x0F);
    FEs8336WriteByte(instance_p, ES8336_CAL_HPLIV_REG1E, 0x90);
    FEs8336WriteByte(instance_p, ES8336_CAL_HPRIV_REG1F, 0x90);
    FEs8336WriteByte(instance_p, ES8336_ADC_VOLUME_REG27, 0x00);
    FEs8336WriteByte(instance_p, ES8336_ADC_PDN_LINSEL_REG22, 0xc0);
    FEs8336WriteByte(instance_p, ES8336_ADC_D2SEPGA_REG24, 0x00);
    FEs8336WriteByte(instance_p, ES8336_ADC_DMIC_REG25, 0x08);
    FEs8336WriteByte(instance_p, ES8336_DAC_SET1_REG30, 0x00);
    FEs8336WriteByte(instance_p, ES8336_DAC_SET2_REG31, 0x20);
    FEs8336WriteByte(instance_p, ES8336_DAC_SET3_REG32, 0x00);
    FEs8336WriteByte(instance_p, ES8336_DAC_VOLL_REG33, 0x00);
    FEs8336WriteByte(instance_p, ES8336_DAC_VOLR_REG34, 0x00);
    FEs8336WriteByte(instance_p, ES8336_SDP_ADCFMT_REG0A, 0x00);
    FEs8336WriteByte(instance_p, ES8336_SDP_DACFMT_REG0B, 0x00);
    FEs8336WriteByte(instance_p, ES8336_SYS_VMIDLOW_REG10, 0x11);
    FEs8336WriteByte(instance_p, ES8336_SYS_VSEL_REG11, 0xFC);
    FEs8336WriteByte(instance_p, ES8336_SYS_REF_REG12, 0x28);
    FEs8336WriteByte(instance_p, ES8336_SYS_LP1_REG0E, 0x04);
    FEs8336WriteByte(instance_p, ES8336_SYS_LP2_REG0F, 0x0C);
    FEs8336WriteByte(instance_p, ES8336_DAC_PDN_REG2F, 0x11);
    FEs8336WriteByte(instance_p, ES8336_HPMIX_SEL_REG13, 0x00);
    FEs8336WriteByte(instance_p, ES8336_HPMIX_SWITCH_REG14, 0x88);
    FEs8336WriteByte(instance_p, ES8336_HPMIX_PDN_REG15, 0x00);
    FEs8336WriteByte(instance_p, ES8336_HPMIX_VOL_REG16, 0xBB);
    FEs8336WriteByte(instance_p, ES8336_CPHP_PDN2_REG1A, 0x10);
    FEs8336WriteByte(instance_p, ES8336_CPHP_LDOCTL_REG1B, 0x30);
    FEs8336WriteByte(instance_p, ES8336_CPHP_PDN1_REG19, 0x02);
    FEs8336WriteByte(instance_p, ES8336_CPHP_ICAL_VOL_REG18, 0x00);
    FEs8336WriteByte(instance_p, ES8336_GPIO_SEL_REG4D, 0x02);
    FEs8336WriteByte(instance_p, ES8336_GPIO_DEBUNCE_INT_REG4E, 0x02);
    FEs8336WriteByte(instance_p, ES8336_TESTMODE_REG50, 0xA0);
    FEs8336WriteByte(instance_p, ES8336_TEST1_REG51, 0x00);
    FEs8336WriteByte(instance_p, ES8336_TEST2_REG52, 0x00);
    FEs8336WriteByte(instance_p, ES8336_SYS_PDN_REG0D, 0x00);
    FEs8336WriteByte(instance_p, ES8336_RESET_REG00, 0xC0);
    FDriverUdelay(50);
    FEs8336WriteByte(instance_p, ES8336_ADC_PGAGAIN_REG23, 0x60);
    FEs8336WriteByte(instance_p, ES8336_ADC_D2SEPGA_REG24, 0x01);
    /* adc ds mode, HPF enable */
    FEs8336WriteByte(instance_p, ES8336_ADC_DMIC_REG25, 0x08);
    FEs8336WriteByte(instance_p, ES8336_ADC_ALC1_REG29, 0xcd);
    FEs8336WriteByte(instance_p, ES8336_ADC_ALC2_REG2A, 0x08);
    FEs8336WriteByte(instance_p, ES8336_ADC_ALC3_REG2B, 0xa0);
    FEs8336WriteByte(instance_p, ES8336_ADC_ALC4_REG2C, 0x05);
    FEs8336WriteByte(instance_p, ES8336_ADC_ALC5_REG2D, 0x06);
    FEs8336WriteByte(instance_p, ES8336_ADC_ALC6_REG2E, 0x61);
    return;
}

/**
 * @name: FEs8336RegsProbe
 * @msg:  probe the es8336 regs
 * @return Null
 */
static void FEs8336RegsProbe(FDevice *dev)
{
    FEs8336Controller *fes8336;
    fes8336 = (FEs8336Controller *)(dev);
    FI2c *instance_p = &fes8336->i2c_controller;

    FEs8336Reset(dev);
    FEs8336InitRegs(dev);
    FEs8336WriteByte(instance_p, ES8336_GPIO_SEL_REG4D, 0x02);
    /* max debance time, enable interrupt, low active */
    FEs8336WriteByte(instance_p, ES8336_GPIO_DEBUNCE_INT_REG4E, 0xf3);
    /* es8336_set_bias_level(codec, SND_SOC_BIAS_OFF); */
    FEs8336WriteByte(instance_p, ES8336_CPHP_OUTEN_REG17, 0x00);
    FEs8336WriteByte(instance_p, ES8336_DAC_PDN_REG2F, 0x11);
    FEs8336WriteByte(instance_p, ES8336_CPHP_LDOCTL_REG1B, 0x03);
    FEs8336WriteByte(instance_p, ES8336_CPHP_PDN2_REG1A, 0x22);
    FEs8336WriteByte(instance_p, ES8336_CPHP_PDN1_REG19, 0x06);
    FEs8336WriteByte(instance_p, ES8336_HPMIX_SWITCH_REG14, 0x00);
    FEs8336WriteByte(instance_p, ES8336_HPMIX_PDN_REG15, 0x33);
    FEs8336WriteByte(instance_p, ES8336_HPMIX_VOL_REG16, 0x00); //此寄存器在播放和录音同时开启时，需设置为0
    FEs8336WriteByte(instance_p, ES8336_SYS_LP1_REG0E, 0xFF);
    FEs8336WriteByte(instance_p, ES8336_SYS_LP2_REG0F, 0xFF);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_CLKSW_REG01, 0xF3);
    FEs8336WriteByte(instance_p, ES8336_ADC_PDN_LINSEL_REG22, 0xD0); //the source init
    return;
}

/**
 * @name: FEs8336Init
 * @msg:  init the es8336, include the mio and i2c params
 * @param Null
 * @return FES8336_SUCCESS or others
 */
static FError FEs8336MioInit(FDevice *dev)
{
    FError ret = FES8336_SUCCESS;

    FEs8336Controller *fes8336;
    fes8336 = (FEs8336Controller *)(dev);

    FMioCtrl *pctrl = &fes8336->mio_controller;
    const FMioConfig *mioconfig_p;
    FI2c *instance_p = &fes8336->i2c_controller;
    FI2cConfig i2c_config;

    mioconfig_p = FMioLookupConfig(fes8336->controller_id);
    if (NULL == mioconfig_p)
    {
        printf("Mio error inval parameters.\r\n");
        return FMIO_ERR_INVAL_PARM;
    }
    pctrl->config = *mioconfig_p;
    ret = FMioFuncInit(pctrl, FMIO_FUNC_SET_I2C);
    if (ret != FES8336_SUCCESS)
    {
        printf("ES8336_MIO MioInit error.\r\n");
        return ret;
    }

    /* Modify configuration */
    memset(&i2c_config, 0, sizeof(i2c_config));
    i2c_config.base_addr = FMioFuncGetAddress(pctrl, FMIO_FUNC_SET_I2C);
    i2c_config.irq_num = FMioFuncGetIrqNum(pctrl, FMIO_FUNC_SET_I2C);
    i2c_config.irq_prority = 0;
    i2c_config.ref_clk_hz = FMIO_CLK_FREQ_HZ;
    i2c_config.work_mode = FI2C_MASTER;
    i2c_config.use_7bit_addr = TRUE;
    i2c_config.speed_rate = FI2C_SPEED_STANDARD_RATE;
    i2c_config.auto_calc = TRUE;
    /* Initialization */
    ret = FI2cCfgInitialize(instance_p, &i2c_config);
    if (ret != FES8336_SUCCESS)
    {
        return ret;
    }
    /*set the i2c parameters */
    ret = FI2cSetAddress(instance_p, FI2C_MASTER, ES8336_ADDR);
    if (FI2C_SUCCESS != ret)
    {
        printf("set mio slave parameters failed, ret: 0x%x\r\n", ret);
        return ret;
    }

    ret = FI2cSetSpeed(instance_p, FI2C_SPEED_STANDARD_RATE, TRUE);
    if (FI2C_SUCCESS != ret)
    {
        printf("set mio slave parameters failed, ret: 0x%x\r\n", ret);
        return ret;
    }
    /*  callback function for FI2C_MASTER_INTR_EVT interrupt  */
    instance_p->master_evt_handlers[FI2C_EVT_MASTER_TRANS_ABORTED] = NULL;
    instance_p->master_evt_handlers[FI2C_EVT_MASTER_READ_DONE] = NULL;
    instance_p->master_evt_handlers[FI2C_EVT_MASTER_WRITE_DONE] = NULL;

    return ret;
}


/**
 * @name: FEs8336Startup
 * @msg:  start up the es8336
 * @return Null
 */

static FError FEs8336Startup(FDevice *dev, u32 open_flag)
{
    FError ret = FES8336_SUCCESS;
    FEs8336Controller *fes8336;
    fes8336 = (FEs8336Controller *)(dev);
    FI2c *instance_p = &fes8336->i2c_controller;
    FEs8336RegsProbe(dev);

    FEs8336WriteByte(instance_p, ES8336_RESET_REG00, 0xC0);
    FEs8336WriteByte(instance_p, ES8336_SYS_PDN_REG0D, 0x00);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_CLKSW_REG01, 0x7f); //set the clock
    FEs8336WriteByte(instance_p, ES8336_SYS_LP1_REG0E, 0x3F); //low power mode of ADC
    FEs8336WriteByte(instance_p, ES8336_SYS_LP2_REG0F, 0x1F); //low power mode of output
    FEs8336WriteByte(instance_p, ES8336_HPMIX_SWITCH_REG14, 0x88); //headphone mixer
    FEs8336WriteByte(instance_p, ES8336_HPMIX_PDN_REG15, 0x00);
    FEs8336WriteByte(instance_p, ES8336_HPMIX_VOL_REG16, 0x00); //the gain of headphone mixer
    FEs8336WriteByte(instance_p, ES8336_CPHP_PDN2_REG1A, 0x10); //Power down charge pump circuits
    FEs8336WriteByte(instance_p, ES8336_CPHP_LDOCTL_REG1B, 0x30); //set the voltage
    FEs8336WriteByte(instance_p, ES8336_CPHP_PDN1_REG19, 0x02);
    FEs8336WriteByte(instance_p, ES8336_DAC_PDN_REG2F, 0x00);
    FEs8336WriteByte(instance_p, ES8336_CPHP_OUTEN_REG17, 0x66);
    FEs8336WriteByte(instance_p, ES8336_RESET_REG00, 0xc0);
    FEs8336WriteByte(instance_p, ES8336_DAC_VOLL_REG33, 0x0);
    FEs8336WriteByte(instance_p, ES8336_DAC_VOLR_REG34, 0x28);
    FEs8336WriteByte(instance_p, ES8336_ADC_PDN_LINSEL_REG22, 0x30); //select the output and input source
    return ret;
}

/**
 * @name: FEs8336ShutDown
 * @msg:  shut down the es8336
 * @return FES8336_SUCCESS
 */
static FError FEs8336ShutDown(FDevice *dev)
{
    FError ret = FES8336_SUCCESS;
    u8 value = 0;
    FEs8336Controller *fes8336;
    fes8336 = (FEs8336Controller *)(dev);
    FI2c *instance_p = &fes8336->i2c_controller;

    FEs8336WriteByte(instance_p, ES8336_CPHP_OUTEN_REG17, 0x00);
    FEs8336WriteByte(instance_p, ES8336_DAC_PDN_REG2F, 0x11);
    FEs8336WriteByte(instance_p, ES8336_CPHP_LDOCTL_REG1B, 0x03);
    FEs8336WriteByte(instance_p, ES8336_CPHP_PDN2_REG1A, 0x22);
    FEs8336WriteByte(instance_p, ES8336_CPHP_PDN1_REG19, 0x06);
    FEs8336WriteByte(instance_p, ES8336_HPMIX_SWITCH_REG14, 0x00);
    FEs8336WriteByte(instance_p, ES8336_HPMIX_PDN_REG15, 0x33);
    FEs8336WriteByte(instance_p, ES8336_HPMIX_VOL_REG16, 0x00);
    FEs8336WriteByte(instance_p, ES8336_SYS_PDN_REG0D, 0x00);
    FEs8336WriteByte(instance_p, ES8336_SYS_LP1_REG0E, 0xFF);
    FEs8336ReadByte(instance_p, ES8336_CLKMGR_CLKSW_REG01, &value);
    value &= (~ES8336_CLKMGR_DAC_ANALOG_EN);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_CLKSW_REG01, value);

    FEs8336WriteByte(instance_p, ES8336_SYS_LP1_REG0E, 0xC0);
    FEs8336ReadByte(instance_p, ES8336_CLKMGR_CLKSW_REG01, &value);
    value &= (~ES8336_CLKMGR_ADC_MCLK_EN);
    value &= (~ES8336_CLKMGR_ADC_ANALOG_EN);
    FEs8336WriteByte(instance_p, ES8336_CLKMGR_CLKSW_REG01, value);

    FI2cDeInitialize(instance_p);
    return ret;
}

/**
 * @name: FEs8336MuteDown
 * @msg:  mute down the es8336
 * @param (u32} mute ,mute or not mute the es8336
 * @return FES8336_SUCCESS
 */
static FError FEs8336MuteDown(FI2c *instance_p, u32 mute)
{
    FError ret = FES8336_SUCCESS;

    if (mute)
    {
        FEs8336WriteByte(instance_p, ES8336_DAC_SET1_REG30, 0x20);
    }
    FEs8336WriteByte(instance_p, ES8336_DAC_SET1_REG30, 0x00);
    return ret;
}

/**
 * @name: FEs8336SetVolumel
 * @msg:  set the vilumel of es8336
 * @param (u32} value ,the volumel value
 * @return FES8336_SUCCESS
 */
static FError FEs8336SetVolumel(FI2c *instance_p, u32 value)
{
    FError ret = FES8336_SUCCESS;

    FEs8336WriteByte(instance_p, ES8336_DAC_VOLL_REG33, value);
    FEs8336WriteByte(instance_p, ES8336_DAC_VOLR_REG34, value);
    FEs8336WriteByte(instance_p, ES8336_ADC_VOLUME_REG27, value);
    FEs8336WriteByte(instance_p, ES8336_ADC_VOLUME_REG28, value);
    return ret;
}

/**
 * @name: FEs8336SetFormat
 * @msg:  set the es8336 word lenth
 * @param (u32} length ,the es8336 word lenth
 * @return FES8336_SUCCESS
 */
static FError FEs8336SetFormat(FI2c *instance_p, FEs8336PcmWordLength length)
{
    FError ret = FES8336_SUCCESS;
    u8 value = 0;
    u8 reg_value;
    switch (length)
    {
        case AUDIO_PCM_STREAM_WORD_LENGTH_16:
            value = ES8336_DACWL_16;
            break;
        case AUDIO_PCM_STREAM_WORD_LENGTH_20:
            value = ES8336_DACWL_20;
            break;
        case AUDIO_PCM_STREAM_WORD_LENGTH_24:
            value = ES8336_DACWL_24;
            break;
        case AUDIO_PCM_STREAM_WORD_LENGTH_32:
            value = ES8336_DACWL_32;
            break;
        default:
            value = ES8336_DACWL_32;
            break;
    }
    FEs8336ReadByte(instance_p, ES8336_SDP_ADCFMT_REG0A, &reg_value);
    FEs8336WriteByte(instance_p, ES8336_SDP_ADCFMT_REG0A, reg_value | value);
    FEs8336ReadByte(instance_p, ES8336_SDP_DACFMT_REG0B, &reg_value);
    FEs8336WriteByte(instance_p, ES8336_SDP_DACFMT_REG0B, reg_value | value);
    FEs8336WriteByte(instance_p, ES8336_DAC_SET1_REG30, 0x00); //the connect of Lin,Lout,Rin,Rout
    FEs8336WriteByte(instance_p, ES8336_DAC_VOLL_REG33, 0x0); //digital volume control
    FEs8336WriteByte(instance_p, ES8336_DAC_VOLR_REG34, 0x28);
    return ret;
}

/**
 * @name: FEs8336SetBiasLevel
 * @msg:  set the es8336 bias level
 * @param (u32} format ,the es8336 word lenth
 * @param (FEs8336WorkFormat} type ,the type of workmode of PCM
 * @return FES8336_SUCCESS
 */
static FError FEs8336SetBiasLevel(FI2c *instance_p, FEs8336BiasLevel level)
{
    FError ret = FES8336_SUCCESS;

    switch (level)
    {
        case AUDIO_PCM_BIAS_LEVEL_ON:
            break;
        case AUDIO_PCM_BIAS_LEVEL_REPARE:
            break;
        case AUDIO_PCM_BIAS_LEVEL_STANDBY:
            break;
        case AUDIO_PCM_BIAS_LEVEL_OFF:
            FEs8336WriteByte(instance_p, ES8336_CPHP_OUTEN_REG17, 0x00);
            FEs8336WriteByte(instance_p, ES8336_DAC_PDN_REG2F, 0x11);
            FEs8336WriteByte(instance_p, ES8336_CPHP_LDOCTL_REG1B, 0x03);
            FEs8336WriteByte(instance_p, ES8336_CPHP_PDN2_REG1A, 0x22);
            FEs8336WriteByte(instance_p, ES8336_CPHP_PDN1_REG19, 0x06);
            FEs8336WriteByte(instance_p, ES8336_HPMIX_SWITCH_REG14, 0x00);
            FEs8336WriteByte(instance_p, ES8336_HPMIX_PDN_REG15, 0x33);
            FEs8336WriteByte(instance_p, ES8336_HPMIX_VOL_REG16, 0x00);
            FEs8336WriteByte(instance_p, ES8336_ADC_PDN_LINSEL_REG22, 0xC0);
            FEs8336WriteByte(instance_p, ES8336_SYS_LP1_REG0E, 0x3F);
            FEs8336WriteByte(instance_p, ES8336_SYS_LP2_REG0F, 0x1F);
            FEs8336WriteByte(instance_p, ES8336_RESET_REG00, 0x00);
        default:
            break;
    }
    return ret;
}

static FError FEs8336Control(FDevice *dev, int cmd, void *args)
{
    FError ret = FES8336_SUCCESS;
    FEs8336Controller *fes8336;
    fes8336 = (FEs8336Controller *)(dev);
    FI2c *instance_p = &fes8336->i2c_controller;

    switch (cmd)
    {
        case FES8336_SET_BIAS_LEVEL:
            FEs8336SetBiasLevel(instance_p, *(u32 *)args);
            break;
        case FES8336_SET_FORMAT:
            FEs8336SetFormat(instance_p, *(u32 *)args);
            break;
        case FES8336_SET_VOLUMEL:
            FEs8336SetVolumel(instance_p, *(u32 *)args);
            break;
        case FES8336_MUTE_DOWN:
            FEs8336MuteDown(instance_p, *(u32 *)args);
            break;
        default:
            break;
    }

    return ret;
}

static const FBusOps mio_bus_ops = {
    .init = FEs8336MioInit,
    .open = FEs8336Startup,
    .close = FEs8336ShutDown,
    .read = NULL,
    .write = NULL,
    .control = FEs8336Control,
};

FError FEs8336DevRegister(FDevice *dev)
{
    FError ret = FES8336_SUCCESS;
    FEs8336Controller *fes8336;
    fes8336 = (FEs8336Controller *)(dev);

    ret = FRegisterDevice(dev);

    if (fes8336->dev_type == DEV_TYPE_MIO)
    {
        dev->ops = &mio_bus_ops;
    }

    return ret;
}
