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
 * Date: 2022-02-24 16:55:00
 * LastEditTime: 2022-03-21 17:00:59
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
-->

# pwm base on freertos

## 1. 例程介绍

本例程示范了freertos环境下的pwm的使用，包括pwm的初始化、pwm占空比周期变化操作；
程序启动后，创建pwm初始化任务，设置pwm时钟分频、周期和占空比等；
创建pwm占空比变化任务FFreeRTOSPwmChangeTask，用于定时变化pwm占空比；
创建单次模式的软件定时器，回调函数为删除FFreeRTOSPwmChangeTask，去初始化pwm，删除软件定时器；
使用C板进行测试，选择pwm模块7的两路pwm_out，也就是pwm14和pwm15作为输出，对应板上J152的引脚18和引脚20；

## 2. 如何使用例程

本例程需要用到
- Phytium开发板（E2000D/E2000Q）
- [Phytium freeRTOS SDK](https://gitee.com/phytium_embedded/phytium-free-rtos-sdk)
- [Phytium standalone SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk)
### 2.1 硬件配置方法

本例程支持的硬件平台包括

- E2000D
- E2000Q

对应的配置项是，

- CONFIG_TARGET_E2000D
- CONFIG_TARGET_E2000Q

### 2.2 SDK配置方法

本例程需要，

- 使能Shell
- 使能Pwm

对应的配置项是，

- CONFIG_USE_LETTER_SHELL
- CONFIG_USE_PWM
- CONFIG_FREERTOS_USE_PWM

本例子已经提供好具体的编译指令，以下进行介绍:
- make 将目录下的工程进行编译
- make clean  将目录下的工程进行清理
- make boot   将目录下的工程进行编译，并将生成的elf 复制到目标地址
- make load_e2000d_aarch64  将预设64bit e2000d 下的配置加载至工程中
- make load_e2000d_aarch32  将预设32bit e2000d 下的配置加载至工程中
- make load_e2000q_aarch64  将预设64bit e2000q 下的配置加载至工程中
- make load_e2000q_aarch32  将预设32bit e2000q 下的配置加载至工程中
- make menuconfig   配置目录下的参数变量
- make backup_kconfig 将目录下的sdkconfig 备份到./configs下

具体使用方法为:
- 在当前目录下
- 执行以上指令

### 2.3 构建和下载

#### 2.3.1 构建过程

- 在host侧完成配置
配置成e2000q，对于其它平台，使用对于的默认配置，如e2000d `make load_e2000d_aarch32`

- 选择目标平台
```
make load_e2000q_aarch32
```

- 选择例程需要的配置
```
make menuconfig
```

- 进行编译
```
make
```

- 将编译出的镜像放置到tftp目录下
```
make boot
```

#### 2.3.2 下载过程

- host侧设置重启host侧tftp服务器
```
sudo service tftpd-hpa restart
```

- 开发板侧使用bootelf命令跳转
```
setenv ipaddr 192.168.4.20  
setenv serverip 192.168.4.50 
setenv gatewayip 192.168.4.1 
tftpboot 0x90100000 freertos.elf
bootelf -p 0x90100000
```

### 2.4 输出与实验现象

- 系统进入后，创建pwm初始化任务，创建占空比变化任务，并创建单次模式软件定时器

![create](./figs/create.png)

- 定时器时间到，触发单次模式软件定时器的回调函数，去初始化pwm，删除占空比变化任务，删除软件定时器

![delete](./figs/delete.png)

## 3. 如何解决问题

- 若不想使能死区输出，可将FFreeRTOSPwmDbSet的部分去除，只需调用FFreeRTOSPwmSet即可。

## 4. 修改历史记录




