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

#include "buffer.h"
#include "str.h"

#include "product.h"


/*

*/

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
#else
struct slot_port_phy
{
	ifindex_t ifindex;
	ifindex_t kifindex;
};
#endif

static struct unit_slot_port iusp_table[] =
{
#ifdef APP_X5BA_MODULE
	{.type = IF_SERIAL, .unit = 0, .slot = 0, .port = 0 },
	{.type = IF_ETHERNET, .unit = 0, .slot = 0, .port = 2 },
#endif
#ifdef APP_V9_MODULE
	{.type = IF_SERIAL, .unit = 0, .slot = 0, .port = 0 },
	{.type = IF_ETHERNET, .unit = 0, .slot = 0, .port = 3 },
#endif

#ifdef PRODUCT_BOJING_BOARD
	{.type = IF_SERIAL, .unit = 0, .slot = 0, .port = 0 },
	{.type = IF_ETHERNET, .unit = 0, .slot = 0, .port = 3 },
#endif

	{.type = IF_GIGABT_ETHERNET, .unit = 0, .slot = 0, .port = 0 },
	{.type = IF_LOOPBACK, .unit = 0, .slot = 1, .port = 1 },
	{.type = IF_TUNNEL, .unit = 0, .slot = 1, .port = 0 },
	{.type = IF_VLAN, .unit = 0, .slot = 1, .port = 0 },
	{.type = IF_LAG, .unit = 0, .slot = 1, .port = 0 },
#ifdef APP_X5BA_MODULE
	{.type = IF_BRIGDE, .unit = 0, .slot = 0, .port = 1 },
	{.type = IF_WIRELESS, .unit = 0, .slot = 0, .port = 1 },
#endif
#ifdef APP_X5BA_MODULE
	{.type = IF_BRIGDE, .unit = 0, .slot = 0, .port = 1 },
	{.type = IF_WIRELESS, .unit = 0, .slot = 0, .port = 0 },
#endif
#ifdef CUSTOM_INTERFACE
	{.type = IF_WIFI, .unit = 0, .slot = 1, .port = 1 },
	{.type = IF_MODEM, .unit = 0, .slot = 1, .port = 1 },
#endif
};

#ifdef USE_IPSTACK_KERNEL

static struct slot_port_phy phy_table[OS_SLOT_MAX][OS_SLOT_HY_MAX + MODEM_PHY_MAX + WIFI_PHY_MAX] =
{
	{
		{.ifindex = 0, .kifindex = 0 },
		{.ifindex = 0, .kifindex = 0 },
		{.ifindex = 0, .kifindex = 0 },
		{.ifindex = 0, .kifindex = 0 },
		{.ifindex = 0, .kifindex = 0 },
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
	static char buf[64];
	int i = 0, j = 0;
	for(i = 0; i < OS_SLOT_MAX; i++)
	{
		for(j = 0; j < OS_SLOT_HY_MAX; j++)
		{
			if( (phy_table[i][j].ifindex != 0) && (phy_table[i][j].ifindex == ifindex) )
			{
				memset(buf, 0, sizeof(buf));
				//return if_indextoname(phy_table[i][j].kifindex, buf);
				//zlog_debug(MODULE_DEFAULT,"+++++++%s: %x -> %d", __func__, ifindex, phy_table[i][j].kifindex);
				if(if_indextoname(phy_table[i][j].kifindex, buf))
					return buf;
				return NULL;
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
			if((phy_table[i][j].kifindex != 0) && phy_table[i][j].kifindex == if_nametoindex(kname))
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
			if( (phy_table[i][j].ifindex != 0) && (phy_table[i][j].ifindex == ifindex) )
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
	//printf("=======%s: IFINDEX=%x  %s->%d\r\n", __func__, ifindex, name, if_nametoindex(name));
	if(if_slot_kernel_lookup(ifindex) == 0)
	{
		for(i = 0; i < OS_SLOT_MAX; i++)
		{
			for(j = 0; j < OS_SLOT_HY_MAX; j++)
			{
				if( (phy_table[i][j].ifindex == 0) )
				{
					phy_table[i][j].ifindex = ifindex;
					//os_memset(phy_table[i][j].kname, 0, sizeof(phy_table[i][j].kname));

					//if(if_nametoindex(name))
					{
						//os_strcpy(phy_table[i][j].kname, name);
						//zlog_debug(MODULE_DEFAULT, "=======%s: IFINDEX=%s  %s", __func__,
						//		if_ifname_make(ifindex), name);
						phy_table[i][j].kifindex = if_nametoindex(name);
					}
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
					//os_memset(phy_table[i][j].kname, 0, sizeof(phy_table[i][j].kname));
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
	char buf[64];
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
					memset(buf, 0, sizeof(buf));
					if(if_indextoname(phy_table[i][j].kifindex, buf))
					{
						char *p = if_ifname_make(phy_table[i][j].ifindex);
						fprintf(fp, "%s:%s\n", p, buf);
					}
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
		char *s = NULL;
		int n = 0;//, *p;
		os_memset(buf, 0, sizeof(buf));
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
	
				//os_msleep(1);
				ifindex = if_ifindex_make(name, NULL);
				//printf("========================%s========================%s(%d)-->%s\r\n", __func__, name, ifindex, kname);
				if(ifindex != 0)
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
	char buf[512];
	int i = 0, j = 0;
	char head = 0;
	for(i = 0; i < OS_SLOT_MAX; i++)
	{
		for(j = 0; j < OS_SLOT_HY_MAX; j++)
		{
			if( (phy_table[i][j].ifindex != 0) )
			{
				if(head == 0)
				{
					head = 1;
					vty_out(vty, " Slot Port Phy: %s",VTY_NEWLINE);
					vty_out(vty, " %-20s %-16s %s", "Interface","kernel",VTY_NEWLINE);
				}
				memset(buf, 0, sizeof(buf));
				if(if_indextoname(phy_table[i][j].kifindex, buf))
					vty_out(vty, " %-20s %-16s %s", ifindex2ifname(phy_table[i][j].ifindex),
							buf, VTY_NEWLINE);
			}
		}
	}
	//head = 1;
	return 0;
}
#endif


static int if_unit_slot_port(int type, int u, int s, int p)
{
	struct interface * ifp = NULL;
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
			ifp = if_create (name, strlen(name));

/*		if(strstr(name, "ethernet 0/0/1"))
		{
			if_kname_set(ifp, "enp0s25");
		}*/
	}
	return OK;
}

static int if_unit_slot(void)
{
	int i = 1;
#ifdef USE_IPSTACK_KERNEL
	if(i)
		if_slot_kernel_read();
	//if_slot_kernel_add(if_ifindex_make("ethernet 0/0/1", NULL), "eth0.1");
	//if_slot_kernel_add(if_ifindex_make("ethernet 0/0/2", NULL), "eth0.2");
	//if_slot_kernel_add(if_ifindex_make("wireless 0/0/1", NULL), "ra0");
	//if_slot_kernel_add(if_ifindex_make("brigde 0/0/1", NULL), "br-lan");
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
