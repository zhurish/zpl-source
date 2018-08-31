/* ospf_export.c - export ospf route into ip route*/

#include "ospf.h"



/*export route compare function*/
int 
ospf_iproute_cmp(
         struct ospf_iproute *p1,
         struct ospf_iproute *p2)
{
    if ((!p1) || (!p2))
    {
        return 0;
    }
   
    OSPF_KEY_CMP(p1, p2, dest);
    OSPF_KEY_CMP(p1, p2, mask);
    OSPF_KEY_CMP(p1, p2, fwdaddr);
    return 0;
}

/* for a special route entry,check if need update to ip table     
     TRUE:can update
     FALSE:can not update*/
u_int 
ospf_sys_route_verify(struct ospf_iproute *p_route)
{
    struct ospf_process *p_process = p_route->p_process;
    u_int8 dstr[16];
    u_int8 maskstr[16];
    u_int8  nhopstr[16];
    u_int dest = p_route->dest;
    u_int netmask = p_route->mask;
    u_int nexthop = p_route->fwdaddr;
        
    if (ospf_debug_route)
    {
        ospf_inet_ntoa(dstr, dest);
        ospf_inet_ntoa(maskstr, netmask);
        ospf_inet_ntoa(nhopstr, nexthop);
        ospf_logx(ospf_debug_route, "%s ip route %s/%s/%s",
              p_route->active == TRUE  ? "add" :"delete",
              dstr, maskstr, nhopstr);
    }

 	ospf_logx(ospf_debug,"%d,p_route->fwdaddr=%x.\r\n",__LINE__,p_route->fwdaddr);    
    
    /*nexthop un-resolved,ignore*/
    if (0 == p_route->fwdaddr)
    {
        ospf_logx(ospf_debug_route, "ignore invalid nexthop route");
        return FALSE;
    }
    /*ignore SHAMLINK and TETUNNEL*/
    if ((OSPF_NEXTHOP_SHAMLINK == p_route->nexthop_type)
        || (OSPF_NEXTHOP_TETUNNEL == p_route->nexthop_type))
    {
        ospf_logx(ospf_debug_route, "ignore shamlink or tetunnel route");
        /*FA:te tennel will reoriginate router lsa*/
        if (OSPF_NEXTHOP_TETUNNEL == p_route->nexthop_type)
        {
            ospf_stimer_start(&p_process->te_tunnel_timer, OSPF_TE_TUNNEL_INTERVAL);
        }
		ospf_logx(ospf_debug_route,"%d\r\n",__LINE__);    
        return FALSE;
    }
    /*host route may delete current arp entry,so,do not add
      this route:dest is same as nexthop*/
    if (dest == nexthop)
    {
        ospf_logx(ospf_debug_route, "ignore invalid nexthop route,nexthop is same as dest");
	    ospf_logx(ospf_debug_route,"%d\r\n",__LINE__);    
        return FALSE;
    }

 	ospf_logx(ospf_debug_route,"%d,path_type=%d.\r\n",__LINE__,p_route->path_type);    
#ifdef OSPF_DCN
	if(p_process->ulDcnFlag != OSPF_DCN_FLAG)
	{
	   /*ignore intra route calculated from local interface*/
	    if ((OSPF_PATH_INTRA == p_route->path_type)
	       && ospf_sys_is_local_addr(p_process->vrid, nexthop))
	    {
	        ospf_logx(ospf_debug_route, "ignore invalid nexthop route,nexthop is local addr");
			ospf_logx(ospf_debug_route,"%d\r\n",__LINE__);    
	        return FALSE;
	    }
	}
#endif		
	
    /*filter checking*/
    if (FALSE == ospf_filter_policy_verify(p_route, NULL))
    {
	 	ospf_logx(ospf_debug_route,"%d\r\n",__LINE__);    
       return TRUE;
    }
    /*this route is filtered by policy*/
    ospf_logx(ospf_debug_route, "route is filtered");

    if (TRUE == p_route->active)
    {
        /*this is active route,add to filtered table*/ 
        ospf_filtered_route_add(p_process, p_route);
    }
    else
    {
        /*this is inactive route,remove from filtered table*/ 
        ospf_filtered_route_delete(p_route);
    }
	ospf_logx(ospf_debug_route,"%d\r\n",__LINE__);    
    return FALSE;
}

/*export time limit,used for large routing table control*/
#define OSPF_EXPORT_TIME_LIMIT 3

/*20110805 new export */
void  
ospf_network_route_change_check(struct ospf_process *p_process)
{
	struct ospf_route last_route;
	struct ospf_route *p_route = NULL;
	struct ospf_route *p_nextroute = NULL;
	struct ospf_route *p_sucess = NULL;
	struct ospf_nexthop *p_nexthop = NULL;
	struct ospf_nexthop *p_next_nexthop = NULL;
	struct ospf_area *p_area = NULL;
	struct ospf_area *p_nextarea = NULL;
	struct ospf_spf_vertex *p_vertex = NULL;
	struct ospf_spf_vertex *p_nextvertex = NULL;
	u_int start = ospf_sys_ticks();
	u_int now = 0;

	/*full export request must exist*/
	if (!p_process->wait_export)
	{
		ospf_logx(ospf_debug, "%d  *****return******\n", __LINE__);
		return;
	}
	memset(&last_route, 0, sizeof(last_route));
	if (OSPF_HOST_NETWORK != p_process->last_export_network)
	{
		last_route.type = OSPF_ROUTE_NETWORK;
		last_route.dest = p_process->last_export_network;
		last_route.mask = p_process->last_export_mask;
	}

	/*start update from next route*/
	for_each_node_greater(&p_process->route_table, p_route, &last_route)
	{
		/*if summary check need,and self is abr,check summary lsa*/
		if ((FALSE == p_route->summary_checked) && (TRUE == p_process->abr))
		{
			p_route->summary_checked = TRUE;
			ospf_summary_lsa_update(p_process, p_route);
		}

		/*update route to system,if failed,do not wait here,try to re-update after a
		 period*/
		if (OK != ospf_sys_route_update(p_process, p_route))
		{
			/*update failed, we will restart update from the current route*/
			if (NULL != p_sucess)
			{
				p_process->last_export_network = p_sucess->dest;
				p_process->last_export_mask = p_sucess->mask;
			}

			/*schedule update timer*/
			p_process->wait_export = TRUE;
			ospf_timer_start(&p_process->ipupdate_timer, 1);
			ospf_logx(ospf_debug, "%d  *****ospf_sys_route_update ERR******\n",
					__LINE__);
			return;
		}

		/*record last success route*/
		p_sucess = p_route;

		/*if do not wait for finish,we will stop update if exceed timer limit*/
		/*if time limit exceed,stop update, we will restart from the next route*/
		now = ospf_sys_ticks();
		if (OSPF_EXPORT_TIME_LIMIT < ospf_time_differ(now, start))
		{
			p_process->last_export_network = p_route->dest;
			p_process->last_export_mask = p_route->mask;
			p_process->wait_export = TRUE;
			ospf_logx(ospf_debug, "wait next process now = %x start = %x\n",
					now, start);
			ospf_timer_start(&p_process->ipupdate_timer, 1);
			return;
		}
	}

	/*clear nexthop's use flag*/
	for_each_node(&p_process->nexthop_table, p_nexthop, p_next_nexthop)
	{
		p_nexthop->active = FALSE;
	}

	/*all update finished,prepare to delete unused routes*/
	for_each_node(&p_process->route_table, p_route, p_nextroute)
	{
		ospf_old_route_clear(p_process, &p_process->route_table, p_route);
	}

	/*set used nexthop according to spf&abr&abr route*/
	for_each_node(&p_process->area_table, p_area, p_nextarea)
	{

		for_each_node(&p_area->abr_table, p_route, p_nextroute)
		{

			if (p_route->path[p_process->current_route].p_nexthop)
			{
				p_route->path[p_process->current_route].p_nexthop->active =
						TRUE;
			}
#ifdef OSPF_DCN
			if(p_process->process_id == OSPF_DCN_PROCESS)
			{
				//ospf_ospf_route_print(p_route);
				if (OK != ospf_sys_route_update(p_process, p_route))
				{
					ospf_logx(ospf_debug,"%d  **ospf_sys_route_update***ERR*****\n",__LINE__);
				}
			}
#endif
		}
		for_each_node(&p_area->asbr_table, p_route, p_nextroute)
		{
			if (p_route->path[p_process->current_route].p_nexthop)
			{
				p_route->path[p_process->current_route].p_nexthop->active =
						TRUE;
			}
		}
		for_each_node(&p_area->spf_table, p_vertex, p_nextvertex)
		{
			if (p_vertex->p_nexthop)
			{
				p_vertex->p_nexthop->active = TRUE;
			}
		}
	}

	/*delete unused nexthop*/
	for_each_node(&p_process->nexthop_table, p_nexthop, p_next_nexthop)
	{
		if (p_nexthop->active == FALSE)
		{
			ospf_lstdel_free(&p_process->nexthop_table, p_nexthop,
					OSPF_MNEXTHOP);
		}
	}

	/*clear export flag*/
	p_process->wait_export = FALSE;
	/*start 5seconds frr timer if enabled*/
#ifdef OSPF_FRR/*20130415 frr test*/
	if (TRUE == p_process->frr_enable)
	{
		p_process->backup_spf_called_count++;
		ospf_stimer_start(&p_process->frr_timer, OSPF_FRR_INTERVAL);
	}
#endif
	return;
}

/*compare function of filter policy*/
int 
ospf_filter_policy_lookup_cmp(
              struct ospf_policy *p1,
              struct ospf_policy *p2)
{
    OSPF_KEY_CMP(p1, p2, policy_index);
    return 0;
}

int 
ospf_filter_policy_nm_cmp(
              struct ospf_policy *p1,
              struct ospf_policy *p2)
{
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
    OSPF_KEY_CMP(p1, p2, policy_index);
    return 0;
}

/*init filter policy table*/
void 
ospf_filter_policy_table_init(struct ospf_process *p_process)
{
    ospf_lstinit(&p_process->filter_policy_table, ospf_filter_policy_lookup_cmp);
    return;
}

/*lookup filter policy*/
struct ospf_policy *
ospf_filter_policy_lookup(
                 struct ospf_process *p_process, 
                 u_int policy_index)
{
    struct ospf_policy ospf_policy;
    ospf_policy.policy_index = policy_index;
    return ospf_lstlookup(&p_process->filter_policy_table, &ospf_policy);
}

/*add a filter policy*/
struct ospf_policy *
ospf_filter_policy_add(
                   struct ospf_process *p_process, 
                   u_int policy_index)
{ 
    struct ospf_policy *p_policy = NULL;
	

    if (NULL == p_process->p_master->route_policy_set)
    {
        return NULL;
    }

    /*if policy exist,do not add again*/
    p_policy = ospf_filter_policy_lookup(p_process, policy_index);
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
     
    ospf_lstadd(&p_process->filter_policy_table, p_policy);   

    ospf_lstadd(&ospf.nm.filter_policy_table, p_policy);
    
    p_process->p_master->route_policy_set(policy_index, 1);

    ospf_timer_init(&p_policy->update_timer, p_policy, ospf_policy_update_event_handler, p_process);
    ospf_timer_init(&p_policy->delete_timer, p_policy, ospf_policy_delete_event_handler, p_process);

    return p_policy;
}

/*delete a filter policy*/
void 
ospf_filter_policy_delete(struct ospf_policy *p_policy)
{
    struct ospf_global *p_master = p_policy->p_process->p_master;
    
    if (NULL != p_master->route_policy_set)
    {
        /*decrease the route policy reference*/
        p_master->route_policy_set(p_policy->policy_index, 0);
    }       
 
    ospf_timer_stop(&p_policy->update_timer);
    ospf_timer_stop(&p_policy->delete_timer);
 
    ospf_lstdel(&ospf.nm.filter_policy_table, p_policy);
               
    /*remove from policy list*/
    ospf_lstdel_free(&p_policy->p_process->filter_policy_table, p_policy, OSPF_MPOLICY);
    return;
}

/*lookup filtered route*/
struct ospf_iproute *
ospf_filtered_route_lookup(
                        struct ospf_process *p_process,
                        struct ospf_iproute *p_route)
{
    return ospf_lstlookup(&p_process->filtered_route_table, p_route);
}

/*add filtered route*/
struct ospf_iproute * 
ospf_filtered_route_add(
              struct ospf_process *p_process,
              struct ospf_iproute *p_route)
{
    struct ospf_iproute *p_new = NULL;
    
    /*if already exist,do not add more*/ 
    p_new = ospf_filtered_route_lookup(p_process, p_route);
    if (NULL != p_new)
    {
        return p_new;
    }
    
    /*create*/     
    p_new = ospf_malloc2(OSPF_MIPROUTE);
    if (NULL == p_new)
    {
        return NULL;
    }

    p_new->dest = p_route->dest;
    p_new->mask = p_route->mask;
    p_new->fwdaddr = p_route->fwdaddr;
    p_new->p_process = p_process;
    ospf_lstadd(&p_process->filtered_route_table, p_new);
    
    return p_new;                     
}

/*delete filtered route*/
void 
ospf_filtered_route_delete(struct ospf_iproute *p_route)
{
    struct ospf_iproute *p_current = NULL;
    struct ospf_process *p_process = p_route->p_process;
    p_current = ospf_filtered_route_lookup(p_process, p_route);
    if (NULL != p_current)
    {
        ospf_lstdel_free(&p_process->filtered_route_table, p_current, OSPF_MIPROUTE);
    }
    return;
}

/*check a route's filter state*/
STATUS 
ospf_filtered_route_update(
                        struct ospf_route *p_route,
                        struct ospf_policy *p_policy)
{  
    struct ospf_route update;
    struct ospf_nexthop nexthop;
    struct ospf_iproute iproute;
    struct ospf_path *p_newpath = NULL;
    struct ospf_process *p_process = p_policy->p_process;
    u_int new_index = p_process->current_route; 
    u_int gateway = 0;
    u_int i = 0;
    STATUS rc;

//	ospf_ospf_route_print(p_route);
    /*ignored internal route type*/
    if (OSPF_ROUTE_NETWORK != p_route->type)
    {
   
        return OK;
    }
  
    p_newpath = &p_route->path[new_index];
    /*ignore invalid node*/
    if ((OSPF_PATH_INVALID == p_newpath->type)
        || (NULL == p_newpath->p_nexthop)
        /*20131018
        || (TRUE == p_route->local_route)*/)
    {
    
        return OK;
    }
    /*20131018*/
    if (p_newpath->local_route)
    {
        return OK;
    }
    /*scan for each nexthop*/
    for (i = 0 ; i < p_newpath->p_nexthop->count; i++)
    {        
        gateway = p_newpath->p_nexthop->gateway[i].addr;
        if ((0 == gateway) || (p_route->dest == gateway))
        {
            continue;
        }

        nexthop.count = 1;
        nexthop.gateway[0].addr = gateway;
        nexthop.gateway[0].if_uint = p_newpath->p_nexthop->gateway[i].if_uint;

        /*export route*/
        ospf_route_to_ip_route(&iproute, p_route, gateway, p_newpath->cost, p_newpath->p_nexthop->gateway[i].if_uint);
        iproute.active = TRUE;
        iproute.path_type = p_newpath->type;
        iproute.p_process = p_process;
        
        /*filter checking:if filtered,add to filter list and delete from ip;if 
        not filtered, delete from filter list and add to ip*/
        if (TRUE == ospf_filter_policy_verify(&iproute, NULL))
        {
            if (ospf_filtered_route_lookup(p_process, &iproute))
            {
                continue;
            }

            /*add to filter table,delete from system*/
            iproute.active = FALSE;

            ospf_update_route_construct(&update, p_route->dest, 
            p_route->mask, 0, 0, &nexthop, 
            p_process->old_route);

            ospf_lstdel(&p_process->filter_policy_table, p_policy);

            rc = ospf_sys_route_update(p_process, &update);

            ospf_lstadd(&p_process->filter_policy_table, p_policy);

            if (OK != rc)
            {
            	return ERR;
            }            
            ospf_filtered_route_add(p_process, &iproute); 
        }
        else
        {
            if (NULL == ospf_filtered_route_lookup(p_process, &iproute))
            {
                continue;
            }
 
            /*remove from filter table,add to system*/
            iproute.active = TRUE;
 
            ospf_update_route_construct(&update, p_route->dest, 
            p_route->mask, p_newpath->cost,
            p_newpath->cost2, &nexthop,
            p_process->current_route);
 
            iproute.p_process = p_process;
            
            rc = ospf_sys_route_update(p_process, &update);
            if (OK != rc)
            {
                return ERR;
            }           
            ospf_filtered_route_delete(&iproute); 
        }
    }
    return OK;
}

/*scheduled policy update processing,must wait
  for normal system update finishing*/
void
ospf_policy_update_event_handler(struct ospf_policy *p_policy)
{
    struct ospf_route *p_route = NULL;
    struct ospf_route *p_nextroute = NULL;
    struct ospf_process *p_process = p_policy->p_process;
    
    /*if full route update not finished,do nothing*/    
    if (p_process->wait_export || p_process->send_routemsg)
    {
        ospf_timer_start(&p_policy->update_timer, 5);
        return;
    }

    /*scan for all route*/
    for_each_node(&p_process->route_table, p_route, p_nextroute)     
    {
        /*if filter route update failed,means some system update problem,
          so wait for a period using timer*/
        if (OK != ospf_filtered_route_update(p_route, p_policy))
        {
            ospf_timer_start(&p_policy->update_timer, 5);
            break;
        }
    }
    return;
}

/*scheduled policy delete processing,must wait 
for normal system update finishing*/
void
ospf_policy_delete_event_handler(struct ospf_policy *p_policy)
{
    struct ospf_route *p_route = NULL;
    struct ospf_route *p_nextroute = NULL;
    struct ospf_process *p_process = p_policy->p_process;
    struct ospf_iproute ip_route;
    struct ospf_route update;
    struct ospf_nexthop nexthop;
    struct ospf_path *p_path = NULL;
    u_int i;
    STATUS rc;

    /*scan for all route*/
    for_each_node(&p_process->route_table, p_route, p_nextroute)
    { 
        /*ignore un-resolved nexthop*/
        p_path = &p_route->path[p_process->current_route];

        if (NULL == p_path->p_nexthop)
        {
            continue;
        }
        /*ignore local connected route*/
        /*20131018*/
        if (p_path->local_route)
        {
            continue;
        }
        /*check each nexthop address*/
        for (i = 0 ; i < p_path->p_nexthop->count ; i++)
        {
            ospf_route_to_ip_route(&ip_route, p_route, p_path->p_nexthop->gateway[i].addr, p_path->cost, p_path->p_nexthop->gateway[i].if_uint);        
            ip_route.path_type = p_path->type;
            ip_route.p_process = p_process;

            /*if route is only filtered by this policy,re-add it to system and delete from filter table*/
            #if 0
            if ((FALSE == ospf_filter_policy_verify(&ip_route, NULL))                                 
                ||  (TRUE == ospf_filter_policy_verify(&ip_route, p_policy)))
            #endif
            if (TRUE == ospf_filter_policy_verify(&ip_route, NULL))
            {
                continue;
            }
             
            ip_route.active = TRUE;
            nexthop.count = 1;
            nexthop.gateway[0].addr = p_path->p_nexthop->gateway[i].addr;
            nexthop.gateway[0].if_uint = p_path->p_nexthop->gateway[i].if_uint;
                      
            ospf_update_route_construct(&update, p_route->dest, p_route->mask, 
             p_path->cost, p_path->cost2, &nexthop, 
             p_process->current_route);
            
            /*remove policy*/
            ospf_lstdel(&p_process->filter_policy_table, p_policy);
            
            /*update route*/
            rc = ospf_sys_route_update(p_process, &update);
            
            /*add policy*/
            ospf_lstadd(&p_process->filter_policy_table, p_policy);

            /*system update failed,wait for a period using timer,do not pend here*/
            if (OK != rc)
            {
                ospf_timer_start(&p_policy->delete_timer, 5);
                return ;
            }
            
            ip_route.p_process = p_process;
            ospf_filtered_route_delete(&ip_route);
        }
    }        
    /*delete policy*/
    ospf_filter_policy_delete(p_policy);
    return;
}
