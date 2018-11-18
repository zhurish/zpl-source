/*
 * if_usp.c
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */
#include <zebra.h>

#include "linklist.h"
#include "vector.h"
#include "vty.h"
#include "command.h"
#include "if.h"
#include "if_manage.h"
#include "sockunion.h"
#include "prefix.h"
#include "memory.h"
#include "table.h"
#include "buffer.h"
#include "str.h"
//#include "net/if_arp.h"


#define OS_SLOT_MAX 	1
#define OS_SLOT_HY_MAX 	5

struct unit_slot_port
{
  int type;
  int unit;
  int slot;
  int port;
};

#ifndef USE_IPSTACK_KERNEL
struct slot_port_phy
{
  int slot;
  int port;
  int phy;
};
#endif

#ifdef USE_IPSTACK_KERNEL
#define SLOT_PORT_CONF	"/etc/plat.conf"
struct slot_port_phy
{
	ifindex_t ifindex;
	ifindex_t kifindex;
	char	kname[64];
};
#endif

static struct unit_slot_port iusp_table[] =
{
	{.type = IF_SERIAL, .unit = 0, .slot = 0, .port = 3 },
	{.type = IF_ETHERNET, .unit = 0, .slot = 0, .port = 3 },
	{.type = IF_GIGABT_ETHERNET, .unit = 0, .slot = 0, .port = 0 },
	{.type = IF_LOOPBACK, .unit = 0, .slot = 1, .port = 1 },
	{.type = IF_TUNNEL, .unit = 0, .slot = 1, .port = 0 },
	{.type = IF_VLAN, .unit = 0, .slot = 1, .port = 0 },
	{.type = IF_LAG, .unit = 0, .slot = 1, .port = 0 },
	{.type = IF_BRIGDE, .unit = 0, .slot = 1, .port = 0 },
	{.type = IF_WIRELESS, .unit = 0, .slot = 0, .port = 1 },
#ifdef CUSTOM_INTERFACE
	{.type = IF_WIFI, .unit = 0, .slot = 1, .port = 1 },
	{.type = IF_MODEM, .unit = 0, .slot = 1, .port = 1 },
#endif
};

#ifdef USE_IPSTACK_KERNEL

static struct slot_port_phy phy_table[OS_SLOT_MAX][OS_SLOT_HY_MAX + MODEM_PHY_MAX + WIFI_PHY_MAX] =
{
	{
		{.ifindex = 0, .kifindex = 0, .kname = "eth" },
		{.ifindex = 0, .kifindex = 0, .kname = "eth" },
		{.ifindex = 0, .kifindex = 0, .kname = "eth" },
		{.ifindex = 0, .kifindex = 0, .kname = "eth" },
		{.ifindex = 0, .kifindex = 0, .kname = "eth" },
	},
};
#else
static struct slot_port_phy phy_table[OS_SLOT_MAX][OS_SLOT_HY_MAX] =
{
	{
		{.slot = 1, .port = 1, .phy = 3 },
		{.slot = 1, .port = 2, .phy = 2 },
		{.slot = 1, .port = 3, .phy = 0 },
		{.slot = 1, .port = 4, .phy = 1 },
		{.slot = 1, .port = 5, .phy = 4 },
	},
};
#endif

static int if_slot_port_to_phy(int s, int p)
{
	int i = 0, j = 0;
	for(i = 0; i < OS_SLOT_MAX; i++)
	{
		for(j = 0; j < OS_SLOT_HY_MAX; j++)
		{
#ifdef USE_IPSTACK_KERNEL
			if( (phy_table[i][j].ifindex == p) )
				return phy_table[i][j].kifindex;
#else
			if( (phy_table[i][j].slot == s) &&
				(phy_table[i][j].port == p) )
				return phy_table[i][j].phy;
#endif
		}
	}
	return 0;
}


const int if_ifindex2phy(ifindex_t ifindex)
{
	if( IF_TYPE_GET(ifindex) == IF_SERIAL ||
			IF_TYPE_GET(ifindex) == IF_ETHERNET ||
			IF_TYPE_GET(ifindex) == IF_GIGABT_ETHERNET)
	{
		return if_slot_port_to_phy(IF_SLOT_GET(ifindex), IF_PORT_GET(ifindex));
	}
	else if(IF_TYPE_GET(ifindex) == IF_VLAN)
	{
		return IF_TYPE_CLR(ifindex);
	}
	else if(IF_TYPE_GET(ifindex) == IF_LAG)
	{
		return IF_TYPE_CLR(ifindex);
	}
	else if(IF_TYPE_GET(ifindex) == IF_LOOPBACK)
	{
		return IF_TYPE_CLR(ifindex);
	}
	else if(IF_TYPE_GET(ifindex) == IF_TUNNEL)
	{
		return IF_TYPE_CLR(ifindex);
	}
	return -1;
}

#ifdef USE_IPSTACK_KERNEL

const char * if_kernel_name_lookup(ifindex_t ifindex)
{
	int i = 0, j = 0;
	for(i = 0; i < OS_SLOT_MAX; i++)
	{
		for(j = 0; j < OS_SLOT_HY_MAX; j++)
		{
			if( (phy_table[i][j].ifindex == ifindex) )
			{
				return phy_table[i][j].kname;
			}
		}
	}
	return NULL;
}

ifindex_t ifindex_lookup_by_kname(const char *kname)
{
	int i = 0, j = 0;
	for(i = 0; i < OS_SLOT_MAX; i++)
	{
		for(j = 0; j < OS_SLOT_HY_MAX; j++)
		{
			if(strcmp(phy_table[i][j].kname, kname) == 0)
			{
				return phy_table[i][j].ifindex;
			}
		}
	}
	return 0;
}

static int if_slot_kernel_lookup(ifindex_t ifindex)
{
	int i = 0, j = 0;
	for(i = 0; i < OS_SLOT_MAX; i++)
	{
		for(j = 0; j < OS_SLOT_HY_MAX; j++)
		{
			if( (phy_table[i][j].ifindex == ifindex) )
			{
				return 1;
			}
		}
	}
	return 0;
}

static int if_slot_kernel_add(ifindex_t ifindex, char *name)
{
	int i = 0, j = 0;
	if(if_slot_kernel_lookup(ifindex) == 0)
	{
		for(i = 0; i < OS_SLOT_MAX; i++)
		{
			for(j = 0; j < OS_SLOT_HY_MAX; j++)
			{
				if( (phy_table[i][j].ifindex == 0) )
				{
					phy_table[i][j].ifindex = ifindex;
					os_memset(phy_table[i][j].kname, 0, sizeof(phy_table[i][j].kname));
					os_strcpy(phy_table[i][j].kname, name);
					if(os_strlen(name))
						phy_table[i][j].kifindex = if_nametoindex(name);
					return 0;
				}
			}
		}
	}
	return 0;
}

static int if_slot_kernel_del(ifindex_t ifindex)
{
	int i = 0, j = 0;
	if(if_slot_kernel_lookup(ifindex))
	{
		for(i = 0; i < OS_SLOT_MAX; i++)
		{
			for(j = 0; j < OS_SLOT_HY_MAX; j++)
			{
				if( (phy_table[i][j].ifindex == ifindex) )
				{
					phy_table[i][j].ifindex = 0;
					os_memset(phy_table[i][j].kname, 0, sizeof(phy_table[i][j].kname));
					phy_table[i][j].kifindex = 0;
					return 0;
				}
			}
		}
	}
	return 0;
}

static int if_slot_kernel_update()
{
	int i = 0, j = 0;
	remove(SLOT_PORT_CONF);
	FILE *fp = fopen(SLOT_PORT_CONF, "w+");
	if(fp)
	{
		for(i = 0; i < OS_SLOT_MAX; i++)
		{
			for(j = 0; j < OS_SLOT_HY_MAX; j++)
			{
				if( (phy_table[i][j].ifindex) )
				{
					fprintf(fp, "%s:%s=%u\n", phy_table[i][j].kname,
							ifindex2ifname(phy_table[i][j].ifindex), phy_table[i][j].ifindex);
				}
			}
		}
		fflush(fp);
		fclose(fp);
	}
	return 0;
}

static int if_slot_kernel_read()
{
	//int i = 0;
	char buf[512];
	ifindex_t ifindex;
	char kname[64], name[128];
	FILE *fp = fopen(SLOT_PORT_CONF, "r");
	if (fp)
	{
		char *s, *p;
		os_memset(buf, 0, sizeof(buf));
		while (fgets(buf, sizeof(buf), fp))
		{
			os_memset(kname, 0, sizeof(kname));
			os_memset(name, 0, sizeof(name));
			s = strstr(buf, ":");
			if(s)
			{
				os_memcpy(kname, buf, s - buf);
				s++;
				p = strstr(s, "=");
				if(p)
				{
					os_memcpy(name, s, p - s);
					//ifindex = ifname2ifindex(name);
					p++;
					ifindex = atoi(p);
				}
/*				else
				{
					ifindex = atoi(s);
				}*/
				if(ifindex)
					if_slot_kernel_add( ifindex, kname);
			}
		}
		fclose(fp);
	}
	return 0;
}

int if_slot_set_port_phy(ifindex_t ifindex, char *name)
{
	/*
	 * name:ifindex
	 */
	remove(SLOT_PORT_CONF);
	if(name)
		if_slot_kernel_add( ifindex, name);
	else
		if_slot_kernel_del( ifindex);
	if_slot_kernel_update();
	return OK;
}

int if_slot_show_port_phy(struct vty *vty)
{
	int i = 0, j = 0;
	char head = 0;
	for(i = 0; i < OS_SLOT_MAX; i++)
	{
		for(j = 0; j < OS_SLOT_HY_MAX; j++)
		{
			if( (phy_table[i][j].ifindex) )
			{
				if(head == 0)
				{
					head = 1;
					vty_out(vty, " %-20s %-16s %s", "Interface","kernel",VTY_NEWLINE);
				}
				vty_out(vty, " %-20s %-16s %s", ifindex2ifname(phy_table[i][j].ifindex), phy_table[i][j].kname, VTY_NEWLINE);
			}
		}
	}
	//head = 1;
	return 0;
}
#endif


static int if_unit_slot_port(int type, int u, int s, int p)
{
	int i = 0;
	char name[64];//[ethernet|gigabitethernet|tunnel|loopback]
	if(p == 0)
		return 0;
	for(i = 1; i <= p; i++)
	{
		memset(name, '\0', sizeof(name));

		if(type == IF_SERIAL)
			sprintf(name,"serial %d/%d/%d",u,s,i);
		else if(type == IF_ETHERNET)
			sprintf(name,"ethernet %d/%d/%d",u,s,i);
		else if(type == IF_GIGABT_ETHERNET)
			sprintf(name,"gigabitethernet %d/%d/%d",u,s,i);
		else if(type == IF_WIRELESS)
			sprintf(name,"wireless %d/%d/%d",u,s,i);
		else if(type == IF_TUNNEL)
			sprintf(name,"tunnel %d/%d/%d",u,s,i);
		else if(type == IF_BRIGDE)
			sprintf(name,"brigde %d/%d/%d",u,s,i);
#ifdef CUSTOM_INTERFACE
		else if(type == IF_WIFI)
			sprintf(name,"wifi %d/%d/%d",u,s,i);
		else if(type == IF_MODEM)
			sprintf(name,"modem %d/%d/%d",u,s,i);
#endif
		else if(type == IF_LOOPBACK)
			sprintf(name,"loopback%d",i);
		else if(type == IF_VLAN)
			sprintf(name,"vlan%d",i);
		else if(type == IF_LAG)
			sprintf(name,"port-channel%d",i);

		if(os_strlen(name))
			if_create (name, strlen(name));
	}
	return OK;
}

static int if_unit_slot(void)
{
	int i = 0;
#ifdef USE_IPSTACK_KERNEL
	if_slot_kernel_read();
#endif
	for(i = 0; i < array_size(iusp_table); i++)
	{
		if_unit_slot_port(iusp_table[i].type, iusp_table[i].unit,
				iusp_table[i].slot, iusp_table[i].port);
	}
	return OK;
}


int bsp_usp_module_init()
{
	return if_unit_slot();
}




#if 0
static int _bsp_create_serial_interface(char *kname)
{
	int i = 0;
	char name[64];//[ethernet|gigabitethernet|tunnel|loopback]
	struct interface *ifp;
	if(kname == NULL)
		return 0;
	ifp = if_lookup_by_kernel_name(kname);
	if(!ifp)
	{
		if(os_strstr(kname, "tty"))
		{
			for(i = 0; i < array_size(iusp_table); i++)
			{
				if(iusp_table[i].type == IF_SERIAL)
				{
					sprintf(name,"serial %d/%d/%d",iusp_table[i].unit, iusp_table[i].slot, ++iusp_table[i].port);
					break;
				}
			}
			if(os_strlen(name))
			{
				if_slot_set_port_phy(if_ifindex_make(name, NULL), kname);
				ifp = if_create (name, strlen(name));
			}
		}
	}
	if(ifp)
	{
		ifp->ll_type = ZEBRA_LLT_MODEM;
		ifp->if_mode = IF_MODE_L3;
		if_slot_set_port_phy(ifp->ifindex, kname);
		//ifp->k_ifindex = serial_kifindex_make(name);
	}
	return OK;
}

static int _bsp_delete_serial_interface(char *kname)
{
	char name[64];//[ethernet|gigabitethernet|tunnel|loopback]
	struct interface *ifp;
	if(kname == NULL)
		return 0;

	ifp = if_lookup_by_kernel_name(kname);
	if(ifp)
	{
		ifp->ll_type = ZEBRA_LLT_MODEM;
		ifp->if_mode = IF_MODE_L3;
		if_slot_set_port_phy(ifp->ifindex, NULL);
		zebra_interface_delete_update (ifp);
	}
	return OK;
}


static int _bsp_create_modem_interface(char *kname)
{
	int i = 0;
	char name[64];//[ethernet|gigabitethernet|tunnel|loopback]
	struct interface *ifp;
	if(kname == NULL)
		return 0;
	ifp = if_lookup_by_kernel_name(kname);
	if(ifp == NULL)
	{
		for(i = 0; i < array_size(iusp_table); i++)
		{
			if(iusp_table[i].type == IF_ETHERNET)
			{
				sprintf(name,"ethernet %d/%d/%d",iusp_table[i].unit, iusp_table[i].slot, ++iusp_table[i].port);
				break;
			}
		}
		if(os_strlen(name))
		{
			if_slot_set_port_phy(if_ifindex_make(name, NULL), kname);
			ifp = if_create (name, strlen(name));
		}
	}
	if(ifp)
	{
		ifp->ll_type = ZEBRA_LLT_MODEM;
		ifp->if_mode = IF_MODE_L3;
		if_slot_set_port_phy(ifp->ifindex, kname);
		if_manage_kernel_update(ifp);
		zebra_interface_add_update(ifp);
	}
	return OK;
}

static int _bsp_delete_modem_interface(char *kname)
{
	char name[64];//[ethernet|gigabitethernet|tunnel|loopback]
	struct interface *ifp;
	if(kname == NULL)
		return 0;

	ifp = if_lookup_by_kernel_name(kname);
	if(ifp)
	{
		ifp->ll_type = ZEBRA_LLT_MODEM;
		ifp->if_mode = IF_MODE_L3;
		if_slot_set_port_phy(ifp->ifindex, NULL);
		zebra_interface_delete_update (ifp);
	}
	return OK;
}

static int _bsp_create_wifi_interface(char *kname)
{
	int i = 0;
	char name[64];//[ethernet|gigabitethernet|tunnel|loopback]
	struct interface *ifp;
	if(kname == NULL)
		return 0;
	ifp = if_lookup_by_kernel_name(kname);
	if(ifp == NULL)
	{
		for(i = 0; i < array_size(iusp_table); i++)
		{
			if(iusp_table[i].type == IF_ETHERNET)
			{
				sprintf(name,"ethernet %d/%d/%d",iusp_table[i].unit, iusp_table[i].slot, ++iusp_table[i].port);
				break;
			}
		}
		if(os_strlen(name))
		{
			if_slot_set_port_phy(if_ifindex_make(name, NULL), kname);
			ifp = if_create (name, strlen(name));
		}
	}
	if(ifp)
	{
		ifp->ll_type = ZEBRA_LLT_WIFI;
		ifp->if_mode = IF_MODE_L3;
		if_slot_set_port_phy(ifp->ifindex, kname);
		if_manage_kernel_update(ifp);
		zebra_interface_add_update(ifp);
	}
	return OK;
}

static int _bsp_delete_wifi_interface(char *kname)
{
	char name[64];//[ethernet|gigabitethernet|tunnel|loopback]
	struct interface *ifp;
	if(kname == NULL)
		return 0;

	ifp = if_lookup_by_kernel_name(kname);
	if(ifp)
	{
		ifp->ll_type = ZEBRA_LLT_WIFI;
		ifp->if_mode = IF_MODE_L3;
		if_slot_set_port_phy(ifp->ifindex, NULL);
		zebra_interface_delete_update (ifp);
	}
	return OK;
}



int bsp_create_interface(char *type, char *kname)
{
	if(os_strstr(type, "tty") || os_strstr(type, "serial"))
		return _bsp_create_serial_interface(kname);
	if(os_strstr(type, "modem"))
		return _bsp_create_modem_interface(kname);
	if(os_strstr(type, "wifi"))
		return _bsp_create_wifi_interface(kname);
	return OK;
}


int bsp_delete_interface(char *kname)
{
	_bsp_delete_serial_interface(kname);
	_bsp_delete_modem_interface(kname);
	_bsp_delete_wifi_interface(kname);
	return OK;
}
#endif
