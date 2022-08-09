<!--
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * FilePath: README.md
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:00:23
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
-->

# Phytium-FreeRTOS-SDK

## 1. 介绍

本项目发布了Phytium系列CPU的FreeRTOS源代码，参考例程以及配置构建工具

---

## 2. 快速入门

- 目前支持在Windows和Linux上使用SDK，支持在x86_64和arm aarch64设备上完成交叉编译

![windows](./docs/fig/windows.png)![linux](./docs/fig/linux.png)![输入图片说明](./docs/fig/kylin.png)

- 参考如下说明搭建Phytium FreeRTOS SDK的软件环境

    [Windows10 快速入门](./docs/reference/usr/install_windows.md)

    [Linux x86_64 快速入门](./docs/reference/usr/install_linux_x86_64.md)

    [Linux arm aarch64 快速入门](./docs/reference/usr/install_linux_aarch64.md)


- 参考[使用说明](./docs/reference/usr/usage.md), 新建Phytium FreeRTOS SDK的应用工程，与开发板建立连接


---

## 3. 硬件参考

### 3.1 FT2000-4

FT-2000/4 是一款面向桌面应用的高性能通用 4 核处理器。每 2 个核构成 1 个处理器核簇（Cluster），并共享 L2 Cache。主要技术特征如下：

- 兼容 ARM v8 64 位指令系统，兼容 32 位指令
- 支持单精度、双精度浮点运算指令
- 支持 ASIMD 处理指令
- 集成 2 个 DDR4 通道，可对 DDR 存储数据进行实时加密
- 集成 34 Lane PCIE3.0 接口：2 个 X16（每个可拆分成 2 个 X8），2 个 X1
- 集成 2 个 GMAC，RGMII 接口，支持 10/100/1000 自适应
- 集成 1 个 SD 卡控制器，兼容 SD 2.0 规范
- 集成 1 个 HDAudio，支持音频输出，可同时支持最多 4 个 Codec
- 集成 SM2、SM3、SM4 模块
- 集成 4 个 UART，1 个 LPC，32 个 GPIO，4 个 I2C，1 个 QSPI，2 个通 用 SPI，2 个 WDT，16 个外部中断（和 GPIO 共用 IO）
- 集成温度传感器

### 3.2 D2000

D2000 是一款面向桌面应用的高性能通用 8 核处理器。每 2 个核构成 1 个处理器核簇（Cluster），并共享 L2 Cache。存储系统包含 Cache 子系统和 DDR，I/O 系统包含 PCIe、高速 IO 子系统、千兆位以太网 GMAC 和低速 IO 子系统，主要技术特征如下，

- 兼容 ARM v8 64 位指令系统，兼容 32 位指令
- 支持单精度、双精度浮点运算指令
- 支持 ASIMD 处理指令
- 集成 2 个 DDR 通道，支持 DDR4 和 LPDDR4，可对 DDR 存储数据进行实时加密
- 集成 34 Lane PCIE3.0 接口：2 个 X16（每个可拆分成 2 个 X8），2 个 X1
- 集成 2 个 GMAC，RGMII 接口，支持 10/100/1000 自适应
- 集成 1 个 SD 卡控制器，兼容 SD 2.0 规范
- 集成 1 个 HDAudio，支持音频输出，可同时支持最多 4 个 Codec
- 集成 SM2、SM3、SM4、SM9 模块
- 集成 4 个 UART，1 个 LPC，32 个 GPIO，4 个 I2C，1 个 QSPI，2 个通用 SPI，2 个 WDT，16 个外部中断（和 GPIO 共用 IO）
- 集成 2 个温度传感器

---

## 4. 参考资料

- The FreeRTOS Reference Manual API Functions and Configuration Options
- Mastering the FreeRTOS Real Time Kernel A Hands-On Tutorial Guide
---

## 5. 贡献方法

请联系飞腾嵌入式软件部

huanghe@phytium.com.cn

zhugengyu@phytium.com.cn

wangxiaodong1030@phytium.com.cn

liushengming1118@phytium.com.cn

---

## 6. 许可协议

Phytium Public License 1.0 (PPL-1.0)