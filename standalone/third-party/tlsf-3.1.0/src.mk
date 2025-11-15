
CSRCS_RELATIVE_FILES += $(wildcard src/*.c) 
 					
ifdef CONFIG_USE_BAREMETAL

CSRCS_RELATIVE_FILES += $(wildcard port/*.c) 

endif #CONFIG_USE_BAREMETAL
