
BOARD_CSRC += pd2308_demo/fboard_init.c \
				pd2308_demo/fio_mux.c

ifdef CONFIG_FSL_SDMMC_USE_FSDIF
BOARD_CSRC += pd2308_demo/fsdif_timing.c
endif