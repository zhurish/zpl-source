/* ospf_main.c - ospf task control and init functions*/

#include "ospf.h"
#include "ospf_nm.h"

/* globals */
u_int8 *pGetIpBuf = NULL;

tOSPF_LSA_MANAGE stOsLsa;
tOSPF_LSA_MANAGE stOspfLsaManage;

struct ospf_global ospf ;


int ospf_route_policy_apply(u_int uiIndex, void *pVlaue)
{
	return OK;
}

int ospf_route_policy_set(u_int uiIndex, u_int uiVlaue)
{
	return OK;
}

/*init called when system up*/
void  ospf_init(
  u_int max_process,
  u_int max_lsa,
  u_int max_route,
  u_int max_if,
  u_int max_area,
  u_int max_nbr, void *m)
{
    int up = FALSE;
	int iRet = 0;
	int i = 0;

    if (up)
    {
        return;
    }
    up = TRUE;
    
    memset(&ospf, 0, sizeof(ospf));
 
 	memset(&stOsLsa, 0, sizeof(stOsLsa));
 	ospf.master = m;
    /*allocate 64k rxbuf*/
    ospf.p_rxbuf = XMALLOC(MTYPE_OSPF_PACKET, OSPF_BIGPKT_BUF);
	pGetIpBuf   = XMALLOC(MTYPE_OSPF_PACKET, 2048);
	ospf.debugFd = -1;
    ospf.malloc = (void*)zmalloc;
    ospf.free = zfree;
    /*semaphore are created once,do not free  even ospf is down*/
    ospf_create_sem ();
    /*create two sockets,do not release them */
    for(i = 0; i < OSPF_L3VPN_NUM_MAX + 1; i++)
    {
        ospf.sock[i] = -1;
    }
    ospf.sock[0] = ospf_socket_init(0); 
	ospf.work_mode = OSPF_MODE_NORMAL;

	/*end*/
	/*Policy*/
	ospf.route_policy_set = ospf_route_policy_set;
 //   ospf.rtsock = ospf_rtsocket_init(); 
//	ospf.debug = 0xffffffff;
// 	ospf.process_debug = 0xffffffff;
    /*adj change log is opened default*/
//    ospf_global_debug_set(OSPF_DBG_NBRCHANGE, 1);
 
    /*global table init here*/       
    ospf_lstinit(&ospf.process_table, ospf_process_lookup_cmp);
 
    ospf_lstinit2(&ospf.real_if_table, ospf_real_if_lookup_cmp, mbroffset(struct ospf_if, global_node));
 
    ospf_lstinit(&ospf.syn_control_table, ospf_syn_control_lookup_cmp);
 
    ospf_lstinit2(&ospf.nm.area_table, ospf_area_nm_cmp, mbroffset(struct ospf_area, nm_node));
    ospf_lstinit2(&ospf.nm.range_table, ospf_range_nm_cmp, mbroffset(struct ospf_range, nm_node));
    ospf_lstinit2(&ospf.nm.if_table, ospf_if_nm_cmp, mbroffset(struct ospf_if, nm_node));
    ospf_lstinit2(&ospf.nm.vif_table, ospf_vif_nm_cmp, mbroffset(struct ospf_if, nm_node));
    ospf_lstinit2(&ospf.nm.shamlink_table, ospf_shamlink_nm_cmp, mbroffset(struct ospf_if, nm_node));       
    ospf_lstinit2(&ospf.nm.nbr_table, ospf_nbr_nm_cmp, mbroffset(struct ospf_nbr, nm_node));
    ospf_lstinit2(&ospf.nm.area_lsa_table, ospf_area_lstable_nm_cmp, mbroffset(struct ospf_lstable, nm_node));       
    ospf_lstinit2(&ospf.nm.as_lsa_table, ospf_as_lstable_nm_cmp, mbroffset(struct ospf_lstable, nm_node));       
    ospf_lstinit2(&ospf.nm.if_lsa_table, ospf_if_lstable_nm_cmp, mbroffset(struct ospf_lstable, nm_node)); 
    ospf_lstinit2(&ospf.nm.vif_lsa_table, ospf_vif_lstable_nm_cmp, mbroffset(struct ospf_lstable, nm_node)); 
    ospf_lstinit2(&ospf.nm.network_table, ospf_network_nm_cmp, mbroffset(struct ospf_network, nm_node));        
    ospf_lstinit2(&ospf.nm.redis_range_table, ospf_redistribute_range_nm_cmp, mbroffset(struct ospf_redis_range, nm_node));               
    ospf_lstinit2(&ospf.nm.redistribute_table, ospf_redistribute_nm_cmp, mbroffset(struct ospf_redistribute, nm_node));               
    ospf_lstinit2(&ospf.nm.redistribute_policy_table, ospf_redis_policy_nm_cmp, mbroffset(struct ospf_policy, nm_node));                      
    ospf_lstinit2(&ospf.nm.filter_policy_table, ospf_filter_policy_nm_cmp, mbroffset(struct ospf_policy, nm_node));
    ospf_lstinit2(&ospf.lsatable_table, ospf_global_lstable_lookup_cmp, mbroffset(struct ospf_lstable, global_node));

    ospf_lstinit2(&ospf.nm.asbr_range_table, ospf_asbr_range_lookup_cmp, mbroffset(struct ospf_asbr_range, nm_node));
    
    ospf_lstinit(&ospf.log_table.nbr_table, NULL);
    ospf_lstinit(&ospf.log_table.lsa_table, NULL);
    ospf_lstinit(&ospf.log_table.spf_table, NULL);
#ifdef HAVE_BFD
    ospf_lstinit(&ospf.log_table.bfd_table, NULL);
#endif
 
    ospf_timer_table_init();
#ifdef OSPF_MASTER_SLAVE_SYNC
    ospf_timer_init(&ospf.workmode_timer, &ospf, ospf_get_workmode, NULL);
 
    //ospf_timer_init(&ospf.sync_check_timer, NULL, ospf_sync_check_timeout, NULL);
#endif
   ospf_timer_init(&ospf.sync_check_timer, NULL, ospf_sync_check_timeout, NULL);

   // ospf_timer_init(&ospf.rx_timer, NULL, ospf_recv_thread, NULL);
    
    ospf_timer_init(&ospf.nbr_state_update_timer, NULL, ospf_nbr_state_update_timeout, NULL);
    /*init global memory resource, only init once*/     
    ospf_set_init_num(max_process, max_lsa, max_route, max_if, max_area, max_nbr);
 
    /*start workmode check timer*/ 
#ifdef OSPF_MASTER_SLAVE_SYNC
    ospf_stimer_start(&ospf.workmode_timer, OSPF_WORKMODE_INTERVAL);
#endif
	ospf_logx(ospf_debug,"######%d\r\n",__LINE__);

    return;
}


void ospf_global_init(
                  u_int max_process,
                  u_int max_lsa,
                  u_int max_route,
                  u_int max_if,
                  u_int max_area,
                  u_int max_nbr,
                  void *p_fun1,
                  void *p_fun2)
{
    ospf_init(max_process, max_lsa, max_route, max_if, max_area, max_nbr, NULL);
    ospf.route_policy_apply = p_fun1;
    ospf.route_policy_set = p_fun2;
    ospf.malloc = (void *)zmalloc;
    ospf.free = zfree;
    return;
}


void ospf_main_init(void *m)
{
    //zhurish swtich_sdk_set(NULL, EN_SDK_MODULE_ACL|EN_SDK_ACL_PORT_OSPF_TO_CPU_ADD, NULL);
	//ospf.master = m;
	//ospf_global_init(0, 0, 0, 0, 0, 0, ospf_route_policy_apply, ospf_route_policy_set);
    ospf_init(0, 0, 0, 0, 0, 0, m);
    ospf.route_policy_apply = ospf_route_policy_apply;
    ospf.route_policy_set = ospf_route_policy_set;
    ospf.malloc = (void *)zmalloc;
    ospf.free = zfree;
#ifdef OSPF_DCN
	ospf_dcn_global_init();
#endif
}





/*instance operation*/
void 
ospf_old_routerid_timeout(struct ospf_process *p_process)
{
    /*reset old router id,it it not used when this timer expired*/
    p_process->old_routerid = 0;
    return;
}

/*compare instance,id is key*/
int 
ospf_process_lookup_cmp(
             struct ospf_process *p1, 
             struct ospf_process *p2)
{
    OSPF_KEY_CMP(p1, p2, process_id);
    return 0;
}

/*lookup instance*/
struct ospf_process*
ospf_process_lookup(
              struct ospf_global *p_master,
              u_int id) 
{
    struct ospf_process search;
    struct ospf_process *p_process = NULL;
    search.process_id = id;
    p_process = ospf_lstlookup(&(p_master->process_table), &search); 
    ospf_set_context(p_process);
    return p_process; 
}

struct ospf_process*
ospf_process_lookup_by_id(u_int id)
{
    struct ospf_process *p_process = NULL;

    p_process = ospf_process_lookup(&ospf, id);

    return p_process;
}

/*add process*/
struct ospf_process *
ospf_process_add(
                struct ospf_global *p_master,
                u_int id)
{
    struct ospf_process *p_process;
    
    p_process = ospf_process_lookup(p_master, id);
    if (NULL != p_process)
    {
        return p_process;
    }
    p_process = ospf_malloc2(OSPF_MINSTANCE);
    if (NULL != p_process)
    {
        ospf_process_up(p_process);
        p_process->max_ecmp = OSPF_ECMP_COUNT;
        p_process->process_id = id;
        p_process->p_master = p_master;
        ospf_lstadd(&p_master->process_table, p_process);
        
        ospf_lstadd(&ospf.nm.as_lsa_table, &p_process->t5_lstable);    
        ospf_lstadd(&ospf.nm.as_lsa_table, &p_process->t11_lstable);
        ospf_lstadd(&ospf.lsatable_table, &p_process->t5_lstable);
        ospf_lstadd(&ospf.lsatable_table, &p_process->t11_lstable);
        ospf_sync_event_set(p_process->syn_flag);
    }

    /*create default sync control*/
    if (1 == ospf_lstcnt(&ospf.process_table))
    {
        ospf_syn_control_create(0);
    }
    return p_process;
}

/*delete instance*/
void 
ospf_process_delete(struct ospf_process *p_process)
{
    struct ospf_nexthop *p_nexthop = NULL;
    struct ospf_nexthop *p_next_nexthop = NULL;
    /*send sync msg now*/
    if (OSPF_MODE_MASTER == p_process->p_master->work_mode)
    {
        ospf_syn_process_send(p_process, FALSE, NULL);
    }

    /*delete all nexthop*/
    for_each_node(&p_process->nexthop_table, p_nexthop, p_next_nexthop)
    {
        ospf_lstdel(&p_process->nexthop_table, p_nexthop);
        ospf_mfree(p_nexthop, OSPF_MNEXTHOP);
    }
    /*reinit router id to 0*/        
    p_process->router_id = 0;

    ospf_flush_filter_policy(p_process);
    ospf_flush_redistribute_policy(p_process);
    ospf_flush_filtered_route(p_process);
    ospf_flush_import_route(p_process);
    ospf_flush_redistribute_range(p_process);

    /*free route msg*/
    if (NULL != p_process->p_rtmsg)
    {
        ospf_mfree(p_process->p_rtmsg, OSPF_MPACKET);
        p_process->p_rtmsg = NULL;
    }

    /*clear type 5 and 11 lsa table*/
    ospf_lsa_table_shutdown(&p_process->t5_lstable);

    ospf_lstdel(&ospf.nm.as_lsa_table, &p_process->t5_lstable);
    ospf_lstdel(&ospf.lsatable_table, &p_process->t5_lstable);

    ospf_lsa_table_shutdown(&p_process->t11_lstable);

    ospf_lstdel(&ospf.nm.as_lsa_table, &p_process->t11_lstable);
    ospf_lstdel(&ospf.lsatable_table, &p_process->t11_lstable);

    /*clear all log table*/
    ospf_spf_log_clear(p_process);
    ospf_nbr_log_clear(p_process);
    ospf_lsa_log_clear(p_process);
#ifdef HAVE_BFD
    ospf_bfd_log_clear(p_process);
#endif
    
   
    /*delete fragment list*/
    ospf_lstwalkup(&p_process->fragment_table, ospf_lsa_fragment_delete);

    ospf_lstwalkup(&p_process->te_tunnel_table, ospf_te_tunnel_delete);

    /*process to be deleted,clear running process*/
    //ospf_set_context(NULL);

    /*stop all timers*/
    ospf_timer_stop(&p_process->import_timer); 
    ospf_timer_stop(&p_process->overlay_timer); 
    ospf_timer_stop(&p_process->spf_timer); 
    ospf_timer_stop(&p_process->db_overload_timer); 
    ospf_timer_stop(&p_process->restart_timer);
    ospf_timer_stop(&p_process->routeid_timer); 

    ospf_timer_stop(&p_process->lsa_aging_timer);
    ospf_timer_stop(&p_process->ipupdate_timer);          
    ospf_timer_stop(&p_process->delete_timer);
    ospf_timer_stop(&p_process->id_reset_timer);
    ospf_timer_stop(&p_process->restart_wait_timer);
    ospf_timer_stop(&p_process->range_update_timer);    
    ospf_timer_stop(&p_process->redis_range_update_timer);    
#ifdef OSPF_FRR
    ospf_timer_stop(&p_process->frr_timer);  
    ospf_timer_stop(&p_process->backup_ipupdate_timer);  
       
#endif
    
    ospf_timer_stop(&p_process->fast_spf_timer);

    ospf_timer_stop(&p_process->te_tunnel_timer);
    
    ospf_timer_stop(&p_process->lsa_check_timer);

    ospf_timer_stop(&p_process->ratelimit_timer);
    ospf_timer_stop(&p_process->rxmtlimit_timer);
#ifdef OSPF_DCN
    //ospf_timer_stop(&p_process->dcn_rtm_tx_timer);    
#endif    
    /*remove it from global table ,and free it*/
    ospf_lstdel_free(&p_process->p_master->process_table, p_process, OSPF_MINSTANCE);
    return;
}

/*scheduled process delete waiting timer:must wait for system update*/
void
ospf_process_delete_event_handler(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next = NULL;
    int i = 0;

    p_process->enable = FALSE;

    /*delete all network configure*/
    ospf_flush_network(p_process); 
    
    /*redistribute control list free*/
    ospf_flush_redistribute_list(p_process);  
          
    /*all lsa in this instance*/
    ospf_lsa_flush_all(p_process);
    
    /*delete interface belong to instance*/ 
    ospf_lstwalkup(&p_process->if_table, ospf_if_delete);
#ifdef OSPF_FRR
    /*last frr update not finished,wait for a period using timer*/
    if (p_process->backup_wait_export || p_process->send_routemsg)
    {
        ospf_timer_start(&p_process->delete_timer, 2);
        return;
    }
    /*force to calculate backup route*/
    ospf_frr_timer_expired(p_process);

    /*current frr update not finished,wait for a period using timer*/
    if (p_process->backup_wait_export || p_process->send_routemsg)
    {
        ospf_timer_start(&p_process->delete_timer, 2);
        return;
    }
#endif
   /*try to delete all the areas */ 
    vty_out_to_all_terminal("This process area is deleted...");
    for_each_ospf_area(p_process, p_area, p_next)
    {                
        ospf_area_delete_event_handler(p_area);
    }

    /*if area not flushed fully,wait for next loop*/
    if (ospf_lstcnt(&p_process->area_table))
    {
        ospf_timer_start(&p_process->delete_timer, 1);
        return;
    }

    /*all area deleted,and route update finished,delete this process*/
    ospf_process_delete(p_process);
    /*check if routesocket option changed*/  
    //zhurish ospf_rtsock_option_update(ospf.rtsock);  
    
    /*if no active instance,close all resource*/
    if (0 == ospf_lstcnt(&ospf.process_table))
    {
        /*delete all sync control nodes*/
        ospf_lstwalkup(&ospf.syn_control_table, ospf_syn_control_delete);

        /*free memory to system*/
        ospf_destory_memory();

        for(i = 1; i < OSPF_L3VPN_NUM_MAX + 1; i++)
        {
            if(ospf.sock[i] > 0)
            {
                ospf_close_sock(ospf.sock[i]);
            }
        }

        //zhurish ospf_set_mcast_to_cpu(0);
    }
    vty_out_to_all_terminal("This process has been deleted.");
    return;
}

/*将lstable按照以下优先级由低向高排序: 
接口上的9类lstable，
区域上的lstable，按照区域id由小到大排序，相同区域id的按照lsa的type来排序
process上的5类，11类lstable*/
int
ospf_lstable_lookup_cmp(
              struct ospf_lstable *p1,
              struct ospf_lstable *p2)
{
    if (p1->p_if && !p2->p_if)
    {
        return -1;
    }
    else if (!p1->p_if && p2->p_if)
    {
        return 1;
    }
    else if (p1->p_if && p2->p_if)
    {
        return ospf_if_lookup_cmp(p1->p_if, p2->p_if);
    }   
    
    if (p1->p_area && !p2->p_area)
    {
        return -1;
    }
    else if (!p1->p_area && p2->p_area)
    {
        return 1;
    }
    else if (p1->p_area && p2->p_area)
    {
        OSPF_KEY_CMP(p1, p2, p_area->id);
    }
    OSPF_KEY_CMP(p1, p2, type);                
    return 0;
}

int
ospf_global_lstable_lookup_cmp(
             struct ospf_lstable *p1,
             struct ospf_lstable *p2)
{
    OSPF_KEY_CMP(p1, p2, p_process->process_id);  
    return ospf_lstable_lookup_cmp(p1, p2);
}

/*scheduled system update timer:used for large routing table*/
void 
ospf_ip_update_timer_expired(struct ospf_process *p_process)
{
    u_int error_count = ospf.stat.sys_add_error + ospf.stat.sys_delete_error + ospf.stat.sys_msg_error;

    ospf_set_context(p_process);
    
  //  ospf_logx(ospf_debug,"%s %d  *****p_process->wait_export %d******\n", __FUNCTION__,__LINE__,p_process->wait_export);
    /*try to update ip route now*/
    if (p_process->wait_export)
    {
        ospf_network_route_change_check(p_process);
    }

    /*send route msg*/
    if (p_process->send_routemsg)
    {
        p_process->send_routemsg = FALSE;
        //zhurish ospf_rtsock_route_msg_output(p_process);
    }  

    /*if update not finished,start timer again*/
    if (p_process->wait_export || p_process->send_routemsg)
    {
        /*if error count changed,use a longer timer*/
        if (error_count != (ospf.stat.sys_add_error + ospf.stat.sys_delete_error + ospf.stat.sys_msg_error))
        {
            ospf_timer_start(&p_process->ipupdate_timer, 3);
        }
        else
        {
            ospf_timer_start(&p_process->ipupdate_timer, 1);
        }
    }
    /*if msg send finished,free buf*/
    if ((FALSE == p_process->send_routemsg) && (p_process->p_rtmsg))
    {
        ospf_mfree(p_process->p_rtmsg, OSPF_MPACKET);
        p_process->p_rtmsg = NULL;
    }
    return;
}

/*limit lsa retransmit packet send rate timer*/
void 
ospf_lsa_rxmt_limit_timer(struct ospf_process *p_process)
{
    p_process->lsa_rxmt_limit = OSPF_UPDATE_RXMT_LIMIT;
    return;
}

void 
ospf_nhopw_init(struct ospf_nhop_weight *nhopw)
{
    int i = 0;
    for(i = 0; i < 64; i++)
    {
        nhopw[i].addr = 0;
        nhopw[i].flag = FALSE;
        nhopw[i].weight = 0;
    }
}

/*cmp func for asbr table*/
int 
ospf_asbr_range_lookup_cmp(
                 struct ospf_asbr_range *p1, 
                 struct ospf_asbr_range *p2)
{
    OSPF_KEY_CMP(p1, p2, dest); 
    OSPF_KEY_CMP(p1, p2, mask); 
    return 0;
}

void ospf_asbr_range_update_timeout(struct ospf_process *p_process)
{
    struct ospf_asbr_range *p_rangeT5 = NULL;
    struct ospf_asbr_range *p_nextT5 = NULL;
    struct ospf_area *p_area_nssa = NULL;
    struct ospf_range *p_rangeT7 = NULL;
    struct ospf_range *p_nextT7 = NULL;

    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }

    //p_area_nssa = ospf_nssa_area_lookup(p_process);
    
    /*check and update all ranges*/
    for_each_node(&ospf.nm.asbr_range_table, p_rangeT5, p_nextT5)
    {
        if (p_rangeT5->p_process != p_process)
        {
            continue;
        }  
        ospf_range_update_asbr_T5(p_rangeT5);
        ospf_asbr_nssa_range_update_verify(p_rangeT5);
    }
    
    return;
}
/*end*/

void 
ospf_process_up(struct ospf_process *p_process)
{
    memset(p_process, 0, sizeof(struct ospf_process));

    p_process->enable = TRUE;
    p_process->overflow_limit = OSPF_DEFAULT_ASE_LIMIT;
    p_process->overflow_period = OSPF_DEFAULT_OVERFLOW_INERVAL;
    p_process->spf_interval = OSPF_SPF_INTERVAL;
    p_process->min_mask = OSPF_HOST_MASK;
    p_process->max_mask = 0;
    p_process->mcast_support = FALSE;
    p_process->restart_status = OSPF_RESTART_NO_RESTARTING;
    p_process->restart_exitreason = OSPF_RESTART_NONE;
    p_process->stub_support = TRUE;
    p_process->stub_adv = FALSE;
    p_process->stub_router_holdtime = 0;
#ifdef OSPF_VPN
    p_process->vrid = OSPF_NO_VRID;
#else
    p_process->vrid = OSPF_NO_VRID/*OSPF_INVALID_VRID*/;
#endif
    p_process->reference_rate = OSPF_DEFAULT_REF_RATE;
    p_process->default_cost_type = OSPF_DEFAULT_COST_TYPE;
    p_process->default_cost = OSPF_DEFAULT_COST;
/*add by xujw*/
#ifdef HAVE_BFD
	p_process->bfd_enable = OSPF_DEFAULT_BFD_STATE;
	p_process->bfd_minrx_interval = OSPF_BFD_SES_MINRX_DEF;
	p_process->bfd_mintx_interval = OSPF_BFD_SES_MINTX_DEF;
	p_process->bfd_detmul = OSPF_BFD_SES_DETMUL_DEF;
#endif
	p_process->packet_block = OSPF_DEFAULT_PACKET_BLOCK;
	
	ospf_nhopw_init(p_process->nhopw);
//	p_process->opaque = nmvaule_to_ospf(ENABLE);
//	p_process->restart_helper = TRUE;
    p_process->preference = OSPF_DEFAULT_DISTANCE;
    p_process->preference_ase = OSPF_DEFAULT_ASE_DISTANCE;
    p_process->rfc1583_compatibility = TRUE;
    p_process->mpls_flag = FALSE;
    p_process->mpls_lsrid = 0;
    //p_process->mpls_te = FALSE;
    p_process->mpls_te = TRUE;
    p_process->mpls_cost = OSPF_DEFAULT_MPLSID_COST;
    p_process->def_route_adv = FALSE;

    ospf_area_table_init(p_process);
    ospf_lstinit(&p_process->filtered_route_table, ospf_iproute_cmp);
    ospf_lstinit(&p_process->req_table, ospf_request_lookup_cmp);
    ospf_lstinit(&p_process->rxmt_table, NULL);
    ospf_lstinit(&p_process->lstable_table, ospf_lstable_lookup_cmp);

#ifdef OSPF_FRR
    ospf_lstinit(&p_process->backup_route_table, ospf_backup_route_cmp);    
#endif
    ospf_lstinit(&p_process->te_tunnel_table, ospf_te_tunnel_lookup_cmp);
#ifdef OSPF_DCN
    ospf_lstinit(&p_process->dcn_rtm_tx_table, ospf_dcn_rxmt_cmp);    
#endif
    ospf_lstinit(&p_process->asbr_range_table, ospf_asbr_range_lookup_cmp);
    ospf_te_router_table_init(p_process);
    ospf_import_table_init(p_process);
    ospf_route_table_init(&p_process->route_table);    
    ospf_network_table_init(p_process);
    ospf_lsa_table_init(&p_process->t5_lstable, OSPF_LS_AS_EXTERNAL, p_process, NULL, NULL);
    ospf_lsa_table_init(&p_process->t11_lstable, OSPF_LS_TYPE_11, p_process, NULL, NULL);
    
    ospf_if_table_init(p_process);
    ospf_redistribute_table_init(p_process);
    /*ospf redis range*/
    ospf_redis_range_table_init(p_process);
    ospf_nexthop_table_init(p_process);
    ospf_process_nbr_table_init(p_process);
    ospf_redis_policy_table_init(p_process);
    ospf_filter_policy_table_init(p_process);

    /*init router id,for multi-instance,it is always 0 at first*/
    p_process->router_id = 0;//ospf_select_router_id(p_process->vrid);
    p_process->routerid_flg = 0;

    /*restart*/
    p_process->restart_period = OSPF_DEFAULT_RESTART_TIME;
    p_process->restart_reason = OSPF_RESTART_NONE;

   
    /*init fragment list,unsorted*/
    ospf_lstinit(&p_process->fragment_table, ospf_lsa_fragment_lookup_cmp);

    ospf_timer_init(&p_process->import_timer, p_process, ospf_import_route_update_all, p_process);
   // zhurish ospf_timer_init(&p_process->overlay_timer, p_process, ospf_dcn_overlay_network_timer_pro, p_process);
	
    ospf_timer_init(&p_process->spf_timer, p_process, ospf_route_calculate_full, p_process);
    ospf_timer_init(&p_process->db_overload_timer, p_process, NULL, p_process);             
    ospf_timer_init(&p_process->routeid_timer, p_process, ospf_old_routerid_timeout, p_process); 

    ospf_timer_init(&p_process->restart_timer, p_process, ospf_restart_timeout, p_process); 

    ospf_timer_init(&p_process->lsa_aging_timer, p_process, ospf_lsa_table_timer_expired, p_process);

    ospf_timer_init(&p_process->ipupdate_timer, p_process, ospf_ip_update_timer_expired, p_process);
    ospf_timer_init(&p_process->ratelimit_timer, p_process, NULL, p_process);

    ospf_timer_init(&p_process->rxmtlimit_timer, p_process, ospf_lsa_rxmt_limit_timer, p_process);
    ospf_timer_init(&p_process->delete_timer, p_process, ospf_process_delete_event_handler, p_process);
    ospf_timer_init(&p_process->id_reset_timer, p_process, ospf_reboot_event_handler, p_process);
    ospf_timer_init(&p_process->restart_wait_timer, p_process, ospf_restart_wait_timeout, p_process);
    ospf_timer_init(&p_process->range_update_timer, p_process, ospf_range_update_timeout, p_process);
    ospf_timer_init(&p_process->redis_range_update_timer, p_process, ospf_redis_range_update_timeout, p_process);
    //ospf_timer_init(&p_process->asbr_range_update_timer, p_process, ospf_asbr_range_update_timeout, p_process);

#ifdef OSPF_FRR/*20130415 frr test*/
    ospf_timer_init(&p_process->frr_timer, p_process, ospf_frr_timer_expired, p_process);   
    ospf_timer_init(&p_process->backup_ipupdate_timer, p_process, ospf_backup_route_export, p_process);   
#endif
   /*20130517 fast protection*/
    ospf_timer_init(&p_process->fast_spf_timer, p_process, ospf_fast_spf_timer_expired, p_process);
    ospf_timer_init(&p_process->te_tunnel_timer, p_process, ospf_te_tunnel_timer_expired, p_process);
    ospf_timer_init(&p_process->lsa_check_timer, p_process, ospf_lsa_check_timer_expired, p_process);
#ifdef OSPF_DCN
 //   ospf_timer_init(&p_process->dcn_rtm_tx_timer, p_process, ospf_rtmsg_dcn_lsa_send, p_process);    
#endif
    ospf_timer_init(&p_process->stub_router_timer, p_process, ospf_stub_router_timeout, p_process);

    p_process->lsa_rxmt_limit = OSPF_UPDATE_RXMT_LIMIT;

    ospf_stimer_start(&p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);
    ospf_stimer_start(&p_process->lsa_check_timer, OSPF_LSA_CHECK_TIME);
    return;
}

/*scheduled process reboot timer:consider large routing table*/
void 
ospf_reboot_event_handler(struct ospf_process *p_process)    
{
    struct ospf_area *p_area = NULL;        
    struct ospf_area *p_next = NULL;           
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_nextlsa = NULL;
    struct ospf_lstable *p_table = NULL;
    struct ospf_lstable *p_next_table = NULL;
    u_int new_id = 0;
    /*reuse old id firstly*/
 //   new_id = p_process->router_id;
	
	if(p_process->new_routerid != 0)
	{
		new_id = p_process->new_routerid;
	}
	else
	{
		new_id = p_process->router_id;
	}
    p_process->router_id = p_process->old_routerid;

    /*timeout all local lsa,and flood them.*/
    for_each_node(&p_process->lstable_table, p_table, p_next_table)
    {
        for_each_node(&p_table->list, p_lsa, p_nextlsa)
        {
            if (ntohl(p_lsa->lshdr->adv_id) == p_process->router_id)
            {
                ospf_lsa_maxage_set(p_lsa);
                ospf_lsa_flood(p_lsa, NULL, NULL);
            }
        }
    }
    /*trigger all interface's flooding buffer*/
    ospf_lstwalkup(&p_process->if_table, ospf_if_flood_timeout);
    
    /*close all areas*/   
    for_each_ospf_area(p_process, p_area, p_next)
    {
        ospf_area_down(p_area);
    }
    
    /*clear all lsa*/
    ospf_lsa_flush_all(p_process);

    /*calculate route*/
    ospf_route_calculate_full(p_process);
#ifdef OSPF_DCN //zhurish
    if(p_process->process_id == OSPF_DCN_PROCESS)
    {
        ospf_logx(ospf_debug_dcn,"%d,wait_export=%d,send_routemsg=%d.\r\n",__LINE__,p_process->wait_export,p_process->send_routemsg);   
        p_process->wait_export = 0;
        p_process->send_routemsg = 0;
    }
#endif
    /*if any update not finished,wait for next loop*/
    if (p_process->wait_export || p_process->send_routemsg)
    {
        p_process->router_id = new_id;
        ospf_timer_start(&p_process->id_reset_timer, 1);
        return;
    }

    /*all update finished,continue processing*/
    
    /*restore new id*/
    p_process->router_id = new_id;

    /*bring up all areas*/
    ospf_lstwalkup(&p_process->area_table, ospf_area_up);
    
    /*save old router id in 60 s,to delete all lsa originated by old instance*/
    ospf_stimer_start(&p_process->routeid_timer, 60);    
    return;
}

/*schedule range update timer:when large routing table exist,we must
   update system route firstly,so start a timer to delay range's update*/
void
ospf_range_update_timeout(struct ospf_process *p_process)
{
    struct ospf_range *p_range = NULL;
    struct ospf_range *p_next = NULL;

    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    /*check and update all ranges*/
    for_each_node(&ospf.nm.range_table, p_range, p_next)
    {
        if (p_range->p_area->p_process != p_process)
        {
            continue;
        }
        
        if (OSPF_LS_SUMMARY_NETWORK == p_range->lstype)
        {
            ospf_range_update(p_range);
        }
        else 
        {
            if (TRUE == p_range->need_check)
            {
                ospf_nssa_range_update(p_range);
                p_range->need_check = FALSE;
            }
        }
    }
    return;
}

void ospf_stub_router_timeout(struct ospf_process *p_process)
{
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    //printf("%s\n",__FUNCTION__);
    p_process->stub_adv = FALSE;

    ospf_stub_router_lsa_originate(p_process);

    return;
}
/*network operation*/

/*create one network configure*/
struct ospf_network *
ospf_network_create(
             struct ospf_process *p_process, 
             u_int dest,
             u_int mask,
             u_int area)
{
    struct ospf_network *p_network = NULL;
    p_network = ospf_malloc2(OSPF_MNETWORK);
    if (NULL == p_network)
    {
        return NULL;
    }
    p_network->dest = dest;
    p_network->mask = mask;
    p_network->area_id = area;
    p_network->p_process = p_process;
    ospf_lstadd(&p_process->network_table, p_network);

    ospf_lstadd(&ospf.nm.network_table, p_network);
    
    return p_network;
}

/*destory one network configure*/
void 
ospf_network_delete(struct ospf_network *p_network)
{
    struct ospf_process *p_process = p_network->p_process;
  
    ospf_lstdel(&ospf.nm.network_table, p_network);

    ospf_lstdel_free(&p_process->network_table, p_network, OSPF_MNETWORK);    
    return;
}

/*compare function of network*/
int
ospf_network_lookup_cmp(
             struct ospf_network *p1,
             struct ospf_network *p2)
{
    OSPF_KEY_CMP(p1, p2, dest);
    OSPF_KEY_CMP(p1, p2, mask);
    return 0;
}    

int
ospf_network_nm_cmp(
             struct ospf_network *p1,
             struct ospf_network *p2)
{
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
    OSPF_KEY_CMP(p1, p2, dest);
    OSPF_KEY_CMP(p1, p2, mask);
    OSPF_KEY_CMP(p1, p2, area_id);
    return 0;
}    

/*init network table*/
void 
ospf_network_table_init(struct ospf_process *p_process)
{
    ospf_lstinit(&p_process->network_table, ospf_network_lookup_cmp);
    return;
}

/*This routine find one network configure according to dest and mask*/
struct ospf_network *
ospf_network_lookup(
              struct ospf_process *p_process, 
              u_int dest, 
              u_int mask)
{
    struct ospf_network *p_network = NULL;
    struct ospf_network *p_next = NULL;
    struct ospf_network search;
    
    /*have mask,search strictly*/
    if (0 != mask)
    {
        search.dest = dest;
        search.mask = mask;
        return ospf_lstlookup(&p_process->network_table, &search);
    }
    /*mask==0 mean search network,do not compare mask*/
    for_each_node(&p_process->network_table, p_network, p_next)
    {
        if (ospf_netmatch(p_network->dest, dest, p_network->mask))
        {        
            return p_network;
        }
    }     
    return NULL; 
}

/*This routine judges whether the network added 
  newly and former network is overlapped*/
struct ospf_network* 
ospf_network_match(
            struct ospf_process *p_process, 
            u_int addr, 
            u_int mask)
{
    struct ospf_network *p_network = NULL;
    struct ospf_network *p_next = NULL;
    struct ospf_process *p_loop_process = NULL;
    struct ospf_process *p_next_process = NULL;

    for_each_ospf_process(p_loop_process, p_next_process)
    {
        if (p_loop_process->vrid != p_process->vrid)
        {
            continue;
        }
        for_each_node(&p_loop_process->network_table, p_network, p_next)
        {  
            if (ospf_netmatch(p_network->dest, addr, mask) 
               && ospf_netmatch(p_network->dest, addr, p_network->mask))
            {
                ospf_log(ospf_debug, "same or overlapped network have been configed");
                return p_network;
            }        
        }     
    }
    return NULL; 
}

/*check if there be at least one network matching special area*/ 
STATUS 
ospf_network_match_area(
          struct ospf_process *p_process, 
          u_int area)
{
    struct ospf_network *p_network = NULL;
    struct ospf_network *p_next = NULL;

    for_each_node(&p_process->network_table, p_network, p_next)
    {                 
        if (p_network->area_id == area)
        {
            return OK;
        }
    } 
    return ERR; 
}

/*This routine create network according to network command on CLI*/
struct ospf_network * 
ospf_network_add(
                  struct ospf_process *p_process,  
                  u_int network,
                  u_int mask,
                  u_int area_id)
{
    struct ospf_network *p_network;
    u_int unit = 0, uiPortType = 0;

	#if 0
    if (ERR != ospf_sys_addr2ifunit(p_process->vrid, network, &unit))
    {
        uiPortType = IFINDEX_TO_TYPE(unit);
    }
    #endif
    //if (IFINDEX_TYPE_LOOPBACK_IF != uiPortType)
    {
        if (OK == ospf_network_search(p_process->process_id,network,mask,area_id,p_process->vrid))
    	{	 
    		return NULL;
    	}
    }
    
    p_network = ospf_network_lookup(p_process, network, mask);    
    if (NULL != p_network)
    {
        return (p_network->area_id != area_id) ? NULL : p_network;
    }
    #if 0
    //if (IFINDEX_TYPE_LOOPBACK_IF != uiPortType)
    {
        if (NULL != ospf_network_match(p_process, network, mask))
        {    
            return NULL;
        }
    }
    #endif
    return ospf_network_create(p_process, network, mask, area_id);
}

/*search network if exist*/
int ospf_network_search(
                  u_int uiProId,  
                  u_int network,
                  u_int mask,
                  u_int area_id,
                  u_int vrf_id)
{
    struct ospf_network *p_network = NULL;
    struct ospf_network *p_next = NULL;
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    u_int uiMinMask = 0;

    for_each_ospf_process(p_process, p_next_process)   
	{
		if(p_process == NULL)
		{
			continue;
		}
	    //p_network = ospf_network_lookup(p_process, network, 0); 
	    for_each_node(&p_process->network_table, p_network, p_next)
        {
            uiMinMask = (mask < p_network->mask) ? mask:p_network->mask;
            if (ospf_netmatch(p_network->dest, network, uiMinMask))
            {        
                if(((p_process->process_id != uiProId) && (vrf_id == p_process->vrid))
    			||((p_network->area_id != area_id) && (p_process->process_id == uiProId)))
    			{
    				vty_out_to_all_terminal("Error: The network confilict with another area or process");
    				return OK;/*存在该网络*/
    			}
            }
        } 
	}

    return ERR;/*不存在该网络*/
}

/*compare ospf te tunnel*/
int
ospf_te_tunnel_lookup_cmp(
             struct ospf_te_tunnel *p1,
             struct ospf_te_tunnel *p2)
{
    OSPF_KEY_CMP(p1, p2, addr_out);
    return 0;
}   

/*te tunnel add*/
struct ospf_te_tunnel *
ospf_te_tunnel_add(
            struct ospf_process *p_process,
            u_int addr_out)
{
    struct ospf_te_tunnel *p_te_tunnel = NULL;
    
    p_te_tunnel = ospf_malloc2(OSPF_MTETUNNEL);
    if (NULL == p_te_tunnel)
    {
        return NULL;
    }

    p_te_tunnel->p_process = p_process;
    p_te_tunnel->addr_out = addr_out;
    ospf_lstadd(&p_process->te_tunnel_table, p_te_tunnel);
    return p_te_tunnel;
}

/*te tunnel del*/
void 
ospf_te_tunnel_delete(struct ospf_te_tunnel *p_te_tunnel)
{ 
    ospf_lstdel_free(&p_te_tunnel->p_process->te_tunnel_table, p_te_tunnel, OSPF_MTETUNNEL);  
    return;
}

/*lookup for te tunnel*/
struct ospf_te_tunnel *
ospf_te_tunnel_lookup(
         struct ospf_process *p_process,
         u_int addr_out)
{
    struct ospf_te_tunnel search;

    search.addr_out = addr_out; 
    return ospf_lstlookup(&p_process->te_tunnel_table, &search);  
}

/*schedule te tunnnel timer:when update system route, start this timer to 
originate router lsa which including te tunnel interface*/
void
ospf_te_tunnel_timer_expired(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;
    
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    for_each_ospf_area(p_process, p_area, p_next_area)
    {   
        ospf_router_lsa_originate(p_area);
    }
    return;
}


/*main loop of ospf*/
#if 1
void ospf_main_task(void)
{

	struct timeval tv;
    fd_set fdset;
    int maxfd = 0;
    int sock = ospf.sock[0];
#ifdef OSPF_RTSOCK_ENABLE
    int rtsock = ospf.rtsock;
#endif
#ifdef OSPF_CMDSOCK_ENABLE
    int cmdsock = ospf.cmdsock;
#endif
    int result = 0;
    int i = 0;

    /* delay awhile so that the parent task will be able to add the needed task
     * variables to our task's context*/
    ospf_sys_delay(1);
    /*set wait time*/
	memset(&tv,0,sizeof(tv));

    tv.tv_sec = 2;
    tv.tv_usec = 0;//100000;

    maxfd = sock;
#ifdef OSPF_RTSOCK_ENABLE
    maxfd = MAX(rtsock, maxfd);
#endif
#ifdef OSPF_CMDSOCK_ENABLE
    maxfd = MAX(cmdsock, maxfd);
#endif
    while (1)
    {
        FD_ZERO(&fdset);
        for(i = 0; i < OSPF_L3VPN_NUM_MAX + 1; i++)
        {
            sock = ospf.sock[i];
            if (0 < sock)
            {
                FD_SET(sock, &fdset);
            }
        }
#ifdef OSPF_RTSOCK_ENABLE
        FD_SET(cmdsock, &fdset);
#endif
#ifdef OSPF_CMDSOCK_ENABLE
        FD_SET(cmdsock, &fdset);
#endif
        tv.tv_sec = ospf.waittime/1000;
        tv.tv_usec = (ospf.waittime%1000) * 1000;

        /*check socket event*/ 
        result = select(maxfd + 1, &fdset, NULL, NULL, &tv);
        /*release cpu for a while if too many rx event detected*/
        ospf_semtake_forever();
        /*sechdule rx operation using timer*/
        if (0 < result)
        {
            for(i = 0; i < OSPF_L3VPN_NUM_MAX + 1; i++)
            {
                sock = ospf.sock[i];
                if(sock > 0 && FD_ISSET(sock, &fdset))
                {
        			ospf_socket_recv(sock);
                }
            }
            ospf_thread_timer();
        }
        if (0 == result)
        	ospf_thread_timer();

        ospf_update_min_timer();

        ospf_semgive();
    }
}
#endif
/*sync msg send checking timer*/
void 
ospf_sync_check_timeout(void *arg)
{
#ifdef OSPF_MASTER_SLAVE_SYNC
    ospf_lstwalkup(&ospf.process_table, ospf_syn_check_event);
#endif
    return;
}

/*recv event as timer*/
/*void
ospf_recv_thread(void *arg)
{
    ospf_socket_recv(ospf.sock[0]);
 //   ospf_rtsock_recv(ospf.rtsock);
   // ospf_cmdsock_recv(ospf.cmdsock);    
    return;
}*/

/*init timer table,create multiple timer list for high performance*/
void 
ospf_timer_table_init(void)
{
    struct ospf_timer_block *p = NULL;
    u_int i; 
         
    ospf_lstinit(&ospf.thread_table, NULL);
   
    ospf.p_thread = XMALLOC(MTYPE_OSPF_TMP,sizeof(struct ospf_timer_block) * OSPF_MAX_THREAD_HEAD);
    if (ospf.p_thread)
    {
        memset(ospf.p_thread, 0, (sizeof(struct ospf_timer_block) * OSPF_MAX_THREAD_HEAD));
        for (i = 0, p = ospf.p_thread; i < OSPF_MAX_THREAD_HEAD; i++, p++)
        {
            INIT_LIST_HEAD(&p->list);
        }        
    }
    return;
} 

/*init a timer node,using posix like interface,setting arg and callback function here*/
void 
ospf_timer_init(
            struct ospf_timer *p_thread,
            void *arg,
            void *func,
            void *context)
{
    INIT_LIST_HEAD(&p_thread->node);
    p_thread->arg = arg;
    p_thread->func = (void (*)(void*))func;
    p_thread->context = context;
    return;
}

/*decide timer's remain expiration ticks*/
u_int 
ospf_timer_remain(
           struct ospf_timer *p_thread,
           u_int now)
{
    u_int diff;
    if (p_thread == NULL)
    {
        return 0;
    }
    diff = ospf_time_differ(now, p_thread->start_time);
    if (p_thread->delay > diff)
    {
        return p_thread->delay - diff;
    }
    return 0;
}

/*schedule a work with delay request, in ticks*/
void 
ospf_timer_start(
          struct ospf_timer *p_thread,
          u_int delay)
{
    struct ospf_timer *p_check = NULL;
    struct list_head * prev = NULL;
    struct list_head * listnode = NULL;
    struct list_head *list = NULL;
    struct ospf_timer_block *p_head;
    u_int index = 0;
    u_int now = ospf_sys_ticks();

    /*decide timer list to be add:if system's timer exceed limit,
        select next timer, else always select the first one*/
    index = (!ospf.thread_overload) ? 0 : (index + 1);
    if (OSPF_MAX_THREAD_HEAD <= index)
    {
        index = 0;
    }

    p_head = &ospf.p_thread[index];

    /*insert to thread table*/
    if (!p_head->active)
    {
        ospf_lstadd_unsort(&ospf.thread_table, p_head);
        p_head->active = TRUE;
    }
    list_del_init(&p_thread->node);
    p_thread->active = TRUE;
    p_thread->delay = delay;
    p_thread->start_time = now;
    //ospf.waittime = MIN(delay, ospf.waittime);
    list = &p_head->list;

    /*try to add to the last*/
    if (!list_empty(list))
    {
        p_check = list_entry(list->prev, struct ospf_timer, node);
        if (ospf_timer_remain(p_check, now) <= delay)
        {
            list_add_tail(&p_thread->node, list);
            return;
        }
    }
    
    /*sort*/
    prev = list;
    list_for_each(listnode, list)
    {
        p_check = list_entry(listnode, struct ospf_timer, node);
        if (ospf_timer_remain(p_check, now) > delay)
        {
            break;
        }
        prev = listnode;
    }
    list_add(&p_thread->node, prev);
    return;
}

/*stop a timer*/
void 
ospf_timer_stop(struct ospf_timer *p_thread)
{
    list_del_init(&p_thread->node);
    p_thread->active = FALSE;
    return;
}

/*check work list,if a work delay expired,call it's function*/
#if 1
void 
ospf_thread_timer()
{
    struct ospf_timer *p_check = NULL;
    struct list_head *list = NULL;
    struct ospf_timer_block *p_head;
    struct ospf_timer_block *p_next;
    u_int start = ospf_sys_ticks();
    u_int now = start;

    /*update overload flag*/
    if ((ospf_lstcnt(&ospf.process_table) + ospf_lstcnt(&ospf.real_if_table)) < OSPF_MAX_THREAD)
    {
        ospf.thread_overload = FALSE;
    }
    else
    {
        ospf.thread_overload = TRUE;
    }
    for_each_node(&ospf.thread_table, p_head, p_next)
    {
        list = &p_head->list;
        while(!list_empty(list))
        {
            p_check = list_entry(list->next, struct ospf_timer, node);
            //remain = ospf_timer_remain(p_check, now);
            if (p_check && ospf_timer_remain(p_check, now)<=0)
            {
                list_del_init(&p_check->node);
                p_check->active = FALSE;
                ospf_set_context(p_check->context);
                if (p_check->func)
                {
                    p_check->func(p_check->arg);
                }
            }
       }
       /*if system's timer exceed limit,do not delete from table*/
       if (!ospf.thread_overload && list_empty(list))
       {
           ospf_lstdel_unsort(&ospf.thread_table, p_head);
           p_head->active = FALSE;
       }
    }
    return;
}
void
ospf_update_min_timer()
{
    struct ospf_timer *p_check = NULL;
    struct list_head *list = NULL;
    struct ospf_timer_block *p_head;
    struct ospf_timer_block *p_next;
    u_int start = ospf_sys_ticks();
    ospf.waittime = OSPF_MAX_TICKS;
    for_each_node(&ospf.thread_table, p_head, p_next)
    {
        list = &p_head->list;
        while(!list_empty(list))
        {
            p_check = list_entry(list->next, struct ospf_timer, node);
            //remain = ospf_timer_remain(p_check, now);
            if (p_check && p_check->active == TRUE)
            {
            	ospf.waittime = MIN(p_check->delay - start, ospf.waittime);
            }
       }
    }
    return;
}
#else
void
ospf_thread_check()
{
    struct ospf_timer *p_check = NULL;
    struct list_head *list = NULL;
    struct ospf_timer_block *p_head;
    struct ospf_timer_block *p_next;
    u_int start = ospf_sys_ticks();
    u_int now = start;
    u_int remain = 0;
    u_int minremain = 0xffffffff;
    u_int max_loop_time = 20;

    /*update overload flag*/
    if ((ospf_lstcnt(&ospf.process_table) + ospf_lstcnt(&ospf.real_if_table)) < OSPF_MAX_THREAD)
    {
        ospf.thread_overload = FALSE;
    }
    else
    {
        ospf.thread_overload = TRUE;
    }
    
    /*default wait 100ms*/
	#if 0
    tv->tv_sec = 0;
    tv->tv_usec = 100000;
	#endif
    for_each_node(&ospf.thread_table, p_head, p_next)
    {
        list = &p_head->list;
        while(!list_empty(list))
        {
            p_check = list_entry(list->next, struct ospf_timer, node);
            remain = ospf_timer_remain(p_check, now);
            if (remain)
            {
                if (minremain > remain)
                {
                    minremain = remain;
                }
                break;
            }
            list_del_init(&p_check->node);
            p_check->active = FALSE;
            ospf_set_context(p_check->context);
            if (p_check->func)
            {
                p_check->func(p_check->arg);
            }
            /*one loop for 4s*/
            now = ospf_sys_ticks();
            if (ospf_time_differ(now, start) > max_loop_time)
            {
                /*wait for 100ms*/
                minremain = 1;
                goto END;
            }
       }
       /*if system's timer exceed limit,do not delete from table*/ 
       if (!ospf.thread_overload && list_empty(list))
       {
           ospf_lstdel_unsort(&ospf.thread_table, p_head);
           p_head->active = FALSE;
       }
    }   
END:
    if (OSPF_MAX_WAIT_TIME < minremain)
    {
        minremain = OSPF_MAX_WAIT_TIME;
    }
 #ifdef WIN32
    minremain = 1;
 #endif
 #if 0
    tv->tv_sec = minremain / OSPF_TICK_PER_SECOND;
    tv->tv_usec = (minremain % OSPF_TICK_PER_SECOND) * 100000;
#endif
    return;
}
#endif

#if 0 //zhurish
void ospf_link_chg_proc(ZEBRA_IF_MSG_REDISTRIBUTE_T *pstL3IfRedisMsg)
{
    char acIfName[64] = {0};
    if(NULL == pstL3IfRedisMsg)
    {
        return;
    }


    if(if_name_get_by_index(pstL3IfRedisMsg->ulIfIndex, acIfName) == ERROR)
    {
        return ;
    }
    switch(pstL3IfRedisMsg->ulSubCode)
    {
        case ZEBRA_REDISTRIBUTE_TYPE_IF_DOWN:
            ospf_logx(ospf_debug_nbr,"interface %s is down now!\n", acIfName);
            ospf_updatIfStatus(pstL3IfRedisMsg->ulIfIndex,OSPF_IFE_DOWN);
            break;
        case ZEBRA_REDISTRIBUTE_TYPE_IF_UP:
            ospf_logx(ospf_debug_nbr,"interface %s is up now!\n", acIfName);
            ospf_updatIfStatus(pstL3IfRedisMsg->ulIfIndex,OSPF_IFE_UP);
            break;
        case ZEBRA_REDISTRIBUTE_TYPE_IF_ADDR_ADD:
            ospf_logx(ospf_debug_nbr,"interface %s ip is added now!\n", acIfName);
            ospf_updataipaddr(pstL3IfRedisMsg->ulVrfId,pstL3IfRedisMsg->ulIfIndex,pstL3IfRedisMsg->stIpPrefix.u.prefix4.s_addr,pstL3IfRedisMsg->ulSubCode);
            break;
        case ZEBRA_REDISTRIBUTE_TYPE_IF_ADDR_DEL:
            ospf_logx(ospf_debug_nbr,"interface %s ip is deleted now!\n", acIfName);
            ospf_updataipaddr(pstL3IfRedisMsg->ulVrfId,pstL3IfRedisMsg->ulIfIndex,pstL3IfRedisMsg->stIpPrefix.u.prefix4.s_addr,pstL3IfRedisMsg->ulSubCode);
            break;
        default:
            break;
    }
    
}

void ospf_ip_chg_proc(ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *pstL3IfRedisMsg)
{
    char acIfName[64] = {0};
    if(NULL == pstL3IfRedisMsg)
    {
        return;
    }

    if(if_name_get_by_index(pstL3IfRedisMsg->ulIfIndex, acIfName) == ERROR)
    {
        return ;
    }
    switch(pstL3IfRedisMsg->ulSubCode)
    {
        case ZEBRA_REDISTRIBUTE_TYPE_IF_ADDR_ADD:
            ospf_logx(ospf_debug_nbr,"interface %s ip is added now!\n", acIfName);
            break;
        case ZEBRA_REDISTRIBUTE_TYPE_IF_ADDR_DELETE:
            ospf_logx(ospf_debug_nbr,"interface %s ip is deleted now!\n", acIfName);
            break;
        default:
            break;
    }
	ospf_updataipaddr(pstL3IfRedisMsg->ulVrfId,pstL3IfRedisMsg->ulIfIndex,pstL3IfRedisMsg->stIfIp.u.prefix4.s_addr,pstL3IfRedisMsg->ulSubCode);
}

void ospf_if_chg_proc(ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *pstL3IfRedisMsg)
{
    char acIfName[64] = {0};
    if(NULL == pstL3IfRedisMsg)
    {
        return;
    }

    if(if_name_get_by_index(pstL3IfRedisMsg->ulIfIndex, acIfName) == ERROR)
    {
        return ;
    }
    switch(pstL3IfRedisMsg->ulSubCode)
    {
        case ZEBRA_REDISTRIBUTE_TYPE_IF_ADD:
            ospf_logx(ospf_debug_nbr,"interface %s is created now!\n", acIfName);
            break;
        case ZEBRA_REDISTRIBUTE_TYPE_IF_DELETE:
			ospf_updatIfDel(pstL3IfRedisMsg->ulIfIndex);
            ospf_logx(ospf_debug_nbr,"interface %s is deleted now!\n", acIfName);
            break;
        default:
            break;
    }
    return;
}


#ifdef OSPF_VPN


int ospf_vpn_instance_cnt(u_int ulVrf)
{
    int iCnt = 0;
    struct ospf_process *pstFristProcess = NULL;
    struct ospf_process *pstNextProcess = NULL;

    for_each_node(&ospf.process_table, pstFristProcess,pstNextProcess)
    {
        if (pstFristProcess->vrid == ulVrf)
        {
            iCnt++;
        }
    }

    return iCnt;
}

void ospf_rtsock_vpn_del_msg_input(L3VPN_MSG_T *pstL3VpnMsg)
{
    struct ospf_process *pstFristProcess = NULL;
    struct ospf_process *pstNextProcess = NULL;
    u_long ulVrf = 0;
    int iCnt = 0;
    
    if(NULL == pstL3VpnMsg)
    {
        return;
    }
    ulVrf = pstL3VpnMsg->vpn_vrid;
    if (ulVrf == 0)
    {
        return;
    }

    iCnt = ospf_vpn_instance_cnt(ulVrf);
    if(iCnt == 0)
    {
        return;
    }

    for_each_node(&ospf.process_table, pstFristProcess,pstNextProcess)
    {
        if (pstFristProcess->vrid == ulVrf)
        {
            pstFristProcess->proto_shutdown = TRUE;
            ospf_timer_start(&pstFristProcess->delete_timer, 1);
        }
    }
    return;
}

int ospf_vpn_msg_input(L3VPN_MSG_T *pstL3VpnMsg)
{
    if(NULL == pstL3VpnMsg)
    {
        return ERR;
    }
#if 0
    switch(pstL3VpnMsg->iState)
    {
        case L3VPN_ROW_STATUS_ACTIVE:
        {
            break;
        }
        case L3VPN_ROW_STATUS_DESTORY:
        {
            ospf_rtsock_vpn_del_msg_input(pstL3VpnMsg);
            break;
        }
        default:
        {
            break;
        }
    }
#endif    
    return OK;
}


int ospf_vpn_cfg_chg_proc(IPC_MSG_T *pstMsg)
{
    L3VPN_MSG_T *pstL3VpnMsg = NULL;
    u_int ulVrf = 0;
    u_long ulMsgType = 0;
    int iRet = OK;
    int iCnt = 0;
    
    if(NULL == pstMsg)
    {
        return ERR;
    }

    pstL3VpnMsg = pstMsg->pucBuf;
    ulVrf = pstL3VpnMsg->vpn_vrid;

    iCnt = ospf_vpn_instance_cnt(ulVrf);
    if(iCnt == 0)
    {
        return ERR;
    }

    switch(pstMsg->ulMsgType)
    {
    #if 0
        case MSG_TYPE_L3VPN_CHG:
            ospf_vpn_msg_input(pstL3VpnMsg);
            break;
    #endif        
        default:
            iRet = ERR;
            break;
    }

    return iRet;
}


#endif

void ospf_stub_router_timeout(struct ospf_process *p_process)
{
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    //printf("%s\n",__FUNCTION__);
    p_process->stub_adv = FALSE;

    ospf_stub_router_lsa_originate(p_process);

    return;
}

#ifdef OSPF_MASTER_SLAVE_SYNC
/*开启指定进程opaque使能*/

int ospf_instance_gr_enable(u_int uiInstance)
{
  
    int value_l = ENABLE,ret = OK;
    struct ospf_process *p_process = NULL;

   // printf("ospf_instance_gr_enable uiInstance:%d\n",uiInstance);

    p_process = ospf_process_lookup(&ospf, uiInstance);
  //  printf("ospf_instance_gr_enable p_process:0x%x\n",p_process);

    if(p_process == NULL)
    {
        return ERR;
    }

  //  ret = ospfSetApi(uiInstance,OSPF_GBL_OPAQUE,&value_l);
    p_process->opaque = nmvaule_to_ospf(value_l);
//    printf("11 ospf_instance_gr_enable uiInstance:%d\n",uiInstance);


//    ret |= ospfSetApi(uiInstance,OSPF_GBL_RESTARTSUPPORT,&value_l);

    p_process->restart_enable = nmvaule_to_ospf(value_l);               
    if ((FALSE == p_process->restart_enable) && p_process->in_restart)
    {                 
        /*if GR is running,exit it*/ 
        ospf_logx(ospf_debug_gr, "nm set gr,exit graceful restart");     
        ospf_restart_finish(p_process, OSPF_RESTART_TOPLOGY_CHANGED);
    }
    //printf("22 ospf_instance_gr_enable uiInstance:%d\n",uiInstance);

//    ret |= ospfSetApi(uiInstance,OSPF_GBL_RESTARTBEGIN,&value_l);

    ospf_restart_request(p_process, OSPF_RESTART_PLANNED);
  //  printf("33 ospf_instance_gr_enable uiInstance:%d\n",uiInstance);

  
    return OK;    
}
/*主备倒换调用*/
int ospf_master_slave_state_chg_proc(u_int uiWorkStat)
{
	u_int uiStaHis = 0,uiInstance = 0;
    int value_l = ENABLE;
	u_long ulInstanceId = 0,ulData;
	u_long ulInstanceNextId = 0;
	int ret = OK,iRtv = 0;

    ospf_get_workmode(&ospf, uiWorkStat);
#if 0
    ospf_master_slave_state_get(&uiStaHis);

    ospf_log(ospf_debug_if,"ospf_master_slave_state_chg_proc uiWorkStat:%d,uiStaHis:%d",uiWorkStat,uiStaHis);
    if(uiStaHis == DCN_SLOT_INIT)
	{
    	ospf_master_slave_state_set(uiWorkStat);

        ospf_syn_wokemode();
    	//ospf.work_mode = uiWorkStat;
        return OK;
	}
    if((uiWorkStat == DCN_SLOT_MASTER)
        &&(uiStaHis == DCN_SLOT_SLAVER))
    {
       #if 0
		/*开启所有进程opaque使能*/
		/*刷新进程的stub-router interval*/
		ospf_refresh_stub_router_all();
		
		ret = ospfInstanceGetFirst(&ulInstanceId);
		if(OK == ret)
		{
			while(OK == ret)
			{
				ret = ospfInstanceGetNext(ulInstanceId, &ulInstanceNextId);
				if((ulInstanceId < 0)||(ulInstanceId > 65536))
				{
					ulInstanceId = ulInstanceNextId;
					continue;
				}
				if (OK != ospf_instance_gr_enable(ulInstanceId))
				{
					return ERR;
				}  
				ulInstanceId = ulInstanceNextId;
			}
		
		}
		#endif
		ospf_master_slave_state_set(uiWorkStat);
		ospf_get_workmode(&ospf);
        //ospf.work_mode = OSPF_MODE_MASTER;
	}
#endif
    return OK;

}
#endif

#ifdef OSPF_L3QUEUE_NEW
int ospf_l3_redis_queue_get()
{
    int iRet = OK;
    u_long iCnt = 50;
    static u_long ulCount = 0;
    ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *pstL3swIfRedisMsgAdd = NULL;


    ulCount++;
    #if 0
    if(ulCount%20 != 0)
    {
        return OK;
    }
    #endif
    
    while(iCnt>0)
    {
        pstL3swIfRedisMsgAdd = (ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *)pqueue_dequeue(ospf.pstL3RedisQueue);
        if (pstL3swIfRedisMsgAdd == NULL)
        {
            iCnt--;
            continue;
        }
        if (OSPF_MODE_SLAVE != ospf.work_mode)
        {
            switch(pstL3swIfRedisMsgAdd->ulMsgType)
            {
                case MSG_TYPE_LINK_CHG:
                    ospf_link_chg_proc(pstL3swIfRedisMsgAdd);
                    break;
                case MSG_TYPE_IP_CHG:
                    ospf_ip_chg_proc(pstL3swIfRedisMsgAdd);
                    break;
                case MSG_TYPE_IF_CHG:
                    ospf_if_chg_proc(pstL3swIfRedisMsgAdd);
                    break;
                default:
                    break;
            }
        }
        if(NULL != pstL3swIfRedisMsgAdd)
        {
            free(pstL3swIfRedisMsgAdd);
            pstL3swIfRedisMsgAdd = NULL;
        }
        iCnt--;
    }    
}
#endif


#ifdef OSPF_REDISTRIBUTE
int ospf_route_queue_get()
{
    int iRet = OK;
    u_long iCnt = 100;
    static u_long ulCount = 0;
    ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *pstRouteAddMsg = NULL;

    if(g_pstOspfRouteQueue == NULL)
    {
        return ERROR;
    }
    
    while(iCnt>0)
    {
        if(!pqueue_is_empty(g_pstOspfRouteQueue))
        {
            pstRouteAddMsg = (ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *)pqueue_dequeue(g_pstOspfRouteQueue);
            if (pstRouteAddMsg == NULL)
            {
                iCnt--;
                continue;
            }
            if (OSPF_MODE_SLAVE != ospf.work_mode)
            {
                ospf_route_update_from_hw(pstRouteAddMsg);
            }

            if(NULL != pstRouteAddMsg)
            {
                XFREE(MTYPE_MSGQUEUE, pstRouteAddMsg);
                pstRouteAddMsg = NULL;
            }
        }
        
        iCnt--;
    }
    
    return OK;
}
#endif


int ospf_if_link_queue_get()
{
    u_long uiCnt = 50;
    ZEBRA_IF_MSG_REDISTRIBUTE_T *pstIfLinkMsg = NULL;

    if(g_pstOspfIfLinkQueue == NULL)
    {
        return ERROR;
    }
    while(uiCnt>0)
    {
        if(!pqueue_is_empty(g_pstOspfIfLinkQueue))
        {
            pstIfLinkMsg = (ZEBRA_IF_MSG_REDISTRIBUTE_T *)pqueue_dequeue(g_pstOspfIfLinkQueue);
            if (pstIfLinkMsg == NULL)
            {
                uiCnt--;
                continue;
            }
            if(pstIfLinkMsg != NULL)
            {
                ospf_link_chg_proc(pstIfLinkMsg);
                XFREE(MTYPE_MSGQUEUE, pstIfLinkMsg);
                pstIfLinkMsg = NULL;
            }
        }
        uiCnt--;
    }

    return OK;
}

void ospf_timer_msg_send(u_char ucMsgType)
{
	IPC_MSG_T stIpcMsg = {0};

	stIpcMsg.pucBuf = NULL;
	stIpcMsg.ulMsgId = IPC_MSG_QUE_OSPF;
	stIpcMsg.ulMsgType = ucMsgType;
	stIpcMsg.ulMsgLen = sizeof(IPC_MSG_T);
//	printf("%s:%d  ucMsgType=%d\n",__FUNCTION__,__LINE__,ucMsgType);
	vos_msg_queue_send(IPC_MSG_QUE_OSPF,&stIpcMsg,NO_WAIT,MSG_PRI_NORMAL);
	return;	
}


void ospf_timer_msg_10s_send(u_char ucMsgType)
{
	IPC_MSG_T stIpcMsg = {0};

	stIpcMsg.pucBuf = NULL;
	stIpcMsg.ulMsgId = IPC_MSG_QUE_OSPF;
	stIpcMsg.ulMsgType = ucMsgType;
	stIpcMsg.ulMsgLen = sizeof(IPC_MSG_T);

	vos_msg_queue_send(IPC_MSG_QUE_OSPF,&stIpcMsg,NO_WAIT,MSG_PRI_NORMAL);
	return;	
}

int ospf_rx_pkt_head_len(u_char *pchDrvRxPkt)
{
    u_char aucPduBuf[26] = {0},*pPktTmp= NULL;
    u_short usTpId = 0;
    u_short usLen = 0;
    u_short usOverLayLen = 0;

    memcpy(aucPduBuf,pchDrvRxPkt,26);
    pPktTmp = aucPduBuf;
    pPktTmp += 2*MAC_ADDR_LEN;
    usLen += 2*MAC_ADDR_LEN;
    memcpy(&usTpId, pPktTmp, sizeof(u_short));
    usTpId = ntohs(usTpId);
    /*一层vlan*/
    if(usTpId == 0x8100)
    {
        pPktTmp += 2*sizeof(u_short);
        usLen += 2*sizeof(u_short);
    }
    
    memcpy(&usTpId, pPktTmp, sizeof(u_short));
    usTpId = ntohs(usTpId);
    /*二层vlan*/
    if(usTpId == 0x8100)
    {
        pPktTmp += 2*sizeof(u_short);
        usLen += 2*sizeof(u_short);
    }
    usLen += 2;
    return usLen;  
}

#ifdef OSPF_DCN_PACKET /*caoyong delete 2017.9.16*/  /*TODO:待确认ospf dcn的报文收发处理*/

/************************************************* 
   Function:    ospf_rx_dcn_process

   Description: OSPF 报文接收处理
  
   Input:   DRV_ETH_PKT_T   *pstRxPkt 接收报文

   Output: 无 

   Return:  True  操作成功
              False  操作失败
  
   Others:  
       
*************************************************/ 
int ospf_rx_dcn_process(PKT_TX_T *pstRxPkt)
{

    u_char *pPktd = NULL,pucData=NULL;
    u_short usLen = 0,usHeadLen = 0;
    u_long ulIfIndex = 0;
    u_short usVlanId = 0;
    u_long ulLPort = 0;
    u_long ulSecCarrIfIndex = 0;
    u_char ucRole = 0;
    u_char aucPduBuf[1500] = {0};
    u_char ucSorMac[MAC_ADDR_LEN] = {0};
    int ret = 0;
    struct ip *ip = NULL;
    u_int iphlen = 0; 
    u_int source = 0;
    u_int dest = 0;
    u_int ulLportIfIndex = 0;
    u_int ulLagGroupIndex = 0;
    u_int ulFlag = L3_EGRESS_FLAG_PORT;
    u_char aucSelectList[PORTLIST_LENS] = {0};
    
    if (pstRxPkt == NULL)
    {
        return ERR;
    }
    if(DRV_PKT_TYPE_ETH != pstRxPkt->ulType)
    {
	//	printf("ospf_rx_dcn_process that is not ethernet packet!");
		return ERR;
    }

    pPktd = pstRxPkt->pPkt;//报文数据
    usLen = pstRxPkt->ulPktLen;//报文长度
    ulLPort = pstRxPkt->ulLPortNum;//赋值端口号
    ulIfIndex = uspIfLogicPortToIndex(ulLPort);
    if (INVALID == ulIfIndex)
    {
       // printf("11ospf_rx_dcn_process (INVALID == ulIfIndex) ulLPort:%d err\n",ulLPort);
        return ERR;
    }

    if(pstRxPkt->ulSubType == PKT_SUBTYPE_SECTION)
    {

        usVlanId = pstRxPkt->stDrvPktVlan.usVlanIdOut;
        ulLportIfIndex = uspIfLogicPortToIndex(pstRxPkt->ulLPortNum);
        lag_group_port_get_api(ulLportIfIndex, LAG_PORT_GROUP_IFINDEX, &ulLagGroupIndex);
        if(ulLagGroupIndex != INVALID)
        {
            lag_group_get_api(ulLagGroupIndex, LAG_GROUP_SELECT_PORTLIST, aucSelectList);
	        if(TRUE == PBMP_PORT_IS_SET(aucSelectList, pstRxPkt->ulLPortNum))
	        {
	            ulFlag = L3_EGRESS_FLAG_LAG;
	        }
        }
        
        ret = arp_mng_if_sdk_index_get(pstRxPkt->ulLPortNum, usVlanId, ulFlag);

        if((0 == ret)||(ERR == ret))
        {
          //  printf("22ospf_rx_dcn_process usVlanId=%d,ret=%d ulLPort:%d err\n",usVlanId,ret,ulLPort);
            return ERR;
        }
        ulIfIndex = ret;
    }
    if(OSPF_DCN_SUBPORT != uspVpIfToSubId(ulIfIndex))
    {
  //    printf("333 ospf_rx_dcn_process usVlanId=%d,ulIfIndex=0x%x ulLPort:%d err\n",usVlanId,ulIfIndex,ulLPort);
      return;
    }
    usHeadLen = ospf_rx_pkt_head_len(pPktd);
 //   printf("ospf_rx_dcn_process usHeadLen=%d.\r\n",usHeadLen);
    usLen = usLen - usHeadLen;
	memcpy(aucPduBuf, &pPktd[usHeadLen], usLen);
    memcpy(ucSorMac, &pPktd[MAC_ADDR_LEN], MAC_ADDR_LEN);

    ip = (struct ip *)aucPduBuf;
    source = ntohl(ip->ip_src.s_addr);
    dest = ntohl(ip->ip_dst.s_addr);

 //   printf("ospf_rx_dcn_process ulIfIndex=0x%x,source=0x%x.\r\n",ulIfIndex,source);
    ospf_dcn_rx_pkt_addr_get(ulIfIndex,source,ucSorMac);

    
    #if 0
    ospf_input(aucPduBuf, usLen,ulIfIndex);
    #endif
	
    return OK;    
}

/************************************************* 
   Function:    ospf_rx_process

   Description: OSPF 报文接收处理
  
   Input:   DRV_ETH_PKT_T   *pstRxPkt 接收报文

   Output: 无 

   Return:  True  操作成功
              False  操作失败
  
   Others:  
       
*************************************************/ 
int ospf_rx_process(PKT_TX_T *pstRxPkt)
{

    u_char *pPktd = NULL,pucData=NULL;
    u_short usLen = 0;
    u_long ulIfIndex = 0;
    u_short usVlanId = 0;
    u_long ulLPort = 0;
    u_long ulSecCarrIfIndex = 0;
    u_char ucRole = 0;
    u_char aucPduBuf[1500] = {0};
    int ret = 0;
    u_int ulLportIfIndex = 0;
    u_int ulLagGroupIndex = 0;
    u_int ulFlag = 0;
    u_char aucSelectList[PORTLIST_LENS] = {0};
    
    if (pstRxPkt == NULL)
    {
        return ERR;
    }
    if(DRV_PKT_TYPE_ETH != pstRxPkt->ulType)
    {
		//printf("ospf_rx_process that is not ethernet packet!");
		return ERR;
    }

    pPktd = pstRxPkt->pPkt;//报文数据
    usLen = pstRxPkt->ulPktLen;//报文长度
    ulLPort = pstRxPkt->ulLPortNum;//赋值端口号
    ulIfIndex = uspIfLogicPortToIndex(ulLPort);
    if (INVALID == ulIfIndex)
    {
        //printf("11ospf_rx_process (INVALID == ulIfIndex) ulLPort:%d err\n",ulLPort);
        return ERR;
    }

    if(pstRxPkt->ulSubType == PKT_SUBTYPE_SECTION)
    {

        usVlanId = pstRxPkt->stDrvPktVlan.usVlanIdOut;
        ulLportIfIndex = uspIfLogicPortToIndex(pstRxPkt->ulLPortNum);
        lag_group_port_get_api(ulLportIfIndex, LAG_PORT_GROUP_IFINDEX, &ulLagGroupIndex);
        if(ulLagGroupIndex != INVALID)
        {
            lag_group_get_api(ulLagGroupIndex, LAG_GROUP_SELECT_PORTLIST, aucSelectList);
	        if(TRUE == PBMP_PORT_IS_SET(aucSelectList, pstRxPkt->ulLPortNum))
	        {
	            ulFlag = L3_EGRESS_FLAG_LAG;
	        }
        }
        ret = arp_mng_if_knet_index_get(pstRxPkt->ulLPortNum, usVlanId, ulFlag);
        if((0 == ret)||(ERR == ret))
        {
            //printf("22ospf_rx_process usVlanId=%d,ulIfIndex=%d ulLPort:%d err\n",usVlanId,ret,ulLPort);
            return ERR;
        }
        ulIfIndex = ret;
    }

    if(OSPF_DCN_SUBPORT == uspVpIfToSubId(ulIfIndex))
    {
      //printf("333 ospf_rx_process usVlanId=%d,ulIfIndex=0x%x ulLPort:%d err\n",usVlanId,ulIfIndex,ulLPort);
    //  return;
    }
    usLen = usLen -18;
	memcpy(aucPduBuf, &pPktd[18], usLen);
    ospf_input(aucPduBuf, usLen,ulIfIndex);

	
    return OK;    
}

#endif



void ospf_ldp_msg_receive(LDP_MSG_T *pstLdpMsg)
{
    u_int uiType = 0;

    if (pstLdpMsg == NULL)
    {
        return;
    }
    if(pstLdpMsg->uiSessionState == LDP_SNMP_SESSION_STATE_OPERATIONAL)
    {
        uiType = OSPF_LDP_UP_MSG;
    }
    else
    {
        uiType = OSPF_LDP_ERR_MSG;
    }
#if 0
    if (pstLdpMsg->uiSessionState == LDP_MSG_SESSION_STATE_DOWN)
    {
        uiType = OSPF_LDP_ERR_MSG;
    }
    else if (pstLdpMsg->uiSessionState == LDP_MSG_SESSION_STATE_INIT)
    {
        uiType = OSPF_LDP_INIT_MSG;
    }
    else
    {
        uiType = OSPF_LDP_UP_MSG;
    }
#endif
    ospf_ldp_control(pstLdpMsg->uiIfIndex, uiType);
}


void ospf_recv_task()
{
    int ret = OK;
    UINT32 modFrom = 0;
    UINT32 msgType = 0;
    IPC_MSG_T msg;
    char acBuf[1500];
    ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *pstL3IfRedisMsg = NULL;
	u_long ulIp = 0;

	#ifdef OSPF_DCN_PACKET /*caoyong delete 2017.9.16*/  /*TODO:待确认ospf dcn的报文收发处理*/
	PKT_TX_T *pstRxPkt = NULL;


    memset(acBuf, 0, sizeof(acBuf));
    msg.pucBuf = NULL;
    msg.ulMsgLen = 3*sizeof(u_int) + sizeof(acBuf);/*接收消息，要填最大长度，包含3个uint*/

	while(1)
    {
        //taskDelay(10);
        ret = msgQReceive(IPC_MSG_QUE_OSPF_REV, &msg, WAIT_FOREVER);
        if(ret <= 0)
        {
            continue;
        }
        modFrom = msg.ulMsgId;
        msgType = msg.ulMsgType;

        vos_pthread_lock(&ospf.Lock);
        
        switch(modFrom)
        {
        	
            case IPC_MSG_QUE_DRV:
            {
                switch(msgType)
                {
                    case MSG_TYPE_PKT_RECV:
                    {
                        pstRxPkt = msg.pucBuf;
                        ospf_rx_process(pstRxPkt);
                        if(NULL != pstRxPkt->pPkt)     /*释放空间*/
                        {
                            free(pstRxPkt->pPkt);
                            pstRxPkt->pPkt = NULL;
                        }
                        break;
                    }
                    
                    default:
                    {
                        break;
                    }
                }
                break;
            }
            
            default:
                break;
        }
        if(NULL != msg.pucBuf)
        {
            free(msg.pucBuf);
            msg.pucBuf = NULL;
        }
        vos_pthread_unlock(&ospf.lock_sem);
    }
    #endif
}

void ospf_config_load_done_process()
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    static u_int uiLoadDone = 0;

    if(uiCfgLoadFlag == 2 && uiLoadDone != uiCfgLoadFlag)
    {
        for_each_ospf_process(p_process, p_next_process)
        {
            if(p_process->process_id != OSPF_DCN_PROCESS)
            {
                if(p_process->routerid_flg != ENABLE)
                {
                    p_process->old_routerid = p_process->router_id;
                    p_process->new_routerid = ospf_select_router_id(p_process->vrid);
                    //printf("ospf process %d config load done over, reset ospf process\n",p_process->process_id);
                    ospf_timer_start(&p_process->id_reset_timer, 1);
                }
            }
        }
        uiLoadDone = uiCfgLoadFlag;
    }
}

void ospf_task()
{
    int ret = OK;
    UINT32 modFrom = 0;
    UINT32 msgType = 0;
    IPC_MSG_T msg;
    ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *pstL3IfRedisMsg = NULL;
    #if 0
	PKT_TX_T *pstRxPkt = NULL;
	#endif
    int iTimeStart = 0;
    int iTimeEnd = 0;

#ifdef OSPF_MASTER_SLAVE_SYNC
	SYN_MSG_T *pstMsgInfo = NULL;
#endif
    LDP_MSG_T *pstLdpMsg = NULL;

    vos_wdog_create_start(WDOG_PERIOD, WDOG_1S,(FUNCPTR)ospf_timer_msg_send,MSG_TYPE_1S);

    msg.pucBuf = NULL;
    msg.ulMsgLen = 3*sizeof(u_int) + 1500;/*接收消息，要填最大长度，包含3个uint*/

    /*reg l3 interface event msg*/
    #if 0/*caoyong delete 2017.9.15 */

    /*TODO:后续需要修改添加注册*/
#ifdef OSPF_L3QUEUE_NEW
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_LINK_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_UP, ospf.pstL3RedisQueue);
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_LINK_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_DOWN, ospf.pstL3RedisQueue);
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_IP_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_ADDR_ADD, ospf.pstL3RedisQueue);
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_IP_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_ADDR_DEL, ospf.pstL3RedisQueue);
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_IF_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_ADD, ospf.pstL3RedisQueue);
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_IF_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_DEL, ospf.pstL3RedisQueue);
#else
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_LINK_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_UP);
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_LINK_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_DOWN);
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_IP_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_ADDR_ADD);
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_IP_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_ADDR_DEL);
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_IF_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_ADD);
    rib_if_route_redistribute_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_IF_CHG, L3_ROUTE_REDISTRIBUTE_TYPE_IF_DEL);
#endif
#ifdef OSPF_MASTER_SLAVE_SYNC
    dev_master_slave_ntf_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_SYN_WORK_STAT_CHG);
	//dev_master_slave_ntf_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_SYN_SLV_START_MSG);
    //dev_master_slave_ntf_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_SYN_NEGO_DONE_MSG);
#endif
#ifdef OSPF_REDISTRIBUTE
    l3_rt_redistribute_reg(g_pstOspfRouteQueue);
#endif
    ldp_session_state_chg_reg(IPC_MSG_QUE_OSPF, MSG_TYPE_LDP_SESSION_CHG);
#ifdef OSPF_VPN
    l3vpn_status_chg_ntf_reg(IPC_MSG_QUE_OSPF,MSG_TYPE_L3VPN_CHG);
#endif

#endif
	while(1)
    {
        //taskDelay(10);
        ret = vos_msg_queue_receive(IPC_MSG_QUE_OSPF, &msg, WAIT_FOREVER);
        if(ret <= 0)
        {
            continue;
        }
        modFrom = msg.ulMsgId;
        msgType = msg.ulMsgType;

        switch(modFrom)
        {
        	case IPC_MSG_QUE_OSPF:
                switch(msgType)
			    {
	                case MSG_TYPE_1S:
	                {
    					vos_pthread_lock(&ospf.lock_sem);
    					ospf_config_load_done_process();
    					ospf_thread_check();
    					vos_pthread_unlock(&ospf.lock_sem);
    					#ifdef OSPF_REDISTRIBUTE
    					ospf_route_queue_get();
    					#endif
    					#ifdef OSPF_L3QUEUE_NEW
    					// ospf_l3_redis_queue_get();
    					#endif
    					vos_pthread_lock(&ospf.lock_sem);
    					ospf_if_link_queue_get();
    					vos_pthread_unlock(&ospf.lock_sem);
    					break;
				    } 	
	                #if 0
	                case MSG_TYPE_10S:
				    {
					    ospf_dcn_recv_count();
					    break;
				    }
				    #endif
				    case MSG_TYPE_IP_CHG:
    				    //printf("process_id = %d cmd = %d\n",msg.pucBuf[0],msg.pucBuf[1]);
    				    ospf_update_route_for_pre_chg(msg.pucBuf[0],msg.pucBuf[1]);
    				    break;
    				case MSG_TYPE_SESSION_CHG:
                    {
                        if (OSPF_MODE_SLAVE == ospf.work_mode)
                        {
                            break;
                        }
                        pstLdpMsg = (LDP_MSG_T *)msg.pucBuf;
                        ospf_ldp_msg_receive(pstLdpMsg);
                        break;
                    }    
                    default:
					{
						break;
					}
			    }
				
			    break;
            #if 0 /*caoyong delete 2017.9.16*/  /*TODO:待确认ospf dcn的报文收发处理*/
            case IPC_MSG_QUE_DRV:
            {
                switch(msgType)
                {
                    case MSG_TYPE_PKT_RECV:
                    {
                        pstRxPkt = msg.pucBuf;
                        //ospf_rx_dcn_process(pstRxPkt);
                        if(NULL != pstRxPkt->pPkt)     /*释放空间*/
                        {
                            free(pstRxPkt->pPkt);
                            pstRxPkt->pPkt = NULL;
                        }
                        break;
                    }
                    
                    default:
                    {
                        break;
                    }
                }
                break;
            }
            #endif          
        #ifndef OSPF_L3QUEUE_NEW
             case IPC_MSG_QUE_L3:
                    if (OSPF_MODE_SLAVE == ospf.work_mode)
                    {
                        break;
                    }
                    switch(msgType)
                    {
                        case MSG_TYPE_LINK_CHG:
                            pstL3IfRedisMsg = (ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *)msg.pucBuf;
                            ospf_link_chg_proc(pstL3IfRedisMsg);
                #if 0
                            if(NULL != pstL3IfRedisMsg)
                            {
                                free(pstL3IfRedisMsg);
                                pstL3IfRedisMsg = NULL;
                            }
                #endif
                            break;
                        case MSG_TYPE_IP_CHG:
                            pstL3IfRedisMsg = (ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *)msg.pucBuf;
                            ospf_ip_chg_proc(pstL3IfRedisMsg);
                #if 0
                            if(NULL != pstL3IfRedisMsg)
                            {
                                free(pstL3IfRedisMsg);
                                pstL3IfRedisMsg = NULL;
                            }
                #endif
                            break;
                        case MSG_TYPE_IF_CHG:
                            pstL3IfRedisMsg = (ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *)msg.pucBuf;
                            ospf_if_chg_proc(pstL3IfRedisMsg);
                            break;
                        default:
                            break;
                    }
                    break;
        #endif
        #ifdef OSPF_MASTER_SLAVE_SYNC
            case IPC_MSG_QUE_PROC:
                switch(msgType)
				{
				    /*由备切换为主时触发GR*/
					case MSG_TYPE_SYN_WORK_STAT_CHG:
                        zlog_notice(MTYPE_SYN,"   ospf receive workStat msg!");
						pstMsgInfo = (SYN_MSG_T *)msg.pucBuf;
					//	printf("%s:%d:ospf_master_slave_state_chg_proc\n",__FILE__,__LINE__);
                        ospf_master_slave_state_chg_proc(pstMsgInfo->uiWorkStat);
                        zlog_notice(MTYPE_SYN,"   ospf msg workStat proc end! workStat : %d",pstMsgInfo->uiWorkStat);
						break;
					
					/*主备板卡数据同步*/
					case MSG_TYPE_SYN_NEGO_DONE_MSG:
                        zlog_notice(MTYPE_SYN,"   ospf receive negoDone msg!");
						//ospf_slaver_card_up();
                        zlog_notice(MTYPE_SYN,"   ospf msg negoDone not need proc!");
						break;
					default:
						break;
				}
				break;
		#endif
		#ifdef OSPF_VPN
		    case IPC_MSG_QUE_L3VPN:
                ospf_vpn_cfg_chg_proc(&msg);
                break;
        #endif
            default:
                break;
        }
        if(NULL != msg.pucBuf)
        {
            XFREE(MTYPE_MSGQUEUE, msg.pucBuf);
            msg.pucBuf = NULL;
        }
    }
}
#endif


