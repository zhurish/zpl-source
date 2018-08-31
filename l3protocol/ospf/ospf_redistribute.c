/* ospf_redistribute.c - redistribute route into ospf as ase/nssa lsa*/
#include "ospf.h"
#include "ospf_os.h"
#ifdef OSPF_POLICY
//#include "pcl_nm.h"
#endif


int g_ospf_redis_new = 1;

struct ospf_iproute * ospf_import_route_create(struct ospf_process *p_process,struct ospf_iproute *p_import_route);
void ospf_redis_range_update(struct ospf_redis_range *p_range);


/*API for upper layer calling*/
int ospf_redis_is_enable (struct ospf_process *p_process, int type, int np)
{
	u_int8 reditribute_flag = 0;
	struct ospf_redistribute *pstRedisInfo = NULL;
    if (np == 0)
    {
        reditribute_flag = BIT_LST_TST(p_process->reditribute_flag, type);
    }
    else
    {
        pstRedisInfo = ospf_redistribute_lookup(p_process, type, np);
        if(pstRedisInfo != NULL)
        {
            reditribute_flag = pstRedisInfo->proto_active;
        }
    }
    return reditribute_flag;
}

/*called from CLI,to redistribute or no-redistribute one 
   type of external route into OSPF.*/
void 
ospf_redistribute_set(
                      struct ospf_process *p_process, 
                      int proto, 
                      int enable,
                      u_int uiProcessId)
{
//#ifdef OSPF_REDISTRIBUTE
	if(enable)
	{
		if(ospf_redis_is_enable (p_process, proto, uiProcessId))
			return;

		if(uiProcessId == 0)
		{
			p_process->asbr = TRUE;
			BIT_LST_SET(p_process->reditribute_flag, proto);
		}
		else
		{
			struct ospf_redistribute *pstRedisInfo = NULL;
			pstRedisInfo = ospf_redistribute_lookup(p_process, proto, uiProcessId);
	        if(pstRedisInfo != NULL)
	        {
	            pstRedisInfo->proto_active = TRUE;
	        }
	    }

        /*����ISIS/BGP/RIP��RTM_MSG*/
        //ospf_ipRouteRedisSet(p_process->vrid, AF_INET, proto, M2_ipRouteProto_ospf);
        ospf_import_route_update_all(p_process);
        ospf_lstwalkup(&p_process->area_table, ospf_router_lsa_originate);
	}
	else if(ospf_redis_is_enable (p_process, proto, uiProcessId))
	{
	    struct ospf_iproute *p_route = NULL;
	    struct ospf_iproute *p_next = NULL;
        /*set delete flag,and start import timer*/
        for_each_node(&p_process->import_table, p_route, p_next)
        {
            if (p_route->proto == proto)
            {
                //if (p_route->process_id == uiProcessId && uiProcessId)
                {
                    p_route->update = TRUE;
                    p_route->active = FALSE;
                }
            }
        }
        ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
        //ospf_asbr_range_update_timeout(p_process);

        /*clear redistribute flag*/
        if(uiProcessId)
        {
			struct ospf_redistribute *pstRedisInfo = NULL;
			struct ospf_redistribute *pstNextRedisInfo = NULL;
			pstRedisInfo = ospf_redistribute_lookup(p_process, proto, uiProcessId);
			if(pstRedisInfo != NULL)
			{
				pstRedisInfo->proto_active = TRUE;
			}
            for_each_node(&p_process->redistribute_config_table, pstRedisInfo, pstNextRedisInfo)
            {
                if(pstRedisInfo->protocol == proto)
                {
                    BIT_LST_SET(p_process->reditribute_flag, proto);
                    break;
                }
            }
        }
        else
        	BIT_LST_CLR(p_process->reditribute_flag, proto);
        /*end*/

        /*if no more protocol is redistributed,set asbr state to false,and update router lsa*/
        if (0 == *(u_int*)p_process->reditribute_flag)
        {
            p_process->asbr = FALSE;
            ospf_lstwalkup(&p_process->area_table, ospf_router_lsa_originate);
        }
	}
    return;
}

/*This routine create one redistribute entry
* we would like to control special external route,
  so we need redistribute conttrol list*/
struct ospf_redistribute *
ospf_redistribute_create(
                   struct ospf_process *p_process,
                   u_int protocol,
                   u_int proto_process)
{
    struct ospf_redistribute *p_node = NULL; 
    
    p_node = ospf_malloc2(OSPF_MREDISTRIBUTE);    
    if (NULL != p_node)
    {
        p_node->proto_process = proto_process;
        p_node->protocol = protocol;
        p_node->action = TRUE;/*default action is permit*/
        p_node->p_process = p_process;
        p_node->proto_active = FALSE;
	    p_node->metric = 1;
	    p_node->tag = 1;
        ospf_lstadd(&p_process->redistribute_config_table, p_node);

        ospf_lstadd(&ospf.nm.redistribute_table, p_node);          
    }    
    return p_node ;
}

/*This routine delete one redistribute entry*/
void 
ospf_redistribute_delete(struct ospf_redistribute *p_redistribute)
{    
    struct ospf_process *p_process = p_redistribute->p_process;

    ospf_lstdel(&ospf.nm.redistribute_table, p_redistribute);

    ospf_lstdel_free(&p_process->redistribute_config_table, p_redistribute, OSPF_MREDISTRIBUTE);
    return;
}

/*lookup redistribute node*/
struct ospf_redistribute *
ospf_redistribute_lookup(
           struct ospf_process *p_process,
           u_int protocol,
           u_int proto_process)
{
    struct ospf_redistribute redistribute;
    redistribute.protocol = protocol;
    redistribute.proto_process = proto_process;
    return ospf_lstlookup(&p_process->redistribute_config_table, &redistribute);
}

/*compare function of redistribute*/
int 
ospf_redistribute_lookup_cmp(
          struct ospf_redistribute *p1, 
          struct ospf_redistribute *p2)
{
    //OSPF_KEY_CMP(p1, p2, dest);
    //OSPF_KEY_CMP(p1, p2, mask);
    OSPF_KEY_CMP(p1, p2, protocol);
    if((p1->protocol == M2_ipRouteProto_rip)
        || (p1->protocol == M2_ipRouteProto_is_is)
        || (p1->protocol == M2_ipRouteProto_ospf))
    {
        OSPF_KEY_CMP(p1, p2, proto_process);
    }
    return 0;
}

int
ospf_redistribute_nm_cmp(
          struct ospf_redistribute *p1, 
          struct ospf_redistribute *p2)
{
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
    return ospf_redistribute_lookup_cmp(p1, p2);
}

/* This routine init redistribtue route list,called when ospf up*/
void 
ospf_redistribute_table_init(struct ospf_process *p_process)
{   
    ospf_lstinit(&p_process->redistribute_config_table, ospf_redistribute_lookup_cmp);
    return;
}

/*check if a route can be redistributed*/
 void
ospf_import_route_verify(struct ospf_iproute *p_route)
{
    struct ospf_process *p_process = p_route->p_process;
    struct ospf_redistribute *p_redis = NULL;
    struct ospf_redistribute *p_next = NULL;
    struct ospf_redis_range *p_range = NULL;
    struct ospf_redis_range *p_next_range = NULL;
    struct ospf_asbr_range *p_asbr_range = NULL;
    struct ospf_asbr_range *p_next_asbr_range = NULL;
    u_int adv = TRUE;
    u_char ucNeedProcess = FALSE;

    if ((p_route->proto == M2_ipRouteProto_ospf)
        || (p_route->proto == M2_ipRouteProto_is_is)
        || (p_route->proto == M2_ipRouteProto_rip))
    {
        ucNeedProcess = TRUE;
    }
    /*check by redistribute command*/
    for_each_node(&p_process->redistribute_config_table, p_redis, p_next)
    {
        /*ignore different protocol*/
        if (p_redis->protocol != p_route->proto)
        {
            continue;
        }
        /*如果协议是isis、ospf、rip需要判断进程号*/
        if ((ucNeedProcess == TRUE)
            && (p_redis->proto_process != p_route->process_id))
        {
            continue;
        }     
        /*lookup entry with matched network range,we use the first matched entry
            if not matched ,accept it*/
        #if 0
        if (!ospf_netmatch(p_route->dest, p_redis->dest, p_redis->mask)) 
        {
            continue;
        }
        #endif

        /*using configured cost*/
        if (p_redis->metric || p_redis->type )
        {
            p_route->metric = ospf_extmetric(p_redis->metric, p_redis->type);
        }
        
        p_route->tag = p_redis->tag;

        p_route->no_translate = p_redis->no_translate;

        /*redistribute is forbbiden,so try to delete it from lsa table*/
        if ((TRUE == p_route->active) && (FALSE == p_redis->action))
        {
            ospf_logx(ospf_debug_rtm, "redistribute is not advtise by config");
  
            adv = FALSE;
        }
    }

    /*check using route policy*/
    if ((TRUE == p_route->active) && (TRUE == adv))
    {
        if (FALSE == ospf_redistribute_policy_verify(p_route))
        {
            ospf_logx(ospf_debug_rtm, "redistribute is not advtise by routepolicy");
            
            adv = FALSE;
        }
    }    
    /*check by range*/
    for_each_node(&p_process->redis_range_table, p_range, p_next_range)
    {
        if (ospf_netmatch(p_route->dest, p_range->dest, p_range->mask))
        {
            /*matched by range,will not adv special route*/
            adv = FALSE;
            continue;
        }
    }
    
    /*no redistribute or not pass the route policy will delete lsa*/
    if ((FALSE == adv) || (FALSE == p_route->active))
    {
        p_route->active = FALSE;
    }
    else
    {
        p_route->active = TRUE;
    }
    return;
}
u_int
ospf_import_exist_same_route(
                      struct ospf_process *p_process,
                      struct ospf_iproute *p_route)
{
    struct ospf_iproute min_route;
    struct ospf_iproute *p_same_route;

    /*if deleted this route,still exist other same dest && mask route, not delete*/
    memset(&min_route, 0, sizeof(min_route));
    min_route.dest = p_route->dest;
    min_route.mask = p_route->mask;
    p_same_route = ospf_lstgreater(&p_process->import_table, &min_route);

    if (p_same_route 
        && (p_same_route->dest == p_route->dest)
        && (p_same_route->mask == p_route->mask))
    {
        return TRUE;
    }
    return FALSE;
}
#define OSPF_REDISTRIBUTE_LIMIT 5000
/*  decide if a rotue can be redistributed,check configured
     redistribute control table,obtain operation ,and summary route,
     may also override original metric*/
/*slave need to process redistribute timer for import_table will be change*/
void 
ospf_import_route_update_all(struct ospf_process *p_process)
{
    struct ospf_iproute *p_route;
    struct ospf_iproute *p_next_route;
    struct ospf_iproute *p_same_route;
    struct ospf_iproute tmp_route;
    struct ospf_iproute min_route;
    u_int faddr = 0;  
    u_int count = 0;
    u_int8 str[64]={0};

    /*when start import_timer,set  import_update */
    if (TRUE == p_process->import_update)
    {
         for_each_node(&p_process->import_table, p_route, p_next_route)
         {
             p_route->update = TRUE;
         }
         p_process->import_update = FALSE;
    }
    /*check all imported route*/
    for_each_node(&p_process->import_table, p_route, p_next_route)
    {
        if (FALSE == p_route->update)
        {
            continue;
        }
        p_route->update = FALSE;
        memcpy(&tmp_route, p_route, sizeof(tmp_route));

        /*special checking for inactive route,there may be multiple routes
          with same network/mask and different protocol/nexthop
          in this case,we must select single route for lsa orgindate,if one of such
          routes is deleted,we must check if any other route exist,if so,do not 
          delete related lsa*/
        if (FALSE == p_route->active)
        {                          
            /*release it*/
            ospf_import_route_delete(p_route);

            /*if delete type 3 lsa,recalculate route*/
            if (tmp_route.vpn_internal)
            {
                ospf_vpn_summary_lsa_originate_all(p_process, &tmp_route, TRUE);
                ospf_spf_request(p_process);
            }
            else
            {
                /*if deleted this route,still exist other same dest && mask route, not delete*/
                memset(&min_route, 0, sizeof(min_route));
                min_route.dest = tmp_route.dest;
                min_route.mask = tmp_route.mask;
                p_same_route = ospf_lstgreater(&p_process->import_table, &min_route);
    
                if (p_same_route 
                    && (p_same_route->dest == tmp_route.dest)
                    && (p_same_route->mask == tmp_route.mask))
                {
                    continue;
                }
            }
        }
        else
        {
            if (p_route->vpn_type_change)
            {
                ospf_logx(ospf_debug_rtm, "vpn internal type changed for route %s", ospf_inet_ntoa(str, p_route->dest));

                p_route->vpn_type_change = FALSE;
                /*originate lsa from type 5 change to type 3,need delete type5 lsa*/
                if (p_route->vpn_internal)
                {
                    if (TRUE == ospf_import_exist_same_route(p_process, p_route))
                    {
                        ospf_logx(ospf_debug_rtm, "flush type5 lsa for route %s", ospf_inet_ntoa(str, p_route->dest));

                        /*forwarding address may change, so recover it*/
                        faddr = p_route->fwdaddr;
                        /*set active TRUE to de delete */
                        p_route->active = FALSE;
                        ospf_external_lsa_originate(p_route);
                        /*set active to FALSE*/
                        p_route->active = TRUE;
                        /*recover nexthop*/
                        p_route->fwdaddr = faddr;
                    }
                }
                else /*need delete type3 lsa*/
                {
                    ospf_logx(ospf_debug_rtm, "flush type3 lsa for route %s", ospf_inet_ntoa(str, p_route->dest));

                    ospf_vpn_summary_lsa_originate_all(p_process, &tmp_route, TRUE);
                    ospf_spf_request(p_process);
                }
            }
        }

        /*update importe state*/        
        ospf_import_route_verify(&tmp_route);

        /*add or delete lsa for this route*/
        ospf_import_route_lsa_originate(&tmp_route);

	    /*asbr range update*/
        //ospf_asbr_range_update_timeout(p_process);
        //ospf_stimer_start(&p_process->asbr_range_update_timer, 5);
  
        if (OSPF_REDISTRIBUTE_LIMIT < ++count)
        {
            ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);           
            return;
        }
    }
    ospf_timer_stop(&p_process->import_timer);
    /**/
    /*after redistribute,should walkup all range*/
    ospf_lstwalkup(&p_process->redis_range_table, ospf_redis_range_update);
    return;
}

/*import ip route*/
void 
ospf_import_iproute (
               u_int enable, 
               struct ospf_iproute *p_route)
{
    struct ospf_iproute *p_import_route;
    struct ospf_process *p_process = NULL;
    u_int8 dest[16];
    u_int8 mask[16];
    u_int8 nexthop[16];

    if (p_route == NULL)
    {
        return;
    }
    p_process = p_route->p_process;

    ospf_logx(ospf_debug_rtm, "RTM %s route %s/%s/%s, instance%d vrid%d, from instance %d",enable == TRUE ? "add" : "delete", ospf_inet_ntoa(dest,p_route->dest),
       ospf_inet_ntoa(mask,p_route->mask), ospf_inet_ntoa(nexthop,p_route->fwdaddr),p_process->process_id,p_process->vrid,p_route->process_id);

    #if 0
    /* ignore routes that ospf added to the routing table */
    if (M2_ipRouteProto_ospf == p_route->proto) 
    {
        ospf_logx(ospf_debug_rtm, "RTM ignore OSPF route");
        return;
    }
    #endif
    if ((p_route->dest & 0xff000000) == 0x7f000000)
    {
        ospf_logx(ospf_debug_rtm, "RTM ignore loopback route");

        return;
    }    
    /*do not checking on non-asbr*/
    if (!p_process->asbr)
    {
        ospf_logx(ospf_debug_rtm, "RTM ignore non asbr");

        return; 
    }
    /*by default,external will have type 2*/
    p_route->metric |= OSPF_ASE_EBIT;

    /*ignore down interface for connected route*/
    if ((M2_ipRouteProto_local == p_route->proto) && enable)
    {
        if ((p_route->flags & RTF_UP) != RTF_UP)
        {
            /*IPSOFT cheng change to redistribute default route*/
            ospf_logx(ospf_debug_rtm, "RTM ignore down interface");    

            return; 
        }    
		#if 0
        /*ignore connected route from invalid interface*/
        if (!ospf_sys_is_local_network(p_process->vrid, p_route->dest|(~p_route->mask)))
        {
            ospf_logx(ospf_debug_rtm, "RTM ignore connected route from invalid interface");             

            return; 
        }
		#endif
    }
    /*protocol is not redistributed,ignore*/
    if (!BIT_LST_TST(p_process->reditribute_flag, p_route->proto))
    {
        ospf_logx(ospf_debug_rtm, "protocal %d not redistribute", p_route->proto);

        return ;
    }
    if ((M2_ipRouteProto_ospf == p_route->proto)
        &&(p_process->process_id == p_route->process_id))
    {

        ospf_logx(ospf_debug_rtm, "not redistribute ospf with same process id");
        return ;
    }
    p_import_route = ospf_lstlookup(&p_process->import_table, p_route);

    if (TRUE == enable)
    {
        /*create imported route if not exist,and schedule timer to originate lsa*/
        if (NULL == p_import_route)
        {
            p_import_route = ospf_import_route_create(p_process, p_route);
        }
        else
        {
            if (p_import_route->vpn_route 
                && p_route->vpn_route
                && p_import_route->vpn_internal != p_route->vpn_internal)
            {
                p_import_route->vpn_type_change = TRUE;
            }
        }
        if (NULL == p_import_route)
        {
            return;
        }
        p_import_route->metric = p_route->metric;
        p_import_route->path_type = p_route->path_type;
        p_import_route->active = TRUE;
        p_import_route->vpn_internal = p_route->vpn_internal;
        p_import_route->process_id = p_route->process_id;
        p_import_route->proto = p_route->proto;

        p_import_route->update = TRUE;
        
        ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
    }
    else 
    {
        /*set inactive flag for route,schedule timer to delete it and related lsa*/
        if (NULL != p_import_route)
        {
            p_import_route->vpn_internal = p_route->vpn_internal;
            p_import_route->active = FALSE;

            p_import_route->update = TRUE;
            
            ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
        }
    }
    return; 
}

/*APIs for import route list control*/

/*api used to create external route and insert into list*/
struct ospf_iproute * 
ospf_import_route_create(
               struct ospf_process *p_process,
               struct ospf_iproute *p_import_route)
{
    struct ospf_iproute *p_route = NULL;
    
    p_route = ospf_malloc2(OSPF_MIPROUTE);
    if (NULL == p_route)
    {
        return NULL;
    }
    memcpy(p_route, p_import_route, sizeof (struct ospf_iproute));
    p_route->p_process = p_process;
    ospf_lstadd(&p_process->import_table, p_route);
    return p_route;
}

/*delete import route*/
void 
ospf_import_route_delete(struct ospf_iproute *p_route)
{
    ospf_lstdel_free(&p_route->p_process->import_table, p_route, OSPF_MIPROUTE);
    return;
}

 /************************************************************
  compare imported route for radix lookup
  rule: dest mask cost, fwdaddr, tag
*/
int 
ospf_import_lookup_cmp(
               struct ospf_iproute *p1, 
               struct ospf_iproute *p2)
{
    if ((NULL == p1) || (NULL == p2))
    {
        return 0;
    }
   
    OSPF_KEY_CMP(p1, p2, dest);
    OSPF_KEY_CMP(p1, p2, mask);
    OSPF_KEY_CMP(p1, p2, proto);
    OSPF_KEY_CMP(p1, p2, fwdaddr);
    if ((p1->proto == M2_ipRouteProto_is_is)
        || (p1->proto == M2_ipRouteProto_ospf)
        || (p1->proto == M2_ipRouteProto_rip))
    {
        OSPF_KEY_CMP(p1, p2, process_id);
    }
    return 0; 
}

void 
ospf_import_table_init(struct ospf_process *p_process)
{
    ospf_lstinit(&p_process->import_table, ospf_import_lookup_cmp);
    return;
}

/*process a special external route imported to ospf*/ 
void 
ospf_import_route_lsa_originate(struct ospf_iproute *p_route)
{
    struct ospf_process *p_process = p_route->p_process;
    struct ospf_iproute *pstImportRoute = NULL;
    u_int faddr = 0;  
    u_int8 dest[16];       
    u_int8 mask[16];        
    u_int8 nhop[16];

    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }

    if((p_route->proto != M2_ipRouteProto_local)
        && (p_route->active == TRUE))
    {
        if(ospf_import_choose_proto2lsa(p_route) == OK)
        {
            return;
        }
    }
    
    if (ospf_debug_rtm)
    {
        ospf_inet_ntoa(dest, p_route->dest);
        ospf_inet_ntoa(mask, p_route->mask);
        ospf_inet_ntoa(nhop, p_route->fwdaddr);
        ospf_logx(ospf_debug_rtm, "detect %s redistribute route %s/%s/%s,internal %d", p_route->active ? "add" : "delete", dest, mask, nhop,p_route->vpn_internal);
    }
    if (p_route->vpn_internal)
    {
        ospf_vpn_summary_lsa_originate_all(p_process, p_route, !p_route->active);

    }
    else
    {
        /*forwarding address may change, so recover it*/
        faddr = p_route->fwdaddr ;
    
        /*check external lsa originate*/
        /*for defult route, only when def_route_adv is not enable can originate*/
        if ((0 == p_route->dest) && (0 == p_route->mask))
        {   
            if (p_process->def_route_adv)
            {
                ospf_external_lsa_originate(p_route);
            }        
        }
        else
        {
            ospf_external_lsa_originate(p_route);
        }
        /*recover nexthop*/
        p_route->fwdaddr = faddr;
        
        /*check nssa lsa originate*/
        ospf_nssa_lsa_originate_for_route(p_route);
    }
    return;
}


int ospf_import_choose_proto2lsa(struct ospf_iproute *pstRoute)
{
    struct ospf_iproute *pstImportRoute = NULL;
    struct ospf_iproute *pstImportRouteNext = NULL;

    if(pstRoute == NULL)
    {
        return ERR;
    }
    
    for_each_node(&pstRoute->p_process->import_table, pstImportRoute, pstImportRouteNext)
    {
        if((pstImportRoute->dest == pstRoute->dest)
            && (pstImportRoute->mask == pstRoute->mask)
            //&& (pstImportRoute->process_id == pstRoute->process_id)
            && (pstImportRoute->active == pstRoute->active))
        {
            if(pstImportRoute->proto == M2_ipRouteProto_local)
            {
                return OK;
            }
            
            if((pstRoute->proto == M2_ipRouteProto_is_is)
                && (pstImportRoute->proto == M2_ipRouteProto_ospf))
            {
                return OK;
            }
        }
    }
    return ERR;
}

/*delete all imported routes with same nexthop*/
void
ospf_import_delete_by_nexthop(
              struct ospf_process *p_process,
              u_int nexthop)
{
    struct ospf_iproute *p_route;
    struct ospf_iproute *p_next_route;

    for_each_node(&p_process->import_table, p_route, p_next_route)
    {
        /*start a timer to delete imported route and lsa*/
        if (p_route->fwdaddr == nexthop)
        {
            p_route->active = FALSE;
            p_route->update = TRUE;
            
            ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
        }
    }
    return;
}

/*compare function of redistribute policy*/
int
ospf_redis_policy_lookup_cmp(
            struct ospf_policy *p1, 
            struct ospf_policy *p2)
{
    OSPF_KEY_CMP(p1, p2, proto);
    OSPF_KEY_CMP(p1, p2, policy_index);
    if((p1->proto == M2_ipRouteProto_is_is)
        || (p1->proto == M2_ipRouteProto_ospf)
        || (p1->proto == M2_ipRouteProto_rip))
    {
        OSPF_KEY_CMP(p1, p2, proto_process);
    }
    return 0;
}

int
ospf_redis_policy_nm_cmp(
            struct ospf_policy *p1, 
            struct ospf_policy *p2)
{
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
    return ospf_redis_policy_lookup_cmp(p1, p2);
}

/*init redistribute policy table*/
void 
ospf_redis_policy_table_init(struct ospf_process *p_process)
{
    ospf_lstinit(&p_process->redis_policy_table, ospf_redis_policy_lookup_cmp);
    return;
}

/*lookup redistribute policy*/
struct ospf_policy *
ospf_redis_policy_lookup(
                 struct ospf_process *p_process, 
                 u_int policy_index, 
                 u_int protocol,
                 u_int proto_process)
{
    struct ospf_policy ospf_policy;     
    ospf_policy.policy_index = policy_index;
    ospf_policy.proto = protocol;
    ospf_policy.proto_process = proto_process;
    return ospf_lstlookup(&p_process->redis_policy_table, &ospf_policy);
}

/*add a redistribute policy*/
struct ospf_policy * 
ospf_redis_policy_add(
                 struct ospf_process *p_process, 
                 u_int policy_index, 
                 u_int potocol,
                 u_int proto_process)
{ 
    struct ospf_policy *p_policy = NULL;
   
    if (NULL == p_process->p_master->route_policy_set)
    {
        return NULL;
    }    
    p_policy = ospf_redis_policy_lookup(p_process, policy_index, potocol, proto_process);
    if (NULL != p_policy)
    {
        return p_policy;
    }
   
    p_policy = ospf_malloc2(OSPF_MPOLICY);
    if (NULL == p_policy)
    {
        return NULL;
    }  
    p_policy->p_process = p_process;
    p_policy->policy_index = policy_index;
    p_policy->proto = potocol;    
    p_policy->proto_process = proto_process;    
#ifdef OSPF_POLICY
    p_policy->policy_type = PCL_TYPE_RMAP;
#else
    p_policy->policy_type = 0;
#endif
    
    ospf_lstadd(&p_process->redis_policy_table, p_policy);   
   
    ospf_lstadd(&ospf.nm.redistribute_policy_table, p_policy);
    
    p_process->p_master->route_policy_set(p_policy->policy_index, 1);
    return p_policy;
}

/*delete a redsitribute policy*/
void 
ospf_redis_policy_delete(struct ospf_policy *p_policy)
{
    struct ospf_process *p_process = p_policy->p_process;
    
    if (p_process->p_master->route_policy_set)
    {
        p_process->p_master->route_policy_set(p_policy->policy_index, 0);
    }
 
    ospf_lstdel(&ospf.nm.redistribute_policy_table, p_policy);
 
    /*remove from policy list*/
    ospf_lstdel_free(&p_process->redis_policy_table, p_policy, OSPF_MPOLICY);
    return;
}

/*20121203 ospf redis range*/
struct ospf_redis_range *
ospf_redis_range_create(
                   struct ospf_process *p_process,
                   u_int protocol,
                   u_int dest,
                   u_int mask)
{
    struct ospf_redis_range *p_node = NULL; 
    
    p_node = ospf_malloc2(OSPF_MREDISTRIBUTE);    
    if (NULL != p_node)
    {
        p_node->dest = dest;
        p_node->mask = mask;
        p_node->protocol = protocol;
        p_node->p_process = p_process;
        p_node->no_translate = FALSE;
        ospf_lstadd(&p_process->redis_range_table, p_node);
        ospf_lstadd(&ospf.nm.redis_range_table, p_node);
    }    
    return p_node;
}

void 
ospf_redis_range_delete(struct ospf_redis_range *p_redistribute)
{    
    struct ospf_process *p_process = p_redistribute->p_process;

    ospf_lstdel(&ospf.nm.redis_range_table, p_redistribute);

    ospf_lstdel_free(&p_process->redis_range_table, p_redistribute, OSPF_MREDISTRIBUTE);
    return;
}

/*lookup redistribute node*/
struct ospf_redis_range *
ospf_redis_range_lookup(
           struct ospf_process *p_process,
           u_int protocol,
           u_int dest,
           u_int mask)
{
    struct ospf_redis_range redistribute;
    redistribute.dest = dest;
    redistribute.mask = mask;
    redistribute.protocol = protocol; 
    return ospf_lstlookup(&p_process->redis_range_table, &redistribute);
}

/*compare function of redistribute*/
int 
ospf_redis_range_lookup_cmp(
          struct ospf_redis_range *p1, 
          struct ospf_redis_range *p2)
{
    OSPF_KEY_CMP(p1, p2, dest);
    OSPF_KEY_CMP(p1, p2, mask);
    //OSPF_KEY_CMP(p1, p2, protocol);
    return 0;
}

int 
ospf_redistribute_range_nm_cmp(
           struct ospf_redis_range *p1, 
           struct ospf_redis_range *p2)
{
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
    return ospf_redis_range_lookup_cmp(p1, p2);
}

void 
ospf_redis_range_table_init(struct ospf_process *p_process)
{   
    ospf_lstinit(&p_process->redis_range_table, ospf_redis_range_lookup_cmp);
    return;
}

/*get best matched range for dest*/
struct ospf_redis_range *
ospf_redis_range_match(
               struct ospf_process *p_process,
               u_int protocol,
               u_int dest,
               u_int mask,
               struct ospf_redis_range *p_range_exclued)
{
    struct ospf_redis_range *p_range = NULL; 
    struct ospf_redis_range *p_next = NULL; 
    struct ospf_redis_range *p_best = NULL; 
    
    for_each_node(&p_process->redis_range_table, p_range, p_next)
    {
        //if ((p_range->protocol != protocol) ||(p_range == p_range_exclued))
        if (p_range == p_range_exclued)
        {
            continue;
        }
  
        if (!ospf_netmatch(dest, p_range->dest, p_range->mask)
            || (mask < p_range->mask))
        {
            continue;
        }        
        if ((NULL == p_best ) || (p_range->mask > p_best->mask))
        {
            p_best = p_range;
        }               
    }
    return p_best;
}

/*originate external lsa for redistribute range*/
void 
ospf_redis_range_lsa_originate(
                      struct ospf_redis_range *p_range,
                      u_int flush)
{
    struct ospf_iproute route;
    int iRet = 0;
	
    memset(&route, 0, sizeof(route));
    route.p_process = p_range->p_process;
    route.dest = p_range->dest;
    route.mask = p_range->mask;
    /*by default,external will have type 2*/
    route.metric |= OSPF_ASE_EBIT;
    route.active = !flush;

    iRet = ospf_only_nssa_area_exist(route.p_process);
    if(iRet != OK)
    {
        ospf_external_lsa_originate(&route);
    }

    if((route.dest == 0)
        && (route.mask == 0)
        && (route.active == FALSE))
    {
        return;
    }
    
    ospf_nssa_lsa_originate_for_route(&route);
    return;
}

int ospf_only_nssa_area_exist(struct ospf_process *pstProcess)
{
    struct ospf_area *pstArea = NULL;
    struct ospf_area *pstAreaFirst = NULL;
    struct ospf_area *pstAreaNext = NULL;
    int iCount = 0;
    
    /* originate nssa for each nssa area*/
    for_each_ospf_area(pstProcess, pstAreaFirst, pstAreaNext)
    {
        iCount++;
        pstArea = pstAreaFirst;
    }
    if((iCount == 1) && (pstArea->is_nssa == TRUE))
    {
        return OK;
    }
    return ERR;
}

/*redistribute range enabled*/
void
ospf_redis_range_up(struct ospf_redis_range *p_range)
{
    struct ospf_process *p_process = p_range->p_process;
    struct ospf_iproute *p_route;
    struct ospf_iproute *p_next_route;
    struct ospf_redis_range *p_best_range = NULL;    
    u_int faddr = 0;  
    u_int count = 0;

    ospf_logx(ospf_debug_rtm, "redistribute range %x/%x up", p_range->dest,p_range->mask); 
    for_each_node(&p_process->import_table, p_route, p_next_route)
    {
        if (FALSE == p_route->active)
        {
            continue;
        }

        /*ignore not matched route*/
        if (!ospf_netmatch(p_route->dest,p_range->dest,p_range->mask)
            || p_route->mask < p_range->mask)
        {
            continue;
        }

        /*get best matched range for route,except p_range*/          
        p_best_range = ospf_redis_range_match(p_process, p_range->protocol, p_route->dest, p_route->mask, p_range);
        /*route still ranged by p_best_range*/
        if (p_best_range && p_best_range->mask > p_range->mask)
        {
            continue;
        }
        count++;

        if (NULL == p_best_range)
        {
            /*forwarding address may change, so recover it*/
            faddr = p_route->fwdaddr;
            /*route will ranged by p_range,flush lsa*/
            p_route->active = FALSE;
            ospf_external_lsa_originate(p_route);
            ospf_nssa_lsa_originate_for_route(p_route);
            p_route->active = TRUE;
            /*recover nexthop*/
            p_route->fwdaddr = faddr;
        }
        else
        {
            p_best_range->update = TRUE;
            ospf_timer_start(&p_process->redis_range_update_timer, 1);
        }
    }

    if (0 < count)
    {
        //ospf_redis_range_lsa_originate(p_range, FALSE);
        ospf_redis_range_lsa_originate(p_range, p_range->no_translate);
    }
    return;
}

void 
ospf_redis_range_down(struct ospf_redis_range *p_range)
{
    struct ospf_process *p_process = p_range->p_process;
    struct ospf_iproute *p_route;
    struct ospf_iproute *p_next_route;
    struct ospf_redis_range *p_best_range = NULL;    
    u_int faddr = 0;  
    u_int count = 0;
    u_int same = FALSE;

    ospf_logx(ospf_debug_rtm, "redistribute range %x/%x down", p_range->dest,p_range->mask); 

    for_each_node(&p_process->import_table, p_route, p_next_route)
    {
        if (FALSE == p_route->active)
        {
            continue;
        }

        /*ignore not matched route*/
        if (!ospf_netmatch(p_route->dest,p_range->dest,p_range->mask)
            || p_route->mask < p_range->mask)
        {
            continue;
        }
        p_best_range = ospf_redis_range_match(p_process, p_route->proto, p_route->dest, p_route->mask, NULL);
        if (p_best_range != p_range)
        {
            continue;
        }
        p_best_range = ospf_redis_range_match(p_process, p_route->proto, p_route->dest, p_route->mask, p_range);
        if (NULL == p_best_range)
        {
            /*forwarding address may change, so recover it*/
            faddr = p_route->fwdaddr;
            /*route will not ranged, originate lsa*/
            ospf_external_lsa_originate(p_route);
            ospf_nssa_lsa_originate_for_route(p_route);
            /*recover nexthop*/
            p_route->fwdaddr = faddr;
        }
        else
        {
            p_best_range->update = TRUE;
            ospf_timer_start(&p_process->redis_range_update_timer, 1);
        }
        count++;
        if ((p_route->dest == p_range->dest) && (p_route->mask == p_range->mask))
        {
            same = TRUE;
        }
    }
    if ((0 < count) && (FALSE == same))
    {
        /*flush lsa originate by range*/
        ospf_redis_range_lsa_originate(p_range, TRUE);
    }
    return;
}

void
ospf_redis_range_update(struct ospf_redis_range *p_range)
{
    struct ospf_process *p_process = p_range->p_process;
    struct ospf_iproute *p_route;
    struct ospf_iproute *p_next_route;
    struct ospf_redis_range *p_best_range = NULL;    
    u_int count = 0;
    u_int faddr = 0;

    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    p_range->update = FALSE;
    ospf_logx(ospf_debug_rtm, "redistribute range %x/%x update", p_range->dest,p_range->mask); 

    for_each_node(&p_process->import_table, p_route, p_next_route)
    {
        if (FALSE == p_route->active)
        {
            continue;
        }

        /*ignore not matched route*/
        if (!ospf_netmatch(p_route->dest,p_range->dest,p_range->mask)
            || p_route->mask < p_range->mask)
        {
            continue;
        } 
        p_best_range = ospf_redis_range_match(p_process, p_route->proto, p_route->dest, p_route->mask, NULL);
        if (p_best_range != p_range)
        {
            continue;
        }

        count++;
        /*route ranged,flush lsa*/
        if (p_range->dest != p_route->dest)
        {
            faddr = p_route->fwdaddr;
            p_route->active = FALSE;
            ospf_external_lsa_originate(p_route);
            ospf_nssa_lsa_originate_for_route(p_route);
            p_route->active = TRUE;
            /*recover nexthop*/
            p_route->fwdaddr = faddr;
        }
    }
    /*originate or flush range lsa*/
    //ospf_redis_range_lsa_originate(p_range, 0 < count ? FALSE : TRUE);
    ospf_redis_range_lsa_originate(p_range, ((0 < count)&&(p_range->no_translate == FALSE)) ? FALSE : TRUE);
    return;
}
void
ospf_redis_range_update_timeout(struct ospf_process *p_process)
{
    struct ospf_redis_range *p_redrange = NULL; 
    struct ospf_redis_range *p_next_redrange = NULL;
    
    for_each_node(&p_process->redis_range_table, p_redrange, p_next_redrange)
    {
        if (TRUE == p_redrange->update)
        {
            ospf_redis_range_update(p_redrange);
        }
    }
    
    return;
}

#if 1
int ospf_import_route(ospf_import_route_t *pstImportRoute)
{
    struct ospf_redistribute *pstRedistrFrist = NULL;
    struct ospf_redistribute *pstRedistrNext = NULL;
	struct ospf_iproute stRoute;
    if(pstImportRoute == NULL)
    {
        return ERR;
    }
    for_each_node(&ospf.nm.redistribute_table, pstRedistrFrist, pstRedistrNext)
    {
        /*vrf id 不匹配不引入，属于同一个进程不引入*/
        if (((pstImportRoute->proto == M2_ipRouteProto_ospf)
                && (pstImportRoute->process_id == pstRedistrFrist->p_process->process_id))
            || (pstRedistrFrist->p_process->vrid != pstImportRoute->vrid)
            || (pstRedistrFrist->action == FALSE))
        {
            continue;
        }
        /*引入配置不匹配，不引入*/
        if ((pstRedistrFrist->protocol != pstImportRoute->proto)
            || (pstRedistrFrist->proto_process != pstImportRoute->process_id))
        {
            continue;
        }
		memset(&stRoute, 0, sizeof(stRoute));

	    stRoute.p_process = pstRedistrFrist->p_process;
	    stRoute.process_id = pstImportRoute->process_id;
        stRoute.metric = pstImportRoute->metric;
        stRoute.proto = pstImportRoute->proto;
        stRoute.dest = pstImportRoute->dest;
        stRoute.mask = pstImportRoute->mask;
        stRoute.fwdaddr = pstImportRoute->fwdaddr;
        stRoute.active = pstImportRoute->active;
        stRoute.flags = IFF_UP;
        ospf_import_iproute(pstImportRoute->active, &stRoute);
    }
    return OK;
}
#else
int ospf_import_static_func(L3_IMPORT_ROUTE_INFO_T *pstImportRoute)
{
    struct ospf_redistribute *pstRedistrFrist = NULL; 
    struct ospf_redistribute *pstRedistrNext = NULL; 
	struct ospf_iproute stRoute;
    //u_long ulCurrent = 0;

    if(pstImportRoute == NULL)
    {
        return ERR;
    }
    
    for_each_node(&ospf.nm.redistribute_table, pstRedistrFrist, pstRedistrNext)
    {
        /*vrf id 不匹配不引入，属于同一个进程不引入*/
        if (((pstImportRoute->uiProto == M2_ipRouteProto_ospf)
                && (pstImportRoute->uiProtoProId == pstRedistrFrist->p_process->process_id))
            || (pstRedistrFrist->p_process->vrid != pstImportRoute->uiVrfId)
            || (pstRedistrFrist->action == FALSE))
        {
            continue;
        }
        /*引入配置不匹配，不引入*/
        if ((pstRedistrFrist->protocol != pstImportRoute->uiProto)
            || (pstRedistrFrist->proto_process != pstImportRoute->uiProtoProId))
        {
            continue;
        }
		memset(&stRoute, 0, sizeof(stRoute));      
		

	    stRoute.p_process = pstRedistrFrist->p_process;
	    stRoute.process_id = pstImportRoute->uiProtoProId;
        stRoute.metric = pstImportRoute->uiMetric;
        stRoute.proto = pstImportRoute->uiProto;
        stRoute.dest = pstImportRoute->uiDest;    
        in_len2mask((struct in_addr *)&stRoute.mask, pstImportRoute->uiMaskLen);
        stRoute.mask = ntohl(stRoute.mask); 
        //stRoute.mask = pstImportRoute->uiMaskLen;
        stRoute.fwdaddr = 0;//pstImportRoute->uiNextHop;
        stRoute.active = pstImportRoute->uiState;
        stRoute.flags = IFF_UP;
        //printf("ospf_import_static_func : dest=%#x mask=%#x process_id=%#x \n",stRoute.dest, stRoute.mask, stRoute.process_id);

        //struct ospf_iproute *p_import_route;
        //p_import_route = ospf_lstlookup(&stRoute.p_process->import_table, &stRoute);
        //printf("1-----ospf_import_static_func : p_import_route = %#x \n", p_import_route);
        ospf_import_iproute(pstImportRoute->uiState, &stRoute);
        //p_import_route = ospf_lstlookup(&stRoute.p_process->import_table, &stRoute);
        //printf("2-----ospf_import_static_func : p_import_route = %#x \n", p_import_route);
    }
    
    return OK;
}

int isis_import_public_func(L3_IMPORT_ROUTE_INFO_T *pstImportRoute)
{

}
#endif
