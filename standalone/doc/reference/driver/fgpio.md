# FGPIO 驱动程序

## 1. 概述

- GPIO(General-purpose input/output)，即通用型输入输出，其引脚可以供使用者通过程序控制其输入、输出，常用于产生时钟，作为片选信号和模拟低速通信协议等场景

- FT2000/4和D2000提供两个 GPIO 模块，每个 GPIO 模块有 16 位接口，每 8 位一组，分别是A组和B组，GPIO可以控制外部IO的输入输出方向，作为输出时，内部寄存器的数据输出到片外，作为输入时，片外的数据被锁存在内部寄存器

- E2000和Phytium PI提供六个 GPIO 模块，每个模块有 16 个接口，0 ~ 2 号 GPIO 模块中每个引脚独立处理中断，3 ~ 5 号 GPIO 模块中各引脚的中断合并上报

- FGPIO 驱动支持配置 GPIO 引脚的输入输出方向，输出高低电平，或者获取输入电平，配置引脚的中断触发模式，配置引脚的中断响应回调函数等

## 2. 功能

- FGPIO 驱动程序主要完成GPIO相关的功能配置，包括

- 1. GPIO 引脚初始化
- 2. GPIO 引脚输入输出方向设置
- 3. GPIO 引脚输出和输入
- 4. GPIO 引脚中断使能和屏蔽
- 5. GPIO 引脚中断类型设置
- 6. GPIO 引脚中断回调函数注册

- 驱动相关的源文件如下，
- drivers/pin/fgpio
```
soc
└── fgpio_table.c

.
├── Kconfig
├── fgpio.c
├── fgpio.h
├── fgpio_hw.h
├── fgpio_selftest.c
├── fgpio_intr.c
└── fgpio_sinit.c
```

## 3. 配置方法

- FGPIO 驱动支持 FT2000/4, D2000和E2000，在 D2000 和 E2000 上完成测试

- 参考以下步骤完成 FGPIO 硬件配置，
- 1. 获取FT2000/4, D2000或E2000的软件编程手册，参考引脚复用表，设置引脚复用为 GPIO
- 2. 初始化 GPIO 引脚实例
- 3. 设置 GPIO 引脚的输入，输出方向
- 4. 获取 GPIO 引脚上的输入，或者设置 GPIO 引脚的输出电平
- 5. 设置 GPIO 引脚的中断屏蔽位和中断触发类型，注册引脚的中断回调函数

## 4 应用示例

- 设置 GPIO 引脚的输出电平，获取引脚的输入电平，通过外部输入触发 GPIO 中断
- 通过 GPIO 产生占空比和频率可配的 PWM 波形

### [fgpio_irq](../../../example/peripherals/pin/)

## 5. API参考

### 5.1. 用户数据结构

#### FGpioConfig

```c
typedef struct
{
    u32 id; /* GPIO标号，0 ~ FGPIO_NUM */
    u32 ctrl; /* GPIO所属的控制器，0 ~ FGPIO_CTRL_NUM */
    u32 port; /* GPIO所属的Port, Port A, B */
    u32 pin; /* GPIO的引脚号，0 ~ FGPIO_PIN_NUM */
    uintptr base_addr;  /* GPIO控制器基地址 */
    u32 irq_num; /* GPIO中断号，如果不支持中断，置位为 0 */
    u32 irq_priority; /* 中断优先级 */
    u32 cap; /* GPIO引脚能力集 */
} FGpioConfig; /* GPIO引脚配置 */
```

#### FGpio

```c
typedef struct
{
    FGpioConfig config;
    u32 is_ready;
    FGpioInterruptCallback irq_cb; /* 中断回调函数 */
    void *irq_cb_params; /* 中断回调函数的入参 */
} FGpio; /* GPIO引脚实例 */
```

### 5.2  错误码定义

- FGPIO_SUCCESS : success

- FGPIO_ERR_INVALID_PARA : invalid input parameters

- FGPIO_ERR_INVALID_STATE : invalid state


### 5.3 用户API接口

#### FGpioLookupConfig

```c
const FGpioConfig *FGpioLookupConfig(u32 gpio_id)
```

Note:

- 获取GPIO引脚的默认配置

Input:

-  {u32} gpio_id, GPIO引脚号, 0 ~ FGPIO_NUM，可以通过宏 FGPIO_ID 获取编码

Return:

- {const FGpioConfig *} GPIO引脚的参数和默认配置

#### FGpioCfgInitialize

```c
FError FGpioCfgInitialize(FGpio *const pin, const FGpioConfig *const config)
```

Note:

- 初始化GPIO引脚实例

Input:

- {FGpio} *instance, GPIO引脚实例
- {FGpioConfig} *config, GPIO引脚配置

Return:

- {FError} FGPIO_SUCCESS 表示初始化成功

#### FGpioDeInitialize

```c
void FGpioDeInitialize(FGpio *const pin);
```

Note:

- 去初始化GPIO引脚实例

Input:

- {FGpio} *instance, GPIO引脚实例

Return:

- 无

#### FGpioSetDirection

```c
void FGpioSetDirection(FGpio *const pin, FGpioDirection dir)
```

Note:

- 设置GPIO引脚的输入输出方向
- 初始化 GPIO 实例后使用此函数

Input:

- @param {FGpio} *pin, GPIO引脚实例
- @param {FGpioDirection} dir, 待设置的GPIO的方向

Return:

- 无

#### FGpioGetDirection

```c
FGpioDirection FGpioGetDirection(FGpio *const pin)
```

Note:

- 获取GPIO引脚的输入输出方向
- 初始化 GPIO 实例后使用此函数

Input:

- {FGpio} *pin, GPIO引脚实例

Return:

- {FGpioDirection} GPIO引脚方向

#### FGpioSetOutputValue

```c
FError FGpioSetOutputValue(FGpio *const pin, const FGpioVal output)
```

Note:

- 设置GPIO引脚的输出值
- 初始化 GPIO 实例后使用此函数，先设置 GPIO 引脚为输出后调用此函数

Input:

- {FGpio} *pin, GPIO引脚实例
- {FGpioVal} output, GPIO引脚的输出值

Return:

- {FError} FGPIO_SUCCESS 表示设置成功

#### FGpioGetInputValue

- 获取GPIO引脚的输入值

```c
FGpioVal FGpioGetInputValue(FGpio *const pin)
```

Note:

- 初始化 GPIO 实例后使用此函数，先设置 GPIO 引脚为输入后调用此函数

Input:

- {FGpio} *instance, GPIO引脚实例

Return:

- {FGpioVal} 获取的输入值，高电平/低电平

#### FGpioGetInterruptMask

- 获取GPIO 引脚的中断屏蔽位

```c
void FGpioGetInterruptMask(FGpio *const pin, u32 *mask, u32 *enabled)
```

Note:

- 获取的是所有Pin的中断屏蔽位和中断使能位

Input:

- {FGpio} *pin, GPIO引脚实例
- {u32} *mask, 返回的GPIO引脚中断屏蔽位
- {u32} *enabled, 返回的GPIO引脚中断使能位

Return:

- 无

#### FGpioSetInterruptMask

-  设置GPIO引脚的中断屏蔽位

```c
void FGpioSetInterruptMask(FGpio *const pin, boolean enable)
```

Note:

- 引脚必须为A组引脚，B组引脚不支持中断

Input:

- {FGpio} *pin, GPIO引脚实例
- {boolean} enable, TRUE表示使能GPIO引脚中断，FALSE表示去使能GPIO引脚中断

Return:

- 无

#### FGpioGetInterruptType

- 获取GPIO引脚的中断类型和中断极性

```c
void FGpioGetInterruptType(FGpio *const pin, FGpioIrqType *type)
```

Note:

- 获取GPIO引脚的中断类型和中断极性

Input:

- {FGpio} *pin, GPIO引脚实例
- {FGpioIrqType} *type, GPIO引脚中断触发类型

Return:

- 无

#### FGpioSetInterruptType

- 设置GPIO引脚的中断类型

```c
void FGpioSetInterruptType(FGpio *const pin, const FGpioIrqType type)
```

Note:

- index对应的引脚必须为A组引脚，B组引脚不支持中断

Input:

- {FGpioPin} *pin, GPIO引脚实例
- {FGpioIrqType} type, GPIO引脚中断触发类型

Return:

- 无

#### FGpioSharedInterruptHandler

- GPIO 引脚中断处理函数

```c
void FGpioInterruptHandler(s32 vector, void *param)
```

Note:

- 需要用户将此函数注册到Interrtup上，使能GPIO中断才能生效

Input:

- {s32} vector, 中断输入参数1
- {void} *param, 中断输入参数2

Return:

- 无

#### FGpioRegisterInterruptCB

- 注册GPIO引脚中断回调函数(引脚通过控制器统一上报中断，共用中断号)

```c
void FGpioRegisterInterruptCB(FGpio *const pin, FGpioInterruptCallback cb, 
                              void *cb_param)
```

Input:

- {FGpio} pin, GPIO引脚 
- {FGpioInterruptCallback} cb, GPIO引脚中断回调函数
- {void} *cb_param, GPIO引脚中断回调函数输入参数

Return:

- 无
