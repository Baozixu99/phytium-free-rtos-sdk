

ifdef CONFIG_USE_LETTER_SHELL
$(BUILD_OUT_PATH)/lib_letter_shell.a: lib_letter_shell.a
lib_letter_shell.a: 
	$(call invoke_make_in_directory,third-party/letter-shell-3.1,makefile,all,COVERAGE_SRC_PATH="$(LETTER_SHELL_COVERAGE_SRC)")
lib_letter_shell_debug:
	$(call invoke_make_in_directory,third-party/letter-shell-3.1,makefile,debug,COVERAGE_SRC_PATH="$(LETTER_SHELL_COVERAGE_SRC)")
lib_letter_shell_info:
	$(call invoke_make_in_directory,third-party/letter-shell-3.1,makefile,compiler_info,COVERAGE_SRC_PATH="$(LETTER_SHELL_COVERAGE_SRC)")
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_letter_shell.a
$(BUILD_OUT_PATH)/lib_letter_shell.json:lib_letter_shell.json
lib_letter_shell.json:
	$(call invoke_make_in_directory,third-party/letter-shell-3.1,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)" COVERAGE_SRC_PATH="$(LETTER_SHELL_COVERAGE_SRC)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_letter_shell.json
endif

ifdef CONFIG_USE_LWIP
$(BUILD_OUT_PATH)/liblwip.a: liblwip.a
liblwip.a: 
	$(call invoke_make_in_directory,third-party/lwip-2.1.2,makefile,all,COVERAGE_SRC_PATH="$(LWIP_COVERAGE_SRC)")
liblwip_debug:
	$(call invoke_make_in_directory,third-party/lwip-2.1.2,makefile,debug,COVERAGE_SRC_PATH="$(LWIP_COVERAGE_SRC)")
liblwip_info:
	$(call invoke_make_in_directory,third-party/lwip-2.1.2,makefile,compiler_info,COVERAGE_SRC_PATH="$(LWIP_COVERAGE_SRC)")
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/liblwip.a
$(BUILD_OUT_PATH)/liblwip.json:liblwip.json
liblwip.json:
	$(call invoke_make_in_directory,third-party/lwip-2.1.2,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)",COVERAGE_SRC_PATH="$(LWIP_COVERAGE_SRC)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/liblwip.json
endif

ifdef CONFIG_USE_SFUD
$(BUILD_OUT_PATH)/libsfud.a: libsfud.a
libsfud.a: 
	$(call invoke_make_in_directory,third-party/sfud-1.1.0,makefile,all,)
libsfud_debug:
	$(call invoke_make_in_directory,third-party/sfud-1.1.0,makefile,debug,)
libsfud_info:
	$(call invoke_make_in_directory,third-party/sfud-1.1.0,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libsfud.a
$(BUILD_OUT_PATH)/libsfud.json:libsfud.json
libsfud.json:
	$(call invoke_make_in_directory,third-party/sfud-1.1.0,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libsfud.json
endif #CONFIG_USE_SFUD

$(BUILD_OUT_PATH)/libtlsf.a: libtlsf.a
libtlsf.a:
	$(call invoke_make_in_directory,third-party/tlsf-3.1.0,makefile,all,)
libtlsf_debug:
	$(call invoke_make_in_directory,third-party/tlsf-3.1.0,makefile,debug,)
libtlsf_info:
	$(call invoke_make_in_directory,third-party/tlsf-3.1.0,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libtlsf.a
$(BUILD_OUT_PATH)/libtlsf.json:libtlsf.json
libtlsf.json:
	$(call invoke_make_in_directory,third-party/tlsf-3.1.0,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libtlsf.json


ifdef CONFIG_USE_SPIFFS
$(BUILD_OUT_PATH)/libspiffs.a: libspiffs.a
libspiffs.a:
	$(call invoke_make_in_directory,third-party/spiffs-0.3.7,makefile,all,)
spiffs_debug:
	$(call invoke_make_in_directory,third-party/spiffs-0.3.7,makefile,debug,)
spiffs_iinfo:
	$(call invoke_make_in_directory,third-party/spiffs-0.3.7,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libspiffs.a
$(BUILD_OUT_PATH)/libspiffs.json:libspiffs.json
libspiffs.json:
	$(call invoke_make_in_directory,third-party/spiffs-0.3.7,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libspiffs.json
endif

# freemodbus-v1.6
ifdef CONFIG_USE_FREEMODBUS
$(BUILD_OUT_PATH)/libfreemodbus.a: libfreemodbus.a
libfreemodbus.a:
	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,all,)
freemodbus_debug:
	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,debug,)
freemodbus_info:
	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libfreemodbus.a
$(BUILD_OUT_PATH)/libfreemodbus.json:libfreemodbus.json
libfreemodbus.json:
	$(call invoke_make_in_directory,third-party/freemodbus-v1.6,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libfreemodbus.json
endif
# libmetal
ifdef CONFIG_USE_LIBMETAL
$(BUILD_OUT_PATH)/lib_libmetal.a: lib_libmetal.a
lib_libmetal.a:
	$(call invoke_make_in_directory,third-party/libmetal,makefile,all,)
lib_libmetal_debug:
	$(call invoke_make_in_directory,third-party/libmetal,makefile,debug,)
lib_libmetal_info:
	$(call invoke_make_in_directory,third-party/libmetal,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_libmetal.a
$(BUILD_OUT_PATH)/lib_libmetal.json:lib_libmetal.json
lib_libmetal.json:
	$(call invoke_make_in_directory,third-party/libmetal,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_libmetal.json
endif

# libgmssl
ifdef CONFIG_USE_GMSSL
$(BUILD_OUT_PATH)/lib_libgmssl.a: lib_libgmssl.a
lib_libgmssl.a:
	$(call invoke_make_in_directory,third-party/GmSSL-3.1.1,makefile,all,)
lib_libgmssl_debug:
	$(call invoke_make_in_directory,third-party/GmSSL-3.1.1,makefile,debug,)
lib_libgmssl_info:
	$(call invoke_make_in_directory,third-party/GmSSL-3.1.1l,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_libgmssl.a
$(BUILD_OUT_PATH)/lib_libgmssl.json:lib_libgmssl.json
lib_libgmssl.json:
	$(call invoke_make_in_directory,third-party/GmSSL-3.1.1,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_libgmssl.json
endif


# openamp
ifdef CONFIG_USE_OPENAMP
$(BUILD_OUT_PATH)/lib_openamp.a: lib_openamp.a
lib_openamp.a:
	$(call invoke_make_in_directory,third-party/openamp,makefile,all,)
lib_openamp_debug:
	$(call invoke_make_in_directory,third-party/openamp,makefile,debug,)
lib_openamp_info:
	$(call invoke_make_in_directory,third-party/openamp,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_openamp.a
$(BUILD_OUT_PATH)/lib_openamp.json:lib_openamp.json
lib_openamp.json:
	$(call invoke_make_in_directory,third-party/openamp,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_openamp.json
endif


# Crypto++
ifdef CONFIG_USE_CRYPTO_PLUS_PLUS
$(BUILD_OUT_PATH)/lib_crypto_pp.a: lib_crypto_pp.a
lib_crypto_pp.a:
	$(call invoke_make_in_directory,third-party/crypto++,makefile,all,)
lib_crypto_pp_debug:
	$(call invoke_make_in_directory,third-party/crypto++,makefile,debug,)
lib_crypto_pp_info:
	$(call invoke_make_in_directory,third-party/crypto++,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_crypto_pp.a
$(BUILD_OUT_PATH)/lib_crypto_pp.json:lib_crypto_pp.json
lib_crypto_pp.json:
	$(call invoke_make_in_directory,third-party/crypto++,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_crypto_pp.json
endif


ifdef CONFIG_USE_LVGL
$(BUILD_OUT_PATH)/lib_lv.a: lib_lv.a
lib_lv.a:
	$(call invoke_make_in_directory,third-party/lvgl-8.3,makefile,all,)
lib_lv_debug:
	$(call invoke_make_in_directory,third-party/lvgl-8.3,makefile,debug,)
lib_lv_info:
	$(call invoke_make_in_directory,third-party/lvgl-8.3,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_lv.a
$(BUILD_OUT_PATH)/lib_lv.json:lib_lv.json
lib_lv.json:
	$(call invoke_make_in_directory,third-party/lvgl-8.3,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_lv.json
endif

# FatFs_0_1_4
ifdef CONFIG_USE_FATFS_0_1_4
$(BUILD_OUT_PATH)/lib_fatfs.a: lib_fatfs.a
lib_fatfs.a:
	$(call invoke_make_in_directory,third-party/fatfs-0.1.4,makefile,all,)
lib_fatfs_debug:
	$(call invoke_make_in_directory,third-party/fatfs-0.1.4,makefile,debug,)
lib_fatfs_info:
	$(call invoke_make_in_directory,third-party/fatfs-0.1.4,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_fatfs.a
$(BUILD_OUT_PATH)/lib_fatfs.json:lib_fatfs.json
lib_fatfs.json:
	$(call invoke_make_in_directory,third-party/fatfs-0.1.4,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_fatfs.json
endif

# fsl sdmmc
ifdef CONFIG_USE_FSL_SDMMC
$(BUILD_OUT_PATH)/libfslsdmmc.a: libfslsdmmc.a
libfslsdmmc.a:
	$(call invoke_make_in_directory,third-party/fsl_sdmmc,makefile,all,)
libfslsdmmc_debug:
	$(call invoke_make_in_directory,third-party/fsl_sdmmc,makefile,debug,)
libfslsdmmc_info:
	$(call invoke_make_in_directory,third-party/fsl_sdmmc,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libfslsdmmc.a
$(BUILD_OUT_PATH)/libfslsdmmc.json:libfslsdmmc.json
libfslsdmmc.json:
	$(call invoke_make_in_directory,third-party/fsl_sdmmc,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libfslsdmmc.json
endif

ifdef CONFIG_USE_MICROPYTHON
$(BUILD_OUT_PATH)/lib_mpy.a: lib_mpy.a
lib_mpy.a:
	$(call invoke_make_in_directory,third-party/micropython,makefile,all,)
lib_mpy_debug:
	$(call invoke_make_in_directory,third-party/micropython,makefile,debug,)
lib_mpy_info:
	$(call invoke_make_in_directory,third-party/micropython,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_mpy.a
$(BUILD_OUT_PATH)/lib_mpy.json:lib_mpy.json
lib_mpy.json:
	$(call invoke_make_in_directory,third-party/micropython,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_mpy.json
endif

ifdef CONFIG_USE_TINYMAIX
$(BUILD_OUT_PATH)/lib_tinymaix.a: lib_tinymaix.a
lib_tinymaix.a:
	$(call invoke_make_in_directory,third-party/TinyMaix,makefile,all,)
lib_tinymaix_debug:
	$(call invoke_make_in_directory,third-party/TinyMaix,makefile,debug,)
lib_tinymaix_info:
	$(call invoke_make_in_directory,third-party/TinyMaix,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_tinymaix.a
$(BUILD_OUT_PATH)/lib_tinymaix.json:TinyMaix.json
lib_tinymaix.json:
	$(call invoke_make_in_directory,third-party/TinyMaix,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_tinymaix.json
endif

ifdef CONFIG_USE_CHERRY_USB
$(BUILD_OUT_PATH)/lib_cherryusb.a: lib_cherryusb.a
lib_cherryusb.a:
	$(call invoke_make_in_directory,third-party/cherryusb,makefile,all,)
lib_cherryusb_debug:
	$(call invoke_make_in_directory,third-party/cherryusb,makefile,debug,)
lib_cherryusb_info:
	$(call invoke_make_in_directory,third-party/cherryusb,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_cherryusb.a
$(BUILD_OUT_PATH)/lib_cherryusb.json:lib_cherryusb.json
lib_cherryusb.json:
	$(call invoke_make_in_directory,third-party/cherryusb,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_cherryusb.json
endif

ifdef CONFIG_USE_CMSIS_DSP
$(BUILD_OUT_PATH)/lib_cmsis_dsp.a: lib_cmsis_dsp.a
lib_cmsis_dsp.a:
	$(call invoke_make_in_directory,third-party/CMSIS/DSP,makefile,all,)
lib_cmsis_dsp_debug:
	$(call invoke_make_in_directory,third-party/CMSIS/DSP,makefile,debug,)
lib_cmsis_dsp_info:
	$(call invoke_make_in_directory,third-party/CMSIS/DSP,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/lib_cmsis_dsp.a
$(BUILD_OUT_PATH)/lib_cmsis_dsp.json:lib_cmsis_dsp.json
lib_cmsis_dsp.json:
	$(call invoke_make_in_directory,third-party/CMSIS,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/lib_cmsis_dsp.json
endif

ifdef CONFIG_USE_ACPICA
$(BUILD_OUT_PATH)/libacpica.a: libacpica.a
libacpica.a:
	$(call invoke_make_in_directory,third-party/acpica,makefile,all,)
libacpica_debug:
	$(call invoke_make_in_directory,third-party/acpica,makefile,debug,)
libacpica_info:
	$(call invoke_make_in_directory,third-party/acpica,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libacpica.a
$(BUILD_OUT_PATH)/libacpica.json:libacpica.json
libacpica.json:
	$(call invoke_make_in_directory,third-party/acpica,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libacpica.json
endif

ifdef CONFIG_USE_CJSON
$(BUILD_OUT_PATH)/libcjson.a: libcjson.a
libcjson.a:
	$(call invoke_make_in_directory,third-party/cJSON-1.7.19,makefile,all,)
libcjson_debug:
	$(call invoke_make_in_directory,third-party/cJSON-1.7.19,makefile,debug,)
libcjson_info:
	$(call invoke_make_in_directory,third-party/cJSON-1.7.19,makefile,compiler_info,)
BAREMETAL_LIBS+= $(BUILD_OUT_PATH)/libcjson.a
$(BUILD_OUT_PATH)/libcjson.json:libcjson.json
libcjson.json:
	$(call invoke_make_in_directory,third-party/cJSON-1.7.19,makefile,json,SDK_PYTHON_TOOLS_DIR="$(SDK_PYTHON_TOOLS_DIR)")
BAREMETAL_IDE_JSON += $(BUILD_OUT_PATH)/libcjson.json
endif