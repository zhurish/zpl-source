/* Redistribution Handler
 * Copyright (C) 1998 Kunihiro Ishiguro
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

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "vector.h"
#include "vty.h"
#include "command.h"
#include "prefix.h"
#include "table.h"
#include "stream.h"
#include "zclient.h"
#include "linklist.h"
#include "log.h"

#include "if.h"
#include "connected.h"
#include "nsm_rib.h"
#include "nsm_zserv.h"
#include "nsm_redistribute.h"
#include "nsm_debug.h"
#include "router-id.h"

/* master zebra server structure */

int nsm_check_addr(struct prefix *p)
{
  if (p->family == IPSTACK_AF_INET)
  {
    zpl_uint32 addr;

    addr = p->u.prefix4.s_addr;
    addr = ntohl(addr);

    if (IPV4_NET127(addr) || IPSTACK_IN_CLASSD(addr) || IPV4_LINKLOCAL(addr))
      return 0;
  }
#ifdef ZPL_BUILD_IPV6
  if (p->family == IPSTACK_AF_INET6)
  {
    if (IPSTACK_IN6_IS_ADDR_LOOPBACK(&p->u.prefix6))
      return 0;
    if (IPSTACK_IN6_IS_ADDR_LINK_LOCAL(&p->u.prefix6))
      return 0;
  }
#endif /* ZPL_BUILD_IPV6 */
  return 1;
}

int is_default(struct prefix *p)
{
  if (p->family == IPSTACK_AF_INET)
    if (p->u.prefix4.s_addr == 0 && p->prefixlen == 0)
      return 1;
#ifdef ZPL_BUILD_IPV6
#if 0  /* IPv6 default separation is now pending until protocol daemon \
          can handle that. */
  if (p->family == IPSTACK_AF_INET6)
    if (IPSTACK_IN6_IS_ADDR_UNSPECIFIED (&p->u.prefix6) && p->prefixlen == 0)
      return 1;
#endif /* 0 */
#endif /* ZPL_BUILD_IPV6 */
  return 0;
}

static void
nsm_redistribute_default(struct zserv *client, vrf_id_t vrf_id)
{
  struct prefix_ipv4 p;
  struct route_table *table;
  struct route_node *rn;
  struct rib *newrib;
#ifdef ZPL_BUILD_IPV6
  struct prefix_ipv6 p6;
#endif /* ZPL_BUILD_IPV6 */

  /* Lookup default route. */
  memset(&p, 0, sizeof(struct prefix_ipv4));
  p.family = IPSTACK_AF_INET;

  /* Lookup table.  */
  table = nsm_vrf_table(AFI_IP, SAFI_UNICAST, vrf_id);
  if (table)
  {
    rn = route_node_lookup(table, (struct prefix *)&p);
    if (rn)
    {
      RNODE_FOREACH_RIB(rn, newrib)
      if (CHECK_FLAG(newrib->flags, NSM_RIB_FLAG_SELECTED) && newrib->distance != DISTANCE_INFINITY)
        nsm_zserv_send_route_multipath(NSM_EVENT_IPV4_ROUTE_ADD, client, &rn->p, newrib);
      route_unlock_node(rn);
    }
  }

#ifdef ZPL_BUILD_IPV6
  /* Lookup default route. */
  memset(&p6, 0, sizeof(struct prefix_ipv6));
  p6.family = IPSTACK_AF_INET6;

  /* Lookup table.  */
  table = nsm_vrf_table(AFI_IP6, SAFI_UNICAST, vrf_id);
  if (table)
  {
    rn = route_node_lookup(table, (struct prefix *)&p6);
    if (rn)
    {
      RNODE_FOREACH_RIB(rn, newrib)
      if (CHECK_FLAG(newrib->flags, NSM_RIB_FLAG_SELECTED) && newrib->distance != DISTANCE_INFINITY)
        nsm_zserv_send_route_multipath(NSM_EVENT_IPV6_ROUTE_ADD, client, &rn->p, newrib);
      route_unlock_node(rn);
    }
  }
#endif /* ZPL_BUILD_IPV6 */
}

/* Redistribute routes. */
static void
nsm_redistribute(struct zserv *client, zpl_uint32 type, vrf_id_t vrf_id)
{
  struct rib *newrib;
  struct route_table *table;
  struct route_node *rn;

  table = nsm_vrf_table(AFI_IP, SAFI_UNICAST, vrf_id);
  if (table)
    for (rn = route_top(table); rn; rn = route_next(rn))
      RNODE_FOREACH_RIB(rn, newrib)
      {
        if (IS_NSM_DEBUG_EVENT)
          zlog_debug(MODULE_NSM, "%s: checking: selected=%d, type=%d, distance=%d, nsm_check_addr=%d",
                     __func__, CHECK_FLAG(newrib->flags, NSM_RIB_FLAG_SELECTED),
                     newrib->type, newrib->distance, nsm_check_addr(&rn->p));
        if (CHECK_FLAG(newrib->flags, NSM_RIB_FLAG_SELECTED) && newrib->type == type && newrib->distance != DISTANCE_INFINITY && nsm_check_addr(&rn->p))
        {
          client->redist_v4_add_cnt++;
          nsm_zserv_send_route_multipath(NSM_EVENT_IPV4_ROUTE_ADD, client, &rn->p, newrib);
        }
      }

#ifdef ZPL_BUILD_IPV6
  table = nsm_vrf_table(AFI_IP6, SAFI_UNICAST, vrf_id);
  if (table)
    for (rn = route_top(table); rn; rn = route_next(rn))
      RNODE_FOREACH_RIB(rn, newrib)
  if (CHECK_FLAG(newrib->flags, NSM_RIB_FLAG_SELECTED) && newrib->type == type && newrib->distance != DISTANCE_INFINITY && nsm_check_addr(&rn->p))
  {
    client->redist_v6_add_cnt++;
    nsm_zserv_send_route_multipath(NSM_EVENT_IPV6_ROUTE_ADD, client, &rn->p, newrib);
  }
#endif /* ZPL_BUILD_IPV6 */
}

void nsm_redistribute_route_add(struct prefix *p, struct rib *rib, struct rib *rib_old)
{
  struct listnode *node, *nnode;
  struct zserv *client;

  for (ALL_LIST_ELEMENTS(nsm_srv->client_list, node, nnode, client))
  {
    if ((is_default(p) &&
         ip_vrf_bitmap_check(client->redist_default, rib->vrf_id)) ||
        ip_vrf_bitmap_check(client->redist[rib->type], rib->vrf_id))
    {
      if (p->family == IPSTACK_AF_INET)
      {
        client->redist_v4_add_cnt++;
        nsm_zserv_send_route_multipath(NSM_EVENT_IPV4_ROUTE_ADD, client, p, rib);
      }
      if (p->family == IPSTACK_AF_INET6)
      {
        client->redist_v6_add_cnt++;
        nsm_zserv_send_route_multipath(NSM_EVENT_IPV6_ROUTE_ADD, client, p, rib);
      }
    }
    else if (rib_old && ip_vrf_bitmap_check(client->redist[rib_old->type],
                                            rib_old->vrf_id))
    {
      /* nsm_redistribute_route_add has implicit withdraw semantics, so there
       * may be an old route already redistributed that is being updated.
       *
       * However, if the new route is of a type that is /not/ redistributed
       * to the client, then we must ensure the old route is explicitly
       * withdrawn.
       */
      if (p->family == IPSTACK_AF_INET)
        nsm_zserv_send_route_multipath(NSM_EVENT_IPV4_ROUTE_DELETE, client, p, rib_old);
      if (p->family == IPSTACK_AF_INET6)
        nsm_zserv_send_route_multipath(NSM_EVENT_IPV6_ROUTE_DELETE, client, p, rib_old);
    }
  }
}

void nsm_redistribute_route_delete(struct prefix *p, struct rib *rib)
{
  struct listnode *node, *nnode;
  struct zserv *client;

  /* Add DISTANCE_INFINITY check. */
  if (rib->distance == DISTANCE_INFINITY)
    return;

  for (ALL_LIST_ELEMENTS(nsm_srv->client_list, node, nnode, client))
  {
    if ((is_default(p) &&
         ip_vrf_bitmap_check(client->redist_default, rib->vrf_id)) ||
        ip_vrf_bitmap_check(client->redist[rib->type], rib->vrf_id))
    {
      if (p->family == IPSTACK_AF_INET)
        nsm_zserv_send_route_multipath(NSM_EVENT_IPV4_ROUTE_DELETE, client, p, rib);
#ifdef ZPL_BUILD_IPV6
      if (p->family == IPSTACK_AF_INET6)
        nsm_zserv_send_route_multipath(NSM_EVENT_IPV6_ROUTE_DELETE, client, p, rib);
#endif /* ZPL_BUILD_IPV6 */
    }
  }
}

void nsm_redistribute_add(zpl_uint16 command, struct zserv *client, zpl_size_t length,
                          vrf_id_t vrf_id)
{
  zpl_uint32 type;

  type = stream_getc(client->ibuf);

  if (type == 0 || type >= ZPL_ROUTE_PROTO_MAX)
    return;

  if (!ip_vrf_bitmap_check(client->redist[type], vrf_id))
  {
    ip_vrf_bitmap_set(client->redist[type], vrf_id);
    nsm_redistribute(client, type, vrf_id);
  }
}

void nsm_redistribute_delete(zpl_uint16 command, struct zserv *client, zpl_size_t length,
                             vrf_id_t vrf_id)
{
  zpl_uint32 type;

  type = stream_getc(client->ibuf);

  if (type == 0 || type >= ZPL_ROUTE_PROTO_MAX)
    return;

  ip_vrf_bitmap_unset(client->redist[type], vrf_id);
}

void nsm_redistribute_default_add(zpl_uint16 command, struct zserv *client, zpl_size_t length,
                                  vrf_id_t vrf_id)
{
  ip_vrf_bitmap_set(client->redist_default, vrf_id);
  nsm_redistribute_default(client, vrf_id);
}

void nsm_redistribute_default_delete(zpl_uint16 command, struct zserv *client,
                                     zpl_size_t length, vrf_id_t vrf_id)
{
  ip_vrf_bitmap_unset(client->redist_default, vrf_id);
}

/* Interface up information. */
void nsm_redistribute_interface_updown(struct interface *ifp, zpl_bool updown)
{
  struct listnode *node, *nnode;
  struct zserv *client;

  if (IS_NSM_DEBUG_EVENT)
    zlog_debug(MODULE_NSM, "MESSAGE: NSM_EVENT_INTERFACE_%s %s", updown ? "UP" : "DOWN", ifp->name);

  for (ALL_LIST_ELEMENTS(nsm_srv->client_list, node, nnode, client))
    if (client->ifinfo)
    {
      nsm_zserv_send_interface_state(updown ? NSM_EVENT_INTERFACE_UP : NSM_EVENT_INTERFACE_DOWN, client, ifp);
      // nsm_zserv_send_interface_link_params (client, ifp);
    }
}

void nsm_redistribute_interface_vrfbind (struct interface *ifp, vrf_id_t vrfid, zpl_bool bind)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  struct stream *s;

  if (IS_NSM_DEBUG_EVENT)
    zlog_debug(MODULE_NSM, "MESSAGE: NSM_EVENT_INTERFACE_%s %s", bind ? "BIND" : "UNBIND", ifp->name);
  s = client->obuf;
  stream_reset(s);

  nsm_zserv_create_header(s, bind ? NSM_EVENT_INTERFACE_VRF_BIND : NSM_EVENT_INTERFACE_VRF_UNBIND, ifp->vrf_id);

  nsm_zserv_encode_interface(s, ifp);
  stream_putw(s, vrfid);
  nsm_zserv_encode_interface_end(s);
  for (ALL_LIST_ELEMENTS(nsm_srv->client_list, node, nnode, client))
    {
      nsm_zserv_send_message(client);
    }
}

/* Interface information update. */
void nsm_redistribute_interface_create(struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;

  if (IS_NSM_DEBUG_EVENT)
    zlog_debug(MODULE_NSM, "MESSAGE: NSM_EVENT_INTERFACE_ADD %s", ifp->name);

  for (ALL_LIST_ELEMENTS(nsm_srv->client_list, node, nnode, client))
    if (client->ifinfo)
    {
      client->ifadd_cnt++;
      nsm_zserv_send_interface_add(client, ifp);
      // nsm_zserv_send_interface_link_params (client, ifp);
    }
}

void nsm_redistribute_interface_destroy(struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct zserv *client;

  if (IS_NSM_DEBUG_EVENT)
    zlog_debug(MODULE_NSM, "MESSAGE: NSM_EVENT_INTERFACE_DELETE %s", ifp->name);

  for (ALL_LIST_ELEMENTS(nsm_srv->client_list, node, nnode, client))
    if (client->ifinfo)
    {
      client->ifdel_cnt++;
      nsm_zserv_send_interface_delete(client, ifp);
    }
}

/* Interface address addition. */
void nsm_redistribute_interface_address_add(struct interface *ifp,
                                            struct connected *ifc)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  struct prefix *p;
  union prefix46constptr up;
  up.p = p;
  if (IS_NSM_DEBUG_EVENT)
  {
    zpl_char buf[PREFIX_STRLEN];

    p = ifc->address;
    zlog_debug(MODULE_NSM, "MESSAGE: NSM_EVENT_INTERFACE_ADDRESS_ADD %s on %s",
               prefix2str(up, buf, sizeof(buf)),
               ifc->ifp->name);
  }

  /*
    if (!CHECK_FLAG(ifc->conf, IF_IFC_REAL))
      zlog_warn("WARNING: advertising address to clients that is not yet usable.");
  */

  router_id_add_address(ifc);

  for (ALL_LIST_ELEMENTS(nsm_srv->client_list, node, nnode, client))
    if (client->ifinfo /* && CHECK_FLAG (ifc->conf, IF_IFC_REAL)*/)
    {
      client->connected_rt_add_cnt++;
      nsm_zserv_send_interface_address(NSM_EVENT_INTERFACE_ADDRESS_ADD, client, ifp, ifc);
    }
}

/* Interface address deletion. */
void nsm_redistribute_interface_address_delete(struct interface *ifp,
                                               struct connected *ifc)
{
  struct listnode *node, *nnode;
  struct zserv *client;
  struct prefix *p;
  union prefix46constptr up;
  up.p = p;
  if (IS_NSM_DEBUG_EVENT)
  {
    zpl_char buf[PREFIX_STRLEN];

    p = ifc->address;
    zlog_debug(MODULE_NSM, "MESSAGE: NSM_EVENT_INTERFACE_ADDRESS_DELETE %s on %s",
               prefix2str(up, buf, sizeof(buf)),
               ifc->ifp->name);
  }

  router_id_del_address(ifc);

  for (ALL_LIST_ELEMENTS(nsm_srv->client_list, node, nnode, client))
    if (client->ifinfo /* && CHECK_FLAG (ifc->conf, IF_IFC_REAL)*/)
    {
      client->connected_rt_del_cnt++;
      nsm_zserv_send_interface_address(NSM_EVENT_INTERFACE_ADDRESS_DELETE, client, ifp, ifc);
    }
}

/* Interface mode update */
void nsm_redistribute_interface_mode_update(struct interface *ifp, zpl_uint32 mode)
{
  struct listnode *node, *nnode;
  struct zserv *client;

  if (IS_NSM_DEBUG_EVENT)
    zlog_debug(MODULE_NSM, "MESSAGE: NSM_EVENT_INTERFACE_MODE %s", ifp->name);

  for (ALL_LIST_ELEMENTS(nsm_srv->client_list, node, nnode, client))
    if (client->ifinfo)
      nsm_zserv_send_interface_mode(client, ifp, mode);
}
