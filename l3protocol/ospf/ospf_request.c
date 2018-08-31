/* ospf_request.c - ls request packet i/o processing*/

#include "ospf.h"

/* section 10.7 of OSPF specification (page 93) */
void 
ospf_request_input(
      struct ospf_nbr *p_nbr,
      struct ospf_hdr *p_hdr)
{
    u_int8 buf[OSPF_MAX_TXBUF+OSPF_MD5_KEY_LEN];
    struct ospf_request_msg *p_req = (struct ospf_request_msg *)p_hdr;
    struct ospf_request_unit  *p_requnit = NULL;
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_updateinfo update;
    struct ospf_lshdr lsh;
    u_int len = ntohs(p_hdr->len) - OSPF_PACKET_HLEN;
    u_int count = 0;
    u_int i;
    u_int8 p_string[100];
    u_int8 nbr[16];
     
    ospf_logx(ospf_debug, "recv request from %s on %s,length %d", ospf_inet_ntoa(nbr,p_nbr->addr), p_if->name, len);
    //printf("recv request from %s on %s,length %d.\r\n", ospf_inet_ntoa(nbr,p_nbr->addr), p_if->name, len);
 
    if (OSPF_NS_EXCHANGE > p_nbr->state)
    {
        ospf_logx(ospf_debug, "neighbor state is invalid"); 
        
        p_if->stat->error[OSPF_IFERR_REQ_NBR]++;
        return;
    }
     
    if (0 >= len)
    {
        p_if->stat->error[OSPF_IFERR_REQ_EMP]++;
        ospf_logx(ospf_debug, "request list is null"); 
        return;
    }

    if (ospf_debug && ospf_debug_msg)
    {
        ospf_log_request_packet(p_req, TRUE);
    }
    /*set update buffer*/
    update.p_nbr = p_nbr;
    update.p_if = NULL;
    update.p_msg = NULL;
    update.maxlen = 0;
 
    /*process each request*/ 
    p_requnit = p_req->lsa;
    count = len/OSPF_REQ_UNIT_LEN;
    p_if->stat->rx_lsainfo[OSPF_PACKET_REQUEST] += count; 
 
    for (i = 0;  i < count; i++, p_requnit ++)
    {
        lsh.type = p_requnit->type;
        lsh.id = p_requnit->id;
        lsh.adv_id = p_requnit->adv_id;
        
        ospf_logx(ospf_debug_lsa, "check request lsa,%s", ospf_print_lshdr(&lsh, p_string)); 
   
        /*local lsa must exist for requested*/
        p_lsa = ospf_lsa_lookup(ospf_lstable_lookup(p_if, lsh.type), &lsh);
        if (NULL == p_lsa)
        {
            ospf_logx(ospf_debug|ospf_debug_error, "invalid requested lsa");
            ospf_logx(ospf_debug_error, "check request lsa,%s", ospf_print_lshdr(&lsh, p_string));
            
            p_if->stat->error[OSPF_IFERR_REQ_INV]++;
            
            ospf_nsm(p_nbr, OSPF_NE_BAD_REQUEST);
            break;
        }
        /*add lsa to update buffer*/
        ospf_update_buffer_insert(&update, p_lsa, buf);
    }
    /*send update to nbr*/
    ospf_update_output(&update);
    return;
}

/* section 10.9 of OSPF specification (page 94) */
void 
ospf_request_output (struct ospf_nbr *p_nbr)
{
    u_int8 buf[OSPF_MAX_TXBUF];
    struct ospf_request_node *p_req = NULL;
    struct ospf_request_node *p_next = NULL;
    struct ospf_request_msg *p_packet = NULL;
    struct ospf_request_unit  *p_fill_lsa = NULL;
    struct ospf_if *p_if = p_nbr->p_if;
    u_int max_payload = p_if->maxlen - OSPF_UPDATE_MIN_LEN;
    u_int reply_len = 0;
    u_int reply_lscount = 0;
    u_int dest = 0;
    u_int8 nbr[16];
    u_int8 lsastr[100];   

    ospf_logx(ospf_debug, "prepare send request to nbr %s", ospf_inet_ntoa(nbr, p_nbr->addr));
    
    /*reset count of request and reply*/
    p_nbr->lsa_requested = 0;
    p_nbr->lsa_reply = 0;
        
    p_packet = (struct ospf_request_msg *)buf;
    memset(p_packet, 0, sizeof(struct ospf_request_msg));
    
    /*head len:request has no type special part*/
    p_packet->h.len = OSPF_PACKET_HLEN;
    p_packet->h.type = OSPF_PACKET_REQUEST;

    /*request body filled*/     
    p_fill_lsa = p_packet->lsa;

    /*scan for all of request unit*/
    for_each_node(&p_if->p_process->req_table, p_req, p_next)
    {
        /*must attached to same nbr*/
        if (p_req->p_nbr != p_nbr)
        {
            continue;
        }
        /*if request message will exceed mtu,stop sending,only one msg need*/
        if ((p_packet->h.len + OSPF_REQ_UNIT_LEN) > p_if->maxlen)
        {
            break;
        }
        /*if calculated replied update message will exceed 2*mtu, stop build request*/
        if ((0 < reply_len)
            && ((reply_len + ntohs(p_req->ls_hdr.len)) > (2 * max_payload)))
        {   
            break;
        }
        ospf_logx(ospf_debug_lsa, "add request lsa,%s", ospf_print_lshdr(&p_req->ls_hdr, lsastr));

        /*20121129*/
        if (ospf_debug_gr
            && p_if->p_process->in_restart 
            && (OSPF_LS_ROUTER == p_req->ls_hdr.type || OSPF_LS_NETWORK == p_req->ls_hdr.type))
        {
            ospf_logx(ospf_debug, "send request to %s-%s, type %d, id %x", 
              ospf_inet_ntoa(nbr, p_nbr->addr), p_if->name, p_req->ls_hdr.type,
              ntohl(p_req->ls_hdr.id));
        }
        /*fill into requested buffer*/
        p_fill_lsa->rsvd[0] = 0x00;
        p_fill_lsa->rsvd[1] = 0x00;
        p_fill_lsa->rsvd[2] = 0x00;
        p_fill_lsa->type = p_req->ls_hdr.type;
        p_fill_lsa->id = p_req->ls_hdr.id;
        p_fill_lsa->adv_id = p_req->ls_hdr.adv_id;
        p_fill_lsa++;
        p_packet->h.len += OSPF_REQ_UNIT_LEN;

        /*calculate replied update length and lsa count*/ 
        reply_len += ntohs(p_req->ls_hdr.len);
        reply_lscount++;    
        
        /*all request included,stop scan*/
        if (reply_lscount >= p_nbr->req_count)
        {
            break;
        }
    }
    /*send request if need*/
    if (0 == reply_lscount)
    {
        return;
    }
    
    /*record expected update count*/
    p_nbr->lsa_requested = reply_lscount;
   
    /*increase statistics*/
    p_if->stat->tx_lsainfo[OSPF_PACKET_REQUEST] += reply_lscount;
   
    /*ucast dest setting*/
    dest = ospf_packet_dest_select(p_if, p_nbr, OSPF_PACKET_REQUEST);
   
    ospf_logx(ospf_debug, "send request to %s on %s,length %d", ospf_inet_ntoa(nbr, dest), p_if->name, p_packet->h.len);
   
    ospf_output (p_packet, p_if, dest, p_packet->h.len);

    if (ospf_debug && ospf_debug_msg)
    {
        ospf_log_request_packet(p_packet, FALSE);
    }

    /*restart request timer*/
    if (p_if->rxmt_interval)
    {
        if (ospf_timer_active(&p_if->p_process->ratelimit_timer))
        {
            /*use short timer for large nbr number*/
            if (OSPF_MAX_EXCHANGE_NBR <= ospf_lstcnt(&p_if->p_process->nbr_table))
            {
                ospf_timer_start(&p_nbr->request_timer, 2);
            }
            else
            {
                /*500ms*/
                ospf_timer_start(&p_nbr->request_timer, 5);
            }
        }
        else
        { 
            ospf_timer_start(&p_nbr->request_timer, ospf_rand(p_if->rxmt_interval*OSPF_TICK_PER_SECOND));
        }
    }
    else
    {
        ospf_timer_start(&p_nbr->request_timer, 2);
    }
    return;
}

