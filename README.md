<!--
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-06-30 10:35:37
 * @LastEditTime: 2021-07-06 09:15:37
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
-->
# Phytium-FreeRTOS-SDK

## 1. 介绍
本项目发布了Phytium系列CPU的FreeRTOS源代码，参考例程以及配置构建工具


## 2. Ubuntu 20.04 x86_64环境下安装FreeRTOS SDK
### 2.1. 选择在线方式安装SDK

- 通过Gitee拉取FreeRTOS SDK源代码
```
$ git clone https://gitee.com/phytium_embedded/phytium-free-rtos-sdk.git ~/freertos_sdk
$ cd ~/freertos_sdk
```

- 在线下载源代码和编译环境，完成FreeRTOS SDK安装
```
$ ./install.sh -online
```

### 2.2. 选择离线方式安装SDK
- 下载[Phytium Standalone SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk)源代码
- 下载[Phytium FreeRTOS SDK](https://gitee.com/phytium_embedded/phytium-free-rtos-sdk)源代码
- 下载[AARCH32](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz)和[AARCH64](https://developer.arm.com/-/media/Files/downloads/gnu-a/10.2-2020.11/binrel/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf.tar.xz)编译器

- 解压phytium_freertos_sdk.zip到'~/freertos_sdk'（路径名可以自定义）
- 解压phytium_standalone_sdk.zip到'~/freertos_sdk/standalone-sdk'（路径名以及相对位置可以自定义）
- 将AARCH32、AARCH64编译器压缩包放置在'~/freertos_sd/tools'（必须放置在tools路径下）
- 完成FreeRTOS SDK安装
```
$ ./install.sh
```

- **通过离线/在线方式完成安装后，关闭当前窗口或者运行以下命令，立即生效SDK环境变量**
```
$ source ~/.profile
```

# 3. 使用FreeRTOS SDK

## 3.1 快速使用例程

### 3.1.1 使用FT2000-4开发板，启动FreeRTOS
>freertos_sdk/example/ft2004_aarch32_hello_world
- 配置freertos aarch32例程
```
$ make menuconfig
```
![menuconfig](https://images.gitee.com/uploads/images/2021/0706/154259_223ae31f_8736513.png "menuconfig.png")

- 进入Project Configuration→ FT2000-4 AARCH32 FreeRTOS Configuration→ Choose The Demo，选择Hello World

- 编译例程
```
$ make
```
![building](https://images.gitee.com/uploads/images/2021/0706/154546_1a86b2f7_8736513.png "building.png")

- 将*.bin通过TFTP服务器烧写到开发板，跳转启动

![hello world](https://images.gitee.com/uploads/images/2021/0707/170359_b847c982_8736513.png "hello world.png")

## 参考资料

- The FreeRTOS Reference Manual API Functions and Configuration Options
- Mastering the FreeRTOS Real Time Kernel A Hands-On Tutorial Guide

## 7. 贡献方法

请联系飞腾嵌入式软件部

huanghe@phytium.com.cn

zhugengyu@phytium.com.cn

---

## 8. 许可协议

 Apache-2.0