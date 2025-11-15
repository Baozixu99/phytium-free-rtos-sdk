BOARD_CSRC += pd2408_test_b/fboard_init.c \
				pd2408_test_b/fio_mux.c
ifdef CONFIG_FSL_SDMMC_USE_FSDIF
ifeq ($(CONFIG_FSL_SDMMC_USE_FSDIF)$(CONFIG_FSL_SDMMC_USE_FSDIF_V2),y)
BOARD_CSRC += pd2408_test_b/fsdif_timing.c
endif
endif