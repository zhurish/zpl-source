/*
 * if_usp.c
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */
#include "zebra.h"
//#include "os_log.h"
#include "linklist.h"
#include "buffer.h"
#include "command.h"
#include "hash.h"
#include "if.h"
#include "if_name.h"
#include "linklist.h"
#include "memory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "os_memory.h"
#include "vty.h"


struct if_name_mgt
{
	if_type_t	type;
	const char *aname;
	const char *kname;
	const char *name;
};


struct if_name_mgt if_name_mgt[] =
{
	{IF_NONE, 			"NONE", "NONE", 	"NONE"},
	{IF_SERIAL, 		"SERIAL", "serial", 	"serial"},
	{IF_ETHERNET, 		"ETH", 	"eth", 		"ethernet"},
	{IF_GIGABT_ETHERNET, "GETH","geth", 	"gigabitethernet"},
	{IF_LOOPBACK, 		"LOOP", "loopback",		"loopback"},
	{IF_TUNNEL, 		"TUN", 	"tunnel",		"tunnel"},
	{IF_VLAN, 			"VLAN", "vlan", 	"vlan"},
	{IF_LAG, 			"LAG", 	"lag", 		"port-channel"},
	{IF_WIRELESS, 		"WIRE", 	"wireless", 	"wireless"},

	{IF_BRIGDE, 		"BRIGDE","br-lan", 	"brigde"},
#ifdef CUSTOM_INTERFACE
	{IF_WIFI, 			"WIFI",  "wan", 	"wifi"},
	{IF_MODEM, 			"MODEM", "modem",	"modem"},
#endif
	{IF_MAX, 			"NONE",  "NONE", 	"NONE"},
};


static int vty_iusp_explain (const char *string, int *unit, int *slot, int *port, int *id);


const char *getkernelname(if_type_t	type)
{
	int i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && if_name_mgt[i].type == type)
			return if_name_mgt[i].kname;
	}
	return NULL;
}

const char *getabstractname(if_type_t	type)
{
	int i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && if_name_mgt[i].type == type)
			return if_name_mgt[i].aname;
	}
	return NULL;
}

const char *getifpname(if_type_t	type)
{
	int i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && if_name_mgt[i].type == type)
			return if_name_mgt[i].name;
	}
	return NULL;
}

if_type_t kernelname2type(const char *name)
{
	int i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && (os_memcmp(if_name_mgt[i].kname, name, 3) == 0))
			return if_name_mgt[i].type;
	}
	return 0;
}

if_type_t abstractname2type(const char *name)
{
	int i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && (os_memcmp(if_name_mgt[i].aname, name, 3) == 0))
			return if_name_mgt[i].type;
	}
	return 0;
}

if_type_t name2type(const char *name)
{
	int i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && (os_memcmp(if_name_mgt[i].name, name, 3) == 0))
			return if_name_mgt[i].type;
	}
	return 0;
}

unsigned int if_name_hash_make(const char *name)
{
	if(name == NULL)
	{
		zlog_err(ZLOG_NSM,"ifname is NULL when make hash code");
		return ERROR;
	}
	if(if_ifname_split(name))
	{
		return string_hash_make(if_ifname_split(name));
	}
	zlog_err(ZLOG_NSM,"ifname :%s split ERROR when make hash code",name);
	return ERROR;
}

static const char * _if_name_make_argv(const char *ifname, const char *uspv)
{
	if_type_t type = 0;
	static char buf[INTERFACE_NAMSIZ];
	if(ifname == NULL || uspv == NULL)
	{
		zlog_err(ZLOG_NSM,"if type or uspv is NULL ifname when make ifname");
		return NULL;
	}
	os_memset(buf, 0, sizeof(buf));
	type = if_iftype_make(ifname);
	if( type == IF_SERIAL
			|| type == IF_ETHERNET
			|| type == IF_GIGABT_ETHERNET
			|| type == IF_TUNNEL
			|| type == IF_BRIGDE
			|| type == IF_WIRELESS

#ifdef CUSTOM_INTERFACE
			|| type == IF_WIFI
			|| type == IF_MODEM
#endif
			)
	{
		sprintf(buf, "%s %s", ifname, uspv);
		return buf;
	}
	else if(type == IF_VLAN || type == IF_LAG || type == IF_LOOPBACK)
	{
		sprintf(buf, "%s%s", ifname, uspv);
		return buf;
	}

	zlog_err(ZLOG_NSM,"if type ERRPR when make ifname:type=%s uspv=%s",
	           ifname ? ifname:"null", uspv ? uspv:"null");
	return NULL;
}

const char * if_ifname_format(const char *ifname, const char *uspv)
{
	return _if_name_make_argv(ifname, uspv);
}

/*
 * interface0/1/2 -> interface 0/1/2
 */
const char * if_ifname_split(const char *name)
{
	int n = 0;
	static char buf[INTERFACE_NAMSIZ];
	//p = strstr(name," ");
/*	n = os_strcspn(name, " ");
	if(n && n != os_strlen(name))
		return name;*/
	if(!string_have_space(name))
	{
		if(os_strlen(name))
			return name;
		zlog_err(ZLOG_NSM,"if name split ERROR input=%s",name);
		return NULL;
	}

	n = os_strcspn(name, "1234567890");
	if(n && n < os_strlen(name))
	{
		os_memset(buf, 0, sizeof(buf));
		os_memcpy(buf, name, n);
		os_strcat(buf, " ");
		os_strcat(buf, name + n);
		return buf;
	}
	zlog_err(ZLOG_NSM,"if name split ERROR input=%s",name);
	return NULL;
}

ifindex_t if_ifindex_make(const char *ifname, const char *uspv)
{
	ifindex_t ifindex = 0;
	int unit = 0, slot = 0, port = 0, id = 0, iuspv = 0;
	if_type_t type = 0;
	char *uspvstring = uspv;

	type = if_iftype_make(ifname);
	if(!uspv)
	{
		uspvstring = os_strstr(ifname, " ");
		if(uspvstring)
			uspvstring++;
		else
			uspvstring = uspv;
	}
	if( type == IF_SERIAL
			|| type == IF_ETHERNET
			|| type == IF_GIGABT_ETHERNET
			|| type == IF_TUNNEL
			|| type == IF_BRIGDE
			|| type == IF_WIRELESS
#ifdef CUSTOM_INTERFACE
			|| type == IF_WIFI
			|| type == IF_MODEM
#endif
			)
	{
		if(	vty_iusp_explain (uspvstring, &unit, &slot, &port, &id))
		{
			iuspv = IF_TYPE_SET(type) | IF_USPV_SET(unit, slot, port, id);
			ifindex = IF_IFINDEX_SET(type, iuspv);
			return ifindex;
		}
	}
	else if(type == IF_VLAN || type == IF_LAG || type == IF_LOOPBACK)
	{
		if(all_digit(uspvstring))
		{
			id = os_atoi(uspvstring);
			iuspv = IF_TYPE_SET(type) | IF_USPV_SET(unit, slot, port, id);
			ifindex = IF_IFINDEX_SET(type, iuspv);
		}
		return ifindex;
	}
	return ifindex;
}

static const char * if_ifname_make_by_ifindex(int abstract, ifindex_t ifindex)
{
	static char buf[INTERFACE_NAMSIZ];

	char **type_str = NULL;
	if(abstract)
		type_str = getabstractname(IF_TYPE_GET(ifindex));
	else
		type_str = getifpname(IF_TYPE_GET(ifindex));
	os_memset(buf, 0, sizeof(buf));
	if(IF_TYPE_GET(ifindex) == IF_SERIAL ||
			IF_TYPE_GET(ifindex) == IF_ETHERNET ||
			IF_TYPE_GET(ifindex) == IF_GIGABT_ETHERNET ||
			IF_TYPE_GET(ifindex) == IF_TUNNEL ||
			IF_TYPE_GET(ifindex) == IF_WIRELESS ||
#ifdef CUSTOM_INTERFACE
			IF_TYPE_GET(ifindex) == IF_WIFI ||
			IF_TYPE_GET(ifindex) == IF_MODEM ||
#endif
			IF_TYPE_GET(ifindex) == IF_BRIGDE )
	{
		if(IF_ID_GET(ifindex))
			sprintf(buf, "%s%d/%d/%d.%d", type_str,
					IF_UNIT_GET(ifindex),IF_SLOT_GET(ifindex),
					IF_PORT_GET(ifindex),IF_ID_GET(ifindex));
		else
			sprintf(buf, "%s%d/%d/%d", type_str,
					IF_UNIT_GET(ifindex),
			        IF_SLOT_GET(ifindex), IF_PORT_GET(ifindex) );
		return buf;
	}
	else if(IF_TYPE_GET(ifindex) == IF_VLAN)
	{
		sprintf(buf, "%s%d",type_str, IF_TYPE_CLR(ifindex) );
		return buf;
	}
	else if(IF_TYPE_GET(ifindex) == IF_LAG)
	{
		sprintf(buf, "%s%d",type_str, IF_TYPE_CLR(ifindex) );
		return buf;
	}
	else if(IF_TYPE_GET(ifindex) == IF_LOOPBACK)
	{
		sprintf(buf, "%s%d",type_str, IF_TYPE_CLR(ifindex) );
		return buf;
	}
	return NULL;
}

const char * if_ifname_make(ifindex_t ifindex)
{
	return if_ifname_make_by_ifindex(0, ifindex);
}

const char * if_ifname_abstract_make(ifindex_t ifindex)
{
	return if_ifname_make_by_ifindex(1, ifindex);
}

const char * if_ifname_alias_make(ifindex_t ifindex)
{
	return if_ifname_make_by_ifindex(1, ifindex);
}

if_type_t if_iftype_make(const char *str)
{
	if_type_t type = 0;
	if(str == NULL)
	{
		zlog_err(ZLOG_NSM,"if type format ERROR input ifname NULL");
		return type;
	}
	type = name2type(str);
	return type;
}

char *if_mac_out_format(unsigned char *mac, int len)
{
	static char buf[32];
	os_memset(buf, 0, sizeof(buf));
	os_snprintf(buf, sizeof(buf), "%02x%02x-%02x%02x-%02x%02x",
	        mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	return buf;
}


int if_uspv_type_setting(struct interface *ifp)
{
	//int iuspv = 0;
	char *str;
	int unit = 0, slot = 0, port = 0, id = 0;
	if(ifp == NULL || ifp->name == NULL)
	{
		zlog_err(ZLOG_NSM,"ifp is NULL when setting unit/solt/port code");
		return ERROR;
	}
	str = strstr(ifp->name," ");
	if(str == NULL)
		str = ifp->name;

	ifp->if_type = name2type(ifp->name);

	if(ifp->if_type == IF_SERIAL || ifp->if_type == IF_ETHERNET ||
			ifp->if_type == IF_GIGABT_ETHERNET || ifp->if_type == IF_TUNNEL ||
			ifp->if_type == IF_BRIGDE || ifp->if_type == IF_WIRELESS
#ifdef CUSTOM_INTERFACE
			|| ifp->if_type == IF_WIFI || ifp->if_type == IF_MODEM
#endif
			)
	{
		str++;
		if(vty_iusp_explain (str, &unit, &slot, &port, &id))
		{
			ifp->uspv = IF_TYPE_SET(ifp->if_type) | IF_USPV_SET(unit, slot, port, id);
			return OK;
		}
		zlog_err(ZLOG_NSM,"format unit/solt/port when setting unit/solt/port code,str=%s",str);
	}
	else if(ifp->if_type == IF_VLAN || ifp->if_type == IF_LAG || ifp->if_type == IF_LOOPBACK)
	{
		ifp->uspv = 0;
		return OK;
	}
	zlog_err(ZLOG_NSM,"format if type when setting unit/solt/port code,str=%s",str);
	return ERROR;
}

//vty_iusp_explain
static int vty_iusp_explain (const char *string, int *unit, int *slot, int *port, int *id)
{
	int len = 0;
	char buf[16];
	char *p, *v, *str;
	int num = 0, count = 0;
#ifdef CMD_IUSPV_SUPPORT
	char *base2 = "0123456789/.";
#else
	char *base2 = "0123456789/";
#endif
	if (string == NULL)
	{
		zlog_err(ZLOG_NSM,"if iusp format ERROR input is NULL");
		return 0;
	}
	str = string;
	while(string)
	{
		if(isdigit(*string))
		{
			str = string;
			break;
		}
		else
			string++;
	}

	count = strspn (str, base2);
	if (count != strlen (str))
	{
		zlog_err(ZLOG_NSM,"if iusp format ERROR input is:%s",str);
		return 0;
	}
	v = p = (char *)str;
	while (1)
	{
		p = strchr (p, '/');
		if (p != NULL)
		{
			num++;
			p++;
		}
		if (p == NULL)
			break;
	}
	count = num;
	if(unit)
		*unit = 0;
	if(slot)
		*slot = 0;
	if(port)
		*port = 0;
	if(id)
		*id = 0;
	v = (char *)str;
	p = strchr (str, '/');
	if (p == NULL)
	{
		zlog_err(ZLOG_NSM,"if iusp format ERROR can find '/'");
	    return 0;
	}
	len = p - v;
	strncpy (buf, v, len);
	if(count == 2)
	{
		if(unit)
			*unit = atoi (buf);

		p++;
		v = p;
		p = strchr (p, '/');
		if (p == NULL)
		{
			zlog_err(ZLOG_NSM,"if iusp format ERROR can find '/'");
		    return 0;
		}
		len = p - v;
		strncpy (buf, v, len);
		if(slot)
			*slot = atoi (buf);

		p++;
		v = p;
		p = strchr (p, '.');
		if(p)
		{
			len = p - v;
			strncpy (buf, v, len);
			if(port)
				*port = atoi (buf);
			p++;
			if(p)
			{
				if(id)
					*id = atoi (p);
			}
		}
		else
		{
			//p++;
			if(v)
			{
				if(port)
					*port = atoi (v);
			}
		}
	}
	else if(count == 1)
	{
		if(slot)
			*slot = atoi (buf);
		if(unit)
			*unit = 0;
		v = ++p;
		p = strchr (p, '.');
		if(p)
		{
			len = p - v;
			strncpy (buf, v, len);
			if(port)
				*port = atoi (buf);
			p++;
			if(p)
			{
				if(id)
					*id = atoi (p);
			}
		}
		else
		{
			//p++;
			if(v)
			{
				if(port)
					*port = atoi (v);
			}
		}
	}
#ifdef VTY_IUSP_DEBUG
	if(count == 2)
		fprintf(stderr,"input str:%s --> %d/%d/%d.%d\r\n",str,*unit,*slot,*port,*id);
	else//if(count == 2)
		fprintf(stderr,"input str:%s --> %d/%d/%d.%d\r\n",str,*unit,*slot,*port,*id);
#endif
	return 1;
}

int vty_iusp_get (const char *str, int *uspv)
{
	int unit = 0, slot = 0, port = 0, id = 0;
	char *uspv_str = strstr(str, " ");
	if_type_t type = if_iftype_make(str);
	if(type == IF_SERIAL || type == IF_ETHERNET ||
			type == IF_GIGABT_ETHERNET || type == IF_TUNNEL ||
			type == IF_BRIGDE || type == IF_WIRELESS
#ifdef CUSTOM_INTERFACE
			|| type == IF_WIFI || type == IF_MODEM
#endif
			)
	{
		if(uspv_str)
		{
			if(vty_iusp_explain (uspv_str, &unit, &slot, &port, &id))
			{
				if(uspv)
				{
					*uspv = (IF_TYPE_SET(type) | IF_USPV_SET(unit, slot, port, id));
				}
				return 1;
			}
		}
	}
	return 0;
}


int vty_mac_get (const char *str, unsigned char *mac)
{
	int count = 0,i = 0;
	char buf[16];
	char *p,*v = NULL;
	char *base2 = "0123456789abcdefABCDEF-";
	if (str == NULL)
	{
		zlog_err(ZLOG_NSM,"mac address format ERROR input NULL");
		return 0;
	}
	count = strspn(str,base2);
	if(count != strlen(str))
	{
		zlog_err(ZLOG_NSM,"mac address decode ERROR input:%s",str);
		return 0;
	}
	//base = os_strdup(str);
	//if(base)
	v = p = (char *)str;
	while(1)
	{
		v = p;
		p = strchr (v, '-');
		if(p)
		{
			strncpy (buf, v, p - v);
			count = strtol (buf,NULL,16);
			mac[i++] = ((count >> 8)&0xff);
			mac[i++] = (count & 0xff);
			p++;
		}
		else
		{
			//p++;
			count = strtol (v, NULL, 16);
			mac[i++] = ((count >> 8)&0xff);
			mac[i++] = (count & 0xff);
			break;
		}
		if(i==5)
			break;
	}
#ifdef VTY_IUSP_DEBUG
	//fprintf(stderr,"input str:%s --> %02x%0x2-%02x%02x-%02x%02x\r\n",str,
	//        mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
#endif
	return 1;
}


int if_loopback_ifindex_create(if_type_t type, const char *name)
{
	int i = 0;
	char strc[16];
	memset(strc, 0, sizeof(strc));
	char *str = name;
	for (; *str != '\0'; str++)
	{
		if (isdigit ((int) *str))
			strc[i++] = *str;
	}
	if(i != 0)
		return atoi(strc);
	return 0;
/*
	int i = 0;
	char strc[16];
	memset(strc, 0, sizeof(strc));
	char *str = name;
	while(str)
	{
		if(isdigit(*str))
		{
			strc[i++] = str;
			break;
		}
		else
			str++;
	}
	if(strc[0]!=0)
		return atoi(strc);
	return 0;
*/
}


/*
int if_kernel_ifindex_update(struct interface *ifp)
{
	//ifp->k_ifindex = 0;
	//ifp->k_name = 0;
#ifdef PL_PAL_MODULE
	pal_kernel_update (ifp);
#endif
	ifp->k_name_hash = if_name_hash_make(ifp->k_name);
	return 0;
}
*/
