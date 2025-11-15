BOARD_CSRC += phytiumpi_firefly/fboard_init.c \
				phytiumpi_firefly/fio_mux.c

ifdef CONFIG_FSL_SDMMC_USE_FSDIF
BOARD_CSRC += phytiumpi_firefly/fsdif_timing.c
endif