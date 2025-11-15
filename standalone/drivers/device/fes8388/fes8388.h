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
 * FilePath: fes8388.h
 * Created Date: 2024-04-03 15:18:46
 * Last Modified: 2024-04-19 17:55:27
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#ifndef FES8388_H
#define FES8388_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* includes */

#include "ftypes.h"
#include "fi2c.h"

#include "fparameters.h"
#include "ferror_code.h"
#include "fdevice.h"

#define FES8388_SUCCESS        FT_SUCCESS
#define FES8388_SET_BIAS_LEVEL 0x1
#define FES8388_SET_FORMAT     0x2
#define FES8388_SET_VOLUMEL    0x3
#define FES8388_MUTE_DOWN      0x4

typedef enum
{
    DEV_TYPE_I2C,
} FEs8388DevType;

typedef struct
{
    FDevice fes8388_device;
    FEs8388DevType dev_type;
    u32 controller_id;
    FI2c i2c_controller;
} FEs8388Controller;

typedef enum
{
    AUDIO_PCM_STREAM_WORD_LENGTH_16 = 0, /*the trams audio length word length*/
    AUDIO_PCM_STREAM_WORD_LENGTH_20,
    AUDIO_PCM_STREAM_WORD_LENGTH_24,
    AUDIO_PCM_STREAM_WORD_LENGTH_32,
    AUDIO_PCM_STREAM_WORD_LENGTH
} FEs8388PcmWordLength;

typedef enum
{
    AUDIO_PCM_STREAM_CAPTURE = 0, /*es8388 work mode*/
    AUDIO_PCM_STREAM_PLAYBACK,
    AUDIO_PCM_STREAM_TYPE

} FEs8388WorkFormat;

typedef enum
{
    AUDIO_I2S_STREAM = 0, /*es8388 stream mode*/
    AUDIO_LEFT_STREAM,
    AUDIO_RIGHT_STREAM,
    AUDIO_PCM_STREAM,
    AUDIO_STREAM_TYPE
} FEs8388Format;

typedef enum
{
    AUDIO_PCM_BIAS_LEVEL_OFF = 0, /*the bias level of es8388, use the default value:AUDIO_PCM_BIAS_LEVEL_ON*/
    AUDIO_PCM_BIAS_LEVEL_STANDBY,
    AUDIO_PCM_BIAS_LEVEL_REPARE,
    AUDIO_PCM_BIAS_LEVEL_ON,
    AUDIO_PCM_BIAS_LEVEL
} FEs8388BiasLevel;

typedef enum
{
    ONE_BYTE_OF_MEMORY = 1, /*8 bits of memory address*/
    TWO_BYTES_OF_MEMORY,    /*16 bits of memory address*/
    THREE_BYTES_OF_MEMORY,  /*24 bits of memory address*/
    FOUR_BYTES_OF_MEMORY,   /*32 bits of memory address*/
    ES838_BYTES_OF_MEMORY
} FEs8388SizeofRegister;

typedef struct
{
    void *pram;
} FEs8388;

/* ES8388 register */
#define ES8388_CONTROL1     0x00
#define ES8388_CONTROL2     0x01

#define ES8388_CHIPPOWER    0x02

#define ES8388_ADCPOWER     0x03
#define ES8388_DACPOWER     0x04

#define ES8388_CHIPLOPOW1   0x05
#define ES8388_CHIPLOPOW2   0x06

#define ES8388_ANAVOLMANAG  0x07

#define ES8388_MASTERMODE   0x08
/* ADC */
#define ES8388_ADCCONTROL1  0x09
#define ES8388_ADCCONTROL2  0x0a
#define ES8388_ADCCONTROL3  0x0b
#define ES8388_ADCCONTROL4  0x0c
#define ES8388_ADCCONTROL5  0x0d
#define ES8388_ADCCONTROL6  0x0e
#define ES8388_ADCCONTROL7  0x0f
#define ES8388_ADCCONTROL8  0x10
#define ES8388_ADCCONTROL9  0x11
#define ES8388_ADCCONTROL10 0x12
#define ES8388_ADCCONTROL11 0x13
#define ES8388_ADCCONTROL12 0x14
#define ES8388_ADCCONTROL13 0x15
#define ES8388_ADCCONTROL14 0x16
/* DAC */
#define ES8388_DACCONTROL1  0x17
#define ES8388_DACCONTROL2  0x18
#define ES8388_DACCONTROL3  0x19
#define ES8388_DACCONTROL4  0x1a
#define ES8388_DACCONTROL5  0x1b
#define ES8388_DACCONTROL6  0x1c
#define ES8388_DACCONTROL7  0x1d
#define ES8388_DACCONTROL8  0x1e
#define ES8388_DACCONTROL9  0x1f
#define ES8388_DACCONTROL10 0x20
#define ES8388_DACCONTROL11 0x21
#define ES8388_DACCONTROL12 0x22
#define ES8388_DACCONTROL13 0x23
#define ES8388_DACCONTROL14 0x24
#define ES8388_DACCONTROL15 0x25
#define ES8388_DACCONTROL16 0x26
#define ES8388_DACCONTROL17 0x27
#define ES8388_DACCONTROL18 0x28
#define ES8388_DACCONTROL19 0x29
#define ES8388_DACCONTROL20 0x2a
#define ES8388_DACCONTROL21 0x2b
#define ES8388_DACCONTROL22 0x2c
#define ES8388_DACCONTROL23 0x2d
#define ES8388_DACCONTROL24 0x2e
#define ES8388_DACCONTROL25 0x2f
#define ES8388_DACCONTROL26 0x30
#define ES8388_DACCONTROL27 0x31
#define ES8388_DACCONTROL28 0x32
#define ES8388_DACCONTROL29 0x33
#define ES8388_DACCONTROL30 0x34
#define ES8388_REG_MAX      0x35

#define ES8388_ADCWL_32     (0x4 << 2)
#define ES8388_ADCWL_24     (0x0 << 2)
#define ES8388_ADCWL_20     (0x1 << 2)
#define ES8388_ADCWL_18     (0x2 << 2)
#define ES8388_ADCWL_16     (0x3 << 2)

#define ES8388_DACWL_32     (0x4 << 3)
#define ES8388_DACWL_24     (0x0 << 3)
#define ES8388_DACWL_20     (0x1 << 3)
#define ES8388_DACWL_18     (0x2 << 3)
#define ES8388_DACWL_16     (0x3 << 3)

#define ES8388_FORMAT_I2S   (0x0)
#define ES8388_FORMAT_LEFT  (0x1 << 2)
#define ES8388_FORMAT_RIGHT (0x0 << 2)
#define ES8388_FORMAT_PCM   (0x2 << 2)


FError FEs8388DevRegister(FDevice *dev);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */