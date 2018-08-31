/* ospf_update.c - ls update message io processing*/

#include "ospf.h"


void ospf_update_lsa_input(struct ospf_nbr * p_nbr, struct ospf_lshdr * p_lsa );
u_int ospf_lsa_checksum_verify(struct ospf_lshdr * p_lsa);
u_int ospf_lsa_rxmt_add(struct ospf_if * p_if,  struct ospf_lsa *p_current, struct ospf_nbr * p_rx_nbr);
void ospf_single_update_output(struct ospf_if *p_if, struct ospf_nbr *p_nbr, struct ospf_lsa *p_lsa);

/* section 13 of OSPF specification (page 133-136) */
void 
ospf_update_input (
              struct ospf_nbr *p_nbr,
              struct ospf_hdr *p_hdr)
{
    struct ospf_update_msg  *p_update = (struct ospf_update_msg  *)p_hdr ;
    struct ospf_lshdr *p_lshdr = NULL;
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_ackinfo direct_ack;
    u_int8 ackbuf[OSPF_MAX_TXBUF];
    u_int reqcount = 0;
    int count = 0;
    u_int8 nbr[16];
    
    ospf_logx(ospf_debug_lsa, "recv update from %s on %s", ospf_inet_ntoa(nbr,p_nbr->addr), p_if->name);
    //ospf_logx(ospf_debug_lsa,"recv update from %s on %s.\r\n", ospf_inet_ntoa(nbr,p_nbr->addr), p_if->name);

    if (ospf_debug && ospf_debug_msg)
    {
        ospf_log_update_packet(p_update, TRUE);
    }
    
    /* section 13 (page 133) */
    if (OSPF_NS_EXCHANGE > p_nbr->state)                                
    {
        p_if->stat->error[OSPF_IFERR_UPD_NBR]++;
        ospf_logx(ospf_debug_lsa, "nbr is in invalid state");        
        return;
    }

    /*prepare direct ack*/
    direct_ack.p_if = p_if;
    direct_ack.p_msg = (struct ospf_hdr*)ackbuf;
    memset(direct_ack.p_msg, 0, 128);
    direct_ack.p_msg->type = OSPF_PACKET_ACK;
    direct_ack.p_msg->len = OSPF_PACKET_HLEN;
    p_if->p_direct_ack = &direct_ack;
    
    /*record current request count*/
    reqcount = ospf_lstcnt(&p_if->p_process->req_table);

    /*process each lsa rxd*/
    count = ntohl(p_update->lscount);    
    p_if->stat->rx_lsainfo[OSPF_PACKET_UPDATE] += count;
    p_lshdr = p_update->lshdr;
	
    for (; count > 0; --count, p_lshdr = (struct ospf_lshdr*)(((u_long)p_lshdr) + ntohs(p_lshdr->len)))
    {
       ospf_logx(ospf_debug_lsa,"%d: len=%d.\n",__LINE__, ntohs(p_lshdr->len));
        ospf_update_lsa_input (p_nbr, p_lshdr);
    }    
	
	ospf_translate_type10_to_tedb(p_if->p_area);
    /*send direct ack buffer*/    
    ospf_ack_output(p_if->p_direct_ack); 
    p_if->p_direct_ack = NULL;
    
    /* if any requests were acknowledged, send new request */    
    if ((OSPF_NS_EXSTART < p_nbr->state) && (reqcount != ospf_lstcnt(&p_if->p_process->req_table)))
    {
        /*decide if expected update all rxd,if so,send request,if not,start a fast timer for retransmiting*/
        if (p_nbr->lsa_reply >= p_nbr->lsa_requested)
        {
            ospf_request_output(p_nbr);
        }
        else
        {
            ospf_timer_start(&p_nbr->request_timer, 5);
        }
    }    
    return;
}

void 
ospf_same_recent_lsa_input(
                  struct ospf_nbr *p_nbr,                              
                  struct ospf_lsa *p_lsa) 
{
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_lstable *p_lstable = ospf_lstable_lookup(p_if, p_lsa->lshdr->type);
    struct ospf_request_node *p_req = ospf_request_lookup(p_lstable, p_lsa->lshdr);
    u_int8 lsstr[100];

    /* section 13, item (6) - (page 135) */
    /*error occurred in DD exchange*/
    if (NULL != p_req && (p_nbr == p_req->p_nbr))                        
    {
        ospf_logx(ospf_debug_lsa, "request exist for same lsa %s", ospf_print_lshdr(p_lsa->lshdr, lsstr));        
        
        ospf_nsm(p_nbr, OSPF_NE_BAD_REQUEST);
        return;
    }
     
    /* section 13, item (7a) - (page 135) */
    ospf_logx(ospf_debug_lsa, "rcvd lsa is same as local lsa");
    
    /*clear retransmit list if necessary*/
    if (!ospf_nbr_rxmt_isset(p_lsa->p_rxmt, p_nbr))
    {
        ospf_ack_add(p_if->p_direct_ack, p_lsa->lshdr); 
        return;
    }    
    ospf_logx(ospf_debug_lsa, "remove lsa from retransmit list");


    ospf_nbr_rxmt_delete(p_nbr, p_lsa->p_rxmt);
    /* section 13, item (7b) - (page 135) */
    if ((OSPF_IFS_BDR == p_if->state) && (p_if->dr == p_nbr->addr))
    {           
        ospf_ack_add(&p_if->delay_ack, p_lsa->lshdr);
        /*start delay ack timer*/
        ospf_delay_ack_timer_start(p_if);
    }
    return;  
}

/*when receive update,if local has more recent copy,call this function.
called by ospf_update_lsa_input
value is in ms not seconds
*/
void 
ospf_less_recent_lsa_input(
                  struct ospf_nbr *p_nbr,                              
                  struct ospf_lsa *p_lsa,
                  struct ospf_lshdr *p_new) 
{
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_lstable *p_lstable = ospf_lstable_lookup(p_if, p_lsa->lshdr->type);
    struct ospf_request_node *p_req = ospf_request_lookup(p_lstable, p_lsa->lshdr);
    u_int now;
    u_int8 lsstr[100];

    /* section 13, item (6) - (page 135) */
    /*error occurred in DD exchange*/
    if (NULL != p_req && (p_nbr == p_req->p_nbr))                        
    {
        ospf_logx(ospf_debug_lsa, "request exist for less lsa %s", ospf_print_lshdr(p_new, lsstr));        
        
        ospf_nsm(p_nbr, OSPF_NE_BAD_REQUEST);
        return;
    }
    
    ospf_logx(ospf_debug_lsa, "rcvd lsa is less recent than local lsa");
    
    /* section 13, item (8) - (page 135) */
    if (OSPF_MAX_LSAGE <= (ntohs(p_lsa->lshdr->age)) 
        && (OSPF_MAX_LS_SEQNUM == ntohl(p_lsa->lshdr->seqnum)))
    {
        ospf_logx(ospf_debug_lsa, "stop processing maxage lsa");        
        return;
    }

    /*record rxd expired lsa,used for testcenter*/
    if (OSPF_MAX_LSAGE <= (ntohs(p_lsa->lshdr->age))
        && (!ospf_is_self_lsa (p_if->p_process, p_lsa->lshdr)))
    {
        if (!ospf_timer_active(&p_nbr->lsa_error_timer))
        {
            ospf_stimer_start(&p_nbr->lsa_error_timer, 1);
        }
        p_nbr->lsa_error_cnt++;
    }

    /* Refer section 13, item (8)   */ 
    /* RFC 2178 G.4 */
    now = ospf_sys_ticks();    
    if (OSPF_MIN_LS_RX_INTERVAL > ospf_time_differ(now, p_lsa->rx_time))
    {
        ospf_logx(ospf_debug_lsa, "ignore rcvd lsa in one second");
        return ;
    }

    ospf_logx(ospf_debug_lsa, "send current lsa back to neighbor");

    /*send local copy back*/
    ospf_update_buffer_insert(&p_if->update, p_lsa, NULL);
    return;
}

/* section 13, item (5) - (page 134) */
void 
ospf_more_recent_lsa_input (
                      struct ospf_nbr *p_nbr,
                      struct ospf_lsa *p_lsa,
                      struct ospf_lshdr *p_lshdr)
{
    struct ospf_lsa *p_new = NULL;
    struct ospf_if *p_if = p_nbr->p_if ;
    struct ospf_process *p_process = p_if->p_process;
    struct ospf_lstable *p_table = NULL;
    u_int flood_back = FALSE;
    u_int8 self = ospf_is_self_lsa (p_process, p_lshdr);
    u_int now = 0;
    

    /* section 13, item (5a) - (page 134) */
    ospf_logx(ospf_debug_lsa, "rcvd lsa is more recent,accept it");

    /*20121129*/
    if (ospf_debug_gr
         && p_if->p_process->in_restart 
         && (OSPF_LS_ROUTER == p_lshdr->type || OSPF_LS_NETWORK == p_lshdr->type))
    {
        ospf_logx(ospf_debug_lsa, "accept lsa during restart");
    }    

    p_table = ospf_lstable_lookup(p_if, p_lshdr->type);

    /* RFC 2178 G.5 */
    now = ospf_sys_ticks();    
    if ((NULL != p_lsa)
        && (!self)
        && (OSPF_MIN_LS_RX_INTERVAL > ospf_time_differ(now, p_lsa->rx_time)))
    {
        ospf_logx(ospf_debug_lsa|ospf_debug_gr, "discard rxd lsa too fastly");
        return;
    }
    
    /* section 13, item (5f) - (page 135) */
    if (self && (!p_process->in_restart))
    {
        ospf_logx(ospf_debug_lsa, "process self originated lsa");

        if (NULL == p_lsa)
        {
            /*install a maxage self originated lsa,it will be flushed quickly*/
            p_lshdr->age = htons(OSPF_MAX_LSAGE);
            ospf_lsa_install(p_table, NULL, p_lshdr);
        }
        else
        {
            /*originate new instance with next sequence*/
            p_lsa->lshdr->seqnum = p_lshdr->seqnum;
            ospf_lsa_refresh (p_lsa);
        }
        return;
    }
/*test for 256 nbr*/
#if 0
if (p_process->process_id!= 1)
{
    /*schedule delay ack*/ 
    ospf_ack_add(&p_if->delay_ack, p_lshdr);
    
    /*start delay ack timer*/
    ospf_delay_ack_timer_start(p_if);
 //   printf("%s %d  *****ospf_ack_add******\n", __FUNCTION__,__LINE__);
    return;
}

#endif
   /*process for reset routerid:clear invalid network lsa*/
//	printf("%s %d  *****ospf_if_lookup******\n", __FUNCTION__,__LINE__);
    if (!self 
        && OSPF_LS_NETWORK == p_lshdr->type
        && ospf_if_lookup(p_process, ntohl(p_lshdr->id)))
    {
	//	printf("%s %d  ***********\n", __FUNCTION__,__LINE__);
        /*install a maxage self originated lsa,it will be flushed quickly*/
        p_lshdr->age = htons(OSPF_MAX_LSAGE);
        ospf_lsa_install(p_table, p_lsa, p_lshdr);
        return;
    }

    /*rxd lsa is orginated by last process instance,flush it*/
    if (p_process->old_routerid
       && (p_process->old_routerid == ntohl(p_lshdr->adv_id)))
    {
        /*install a maxage self originated lsa,it will be flushed quickly*/
        p_lshdr->age = htons(OSPF_MAX_LSAGE - 30);
        ospf_lsa_install(p_table, p_lsa, p_lshdr);
//	printf("%s %d  *****ospf_lsa_install******\n", __FUNCTION__,__LINE__);
        return;
    }
    /*install lsa*/
    /* section 13, item (5d) - (page 134) */     
    p_new = ospf_lsa_install(p_table, p_lsa, p_lshdr);
    if (NULL == p_new)
    {
//	printf("%s %d  *****(NULL == p_new)******\n", __FUNCTION__,__LINE__);
        return;
    }

    /*special for rxd maxaged lsa,it is flooded blow,do not flood again when aging timer checking*/
    if (OSPF_MAX_LSAGE <= ntohs(p_new->lshdr->age))
    {
        /*set expired flag*/
        p_new->expired = TRUE;
        ospf_trap_maxage(p_new); 
    }
    
    /* section 13, item (5b) - (page 143) */          
    flood_back = ospf_lsa_flood(p_new, p_if, p_nbr);
    if (flood_back)
    {
       /*table1:do not send any ack if flooding-back*/
       ospf_logx(ospf_debug_lsa, "lsa is flood back,do not send ack");
       return;
    }

   /*table 2: non-bdr will send ack, bdr send ack if lsa rxd from dr*/
    if ((OSPF_IFS_BDR == p_if->state) && (p_if->dr != p_nbr->addr))
    {
        ospf_logx(ospf_debug_lsa, "bdr no send delay ack");
        return;
    }
    ospf_logx(ospf_debug_lsa, "schedule delay ack for lsa");

    /*schedule delay ack*/ 
    ospf_ack_add(&p_if->delay_ack, p_lshdr);
    
    /*start delay ack timer*/
    ospf_delay_ack_timer_start(p_if);
    return;
}

/* section 13 of OSPF specification (page 133-136) */
void 
ospf_update_lsa_input (
              struct ospf_nbr *p_nbr,
              struct ospf_lshdr *p_lshdr)
{
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_process *p_process = p_if->p_process;
    struct ospf_request_node *p_req = NULL;
    struct ospf_lstable *p_lstable = ospf_lstable_lookup(p_if, p_lshdr->type);
    u_int8 state_mask[4] = {0};
    signed long rx_better = 0;
    u_int8 str[100];
    
    ospf_logx(ospf_debug_lsa, "proc rcvd lsa,%s", ospf_print_lshdr(p_lshdr, str));

    if (ospf_debug_lsa && ospf_debug_msg)
    {
        ospf_logx(ospf_debug_lsa, "  lsa");
        ospf_log_lsa_header(p_lshdr);
        ospf_logx(ospf_debug_lsa, " ");
    }
    
    ospf.stat.rxd_lsa[p_lshdr->type]++;

    
    if (ospf_debug_gr
         && p_if->p_process->in_restart 
         && (OSPF_LS_ROUTER == p_lshdr->type || OSPF_LS_NETWORK == p_lshdr->type))
    {
        ospf_logx(ospf_debug_lsa,"rx lsa from %s, type %d, id %x, age %d, seq %x,len %d", 
         ospf_inet_ntoa(str, p_nbr->addr), p_lshdr->type, ntohl(p_lshdr->id),ntohs(p_lshdr->age),
         ntohl(p_lshdr->seqnum), ntohs(p_lshdr->len));
    }    
    
    /* section 13, item (1) - (page 133) */
    if (!ospf_lsa_checksum_verify (p_lshdr))
    {
        ospf_logx(ospf_debug_lsa, "lsa checksum is invalid,ignore it");
        return; 
    }
    /*verify lsa type*/
    if (!ospf_lstype_is_valid(p_if, p_lshdr->type))
    {
        return; 
    }
    /*verify lsa header*/
    if (!ospf_lshdr_is_valid(p_lshdr))
    {
        return; 
    }
    
    /* section 13, item (4) - (page 133-134) */    
    p_lsa = ospf_lsa_lookup(p_lstable, p_lshdr);
    
    if (NULL == p_lsa) 
    {
        ospf_logx(ospf_debug_lsa, "local lsa not exist");
    }
    else/*20130815*/
    {
        ospf_logx(ospf_debug_lsa, "local lsa exist");
        if (ospf_debug_lsa && ospf_debug_msg)
        {
            ospf_logx(ospf_debug_lsa, "  lsa");
            ospf_log_lsa_header(p_lsa->lshdr);
            ospf_logx(ospf_debug_lsa, " ");
        }
    }
    /*rxd expired lsa,and local lsdb has no this lsa,send direct ack*/
    /*set state mask,check exchange or loading nbr*/
    BIT_LST_SET(state_mask, OSPF_NS_EXCHANGE);
    BIT_LST_SET(state_mask, OSPF_NS_LOADING);
    
    /*neighbor state must be loading or exchange*/
    if ((OSPF_MAX_LSAGE <= ntohs(p_lshdr->age))
        && (NULL == p_lsa)
        && (!ospf_nbr_count_in_state(p_process, state_mask)))
    {
        /* section 13, item (4a) - (page 134) */
        ospf_logx(ospf_debug_lsa, "rcvd new lsa has maxage,send direct ack");
        
        ospf_ack_add(p_if->p_direct_ack, p_lshdr);        
        return;
    }

    /* section 13, item (5) - (page 134) */
    /*check if rxd lsa is more recent than local instance*/
    rx_better = (NULL != p_lsa) ? ospf_lshdr_cmp (p_lshdr, p_lsa->lshdr) : 1 ;
    if (0 < rx_better)
    {
        ospf.stat.rxd_more_recent_lsa[p_lshdr->type]++;

        /* section 13, items (5a-5f) - (page 134-135) */
        ospf_more_recent_lsa_input(p_nbr, p_lsa, p_lshdr);
        
        /*check all request node,if request node lsa is less recent,delete request node*/
        
        p_req = ospf_request_lookup(p_lstable, p_lshdr);
        /*check if rxd lsa match the request node,it must be more rencent or 100same as requested lsa*/              
        /*20150522  add (p_req->p_nbr == p_nbr) for test center to avoid
        nbr state change between Exchange and Exstart*/
        if ((NULL != p_req)
            && ((0 >= ospf_lshdr_cmp(&p_req->ls_hdr, p_lshdr))
               || (p_req->p_nbr == p_nbr)))
        {
            ospf_logx(ospf_debug_lsa, "delete lsa from nbr request list"); 
                
            ospf_request_delete(p_req);                
        }        
    }
    else if (0 > rx_better)
    {
        ospf.stat.rxd_less_recent_lsa[p_lshdr->type]++; 

        ospf_less_recent_lsa_input(p_nbr, p_lsa, p_lshdr);
    }
    else
    {
        ospf.stat.rxd_same_recent_lsa[p_lshdr->type]++; 

        ospf_same_recent_lsa_input(p_nbr, p_lsa);
    }
    return;
}

/* section 13, item (1) - (page 133) */
u_int 
ospf_lsa_checksum_verify (struct ospf_lshdr *p_lshdr)
{
    u_short ocks = 0;
    u_short cks = 0;    
    u_short age = 0;
    u_short len = ntohs(p_lshdr->len);
    u_int test = TRUE;

    /*save age in network order*/           
    age = p_lshdr->age;
    ocks = p_lshdr->checksum;
    
    /* reset age before calculation*/
    p_lshdr->age = 0x0000;                
    p_lshdr->checksum = 0x0000;
    
    cks = ospf_lsa_checksum_calculate(p_lshdr);
    if (cks != ocks)
    {
       ospf_logx(ospf_debug_lsa, "rxd checksum=%x, new checksum=%x, len=%d", ocks, cks,len);
       test = FALSE;
    }
    
    /* restore age to its previous value except keep it in host order */
    p_lshdr->age = age;                
    p_lshdr->checksum = ocks;
    return test;
}

/* section 13.3 of OSPF specification (page 138) */
void 
ospf_lsa_retransmit (struct ospf_nbr *p_nbr) 
{
    u_int8 buf[OSPF_MAX_TXBUF + OSPF_MD5_KEY_LEN];
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_process *p_process = p_if->p_process;
    struct ospf_lst *rxmt_table = &p_process->rxmt_table;
    struct ospf_retransmit *p_rxmt = NULL;
    struct ospf_retransmit *p_next_rxmt = NULL;
    struct ospf_updateinfo update;
    u_int lscount = 0;
    u_int max_len = p_if->maxlen;
    u_int update_len = OSPF_UPDATE_MIN_LEN;
    u_int ls_len = 0;
    u_int total = 0;
    u_int8 nbr[16];   

    ospf_logx(ospf_debug_lsa, "prepare send update on if %s", ospf_inet_ntoa(nbr,p_if->addr));
    
    /*clear retransmit lsa and acked lsa*/
    p_nbr->lsa_rxmted = 0;
    p_nbr->lsa_acked = 0;

    /*if rxmt limit is running,do not send retransmit now*/
    if (ospf_timer_active(&p_process->rxmtlimit_timer))
    {
        return;
    }
    /*set update buffer*/
    update.p_nbr = p_nbr;
    update.p_if = NULL;
    update.p_msg = NULL;
    update.maxlen = 0;

    for_each_node(rxmt_table, p_rxmt, p_next_rxmt)
    {
        if (NULL == p_rxmt->p_lsa) 
        {
            continue;
        }
         /*neighbor must be on retransmit list*/
        if (!ospf_nbr_rxmt_isset(p_rxmt, p_nbr))
        {
            continue;
        }
        p_process->trap_packet = OSPF_PACKET_UPDATE;
        if (OSPF_IFT_VLINK != p_if->type)
        {
            ospf_trap_ifretransmit(p_rxmt->p_lsa->lshdr, p_if, p_nbr);
        }
        else 
        {
            ospf_trap_vifretransmit(p_rxmt->p_lsa->lshdr, p_if);
        }
        
        ls_len = ntohs(p_rxmt->p_lsa->lshdr->len);
 
        /*update length exceed mtu,need send packet*/
        if (((update_len + ls_len) > max_len) && (0 != lscount))
        {
            break;
        }
        lscount++;
        /*increase retransmitted lsa count*/
        p_nbr->lsa_rxmted++;
        update_len += ls_len;
        
        /*add lsa to neighbor's update list*/
        ospf_update_buffer_insert(&update, p_rxmt->p_lsa, buf);
 
        if (OSPF_LS_TYPE_9 == p_rxmt->p_lsa->lshdr->type)
        {
            ospf_logx(ospf_debug_lsa, "send grace lsa on %s", ospf_inet_ntoa(nbr,p_if->addr));
        }
        /*all rxmt inserted,stop*/
        total++;
        if (total >= p_nbr->rxmt_count)
        {
            break;
        }
    }
	
    /*send update if necessary*/
    ospf_update_output(&update);

    /*limit retransmit rate,if send too many retransmit,stop sending for a period*/
    if (p_process->lsa_rxmt_limit)
    {
        p_process->lsa_rxmt_limit--;
    }
    if (0 == p_process->lsa_rxmt_limit)
    {
        ospf_timer_start(&p_process->rxmtlimit_timer, 1);
    }
    return;
}

/*check if a lsa can be flooded on special interface
  return TRUE ,it can be flooded,FALSE,can not be flooded*/ 
u_int 
ospf_lsa_flood_verify(
              struct ospf_lsa *p_lsa,
              struct ospf_if *p_if ,
              struct ospf_if *p_rx_if,
              struct ospf_nbr *p_rx_nbr)
{
    struct ospf_area *p_area = p_lsa->p_lstable->p_area;
    u_int8 lstype = p_lsa->lshdr->type;
    
    /*area must exist*/
    if ((NULL == p_if) || (NULL == p_if->p_area)
         || (NULL == p_lsa) || (OSPF_IFS_DOWN == p_if->state))
    {
        return FALSE;
    }
//    ospf_logx(ospf_debug_lsa,"ifaddr %x area id %d lsa area %d lsa type %d adv %x id %x",p_if->addr,p_if->p_area->id, p_lsa->p_lstable->p_area->id,p_lsa->lshdr[0].type,p_lsa->lshdr[0].adv_id,p_lsa->lshdr[0].id);
//	printf("ospf_lsa_flood_verify:222  p_if %p p_lsa %p,p_rx_if %p p_rx_nbr %p %s:%d\n", p_if,p_lsa, p_rx_if, p_rx_nbr, __FUNCTION__, __LINE__);
	//ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_if %p p_lsa %p,p_rx_if %p p_rx_nbr %p %s:%d\n", p_if,p_lsa, p_rx_if, p_rx_nbr, __FUNCTION__, __LINE__);
    /*flooding group*/
    if (p_rx_if
        && (p_rx_if != p_if)
        && (0 != p_rx_if->flood_group) 
        && (p_rx_if->flood_group == p_if->flood_group))
    {
//		printf("ospf_lsa_flood_verify:222  p_if %p ,p_rx_if %p ,p_rx_if->flood_group %x,p_if->flood_group %x%s:%d\n", p_if, p_rx_if, p_rx_if->flood_group,p_if->flood_group, __FUNCTION__, __LINE__);
	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_if %p ,p_rx_if %p ,p_rx_if->flood_group %x,p_if->flood_group %x%d\n", p_if, p_rx_if, p_rx_if->flood_group,p_if->flood_group, __LINE__);
        return FALSE;
    }
    
    /* section 13.3, item (3) - (page 140) :do not flood on same if from dr/bdr*/
    if ((NULL !=  p_rx_if) && (NULL != p_rx_nbr))
    {
        if ((p_if == p_rx_if) 
            && ((p_rx_if->dr == p_rx_nbr->addr) 
            || (p_rx_if->bdr == p_rx_nbr->addr)))
        {
	//		printf("ospf_lsa_flood_verify:222  p_if %p ,p_rx_if %p ,p_rx_if:dr= %x,bdr= %x,p_rx_nbr->addr %x%s:%d\n", p_if, p_rx_if, p_rx_if->dr,p_rx_if->bdr,p_rx_nbr->addr, __FUNCTION__, __LINE__);
 	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_if %p ,p_rx_if %p ,p_rx_if:dr= %x,bdr= %x,p_rx_nbr->addr %x%d\n", p_if, p_rx_if, p_rx_if->dr,p_rx_if->bdr,p_rx_nbr->addr, __LINE__);
           return FALSE;
        }
    }
    /*if rx interface has only one nbr,do not send back*/
    if ((p_if == p_rx_if) && (1 == ospf_lstcnt(&p_if->nbr_table)))
    {
//		printf("ospf_lsa_flood_verify:222  p_if %p ,p_rx_if %p ,ospf_lstcnt(&p_if->nbr_table):%d%s:%d\n", p_if, p_rx_if, ospf_lstcnt(&p_if->nbr_table), __FUNCTION__, __LINE__);
	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_if %p ,p_rx_if %p ,ospf_lstcnt(&p_if->nbr_table):%d%d\n", p_if, p_rx_if, ospf_lstcnt(&p_if->nbr_table), __LINE__);
        return FALSE;
    }
//	printf("ospf_lsa_flood_verify:222  p_if %p p_lsa %p,p_rx_if %p p_rx_nbr %p %s:%d\n", p_if,p_lsa, p_rx_if, p_rx_nbr, __FUNCTION__, __LINE__);
    
	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_if %p p_lsa %p,p_rx_if %p p_rx_nbr %p %d\n", p_if,p_lsa, p_rx_if, p_rx_nbr, __LINE__);
    /* section 13.3, item (4) - (page 140) */
    if ((p_if == p_rx_if) 
        && (OSPF_IFS_BDR == p_if->state)
        && (OSPF_LS_TYPE_9 != lstype))
    {
//		printf("ospf_lsa_flood_verify:222  p_if %p , p_rx_if %p,p_if->state %d ,lstype:%d%s:%d\n", p_if,p_rx_if, p_if->state, lstype, __FUNCTION__, __LINE__);
	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_if %p , p_rx_if %p,p_if->state %d ,lstype:%d%d\n", p_if,p_rx_if, p_if->state, lstype, __LINE__);
        return FALSE;
    }
    
    /*for ase lsa, must not flood on virtual link,or stub,nssa area*/
    if ((OSPF_LS_AS_EXTERNAL == lstype) || (OSPF_LS_TYPE_11 == lstype))
    {
        if (OSPF_IFT_VLINK == p_if->type)
        {
	//		printf("ospf_lsa_flood_verify:222  p_if %p ,p_if->type %d ,lstype:%d%s:%d\n", p_if, p_if->type, lstype, __FUNCTION__, __LINE__);
	   ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_if %p ,p_if->type %d ,lstype:%d%d\n", p_if, p_if->type, lstype, __LINE__);
            return FALSE;
        }
   
        if (p_if->p_area->is_stub || p_if->p_area->is_nssa)
        {
	//		printf("ospf_lsa_flood_verify:222  p_if %p ,p_if->p_area->is_stub %d ,p_if->p_area->is_nssa:%d%s:%d\n", p_if, p_if->p_area->is_stub, p_if->p_area->is_nssa, __FUNCTION__, __LINE__);
	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_if %p ,p_if->p_area->is_stub %d ,p_if->p_area->is_nssa:%d%d\n", p_if, p_if->p_area->is_stub, p_if->p_area->is_nssa, __LINE__);
            return FALSE;
        }
        return TRUE;
    }

    if (OSPF_LS_TYPE_9 == lstype)
    {
        /*for opaque 9 lsa, rx interface is same as tx interface*/
        if (p_if != p_lsa->p_lstable->p_if)
        {
	//		printf("ospf_lsa_flood_verify:222  p_if %p ,p_lsa->p_lstable->p_if %p%s:%d\n", p_if, p_lsa->p_lstable->p_if, __FUNCTION__, __LINE__);
	   ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_if %p ,p_lsa->p_lstable->p_if %p%d\n", p_if, p_lsa->p_lstable->p_if, __LINE__);
            return FALSE;
        }
        return TRUE;
    }
    /*area scope lsa*/
    /*area macth*/
    if (p_area != p_if->p_area)
    {
//		printf("ospf_lsa_flood_verify:222  p_area %p ,p_if->p_area %p%s:%d\n", p_area, p_if->p_area, __FUNCTION__, __LINE__);
	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_area %p ,p_if->p_area %p%d\n", p_area, p_if->p_area, __LINE__);
        return FALSE;
    }
   
    /*ignore nssa lsa for non nssa area*/ 
    if ((OSPF_LS_TYPE_7 == lstype) && (!p_if->p_area->is_nssa))
    {
//		printf("ospf_lsa_flood_verify:222  lstype %d ,p_if->p_area->is_nssa %d%s:%d\n", lstype, p_if->p_area->is_nssa, __FUNCTION__, __LINE__);
	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  lstype %d ,p_if->p_area->is_nssa %d%d\n", lstype, p_if->p_area->is_nssa, __LINE__);
        return FALSE;
    }
           
    /*flood on normal interface*/ 
    if (((NULL != p_area) && (OSPF_BACKBONE == p_area->id))
        || (p_if->type != OSPF_IFT_VLINK))
    {
//	printf("ospf_lsa_flood_verify:222  p_area %p ,p_area->id %d,p_if->type:%d,%s:%d\n", p_area, p_area->id,p_if->type, __FUNCTION__, __LINE__);
	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_area %p ,p_area->id %d,p_if->type:%d,%d\n", p_area, p_area->id,p_if->type, __LINE__);
        return TRUE;
    }
    else if ((OSPF_IFT_VLINK == p_if->type) 
              && ((NULL == p_rx_if) || (p_rx_if != p_if)))  
    {
        /*incase of virtual link: lsa is self originated ,or
          * tx interface is not input virtual link 
         */
//	printf("ospf_lsa_flood_verify:222  p_if %p ,p_rx_if %p,p_if->type:%d,%s:%d\n", p_if, p_rx_if,p_if->type, __FUNCTION__, __LINE__);
	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood_verify:222  p_if %p ,p_rx_if %p,p_if->type:%d,%d\n", p_if, p_rx_if,p_if->type, __LINE__);
        return TRUE;
    }
    return FALSE;
}

/*flood lsa,select valid interface for flooding.*/
u_int 
ospf_lsa_flood (
         struct ospf_lsa *p_lsa,
         struct ospf_if *p_rx_if,
         struct ospf_nbr *p_rx_nbr)
{  
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
  //  struct ospf_process *p_process = p_lsa->p_lstable->p_process;
    struct ospf_process *p_process = NULL;
    u_int flood_back = FALSE;
    u_int if_count = 0;
    u_int if_max_count = 0;
    u_int8 str[100]; 
	u_long ulAddr = 0;
	u_int uiFlag = 0;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next = NULL;
    ulAddr = p_lsa;
    //ospf_logx(ospf_debug_lsa, "prepare flood lsa,%s",ospf_print_lshdr(p_lsa->lshdr, str));
     
    /*consider failure case*/
    #if 0
    if (NULL == p_lsa)
    {
        return flood_back;
    }
    #else
    if ((NULL == p_lsa)||(NULL == p_lsa->p_lstable))
    {
        return flood_back;
    }
    p_process = p_lsa->p_lstable->p_process;
    
    if (NULL == p_process)
    {
        return flood_back;
    }
    #endif
    /*decide flood if num*/
    if (p_process->wait_export)
    {
        if_max_count = OSPF_MIN_FLOOD_IF_NUM;
    }
    else
    {
        if_max_count = OSPF_MAX_FLOOD_IF_NUM;
    }
	p_lsa = ulAddr ;
//    ospf_logx(ospf_debug,"ospf_lsa_flood: p_lsa %p p_lstable %p,p_rx_if %p p_rx_nbr %p %s:%d\n", p_lsa,p_lsa->p_lstable, p_rx_if, p_rx_nbr, __FUNCTION__, __LINE__);
    /*scan for all of interfaces,if flood is enabled,flood it*/
    for_each_ospf_if(p_process, p_if, p_next_if)
    {  
        #if 1
        uiFlag = 0;

        /*���ھӽӿ�ȡ������*/
        for_each_ospf_nbr(p_if, p_nbr, p_next)
        {
            if(p_nbr != NULL)
            {
                uiFlag = 1;
                break;
            }
        }
        if(uiFlag != 1)
        {
            ospf_logx(ospf_debug_lsa,"ospf_lsa_flood continue: %s has no nbr .%d\n",p_if->name, __LINE__);
            continue; 
        }

    #endif
   	    ospf_logx(ospf_debug_lsa,"ospf_lsa_flood:111  p_lsa %p p_lstable %p,p_rx_if %p p_rx_nbr %p %d\n", p_lsa,p_lsa->p_lstable, p_rx_if, p_rx_nbr, __LINE__);
		p_lsa = ulAddr ;
		ospf_logx(ospf_debug,"ospf_lsa_flood:222  p_lsa %p p_lstable %p,p_rx_if %p p_rx_nbr %p %d\n", p_lsa,p_lsa->p_lstable, p_rx_if, p_rx_nbr, __LINE__);
        if (!ospf_lsa_flood_verify(p_lsa, p_if, p_rx_if, p_rx_nbr))
        {
            continue; 
        }
    	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood: p_lsa %p p_lstable %p,p_rx_if %p p_rx_nbr %p %d\n", p_lsa,p_lsa->p_lstable, p_rx_if, p_rx_nbr, __LINE__);
        ospf_logx(ospf_debug_lsa, "flooding lsa on %s", p_if->name);
         
        /* section 13.3, item (1) - (page 139) */
		p_lsa = ulAddr ;
        if (!ospf_lsa_rxmt_add (p_if, p_lsa, p_rx_nbr))  
        {        
            ospf_logx(ospf_debug_lsa, "can not flood this lsa for rxmt add error");
            continue;
        }
        /*end*/
    	//ospf_logx(ospf_debug_lsa,"ospf_lsa_flood: p_lsa %p p_lstable %p,p_rx_if %p p_rx_nbr %p %s:%d\n", p_lsa,p_lsa->p_lstable, p_rx_if, p_rx_nbr, __FUNCTION__, __LINE__);
        if (p_if == p_rx_if) 
        {
            flood_back = TRUE;
        }        
        /*limit output interface number*/
        if (++if_count > if_max_count)
        {
            continue;
        }
        /* section 13.3, item (5) - (page 140) */
    //	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood: p_lsa %p p_lstable %p,p_rx_if %p p_rx_nbr %p %s:%d\n", p_lsa,p_lsa->p_lstable, p_rx_if, p_rx_nbr, __FUNCTION__, __LINE__);
		p_lsa = ulAddr ;
        ospf_update_buffer_insert(&p_if->update, p_lsa, NULL);   
    //	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood: p_lsa %p p_lstable %p,p_rx_if %p p_rx_nbr %p %s:%d\n", p_lsa,p_lsa->p_lstable, p_rx_if, p_rx_nbr, __FUNCTION__, __LINE__);
		p_lsa = ulAddr ;
        /*20130517 send fast for important lsa*/
        if ((OSPF_LS_ROUTER == p_lsa->lshdr->type)
            || (OSPF_LS_NETWORK == p_lsa->lshdr->type))
        {
        	p_lsa = ulAddr ;
            ospf_update_output(&p_if->update);
  //  	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood: p_lsa %p p_lstable %p,p_rx_if %p p_rx_nbr %p %s:%d\n", p_lsa,p_lsa->p_lstable, p_rx_if, p_rx_nbr, __FUNCTION__, __LINE__);
        }
    //	ospf_logx(ospf_debug_lsa,"ospf_lsa_flood: p_lsa %p p_lstable %p,p_rx_if %p p_rx_nbr %p %s:%d\n", p_lsa,p_lsa->p_lstable, p_rx_if, p_rx_nbr, __FUNCTION__, __LINE__);
    }     
    return flood_back;
}

/*ospf_lsa_rxmt_add - add advertisement to neighbor retransmit list
*
* This routine will examine each neighbor and add the 
* advertisement to the retransmit list.
*/
/* section 13.3, item (1) - (page 139) */
u_int 
ospf_lsa_rxmt_add(
             struct ospf_if *p_if,
             struct ospf_lsa *p_lsa,
             struct ospf_nbr *p_rx_nbr)
{
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    u_int rxmt_add = FALSE;
    struct ospf_request_node *p_req = NULL;  
    signed long test;   
    
  //   printf("%s %d  ***********\n", __FUNCTION__,__LINE__);
    /*scan neighbor on interface*/
    for_each_ospf_nbr(p_if, p_nbr, p_next_nbr) 
    {
    
//	printf("%s %d  *****p_nbr->state %d******\n", __FUNCTION__,__LINE__,p_nbr->state);
        /* section 13.3, item (1a) - (page 139) */
        if (OSPF_NS_EXCHANGE > p_nbr->state)
        {
//		printf("%s %d  *****p_nbr->state %d******\n", __FUNCTION__,__LINE__,p_nbr->state);
            continue;
        }

        /* section 13.3, item (1b) - (page 139) */
        if ((OSPF_NS_EXCHANGE == p_nbr->state)
            || (OSPF_NS_LOADING ==  p_nbr->state))
        {
	//	printf("%s %d  *****p_nbr->state %d******\n", __FUNCTION__,__LINE__,p_nbr->state);
   //         p_req = ospf_request_lookup(p_lsa->p_lstable, p_lsa->lshdr);
	if(NULL != p_req)
	{
	//	printf("%s %d  *****p_req 0x%x,p_req->p_nbr %x,p_nbr %x******\n", __FUNCTION__,__LINE__,p_req,p_req->p_nbr,p_nbr);
	}
	if ((NULL != p_req) && (p_req->p_nbr == p_nbr))
         {
                /* section 13.3, item (1b), bullet #1 - (page 139) */
                /*requested lsa is more recent,do not send update*/
                test = ospf_lshdr_cmp(&p_req->ls_hdr, p_lsa->lshdr);
	//	printf("%s %d  *****test %d******\n", __FUNCTION__,__LINE__,test);
                if (0 < test )
                {    
                    continue;
                }                
                ospf_request_delete(p_req);
                /*same instance,no update need*/
                if (0 == test)
                {
                    continue;
                }
            }
        }
        /* section 13.3, item (1c) - (page 139) */
		
//	printf("%s %d  *****p_nbr %x,p_rx_nbr %x******\n", __FUNCTION__,__LINE__,p_nbr,p_rx_nbr);
        if (p_nbr == p_rx_nbr)
        {
            continue;
        }
        
        /*opaque lsa examine each neighbor retransmission list jkw*/
        /*RFC 2370 Section 3.1 item 4*/
//	printf("%s %d  *****p_lsa->lshdr->type %d,p_nbr->opaque_enable %d******\n", __FUNCTION__,__LINE__,p_lsa->lshdr->type,p_nbr->opaque_enable);
        if ((OSPF_LS_TYPE_9 <= p_lsa->lshdr->type) && (!p_nbr->opaque_enable))
        {
            continue;
        }
        
        /* section 13.3, item (1d) - (page 139) */
        if (NULL == ospf_nbr_rxmt_add(p_nbr, p_lsa))
        {
            /*some memory error,enter overload state*/
      //      printf("%s %d  *****p_nbr %x,p_lsa %x******\n", __FUNCTION__,__LINE__,p_nbr,p_lsa);
            p_nbr->force_down = TRUE;
            continue ;
        }
 //	printf("%s %d  *****rxmt_add = TRUE******\n", __FUNCTION__,__LINE__);
        rxmt_add = TRUE;
    }
 //   printf("%s %d  *****rxmt_add = %d******\n", __FUNCTION__,__LINE__,rxmt_add);
    return rxmt_add;
}

/*Insert database list into interface's flood buffer,if buffer length is full,send
update packet and continue to process rest database.*/
void 
ospf_update_buffer_insert(
            struct ospf_updateinfo *p_update,
            struct ospf_lsa *p_lsa,
            u_int8 *p_buf)
{
    struct ospf_if *p_if = NULL;
    struct ospf_lshdr *p_filled = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next = NULL;
    u_short len = ntohs(p_lsa->lshdr->len);
    u_int maxlen = 0;

	ospf_logx(ospf_debug_lsa,"ospf_update_buffer_insert: p_lsa %p len %d\n", p_lsa, p_lsa->lshdr->len);
    /*decide output interface*/
    if (NULL != p_update->p_if)
    {
        p_if = p_update->p_if;
    }
    else if (NULL != p_update->p_nbr)
    {
        p_if = p_update->p_nbr->p_if;
    }
    
    /*lsa length exceed max txbuf length,do not send it*/
    if (OSPF_MAX_TXBUF < (len + OSPF_UPDATE_MIN_LEN))
    {
        ospf_single_update_output(p_if, p_update->p_nbr, p_lsa);
        return;
    }

    /*msg buffer not exist, allocate new buffer according to lsa length*/
    if (NULL == p_update->p_msg)
    {
        /*use max length for larger lsa*/
        if ((len + OSPF_UPDATE_MIN_LEN) > p_if->maxlen)
        {
            maxlen = OSPF_MAX_TXBUF;
        }
        else
        {
            maxlen = p_if->maxlen;
        }
        /*allocate and fill,if msg buffer is exist,do not create anymore*/
        if (NULL != p_buf)
        {
            p_update->p_msg = (struct ospf_update_msg *)p_buf;
            memset(p_buf, 0, OSPF_UPDATE_MIN_LEN);
        }
        else
        {
            u_int ulLen = 0;

            ulLen = (maxlen/sizeof(struct ospf_update_msg))*sizeof(struct ospf_update_msg);
        //    printf("ospf_nexthop_add ulLen=%d,maxlen=%d,lst = %d\n",ulLen,maxlen,sizeof(struct ospf_update_msg));
           p_update->p_msg = ospf_malloc(maxlen, OSPF_MPACKET);
     //      p_update->p_msg = ospf_malloc(ulLen, OSPF_MPACKET);
           if (NULL == p_update->p_msg)  
           {
               return;
           }
        }
        
        p_update->maxlen = maxlen;
        /*reserve space for update head*/
        p_update->p_msg->h.type = OSPF_PACKET_UPDATE;
        p_update->p_msg->h.len = OSPF_UPDATE_MIN_LEN;
        p_update->p_msg->lscount = 0;
        goto FILL_IT;
    }

    /*buffer exist, if lsa can be filled,fill it*/
    if ((len + p_update->p_msg->h.len) <= p_update->maxlen)
    {
        goto FILL_IT;
    }

    /*lsa can not be filled,send current update,do not release buffer*/
    ospf_update_output(p_update);

    /*restart nbr'rxmt timer*/
    for_each_node(&p_if->nbr_table, p_nbr, p_next)
    {
        if (p_if->rxmt_interval)
        { 
            ospf_timer_start(&p_nbr->lsrxmt_timer, ospf_rand(p_if->rxmt_interval*OSPF_TICK_PER_SECOND));
        }
        else
        {
            ospf_timer_start(&p_nbr->lsrxmt_timer, 2);
        }
    }

    /*if lsa lenght is less than buffer's max length, fill it,else create large buffer*/
    if ((len + OSPF_UPDATE_MIN_LEN) <= p_update->maxlen)
    {
        goto FILL_IT;
    }

    /*free and reallocate new buffer according to max value*/
    if (NULL == p_buf)
    {
        ospf_mfree(p_update->p_msg, OSPF_MPACKET);
    }
    p_update->p_msg = NULL;
     
    maxlen = OSPF_MAX_TXBUF;
    if (NULL != p_buf)
    {
        p_update->p_msg = (struct ospf_update_msg *)p_buf;
        memset(p_buf, 0, OSPF_UPDATE_MIN_LEN);
    }
    else
    {
        p_update->p_msg = ospf_malloc(maxlen, OSPF_MPACKET);
    }
     
    if (NULL == p_update->p_msg)  
    {
        return;
    }
    p_update->maxlen = maxlen;
    /*reserve space for update head*/
    p_update->p_msg->h.type = OSPF_PACKET_UPDATE;
    p_update->p_msg->h.len = OSPF_UPDATE_MIN_LEN;
    p_update->p_msg->lscount = 0;
    
FILL_IT :
     /*set current buffer using offset*/
     p_filled = (struct ospf_lshdr *)(((u_int8 *)p_update->p_msg) + p_update->p_msg->h.len) ;
     memcpy(p_filled, p_lsa->lshdr, len);
     
     /*adjust age*/
     ospf_lsa_age_update(p_lsa);
     p_filled->age = ospf_adjust_age(p_lsa->lshdr->age, p_if->tx_delay) ;

     /*increase length and count*/
     p_update->p_msg->h.len += len;
     p_update->p_msg->lscount++;

     ospf_timer_try_start(&p_if->flood_timer, 1);
     return;
}

/*send packet according to interface's flooding buffer.*/ 
void 
ospf_update_output(struct ospf_updateinfo *p_update)
{
    struct ospf_if *p_if = NULL;
    u_int8 nbr[32];
    u_int dest;
   
    if (NULL != p_update->p_if)
    {
        p_if = p_update->p_if;
    }
    else if (NULL != p_update->p_nbr)
    {
        p_if = p_update->p_nbr->p_if;
    }
   
    if ((NULL == p_update->p_msg) || (0 == p_update->p_msg->lscount))
    {
        return;  
    }
   
    p_if->stat->tx_lsainfo[OSPF_PACKET_UPDATE] += p_update->p_msg->lscount;
       
    /*select dest*/          
    dest = ospf_packet_dest_select(p_if, p_update->p_nbr, OSPF_PACKET_UPDATE);
   
    if ((OSPF_ADDR_ALLSPF == dest) || (OSPF_ADDR_ALLDR == dest))
    {
        p_if->stat->mcast_update++;
    }
    else
    {
        p_if->stat->ucast_update++;
    }
    if (ospf_debug)
    {
        ospf_inet_ntoa(nbr, dest);
        ospf_logx(ospf_debug_lsa, "send update to %s on %s,length %d", nbr, p_if->name, p_update->p_msg->h.len);
    }
    p_update->p_msg->lscount = htonl(p_update->p_msg->lscount);
    ospf_output (p_update->p_msg, p_if, dest,p_update->p_msg->h.len);

    if (ospf_debug && ospf_debug_msg)
    {
        ospf_log_update_packet(p_update->p_msg, FALSE);
    } 
    /*reset buffer*/  
    p_update->p_msg->h.type = OSPF_PACKET_UPDATE;
    p_update->p_msg->h.len = OSPF_UPDATE_MIN_LEN;
    p_update->p_msg->lscount = 0;         
    return;
}

/*20110709 big packet*/
/*send single update for lsa with large length*/
void
ospf_single_update_output(
            struct ospf_if *p_if,
            struct ospf_nbr *p_nbr,
            struct ospf_lsa *p_lsa)
{
    struct ospf_lshdr *p_filled = NULL;
    struct ospf_updateinfo update;
    u_int ls_len = 0;
    
    /*set update buffer*/
    update.p_nbr = p_nbr;
    update.p_if = p_if;
    update.p_msg = NULL;
    update.maxlen = 0;
    u_int ulLen = 0;

    ulLen = (OSPF_BIGPKT_BUF/sizeof(struct ospf_update_msg))*sizeof(struct ospf_update_msg);
 //   printf("ospf_nexthop_add ulLen=%d,uiMlen=%d,lst = %d\n",ulLen,OSPF_BIGPKT_BUF,sizeof(struct ospf_update_msg));
 //   update.p_msg = ospf_malloc(ulLen, OSPF_MPACKET);
    update.p_msg = ospf_malloc(OSPF_BIGPKT_BUF, OSPF_MPACKET);
    if (NULL == update.p_msg)
    {
        return;
    }
    memset(update.p_msg, 0, OSPF_UPDATE_MIN_LEN);
    
    /*reserve space for update head*/
    update.p_msg->h.type = OSPF_PACKET_UPDATE;
    update.p_msg->h.len = OSPF_UPDATE_MIN_LEN;
    update.p_msg->lscount = 0;
          
    ls_len = ntohs(p_lsa->lshdr->len);

    /*set current buffer using offset*/
    p_filled = (struct ospf_lshdr *)(((u_int8 *)update.p_msg) + update.p_msg->h.len);
    memcpy(p_filled, p_lsa->lshdr, ls_len);
     
    /*adjust age*/
    ospf_lsa_age_update(p_lsa);
    p_filled->age = ospf_adjust_age(p_lsa->lshdr->age, p_if->tx_delay);

    /*increase length and count*/
    update.p_msg->h.len += ls_len;
    update.p_msg->lscount++;
   
    /*send update and free buffer*/
    ospf_update_output(&update);
    
    ospf_mfree(update.p_msg, OSPF_MPACKET);
    return;
}
