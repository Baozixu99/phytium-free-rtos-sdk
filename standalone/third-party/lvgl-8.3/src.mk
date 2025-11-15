
		CSRCS_RELATIVE_FILES +=$(wildcard src/core/*.c) \
				$(wildcard src/draw/*.c) \
				$(wildcard src/font/lv_font_loader.c) \
				$(wildcard src/font/lv_font.c) \
				$(wildcard src/font/lv_font_fmt_txt.c) \
			    $(wildcard src/font/lv_font_montserrat_14.c) \
				$(wildcard src/hal/*.c) \
				$(wildcard src/draw/sw/*.c) \
				$(wildcard src/layouts/*.c) \
				$(wildcard src/libs/*.c) \
				$(wildcard src/misc/*.c)\
				$(wildcard src/themes/*.c) \
				$(wildcard src/widgets/*.c) \
				$(wildcard demos/benchmark/assets/*.c) \
				$(wildcard demos/benchmark/*.c)\
				$(wildcard demos/widgets/assets/*.c)\
				$(wildcard demos/widgets/*.c)\
				$(wildcard demos/stress/*.c)\

ifdef CONFIG_USE_BAREMETAL

CSRCS_RELATIVE_FILES += $(wildcard port/*.c) 

endif #CONFIG_USE_BAREMETAL

