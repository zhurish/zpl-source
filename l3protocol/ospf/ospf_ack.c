/* ospf_ack.c - ls ack message i/o processing*/

#include "ospf.h"
//#include "ospf_main.h"

extern struct ospf_global ospf ;

void ospf_ack_output(struct ospf_ackinfo *p_ack);

/*process lsa header in ack packet*/
 u_int
ospf_process_acked_lsa(
           struct ospf_nbr *p_nbr,
           struct ospf_lshdr *p_lshdr)
{
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_lsa *p_lsa = NULL;
    u_int8 sstr[100];

    ospf_logx(ospf_debug_lsa, "check acked lsa,%s", ospf_print_lshdr(p_lshdr, sstr));

    if (ospf_debug_lsa && ospf_debug_msg)
    {
        ospf_logx(ospf_debug, "  lsa header");
        ospf_log_lsa_header(p_lshdr); 
        ospf_logx(ospf_debug, " ");
    }
    
    if (OSPF_LS_TYPE_9 == p_lshdr->type)
    {
        ospf_logx(ospf_debug_gr, "rxd grace lsa ack from %s", ospf_inet_ntoa(sstr,p_nbr->addr));
    }

    /*local lsa has retransmit and same as ack,delete retransmit*/            
    p_lsa = ospf_lsa_lookup(ospf_lstable_lookup(p_if, p_lshdr->type), p_lshdr);            

    if ((NULL != p_lsa)
         && ospf_nbr_rxmt_isset(p_lsa->p_rxmt, p_nbr) 
         && (0 == ospf_lshdr_cmp(p_lshdr, p_lsa->lshdr)))
    {                            
        ospf_logx(ospf_debug_lsa, "expected ack rxd,delete nbr retransmit");

        ospf_nbr_rxmt_delete(p_nbr, p_lsa->p_rxmt);
        p_nbr->lsa_acked++;
        return TRUE;
    } 
    return FALSE;
}

/********************************************************
  process rxd ack message according to rfc2328 13.7
 *******************************************************/
void 
ospf_ack_input (
              struct ospf_nbr *p_nbr,
              struct ospf_hdr *p_hdr)
{
    struct ospf_ack_msg *p_ack = (struct ospf_ack_msg *)p_hdr;
    struct ospf_lshdr *p_lshdr = NULL;
    struct ospf_if *p_if = p_nbr->p_if;
    u_int acked = 0;
    u_int len = (ntohs(p_hdr->len) - OSPF_PACKET_HLEN);
    u_int count = (len/OSPF_LSA_HLEN);
    u_int i = 0;
    u_int8 nbr[16];

   //printf("recv ack from %s on %s,length %d.\r\n", ospf_inet_ntoa(nbr, p_nbr->addr), p_nbr->p_if->name, len);
    ospf_logx(ospf_debug, "recv ack from %s on %s,length %d", ospf_inet_ntoa(nbr, p_nbr->addr), p_nbr->p_if->name, len);
    if (OSPF_NS_EXCHANGE > p_nbr->state )
    {
        ospf_logx(ospf_debug, "invalid neighbor state");
        p_if->stat->error[OSPF_IFERR_ACK_NBR]++;
        return;
    }

    if (ospf_debug && ospf_debug_msg)
    {
        ospf_log_ack_packet(p_ack, TRUE);
    }
    
    p_nbr->p_if->stat->rx_lsainfo[OSPF_PACKET_ACK] += count; 

    /*scan for all of lsa headers in this packet*/
    for (i = 0, p_lshdr = p_ack->lshdr; i < count; i++, p_lshdr++)
    {
        if (TRUE == ospf_process_acked_lsa(p_nbr, p_lshdr))
        {
            acked++;
        }
    }
    
    /*if enough lsa is acked,send next retransmit fast,
      do not wait for retransmit interval*/
    if ((0 != p_nbr->lsa_rxmted) && (0 != acked))
    {             
        if (p_nbr->lsa_acked >= p_nbr->lsa_rxmted)
        {
            p_nbr->lsa_acked = 0;
            p_nbr->lsa_rxmted = 0;
            ospf_timer_start(&p_nbr->lsrxmt_timer, 0);
        }
        else
        {
            ospf_timer_start(&p_nbr->lsrxmt_timer, 5);
        }
    }
    return;
}

/************************************************************
   insert a lsa header into ack buffer.
*/
void 
ospf_ack_add(
            struct ospf_ackinfo *p_ack, 
            struct ospf_lshdr *p_lshdr)
{
    /*allocate message*/
    if (NULL == p_ack->p_msg)
    {
        u_int ulLen = 0;

        ulLen = (p_ack->p_if->mtu/sizeof(struct ospf_hdr))*sizeof(struct ospf_hdr);
  //      printf("ospf_ack_add ulLen=%d,mtu=%d,ospf_hdr = %d\n",ulLen,p_ack->p_if->mtu,sizeof(struct ospf_hdr));
       // p_ack->p_msg = ospf_malloc(ulLen, OSPF_MPACKET);
          p_ack->p_msg = ospf_malloc(p_ack->p_if->mtu, OSPF_MPACKET);

        if (NULL == p_ack->p_msg)
        {
            return;
        }
        /*set message type,init length*/
        p_ack->p_msg->type = OSPF_PACKET_ACK;
        p_ack->p_msg->len = OSPF_PACKET_HLEN;
    }
    
    /*buffer full,send now,buffer remained*/
    if ((p_ack->p_msg->len + OSPF_LSA_HLEN) > p_ack->p_if->maxlen)
    {
        ospf_ack_output((struct ospf_ackinfo *)p_ack);
        /*set message type,init length*/
        p_ack->p_msg->type = OSPF_PACKET_ACK;
        p_ack->p_msg->len = OSPF_PACKET_HLEN;
    }

    /*copy lsa ack header to buffer*/
    memcpy(((u_int8 *)p_ack->p_msg) + p_ack->p_msg->len, p_lshdr, OSPF_LSA_HLEN);        
    p_ack->p_msg->len += OSPF_LSA_HLEN;  
    return;
}

/************************************************************
   send ack after processing update 
*/
void 
ospf_ack_output(struct ospf_ackinfo *p_ack)
{
    struct ospf_if *p_if = p_ack->p_if;
    u_int dest = 0;
    u_int8 dstaddr[32];

    /*have no lsa header for ack,do nothing*/
    if ((NULL == p_ack->p_msg) || (OSPF_PACKET_HLEN >= p_ack->p_msg->len ))
    {
        return;
    }

    /*increase statistics*/ 
    p_if->stat->tx_lsainfo[OSPF_PACKET_ACK] += (p_ack->p_msg->len - OSPF_PACKET_HLEN)/OSPF_LSA_HLEN;

    dest = ospf_packet_dest_select(p_if, NULL, OSPF_PACKET_ACK);
        
    ospf_logx(ospf_debug, "send ack to %s on %s,len %d", ospf_inet_ntoa(dstaddr, dest), p_if->name, p_ack->p_msg->len);

    ospf_output (p_ack->p_msg, p_if, dest, p_ack->p_msg->len);

    if (ospf_debug && ospf_debug_msg)
    {
        ospf_log_ack_packet((struct ospf_ack_msg *)p_ack->p_msg, FALSE);
    }
    return;
}

