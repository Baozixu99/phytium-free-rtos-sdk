

ARCH_CSRCS += common/fcpp_setup.c common/fpsci.c common/fgic_v3.c common/fpmu_perf.c common/fboot_core.c

ifeq ($(CONFIG_SOC_NAME),"pd2008")
ARCH_CSRCS += common/fl3cache.c
endif

ifeq ($(CONFIG_SOC_NAME),"pd1904")
ARCH_CSRCS += common/fl3cache.c
endif

ifeq ($(CONFIG_ENABLE_GIC_ITS),y)
ARCH_CSRCS += common/fgic_its.c
endif

ifeq ($(CONFIG_ENABLE_GDB_STUB),y)
ARCH_CSRCS += common/gdb/fgdb_packet.c \
			  common/gdb/fgdb_thread.c \
			  common/gdb/fgdb_uart.c \
			  common/gdb/fgdb_main.c \
			  common/gdb/fgdb.c
endif