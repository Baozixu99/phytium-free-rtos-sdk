# i2c base on freertos

## 1. 例程介绍

I2C主从机模拟Eeprom通信测试例程(i2c_ms_example.c)
注：该例程目前仅支持在PhytiumPi开发板上进行测试，E2000 D/Q DEMO板未引出两组I2C或MIO控制器引脚
- 初始化MIO1控制器，设置为主机工作模式；初始化MIO2控制器，设置为从机工作模式，从机地址设置为0x30
- 主机分别向从机内部偏移地址0x01和0x31处写入16字节长度数组(0x01-0x10)
- 主机依次读取从机内部偏移地址0x01和0x31两处地址16字节长度的数据，与上述写入数据进行对比并打印
- 打印从机内部所有存储数据，观测已写入内容
- 去初始化MIO1/MIO2控制器

I2C与RTC通信测试例程(i2c_rtc_example.c)
注：该例程目前仅支持在E2000 D/Q DEMO开发板上进行测试，PhytiumPi开发板未内嵌RTC芯片
- 初始化MIO9控制器，设置为主机工作模式
- 向RTC芯片写入初始化时间进行，并开启计时
- 每间隔1s，连续两次读取RTC内部时间并打印，观测打印结果
- 去初始化MIO9控制器

## 2. 如何使用例程

本例程需要用到

- Phytium开发板（E2000 D/Q DEMO、PD2308 DEMO、PhytiumPi）
- [Phytium FreeRTOS SDK](https://gitee.com/phytium_embedded/phytium-free-rtos-sdk)
- [Phytium standalone SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk)

### 2.1 硬件配置方法

本例程支持的硬件平台包括

- E2000 D/Q DEMO、PD2308 DEMO、PhytiumPi开发板

对应的配置项是

- CONFIG_E2000D_DEMO_BOARD
- CONFIG_E2000Q_DEMO_BOARD
- CONFIG_PD2308_DEMO_BOARD
- CONFIG_PHYTIUMPI_FIREFLY_BOARD

### 2.1.1 硬件连线

- 下图所示为E2000 D/Q DEMO开发板RTC芯片

![e2000_1339](figs/e2000_1339.jpg)


- PhytiumPi请按照下图,将PIN_3与PIN_8引脚相连，PIN_5与PIN_10引脚相连

![phytiumpi](figs/hw_i2c_pi.png)

如需使用其他控制器测试，可参考下表进行连接，修改`i2c_ms_example.c`，`I2C_MS_TEST_MASTER`和`I2C_MS_TEST_SLAVE`即可

|   **引脚**    | **控制器与通道** |
| :----------:  | :-----------------|
|  J1 PIN_3     | MIO1  I2C_SDA |
|  J1 PIN_5     | MIO1  I2C_SCL |
|  J1 PIN_8     | MIO2  I2C_SDA |
|  J1 PIN_10    | MIO2  I2C_SCL |
|  J1 PIN_27    | MIO8  I2C_SDA |
|  J1 PIN_28    | MIO8  I2C_SCL |
|  J1 PIN_16    | MIO10 I2C_SDA |
|  J1 PIN_11    | MIO10 I2C_SCL |
|  J2 PIN_4     | MIO0  I2C_SDA |
|  J2 PIN_2     | MIO0  I2C_SCL |

### 2.2 SDK配置方法

本例程需要，

- 使能Shell
- 使能I2C
- 如果是E2000 D/Q DEMO开发板，使能MIO

对应的配置项是，

- CONFIG_USE_LETTER_SHELL
- CONFIG_FREERTOS_USE_I2C
如果是E2000 D/Q DEMO开发板：
- CONFIG_FREERTOS_USE_MIO

本例子已经提供好具体的编译指令，以下进行介绍:
    1. make 将目录下的工程进行编译
    2. make clean  将目录下的工程进行清理
    3. make image   将目录下的工程进行编译，并将生成的elf 复制到目标地址
    4. make list_kconfig 当前工程支持哪些配置文件
    5. make load_kconfig LOAD_CONFIG_NAME=`<kconfig configuration files>`  将预设配置加载至工程中
    6. make menuconfig   配置目录下的参数变量
    7. make backup_kconfig 将目录下的sdkconfig 备份到./configs下

具体使用方法为:

- 在当前目录下
- 执行以上指令

### 2.3 构建和下载

#### 2.3.1 构建过程

- 在host侧完成配置
使用待测试平台对应的默认配置，例如在E2000D DEMO开发板上测试aarch64程序：
```
make load_kconfig LOAD_CONFIG_NAME=pe2202_aarch64_demo_i2c
```

- 选择例程需要的配置

```
make menuconfig
```

- 进行编译

```
make image 
```

- 将编译出的镜像放置到tftp目录下

```
make image
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

- 系统进入后，创建i2c初始化任务，创建i2c从机中断初始化，注册中断服务函数，创建i2c主机的读写任务函数。
- E2000 D/Q DEMO开发板支持i2c 读写RTC

```
i2c rtc
```

![rtc_example](figs/rtc_example.png)

- PhytiumPi开发板支持主从机进行通信，从设备控制器模拟eeprom

```
i2c ms_example
```

![ms_example](./figs/ms_example.png)

## 3. 如何解决问题

关于飞腾派上RTC问题，因未焊接DS1339芯片，所以未对飞腾派做例程操作支持。如果需要，可以参考现有的RTC例程，添加飞腾派配置即可。

## 4. 修改历史记录