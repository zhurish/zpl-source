/*
 * ospf_zebra.c
 *
 *  Created on: May 18, 2018
 *      Author: zhurish
 */

#include <zebra.h>

#include "thread.h"
#include "command.h"
#include "network.h"
#include "prefix.h"
//#include "routemap.h"
#include "table.h"
#include "stream.h"
#include "memory.h"
#include "zclient.h"
//#include "filter.h"
//#include "plist.h"
#include "log.h"
#include "eloop.h"
#include "thread.h"
#include "ospf.h"

/* Zebra structure to hold current status. */
struct zclient *zclient = NULL;

/* For registering threads. */
extern struct thread_master *master;
struct in_addr router_id_zebra;

static void ospf_zebra_update_status(ifindex_t uiIfindex,u_int uiLinkStatus, vrf_id_t vrf_id)
{
	struct ospf_if * p_if = NULL;
	struct ospf_process * process = ospf_process_lookup_by_ifindex(uiIfindex);
	if (process)
	{
		p_if = ospf_if_lookup_by_ifindex(process, uiIfindex);
		if (p_if)
		{
			ospf_if_state_update(p_if);
			/*for safe,update all interface state in this process*/
			//ospf_lstwalkup(&p_process->if_table, ospf_if_state_update);
			if (p_if->stat != NULL)
			{
				if (uiLinkStatus == OSPF_IFE_UP)
				{
					p_if->stat->linkup_msg_cnt++;
				}
				else
				{
					p_if->stat->linkdown_msg_cnt++;
				}
			}
			p_if->link_up = (uiLinkStatus == OSPF_IFE_UP) ? TRUE : FALSE;
			if (p_if->link_up == FALSE)
			{
				p_if->ulOspfSyncState = OSPF_LDP_INIT;
				ospf_if_hold_down_stop(p_if);
				p_if->ucHoldCostState = FALSE;
				if (ospf_timer_active(&p_if->hold_cost_timer))
				{
					ospf_timer_stop(&p_if->hold_cost_timer);
				}
			}
		}
	}
}


static void ospf_zebra_addr_add(ifindex_t uiIfindex, struct prefix prefix, vrf_id_t vrf_id)
{
	struct ospf_if * p_if = NULL;
	struct ospf_process * process = ospf_process_lookup_by_ifindex(uiIfindex);
	if (process)
	{
		p_if = ospf_if_lookup_by_ifindex(process, uiIfindex);
		if (p_if)
		{
			ospf_if_addr_change(vrf_id, prefix.u.prefix4.s_addr, ZEBRA_INTERFACE_ADDRESS_ADD, uiIfindex);
		}
	}
}

static void ospf_zebra_addr_del(ifindex_t uiIfindex, struct prefix prefix, vrf_id_t vrf_id)
{
	struct ospf_if * p_if = NULL;
	struct ospf_process * process = ospf_process_lookup_by_ifindex(uiIfindex);
	if (process)
	{
		p_if = ospf_if_lookup_by_ifindex(process, uiIfindex);
		if (p_if)
		{
			ospf_if_addr_change(vrf_id, prefix.u.prefix4.s_addr, ZEBRA_INTERFACE_ADDRESS_DELETE, uiIfindex);
		}
	}
}

static void ospf_zebra_interface_add(ifindex_t uiIfindex, vrf_id_t vrf_id)
{
/*	struct ospf_if * p_if = NULL;
	struct ospf_process * process = ospf_process_lookup_by_ifindex(uiIfindex);
	if (process)
	{
		p_if = ospf_if_lookup_by_ifindex(process, uiIfindex);
		if (p_if)
		{
			ospf_if_addr_change(vrf_id, prefix.u.prefix4.s_addr, ZEBRA_INTERFACE_ADDRESS_ADD, uiIfindex);
		}
	}*/
}

static void ospf_zebra_interface_delete(ifindex_t uiIfindex, vrf_id_t vrf_id)
{
	struct ospf_if * p_if = NULL;
	struct ospf_process * process = ospf_process_lookup_by_ifindex(uiIfindex);
	if (process)
	{
		p_if = ospf_if_lookup_by_ifindex(process, uiIfindex);
		if (p_if)
		{
			ospf_if_delete(p_if);
		}
	}
}

static void ospf_zebra_vrf_del(vrf_id_t vrf_id)
{
    struct ospf_process *p_check_process = NULL;
    struct ospf_process *p_next_process = NULL;
    for_each_ospf_process(p_check_process, p_next_process)
    {
        if (p_check_process->vrid == vrf_id)
        {
        	p_check_process->proto_shutdown = TRUE;
            ospf_timer_start(&p_check_process->delete_timer, 1);
        }
    }
    return;
}


int ospf_zebra_get_frist(vrf_id_t vrf_id, ifindex_t *ifindex)
{
	struct listnode *node;
	struct interface *ifp;
	node = listhead(if_list_get());
	if(node)
	{
		ifp = listgetdata(node);
		if (ifp && ifp->vrf_id == vrf_id)
		{
			if(ifindex)
				*ifindex = ifp->ifindex;
			return OK;
		}
	}
	return ERROR;
}

int ospf_zebra_get_next(vrf_id_t vrf_id, ifindex_t *ifindex)
{
	struct listnode *node;
	struct interface *ifp;

	for (ALL_LIST_ELEMENTS_RO(if_list_get(), node, ifp))
	{
		if ((ifp->ifindex == ifindex) && (ifp->vrf_id == vrf_id))
			break;
	}
	node = listnextnode(node);
	if(node)
	{
		ifp = listgetdata(node);
		if (ifp && ifp->vrf_id == vrf_id)
		{
			if(ifindex)
				*ifindex = ifp->ifindex;
			return OK;
		}
	}
	return ERROR;
}

int ospf_zebra_get_address(struct interface *ifp, struct prefix *address)
{
	struct connected *connected;
	struct listnode *node;
	struct prefix *p;
	for (ALL_LIST_ELEMENTS_RO(ifp->connected, node, connected))
	{
		p = connected->address;
		if(p)
		{
			memcpy(address, p, sizeof(struct prefix));
			return OK;
		}
	}
	return ERROR;
}

int ospf_zebra_get_routeid(vrf_id_t vrf_id, struct prefix *address)
{
	struct listnode *node;
	struct interface *ifp;
	//loopback
	for (ALL_LIST_ELEMENTS_RO(if_list_get(), node, ifp))
	{
		if ((ifp->vrf_id == vrf_id))
		{
			if(IF_TYPE_GET(ifp->ifindex) == IF_LOOPBACK)
			{
				if(ospf_zebra_get_address(ifp, address) == OK)
					return OK;
			}
		}
	}

	//loopback
	for (ALL_LIST_ELEMENTS_RO(if_list_get(), node, ifp))
	{
		if ((ifp->vrf_id == vrf_id))
		{
			if(IF_TYPE_GET(ifp->ifindex) == IF_ETHERNET ||
				IF_TYPE_GET(ifp->ifindex) == IF_GIGABT_ETHERNET)
			{
				if(ospf_zebra_get_address(ifp, address) == OK)
					return OK;
			}
		}
	}
	return ERROR;
}



/* Router-id update message from zebra. */
static int
ospf_router_id_update_zebra (int command, struct zclient *zclient,
			     zebra_size_t length, vrf_id_t vrf_id)
{
  struct ospf *ospf;
  struct prefix router_id;
  zebra_router_id_update_read(zclient->ibuf,&router_id);
  /*
  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    {
      char buf[128];
      prefix2str(&router_id, buf, sizeof(buf));
      zlog_debug("Zebra rcvd: router id update %s", buf);
    }

  router_id_zebra = router_id.u.prefix4;

  ospf = ospf_lookup ();

  if (ospf != NULL)
    ospf_router_id_update (ospf);
  */
  return 0;
}

/* Inteface addition message from zebra. */
static int
ospf_interface_add (int command, struct zclient *zclient, zebra_size_t length,
    vrf_id_t vrf_id)
{
  struct interface *ifp;

  ifp = zebra_interface_add_read (zclient->ibuf, vrf_id);

/*
  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    zlog_debug ("Zebra: interface add %s index %d flags %llx metric %d mtu %d",
               ifp->name, ifp->ifindex, (unsigned long long)ifp->flags,
               ifp->metric, ifp->mtu);
*/

  assert (ifp->info);
  ospf_zebra_interface_add(ifp->ifindex, vrf_id);

  return 0;
}

static int
ospf_interface_delete (int command, struct zclient *zclient,
                       zebra_size_t length, vrf_id_t vrf_id)
{
  struct interface *ifp;
  struct stream *s;
  struct route_node *rn;

  s = zclient->ibuf;
  /* zebra_interface_state_read() updates interface structure in iflist */
  ifp = zebra_interface_state_read (s, vrf_id);

  if (ifp == NULL)
    return 0;

  if (if_is_up (ifp))
    zlog_warn ("Zebra: got delete of %s, but interface is still up",
               ifp->name);

  ospf_zebra_interface_delete(ifp->ifindex, vrf_id);
/*
  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    zlog_debug
      ("Zebra: interface delete %s index %d flags %llx metric %d mtu %d",
       ifp->name, ifp->ifindex, (unsigned long long)ifp->flags, ifp->metric, ifp->mtu);
*/


  return 0;
}

static struct interface *
zebra_interface_if_lookup (struct stream *s, vrf_id_t vrf_id)
{
  char ifname_tmp[INTERFACE_NAMSIZ];

  /* Read interface name. */
  stream_get (ifname_tmp, s, INTERFACE_NAMSIZ);

  /* And look it up. */
  return if_lookup_by_name_len(ifname_tmp,
			       strnlen(ifname_tmp, INTERFACE_NAMSIZ));
}

static int
ospf_interface_state_up (int command, struct zclient *zclient,
                         zebra_size_t length, vrf_id_t vrf_id)
{
  struct interface *ifp;
  //struct ospf_interface *oi;
  //struct route_node *rn;

  ifp = zebra_interface_if_lookup (zclient->ibuf, vrf_id);

  if (ifp == NULL)
    return 0;
  ospf_zebra_update_status(ifp->ifindex, OSPF_IFE_UP, vrf_id);
#if 0
  /* Interface is already up. */
  if (if_is_operative (ifp))
    {
      /* Temporarily keep ifp values. */
      struct interface if_tmp;
      memcpy (&if_tmp, ifp, sizeof (struct interface));

      zebra_interface_if_set_value (zclient->ibuf, ifp);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
        zlog_debug ("Zebra: Interface[%s] state update.", ifp->name);

      if (if_tmp.bandwidth != ifp->bandwidth)
        {
          if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
            zlog_debug ("Zebra: Interface[%s] bandwidth change %d -> %d.",
                       ifp->name, if_tmp.bandwidth, ifp->bandwidth);

          ospf_if_recalculate_output_cost (ifp);
        }

      if (if_tmp.mtu != ifp->mtu)
        {
          if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
            zlog_debug ("Zebra: Interface[%s] MTU change %u -> %u.",
                       ifp->name, if_tmp.mtu, ifp->mtu);

	  /* Must reset the interface (simulate down/up) when MTU changes. */
          ospf_if_reset(ifp);
	}
      return 0;
    }

  zebra_interface_if_set_value (zclient->ibuf, ifp);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    zlog_debug ("Zebra: Interface[%s] state change to up.", ifp->name);

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      if ((oi = rn->info) == NULL)
        continue;

      ospf_if_up (oi);
    }
#endif
  return 0;
}

static int
ospf_interface_state_down (int command, struct zclient *zclient,
                           zebra_size_t length, vrf_id_t vrf_id)
{
  struct interface *ifp;
  //struct ospf_interface *oi;
  //struct route_node *node;

  ifp = zebra_interface_state_read (zclient->ibuf, vrf_id);

  if (ifp == NULL)
    return 0;
  ospf_zebra_update_status(ifp->ifindex, OSPF_IFE_DOWN, vrf_id);

#if 0
  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    zlog_debug ("Zebra: Interface[%s] state change to down.", ifp->name);

  for (node = route_top (IF_OIFS (ifp)); node; node = route_next (node))
    {
      if ((oi = node->info) == NULL)
        continue;
      ospf_if_down (oi);
    }
#endif
  return 0;
}

static int
ospf_interface_address_add (int command, struct zclient *zclient,
                            zebra_size_t length, vrf_id_t vrf_id)
{
  struct connected *c;

  c = zebra_interface_address_read (command, zclient->ibuf, vrf_id);

  if (c == NULL)
    return 0;

  ospf_zebra_addr_add(c->ifp->ifindex, *c->address,  vrf_id);
#if 0
  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    {
      char buf[128];
      prefix2str(c->address, buf, sizeof(buf));
      zlog_debug("Zebra: interface %s address add %s", c->ifp->name, buf);
    }

  ospf_if_update (NULL, c->ifp);

#ifdef HAVE_SNMP
  ospf_snmp_if_update (c->ifp);
#endif /* HAVE_SNMP */
#endif
  return 0;
}

static int
ospf_interface_address_delete (int command, struct zclient *zclient,
                               zebra_size_t length, vrf_id_t vrf_id)
{
  struct connected *c;
  struct interface *ifp;
  struct ospf_interface *oi;
  struct route_node *rn;
  struct prefix p;

  c = zebra_interface_address_read (command, zclient->ibuf, vrf_id);

  if (c == NULL)
    return 0;

  ospf_zebra_addr_del(c->ifp->ifindex, *c->address,  vrf_id);
#if 0
  if (IS_DEBUG_OSPF (zebra, ZEBRA_INTERFACE))
    {
      char buf[128];
      prefix2str(c->address, buf, sizeof(buf));
      zlog_debug("Zebra: interface %s address delete %s", c->ifp->name, buf);
    }

  ifp = c->ifp;
  p = *c->address;
  p.prefixlen = IPV4_MAX_PREFIXLEN;

  rn = route_node_lookup (IF_OIFS (ifp), &p);
  if (!rn)
    {
      connected_free (c);
      return 0;
    }

  assert (rn->info);
  oi = rn->info;
  route_unlock_node (rn);

  /* Call interface hook functions to clean up */
  ospf_if_free (oi);

#ifdef HAVE_SNMP
  ospf_snmp_if_update (c->ifp);
#endif /* HAVE_SNMP */

  connected_free (c);
#endif
  return 0;
}




void
ospf_zebra_add (struct ospf_iproute *pstOspf, struct ospf_process *pstProcess)
{
	//ifindex_t	ifindex = 0;
	struct zapi_ipv4 api;
	struct in_addr netmask;
	struct prefix_ipv4 p;
	{
		api.type = ZEBRA_ROUTE_OSPF;
		//api.flags = ZEBRA_FLAG_BLACKHOLE;
		api.message = 0;
		api.safi = SAFI_UNICAST;
		api.vrf_id = pstProcess->vrid;
		if(pstProcess->process_id)
		{
			SET_FLAG(api.message, ZAPI_MESSAGE_INSTANCE);
			api.instance = pstProcess->process_id;
		}
		/* OSPF pass  and metric */

		if(pstOspf->fwdaddr)
			SET_FLAG(api.message, ZAPI_MESSAGE_NEXTHOP);
		if(pstOspf->if_unit)
			SET_FLAG(api.message, ZAPI_MESSAGE_IFINDEX);

		SET_FLAG(api.message, ZAPI_MESSAGE_METRIC);
		//api.nexthop_num = 1;
		//api.ifindex_num = 1;
		api.tag = pstOspf->tag;
		api.metric = pstOspf->metric;
		api.flags = pstOspf->flags;

		netmask.s_addr = htonl(pstOspf->mask);
		p.family = AF_INET;
		p.prefixlen = ip_masklen(netmask);
		p.prefix.s_addr = htonl(pstOspf->dest);
		if(CHECK_FLAG(api.message, ZAPI_MESSAGE_IFINDEX))
		{
			//api.ifindex = XMALLOC(MTYPE_TMP, sizeof(ifindex_t));
			api.ifindex = pstOspf->if_unit;
		}
		if(CHECK_FLAG(api.message, ZAPI_MESSAGE_NEXTHOP))
		{
			//api.nexthop[0] = XMALLOC(MTYPE_TMP, sizeof(struct in_addr));
			api.nexthop.s_addr = htonl(pstOspf->fwdaddr);
		}
		zapi_ipv4_route(ZEBRA_IPV4_ROUTE_ADD, zclient, &p, &api);

		//if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
		//zlog_debug("Zebra: Route add discard %s/%d", inet_ntoa(p->prefix),
		//		p->prefixlen);
	}
}

void
ospf_zebra_delete (struct ospf_iproute *pstOspf, struct ospf_process *pstProcess)
{
	struct zapi_ipv4 api;
	struct in_addr netmask;
	struct prefix_ipv4 p;
	{
		api.type = ZEBRA_ROUTE_OSPF;
		api.flags = ZEBRA_FLAG_BLACKHOLE;
		api.message = 0;
		api.safi = SAFI_UNICAST;
		api.vrf_id = pstProcess->vrid;
		api.instance = pstProcess->process_id;
		/* OSPF pass  and metric */
		SET_FLAG(api.message, ZAPI_MESSAGE_NEXTHOP);
		SET_FLAG(api.message, ZAPI_MESSAGE_IFINDEX);
		SET_FLAG(api.message, ZAPI_MESSAGE_METRIC);
		//api.nexthop_num = 1;
		//api.ifindex_num = 1;
		api.tag = pstOspf->tag;
		api.metric = pstOspf->metric;
		api.flags = pstOspf->flags;

		netmask.s_addr = htonl(pstOspf->mask);
		p.family = AF_INET;
		p.prefixlen = ip_masklen(netmask);
		p.prefix.s_addr = htonl(pstOspf->dest);

		api.ifindex = XMALLOC(MTYPE_TMP, sizeof(ifindex_t));
		api.ifindex = pstOspf->if_unit;

		//api.nexthop = XMALLOC(MTYPE_TMP, sizeof(struct in_addr));
		api.nexthop.s_addr = htonl(pstOspf->fwdaddr);

		zapi_ipv4_route(ZEBRA_IPV4_ROUTE_DELETE, zclient, &p, &api);

		//if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
		//zlog_debug("Zebra: Route add discard %s/%d", inet_ntoa(p->prefix),
		//		p->prefixlen);
    }
}

#if 0
void
ospf_zebra_add_discard (struct prefix_ipv4 *p)
{
  struct zapi_ipv4 api;

  if (vrf_bitmap_check (zclient->redist[ZEBRA_ROUTE_OSPF], VRF_DEFAULT))
    {
      api.vrf_id = VRF_DEFAULT;
      api.type = ZEBRA_ROUTE_OSPF;
      api.flags = ZEBRA_FLAG_BLACKHOLE;
      api.message = 0;
      api.safi = SAFI_UNICAST;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      //api.nexthop_num = 0;
      //api.ifindex_num = 0;
      api.tag = 0;

      zapi_ipv4_route (ZEBRA_IPV4_ROUTE_ADD, zclient, p, &api);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
        zlog_debug ("Zebra: Route add discard %s/%d",
                   inet_ntoa (p->prefix), p->prefixlen);
    }
}

void
ospf_zebra_delete_discard (struct prefix_ipv4 *p)
{
  struct zapi_ipv4 api;

  if (vrf_bitmap_check (zclient->redist[ZEBRA_ROUTE_OSPF], VRF_DEFAULT))
    {
      api.vrf_id = VRF_DEFAULT;
      api.type = ZEBRA_ROUTE_OSPF;
      api.flags = ZEBRA_FLAG_BLACKHOLE;
      api.message = 0;
      api.safi = SAFI_UNICAST;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      //api.nexthop_num = 0;
      //api.ifindex_num = 0;
      api.tag = 0;

      zapi_ipv4_route (ZEBRA_IPV4_ROUTE_DELETE, zclient, p, &api);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
        zlog_debug ("Zebra: Route delete discard %s/%d",
                   inet_ntoa (p->prefix), p->prefixlen);

    }
}



int
ospf_redistribute_set (struct ospf *ospf, int type, int mtype, int mvalue)
{
  int force = 0;

  if (ospf_is_type_redistributed (type))
    {
      if (mtype != ospf->dmetric[type].type)
        {
          ospf->dmetric[type].type = mtype;
          force = LSA_REFRESH_FORCE;
        }
      if (mvalue != ospf->dmetric[type].value)
        {
          ospf->dmetric[type].value = mvalue;
          force = LSA_REFRESH_FORCE;
        }

      ospf_external_lsa_refresh_type (ospf, type, force);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
        zlog_debug ("Redistribute[%s]: Refresh  Type[%d], Metric[%d]",
                   ospf_redist_string(type),
                   metric_type (ospf, type), metric_value (ospf, type));

      return CMD_SUCCESS;
    }

  ospf->dmetric[type].type = mtype;
  ospf->dmetric[type].value = mvalue;

  zclient_redistribute (ZEBRA_REDISTRIBUTE_ADD, zclient, type, VRF_DEFAULT);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_debug ("Redistribute[%s]: Start  Type[%d], Metric[%d]",
               ospf_redist_string(type),
               metric_type (ospf, type), metric_value (ospf, type));

  ospf_asbr_status_update (ospf, ++ospf->redistribute);

  return CMD_SUCCESS;
}

int
ospf_redistribute_unset (struct ospf *ospf, int type)
{
  if (type == zclient->redist_default)
    return CMD_SUCCESS;

  if (!ospf_is_type_redistributed (type))
    return CMD_SUCCESS;

  zclient_redistribute (ZEBRA_REDISTRIBUTE_DELETE, zclient, type, VRF_DEFAULT);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_debug ("Redistribute[%s]: Stop",
               ospf_redist_string(type));

  ospf->dmetric[type].type = -1;
  ospf->dmetric[type].value = -1;

  /* Remove the routes from OSPF table. */
  ospf_redistribute_withdraw (ospf, type);

  ospf_asbr_status_update (ospf, --ospf->redistribute);

  return CMD_SUCCESS;
}

int
ospf_redistribute_default_set (struct ospf *ospf, int originate,
                               int mtype, int mvalue)
{
  ospf->default_originate = originate;
  ospf->dmetric[DEFAULT_ROUTE].type = mtype;
  ospf->dmetric[DEFAULT_ROUTE].value = mvalue;

  if (ospf_is_type_redistributed (DEFAULT_ROUTE))
    {
      /* if ospf->default_originate changes value, is calling
	 ospf_external_lsa_refresh_default sufficient to implement
	 the change? */
      ospf_external_lsa_refresh_default (ospf);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
        zlog_debug ("Redistribute[%s]: Refresh  Type[%d], Metric[%d]",
                   ospf_redist_string(DEFAULT_ROUTE),
                   metric_type (ospf, DEFAULT_ROUTE),
                   metric_value (ospf, DEFAULT_ROUTE));
      return CMD_SUCCESS;
    }

  zclient_redistribute_default (ZEBRA_REDISTRIBUTE_DEFAULT_ADD, zclient,
                                VRF_DEFAULT);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_debug ("Redistribute[DEFAULT]: Start  Type[%d], Metric[%d]",
               metric_type (ospf, DEFAULT_ROUTE),
               metric_value (ospf, DEFAULT_ROUTE));

  if (ospf->router_id.s_addr == 0)
    ospf->external_origin |= (1 << DEFAULT_ROUTE);
  else
    thread_add_timer (master, ospf_default_originate_timer, ospf, 1);

  ospf_asbr_status_update (ospf, ++ospf->redistribute);

  return CMD_SUCCESS;
}

int
ospf_redistribute_default_unset (struct ospf *ospf)
{
  if (!ospf_is_type_redistributed (DEFAULT_ROUTE))
    return CMD_SUCCESS;

  ospf->default_originate = DEFAULT_ORIGINATE_NONE;
  ospf->dmetric[DEFAULT_ROUTE].type = -1;
  ospf->dmetric[DEFAULT_ROUTE].value = -1;

  zclient_redistribute_default (ZEBRA_REDISTRIBUTE_DEFAULT_DELETE, zclient,
                                VRF_DEFAULT);

  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_debug ("Redistribute[DEFAULT]: Stop");

  ospf_asbr_status_update (ospf, --ospf->redistribute);

  return CMD_SUCCESS;
}

static int
ospf_external_lsa_originate_check (struct ospf *ospf,
                                   struct external_info *ei)
{
  /* If prefix is multicast, then do not originate LSA. */
  if (IN_MULTICAST (htonl (ei->p.prefix.s_addr)))
    {
      zlog_info ("LSA[Type5:%s]: Not originate AS-external-LSA, "
                 "Prefix belongs multicast", inet_ntoa (ei->p.prefix));
      return 0;
    }

  /* Take care of default-originate. */
  if (is_prefix_default (&ei->p))
    if (ospf->default_originate == DEFAULT_ORIGINATE_NONE)
      {
        zlog_info ("LSA[Type5:0.0.0.0]: Not originate AS-external-LSA "
                   "for default");
        return 0;
      }

  return 1;
}

/* If connected prefix is OSPF enable interface, then do not announce. */
int
ospf_distribute_check_connected (struct ospf *ospf, struct external_info *ei)
{
  struct listnode *node;
  struct ospf_interface *oi;


  for (ALL_LIST_ELEMENTS_RO (ospf->oiflist, node, oi))
      if (prefix_match (oi->address, (struct prefix *) &ei->p))
          return 0;
  return 1;
}

/* return 1 if external LSA must be originated, 0 otherwise */
int
ospf_redistribute_check (struct ospf *ospf,
                         struct external_info *ei, int *changed)
{
  struct route_map_set_values save_values;
  struct prefix_ipv4 *p = &ei->p;
  u_char type = is_prefix_default (&ei->p) ? DEFAULT_ROUTE : ei->type;

  if (changed)
    *changed = 0;

  if (!ospf_external_lsa_originate_check (ospf, ei))
    return 0;

  /* Take care connected route. */
  if (type == ZEBRA_ROUTE_CONNECT &&
      !ospf_distribute_check_connected (ospf, ei))
    return 0;

  if (!DEFAULT_ROUTE_TYPE (type) && DISTRIBUTE_NAME (ospf, type))
    /* distirbute-list exists, but access-list may not? */
    if (DISTRIBUTE_LIST (ospf, type))
      if (access_list_apply (DISTRIBUTE_LIST (ospf, type), p) == FILTER_DENY)
        {
          if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
            zlog_debug ("Redistribute[%s]: %s/%d filtered by ditribute-list.",
                       ospf_redist_string(type),
                       inet_ntoa (p->prefix), p->prefixlen);
          return 0;
        }

  save_values = ei->route_map_set;
  ospf_reset_route_map_set_values (&ei->route_map_set);

  /* apply route-map if needed */
  if (ROUTEMAP_NAME (ospf, type))
    {
      int ret;

      ret = route_map_apply (ROUTEMAP (ospf, type), (struct prefix *) p,
                             RMAP_OSPF, ei);

      if (ret == RMAP_DENYMATCH)
        {
          ei->route_map_set = save_values;
          if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
            zlog_debug ("Redistribute[%s]: %s/%d filtered by route-map.",
                       ospf_redist_string(type),
                       inet_ntoa (p->prefix), p->prefixlen);
          return 0;
        }

      /* check if 'route-map set' changed something */
      if (changed)
        *changed = !ospf_route_map_set_compare (&ei->route_map_set,
                                                &save_values);
    }

  return 1;
}

/* OSPF route-map set for redistribution */
void
ospf_routemap_set (struct ospf *ospf, int type, const char *name)
{
  if (ROUTEMAP_NAME (ospf, type))
    free (ROUTEMAP_NAME (ospf, type));

  ROUTEMAP_NAME (ospf, type) = strdup (name);
  ROUTEMAP (ospf, type) = route_map_lookup_by_name (name);
}

void
ospf_routemap_unset (struct ospf *ospf, int type)
{
  if (ROUTEMAP_NAME (ospf, type))
    free (ROUTEMAP_NAME (ospf, type));

  ROUTEMAP_NAME (ospf, type) = NULL;
  ROUTEMAP (ospf, type) = NULL;
}

/* Zebra route add and delete treatment. */
static int
ospf_zebra_read_ipv4 (int command, struct zclient *zclient,
                      zebra_size_t length, vrf_id_t vrf_id)
{
  struct stream *s;
  struct zapi_ipv4 api;
  unsigned long ifindex;
  struct in_addr nexthop;
  struct prefix_ipv4 p;
  struct external_info *ei;
  struct ospf *ospf;
  unsigned char plength = 0;

  s = zclient->ibuf;
  ifindex = 0;
  nexthop.s_addr = 0;

  /* Type, flags, message. */
  api.type = stream_getc (s);
  api.flags = stream_getc (s);
  api.message = stream_getc (s);

  /* IPv4 prefix. */
  memset (&p, 0, sizeof (struct prefix_ipv4));
  p.family = AF_INET;
  plength = stream_getc (s);
  p.prefixlen = MIN(IPV4_MAX_PREFIXLEN, plength);
  stream_get (&p.prefix, s, PSIZE (p.prefixlen));
  //NEXTHOP TYPE
  stream_getc (s);

  if (IPV4_NET127(ntohl(p.prefix.s_addr)))
    return 0;

  /* Nexthop, ifindex, distance, metric. */
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP))
    {
      nexthop.s_addr = stream_get_ipv4 (s);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_IFINDEX))
    {
      ifindex = stream_getl (s);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_DISTANCE))
    api.distance = stream_getc (s);
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_METRIC))
    api.metric = stream_getl (s);
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_TAG))
    api.tag = stream_getl (s);
  else
    api.tag = 0;
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_INSTANCE))
    api.instance = stream_getl (s);

  ospf = ospf_lookup ();
  if (ospf == NULL)
    return 0;

  if (command == ZEBRA_IPV4_ROUTE_ADD)
    {
      /* XXX|HACK|TODO|FIXME:
       * Maybe we should ignore reject/blackhole routes? Testing shows that
       * there is no problems though and this is only way to "summarize"
       * routes in ASBR at the moment. Maybe we need just a better generalised
       * solution for these types?
       *
       * if ( CHECK_FLAG (api.flags, ZEBRA_FLAG_BLACKHOLE)
       *     || CHECK_FLAG (api.flags, ZEBRA_FLAG_REJECT))
       * return 0;
       */

      /* Protocol tag overwrites all other tag value send by zebra */
      if (ospf->dtag[api.type] > 0)
       api.tag = ospf->dtag[api.type];

      ei = ospf_external_info_add (api.type, p, ifindex, nexthop, api.tag);

      if (ospf->router_id.s_addr == 0)
        /* Set flags to generate AS-external-LSA originate event
           for each redistributed protocols later. */
        ospf->external_origin |= (1 << api.type);
      else
        {
          if (ei)
            {
              if (is_prefix_default (&p))
                ospf_external_lsa_refresh_default (ospf);
              else
                {
                  struct ospf_lsa *current;

                  current = ospf_external_info_find_lsa (ospf, &ei->p);
                  if (!current)
                    ospf_external_lsa_originate (ospf, ei);
                  else if (IS_LSA_MAXAGE (current))
                    ospf_external_lsa_refresh (ospf, current,
                                               ei, LSA_REFRESH_FORCE);
                  else
                    zlog_warn ("ospf_zebra_read_ipv4() : %s already exists",
                               inet_ntoa (p.prefix));
                }
            }
        }
    }
  else                          /* if (command == ZEBRA_IPV4_ROUTE_DELETE) */
    {
      ospf_external_info_delete (api.type, p);
      if (is_prefix_default (&p))
        ospf_external_lsa_refresh_default (ospf);
      else
        ospf_external_lsa_flush (ospf, api.type, &p, ifindex /*, nexthop */);
    }

  return 0;
}


int
ospf_distribute_list_out_set (struct ospf *ospf, int type, const char *name)
{
  /* Lookup access-list for distribute-list. */
  DISTRIBUTE_LIST (ospf, type) = access_list_lookup (AFI_IP, name);

  /* Clear previous distribute-name. */
  if (DISTRIBUTE_NAME (ospf, type))
    free (DISTRIBUTE_NAME (ospf, type));

  /* Set distribute-name. */
  DISTRIBUTE_NAME (ospf, type) = strdup (name);

  /* If access-list have been set, schedule update timer. */
  if (DISTRIBUTE_LIST (ospf, type))
    ospf_distribute_list_update (ospf, type);

  return CMD_SUCCESS;
}

int
ospf_distribute_list_out_unset (struct ospf *ospf, int type, const char *name)
{
  /* Schedule update timer. */
  if (DISTRIBUTE_LIST (ospf, type))
    ospf_distribute_list_update (ospf, type);

  /* Unset distribute-list. */
  DISTRIBUTE_LIST (ospf, type) = NULL;

  /* Clear distribute-name. */
  if (DISTRIBUTE_NAME (ospf, type))
    free (DISTRIBUTE_NAME (ospf, type));

  DISTRIBUTE_NAME (ospf, type) = NULL;

  return CMD_SUCCESS;
}

/* distribute-list update timer. */
static int
ospf_distribute_list_update_timer (struct thread *thread)
{
  struct route_node *rn;
  struct external_info *ei;
  struct route_table *rt;
  struct ospf_lsa *lsa;
  int type, default_refresh = 0;
  struct ospf *ospf;

  ospf = ospf_lookup ();
  if (ospf == NULL)
    return 0;

  ospf->t_distribute_update = NULL;

  zlog_info ("Zebra[Redistribute]: distribute-list update timer fired!");

  /* foreach all external info. */
  for (type = 0; type <= ZEBRA_ROUTE_MAX; type++)
    {
      rt = EXTERNAL_INFO (type);
      if (!rt)
	continue;
      for (rn = route_top (rt); rn; rn = route_next (rn))
	if ((ei = rn->info) != NULL)
	  {
	    if (is_prefix_default (&ei->p))
	      default_refresh = 1;
	    else if ((lsa = ospf_external_info_find_lsa (ospf, &ei->p)))
	      ospf_external_lsa_refresh (ospf, lsa, ei, LSA_REFRESH_IF_CHANGED);
	    else
	      ospf_external_lsa_originate (ospf, ei);
	  }
    }
  if (default_refresh)
    ospf_external_lsa_refresh_default (ospf);
  return 0;
}

/* Update distribute-list and set timer to apply access-list. */
void
ospf_distribute_list_update (struct ospf *ospf, uintptr_t type)
{
  struct route_table *rt;

  /* External info does not exist. */
  if (!(rt = EXTERNAL_INFO (type)))
    return;

  /* If exists previously invoked thread, then let it continue. */
  if (ospf->t_distribute_update)
    return;

  /* Set timer. */
  ospf->t_distribute_update =
    thread_add_timer_msec (master, ospf_distribute_list_update_timer,
                      (void *) type, ospf->min_ls_interval);
}

/* If access-list is updated, apply some check. */
static void
ospf_filter_update (const char *name)
{
  struct ospf *ospf;
  int type;
  int abr_inv = 0;
  struct ospf_area *area;
  struct listnode *node;

  /* If OSPF instatnce does not exist, return right now. */
  ospf = ospf_lookup ();
  if (ospf == NULL)
    return;

  /* Update distribute-list, and apply filter. */
  for (type = 0; type <= ZEBRA_ROUTE_MAX; type++)
    {
      if (ROUTEMAP (ospf, type) != NULL)
        {
          /* if route-map is not NULL it may be using this access list */
          ospf_distribute_list_update (ospf, type);
          continue;
        }

      /* There is place for route-map for default-information (ZEBRA_ROUTE_MAX),
       * but no distribute list. */
      if (type == ZEBRA_ROUTE_MAX)
	break;

      if (DISTRIBUTE_NAME (ospf, type))
        {
          /* Keep old access-list for distribute-list. */
          struct access_list *old = DISTRIBUTE_LIST (ospf, type);

          /* Update access-list for distribute-list. */
          DISTRIBUTE_LIST (ospf, type) =
            access_list_lookup (AFI_IP, DISTRIBUTE_NAME (ospf, type));

          /* No update for this distribute type. */
          if (old == NULL && DISTRIBUTE_LIST (ospf, type) == NULL)
            continue;

          /* Schedule distribute-list update timer. */
          if (DISTRIBUTE_LIST (ospf, type) == NULL ||
              strcmp (DISTRIBUTE_NAME (ospf, type), name) == 0)
            ospf_distribute_list_update (ospf, type);
        }
    }

  /* Update Area access-list. */
  for (ALL_LIST_ELEMENTS_RO (ospf->areas, node, area))
    {
      if (EXPORT_NAME (area))
        {
          EXPORT_LIST (area) = NULL;
          abr_inv++;
        }

      if (IMPORT_NAME (area))
        {
          IMPORT_LIST (area) = NULL;
          abr_inv++;
        }
    }

  /* Schedule ABR tasks -- this will be changed -- takada. */
  if (IS_OSPF_ABR (ospf) && abr_inv)
    ospf_schedule_abr_task (ospf);
}

/* If prefix-list is updated, do some updates. */
void
ospf_prefix_list_update (struct prefix_list *plist)
{
  struct ospf *ospf;
  int type;
  int abr_inv = 0;
  struct ospf_area *area;
  struct listnode *node;

  /* If OSPF instatnce does not exist, return right now. */
  ospf = ospf_lookup ();
  if (ospf == NULL)
    return;

  /* Update all route-maps which are used as redistribution filters.
   * They might use prefix-list.
   */
  for (type = 0; type <= ZEBRA_ROUTE_MAX; type++)
    {
      if (ROUTEMAP (ospf, type) != NULL)
        {
          /* If route-map is not NULL it may be using this prefix list */
          ospf_distribute_list_update (ospf, type);
          continue;
        }
    }

  /* Update area filter-lists. */
  for (ALL_LIST_ELEMENTS_RO (ospf->areas, node, area))
    {
      /* Update filter-list in. */
      if (PREFIX_NAME_IN (area))
        if (strcmp (PREFIX_NAME_IN (area), prefix_list_name (plist)) == 0)
          {
            PREFIX_LIST_IN (area) =
              prefix_list_lookup (AFI_IP, PREFIX_NAME_IN (area));
            abr_inv++;
          }

      /* Update filter-list out. */
      if (PREFIX_NAME_OUT (area))
        if (strcmp (PREFIX_NAME_OUT (area), prefix_list_name (plist)) == 0)
          {
            PREFIX_LIST_IN (area) =
              prefix_list_lookup (AFI_IP, PREFIX_NAME_OUT (area));
            abr_inv++;
          }
    }

  /* Schedule ABR task. */
  if (IS_OSPF_ABR (ospf) && abr_inv)
    ospf_schedule_abr_task (ospf);
}

static struct ospf_distance *
ospf_distance_new (void)
{
  return XCALLOC (MTYPE_OSPF_DISTANCE, sizeof (struct ospf_distance));
}

static void
ospf_distance_free (struct ospf_distance *odistance)
{
  XFREE (MTYPE_OSPF_DISTANCE, odistance);
}

int
ospf_distance_set (struct vty *vty, struct ospf *ospf,
                   const char *distance_str,
                   const char *ip_str,
                   const char *access_list_str)
{
  int ret;
  struct prefix_ipv4 p;
  u_char distance;
  struct route_node *rn;
  struct ospf_distance *odistance;

  ret = str2prefix_ipv4 (ip_str, &p);
  if (ret == 0)
    {
      vty_out (vty, "Malformed prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  distance = atoi (distance_str);

  /* Get OSPF distance node. */
  rn = route_node_get (ospf->distance_table, (struct prefix *) &p);
  if (rn->info)
    {
      odistance = rn->info;
      route_unlock_node (rn);
    }
  else
    {
      odistance = ospf_distance_new ();
      rn->info = odistance;
    }

  /* Set distance value. */
  odistance->distance = distance;

  /* Reset access-list configuration. */
  if (odistance->access_list)
    {
      free (odistance->access_list);
      odistance->access_list = NULL;
    }
  if (access_list_str)
    odistance->access_list = strdup (access_list_str);

  return CMD_SUCCESS;
}

int
ospf_distance_unset (struct vty *vty, struct ospf *ospf,
                     const char *distance_str,
                     const char *ip_str, char
                     const *access_list_str)
{
  int ret;
  struct prefix_ipv4 p;
  struct route_node *rn;
  struct ospf_distance *odistance;

  ret = str2prefix_ipv4 (ip_str, &p);
  if (ret == 0)
    {
      vty_out (vty, "Malformed prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  rn = route_node_lookup (ospf->distance_table, (struct prefix *) &p);
  if (!rn)
    {
      vty_out (vty, "Can't find specified prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  odistance = rn->info;

  if (odistance->access_list)
    free (odistance->access_list);
  ospf_distance_free (odistance);

  rn->info = NULL;
  route_unlock_node (rn);
  route_unlock_node (rn);

  return CMD_SUCCESS;
}

void
ospf_distance_reset (struct ospf *ospf)
{
  struct route_node *rn;
  struct ospf_distance *odistance;

  for (rn = route_top (ospf->distance_table); rn; rn = route_next (rn))
    if ((odistance = rn->info) != NULL)
      {
        if (odistance->access_list)
          free (odistance->access_list);
        ospf_distance_free (odistance);
        rn->info = NULL;
        route_unlock_node (rn);
      }
}

u_char
ospf_distance_apply (struct prefix_ipv4 *p, struct ospf_route *or)
{
  struct ospf *ospf;

  ospf = ospf_lookup ();
  if (ospf == NULL)
    return 0;

  if (ospf->distance_intra)
    if (or->path_type == OSPF_PATH_INTRA_AREA)
      return ospf->distance_intra;

  if (ospf->distance_inter)
    if (or->path_type == OSPF_PATH_INTER_AREA)
      return ospf->distance_inter;

  if (ospf->distance_external)
    if (or->path_type == OSPF_PATH_TYPE1_EXTERNAL
        || or->path_type == OSPF_PATH_TYPE2_EXTERNAL)
      return ospf->distance_external;

  if (ospf->distance_all)
    return ospf->distance_all;

  return 0;
}

static void
ospf_zebra_connected (struct zclient *zclient)
{
  zclient_send_requests (zclient, VRF_DEFAULT);
}
#endif
void
ospf_zebra_init (struct thread_master *master)
{
  /* Allocate zebra structure. */
  zclient = zclient_new (master);
  zclient_init (zclient, ZEBRA_ROUTE_OSPF);
  //zclient->zebra_connected = ospf_zebra_connected;
  zclient->router_id_update = ospf_router_id_update_zebra;
  zclient->interface_add = ospf_interface_add;
  zclient->interface_delete = ospf_interface_delete;
  zclient->interface_up = ospf_interface_state_up;
  zclient->interface_down = ospf_interface_state_down;
  zclient->interface_address_add = ospf_interface_address_add;
  zclient->interface_address_delete = ospf_interface_address_delete;
  //zclient->interface_link_params = ospf_interface_link_params;

/*  zclient->ipv4_route_add = ospf_zebra_read_ipv4;
  zclient->ipv4_route_delete = ospf_zebra_read_ipv4;

  access_list_add_hook (ospf_filter_update);
  access_list_delete_hook (ospf_filter_update);
  prefix_list_add_hook (ospf_prefix_list_update);
  prefix_list_delete_hook (ospf_prefix_list_update);*/
}








static int ospf_task_id = 0;
static struct eloop *t_time = NULL;

static int ospf_timer_thread(struct eloop *thread)
{
	ospf_thread_timer();
	ospf_update_min_timer();

	t_time = eloop_add_timer_msec(thread->master, ospf_timer_thread, NULL, ospf.waittime * OSPF_MS_PER_TICK);
	return 0;
}

static int ospf_recv_thread(struct eloop *thread)
{
	int sock = ELOOP_FD(thread);
	ospf_socket_recv(sock);
	eloop_add_read(thread->master, ospf_recv_thread, NULL, sock);
	return 0;
}

int ospf_thread_add(int vrf, int sock)
{
	ospf.t_read[vrf] = eloop_add_read(ospf.master, ospf_recv_thread, NULL, sock);
	return 0;
}

int ospf_thread_del(int vrf)
{
	if(ospf.t_read[vrf])
		eloop_cancel(ospf.t_read[vrf]);
}

static int ospf_main_thread(void *argv)
{
	struct eloop thread;
	t_time = eloop_add_timer(master_eloop[MODULE_OSPF], ospf_timer_thread, NULL, 1);
	while (eloop_fetch (master_eloop[MODULE_OSPF], &thread))
		eloop_call (&thread);
	return 0;
}



int ospf_task_init ()
{
	ospf_task_id = os_task_create("ospfTask", OS_TASK_DEFAULT_PRIORITY,
	               0, ospf_main_task, NULL, OS_TASK_DEFAULT_STACK);
	if(ospf_task_id)
		return OK;
	return ERROR;

}

void ospf_module_init()
{
	/* Make master thread emulator. */
	master_eloop[MODULE_OSPF] = eloop_master_module_create (MODULE_OSPF);
	ospf_main_init(master_eloop[MODULE_OSPF]);
}
