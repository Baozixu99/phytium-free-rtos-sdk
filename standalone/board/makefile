include $(PROJECT_DIR)/sdkconfig

# Use E2000Q demo board
ifdef CONFIG_E2000Q_DEMO_BOARD
include e2000q_demo/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

# Use E2000D demo board
ifdef CONFIG_E2000D_DEMO_BOARD
include e2000d_demo/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

# Use firefly demo board
ifdef CONFIG_PHYTIUMPI_FIREFLY_BOARD
include phytiumpi_firefly/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

# Use d2000 test board
ifdef CONFIG_D2000_TEST_BOARD
include d2000_test/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

# Use ft2004 dsk board
ifdef CONFIG_FT2004_DSK_BOARD
include ft2004_dsk/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

include port/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 

# Use pd2308 demo board
ifdef CONFIG_PD2308_DEMO_BOARD
include pd2308_demo/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

# Use PD2408 test a board
ifdef CONFIG_PD2408_TEST_A_BOARD
include pd2408_test_a/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif
# Use PD2408 test b board
ifdef CONFIG_PD2408_TEST_B_BOARD
include pd2408_test_b/src.mk
CSRCS_RELATIVE_FILES := $(BOARD_CSRC) 
endif

include $(SDK_DIR)/tools/build/compiler.mk
