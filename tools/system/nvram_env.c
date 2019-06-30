/*
 * nvram_env.c
 *
 *  Created on: 2019年3月7日
 *      Author: DELL
 */


#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "getopt.h"
#include "os_list.h"
#include "os_nvram.h"

static char *progname = NULL;
/*
 * nvramenv add/set/del/show NAME [VALUE]
 * nvramenv show voip  -> argc = 3
 */


static int nvramenv_show_one(void *p, os_nvram_env_t *node)
{
	if(node)
	{
		printf("%s=%s\r\n", node->name, node->ptr.va_p);
	}
	return 0;
}


int main(int argc, char *argv[])
{
	int ret = 0;
	char *p = NULL;
	progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);

	//printf(" use: nvramenv argc=%d\r\n", argc);

	if(argc == 4 && argv[1] && strstr(argv[1], "add"))
	{
		if(argv[2] && argv[3])
		{
			os_nvram_env_init();
			 ret = os_nvram_env_add(argv[2], argv[3]);
			 if(ret == 0)
			 {
				os_nvram_env_exit();
				return 0;
			 }
			 os_nvram_env_exit();
			 printf("Error:Can not add env '%s=%s'\r\n", argv[2], argv[3]);
		}
	}
	else if(argc == 4 && argv[1] && strstr(argv[1], "set"))
	{
		if(argv[2] && argv[3])
		{
			os_nvram_env_init();
			 ret = os_nvram_env_set(argv[2], argv[3]);
			 if(ret == 0)
			 {
				os_nvram_env_exit();
				return 0;
			 }
			 os_nvram_env_exit();
			 printf("Error:Can not set env '%s=%s'\r\n", argv[2], argv[3]);
		}
	}
	else if(argc == 3 && argv[1] && strstr(argv[1], "del"))
	{
		if(argv[2])
		{
			os_nvram_env_init();
			 ret = os_nvram_env_del(argv[2]);
			 if(ret == 0)
			 {
				os_nvram_env_exit();
				return 0;
			 }
			 os_nvram_env_exit();
			 printf("Error:Can not find env '%s'\r\n", argv[2]);
		}
	}
	else if(argc == 3 && argv[1] && strstr(argv[1], "show"))
	{
		if(argv[2])
		{
			os_nvram_env_init();
			 p = os_nvram_env_lookup(argv[2]);
			 if(p)
			 {
				printf("%s=%s\r\n",argv[2], p);
				os_nvram_env_exit();
				return 0;
			 }
			 os_nvram_env_exit();
			 printf("Error:Can not find env '%s'\r\n", argv[2]);
		}
	}
	else if(argc == 2 && argv[1] && strstr(argv[1], "show"))
	{
		//if(argv[1])
		{
			os_nvram_env_init();
			printf("nvram env list:\r\n");
			os_nvram_env_show(NULL, nvramenv_show_one, NULL);
			printf("end list:\r\n");
			os_nvram_env_exit();
			return 0;
		}
	}
	else
	{
		 printf(" use: nvramenv add/set NAME [VALUE]\r\n");
		 printf("      nvramenv del/show [NAME]\r\n");
		 printf("      nvramenv show\r\n");
	}
	return 1;
}

