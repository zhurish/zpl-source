/*
 * if_usp.c
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "if.h"
#include "if_name.h"
#include "hash.h"
#include "str.h"
#include "log.h"
#include "prefix.h"


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
	{IF_XGIGABT_ETHERNET, "XGETH","xgeth", 	"xgigabitethernet"},
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
	{IF_VXLAN, 			"XVLAN", "xvlan", 	"xvlan"},
	{IF_E1, 			"E1", 	"e1", 		"e1"},
	{IF_EPON, 			"EPON", 	"epon", 	"epon"},
	{IF_GPON, 			"GEPON","gpon", 	"gpon"},
	{IF_MAX, 			"NONE",  "NONE", 	"NONE"},
};
 
static int vty_iusp_explain (const char *string, zpl_uint32 *unit, zpl_uint32 *slot, zpl_uint32 *port, zpl_uint32 *id, zpl_uint32 *rend);
static const char * if_ifname_make_by_ifindex(zpl_bool abstract, ifindex_t ifindex);

const char *getkernelname(if_type_t	type)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && if_name_mgt[i].type == type)
			return if_name_mgt[i].kname;
	}
	return NULL;
}

const char *getabstractname(if_type_t	type)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && if_name_mgt[i].type == type)
			return if_name_mgt[i].aname;
	}
	return NULL;
}

const char *getifpname(if_type_t	type)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && if_name_mgt[i].type == type)
			return if_name_mgt[i].name;
	}
	return NULL;
}

if_type_t kernelname2type(const char *name)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && (os_memcmp(if_name_mgt[i].kname, name, 3) == 0))
			return if_name_mgt[i].type;
	}
	return 0;
}

if_type_t abstractname2type(const char *name)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && (os_memcmp(if_name_mgt[i].aname, name, 3) == 0))
			return if_name_mgt[i].type;
	}
	return 0;
}

if_type_t name2type(const char *name)
{
	zpl_uint32 i = 0;
	for(i = 0; i < array_size(if_name_mgt); i++)
	{
		if(if_name_mgt[i].type && (os_memcmp(if_name_mgt[i].name, name, 3) == 0))
			return if_name_mgt[i].type;
	}
	return 0;
}

zpl_uint32  if_name_hash_make(const char *name)
{
	if(name == NULL)
	{
		zlog_err(MODULE_DEFAULT,"ifname is NULL when make hash code");
		return ERROR;
	}
	if(if_ifname_split(name))
	{
		return string_hash_make(if_ifname_split(name));
	}
	zlog_err(MODULE_DEFAULT,"ifname :%s split ERROR when make hash code",name);
	return ERROR;
}

static const char * _if_name_make_argv(const char *ifname, const char *uspv)
{
	if_type_t type = 0;
	static zpl_char buf[IF_NAME_MAX];
	if(ifname == NULL || uspv == NULL)
	{
		zlog_err(MODULE_DEFAULT,"if type or uspv is NULL ifname when make ifname");
		return NULL;
	}
	os_memset(buf, 0, sizeof(buf));
	type = if_iftype_make(ifname);
	if( type == IF_SERIAL
			|| type == IF_ETHERNET
			|| type == IF_GIGABT_ETHERNET
			|| type == IF_XGIGABT_ETHERNET
			|| type == IF_TUNNEL
			|| type == IF_BRIGDE
			|| type == IF_WIRELESS
			|| type == IF_VXLAN
			|| type == IF_E1
			|| type == IF_EPON
			|| type == IF_GPON
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
	zlog_err(MODULE_DEFAULT,"if type ERRPR when make ifname:type=%s uspv=%s",
	           ifname ? ifname:"null", uspv ? uspv:"null");
	return NULL;
}

const char * if_ifname_format(const char *ifname, const char *uspv)
{
	return _if_name_make_argv(ifname, uspv);
}

ifindex_t if_uspv2ifindex(if_type_t type, zpl_uint32 unit, zpl_uint32 slot, zpl_uint32 port, zpl_uint32 vid)
{
	ifindex_t ifindex = 0;
	zpl_uint32 iuspv = 0;
	if( type == IF_SERIAL
			|| type == IF_ETHERNET
			|| type == IF_GIGABT_ETHERNET
			|| type == IF_XGIGABT_ETHERNET
			|| type == IF_TUNNEL
			|| type == IF_BRIGDE
			|| type == IF_WIRELESS
			|| type == IF_VXLAN
			|| type == IF_E1
			|| type == IF_EPON
			|| type == IF_GPON
#ifdef CUSTOM_INTERFACE
			|| type == IF_WIFI
			|| type == IF_MODEM
#endif
			)
	{
		iuspv = IF_USPV_SET(unit, slot, port, vid);
		ifindex = IF_IFINDEX_SET(type, iuspv);
		return ifindex;
	}
	else if(type == IF_VLAN || type == IF_LAG || type == IF_LOOPBACK)
	{
		iuspv = IF_USPV_SET(unit, slot, port, vid);
		ifindex = IF_IFINDEX_SET(type, iuspv);
		return ifindex;
	}
	return ifindex;
}

const char * if_uspv2ifname(if_type_t type, zpl_uint32 unit, zpl_uint32 slot, zpl_uint32 port, zpl_uint32 vid)
{
	ifindex_t ifindex = if_uspv2ifindex( type,  unit,  slot,  port,  vid);
	return if_ifname_make_by_ifindex(zpl_false, ifindex);
}
/*
 * interface0/1/2 -> interface 0/1/2
 */
const char * if_ifname_split(const char *name)
{
	zpl_uint32 n = 0;
	static zpl_char buf[IF_NAME_MAX];
	//p = strstr(name," ");
/*	n = os_strcspn(name, " ");
	if(n && n != os_strlen(name))
		return name;*/
	if(!string_have_space(name))
	{
		if(os_strlen(name))
			return name;
		zlog_err(MODULE_DEFAULT,"if name split ERROR input=%s",name);
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
	zlog_err(MODULE_DEFAULT,"if name split ERROR input=%s",name);
	return NULL;
}

ifindex_t if_ifindex_make(const char *ifname, const char *uspv)
{
	ifindex_t ifindex = 0;
	zpl_uint32 unit = 0, slot = 0, port = 0, id = 0, rend = 0;
	if_type_t type = 0;
	zpl_char *uspvstring = ifname;

	type = if_iftype_make(ifname);
	if(uspv)
	{
		uspvstring = uspv;
	}
	else
	{
		char *strb = uspvstring;
		for (; *strb != '\0'; strb++)
		{
			if (isdigit ((int) *strb))
			{
				break;
			}
		}
		uspvstring = strb;
	}
//printf("========================%s========================type=%d-->%s\r\n", __func__, type, uspvstring);
	if( type == IF_SERIAL
			|| type == IF_ETHERNET
			|| type == IF_GIGABT_ETHERNET
			|| type == IF_XGIGABT_ETHERNET
			|| type == IF_TUNNEL
			|| type == IF_BRIGDE
			|| type == IF_WIRELESS
			|| type == IF_VXLAN
			|| type == IF_E1
			|| type == IF_EPON
			|| type == IF_GPON
#ifdef CUSTOM_INTERFACE
			|| type == IF_WIFI
			|| type == IF_MODEM
#endif
			)
	{
		if(uspvstring && vty_iusp_explain (uspvstring, &unit, &slot, &port, &id, &rend))
		{
			ifindex = if_uspv2ifindex( type,  unit,  slot,  port,  id);
			return ifindex;
		}
	}
	else if(type == IF_VLAN || type == IF_LAG || type == IF_LOOPBACK)
	{
		if(uspvstring && all_digit(uspvstring))
		{
			id = os_atoi(uspvstring);
			ifindex = if_uspv2ifindex( type,  unit,  slot,  port,  id);
		}
		return ifindex;
	}
	return ifindex;
}

static const char * if_ifname_make_by_ifindex(zpl_bool abstract, ifindex_t ifindex)
{
	static zpl_char buf[IF_NAME_MAX];

	zpl_char *type_str = NULL;
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
			IF_TYPE_GET(ifindex) == IF_XGIGABT_ETHERNET ||
			IF_TYPE_GET(ifindex) == IF_VXLAN ||
			IF_TYPE_GET(ifindex) == IF_E1 ||
			IF_TYPE_GET(ifindex) == IF_EPON ||
			IF_TYPE_GET(ifindex) == IF_GPON ||
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
		zlog_err(MODULE_DEFAULT,"if type format ERROR input ifname NULL");
		return type;
	}
	type = name2type(str);
	return type;
}

const char *if_mac_out_format(zpl_uchar *mac)
{
	static zpl_char buf[32];
	os_memset(buf, 0, sizeof(buf));
	os_snprintf(buf, sizeof(buf), "%02x%02x-%02x%02x-%02x%02x",
	        mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	return (const char *)buf;
}


int if_uspv_type_setting(struct interface *ifp)
{
	zpl_char *str = NULL;
	zpl_uint32 unit = 0, slot = 0, port = 0, id = 0, rend = 0;
	if(ifp == NULL || ifp->name == NULL)
	{
		zlog_err(MODULE_DEFAULT,"ifp is NULL when setting unit/solt/port code");
		return ERROR;
	}
	str = strstr(ifp->name," ");
	if(str == NULL)
		str = ifp->name;


	if(ifp->if_type == IF_SERIAL || ifp->if_type == IF_ETHERNET ||
			ifp->if_type == IF_GIGABT_ETHERNET || ifp->if_type == IF_TUNNEL ||
			ifp->if_type == IF_BRIGDE || ifp->if_type == IF_WIRELESS
#ifdef CUSTOM_INTERFACE
			|| ifp->if_type == IF_WIFI || ifp->if_type == IF_MODEM
#endif
			|| ifp->if_type == IF_XGIGABT_ETHERNET || ifp->if_type == IF_VXLAN
			|| ifp->if_type == IF_E1 || ifp->if_type == IF_EPON
			|| ifp->if_type == IF_GPON
			)
	{
		str++;
		if(vty_iusp_explain (str, &unit, &slot, &port, &id, &rend))
		{
			ifp->uspv = IF_TYPE_SET(ifp->if_type) | IF_USPV_SET(unit, slot, port, id);
			//ifp->k_name[IF_NAME_MAX + 1];
			//ifp->k_name_hash;
			//ifp->k_ifindex;
			//ifp->phyid = id;
			zlog_err(MODULE_DEFAULT,"============ unit/solt/port 0x%x %d/%d/%d %d  uspv=0x%x",ifp->if_type, unit, slot, port, id, ifp->uspv);
			return OK;
		}
		zlog_err(MODULE_DEFAULT,"format unit/solt/port when setting unit/solt/port code,str=%s",str);
	}
	else if(ifp->if_type == IF_VLAN || ifp->if_type == IF_LAG || ifp->if_type == IF_LOOPBACK)
	{
		ifp->uspv = 0;
		
		return OK;
	}
	zlog_err(MODULE_DEFAULT,"format if type when setting unit/solt/port code,str=%s",str);
	return ERROR;
}


#if 1
static int vty_iusp_explain (const char *string, zpl_uint32 *unit, zpl_uint32 *slot, 
	zpl_uint32 *port, zpl_uint32 *id, zpl_uint32 *rend)
{
	zpl_char *str = NULL;
	zpl_uint32 ounit = 0, oslot = 0, oport = 0,  oid= 0;
	zpl_uint32  count = 0;
#ifdef CMD_IUSPV_SUPPORT
	zpl_char *base2 = "0123456789/.-";
#else
	zpl_char *base2 = "0123456789/-";
#endif
	if (string == NULL)
	{
		zlog_err(MODULE_DEFAULT,"if iusp format ERROR input is NULL");
		return 0;
	}
	//os_memset(buf, 0, sizeof(buf));
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
		zlog_err(MODULE_DEFAULT,"if iusp format ERROR input is:%s(%s)",str, string);
		return 0;
	}
	if(strchr_count(str, '/') == 2)
	{
		if(strstr(str, ".") && strchr_count(str, '-'))
		{
			//sscanf(str, "%d[^/]/%d[^/]/%d[^.].%d", &ounit, &oslot, &oport, &oid);
			sscanf(str, "%d/%d/%d.%d-%d", &ounit, &oslot, &oport, &oid, rend);
		}
		else if(strstr(str, ".") && !strchr_count(str, '-'))
			sscanf(str, "%d/%d/%d.%d", &ounit, &oslot, &oport, &oid);
		else if(!strstr(str, ".") && strchr_count(str, '-'))
			sscanf(str, "%d/%d/%d-%d", &ounit, &oslot, &oport, rend);
		else if(!strstr(str, ".") && !strchr_count(str, '-'))
			sscanf(str, "%d/%d/%d", &ounit, &oslot, &oport);
			//sscanf(str, "%d[^/]/%d[^/]/%d", &ounit, &oslot, &oport);
	}
	else if(strchr_count(str, '/') == 1)
	{
		if(strstr(str, ".") && strchr_count(str, '-'))
		{
			//sscanf(str, "%d[^/]/%d[^/]/%d[^.].%d", &ounit, &oslot, &oport, &oid);
			sscanf(str, "%d/%d.%d-%d", &oslot, &oport, &oid, rend);
		}
		else if(strstr(str, ".") && !strchr_count(str, '-'))
			sscanf(str, "%d/%d.%d", &oslot, &oport, &oid);
		else if(!strstr(str, ".") && strchr_count(str, '-'))
			sscanf(str, "%d/%d-%d", &oslot, &oport, rend);
		else if(!strstr(str, ".") && !strchr_count(str, '-'))
			sscanf(str, "%d/%d", &oslot, &oport);
	}
	if(unit)
		*unit = ounit;
	if(slot)
		*slot = oslot;
	if(port)
		*port = oport;
	if(id)
		*id = oid;

	//fprintf(stderr,"---%s---:input str:%s --> %d/%d/%d.%d\r\n", __func__, str,*unit,*slot,*port,*id);

#ifdef VTY_IUSP_DEBUG
	//if(count == 2)
		fprintf(stderr,"input str:%s --> %d/%d/%d.%d\r\n",str,*unit,*slot,*port,*id);
	//else//if(count == 2)
	//	fprintf(stderr,"input str:%s --> %d/%d/%d.%d\r\n",str,*unit,*slot,*port,*id);
#endif
	return 1;
}

int vty_iusp_get (const char *str, zpl_uint32 *uspv, zpl_uint32 *end)
{
	zpl_uint32 unit = 0, slot = 0, port = 0, id = 0;
	zpl_char *uspv_str = strstr(str, " ");
	if_type_t type = if_iftype_make(str);
	if(type == IF_SERIAL || type == IF_ETHERNET ||
			type == IF_GIGABT_ETHERNET || type == IF_TUNNEL ||
			type == IF_BRIGDE || type == IF_WIRELESS
#ifdef CUSTOM_INTERFACE
			|| type == IF_WIFI || type == IF_MODEM
#endif
			|| type == IF_XGIGABT_ETHERNET || type == IF_VXLAN
			|| type == IF_E1 || type == IF_EPON
			|| type == IF_GPON
			)
	{
		if(uspv_str)
		{
			if(vty_iusp_explain (uspv_str, &unit, &slot, &port, &id, end))
			{
				if(uspv)
				{
					*uspv = (IF_TYPE_SET(type) | IF_USPV_SET(unit, slot, port, id));
				}
				return 1;
			}
		}
		else
		{
			if(vty_iusp_explain (str, &unit, &slot, &port, &id, end))
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

/*
 * 0000-0000-0000 --> 00:00:00:00:00:00
 */
int vty_mac_get (const char *str, zpl_uchar *mac)
{
	zpl_uint32 count = 0,i = 0;
	zpl_char buf[16];
	zpl_char *p,*v = NULL;
	zpl_char *base2 = "0123456789abcdefABCDEF-";
	if (str == NULL)
	{
		zlog_err(MODULE_DEFAULT,"mac address format ERROR input NULL");
		return 0;
	}
	os_memset(buf, 0, sizeof(buf));
	count = strspn(str,base2);
	if(count != strlen(str))
	{
		zlog_err(MODULE_DEFAULT,"mac address decode ERROR input:%s",str);
		return 0;
	}
	//base = os_strdup(str);
	//if(base)
	v = p = (zpl_char *)str;
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
#else
//vty_iusp_explain
static int vty_iusp_explain (const char *string, int *unit, int *slot, int *port, int *id)
{
	zpl_uint32 len = 0;
	zpl_char buf[16];
	zpl_char *p, *v, *str;
	int num = 0, count = 0;
#ifdef CMD_IUSPV_SUPPORT
	zpl_char *base2 = "0123456789/.";
#else
	zpl_char *base2 = "0123456789/";
#endif
	if (string == NULL)
	{
		zlog_err(MODULE_DEFAULT,"if iusp format ERROR input is NULL");
		return 0;
	}
	os_memset(buf, 0, sizeof(buf));
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
		zlog_err(MODULE_DEFAULT,"if iusp format ERROR input is:%s",str);
		return 0;
	}
	v = p = (zpl_char *)str;
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
	v = (zpl_char *)str;
	p = strchr (str, '/');
	if (p == NULL)
	{
		zlog_err(MODULE_DEFAULT,"if iusp format ERROR can find '/'");
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
			zlog_err(MODULE_DEFAULT,"if iusp format ERROR can find '/'");
		    return 0;
		}
		len = p - v;
		os_memset(buf, 0, sizeof(buf));
		strncpy (buf, v, len);
		if(slot)
			*slot = atoi (buf);

		p++;
		v = p;
		p = strchr (p, '.');
		if(p)
		{
			len = p - v;
			os_memset(buf, 0, sizeof(buf));
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
			os_memset(buf, 0, sizeof(buf));
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
	zpl_char *uspv_str = strstr(str, " ");
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


int vty_mac_get (const char *str, zpl_uchar *mac)
{
	int count = 0,i = 0;
	zpl_char buf[16];
	zpl_char *p,*v = NULL;
	zpl_char *base2 = "0123456789abcdefABCDEF-";
	if (str == NULL)
	{
		zlog_err(MODULE_DEFAULT,"mac address format ERROR input NULL");
		return 0;
	}
	os_memset(buf, 0, sizeof(buf));
	count = strspn(str,base2);
	if(count != strlen(str))
	{
		zlog_err(MODULE_DEFAULT,"mac address decode ERROR input:%s",str);
		return 0;
	}
	//base = os_strdup(str);
	//if(base)
	v = p = (zpl_char *)str;
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
#endif

int if_loopback_ifindex_create(if_type_t type, const char *name)
{
	zpl_uint32 i = 0;
	zpl_char strc[16];
	memset(strc, 0, sizeof(strc));
	zpl_char *str = name;
	for (; *str != '\0'; str++)
	{
		if (isdigit ((int) *str))
			strc[i++] = *str;
	}
	if(i != 0)
		return atoi(strc);
	return 0;
}

