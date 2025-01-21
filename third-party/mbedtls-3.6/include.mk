ifdef CONFIG_USE_MBEDTLS

THIRDP_CUR_DIR := $(FREERTOS_SDK_DIR)/third-party

# src files
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/mbedtls-3.6
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/mbedtls-3.6/include
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/mbedtls-3.6/include/mbedtls
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/mbedtls-3.6/library
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/mbedtls-3.6/ports
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/mbedtls-3.6/ports/inc
BUILD_INC_PATH_DIR += $(THIRDP_CUR_DIR)/mbedtls-3.6/ports/src

endif #CONFIG_USE_MBEDTLS