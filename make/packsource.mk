
ifeq ($(MAKECMDGOALS),pack)
ifndef OUTPATH
$(error error, please type in 'make pack OUTPATH=<dir>'!!)
endif
DST_PACK_PATH ?= $(OUTPATH)/$(CONFIG_TARGET_NAME)
TAR_PACK_PATH ?= $(OUTPATH)/$(CONFIG_TARGET_NAME).tar.gz
# remove cur dir '.'
EXCL_PATH ?= . 
SRC_PACK_PATH := $(filter-out $(EXCL_PATH), $(SRC_DIR))
# add common dir for pack
SRC_PACK_PATH += $(FREERTOS_STANDALONE)	  \
				$(FREERTOS_STANDALONE)/make \
				$(FREERTOS_STANDALONE)/bsp  \
				$(FREERTOS_STANDALONE)/lib  \
				$(PWD)	
# filter src files if you do not need
SRC_PACK_FILES ?= $(foreach dir, $(SRC_PACK_PATH), $(wildcard $(dir)/*))
endif

pack:	
	$(foreach dir, $(SRC_PACK_PATH), if [ -d $(dir) ]; then sudo mkdir -p $(dir) $(dir:$(FREERTOS_STANDALONE)/%=$(DST_PACK_PATH)/%);fi;)
	$(foreach file, $(SRC_PACK_FILES), if [ -f $(file) ]; then sudo cp -f $(file) $(file:$(FREERTOS_STANDALONE)/%=$(DST_PACK_PATH)/%);fi;)
	$(shell sudo tar -zcvf $(TAR_PACK_PATH) $(DST_PACK_PATH))
	@echo "*****************************"
	@echo "source code from: " $(SRC_PACK_PATH)
	@echo "*****************************"
	@echo "source copy to: " $(DST_PACK_PATH)
	@echo "*****************************"
	@echo "pack source code as: " $(OUTPATH)/$(CONFIG_TARGET_NAME).tar.gz
