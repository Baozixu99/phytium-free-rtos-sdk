<!--
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-06-30 10:35:37
 * @LastEditTime: 2021-07-05 16:46:13
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
$ ./export.sh -online
```

### 2.2. 选择离线方式安装SDK
- 下载[Phytium Standalone SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk)源代码
- 下载[Phytium FreeRTOS SDK](https://gitee.com/phytium_embedded/phytium-free-rtos-sdk)源代码
- 下载[AARCH32](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz)和[AARCH64](https://developer.arm.com/-/media/Files/downloads/gnu-a/10.2-2020.11/binrel/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf.tar.xz)编译器

- 解压phytium_freertos_sdk.zip到'~/freertos_sdk'
- 解压phytium_standalone_sdk.zip到'~/freertos_sdk/standalone-sdk'
- 将AARCH32、AARCH64编译器压缩包放置在'~/freertos_sd/tools'
- 完成FreeRTOS SDK安装
```
$ ./export.sh
```

- **通过离线/在线方式完成安装后，关闭当前窗口或者运行以下命令，立即生效SDK环境变量**
```
$ source ~/.profile
```

# 3. 使用FreeRTOS SDK



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