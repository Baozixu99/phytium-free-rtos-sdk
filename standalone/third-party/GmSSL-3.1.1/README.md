# GmSSL


GmSSL是由北京大学自主开发的国产商用密码开源库，实现了对国密算法、标准和安全通信协议的全面功能覆盖，支持包括移动端在内的主流操作系统和处理器，支持密码钥匙、密码卡等典型国产密码硬件，提供功能丰富的命令行工具及多种编译语言编程接口。


## 主要特性

* 超轻量：GmSSL 3 大幅度降低了内存需求和二进制代码体积，不依赖动态内存，可以用于无操作系统的低功耗嵌入式环境(MCU、SOC等)，开发者也可以更容易地将国密算法和SSL协议嵌入到现有的项目中。
* 更合规：GmSSL 3 可以配置为仅包含国密算法和国密协议(TLCP协议)，依赖GmSSL 的密码应用更容易满足密码产品型号检测的要求，避免由于混杂非国密算法、不安全算法等导致的安全问题和合规问题。
* 更安全：TLS 1.3在安全性和通信延迟上相对之前的TLS协议有巨大的提升，GmSSL 3 支持TLS 1.3协议和RFC 8998的国密套件。GmSSL 3 默认支持密钥的加密保护，提升了密码算法的抗侧信道攻击能力。
* 跨平台：GmSSL 3 更容易跨平台，构建系统不再依赖Perl，默认的CMake构建系统可以容易地和Visual Studio、Android NDK等默认编译工具配合使用，开发者也可以手工编写Makefile在特殊环境中编译、剪裁。

## 下载

* GmSSL的主分支版本为 [GmSSL-3.1.1](https://github.com/guanzhi/GmSSL/releases/tag/v3.1.1)，主要增加跨平台特性，特别是对Phytium平台的开发者当前使用GmSSL 3.1.2 Dev该版本。

## 编译与安装

### phytium-standalone-sdk环境编译
* 详细编译过程，请参见飞腾裸机SDK环境下编译
* 完成gmssl example用例编译后，在example/system/gmssl/gmssl_example/build目录下生成lib_libgmssl.a文件。


```

## 主要功能

### 密码算法

* 分组密码：SM4 (CBC/CTR/GCM/ECB/CFB/OFB/CCM/XTS), AES (CBC/CTR/GCM)
* 序列密码：ZUC/ZUC-256, ChaCha20
* 哈希函数: SM3, SHA-1, SHA-224/256/384/512
* 公钥密码：SM2加密/签名, SM9加密/签名
* MAC算法：HMAC, GHASH, CBC-MAC
* 密钥导出函数：PBKDF2、HKDF
* 随机数生成器：PHYTIUM RAND

### 证书和数字信封

* 数字证书：X.509证书, CRL证书注销列表, CSR (PKCS #10) 证书签名请求
* 私钥加密：基于SM4/SM3口令加密的PEM格式私钥 (PKCS #8)
* 数字信封：SM2密码消息 (GM/T 0010-2012)

### SSL协议

* TLCP 1.1，支持密码套`TLS_ECC_SM4_CBC_SM3 {0xE0,0x13}` (GB/T 38636-2020、GM/T 0024-2014)
* TLS 1.2，支持密码套件`TLS_ECDHE_SM4_CBC_SM3 {0xE0,0x11}` (GB/T 38636-2020、GM/T 0024-2014)
* TLS 1.3，支持密码套件`TLS_SM4_GCM_SM3 {0x00,0xC6}`  (RFC 8998)



### 支持国密硬件

GmSSL内置支持飞腾硬件随机数支持，可联系https://gitee.com/phytium_embedded/phytium-standalone-sdk获取支持。



### OpenSSL兼容性

GmSSL 3.0版本重写了所有的代码并改变了原有API，因此当前GmSSL版本和OpenSSL不兼容，无法直接用GmSSL替换OpenSSL进行编译。GmSSL提供了子项目 OpenSSL-Compatibility-Layer (https://github.com/GmSSL/OpenSSL-Compatibility-Layer) 提供了OpenSSL的兼容层，Nginx等应用可以通过OpenSSL-Compatibility-Layer调用GmSSL功能。经过测试目前兼容层可以兼容Nginx 1.16 ~ 1.25 之间的版本。






```

## ChangeLog

自从3.1.1版本以来

* 提升了全部国密算法的性能，并在`tests`测试程序中增加了国密算法的性能测试
* 增加了SM4 ECB/CFB/OFB/CCM/XTS加密模式，带SM3-HMAC的SM4 CBC/CTR模式，并且在`gmssl`命令行工具中增加了所有SM4加密模式的选项
* 在`gmssl`命令行中增加了GHASH计算的选项
* 增加了`sdftest`正确性和兼容性测试命令，以独立子项目的形式提供了SDF的软件实现SoftSDF
* 移除了RC4, MD5等已被攻破的密码算法
* 可获得飞腾硬随机数支持(3.1.2版本)

## 开发者们
<a href="https://github.com/guanzhi/GmSSL/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=guanzhi/GmSSL" />
</a>

## Stargazers over time

[![Stargazers over time](https://starchart.cc/guanzhi/GmSSL.svg)](https://starchart.cc/guanzhi/GmSSL)

