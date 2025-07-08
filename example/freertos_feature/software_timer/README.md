# software timer feature base on freertos

## 1. 例程介绍

本例程示范了freertos环境下的软件定时器的使用，包括create、start、stop、reset和set id
等不同的操作；
FreeRTOS 提供的软件定时器支持单次模式和周期模式；
单次模式：当用户创建了定时器并启动了定时器后，定时时间到了，只执行一次回调函数之后就将该定时器删除，不再重新执行;
周期模式：这个定时器会按照设置的定时时间循环执行回调函数，直到用户将定时器删除;

## 2. 如何使用例程

本例程需要用到
- Phytium开发板（D2000 TEST、FT2000/4 DSK、E2000D/Q DEMO、PD2308 DEMO、PD2408 TEST_A/TEST_B、PhytiumPi）
- [Phytium FreeRTOS SDK](https://gitee.com/phytium_embedded/phytium-free-rtos-sdk)
- [Phytium standalone SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk)
### 2.1 硬件配置方法

本例程支持的硬件平台包括
- D2000 TEST、FT2000/4 DSK、E2000D/Q DEMO、PD2308 DEMO、PD2408 TEST_A/TEST_B、PhytiumPi开发板

对应的配置项是
- CONFIG_FT2004_DSK_BOARD
- CONFIG_D2000_TEST_BOARD
- CONFIG_E2000D_DEMO_BOARD
- CONFIG_E2000Q_DEMO_BOARD
- CONFIG_PD2308_DEMO_BOARD
- CONFIG_PD2408_TEST_A_BOARD
- CONFIG_PD2408_TEST_B_BOARD
- CONFIG_PHYTIUMPI_FIREFLY_BOARD

### 2.2 SDK配置方法

本例程需要，

- 使能Shell

对应的配置项是，

- CONFIG_USE_LETTER_SHELL

- CONFIG_NON_SECURE_PHYSICAL_TIMER 或 CONFIG_NON_SECURE_VIRTUAL_TIMER 选择使用非安全物理定时器或者虚拟定时器作为Tick中断源
![tick_interrupt_source_config](./figs/tick_interrupt_source_config.png)

本例子已经提供好具体的编译指令，以下进行介绍:
- make                 将目录下的工程进行编译
- make clean           将目录下的工程进行清理
- make image           将目录下的工程进行编译，并将生成的elf 复制到目标地址
- make list_kconfig    当前工程支持哪些配置文件
- make load_kconfig    LOAD_CONFIG_NAME=<kconfig configuration files>  将预设配置加载至工程中
- make menuconfig      配置目录下的参数变量
- make backup_kconfig  将目录下的sdkconfig 备份到./configs下

具体使用方法为:
- 在当前目录下
- 执行以上指令

### 2.3 构建和下载

><font size="1">描述构建、烧录下载镜像的过程，列出相关的命令</font><br />

[参考 freertos 使用说明](../../../docs/reference/usr/usage.md)

#### 2.3.1 下载过程

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

- 系统进入后，输入```timer```查看指令说明
- 输入```timer create_start_example```，启动创建、使能定时器测试

![cre](./figs/timer_cre.png)

- 输入```timer id_reset_example```，启动定时器复位、设置id测试

![reset](./figs/timer_reset.png)

- 测试任务能够能正常创建和删除，输入```ps```查看任务状态正常，即测试正常

## 3. 如何解决问题

## 4. 修改历史记录