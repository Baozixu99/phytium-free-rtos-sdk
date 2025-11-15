ACPICA_INC_DIR := $(SDK_DIR)/third-party/acpica/acpica
BUILD_INC_PATH_DIR += $(ACPICA_INC_DIR)/include \
					  $(ACPICA_INC_DIR)/include/platform \
					  $(ACPICA_INC_DIR)/compiler

BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party \
					  $(SDK_DIR)/third-party/acpica \
					  $(SDK_DIR)/third-party/acpica/os/include \
					  $(SDK_DIR)/third-party/acpica/drivers