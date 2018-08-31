/* ospf_restart.c for graceful restart

*/


#include "ospf.h"

u_int ospf_lsa_flood_verify(struct ospf_lsa * p_lsa, struct ospf_if * p_if, struct ospf_if * p_rx_if, struct ospf_nbr * p_rx_nbr);

/*request enter graceful restart*/
void 
ospf_restart_request(
                   struct ospf_process *p_process,
                   u_int restart_status)   
{
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    u_int nbrcount = 0;
    u_int8 state_mask[4] = {0};

    if (FALSE == p_process->restart_enable)
    {
        ospf_logx(ospf_debug_gr, "graceful restart is not enable");
        return;
    }
    BIT_LST_SET(state_mask, OSPF_NS_FULL);

    ospf_logx(ospf_debug_gr, "ospf request  graceful restart");
    
    /*already in restarting ,do nothing*/
    if (p_process->in_restart) 
    {
        ospf_logx(ospf_debug_gr, "ospf is in graceful restart, return ");
        return;
    }
    p_process->restart_status = restart_status;
    p_process->gr_nbr_count = 0;
    /*if there are neighbors on one interface, it must be full state,else can not be in restarting*/
    for_each_ospf_if(p_process, p_if, p_next_if)
    {
        for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
        {
            if (FALSE == p_nbr->opaque_enable)
            {
                ospf_logx(ospf_debug_gr, "ospf have nbr not support opaque,return");
                return;
            }
        }
        nbrcount = ospf_lstcnt(&p_if->nbr_table);
        if ((0 != nbrcount) 
            && (nbrcount != ospf_if_nbr_count_in_state(p_if, state_mask)))
        {
            ospf_logx(ospf_debug_gr, "ospf have nbr not in FULL ,return");
            return;
        }
        /*is some retransmit exist for neighbor? TBI*/
        
        /*originate restart lsa for up interface*/
        if (OSPF_IFS_DOWN != p_if->state)        
        {
            ospf_restart_lsa_originate(p_if);
   
            /*send update now,do not wait*/
            ospf_update_output(&p_if->update);
            
            if (p_if->update.p_msg)
            {
                ospf_mfree(p_if->update.p_msg, OSPF_MPACKET);
                p_if->update.p_msg = NULL;
                p_if->update.maxlen = 0;
            }       
        }
        p_process->gr_nbr_count += nbrcount;
    }

    /*enter restart state,but do not flush dynamic information,we do it when all lsa are acked*/
    ospf_stimer_start(&p_process->restart_timer, p_process->restart_period);

    /*prepare to wait for all ack*/
    p_process->in_restart = TRUE;
    ospf_timer_start(&p_process->restart_wait_timer, 10);
    return;
}

/*enter restart state,clear all dynmaic information,do not update ip route*/
void 
ospf_restart_start(struct ospf_process *p_process) 
{
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;    
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_nextarea = NULL;

    /*stop waiting timer*/
    ospf_timer_stop(&p_process->restart_wait_timer);

    /*start restarting timer*/
    p_process->in_restart = TRUE ;
    ospf_stimer_start(&p_process->restart_timer, p_process->restart_period);

    ospf_logx(ospf_debug_gr, "all grace lsa acked,graceful restart begin");

    /*clear area's dynamic information*/
    for_each_ospf_area(p_process, p_area, p_nextarea)
    {
        ospf_area_down(p_area);
    }
    /*clear all lsa*/
    ospf_lsa_flush_all(p_process);

    /*recover area*/
    for_each_ospf_area(p_process, p_area, p_nextarea)
    {
        ospf_area_up(p_area);
    }

    /*bring up all valid interface*/
    for_each_ospf_if(p_process, p_if, p_next_if)
    {
       ospf_if_state_update(p_if);
    }
       
    ospf_trap_restartstatus(p_process);
    return;
}

/*delete all self origianted graceful restart lsa,called when restarting finished*/
void
ospf_restart_local_graceful_lsa_flush(struct ospf_process *p_process)
{ 
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next_lsa = NULL;
    u_int8 ifname[32];
    
    for_each_ospf_if(p_process, p_if, p_next_if)
    {
        for_each_node(&p_if->opaque_lstable.list, p_lsa, p_next_lsa)
        {
            /*type is graceful restart,adv id is same to me*/
            if ((OSPF_GR_LSID == ntohl(p_lsa->lshdr->id)) 
               && (p_process->router_id == ntohl(p_lsa->lshdr->adv_id) ))
            {
                ospf_logx(ospf_debug_gr, "clear gr lsa on %s", ospf_inet_ntoa(ifname, p_if->addr));
  
                /*clear gr flag and set age to max*/
                p_lsa->self_rx_in_restart = FALSE;
                ospf_lsa_maxage_set(p_lsa);
                ospf_lsa_flood(p_lsa, NULL, NULL);
  
            }
        }
        ospf_ism(p_if, OSPF_IFE_NBR_CHANGE);
    }
    return;
}

/*
Called when a router exit graceful restarting.Graceful restarting may fully finish
or aborted because of some exception event.
This function will reset graceful restart state,and do some refresh operation on current
interface state,lsdb and routing table.*/
void 
ospf_restart_finish(
               struct ospf_process *p_process,
               u_int reason)
{
    struct ospf_iproute *p_route = NULL;
    struct ospf_iproute *p_next = NULL;
    struct ospf_lst list;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    u_int i = 0;
    u_int8 str[OSPF_RESTART_TOPLOGY_CHANGED+1][20] = {" ","none","inprogress","complete","timeout", "topo changed"};
   
    /*if currently graceful restart is not started,do nothing*/
    if (!p_process->in_restart)
    {
        return;
    }
    ospf_logx(ospf_debug_gr, "leave graceful restarting, reason=%s", str[reason]);
    
    /*clear GR state and stop GR timer*/
    p_process->in_restart = FALSE;
    p_process->restart_status = OSPF_RESTART_NO_RESTARTING;
    p_process->restart_route_finish = TRUE;
    p_process->restart_exitreason = reason;
    if (reason == OSPF_RESTART_TIMEOUT)
    {
     //   route_del_by_type(ZEBRA_ROUTE_PROTOCOL_OSPF);
    }
    ospf_timer_stop(&p_process->restart_timer);
   
    /*clear graceful restart lsa*/
    ospf_restart_local_graceful_lsa_flush(p_process);
   
    /*rebuild router lsa*/
    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        ospf_router_lsa_originate (p_area);      
    }
   
    for_each_ospf_if(p_process, p_if, p_next_if)
    {
        /*rebuild network lsa*/
        if (p_if->dr == p_if->addr)
        {
            ospf_network_lsa_originate (p_if);
        }
        /*check interface state*/
        ospf_if_state_update(p_if);
   
        /*calculate cost???*/
        if (!p_if->configcost)
        {
            ospf_if_cost_update(p_if, 0, 0);   
        }
    }
   
    /*get all route with special protocol*/
    ospf_lstinit(&list, NULL);
    
    for (i = 0 ; i < M2_ipRouteProto_max; i++)
    {
        if (BIT_LST_TST(p_process->reditribute_flag, i))
        {
            ospf_sys_route_list_get(p_process->vrid, i, &list);
        }
    }
   
    for_each_node(&list, p_route, p_next)
    {
        p_route->p_process = p_process;
        ospf_import_iproute(TRUE, p_route);
   
        ospf_lstdel_unsort(&list, p_route);
        ospf_mfree(p_route, OSPF_MIPROUTE);
    }  
    
    ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
    p_process->import_update = TRUE;
    
    ospf_import_route_update_all(p_process);
   
    /*force to calculate route*/
    ospf_route_calculate_full(p_process);
   
    /*force to originate summary lsa*/
    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        ospf_summary_lsa_originate_for_area(p_area, !p_area->p_process->abr) ;
    }
   
    /*force to refresh all self originated lsa*/
    ospf_lsa_force_refresh(p_process);
    
    ospf_trap_restartstatus(p_process);
    return;
}

/*check if all grlsa are acked,if so ,we can begin restart*/
u_int 
ospf_all_lsa_acked(struct ospf_process *p_process)
{
    struct ospf_retransmit *p_rxmt = NULL;
    struct ospf_retransmit *p_next = NULL;
    /*decide if all lsa are acked,includ grace lsa*/
    for_each_node(&p_process->rxmt_table, p_rxmt, p_next)
    {
        if (p_rxmt->p_lsa && (TRUE == ospf_lsa_rxmt_exist(p_rxmt->p_lsa)))
        {
            ospf_logx(ospf_debug_gr, "some lsa is in retrantmist list");
            return FALSE; 
        }
    }
    return TRUE;
}

/*function to check if area's topo changed during restarting
  called when nbr in this area enter full state
*/
u_int 
ospf_restart_topo_changed(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_nextarea = NULL;
    struct ospf_network_lsa *p_network = NULL;
    struct ospf_router_lsa *p_router = NULL;
    struct ospf_router_link *p_link = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_nbr_lsa = NULL;
    struct ospf_lsa *p_netlsa = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_vif = NULL;
    struct ospf_lshdr lshdr;
    u_int self_appear = FALSE;
    u_int *p_netlink = NULL;
    u_int8 state_mask[4] = {0};
 
    for_each_ospf_area(p_process, p_area, p_nextarea)
    {
        /*no full nbr exist,can not decide anything*/
        if (FALSE == ospf_area_full_nbr_exist(p_area))
        {
            continue;
        }
        /*self originated router lsa must exist*/
        lshdr.type = OSPF_LS_ROUTER;
        lshdr.id = htonl(p_process->router_id);
        lshdr.adv_id = htonl(p_process->router_id);
     
        /*self router lsa not exist,finish restart*/
        p_lsa = ospf_lsa_lookup(p_area->ls_table[lshdr.type], &lshdr);
        if (NULL == p_lsa)
        {
           ospf_logx(ospf_debug_gr, "self router lsa not exit, lsa count %d", ospf_lstcnt(&p_area->ls_table[lshdr.type]->list)); 
           return TRUE ;
        }
     
        /*scan all router link in router lsa*/
        p_router = (struct ospf_router_lsa *)p_lsa->lshdr;
        for_each_router_link(p_router, p_link)
        {
            switch (p_link->type) {
                case OSPF_RTRLINK_STUB:
                     /*up interface exist for stub:id is network of interface*/
                     /*if there are some neighbor on this interfac,this is not stub link,totochanged*/
                     p_if = ospf_if_lookup_by_network(p_process, ntohl(p_link->id));
    
                     /*set state mask,check full nbr*/
                     BIT_LST_SET(state_mask, OSPF_NS_FULL);
    
                     if ((NULL == p_if )
                         || (p_if->p_process != p_process)
                         || (p_if->p_area != p_area)
                         || (OSPF_IFS_DOWN == p_if->state)
                         ||((p_if->type == OSPF_IFT_BCAST) 
                            && (ospf_if_nbr_count_in_state(p_if, state_mask))))
                     {
                         /*if interface is bcast,and have full nbr on it,this is error*/
                         ospf_logx(ospf_debug_gr, "stub link error"); 
                         return TRUE;
                     }
                     break;
                     
               case OSPF_RTRLINK_PPP:
                    /*id is nbr id,data is self address,if neighbor not in full state,exit*/
			//		printf("%s %d  ****ospf_if_lookup*******\n", __FUNCTION__,__LINE__);
                    p_if = ospf_if_lookup(p_process, ntohl(p_link->data));             
			//		printf("%s %d  ****p_if 0x%x*******\n", __FUNCTION__,__LINE__,p_if);
					if (p_if != NULL)
					{
			//			printf("%s %d  ****addr 0x%x, ifnet_uint 0x%x*******\n", __FUNCTION__,__LINE__,p_if->addr, p_if->ifnet_uint);
					}
                    if (NULL == p_if)
                    {
                        ospf_logx(ospf_debug_gr, "ppp interface not exist"); 
                        return TRUE;
                    }
       
                    /*nbr's router lsa must exist,and pointer to self*/
                    lshdr.type = OSPF_LS_ROUTER;
                    lshdr.id = p_link->id;
                    lshdr.adv_id = p_link->id;
                    p_nbr_lsa = ospf_lsa_lookup(p_area->ls_table[lshdr.type], &lshdr);
                    if (NULL == p_nbr_lsa)
                    {
                        ospf_logx(ospf_debug_gr, "ppp nbr's router lsa not exist"); 
                        return TRUE;
                    }
                    /*check back link to self,TBI*/
                    break;
                    
               case OSPF_RTRLINK_VLINK:
                    /*id is nbr id,data is self address in transit area*/                        
			//		printf("%s %d  ****ospf_if_lookup*******\n", __FUNCTION__,__LINE__);
                    p_if = ospf_if_lookup(p_process, ntohl(p_link->data));
			//		printf("%s %d  ****p_if 0x%x*******\n", __FUNCTION__,__LINE__,p_if);
					if (p_if != NULL)
					{
			//			printf("%s %d  ****addr 0x%x, ifnet_uint 0x%x*******\n", __FUNCTION__,__LINE__,p_if->addr, p_if->ifnet_uint);
					}
                    if (NULL == p_if)
                    {
                       /*transit area if not exist, finish restart*/
                        ospf_logx(ospf_debug_gr, "transit area interface not exit"); 
                        return TRUE;
                    }
                    p_vif = ospf_vif_lookup(p_process, p_if->p_area->id, ntohl(p_link->id));
                    if (NULL == p_vif)
                    {
                        /*vlink not exist,finish restart*/
                        ospf_logx(ospf_debug_gr, "vif not exit"); 
                        return TRUE;
                    }
                    /*nbr's router lsa must exist,and pointer to self.TBI*/
                    break;
                    
               case OSPF_RTRLINK_TRANSIT:
                    /*id is dr address, data is self address*/
				//	printf("%s %d  ****ospf_if_lookup*******\n", __FUNCTION__,__LINE__);
                    p_if = ospf_if_lookup(p_process, ntohl(p_link->data));
			//	printf("%s %d  ****p_if 0x%x*******\n", __FUNCTION__,__LINE__,p_if);
				if (p_if != NULL)
				{
			//		printf("%s %d  ****addr 0x%x, ifnet_uint 0x%x*******\n", __FUNCTION__,__LINE__,p_if->addr, p_if->ifnet_uint);
				}
                    if (NULL == p_if)
                    {
                        ospf_logx(ospf_debug_gr, "interface not exist");
                        return TRUE;
                    }
                    if (p_if->dr != ntohl(p_link->id))
                    {
                        ospf_logx(ospf_debug_gr, "interface dr changed");
                        return TRUE;
                    }
                    /*network lsa originated by dr must exist*/
                    lshdr.type = OSPF_LS_NETWORK;
                    lshdr.id = p_link->id;
                    lshdr.adv_id = 0;
                    p_netlsa = ospf_lsa_lookup(p_area->ls_table[lshdr.type], &lshdr);
                    if (NULL == p_netlsa)
                    {
                        ospf_logx(ospf_debug_gr, "network lsa not exist"); 
                        return TRUE;
                    }
                    /*scan for all attached routers,self must exist,and all router lsa must exist*/
                    p_network = (struct ospf_network_lsa *)p_netlsa->lshdr;
                    for_each_network_link(p_network, p_netlink)
                    {
                        if (*p_netlink == htonl(p_process->router_id))
                        {
                            self_appear = TRUE;
                        }
                        else
                        {
                            lshdr.type = OSPF_LS_ROUTER;
                            lshdr.id = *p_netlink;
                            lshdr.adv_id = *p_netlink;
                            p_nbr_lsa = ospf_lsa_lookup(p_area->ls_table[lshdr.type], &lshdr); 
                            if (NULL == p_nbr_lsa)
                            {
                                ospf_logx(ospf_debug_gr, "nbr router lsa not exist"); 
                                return TRUE;
                            }
                            /*check back link to self,TBI*/
                        }
                    }
                    /*self not appear*/
                    if (FALSE == self_appear)
                    {
                       ospf_logx(ospf_debug_gr, "self not appear in network lsa"); 
                       return TRUE;
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }
    /*topo not changed*/
    return FALSE;
}

/*decide if restart completed sucessfully:
  all nbr are full established.called when a nbr enter full state*/
u_int 
ospf_restart_completed(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_nextarea = NULL;
    struct ospf_network_lsa *p_network = NULL;
    struct ospf_router_lsa *p_router = NULL;
    struct ospf_router_link *p_link = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_netlsa = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_vif = NULL;
    struct ospf_lshdr lshdr;
    u_int *p_netlink = NULL;
    u_int lsa_full_count = 0 ;
    u_int8 state_mask[4] = {0};
 
    BIT_LST_SET(state_mask, OSPF_NS_FULL);
    
    for_each_ospf_area(p_process, p_area, p_nextarea)
    {
        /*no full nbr exist,can not decide anything*/
        if (FALSE == ospf_area_full_nbr_exist(p_area))
        {
            continue;
        }
        /*self originated router lsa must exist*/
        lshdr.type = OSPF_LS_ROUTER;
        lshdr.id = htonl(p_process->router_id);
        lshdr.adv_id = htonl(p_process->router_id);
        /*self router lsa not exist,finish restart*/
        p_lsa = ospf_lsa_lookup(p_area->ls_table[lshdr.type], &lshdr);
        if (NULL == p_lsa)
        {
            return FALSE;
        }
        /*scan all router link in router lsa*/
        p_router = (struct ospf_router_lsa *)p_lsa->lshdr;
        for_each_router_link(p_router, p_link)
        {
            switch (p_link->type) {
                case OSPF_RTRLINK_STUB:
                     p_if = ospf_if_lookup_by_network(p_process, ntohl(p_link->id));
                     if (NULL == p_if)
                     {
                        return FALSE;
                     }
                     if (ospf_if_nbr_count_in_state(p_if, state_mask) != 0)
                     {
                        return FALSE;
                     }
                     break;
                     
                case OSPF_RTRLINK_PPP:
			//		printf("%s %d  ****ospf_if_lookup*******\n", __FUNCTION__,__LINE__);
                     p_if = ospf_if_lookup(p_process, ntohl(p_link->data));
			//		printf("%s %d  ****p_if 0x%x*******\n", __FUNCTION__,__LINE__,p_if);
					if (p_if != NULL)
					{
				//		printf("%s %d  ****addr 0x%x, ifnet_uint 0x%x*******\n", __FUNCTION__,__LINE__,p_if->addr, p_if->ifnet_uint);
					}
                     if (NULL == p_if)
                     {
                        return FALSE;
                     }
                     if (ospf_if_nbr_count_in_state(p_if, state_mask) != 1)
                     {
                        return FALSE;
                     }
                     break;
                     
                case OSPF_RTRLINK_VLINK:
				//	printf("%s %d  ****ospf_if_lookup*******\n", __FUNCTION__,__LINE__);
                     p_if = ospf_if_lookup(p_process, ntohl(p_link->data));
				//	printf("%s %d  ****p_if 0x%x*******\n", __FUNCTION__,__LINE__,p_if);
					if (p_if != NULL)
					{
				//		printf("%s %d  ****addr 0x%x, ifnet_uint 0x%x*******\n", __FUNCTION__,__LINE__,p_if->addr, p_if->ifnet_uint);
					}
                     if (NULL == p_if)
                     {
                        return FALSE;
                     }
                     p_vif = ospf_vif_lookup(p_process, p_if->p_area->id, ntohl(p_link->id));
                     if (NULL == p_vif)
                     {
                        return FALSE;
                     }
                     if (ospf_if_nbr_count_in_state(p_vif, state_mask) != 1)
                     {
                        return FALSE;
                     }
                     break;
                     
                case OSPF_RTRLINK_TRANSIT:
			//		printf("%s %d  ****ospf_if_lookup*******\n", __FUNCTION__,__LINE__);
                     p_if = ospf_if_lookup(p_process, ntohl(p_link->data));
			//		printf("%s %d  ****p_if 0x%x*******\n", __FUNCTION__,__LINE__,p_if);
					if (p_if != NULL)
					{
			//			printf("%s %d  ****addr 0x%x, ifnet_uint 0x%x*******\n", __FUNCTION__,__LINE__,p_if->addr, p_if->ifnet_uint);
					}
                     if (NULL == p_if)
                     {
                        return FALSE;
                     }
                     /*network lsa originated by dr must exist*/
                     lshdr.type = OSPF_LS_NETWORK;
                     lshdr.id = p_link->id;
                     lshdr.adv_id = 0;
                     p_netlsa = ospf_lsa_lookup(p_area->ls_table[lshdr.type], &lshdr);
                     if (NULL == p_netlsa)
                     {
                        return FALSE;
                     }
                     lsa_full_count = 0;
                     /*scan for all attached routers,self must exist,and all router lsa must exist*/
                     p_network = (struct ospf_network_lsa *)p_netlsa->lshdr;
                     for_each_network_link(p_network, p_netlink)
                     {
                         if (*p_netlink != htonl(p_process->router_id))
                         {
                             lsa_full_count++;
                         }
                     }
                     if (ospf_if_nbr_count_in_state(p_if, state_mask) != lsa_full_count)
                     {
                        return FALSE;
                     }
                     break;
                     
                 default:
                     break;
            }
        }
    }
    ospf_logx(ospf_debug_gr, "all nbr established, restart finished");
    return TRUE;
}

/*
Originate graceful restart lsa for special lsa and flood it on this interface.
Called when interface is created and before sending the first hello packet.
*/
void 
ospf_restart_lsa_originate(struct ospf_if *p_if)
{
    u_int8 buf[OSPF_MAX_TXBUF]; 
    struct ospf_opaque_lsa *p_opaque;
    struct ospf_process *p_process = p_if->p_process;
    u_int8 *p_data = NULL;
    u_int restart_time = p_process->restart_period;
    u_int if_address = p_if->addr;
    u_int8 ifname[32];

    ospf_logx(ospf_debug_gr, "originate graceful lsa for if %s", ospf_inet_ntoa(ifname, p_if->addr));
        
    /*if interface state is Down ,do not originate lsa*/
    if (OSPF_IFS_DOWN == p_if->state)
    {
        ospf_logx(ospf_debug_gr, "do not originate for down if\r\n");
        return;
    }
    p_opaque = (struct ospf_opaque_lsa *)buf;
    memset(p_opaque, 0, sizeof(struct ospf_opaque_lsa));
    
    ospf_init_lshdr(&p_opaque->h, OSPF_LS_TYPE_9, OSPF_GR_LSID, p_process->router_id);
    ospf_lsa_option_build(p_if->p_area, &p_opaque->h);
    p_opaque->h.len = OSPF_LSA_HLEN;

    p_data = p_opaque->data;
    
    /*restart time tlv*/
    ospf_fill_16(p_data, OSPF_GR_TLV_TYPE_TIME);
    p_data += 2;
    
    ospf_fill_16(p_data, OSPF_GR_TLV_LEN_TIME);
    p_data += 2;  
    
    /*time value*/
    ospf_fill_32(p_data, restart_time);
    p_data += 4;
    
    /*restart reason*/
    ospf_fill_16(p_data, OSPF_GR_TLV_TYPE_REASON);
    p_data += 2;  
    
    ospf_fill_16(p_data, OSPF_GR_TLV_LEN_REASON);
    p_data += 2;  
        
    /*reason has one byte,but has three bytes padding*/
    ospf_fill_32(p_data, 0);
    *p_data = p_process->restart_reason;
     p_data += 4;
    
    /*interface address*/
    ospf_fill_16(p_data, OSPF_GR_TLV_TYPE_ADDR);
    p_data += 2;  
    
    ospf_fill_16(p_data, OSPF_GR_TLV_LEN_ADDR);
    p_data += 2;  
    
    ospf_fill_32(p_data, if_address);
    p_data += 4;
        
    /*add padding*/
    p_opaque->h.len += ((u_long)p_data - (u_long)p_opaque->data);
    p_opaque->h.len += (4 - (p_opaque->h.len % 4))%4;
    p_opaque->h.len = htons(p_opaque->h.len);

    /*install it*/
    ospf_local_lsa_install(&p_if->opaque_lstable, &p_opaque->h);
    return;
}

 
 /*get gr period form link state database buffer.0 means no gr period*/
u_int 
ospf_restart_period_get(struct ospf_lshdr *p_lshdr)
{
    struct ospf_opaque_lsa *p_lsa = (struct ospf_opaque_lsa *)p_lshdr;
    u_int8 *p_buf = NULL; 
    u_short len = 0;
    u_short tlv_type;
    u_short tlv_len;
    u_short pad_len;
    u_int period = 0;
   
    /*parse database body to get GR period*/   
    p_buf = (u_int8 *)p_lsa->data;      
    
    len = ntohs(p_lsa->h.len);
    len -= 20;
    
    /*parse TLV*/   
    while (len > 0)
    {
        memcpy(&tlv_type, p_buf, 2);
        tlv_type = ntohs(tlv_type);
        
        memcpy(&tlv_len, p_buf + 2, 2);
        tlv_len = ntohs(tlv_len);
        
        if ((tlv_len % 4) == 0)
        {
            pad_len = tlv_len;
        }
        else
        {
            pad_len = tlv_len + 4 - (tlv_len % 4); 
        }
        
        switch (tlv_type){
            case OSPF_GR_TLV_TYPE_TIME:
                 memcpy(&period, p_buf + 4, 4);
                 period = ntohl(period);
                 break;
                
            case OSPF_GR_TLV_TYPE_REASON:
            case OSPF_GR_TLV_TYPE_ADDR:
            default:
                 break;
        }
        
        if (len <= (pad_len + 4))
        {
            break;
        }
        
        len -= (pad_len + 4);
        p_buf += (pad_len + 4);
    }
    return period;
}
 
 /*
 Call when processing a more recent received grace lsa.
 1 if normal gr lsa receieved,try to enter hepler mode
 2 if maxaged gr lsa receieved,try to leave hepler mode
 1 if some normal lsa changed,exit any helper mode
 2 if receive gr lsa from neighbor ,try to enter helper mode
 */
void 
ospf_restart_lsa_recv(struct ospf_lsa *p_lsa) 
{
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    struct ospf_if *p_if = p_lsa->p_lstable->p_if;
    struct ospf_process *p_process = p_if->p_process;
    u_int period = 0;
    
    ospf_logx(ospf_debug_gr, "recv graceful restart lsa");

    /*if self is in restarting ,do nothing*/
    if (p_process->in_restart)
    {
        ospf_logx(ospf_debug_gr, "self is in restarting");
        return;
    }
    
    /*if current some non-full and not-gr neighbor exist,do nothing*/
    for_each_node(&p_process->nbr_table, p_nbr, p_next_nbr)
    {
        if ((!p_nbr->in_restart) && (OSPF_NS_FULL != p_nbr->state ))
        {
            ospf_logx(ospf_debug_gr, "some non full neighbor exist,ignore this lsa");
            return;
        }      
    }
    /*if retransmit list is not null,we must know if the update is period refresh or
       lsa changed.How doed we get this information ?*/
    /*decide restarted neighbor*/
    for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
    {
        if (p_nbr->id == ntohl(p_lsa->lshdr->adv_id))
        {
            break;
        }
    }

    if (NULL == p_nbr)
    {
        ospf_logx(ospf_debug_gr, "no neighbor found");
        return;
    }

    /*if input gr lsa has max age,exit helper mode for this neighbor*/
    if (OSPF_MAX_LSAGE <= ntohs(p_lsa->lshdr->age))
    {
        ospf_logx(ospf_debug_gr, "lsa has max age,leave helper for this neighbor");
             
        ospf_restart_helper_finish(p_nbr, OSPF_RESTART_COMPLETED);
        return;
    }

   /*try to enter hepler mode for neighbor*/
   /*if self is in GR state,do nothing*/
    if (p_nbr->in_restart)
    {
        ospf_logx(ospf_debug_gr, "local node is in restarting,ignore this lsa");
        return;
    }
    /*if gr-period alraedy expired,do nothing*/
    period = ospf_restart_period_get(p_lsa->lshdr);             
    if (period <= ntohs(p_lsa->lshdr->age))
    {
        ospf_logx(ospf_debug_gr, "restart period expired in lsa,ignore this lsa");                 
        return;
    }
   
    /*if request or retransmit is not null,do nothing*/
    if ((0 != p_nbr->req_count) || (0 != p_nbr->rxmt_count))
    {
        ospf_logx(ospf_debug_gr, "neighbor request list is not null,ignore this lsa,req_count:%d,rxmt_count:%d",p_nbr->req_count,p_nbr->rxmt_count);
        return;
    }
    ospf_logx(ospf_debug_gr, "neighbor enter restarting state,i am helper"); 
            
    /*enter helper mode for this neighbor*/
    p_nbr->in_restart = TRUE;
   
  
    //p_nbr->state = OSPF_NS_DOWN;
   
    /* md5*/
    memset(p_nbr->auth_seqnum, 0, sizeof(p_nbr->auth_seqnum));
    
    /*restart timer*/
    ospf_stimer_start(&p_nbr->restart_timer, period);
    ospf_trap_helperstatus(p_nbr);
    return;
}

/*decide if a changed lsa will affect current helper mode:
    scan for all interfaces in instance, if lsa will be flooded on
    this interface,and there are restarting neighbor on this 
    interface,I will stop all helper mode.
*/
u_int 
ospf_restart_helper_affected_by_lsa(struct ospf_lsa *p_lsa)
{
    struct ospf_if *p_if;
    struct ospf_if *p_next_if = NULL;
    struct ospf_nbr *p_nbr;
    struct ospf_nbr *p_next_nbr = NULL;
    
    for_each_ospf_if(p_lsa->p_lstable->p_process, p_if, p_next_if)
    {
        if (ospf_lsa_flood_verify(p_lsa, p_if, NULL, NULL) == FALSE)
        {
             continue;
        }
        for_each_ospf_nbr(p_if, p_nbr, p_next_nbr) 
        {
            if (p_nbr->in_restart)
            {                 
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*exit current helper mode for all of neighbor*/
void 
ospf_restart_helper_finish_all(
               struct ospf_process *p_process, 
               u_int reason)
{
    struct ospf_nbr *p_nbr;
    struct ospf_nbr *p_next_nbr = NULL;
   
    for_each_node(&p_process->nbr_table, p_nbr, p_next_nbr) 
    {
        if (p_nbr->in_restart)
        {                 
            ospf_restart_helper_finish(p_nbr, reason);
        }
    }    
    return;
}

/*exit current helper mode for a special neighbor*/
void 
ospf_restart_helper_finish(
                   struct ospf_nbr *p_nbr,  
                   u_int reason)
{
    struct ospf_if *p_if = p_nbr->p_if;
    u_int8 nbr[32];
    u_int8 str[OSPF_RESTART_TOPLOGY_CHANGED+1][20] = {" ","none","inprogress","complete","timeout", "topo changed"};
   
    ospf_logx(ospf_debug_gr, "stop helper for restarter %s,reason %s", ospf_inet_ntoa(nbr, p_nbr->addr),str[reason]);
   
    /*Clear GR flag*/
    p_nbr->in_restart = FALSE;
    p_nbr->restart_exitreason = reason;
    if (reason == OSPF_RESTART_TIMEOUT)
    {
     //   route_del_by_type(ZEBRA_ROUTE_OSPF);
    }
    ospf_timer_stop(&p_nbr->restart_timer);
   
    /*elect DR for each interface*/
    ospf_ism_wait_timer(p_if);
    
    /*regenerate router lsa for each area*/
    ospf_lstwalkup(&p_if->p_process->area_table, ospf_router_lsa_originate);
   
    ospf_summary_lsa_originate_for_area(p_if->p_area, !p_if->p_process->abr) ;
   
   
    if (p_if->dr == p_if->addr)
    {
        ospf_network_lsa_originate(p_if);
    }
    ospf_stimer_start(&p_if->p_process->import_timer, OSPF_IMPORT_INTERVAL);
    p_if->p_process->import_update = TRUE;
   
    ospf_lsa_force_refresh(p_if->p_process);
    
    ospf_trap_helperstatus(p_nbr);
    return;
}

void 
ospf_restart_timeout(struct ospf_process *p_process)
{
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    ospf_set_context(p_process);
    ospf_restart_finish(p_process, OSPF_RESTART_TIMEOUT);
    return;
}

void 
ospf_restart_helper_timeout(struct ospf_nbr *p_nbr)
{
    ospf_restart_helper_finish(p_nbr, OSPF_RESTART_TIMEOUT);
    return;
}

void
ospf_restart_wait_timeout(struct ospf_process *p_process)
{
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    /*check if all restart lsa acked,if so,enter real restart operation*/
    /*grace_lsa  hace rcvd ack,grace start begin */
    if (TRUE == ospf_all_lsa_acked(p_process))
    {
        ospf_restart_start(p_process);    
    }/*check if restart can be started in next loop*/
    else
    {
        ospf_timer_start(&p_process->restart_wait_timer, 10);
    }
    return;
}
