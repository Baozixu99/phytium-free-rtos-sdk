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
 * FilePath: fdcdp_v2_reg.h
 * Created Date: 2025-01-22 14:05:55
 * Last Modified: 2025-07-08 16:28:45
 * Description:  This file is for
 *
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0      wzq         2025/07/08         first version
 */

#ifndef FDC_DP_V2_REG_H
#define FDC_DP_V2_REG_H

#ifdef __cplusplus
extern "C"
{
#endif
/******************************dc register start******************************************/
#define DC_REG_OFFSET                       0x8000
#define FDC_FRAMEBUFFER_Y_ADDRESS           0x1400
#define ADDRESS_MASK                        0xffffff80
#define FDC_FRAMEBUFFER_X_ADDRESS           0x1404
#define FDC_FRAMEBUFFER_Y_STRIDE            0x1408
#define FDC_DISPLAY_PANEL_CONFIG            0x1418
#define FDC_DISPLAY_DITHER_CONFIG_0         0x1410
#define FDC_DISPLAY_DITHER_CONFIG_1         0x1414
#define FDC_DISPLAY_DITHER_TABLE_LOW_0      0x1420
#define FDC_DISPLAY_DITHER_TABLE_LOW_1      0x1424
#define FDC_DISPLAY_DITHER_TABLE_HIGH_0     0x1428
#define FDC_DISPLAY_PANEL_CONFIG_X          0x2518
#define PANEL_DATAENABLE_ENABLE             (1 << 0)
#define PANEL_DATA_ENABLE                   (1 << 4)
#define PANEL_CLOCK_ENABLE                  (1 << 8)
#define FDC_HDISPLAY                        0x1430
#define HDISPLAY_END_SHIFT                  0
#define HDISPLAY_END_MASK                   0x7fff
#define HDISPLAY_TOTAL_SHIFT                16
#define HDISPLAY_TOTAL_MASK                 0x7fff
#define FDC_HSYNC                           0x1438
#define HSYNC_START_SHIFT                   0
#define HSYNC_START_MASK                    0x7fff
#define HSYNC_END_SHIFT                     15
#define HSYNC_END_MASK                      0x7fff
#define HSYNC_PULSE_ENABLED                 (1 << 30)
#define HSYNC_NEGATIVE                      (1 << 31)
#define FDC_VDISPLAY                        0x1440
#define VDISPLAY_END_SHIFT                  0
#define VDISPLAY_END_MASK                   0x7fff
#define VDISPLAY_TOTAL_SHIFT                16
#define VDISPLAY_TOTAL_MASK                 0x7fff
#define FDC_PHYYNC                          0x1448
#define PHYYNC_START_SHIFT                  0
#define PHYYNC_START_MASK                   0x7fff
#define PHYYNC_END_SHIFT                    15
#define PHYYNC_END_MASK                     0x7fff
#define PHYYNC_PULSE_ENABLED                (1 << 30)
#define FDC_DISPLAY_CURRENT_LOCATION        0x1450
#define PHYYNC_NEGATIVE                     (1 << 31)
#define FDC_GAMMA_INDEX                     0x1458
#define GAMMA_INDEX_MAX                     256
#define FDC_GAMMA_DATA                      0x1460
#define GAMMA_BLUE_SHIFT                    0
#define GAMMA_BLUE_MASK                     0x3ff
#define GAMMA_GREEN_SHIFT                   10
#define GAMMA_GREEN_MASK                    0x3ff
#define GAMMA_RED_SHIFT                     20
#define GAMMA_RED_MASK                      0x3ff
#define FDC_CURSOR_CONFIG                   0x1468
#define CURSOR_FORMAT_DISABLED              0x0
#define CURSOR_FORMAT_MASKMODE              0x3
#define CURSOR_FORMAT_ARGB8888              0x2
#define CURSOR_FORMAT_MASK                  0x3
#define CURSOR_HOT_Y_SHIFT                  8
#define CURSOR_HOT_Y_MASK                   0x1f
#define CURSOR_HOT_X_SHIFT                  16
#define CURSOR_HOT_X_MASK                   0x1f
#define FDC_CURSOR_ADDRESS                  0x146c
#define FDC_CURSOR_LOCATION                 0x1470
#define CURSOR_X_SHIFT                      0
#define CURSOR_X_MASK                       0x7fff
#define CURSOR_Y_SHIFT                      16
#define CURSOR_Y_MASK                       0x7fff
#define FDC_CURSOR_BACKGROUND               0x1474
#define FDC_CURSOR_FOREGROUND               0x1478
#define FDC_INT_STATUS                      0x147c
#define INT_STATUS                          0x1
#define FDC_INT_ENABLE                      0x1480
#define INT_ENABLE                          0x1
#define INT_DISABLE                         0x0

#define FDC_FRAMEBUFFER_CONFIG              0x1518
#define FRAMEBUFFER_OUTPUT                  BIT(0)
#define FRAMEBUFFER_GAMMA_ENABLE            BIT(2)
#define FRAMEBUFFER_VALID_PENDING           BIT(3)
#define FRAMEBUFFER_RESET                   BIT(4)
#define FRAMEBUFFER_PROGRESS                BIT(6)
#define FRAMEBUFFER_ROT_ANGLE_SHIFT         (11)
#define FRAMEBUFFER_ROT_ANGLE_MASK          (0x7)
#define FRAMEBUFFER_ROT_ANGLE_ROT0          (0)
#define FRAMEBUFFER_ROT_ANGLE_FLIP_X        (1)
#define FRAMEBUFFER_ROT_ANGLE_FLIP_Y        (2)
#define FRAMEBUFFER_TILE_MODE_SHIFT         (17)
#define FRAMEBUFFER_TILE_MODE_MASK          (0x1f)
#define FRAMEBUFFER_LINEAR                  0
#define FRAMEBUFFER_TILE_MODE0              4
#define FRAMEBUFFER_TILE_MODE3              7
#define FRAMEBUFFER_FORMAT_SHIFT            26
#define FRAMEBUFFER_FORMAT_MASK             0x3f
#define FRAMEBUFFER_FORMAT_XRGB4444         0x0
#define FRAMEBUFFER_FORMAT_ARGB4444         0x1
#define FRAMEBUFFER_FORMAT_XRGB1555         0x2
#define FRAMEBUFFER_FORMAT_ARGB1555         0x3
#define FRAMEBUFFER_FORMAT_RGB565           0x4
#define FRAMEBUFFER_FORMAT_XRGB8888         0x5
#define FRAMEBUFFER_FORMAT_ARGB8888         0x6
#define FRAMEBUFFER_FORMAT_YUYV             0x7
#define FRAMEBUFFER_FORMAT_UYVY             0x8
#define FRAMEBUFFER_FORMAT_NV12             0x11
#define FRAMEBUFFER_FORMAT_NV16             0x12
#define FRAMEBUFFER_FORMAT_ARGB2101010      0x16
#define FRAMEBUFFER_SWIZZLE_SHIFT           23
#define FRAMEBUFFER_SWIZZLE_MASK            0x3
#define FRAMEBUFFER_SWIZZLE_ARGB            0
#define FRAMEBUFFER_SWIZZLE_RGBA            1
#define FRAMEBUFFER_SWIZZLE_ABGR            2
#define FRAMEBUFFER_SWIZZLE_BGRA            3
#define FRAMEBUFFER_CONFIG0_UV_SWIZZLE      BIT(25)

#define FRAMEBUFFER_UPHYWIZZLE_SHIFT        25
#define FRAMEBUFFER_UPHYWIZZLE_DISABLE      0
#define FRAMEBUFFER_UPHYWIZZLE_ENABLE       1
#define FRAMEBUFFER_CLEAR                   BIT(8)
#define FRAMEBUFFER_SCALE_ENABLE            BIT(22)

#define FDC_FRAMEBUFFER_CONFIG_EX           0x1CC0
#define FDC_FRAMEBUFFER_SCALECONFIG         0x1520
#define FDC_FRAMEBUFFER_ALPHABLEND_CONFIG_0 0x2510
#define FRAMEBUFFER_FILTER_TAP              3
#define FRAMEBUFFER_HORIZONTAL_FILTER_TAP   3
#define FRAMEBUFFER_TAP                     0x33
#define FDC_FRAMEBUFFER_U_ADDRESS           0x1530
#define FDC_FRAMEBUFFER_V_ADDRESS           0x1538
#define FDC_OVERLAY_CONFIG                  0x1540

#define FDC_FRAMEBUFFER_U_STRIDE            0x1800
#define FDC_FRAMEBUFFER_V_STRIDE            0x1808
#define FDC_FRAMEBUFFER_SIZE                0x1810
#define FDC_FRAMEBUFFER_TL_SIZE             0x24D8
#define FDC_FRAMEBUFFER_BR_SIZE             0x24E0
#define WIDTH_SHIFT                         0
#define WIDTH_MASK                          0x7fff
#define HEIGHT_SHIFT                        15
#define HEIGHT_MASK                         0x7fff

#define FDC_FRAMEBUFFER_SCALE_FACTOR_X      0x1828
#define SCALE_FACTOR_X_MASK                 0x7fffffff
#define FDC_FRAMEBUFFER_SCALE_FACTOR_Y      0x1830
#define SCALE_FACTOR_Y_MASK                 0x7fffffff
#define SCALE_FACTOR_Y_MAX                  0x3
#define SCALE_FACTOR_SRC_OFFSET             16

#define FDC_FRAMEBUFFER_HORI_FILTER_INDEX   0x1838
#define HORI_FILTER_INDEX                   0x0
#define FDC_FRAMEBUFFER_HORI_FILTER         0x1a00
#define FDC_FRAMEBUFFER_VERT_FILTER_INDEX   0x1a08
#define VERT_FILTER_INDEX                   0x0
#define FDC_FRAMEBUFFER_VERT_FILTER         0x1a10
#define FDC_FRAMEBUFFER_CLEARVALUE          0x1a18
#define CLEAR_VALUE_RED                     0x00ff0000
#define CLEAR_VALUE_GREEN                   0x0000ff00
#define CLEAR_VALUE_BLACK                   0x00000000
#define FDC_FRAMEBUFFER_INITIALOFFSET       0x1a20
#define INITIALOFFSET                       (0x8000 | (0X8000 << 16))
#define FDC_DP_CONFIG                       0x1cd0
#define OUTPUT_DP                           (1 << 3)
#define DP_RGB666                           (0x1)
#define DP_RGB888                           (0x2)
#define DP_RGB101010                        (0x3)

#define FDC_DP_CONFIG_1                     0x1cd4
#define FDC_DISPLAY_PANEL_START             0x1CCC
#define AQ_AXI_CONFIG                       0x00008
#define AQ_HI_CLOCK_CONTROL                 0x00000
#define DC1_CORE_RESET                      (1 << 18)
#define DC0_CORE_RESET                      (1 << 17)
#define AXI_RESET                           (1 << 16)
#define AHB_RESET                           (1 << 12)

#define AQ_INTR_ENBL_Address                0x00014
/******************************dc register end********************************************/

/******************************phy access register****************************************/
#define FPHY_ACCESS_ADDRESS                 0x0000
#define FPHY_WRITE_DATA                     0x0004
#define FPHY_READ_DATA                      0x0008
#define FPHY_ACCESS_CTRL                    0x000c
#define ACCESS_WRITE                        (1 << 0)
#define ACCESS_READ                         (1 << 1)
/******************************phy access register end*************************************/

/******************************DPCD phy access register****************************************/
#define FDP_TEST_PHY_PATTERN                0x248
#define FDP_MAX_LINK_RATE                   0x001
#define FDP_MAX_LANE_COUNT                  0x002

#define MAX_LINK_RATE                       0x00001
#define MAX_LANE_COUNT                      0x00002
/******************************DPCD phy access register end*************************************/

/******************************dp register start******************************************/
#define DP_TRAINING_PATTERN_SET             0x102
#define DP_TRAINING_PATTERN_DISABLE         0
#define DP_TRAINING_PATTERN_1               1
#define DP_TRAINING_PATTERN_2               2
#define DP_TRAINING_PATTERN_3               3 /* 1.2 */
#define DP_TRAINING_PATTERN_4               7 /* 1.4 */
#define DP_TRAINING_PATTERN_MASK            0x3
#define DP_TRAINING_PATTERN_MASK_1_4        0xf

#define DP_LANE0_1_STATUS                   0x202
#define DP_LANE2_3_STATUS                   0x203
#define LANE_ALIGN_STATUS_UPDATED           0x00204
#define SINK_STATUS                         0x00205
#define ADJUST_REQUEST_LANE0_1              0x00206
#define ADJUST_REQUEST_LANE2_3              0x00207
#define TRAINING_SCORE_LANE0                0x00208
#define TRAINING_SCORE_LANE1                0x00209
#define TRAINING_SCORE_LANE2                0x0020A
#define TRAINING_SCORE_LANE3                0x0020B
#define SYMBOL_ERROR_COUNT_LANE0_LOW        0x00210
#define SYMBOL_ERROR_COUNT_LANE0_HIGH       0x00211
#define SYMBOL_ERROR_COUNT_LANE1_LOW        0x00212
#define SYMBOL_ERROR_COUNT_LANE1_HIGH       0x00213
#define SYMBOL_ERROR_COUNT_LANE2_LOW        0x00214
#define SYMBOL_ERROR_COUNT_LANE2_HIGH       0x00215
#define SYMBOL_ERROR_COUNT_LANE3_LOW        0x00216
#define SYMBOL_ERROR_COUNT_LANE3_HIGH       0x00217

#define LINK_TRAINING_TPS1                  0x01
#define LINK_TRAINING_TPS2                  0x02
#define LINK_TRAINING_TPS3                  0x03
#define LINK_TRAINING_TPS4                  0x07

#define FDP_LINK_BW_SET                     0x0000
#define FDP_LANE_COUNT_SET                  0x0004
#define FDP_ENHANCED_FRAME_EN               0x0008
#define ENHANCED_FRAME_ENABLE               0x1
#define ENHANCED_FRAME_DISABLE              0x0
#define FDP_TRAINING_PATTERN_SET            0x000c
#define TRAINING_OFF                        0x0
#define TRAINING_PATTERN_1                  0x1
#define TRAINING_PATTERN_2                  0x2
#define TRAINING_PATTERN_3                  0x3
#define TRAINING_PATTERN_4                  0x4
#define FDP_LINK_QUAL_PATTERN_SET           0x0010
#define TEST_PATTERN_NONE                   0x0
#define TEST_PATTERN_D10_2                  0x1
#define TEST_PATTERN_SYMBOL_ERROR           0x2
#define TEST_PATTERN_PRBS7                  0x3
#define TEST_PATTERN_80BIT_CUSTOM           0x4
#define TEST_PATTERN_CP2520_1               0x5
#define TEST_PATTERN_CP2520_2               0x6
#define TEST_PATTERN_CP2520_3               0x7
#define TEST_PATTERN_LANE_SHIFT             8
#define FDP_SCRAMBLING_DISABLE              0x0014
#define SCRAMBLING_ENABLE                   0x0
#define SCRAMBLING_DISABLE                  0x1
#define FDP_DOWNSPREAD_CTRL                 0x0018
#define FDP_ALT_SCRAMBLER_RESET             0x001c
#define FDP_HBR2_SCRAMBLER_RESET            0x0020
#define FDP_DISPLAYPORT_VERSION             0x0024
#define FDP_CUSTOM_80BIT_PATTERN_0          0x0030
#define FDP_CUSTOM_80BIT_PATTERN_1          0x0034
#define FDP_CUSTOM_80BIT_PATTERN_2          0x0038
#define FDP_TRANSMITTER_OUTPUT_ENABLE       0x0080
#define TRANSMITTER_OUTPUT_ENABLE           BIT(0)
#define TRANSMITTER_OUTPUT_DISABLE          0
#define FDP_VIDEO_STREAM_ENABLE             0x0084
#define SST_MST_SOURCE_0_ENABLE             BIT(0)
#define SST_MST_SOURCE_0_ENABLE_MASK        0x1
#define SST_MST_SOURCE_0_DISABLE            0
#define FDP_SECONDARY_STREAM_ENABLE         0x0088
#define SECONDARY_STREAM_ENABLE             0x1
#define SECONDARY_STREAM_DISABLE            0x0
#define FDP_SEC_DATA_WINDOW                 0x008C
#define FDP_SOFT_RESET                      0x0090
#define LINK_SOFT_RESET                     (0x1 << 0)
#define VIDEO_SOFT_RESET                    (0x1 << 1)
#define FDP_INPUT_SOURCE_ENABLE             0x0094
#define VIRTUAL_SOURCE_0_ENABLE             BIT(0)
#define VIRTUAL_SOURCE_0_ENABLE_MASK        0x1
#define FDP_FORCE_SCRAMBLER_RESET           0x00C0
#define SCRAMBLER_RESET                     BIT(0)
#define FDP_SOURCE_CONTROL_STATUS           0x00C4
#define FDP_DATA_CONTROL                    0x00C8
#define FDP_CORE_CAPABILITY                 0x00F8
#define FDP_CORE_ID                         0x00FC
#define FDP_AUX_COMMAND                     0x0100
#define BYTE_COUNT_MASK                     0xf
#define COMMAND_SHIFT                       8
#define COMMAND_MASK                        0xf
#define ADDRESS_ONLY                        (1 << 12)
#define FDP_AUX_WRITE_FIFO                  0x0104
#define FDP_AUX_ADDRESS                     0x0108
#define FDP_AUX_CLK_DIVIDER                 0x010C
#define AUX_CLK_DIVIDER                     48
#define AUX_CLK_DIVIDER_100                 100
#define FDP_SINK_HPD_STATE                  0x0128
#define HPD_CONNECT                         0x1
#define HPD_DISCONNECT                      0x0
#define FDP_INTERRUPT_RAW_STATUS            0x0130
#define REPLY_TIMEOUT                       (1 << 3)
#define DP_STATUS_REQUEST_IN_PROGRESS       (1 << 1)
#define HPD_STATE                           (0 << 1)
#define FDP_HPD_STATE_RESET                 0x900
#define FDP_AUX_REPLY_DATA                  0x0134
#define FDP_AUX_REPLY_CODE                  0x0138
#define AUX_NATIVE_ACK                      (0x0 << 0)
#define AUX_NATIVE_NACK                     (0x1 << 0)
#define AUX_NATIVE_DEFER                    (0x2 << 0)
#define AUX_NATIVE_MASK                     (0x3 << 0)
#define AUX_I2C_ACK                         (0x0 << 2)
#define AUX_I2C_NACK                        (0x1 << 2)
#define AUX_I2C_DEFER                       (0x2 << 2)
#define AUX_I2C_MASK                        (0x3 << 2)
#define PHYTIUM_DP_INTERRUPT_STATUS         0x0140
#define HPD_IRQ                             (1 << 1)
#define HPD_EVENT                           (1 << 0)
#define FDP_INTERRUPT_MASK                  0x0144
#define HPD_IRQ_MASK                        (1 << 1)
#define HPD_EVENT_MASK                      (1 << 0)
#define HPD_OTHER_MASK                      0x3c
#define FDP_TX_AUX_ERROR_MASK               BIT(6) // AUX reply error
#define FDP_TX_GP_TIME_MASK                 BIT(4) // 通用定时器中断
#define FDP_TX_AUX_TIMEOUT_MASK             BIT(3) // 因等待 AUX reply 超时发起中断
#define FDP_TX_AUX_RECEIVED_MASK            BIT(2) // 接收到 AUX reply 发起中断
#define FDP_TX_HPD_INTR_MASK                BIT(1) // hpd irq 中断
#define FDP_TX_HPD_EVENT_MASK               BIT(0) // HPD 连接或断开事件 中断

#define FDP_TX_STATUS_AUX_ERROR             BIT(6)
#define FDP_TX_STATUS_GP_TIME               BIT(4)
#define FDP_TX_STATUS_AUX_TIMEOUT           BIT(3)
#define FDP_TX_STATUS_AUX_RECEIVED          BIT(2)
#define FDP_TX_STATUS_HPD_INTR              BIT(1)
#define FDP_TX_STATUS_HPD_EVENT             BIT(0)
#define FDP_AUX_REPLY_DATA_COUNT            0x0148
#define FDP_AUX_STATUS                      0x014C
#define REPLY_RECEIVED                      0x1
#define REPLY_IN_PROGRESS                   0x2
#define REQUEST_IN_PROGRESS                 0x4
#define REPLY_ERROR                         0x8
#define FDP_AUX_TIMER                       0x0158
#define FDP_MAIN_LINK_HTOTAL                0x0180
#define FDP_MAIN_LINK_VTOTAL                0x0184
#define FDP_MAIN_LINK_POLARITY              0x0188
#define PHYYNC_POLARITY_LOW                 BIT(1)
#define HSYNC_POLARITY_LOW                  BIT(0)
#define FDP_MAIN_LINK_HSWIDTH               0x018C
#define FDP_MAIN_LINK_VSWIDTH               0x0190
#define FDP_MAIN_LINK_HRES                  0x0194
#define FDP_MAIN_LINK_VRES                  0x0198
#define FDP_MAIN_LINK_HSTART                0x019C
#define FDP_MAIN_LINK_VSTART                0x01A0
#define FDP_MAIN_LINK_MISC0                 0x01A4
#define MISC0_SYNCHRONOUS_CLOCK             BIT(0)
#define MISC0_BIT_DEPTH_OFFSET              5
#define MISC0_BIT_DEPTH_6BIT                0x0
#define MISC0_BIT_DEPTH_8BIT                0x1
#define MISC0_BIT_DEPTH_10BIT               0x2
#define MISC0_COMPONENT_FORMAT_SHIFT        1
#define MISC0_COMPONENT_FORMAT_RGB          0x0
#define MISC0_COLOR_FORMAT                  GENMASK(2, 1)
#define MISC0_BIT_DEPTH                     GENMASK(7, 5)
#define FDP_MAIN_LINK_MISC1                 0x01A8
#define FDP_M_VID                           0x01AC
#define FDP_TRANSFER_UNIT_SIZE              0x01B0
#define FDP_N_VID                           0x01B4
#define FDP_USER_PIXEL_WIDTH                0x01B8
#define FDP_DATA_COUNT                      0x01BC
#define FDP_INTERLACED                      0x01C0
#define FDP_USER_SYNC_POLARITY              0x01C4
#define USER_ODDEVEN_POLARITY_HIGH          BIT(3)
#define USER_DATA_ENABLE_POLARITY_HIGH      BIT(2)
#define USER_PHYYNC_POLARITY_HIGH           BIT(1)
#define USER_HSYNC_POLARITY_HIGH            BIT(0)
#define FDP_USER_CONTROL                    0x01C8
#define FEDP_CRC_ENABLE                     0x01D0
#define ENABLE_CRC                          BIT(0)
#define SUPPORT_EDP_1_4                     BIT(1)
#define FEDP_CRC_RED                        0x01D4
#define FEDP_CRC_GREEN                      0x01D8
#define FEDP_CRC_BLUE                       0x01DC
#define FDP_SEC_AUDIO_ENABLE                0x0300
#define SEC_AUDIO_ENABLE                    BIT(0)
#define CHANNEL_MUTE_ENABLE                 BIT(1)
#define FDP_SEC_INPUT_SELECT                0x0304
#define INPUT_SELECT_I2S                    0x0
#define FDP_SEC_CHANNEL_COUNT               0x0308
#define CHANNEL_2                           0x2
#define CHANNEL_2_LFE                       0x3
#define CHANNEL_5_1                         0x6
#define CHANNEL_7_1                         0x7
#define CHANNEL_MASK                        0xf
#define FDP_SEC_DIRECT_CLKDIV               0x030c
#define APB_CLOCK                           48000000
#define FDP_SEC_MAUD                        0x0318
#define FDP_SEC_NAUD                        0x031c
#define FDP_SEC_CLOCK_MODE                  0x0320
#define CLOCK_MODE_SYNC                     0x1
#define FDP_SEC_CS_SOURCE_FORMAT            0x0340
#define CS_SOURCE_FORMAT_DEFAULT            0x0
#define FDP_SEC_CS_CATEGORY_CODE            0x0344
#define FDP_SEC_CS_LENGTH_ORIG_FREQ         0x0348
#define FDP_AUDIO_INFOFRAME_SELECT          0x334

#define AUDIO_INFOFRAME_VENDOR_SPECIFIC     0x0
#define AUDIO_INFOFRAME_AUX_VIDEO           0x1
#define AUDIO_INFOFRAME_SOURCE_PRODUCT      0x2
#define AUDIO_INFOFRAME_AUDIO_DESCRIPTION   0x3
#define AUDIO_INFOFRAME_NTSC_VBI            0x4
#define SET_POWER_SET_DP_PWR_VALTAGE        0x00600

#define FDP_AUDIO_CLOCK_MODE_MASK           BIT(0)
#define FDP_TX_ENABLE_SEC_STREAM_MASK       BIT(0)
#define ORIG_FREQ_32000                     0xc
#define ORIG_FREQ_44100                     0xf
#define ORIG_FREQ_48000                     0xd
#define ORIG_FREQ_88200                     0x7
#define ORIG_FREQ_96000                     0x5
#define ORIG_FREQ_176400                    0x3
#define ORIG_FREQ_192000                    0x1
#define ORIG_FREQ_MASK                      0xf
#define ORIG_FREQ_SHIFT                     0
#define WORD_LENGTH_16                      0x4
#define WORD_LENGTH_18                      0x2
#define WORD_LENGTH_20                      0xc
#define WORD_LENGTH_24                      0xd
#define WORD_LENGTH_MASK                    0xf
#define WORD_LENGTH_SHIFT                   4
#define FDP_SEC_CS_FREQ_CLOCK_ACCURACY      0x034c // not used
#define SAMPLING_FREQ_32000                 0xc
#define SAMPLING_FREQ_44100                 0x0
#define SAMPLING_FREQ_48000                 0x4
#define SAMPLING_FREQ_88200                 0x1
#define SAMPLING_FREQ_96000                 0x5
#define SAMPLING_FREQ_176400                0x3
#define SAMPLING_FREQ_192000                0x7
#define SAMPLING_FREQ_MASK                  0xf
#define SAMPLING_FREQ_SHIFT                 4
#define FDP_SEC_CHANNEL_MAP                 0x035C
#define CHANNEL_MAP_DEFAULT                 0x87654321
#define FDP_IDLE_PATTERN_DISBALE            0x910
#define DISABLE                             0x1
#define ENABLE                              0x0
#define FDP_DP_CONTROLLER_RESET             0x850
#define DP_I2S_RESET                        0x1
#define DP_RESET_RELEASE                    0x7

/******************************dp register end********************************************/

/******************************edp pwm backlight register start**************************/
#define EDP_BACKLIGHT_CONTROL               0x904
#define EDP_BL_PWR_ON                       BIT(0)
#define EDP_BL_PWR_OFF_MASK                 (1 << 0)
#define EDP_BL_ENABLE                       BIT(1)
#define EDP_BL_DISABLE_MASK                 (1 << 1)
#define PWM_TIM_CTRL                        0x04
#define PWM_ENABLE_MASK                     0x2
#define PWM_DIV_MASK                        0xfff0000
#define PWM_CTRL                            0x10
#define PWM_IE_MASK                         0x8
#define PWM_MODE_MASK                       0x4
#define PWM_DUTY_SEL_MASK                   0x100
#define PWM_PERIOD                          0x0C
#define PWM_PERIOD_MASK                     0xffff
#define PWM_CCR                             0x14
#define PWM_DUTY_MASK                       0xffff
#define PWM_STATE                           0x08

#define SINK_COUNT                          0x00200
/* FRAMEBUFFER_CONFIG0 */
#define FRAMEBUFFER_CONFIG0_FORMAT_SET(x) \
    SET_REG32_BITS((x), 31, 26) /* framebuffer configuration register  can see the struct in the VIDEO_FORMAT*/
#define FRAMEBUFFER_CONFIG0_UV_SWIZZLE BIT(25) /* UV swizzle */
#define FRAMEBUFFER_CONFIG0_SWIZZLE_SET(x) \
    SET_REG32_BITS((x), 24, 23) /* swizzle mode ,0:ARGB, 1:RGBA, 2:ABGR, 3:BGRA */
#define FRAMEBUFFER_CONFIG0_TILE_MODE_SET(x) \
    SET_REG32_BITS((x), 21, 17) /* set the tilemode */
#define FRAMEBUFFER_CONFIG0_YUV_SET(x) \
    SET_REG32_BITS((x), 16, 14) /* yuv standard, 1: 709*bt709, 3:2020*bt2020*/
#define FRAMEBUFFER_CONFIG0_ROT_ANGLE_SET(x) \
    SET_REG32_BITS((x), 13, 11) /* the angle of the rot, 0:roto, 1:flip_x, 2:flip_y, 3:flip_xy, 5:rot180 */
#define FRAMEBUFFER_CONFIG0_TRANSPARENCY_SET(x) \
    SET_REG32_BITS((x), 10, 9) /* transparency of framebuffer */
#define FRAMEBUFFER_CONFIG0_UNDERFLOW                  BIT(5)
/******************************edp pwm backlightregister end****************************/

/******************************edp psr start******************************/
#define FDP_SEC_PSR_3D_ENABLE                          0x0360
#define PSR_ENABLE                                     BIT(0)
#define FDP_SEC_PSR_CONFIG                             0x364
#define PSR_DISABLE_CAPTURE                            BIT(0)
#define PSR_TWO_FRAMES_TO_ACTIVE                       BIT(1)
#define PSR_ONE_FRAMES_TO_ACTIVE                       (0 << 1)
#define INACTIVE_NO_EXIT                               BIT(2)
#define ENABLE_Y_COORD                                 BIT(3)
#define FDP_SEC_PSR_STATE                              0x0368
#define PSR_ACTIVE                                     BIT(0)
#define PSR_INACTIVE                                   0
#define PSR_SINGLE_FRAME_UPDATE                        0x3
#define SEC_PSR_STATE_INTERNAL                         0x036c
#define PSR_STATE_DISABLE                              BIT(0)
#define PSR_STATE_INACTIVE                             BIT(1)
#define PSR_STATE_TRANS_TO_ACTIVE                      BIT(2)
#define PSR_STATE_ACTIVE                               BIT(3)
#define PSR_STATE_ACTIVE_UPDATE                        BIT(4)
#define PSR_STATE_EXIT                                 BIT(5)
/******************************edp psr end******************************/

/******************************dp dplp start******************************/
#define FDPLP_HPD_PWEUP_INIT_MASK                      0X0
#define DPLP_HPD_CONNECT_MASK                          BIT(0)
#define DPLP_HPD_DISCONNECT_MASK                       BIT(1)
#define DPLP_HPD_IRQ_MASK                              BIT(2)
#define FDPLP_HPD_PWRUP_INT_CLEAR                      0X0c
#define CLEAN_HPD_CONNECT                              BIT(0)
#define CLEAN_HPD_DISCONNECT                           BIT(1)
#define CLEAN_HPD_IRQ                                  BIT(2)
#define FDPLP_FRAME_COMP_CFG                           0x20
#define FRAME_COMPARE_EN                               BIT(0)
#define LOWFPS_FUNC_EN                                 BIT(1)
#define PSR_FUNC_EN                                    BIT(2)
#define AUTO_LOWFPS_EN                                 BIT(3)
#define FDPLP_LOWFPS_CNT                               0x24
#define FDPLP_PSR_CNT                                  0x28
#define FDPLP_FRAME_COMP_INIT_MASK                     0x2c
#define LOWFPS_AVAILABLE_MASK                          BIT(0)
#define LOWFPS_EXIT_MASK                               BIT(3)
#define LOW_FPS_ENABLE_MASK                            0x3f6
#define PSR_AVAILABLE_MASK                             0x3fd
#define PSR_SHIELD_MASK                                0x272
#define ALL_SHIELD_MAKS                                0x3ff
#define FDPLP_FRAME_COMP_INIT_STATE                    0x34
#define LOWFPS_AVAILABLE                               BIT(0)
#define PSR_AVAILABLE                                  BIT(1)
#define LOWFPS_EXIT                                    BIT(3)
#define FRAME_CHANGE_IN_PSR                            BIT(6)
#define FRAME_CHANGE_IN_WAIT_LOWFPS                    BIT(8)
#define FDPLP_FRAME_COMP_INIT_CLEAR                    0x38
#define FDPLP_FRAME_COMP_STATE                         0x3c
#define DPLP_LOWFPS_AVAILABLE                          BIT(0)
#define DPLP_PSR_AVAILABLE                             BIT(1)
#define DPLP_LOWFPS_ACTIVE                             BIT(2)
#define DPLP_PSR_ACTIVE                                BIT(3)
#define FDPLP_FRAME_COMP_REQ                           0x44
#define ENTER_LOWFPS_REQ                               BIT(0)
#define EXIT_LOWFPS_REQ                                BIT(1)
#define ENTER_PSR_REQ                                  BIT(2)
#define EXIT_PSR_REQ                                   BIT(3)
#define FDPLP_HPD_STATE_RESET                          0x4c
/******************************dplp end******************************/
/* phy register start */
#define FDP_PHY_MODE_DPX4                              0x01
#define FDP_PHY_MODE_DP_LANE23                         0x02 // pciex2+dpx2
#define FDP_PHY_MODE_DP_LANE01                         0x03 // dpx2+satax2

#define FDP_PHY_PIPE_SEL                               0x40000
#define SEL_32BIT                                      0xf
#define FDP_PHY_LANE_BUFFER                            0x40004
#define LANE_BUFFER_INIT                               0x0
#define FDP_PHY_PIPE_RESET                             0x40094
#define RESET                                          0x0
#define RESET_DEASSERT                                 0x1

#define FDP_PHY_MODE                                   0x40084
#define LANE_BIT                                       (0x3)
#define LANE_BIT_SHIFT                                 0x2

#define FDP_PHY_LINK_CFG                               0x40080
#define LANE_MASTER                                    0x1
#define LANE_MASTER_SHIFT                              1
#define LANE_SLAVE                                     0x0

#define FDP_PHY_PLL_EN                                 0x4000C
#define PLL_EN                                         0x1
#define PLL_EN_SHIFT                                   1

#define FDP_PHY_PMA_WIDTH                              0x4001c
#define BIT_20                                         0x5
#define BIT_20_SHIFT                                   4

#define FDP_PHY_PMA0_POWER                             0x40014
#define A0_ACTIVE                                      0x1
#define A0_ACTIVE_SHIFT                                8
#define A3_POWERDOWN3                                  0x8
#define A3_POWERDOWN3_SHIFT                            8

#define FDP_PHY_LINK_RESET                             0x40098
#define LINK_RESET                                     0x1
#define LINK_RESET_MASK                                0x1
#define LINTK_RESET_SHIFT                              0x1

#define FDP_PHY_APB_RESET                              0x40090
#define PHY_APB_RESET                                  0x1

/* phy origin register */
#define FDP_PHY0_PLL_CFG                               0x30038
#define SINGLE_LINK                                    0x0
#define DOUBLE_LINK                                    0x2

#define FDP_PHY0_PMA_CONTROL                           0x3800c
#define CONTROL_ENABLE                                 0x1
#define PLL1_CONTROL_ENABLE                            0x2
#define CONTROL_ENABLE_MASK                            0x1
#define CONTROL_ENABLE_SHIFT                           0x1

#define FDP_PHY0_PMA_CONTROL2                          0x38004
#define PLL0_LOCK_DONE                                 (0x1 << 6)
#define PLL1_LOCK_DONE                                 (0x1 << 7)

#define FDP_PHY0_PLL0_CLK_SEL                          0x684
#define FDP_PHY0_PLL1_CLK_SEL                          0x704
#define PLL_LINK_RATE_162000                           0xf01
#define PLL_LINK_RATE_270000                           0x701
#define PLL_LINK_RATE_540000                           0x301
#define PLL_LINK_RATE_810000                           0x200

#define FDP_PHY0_HSCLK0_SEL                            0x18398
#define FDP_PHY0_HSCLK1_SEL                            0x19398
#define HSCLK_LINK_0                                   0x0
#define HSCLK_LINK_1                                   0x1

#define FDP_PHY0_HSCLK0_DIV                            0x1839c
#define FDP_PHY0_HSCLK1_DIV                            0x1939c
#define HSCLK_LINK_RATE_162000                         0x2
#define HSCLK_LINK_RATE_270000                         0x1
#define HSCLK_LINK_RATE_540000                         0x0
#define HSCLK_LINK_RATE_810000                         0x0

#define FDP_PHY0_PLLDRC0_CTRL                          0x18394
#define FDP_PHY0_PLLDRC1_CTRL                          0x19394
#define PLLDRC_LINK0                                   0x1
#define PLLDRC_LINK1                                   0x9

#define FDP_PHY0_PLL0_DSM_M0                           0x250
#define PLL0_DSM_M0                                    0x4
#define FDP_PHY0_PLL1_DSM_M0                           0x350
#define PLL1_DSM_M0                                    0x4

#define FDP_PHY0_PLL0_VCOCAL_START                     0x218
#define PLL0_VCOCAL_START                              0xc5e
#define FDP_PHY0_PLL1_VCOCAL_START                     0X318
#define PLL1_VCOCAL_START                              0xc5e

#define FDP_PHY0_PLL0_VCOCAL_CTRL                      0x208
#define PLL0_VCOCAL_CTRL                               0x3
#define FDP_PHY0_PLL1_VCOCAL_CTRL                      0x308
#define PLL1_VCOCAL_CTRL                               0x3

#define FDP_PHY0_PLL0_CP_PADJ                          0x690
#define FDP_PHY0_PLL0_CP_IADJ                          0x694
#define FDP_PHY0_PLL0_CP_FILT_PADJ                     0x698
#define FDP_PHY0_PLL0_INTDIV                           0x240
#define FDP_PHY0_PLL0_FRACDIVL                         0x244
#define FDP_PHY0_PLL0_FRACDIVH                         0x248
#define FDP_PHY0_PLL0_HIGH_THR                         0x24c
#define FDP_PHY0_PLL0_PDIAG_CTRL                       0x680
#define FDP_PHY0_PLL0_VCOCAL_PLLCNT_START              0x220
#define FDP_PHY0_PLL0_LOCK_PEFCNT                      0x270
#define FDP_PHY0_PLL0_LOCK_PLLCNT_START                0x278
#define FDP_PHY0_PLL0_LOCK_PLLCNT_THR                  0x27c

#define FDP_PHY0_PLL1_CP_PADJ                          0x710
#define FDP_PHY0_PLL1_CP_IADJ                          0x714
#define FDP_PHY0_PLL1_CP_FILT_PADJ                     0x718
#define FDP_PHY0_PLL1_INTDIV                           0x340
#define FDP_PHY0_PLL1_FRACDIVL                         0x344
#define FDP_PHY0_PLL1_FRACDIVH                         0x348
#define FDP_PHY0_PLL1_HIGH_THR                         0x34c
#define FDP_PHY0_PLL1_PDIAG_CTRL                       0x700
#define FDP_PHY0_PLL1_VCOCAL_PLLCNT_START              0x320
#define FDP_PHY0_PLL1_LOCK_PEFCNT                      0x370
#define FDP_PHY0_PLL1_LOCK_PLLCNT_START                0x378
#define FDP_PHY0_PLL1_LOCK_PLLCNT_THR                  0x37c

#define FDP_PHY0_PLL0_TX_PSC_A0                        0x18400
#define PLL0_TX_PSC_A0                                 0x2fb
#define FDP_PHY0_PLL0_TX_PSC_A1                        0x18404
#define PLL0_TX_PSC_A1                                 0x6ab
#define FDP_PHY0_PLL0_TX_PSC_A2                        0x18408
#define PLL0_TX_PSC_A2                                 0x6aa
#define FDP_PHY0_PLL0_TX_PSC_A3                        0x1840c
#define PLL0_TX_PSC_A3                                 0x6aa
#define FDP_PHY0_PLL0_RX_PSC_A0                        0x28000
#define PLL0_RX_PSC_A0                                 0xd1d
#define FDP_PHY0_PLL0_RX_PSC_A1                        0x28004
#define PLL0_RX_PSC_A1                                 0xd1d
#define FDP_PHY0_PLL0_RX_PSC_A2                        0x28008
#define PLL0_RX_PSC_A2                                 0xd00
#define FDP_PHY0_PLL0_RX_PSC_A3                        0x2800c
#define PLL0_RX_PSC_A3                                 0x500
#define FDP_PHY0_PLL0_RX_PSC_CAL                       0x28018
#define PLL0_RX_PSC_CAL                                0x0

#define FDP_PHY0_PLL1_TX_PSC_A0                        0x19400
#define PLL1_TX_PSC_A0                                 0x2fb
#define FDP_PHY0_PLL1_TX_PSC_A1                        0x19404
#define PLL1_TX_PSC_A1                                 0x6ab
#define FDP_PHY0_PLL1_TX_PSC_A2                        0x19408
#define PLL1_TX_PSC_A2                                 0x6aa
#define FDP_PHY0_PLL1_TX_PSC_A3                        0x1940c
#define PLL1_TX_PSC_A3                                 0x6aa
#define FDP_PHY0_PLL1_RX_PSC_A0                        0x29000
#define PLL1_RX_PSC_A0                                 0xd1d
#define FDP_PHY0_PLL1_RX_PSC_A1                        0x29004
#define PLL1_RX_PSC_A1                                 0xd1d
#define FDP_PHY0_PLL1_RX_PSC_A2                        0x29008
#define PLL1_RX_PSC_A2                                 0xd00
#define FDP_PHY0_PLL1_RX_PSC_A3                        0x2900c
#define PLL1_RX_PSC_A3                                 0x500
#define FDP_PHY0_PLL1_RX_PSC_CAL                       0x29018
#define PLL1_RX_PSC_CAL                                0x0

#define FDP_PHY0_PLL0_XCVR_CTRL                        0x183a8
#define PLL0_XCVR_CTRL                                 0xf
#define FDP_PHY0_PLL1_XCVR_CTRL                        0x193a8
#define PLL1_XCVR_CTRL                                 0xf

#define FDP_PHY0_PLL0_RX_GCSM1_CTRL                    0x28420
#define PLL0_RX_GCSM1_CTRL                             0x0

#define FDP_PHY0_PLL1_RX_GCSM1_CTRL                    0x29420
#define PLL1_RX_GCSM1_CTRL                             0x0

#define FDP_PHY0_PLL0_RX_GCSM2_CTRL                    0x28440
#define PLL0_RX_GCSM2_CTRL                             0x0
#define FDP_PHY0_PLL1_RX_GCSM2_CTRL                    0x29440
#define PLL1_RX_GCSM2_CTRL                             0x0

#define FDP_PHY0_PLL0_RX_PERGCSM_CTRL                  0x28460
#define PLL0_RX_PERGCSM_CTRL                           0x0
#define FDP_PHY0_PLL1_RX_PERGCSM_CTRL                  0x29460
#define PLL1_RX_PERGCSM_CTRL                           0x0

/* swing and emphasis */
#define FDP_PHY0_PLL0_TX_DIAG_ACYA                     0x1879c
#define FDP_PHY0_PLL1_TX_DIAG_ACYA                     0x1979c
#define LOCK                                           1
#define UNLOCK                                         0

#define FDP_PHY0_PLL0_TX_TXCC_CTRL                     0x18100
#define FDP_PHY0_PLL1_TX_TXCC_CTRL                     0x19100
#define TX_TXCC_CTRL                                   0x8a4

#define FDP_PHY0_PLL0_TX_DRV                           0x18318
#define FDP_PHY0_PLL1_TX_DRV                           0x19318
#define TX_DRV                                         0x3

#define FDP_PHY0_PLL0_TX_MGNFS                         0x18140
#define FDP_PHY0_PLL1_TX_MGNFS                         0x19140

#define FDP_PHY0_PLL0_TX_CPOST                         0x18130
#define FDP_PHY0_PLL1_TX_CPOST                         0x19130

#define LINK_BW_SET                                    0x00100
#define LANE_COUNT_SET                                 0x00101
#define TRAINING_PATTERN_SET                           0x00102
#define TRAINING_LANE0_SET                             0x00103
#define TRAINING_LANE1_SET                             0x00104
#define TRAINING_LANE2_SET                             0x00105
#define TRAINING_LANE3_SET                             0x00106
#define DOWNSPREAD_CTRL                                0x00107
#define MAIN_LINK_CHANNEL_CODING_SET                   0x00108
#define I2C_SPEED_CONTROL_STATUS                       0x00109
#define EDP_CONFIGURATION_SET                          0x0010A
#define LINK_QUAL_LANE0_SET                            0x0010B
#define LINK_QUAL_LANE1_SET                            0x0010C
#define LINK_QUAL_LANE2_SET                            0x0010D
#define LINK_QUAL_LANE3_SET                            0x0010E
#define MSTM_CTRL                                      0x00111
#define AUDIO_DELAY_7_0                                0x00112
#define AUDIO_DELAY_15_8                               0x00113
#define AUDIO_DELAY_23_16                              0x00114
#define LINK_RATE_SET_TX_GTC_CAPABILITY                0x00115
#define UPSTREAM_DEVICE_DP_PWR_NEED                    0x00118
#define EXTENDED_DPRX_SLEEP_WAKE_TIMEOUT_GRANT         0x00119
#define FEC_CONFIGURATION                              0x00120
#define TX_GTC_VALUE_7_0                               0x00154
#define TX_GTC_VALUE_15_8                              0x00155
#define TX_GTC_VALUE_23_16                             0x00156
#define TX_GTC_VALUE_31_24                             0x00157
#define RX_GTC_VALUE_PHASE_SKEW_EN                     0x00158
#define TX_GTC_FREQ_LOCK_DONE                          0x00159
#define TX_GTC_PHASE_CKEW_OFFERT_7_0                   0x0015A
#define TX_GTC_PHASE_CKEW_OFFERT_15_8                  0x0015B
#define DSC_ENABLE                                     0x00160
#define ADAPTOR_CTRL                                   0x001A0
#define BRANCH_DEVICE_CTRL                             0x001A1
#define PAYLOAD_ALLOCATE_SET                           0x001C0
#define PAYLOAD_ALLOCATE_START_TIME_SLOT               0x001C1
#define PAYLOAD_ALLOCATE_TIME_SLOT_COUNT               0x001C2

#define SINK_COUNT                                     0x00200
#define DEVICE_SERVICE_IRQ_VECTOR                      0x00201
#define LANE0_1_STATUS                                 0x00202
#define LANE2_3_STATUS                                 0x00203
#define LANE_ALIGN_STATUS_UPDATED                      0x00204
#define SINK_STATUS                                    0x00205
#define ADJUST_REQUEST_LANE0_1                         0x00206
#define ADJUST_REQUEST_LANE2_3                         0x00207
#define TRAINING_SCORE_LANE0                           0x00208
#define TRAINING_SCORE_LANE1                           0x00209
#define TRAINING_SCORE_LANE2                           0x0020A
#define TRAINING_SCORE_LANE3                           0x0020B
#define SYMBOL_ERROR_COUNT_LANE0_LOW                   0x00210
#define SYMBOL_ERROR_COUNT_LANE0_HIGH                  0x00211
#define SYMBOL_ERROR_COUNT_LANE1_LOW                   0x00212
#define SYMBOL_ERROR_COUNT_LANE1_HIGH                  0x00213
#define SYMBOL_ERROR_COUNT_LANE2_LOW                   0x00214
#define SYMBOL_ERROR_COUNT_LANE2_HIGH                  0x00215
#define SYMBOL_ERROR_COUNT_LANE3_LOW                   0x00216
#define SYMBOL_ERROR_COUNT_LANE3_HIGH                  0x00217
#define TEST_REQUEST                                   0x00208
#define TEST_LINK_RATE                                 0x00219
#define TEST_LANE_COUNT                                0x00220
#define TEST_PATTERN                                   0x00221
#define TEST_H_TOTAL_HIGH                              0x00222
#define TEST_H_TOTAL_LOW                               0x00223
#define TEST_V_TOTAL_HIGH                              0x00224
#define TEST_V_TOTAL_LOW                               0x00225
#define TEST_H_START_HIGH                              0x00226
#define TEST_H_START_LOW                               0x00227
#define TEST_V_START_HIGH                              0x00228
#define TEST_V_START_LOW                               0x00229
#define TEST_HSYNC_HIGH                                0x0022A
#define TEST_HSYNC_LOW                                 0x0022B
#define TEST_VSYNC_HIGH                                0x0022C
#define TEST_VSYNC_LOW                                 0x0022D
#define TEST_H_WIDTH_HIGH                              0x0022E
#define TEST_H_WIDTH_LOW                               0x0022F
#define TEST_V_HEIGHT_HIGH                             0x00230
#define TEST_V_HEIGHT_LOW                              0x00231
#define TEST_MISC_LOW                                  0x00232
#define TEST_MISC_HIGH                                 0x00233
#define TEST_REFRESH_RATE_NUMERATOR                    0x00234
#define TEST_CRC_R_CR_LOW                              0x00240
#define TEST_CRC_R_CR_HIGH                             0x00241
#define TEST_CRC_G_Y_LOW                               0x00242
#define TEST_CRC_G_Y_HIGH                              0x00243
#define TEST_CRC_B_CB_LOW                              0x00244
#define TEST_CRC_B_CB_HIGH                             0x00245
#define TEST_CRC_COUNT                                 0x00246
#define PHY_TEST_PATTERN                               0x00248
#define HBR2_COMPLIANCE_SCRAMBLER_RESET_7_0            0x0024A
#define HBR2_COMPLIANCE_SCRAMBLER_RESET_15_8           0x0024B
#define TEST_80BIT_CUSTOM_PATTERN_7_0                  0x00250
#define TEST_80BIT_CUSTOM_PATTERN_15_8                 0x00251
#define TEST_80BIT_CUSTOM_PATTERN_23_16                0x00252
#define TEST_80BIT_CUSTOM_PATTERN_31_24                0x00253
#define TEST_80BIT_CUSTOM_PATTERN_39_32                0x00254
#define TEST_80BIT_CUSTOM_PATTERN_47_40                0x00255
#define TEST_80BIT_CUSTOM_PATTERN_55_48                0x00256
#define TEST_80BIT_CUSTOM_PATTERN_63_56                0x00257
#define TEST_80BIT_CUSTOM_PATTERN_71_64                0x00258
#define TEST_80BIT_CUSTOM_PATTERN_79_72                0x00259
#define CONTINUOUS_80BIT_PATTERN_FROM_DPRX_AUX_CH_CAP  0x0025A
#define CONTINUOUS_80BIT_PATTERN_FROM_DPRX_AUX_CH_CTRL 0x0025B
#define TEST_RESPONSE                                  0x00260
#define TEST_EDID_CHECHSUM                             0x00261
#define TEST_SINK                                      0x00270
#define TEST_AUDIO_MODE                                0x00271
#define TEST_AUDIO_PATTERN_TYPE                        0x00272
#define TEST_AUDIO_PERIOD_CH_1                         0x00273
#define TEST_AUDIO_PERIOD_CH_2                         0x00274
#define TEST_AUDIO_PERIOD_CH_3                         0x00275
#define TEST_AUDIO_PERIOD_CH_4                         0x00276
#define TEST_AUDIO_PERIOD_CH_5                         0x00277
#define TEST_AUDIO_PERIOD_CH_6                         0x00278
#define TEST_AUDIO_PERIOD_CH_7                         0x00279
#define TEST_AUDIO_PERIOD_CH_8                         0x0027A
#define FEC_STATUS                                     0x00280
#define FEC_ERROR_COUNT0                               0x00281
#define FEC_ERROR_COUNT1                               0x00282
#define PAYLOAD_TABLE_UPDATE_STATUS                    0x002C0

#define IEEE_OUI                                       0x00300
#define DEVICE_ID_STRING                               0x00303
#define HARDWARE_REVISION                              0x00309
#define FIRMWARE_SOFTWARE_MAJOR_REV                    0x0030A
#define FIRMWARE_SOFTWARE_MINOR_REV                    0x0030B

#define SET_POWER_SET_DP_PWR_VALTAGE                   0x00600

#define SINK_COUNT_ESI                                 0x02002
#define DEVICE_SERVICE_IRQ_VECTOR_ESI0                 0x02003
#define DEVICE_SERVICE_IRQ_VECTOR_ESI1                 0x02004
#define LINK_SERVICE_IRQ_VECTOR_ESI0                   0x02005
#define LANE0_1_STATUS_ESI                             0x0200C
#define LANE2_3_STATUS_ESI                             0x0200D
#define LANE_ALIGN_STATUS_UPDATED_ESI                  0x0200E
#define SINK_STATUS_ESI                                0x0200F

#define LINK_TRAINING_TPS1                             0x01
#define LINK_TRAINING_TPS2                             0x02
#define LINK_TRAINING_TPS3                             0x03
#define LINK_TRAINING_TPS4                             0x07

#ifdef __cplusplus
}
#endif

#endif /* __FDCDP_REG_H__ */
