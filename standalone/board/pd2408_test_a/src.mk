


BOARD_CSRC += pd2408_test_a/fboard_init.c \
			  	          pd2408_test_a/fio_mux.c

ifeq ($(CONFIG_FSL_SDMMC_USE_FSDIF)$(CONFIG_FSL_SDMMC_USE_FSDIF_V2),y)
BOARD_CSRC += pd2408_test_a/fsdif_timing.c
endif