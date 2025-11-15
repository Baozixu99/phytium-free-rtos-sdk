SHELL_CSRCS = shell.c \
				shell_cmd_list.c \
				shell_companion.c \
				shell_ext.c \
				shell_port.c

ifdef CONFIG_LS_PL011_UART
SHELL_CSRCS += fpl011_port.c
endif

ifdef CONFIG_LS_SERIAL_V2
SHELL_CSRCS += fserial_v2_port.c
endif

SHELL_CSRCS += cmd_bootelf.c \
				cmd_codeloader.c \
				cmd_echo.c \
				cmd_md.c \
				cmd_mw.c \
				cmd_reboot.c \
				cmd_rw.c \
				cmd_sleep.c \
				cmd_version.c \
				cmd_mmu.c