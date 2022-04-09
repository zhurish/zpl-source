/*
 * if_ktest.c
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */
#include "auto_include.h"
#include <zplos_include.h>
#include "nsm_include.h"
#include "if_manage.h"
#include "bmgt.h"


#ifdef ZPL_KERNEL_MODULE

static int if_slot_kernel_read(void)
{
	struct interface * ifp = NULL;
	char buf[512];
	ifindex_t ifindex;
	unit_board_mgt_t *board = NULL;
	char kname[64], name[128];
	FILE *fp = fopen(SLOT_PORT_CONF, "r");
	if (fp)
	{
		char *s = NULL;
		int n = 0;//, *p;
		//printf("========================%s========================open %s\r\n", __func__, SLOT_PORT_CONF);
		os_memset(buf, 0, sizeof(buf));
		board = unit_board_add(0, 0);
		while (fgets(buf, sizeof(buf), fp))
		{
			os_memset(kname, 0, sizeof(kname));
			os_memset(name, 0, sizeof(name));
			s = strstr(buf, ":");
			if(s)
			{
				os_memcpy(name, buf, s - buf);
				s++;
				n = strspn(s, "qwertyuiopasdfghjklzxcvbnm.1234567890");
				if(strstr(s, "-"))
					os_strcpy(kname, "br-lan");
				else
					os_strncpy(kname, s, n);
				//kname[strlen(kname)-1] = '\0';

				ifp = if_create (name, strlen(name));
				//os_msleep(1);
				ifindex = if_ifindex_make(name, NULL);
				//printf("========================%s========================%s(%d)-->%s\r\n", __func__, name, ifindex, kname);
				if(ifp && ifindex != 0)
				{
					if_kname_set(ifp, kname);

					if(strstr(name,"eth"))
						unit_board_port_add(board,IF_ETHERNET, 1, if_nametoindex(kname));
					else if(strstr(name,"wireless"))
						unit_board_port_add(board,IF_WIRELESS, 1, if_nametoindex(kname));						
				}
			}
		}
		fclose(fp);
	}
	else
	{
		//printf("========================%s========================can not open %s\r\n", __func__, SLOT_PORT_CONF);
	}
	return 0;
}


#endif

#ifdef ZPL_KERNEL_MODULE
int if_ktest_init(void)
{
	if_slot_kernel_read();
	return OK;
}
#endif




