TOOL_CHAIN = $(AARCH32_CROSS_PATH)/bin/arm-none-eabi-
CC = ${TOOL_CHAIN}gcc
AS = ${TOOL_CHAIN}as
LD = ${TOOL_CHAIN}ld
OBJCOPY = ${TOOL_CHAIN}objcopy
OBJDUMP = $(TOOL_CHAIN)objdump

CFLAGS 		:= -Wall -g -fno-builtin -gdwarf-2 -gstrict-dwarf -Iinclude -mcpu=cortex-a7
LDFLAGS 	:= -g

objs := start.o \
		entry.o \
		driver/imx_uart.o \
		driver/imx_usdhc.o \
		device/qemu_print.o \
		device/sd_card.o \
		fatfs/diskio.o \
		fatfs/ff.o \
		fatfs/ffsystem.o \
		fatfs/ffunicode.o

bootloader.bin: $(objs)
	${LD} -T bootloader.ld -o bootloader.elf $^
	${OBJCOPY} -O binary -S bootloader.elf $@
	${OBJDUMP} -D -m arm bootloader.elf > bootloader.dis


%.o:%.c
	${CC} $(CFLAGS) -c -o $@ $<

%.o:%.s
	${CC}  $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o *.elf *.bin *.dis driver/*.o device/*.o fatfs/*.o
