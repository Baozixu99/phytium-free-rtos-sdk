# OPENAMP 测试

## 1. 例程介绍

><font size="1">介绍例程的用途，使用场景，相关基本概念，描述用户可以使用例程完成哪些工作</font><br />

- OpenAMP(Open Asymmetric Multi-processing) 是一个软件架构，为多核之间非对称运行提供软件支持 。

- OpenAMP 提供了以下关键特性:

1. 提供生命周期管理

2. 兼容裸跑、RTOS等不同的软件环境

3. 兼容linux系统中的 remoteproc, rpmsg and VirtIO 模块

- 本例程基于开源openamp项目
[OpenAMP](https://github.com/OpenAMP/open-amp.git)

- 本例程主要提供了D2000/FT2004 裸机与裸机之间的测试例程 ，D2000裸机 与  linux kernel 的例程

## 2. 如何使用例程

><font size="1">描述开发平台准备，使用例程配置，构建和下载镜像的过程</font><br />


### 2.1 硬件配置方法

><font size="1">哪些硬件平台是支持的，需要哪些外设，例程与开发板哪些IO口相关等（建议附录开发板照片，展示哪些IO口被引出）</font><br />

1. 准备一块ft2000/4 或者d2000 开发板
2. 将串口连接好电脑，波特率设为 115200-8-1-N

### 2.2 SDK配置方法

><font size="1">依赖哪些驱动、库和第三方组件，如何完成配置（列出需要使能的关键配置项）</font><br />
- 本例子已经提供好具体的编译指令，以下进行介绍：
    1. make core0 将./core0 目录下的工程进行编译
    2. make core1 将./core1 目录下的工程进行编译
    3. make clean_core0  将./core0 目录下的工程进行清理
    4. make clean_core1  将./core1 目录下的工程进行清理
    5. make boot_core0   将./core0 目录下的编译后的elf 复制到目标地址
    6. make boot_core1   将./core1 目录下的编译后的elf 复制到目标地址
    7. make config_amp_d2000_aarch64 将预设64bit d2000 下的配置分别加载至 ./core0 ./core1
    8. make config_amp_d2000_aarch32 将预设32bit d2000 下的配置分别加载至 ./core0 ./core1
    9. make config_amp_ft2004_aarch64 将预设64bit ft2004 下的配置分别加载至 ./core0 ./core1
    10. make config_amp_ft2004_aarch32 将预设32bit ft2004 下的配置分别加载至 ./core0 ./core1
    11. make menuconfig_core0   配置./core0 目录下的配置变量
    12. make menuconfig_core1   配置./core1 目录下的配置变量
    13. make all                编译./core0 ./core1 下的项目工程
    14. make clean              清理./core0 ./core1 下的编译结果
    15. make boot               将 ./core0 ./core1 下的目标elf 复制到目标路径下

- 具体使用方法为：
    - 在 //example/amp/openamp  下
    - 使用以上指令

### 2.3 构建和下载

><font size="1">描述构建、烧录下载镜像的过程，列出相关的命令</font><br />

#### OpenAMP 配置
![OpenAMP配置](./figs/OpenAmpConfig.png)

#### core0 构建配置
![core0配置2](./figs/Core0Openampconfig.png)

#### core1 构建配置
![core1配置2](./figs/Core1Openampconfig.png)



- Remoteproc use ipi       : 使用ipi 中断模式进行提醒
- Openamp resource address : OpenAMP 中共享资源表中地址
- Vring tx address         : 共享发送缓冲区的起始地址，同时也是共享buffer 区域的起始地址
- Vring rx address         : 共享接收缓冲区的起始地址
- table of base physical address of each of the pages in the I/O region : 用于核心间提醒机制的共享内存初始点

- DEBUG_CODE                         : 增加裸跑shell 功能
- Destination CPU communication mask : 主核心用于唤醒从核的掩码
- Destination IPI mask               : ipi 中断中，用于唤醒其他核心的掩码
- Spin-lock shared memory            : 互斥锁中关注的共享内存 

- Select mem default attribute       : 提供内存属性选择

### 2.4 输出与实验现象

><font size="1">描述输入输出情况，列出存在哪些输入，对应的输出是什么（建议附录相关现象图片）</font><br />

#### D2000/FT2004 aarch32 裸跑程序测试 （裸机间）

1. 在编译环境下，切换至 phytium-standalone-sdk/baremetal/example/system/amp/openamp 目录
1.1 输入 'make config_amp_d2000_aarch32' 加载默认D2000配置信息，或者输入 'make config_amp_ft2004_aarch32' 加载默认ft2004配置信息 
1.2 输入 'make all' 编译core0 / core1 代码，并且生成对应的elf 文件
1.3 先将 ./core0/makefile 与 ./core1/makefile 中 的 USR_BOOT_DIR 修改为您的tftp 所覆盖的目录
1.4 输入 'make boot' 将生成的elf 拷贝至 tftp 的目录下

2. 使用串口连接D2000/FT2004 开发板 ，并且打开串口终端工具
2.1 复位开发板之后，将D2000/FT2004 开发板的网络与tftp 服务器在同一局域网中
2.2  在中断工具下输入以下命令

```
    setenv ipaddr 192.168.4.20            
    setenv serverip 192.168.4.50         
    setenv gatewayip 192.168.4.1          
    tftpboot f0000000 openamp_core0.elf  
    tftpboot f1000000 openamp_core1.elf  
    bootelf -p f0000000                  
```
2.3 会显示如下内容

![](./figs/d2000_aarch32_openamp_startup.png)


2.4 输入 'loadelf -p f1000000' 加载从核程序
2.5 输入 'rpmsg_echo_task' 运行openamp 测试程序
2.6 结果显示为

![](./figs/d2000_aarch32_openamp_reault.png)


#### D2000/FT2004 aarch64 裸跑程序测试 （裸机间）

1. 在编译环境下，切换至 phytium-standalone-sdk/baremetal/example/system/amp/openamp 目录
1.1 输入 'make config_amp_d2000_aarch64' 加载默认配置信息,或输入 'make config_amp_ft2004_aarch64' 加载默认配置信息
1.2 输入 'make all' 编译core0 / core1 代码，并且生成对应的elf 文件
1.3 先将 ./core0/makefile 与 ./core1/makefile 中 的 USR_BOOT_DIR 修改为您的tftp 所覆盖的目录
1.4 输入 'make boot' 将生成的elf 拷贝至 tftp 的目录下

2. 使用串口连接D2000/FT2004 开发板 ，并且打开串口终端工具
2.1 复位开发板之后，将D2000/FT2004 开发板的网络与tftp 服务器在同一局域网中
2.2  在中断工具下输入以下命令

```
    setenv ipaddr 192.168.4.20            
    setenv serverip 192.168.4.50         
    setenv gatewayip 192.168.4.1          
    tftpboot f0000000 openamp_core0.elf  
    tftpboot f1000000 openamp_core1.elf  
    bootelf -p f0000000                  
```
2.3 会显示如下内容

![](./figs/d2000_aarch64_openamp_startup.png)


2.4 输入 'loadelf -p f1000000' 加载从核程序
2.5 输入 'rpmsg_echo_task' 运行openamp 测试程序
2.6 结果显示为

![](./figs/d2000_aarch64_openamp_reault.png)



#### D2000 aarch32 裸机程序测试 （与linux）

1. 进入amp/openamp 目录
2. 进入core0 下 ，输入 make load_d2000_aarch32 ,加载预先配置信息
3. 输入make menuconfig  ,在 Project Configuration → AMP Config 下 取消 DEBUG_CODE
![](./figs/freertos_openamp_d2000_config.png)
4. 在 Third-Party Configuration → Use Asymmetric Multi-processing → OpenAmp 下，选择 Skip local rvdev->shbuf_io ,并选择Use the Vring definition in the master
![](./figs/freertos_openamp_d2000_aarch32_ampconfig.png)
5. 进入core0 目录下输入 "make clean"  "make" ,生成elf 文件之后，将其拷贝至linux 指定目录下
![](./figs/d2000_linux_reasult.png)


#### D2000 aarch64 裸机程序测试 （与linux）

1. 进入amp/openamp 目录
2. 进入core0 下 ，输入 make load_d2000_aarch64 ,加载预先配置信息
3. 输入make menuconfig  ,在 Project Configuration → AMP Config 下 取消 DEBUG_CODE
![](./figs/baremetal_openamp_d2000_config.png)
4. 在 Third-Party Configuration → Use Asymmetric Multi-processing → OpenAmp 下，选择 Skip local rvdev->shbuf_io ,并选择Use the Vring definition in the master
![](./figs/freertos_openamp_d2000_aarch32_ampconfig.png)
5. 进入core0 目录下输入 "make clean"  "make" ,生成elf 文件之后，将其拷贝至linux 指定目录下
![](./figs/d2000_linux_reasult.png)



## 3. 如何解决问题 (Q&A)

><font size="1">主要记录使用例程中可能会遇到的问题，给出相应的解决方案</font><br />

## 4. 修改历史记录

><font size="1">记录例程的重大修改记录，标明修改发生的版本号 </font><br />

- 2021-03-21 ：v0.1.0 初始化项目
