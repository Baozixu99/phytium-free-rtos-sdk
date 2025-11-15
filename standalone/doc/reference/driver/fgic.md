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
 * FilePath: fgic.md
 * Date: 2022-04-15 13:57:32
 * LastEditTime: 2022-04-15 13:57:32
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
-->


# GIC 驱动说明

## 1.驱动概述

GIC 是通用中断控制器，它为ARM CPU提供外设中断与软件中断的控制，GIC主要支持SPI\SGI\PPI\LPI等不同类型的中断，本版本支持的驱动版本为GICv3，并且支持了以下特性：
1.SPI支持多核心路由配置、优先级配置、使能关闭功能
2.SGI提供软件触发接口、优先级配置、支持多核触发、目标核心触发、使能关闭功能
3.PPI 提供优先级配置、使能关闭功能
4.支持CPU interface 中断掩码配置
5.支持组优先级分组配置
6.当前仅支持双安全态下的Non-secure访问


## 2.驱动功能

驱动组成由以下所示
```
.
├── fgic_v3.c
├── fgic_v3.h
├── fgic_its.c
└── fgic_its.h
```
- 其中fgic.c/h 为开发者提供以下功能：
1. 初始化GIC 中断实例
2. 提供基于中断号中断开关功能
3. 提供基于中断号中断优先级设置与获取功能
4. 提供基于中断号触发方式
5. 提供SGI中断触发功能
6. 提供中断承认（Acknowledge）接口
7. 提供中断优先级掩码配置与获取功能
8. 提供Distrutior、Redistributor、CPU interface 的 初始化功能

## 3.数据结构

```
typedef struct
{
    u32 instance_id; /*Gic 实例编号*/
    uintptr dist_base; /* Distributor 基地址*/
    
} FGicConfig;

```

```
typedef struct
{
    FGicConfig config; /* 配置数据结构*/
    u32 is_ready;       /* 驱动实例的状态 */
    uintptr redist_base; /* 当前实例核心对应的Redistributor 地址  */
    SECURITY_STATE security ; /* GIC 驱动当前支持的安全态模式 */
    s32 max_spi_num;    /* SPI 最大中断优先级ID */
} FGic;
```

## 4.错误码定义

```
FGIC_CTLR_ERR_TYPE    /* 错误选择CTLR 寄存器 */
FGIC_CTLR_ERR_NUM     /* 当前控制器不支持此中断id */
FGIC_CTLR_ERR_IN_SET   /* 在设置过程中出现的异常 */
FGIC_CTLR_ERR_IN_GET   /* 在获取过程中出现的异常 */
```

## 5.应用示例

common/interrupt.c	中断接口提供底层支持

### [gic特性例程](../../../example/peripherals/gicv3/)

## 6.API介绍

### 1. FGicLookupConfig

```
FGicConfig *FGicLookupConfig(u32 instance_id)
```

#### 介绍
获取当前GIC 驱动默认配置
#### 参数
- u32 instance_id ：当前GIC 驱动中对应的ID

#### 返回
- FGicConfig * ：静态默认配置

### 2. FGicCfgInitialize

```
FError FGicCfgInitialize(FGic *instance_p, const FGicConfig *input_config_p , uintptr redist_base)
```

#### 介绍
根据传入配置，初始化GIC 驱动实例

#### 参数
- FGic *instance_p：指向FGic实例的指针
- const FGicConfig *input_config_p：需要应用于示例中的配置项
- redist_base：是当前核心的重分发地址  

#### 返回
- FError ：FGIC_SUCCESS 为初始成功


### 3. FGicDistrubutiorInit

```
void FGicDistrubutiorInit(FGic *instance_p)
```

#### 介绍
初始化Distrubutior 模块

#### 参数
- FGic *instance_p：指向FGic实例的指针

#### 返回
- 无

### 4. FGicRedistributorInit

```
FError FGicRedistributorInit(uintptr redist_base)
```

#### 介绍
初始化Redistributor 模块

#### 参数
- uintptr redist_base：当前核心的重分发地址

#### 返回
- {FError} FGIC_SUCCESS is success ,FGIC_ERR_IN_TIMEOUT is timeout

### 5. FGicCpuInterfaceInit

```
void FGicCpuInterfaceInit(void)
```

#### 介绍
初始化当前核心下的cpu interface模块

#### 返回
- 无

### 6. FGicIntEnable

```
FError FGicIntEnable(FGic *instance_p,s32 int_id)
```

#### 介绍
基于中断号使能中断功能

#### 参数
- FGic *instance_p：指向FGic实例的指针
- s32 int_id：中断ID

#### 返回
- FError ：FGIC_SUCCESS 为操作成功

### 7. FGicIntDisable

```
FError FGicIntDisable(FGic *instance_p,s32 int_id)
```

#### 介绍
基于中断号关闭中断功能

#### 参数
- FGic *instance_p：指向FGic实例的指针
- s32 int_id：中断ID

#### 返回
- FError ：FGIC_SUCCESS 为操作成功

### 8. FGicSetPriority

```
FError FGicSetPriority(FGic *instance_p,s32 int_id,u32 priority)
```

#### 介绍
基于中断号设置当前中断优先级的值

#### 参数
- FGic *instance_p：指向FGic实例的指针
- s32 int_id：中断ID
- u32 priority： 中断优先级的值，有效值8bit

#### 返回
- FError ：FGIC_SUCCESS 为操作成功 ，当反馈为FGIC_CTLR_ERR_IN_GET 时，表示中断ID超过范围

### 9. FGicGetPriority

```
u32 FGicGetPriority(FGic *instance_p,s32 int_id)
```

#### 介绍
基于中断号获取当前中断优先级的值

#### 参数
- FGic *instance_p：指向FGic实例的指针
- s32 int_id：中断ID

#### 返回
- u32 ：中断优先级的值，有效值8bit 。当反馈为FGIC_CTLR_ERR_IN_GET 时，表示中断ID超过范围

### 10. FGicSetTriggerLevel

```
FError FGicSetTriggerLevel(FGic *instance_p,s32 int_id,TRIGGER_LEVEL trigger_way)
```

#### 介绍
基于当前中断号配置中断触发模式

#### 参数
- FGic *instance_p：指向FGic实例的指针
- s32 int_id：中断ID
- TRIGGER_LEVEL trigger_way：中断触发模式 ， 参数为0的时为电平触发，参数为1的时为电平触发

#### 返回
- FError ：FGIC_SUCCESS 为操作成功 ，当反馈为FGIC_CTLR_ERR_IN_GET 时，表示中断ID超过范围

### 11. FGicGetTriggerLevel

```
u32 FGicGetTriggerLevel(FGic *instance_p,s32 int_id)
```

#### 介绍
基于当前中断号获取中断触发模式

#### 参数
- FGic *instance_p：指向FGic实例的指针
- s32 int_id：中断ID

#### 返回
- TRIGGER_LEVEL trigger_way：中断触发模式 ， 参数为0的时为电平触发，参数为1的时为电平触发

### 12. FGicSetSpiAffinityRouting

```
FError FGicSetSpiAffinityRouting(FGic *instance_p,s32 int_id,SPI_ROUTING_MODE route_mode,u64 affinity)
```

#### 介绍
为特定的SPI中断设置中间路由信息  

#### 参数
- FGic *instance_p：指向FGic实例的指针
- s32 int_id：中断ID
- SPI_ROUTING_MODE route_mode：中断路由模式 ，SPI_ROUTING_TO_SPECIFIC spi 中断路由给特定PE ,SPI_ROUTING_TO_ANY spi 中断将路由给任何开启本SPI 的PE处
- u64 affinity：亲和度参数，它的格式为：
 * |--------[bit39-32]-------[bit23-16]-------------[bit15-8]--------[bit7-0]
 * |--------Affinity level3-----Affinity level2-----Affinity level1---Affinity level0   

#### 返回
- FError ：FGIC_SUCCESS 为操作成功，FGIC_CTLR_ERR_IN_SET 为中断ID 不符合SPI的范围


### 13. FGicGetAffinityRouting

```
FError FGicGetAffinityRouting(FGic *instance_p,s32 int_id,SPI_ROUTING_MODE *route_mode_p,u64 *affinity_p)
```

#### 介绍
获取特定的SPI中断设置路由信息  

#### 参数
- FGic *instance_p：指向FGic实例的指针
- s32 int_id：中断ID
- SPI_ROUTING_MODE  *route_mode_p：中断路由模式实例的指针 ，当参数为SPI_ROUTING_TO_SPECIFIC  spi 中断路由给特定PE , SPI_ROUTING_TO_ANY  spi 中断将路由给任何开启本SPI 中断的PE处
- u64  *affinity_p：亲和度参数实例的指针，它的值格式为：
 * |--------[bit39-32]-------[bit23-16]-------------[bit15-8]--------[bit7-0]
 * |--------Affinity level3-----Affinity level2-----Affinity level1---Affinity level0   

#### 返回
- FError ：FGIC_SUCCESS 为操作成功，FGIC_CTLR_ERR_IN_GET 为中断ID 不符合SPI的范围

### 14. FGicGenerateSgi
```
FError FGicGenerateSgi(s32 int_id,u32 target_list,SGI_ROUTING_MODE routing_mode,u64 affinity)
```

#### 介绍
基于中断id产生SGI中断的接口 

#### 参数
- s32 int_id：中断ID
- u32 target_list：将为其生成SGI中断的pe集合，每一位对应一个集群内的一个PE
SGI_ROUTING_MODE routing_mode：SGI 的路由模式，SGI_ROUTING_TO_SPECIFIC 为特定的PE,SGI_ROUTING_TO_ANY 为所有的PE 除开自己这个核心。
- u64 affinity ：亲和度参数，它的格式为：
 * |--------[bit55-48]-------[bit39-32]-------------[bit23-16]
 * |--------Affinity level3-----Affinity level2-----Affinity level1   

#### 返回
- FError ：FGIC_SUCCESS 为操作成功，FGIC_CTLR_ERR_IN_SET 为中断ID 不符合SGI的范围

### 15. FGicDeactionInterrupt

```
void FGicDeactionInterrupt(s32 int_id)
```

#### 介绍
钝化正处于激活态的中断

#### 参数
- s32 int_id：中断ID

#### 返回
- 无

### 16. FGicAcknowledgeInt

```
s32 FGicAcknowledgeInt()
```

#### 介绍
承认当前被挂起的中断

#### 参数
- 无

#### 返回
- s32 int_id：被承认的中断ID


### 17. FGicSetPriorityFilter

```
void FGicSetPriorityFilter(u32 priority_mask)
```

#### 介绍
通过设置ICC_PMR参数，确定中断控制器可以响应的中断范围
 
#### 参数
- u32 priority_mask：如果一个中断的优先级低于该字段所指定的值，则接口向PE发送中断信号。 priority_mask的参考值如下所示  ：
 * |priority_mask---------------256-------254--------252------248-------240
 * |Implemented priority bits---[7:0]----[7:1]------[7:2]-----[7:3]-----[7:4]  
 * |priority the growing steps--any-----even value----4---------8--------16  

#### 返回
- 无

### 18. FGicSetPriorityGroup

```
void FGicSetPriorityGroup(u32 binary_point)
```

#### 介绍
设置当前组优先级的值 

#### 参数
- u32 binary_point：该字段的值控制如何将8位中断优先级字段拆分为组优先级和子优先级， 
- binary_point值与分组的关系如下所示  
 * |return value----------------0-------1--------2------3-------4------5------6-------7
 * |Group priority field------[---]----[7:1]---[7:2]--[7:3]---[7:4]---[7:5]---[7:6]---[7]  
 * |Subpriority field---------[---]-----[0]----[1:0]--[2:0]---[3:0]---[4:0]---[5:0]---[6:0] 

#### 返回
- 无


### 19. FGicGetPriorityFilter

```
u32 FGicGetPriorityFilter()
```

#### 介绍
获取当前优先级过滤值

#### 参数
- 无

#### 返回
- u32 Priority Mask，CPU接口中的优先级掩码 ，如果一个中断的优先级低于该字段所指定的值，则接口向PE发送中断信号。  priority_mask的参考值如下所示 ：
 * |return value------------------256------254--------252------248-------240
 * |Implemented priority bits---[7:0]----[7:1]------[7:2]-----[7:3]-----[7:4]  
 * |priority the growing steps---any-----even value----4--------8-------16  

### 20. FGicGetPriorityGroup

```
u32 FGicGetPriorityGroup(void)
```

#### 介绍
获取当前CPU interface 中的分组值

#### 参数
- 无

#### 返回
- u32 Priority group，该字段的值控制如何将8位中断优先级字段拆分为组优先级和子优先级， 
binary_point值与分组的关系如下所示  
 * |return value----------------0-------1--------2------3-------4------5------6-------7
 * |Group priority field------[---]----[7:1]---[7:2]--[7:3]---[7:4]---[7:5]---[7:6]---[7]  
 * |Subpriority field---------[---]-----[0]----[1:0]--[2:0]---[3:0]---[4:0]---[5:0]---[6:0] 

### 21 FGicItsInit

FError FGicItsInit(FGic *instanse_p)

#### 介绍
Initialize the ITS.

#### 参数
- FGic *instanse_p：指向FGic实例的指针

#### 返回
- FError：Result of the operation.

### 22. FGicLpisSetup

void FGicLpisSetup(FGic *instanse_p)

#### 介绍
Setup LPIs.

#### 参数
- FGic *instanse_p：指向FGic实例的指针

#### 返回
- 无


### 23.FGicItsDeviceIdInit

FError FGicItsDeviceIdInit(const FGic *instanse_p, u32 device_id, u32 nites)

#### 介绍
Initialize a device ID.

#### 参数
- FGic *instanse_p：指向FGic实例的指针
- u32 device_id：Device ID.
- u32 nites：Number of ITT entries.

#### 返回
- FError：Result of the operation.


### 24. FGicItsDeviceIdDeinit

FError FGicItsDeviceIdDeinit(const FGic *instanse_p, u32 device_id)

#### 介绍
Deinitialize a device ID.

#### 参数
- FGic *instanse_p：指向FGic实例的指针
- u32 device_id：Device ID.

#### 返回
- FError：Result of the operation.


### 25. FGicItsMapIntId

FError FGicItsMapIntId(const FGic *instanse_p, u32 device_id, u32 event_id, u32 intid)

#### 介绍
Map an interrupt ID.

#### 参数
- FGic *instanse_p：指向FGic实例的指针
- u32 device_id：Device ID.
- u32 event_id：Event ID.
- u32 intid：Interrupt ID.

#### 返回
- FError：Result of the operation.


### 26. FGicItsLpiSetup

void FGicItsLpiSetup(const FGic *instanse_p, u32 intid, u8 enable)

#### 介绍
Setup the LPI configuration.

#### 参数
- FGic *instanse_p：指向FGic实例的指针
- u32 intid：Interrupt ID.
- u8 enable：Enable flag.

#### 返回
- 无

### 27. FGicItsLpiSetPriority

void FGicItsLpiSetPriority(const FGic *instanse_p, u32 intid, u32 prio)

#### 介绍
Set the priority of an LPI.

#### 参数
- FGic *instanse_p：指向FGic实例的指针
- u32 intid：Interrupt ID.
- u32 prio：Priority value.

#### 返回
- 无

### 28. FGicItsSendIntCmd

FError FGicItsSendIntCmd(FGicItsData *data, u32 device_id, u32 event_id)

#### 介绍
Send an INT command to the ITS.

#### 参数
- FGicItsData *data：指向ITS数据的指针
- u32 device_id：Device ID.
- u32 event_id：Event ID.

#### 返回
- FError：Result of the operation.

### 29. FGicItsGetTransAddr

u32 FGicItsGetTransAddr(const FGic *instanse_p)

#### 介绍
Get the translation address.

#### 参数
- FGic *instanse_p：指向FGic实例的指针

#### 返回
- u32：Translation address.

### 30.FGicItsLpiIdAlloc

u32 FGicItsLpiIdAlloc(FGic *instanse_p)

#### 介绍
Allocate an LPI (Locality-specific Peripheral Interrupt) ID.

#### 参数
- FGic *instanse_p：指向FGic实例的指针

#### 返回
- u32：Allocated LPI ID or 0 if no ID is available.

### 31. FGicItsLpiIdFree

void FGicItsLpiIdFree(FGic *instanse_p, u32 intid)

#### 介绍
Free a previously allocated LPI ID.

#### 参数
- FGic *instanse_p：指向FGic实例的指针
- u32 intid：Interrupt ID to be freed.

#### 返回
- 无
