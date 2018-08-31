/* ospf_dbd.c - database description packet input and output processing*/

#include "ospf.h"
//#include "port_api.h"

void ospf_dd_input_exstart(struct ospf_nbr * p_nbr, struct ospf_dd_msg * p_dd, u_int len);
void ospf_dd_input_exchange(struct ospf_nbr * p_nbr, struct ospf_dd_msg * p_dd, u_int len);
void ospf_dd_input_full(struct ospf_nbr * p_nbr, struct ospf_dd_msg * p_dbd);
void ospf_check_dd_from_slave(struct ospf_nbr * p_nbr, struct ospf_dd_msg * p_dd, u_int len);
void ospf_check_dd_from_master(struct ospf_nbr * p_nbr, struct ospf_dd_msg * p_dd, u_int len);
int ospf_process_dd_lsa(struct ospf_nbr * p_nbr, struct ospf_dd_msg * p_dd, u_int len);
u_int ospf_dd_option_is_valid(struct ospf_nbr * p_nbr, u_int8 option);

/* section 10.6 of OSPF specification (page 90) */
void 
ospf_dd_input(
             struct ospf_nbr *p_nbr,
             struct ospf_hdr *p_hdr)
{
    struct ospf_dd_msg *p_dd = (struct ospf_dd_msg *)p_hdr;
    struct ospf_if *p_if = p_nbr->p_if;
    u_int len = (ntohs(p_hdr->len) - OSPF_PACKET_HLEN - OSPF_DBD_HLEN);
    u_int req_count = ospf_lstcnt(&p_if->p_process->req_table);
    u_int8 nbr[16];
    u_int8 statemask[4] = {0};

    ospf_logx(ospf_debug, "receive dbd from %s on %s,length %d", ospf_inet_ntoa(nbr,p_nbr->addr), p_if->name, len);
 //   ospf_logx(ospf_debug,"recv dbd from %s on %s,length %d.\r\n", ospf_inet_ntoa(nbr,p_nbr->addr), p_if->name, len);

    if (ospf_debug && ospf_debug_msg)
    {
        ospf_log_dd_packet(p_dd, TRUE);
    }
    
    /* RFC 2178 G.9 if interface is not virtual-link,
       compare mtu in packet*/    
    if (OSPF_IFT_VLINK != p_if->type)
    {        
        /*according to huawei:do not check mtu if expected*/
        if ((FALSE == p_if->mtu_ignore)
            && (ntohs(p_dd->mtu) > ospf_if_mtu_len(p_if)))
        {
            ospf_logx(ospf_debug|ospf_debug_error, "mtu mismatch,local %d,remote %d", ospf_if_mtu_len(p_if), ntohs(p_dd->mtu));            
            p_if->stat->error[OSPF_IFERR_MTU]++;
            p_if->stat->error_data[OSPF_IFERR_MTU] = ntohs(p_dd->mtu);
          
            ospf_trap_iferror( ERROR_MTU, OSPF_PACKET_DBD, p_nbr->addr, p_if);
            return; 
        }
    }
    
    ospf_logx(ospf_debug, "receive dbd from  p_nbr->stat=%d\r\n", p_nbr->state);
    switch (p_nbr->state){
        case OSPF_NS_DOWN:
        case OSPF_NS_ATTEMPT:
        case OSPF_NS_2WAY:
             p_if->stat->error[OSPF_IFERR_DD_NBR];
             ospf_logx(ospf_debug|ospf_debug_error, "invalid nbr state, ignore"); 
             break;
             
        case OSPF_NS_INIT:
             /*try to enter exstart state*/
             ospf_nsm(p_nbr, OSPF_NE_2WAY);

             if (OSPF_NS_EXSTART != p_nbr->state)
             {
                 ospf_logx(ospf_debug|ospf_debug_error, "can not enter exstart");
                 //break;
             }
             break;
             /*fall through*/
             
        case OSPF_NS_EXSTART:
             /*slave:reject this packet if nbr count exceed limit*/
             if (p_nbr->id > p_if->p_process->router_id)
             {
                 BIT_LST_SET(statemask, OSPF_NS_EXCHANGE);
                 /*BIT_LST_SET(statemask, OSPF_NS_LOADING);*//*do not control loading number*/
                 if ((OSPF_MAX_EXCHANGE_NBR <= ospf_nbr_count_in_state(p_if->p_process, statemask))
                     || (OSPF_MAX_GLOBAL_EXCHANGE_NBR <= ospf_nbr_count_in_state(NULL, statemask)))
                 {
                     break;
                 }
             }
             ospf_dd_input_exstart(p_nbr, p_dd, len);
             break;          
          
        case OSPF_NS_EXCHANGE:
             ospf_dd_input_exchange(p_nbr, p_dd, len);
             break;
          
        case OSPF_NS_LOADING:
        case OSPF_NS_FULL:
             ospf_logx(ospf_debug, "neighbor is in full state");
		//	 ospf_logx(ospf_debug,"ospf_dd_input:neighbor is in full state\n");
 
             /*slave rcvd duplicated packet,special processing for N2X testing*/
             if ((OSPF_SLAVE == p_nbr->dd_state) 
                 && (p_nbr->dd_seqnum == ntohl(p_dd->seqnum)))
             {
                 /*if packet has More bit 0,and rcvd packet has ls header,this is an error*/
		//		 ospf_logx(ospf_debug,"%s,%d p_dd->flags=%d\n",__FUNCTION__,__LINE__,p_dd->flags);
                 if (!ospf_dd_flag_more(p_dd->flags) && p_nbr->empty_dd_rcvd && (0 != len))
                 {                    
                     ospf_logx(ospf_debug|ospf_debug_gr|ospf_debug_error, "invalid packet,more=0 and include lsa"); 
		//			 ospf_logx(ospf_debug,"%s %d OSPF_NE_SEQ_MISMATCH \n", __FUNCTION__,__LINE__);
                     ospf_nsm(p_nbr, OSPF_NE_SEQ_MISMATCH);
                     break;
                 }
             }
             ospf_dd_input_full (p_nbr, p_dd);
             break;
          
        default:
             break;
    }

    /*call interface fsm if nbr state changed*/
    if (p_if->nbrchange)
    {
        ospf_ism(p_if, OSPF_IFE_NBR_CHANGE);
    }

    /*send lsa request if necessary*/
    if ((OSPF_NS_EXCHANGE <= p_nbr->state) 
        && (req_count != ospf_lstcnt(&p_if->p_process->req_table)))
    {
        /*start a 200ms timer for request*/
        ospf_timer_try_start(&p_nbr->request_timer, 2);
    }
    return;
}

/*process db packet in exstart state*/
void 
ospf_dd_input_exstart(
                    struct ospf_nbr *p_nbr,
                    struct ospf_dd_msg *p_dd,
                    u_int len)
{
    struct ospf_process *p_process = p_nbr->p_if->p_process;
    u_int init = ospf_dd_flag_init(p_dd->flags);
    u_int more = ospf_dd_flag_more(p_dd->flags);
    u_int master = ospf_dd_flag_master(p_dd->flags);
    int seq = ntohl(p_dd->seqnum);
    u_int8 str[32];

    p_nbr->ulDdMtu = ntohs(p_dd->mtu);
   // printf("ospf_dd_input_exstart-- p_dd->mtu= %d\n",p_dd->mtu);
    
//	ospf_logx(ospf_debug,"%s,%d p_dd->flags=%d\n",__FUNCTION__,__LINE__,p_dd->flags);
//	ospf_logx(ospf_debug, "ospf_dd_input_exstart seqnum %d,nbr id %sinit=%d,more=%d,master=%d.", seq, ospf_inet_ntoa(str, p_nbr->id),init,more,master);
    /*decide role, according to 10.6*/
    if (init && more && master && (0 == len) 
        && (p_nbr->id > p_process->router_id))
    {
        /*input is init dbd, nbr id > self, become slave*/
        p_nbr->dd_state = OSPF_SLAVE;  
        p_nbr->dd_flag = 0;  
        p_nbr->dd_seqnum = seq;
	//	ospf_logx(ospf_debug,"%s,%d p_nbr->dd_seqnum=%d\n",__FUNCTION__,__LINE__,p_nbr->dd_seqnum);
        ospf_logx(ospf_debug, "become slave,accept seqnum %d,nbr id %s", seq, ospf_inet_ntoa(str, p_nbr->id));
    }
    else if (!init && !master 
           && (p_nbr->dd_seqnum == seq) 
           && (p_nbr->id < p_process->router_id))
    {
        /*i/ms clear, seq is same as self, nbr id < self, 
        this is ack from slave,become master*/
        p_nbr->dd_state = OSPF_MASTER;  
        p_nbr->dd_flag = 0;
        ospf_set_dd_flag_master(p_nbr->dd_flag);
        
        ospf_logx(ospf_debug, "become master,nbr id %s", ospf_inet_ntoa(str, p_nbr->id));
    }
    else
    {
        ospf_logx(ospf_debug, "ignore packet"); 
        return;
    }
    
    /*rfc2328 10.6:the packet's Options field should 
    be recorded in the neighbor structure's Neighbor Options field*/
    p_nbr->option = p_dd->option;

    /*record opaque capability */
    p_nbr->opaque_enable = ospf_option_opaque(p_dd->option);

    /*neg-done,prepare real dbd exchanging*/
    ospf_nsm(p_nbr, OSPF_NE_NEGDONE);
    
    /* check to make sure the state transitioned to Exchange */
    if (OSPF_NS_EXCHANGE != p_nbr->state)
    {
//        ospf_logx(ospf_debug,"%s %d\n", __FUNCTION__,__LINE__);
        /*reinit dbd flags*/
        ospf_set_dd_flag_master(p_nbr->dd_flag);
        ospf_set_dd_flag_init(p_nbr->dd_flag);
        ospf_set_dd_flag_more(p_nbr->dd_flag);
        p_nbr->dd_state = OSPF_CLEAR_MODE;
        return;
    }
    
    /* See ExStart state actions in 10.6 */
    if (ospf_process_dd_lsa(p_nbr, p_dd, len) == OK)
    {
  //      ospf_logx(ospf_debug,"%s %d\n", __FUNCTION__,__LINE__);
        ospf_dd_output (p_nbr);
    }
    return;
}

/* process db packet in exchange state*/
void 
ospf_dd_input_exchange(
                   struct ospf_nbr *p_nbr,
                   struct ospf_dd_msg *p_dd,
                   u_int len)
{
    u_int master = ospf_dd_flag_master(p_dd->flags);
    int seq = ntohl(p_dd->seqnum);

    /* first bullet item under Exchange, section 10.6 of spec. (p 91) */
    /*master role mismatch*/
    if ( (master && (OSPF_SLAVE != p_nbr->dd_state))
         || (!master && (OSPF_MASTER != p_nbr->dd_state))
         || ospf_dd_flag_init(p_dd->flags))
    {               
//		ospf_logx(ospf_debug,"%s,%d p_nbr->dd_state=0x%x,  p_dd->flags=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_state,p_dd->flags);
        /*even I bit set,there may still be duplicated packet */
        if ((OSPF_SLAVE == p_nbr->dd_state) && (seq == p_nbr->dd_seqnum))
        {
            ospf_logx(ospf_debug, "send reply to master,duplicate Init packet");
            ospf_dd_output(p_nbr);
            return;
        }  
        ospf_logx(ospf_debug|ospf_debug_gr|ospf_debug_error, "invalid I/Ms bits in packet"); 
  //      ospf_logx(ospf_debug,"%s %d OSPF_NE_SEQ_MISMATCH \n", __FUNCTION__,__LINE__);
        ospf_nsm(p_nbr, OSPF_NE_SEQ_MISMATCH);
    }
    /* second bullet item under Exchange, (p 91) */
    else if (!ospf_dd_option_is_valid(p_nbr, p_dd->option))
    {
        ospf_logx(ospf_debug|ospf_debug_gr|ospf_debug_error, "failed to check packet option,discard packet");
//		ospf_logx(ospf_debug,"%s,%d p_dd->option=0x%x\n",__FUNCTION__,__LINE__,p_dd->option);
 //       ospf_logx(ospf_debug,"%s %d OSPF_NE_SEQ_MISMATCH \n", __FUNCTION__,__LINE__);
        ospf_nsm(p_nbr, OSPF_NE_SEQ_MISMATCH);
        return;
    }
    else if ((OSPF_MASTER == p_nbr->dd_state)
           && (p_nbr->dd_seqnum == seq)) 
    {
//		ospf_logx(ospf_debug,"%s,%d p_nbr->dd_state=0x%x,p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_state,p_nbr->dd_seqnum);
        ospf_check_dd_from_slave(p_nbr, p_dd, len);
    }
    else if ((OSPF_MASTER == p_nbr->dd_state) 
          && (1 == (p_nbr->dd_seqnum - seq))) 
    {
//		ospf_logx(ospf_debug,"%s,%d p_nbr->dd_state=0x%x,p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_state,p_nbr->dd_seqnum);
        ospf_logx(ospf_debug, "received duplicate packet,discard it");        
        return;/* discard duplicate packet */
    }
    else if ((OSPF_SLAVE == p_nbr->dd_state ) 
          && (1 == (seq - p_nbr->dd_seqnum)))
    {
//		ospf_logx(ospf_debug,"%s,%d p_nbr->dd_state=0x%x,p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_state,p_nbr->dd_seqnum);
        ospf_check_dd_from_master(p_nbr, p_dd, len);
    }
    else if ((OSPF_SLAVE == p_nbr->dd_state) 
         && (seq == p_nbr->dd_seqnum))
    {
//		ospf_logx(ospf_debug,"%s,%d p_nbr->dd_state=0x%x,p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_state,p_nbr->dd_seqnum);
        ospf_logx(ospf_debug, "send reply to master");
        
        /* duplicate packet, so repeat last Database 
        Description packet that was sent */                 
        ospf_dd_output(p_nbr);
    }
    else                                                                                                                                                                                                                                                                                                         /* seventh bullet */
    {
        ospf_logx(ospf_debug|ospf_debug_gr|ospf_debug_error, "invalid packet,discard it");
        
        /* stop processing packet */
  //      ospf_logx(ospf_debug,"%s %d OSPF_NE_SEQ_MISMATCH \n", __FUNCTION__,__LINE__);
        ospf_nsm (p_nbr, OSPF_NE_SEQ_MISMATCH); 
    }    
    return;
}

/* process db packet in loading or full state*/
void 
ospf_dd_input_full(
                   struct ospf_nbr *p_nbr,
                   struct ospf_dd_msg *p_dbd)
{
    int seq = ntohl(p_dbd->seqnum);

//	ospf_logx(ospf_debug,"%s,%d seq=0x%x, p_dbd->seqnum=0x%x\n",__FUNCTION__,__LINE__,seq,p_dbd->seqnum);
//	ospf_logx(ospf_debug,"%s,%d p_nbr=%p, p_nbr->p_if=%p,p_nbr->addr=0x%x,p_nbr->p_if->addr=0x%x\n",__FUNCTION__,__LINE__,p_nbr,p_nbr->p_if,p_nbr->addr,p_nbr->p_if->addr);
	//p_nbr->dd_seqnum = seq;
	/*if option changed,re-establish nbr*/
    if (!ospf_dd_option_is_valid(p_nbr, p_dbd->option))
    {
        ospf_logx(ospf_debug|ospf_debug_gr|ospf_debug_error, "failed to check packet option,discard packet");

        ospf_nsm(p_nbr, OSPF_NE_SEQ_MISMATCH);
        return;
    }    
    /*slave rxd duplicate packet*/
    if (((OSPF_SLAVE_HOLD == p_nbr->dd_state) 
        || (OSPF_SLAVE == p_nbr->dd_state))
        && (p_nbr->dd_seqnum == seq))
    {
    	//p_dbd->seqnum = htonl(p_nbr->dd_seqnum);
    	
//		ospf_logx(ospf_debug,"%s,%d p_nbr->dd_state=0x%x,  p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_state,p_nbr->dd_seqnum);
        ospf_logx(ospf_debug, "local is slave,send last dbd to master");        
		//p_nbr->dd_seqnum = seq;
        ospf_dd_output(p_nbr);
    }
    else if ((OSPF_MASTER == p_nbr->dd_state) 
        && (1 == (p_nbr->dd_seqnum - seq)))
    {
//		ospf_logx(ospf_debug,"%s,%d p_nbr->dd_state=0x%x,  p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_state,p_nbr->dd_seqnum);
        ospf_logx(ospf_debug, "local is master,ignore duplicate packet");
        return;    /* discard duplicate packet */
    }
    else
    {
		ospf_logx(ospf_debug|ospf_debug_gr|ospf_debug_error,"%d p_nbr->dd_state=0x%x,  p_nbr->dd_seqnum=0x%x,seq=0x%x\n",__LINE__,p_nbr->dd_state,p_nbr->dd_seqnum,seq);
        ospf_logx(ospf_debug|ospf_debug_gr|ospf_debug_error, "invalid summary packet in full state");                                            
  //      ospf_logx(ospf_debug,"%s %d OSPF_NE_SEQ_MISMATCH \n", __FUNCTION__,__LINE__);
        ospf_nsm(p_nbr, OSPF_NE_SEQ_MISMATCH);
    }
    return;
}

/*self is master,and receive database from slave with
same sequence as outgoing packet.accept it*/
void 
ospf_check_dd_from_slave(
                     struct ospf_nbr *p_nbr,
                     struct ospf_dd_msg *p_dd,
                     u_int len)
{
    struct ospf_dd_msg *p_last_dbd = NULL;
    u_int more = FALSE;

  //  ospf_logx(ospf_debug, "ospf_check_dd_from_slave \r\n");
    /* third bullet,(p 91),master receiving expected reply*/
    if (ospf_process_dd_lsa(p_nbr, p_dd, len) != OK)
    {
        ospf_logx(ospf_debug, "stop processing this packet");
        return;
    }

    /*get more flag from last sent dd msg*/
    p_last_dbd = p_nbr->p_last_dd;
    if (NULL != p_last_dbd)
    {
 //       ospf_logx(ospf_debug,"%s %d\n", __FUNCTION__,__LINE__);
        more = ospf_dd_flag_more(p_last_dbd->flags);
    }

    /*both sides finish the exchanging operation, exchange done*/
//	ospf_logx(ospf_debug,"%s,%d p_dd->flags=%d\n",__FUNCTION__,__LINE__,p_dd->flags);
    if (!ospf_dd_flag_more(p_dd->flags) && !more)
    {
        ospf_logx(ospf_debug, "master exchanging is finished ");
        ospf_nsm(p_nbr, OSPF_NE_EXCHANGE_DONE);
        return;
    }
    
    ospf_logx(ospf_debug, "need send next summary packet");
    
    /*if the end of summary packet is acked by slave,but slave
    has more lsa to exchange,we set neighbor's
    empty send flag,for the rested exchanging,master only send 
    empty dd ,not the last one
    */
    if ((NULL != p_last_dbd) && !more)
    {
        ospf_logx(ospf_debug, "need send empty summary packet");       
        p_nbr->empty_dd_need = TRUE;
    }
    
    /*if self has more database,remove current one,it is acked by slave*/
    if ((NULL != p_last_dbd) && more)
    {
  //      ospf_logx(ospf_debug,"%s %d\n", __FUNCTION__,__LINE__);
        ospf_delete_last_send_dd(p_nbr);
    }
    ospf_dd_output(p_nbr);         
    return;
}

/*self is slave,receive packet from master with next sequence
accept it*/
void 
ospf_check_dd_from_master(
                        struct ospf_nbr *p_nbr,
                        struct ospf_dd_msg *p_dd,
                        u_int len)
{
 //   ospf_logx(ospf_debug, "ospf_check_dd_from_master \r\n");
    /*special for summary from master with More=0*/  
//	ospf_logx(ospf_debug,"%s,%d p_dd->flags=%d\n",__FUNCTION__,__LINE__,p_dd->flags);
    if (!ospf_dd_flag_more(p_dd->flags))
    {
        /*if this is the first m=0 packet,record it;
        if not the first one,packet size must be 0,else is error
        */
        if (!p_nbr->empty_dd_rcvd)
        {
            p_nbr->empty_dd_rcvd = TRUE;
        }
        else if (0 != len)
        {
            ospf_logx(ospf_debug|ospf_debug_gr|ospf_debug_error, "invalid packet,more=0 and include lsa");             
//			ospf_logx(ospf_debug,"%s %d OSPF_NE_SEQ_MISMATCH \n", __FUNCTION__,__LINE__);
            ospf_nsm(p_nbr, OSPF_NE_SEQ_MISMATCH);
            return;
        }
    }
    
    if (ospf_process_dd_lsa(p_nbr, p_dd, len) != OK)
    {
  //      ospf_logx(ospf_debug,"%s %d\n", __FUNCTION__,__LINE__);
        return;
    }
    
    /*remove last database*/ 
    ospf_delete_last_send_dd(p_nbr); 

    /* we're the slave, so send a dd in reply */
    ospf_dd_output(p_nbr);

    /*if both sides have more==0,exchange-done,do not delete last dd*/
//	ospf_logx(ospf_debug,"%s,%d p_dd->flags=%d\n",__FUNCTION__,__LINE__,p_dd->flags);
    if (!ospf_dd_flag_more(p_dd->flags) 
       && (NULL == p_nbr->p_last_dd 
           || !ospf_dd_flag_more(p_nbr->p_last_dd->flags)))                                  
    {
        ospf_logx(ospf_debug, "exchanging is finished");
        
        ospf_nsm(p_nbr, OSPF_NE_EXCHANGE_DONE);
    }
    return;
}

/*check if options in database packet changed from last packet*/
u_int 
ospf_dd_option_is_valid(
                     struct ospf_nbr *p_nbr,
                     u_int8 option)
{
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_process *p_process = p_if->p_process;

 //   ospf_logx(ospf_debug, "ospf_dd_option_is_valid \r\n");
    /*if not changed,accept it*/
    if (option == p_nbr->option)
    {
        return TRUE;
    }
    
    /*compare e.n,o bit*/
    if (ospf_option_external(option) != ospf_option_external(p_nbr->option))
    {
        ospf_logx(ospf_debug, "external option changed or invalid:%x,%x", option, p_nbr->option);
        return FALSE;
    }
        
    if (ospf_option_nssa(option) != ospf_option_nssa(p_nbr->option))
    {
        ospf_logx(ospf_debug, "NSSA option changed:%x,%x", option, p_nbr->option);
        return FALSE;
    }
    if (ospf_option_opaque(option) != ospf_option_opaque(p_nbr->option))
    {
        ospf_logx(ospf_debug, "OPAQUE option changed:%x,%x", option, p_nbr->option);
        return FALSE;
    }
    if (ospf_option_opaque(p_nbr->option) && p_process->opaque)
    {
     //   ospf_logx(ospf_debug,"%s %d\n", __FUNCTION__,__LINE__);
        p_nbr->opaque_enable = TRUE;
    }
    return TRUE;
}

/*process dd summary information in dd msg*/
STATUS 
ospf_process_dd_lsa(
                    struct ospf_nbr *p_nbr,
                    struct ospf_dd_msg *p_dbd,
                    u_int len)
{
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_lshdr *p_lshdr = NULL;
    struct ospf_lsa *p_lsa = NULL;
    u_int count = (len/OSPF_DBD_UNIT_LEN);
    u_int i = 0;
    u_int8 str[100];

  //  ospf_logx(ospf_debug, "ospf_process_dd_lsa count=%d.\r\n",count);
    p_if->stat->rx_lsainfo[OSPF_PACKET_DBD] += count;
    /*scan for each lsa header in packet*/       
    p_lshdr = p_dbd->lshdr;
    for (i = 0 ; i < count; i++, p_lshdr++)
    {
        ospf_logx(ospf_debug_lsa, "proc dd lsa,%s", ospf_print_lshdr(p_lshdr, str));

        if (ospf_debug_lsa && ospf_debug_msg)
        {
            ospf_logx(ospf_debug, "  lsa");
            ospf_log_lsa_header(p_lshdr);
            ospf_logx(ospf_debug, " ");
        }
        
        /*ignore ase lsa on vlink,but continue to process next*/
        if ( ((OSPF_IFT_VLINK == p_if->type) 
                || (p_if->p_area->is_stub) 
                || (p_if->p_area->is_nssa))
            && (OSPF_LS_AS_EXTERNAL == p_lshdr->type))
        {
            ospf_logx(ospf_debug_lsa, "ignore external lsa on virtual link,stub,nssa");
            continue;
        }

        /*verify lsa header*/
        if (!ospf_lshdr_is_valid(p_lshdr))
        {
            ospf_logx(ospf_debug_lsa|ospf_debug_gr|ospf_debug_error, "lshdr is invalild");

//			ospf_logx(ospf_debug,"%s %d OSPF_NE_SEQ_MISMATCH \n", __FUNCTION__,__LINE__);
            ospf_nsm(p_nbr, OSPF_NE_SEQ_MISMATCH);
            return ERR;        
        }           
        /*lookup current lsa instance*/
        p_lsa = ospf_lsa_lookup(ospf_lstable_lookup(p_if, p_lshdr->type), p_lshdr);

        if (NULL == p_lsa) 
        {
            ospf_logx(ospf_debug_lsa, "local lsa not exist");
        }
        else
        {
            ospf_logx(ospf_debug_lsa, "local lsa exist");
            if (ospf_debug_lsa && ospf_debug_msg)
            {
                ospf_logx(ospf_debug, "  lsa");
                ospf_log_lsa_header(p_lsa->lshdr);
                ospf_logx(ospf_debug, " ");
            }
        }
        /*local instance not exist,or rxd lsa is more recent,prepare request*/        
        if ((NULL == p_lsa) || (0 < ospf_lshdr_cmp (p_lshdr, p_lsa->lshdr)))
        {
            /*delete retransmit node*/
            if (NULL != p_lsa)
            {
//				ospf_logx(ospf_debug,"%s %d \n", __FUNCTION__,__LINE__);
                ospf_nbr_rxmt_delete(p_nbr, p_lsa->p_rxmt);
                /*delete dd node for this lsa*/
                ospf_nbr_dd_delete(p_nbr, p_lsa->p_rxmt);
            }
            ospf_request_add(p_nbr, p_lshdr);
    
            if (ospf_debug_gr
                && p_if->p_process->in_restart 
                && (OSPF_LS_ROUTER == p_lshdr->type || OSPF_LS_NETWORK == p_lshdr->type))
            {
                ospf_logx(ospf_debug, "add lsa to request list on %s, id %x, type %d, age %d, seq %x,len %d", 
                		ospf_inet_ntoa(str, p_nbr->addr),ntohl(p_lshdr->id),
                  p_lshdr->type, ntohs(p_lshdr->age),
                  ntohl(p_lshdr->seqnum), ntohs(p_lshdr->len));
                  
                if (NULL == p_lsa)
                {
                    ospf_logx(ospf_debug, "local lsa not exist");
                }
                else
                {
                    ospf_logx(ospf_debug, "local lsa, age %d, seq %x, len %d",
                    ntohs(p_lsa->lshdr->age), ntohl(p_lsa->lshdr->seqnum),
                    ntohs(p_lsa->lshdr->len));
                }
            }
        } 
        else
        {
            ospf_logx(ospf_debug_lsa, "local lsa is more recent,ignore rcvd lsa");

            if (ospf_debug_gr
                && p_if->p_process->in_restart 
                && (OSPF_LS_ROUTER == p_lshdr->type || OSPF_LS_NETWORK == p_lshdr->type))
            {
                ospf_logx(ospf_debug, "ignore lsa header from %s, type %d, age %d, seq %x, len %d", 
                 ospf_inet_ntoa(str, p_nbr->addr),p_lshdr->type, ntohs(p_lshdr->age),
                 ntohl(p_lshdr->seqnum), ntohs(p_lshdr->len));
            }
        }
    }

    /*update sequence number:slave accept sequm from master,master:increase seqnum*/
    if (OSPF_SLAVE == p_nbr->dd_state)
    {
        p_nbr->dd_seqnum = ntohl(p_dbd->seqnum);
//		ospf_logx(ospf_debug,"%s,%d p_dbd->seqnum=0x%x\n",__FUNCTION__,__LINE__,p_dbd->seqnum);
//		ospf_logx(ospf_debug,"%s,%d p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_seqnum);
    }
    else
    {
        ++p_nbr->dd_seqnum;
//		ospf_logx(ospf_debug,"%s,%d p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_seqnum);
    }
    return OK;
}

/*prepare txd dd msg header*/
void 
ospf_dd_header_init(
                 struct ospf_nbr *p_nbr,
                 struct ospf_dd_msg *p_dbd)
{
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_process *p_process = p_if->p_process;    
    u_int more = FALSE;
	u_int uiPortMtulen = 0;
	int ret = 0;
	u_int ulIndex = 0;
 //   ospf_logx(ospf_debug, "ospf_dd_header_init \r\n");
    /* RFC 2178,set interface mtu*/
    if ((OSPF_IFT_VLINK == p_if->type)
        || (TRUE == p_if->mtu_ignore))
    {
        p_dbd->mtu = 0;
    }
    else
    {
	    /*若配置mtu值小于jumboframe值，则发出报文中MTU为配置值；?
		反之，若配置MTU值超过jumboframe值，则发出报文中携带MTU 值为jumboframe实际值*/
    	/* 获取端口MTU限制*/
        //ulIndex = ospf_vfindextolpindex(p_if->ifnet_uint);
    	uiPortMtulen = ospf_if_get_mtu(p_if->ifnet_uint);
    	//ret = port_get_api(p_if->ifnet_uint, PORT_API_MTU_LEN, &uiPortMtulen);
		if(ospf_if_mtu_len(p_if) <= uiPortMtulen)
		{
    		p_dbd->mtu = htons(ospf_if_mtu_len(p_if));
		}
		else
		{
    		p_dbd->mtu = htons(uiPortMtulen);
		}

		if(uiPortMtulen == 0)
		{
    		    p_dbd->mtu = htons(ospf_if_mtu_len(p_if));
		}

		//printf("ospf_dd_header_init:ret=%d,p_dbd->mtu=%d,uiPortMtulen=%d,p_if mtu=%d \n",
		//	ret,htons(p_dbd->mtu),uiPortMtulen,ospf_if_mtu_len(p_if));
    }

    /*E/N bit:
        normal:E=1,N=0
        sutb:E=0,N=0
        nssa:E=0,N=1
     */
    if (p_if->p_area->is_nssa) 
    {        
        ospf_set_option_nssa(p_dbd->option);
    }
    else if (!p_if->p_area->is_stub)
    {
        ospf_set_option_external(p_dbd->option);
    }
    
    /*opaque lsa set DD packet options with opaque capability on jkw*/
    if (p_process->opaque)
    {
        ospf_set_option_opaque(p_dbd->option);
    }
       
    /*record More bit from buffer,not from negihbor flag ,it's an error*/
//	ospf_logx(ospf_debug,"%s,%d p_dbd->flags=%d\n",__FUNCTION__,__LINE__,p_dbd->flags);
    more = ospf_dd_flag_more(p_dbd->flags);
    
    p_dbd->flags = p_nbr->dd_flag;
    
    if (more)
    {
        ospf_set_dd_flag_more(p_dbd->flags);
    }

    /*sequence number setting*/
    p_dbd->seqnum = htonl(p_nbr->dd_seqnum);    
//	ospf_logx(ospf_debug,"%s,%d p_dbd->seqnum=0x%x\n",__FUNCTION__,__LINE__,p_dbd->seqnum);
//	ospf_logx(ospf_debug,"%s,%d p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_seqnum);
    return;
}

/* section 10.8 of OSPF specification (page 93) */
void 
ospf_initial_dd_output(struct ospf_nbr *p_nbr)
{
    u_int8 buf[OSPF_DBD_MIN_LEN + OSPF_MD5_KEY_LEN] = {0};
    struct ospf_dd_msg *p_dd = (struct ospf_dd_msg *)buf;
    struct ospf_if *p_if = p_nbr->p_if;
    u_int8 nbr[16];
    u_short len = OSPF_DBD_MIN_LEN;
    u_int dest = OSPF_ADDR_ALLSPF;

  //  ospf_logx(ospf_debug, "send init dbd to %s on %s", ospf_inet_ntoa(nbr,p_nbr->addr), p_nbr->p_if->name);

    /*fill header,use min length*/
    ospf_dd_header_init(p_nbr, p_dd);
    p_dd->h.type = OSPF_PACKET_DBD;

    dest = ospf_packet_dest_select(p_if, p_nbr, OSPF_PACKET_DBD); 
    
    ospf_output(p_dd, p_if, dest, len);

    if (ospf_debug && ospf_debug_msg)
    {
        ospf_log_dd_packet(p_dd, FALSE);
    }
    
    /*restart dd timer*/
    if (p_if->rxmt_interval)
    { 
        ospf_timer_start(&p_nbr->dd_timer, ospf_rand(p_if->rxmt_interval * OSPF_TICK_PER_SECOND));
    }
    else/*fixed to short value*/
    {
        ospf_timer_start(&p_nbr->dd_timer, 2);
    }
    return;
}

/*if current lsa's count exceed this limit,use fast timer for dd retransmit*/
#define OSPF_DD_RATELIMIT_LSA_COUNT 1000

/* section 10.8 of OSPF specification (page 93) */
void 
ospf_dd_output(struct ospf_nbr *p_nbr)
{
    struct ospf_retransmit *p_rxmt = NULL;
    struct ospf_retransmit *p_next = NULL;
    struct ospf_process *p_process = p_nbr->p_if->p_process;
    struct ospf_dd_msg *p_dd = NULL;
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_lstable *p_table = NULL;
    struct ospf_lshdr *p_filled = NULL;
    struct ospf_lshdr *p_lshdr = NULL;
    struct ospf_lsa *p_lsa = NULL;
    u_int dest = p_nbr->addr;
    u_int8 nbr[16]; 
    
 //   ospf_logx(ospf_debug, "ospf_dd_output \r\n");
    /*if last sent dd exist,send it again*/
    if (NULL != p_nbr->p_last_dd)
    {
        p_dd = p_nbr->p_last_dd;
        ospf_dd_header_init(p_nbr, p_dd);   
//		ospf_logx(ospf_debug,"%s,%d p_dd->flags=%d\n",__FUNCTION__,__LINE__,p_dd->flags);
        
        /*special case for last summary packet,we only send empty packet*/
        if (!ospf_dd_flag_more(p_dd->flags) && p_nbr->empty_dd_need)
        {
            p_nbr->last_dd_len =  OSPF_DBD_MIN_LEN;
        }
 
        /*update lsa age?*/
        goto SEND_IT;
    }

    /*create new message*/
    p_nbr->p_last_dd = ospf_malloc2(OSPF_MDBD);
    if (NULL == p_nbr->p_last_dd)
    {
        /*call nsm event to delete nbr*/
        p_nbr->force_down = TRUE;
        
        /*restart dd timer*/
        if (p_if->rxmt_interval)
        { 
            ospf_timer_start(&p_nbr->dd_timer, ospf_rand(p_if->rxmt_interval * OSPF_TICK_PER_SECOND));
        }
        else/*fixed to short value*/
        {
            ospf_timer_start(&p_nbr->dd_timer, 2);
        }
        return;
    }

    /*init packet header,len remain in host order*/
    p_dd = p_nbr->p_last_dd;
    p_dd->h.type = OSPF_PACKET_DBD;
    p_nbr->last_dd_len = OSPF_DBD_MIN_LEN;

    /*init dd header*/
    ospf_dd_header_init(p_nbr, p_dd);   

    /*clear more flag*/
    ospf_reset_dd_flag_more(p_dd->flags);
//	ospf_logx(ospf_debug,"%s,%d p_dd->flags=0x%x\n",__FUNCTION__,__LINE__,p_dd->flags);
    
    /*pointer to first lshdr*/
    p_filled = p_dd->lshdr;

    /*decide next dd to be filled,do not start from the first one*/
    p_next = ospf_lstfirst(&p_process->rxmt_table);
    
    p_lshdr = (struct ospf_lshdr *)p_nbr->next_dd_hdr;
    if (p_lshdr)
    {
        p_table = ospf_lstable_lookup(p_if, p_lshdr->type);
        if (p_table)
        {
            p_lsa = ospf_lsa_lookup(p_table, p_lshdr);
            if (p_lsa && p_lsa->p_rxmt)
            {
                p_next = p_lsa->p_rxmt;
            }
        }
    }
    /*reset next hdr*/
    memset(p_nbr->next_dd_hdr, 0, sizeof(p_nbr->next_dd_hdr));

#if 1/*start from next node*/ 
    for (p_rxmt = p_next, 
         p_next = ospf_lstnext(&p_process->rxmt_table, p_rxmt); 
         p_rxmt; 
         p_rxmt = p_next, 
         p_next = ospf_lstnext(&p_process->rxmt_table, p_rxmt))
#else
    /*scan all dd nodes, rebuild some*/
    for_each_node(&p_process->rxmt_table, p_rxmt, p_next)
#endif    
    {      
       /*must set dd flag for this nbr,and must have lsa*/
       if ((NULL == p_rxmt->p_lsa) || (!ospf_nbr_dd_isset(p_rxmt, p_nbr)))
       {
           continue;
       }
        
       /*update lsa age,if it is maxaged,do not add to dd,it will be added to retransmit list*/
       ospf_lsa_age_update(p_rxmt->p_lsa);
       if (OSPF_MAX_LSAGE <= ntohs(p_rxmt->p_lsa->lshdr->age))
       {
           ospf_nbr_dd_delete( p_nbr,  p_rxmt);
           continue;
       }
        
        /* packet is full, set more flag and stop build*/
       if ((p_nbr->last_dd_len + sizeof(struct ospf_lshdr)) > p_if->maxlen)
       {
       		//p_dd->flags &= 0x3; 
//		ospf_logx(ospf_debug,"%s,%d p_dd->flags=%d\n",__FUNCTION__,__LINE__,p_dd->flags);
           ospf_set_dd_flag_more(p_dd->flags);
           /*store next lsa to send*/
           memcpy(p_nbr->next_dd_hdr, p_rxmt->p_lsa->lshdr, sizeof(struct ospf_lshdr)); 
           break;
       }
       
       /*insert buffer to dd and increase buffer length*/
       memcpy(p_filled, p_rxmt->p_lsa->lshdr, sizeof(struct ospf_lshdr));        
       p_nbr->last_dd_len += sizeof(struct ospf_lshdr);    
       p_filled++;
       
       /*clear dd node for this nbr*/
       ospf_nbr_dd_delete(p_nbr, p_rxmt);

       /*if no dd exist,stop*/
       if (0 == p_nbr->dd_count)
       {
          break;
       }
    }

SEND_IT :
    p_if->stat->tx_lsainfo[OSPF_PACKET_DBD] += (p_nbr->last_dd_len-OSPF_DBD_MIN_LEN)/sizeof(struct ospf_lshdr);
    dest = ospf_packet_dest_select(p_if, p_nbr, OSPF_PACKET_DBD);
 //   dest = OSPF_ADDR_ALLSPF;//修改为广播发送
    ospf_logx(ospf_debug, "send dbd to %s on %s,length %d", ospf_inet_ntoa(nbr, dest), p_if->name, p_nbr->last_dd_len);
        
    ospf_output(p_dd, p_if, dest, p_nbr->last_dd_len);        

    if (ospf_debug && ospf_debug_msg)
    {
        ospf_log_dd_packet(p_dd, FALSE);
    }
    
    /*restart dd timer*/
    if (p_if->rxmt_interval)
    {
       /*rate limit*/  
       /* for testcenter:if lsa's count do not exceed limit,remain default rxmt interval*/
       if (ospf_timer_active(&p_process->ratelimit_timer)
           && (OSPF_DD_RATELIMIT_LSA_COUNT < ospf_lstcnt(&p_process->rxmt_table)))
       { 
           ospf_timer_start(&p_nbr->dd_timer, ospf_rand(OSPF_TICK_PER_SECOND));
       }
       else
       { 
           ospf_timer_start(&p_nbr->dd_timer, ospf_rand(p_if->rxmt_interval*OSPF_TICK_PER_SECOND));
       }
    }
    else/*short value*/
    {
       ospf_timer_start(&p_nbr->dd_timer, 2);
    }
    return;
}

