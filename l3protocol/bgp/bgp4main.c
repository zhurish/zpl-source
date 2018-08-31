#include "bgp4_api.h"
//#include "routepolicy_api.h"

#ifdef NEW_BGP_WANTED
#include "bgp4com.h"

/*通过IP_IP_X_VRID设置socket的vrfid*/
tBGP4_GLOBAL gbgp4;
struct pqueue *g_pstBgpRouteQueue = NULL;
struct pqueue *g_pstBgpIfLinkQueue = NULL;
struct pqueue *g_pstBgpDelLabelQueue = NULL;

void *bgpMalloc(u_int uiModule, u_int len)
{
    return vos_xmalloc(__FILE__, __LINE__, uiModule, len);
}

void bgpFree(void *ptr)
{
    vos_xfree(MTYPE_BGP, ptr);
}

int 
bgp4_global_init(
     u_int max_route, 
     u_int max_path, 
     u_int max_peer,
     u_int max_vpn_instance,
     u_int max_l2vpn_instance,
     u_int ipv6pe_enable,
     void *func1,
     void *func2)
{    
    return bgp4_global_initEx(max_route, max_path,
               max_peer, max_vpn_instance, 
               max_l2vpn_instance,
               ipv6pe_enable,
               func1, func2, bgpMalloc, bgpFree);
}

/*called only once when system up*/
int 
bgp4_global_initEx(
       u_int max_route, 
       u_int max_path, 
       u_int max_peer,
       u_int max_vpn_instance,
       u_int max_l2vpn_instance,
       u_int ipv6pe_enable,
       void *func1,
       void *func2, 
       void *(*mAlloc)(u_int, u_int), 
       void (*mFree)(void *))
{
    int i = 0;
    
    memset(&gbgp4, 0, sizeof(tBGP4_GLOBAL));

    /*init some resource need*/ 

    gbgp4.ipv6pe_enable = ipv6pe_enable;
        
    gbgp4.max_peer = max_peer ? max_peer : BGP4_DEFAULT_PEER_NUMBER;
    gbgp4.max_path = max_path ? max_path : BGP4_DEFAULT_PATH_NUMBER;
    gbgp4.max_route = max_route ? max_route : BGP4_DEFAULT_ROUTE_NUMBER;
    gbgp4.max_vpn_instance = max_vpn_instance;
    gbgp4.max_l2vpn_instance = max_l2vpn_instance;
    
    gbgp4.tick_per_second = vos_get_clock_rate();
    gbgp4.server_port = BGP4_SERVER_PORT;

    for(i = 0; i < L3_VPN_INSTANCE_NUM + 1; i++)
    {
        gbgp4.server_sock[i] = BGP4_INVALID_SOCKET;
        gbgp4.server_sock6[i] = BGP4_INVALID_SOCKET;
    }
    
    gbgp4.local_pref = BGP4_DEFAULT_LOCALPREF;    
    gbgp4.max_update_len = BGP4_DEFAULT_UPDATE_LIMIT;
    gbgp4.tcp_server_enable = TRUE;
    gbgp4.tcp_client_enable = TRUE;
        
    gbgp4.in_restarting = FALSE;
    
    gbgp4.work_mode = BGP4_MODE_OTHER;

    gbgp4.restart_period = BGP4_DFLT_GRTIME;
    gbgp4.restart_wait_period = BGP4_DFLT_DEFERRALTIME;

    /*initiate lists*/    
    bgp4_unsort_avl_init(&gbgp4.ecom_table);
    
    bgp4_avl_init(&gbgp4.instance_table, bgp4_instance_lookup_cmp);
        
    /*initiate semaphore*/
    vos_pthread_lock_init(&gbgp4.sem);
    bgp4_attribute_desc_init();

    gbgp4.policy_func = func1;
    gbgp4.policy_ref_func = func2;    

    gbgp4.malloc = mAlloc;
    gbgp4.mfree = mFree;

    /*allocate global rxbuf.64K*/
    gbgp4.p_rxbuf = (u_char *)gbgp4.malloc(MTYPE_BGP,BGP4_RXBUF_LEN);
    if (gbgp4.p_rxbuf)
    {
        memset(gbgp4.p_rxbuf, 0, BGP4_RXBUF_LEN);
    }
    
    bgp4_timerlist_init();
    #if 0
    /*create damon task*/
    if (OsixCreateTask(BGP4_TASKNAME, BGP4_TASK_PRIORITY, BGP4_TASK_DEF_DEPTH, 
        bgp4_task_main, 0, 
        OSIX_DEFAULT_TASK_MODE, &tid) == OSIX_FAILURE)
    {
        printf("\r\nbgp4 Task create failed,bgp4 can not start!");
    }    
    #endif

    /*init sock id stored fd-set*/
    gbgp4.fd_read = (fd_set *)gbgp4.malloc(MTYPE_BGP, sizeof(fd_set));
    memset(gbgp4.fd_read, 0, sizeof(fd_set));
    
    gbgp4.fd_write = (fd_set *)gbgp4.malloc(MTYPE_BGP, sizeof(fd_set));    
    memset(gbgp4.fd_write, 0, sizeof(fd_set));
    
    /*init all timers*/
    bgp4_timer_init(&gbgp4.gr_waiting_timer, bgp4_restart_waiting_timeout, NULL);
    bgp4_timer_init(&gbgp4.workmode_timer, bgp4_workmode_update_timeout, NULL);
    bgp4_timer_init(&gbgp4.igp_sync_timer, bgp4_igp_sync_timeout, NULL);
    bgp4_timer_init(&gbgp4.init_update_timer, bgp4_init_update_timeout, NULL);
    bgp4_timer_init(&gbgp4.init_sync_timer, bgp4_sync_init_send, NULL);
    bgp4_timer_init(&gbgp4.update_ratelimit_timer, bgp4_update_limit_timeout, NULL);

    /*start some timer*/
    bgp_sem_take();
    
    bgp4_timer_start(&gbgp4.workmode_timer, BGP4_WORKMODE_INTERVAL); 
    bgp4_timer_start(&gbgp4.igp_sync_timer, BGP4_IGP_SYNC_MAX_INTERVAL); 

    if(vos_msg_queue_create(BGP4_MAX_MSG,BGP4_MSG_SIZE,MSG_Q_FIFO,IPC_MSG_QUE_BGP)!= VOS_OK)
    {
        bgp4_log(BGP_DEBUG_EVT,"create bgp msg queue failed\n");
    }

    bgp_sem_give();    
    return TRUE;
}

tBGP4_VPN_INSTANCE * 
bgp4_vpn_instance_lookup(
              u_int type,
              u_int vrf)
{
    tBGP4_VPN_INSTANCE instance;
    instance.type = type;
    instance.vrf = vrf;
    return bgp4_avl_lookup(&gbgp4.instance_table, &instance);
}

int
bgp4_instance_lookup_cmp(
         tBGP4_VPN_INSTANCE *p1,
         tBGP4_VPN_INSTANCE *p2)
{
    if (p1->vrf != p2->vrf)
    {
        return (p1->vrf > p2->vrf) ? 1 : -1;
    }
    /*vrf 0 is public ip instance,no type compare need*/
    if (p1->vrf == 0)
    {
        return 0;
    }
    
    if (p1->type != p2->type)
    {
        return (p1->type > p2->type) ? 1 : -1;
    }
    return 0;
}

int bgp4_timer_init(tTimerNode *tmr, void *func, u_long arg)
{
    tmr->pFunc = (TIMERFUNCPTR)(func);
    tmr->arg1 = (arg);
    tmr->arg2 = NULL;
    tmr->arg3 = NULL;

    return VOS_OK;
}

tBGP4_RIB *
bgp4_rib_create(
    tBGP4_VPN_INSTANCE *p_instance,
    u_int af)
{
    tBGP4_RIB *p_rib = bgp4_malloc(sizeof(*p_rib), MEM_BGP_RIB);
    if (p_rib == NULL)
    {
        return NULL;
    }
    
    p_instance->rib[af] = p_rib;
    p_rib->af = af;
    p_rib->p_instance = p_instance;
    
    bgp4_route_table_init(&p_rib->rib_table);
    
    bgp4_route_table_init(&p_rib->rib_in_table);

    bgp4_unsort_avl_init(&p_rib->path_table);

    bgp4_unsort_avl_init(&p_rib->out_withdraw_table);
    
    bgp4_unsort_avl_init(&p_rib->out_feasible_table);

    bgp4_avl_init(&p_rib->damp_table, bgp4_damp_route_lookup_cmp);

    bgp4_timer_init(&p_rib->rib_in_timer, bgp4_rib_in_check_timeout, p_rib);

    bgp4_timer_init(&p_rib->system_timer, bgp4_rib_system_update_timeout, p_rib);

    bgp4_timer_init(&p_rib->path_timer, bgp4_path_clear_timeout, p_rib); 

    bgp4_timer_init(&p_rib->nexthop_timer, bgp4_nexthop_check_timout, p_rib); 
    
    bgp4_timer_init(&p_rib->route_clear_timer, bgp4_unused_route_clear, p_rib); 

    bgp4_timer_start(&p_rib->path_timer, BGP4_PATH_CLEAR_INTERVAL);

    bgp4_timer_start(&p_rib->system_timer, 1);

    bgp4_timer_start(&p_rib->nexthop_timer, BGP4_IGP_SYNC_MAX_INTERVAL); 
    return p_rib;
}

void
bgp4_rib_delete(tBGP4_RIB *p_rib)
{
    bgp4_route_table_flush(&p_rib->rib_table);
    
    bgp4_route_table_flush(&p_rib->rib_in_table);

    bgp4_path_clear_timeout(p_rib);

    bgp4_damp_table_flush(p_rib);
    
    bgp4_timer_stop(&p_rib->rib_in_timer);

    bgp4_timer_stop(&p_rib->system_timer);

    bgp4_timer_stop(&p_rib->path_timer);

    bgp4_timer_stop(&p_rib->nexthop_timer);

    bgp4_timer_stop(&p_rib->route_clear_timer);
    
    if (p_rib->p_hwmsg)
    {
        bgp4_free(p_rib->p_hwmsg, MEM_BGP_BUF);
        p_rib->p_hwmsg = NULL;
    }
    /*withdraw/feasible table????*/
    bgp4_free(p_rib, MEM_BGP_RIB);
    return;
}

tBGP4_VPN_INSTANCE *
bgp4_vpn_instance_create(
               u_int type,
               u_int vrf)
{
    tBGP4_VPN_INSTANCE *p_new = NULL;
    u_int i = 0;
    
    p_new = bgp4_malloc(sizeof(tBGP4_VPN_INSTANCE), MEM_BGP_VPN_INSTANCE);
    if (p_new == NULL) 
    {
        return NULL;
    }
    p_new->type = type;
    p_new->vrf = vrf;
    /*initiate lists*/    
    bgp4_avl_init(&p_new->peer_table, bgp4_peer_lookup_cmp);
    bgp4_avl_init2(&p_new->peer_index_table, bgp4_peer_index_lookup_cmp, sizeof(p_new->node));
    bgp4_avl_init(&p_new->network_table, bgp4_network_lookup_cmp);
    bgp4_avl_init(&p_new->range_table, bgp4_range_lookup_cmp);

    /*allocate all rib tables*/
    for (i = 0 ; i < BGP4_PF_MAX ; i++)
    {
        /*vpn instance,only consider normal af*/
        if (vrf != 0)
        {
            if ((type == BGP4_INSTANCE_IP) && (i != BGP4_PF_IPUCAST))
            {
                continue;
            }
            if ((type == BGP4_INSTANCE_VPLS) && (i != BGP4_PF_L2VPLS))
            {
                continue;
            }
        }
        bgp4_rib_create(p_new, i);        
    }
    /*policy list*/
    bgp4_avl_init(&p_new->policy_table, bgp4_policy_lookup_cmp);
    bgp4_avl_init(&p_new->redistribute_policy_table, bgp4_redistribute_policy_lookup_cmp);

    /*start range update timer*/
    bgp4_timer_init(&p_new->range_update_timer, bgp4_range_update_timeout, p_new);

    bgp4_timer_init(&p_new->delete_timer, bgp4_vpn_instance_delete_timeout, p_new);    

    bgp4_timer_start(&p_new->range_update_timer, 1);

    bgp_relate_vpn_instance_process(p_new->vrf, L3VPN_REFERENCE_ADD);
	
    /*sorted by vrf id*/
    bgp4_avl_add(&gbgp4.instance_table, p_new);

    /*originate default upe route if necessary*/
    bgp4_upe_default_route_update(p_new);

    bgp4_server_sock_open(vrf);
    
    return p_new;
}
/*delete instance data from instance list,called when vpn instance is unconfiged*/
void 
bgp4_vpn_instance_del(tBGP4_VPN_INSTANCE *p_instance)
{
    tBGP4_RIB *p_rib = NULL;
    u_int i = 0;
    
    /*close all peer*/
    bgp4_avl_walkup(&p_instance->peer_table, bgp4_fsm_invalid);

    /*delete all peer*/
    bgp4_avl_walkup(&p_instance->peer_table, bgp4_peer_delete);

    bgp4_network_delete_all(p_instance);

    bgp4_range_table_flush(p_instance);

    bgp4_policy_delete_all(&p_instance->policy_table);

    for (i = 0 ; i < BGP4_PF_MAX ; i++)
    {
        p_rib = p_instance->rib[i];
        if (p_rib)
        {
            bgp4_rib_delete(p_rib);
        }
        p_instance->rib[i] = NULL;
    }

    /*delete redistributed configuration*/
    bgp4_avl_walkup(&p_instance->redistribute_policy_table, 
              bgp4_redistribute_policy_delete);
    
    bgp4_timer_stop(&p_instance->range_update_timer);
    
    bgp4_avl_delete(&gbgp4.instance_table, p_instance);
    bgp_relate_vpn_instance_process(p_instance->vrf, L3VPN_REFERENCE_DEL);

    bgp4_server_sock_close(p_instance->vrf);
    
    bgp4_free(p_instance, MEM_BGP_VPN_INSTANCE);
    
    return;
}

void 
bgp4_vpn_instance_delete_timeout(tBGP4_VPN_INSTANCE *p_instance)    
{    
    u_int af = 0;
           
    /*if any rib exist,do not delete*/
    for (af = 0; af < BGP4_PF_MAX; af++)
    {
        if (p_instance->rib[af] == NULL)
        {
            continue;
        }
        if (bgp4_avl_count(&p_instance->rib[af]->rib_table)
           || bgp4_avl_count(&p_instance->rib[af]->rib_in_table))
        {
            bgp4_timer_start(&p_instance->delete_timer, 1);
            return;
        }
        /*all msg must be sent*/
        if (p_instance->rib[af]->p_hwmsg)
        {
            bgp4_timer_start(&p_instance->delete_timer, 1);
            return;
        }
    }
    bgp4_vpn_instance_del(p_instance);
    return;
}

/*start BGP protocol*/
int 
bgp4_enable(u_int asnum)  
{    
    int proto = M2_ipRouteProto_bgp;

    if ((gbgp4.as > 0) || (asnum == 0))
    {
        bgp4_log(BGP_DEBUG_EVT,"bgp4 can not start! as num %d error\n", asnum);
        return VOS_ERR;
    }
    /*reset stat*/
    memset(&gbgp4.stat, 0, sizeof(gbgp4.stat));
    
    /*create route socket*/    
    //bgp4_rtsock_init();

    /*create tcp listen port*/
    //bgp4_server_sock_open(0);

    bgp4_mem_init(gbgp4.max_route, gbgp4.max_path, gbgp4.max_peer, gbgp4.max_vpn_instance + 1);

    /*create public instance,id 0*/
    if (bgp4_vpn_instance_create(BGP4_INSTANCE_IP, 0) == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,"bgp fail to create public instance !!!");
        return VOS_ERR;
    }
    
    /*get local vpn conf,create configured vpn instances*/
    bgp4_up_notify();
    
    vos_pthread_delay(10);

    /*system enable for BGP*/
    //uspScalarSetApi(NULL, SYS_PROTO_ENABLE, &proto); /*caoyong delete 2017.9.28*/

    /*get local vpn conf,create configured vpn instances*/
    //bgp4_init_private_instance();

    /*auto calculate router id*/
    bgp4_set_router_id(0);

    /*enable to process*/
    gbgp4.enable = TRUE;
    
    gbgp4.as = asnum;

    gbgp4.debug |= BGP_DEBUG_FSMCHANGE;

    /*damp params inited*/
    gbgp4.penalty_half_time = BGP4_PENALTY_HALF_TIME;
    gbgp4.penalty_max = BGP4_PENALTY_MAX;
    gbgp4.penalty_supress = BGP4_PENALTY_SUPRESS;
    gbgp4.penalty_reuse = BGP4_PENALTY_REUSE;

    gbgp4.uiPeerCount = 0;
    gbgp4.ucEbgpPre = 255;
    gbgp4.ucIbgpPre = 255;
    gbgp4.ucLocalPre = 255;
    
    return VOS_OK;
} 

int 
bgp4_disable(void) 
{        
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_VPN_INSTANCE *p_next = NULL;    
    tBGP4_PEER *p_peer = NULL;
    tBGP4_RIB *p_rib = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE delete_route;
    int proto = M2_ipRouteProto_bgp;        
    u_int count = 0 ;
    u_int af = 0;
    int i = 0;
    
    /*close all rtsock option*/
    for (af = 0; af < M2_ipRouteProto_max ; af++)
    {
        rtSockOptionSet(gbgp4.rtsock, af, FALSE);
    }
    
    printf("The BGP is to be released,please wait\n\r");    
    bgp4_instance_for_each_safe(p_instance, p_next)
    {
        /*first stop all peer in instance*/
        bgp4_avl_for_each(&p_instance->peer_table, p_peer)
        {
            bgp4_fsm(p_peer, BGP4_EVENT_STOP);
        }

        bgp4_avl_walkup(&p_instance->range_table, bgp4_range_down);
        
        /*wait for routing updated*/
        for (af = 0 ; af < BGP4_PF_MAX ; af++)
        {            
            p_rib = p_instance->rib[af];
            if (p_rib == NULL)
            {
                continue;
            }
            /*remove all route in vpls*/
            bgp4_avl_for_each(&p_rib->rib_table, p_route)
            {
                if (p_route->is_deleted == TRUE)
                {
                    continue;
                }

                /*build route to be deleted*/
                memset(&delete_route, 0, sizeof(delete_route));
                memcpy(&delete_route.dest, &p_route->dest, sizeof(tBGP4_ADDR));
                delete_route.proto = p_route->proto;
                delete_route.is_deleted = TRUE;
                delete_route.p_path = p_route->p_path;                
                
                /*notify delete to public instance*/
                if (p_route->p_path->origin_vrf == p_instance->vrf)
                {
                    bgp4_vrf_route_export_check(p_route, FALSE);
                }
                bgp4_rib_in_table_update(&delete_route);
            }

            do {
                /*try to process any route in rib table*/ 
                bgp4_rib_in_check_timeout(p_rib);
                
                while (p_rib->system_check_need == TRUE)
                {
                   bgp4_rib_system_update_check(p_rib);
                   vos_pthread_delay(1);
                   if (count++ >= vos_get_clock_rate())
                   {
                       count = 0;
                       printf(". ");
                   } 
                   bgp4_rtsock_recv();
                }
                /*send msg for delete*/
                while (p_rib->p_hwmsg)
                {
                    if (bgp4_sys_msg_send(p_rib->p_hwmsg) != VOS_OK)
                    {
                        vos_pthread_delay(1);
                        if (count++ >= vos_get_clock_rate())
                        {
                            count = 0;
                            printf(". ");
                        } 
                        bgp4_rtsock_recv();
                    }
                    else
                    {
                        bgp4_free(p_rib->p_hwmsg, MEM_BGP_BUF);
                        p_rib->p_hwmsg = NULL;
                    }
                }
            }while(bgp4_avl_count(&p_rib->rib_in_table));
        }
        /*all route update finished,delete instance here*/
        bgp4_vpn_instance_del(p_instance);
    }

    bgp4_server_sock_close(0); 
    
    bgp4_rtsock_close();
    
    bgp4_confedration_as_flush();/*IPSOFT cheng add for confederation*/

    bgp4_delete_all_ext_comm();

    gbgp4.enable = FALSE;
    gbgp4.as = 0;
    gbgp4.router_id = 0;
    gbgp4.confedration_id = 0;    
    gbgp4.igp_sync_enable = FALSE; 
    for(i = 0; i < L3_VPN_INSTANCE_NUM + 1; i++)
    {
        gbgp4.server_sock[i] = BGP4_INVALID_SOCKET;
        gbgp4.server_sock6[i] = BGP4_INVALID_SOCKET;
    }
    gbgp4.local_pref = BGP4_DEFAULT_LOCALPREF;  
    gbgp4.med = BGP4_DEFAULT_MED;
    gbgp4.reflector_enable = 0;
    gbgp4.cluster_id = 0;
    gbgp4.community_action = 0;
    gbgp4.trap_enable = 0;
    gbgp4.damp_enable = 0;
    gbgp4.restart_enable = FALSE;
    gbgp4.max_update_len = BGP4_DEFAULT_UPDATE_LIMIT;
    gbgp4.tcp_server_enable = TRUE;
    gbgp4.tcp_client_enable = TRUE;    
    gbgp4.restart_period = BGP4_DFLT_GRTIME;
    gbgp4.restart_wait_period = BGP4_DFLT_DEFERRALTIME;
    gbgp4.debug = 0;
    gbgp4.as4_enable = FALSE;  
    /*damp params inited*/
    gbgp4.penalty_half_time = BGP4_PENALTY_HALF_TIME;
    gbgp4.penalty_max = BGP4_PENALTY_MAX;
    gbgp4.penalty_supress = BGP4_PENALTY_SUPRESS;
    gbgp4.penalty_reuse = BGP4_PENALTY_REUSE;
    gbgp4.ipv4_vpnv4 = FALSE;
    gbgp4.uiPeerCount = 0;
    gbgp4.BgpRouteIdFlag = 0;
    vos_pthread_delay(10);

    /*clear sync fragment pdu*/
    bgp4_sync_fragment_pdu_clear();
    
    bgp4_mem_deinit();

    printf("BGP release finished\n");
    return VOS_OK;
}

/*set sock fdset and select*/
int 
bgp4_socket_select(
      fd_set *p_rfd,
      fd_set *p_wfd)
{
    struct timeval tv;
    int fdmax = 0;
    int i = 0;
    
    /*waiting time is 100ms*/
    tv.tv_sec = 0;
    tv.tv_usec =100000;/*100 ms*/

    /*copy current fd set and max value*/
    if (gbgp4.work_mode != BGP4_MODE_SLAVE)
    {
        memcpy(p_rfd, gbgp4.fd_read, sizeof(fd_set));
        memcpy(p_wfd, gbgp4.fd_write, sizeof(fd_set));
        fdmax = gbgp4.max_sock;

        for(i = 0; i < L3_VPN_INSTANCE_NUM + 1; i++)
        {
            if (gbgp4.tcp_server_enable && (gbgp4.server_sock[i] > 0))
            {
                FD_SET(gbgp4.server_sock[i], p_rfd);
                fdmax = BGP4_MAX(fdmax, gbgp4.server_sock[i]);
            }
        
            if (gbgp4.tcp_server_enable && (gbgp4.server_sock6[i] > 0))
            {
                FD_SET(gbgp4.server_sock6[i] , p_rfd);
                fdmax = BGP4_MAX(fdmax, gbgp4.server_sock6[i]);
            }
        }
    }
    else
    {
        FD_ZERO(p_rfd);
        FD_ZERO(p_wfd);
    }
    /*route socket*/
    if (gbgp4.rtsock > 0)
    {
        FD_SET(gbgp4.rtsock, p_rfd);
        fdmax = BGP4_MAX(fdmax, gbgp4.rtsock);
    }
    
    /*if no socket found,do nothing*/
    if (fdmax == 0)
    {
        vos_pthread_delay(gbgp4.tick_per_second);
        return VOS_OK;
    }
    /*do select operation*/
    return select(fdmax + 1, p_rfd, p_wfd, 0, &tv);
}


void bgp4_send_msg()
{
    IPC_MSG_T stIpcMsg = {0};
    int iRet = VOS_OK;

    stIpcMsg.ulMsgId = IPC_MSG_QUE_BGP;
    stIpcMsg.ulMsgType = MSG_TYPE_50MS;
    stIpcMsg.ulMsgLen = sizeof(IPC_MSG_T);

    iRet = vos_msg_queue_send(IPC_MSG_QUE_BGP,&stIpcMsg,NO_WAIT,MSG_PRI_NORMAL);
    
    if(iRet != VOS_OK)
    {
		//printf("bgp4_send_msg ret=%d !\n", iRet);
    }

    return;
}
#if 1
int route_policy_set_api(char *pcName, int cmd, void *var)
{

}
#endif
int bgp4_init()
{
    return bgp4_global_init(0,0,BGP_MAX_PEER_NUM,L3_VPN_INSTANCE_NUM,0,0,route_policy_set_api,NULL);
}

int bgp4_route_queue_get()
{
    u_long uiCnt = BGP4_MAX_MSG;
    static u_long ulCount = 0;
    ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *pstRouteAddMsg = NULL;

    if(g_pstBgpRouteQueue == NULL)
    {
        return VOS_ERR;
    }
    
    ulCount++;
    if(ulCount%5 != 0)
    {
        return VOS_OK;
    }
    
    while(uiCnt>0)
    {
        if(!pqueue_is_empty(g_pstBgpRouteQueue))
        {
            pstRouteAddMsg = (ZEBRA_ROUTE_MSG_REDISTRIBUTE_T *)pqueue_dequeue(g_pstBgpRouteQueue);
            if (pstRouteAddMsg == NULL)
            {
                uiCnt--;
                continue;
            }
            
            if(pstRouteAddMsg != NULL)
            {
                bgp_redistribute_route_change_process(pstRouteAddMsg);
                XFREE(MTYPE_MSGQUEUE, pstRouteAddMsg);
                pstRouteAddMsg = NULL;
            }
        }
        uiCnt--;
    }

    return VOS_OK;
}

int bgp4_if_link_queue_get()
{
    u_long uiCnt = BGP4_MAX_MSG;
    ZEBRA_IF_MSG_REDISTRIBUTE_T *pstIfLinkMsg = NULL;

    if(g_pstBgpIfLinkQueue == NULL)
    {
        return VOS_ERR;
    }
    
    while(uiCnt>0)
    {
        if(!pqueue_is_empty(g_pstBgpIfLinkQueue))
        {
            pstIfLinkMsg = (ZEBRA_IF_MSG_REDISTRIBUTE_T *)pqueue_dequeue(g_pstBgpIfLinkQueue);
            if (pstIfLinkMsg == NULL)
            {
                uiCnt--;
                continue;
            }
            
            if(pstIfLinkMsg != NULL)
            {
                bgp_if_link_change_process(pstIfLinkMsg);
                XFREE(MTYPE_MSGQUEUE, pstIfLinkMsg);
                pstIfLinkMsg = NULL;
            }
        }
        uiCnt--;
    }

    return VOS_OK;
}

int bgp4_del_label_queue_process()
{
    u_int *puiDelLabel = NULL;
    
    if(g_pstBgpDelLabelQueue == NULL)
    {
        return VOS_ERR;
    }

    if(!pqueue_is_empty(g_pstBgpDelLabelQueue))
    {
        puiDelLabel = (u_int *)pqueue_dequeue(g_pstBgpDelLabelQueue);
        if(puiDelLabel != NULL)
        {
            bgp_label_handle(BGP4_RELEASE_MPLS_LABLE,puiDelLabel);
            XFREE(MTYPE_MSGQUEUE, puiDelLabel);
            puiDelLabel = NULL;
        }
    }
}
int bgp4_vpn_del_process(IPC_MSG_T *pMsg)
{
    u_int uiInstanceId = 0;
    u_int uiOpMode = 0;
    L3VPN_MSG_T stVpnMsg = {0};
    struct vpn_msghdr stVpnMsgHdr = {0};

    if(!pMsg || !pMsg->pucBuf)
    {
        return VOS_ERR;
    }
    
    uiInstanceId = *(u_int *)pMsg->pucBuf;
    uiOpMode = *(u_int *)(pMsg->pucBuf + 1);

    if(uiOpMode == L3VPN_INVALID)
    {
        memset(&stVpnMsg, 0, sizeof(L3VPN_MSG_T));
        memset(&stVpnMsgHdr, 0, sizeof(struct vpn_msghdr));
        stVpnMsg.vpn_vrid = uiInstanceId;
        stVpnMsgHdr.vpn_state = 1;
        stVpnMsgHdr.cnt = 1;
        stVpnMsgHdr.l3vpnMsg = &stVpnMsg;
        bgp4_rtsock_vpn_del_msg_input((u_char *)&stVpnMsgHdr);
    }

    XFREE(MTYPE_MSGQUEUE, pMsg->pucBuf);
    pMsg->pucBuf = NULL;

    return VOS_OK;
}
void bgp4_task_socket_recv()
{
    int iRetSelect = 0;
    fd_set rfd;
    fd_set wfd;

    while(1)
    {
        //bgp_sem_take();
        /*rtsock checking*/
        iRetSelect = bgp4_socket_select(&rfd, &wfd);
        /*rtsock rx*/
        #if 0
        if ((iRetSelect > 0) && FD_ISSET(gbgp4.rtsock, &rfd))
        {
            bgp4_rtsock_recv();
        }
        #endif
        /*tcp socket checking*/
        if (gbgp4.work_mode != BGP4_MODE_SLAVE)
        {
            bgp4_tcp_socket_recv(&rfd, &wfd); 
        }

        //bgp_sem_give();
        vos_pthread_delay(10);
    }
}

int bgp4_second_init()
{
    ZEBRA_IF_MSG_REGISTER_T stIfMsgRegister = {0};
    ZEBRA_ROUTE_INDEX_T stRouteIndex = {0};
    L3VPN_INSTANCE_MSG_T stVpnMsgRegister = {0};

    vos_pthread_delay(100);
    g_pstBgpRouteQueue = pqueue_create(BGP4_ROUTE_MSG_QUEUE_SIZE);
    
    if(g_pstBgpRouteQueue != NULL)
    {
        if(zebra_ip_route_set_api(&stRouteIndex, ZEBRA_ROUTE_NOTIFY_REGISTER, g_pstBgpRouteQueue) != VOS_OK)
        {
            bgp4_log(BGP_DEBUG_EVT,"bgp register route redistribute failed %p\n",g_pstBgpRouteQueue);
        }
    }
    else
    {
       bgp4_log(BGP_DEBUG_EVT,"bgp route queue is NULL\n"); 
    }

    g_pstBgpIfLinkQueue = pqueue_create(BGP4_IF_LINK_MSG_QUEUE_SIZE);
    
    if(g_pstBgpIfLinkQueue != NULL)
    {
        memset(&stIfMsgRegister, 0, sizeof(ZEBRA_IF_MSG_REGISTER_T));
        stIfMsgRegister.ulDesModId = MTYPE_BGP;
        stIfMsgRegister.ulMsgType = MSG_TYPE_IF_CHG;
        stIfMsgRegister.ulRedistributeType = ZEBRA_REDISTRIBUTE_TYPE_IF_DOWN;
        stIfMsgRegister.pstMsgQue = g_pstBgpIfLinkQueue;    
        if(zebra_if_set_api(0, ZEBRA_IF_NOTIFY_REGISTER, &stIfMsgRegister) != VOS_OK)
        {
             bgp4_log(BGP_DEBUG_EVT,"register interface link msg failed\n");
        }

        stIfMsgRegister.ulRedistributeType = ZEBRA_REDISTRIBUTE_TYPE_IF_ADDR_DELETE;
        if(zebra_if_set_api(0, ZEBRA_IF_NOTIFY_REGISTER, &stIfMsgRegister) != VOS_OK)
        {
             bgp4_log(BGP_DEBUG_EVT,"register interface address delete failed\n");
        }
    }
    else
    {
         bgp4_log(BGP_DEBUG_EVT,"bgp interface link queue is NULL\n");
    }

    g_pstBgpDelLabelQueue = pqueue_create(BGP4_ROUTE_MSG_QUEUE_SIZE);
    if(g_pstBgpDelLabelQueue == NULL)
    {
       printf("bgp del label queue is NULL\n");
    }
    #if 0
    stVpnMsgRegister.ucDesModId = IPC_MSG_QUE_BGP;
    stVpnMsgRegister.ucMsgType = MSG_TYPE_VPN_DEL;
    if(l3vpn_notify_register(&stVpnMsgRegister) != VOS_OK)
    {
        printf("register vpn change msg failed\n");
    }
    #endif
    return VOS_OK;
}

void 
bgp4_task_main(void)
{    
    int iRetMsg = VOS_OK;
    int iRetSelect = 0;
    u_int  ulMsgId = 0;
    u_int ulMsgType = 0;
    IPC_MSG_T stMsg;
    fd_set rfd;
    fd_set wfd;
    
    vos_wdog_create_start(WDOG_PERIOD, WDOG_50MS, 
                          (FUNCPTR)bgp4_send_msg,0);
                          

    while (1)  
    {
        iRetMsg = vos_msg_queue_receive(IPC_MSG_QUE_BGP, &stMsg, WAIT_FOREVER);
        if(iRetMsg > 0)
        {
            ulMsgId = stMsg.ulMsgId;
            ulMsgType = stMsg.ulMsgType; 
            switch(ulMsgId)
            {
                case IPC_MSG_QUE_BGP:
                    switch(ulMsgType)
                    {
                        case MSG_TYPE_50MS:
                            bgp_sem_take();
                            bgp4_timerlist_checking();
                            bgp_sem_give();
                            bgp4_route_queue_get();
                            bgp4_if_link_queue_get();
                            bgp4_del_label_queue_process();
                            break;
                        #if 0    
                        case MSG_TYPE_VPN_DEL:
                            bgp4_vpn_del_process(&stMsg);
                        #endif    
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }

            if(stMsg.pucBuf != NULL)
            {
                XFREE(MTYPE_MSGQUEUE, stMsg.pucBuf);
                stMsg.pucBuf = NULL;
            }
        }
    }   
    return;
}

void 
bgp4_confedration_as_flush(void)  
{
    memset(gbgp4.confedration_as, 0, sizeof(gbgp4.confedration_as));
    return;
}

int
bgp4_confedration_as_add(u_short as) 
{
    u_int i;

    for (i = 0 ; i < BGP4_MAX_CONFEDPEER; i++)
    {
        if (gbgp4.confedration_as[i] == 0)
        {
            gbgp4.confedration_as[i] = as;
            return VOS_OK;
        }        
    }

    return VOS_ERR;
}

void 
bgp4_confedration_as_delete(u_short as) 
{
    u_int i;

    for (i = 0 ; i < BGP4_MAX_CONFEDPEER ; i++)
    {
        if (gbgp4.confedration_as[i] == as)
        {
            gbgp4.confedration_as[i] = 0;
        }        
    }
    return;
}


/*IPSOFT cheng write the functions for confederation*/
u_int * 
bgp4_confedration_as_lookup(u_short as) 
{
    u_int i;

    for (i = 0 ; i < BGP4_MAX_CONFEDPEER ; i++)
    {
        if (gbgp4.confedration_as[i] == as)
        {
            return &gbgp4.confedration_as[i];
        }        
    }
    return NULL;
}

/*expire function for workmode updating*/
void
bgp4_workmode_update_timeout(void)    
{
    u_int state = gbgp4.work_mode;
    /*get current work mode*/
    bgp4_get_workmode();

    /*state change from slave to master*/
    if ((state != gbgp4.work_mode) && (state == BGP4_MODE_SLAVE))
    {
        bgp4_sync_become_master();
    }

    /*start timer*/
    bgp4_timer_start(&gbgp4.workmode_timer, BGP4_WORKMODE_INTERVAL);
    return;
}

/*expire function for path clear*/
void
bgp4_path_clear_timeout(tBGP4_RIB *p_rib)    
{
    /*clear all unused path*/
    bgp4_path_garbage_collect(p_rib);

    /*start timer*/
    if (bgp4_avl_count(&p_rib->path_table))
    {
        bgp4_timer_start(&p_rib->path_timer, BGP4_PATH_CLEAR_INTERVAL);
    }
    return;
}

void
bgp4_igp_sync_timeout(void)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_ROUTE *p_route = NULL;
    u_int af = 0;

    /*start timer,it may be restarted...*/
    bgp4_timer_start(&gbgp4.igp_sync_timer, BGP4_IGP_SYNC_MAX_INTERVAL);
    
    if (gbgp4.igp_sync_enable)
    {
        /*start a new rib walkup round from the every beginning*/
        bgp4_instance_for_each(p_instance) 
        {
            /*only consider ipv4&ipv6 unicast*/
            for (af = 0; af < BGP4_PF_MAX; af++)
            {
                if ((af != BGP4_PF_IPUCAST) && (af != BGP4_PF_IP6UCAST))
                {
                    continue;
                }
                if (p_instance->rib[af] == NULL)
                {
                    continue;
                }
                /*check if igp exist,only for remote route*/
                bgp4_avl_for_each(&p_instance->rib[af]->rib_table, p_route)
                {
                    if (p_route->p_path->p_peer == NULL)
                    {
                        continue;
                    }
                    if (p_route->is_deleted)
                    {
                        continue;
                    }
                        
                    /*same as new route rxd*/
                    if (bgp4_sysroute_lookup(p_route->p_path->p_instance->vrf, p_route) == VOS_OK)
                    {
                        if (p_route->igp_sync_wait == TRUE)
                        {
                            bgp4_rib_in_table_update(p_route);
                        }
                    }
                    else if (p_route->igp_sync_wait == FALSE)
                    {
                        bgp4_rib_in_table_update(p_route);
                    }
                }
            }
        }
    }
    return;
}

void
bgp4_rib_system_update_timeout(tBGP4_RIB *p_rib)
{
    /*do not update system route during restarting*/
    if (gbgp4.in_restarting != TRUE)
    {
        bgp4_rib_system_update_check(p_rib);
    }
    return;
}

void
bgp4_init_update_timeout(void)
{ 
    if (gbgp4.work_mode != BGP4_MODE_SLAVE)
    {
       /*check if need to send init update to all peer
       if last check result is not ok,and current result is ok
       mean need send init update to all peer*/
       bgp4_schedule_all_init_update();
    }
    return;
}

/*check range state*/
void
bgp4_range_update_timeout(tBGP4_VPN_INSTANCE *p_instance)
{
    tBGP4_RANGE *p_range = NULL;

    bgp4_avl_for_each(&p_instance->range_table, p_range)
    {
        if (p_range->update_need)
        {
            p_range->update_need = FALSE;
            bgp4_range_update(p_range);
        }
    }
    bgp4_timer_start(&p_instance->range_update_timer, 1);
    return;
}

/*damp related*/
/*global enable damp*/
void
bgp4_damp_enable(void)
{
    /*only damp flag set*/
    gbgp4.damp_enable = TRUE;
    return;
}

/*global disable damp*/
void
bgp4_damp_disable(void)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    u_int af = 0;

    /*clear damp flag*/
    gbgp4.damp_enable = FALSE;

    /*delete all damp information from all rib*/
    bgp4_instance_for_each(p_instance)
    {
        /*wait for routing updated*/
        for (af = 0 ; af < BGP4_PF_MAX ; af++)
        {
            if (p_instance->rib[af] == NULL)
            {
                continue;
            }
            bgp4_damp_table_flush(p_instance->rib[af]);
        }
    }
    return;
}

void
bgp4_update_limit_timeout(void)
{
    /*restart timer if current update size is not zero*/
    if (gbgp4.update_tx_len)
    {
        bgp4_timer_start_ms(&gbgp4.update_ratelimit_timer, 100);
    }
    /*restart update length*/
    gbgp4.update_tx_len = 0;
    return;
}

/*calculate 6pe peer count*/
u_int
bgp4_6pe_peer_calculate(tBGP4_VPN_INSTANCE *p_instance)
{
    tBGP4_PEER *p_peer = NULL;
    u_int count = 0;
    /*only public instance considerd*/
    if (p_instance->vrf == 0)
    {
        return 0;
    }
    bgp4_avl_for_each(&p_instance->peer_table, p_peer)
    {
        if (p_peer->ip.afi != BGP4_PF_IPUCAST)
        {
            continue;
        }
        if (flag_isset(p_peer->local_mpbgp_capability, BGP4_PF_IP6LABEL))
        {
            count++;
        }
    }
    return count;
}
#else

#include "bgp4com.h"
#if !defined(USE_LINUX) && !defined(WIN32)
#include "uspSysApi.h"
#include "uspDfs.h"
#endif

tBGP4_GLOBAL gBgp4 ;

extern int32_t uspIpRouterId(int32_t vrId, int32_t family, u_char *addr);

tBGP4_VPN_INSTANCE* bgp4_vpn_instance_lookup(u_int instance_id)
{
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    
    LST_LOOP(&gBgp4.vpn_instance_list,p_instance, node, tBGP4_VPN_INSTANCE) 
    {
        if(p_instance->instance_id == instance_id)
        {
            return p_instance;
        }
    }

    return NULL;

}

tBGP4_VPN_INSTANCE* bgp4_vpn_instance_create(u_int instance_id)
{
    tBGP4_VPN_INSTANCE* p_new_instance = NULL;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_VPN_INSTANCE* p_prev = NULL;
    
    p_new_instance = bgp4_malloc(sizeof(tBGP4_VPN_INSTANCE),MEM_BGP_VPN_INSTANCE);

    if (p_new_instance == NULL) 
    {
        printf("\r\nbgp vpn instance data malloc failed");
        return NULL;
    }
    memset(p_new_instance,0,sizeof(tBGP4_VPN_INSTANCE));

    p_new_instance->instance_id = instance_id;
    
    /*initiate lists*/    
    bgp4_lstinit(&p_new_instance->peer_list);
    bgp4_init_rib(&p_new_instance->rib);
    
    bgp4_lstinit(&p_new_instance->network);
    bgp4_lstinit(&p_new_instance->aggr_list);

    /*policy list*/
    bgp4_lstinit(&p_new_instance->route_policy_import);
    bgp4_lstinit(&p_new_instance->route_policy_export);
    bgp4_lstinit(&p_new_instance->import_policy);

    /*sorted by vrf id*/
    LST_LOOP(&gBgp4.vpn_instance_list, p_instance, node, tBGP4_VPN_INSTANCE)
    {
        if (p_instance->instance_id > p_new_instance->instance_id)
        {
            break;
        }
        else
        {
            p_prev = p_instance;
        }
    }

    if (p_prev)
    {
        bgp4_lstinsert(&gBgp4.vpn_instance_list, &p_prev->node, &p_new_instance->node);
    }
    else
    {
        bgp4_lstadd(&gBgp4.vpn_instance_list, &p_new_instance->node);
    }
	
    return p_new_instance;

}

/*release instance resource*/
void bgp4_vpn_instance_release(tBGP4_VPN_INSTANCE* p_instance)
{
    tBGP4_PEER* p_peer = NULL;
    tBGP4_PEER* p_next_peer = NULL;
    
    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp instance is null");
        return ;
    }

    LST_LOOP_SAFE(&p_instance->peer_list, p_peer, p_next_peer, node, tBGP4_PEER)
    {
        bgp4_fsm_invalid(p_peer);
    }

    bgp4_delete_all_peer(&p_instance->peer_list);   

    bgp4_delete_all_network(p_instance);

    bgp4_delete_all_aggregate(p_instance);

    bgp4_delete_policy_list(&p_instance->route_policy_import);
    bgp4_delete_policy_list(&p_instance->route_policy_export);
    
    bgp4_delete_policy_list(&p_instance->import_policy);    

    bgp4_withdraw_import_vpn_route(p_instance->instance_id);
#if 0   
    /*update ip table right away, before rib release*/
    bgp4_rib_walkup(TRUE);
#endif  
    /*release resource*/    
    bgp4_clear_rib(&p_instance->rib); 

    bgp4_log(BGP_DEBUG_EVT,1,"vpn instance %d is released",p_instance->instance_id);

    
}

/*delete instance data from instance list,called when vpn instance is unconfiged*/
void bgp4_vpn_instance_del(tBGP4_VPN_INSTANCE* p_instance)
{
    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp instance is null");
        return ;
    }
    bgp4_lstdelete(&gBgp4.vpn_instance_list, &p_instance->node);
    bgp4_free(p_instance, MEM_BGP_VPN_INSTANCE);

}

int bgp4_global_init(u_int max_route, u_int max_path, u_int max_peer,u_int max_vpn_instance,u_int max_l2vpn_instance,u_int if_support_6pe,
                        void*func1,void* func2 )
{    
    return bgp4_global_initEx(max_route, max_path, max_peer, max_vpn_instance, max_l2vpn_instance,if_support_6pe,
                        func1, func2, calloc, free);
}

int bgp4_global_initEx(u_int max_route, u_int max_path, u_int max_peer,u_int max_vpn_instance,u_int max_l2vpn_instance,u_int if_support_6pe,
                        void*func1,void* func2, void *(*mAlloc)(u_int,u_int), void (* mFree)(void *))
{
    int i = 0;
    u_short usTimerId = 0;

    if(mAlloc == NULL || mFree == NULL)
    {
        return FALSE;
    }

    memset(&gBgp4,0,sizeof(tBGP4_GLOBAL));

    /*init some resource need*/
    gBgp4.priority = BGP4_TASK_PRIORITY;
    gBgp4.stacksize = BGP4_TASK_DEF_DEPTH;

    gBgp4.if_support_6pe = if_support_6pe;
        
    if(max_peer!=0)
    {
        gBgp4.max_peer = max_peer;
    }
    else
    {
        gBgp4.max_peer=BGP4_DEFAULT_PEER_NUMBER;
    }

    if(max_path!=0)
    {
        gBgp4.max_path = max_path;
    }
    else
    {
        gBgp4.max_path=BGP4_DEFAULT_PATH_NUMBER;
    }

    if(max_route!=0)
    {
        gBgp4.max_route = max_route;
    }
    else
    {
        gBgp4.max_route=BGP4_DEFAULT_ROUTE_NUMBER;
    }

    if(max_vpn_instance != 0)
    {
        gBgp4.max_vpn_instance = max_vpn_instance;
    }
#if 0
    else
    {
        gBgp4.max_vpn_instance = L3VPN_DFLT_NUM;/*system default value*/
    }
#endif

    if(max_l2vpn_instance != 0)
    {
        gBgp4.max_l2vpn_instance = max_l2vpn_instance;
    }   
    
    gBgp4.timerate = vos_get_clock_rate();
    gBgp4.confed_id = 0;    
    gBgp4.server_port = BGP4_SERVER_PORT;
    gBgp4.client_port = BGP4_CLIENT_PORT;
    gBgp4.sync_enable = FALSE; 
    gBgp4.server_sock = BGP4_INVALID_SOCKET;
    gBgp4.server_sock6= BGP4_INVALID_SOCKET;
    gBgp4.default_lpref = BGP4_DEFAULT_LOCALPREF;    
    gBgp4.aggr_apply = BGP4_EBGP; 
    gBgp4.stat.rr_client = 0;
    gBgp4.is_reflector = 0;
    gBgp4.cluster_id = 0;
    gBgp4.community_action = 0;
    gBgp4.trap_enable = 0;
    gBgp4.damp_enable = 0;
    gBgp4.max_len = BGP4_DEFAULT_UPDATE_LIMIT;
    gBgp4.server_open = TRUE;
    gBgp4.active_open = TRUE;
        
    gBgp4.gr_current_update = TRUE;
    gBgp4.gr_last_update = TRUE;
    gBgp4.gr_restart_flag = FALSE;
    
    gBgp4.work_mode = BGP4_MODE_OTHER;

    gBgp4.gr_restart_time = BGP4_DFLT_GRTIME;
    gBgp4.gr_selection_deferral_time = BGP4_DFLT_DEFERRALTIME;

    gBgp4.rt_matching_vpnv4 = 1;
    gBgp4.rt_matching_vpnv6 = 1;

    /*initiate lists*/    
    bgp4_lstinit(&gBgp4.ext_community_list);
    
    bgp4_lstinit(&gBgp4.vpn_instance_list);

    if(gBgp4.max_l2vpn_instance!=0)
    {
        bgp4_lstinit(&gBgp4.l2vpn_instance_list);
    }
    
    for(i=0;i<BGP4_PF_MAX;i++)
    {
        bgp4_lstinit(&gBgp4.attr_list[i]);
    }
    
    bgp4_lstinit(&gBgp4.delay_update_list);

    /*initiate semaphore*/
    vos_pthread_lock_init(&gbgp4.sem);
    bgp4_attribute_desc_init();

    gBgp4.routePolicyFunc = func1;
    gBgp4.routePolicyRefCntFunc = func2;

    gBgp4.memAllocFunc = mAlloc;
    gBgp4.memFreeFunc = mFree;

    for (usTimerId = 0 ; usTimerId < BGP4_MAX_TIMER_TABLE ; usTimerId++)
    {
        timerListInit(&gbgp4.timer_table[usTimerId]);
    }
    vos_msg_queue_create(BGP4_MAX_MSG,BGP4_MSG_SIZE,MSG_Q_FIFO,IPC_MSG_QUE_BGP);
    return TRUE ;
}

int bgp4_enable(u_int asnum)  
{    
    #if 0
    tOsixTaskId tid;
    int proto=IPPROTO_BGP4;
    #endif

    if(gBgp4.asnum >0 || asnum== 0)
    {
        printf("\r\nbgp4 can not start!as num %d ERROR",asnum);
        return VOS_ERR;
    }
    #if 0
    if (taskNameToId(BGP4_TASKNAME) != VOS_ERR)
    {
        printf("\r\nbgp4 task is already exist!");
        return EUSP_PROTOINITERR;
    } 
    #endif    
    if(bgp4_init_rtsock() != TRUE) 
    {
        printf("\r\nbgp4 rtsock initiate failed,bgp4 can not start!");
        return VOS_ERR;
    }
    
    if(bgp4_init_timer_list() != TRUE)
    {
        printf("\r\nbgp4 timer initiate failed,bgp4 can not start!");
        return VOS_ERR;
    }
    
    if(bgp4_server_open() != TRUE)
    {
        printf("\r\nbgp4 server open failed,bgp4 can not start!");
        return VOS_ERR;
    }   

    if(gBgp4.memAllocFunc == NULL || gBgp4.memFreeFunc == NULL)
    {
        printf("\r\nbgp4 function of memory allocation failed,bgp4 can not start!");
        return VOS_ERR;
    }

    if(bgp4_mem_init(gBgp4.max_route, gBgp4.max_path, gBgp4.max_peer,
        gBgp4.max_vpn_instance+1) != VOS_OK)
    {
        printf("\r\nbgp4 mem init failed,bgp4 can not start!");
        return VOS_ERR;
    }   

    /*create public instance,id 0*/
    if(bgp4_vpn_instance_create(0) == NULL)
    {
        printf("\r\nbgp create public instance fail!!!");
        return VOS_ERR;
    }
    
    
    #if 0
    if (taskNameToId(BGP4_TASKNAME) == VOS_ERR)
    {
        if(OsixCreateTask(BGP4_TASKNAME, gBgp4.priority, gBgp4.stacksize,
                    bgp4_task_main, 0,
                    OSIX_DEFAULT_TASK_MODE, &tid) == OSIX_FAILURE)
        {
            bgp4_mem_deinit();
            printf("\r\nbgp4 Task create failed,bgp4 can not start!");
            return VOS_ERR;
        }
    }   
    #endif
    vos_pthread_delay(10);

    
    /*get local vpn conf,create configured vpn instances*/
    bgp4_init_private_instance();


    bgp4_set_router_id(0);  

    
#if 0
    
    if(bgp4_up_notify(0) == FALSE)
    {
        printf("\r\nbgp notify mpls fail!!!");
    }
#endif

    //uspScalarSetApi(NULL,SYS_PROTO_ENABLE,&proto);

    gBgp4.admin = TRUE;
    gBgp4.asnum=asnum;

    gBgp4.dbg_flag |= BGP_DEBUG_FSMCHANGE;


    return VOS_OK;
}

int bgp4_disable() 
{        
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_VPN_INSTANCE* p_next_instance = NULL;    
    int proto=IPPROTO_BGP4;        
        
    LST_LOOP_SAFE(&gBgp4.vpn_instance_list,p_instance,p_next_instance,node,tBGP4_VPN_INSTANCE)
    {
        bgp4_vpn_instance_release(p_instance);
        bgp4_vpn_instance_del(p_instance);
        
    }

    bgp4_rtlist_clear(&gBgp4.delay_update_list);

    bgp4_server_close(); 

    bgp4_delete_all_confed_peer();/*IPSOFT cheng add for confederation*/

    bgp4_delete_all_ext_comm();

    bgp4_clear_unused_path();
    
    bgp4_shutdown_timer(); 

    bgp4_close_rtsock();

    gBgp4.admin = FALSE;
    gBgp4.asnum=0;
    gBgp4.router_id = 0;
    gBgp4.priority = BGP4_TASK_PRIORITY;
    gBgp4.stacksize = BGP4_TASK_DEF_DEPTH;

    gBgp4.confed_id = 0;    
    gBgp4.server_port = BGP4_SERVER_PORT;
    gBgp4.client_port = BGP4_CLIENT_PORT;
    gBgp4.sync_enable = FALSE; 
    gBgp4.server_sock = BGP4_INVALID_SOCKET;
    gBgp4.server_sock6= BGP4_INVALID_SOCKET;
    gBgp4.default_lpref = BGP4_DEFAULT_LOCALPREF;    
    gBgp4.aggr_apply = BGP4_EBGP; 
    gBgp4.stat.rr_client = 0;
    gBgp4.is_reflector = 0;
    gBgp4.cluster_id = 0;
    gBgp4.community_action = 0;
    gBgp4.trap_enable = 0;
    gBgp4.damp_enable = 0;
    gBgp4.max_len = BGP4_DEFAULT_UPDATE_LIMIT;
    gBgp4.server_open = TRUE;
    gBgp4.active_open = TRUE;
    gBgp4.gr_current_update = FALSE;
    gBgp4.gr_last_update = FALSE;
    gBgp4.dbg_flag = 0;

    vos_pthread_delay(10);
        
    bgp4_mem_deinit();

    return VOS_OK;
}

void bgp4_task_slave()
{
    struct fd_set read_fd;
    struct timeval timeout ;
    u_int rc = 0;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    tBGP4_LINK *p_link = NULL;
    tBGP4_LINK *p_next = NULL;  
    tBGP4_ROUTE * p_route = NULL;
    u_int rt_sock_set=0;
    u_int vrf_id = 0;
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;/*100ms*/

    FD_ZERO(&read_fd);

    if (0 < gBgp4.rtsock)
    {
        FD_SET (gBgp4.rtsock, &read_fd);
        rc = select(gBgp4.rtsock + 1, &read_fd, NULL, NULL, &timeout);
               
        if ((0 < rc) && FD_ISSET (gBgp4.rtsock, &read_fd))
        {
            bgp_sem_take();   
            bgp4_rtsock_recv();
            bgp_sem_give();  
        }
    }
    else
    {
        vos_pthread_delay(1);
    }
            
    bgp_sem_take();  
            
    /*walkup rib for ip and msg update*/
    bgp4_rib_walkup(FALSE);
                
    bgp4_clear_unused_path();

    /*check IGP Sync*/
    if(gBgp4.sync_enable)
    {
        LST_LOOP(&gBgp4.vpn_instance_list,p_instance, node, tBGP4_VPN_INSTANCE) 
        {
            bgp4_init_next_ip_update_route(p_instance);
        }
        p_instance = NULL;
        bgp4_init_next_rib_update_route();
        gBgp4.rib_walkup_need = TRUE;
        bgp4_rib_walkup(FALSE);
    }
            
    bgp_sem_give();   
            
    /*if exist non-direct nexthop,turn on the listening for route change*/
    if(rt_sock_set == 0 && gBgp4.direct_nexthop_exist)
    {
        bgp_sem_take();

        rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_netmgmt,1);
        rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_local,1);
        rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_rip,1);
        rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_is_is,1);
        rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_ospf,1);
        rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_bgp,1);
        rt_sock_set = 1;

        bgp_sem_give();

        bgp4_log(BGP_DEBUG_RT,1,"direct nexthop exist,rt sock option set,listen to route change of any protocol");

    }
            
    /*recaculate direct nexthop again and send HW ROUTE if changed*/
    if(gBgp4.direct_nexthop_exist && gBgp4.nexthop_lookup_need)
    {
        bgp_sem_take();

        /*recaculate direct nexthop again and send HW ROUTE if changed*/
        bgp4_recalculate_direct_nexthop();
        gBgp4.nexthop_lookup_need = 0;
                
        bgp_sem_give();
    }
        
    /*Loop global list*/
    LST_LOOP_SAFE(&gBgp4.delay_update_list, p_link,p_next, node, tBGP4_LINK)
    {
        bgp_sem_take();
                
        p_route = p_link -> p_route ;
            
        if(p_route == NULL)
        {
            bgp_sem_give();   
            continue;
        }

        /* if route has no reference,delete it*/
        if (p_route->refer <= 1)
        {
            bgp4_rtlist_delete(&gBgp4.delay_update_list, p_link);
            bgp_sem_give();
            continue;
        }
        
        if(bgp4_get_direct_nexthop(p_route,NULL) == TRUE)
        {
            vrf_id = p_route->p_path->p_instance->instance_id;
            if(bgp4_process_ip_update(vrf_id , p_route) == TRUE)
            {
                bgp4_rtlist_delete(&gBgp4.delay_update_list, p_link);
            }
        }

        bgp_sem_give();
    }
            

}





void bgp4_task_main()
{    
    struct fd_set read_fd;
    u_int rc = 0;
    struct timeval timeout ;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int bgp_is_slave = 0;
    u_char peerstr[64];
    tBGP4_LINK *p_link = NULL;
    tBGP4_LINK *p_next = NULL;  
    tBGP4_ROUTE * p_route = NULL;
    u_int rt_sock_set=0;
    u_int vrf_id = 0;
    u_short usTimerId = 0;

    vos_pthread_delay(60);
    
    while (1)  
    {

         for (usTimerId = 0 ; usTimerId < BGP4_MAX_TIMER_TABLE ; usTimerId++)
         {
            timerListCheck(&gbgp4.timer_table[usTimerId],50)
         }
        /*if current is backup,just wait for role change event in RT_SOCK*/
        while (BGP4_MODE_SLAVE == bgp4_get_workmode())
        {
            bgp_is_slave = 1;
            
            bgp4_task_slave();
            
            /*if end bgp task*/
            if(bgp4_check_router_end()==TRUE)
            {
                goto bgp4_end;          
            }

        }
        
        /*if state change from slave to master reset all established peer's hold timer*/
        if(bgp_is_slave == 1)
        {
            tBGP4_PEER* p_peer=NULL;
            bgp_is_slave = 0;
            p_instance = NULL;
            
            bgp_sem_take(); 
            LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
            {
                LST_LOOP(&p_instance->peer_list,p_peer,node,tBGP4_PEER)
                {
                    if(p_peer->state == BGP4_NS_ESTABLISH)
                    {
                        bgp4_restart_hold_timer(p_peer);
                        bgp4_stop_keepalive_timer(p_peer) ;
                        bgp4_start_keepalive_timer(p_peer) ;
                        bgp4_fill_tcp_address(p_peer);
                    }
                    else
                    {
                        bgp4_printf_peer(p_peer, peerstr);
                        bgp4_log(BGP_DEBUG_EVT,1,"peer %s not in establish when switch to master",peerstr); 
                    }
                }
            }
            bgp_sem_give();  

        }
        /*select socket,if some event detected,process it*/
        bgp4_check_socket(); 
        
        bgp_sem_take();     
                
        //bgp4_check_timer();

        /*walkup rib for ip and msg update*/
        bgp4_rib_walkup(FALSE); 
        
        /*check if need to send init update to all peer
        if last check result is not ok,and current result is ok
        mean need send init update to all peer*/
        bgp4_gr_send_all_init_update();
        
        bgp4_clear_unused_path();

        bgp_sem_give();
        
        /*check IGP Sync*/
        if(gBgp4.sync_enable)
        {

            bgp_sem_take();

            /*start a new rib walkup round from the every beginning*/
            LST_LOOP(&gBgp4.vpn_instance_list,p_instance, node, tBGP4_VPN_INSTANCE) 
            {
                bgp4_init_next_ip_update_route(p_instance);
            }
            p_instance = NULL;
            bgp4_init_next_rib_update_route();
        
            gBgp4.rib_walkup_need = TRUE;
            bgp4_rib_walkup(FALSE);

            bgp_sem_give();

        }

        /*if exist non-direct nexthop,turn on the listening for route change*/
        if(rt_sock_set == 0 && gBgp4.direct_nexthop_exist)
        {
            bgp_sem_take();

            rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_netmgmt,1);
            rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_local,1);
            rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_rip,1);
            rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_is_is,1);
            rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_ospf,1);
            rtSockOptionSet(gBgp4.rtsock,M2_ipRouteProto_bgp,1);
            rt_sock_set = 1;

            bgp_sem_give();

            bgp4_log(BGP_DEBUG_RT,1,"direct nexthop exist,rt sock option set,listen to route change of any protocol");

        }

        /*recaculate direct nexthop again and send HW ROUTE if changed*/
        if(gBgp4.direct_nexthop_exist && gBgp4.nexthop_lookup_need)
        {
            bgp_sem_take();

            /*recaculate direct nexthop again and send HW ROUTE if changed*/
            bgp4_recalculate_direct_nexthop();
            gBgp4.nexthop_lookup_need = 0;
                    
            bgp_sem_give();
        }
            
        /*Loop global list*/
        LST_LOOP_SAFE(&gBgp4.delay_update_list, p_link,p_next, node, tBGP4_LINK)
        {
            bgp_sem_take();
                    
            p_route = p_link -> p_route ;
                
            if(p_route == NULL)
            {
                bgp_sem_give();   
                continue;
            }
            
            /* if route has no reference,delete it*/
            if (p_route->refer <= 1)
            {
                bgp4_rtlist_delete(&gBgp4.delay_update_list, p_link);
                bgp_sem_give();
                continue;
            }
            
            if(bgp4_get_direct_nexthop(p_route,NULL) == TRUE)
            {
                vrf_id = p_route->p_path->p_instance->instance_id;
                if(bgp4_process_ip_update(vrf_id , p_route) == TRUE)
                {
                    bgp4_rtlist_delete(&gBgp4.delay_update_list, p_link);
                }
            }

            bgp_sem_give();
        }

        /*delay initial sync when newly launched card is ready*/
        if((gBgp4.slave_up==1)&&(gBgp4.have_slave == 1))
        {
            bgp_sem_take();
            
            if((vos_get_system_tick()-gBgp4.sync_tick)>600)
            {
                bgp4_send_sync_all();
                gBgp4.slave_up=0;
                gBgp4.sync_tick=0;
            } 

            bgp_sem_give();
        }

        
        
        /*if end bgp task*/
        if(bgp4_check_router_end()==TRUE)
        {
            goto bgp4_end;
        }

    }       
    

bgp4_end:
    bgp_sem_take();
    bgp4_disable();
    bgp_sem_give();
    
    return;/*never enter here if as not equal zero*/
}


void bgp4_check_timer()  
{
    timerListCheck(gBgp4.p_timerlist, 50);
    
    return ;
}

INT1 bgp4_check_router_end()
{
    u_int bgp4_retry_times = 0;
    tBGP4_VPN_INSTANCE* p_instance = NULL;
    u_int bgp4_max_retry = 0;

    /*get total rib counts of all existing instance*/
    LST_LOOP(&gBgp4.vpn_instance_list,p_instance,node,tBGP4_VPN_INSTANCE)
    {
        bgp4_max_retry += bgp4_rib_count(&p_instance->rib);
    }

    if(gBgp4.asnum==0)
    {   
        bgp_sem_take();
        bgp4_delete_all_peer_route();
        bgp_sem_give();
            
        while(1)
        {
            bgp4_retry_times++;
            vos_pthread_delay(5);
            bgp_sem_take();

            bgp4_rib_walkup(FALSE);
            if(gBgp4.rib_walkup_need ==FALSE)
            {
                bgp_sem_give();
                break;
            }
            /*check bgp4_del_times is bigger than bgp4_max_del,we shall 
            get out of the end round and end bgp task*/
            if(bgp4_retry_times>bgp4_max_retry)
            {
                bgp_sem_give();
                break;
            }   
            
            bgp_sem_give();
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

int bgp4_delete_all_confed_peer()  
{
    memset(gBgp4.confedpeer, 0, sizeof(gBgp4.confedpeer));
    return TRUE;
}

INT1 bgp4_add_confed_peer(u_short as) 
{
    u_int i;

    for (i = 0 ; i < BGP4_MAX_CONFEDPEER ; i++)
    {
        if (gBgp4.confedpeer[i] == 0)
        {
            gBgp4.confedpeer[i] = as;
            return TRUE;
        }        
    }
    return FALSE;
}

INT1 bgp4_delete_confed_peer(u_short as) 
{
    u_int i;

    for (i = 0 ; i < BGP4_MAX_CONFEDPEER ; i++)
    {
        if (gBgp4.confedpeer[i] == as)
        {
            gBgp4.confedpeer[i] = 0;
        }        
    }
    return TRUE;
}


/*IPSOFT cheng write the functions for confederation*/
INT1 bgp4_is_confed_peer(u_short as) 
{
    u_int i;

    for (i = 0 ; i < BGP4_MAX_CONFEDPEER ; i++)
    {
        if (gBgp4.confedpeer[i] == as)
        {
            return TRUE;
        }        
    }
    return FALSE;
}

STATUS bgp4_up_notify(u_int vr_id)
{
    u_int  len = 0;
    struct vpnGen_msghdr rtmsg = {0};

    rtmsg.rtm_msglen = sizeof(struct vpnGen_msghdr);
    rtmsg.rtm_version = RTM_VERSION;
    rtmsg.rtm_type = RTM_BGP_UP;
    rtmsg.process_id = vr_id;

    len = rtmsg.rtm_msglen;

    gBgp4.stat.ipmsgsend++;

    if (0 > send(gBgp4.rtsock, &rtmsg, len, 0))
    {
        gBgp4.stat.ipmsgfail++;
        return FALSE;
    }

    return TRUE;
}

#endif
