# user code
$(BUILD_OUT_PATH)/libuser.a: libuser.a
libuser.a: 
	$(call invoke_make_in_directory,board/user,makefile,all,USER_CSRC="$(USER_CSRC)" USER_ASRC="$(USER_ASRC)" USER_CXXSRC="$(USER_CXXSRC)" USER_INCLUDE_PATH="$(USER_INCLUDE)" LIBS_NAME=libuser EXTEND_CSRC="$(EXTEND_CSRC)" EXTEND_ASRC="$(EXTEND_ASRC)" EXTEND_CXXSRC="$(EXTEND_CXXSRC)")
libuser_debug:
	$(call invoke_make_in_directory,board/user,makefile,debug,USER_CSRC="$(USER_CSRC)" USER_ASRC="$(USER_ASRC)" USER_CXXSRC="$(USER_CXXSRC)" USER_INCLUDE_PATH="$(USER_INCLUDE)" EXTEND_CSRC="$(EXTEND_CSRC)" EXTEND_ASRC="$(EXTEND_ASRC)" EXTEND_CXXSRC="$(EXTEND_CXXSRC)")
libuser_info:
	$(call invoke_make_in_directory,board/user,makefile,compiler_info,USER_CSRC="$(USER_CSRC)" USER_ASRC="$(USER_ASRC)" USER_CXXSRC="$(USER_CXXSRC)" USER_INCLUDE_PATH="$(USER_INCLUDE)" EXTEND_CSRC="$(EXTEND_CSRC)" EXTEND_ASRC="$(EXTEND_ASRC)" EXTEND_CXXSRC="$(EXTEND_CXXSRC)")
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libuser.a
$(BUILD_OUT_PATH)/libuser.json:libuser.json
libuser.json:
	$(call invoke_make_in_directory,board/user,makefile,json,USER_CSRC="$(USER_CSRC)" USER_ASRC="$(USER_ASRC)" USER_CXXSRC="$(USER_CXXSRC)" USER_INCLUDE_PATH="$(USER_INCLUDE)" LIBS_NAME=libuser EXTEND_CSRC="$(EXTEND_CSRC)" EXTEND_ASRC="$(EXTEND_ASRC)" EXTEND_CXXSRC="$(EXTEND_CXXSRC)" SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libuser.json

# board code
$(BUILD_OUT_PATH)/lib_board.a: lib_board.a
lib_board.a:
	$(call invoke_make_in_directory,board,makefile,all,)
lib_board_debug:
	$(call invoke_make_in_directory,board,makefile,debug,)
lib_board_info:
	$(call invoke_make_in_directory,board,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_board.a
$(BUILD_OUT_PATH)/lib_board.json:lib_board.json
lib_board.json:
	$(call invoke_make_in_directory,board,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_board.json



AMP_PACK_FILE = $(shell [ -f $(BUILD_OUT_PATH)/amp_packed.c ] && echo '$(BUILD_OUT_PATH)/amp_packed.c' || echo '')
AMP_PACK_OBJS :=  $(AMP_PACK_FILE:.c=.o)
AMP_IMG_EXPORT_IMG = $(shell [ -f $(PROJECT_DIR)/packed.bin ] && echo '$(PROJECT_DIR)/packed.bin' || echo '')

ak_test:
	@echo "BUILD_OUT_PATH is $(BUILD_OUT_PATH)"
	@echo "AMP_PACK_FILE is $(AMP_PACK_FILE)"
	@echo "AMP_PACK_OBJS is $(AMP_PACK_OBJS)"
	@echo "AMP_IMG_EXPORT_IMG is $(AMP_IMG_EXPORT_IMG)"

$(AMP_PACK_OBJS):$(AMP_PACK_FILE) $(AMP_IMG_EXPORT_IMG)
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -s -c   $(AMP_PACK_FILE) -o $(AMP_PACK_OBJS)
	$(OBJCOPY) --add-section .my_amp_img=$(AMP_IMG_EXPORT_IMG) --set-section-flags .my_amp_img=alloc,load,readonly $(AMP_PACK_OBJS)

EXTRA_OBJS += $(AMP_PACK_OBJS)
