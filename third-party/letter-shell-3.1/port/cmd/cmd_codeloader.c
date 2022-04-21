/*
 * @Author: your name
 * @Date: 2021-09-06 17:11:48
 * @LastEditTime: 2022-02-10 15:01:05
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \phytium-standalone-sdk\third-party\letter-shell-3.1\port\cmd\cmd_codelode.c
 */

#include <stdio.h>
#include "../src/shell.h"
#include "strto.h"
#include "ft_elf.h"

static unsigned long image_load_addr = 0x80100000;

static void LoadElfCmdUsage()
{
	printf("usage:\r\n");
	printf("    loadelf [-p|-s] [address] \r\n");
	printf("    load ELF image at [address] via program headers (-p)\r\n");
	printf("    or via section headers (-s)\r\n");
}

static int LoadElfCmdEntry(int argc, char *argv[])
{
	unsigned long addr; /* Address of the ELF image */
	unsigned long rc;	/* Return value from user code */
	unsigned long load_flg = 0;
	char *sload = NULL;

	if (argc < 2)
	{
		LoadElfCmdUsage();
		return -1;
	}
	/* Consume 'LoadElf' */
	argc--;
	argv++;

	/* Check for flag. */
	if (argc >= 1 && (argv[0][0] == '-' && (argv[0][1] == 'p' || argv[0][1] == 's')))
	{
		sload = argv[0];
		/* Consume flag. */
		argc--;
		argv++;
	}

	/* Check for address. */
	if (argc >= 1 && strict_strtoul(argv[0], 16, &addr) == 0)
	{
		/* Consume address */
		argc--;
		argv++;
	}
	else
	{
		addr = image_load_addr;
	}

	if (!ElfIsImageValid(addr))
		return 1;

	if (sload && sload[1] == 'p')
		addr = ElfLoadElfImagePhdr(addr);
	else
		addr = ElfLoadElfImageShdr(addr);

	/*
	 * pass address parameter as argv[0] (aka command name),
	 * and all remaining args
	 */
	printf("## Load application at 0x%08lx ...\n", addr);

	return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN),
				 loadelf, LoadElfCmdEntry, Load from an ELF image in memory);