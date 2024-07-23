# 多元异构系统部署

## 1. 例程介绍

> `<font size="1">`介绍例程的用途，使用场景，相关基本概念，描述用户可以使用例程完成哪些工作 `</font><br />`

1. 随着硬件技术的快速发展，嵌入式芯片的硬件能力越来越强大，从单核到多核、从多核到异构多核，为在一个片上系统（SoC）中部署多个OS提供了坚实的物理基础。多元异构系统部署是为了在多核片上实现多个操作系统镜像打包并同时运行，包含了裸机、freertos等多种实现形态。不同的形态有各自的特点，例如裸机可以得到最佳的性能、嵌入式系统具有更好的易用性与灵活性等等。实践中也往往需要将不同形态根据用户的需求构建选项（芯片类型，功能特性等等），创建出定制化的镜像文件。
2. 部署方案可以在飞腾E2000Q/D系列，PHYTIUMPI，D2000上进行部署操作，但例程实际是否可运行需要具体到芯片是否支持，可以查阅例程目录的 ./configs 配置项
3. 部署方案可以指定程序运行在那个核心，所以也支持单例程任务镜像运行在设定核心(特别是E2000Q、PHYTIUMPI这类大小核心配置，boot启动运行默认在小核心，但是需要程序运行在大核的需求)。
4. 通过多重打包技术，可以将不同形态的程序打包到一个镜像文件中。
5. 适配openamp的动态加载过程，可以实现openamp生命周期管理，动态加载镜像文件，并在运行过程中动态加载用户态程序。
6. 自由组合，可以自由组合不同形态的程序，例如将裸机程序、freertos程序、linux程序等等同时运行在同一个CPU上。

> 注：目前freertos 1个例程 `openamp` 支持(更多例程可参阅裸机standalone [Phytium-Standalone-SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk))，`openamp_for_linux` 暂时需要分开生成单个镜像用于linux系统加载。

7. 例程介绍：

- `openamp`：本例程演示了如何使用openamp库在多核片上运行。
- `openamp_for_linux`：本例程演示了如何使用编译好的基于freertos的 openamp例程镜像，在linux系统加载下运行(需要linux系统支持)。

> 注：建议学习顺序：先裸机standalone，再freertos，最后linux。

## 2. 资源分配

主要集中在common文件中，以使用openamp库的例程为例，包含三个文件：

- `libmetal_configs.h`：包含了libmetal库的配置，如Device name、irq_info等。
- `memory_layout.h`：包含了openamp多核在系统内存分配，如共享内存位置、内存属性、资源表位置、vrings传输通道等。
- `openamp_configs.h`：包含了openamp库的配置，如rpmsg的name service、address等。

## 3. 构建方式

- 参阅裸机standalone [Phytium-Standalone-SDK](https://gitee.com/phytium_embedded/phytium-standalone-sdk)
- 文档路径：`/example/system/amp/README.md`
- 编译命令见具体例程的README.md文件
