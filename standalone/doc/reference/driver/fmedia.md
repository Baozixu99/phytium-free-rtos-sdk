<!--
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
 * FilePath: fmedia.md
 * Date: 2022-12-07 19:57:17
 * LastEditTime: 2022-12-07 19:57:18
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
-->

# 驱动概述

- DC 是一个显示控制器，主要完成将 CPU/GPU/VPU 处理后的图像数据，按照 Display 协议处理后送给 DP PHY 接入显示器。E2000 下 DC 主要有以下特性

1. 支持两路Display,两路Display相互独立(E2000D 只有一路)
2. 支持的 size 有 640×480、800×600、1024×768、1152×864、1280×720、1280×960、1366×768、1440×900、1600×90、1920×1080，最大帧率为 2.7G/(size*32)
3. 支持AHB配置寄存器
4. 支持Hsync,Vsync配置
5. 输出支持像素格式：RGB2101010，RGB888，RGB666，RGB555
6. 输入图像格式：ARGB2101010，A/XRGB8888，A/XRGB1555，A/XRGB4444，RGB565；YUV422(YUY2，UYVY，NV16)；YUV420(YUY2(P010)YV12，NV12，NV21)

当前Media 驱动控制器主要为用户提供了以下功能接口

1. DC,DP 控制器状态初始化函数
2. 中断接口
3. 热插拔检测处理接口
4. 多屏模式处理接口


##  驱动功能

驱动组成由以下所示 :

```
.
├── fdc.c
├── fdc.h
├── fdc_hw.c
├── fdc_hw.h
├── fdc_hw.h
├── fdcdp_aux.c
├── fdcdp_aux.h
├── fdcdp_crtc.c
├── fdcdp_crtc.h
├── fdcdp_edid.c
├── fdcdp_edid.h
├── fdcdp_g.c
├── fdcdp_intr.c
├── fdcdp_reg.h
├── fdcdp_sinit.h
├── fdcdp.c
├── fdcdp.h
├── fdp.c
├── fdp.h
├── fdp_hw.c
├── fdp_hw.h

```

其中fdcdp.h 为用户开发者主要使用接口，提供了以下功能:
1. DC,DP 控制器控制器初始化接口
2. DC,DP 控制器相关状态位的回调函数注册
3. DC,DP 控制器中断处理函数
4. DP热插拔检测处理函数


## 数据结构
```c
typedef struct
{
    /* fdc instace object */
    FDcCtrl dc_instance_p[FDC_INSTANCE_NUM];
    /* fdp instace object */
    FDpCtrl dp_instance_p[FDP_INSTANCE_NUM];
    /*the intr config of dcdp*/
    FDcDpIntrConfig intr_event[FDCDP_INTR_MAX_NUM];
    u32 is_ready[FDP_INSTANCE_NUM];
    u32 connect_flg[FDP_INSTANCE_NUM];
    u32 is_initialized[FDP_INSTANCE_NUM];
    /*clone, horizontal and vertical display*/
    u32 multi_mode; 
} FDcDp;

其中FDcCtrl结构体为dc模块相关变量，包含有时序，frambuffer，pannel,gamma,dither,cursor等等；FDpCtrl结构体为dp模块相关变量，包括有时序，传输参数，phy等等。

```c
dc 相关结构体定义
typedef struct
{
    FDcDisplayVideoMode display_mode;
    FDcCrtc    crtc;
    FDcTimming timming;
    FDcConfig config;
    u32 channel;
	  uintptr fb_addr;
    uintptr fb_virtual;
    u32 multi_mode; /* The display mode of the device , including clone, horizontal and vertical display*/
	u32 timing_id;
	u32 pixel_clock;
  u32 color_depth;
} FDcCtrl;

typedef struct
{
  u32 instance_id;         /* dc id */
  uintptr dcch_base_addr;   /* DC channel register address*/
  uintptr dcctrl_base_addr; /* DC control register address */
  u32 irq_num;             /* Device interrupt id */
} FDcConfig;

dp 相关结构体

typedef struct
{
    FDpAuxConfig    aux;
    FDpConfig config;
	  FDpTransConfig trans_config;
	  FDpTimming timming;
    u32 channel;
    u32 multi_mode; /* The display mode of the device , including clone, horizontal and vertical display*/
	  u32 port; /* The port of the device */
} FDpCtrl;

typedef struct
{
  u32 instance_id;
  uintptr dp_channe_base_addr;
  uintptr dp_phy_base_addr;
  u32 irq_num;
} FDpConfig;


中断 相关结构体

typedef enum
{
    FDCDP_HPD_IRQ_CONNECTED = 0, /* dp disconnected */
    FDCDP_HPD_IRQ_DISCONNECTED, /*the dp disconnected*/
    FDCDP_AUX_REPLY_TIMEOUT, /*receive aux reply timeout*/
    FDCDP_AUX_REPLY_ERROR, /*the aux reply is invalid*/

    FDCDP_INTR_MAX_NUM
} FDcDpIntrEventType;

typedef void (*FDCDPIntrHandler)(void *param, u32 index);

typedef struct 
{
  FDcDpIntrEventType;  /* data */
  FDcDpIntrHandler   handler;
  void *param;
} FDcDpIntrConfig;
```

## 错误码定义
```
FDP_PHY_INIT_ERR					/*获取默认参数失败 */
FDC_CRTC_SCAL_ERR							/* 时钟设置错误 */
FDC_PIXEL_ERR                 /* 像素格式设置错误 */

```
## 关键配置参数介绍

当您在使用显示驱动时，有必要将显示系统中的关键参数为您介绍：

- resolution 分辨率

显示器能够呈现的最基本的单元是像素，显示器的显示区域正是被像素点所填充，显示器的分辨率就代表了显示器所能呈现的最大像素点，通常使用较多的是1024*768，1920*1080几种。

- color depth 色深

色彩深度是用「n位元颜色」(n-bit colour)来作为单位的。若色彩深度是n位元,即有2的n次方种颜色选择,而储存每像素所用的位数目就是n，通常您见到最多的一般是16bit或32bit

- fresh rate 刷新率

显示器每秒刷新画面的次数；单位是Hz；例如60Hz,120Hz

- pixel clock 像素时钟

像素时钟 = 实际的水平像素*垂直像素*刷新率（像素数包括消隐数据）

比如 {800 , 525 , 1 , 1 , 96 , 2 , 640 , 480 , 144 , 35 , 0 , 0 } 640*480@60Hz

像素时钟 = 800*525*60 = 25.2 Mhz Vesa标准采用25.175

## 应用例程
``
[media_test](../../../example/media/media_test/)
[lvgl_demo_test](../../../example/media/lvgl_demo_test/)

 其中第一个例程只是点亮屏幕；第二个例程可与lvgl库进行连接，测试

## API 介绍

### 1. FDcConfig && FDpConfig

```
FDcConfig *FDcLookupConfig(u32 instance_id)
FDpConfig *FDpLookupConfig(u32 instance_id)
```
#### 介绍

- 获取当前FDC驱动默认配置

- 获取当前FDP驱动默认配置
#### 参数
- u32 instance_id :当前media驱动中对应的ID

#### 返回
FDcConfig * :dc静态默认配置
FDpConfig * :dp静态默认配置


### 2. FDcDpInitial

```
FError FDcDpInitial(FDcDp *instance, u8 channel, u32 width, u32 height)
```
#### 介绍
- 初始化DC,DP，设置参数，包括dc,dp的初始化和连接

#### 参数
- FDcDp *instance_p  FDcDp 控制器实例的指针
- u8 channel DP通道号
- u32 width 显示宽度
- u32 height 显示高度

#### 返回
- FError :FDP_SUCCESS 为初始成功


### 3. FDcDpRegisterHandler

```
void FDcDpRegisterHandler(FDcDp *instance_p, FDcDpIntrEventType type,FDCDPIntrHandler  handler,void *param)
```
#### 介绍

- 中断注册函数

#### 参数
- FDcDp *instance_p  FDcDp 控制器实例的指针
- FDcDpIntrEventType type 中断事件
- FDCDPIntrHandler  中断事件响应函数
- void *param        回调函数参数

### 4. FDcDpInterruptHandler

```
void FDcDpInterruptHandler(s32 vector, void *args);
```

#### 介绍
-中断handle处理函数

#### 参数
```
s32 vector 中断返回值
void *irq_args 回调结构体指针
```
#### 返回
- NULL

### 5. FDcDpIrqEnable

```
void FDcDpIrqEnable(FDcDp *instance_p,u32 index, FDcDpIntrEventType intr_event_p);
```

#### 介绍
- 中断使能函数

#### 参数
```
FDcDp *instance_p dcdp驱动实例
u32 index   通道序号
FDcDpIntrConfig intr_event_p 中断类型
```
#### 返回
- NULL

