/* FIB SNMP.
 * Copyright (C) 1999 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

/*
 * Currently SNMP is only running properly for MIBs in the default VRF.
 */

#include "auto_include.h"
#include "zplos_include.h"

#include "lib_include.h"
#include "nsm_include.h"


#ifdef ZPL_NSM_SNMP
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "smux.h"
//#include <net-snmp/agent/snmp_vars.h>
    typedef int     (WriteMethod) (int action,
                                   u_char * var_val,
                                   u_char var_val_type,
                                   size_t var_val_len,
                                   u_char * statP,
                                   oid * name, size_t length);

#define IPFWMIB 1,3,6,1,2,1,4,24

/* ipForwardTable */
#define IPFORWARDDEST                         1
#define IPFORWARDMASK                         2
#define IPFORWARDPOLICY                       3
#define IPFORWARDNEXTHOP                      4
#define IPFORWARDIFINDEX                      5
#define IPFORWARDTYPE                         6
#define IPFORWARDPROTO                        7
#define IPFORWARDAGE                          8
#define IPFORWARDINFO                         9
#define IPFORWARDNEXTHOPAS                   10
#define IPFORWARDMETRIC1                     11
#define IPFORWARDMETRIC2                     12
#define IPFORWARDMETRIC3                     13
#define IPFORWARDMETRIC4                     14
#define IPFORWARDMETRIC5                     15

/* ipCidrRouteTable */
#define IPCIDRROUTEDEST                       1
#define IPCIDRROUTEMASK                       2
#define IPCIDRROUTETOS                        3
#define IPCIDRROUTENEXTHOP                    4
#define IPCIDRROUTEIFINDEX                    5
#define IPCIDRROUTETYPE                       6
#define IPCIDRROUTEPROTO                      7
#define IPCIDRROUTEAGE                        8
#define IPCIDRROUTEINFO                       9
#define IPCIDRROUTENEXTHOPAS                 10
#define IPCIDRROUTEMETRIC1                   11
#define IPCIDRROUTEMETRIC2                   12
#define IPCIDRROUTEMETRIC3                   13
#define IPCIDRROUTEMETRIC4                   14
#define IPCIDRROUTEMETRIC5                   15
#define IPCIDRROUTESTATUS                    16

#define INTEGER32 ASN_INTEGER
#define GAUGE32 ASN_GAUGE
#define ENUMERATION ASN_INTEGER
#define ROWSTATUS ASN_INTEGER
#define IPADDRESS ASN_IPADDRESS
#define OBJECTIDENTIFIER ASN_OBJECT_ID


oid ipfw_oid [] = { IPFWMIB };

/* Hook functions. */
static zpl_uchar * ipFwNumber (struct variable *, oid [], zpl_size_t *,
		     int, zpl_size_t *, WriteMethod **);
static zpl_uchar * ipFwTable (struct variable *, oid [], zpl_size_t *,
			   int, zpl_size_t *, WriteMethod **);
static zpl_uchar * ipCidrNumber (struct variable *, oid [], zpl_size_t *,
			      int, zpl_size_t *, WriteMethod **);
static zpl_uchar * ipCidrTable (struct variable *, oid [], zpl_size_t *,
			     int, zpl_size_t *, WriteMethod **);


struct variable nsm_variables[] = 
  {
    {0, GAUGE32, RONLY, ipFwNumber, 1, {1}},
    {IPFORWARDDEST, IPADDRESS, RONLY, ipFwTable, 3, {2, 1, 1}},
    {IPFORWARDMASK, IPADDRESS, RONLY, ipFwTable, 3, {2, 1, 2}},
    {IPFORWARDPOLICY, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 3}},
    {IPFORWARDNEXTHOP, IPADDRESS, RONLY, ipFwTable, 3, {2, 1, 4}},
    {IPFORWARDIFINDEX, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 5}},
    {IPFORWARDTYPE, ENUMERATION, RONLY, ipFwTable, 3, {2, 1, 6}},
    {IPFORWARDPROTO, ENUMERATION, RONLY, ipFwTable, 3, {2, 1, 7}},
    {IPFORWARDAGE, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 8}},
    {IPFORWARDINFO, OBJECTIDENTIFIER, RONLY, ipFwTable, 3, {2, 1, 9}},
    {IPFORWARDNEXTHOPAS, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 10}},
    {IPFORWARDMETRIC1, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 11}},
    {IPFORWARDMETRIC2, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 12}},
    {IPFORWARDMETRIC3, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 13}},
    {IPFORWARDMETRIC4, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 14}},
    {IPFORWARDMETRIC5, INTEGER32, RONLY, ipFwTable, 3, {2, 1, 15}},
    {0, GAUGE32, RONLY, ipCidrNumber, 1, {3}},
    {IPCIDRROUTEDEST, IPADDRESS, RONLY, ipCidrTable, 3, {4, 1, 1}},
    {IPCIDRROUTEMASK, IPADDRESS, RONLY, ipCidrTable, 3, {4, 1, 2}},
    {IPCIDRROUTETOS, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 3}},
    {IPCIDRROUTENEXTHOP, IPADDRESS, RONLY, ipCidrTable, 3, {4, 1, 4}},
    {IPCIDRROUTEIFINDEX, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 5}},
    {IPCIDRROUTETYPE, ENUMERATION, RONLY, ipCidrTable, 3, {4, 1, 6}},
    {IPCIDRROUTEPROTO, ENUMERATION, RONLY, ipCidrTable, 3, {4, 1, 7}},
    {IPCIDRROUTEAGE, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 8}},
    {IPCIDRROUTEINFO, OBJECTIDENTIFIER, RONLY, ipCidrTable, 3, {4, 1, 9}},
    {IPCIDRROUTENEXTHOPAS, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 10}},
    {IPCIDRROUTEMETRIC1, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 11}},
    {IPCIDRROUTEMETRIC2, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 12}},
    {IPCIDRROUTEMETRIC3, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 13}},
    {IPCIDRROUTEMETRIC4, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 14}},
    {IPCIDRROUTEMETRIC5, INTEGER32, RONLY, ipCidrTable, 3, {4, 1, 15}},
    {IPCIDRROUTESTATUS, ROWSTATUS, RONLY, ipCidrTable, 3, {4, 1, 16}}
  };


static zpl_uchar *
ipFwNumber (struct variable *v, oid objid[], zpl_size_t *objid_len,
	    int exact, zpl_size_t *val_len, WriteMethod **write_method)
{
  static int result;
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;

  if (smux_header_generic(v, objid, objid_len, exact, val_len, write_method) == MATCH_FAILED)
    return NULL;

  table = nsm_vrf_table (AFI_IP, SAFI_UNICAST, VRF_DEFAULT);
  if (! table)
    return NULL;

  /* Return number of routing entries. */
  result = 0;
  for (rn = route_top (table); rn; rn = route_next (rn))
    RNODE_FOREACH_RIB (rn, rib)
      result++;

  return (zpl_uchar *)&result;
}

static zpl_uchar *
ipCidrNumber (struct variable *v, oid objid[], zpl_size_t *objid_len,
	      int exact, zpl_size_t *val_len, WriteMethod **write_method)
{
  static int result;
  struct route_table *table;
  struct route_node *rn;
  struct rib *rib;

  if (smux_header_generic(v, objid, objid_len, exact, val_len, write_method) == MATCH_FAILED)
    return NULL;

  table = nsm_vrf_table (AFI_IP, SAFI_UNICAST, VRF_DEFAULT);
  if (! table)
    return 0;

  /* Return number of routing entries. */
  result = 0;
  for (rn = route_top (table); rn; rn = route_next (rn))
    RNODE_FOREACH_RIB (rn, rib)
      result++;

  return (zpl_uchar *)&result;
}

static int
in_addr_cmp(zpl_uchar *p1, zpl_uchar *p2)
{
  zpl_uint32 i;

  for (i=0; i<4; i++)
    {
      if (*p1 < *p2)
        return -1;
      if (*p1 > *p2)
        return 1;
      p1++; p2++;
    }
  return 0;
}

static int 
in_addr_add(zpl_uchar *p, int num)
{
  zpl_uint32 i, ip0;

  ip0 = *p;
  p += 4;
  for (i = 3; 0 <= i; i--) {
    p--;
    if (*p + num > 255) {
      *p += num;
      num = 1;
    } else {
      *p += num;
      return 1;
    }
  }
  if (ip0 > *p) {
    /* ip + num > 0xffffffff */
    return 0;
  }
  
  return 1;
}

static int
proto_trans(zpl_uint32 type)
{
  switch (type)
    {
    case ZPL_ROUTE_PROTO_SYSTEM:
      return 1; /* other */
    case ZPL_ROUTE_PROTO_KERNEL:
      return 1; /* other */
    case ZPL_ROUTE_PROTO_CONNECT:
      return 2; /* local interface */
    case ZPL_ROUTE_PROTO_STATIC:
      return 3; /* static route */
    case ZPL_ROUTE_PROTO_RIP:
      return 8; /* rip */
    case ZPL_ROUTE_PROTO_RIPNG:
      return 1; /* shouldn't happen */
    case ZPL_ROUTE_PROTO_OSPF:
      return 13; /* ospf */
    case ZPL_ROUTE_PROTO_OSPF6:
      return 1; /* shouldn't happen */
    case ZPL_ROUTE_PROTO_BGP:
      return 14; /* bgp */
    default:
      return 1; /* other */
    }
}

static void
check_replace(struct route_node *np2, struct rib *rib2, 
              struct route_node **np, struct rib **rib)
{
  zpl_proto_t proto, proto2;

  if (!*np)
    {
      *np = np2;
      *rib = rib2;
      return;
    }

  if (in_addr_cmp(&(*np)->p.u.prefix, &np2->p.u.prefix) < 0)
    return;
  if (in_addr_cmp(&(*np)->p.u.prefix, &np2->p.u.prefix) > 0)
    {
      *np = np2;
      *rib = rib2;
      return;
    }

  proto = proto_trans((*rib)->type);
  proto2 = proto_trans(rib2->type);

  if (proto2 > proto)
    return;
  if (proto2 < proto)
    {
      *np = np2;
      *rib = rib2;
      return;
    }

  if (in_addr_cmp((zpl_uchar *)&(*rib)->nexthop->gate.ipv4, 
                  (zpl_uchar *)&rib2->nexthop->gate.ipv4) <= 0)
    return;

  *np = np2;
  *rib = rib2;
  return;
}

static void
get_fwtable_route_node(struct variable *v, oid objid[], zpl_size_t *objid_len, 
		       int exact, struct route_node **np, struct rib **rib)
{
  struct ipstack_in_addr dest;
  struct route_table *table;
  struct route_node *np2;
  struct rib *rib2;
  zpl_proto_t proto;
  zpl_uint32 policy;
  struct ipstack_in_addr nexthop_addr;
  zpl_uchar *pnt;
  zpl_uint32 i;

  /* Init index variables */

  pnt = (zpl_uchar *) &dest;
  for (i = 0; i < 4; i++)
    *pnt++ = 0;

  pnt = (zpl_uchar *) &nexthop_addr;
  for (i = 0; i < 4; i++)
    *pnt++ = 0;

  proto = 0;
  policy = 0;
 
  /* Init return variables */

  *np = NULL;
  *rib = NULL;

  /* Short circuit exact matches of wrong length */

  if (exact && (*objid_len != (unsigned) v->namelen + 10))
    return;

  table = nsm_vrf_table (AFI_IP, SAFI_UNICAST, VRF_DEFAULT);
  if (! table)
    return;

  /* Get INDEX information out of OID.
   * ipForwardDest, ipForwardProto, ipForwardPolicy, ipForwardNextHop
   */

  if (*objid_len > (unsigned) v->namelen)
    oid2in_addr (objid + v->namelen, MIN(4, *objid_len - v->namelen), &dest);

  if (*objid_len > (unsigned) v->namelen + 4)
    proto = objid[v->namelen + 4];

  if (*objid_len > (unsigned) v->namelen + 5)
    policy = objid[v->namelen + 5];

  if (*objid_len > (unsigned) v->namelen + 6)
    oid2in_addr (objid + v->namelen + 6, MIN(4, *objid_len - v->namelen - 6),
		 &nexthop_addr);

  /* Apply GETNEXT on not exact search */

  if (!exact && (*objid_len >= (unsigned) v->namelen + 10))
    {
      if (! in_addr_add((zpl_uchar *) &nexthop_addr, 1)) 
        return;
    }

  /* For exact: search matching entry in rib table. */

  if (exact)
    {
      if (policy) /* Not supported (yet?) */
        return;
      for (*np = route_top (table); *np; *np = route_next (*np))
	{
	  if (!in_addr_cmp(&(*np)->p.u.prefix, (zpl_uchar *)&dest))
	    {
	      RNODE_FOREACH_RIB (*np, *rib)
	        {
		  if (!in_addr_cmp((zpl_uchar *)&(*rib)->nexthop->gate.ipv4,
				   (zpl_uchar *)&nexthop_addr))
		    if (proto == proto_trans((*rib)->type))
		      return;
		}
	    }
	}
      return;
    }

  /* Search next best entry */

  for (np2 = route_top (table); np2; np2 = route_next (np2))
    {

      /* Check destination first */
      if (in_addr_cmp(&np2->p.u.prefix, (zpl_uchar *)&dest) > 0)
	RNODE_FOREACH_RIB (np2, rib2)
	  check_replace(np2, rib2, np, rib);

      if (in_addr_cmp(&np2->p.u.prefix, (zpl_uchar *)&dest) == 0)
        { /* have to look at each rib individually */
	  RNODE_FOREACH_RIB (np2, rib2)
	    {
	      int proto2, policy2;

	      proto2 = proto_trans(rib2->type);
	      policy2 = 0;

	      if ((policy < policy2)
		  || ((policy == policy2) && (proto < proto2))
		  || ((policy == policy2) && (proto == proto2)
		      && (in_addr_cmp((zpl_uchar *)&rib2->nexthop->gate.ipv4,
				      (zpl_uchar *) &nexthop_addr) >= 0)
		      ))
		check_replace(np2, rib2, np, rib);
	    }
	}
    }

  if (!*rib)
    return;

  policy = 0;
  proto = proto_trans((*rib)->type);

  *objid_len = v->namelen + 10;
  pnt = (zpl_uchar *) &(*np)->p.u.prefix;
  for (i = 0; i < 4; i++)
    objid[v->namelen + i] = *pnt++;

  objid[v->namelen + 4] = proto;
  objid[v->namelen + 5] = policy;

  {
    struct nexthop *nexthop;

    nexthop = (*rib)->nexthop;
    if (nexthop)
      {
	pnt = (zpl_uchar *) &nexthop->gate.ipv4;
	for (i = 0; i < 4; i++)
	  objid[i + v->namelen + 6] = *pnt++;
      }
  }

  return;
}

static zpl_uchar *
ipFwTable (struct variable *v, oid objid[], zpl_size_t *objid_len,
	   int exact, zpl_size_t *val_len, WriteMethod **write_method)
{
  struct route_node *np;
  struct rib *rib;
  static int result;
  static int resarr[2];
  static struct ipstack_in_addr netmask;
  struct nexthop *nexthop;

  if (smux_header_table(v, objid, objid_len, exact, val_len, write_method)
      == MATCH_FAILED)
    return NULL;

  get_fwtable_route_node(v, objid, objid_len, exact, &np, &rib);
  if (!np)
    return NULL;

  nexthop = rib->nexthop;
  if (! nexthop)
    return NULL;

  switch (v->magic)
    {
    case IPFORWARDDEST:
      *val_len = 4;
      return &np->p.u.prefix;
      break;
    case IPFORWARDMASK:
      masklen2ip(np->p.prefixlen, &netmask);
      *val_len = 4;
      return (zpl_uchar *)&netmask;
      break;
    case IPFORWARDPOLICY:
      result = 0;
      *val_len  = sizeof(int);
      return (zpl_uchar *)&result;
      break;
    case IPFORWARDNEXTHOP:
      *val_len = 4;
      return (zpl_uchar *)&nexthop->gate.ipv4;
      break;
    case IPFORWARDIFINDEX:
      *val_len = sizeof(int);
      return (zpl_uchar *)&nexthop->ifindex;
      break;
    case IPFORWARDTYPE:
      if (nexthop->type == NEXTHOP_TYPE_IFINDEX
	  || nexthop->type == NEXTHOP_TYPE_IFNAME)
        result = 3;
      else
        result = 4;
      *val_len  = sizeof(int);
      return (zpl_uchar *)&result;
      break;
    case IPFORWARDPROTO:
      result = proto_trans(rib->type);
      *val_len  = sizeof(int);
      return (zpl_uchar *)&result;
      break;
    case IPFORWARDAGE:
      result = 0;
      *val_len  = sizeof(int);
      return (zpl_uchar *)&result;
      break;
    case IPFORWARDINFO:
      resarr[0] = 0;
      resarr[1] = 0;
      *val_len  = 2 * sizeof(int);
      return (zpl_uchar *)resarr;
      break;
    case IPFORWARDNEXTHOPAS:
      result = -1;
      *val_len  = sizeof(int);
      return (zpl_uchar *)&result;
      break;
    case IPFORWARDMETRIC1:
      result = 0;
      *val_len  = sizeof(int);
      return (zpl_uchar *)&result;
      break;
    case IPFORWARDMETRIC2:
      result = 0;
      *val_len  = sizeof(int);
      return (zpl_uchar *)&result;
      break;
    case IPFORWARDMETRIC3:
      result = 0;
      *val_len  = sizeof(int);
      return (zpl_uchar *)&result;
      break;
    case IPFORWARDMETRIC4:
      result = 0;
      *val_len  = sizeof(int);
      return (zpl_uchar *)&result;
      break;
    case IPFORWARDMETRIC5:
      result = 0;
      *val_len  = sizeof(int);
      return (zpl_uchar *)&result;
      break;
    default:
      return NULL;
      break;
    }  
  return NULL;
}

static zpl_uchar *
ipCidrTable (struct variable *v, oid objid[], zpl_size_t *objid_len,
	     int exact, zpl_size_t *val_len, WriteMethod **write_method)
{
  if (smux_header_table(v, objid, objid_len, exact, val_len, write_method)
      == MATCH_FAILED)
    return NULL;

  switch (v->magic)
    {
    case IPCIDRROUTEDEST:
      break;
    default:
      return NULL;
      break;
    }  
  return NULL;
}

void nsm_snmp_init (void)
{
  smux_init (nsm_srv->master);
  REGISTER_MIB("mibII/ipforward", nsm_variables, variable, ipfw_oid);
}
#endif /* ZPL_NSM_SNMP */

/*
生成标量对象框架代码：mib2c.scalar.conf
生成表对象框架代码：mib2c.iterate.conf 或 mib2c.mfd.conf
mib2c -c mib2c.old-api.conf ipForward
*/