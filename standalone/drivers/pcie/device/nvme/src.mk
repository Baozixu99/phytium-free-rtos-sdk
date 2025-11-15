

ifdef CONFIG_ENABLE_FPCIE_ECAM
DRIVERS_CSRCS += \
    nvme/nvme_cmd.c \
    nvme/nvme_controller.c \
    nvme/nvme_controller_cmd.c \
    nvme/nvme_namespace.c \
    nvme/nvme_disk.c \
    nvme/nvme_intr.c
endif
