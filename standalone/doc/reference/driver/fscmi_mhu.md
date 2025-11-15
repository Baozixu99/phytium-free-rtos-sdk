# SCMI_MHU 驱动程序

## 1. 概述

- SCMI_MHU(System Control and Management Interface)(Message Handling Unit)SCMI是用于系统管理的一组独立于操作系统的软件接口,MHU模块是SCP(System Control Processor)子系统的一个模块因为存在安全访问与非安全访问的区分所以采用APB4接口与SCP子系统和CPU核相连。

- AP与SCP之间的通信通过MHU与Shared memory实现。

- MHU模块主要实现CPU核与MCU通信通道的定义，分为物理通道和虚拟通道以及二者消息传递的控制功能，可实现双向通信。

```
 ______________________         ___________________________         __________________________
|Application processor |       |Message Handling Unit(MHU) |       |System Control Processor  |
|        (AP)          | <---> |       Shared  Memory      | <---> |        (SCP)             |
 ——————————————————————         ———————————————————————————         ——————————————————————————
```

## 2. 功能

SCMI_MHU 驱动程序主要完成SCMI协议和MHU模块的初始化、MHU模块的收发置位、协议的组包和内存读写解析函数，该驱动程序具备以下功能：

- 模块初始化
- 模块的设置
- 协议的组包
- 共享内存的解析

相关源文件为：

```
fscmi_mhu
    ├── fmhu_g.c
    ├── fnhu_hw.h
    ├── fmhu_intr.c(暂未实现)
    ├── fmhu.c
    ├── fmhu.h
    ├── fscmi_base.c
    ├── fscmi_base.h
	├── fscmi_clock.c
    ├── fscmi_clock.h
	├── fscmi_perf.c
    ├── fscmi_perf.h
	├── fscmi_power.c
    ├── fscmi_power.h
	├── fscmi_reset.c
    ├── fscmi_reset.h
    ├── fscmi_sensors.c
    ├── fscmi_sensors.h
    ├── fscmi.c
    └── fscmi.h
```

## 3. 配置方法

以下部分将指导您完成 SCMI_MHU 驱动的软件配置:

- 配置驱动程序，新建应用工程，make menuconfig使能SCMI_MHU驱动模块
- 设置配置参数，使用虚拟通道1，poll模式。（后续增加中断模式）
- 获取默认配置，设置使用MHU和shared memory地址
- 调用API函数，获取或者设置SCP参数

## 4. 应用示例


### [scmi_mhu](../../../example/system/scmi_mhu/README.md)


## 5. API参考


### 5.1. 用户数据结构

- drivers/scmi/fscmi_mhu/fscmi.h

- scmi_mhu实例配置

```c
typedef struct
{
    u32 is_ready; /* Device is ininitialized and ready*/
    struct FScmiConfig config;
    struct FScmiRevisionInfo revision; /* Base protocol */
	struct FScmiSensorsInfo sensors; /* Sensor protocol */
	struct FScmiPerfInfo perf;	/* Performance domain protocol */
	struct FScmiPowerInfo power; /* Power domain protocol */
	struct FScmiClockInfo clock; /* Clock domain protocol */
	struct FScmiResetInfo reset; /* Reset domain protocol */
	struct FScmiTransferInfo info[FSCMI_SUPPORT_PROTOCOL_NUM];
	u8 protocols_imp[FSCMI_MAX_PROTOCOLS_IMP];/* List of protocols implemented, currently maximum of FSCMI_MAX_PROTOCOLS_IMP elements allocated by the base protocol */
	FScmiMhu scmi_mhu;
} FScmi;
```

```c
struct FScmiConfig
{
	uintptr share_mem;	/* Chan transport protocol shared memory */
	u32 mbox_type;      /* select mbox driver */
};
```

- 协议消息结构类型

```c
struct FScmiMsgHdr 
{
	u8 id;              /* message id   */
	u8 protocol_id;     /* protocol id */
	u16 seq;            /* message token */ 
	u32 status;         /* protocal status */
};

struct FScmiMsg
{
	u8 buf[FSCMI_MSG_SIZE];          /* buffer in normal memory */
	fsize_t len;        /* buffer length */
};

struct FScmiTransferInfo 
{
    struct FScmiMsgHdr hdr ; /* Message(Tx/Rx) header */
    struct FScmiMsg tx ;
    struct FScmiMsg rx ;
    boolean poll_completion;
};
```

- 传感器信息结构

```c
struct FScmiSensorsInfo {
	u32 version;
	u16 major_ver;
    u16 minor_ver;
	u32 num_sensors;
	u32 max_requests;
	u64 reg_addr;
	u32 reg_size;
	struct FScmiSensorInfo sensor_info[FSCMI_MAX_NUM_SENSOR];/* TS0 TS1 */
};
```

```c
struct FScmiSensorInfo {
	u32 id;
	u8 type;
	char name[FSCMI_MAX_STR_SIZE];
};
```

- Performance domain protocol

```c
struct FScmiOpp {
	u32 perf;
	u32 power;
	u32 trans_latency_us;
};

struct FPerfDomInfo {
	boolean set_limits;
	boolean set_perf;
	boolean perf_limit_notify;
	boolean perf_level_notify;
	u32 opp_count;
	u32 sustained_freq_khz;
	u32 sustained_perf_level;
	u32 mult_factor;
	char name[FSCMI_MAX_STR_SIZE];
	struct FScmiOpp opp[FSCMI_MAX_OPPS];
};

struct FScmiPerfInfo {
	u32 version;
    u16 major_ver;
    u16 minor_ver;
	u32 num_domains;
	boolean power_scale_mw;
	u64 stats_addr;
	u32 stats_size;
	struct FPerfDomInfo dom_info[FSCMI_MAX_PERF_DOMAINS];
};

struct FPowerDomInfo {
	boolean state_change_notify;/*Power state change notifications support.*/
	boolean state_asyn; /*Power state asynchronous support.*/
	boolean state_syn; /*Power state synchronous support.*/
	boolean state_change_req_notify; /*Power state change requested notifications support.*/
	char name[FSCMI_MAX_STR_SIZE];
};

struct FScmiPowerInfo {
	u32 version; /* version of the SCMI power specification */
    u16 major_ver; /* major version of the SCMI power specification */
    u16 minor_ver; /* minor version of the SCMI power specification */
	u32 num_domains; /* Number of domains for which statistics are collected */
	u64 stats_addr; /* Address of the power domain statistics */
	u32 stats_size; /* The length in bytes of the statistics shared memory region. */
	struct FPowerDomInfo dom_info[FSCMI_MAX_POWER_DOMAINS];
};

struct FClockDomainInfo {
	char name[FSCMI_MAX_STR_SIZE]; /* Clock domain name */
	boolean rate_discrete; /* Discrete clock rate support */
	union 
	{
		struct {
			int num_rates; /* Number of discrete clock rates */
			u64 rates[FSCMI_MAX_CLOCK_RATES]; /* clock rates */
		} list;
		struct {
			u64 min_rate; /* Minimum clock rate */
			u64 max_rate; /* Maximum clock rate */
			u64 step_size; /* Clock rate step size */
		} range;
	};
};

struct FScmiClockInfo {
	u32 version; /* version of the SCMI clock specification */
    u32 num_clocks; /* number of clocks implemented */
	struct FClockDomainInfo clock_info[FSCMI_MAX_CLOCK_DOMAINS];
};

/* Reset domain protocol */
struct FResetDomainInfo {
	boolean async_reset;
	boolean reset_notify;
	u32 latency_us;
	char name[FSCMI_MAX_STR_SIZE];
};

struct FScmiResetInfo {
	u32 version; /* version of the SCMI reset specification */
	u32 num_domains; /* number of reset domains implemented */
	struct FResetDomainInfo dom_info[FSCMI_MAX_RESET_DOMAINS];
};

```

- 基础协议信息结构

```c
struct FScmiRevisionInfo /* base protocol */
{
    u32 version;
    u16 major_ver;
    u16 minor_ver;
	u8 num_protocols;
	u8 num_agents;
	u32 impl_ver;
	char vendor_id[FSCMI_MAX_STR_SIZE];
	char sub_vendor_id[FSCMI_MAX_STR_SIZE];
};
```

### 5.2  错误码定义

- FSCMI_ERROR_TYPE : 使用错误的驱动类型
- FSCMI_ERROR_RANGE : 超出设定值范围
- FSCMI_ERROR_NOT_FOUND : 没有找的对应的协议类型
- FSCMI_ERROR_NULL_POINTER : 空指针
- FSCMI_ERROR_WAIT_MBOX_TIMEOUT : mailbox超时
- FSCMI_ERROR_WAIT_MEM_TIMEOUT : 共享内存访问超时
- FSCMI_ERROR_FETCH_RESPONSE : 回复错误
- FSCMI_ERROR_REQUEST :	错误请求
- FSCMI_ERROR_VERSION : 版本号错误
- FSCMI_ERROR_INIT : 初始化错误

### 5.3. 用户API接口

- 配置scmi初始化参数

```c
FError FScmiCfgInitialize(FScmi *instance_p, const struct FScmiConfig *config);
```

- 获取传感器的信息

```c
FError FScmiSensorGetInfo(FScmi *instance_p);
```

- scmi协议消息的初始化

```c
FError FScmiMessageInit(FScmi *instance_p, u8 msg_id, u8 pro_id, u32 tx_size, u32 rx_size, u8 *tx_buffer);
```

- scmi协议发送准备，将初始化完成消息协议写入共享内存

```c
FError FScmiProtocolTxPrepare(FScmi *instance_p, u8 pro_id);
```

- 等待共享内存的协议完成

```c
FError FScmiProtocolPollDone(FScmi *instance_p, u8 pro_id);
```

- 获取共享内存SCP的回复数据

```c
FError FScmiFetchResponse(FScmi *instance_p, u8 pro_id);
```

- 获取对应协议的消息表指针

```c
struct FScmiTransferInfo *FScmiGetInfo(FScmi *instance_p, u8 pro_id);
```

- 进行传输

```c
FError FScmiDoTransport(FScmi *instance_p, struct FScmiTransferInfo *info, u32 protocol_index,boolean secu_flag);
```

- 传感器初始化

```c
FError FScmiSensorInit(FScmi *instance_p, boolean poll_completion);
```

- 传感器温度的获取，需要初始化成功

```c
FError FScmiSensorGetTemp(FScmi *instance_p, u32 sensor_id,s64 *temp);
```

- 性能域的初始化

```c
FError FScmiPerfInit(FScmi *instance_p, boolean poll_completion);
```

- 设置域性能

```c
FError FScmiDvfsFreqSet(FScmi *instance_p, u32 domain, u64 freq);
```

- 获取域性能

```c
FError FScmiDvfsFreqGet(FScmi *instance_p, u32 domain, u64 *freq);
```

- 设置域性能性能等级

```c
u64 FScmiPerfGetOppFreq(FScmi *instance_p, u32 domain, u32 opp_num);
```

- 电源域初始化

```c
FError FScmiPowerInit(FScmi *instance_p, boolean poll_completion);
```

- 设置域电源状态

```c
FError FScmiPowerStateSet(FScmi *instance_p,u32 asyn, u32 domain, u32 state);
```

- 获取域电源状态

```c
FError FScmiPowerStateGet(FScmi *instance_p,u32 asyn, u32 domain, u32 *state);
```

- 复位域初始化

```c
FError FScmiResetInit(FScmi *instance_p, boolean poll_completion);
```

- 复位域设置

```c
FError FScmiResetReset(FScmi *instance_p, u32 domain);
```

- 复位域复位

```c
FError FScmiResetAssert(FScmi *instance_p, u32 domain);
```

- 复位域复位状态解除

```c
FError FScmiResetDeassert(FScmi *instance_p, u32 domain);
```

- 时钟域初始化

```c
FError FScmiClockInit(FScmi *instance_p, boolean poll_completion);
```

- 时钟域时钟频率设置

```c
FError FScmiClockRateSet(FScmi *instance_p, u32 domain, u64 rate);
```

- 时钟域时钟频率获取

```c
FError FScmiClockRateGet(FScmi *instance_p, u32 domain, u64 *rate);
```
