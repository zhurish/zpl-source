/*
 * Router ID for zebra daemon.
 *
 * Copyright (C) 2004 James R. Leu 
 *
 * This file is part of Quagga routing suite.
 *
 * Quagga is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * Quagga is distributed in the hope that it will be useful, but
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
#include "router-id.h"
#include "if.h"
#ifdef ZPL_NSM_MODULE
#include "nsm_zserv.h"
#endif
/* master zebra server structure */


static struct connected *
router_id_find_node (struct list *l, struct connected *ifc)
{
  struct listnode *node;
  struct connected *c;

  for (ALL_LIST_ELEMENTS_RO (l, node, c))
    if (prefix_same (ifc->address, c->address))
      return c;

  return NULL;
}

static int
router_id_bad_address (struct connected *ifc)
{
  if (ifc->address->family != IPSTACK_AF_INET)
    return 1;
  
  /* non-redistributable addresses shouldn't be used for RIDs either */
  if (!prefix_check_addr (ifc->address))
    return 1;
  
  return 0;
}

void
router_id_get (struct prefix *p, vrf_id_t vrf_id)
{
  struct listnode *node;
  struct connected *c;
  struct nsm_ip_vrf *zvrf = ip_vrf_info_lookup (vrf_id);
  if(!zvrf)
  {
	  zlog_err(MODULE_NSM,"ip_vrf_info_lookup NULL\r\n");
	  return;
  }
  p->u.prefix4.s_addr = 0;
  p->family = IPSTACK_AF_INET;
  p->prefixlen = 32;

  if (zvrf->rid_user_assigned.u.prefix4.s_addr)
    p->u.prefix4.s_addr = zvrf->rid_user_assigned.u.prefix4.s_addr;
  else if (!list_isempty (zvrf->rid_lo_sorted_list))
    {
      node = listtail (zvrf->rid_lo_sorted_list);
      c = listgetdata (node);
      p->u.prefix4.s_addr = c->address->u.prefix4.s_addr;
    }
  else if (!list_isempty (zvrf->rid_all_sorted_list))
    {
      node = listtail (zvrf->rid_all_sorted_list);
      c = listgetdata (node);
      p->u.prefix4.s_addr = c->address->u.prefix4.s_addr;
    }
}

void
router_id_set (struct prefix *p, vrf_id_t vrf_id)
{
  struct prefix p2;
  #ifdef ZPL_NSM_MODULE
  struct listnode *node;
  struct zserv *client;
  #endif
  struct nsm_ip_vrf *zvrf;

  //if (p->u.prefix4.s_addr == 0) /* unset */
    {
      zvrf = ip_vrf_info_lookup (vrf_id);
      if (! zvrf)
        return;
    }
//  else /* set */
//    zvrf = vrf_info_get (vrf_id);

  zvrf->rid_user_assigned.u.prefix4.s_addr = p->u.prefix4.s_addr;

  router_id_get (&p2, vrf_id);
#ifdef ZPL_NSM_MODULE
  for (ALL_LIST_ELEMENTS_RO (nsm_srv->client_list, node, client))
    zsend_router_id_update (client, &p2, vrf_id);
#endif
}

void
router_id_add_address (struct connected *ifc)
{
  struct list *l = NULL;
  struct prefix before;
  struct prefix after;
  #ifdef ZPL_NSM_MODULE
  struct listnode *node;
  struct zserv *client;
  #endif
  struct nsm_ip_vrf *zvrf = ip_vrf_info_lookup (ifc->ifp->vrf_id);
  if(!zvrf)
	  return;
  if (router_id_bad_address (ifc))
    return;

  router_id_get (&before, zvrf->vrf_id);

  if (!strncmp (ifc->ifp->name, "lo", 2)
      || !strncmp (ifc->ifp->name, "dummy", 5))
    l = zvrf->rid_lo_sorted_list;
  else
    l = zvrf->rid_all_sorted_list;
  
  if (!router_id_find_node (l, ifc))
    listnode_add_sort (l, ifc);

  router_id_get (&after, zvrf->vrf_id);

  if (prefix_same (&before, &after))
    return;
#ifdef ZPL_NSM_MODULE
  for (ALL_LIST_ELEMENTS_RO (nsm_srv->client_list, node, client))
    zsend_router_id_update (client, &after, zvrf->vrf_id);
#endif
}

void
router_id_del_address (struct connected *ifc)
{
  struct connected *c;
  struct list *l;
  struct prefix after;
  struct prefix before;
  #ifdef ZPL_NSM_MODULE
  struct listnode *node;
  struct zserv *client;
  #endif
  struct nsm_ip_vrf *zvrf = ip_vrf_info_lookup (ifc->ifp->vrf_id);
  if(!zvrf)
	  return;
  if (router_id_bad_address (ifc))
    return;

  router_id_get (&before, zvrf->vrf_id);

  if (!strncmp (ifc->ifp->name, "lo", 2)
      || !strncmp (ifc->ifp->name, "dummy", 5))
    l = zvrf->rid_lo_sorted_list;
  else
    l = zvrf->rid_all_sorted_list;

  if ((c = router_id_find_node (l, ifc)))
    listnode_delete (l, c);

  router_id_get (&after, zvrf->vrf_id);

  if (prefix_same (&before, &after))
    return;
#ifdef ZPL_NSM_MODULE
  for (ALL_LIST_ELEMENTS_RO (nsm_srv->client_list, node, client))
    zsend_router_id_update (client, &after, zvrf->vrf_id);
#endif
}

static int
router_id_cmp (void *a, void *b)
{
  const struct connected *ifa = (const struct connected *)a;
  const struct connected *ifb = (const struct connected *)b;

  return IPV4_ADDR_CMP(&ifa->address->u.prefix4.s_addr,&ifb->address->u.prefix4.s_addr);
}


void
router_id_init (struct nsm_ip_vrf *zvrf)
{
  zvrf->rid_all_sorted_list = &zvrf->_rid_all_sorted_list;
  zvrf->rid_lo_sorted_list = &zvrf->_rid_lo_sorted_list;
  fprintf(stdout, "=========%s\r\n", __func__);
  memset (zvrf->rid_all_sorted_list, 0, sizeof (zvrf->_rid_all_sorted_list));
  memset (zvrf->rid_lo_sorted_list, 0, sizeof (zvrf->_rid_lo_sorted_list));
  memset (&zvrf->rid_user_assigned, 0, sizeof (zvrf->rid_user_assigned));

  zvrf->rid_all_sorted_list->cmp = router_id_cmp;
  zvrf->rid_lo_sorted_list->cmp = router_id_cmp;

  zvrf->rid_user_assigned.family = IPSTACK_AF_INET;
  zvrf->rid_user_assigned.prefixlen = 32;
}
