#include "ospf.h"

#include "ospf_nm.h"

#define OSPF_SYN_MSG_LEN 256

extern int log_time_print(u_int8 *);

void ospf_syn_switch_to_master(struct ospf_process *p_process);

struct ospf_lsa_fragment *ospf_lsa_fragment_lookup(struct ospf_process *p_process,void *p_scope,struct ospf_lshdr *p_lshdr);
struct ospf_lsa_fragment *ospf_lsa_fragment_add(struct ospf_process *p_process,void *p_scope,struct ospf_lshdr *p_lshdr);

int
ospf_syn_control_lookup_cmp(
       struct ospf_syn_control *p1,
       struct ospf_syn_control *p2)
{
    OSPF_KEY_CMP(p1, p2, synflag);
    return 0;
}

struct ospf_syn_control *
ospf_syn_control_lookup(u_int synflag)
{
    struct ospf_syn_control syn_control;

    syn_control.synflag = synflag;
    return ospf_lstlookup(&ospf.syn_control_table, &syn_control);
}

struct ospf_syn_control *
ospf_syn_control_create(u_int synflag)
{
    struct ospf_syn_control *p_control = NULL;

    p_control = ospf_syn_control_lookup(synflag);
    if (p_control)
    {
        return p_control;
    }
    
    p_control = ospf_malloc(sizeof(struct ospf_syn_control), OSPF_MPACKET);
    if (NULL == p_control)
    {
        return NULL;
    }

    p_control->synflag = synflag;
    p_control->p_msg = (struct ospf_syn_pkt *)p_control->buf;
    ospf_lstadd(&ospf.syn_control_table, p_control);

    return p_control;
}

void
ospf_syn_control_delete(struct ospf_syn_control *p_control)
{
    p_control->p_msg = NULL;
    ospf_lstdel(&ospf.syn_control_table, p_control);
    ospf_mfree(p_control, OSPF_MPACKET);
    return;
}

void 
ospf_syn_spf_request(struct ospf_process *p_process)
{
    u_int old_val = p_process->spf_interval;
    
    p_process->spf_interval = OSPF_MAX_SPF_INTERVAL; /*30S*/
    ospf_spf_request(p_process);
    p_process->spf_interval = old_val;
    return;
}

/*update system working mode*/
u_int 
ospf_get_workmode(struct ospf_global *p_ospf, u_long ulWorkMode)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_syn_control *p_control = NULL;
    struct ospf_syn_control *p_next_control = NULL;
    
    u_int new_mode = OSPF_MODE_NORMAL;
    u_int old_mode = p_ospf->work_mode;
#ifdef OSPF_MASTER_SLAVE_SYNC
 	//int iWorkState = SYN_STAT_MAX;
    /*get system working mode*/
    /*for realtime syn, only master on master system send syn msg;
    for init syn, master on master system and master on sub-system all send syn msg*/
#if 1
    if (dev_local_nego_is_master() == FALSE)
    {
        ulWorkMode = SYN_STAT_BKP;
    }
    else
    {
        ulWorkMode = SYN_STAT_WORK;
    }
#endif
    switch(ulWorkMode)
    {
        case SYN_STAT_WORK:
            //ospf.stat.syn_chg_msg_cnt[0] ++;
            //memcpy(ospf.stat.syn_work_msg_time_old,ospf.stat.syn_work_msg_time,sizeof(ospf.stat.syn_work_msg_time));
            //log_time_print(ospf.stat.syn_work_msg_time);
            new_mode = OSPF_MODE_MASTER;
            break;
        case SYN_STAT_BKP:
            //ospf.stat.syn_chg_msg_cnt[1] ++;
            //memcpy(ospf.stat.syn_bkp_msg_time_old,ospf.stat.syn_bkp_msg_time,sizeof(ospf.stat.syn_bkp_msg_time));
            //log_time_print(ospf.stat.syn_bkp_msg_time);
            new_mode = OSPF_MODE_SLAVE;
            break;
        default:
            //ospf.stat.syn_chg_msg_cnt[2] ++;
            //memcpy(ospf.stat.syn_other_msg_time_old,ospf.stat.syn_other_msg_time,sizeof(ospf.stat.syn_other_msg_time));
            //log_time_print(ospf.stat.syn_other_msg_time);
            new_mode = OSPF_MODE_SLAVE;
            break;
    }
    if((old_mode == OSPF_MODE_MASTER)&&(new_mode == OSPF_MODE_SLAVE))
    {
        //OSPF_LOG_WARN("ospf old_mode:%d  new_mode:%d",old_mode, new_mode);
  //      return old_mode;
    }
#endif

	#if 0
    if (uspHwScalarGet(NULL,HW_SYS_SWRUNNINGROLEISMASTER,NULL) == OK)
    {
        new_mode = OSPF_MODE_MASTER;
    }
    else if ((uspHwScalarGet(NULL,HW_SYS_SWRUNNINGROLEISSLAVE,NULL) == OK)
        ||(uspHwScalarGet(NULL,HW_SYS_HAROLEISSLAVE,NULL) == OK))
    {
        new_mode = OSPF_MODE_SLAVE;
    }
	#endif
    
    if (new_mode == old_mode)
    {
        goto END;
    }
    switch(ulWorkMode)
    {
        /*caoyong delete 2017.9.18  同步代码暂时注释掉不考虑*/
        #if 0
        case SYN_STAT_WORK:
            ospf.stat.syn_chg_msg_cnt[0] ++;
            memcpy(ospf.stat.syn_work_msg_time_old,ospf.stat.syn_work_msg_time,sizeof(ospf.stat.syn_work_msg_time));
            log_time_print(ospf.stat.syn_work_msg_time);
            //new_mode = OSPF_MODE_MASTER;
            break;
        case SYN_STAT_BKP:
            ospf.stat.syn_chg_msg_cnt[1] ++;
            memcpy(ospf.stat.syn_bkp_msg_time_old,ospf.stat.syn_bkp_msg_time,sizeof(ospf.stat.syn_bkp_msg_time));
            log_time_print(ospf.stat.syn_bkp_msg_time);
            //new_mode = OSPF_MODE_SLAVE;
            break;
        #endif    
        default:
            ospf.stat.syn_chg_msg_cnt[2] ++;
            memcpy(ospf.stat.syn_other_msg_time_old,ospf.stat.syn_other_msg_time,sizeof(ospf.stat.syn_other_msg_time));
            log_time_print(ospf.stat.syn_other_msg_time);
            //new_mode = OSPF_MODE_SLAVE;
            break;
    }
    ospf_logx(ospf_debug_syn,"ospf_get_workmode iWorkState:%d",ulWorkMode);
    OSPF_LOG_WARN("ospf_get_workmode iState:%d",ulWorkMode);
    
    ospf_logx(ospf_debug_syn, "get work mode changed old: %d,new:%d(0:nomal,1:master,2:slave)", old_mode, new_mode); 
    p_ospf->syn_seq_rcv = 0;
    p_ospf->syn_seq_send = 0;
    p_ospf->stat.sync.seq_mismatch = 0;
    /*set new working mode*/
    p_ospf->work_mode = new_mode;
  
    /* wait for a period before switch*/
    ospf_sys_delay(2);
  
    for_each_node(&ospf.syn_control_table, p_control, p_next_control)
    {
        p_control->len = 0; /*clear rest sync buffer*/         
    }
 
    if (OSPF_MODE_SLAVE == old_mode)
    {
        /*perform switch operation for all process*/
        for_each_ospf_process(p_process, p_next_process)
        {
            ospf_set_context(p_process);
            ospf_syn_switch_to_master(p_process);
        }
    }
    
 END:   
   ospf_stimer_start(&p_ospf->workmode_timer, OSPF_WORKMODE_INTERVAL);
    return new_mode;
}


void ospf_syn_wokemode(void)
{
    /*caoyong delete 2017.9.18  同步代码暂时注释掉不考虑*/
    #if 0
    u_int new_mode = OSPF_MODE_NORMAL;
    int iState = 0;
              
    if (dev_local_nego_is_master() == FALSE)
    {
        iState = SYN_STAT_BKP;
    }
    else
    {
        iState = SYN_STAT_WORK;
    }
    ospf_log(ospf_debug_if,"ospf_syn_wokemode iState:%d", iState);
    OSPF_LOG_WARN("ospf_syn_wokemode iState:%d",iState);

    switch(iState)
    {
        case SYN_STAT_WORK:
            new_mode = OSPF_MODE_MASTER;
            break;
        case SYN_STAT_BKP:
            new_mode = OSPF_MODE_SLAVE;
            break;
        default:
            new_mode = OSPF_MODE_NORMAL;
            break;
    }
    ospf.work_mode = new_mode;
    
    return ;
    #endif
}

/*insert message to output buffer*/
void 
ospf_syn_insert_message(                    
                     struct ospf_syn_hdr *p_hdr,
                     struct ospf_syn_control *p_control)
{
    u_int len = ntohs(p_hdr->len) + sizeof(*p_hdr);
    u_int event = 0;
  
    if (OSPF_MODE_MASTER != ospf.work_mode)
    {
        return;
    }
    /*get default control if not assigned*/
    if (NULL == p_control)
    {
        p_control = ospf_syn_control_lookup(0);
        if (NULL == p_control) 
        {
            return;
        }
    }
  
    ospf_logx(ospf_debug_syn, "insert msg, sync_len=%d\r\n", p_control->len);
   /*if output buffer has enough space, copy message directly.
       else,send packet now,and reset buffer,do copy again
    */
    if (OSPF_SYN_MAX_TXBUF < (p_control->len + len))
    {
        ospf_syn_send(p_control);
    }
    /*prevent memory overflow for single msg*/
    if (OSPF_SYN_MAX_TXBUF < len)
    {
        return;
    }
    /*copy message and update msg length*/
    memcpy(p_control->p_msg->msg + p_control->len, p_hdr, len);
    p_control->len += len;
    
    ospf_sync_event_set(event);
  
    ospf.stat.sync.send_msg[p_hdr->cmd]++;
    return;
}

/*build sync message for instance ,return length of message*/
void 
ospf_syn_process_send(
                      struct ospf_process *p_process,
                      u_int add,
                      struct ospf_syn_control *p_control)
{
    u_int8 buf[OSPF_SYN_MSG_LEN] = {0};
    struct ospf_syn_hdr *p_hdr = (void *)buf;
    struct ospf_syn_instance *p_msg = NULL;
    u_int i = 0;
    
    ospf_logx(ospf_debug_syn, "ospf send syn %s processid %d ,routerid %x ", 
       (add == TRUE)?"add":"del", p_process->process_id, p_process->router_id); 

    /*do not send msg for process to be deleted*/
    if (p_process->proto_shutdown)
    {
        return;
    }
    /*get default control if not assigned*/
    if (NULL == p_control)
    {
        p_control = ospf_syn_control_lookup(0);
    }
    p_hdr->cmd = OSPF_SYN_TYPE_INSTANCE;
    p_hdr->add = add;
    p_msg = (struct ospf_syn_instance *)(buf + sizeof(struct ospf_syn_hdr));

    if (FALSE == add)
    {
        p_msg->process_id = htonl(p_process->process_id);
        p_hdr->len = htons(4);
        goto SEND_MSG;
    }
    
    p_msg->process_id = htonl(p_process->process_id);
    p_msg->router_id = htonl(p_process->router_id);
    p_msg->overflow_limit = htonl(p_process->overflow_limit);
    p_msg->overflow_period = htonl(p_process->overflow_period);
    p_msg->reference_rate = htonl(p_process->reference_rate);
    p_msg->spf_interval= htons(p_process->spf_interval);
    p_msg->restart_period= htons(p_process->restart_period);
        
    for ( i = 0; i < OSPF_MAX_PROTOCOL; i++)
    {
        if (BIT_LST_TST(p_process->reditribute_flag, i))
        {
            p_msg->reditribute_flag[i] = 1;
        }
    }
        
    p_msg->trap_enable = p_process->trap_enable;
    p_msg->asbr = p_process->asbr;
    p_msg->abr = p_process->abr;
    p_msg->opaque = p_process->opaque;
    p_msg->rfc1583_compatibility = p_process->rfc1583_compatibility;
    p_msg->tos_support = p_process->tos_support;
    p_msg->dc_support = p_process->dc_support;
    p_msg->mcast_support = p_process->mcast_support;
    p_msg->stub_support = p_process->stub_support;
    p_msg->stub_adv = p_process->stub_adv;
    p_msg->restart_enable = p_process->restart_enable;
    p_msg->restart_reason = p_process->restart_reason;
    p_msg->restart_helper = p_process->restart_helper;      
    p_msg->strictlsa_check = p_process->strictlsa_check;
    p_msg->current_time = htonl(os_system_tick())+ospf.seq_offset;
    
    p_hdr->len = htons(sizeof(*p_msg));

SEND_MSG:    
    ospf_syn_insert_message(p_hdr, p_control);
    /*send directly,no wait*/
    ospf_syn_send(p_control);        
    return;
}

/*send sync msg for area*/
void 
ospf_syn_area_send(
      struct ospf_area *p_area,
      u_int add,
      struct ospf_syn_control *p_control)
{
    u_int8 buf[OSPF_SYN_MSG_LEN]={0};
    struct ospf_syn_hdr *p_hdr = (void *)buf;
    struct ospf_syn_area *p_msg = NULL;
    u_int i = 0;

    ospf_logx(ospf_debug_syn, "ospf send syn %s area %d  ", (add == TRUE)?"add":"del", p_area->id); 

    /*do not send msg for process to be deleted*/
    if (p_area->p_process->proto_shutdown)
    {
        return;
    }

    p_hdr->cmd = OSPF_SYN_TYPE_AREA;
    p_hdr->add = add;
    p_msg = (struct ospf_syn_area *)(buf + sizeof(struct ospf_syn_hdr));

    if (FALSE == add)
    {
        p_msg->id = htonl(p_area->id);
        p_msg->process_id = htonl(p_area->p_process->process_id);
        p_hdr->len = htons(8);
        goto SEND_MSG;
    }

    p_msg->id = htonl(p_area->id);
    p_msg->process_id = htonl(p_area->p_process->process_id);
    for (i = 0; i < OSPF_MAX_TOS; i++)
    {
        p_msg->stub_default_cost[i].cost = htons(p_area->stub_default_cost[i].cost);
        p_msg->stub_default_cost[i].type = p_area->stub_default_cost[i].type;
        p_msg->stub_default_cost[i].status = p_area->stub_default_cost[i].status;                   
    }
        
    p_msg->authtype = htons(p_area->authtype);
    p_msg->keyid = p_area->keyid;
    p_msg->keylen = p_area->keylen;
    memcpy(p_msg->key, p_area->key, (p_area->keylen < (OSPF_MAX_KEY_LEN + 4)) ? p_area->keylen : (OSPF_MAX_KEY_LEN + 4));
    p_msg->nssa_default_cost = htonl(p_area->nssa_default_cost);
    p_msg->nssa_cost_type = htons(p_area->nssa_cost_type);
    p_msg->nssa_wait_time = htons(p_area->nssa_wait_time);
    p_msg->te_instance = htonl(p_area->te_instance);
    p_msg->nssa_tag = htonl(p_area->nssa_tag);
    p_msg->nssa_always_translate = p_area->nssa_always_translate;
    p_msg->nssa_translator = p_area->nssa_translator;
    p_msg->is_nssa = p_area->is_nssa;
    p_msg->is_stub = p_area->is_stub;
    p_msg->nosummary = p_area->nosummary;
    p_msg->te_enable = p_area->te_enable;
    
    p_hdr->len = htons(sizeof(*p_msg));

SEND_MSG:    
    ospf_syn_insert_message(p_hdr, p_control);
    return;
}

/*send sync msg for interface*/
void 
ospf_syn_if_send(
     struct ospf_if *p_if,
     u_int add,
     struct ospf_syn_control *p_control)
{    
    u_int8 buf[OSPF_SYN_MSG_LEN] = {0};
    struct ospf_syn_hdr *p_hdr = (void *)buf;
    struct ospf_syn_if *p_msg = NULL;
    u_int i = 0;

    ospf_logx(ospf_debug_syn, "ospf send syn %s if %x unit%d,index%d state%d,processid%d ", (add==TRUE)?"add":"del", p_if->addr,p_if->ifnet_uint,p_if->ifnet_index,p_if->state,p_if->p_process->process_id); 
    
    /*do not send msg for process to be deleted*/
    if (p_if->p_process->proto_shutdown)
    {
        return;
    }

    p_hdr->cmd = OSPF_SYN_TYPE_IF;
    p_hdr->add = add;

    p_msg = (struct ospf_syn_if *)(buf + sizeof(struct ospf_syn_hdr));
    
    p_msg->process_id = htonl(p_if->p_process->process_id);
    p_msg->addr = htonl(p_if->addr);
    p_msg->nbrid = htonl(p_if->nbr);
    if (p_if->p_transit_area)
    {
        p_msg->transit_area_id = htonl(p_if->p_transit_area->id);
    }
    p_msg->type = htonl(p_if->type);

    if (FALSE == add)
    {
        p_hdr->len = htons(20);
        goto SEND_MSG;
    }

    if (p_if->p_area)
    {
        p_msg->area_id = htonl(p_if->p_area->id);
    }
    p_msg->ifnet_index = htonl(p_if->ifnet_index);
    p_msg->ifnet_uint = htonl(p_if->ifnet_uint);
    p_msg->link_up = p_if->link_up;
    p_msg->mask = htonl(p_if->mask);

    for (i = 0; i < OSPF_MAX_TOS; i++)
    {
        p_msg->cost[i] = htons(p_if->cost[i]);     
    }
    p_msg->dr = htonl(p_if->dr);
    p_msg->bdr = htonl(p_if->bdr);
    p_msg->mtu = htons(p_if->mtu);
    p_msg->dead_interval = htonl(p_if->dead_interval);
    p_msg->priority = htons(p_if->priority);
    p_msg->hello_interval = htons(p_if->hello_interval);
    p_msg->poll_interval = htonl(p_if->poll_interval);
    p_msg->rxmt_interval = htons(p_if->rxmt_interval);
    p_msg->tx_delay = p_if->tx_delay;
    p_msg->state = p_if->state;
    p_msg->authtype = p_if->authtype;
    p_msg->md5id = p_if->md5id;
    memcpy(p_msg->key, p_if->key, (p_if->keylen < (OSPF_MD5_KEY_LEN + 4)) ? p_if->keylen : (OSPF_MD5_KEY_LEN + 4));
    p_msg->keylen = p_if->keylen;
    p_msg->passive = p_if->passive;
    p_msg->mcast = p_if->mcast;
    p_msg->demand = p_if->demand;
    p_msg->configcost = p_if->configcost;
    p_msg->te_enable = p_if->te_enable;
#ifdef HAVE_BFD
    p_msg->bfd_enable = p_if->bfd_enable;
#endif
    p_msg->fast_dd_enable = p_if->fast_dd_enable;
    p_msg->te_instance = htonl(p_if->te_instance);
    p_msg->te_cost = htonl(p_if->te_cost);
    p_msg->te_group = htonl(p_if->te_group);
    p_msg->max_bd = htonl(p_if->max_bd);
    p_msg->max_rsvdbd = htonl(p_if->max_rsvdbd);
    for (i = 0; i < OSPF_MAX_BD_CLASS; i++)
    {
        p_msg->unrsvdbd[i] = htonl(p_if->unrsvdbd[i]);     
    }
    p_msg->flood_group = htonl(p_if->flood_group);
    
    p_hdr->len = htons(sizeof(*p_msg));
    
 SEND_MSG:
    ospf_syn_insert_message(p_hdr, p_control);
    return;
}

/*send sync msg for nbr*/
void 
ospf_syn_nbr_send(
          struct ospf_nbr *p_nbr,
          u_int add,
          struct ospf_syn_control *p_control)
{
    u_int8 buf[OSPF_SYN_MSG_LEN] = {0};
    struct ospf_syn_hdr *p_hdr = (void *)buf;
    struct ospf_syn_nbr *p_msg = NULL;
    struct ospf_if *p_if = p_nbr->p_if;

    ospf_logx(ospf_debug_syn, "ospf send syn %s nbr %x,dr%x,bdr%x  ", (add==TRUE)?"add":"del", p_nbr->addr,p_nbr->dr,p_nbr->bdr); 

    /*do not send msg for process to be deleted*/
    if (p_nbr->p_if->p_process->proto_shutdown && (FALSE == add))
    {
        return;
    }

    p_hdr->cmd = OSPF_SYN_TYPE_NBR;
    p_hdr->add = add;
    p_msg = (struct ospf_syn_nbr *)(buf + sizeof(struct ospf_syn_hdr));
    
    p_msg->process_id = htonl(p_if->p_process->process_id);
    p_msg->iftype = htonl(p_if->type);
    p_msg->if_addr = htonl(p_if->addr);
    p_msg->if_uint = htonl(p_if->ifnet_uint);
	
    if (p_if->p_transit_area)
    {
        p_msg->transit_area_id = htonl(p_if->p_transit_area->id);
    }                
    p_msg->nbr_id = htonl(p_nbr->id);
    p_msg->addr = htonl(p_nbr->addr);
    p_msg->dr = htonl(p_nbr->dr);
    p_msg->bdr = htonl(p_nbr->bdr);
    p_msg->priority = htons(p_nbr->priority);
    p_msg->option = p_nbr->option;
    p_msg->state = p_nbr->state;
    p_msg->ulDdMtu = p_nbr->ulDdMtu;
                   
    ospf_logx(ospf_debug_syn, "ospf_syn_nbr_send nbr_id %x,priority %d,state %d ,option %d  ", 
		p_nbr->id,p_nbr->priority,p_nbr->state,p_nbr->option); 
	
    ospf_logx(ospf_debug_syn, "ospf_syn_nbr_send p_if process_id%x,ifnet_uint %x,addr %x ,type %d  ", 
		p_if->p_process->process_id,p_if->ifnet_uint,p_if->addr,p_if->type);


    p_hdr->len = htons(sizeof(*p_msg));

    ospf_syn_insert_message(p_hdr, p_control);
    return;
}


/*send sync message for large lsa add operation*/
void
ospf_syn_lsa_fragment_send(
           struct ospf_lsa *p_lsa,
           struct ospf_syn_control *p_control)
{
    u_int8 buf[OSPF_SYN_MAX_TXBUF];   
    struct ospf_syn_lsa_fragment *p_syn_lsa = NULL;
    struct ospf_syn_hdr *p_hdr = (void *)buf;
    struct ospf_process *p_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_area *p_area = NULL;
    u_int8 *p_body = NULL;
    u_int fragment_len = OSPF_SYN_MAX_TXBUF - sizeof(struct ospf_syn_lsa_fragment) - sizeof(struct ospf_syn_hdr);
    u_int copy_len = 0;   
    u_int lsbody_len = 0;
    u_int count = 0;
    u_int i = 0;
 
    if (OSPF_MODE_MASTER != ospf.work_mode)
    {
        return;
    }
    ospf.stat.sync.send_lsa_add++;
    
    p_hdr->cmd = OSPF_SYN_TYPE_LSA_FRAGMENT;
    p_hdr->add = TRUE;
    p_syn_lsa = (struct ospf_syn_lsa_fragment*)(buf+sizeof(struct ospf_syn_hdr ));
     
    p_process = p_lsa->p_lstable->p_process;
    p_if = p_lsa->p_lstable->p_if;
    p_area = p_lsa->p_lstable->p_area;
 
    if (NULL != p_process)
    {
        p_syn_lsa->process_id = htonl(p_process->process_id);
    }
    if (NULL != p_area)
    {
        p_syn_lsa->area_id = htonl(p_area->id);
    }
    if (NULL != p_if)
    {
        p_syn_lsa->iftype = p_if->type;
        p_syn_lsa->if_addr = htonl(p_if->addr);
        if (NULL != p_if->p_transit_area )
        {
            p_syn_lsa->tansit_area_id = htonl(p_if->p_transit_area->id);
        }                
        p_syn_lsa->nbr_id= htonl(p_if->nbr);
    }
     
    if (NULL != p_lsa->p_rxmt)
    {
        p_syn_lsa->retransmit_flag = TRUE;
    }      
 
    /*set lsa header*/
    memcpy(&p_syn_lsa->lshdr, p_lsa->lshdr, sizeof(p_syn_lsa->lshdr));
     
    /*get total lsa body length*/
    lsbody_len = ntohs(p_lsa->lshdr[0].len) - sizeof(struct ospf_lshdr);
 
    /*get fragment count*/
    count = (lsbody_len + fragment_len - 1)/fragment_len;
    
    /*build all fragment*/
    p_syn_lsa->fragment_count = count;
 
    p_body = (u_int8 *)(p_lsa->lshdr + 1);
    for (i = 0 ; i < count ; i++)
    {
        p_syn_lsa->fragment = i;
        if (i == (count - 1))
        {
            /*set last fragment length*/
            if ((lsbody_len % fragment_len))
            {
                copy_len = lsbody_len % fragment_len;
            }
            else
            {
                copy_len = fragment_len;
            }
        }
        else
        {
            copy_len = fragment_len;
        }
        p_syn_lsa->buf_len = copy_len;
        memcpy(p_syn_lsa->buf, p_body, copy_len);
        p_body += copy_len;
        p_hdr->len = sizeof(struct ospf_syn_lsa_fragment) + copy_len - sizeof(p_syn_lsa->buf);
        p_hdr->len = htons(p_hdr->len);
        
        ospf_syn_insert_message(p_hdr, p_control);
    }   
    return;
}

/*send sync msg for lsa*/
void 
ospf_syn_lsa_send(
      struct ospf_lsa *p_lsa,
      u_int add,
      struct ospf_syn_control *p_control)
{
    u_int8 buf[OSPF_SYN_MAX_TXBUF];
    struct ospf_syn_hdr *p_hdr = (void *)buf;
    struct ospf_syn_lsa *p_msg = NULL;
    struct ospf_process *p_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_area *p_area = NULL;
    u_int len = 0;

    if (OSPF_MODE_MASTER != ospf.work_mode)
    {
        return;
    }
    
    ospf_logx(ospf_debug_syn, "send syn %s lsa %d/%x/%x,len=%d  ", 
        (add == TRUE) ? "add" : "del", p_lsa->lshdr[0].type, p_lsa->lshdr[0].adv_id,p_lsa->lshdr[0].id,ntohs(p_lsa->lshdr[0].len)); 

    /*do not send msg for process to be deleted*/
    if (p_lsa->p_lstable->p_process->proto_shutdown)
    {
        return;
    }
    memset(buf, 0, sizeof(buf));
    
    /*if lsa's length exceed max buf length,use fragment send*/
    if (TRUE == add)
    {
        len = sizeof(*p_msg)+ntohs(p_lsa->lshdr->len) - sizeof(struct ospf_lshdr);
    
        if (OSPF_SYN_MAX_TXBUF < (len + sizeof(p_hdr)))
        {
            ospf_logx(ospf_debug_syn, "lsa is too long,not syn  ");

            
            /*send sync message for large lsa add operation*/
            ospf_syn_lsa_fragment_send(p_lsa, p_control);
            return;
        }
    }
  
    p_hdr->cmd = OSPF_SYN_TYPE_LSA;
    p_hdr->add = add;
    p_msg = (struct ospf_syn_lsa *)(buf+sizeof(struct ospf_syn_hdr ));
    
    p_process = p_lsa->p_lstable->p_process;
    p_if = p_lsa->p_lstable->p_if;
    p_area = p_lsa->p_lstable->p_area;

    if (NULL != p_process)
    {
        p_msg->process_id = htonl(p_process->process_id);
    }
    if (NULL != p_area)
    {
        p_msg->area_id = htonl(p_area->id);
    }
    if (NULL != p_if)
    {
        p_msg->iftype = htonl(p_if->type);
        p_msg->if_addr = htonl(p_if->addr);
        if (NULL != p_if->p_transit_area )
        {
            p_msg->tansit_area_id = htonl(p_if->p_transit_area->id);
        }                
        p_msg->nbr_id = htonl(p_if->nbr);
    }
    
    if ((NULL != p_lsa->p_rxmt)
        && (p_lsa->p_rxmt->rxmt_count))
    {
        p_msg->retransmit_flag = TRUE;
    }      
    
    if (TRUE == add)
    {
        memcpy(p_msg->lshdr, p_lsa->lshdr, ntohs(p_lsa->lshdr->len));
    
        p_hdr->len = htons(sizeof(*p_msg) + ntohs(p_lsa->lshdr->len) - sizeof(struct ospf_lshdr));

        ospf.stat.sync.send_lsa_add++;
    }
    else
    {
        memcpy(p_msg->lshdr, p_lsa->lshdr, sizeof(struct ospf_lshdr));
    
        p_hdr->len = htons(sizeof(*p_msg));
        
        ospf.stat.sync.send_lsa_del++;
    }
    ospf_syn_insert_message(p_hdr, p_control);
    return;
}

/*send sync msg about lsa's retransmit*/
void 
ospf_syn_lsa_rxmt_send(
                      struct ospf_lsa *p_lsa,
                      u_int add,
                      struct ospf_syn_control *p_control)
{
    u_int8 buf[OSPF_SYN_MAX_TXBUF];
    struct ospf_syn_hdr *p_hdr = (void *)buf;
    struct ospf_syn_lsa *p_msg = NULL;
    struct ospf_process *p_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_area *p_area = NULL;

    if (OSPF_MODE_MASTER != ospf.work_mode)
    {
        return;
    }
    
    ospf_logx(ospf_debug_syn, "send syn %s lsa rxmt %d/%x/%x,len=%d  ", 
        (add == TRUE) ? "add" : "del", p_lsa->lshdr[0].type, p_lsa->lshdr[0].adv_id,p_lsa->lshdr[0].id,ntohs(p_lsa->lshdr[0].len)); 

    /*do not send msg for process to be deleted*/
    if (p_lsa->p_lstable->p_process->proto_shutdown)
    {
        return;
    }
    
    memset(buf, 0, sizeof(buf));
    
    p_hdr->cmd = OSPF_SYN_TYPE_LSA_RXMT;
    p_hdr->add = add;
    p_msg = (struct ospf_syn_lsa *)(buf+sizeof(struct ospf_syn_hdr ));
    
    p_process = p_lsa->p_lstable->p_process;
    p_if = p_lsa->p_lstable->p_if;
    p_area = p_lsa->p_lstable->p_area;

    if (NULL != p_process)
    {
        p_msg->process_id = htonl(p_process->process_id);
    }
    if (NULL != p_area)
    {
        p_msg->area_id = htonl(p_area->id);
    }
    if (NULL != p_if)
    {
        p_msg->iftype = htonl(p_if->type);
        p_msg->if_addr = htonl(p_if->addr);
        if (NULL != p_if->p_transit_area )
        {
            p_msg->tansit_area_id = htonl(p_if->p_transit_area->id);
        }                
        p_msg->nbr_id = htonl(p_if->nbr);
    }
    
    if ((NULL != p_lsa->p_rxmt)
        && (p_lsa->p_rxmt->rxmt_count))
    {
        p_msg->retransmit_flag = TRUE;
    }      
    
    memcpy(p_msg->lshdr, p_lsa->lshdr, sizeof(struct ospf_lshdr));
    p_hdr->len = htons(sizeof(*p_msg));

    ospf_syn_insert_message( p_hdr, p_control);
    return;
}

/*process rxd sync msg for process*/
void 
ospf_syn_process_recv(struct ospf_syn_hdr *p_hdr)
{
    struct ospf_process *p_process = NULL;
    struct ospf_syn_instance *p_syn_ins = (struct ospf_syn_instance *)((u_long)p_hdr + sizeof(struct ospf_syn_hdr));
    u_int process_id;
    u_int  i = 0;
    u_int master_time = 0;
    u_int local_time = 0;

    ospf_logx(ospf_debug_syn, "ospf parse syn %s process %d,routerid %x ", 
        (p_hdr->add == TRUE) ? "add" : "del", ntohl(p_syn_ins->process_id),ntohl(p_syn_ins->router_id)); 

    process_id = ntohl(p_syn_ins->process_id);
    ospfSyncApi(process_id, OSPF_GBL_INSTANCEID, &process_id);

    /*do not process process delete request*/
    if (FALSE == p_hdr->add)
    {
        return;
    }

    /*process must exist*/        
    p_process = ospf_process_lookup(&ospf, process_id);
    if (NULL == p_process)
    {
        return;
    }

    p_process->router_id = ntohl(p_syn_ins->router_id);
    p_process->overflow_limit = ntohl(p_syn_ins->overflow_limit);
    p_process->overflow_period = ntohl(p_syn_ins->overflow_period);
    p_process->reference_rate = ntohl(p_syn_ins->reference_rate);
    p_process->spf_interval = ntohs(p_syn_ins->spf_interval);
    p_process->restart_period = ntohs(p_syn_ins->restart_period);

    for (i = 0; i < OSPF_MAX_PROTOCOL; i++)
    {
        if (p_syn_ins->reditribute_flag[i])
        {
            BIT_LST_SET(p_process->reditribute_flag, i);
        }
        else
        {
            BIT_LST_CLR(p_process->reditribute_flag, i);
        }
    }
        
    p_process->trap_enable = p_syn_ins->trap_enable;
    p_process->asbr = p_syn_ins->asbr;
    p_process->abr = p_syn_ins->abr;
    p_process->opaque = p_syn_ins->opaque;
    p_process->rfc1583_compatibility = p_syn_ins->rfc1583_compatibility;
    p_process->tos_support = p_syn_ins->tos_support;
    p_process->dc_support = p_syn_ins->dc_support;
    p_process->mcast_support = p_syn_ins->mcast_support;
    p_process->stub_support = p_syn_ins->stub_support;
    p_process->stub_adv = p_syn_ins->stub_adv;
    p_process->restart_reason = p_syn_ins->restart_reason;
    p_process->restart_enable = p_syn_ins->restart_enable;
    p_process->restart_helper = p_syn_ins->restart_helper;
    p_process->strictlsa_check = p_syn_ins->strictlsa_check;

    /*md5:add 10s offset for master salve syn */
    master_time = ntohl(p_syn_ins->current_time) + 10;
    local_time = os_system_tick();
    if (master_time > local_time)
    {
        ospf.seq_offset = master_time - local_time;
    }
    else
    {
        ospf.seq_offset = 0;
    }        
    return;
}

/*process rxd sync msg for area*/
void 
ospf_syn_area_recv(struct ospf_syn_hdr *p_hdr)
{
    struct ospf_process *p_process = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_syn_area *p_syn_area = (struct ospf_syn_area *)((u_long)p_hdr + sizeof(struct ospf_syn_hdr));
    u_int i = 0;

    ospf_logx(ospf_debug_syn, "ospf parse syn %s area %d ", (p_hdr->add == TRUE) ? "add" : "del", ntohl(p_syn_area->id)); 

    /*process must exist*/
    p_process = ospf_process_lookup(&ospf, ntohl(p_syn_area->process_id));
    if (NULL == p_process)
    {
        return;
    }
    p_area = ospf_area_lookup(p_process, ntohl(p_syn_area->id));

    /*schedule area's deletion*/ 
    if (FALSE == p_hdr->add)
    {
        if ((NULL != p_area) && (FALSE == ospf_area_if_exist(p_area)))
        {
            ospf_timer_start(&p_area->delete_timer, 2);
        }
        return;
    }
    
    /*create and update area*/
    if (NULL == p_area)
    {
        p_area = ospf_area_create(p_process, ntohl(p_syn_area->id));
    }
        
    if (NULL == p_area)
    {
        return;
    }
    
    p_area->id = ntohl(p_syn_area->id);
    p_area->p_process->process_id = ntohl(p_syn_area->process_id);
    for (i = 0; i < OSPF_MAX_TOS; i++)
    {
        p_area->stub_default_cost[i].cost = ntohs(p_syn_area->stub_default_cost[i].cost);
        p_area->stub_default_cost[i].type = p_syn_area->stub_default_cost[i].type;
        p_area->stub_default_cost[i].status = p_syn_area->stub_default_cost[i].status;
    }
    p_area->authtype = ntohs(p_syn_area->authtype);
    p_area->keyid = p_syn_area->keyid;
    p_area->keylen = p_syn_area->keylen;
    memcpy(p_area->key, p_syn_area->key, (p_area->keylen < (OSPF_MAX_KEY_LEN + 4)) ? p_area->keylen : (OSPF_MAX_KEY_LEN + 4));
    p_area->nssa_default_cost = ntohl(p_syn_area->nssa_default_cost);
    p_area->nssa_cost_type = ntohs(p_syn_area->nssa_cost_type);
    p_area->nssa_wait_time = ntohs(p_syn_area->nssa_wait_time);
    p_area->te_instance = ntohl(p_syn_area->te_instance);
    p_area->nssa_tag = ntohl(p_syn_area->nssa_tag);
    p_area->nssa_always_translate = p_syn_area->nssa_always_translate;
    p_area->nssa_translator = p_syn_area->nssa_translator;
    p_area->is_stub = p_syn_area->is_stub;
    p_area->is_nssa = p_syn_area->is_nssa;
    p_area->nosummary = p_syn_area->nosummary;
    p_area->te_enable = p_syn_area->te_enable;        

    ospf_syn_spf_request(p_process);
    return;
}

/*process rxd sync msg for interface*/
void 
ospf_syn_if_recv(struct ospf_syn_hdr * p_hdr)
{
    struct ospf_process *p_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_syn_if *p_syn_if = (struct ospf_syn_if *)((u_long)p_hdr + sizeof(struct ospf_syn_hdr));
    u_int i = 0;
    u_int iftype = ntohl(p_syn_if->type);
    u_int if_addr = ntohl(p_syn_if->addr);
    u_int if_uint = ntohl(p_syn_if->ifnet_uint);
    u_int nbr_id = ntohl(p_syn_if->nbrid);
    u_int transit_area_id = ntohl(p_syn_if->transit_area_id);

    ospf_logx(ospf_debug_syn, "ospf parse syn %s if %x index%d,state%d ,processid%d", (p_hdr->add == TRUE) ? "add" : "del", ntohl(p_syn_if->addr),ntohl(p_syn_if->ifnet_index),ntohs(p_syn_if->state),ntohl(p_syn_if->process_id)); 

    /*process must exist*/
    p_process = ospf_process_lookup(&ospf, ntohl(p_syn_if->process_id));
    if (NULL == p_process)
    {
        return;
    }
   
    if (OSPF_IFT_VLINK == iftype)
    {
        p_if = ospf_vif_lookup(p_process, transit_area_id,nbr_id);
    }
    else 
    {
        #ifdef OSPF_DCN
	//	printf("%s %d  *****ospf_if_lookup******\n", __FUNCTION__,__LINE__);
		if(p_process->process_id == OSPF_DCN_PROCESS)
		{
			p_if = ospf_if_lookup_forDcnCreat(p_process, if_addr, if_uint);
		}
		#else
        p_if = ospf_if_lookup(p_process, if_addr);
        #endif
		if (p_if != NULL)
		{
		//	printf("%s %d, p_if->addr = 0x%x, p_if->ifnet_uint = 0x%x,if_uint:%x\n", __FUNCTION__,__LINE__, p_if->addr, p_if->ifnet_uint,if_uint);
		}
    }

    /*interface delete */
    if (FALSE == p_hdr->add)
    {
        if (NULL != p_if)
        {
            ospf_if_delete(p_if);
            ospf_syn_spf_request(p_process);
        }
        return;
    }

    /*interface add or update*/
    if (NULL == p_if)
    {
        if (OSPF_IFT_VLINK == iftype)
        {
            p_area = ospf_area_lookup(p_process, transit_area_id);

            p_if = ospf_virtual_if_create(p_process, nbr_id, p_area);
        }
        else
        {
            p_area = ospf_area_lookup(p_process, ntohl(p_syn_if->area_id));
#ifdef OSPF_DCN
            if (p_process->process_id == OSPF_DCN_PROCESS)
            {
                p_if = ospf_Dcn_real_if_create(p_process,if_uint,if_addr,OSPF_DCNDEF_IPMASK, p_area);
            }
            else
#endif
            {
                p_if = ospf_real_if_create(p_process, if_addr, p_area);
            }
        }   
    }

    if (NULL == p_if)
    {
        return;
    }
    p_if->addr = ntohl(p_syn_if->addr);
    p_if->nbr = ntohl(p_syn_if->nbrid);
    if (0 != p_syn_if->transit_area_id) 
    {
        p_if->p_transit_area = ospf_area_lookup(p_process, ntohl(p_syn_if->transit_area_id));
    }
    p_if->type = ntohl(p_syn_if->type);
    p_if->link_up = p_syn_if->link_up;
    p_if->mask = ntohl(p_syn_if->mask);
    p_if->p_area = ospf_area_lookup(p_process, ntohl(p_syn_if->area_id));;
   
    for (i = 0; i < OSPF_MAX_TOS; i++)
    {
        p_if->cost[i] = ntohs(p_syn_if->cost[i]);           
    }
    p_if->dr = ntohl(p_syn_if->dr);
    p_if->bdr = ntohl(p_syn_if->bdr);
    p_if->mtu = ntohs(p_syn_if->mtu);
    p_if->dead_interval = ntohl(p_syn_if->dead_interval);
    p_if->priority = ntohs(p_syn_if->priority);
    p_if->hello_interval = ntohs(p_syn_if->hello_interval);
    p_if->poll_interval = ntohl(p_syn_if->poll_interval);
    p_if->rxmt_interval = ntohs(p_syn_if->rxmt_interval);
    p_if->tx_delay = p_syn_if->tx_delay;
    p_if->state = p_syn_if->state;
    p_if->authtype = p_syn_if->authtype;
    p_if->md5id= p_syn_if->md5id;
    p_if->keylen = p_syn_if->keylen;
    memcpy(p_if->key, p_syn_if->key,(p_if->keylen < (OSPF_MD5_KEY_LEN + 4)) ? p_if->keylen : (OSPF_MD5_KEY_LEN + 4));
    p_if->passive = p_syn_if->passive;
    p_if->mcast = p_syn_if->mcast;
    p_if->demand = p_syn_if->demand;
    p_if->configcost = p_syn_if->configcost;
    p_if->te_enable = p_syn_if->te_enable;
#ifdef HAVE_BFD
    p_if->bfd_enable = p_syn_if->bfd_enable;
#endif
    p_if->fast_dd_enable = p_syn_if->fast_dd_enable;
    p_if->te_instance = ntohl(p_syn_if->te_instance);
    p_if->te_cost = ntohl(p_syn_if->te_cost);
    p_if->te_group = ntohl(p_syn_if->te_group);
    p_if->max_bd = ntohl(p_syn_if->max_bd);
    p_if->max_rsvdbd = ntohl(p_syn_if->max_rsvdbd);
    for (i = 0; i < OSPF_MAX_BD_CLASS; i++)
    {
        p_if->unrsvdbd[i] = ntohs(p_syn_if->unrsvdbd[i]);           
    }
    p_if->flood_group = ntohl(p_syn_if->flood_group);  

    ospf_syn_spf_request(p_process);
    return;
}

/*process rxd sync msg for nbr*/
void 
ospf_syn_nbr_recv(struct ospf_syn_hdr * p_hdr)
{
    struct ospf_process *p_process = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_syn_nbr *p_sys_nbr = (struct ospf_syn_nbr *)((u_long)p_hdr + sizeof(struct ospf_syn_hdr));
    u_int iftype = ntohl(p_sys_nbr->iftype);
    u_int if_addr = ntohl(p_sys_nbr->if_addr);
    u_int nbr_id = ntohl(p_sys_nbr->nbr_id);
    u_int if_uint = ntohl(p_sys_nbr->if_uint);
    u_int transit_area_id = ntohl(p_sys_nbr->transit_area_id);
    u_int old_state;

    ospf_logx(ospf_debug_syn, "ospf parse syn %s nbr %x,dr%x,bdr%x ", (p_hdr->add == TRUE) ? "add" : "del", ntohl(p_sys_nbr->addr), ntohl(p_sys_nbr->dr),ntohl(p_sys_nbr->bdr)); 

    p_process = ospf_process_lookup(&ospf, ntohl(p_sys_nbr->process_id));
    if (NULL == p_process)
    {      
        return;
    }

    if (OSPF_IFT_VLINK == iftype)
    {
        p_if = ospf_vif_lookup(p_process, transit_area_id,nbr_id);
        if (NULL == p_if)
        {
            return;
        }
        p_nbr = ospf_nbr_first(p_if);
    }
    else 
    {
        #ifdef OSPF_DCN
	//	printf("%s %d  *****ospf_if_lookup******\n", __FUNCTION__,__LINE__);
		if(p_process->process_id == OSPF_DCN_PROCESS)
		{
			p_if = ospf_if_lookup_forDcnCreat(p_process, if_addr, if_uint);
		}
		#else
        p_if = ospf_if_lookup(p_process, if_addr);
        #endif

		if (p_if != NULL)
		{
		//	printf("%s %d, p_if->addr = 0x%x, p_if->ifnet_uint = 0x%x,if_uint:%x\n", __FUNCTION__,__LINE__, p_if->addr, p_if->ifnet_uint,if_uint);
		}
        if (NULL == p_if)
        {
            return;
        }
        p_nbr = ospf_nbr_lookup(p_if, ntohl(p_sys_nbr->addr));
    }  

    if (FALSE == p_hdr->add)
    {
        if (NULL != p_nbr)
        {
            ospf_nbr_delete(p_nbr);
            ospf_syn_spf_request(p_process);
        }
        return;
    }

    if (NULL == p_nbr)
    {
        p_nbr = ospf_nbr_create(p_if, ntohl(p_sys_nbr->addr));
    }
    
    if (NULL == p_nbr)
    {
        return;
    }

    p_nbr->id = ntohl(p_sys_nbr->nbr_id);
    p_nbr->addr = ntohl(p_sys_nbr->addr);
    p_nbr->dr = ntohl(p_sys_nbr->dr);
    p_nbr->bdr = ntohl(p_sys_nbr->bdr);
    p_nbr->priority = ntohs(p_sys_nbr->priority);
    p_nbr->option = p_sys_nbr->option;
    p_nbr->opaque_enable = ospf_option_opaque(p_nbr->option);
    old_state = p_nbr->state;
    p_nbr->state = p_sys_nbr->state;
    p_nbr->ulDdMtu = p_sys_nbr->ulDdMtu;
    
    ospf_logx(ospf_debug_syn, "ospf_syn_nbr_recv nbr_id %x,priority %d,state %d ,option %d  ", 
		p_nbr->id,p_nbr->priority,p_nbr->state,p_nbr->option); 
	
    ospf_logx(ospf_debug_syn, "ospf_syn_nbr_recv p_if process_id%x,ifnet_uint %x,addr %x ,type %d  ", 
		p_if->p_process->process_id,p_if->ifnet_uint,p_if->addr,p_if->type); 
	

    ospf_trap_nbrstate(old_state, p_nbr);

   
    if (OSPF_NS_FULL == p_nbr->state)
    {
        /*record time enter FULL state*/
        p_nbr->full_time = os_system_tick();
    }

    ospf_syn_spf_request(p_process);
    return;
}


/*rx sync message for large lsa add operation*/
void 
ospf_syn_lsa_fragment_recv(struct ospf_syn_hdr * p_hdr)
{
    struct ospf_process *p_process = NULL;
    struct ospf_syn_lsa_fragment *p_syn_lsa = (struct ospf_syn_lsa_fragment *)((u_long)p_hdr+sizeof(struct ospf_syn_hdr));
    struct ospf_lsa_fragment *p_fragment = NULL;
    struct ospf_lshdr *p_lshdr = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_lstable *p_table = NULL;
    u_int8 *p_buf = NULL; 
    u_int8 *p_body = NULL;
    u_int i = 0;
    u_int len = 0;
    
    ospf_logx(ospf_debug_syn, "ospf parse syn %s lsa %x/%x/%x, fragment %d", (p_hdr->add==TRUE)?"add":"del",p_syn_lsa->lshdr.type,p_syn_lsa->lshdr.adv_id,p_syn_lsa->lshdr.id, p_syn_lsa->fragment); 
   
    if (TRUE == p_hdr->add)
    {
        ospf.stat.sync.rcv_lsa_add++;
    }
    else 
    {
        ospf.stat.sync.rcv_lsa_del++;   
    }
    p_process = ospf_process_lookup(&ospf, ntohl(p_syn_lsa->process_id));
    if (NULL == p_process)
    {
        return;
    }
    p_area = ospf_area_lookup(p_process, ntohl( p_syn_lsa->area_id));
    if (OSPF_IFT_VLINK == p_syn_lsa->iftype)
    {
        p_if = ospf_vif_lookup(p_process, ntohl(p_syn_lsa->tansit_area_id), ntohl(p_syn_lsa->nbr_id));
    }
    else 
    {
	//	printf("%s %d  *****ospf_if_lookup******\n", __FUNCTION__,__LINE__);
        p_if = ospf_if_lookup(p_process, ntohl(p_syn_lsa->if_addr));
	//	printf("%s %d, p_if = %p\n", __FUNCTION__,__LINE__, p_if);
		if (p_if != NULL)
		{
	//		printf("%s %d, p_if->addr = 0x%x, p_if->ifnet_uint = 0x%x\n", __FUNCTION__,__LINE__, p_if->addr, p_if->ifnet_uint);
		}
    }
    p_table = ospf_lsa_scope_to_lstable(p_process, p_area, p_if, p_syn_lsa->lshdr.type);
    if (NULL == p_table)
    {
        return;
    }
    p_fragment = ospf_lsa_fragment_add(p_process, p_table, &p_syn_lsa->lshdr);
    if (NULL == p_fragment)
    {
        ospf_logx(ospf_debug_syn, "malloc error");
        return;
    }
    
    if (NULL != p_fragment->fragment[p_syn_lsa->fragment].p_buf)
    {
        return;
    }
   
    /*insert buffer*/
    len = ntohl(p_syn_lsa->buf_len);
    p_buf = ospf_malloc(len, OSPF_MPACKET);
    if (NULL == p_buf)
    {
        ospf_logx(ospf_debug_syn, "malloc error");
        return;
    }
    memcpy(p_buf, p_syn_lsa->buf, len);
    
    p_fragment->fragment[p_syn_lsa->fragment].len = len;
    p_fragment->fragment[p_syn_lsa->fragment].p_buf = p_buf;
   
    /*check if all fragments rxd*/
    for (i = 0; i < p_syn_lsa->fragment; i++)
    {
        if (NULL == p_fragment->fragment[i].p_buf)
        {
            ospf_logx(ospf_debug_syn, "rcv lsa fragment lost");
            break;
        }
    }
    if ((p_syn_lsa->fragment + 1) != p_syn_lsa->fragment_count)
    {
        return;
    }
    /*rebuild lsa from fragment*/
    p_lshdr = ospf_malloc(ntohs(p_syn_lsa->lshdr.len), OSPF_MPACKET);
    if (NULL == p_lshdr)
    {
        return;
    }
   
    memcpy(p_lshdr, &p_syn_lsa->lshdr, sizeof(struct ospf_lshdr));
    p_body = (u_int8*)(p_lshdr + 1);
    for (i = 0; i < p_syn_lsa->fragment_count; i++)
    {
        memcpy(p_body, p_fragment->fragment[i].p_buf, p_fragment->fragment[i].len);
        p_body += p_fragment->fragment[i].len;
    }
    
    p_lsa = ospf_lsa_lookup(p_table, &p_syn_lsa->lshdr);
   
    ospf_logx(ospf_debug_syn, "add lsa from fragment");
    ospf_lsa_install(p_table, p_lsa, p_lshdr);
   
    /*free lsa buffer*/
    ospf_mfree(p_lshdr, OSPF_MPACKET);
   
    /*delete fragment*/
    ospf_lsa_fragment_delete(p_fragment);
    return;
}

/*process rxd sync msg for lsa*/
void 
ospf_syn_lsa_recv(struct ospf_syn_hdr * p_hdr)
{
    struct ospf_process *p_process = NULL;
    struct ospf_syn_lsa *p_syn_lsa = (struct ospf_syn_lsa *)((u_long)p_hdr+sizeof(struct ospf_syn_hdr));
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa_fragment *p_fragment = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_lstable *p_table = NULL;
   
    ospf_logx(ospf_debug_syn, "ospf parse syn %s lsa %x/%x/%x", (p_hdr->add==TRUE)?"add":"del",p_syn_lsa->lshdr[0].type,p_syn_lsa->lshdr[0].adv_id,p_syn_lsa->lshdr[0].id); 
   
    if (TRUE == p_hdr->add)
    {
        ospf.stat.sync.rcv_lsa_add++;
    }
    else 
    {
        ospf.stat.sync.rcv_lsa_del++;   
    }
   
    p_process = ospf_process_lookup(&ospf, ntohl(p_syn_lsa->process_id));
    if (NULL == p_process)
    {
        return;
    }
    p_area = ospf_area_lookup(p_process, ntohl( p_syn_lsa->area_id));
    if (OSPF_IFT_VLINK == p_syn_lsa->iftype)
    {
        p_if = ospf_vif_lookup(p_process, ntohl(p_syn_lsa->tansit_area_id), ntohl(p_syn_lsa->nbr_id));
    }
    else 
    {
	//	printf("%s %d  *****ospf_if_lookup******\n", __FUNCTION__,__LINE__);
        p_if = ospf_if_lookup(p_process, ntohl(p_syn_lsa->if_addr));
	//	printf("%s %d, p_if = %p\n", __FUNCTION__,__LINE__, p_if);
		if (p_if != NULL)
		{
	//		printf("%s %d, p_if->addr = 0x%x, p_if->ifnet_uint = 0x%x\n", __FUNCTION__,__LINE__, p_if->addr, p_if->ifnet_uint);
		}
    }
    p_table = ospf_lsa_scope_to_lstable(p_process, p_area, p_if, p_syn_lsa->lshdr->type);
    if (NULL == p_table)
    {
        ospf_logx(ospf_debug_syn, "rcv syn lsa del %d/%x/%x, p_process %x/%d, p_area %x/%d, p_if %x/%x\r\n",p_syn_lsa->lshdr[0].type,p_syn_lsa->lshdr[0].id,p_syn_lsa->lshdr[0].adv_id,
             p_process, p_syn_lsa->process_id, p_area, p_syn_lsa->area_id, p_if, p_syn_lsa->if_addr);
        return;
    }
    
    p_lsa = ospf_lsa_lookup( p_table, p_syn_lsa->lshdr);
   
    if (TRUE == p_hdr->add)
    {
        if (ntohs(p_hdr->len) != sizeof(struct ospf_syn_lsa)+ntohs(p_syn_lsa->lshdr->len)-sizeof(struct ospf_lshdr))
        {
            ospf_logx(ospf_debug_syn, "rcv syn lsa lenth is error");
            ospf.stat.sync.rcv_lsa_error++;
            return;
        }
        if (ospf_debug_gr 
            && (OSPF_LS_ROUTER == p_syn_lsa->lshdr->type
                || OSPF_LS_NETWORK == p_syn_lsa->lshdr->type))
        {
           ospf_logx(ospf_debug_syn, "recv sync lsa,id %x, type %d,seq %x,len %d",
           p_syn_lsa->lshdr->id, p_syn_lsa->lshdr->type,
           ntohl(p_syn_lsa->lshdr->seqnum), ntohs(p_syn_lsa->lshdr->len));
        }
        p_lsa = ospf_lsa_install(p_table, p_lsa, p_syn_lsa->lshdr);
        if (p_lsa && p_syn_lsa->retransmit_flag)
        {
            p_lsa->slave_rxmt = TRUE;
        }
    }
    else if (NULL != p_lsa)
    {
        ospf_lsa_delete(p_lsa);  
        ospf_syn_spf_request(p_process);
   
        
        /*delete fragment if exist*/
        p_fragment = ospf_lsa_fragment_lookup(p_process, p_table, p_syn_lsa->lshdr);
        if (p_fragment)
        {
            ospf_lsa_fragment_delete(p_fragment);
        }
    }
    return;
}

/*process rxd sync msg for lsa's retransmit*/
void 
ospf_syn_lsa_rxmt_recv(struct ospf_syn_hdr *p_hdr)
{
     struct ospf_process *p_process = NULL;
     struct ospf_syn_lsa *p_syn_lsa = (struct ospf_syn_lsa *)((u_long)p_hdr + sizeof(struct ospf_syn_hdr));
     struct ospf_lsa *p_lsa = NULL;
     struct ospf_if *p_if = NULL;
     struct ospf_area *p_area = NULL;
     struct ospf_lstable *p_table = NULL;

     ospf_logx(ospf_debug_syn, "ospf parse syn %s lsa %x/%x/%x", (p_hdr->add == TRUE) ? "add" : "del", p_syn_lsa->lshdr[0].type,p_syn_lsa->lshdr[0].adv_id,p_syn_lsa->lshdr[0].id); 

     p_process = ospf_process_lookup(&ospf, ntohl(p_syn_lsa->process_id));
     if (NULL == p_process)
     {
         return;
     }
     p_area = ospf_area_lookup(p_process, ntohl(p_syn_lsa->area_id));
     if (OSPF_IFT_VLINK == p_syn_lsa->iftype)
     {
         p_if = ospf_vif_lookup(p_process, ntohl(p_syn_lsa->tansit_area_id), ntohl(p_syn_lsa->nbr_id));
     }
     else 
     {
//		 printf("%s %d	*****ospf_if_lookup******\n", __FUNCTION__,__LINE__);
         p_if = ospf_if_lookup(p_process, ntohl(p_syn_lsa->if_addr));
	//	printf("%s %d, p_if = %p\n", __FUNCTION__,__LINE__, p_if);
		if (p_if != NULL)
		{
//			printf("%s %d, p_if->addr = 0x%x, p_if->ifnet_uint = 0x%x\n", __FUNCTION__,__LINE__, p_if->addr, p_if->ifnet_uint);
		}
     }
     p_table = ospf_lsa_scope_to_lstable(p_process, p_area, p_if, p_syn_lsa->lshdr->type);
     if (NULL == p_table)
     {
         return;
     }     
     p_lsa = ospf_lsa_lookup( p_table, p_syn_lsa->lshdr);
     if (p_lsa)
     {
         p_lsa->slave_rxmt = p_syn_lsa->retransmit_flag;
     }
     return;
}

#define OSPF_MAX_LSA_SYN_TIME 1*OSPF_TICK_PER_SECOND/*10s*/

/*send all lsa's sync msg to special sync control block*/
void
ospf_syn_lsa_table_send_all(struct ospf_syn_control *p_control)
{
    struct ospf_process search_process;
    struct ospf_lstable *p_table = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa last_lsa;
    struct ospf_lstable search_lstable;
    struct ospf_area area;
    struct ospf_lshdr *p_next_lsa = (struct ospf_lshdr *)p_control->lshdr;
     u_int start_time = 0;
    u_int now = 0;
    
    p_control->lsa_syn_need = FALSE;
    memset(&search_lstable, 0, sizeof(search_lstable));
    /*start from expected lsa*/
    search_lstable.type = p_next_lsa->type;
    search_lstable.p_process = &search_process;
    search_process.process_id = p_control->process_id;

    if ((OSPF_LS_AS_EXTERNAL == p_next_lsa->type) 
        || (OSPF_LS_TYPE_11 == p_next_lsa->type))
    {
        search_lstable.p_area = NULL;
    }
    else
    {
         search_lstable.p_area = &area;
         area.id = p_control->area_id;
    }    
    for_each_node_noless(&ospf.lsatable_table, p_table, &search_lstable)
    {   
        memcpy(last_lsa.lshdr, p_control->lshdr, sizeof(struct ospf_lshdr));
        
        for_each_node_greater(&p_table->list, p_lsa, &last_lsa)
        {
            ospf_logx(ospf_debug_syn, "init lsa synflag%d %d/%x/%x", p_control->synflag,p_lsa->lshdr[0].type,p_lsa->lshdr[0].id, p_lsa->lshdr[0].adv_id);
 
            ospf_syn_lsa_send(p_lsa, TRUE, p_control);
            now = ospf_sys_ticks();
            
            /*if large lsa table exist,wait for a period if sync sending will spend a long time*/
            if (OSPF_MAX_LSA_SYN_TIME <= ospf_time_differ(now, start_time))
            {
                start_time = now;
                if (p_table->p_area)
                {
                    p_control->area_id = p_table->p_area->id;
                }
                memcpy(p_control->lshdr, p_lsa->lshdr, sizeof(struct ospf_lshdr));
                p_control->process_id = p_table->p_process->process_id;
                ospf_sync_event_set(p_control->lsa_syn_need);
                return;
            } 
        }
        memset(p_control->lshdr, 0, sizeof(struct ospf_lshdr));    
    }
    memset(p_control->lshdr, 0xff, sizeof(struct ospf_lshdr));        
    return;
}

/*send init message to new card when new card up*/
void 
ospf_syn_init_send(struct ospf_syn_control *p_control)
{    
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;

    memset(&ospf.stat.sync, 0, sizeof(ospf.stat.sync));

    for_each_ospf_process(p_process, p_next_process)
    {
        if (p_process->proto_shutdown)
        {
            continue;
        }
        ospf_set_context(p_process);
        ospf_syn_process_send(p_process, TRUE, p_control);

        for_each_ospf_area(p_process, p_area, p_next_area)
        {
            ospf_syn_area_send(p_area, TRUE, p_control);                                     
        }
        
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
            ospf_syn_if_send(p_if, TRUE, p_control);                                         
        }

        /*20140813 send sync for interface again for safe.backup card will
        obtain system interface state when created or updated,if
        system interface state is down,ospf interface will return
        to down state.so send interface again for safe.some other
        method may be used later,but it is the simplest currently*/
      #if 0  
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
            ospf_syn_if_send(p_if, TRUE, p_control);                                         
        }
       #endif 
        for_each_node(&p_process->nbr_table,p_nbr, p_next_nbr)
        {
            ospf_syn_nbr_send(p_nbr, TRUE, p_control);
        }
    }
    memset(p_control->lshdr, 0, sizeof(struct ospf_lshdr));  
    
    ospf_syn_lsa_table_send_all(p_control);

    /*send packet at last*/
    ospf_syn_send(p_control);
    return; 
}

/*check if process has any sync msg to send*/
void 
ospf_syn_check_event(struct ospf_process *p_process)
{    
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_nbr *p_nbr= NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    struct ospf_syn_control *p_control = NULL;
    struct ospf_syn_control *p_next_control = NULL;
    
    if (OSPF_MODE_MASTER != p_process->p_master->work_mode)
    {
        return;
    }

    if (p_process->syn_flag)
    {
        ospf_syn_process_send(p_process, TRUE, NULL);
        p_process->syn_flag = FALSE;
    }

    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        if (p_area->syn_flag)
        {
            ospf_syn_area_send(p_area, TRUE, NULL);
            p_area->syn_flag = FALSE;
        }                           
    }
    
    for_each_ospf_if(p_process, p_if, p_next_if)
    {
        if (p_if->syn_flag)
        {
            ospf_syn_if_send(p_if, TRUE, NULL);
            p_if->syn_flag = FALSE;
        }                                   
    }
    for_each_node(&p_process->nbr_table,p_nbr, p_next_nbr)
    {
        if (p_nbr->syn_flag)
        {
            ospf_syn_nbr_send(p_nbr, TRUE, NULL);
            p_nbr->syn_flag = FALSE;
        }
    }

    for_each_node(&ospf.syn_control_table, p_control, p_next_control)
    {
        if (TRUE == p_control->lsa_syn_need)
        {
            ospf_syn_lsa_table_send_all(p_control);
            ospf_syn_send(p_control);  
        }
        /*delete when all lsa syn finished,and only delete synflag !0*/
        if ((FALSE == p_control->lsa_syn_need) && p_control->synflag)
        {
            ospf_syn_control_delete(p_control);
        }
    }

    /*send last pkt in main task*/
    ospf_syn_send(NULL);     
    return; 
 }

/*backup switch to master*/
void 
ospf_syn_switch_to_master(struct ospf_process *p_process)
{
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    struct ospf_lstable *p_table = NULL;
    struct ospf_lstable *p_next_table = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next_lsa = NULL;
    u_int8 nbr_str[20];
 
    ospf_lstwalkup(&p_process->if_table, ospf_if_state_update);
    ospf_lstwalkup(&p_process->area_table, ospf_router_lsa_originate);
    ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
    p_process->import_update = TRUE;
          
    if (!p_process->restart_enable)
    {
        ospf_lsa_force_refresh(p_process);
    }
 
    /*restart neighbor inactive timer*/
    for_each_ospf_if(p_process, p_if, p_next_if)
    {
        ospf_if_state_update(p_if);
        
        if (OSPF_IFS_DOWN == p_if->state)
        {
            continue;
        }
        ospf_if_hello_timeout(p_if);
        
        if( OSPF_IFS_WAITING == p_if->state)
        {
            ospf_stimer_start(&p_if->wait_timer, p_if->dead_interval);
        } 
        
        for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
        {
            if (OSPF_NS_FULL != p_nbr->state)
            {
                ospf_nsm(p_nbr, OSPF_NE_KILL);
            }
            else
            {
                ospf_stimer_start(&p_nbr->hold_timer, p_if->dead_interval);
                #ifdef HAVE_BFD
                if (((p_nbr->p_if->bfd_enable == OSPF_BFD_GBL_ENABLE)
                    || (p_nbr->p_if->bfd_enable == OSPF_BFD_IF_ENABLE))  
                        && (0 == p_nbr->bfd_discribe))
                {
                    ospf_bind_bfd(p_nbr);
                }
                #endif
            }
        }
    }
    /*retrasmit lsa*/
    for_each_node(&ospf.lsatable_table, p_table, p_next_table)
    {          
        for_each_node(&p_table->list, p_lsa, p_next_lsa)
        {
            if (p_lsa->slave_rxmt)
            {
                ospf_lsa_flood(p_lsa, NULL, NULL);
            }
        }
    }
    ospf_syn_spf_request(p_process);
  
    if (p_process->restart_enable)
    {
        ospf_restart_request(p_process, OSPF_RESTART_UNPLANNED);
    }
    return;
}

/*api for send buffer to lowlayer*/
void
ospf_syn_send(struct ospf_syn_control *p_control)
{
    tlv_t octet;
    int rc = 0;

    if (NULL == p_control)
    {
        p_control = ospf_syn_control_lookup(0);
        if (NULL == p_control)
        {
            return;
        }
    } 
    if ((0 == p_control->len) || (OSPF_MODE_MASTER != ospf.work_mode))
    {
        p_control->len = 0; 
        return;
    }
    ospf.stat.sync.send_pkt_cnt++;
    
    ospf.syn_seq_send++;
    p_control->p_msg->seqnum = ospf.syn_seq_send;
    memcpy(p_control->p_msg->cookie, "OSPF", 4);
    
    octet.len = p_control->len + OSPF_SYN_PKY_HDR_LEN;
    octet.data = (u_int8 *)p_control->p_msg;
    
    ospf_logx(ospf_debug_syn, "send syn pkt ,len=%d, synfalg%d", octet.len,p_control->synflag);

   #if OS != LINUX
   #ifndef WIN32 
   #ifdef USP_MULTIINSTANCE_WANTED
    _ospfSetApi(NULL, OSPF_GBL_SYNPACKET, &octet, (p_control->synflag==0)? USP_SYNC_LOCAL|USP_SYNC_REMOTE:p_control->synflag);
   #else
    ospfSetApi(NULL, OSPF_GBL_SYNPACKET, &octet);
   #endif
   #else
    sync_information_send("ospf", &octet);
   #endif
   #endif

    /*reset length*/
    p_control->len = 0;
    return;
}

/*api for recv buffer from lowlayer*/
void 
ospf_syn_recv(tlv_t *p_octet)
{
    u_int total_len = p_octet->len;
    u_int8 *p_msg = NULL;
    u_int msg_len = 0;
    u_int read_len = 0;
    struct ospf_syn_hdr *p_hdr = NULL;   
    struct ospf_syn_pkt *p_packet = (struct ospf_syn_pkt *)p_octet->data;
   
    ospf_logx(ospf_debug_syn, "ospf rcv syn pkt,len=%d", p_octet->len); 
   
    //if (ospf_get_workmode(&ospf) != OSPF_MODE_SLAVE)
    if (ospf.work_mode != OSPF_MODE_SLAVE)
    {
        ospf_logx(ospf_debug_syn, "work mode is not slave ,ignore"); 
        return;
    }
      
    ospf.stat.sync.rcv_pkt_cnt++;
    if (memcmp(p_packet->cookie, "OSPF", 4))
    {
        ospf_logx(ospf_debug_syn,"rcv syn packet is not OSPF\r\n");
        return;
    }
   
    if (p_packet->seqnum != (ospf.syn_seq_rcv + 1))
    {
        ospf_logx(ospf_debug_syn, "syn packet missing, rcv %d,last rcv %d\r\n",p_packet->seqnum,ospf.syn_seq_rcv);
        ospf.stat.sync.seq_mismatch++;
        log_time_print(ospf.stat.sync.seq_mismatch_time);
    }
    
    ospf.syn_seq_rcv = p_packet->seqnum;
    
    /*process each msg in packet*/
    for (p_msg = p_packet->msg; read_len < total_len  - OSPF_SYN_PKY_HDR_LEN; read_len += msg_len)
    {
        p_hdr = (struct ospf_syn_hdr *)(p_msg+read_len);
        msg_len = ntohs(p_hdr->len)+sizeof(struct ospf_syn_hdr);
   
        ospf.stat.sync.rcv_msg[p_hdr->cmd]++;
   
        switch (p_hdr->cmd) {
            case OSPF_SYN_TYPE_INSTANCE:
                 ospf_syn_process_recv(p_hdr);
                 break;

            case OSPF_SYN_TYPE_AREA:
                 ospf_syn_area_recv(p_hdr);
                 break;

            case OSPF_SYN_TYPE_IF:
                 ospf_syn_if_recv(p_hdr);
                 break;

            case OSPF_SYN_TYPE_NBR:
                 ospf_syn_nbr_recv(p_hdr);
                 break;

            case OSPF_SYN_TYPE_LSA:
                 ospf_syn_lsa_recv(p_hdr);
                 break;

            case OSPF_SYN_TYPE_LSA_FRAGMENT:
                 ospf_syn_lsa_fragment_recv(p_hdr);
                 break; 

            case OSPF_SYN_TYPE_LSA_RXMT:
                 ospf_syn_lsa_rxmt_recv(p_hdr);
                 break;  

            default:
                 break;
        }
    }     
    return;
}


/*lookup compare*/
int
ospf_lsa_fragment_lookup_cmp(
                 struct ospf_lsa_fragment *p1,
                 struct ospf_lsa_fragment *p2)
{
    /*scope*/
    OSPF_KEY_CMP(p1, p2, p_scope);
 
    /*lsa type*/
    OSPF_KEY_CMP(&p1->lshdr, &p2->lshdr, type);
 
    OSPF_KEY_HOST_CMP(&p1->lshdr, &p2->lshdr, id);
 
    OSPF_KEY_HOST_CMP(&p1->lshdr, &p2->lshdr, adv_id);
 
    return 0;
}

/*lookup*/
struct ospf_lsa_fragment *
ospf_lsa_fragment_lookup(
                   struct ospf_process *p_process,
                   void *p_scope,
                   struct ospf_lshdr *p_lshdr)
{
    struct ospf_lsa_fragment fragment;
    fragment.p_scope = p_scope;
    memcpy(&fragment.lshdr, p_lshdr, sizeof(struct ospf_lshdr));
    return ospf_lstlookup(&p_process->fragment_table, &fragment);
}

/*add fragment*/
struct ospf_lsa_fragment *
ospf_lsa_fragment_add(
                   struct ospf_process *p_process,
                   void *p_scope,
                   struct ospf_lshdr *p_lshdr)
{
    struct ospf_lsa_fragment *p_fragment = ospf_lsa_fragment_lookup(p_process, p_scope, p_lshdr);
    if (NULL == p_fragment)
    {
        p_fragment = ospf_malloc(sizeof(struct ospf_lsa_fragment), OSPF_MPACKET);
        if (NULL == p_fragment)
        {
            return NULL;
        }
        memset(p_fragment, 0 ,sizeof(struct ospf_lsa_fragment));
        memcpy(&p_fragment->lshdr, p_lshdr,sizeof(struct ospf_lshdr));
        p_fragment->p_scope = p_scope;
        p_fragment->p_process = p_process;
        ospf_lstadd(&p_process->fragment_table, p_fragment);
    }
    return p_fragment;
}

/*free fragment*/
void
ospf_lsa_fragment_delete(struct ospf_lsa_fragment *p_fragment)
{
    u_int i = 0;
    
    ospf_lstdel(&p_fragment->p_process->fragment_table, p_fragment);

    for (i = 0; i < p_fragment->fragment_count; i++)
    {
        if (p_fragment->fragment[i].p_buf)
        {
            ospf_mfree(p_fragment->fragment[i].p_buf, OSPF_MPACKET);
            p_fragment->fragment[i].p_buf = NULL;
        }
    }
    ospf_mfree(p_fragment, OSPF_MPACKET);
    return;
}
void ospf_syn_hdr_print(struct ospf_syn_hdr * p_hdr)
{

	if(p_hdr == NULL)
	{
		return;
	}
	vty_out_to_all_terminal("##ospf_syn_hdr_print p_hdr 0x%x:##",p_hdr);
	
	vty_out_to_all_terminal("## cmd:0x%x,add %x,len %x,seq_num %x",
		p_hdr->cmd,p_hdr->add,p_hdr->len,p_hdr->seq_num);


}

void ospf_syn_nbr_print(struct ospf_syn_nbr* p_nbr)
{

	if(p_nbr == NULL)
	{
		return;
	}
	vty_out_to_all_terminal("##ospf_syn_nbr_print p_nbr 0x%x:##",p_nbr);
	
	vty_out_to_all_terminal("## process_id:0x%x,iftype %x,if_addr %x,",
		p_nbr->process_id,p_nbr->iftype,p_nbr->if_addr);
	vty_out_to_all_terminal("## transit_area_id %x,nbr_id  %x,addr %x,",
		p_nbr->transit_area_id,p_nbr->nbr_id,p_nbr->addr);
	vty_out_to_all_terminal("## dr %x,bdr %x,priority %x,",
		p_nbr->dr,p_nbr->bdr,p_nbr->priority);
	vty_out_to_all_terminal("## option %x,state %x ##",
		p_nbr->option,p_nbr->state);

}

#ifdef OSPF_MASTER_SLAVE_SYNC
void ospf_slaver_card_up()
{
	
    struct ospf_syn_control *p_control = NULL;

    ospf.stat.syn_done_msg_cnt ++;
    memcpy(ospf.stat.syn_done_msg_time_old,ospf.stat.syn_done_msg_time,sizeof(ospf.stat.syn_done_msg_time));
    log_time_print(ospf.stat.syn_done_msg_time);
	
	/*add sync control for new system,and send init sync msg*/ 
	p_control = ospf_syn_control_create(0);
	
	if (p_control)
	{
		ospf_syn_init_send(p_control);
	} 

}
#endif
