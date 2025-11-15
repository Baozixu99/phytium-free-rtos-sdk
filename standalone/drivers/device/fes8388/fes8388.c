/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fes8388.c
 * Created Date: 2024-04-03 15:18:46
 * Last Modified: 2024-04-19 18:00:08
 * Description:  This file is for
 *
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#include <string.h>
#include <stdio.h>
#include "ftypes.h"
#include "fdrivers_port.h"

#include "fiopad.h"
#include "fparameters.h"
#include "fio_mux.h"
#include "fi2c.h"
#include "fi2c_hw.h"
#include "fes8388.h"

/* ES8388 address */
#define ES8388_ADDR 0x11
/**
 * @name: FEs8388WriteByte
 * @msg:  write one byte to es8388
 * @param Null
 * @return FES8388_SUCCESS or others
 */
static FError FEs8388WriteByte(FI2c *instance_p, u32 mem_addr, u8 value)
{
    FError ret = FES8388_SUCCESS;
    ret = FI2cMasterWritePoll(instance_p, mem_addr, ONE_BYTE_OF_MEMORY, &value, 1);
    FDriverUdelay(8); //Bus Free Time Between Transmissions
    return ret;
}

/**
 * @name: FEs8388ReadByte
 * @msg:  read one byte to es8388
 * @param Null
 * @return FES8388_SUCCESS or others
 */
static FError FEs8388ReadByte(FI2c *instance_p, u32 mem_addr, u8 *value)
{
    FError ret = FES8388_SUCCESS;
    ret = FI2cMasterReadPoll(instance_p, mem_addr, ONE_BYTE_OF_MEMORY, value, 1);
    FDriverUdelay(8); //Bus Free Time Between Transmissions
    return ret;
}

/**
 * @name: FEs8388ConfigI2c
 * @msg:  set the mio and i2c params
 * @param Null
 * @return FES8388_SUCCESS or others
 */
FError FEs8388I2cInit(FDevice *dev)
{
    FError ret = FES8388_SUCCESS;
    FEs8388Controller *fes8388;
    fes8388 = (FEs8388Controller *)(dev);
    FI2c *instance_p = &fes8388->i2c_controller;
    FI2cConfig config_p;

    /* get standard config of i2c */
    config_p = *FI2cLookupConfig(fes8388->controller_id);
    /* Setup iomux */
    FIOPadSetI2CMux(fes8388->controller_id);
    memset(instance_p, 0, sizeof(*instance_p));
    FI2cDeInitialize(instance_p);
    /* Initialization */
    ret = FI2cCfgInitialize(instance_p, &config_p);
    if (FI2C_SUCCESS != ret)
    {
        printf("Init master I2c failed, ret: 0x%x\r\n", ret);
        return ret;
    }
    /*set the i2c parameters */
    ret = FI2cSetAddress(instance_p, FI2C_MASTER, ES8388_ADDR);
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

    printf("Set target slave_addr: 0x%x with I2c-%d\r\n", instance_p->config.slave_addr,
           fes8388->controller_id);

    return ret;
}

//use single LIN2 and RIN2
/**
 * @name: FEs8388Init
 * @msg:  init the es8388 regs
 * @return Null
 */
FError FEs8388Startup(FDevice *dev, u32 open_flag)
{
    FError ret = FES8388_SUCCESS;
    FEs8388Controller *fes8388;
    fes8388 = (FEs8388Controller *)(dev);
    FI2c *instance_p = &fes8388->i2c_controller;

    FEs8388WriteByte(instance_p, ES8388_CHIPPOWER, 0xF3); //Stop STM, DLL, and digital block
    FEs8388WriteByte(instance_p, ES8388_MASTERMODE, 0x00); // ES8388 in I2S slave mode
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL21, 0x80); // Set ADC and DAC have the same LRCK
    FEs8388WriteByte(instance_p, ES8388_CONTROL1, 0x05); //start up reference)
    FEs8388WriteByte(instance_p, ES8388_CONTROL2, 0x40); //start up reference)

    FEs8388WriteByte(instance_p, ES8388_ADCPOWER, 0x00); //Power on ADC and LIN/RIN input
    FEs8388WriteByte(instance_p, ES8388_DACPOWER, 0xc);  //Power on LIN/RIN

    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL1, 0x00); //+0dB
    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL2, 0x50); //use single LIN2 and RIN2

    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL4, 0x40); //I2S – 24bits, Ldata = LADC, Rdata = LADC)
    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL5, 0x02); //MCLK/LRCK = 256
    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL8, 0x00); //LADC volume = 0dB
    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL9, 0x00); //RADC volume = 0dB

    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL10, 0xe2); //ALC setting
    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL11, 0xa0); //ALC setting
    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL12, 0x12); //ALC setting
    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL13, 0x06); //ALC setting
    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL14, 0xc3); //nose gate = -40.5dB, NGG = 0x01(mute ADC))

    FEs8388WriteByte(instance_p, ES8388_DACCONTROL1, 0x00); //I2S mode, 24bits
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL2, 0x02); //MCLK/LRCK = 256
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL4, 0x00); //0dB volume
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL5, 0x00); //0dB volume

    FEs8388WriteByte(instance_p, ES8388_DACCONTROL3, 0x32); //dac unmute
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL16, 0x9); //Left input select for output mix
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL17, 0xB8); //left DAC to left mixer enable
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL18, 0x38); //default
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL19, 0x38); //default
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL20, 0xB8); //right DAC to left mixer enable

    /*volume control*/
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL24, 0x1e);
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL25, 0x1e);
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL26, 0x1e);
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL27, 0x1e);

    FEs8388WriteByte(instance_p, ES8388_CHIPPOWER, 0x00); //Start up DLL, STM and Digital block for recording

    return ret;
}

/**
 * @name: FEs8388DeInit
 * @msg:  init the es8388 regs
 * @return Null
 */
FError FEs8388ShutDown(FDevice *dev)
{
    FError ret = FES8388_SUCCESS;
    FEs8388Controller *fes8388;
    fes8388 = (FEs8388Controller *)(dev);
    FI2c *instance_p = &fes8388->i2c_controller;
    FEs8388WriteByte(instance_p, ES8388_CHIPPOWER, 0xFF); /*reset and stop es8388*/

    return ret;
}

/**
 * @name: FEs8388SetWordlength
 * @msg:  set the es8388 word lenth
 * @param (u32} length ,the es8388 word lenth
 * @return Null
 */
FError FEs8388SetWordlength(FI2c *instance_p, FEs8388PcmWordLength length)
{
    FError ret = FES8388_SUCCESS;
    u8 value = 0;
    u8 reg_value = 0;

    /*set the word lenth*/
    switch (length)
    {
        case AUDIO_PCM_STREAM_WORD_LENGTH_16:
            value = ES8388_DACWL_16;
            FEs8388ReadByte(instance_p, ES8388_DACCONTROL1, &reg_value);
            FEs8388WriteByte(instance_p, ES8388_DACCONTROL1, reg_value | value);
            value = ES8388_ADCWL_16;
            FEs8388ReadByte(instance_p, ES8388_ADCCONTROL4, &reg_value);
            FEs8388WriteByte(instance_p, ES8388_ADCCONTROL4, reg_value | value);
            break;
        case AUDIO_PCM_STREAM_WORD_LENGTH_20:
            value = ES8388_DACWL_20;
            FEs8388ReadByte(instance_p, ES8388_DACCONTROL1, &reg_value);
            FEs8388WriteByte(instance_p, ES8388_DACCONTROL1, reg_value | value);
            value = ES8388_ADCWL_20;
            FEs8388ReadByte(instance_p, ES8388_ADCCONTROL4, &reg_value);
            FEs8388WriteByte(instance_p, ES8388_ADCCONTROL4, reg_value | value);
            break;
        case AUDIO_PCM_STREAM_WORD_LENGTH_24:
            value = ES8388_DACWL_24;
            FEs8388ReadByte(instance_p, ES8388_DACCONTROL1, &reg_value);
            FEs8388WriteByte(instance_p, ES8388_DACCONTROL1, reg_value | value);
            value = ES8388_ADCWL_24;
            FEs8388ReadByte(instance_p, ES8388_ADCCONTROL4, &reg_value);
            FEs8388WriteByte(instance_p, ES8388_ADCCONTROL4, reg_value | value);
            break;
        case AUDIO_PCM_STREAM_WORD_LENGTH_32:
            value = ES8388_DACWL_32;
            FEs8388ReadByte(instance_p, ES8388_DACCONTROL1, &reg_value);
            FEs8388WriteByte(instance_p, ES8388_DACCONTROL1, reg_value | value);
            value = ES8388_ADCWL_32;
            FEs8388ReadByte(instance_p, ES8388_ADCCONTROL4, &reg_value);
            FEs8388WriteByte(instance_p, ES8388_ADCCONTROL4, reg_value | value);
            break;
        default:
            value = ES8388_DACWL_16;
            FEs8388ReadByte(instance_p, ES8388_DACCONTROL1, &reg_value);
            FEs8388WriteByte(instance_p, ES8388_DACCONTROL1, reg_value | value);
            value = ES8388_ADCWL_16;
            FEs8388ReadByte(instance_p, ES8388_ADCCONTROL4, &reg_value);
            FEs8388WriteByte(instance_p, ES8388_ADCCONTROL4, reg_value | value);
            break;
    }

    return ret;
}

/**
 * @name: FEs8388SetVolumel
 * @msg:  set the es8388 voice volume
 * @param (u32} value ,the volumel value
 * @return FES8388_SUCCESS
 */
FError FEs8388SetVolumel(FI2c *instance_p, u32 value)
{
    FError ret = FES8388_SUCCESS;

    if (value > 0xc0)
    {
        printf("volume value is too small,the min value is 0xc0 = -96dB\r\n");
        value = 0xc0;
    }
    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL8, value);
    FEs8388WriteByte(instance_p, ES8388_ADCCONTROL9, value);
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL5, value);
    FEs8388WriteByte(instance_p, ES8388_DACCONTROL4, value);

    return ret;
}

void FDumpEs8388Regs(FI2c *instance_p)
{
    u8 reg_value = 0;

    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_CONTROL1,
           FEs8388ReadByte(instance_p, ES8388_CONTROL1, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_CONTROL2,
           FEs8388ReadByte(instance_p, ES8388_CONTROL2, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_CHIPPOWER,
           FEs8388ReadByte(instance_p, ES8388_CHIPPOWER, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCPOWER,
           FEs8388ReadByte(instance_p, ES8388_ADCPOWER, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACPOWER,
           FEs8388ReadByte(instance_p, ES8388_DACPOWER, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_CHIPLOPOW1,
           FEs8388ReadByte(instance_p, ES8388_CHIPLOPOW1, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_CHIPLOPOW2,
           FEs8388ReadByte(instance_p, ES8388_CHIPLOPOW2, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ANAVOLMANAG,
           FEs8388ReadByte(instance_p, ES8388_ANAVOLMANAG, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_MASTERMODE,
           FEs8388ReadByte(instance_p, ES8388_MASTERMODE, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL1,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL1, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL2,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL2, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL3,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL3, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL4,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL4, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL5,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL5, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL6,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL6, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL7,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL7, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL8,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL8, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL9,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL9, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL10,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL10, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL11,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL11, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL12,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL12, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL13,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL13, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_ADCCONTROL14,
           FEs8388ReadByte(instance_p, ES8388_ADCCONTROL14, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL1,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL1, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL2,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL2, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL3,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL3, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL4,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL4, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL5,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL5, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL6,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL6, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL7,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL7, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL8,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL8, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL9,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL9, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL10,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL10, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL11,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL11, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL12,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL12, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL13,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL13, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL14,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL14, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL15,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL15, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL16,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL16, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL17,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL17, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL18,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL18, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL19,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL19, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL20,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL20, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL21,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL21, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL22,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL22, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL23,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL23, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL24,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL24, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL25,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL25, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL26,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL26, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL27,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL27, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL28,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL28, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL29,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL29, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_DACCONTROL30,
           FEs8388ReadByte(instance_p, ES8388_DACCONTROL30, &reg_value), reg_value);
    printf("addr = %x,%d,reg_value = %x\r\n", ES8388_REG_MAX,
           FEs8388ReadByte(instance_p, ES8388_REG_MAX, &reg_value), reg_value);
}

static FError FEs8388Control(FDevice *dev, int cmd, void *args)
{
    FError ret = FES8388_SUCCESS;
    FEs8388Controller *fes8388;
    fes8388 = (FEs8388Controller *)(dev);
    FI2c *instance_p = &fes8388->i2c_controller;

    switch (cmd)
    {
        case FES8388_SET_BIAS_LEVEL:

            break;
        case FES8388_SET_FORMAT:
            ret = FEs8388SetWordlength(instance_p, *(u32 *)args);
            break;
        case FES8388_SET_VOLUMEL:
            ret = FEs8388SetVolumel(instance_p, *(u32 *)args);
            break;
        case FES8388_MUTE_DOWN:

            break;
        default:
            break;
    }

    return ret;
}

static const FBusOps i2c_bus_ops = {
    .init = FEs8388I2cInit,
    .open = FEs8388Startup,
    .close = FEs8388ShutDown,
    .read = NULL,
    .write = NULL,
    .control = FEs8388Control,
};

FError FEs8388DevRegister(FDevice *dev)
{
    FError ret = FES8388_SUCCESS;
    FEs8388Controller *fes8388;
    fes8388 = (FEs8388Controller *)(dev);

    ret = FRegisterDevice(dev);

    if (fes8388->dev_type == DEV_TYPE_I2C)
    {
        dev->ops = &i2c_bus_ops;
    }

    return ret;
}