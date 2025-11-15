# SEC 驱动程序

## 1. 概述


- 本驱动SEC程序提供了飞腾平台的安全组件SCE密码引擎部件，主要由SCA、ACA、HASH、PTRNG四个功能模块组成。
-   1、SCA 为对称密码加速器，支持分组加密密算法 AES128/192/256、DES、3DES、SM4。
-   2、ACA 为非对称密码加速器，支持 RSA 非对称加密算法、dh 与 ecdh 密钥识别算法、SM2 算法。
-   3、HASH 为哈希算法加速器，支持分组加密算法 SHA1/224/256/384/512、MD5、SM3，工作模式支持普通工作模式和HMAC工作模式。
-   4、PTRNG 为物理真随机数生成器，包括熵源、采样块、后处理、评估与测试、随机数池五个主要块。
- SEC程序rnd驱动支持PE220x平台(包括PE220x的Q/Ddemo板、飞腾派)、PD2408平台的ptrng随机数生成功能。
- SEC程序sha驱动支持PE220x平台(包括PE220x的Q/Ddemo板、飞腾派)。

.
├── fsec_rnd.c
├── fsec_rnd.h
├── fsec_rnd_debug.c
├── fsec_rnd_g.c
├── fsec_rnd_hw.h
├── fsec_rnd_init.c
├── fsec_rnd_util.c
└── fsec_rnd_util.h
.
├── fsec_sha.c
├── fsec_sha.h
├── fsec_sha_hw.h
├── fsec_sha_utils.c
└── fsec_sha_utils.h

## 2. 驱动功能

SEC下rnd驱动程序管理在phytium随机数生成：

- 支持物理池产生随机数
- 支持自动或手动触发模式

SEC下sha驱动程序管理在摘要hash生成：

- 支持HASH下shaX(sha1、sha128、sha224、sha256、sha384、sha512）、md5、sm3多种算法
- 支持手动输入数据生成摘要、update、finish

## 3. 使用方法

以下部分将指导您完成SEC下rnd驱动：

- 1. SEC下rnd驱动支持PE220x的Q/D板完成测试，支持PD2308的配置
- 2. SEC下rnd驱动支持PE2408平台的随机数配置及验证

以下部分将指导您完成SEC下sha驱动：

- 1. SEC下sha驱动支持PE220x的Q/D板完成测试


## 4. 应用示例

### [fsec](../../../example/peripherals/sec/rnd/README.md)

### [fsec](../../../example/peripherals/sec/sha/README.md)

## 5. API参考


### 5.1. 用户数据结构

- peripherals/sec/rnd/fsec_rnd.h

```c

typedef struct {
#ifndef SDT
	u16 device_id;		/**< device_id 0 = pe220x平台； 1 = pd2408平台*/
#else
	char *name;
#endif
	uintptr_t base_address;	/**< base_address is the physical base address	*  of the device's registers */
} FSecRndConfig;

```

- SEC驱动RND实例配置

```c

typedef struct {
	FSecRndConfig config; /**< Hardware Configuration */
	FSecRndUsrConfig usr_cfg; /**< Configuration from the user */
	FSecRndStats trng_stats; /**< FSECRND Statistics */
	FSecRndCtrl_State state; /**< takes one of the possible	 values 	indicated by FSecRndCtrl_State */
	u32 rand_bit_buf[FSEC_RND_BUF_LEN]; /**< buffer of random bits to minimize latency */
	u32 entropy_size; /**< actual size (in bytes) of seed */
	FSecRndDFInput df_input; /**< data structure which is input	 to the DF operation */
	u8 df_output[FSEC_RND_SEED_LEN_BYTES]; /**< Output of the DF operation	 (contains updated seed or random number) */
} FSecRndCtrl;
```

- SEC驱动SHA实例配置

```c

	typedef struct
	{
		FSecDmaDev *csu_dma_ptr; /**< Pointer to FSEC DMA Instance */
		u32 baseaddress;				 /**< Device Base Address */
		u32 islastupdate;                /*is last update flag*/
		u32 link_mode;                   /*single sg mode*/
		u32 actx_block_size;             /*block size */
		u32 actx_alg;                    /*alg name */
		u32 actx_mode;                   /*normal hmac */
		u32 actx_digest_size;            /*gigest size */
		uintptr_t actx_key_ioaddr;             /*key ipput address */
		u32 key_len;                     /*key lenth */
		u8 *iv_vaddr_base;               /*slat iv addr */
		u8 hash_flag;                    /*hash todo flag */
		u8 *state_ioaddr;                /*temp data */
		uintptr_t buffer_ioaddr;               /*input be processed data address*/
		u32 buffer_data_len;             /*input be processed data len*/
		uintptr_t list_buf_ioaddr;             /*input be processed data sg address*/
		struct link_list_item list_buf[32 + 2]; /*input be processed data item list address*/
		FSecShaState shastate;           /*sha control state*/
	} FSecShaCtrl;

```

- SEC驱动RND真随机数生成器驱动实例

### 5.2  错误码定义

- RND驱动错误码

#define FSECRND_SUCCESS                   : 成功
#define FSECRND_ERR_FAILURE                  : 常规报错
#define FSECRND_ERR_TIMEOUT               : 数据接收延时报错
#define FSECRND_ERR_GLITCH                : 小故障报错
#define FSECRND_ERR_INVALID_PARAM         : 无效参数报错

- SHA驱动错误码

#define FSHA_ERR_INVAL_PARM               : 常规参数报错
#define FSHA_ERR_NOT_READY                : 未准备完成报错
#define FSHA_ERR_TIMEOUT                  : 超时报错
#define FSHA_ERR_NOT_SUPPORT              : 状态不支持报错
#define FSHA_ERR_INVAL_STATE              : 无效状态报错
#define FSHA_ERR_FAILURE                  : 常规报错


### 5.3 用户API接口

---- SEC驱动RND API接口

```c
FSecRndConfig *FSecRndLookupConfig(u16 device_id);
```
- 获取SEC下rnd驱动的默认配置参数

Note:
    
    - 用户需要修改配置参数时，作为FSecRndLookupConfig函数的入参，

Input:

    - u16 device_id, 当前控制的SEC控制器实例号    

Return:

    - FSecRndConfig *, 返回驱动默认参数， NULL表示失败


```c
s32 FSecRndCfgInitialize(FSecRndCtrl *instance_p, const FSecRndConfig *cfgptr,uintptr_t effective_addr);
```
- 完成SEC下rnd驱动实例的初始化，使之可以使用

Note:

    - 此函数会将驱动实例中的所有数据全部重置

Input:

    - FSecRndCtrl *instance_p, rnd驱动实例数据 
    
    - const FSecRndConfig *cfgptr， rnd驱动配置数据 

		- uintptr_t effective_addr， rnd驱动soc寄存器地址 

Return:

    - FError，参考sec错误码定义

```c
s32 FSecRndInstantiate(FSecRndCtrl *instance_p, const FSecRndUsrConfig *configur_values);
```

- 完成SEC下rnd驱动实例的配置参数

Note:

    - 此函数会将驱动实例中配置随机数的配置数据，

Input:

    - FSecRndCtrl *instance_p, SEC下rnd驱动实例数据 

		- const FSecRndUsrConfig *configur_values，用户随机数的如种子等其它配置结构体实例数据

Return:

    - FError，参考驱动sec错误码定义表
    
```c
s32 FSecRndGenerate(FSecRndCtrl *instance_p, u8 *rand_buf_ptr, u32 rand_buf_size, u8 pred_resistance_en, u32 rand_size);;
```
- 完成SEC下rnd驱动随机数产生


Input:
    - @param {rnd} *instance_p , SEC下rnd驱动实例数据 
    - @param {u8} rand_buf_ptr ,存放随机生成指针
    - @param {u32} rand_buf_size, 默认随机数4096
    - @param {u8} pred_resistance_en ,随机数池状态控制
    - @param {u32} rand_size ,实际获取进入缓冲区的长度

Return:
    - FError，参考驱动sec错误码定义表

```c
s32 FSecRndUninstantiate(FSecRndCtrl *instance_p);
```

- SEC下rnd驱动实例的配置参数重置

Note:

    - 此函数会将驱动实例中配置随机数的配置数据复位为0，

Input:

    - FSecRndCtrl *instance_p, SEC下rnd驱动实例数据 

Return:

    - FError，参考驱动sec错误码定义表

```c
s32 FSecRndRunHealthTest(FSecRndCtrl *instance_p);
```

- SEC下rnd驱动实例的配置参数测试

Note:

    - 此函数会将驱动实例中配置参数满足性健康测试，比如模式选择、随机数存储池状态等

Input:

    - FSecRndCtrl *instance_p, SEC下rnd驱动实例数据指针句柄 

Return:

    - FError，参考驱动sec错误码定义表


---- SEC驱动SHA API接口

```c
void FSecShaStart(FSecShaCtrl *instance_p);
```
- 获取SEC下sha驱动的默认配置参数

Note:
    
    - sha驱动参数变量的初始化，

Input:

    - FSecShaCtrl *instance_p, SEC下sha驱动实例数据指针句柄    

Return:

    - 无

```c
int FSecShaInit(FSecShaCtrl *instance_p, uintptr_t state_ioaddr, uintptr_t buffer_ioaddr,uintptr_t list_buf_ioaddr);
```
- 获取SEC下sha驱动的init准备数据

Note:
    
    - sha驱动参数变量的初始化，

Input:

    - @param {sha} FSecShaCtrl *instance_p, SEC下sha驱动实例数据指针句柄    
    - @param {uintptr_t}  state_ioaddr ,待处理数据存放地址
    - @param {uintptr_t}  buffer_ioaddr, 待导入key值数据地址
    - @param {uintptr_t}  list_buf_ioaddr, 导入list缓存区域地址
Return:

    - 无

```c
FError FSecShaUpdate(FSecShaCtrl *instance_p, const u8 *data,	const u32 size);
```  
- 获取SEC下sha驱动会根据当前输入更新内部状态

Note:
    
    - sha驱动会维护哈希计算的中间状态

Input:
    - @param {sha} *instance_p , SEC下sha驱动实例数据 
    - @param {u8} *data ,处理数据存放指针
    - @param {u32} size, 处理数据大小

Return:

    - FError，参考sec错误码定义 

```c
FError FSecShaFinish(FSecShaCtrl *instance_p, u8 *hash);
```  
- 获取SEC下sha驱动内部状态会被转换为最终的哈希值

Note:
    
    - sha驱动会维护哈希计算的最终哈希值，

Input:
    - @param {sha} *instance_p , SEC下sha驱动实例数据 
    - @param {u8} *hash ,最终哈希值存放指针

Return:

    - FError，参考sec错误码定义 

```c
FError FSceHashSetKey(FSecShaCtrl *instance_p, const u8 *key, unsigned int keylen);
```  
- 获取SEC下sha驱动根据normal或hmac，使用设置key值函数

Note:
    
    - sha驱动会通过设置key值，最终支持HMAC消息认证码功能

Input:
    - @param {sha} *instance_p , SEC下sha驱动实例数据 
    - @param {u8} *key ,处理数据存放指针
    - @param {u32} keylen, 处理数据大小

Return:

    - FError，参考sec错误码定义 

```c
FError FSecShaXDigest(FSecShaCtrl *instance_p, const u8 *indataptr, const u32 indatalen, u8 *outdataptr);
```  
- 获取SEC下sha驱动会根据初始化（Init）→ 更新（Update）→ 最终化（Final）。EVP_DigestFinal() 作为最终阶段，生成固定长度的哈希值

Note:
    
    - sha驱动会生成最终的消息摘要，并为上层密码学操作提供基础支持

Input:
    - @param {sha} *instance_p , SEC下sha驱动实例数据 
    - @param {u8} *indataptr ,处理数据存放指针
    - @param {u32} indatalen, 处理数据大小
    - @param {u8} *outdataptr, 输出hash处理后数据指针

Return:

    - FError，参考sec错误码定义 
 