<!--
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-06-30 10:35:37
 * @LastEditTime: 2021-07-22 15:02:51
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

- 检查install.sh的执行权限, 如果没有执行权限`x`，需要利用`chmod +x ./install.sh`添加执行权限
```
$ ls ./install.sh  -l
-rwxrwxrwx 1 usr usr 6353 Jul 21 10:13 ./install.sh
```

- 在线下载源代码和编译环境，完成FreeRTOS SDK安装
```
$ ./install.sh -online
```

### 2.2. 选择离线方式安装SDK
- 下载[Phytium Standalone SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk)源代码
- 下载[Phytium FreeRTOS SDK](https://gitee.com/phytium_embedded/phytium-free-rtos-sdk)源代码
- 下载[AARCH32](https://gitee.com/phytium_embedded/phytium-standalone-sdk/attach_files/776847/download/gcc-arm-x86_64-none-eabi-10-2020-q4-major.tar.xz)编译器
- 下载[AARCH64](https://gitee.com/phytium_embedded/phytium-standalone-sdk/attach_files/776846/download/gcc-arm-x86_64-aarch64-none-elf-10.2-2020.11.tar.xz)编译器

- 解压phytium_freertos_sdk.zip为`~/freertos_sdk`（路径名可以自定义）
- 解压phytium_standalone_sdk.zip为`~/freertos_sdk/standalone`（必须放置在standalone路径下），除了bsp和lib两个目录及其文件外，`~/freertos_sdk/standalone`中其它的目录及文件不需要使用，可以删除
- 将AARCH32、AARCH64编译器压缩包放置在`~/freertos_sdk/tools`（必须放置在tools路径下）
- 确保install.sh有执行权限，`~/freertos_sdk`目录下在运行以下命令，完成FreeRTOS SDK安装
```
$ ./install.sh
```

- **安装完成后重启系统，生效FreeRTOS SDK环境**

# 3. 使用FreeRTOS SDK

## 3.1 快速使用例程

### 3.1.1 使用FT2000-4开发板，启动FreeRTOS
>freertos_sdk/example/function_test
- 加载FT2000-4 aarch32的，默认配置
```
$ make config_ft2004_aarch32
```

- 如果需要修改配置，例如修改编译输出文件的名字
```
$ make menuconfig
```
![menuconfig](https://images.gitee.com/uploads/images/2021/0722/093202_781126bc_8736513.png "屏幕截图.png")


- 进入Project Configuration → Build Target Name

- 编译例程
```
$ make
```
![输入图片说明](https://images.gitee.com/uploads/images/2021/0722/093410_25ad64a4_8736513.png "屏幕截图.png")

- 将*.bin通过TFTP服务器烧写到开发板，跳转启动
```
sudo service tftpd-hpa restart  //重启host侧tftp服务器
```

```
setenv ipaddr 192.168.4.20  //配置开发板侧的ip
setenv serverip 192.168.4.50 
setenv gatewayip 192.168.4.1 
tftpboot 80100000 ft2004_freertos.bin   //通过tftp服务加载编译二进制文件
dcache flush
go 0x80100000 // 跳转执行
```

![输入图片说明](https://images.gitee.com/uploads/images/2021/0722/093443_cc4d8166_8736513.png "屏幕截图.png")

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