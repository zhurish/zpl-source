/*
 * web_switch_html.c
 *
 *  Created on: 2019年8月30日
 *      Author: DELL
 */

#include "zplos_include.h"
#include "zassert.h"
#include "vty.h"
#include "if.h"
#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "zmemory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "nsm_ipvrf.h"
#include "nsm_interface.h"
#include "nsm_dhcp.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"



#ifdef ZPL_BUILD_OS_OPENWRT
/*
config switch
        option name 'switch0'
        option reset '1'
        option enable_vlan '1'

config switch_vlan
        option device 'switch0'
        option vlan '1'
        option ports '1 2 3 4 6t'

config switch_vlan
        option device 'switch0'
        option vlan '2'
        option ports '0 6t'
*/
#define SWITCH_PORTS_MAX	8
#define SWITCH_VLAN_MAX		16
#define SWITCH_PORT_CPU		6
typedef struct sw_port
{
	zpl_uint8		port;
	zpl_uint8		is_active:1;
	zpl_uint8		tagged:1;
	zpl_uint8		wan:1;
	zpl_uint8		is_cpu:1;
	zpl_uint8		disable:1;
	zpl_uint8		res:3;
}sw_port_t;

typedef struct sw_vlan
{
	zpl_uint16		vlan;
	zpl_uint8		wanlan;
	sw_port_t	ports[SWITCH_PORTS_MAX];
}sw_vlan_t;

typedef struct switch_dev
{
	zpl_bool			vlan_enable;
	sw_vlan_t 		vlan[SWITCH_VLAN_MAX];
}switch_dev_t;

static switch_dev_t		*switch_dev = NULL;



#ifdef ZPL_OPENWRT_UCI
/*
network.wan6.ifname='eth0.2'
network.@switch[0]=switch
network.@switch[0].name='switch0'
network.@switch[0].reset='1'
network.@switch[0].enable_vlan='1'
network.@switch_vlan[0]=switch_vlan
network.@switch_vlan[0].device='switch0'
network.@switch_vlan[0].vlan='1'
network.@switch_vlan[0].ports='1 2 3 4 6t'
network.@switch_vlan[1]=switch_vlan
network.@switch_vlan[1].device='switch0'
network.@switch_vlan[1].vlan='2'
network.@switch_vlan[1].ports='0 6t'
root@TSLSmart-X5B:/# uci get network.switch.enable_vlan
uci: Entry not found
root@TSLSmart-X5B:/# uci get network.@switch[0].enable_vlan
1
root@TSLSmart-X5B:/#
*/

static int switch_dev_config_load(switch_dev_t *dev)
{
	char tmp[64];
	char cmd[64];
	int ret = ERROR, i = 0, val = 0, lan_vid = 0, wan_vid = 0;
	web_assert(dev);

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("network.wan.ifname", tmp);
	//_WEB_DBG_TRAP("-----%s-----:wan %s\r\n", __func__, tmp);
	if(ret == OK)
	{
		char *brk = strstr(tmp, ".");
		if(brk)
		{
			brk++;
			wan_vid = atoi(brk);
			//_WEB_DBG_TRAP("-----%s-----:wan vid%d\r\n", __func__, wan_vid);
		}
	}
	ret = os_uci_get_string("network.lan.ifname", tmp);
	//_WEB_DBG_TRAP("-----%s-----:lan %s\r\n", __func__, tmp);
	if(ret == OK)
	{
		char *brk = strstr(tmp, ".");
		if(brk)
		{
			brk++;
			lan_vid = atoi(brk);
			//_WEB_DBG_TRAP("-----%s-----:lan vid%d\r\n", __func__, lan_vid);
		}
	}
	ret = os_uci_get_integer("network.@switch[0].enable_vlan", &dev->vlan_enable);
	////_WEB_DBG_TRAP("-----%s-----:network.@switch[0].enable_vlan=%d\r\n", __func__, dev->vlan_enable);
	if(ret == OK && dev->vlan_enable)
	{
		for(i = 0; i < SWITCH_VLAN_MAX; i++)
		{
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "network.@switch_vlan[%d].vlan", i);
			ret = os_uci_get_integer(cmd, &dev->vlan[i].vlan);
			//_WEB_DBG_TRAP("-----%s-----:dev->vlan[%d].vlan=%d\r\n", __func__, i,dev->vlan[i].vlan);
			if(ret != OK)
				break;
			if(ret == OK && dev->vlan[i].vlan > 0)
			{
				if(dev->vlan[i].vlan == lan_vid)
					dev->vlan[i].wanlan = 0;

				if(dev->vlan[i].vlan == wan_vid)
					dev->vlan[i].wanlan = 1;

				memset(tmp, 0, sizeof(tmp));
				memset(cmd, 0, sizeof(cmd));
				sprintf(cmd, "network.@switch_vlan[%d].ports", i);
				ret = os_uci_get_string(cmd, tmp);

				//_WEB_DBG_TRAP("-----%s-----:dev->vlan[%d].ports=%s\r\n", __func__, i, tmp);
				if(ret == OK)
				{
					int j = 0;
					int find = 0, ind = 0;
					val = strlen(tmp);
					while(val)
					{
						switch(tmp[j])
						{
						case '0':
							dev->vlan[i].ports[0].port = 0;
							dev->vlan[i].ports[0].is_active = 1;
							dev->vlan[i].ports[0].tagged = 0;
							dev->vlan[i].ports[0].wan = dev->vlan[i].wanlan;
							dev->vlan[i].ports[0].is_cpu = 0;
							dev->vlan[i].ports[0].res = 0;
							ind = 0;
							find = 1;
							//_WEB_DBG_TRAP("-----%s-----:tmp[j]=%c\r\n", __func__, tmp[j]);
							break;
						case '1':
							dev->vlan[i].ports[1].port = 1;
							dev->vlan[i].ports[1].is_active = 1;
							dev->vlan[i].ports[1].tagged = 0;
							dev->vlan[i].ports[1].wan = dev->vlan[i].wanlan;
							dev->vlan[i].ports[1].is_cpu = 0;
							dev->vlan[i].ports[1].res = 0;
							ind = 1;
							find = 1;
							//_WEB_DBG_TRAP("-----%s-----:tmp[j]=%c\r\n", __func__, tmp[j]);
							break;
						case '2':
							dev->vlan[i].ports[2].port = 2;
							dev->vlan[i].ports[2].is_active = 1;
							dev->vlan[i].ports[2].tagged = 0;
							dev->vlan[i].ports[2].wan = dev->vlan[i].wanlan;
							dev->vlan[i].ports[2].is_cpu = 0;
							dev->vlan[i].ports[2].res = 0;
							ind = 2;
							find = 1;
							//_WEB_DBG_TRAP("-----%s-----:tmp[j]=%c\r\n", __func__, tmp[j]);
							break;
						case '3':
							dev->vlan[i].ports[3].port = 3;
							dev->vlan[i].ports[3].is_active = 1;
							dev->vlan[i].ports[3].tagged = 0;
							dev->vlan[i].ports[3].wan = dev->vlan[i].wanlan;
							dev->vlan[i].ports[3].is_cpu = 0;
							dev->vlan[i].ports[3].res = 0;
							ind = 3;
							find = 1;
							//_WEB_DBG_TRAP("-----%s-----:tmp[j]=%c\r\n", __func__, tmp[j]);
							break;
						case '4':
							dev->vlan[i].ports[4].port = 4;
							dev->vlan[i].ports[4].is_active = 1;
							dev->vlan[i].ports[4].tagged = 0;
							dev->vlan[i].ports[4].wan = dev->vlan[i].wanlan;
							dev->vlan[i].ports[4].is_cpu = 0;
							dev->vlan[i].ports[4].res = 0;
							ind = 4;
							find = 1;
							//_WEB_DBG_TRAP("-----%s-----:tmp[j]=%c\r\n", __func__, tmp[j]);
							break;
						case '5':
							dev->vlan[i].ports[5].port = 5;
							dev->vlan[i].ports[5].is_active = 1;
							dev->vlan[i].ports[5].tagged = 0;
							dev->vlan[i].ports[5].wan = dev->vlan[i].wanlan;
							dev->vlan[i].ports[5].is_cpu = 0;
							dev->vlan[i].ports[5].res = 0;
							ind = 5;
							find = 1;
							//_WEB_DBG_TRAP("-----%s-----:tmp[j]=%c\r\n", __func__, tmp[j]);
							break;
						case '6':
							dev->vlan[i].ports[6].port = 6;
							dev->vlan[i].ports[6].is_active = 1;
							dev->vlan[i].ports[6].tagged = 0;
							dev->vlan[i].ports[6].wan = dev->vlan[i].wanlan;
							dev->vlan[i].ports[6].is_cpu = 1;
							dev->vlan[i].ports[6].res = 0;
							ind = 6;
							find = 1;
							//_WEB_DBG_TRAP("-----%s-----:tmp[j]=%c\r\n", __func__, tmp[j]);
							break;
						case '7':
							dev->vlan[i].ports[7].port = 7;
							dev->vlan[i].ports[7].is_active = 1;
							dev->vlan[i].ports[7].tagged = 0;
							dev->vlan[i].ports[7].wan = dev->vlan[i].wanlan;
							dev->vlan[i].ports[7].is_cpu = 0;
							dev->vlan[i].ports[7].res = 0;
							ind = 7;
							find = 1;
							//_WEB_DBG_TRAP("-----%s-----:tmp[j]=%c\r\n", __func__, tmp[j]);
							break;
						case 't':
							find = 0;
							if(ind)
								dev->vlan[i].ports[ind].tagged = 1;
							if(tmp[j-1] == '6')
							{
								dev->vlan[i].ports[6].is_cpu = 1;
								dev->vlan[i].ports[6].tagged = 1;
							}
							//_WEB_DBG_TRAP("-----%s-----:tmp[j]=%c\r\n", __func__, tmp[j]);
							break;
						case ' ':
							ind = 0;
							find = 0;
							break;
						}
						j++;
						val--;
					}
				}
			}
		}
	}
	return OK;
}


static int _switch_dev_config_save(switch_dev_t *dev)
{
	int i = 0, j = 0, set = 0;
	char cmd[64];
	char tmp[64];
	for(i = 0; i < SWITCH_VLAN_MAX; i++)
	{
		if(dev->vlan_enable && dev->vlan[i].vlan > 0)
		{
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "network.@switch_vlan[%d].vlan", i);
			os_uci_set_integer(cmd, dev->vlan[i].vlan);
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "network.@switch_vlan[%d].ports", i);

			memset(tmp, 0, sizeof(tmp));
			set = 0;
			for(j = 0; j < SWITCH_PORTS_MAX; j++)
			{
				if(dev->vlan[i].ports[j].is_active)
				{
					strcat(tmp, itoa(j, 10));
					if(dev->vlan[i].ports[j].tagged)
						strcat(tmp, "t");

					strcat(tmp, " ");
					set = 1;
				}
			}
			if(set)
				os_uci_set_string(cmd, tmp);
		}
	}
	os_uci_save_config("network");
	return OK;
}

static int switch_dev_config_save(void)
{
	if(switch_dev)
		return _switch_dev_config_save(switch_dev);
	return ERROR;
}


static int switch_dev_init()
{
	if(switch_dev == NULL)
	{
		switch_dev = malloc(sizeof(switch_dev_t));
		if(switch_dev == NULL)
			return ERROR;
	}
	memset(switch_dev, 0, sizeof(switch_dev_t));
#ifdef ZPL_OPENWRT_UCI
	switch_dev_config_load(switch_dev);
#endif
	return OK;
}


static int _switch_dev_vlan_clean_by_noport(switch_dev_t *dev)
{
	int i = 0, j = 0, set = 0;
	for(i = 0; i < SWITCH_VLAN_MAX; i++)
	{
		if(dev->vlan_enable && dev->vlan[i].vlan > 0)
		{
			set = 0;
			for(j = 0; j < SWITCH_PORTS_MAX; j++)
			{
				if(dev->vlan[i].ports[j].is_active)
				{
					set++;
					break;
				}
			}
			if(set == 0)
			{
				dev->vlan[i].vlan = 0;
				memset(dev->vlan[i].ports, 0, sizeof(dev->vlan[i].ports));
			}
		}
	}
	return OK;
}

static int _switch_dev_vlan_create(switch_dev_t *dev, zpl_uint16 vlan)
{
	int i = 0;
	for(i = 0; i < SWITCH_VLAN_MAX; i++)
	{
		if(dev->vlan_enable && dev->vlan[i].vlan == vlan)
		{
			return OK;
		}
	}
	for(i = 0; i < SWITCH_VLAN_MAX; i++)
	{
		if(dev->vlan_enable && dev->vlan[i].vlan == 0)
		{
			dev->vlan[i].vlan = vlan;
			return OK;
		}
	}
	return OK;
}

static int _switch_dev_vlan_get_port(switch_dev_t *dev, zpl_uint16 *vlan, zpl_uint8 port)
{
	int i = 0, j = 0;
	for(i = 0; i < SWITCH_VLAN_MAX; i++)
	{
		if(dev->vlan_enable && dev->vlan[i].vlan > 0)
		{
			for(j = 0; j < SWITCH_PORTS_MAX; j++)
			{
				if(dev->vlan[i].ports[port].is_active)
				{
					if(vlan)
						*vlan =  dev->vlan[i].vlan;
					return OK;
				}
			}
		}
	}
	return ERROR;
}

static int _switch_dev_vlan_add_port(switch_dev_t *dev, zpl_uint16 vlan, zpl_uint8 port, zpl_uint8 tag, zpl_uint8 wan)
{
	int i = 0, j = 0;
	for(i = 0; i < SWITCH_VLAN_MAX; i++)
	{
		if(dev->vlan_enable && dev->vlan[i].vlan == vlan)
		{
			for(j = 0; j < SWITCH_PORTS_MAX; j++)
			{
				if(dev->vlan[i].ports[port].is_active)
				{
					dev->vlan[i].ports[port].port = port;
					dev->vlan[i].ports[port].is_active = 1;
					dev->vlan[i].ports[port].tagged = tag;
					dev->vlan[i].ports[port].wan = wan;
					return OK;
				}
				dev->vlan[i].ports[port].port = port;
				dev->vlan[i].ports[port].is_active = 1;
				dev->vlan[i].ports[port].tagged = tag;
				dev->vlan[i].ports[port].wan = wan;
				return OK;
			}
		}
	}
	return ERROR;
}

static int _switch_dev_vlan_del_port(switch_dev_t *dev, zpl_uint16 vlan, zpl_uint8 port)
{
	int i = 0, j = 0;
	for(i = 0; i < SWITCH_VLAN_MAX; i++)
	{
		if(dev->vlan_enable && dev->vlan[i].vlan == vlan)
		{
			for(j = 0; j < SWITCH_PORTS_MAX; j++)
			{
				if(dev->vlan[i].ports[port].is_active)
				{
					dev->vlan[i].ports[port].port = 0;
					dev->vlan[i].ports[port].is_active = 0;
					dev->vlan[i].ports[port].tagged = 0;
					return OK;
				}
			}
		}
	}
	return ERROR;
}

static int _switch_dev_vlan_update_port(switch_dev_t *dev, zpl_uint16 vlan,
		zpl_uint8 port, zpl_uint8 tag, zpl_uint8 wan)
{
	int i = 0, j = 0;
	for(i = 0; i < SWITCH_VLAN_MAX; i++)
	{
		if(dev->vlan_enable && dev->vlan[i].vlan == vlan)
		{
			for(j = 0; j < SWITCH_PORTS_MAX; j++)
			{
				if(dev->vlan[i].ports[port].is_active)
				{
					dev->vlan[i].ports[port].port = port;
					dev->vlan[i].ports[port].is_active = 1;
					dev->vlan[i].ports[port].tagged = tag;
					dev->vlan[i].ports[port].wan = wan;
					return OK;
				}
			}
		}
	}
	return ERROR;
}

static int _switch_dev_vlan_port_disable(switch_dev_t *dev, zpl_uint8 port, zpl_uint8 disable)
{
	int i = 0, j = 0;
	char cmd[128];
	for(i = 0; i < SWITCH_VLAN_MAX; i++)
	{
		if(dev->vlan_enable && dev->vlan[i].vlan > 0)
		{
			for(j = 0; j < SWITCH_PORTS_MAX; j++)
			{
				if(dev->vlan[i].ports[port].is_active)
				{
					dev->vlan[i].ports[port].port = port;
					dev->vlan[i].ports[port].is_active = 1;
					dev->vlan[i].ports[port].disable = disable;
					memset(cmd, 0, sizeof(cmd));
					sprintf(cmd, "swconfig dev switch0 port %d set disable %d", port, disable);
					super_system(cmd);
					//swconfig dev switch0 port 0 set disable 0
					return OK;
				}
			}
		}
	}
	return ERROR;
}
#endif

static int web_switch_save_handle(Webs *wp, void *p)
{
	//argv = "ACTION=switch&BTNID=" + obj.id + "&port=" + port + "&vlanid=" + vlanid +
	//"&tagged=" + tagged + "&wanlan=" + wanlan;
	int ret = 0;
	zpl_uint8 port = 0;
	zpl_uint16 vlan = 0;
	zpl_uint8 tagged = 0;
	zpl_uint8 wanlan = 0;
	char *strval = NULL;
	strval = webs_get_var(wp, T("port"), T(""));
	if (NULL == strval)
	{
		return ERROR;//web_return_text_plain(wp, ERROR);
	}
	port = atoi(strval);

	strval = webs_get_var(wp, T("vlanid"), T(""));
	if (NULL == strval)
	{
		return ERROR;//web_return_text_plain(wp, ERROR);
	}
	vlan = atoi(strval);

	strval = webs_get_var(wp, T("tagged"), T(""));
	if (NULL == strval)
	{
		return ERROR;
	}
	if(strstr(strval, "untagged"))
		tagged = 0;
	else
		tagged = 1;

	strval = webs_get_var(wp, T("wanlan"), T(""));
	if (NULL == strval)
	{
		return ERROR;
	}
	if(strstr(strval, "WAN"))
		wanlan = 1;
	else
		wanlan = 0;
	ret = 0;
	if(switch_dev && (_switch_dev_vlan_create(switch_dev, vlan) == OK))
	{
		zpl_uint16 oldvlan = 0;//获取该接口所在的VLAN
		if(_switch_dev_vlan_get_port(switch_dev, &oldvlan, port) == OK)
		{
			if(oldvlan == vlan)//该接口所在的VLAN和配置的VLAN一致；更新接口信息
				ret |= _switch_dev_vlan_update_port(switch_dev, vlan, port, tagged, wanlan);
			else
			{
				//把该接口从旧的VLAN上删除
				ret |= _switch_dev_vlan_del_port(switch_dev, oldvlan, port);
				//把接口加到新的VLAN上
				ret |= _switch_dev_vlan_add_port(switch_dev, vlan, port, tagged, wanlan);
				//删除没有接口成员的VLAN
				ret |= _switch_dev_vlan_clean_by_noport(switch_dev);
			}
		}
		else
		{
			ret |= _switch_dev_vlan_add_port(switch_dev, vlan, port, tagged, wanlan);
		}
	}

	if(ret == 0)
	{
#ifdef ZPL_OPENWRT_UCI
		switch_dev_config_save();
#endif
		web_return_text_plain(wp, OK);
		return OK;
	}
	return ERROR;
}

static int web_switch_connect(Webs *wp, void *p)
{
	//argv = "ACTION=switch&BTNID=" + obj.id + "&" + "port=" + port + "&state=up";
	//argv = "ACTION=switch&BTNID=" + obj.id + "&" + "port=" + port + "&state=down";
	int port = 0;
	char *strval = NULL;
	strval = webs_get_var(wp, T("port"), T(""));
	if (NULL == strval)
	{
		return ERROR;//web_return_text_plain(wp, ERROR);
	}
	port = atoi(strval);
	strval = webs_get_var(wp, T("state"), T(""));
	if (NULL == strval)
	{
		return ERROR;//web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval, "down"))
	{
		if(_switch_dev_vlan_port_disable(switch_dev, port, 1) == OK)
			return web_return_text_plain(wp, OK);
		else
			return ERROR;//
	}
	else
	{
		if(_switch_dev_vlan_port_disable(switch_dev, port, 0) == OK)
			return web_return_text_plain(wp, OK);
		else
			return ERROR;//
		//return _switch_dev_vlan_port_disable(switch_dev, port, 0);
	}
}

static int web_switch_handle(Webs *wp, void *p)
{
	char *strval = NULL;
	strval = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL == strval)
	{
		return ERROR;//web_return_text_plain(wp, ERROR);

	}
	if(strstr(strval, "connect"))
	{
		return web_switch_connect(wp, p);
	}
	if(strstr(strval, "save"))
	{
		return web_switch_save_handle(wp, p);
	}
	return ERROR;
}

static int web_switch_port_tbl(Webs *wp, char *path, char *query)
{
	int i = 0, j = 0, hascpu = 0;
	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");
	wp->iValue = 0;
	if(switch_dev)
	{
		for(i = 0; i < SWITCH_VLAN_MAX; i++)
		{
			if(switch_dev->vlan_enable && switch_dev->vlan[i].vlan > 0)
			{
				for(j = 0; j < SWITCH_PORTS_MAX; j++)
				{
					if(switch_dev->vlan[i].ports[j].is_active)
					{
						if(switch_dev->vlan[i].ports[j].is_cpu && hascpu == 0)
						{
							/*
							if(wp->iValue > 0)
								websWrite(wp, "%s", ",");

							websWrite(wp, "{\"port\":\"%d\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"}",
									switch_dev->vlan[i].ports[j].port, "CPU", "ALL","tagged", "CPU", "up");
							wp->iValue++;
							hascpu = 1;
							*/
						}
						else
						{
							if(switch_dev->vlan[i].ports[j].wan)
							{
								if(wp->iValue > 0)
									websWrite(wp, "%s", ",");
								websWrite(wp, "{\"port\":\"%d\", \"name\":\"WAN%d\", \"vlan\":\"%d\","
									"\"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"}",
									switch_dev->vlan[i].ports[j].port,
									switch_dev->vlan[i].ports[j].port,
									switch_dev->vlan[i].vlan,
									switch_dev->vlan[i].ports[j].tagged ? "tagged":"untagged",
									switch_dev->vlan[i].ports[j].wan ? "WAN":"LAN",
									switch_dev->vlan[i].ports[j].disable ? "down":"up");
								wp->iValue++;
							}
							else
							{
								if(wp->iValue > 0)
									websWrite(wp, "%s", ",");
								websWrite(wp, "{\"port\":\"%d\", \"name\":\"LAN%d\", \"vlan\":\"%d\","
									"\"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"}",
									switch_dev->vlan[i].ports[j].port,
									switch_dev->vlan[i].ports[j].port,
									switch_dev->vlan[i].vlan,
									switch_dev->vlan[i].ports[j].tagged ? "tagged":"untagged",
									switch_dev->vlan[i].ports[j].wan ? "WAN":"LAN",
									switch_dev->vlan[i].ports[j].disable ? "down":"up");
								wp->iValue++;
							}
						}
					}
				}
			}
		}
	}
	if(wp->iValue > 0)
	{
		/*
		struct interface *ifp = NULL;
		ifp = if_lookup_by_name("wireless 0/0/1");
		if(ifp)
		{
			if(nsm_iw_mode(ifp) == IW_MODE_MANAGE || nsm_iw_mode(ifp) == IW_MODE_CLIENT)
			{
		websWrite(wp, "%s", ",");
		websWrite(wp, "{\"port\":\"%d\", \"name\":\"WIFI%d\", \"vlan\":\"%d\","
			"\"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"}",
			switch_dev->vlan[i].ports[j].port,
			switch_dev->vlan[i].ports[j].port,
			switch_dev->vlan[i].vlan,
			switch_dev->vlan[i].ports[j].tagged ? "tagged":"untagged",
			switch_dev->vlan[i].ports[j].wan ? "WAN":"LAN",
			switch_dev->vlan[i].ports[j].disable ? "down":"up");
			}
		}
		*/
	}
	wp->iValue = 0;
/*
	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"},",
			"1", "LAN1", "1","untagged","lan", "up");
	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"},",
			"2", "LAN2", "1","untagged","lan", "up");
	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"},",
			"3", "LAN3", "1","untagged","lan", "up");
	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"},",
			"4", "LAN4", "2","untagged","wan", "up");
	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"},",
			"5", "WWAN", "1","untagged","lan", "up");
	websWrite(wp, "{\"port\":\"%s\", \"name\":\"%s\", \"vlan\":\"%s\", \"tagged\":\"%s\", \"wanlan\":\"%s\", \"state\":\"%s\"}",
			"6", "CPU", "1","tagged","cpu", "up");
*/

	websWrite(wp, "%s", "]");
	websDone(wp);
	return 0;
}

#endif

int web_switch_app(void)
{

#ifdef ZPL_BUILD_OS_OPENWRT
	switch_dev_init();
	websFormDefine("port-tbl", web_switch_port_tbl);
	web_button_add_hook("switch", "save", web_switch_handle, NULL);
	web_button_add_hook("switch", "connect", web_switch_handle, NULL);
#endif

	return 0;
}
