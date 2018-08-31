/* ospf_lsa.c - contain lsa body operation and lsdb operation*/

#include "ospf.h"
extern tOSPF_LSA_MANAGE stOsLsa;
extern u_long ulOspfmemFlag;
extern u_char ucLsa_te;

extern void ospf_local_lsa_install(struct ospf_lstable *p_table,struct ospf_lshdr * p_lsa);
extern void ospf_router_lsa_option_build(struct ospf_router_lsa *p_lsa,struct ospf_area *p_area);
extern void ospf_lsa_stat_update(struct ospf_lsa *p_lsa, int step);
extern void ospf_lsa_id_conflict_check (struct ospf_lstable *p_table, struct ospf_lshdr *p_lshdr);
extern void ospf_conflict_network_table_update(struct ospf_lstable *p_table);
extern u_int ospf_router_lsa_link_fill(struct ospf_router_link * p_link, struct ospf_if * p_if);
extern u_int ospf_router_lsa_te_tunnel_fill(struct ospf_router_link * p_link, struct ospf_area * p_area);
u_int ospf_router_lsa_dcn_fill(struct ospf_router_link * p_link, struct ospf_area * p_area);
extern u_int ospf_get_te_instance(struct ospf_area * p_area);
extern u_int ospf_lsa_changed(struct ospf_lshdr * p_ulsa, struct ospf_lshdr * p_ulsa2);
extern void ospf_vpn_summary_lsa_originate(struct ospf_iproute * p_route, struct ospf_area * p_area, uint32_t need_flush);

extern int ospf_conflict_network_cmp(struct ospf_conflict_network *p1, struct ospf_conflict_network *p2);
extern struct ospf_lsa *ospf_lsa_add(struct ospf_lsa *p_old, struct ospf_lstable *p_table,struct ospf_lshdr * p_lsa);
extern int log_time_print (u_int8 *buf);

/*add log for lsa,if current count exceed limit,reuse the first one*/
void
ospf_lsa_log_add(
         struct ospf_process *p_process, 
         struct ospf_lsa_loginfo *p_log)
{
    struct ospf_lsa_loginfo *p_new = NULL;

    memset(&p_log->node, 0, sizeof(p_log->node));

    if (OSPF_STAT_MAX_NUM <= ospf_lstcnt(&ospf.log_table.lsa_table))
    {
        p_new = ospf_lstfirst(&ospf.log_table.lsa_table);
        ospf_lstdel_unsort(&ospf.log_table.lsa_table, p_new);
    }
    else
    {
        p_new = ospf_malloc(sizeof(struct ospf_lsa_loginfo), OSPF_MSTAT);
    }
    if (NULL == p_new)
    {
        return;
    }    
    memcpy(p_new, p_log, sizeof(struct ospf_lsa_loginfo));

    ospf_lstadd_unsort(&ospf.log_table.lsa_table, p_new);
    return;   
}

/*compare lsa for lookup*/ 
int 
ospf_lsa_lookup_cmp(
                 struct ospf_lsa *p1, 
                 struct ospf_lsa *p2)
{
    struct ospf_lshdr *h1 = NULL;
    struct ospf_lshdr *h2 = NULL;
 
    if (!p1 || !p2) 
    {
       return 1;
    }
 
    h1 = p1->lshdr;
    h2 = p2->lshdr;
    
    OSPF_KEY_CMP(h1, h2, type);
    /*compare id and router with host order*/
    OSPF_KEY_HOST_CMP(h1, h2, id);
    OSPF_KEY_HOST_CMP(h1, h2, adv_id);
    return 0;
}

int
ospf_area_lstable_nm_cmp(
                   struct ospf_lstable *p1,
                   struct ospf_lstable *p2)
{
    /*process id*/
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
 
    /*area id*/
    OSPF_KEY_CMP(p1, p2, p_area->id);
 
    /*type*/
    OSPF_KEY_CMP(p1, p2, type);
    return 0;
}

int
ospf_as_lstable_nm_cmp(
                   struct ospf_lstable *p1,
                   struct ospf_lstable *p2)
{
    /*process id*/
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
 
    /*type*/
    OSPF_KEY_CMP(p1, p2, type);
    return 0;
}

int 
ospf_if_lstable_nm_cmp(
                   struct ospf_lstable *p1,
                   struct ospf_lstable *p2)
{
    /*process id*/
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
 
    /*interface address*/  
    OSPF_KEY_CMP(p1, p2, p_if->addr);
    #ifdef OSPF_DCN
    /*interface uinit*/ 
    if ((OSPF_DCN_FLAG == p1->p_if->ulDcnflag) && (OSPF_DCN_FLAG == p2->p_if->ulDcnflag))
    {
        OSPF_KEY_CMP(p1, p2, p_if->ifnet_uint);
    }
    #endif
    /*type*/
    OSPF_KEY_CMP(p1, p2, type);
    return 0;
}  

int
ospf_vif_lstable_nm_cmp (
                   struct ospf_lstable *p1,
                   struct ospf_lstable *p2)
{
    /*process id*/
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
 
    /*vif*/  
    /*transit id*/
    if (p1->p_if->p_transit_area && p2->p_if->p_transit_area)
    {
        OSPF_KEY_CMP(p1->p_if, p2->p_if, p_transit_area->id);
    }
    /*nbr id*/ 
    OSPF_KEY_CMP(p1->p_if, p2->p_if, nbr);
    
    /*type*/
    OSPF_KEY_CMP(p1, p2, type);
    return 0;
}  

/*init a lsa table*/
void 
ospf_lsa_table_init(
           struct ospf_lstable *p_table,
           u_int type,
           struct ospf_process *p_process,
           struct ospf_area *p_area,
           struct ospf_if *p_if)
{
    ospf_lstinit(&p_table->list, ospf_lsa_lookup_cmp);
    ospf_lstinit(&p_table->conflict_list, ospf_conflict_network_cmp);
    p_table->p_process = p_process;
    p_table->p_area = p_area;
    p_table->p_if = p_if;
    p_table->type = type;
    
    ospf_lstadd(&p_process->lstable_table, p_table);
    return;
}

void
ospf_lsa_table_shutdown(struct ospf_lstable *p_table)
{
    ospf_lsa_table_flush(p_table);
    ospf_lstdel(&p_table->p_process->lstable_table, p_table);
    return;
}

/*build common option fields in lsa header*/
void 
ospf_lsa_option_build(
                    struct ospf_area *p_area,
                    struct ospf_lshdr *p_lshdr)
{
    u_int type = p_lshdr->type;
    /*external lsa only has external bit*/
    if ((OSPF_LS_AS_EXTERNAL == type) || (OSPF_LS_TYPE_11 == type))
    {
        ospf_set_option_external(p_lshdr->option);
        return;
    }
 
    /*area must exist for other type of lsa*/
    if (NULL == p_area)
    {
        return;
    }
    /*external bit*/
    if (!p_area->is_stub)
    {
        ospf_set_option_external(p_lshdr->option);
    }
    else
    {
        p_lshdr->option = 0;
    }
    /*nssa bit*/ 
    if (p_area->is_nssa)
    {
        ospf_set_option_nssa(p_lshdr->option);
    }

    return;
}

/*get next seqnum*/
int 
ospf_lsa_next_seqnum(int seq)
{
    int hseq = ntohl(seq) + 1;
        
    if (OSPF_INVALID_LS_SEQNUM == hseq)
    {
        hseq = (int)OSPF_INIT_LS_SEQNUM;
    }
    return htonl(hseq);
}

/*decide if we can accept a special lsa type in msg*/
u_int 
ospf_lstype_is_valid (
                      struct ospf_if *p_if,
                      u_int8 lstype)
{
    switch (lstype){
        case OSPF_LS_ROUTER:
        case OSPF_LS_NETWORK:
        case OSPF_LS_SUMMARY_NETWORK:
        case OSPF_LS_SUMMARY_ASBR:
             return TRUE;
             break;
        
        case OSPF_LS_TYPE_7:
             return TRUE;
             break;
         
        case OSPF_LS_TYPE_9:
        case OSPF_LS_TYPE_10:
             return TRUE;
             break;
             
        case OSPF_LS_AS_EXTERNAL:
        case OSPF_LS_TYPE_11:
             /* opaque 11 not flooded in stub/nssa areas.*/
             if (p_if->p_area->is_stub || p_if->p_area->is_nssa)
             {               
                 return FALSE;                                            
             }
             else
             {
                 return TRUE;
             }        
             break;  
            
        default:
             break;
    }    
    return FALSE;
}

/*New instance of lsa originate*/
void 
ospf_lsa_refresh (struct ospf_lsa *p_lsa)
{
    struct ospf_process *p_process = p_lsa->p_lstable->p_process;
    int seqnum = 0;
    u_int8 lstype = 0;
    u_int8 str[100]; 
        
    if (NULL == p_lsa->lshdr)
    {
        return;
    }

    /*if GR is in progress,do not originate lsa*/
    if (p_process->in_restart)
    {
        ospf_logx(ospf_debug_lsa|ospf_debug_gr, "router is in graceful restart,do not originate lsa"); 
        return;
    }
    
    lstype = p_lsa->lshdr->type;
    seqnum = ntohl(p_lsa->lshdr->seqnum);
    
    ospf_logx(ospf_debug_lsa, "regenerate lsa,%s", ospf_print_lshdr(p_lsa->lshdr, str));
	//printf("ospf_lsa_refresh: p_lsa %p len %d\n", p_lsa, p_lsa->lshdr->len);
    /*case of max sequence*/
    if ((OSPF_MAX_LS_SEQNUM == seqnum) && (!p_lsa->maxseq_wait))
    {
        ospf_logx(ospf_debug_lsa, "need delete this lsa");
        
        /* section 12.1.6 (page 109-110) */
        ospf_lsa_maxage_set(p_lsa);
        
        ospf_lsa_flood(p_lsa, NULL, NULL);
        /*set max-seq flag,when acked,originate it using init-seq*/
        p_lsa->maxseq_wait = TRUE;
        return;
    }

    /*already have maxseq,reoriginate it*/
    if (p_lsa->maxseq_wait)
    {
        /*clear max-seq flag*/
        p_lsa->maxseq_wait = FALSE;

        /*next seq will be init value*/
        seqnum = OSPF_INVALID_LS_SEQNUM;
    }
    
    /*decide new sequence and new checksum*/
    p_lsa->lshdr->seqnum = ospf_lsa_next_seqnum(htonl(seqnum));
    p_lsa->lshdr->age = 0x0000;
    
    p_lsa->lshdr->checksum = 0x0000;
    p_lsa->lshdr->checksum = ospf_lsa_checksum_calculate(p_lsa->lshdr);

    /*add to lsa table*/
    p_lsa = ospf_lsa_install(p_lsa->p_lstable, p_lsa, p_lsa->lshdr);
    if (NULL != p_lsa)
    {
    	//printf("ospf_lsa_refresh: p_lsa %p len %d after\n", p_lsa, p_lsa->lshdr->len);
        ospf_trap_origin(p_lsa);  
        ospf_lsa_flood(p_lsa, NULL, NULL);
    }
    p_process->origin_lsa++;
    return;
}

/*next router link in router lsa,consider TOS count*/
struct ospf_router_link *
ospf_router_link_next(
            struct ospf_router_lsa *p_lsa, 
            struct ospf_router_link *p_link)
{
    struct ospf_router_link *p_next = NULL;
    u_long limit = 0;
    u_long next = 0;
    if (p_link)
    {
        limit = (u_long)(p_lsa) + ntohs(p_lsa->h.len);
        next = ((u_long)(p_link) + ospf_router_link_len((p_link)));
        p_next = (struct ospf_router_link *) ((next >= limit) ? 0 : next);
    }
    return p_next;
}

/*next network link*/
u_int *
ospf_network_link_next(
              struct ospf_network_lsa *p_lsa,
              u_int *p_link) 
{
    u_long limit = (u_long)(p_lsa) + ntohs(p_lsa->h.len);
    u_long next = (u_long)(p_link + 1);
    u_int *p_next = NULL;
    if ((p_link))
    {
        p_next = (u_int *) ((next >= limit) ? 0 : next);
    }
    return p_next;
}

/*decide if some vlink using this area as transit,and vnbr is full*/
 u_int
ospf_full_vif_nbr_exist(struct ospf_area *p_area)
{
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_if* p_if = NULL;
    struct ospf_if *p_next = NULL;
    struct ospf_nbr* p_nbr = NULL;

    for_each_node(&p_process->virtual_if_table, p_if, p_next)
    {
        if (p_if->p_transit_area == p_area)
        {
            p_nbr = ospf_nbr_first(p_if);
            if ((NULL != p_nbr) && (OSPF_NS_FULL == p_nbr->state))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*when regenerate router lsa,need check if option changed,and update 
* this routine updateoption of router lsa when refreshing router lsa*/
void 
ospf_router_lsa_option_build(
                  struct ospf_router_lsa *p_lsa,
                  struct ospf_area *p_area)
{
    struct ospf_area *p_nssa_area = NULL;
    struct ospf_area *p_next_area = NULL;
    struct ospf_process *p_process = p_area->p_process;
    
    ospf_lsa_option_build(p_area, &p_lsa->h);

    p_lsa->flag = 0;
    
    /*ABR flag*/    
    if (p_process->abr)
    {
        ospf_set_router_flag_abr(p_lsa->flag);
    }
    
    /*ASBR flag*/
    if ((!p_area->is_stub) && p_process->asbr) 
    {
        ospf_set_router_flag_asbr(p_lsa->flag);
    }
    /* ASBR flag set for default route advertise*/
    if ((!p_area->is_stub) && p_process->def_route_adv) 
    {
        ospf_set_router_flag_asbr(p_lsa->flag);
    }
    
    /*auto set ASBR bit when there be nssa area,and i am abr*/
    if (p_process->abr)
    {
        for_each_ospf_area(p_process, p_nssa_area, p_next_area)
        {            
            if (p_nssa_area->is_nssa)
            {
               
                #if 0
                if (FALSE == ospf_backbone_full_nbr_exsit(p_process))
                {
                    p_lsa->flag &= ~0x01;
                }
                #endif
                ospf_set_router_flag_asbr(p_lsa->flag);
                break;
            }
        }
    }    

    /*virtual flag:a vlink use this area as transit area, and nbr is full*/
    if (TRUE == ospf_full_vif_nbr_exist(p_area))
    {
        ospf_set_router_flag_vlink(p_lsa->flag);
    }
        
    /*if this is nssa area and translator role is always,set nt bit*/
    /*only abr set nt bit*/
    if (p_area->is_nssa 
		&& p_area->nssa_always_translate 
		&& p_process->abr 
		&& ospf_backbone_full_nbr_exsit(p_process))
    {
        ospf_set_router_flag_translator(p_lsa->flag);
    }     
    return;
}

/*construct router lsa*/
void 
ospf_router_lsa_build(
              struct ospf_area *p_area,
              struct ospf_router_lsa *p_router)
{
    struct ospf_router_link *p_link = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL; 
    struct ospf_process *p_process = p_area->p_process;
    u_int len = 0;
    
    ospf_init_lshdr(&p_router->h, OSPF_LS_ROUTER, p_process->router_id, p_process->router_id);
    
    /*decide header option,and bit flag*/ 
    ospf_router_lsa_option_build(p_router, p_area);
    
    p_router->link_count = 0;
    p_router->h.len = OSPF_ROUTER_LSA_HLEN;
    
    /* section 12.4.1, first bullet item (page 119) - only examine 
    those interfaces with attached networks belonging to this area */
    p_link = p_router->link;
    for_each_node(&p_area->if_table, p_if, p_next_if)
    {
        len = ospf_router_lsa_link_fill(p_link, p_if);
        if (0 != len)
        {
            p_router->link_count ++;
            /*ppp may add 2 interfaces*/ 
            if (len != ospf_router_link_len(p_link))
            {
                p_router->link_count++;
            }            
        }
        p_router->h.len += len;
        p_link = (struct ospf_router_link *) ((u_long) p_link + len);       
    }
#ifdef OSPF_DCN
	if(OSPF_DCN_PROCESS == p_process->process_id)
	{
	    /*dcn fill*/
	    len = ospf_router_lsa_dcn_fill(p_link, p_area);
	    if (0 != len)
	    {
	        p_router->link_count++;
	    }
	    p_router->h.len += len;
	    p_link = (struct ospf_router_link *) ((u_long) p_link + len);       
	}
#endif
    //if(p_process->mpls_flag == TRUE)
    if((p_process->mpls_flag == TRUE)
        && (p_process->mpls_te == TRUE))
    {
	    len = ospf_router_lsa_lsrid_fill(p_link, p_area);
	    if (0 != len)
	    {
	        p_router->link_count++;
	    }
	    p_router->h.len += len;
	    p_link = (struct ospf_router_link *) ((u_long) p_link + len); 
	}

    /*te tunnel fill*/
    len = ospf_router_lsa_te_tunnel_fill(p_link, p_area);
    if (0 != len)
    {
        p_router->link_count++;
    }
    p_router->h.len += len;
    
    p_router->h.len = htons(p_router->h.len);
    p_router->link_count = htons(p_router->link_count);    
    
    /*if no link contained,set age to max*/
    if (0 == p_router->link_count)
    {
        p_router->h.age = htons(OSPF_MAX_LSAGE);
        ospf_stimer_safe_start(&p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);
    }
    return;
}

/* section 12.4.1 (page 116-123) */
void 
ospf_router_lsa_originate (struct ospf_area *p_area)
{
    u_char buf[OSPF_MAX_TXBUF];
    struct ospf_router_lsa *p_router = NULL;
    struct ospf_process *p_process = p_area->p_process;
    u_int if_count = (OSPF_MAX_TXBUF - OSPF_ROUTER_LSA_HLEN)/OSPF_ROUTER_LINK_LEN;
    u_int dynamic_buffer = FALSE;
    u_int now = ospf_sys_ticks();
    u_char state_mask[4] = {0};
    
//    printf("originate router lsa for area %u", p_area->id);
    ospf_logx(ospf_debug_lsa, "originate router lsa for area %u", p_area->id);

    /*set state mask,check exchange or loading nbr*/
    BIT_LST_SET(state_mask, OSPF_NS_EXCHANGE);
    BIT_LST_SET(state_mask, OSPF_NS_LOADING);

    p_area->last_routelsa_time = now;

    /* big packet:if lsa length exceed current buffer, allocate larger buffer*/
    /* sub 10 for safe.*/
    if (ospf_lstcnt(&p_process->if_table) >= (if_count - 10))
    {
        u_int ulLen = 0;

        ulLen = (OSPF_BIGPKT_BUF/sizeof(struct ospf_router_lsa))*sizeof(struct ospf_router_lsa);
        dynamic_buffer = TRUE;
    //    printf("ospf_router_lsa_originate ulLen=%d,buf_len=%d,ospf_hdr = %d\n",ulLen,OSPF_BIGPKT_BUF,sizeof(struct ospf_router_lsa));
     //   p_router = ospf_malloc(ulLen, OSPF_MPACKET);

        p_router = ospf_malloc(OSPF_BIGPKT_BUF, OSPF_MPACKET);
        if (NULL == p_router)
        {
            return;
        }
    }
    else
    {
        p_router = (struct ospf_router_lsa *)buf;
    }

    memset(p_router, 0, sizeof(struct ospf_router_lsa));
    ospf_router_lsa_build(p_area, p_router);
	
   // ospf_router_lsa_print(p_router);//新加打印
    if (p_router == NULL)
    {
        return;
    }
    ospf_local_lsa_install(p_area->ls_table[p_router->h.type], &p_router->h);

    /*big packet*/
    if (TRUE == dynamic_buffer)
    {
        ospf_mfree(p_router, OSPF_MPACKET);
    }
    return;
}

/*insert one interface in loopback state into router lsa*/
u_int 
ospf_router_lsa_loop_link_fill(
                        struct ospf_router_link *p_link,
                        struct ospf_if *p_if)
{
    u_int len = 0; 
    
    if (OSPF_IFT_PPP != p_if->type)
    {
        p_link->id = htonl(p_if->addr);
        p_link->data = htonl(OSPF_HOST_MASK);
        p_link->type = OSPF_RTRLINK_STUB;
        p_link->tos0_metric = htons(0x0000);
        len = ospf_router_link_len(p_link);            
    }
    return len;
}

/*insert one interface with p2mp type into router lsa*/
u_int 
ospf_router_lsa_p2mp_link_fill(
                           struct ospf_router_link *p_link,
                           struct ospf_if *p_if)
{
    struct ospf_nbr *p_nbr = NULL; 
    u_int len = 0;
    
    p_nbr = ospf_nbr_first(p_if);

    if ((NULL != p_nbr) && (OSPF_NS_FULL == p_nbr->state))
    {
        /* section 12.4.1.1, first bullet item (page 119-120) */
        p_link->id = htonl(p_nbr->id);
        p_link->data = htonl(p_if->addr);
        p_link->type = OSPF_RTRLINK_PPP;
        if (p_if->ucLdpSyncEn == TRUE)
        {
            p_link->tos0_metric = (ospf_timer_active(&p_if->hold_cost_timer) == TRUE) ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        }
        else
        {
            p_link->tos0_metric =  p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        }
        /*end*/
        //p_link->tos0_metric = p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);

        len = ospf_router_link_len(p_link);

        p_link = (struct ospf_router_link *)((u_long)p_link + len);
    }
                    
    /* section 12.4.1.1, second bullet item (page 120) */
    /*we only support option 2 */
    p_link->id = htonl(p_if->addr);
    p_link->data = htonl(OSPF_HOST_MASK);
    p_link->type = OSPF_RTRLINK_STUB;
    //p_link->tos0_metric =  p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : 0;
    p_link->tos0_metric =  0;

    len += ospf_router_link_len(p_link);    
    return len;
}

/*insert one interface with ppp type into router lsa*/
uint32_t 
ospf_router_lsa_add_ppp_link_fill(
                           struct ospf_router_link *p_link,u_short cost,u_int len)
{
	u_int uilen = len;
	u_short usTos = cost+1;
	int i;
    return len;

//	printf("%s %d  *****p_link: id 0x%x, data 0x%x, type %d,tos0_metric:%d,len:%d,cost:%d******\n", 
//	__FUNCTION__,__LINE__,p_link->id,p_link->data,p_link->type,p_link->tos0_metric,len,cost);
 	if((stOsLsa.ucEn != OSPF_LSA_MNG_EN)||(len == 0))
 	{
		return len;
 	}

	for(i = 0;i < stOsLsa.uiCnt;i++)
	{
		if((stOsLsa.stNet[i].uiIp == 0)||(stOsLsa.stNet[i].uiMask == 0))
		{
			continue;
		}
		 p_link = (struct ospf_router_link *)((u_long)p_link + uilen);                    

		p_link->id = htonl(stOsLsa.stNet[i].uiIp);
		p_link->data = htonl(stOsLsa.stNet[i].uiMask);
		p_link->type = OSPF_RTRLINK_STUB;
		p_link->tos0_metric = htons(usTos+i);

		uilen += ospf_router_link_len(p_link);

	//	printf("%s %d  *****p_link: id 0x%x, data 0x%x, type %d,tos0_metric:%d,len:%d,i=%d******\n", 
	//	__FUNCTION__,__LINE__,p_link->id,p_link->data,p_link->type,p_link->tos0_metric,uilen,i);
		ospf_logx(ospf_debug_lsa,"%d  *****p_link: id 0x%x, data 0x%x, type %d,tos0_metric:%d,len:%d,i=%d******\n",__LINE__,p_link->id,p_link->data,p_link->type,p_link->tos0_metric,uilen,i);
	}
	
    	return uilen;
}

#if 0
/*insert one interface with ppp type into router lsa*/
uint32_t 
ospf_router_lsa_unnumber_ppp_link_fill(
                           struct ospf_router_link *p_link,
                           struct ospf_if *p_if)
{
    struct ospf_nbr *p_nbr = NULL; 
    u_int len = 0;
    
    p_nbr = ospf_nbr_first(p_if);

    if ((NULL != p_nbr) && (OSPF_NS_FULL == p_nbr->state))
    {
        /* section 12.4.1.1, first bullet item (page 119-120) */
        p_link->id = htonl(p_nbr->id);
        p_link->data = htonl(p_if->ifnet_uint);
        p_link->type = OSPF_RTRLINK_PPP;
        p_link->tos0_metric =  p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        len = ospf_router_link_len(p_link);    

        p_link = (struct ospf_router_link *)((uint32_t)p_link + len);
    }
                    
    /* section 12.4.1.1, second bullet item (page 120) */
    /*we only support option 2 20061201*/
    p_link->id = htonl(p_if->p_process->router_id);
    p_link->data = htonl(0xffffffff);
    p_link->type = OSPF_RTRLINK_STUB;
    p_link->tos_count = 0;
    p_link->tos0_metric = p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);

    len += ospf_router_link_len(p_link);
    return len;
}
#else
/*insert one interface with ppp type into router lsa*/
uint32_t 
ospf_router_lsa_unnumber_ppp_link_fill(
                           struct ospf_router_link *p_link,
                           struct ospf_if *p_if)
{
	struct ospf_nbr *p_nbr = NULL; 
	u_int len = 0;

	p_nbr = ospf_nbr_first(p_if);

	if ((NULL != p_nbr) && (OSPF_NS_FULL == p_nbr->state))
	{
	    /* section 12.4.1.1, first bullet item (page 119-120) */
	    p_link->id = htonl(p_nbr->id);
	    p_link->data = htonl(p_if->ifnet_uint);
	    p_link->type = OSPF_RTRLINK_PPP;
	    /*ldp sync*/
        if (p_if->ucLdpSyncEn == TRUE)
        {
            p_link->tos0_metric = (ospf_timer_active(&p_if->hold_cost_timer) == TRUE) ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        }
        else
        {
            p_link->tos0_metric =  p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        }
        /*end*/
	    //p_link->tos0_metric =  p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
		ospf_logx(ospf_debug_lsa," %d  *****p_link: id 0x%x, data 0x%x, type %d,tos0_metric:%d******\n",__LINE__,p_link->id,p_link->data,p_link->type,p_link->tos0_metric);
 //       printf("%s %d  *****p_link: id 0x%x, data 0x%x, type %d,tos0_metric:%d******\n", 
//			__FUNCTION__,__LINE__,p_nbr->id,p_if->ifnet_uint,p_link->type,p_link->tos0_metric);
	    len = ospf_router_link_len(p_link);    
	}     
//	return ospf_router_lsa_add_ppp_link_fill(p_link,htons(p_link->tos0_metric),len);

    return len;
}
#endif
/*insert one interface with ppp type into router lsa*/
uint32_t 
ospf_router_lsa_ppp_link_fill(
                           struct ospf_router_link *p_link,
                           struct ospf_if *p_if)
{
    struct ospf_nbr *p_nbr = NULL; 
    u_int len = 0;
    u_int active = p_if->hold_cost_timer.active;
    
    p_nbr = ospf_nbr_first(p_if);

    if ((NULL != p_nbr) && (OSPF_NS_FULL == p_nbr->state))
    {
        /* section 12.4.1.1, first bullet item (page 119-120) */
        p_link->id = htonl(p_nbr->id);
        p_link->data = htonl(p_if->addr);
        p_link->type = OSPF_RTRLINK_PPP;
        /*ldp sync*/
        if (p_if->ucLdpSyncEn == TRUE)
        {
            p_link->tos0_metric = (active == TRUE) ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        }
        else
        {
            p_link->tos0_metric =  p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        }

        len = ospf_router_link_len(p_link);
//	ospf_logx(ospf_debug_lsa,"%s %d  *****p_link: id 0x%x, data 0x%x, type %d,tos0_metric:%d******\n", 
//		__FUNCTION__,__LINE__,p_link->id,p_link->data,p_link->type,p_link->tos0_metric);

        p_link = (struct ospf_router_link *)((u_long)p_link + len);
    }
                    
    /* section 12.4.1.1, second bullet item (page 120) */
    /*we only support option 2 20061201*/
    p_link->id = htonl(p_if->addr & p_if->mask);
    p_link->data = htonl(p_if->mask);
    p_link->type = OSPF_RTRLINK_STUB;
    p_link->tos_count = 0;
    //p_link->tos0_metric = p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
    p_link->tos0_metric = htons(p_if->cost[0]);
    /*end*/
//	ospf_logx(ospf_debug_lsa,"%s %d  *****p_link: id 0x%x, data 0x%x, type %d,tos0_metric:%d******\n", 
//		__FUNCTION__,__LINE__,p_link->id,p_link->data,p_link->type,p_link->tos0_metric);

    len += ospf_router_link_len(p_link);
    return len;
}



u_int 
ospf_router_lsa_sham_link_fill(
                           struct ospf_router_link *p_link,
                           struct ospf_if *p_if)
{
    struct ospf_nbr *p_nbr = NULL; 
    u_int len = 0;
    
    p_nbr = ospf_nbr_first(p_if);

    if ((NULL != p_nbr) && (OSPF_NS_FULL == p_nbr->state))
    {
        /* section 12.4.1.1, first bullet item (page 119-120) */
        p_link->id = htonl(p_nbr->id);
        p_link->data = htonl(p_if->addr);
        p_link->type = OSPF_RTRLINK_PPP;
        /*ldp sync*/
        if (p_if->ucLdpSyncEn == TRUE)
        {
            p_link->tos0_metric = (ospf_timer_active(&p_if->hold_cost_timer) == TRUE) ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        }
        else
        {
            p_link->tos0_metric =  p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        }
        /*end*/
        //p_link->tos0_metric =  p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);

        len = ospf_router_link_len(p_link);
    }
    return len;
}

/*insert te tunnel interface with ppp type into router lsa*/
u_int 
ospf_router_lsa_dcn_fill(
                           struct ospf_router_link *p_link,
                           struct ospf_area *p_area)
{
	u_short cost = 0;
	struct ospf_process *p_process = p_area->p_process;
    u_int len = 0;
    
    memset(p_link, 0, sizeof(struct ospf_router_link));
    p_link->id = htonl(p_process->router_id);
    p_link->data = htonl(0xffffffff);
    p_link->type = OSPF_RTRLINK_STUB;
    p_link->tos_count = 0;
    //p_link->tos0_metric = p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(cost);
    p_link->tos0_metric = htons(cost);
    len += ospf_router_link_len(p_link);
    return len;
}

/*insert te tunnel interface with ppp type into router lsa*/
u_int 
ospf_router_lsa_te_tunnel_fill(
                           struct ospf_router_link *p_link,
                           struct ospf_area *p_area)
{
    struct ospf_te_tunnel *p_te_tunnel = NULL;
    struct ospf_te_tunnel *p_next_te_tunnel = NULL;
    struct ospf_process *p_process = p_area->p_process;
    u_int len = 0;
    
    memset(p_link, 0, sizeof(struct ospf_router_link));
    for_each_node(&p_process->te_tunnel_table, p_te_tunnel, p_next_te_tunnel)
    {
        /*te tunnel fa ,shortcut ability be TRUE,and te tunnel is for this area*/
        if ((p_area->id == p_te_tunnel->area)
            && (TRUE == p_te_tunnel->fa)
            && (TRUE == p_te_tunnel->active))
        {
            p_link->id = htonl(p_te_tunnel->addr_out);
            p_link->data = htonl(p_te_tunnel->addr_in);
            p_link->type = OSPF_RTRLINK_PPP;
            p_link->tos0_metric =  htons(p_te_tunnel->cost);
            len = ospf_router_link_len(p_link); 
        }
    } 
    return len;
}

/*add one broadcast or nbma interface to router lsa*/
u_int 
ospf_router_lsa_bcast_link_fill(
                              struct ospf_router_link *p_link,
                              struct ospf_if *p_if)
{
    u_int8 state_mask[4] = {0};
    u_int active = p_if->hold_cost_timer.active;

    /*set state mask,check full or restart nbr*/
    BIT_LST_SET(state_mask, OSPF_NS_FULL);
    BIT_LST_SET(state_mask, OSPF_NS_RESTART);

    /*at least one full neighbor,or any neighbor in restart*/
    if (ospf_if_nbr_count_in_state(p_if, state_mask))
    {
        /* section 12.4.1.2, second bullet item, except for last sentence (page 120) */
        p_link->id = htonl(p_if->dr);
        p_link->data = htonl(p_if->addr);
        p_link->type = OSPF_RTRLINK_TRANSIT;
        if (p_if->ucLdpSyncEn == TRUE)
        {
            p_link->tos0_metric = (active == TRUE) ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        }
        else
        {
            p_link->tos0_metric =  p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        }
        //p_link->tos0_metric =  p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
    }
    else
    {
       /* section 12.4.1.2, first bullet item and last sentence 
          of second bullet item (page 120-121) */
        p_link->id = htonl(p_if->addr & p_if->mask);
        p_link->data = htonl(p_if->mask);
        p_link->type = OSPF_RTRLINK_STUB;
        //p_link->tos0_metric = p_if->p_process->stub_adv ? OSPF_STUB_ROUTER_DEFAULT_COST : htons(p_if->cost[0]);
        p_link->tos0_metric = htons(p_if->cost[0]);
        /*end*/
    }
    return ospf_router_link_len(p_link);
}

/*add one virtual link interface to router lsa*/
u_int 
ospf_router_lsa_virtual_link_fill(
                  struct ospf_router_link *p_link,
                  struct ospf_if *p_if)
{
    struct ospf_nbr *p_nbr = NULL; 
    u_int len = 0;
    /*vlink must have full nbr*/
    p_nbr = ospf_nbr_first(p_if);
    if ((NULL != p_nbr ) && (OSPF_NS_FULL == p_nbr->state))
    {
        p_link->id = htonl(p_nbr->id);
        p_link->data = htonl(p_if->addr);
        p_link->type = OSPF_RTRLINK_VLINK;
        p_link->tos0_metric = htons(p_if->cost[0]);

        len = ospf_router_link_len(p_link);
    }
    return len;                   
}

/*check and insert one interface to router lsa*/
u_int
ospf_router_lsa_link_fill(
                 struct ospf_router_link *p_link,
                 struct ospf_if *p_if)
{
    memset(p_link, 0, sizeof(struct ospf_router_link));
    
  /* section 12.4.1, second bullet item (page 119) */
    if (OSPF_IFS_DOWN == p_if->state)
    {
        return 0;
    }

    /* section 12.4.1, third bullet item (page 119) */
    if (OSPF_IFS_LOOPBACK == p_if->state)
    {              
        return ospf_router_lsa_loop_link_fill(p_link, p_if);         
    }

    /* section 12.4.1, fourth bullet item (page 119) */
    switch (p_if->type){
        case OSPF_IFT_PPP: /* section 12.4.1.1 (page 119-120) */
         #ifdef OSPF_DCN
             if (OSPF_DCN_FLAG != p_if->ulDcnflag)
             {
                 return ospf_router_lsa_ppp_link_fill(p_link, p_if);
             }
             else
             {
                 return ospf_router_lsa_unnumber_ppp_link_fill(p_link, p_if);
             }
         #else
             return ospf_router_lsa_ppp_link_fill(p_link, p_if);
         #endif
             break;
             
        case OSPF_IFT_BCAST: /* section 12.4.1.2 (page 120-121) */
        case OSPF_IFT_NBMA:
             return ospf_router_lsa_bcast_link_fill(p_link, p_if);
             break;
   
        case OSPF_IFT_VLINK:  /* section 12.4.1.3 (page 121) */
             return ospf_router_lsa_virtual_link_fill(p_link, p_if);
             break;
            
        case OSPF_IFT_P2MP: /* section 12.4.1.4 (page 121) */
             return ospf_router_lsa_p2mp_link_fill(p_link, p_if);
             break;
             
        case OSPF_IFT_SHAMLINK: 
             return ospf_router_lsa_sham_link_fill(p_link, p_if);
             break;
             
        default:
             break;
    }
    return 0;
}

/*NETWORK LSA */

#define ospf_network_lsa_link_fill(x,y,z) do{\
    *(x) = htonl(y);\
    (x)++;(z) += OSPF_NETWORK_LINK_LEN;\
}while(0)

/* section 12.4.2 (page 124) */
void 
ospf_network_lsa_build(
           struct ospf_if *p_if,
           struct ospf_network_lsa *p_network)
{
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next = NULL;    
    struct ospf_process *p_process = p_if->p_process;
    u_int *p_router = NULL;
    u_int count = 0;
    
  	//printf("#%s:%d\r\n",__FUNCTION__,__LINE__);
   ospf_init_lshdr(&p_network->h, OSPF_LS_NETWORK, p_if->addr, p_process->router_id);
    ospf_lsa_option_build(p_if->p_area, &p_network->h);

    p_network->mask = htonl(p_if->mask);
    
    p_router = p_network->router;
    
    p_network->h.len = OSPF_NETWORK_LSA_HLEN;

    /*insert self as nbr*/
    ospf_network_lsa_link_fill(p_router, p_process->router_id, p_network->h.len);

    /*insert all nbr in FULL state or restarting state*/
    for_each_ospf_nbr(p_if, p_nbr, p_next)
    {
        if ((OSPF_NS_FULL == p_nbr->state) || p_nbr->in_restart)
        {
            count++;
            ospf_network_lsa_link_fill(p_router, p_nbr->id, p_network->h.len);              
        }
    }
    p_network->h.len = htons(p_network->h.len);
    
    /*if no full neighbor exist,or self is not dr,flush network lsa*/
    if ((0 == count) || (OSPF_IFS_DR != p_if->state))
    {
        p_network->h.age = htons(OSPF_MAX_LSAGE);
        ospf_stimer_safe_start(&p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);
    }
    return;
}

/* section 12.4.2 (page 124) */
void 
ospf_network_lsa_originate(struct ospf_if *p_if)
{
    u_int8 buf[OSPF_MAX_TXBUF];
    struct ospf_network_lsa *p_network = NULL;
    
    if ((NULL == p_if) || (NULL == p_if->p_area))
    {
        return;
    }
    
    ospf_logx(ospf_debug_lsa, "originate network lsa for %s", p_if->name); 
    
    p_network = (struct ospf_network_lsa *)buf;
    memset(p_network, 0, sizeof(struct ospf_network_lsa));
    ospf_network_lsa_build(p_if, p_network);
    
    ospf_local_lsa_install(p_if->p_area->ls_table[p_network->h.type], &p_network->h);
    return;
}
#undef ospf_network_lsa_link_fill

/*SUMMARY LSA*/
/* section 12.4.3 (page 125-128) */
void
ospf_summary_lsa_originate_for_area(
                  struct ospf_area *p_area, 
                  u_int need_flush)
{
    struct ospf_route *p_route = NULL;
    struct ospf_route *p_nextroute = NULL;
    struct ospf_iproute *p_iproute = NULL;
    struct ospf_iproute *p_nextiproute = NULL;
    struct ospf_area *p_area_src = NULL;
    struct ospf_area *p_area_nxtsrc = NULL;
    struct ospf_process *p_process = p_area->p_process;
    u_int current = p_process->current_route;

    ospf_logx(ospf_debug_lsa, "originate summary lsa for area %u", p_area->id);  
    
    /*originate default type 3 lsa for stub area and nssa no_summary area*/
    if (p_area->is_stub || (p_area->is_nssa && p_area->nosummary))
    {
        ospf_originate_summary_default_lsa(p_area, need_flush);
    }
    
    /*if area is stub total,do not originate summary lsa for the ara*/    
    if (p_area->is_stub && p_area->nosummary && (!need_flush))
    {
        ospf_logx(ospf_debug_lsa, "total stub area do not originate summary lsa");
        return;
    }

    /*check all summary network routes*/
    for_each_node(&p_process->route_table, p_route, p_nextroute)
    {                  
        /*try to originate summary lsa*/
        if ((OSPF_PATH_INTRA == p_route->path[current].type)
            || (OSPF_PATH_INTER == p_route->path[current].type))
        {
            ospf_summary_lsa_originate(p_route, p_area, need_flush);
        }
    }
    if (ospf_is_vpn_process(p_process)
        && (BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_bgp)))
    {
        /*check all redistribute route*/
        for_each_node(&p_process->import_table, p_iproute, p_nextiproute)
        {
            if (p_iproute->vpn_internal || need_flush)
            {
                ospf_vpn_summary_lsa_originate(p_iproute, p_area, need_flush);
            }
        }
    }

    /*check all asbr route*/
    for_each_ospf_area(p_process, p_area_src, p_area_nxtsrc)
    {
        if (p_area_src == p_area)
        {
            continue;
        }
        for_each_node(&p_area_src->asbr_table, p_route, p_nextroute)
        {                  
            ospf_summary_lsa_originate(p_route, p_area, need_flush);
        }
    }
    return;
}

void 
ospf_summary_lsa_build(
             struct ospf_area *p_area,
             struct ospf_route *p_route,
             struct ospf_summary_lsa *p_summary,
             u_int need_flush)
{
    struct ospf_process *p_process = p_area->p_process;
    u_int lstype = (OSPF_ROUTE_ASBR == p_route->type) ? OSPF_LS_SUMMARY_ASBR : OSPF_LS_SUMMARY_NETWORK;
    u_int current = p_process->current_route;

    ospf_init_lshdr(&p_summary->h, lstype, p_route->dest, p_process->router_id);
    ospf_lsa_option_build(p_area, &p_summary->h);

    p_summary->mask = (OSPF_LS_SUMMARY_NETWORK == lstype) ? htonl(p_route->mask) : 0;
    p_summary->metric = htonl(p_route->path[current].cost);
    p_summary->h.len = htons(OSPF_SUMMARY_LSA_HLEN);
    
    /*if need delete,set maxage*/    
    if (need_flush)
    {
        p_summary->h.age = htons(OSPF_MAX_LSAGE);
        ospf_stimer_safe_start(&p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);
    }
    return;
}

u_int 
ospf_summary_lsa_originate_verify(      
                             struct ospf_route *p_route,
                             struct ospf_area *p_area)
{
    struct ospf_range *p_range = NULL;
    struct ospf_process *p_process = p_area->p_process;    
    struct ospf_path *p_path = NULL;
    u_int current = p_process->current_route;
        
    if ((NULL == p_area) ||( NULL == p_route))
    {
        return FALSE;
    }

    p_path = &p_route->path[current];

    /*type and area,backbone can only accept intra*/
    if ((p_process->p_backbone == p_area) && (OSPF_PATH_INTRA != p_path->type))
    {
        return FALSE;
    }
     
    /*special process for nssa*/
    if (p_area->is_nssa)
    {
        /*if do not import summary lsa,ignore non-default route*/     
        if ( (OSPF_ROUTE_ASBR == p_route->type) 
            || (p_area->nosummary && p_route->dest))
        {
            return FALSE;
        }
    }

    /*vif checking ,TBI*/       

    /*validate route type*//* section 12.4.3, first bullet item (page 125) */
    if (OSPF_ROUTE_ABR == p_route->type)
    {
        ospf_logx(ospf_debug_lsa, "route type is not ASBR or Network,stop");
        return FALSE;
    }

    /* section 12.4.3, second bullet item (page 126) */
    if ((OSPF_PATH_ASE == p_path->type)
        || (OSPF_PATH_ASE2 == p_path->type))
    {        
        ospf_logx(ospf_debug_lsa, "route path type is external,stop");
        return FALSE;
    }

    /* section 12.4.3, third bullet item (page 126) */
    if (p_path->p_area == p_area)
    {        
        ospf_logx(ospf_debug_lsa, "route is formed from this area,stop");
        return FALSE;
    }

    /* section 12.4.3, forth bullet item (page 126) */
    if (NULL == p_path->p_nexthop)
    {
        ospf_logx(ospf_debug_lsa, "route nexthop is NULL,stop");
        return FALSE;
    }

    /*route in backbone may use this area as transit area,do not originate summary into it*/
    if (p_process->vlink_configured && (p_path->p_area == p_process->p_backbone))
    {
        if (TRUE == ospf_nexhop_in_the_area (p_path->p_nexthop, p_area))
        {
            ospf_logx(ospf_debug_lsa, "route in backbone may use this area as transit area,stop");
            return FALSE;
        }
    }
     
    /* section 12.4.3, fifth bullet item (page 126) */
    if (ospf_invalid_metric(p_path->cost))
    {
        ospf_logx(ospf_debug_lsa, "route cost is invalid,stop");
        return FALSE;
    }
    
    if (OSPF_ROUTE_ASBR == p_route->type)
    {
        if ((!p_area->is_stub) && (!p_area->is_nssa))
        {
            /* section 12.4.3, sixth bullet item (page 126) */
            return TRUE;
        }        
        ospf_logx(ospf_debug_lsa, "route type is asbr,stop");
        return FALSE;
    }

    /* section 12.4.3, seventh bullet item (page 126) */        
    if (OSPF_PATH_INTER == p_path->type)
    {
        return TRUE;
    }    

    /*if source area is backbone,dest area is transit area,do not check range*/
    if ((NULL != p_path->p_area) 
         && (p_process->p_backbone == p_path->p_area)
         && p_area->transit)
    {       
        return TRUE;
    }
        /* RFC 2178 G.7 */
    /*find the address range entry*/
    p_range = ospf_range_match(p_path->p_area, 
                    OSPF_LS_SUMMARY_NETWORK, 
                    p_route->dest, p_route->mask, NULL);
    if ((NULL != p_range) 
        && (! p_range->isdown)
        && (!((p_range->network == p_route->dest) && (p_range->mask == p_route->mask))))
    {       
        ospf_logx(ospf_debug_lsa, "exit range,return "); 
        return FALSE;
    }
    return TRUE;
}

void 
ospf_summary_lsa_originate (
                       struct ospf_route *p_route,
                       struct ospf_area *p_area,
                       u_int need_flush)
{
    struct ospf_summary_lsa summary;
    u_int8 deststr[20];
    if (ospf_debug)
    {
    	ospf_inet_ntoa(deststr, p_route->dest);
        ospf_logx(ospf_debug_lsa, "generate summary lsa %s,area %u,flag %d",
        deststr, p_area->id, need_flush);
    }

    /*do not prevent flush,but check strictly for add*/
    if ((!need_flush) && (FALSE == ospf_summary_lsa_originate_verify(p_route, p_area)))
    {
       
        need_flush = TRUE;
        /*return;*/
    }

    memset(&summary, 0, sizeof(summary));
    ospf_summary_lsa_build(p_area, p_route, &summary, need_flush);
    ospf_local_lsa_install(p_area->ls_table[summary.h.type], &summary.h);
    return;
}

void 
ospf_vpn_summary_lsa_build(
             struct ospf_area *p_area,
             struct ospf_iproute *p_route,
             struct ospf_summary_lsa *p_summary,
             u_int need_flush)
{
    struct ospf_process *p_process = p_area->p_process;

    ospf_init_lshdr(&p_summary->h, OSPF_LS_SUMMARY_NETWORK, p_route->dest, p_process->router_id);
    ospf_lsa_option_build(p_area, &p_summary->h);

    /*for PE,should add dn bit for type3,5,7*/         
    ospf_set_option_dn(p_summary->h.option);    

    p_summary->mask = htonl(p_route->mask);
    p_summary->metric = p_route->metric & 0x00FFFFFF;
    p_summary->metric = htonl(p_summary->metric);
    p_summary->h.len = htons(OSPF_SUMMARY_LSA_HLEN);
    
    /*if need delete,set maxage*/    
    if (need_flush)
    {
        p_summary->h.age = htons(OSPF_MAX_LSAGE);
        ospf_stimer_safe_start(&p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);
    }
    return;
}

/*need_flush used to delete lsa, p_route->acive is not used*/
void 
ospf_vpn_summary_lsa_originate (
                       struct ospf_iproute *p_route,
                       struct ospf_area *p_area,
                       u_int need_flush)
{
    struct ospf_summary_lsa summary;
    u_int8 deststr[20];
    if (ospf_debug)
    {
    	ospf_inet_ntoa(deststr, p_route->dest);
     //   ospf_logx(1, "generate vpn summary lsa %s,area %u,flush %d",
     //   deststr, p_area->id, need_flush);
    }

    memset(&summary, 0, sizeof(summary));
    ospf_vpn_summary_lsa_build(p_area, p_route, &summary, need_flush);
    ospf_local_lsa_install(p_area->ls_table[summary.h.type], &summary.h);
    return;
}
void
ospf_vpn_summary_lsa_originate_all (
                       struct ospf_process *p_process,
                       struct ospf_iproute *p_route,                       
                       u_int need_flush) 
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;

    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        if ((!p_area->nosummary) || need_flush) 
        {
            ospf_vpn_summary_lsa_originate(p_route, p_area, need_flush);
        }
    }
    return;
}
/*External LSA*/

/*when originate an external lsa,try to decide forwarding address*/
u_int 
ospf_forward_addr_get(
                    struct ospf_iproute *p_route, 
                    struct ospf_area *p_area)
{
    struct ospf_process *p_process = p_route->p_process;
    struct ospf_if *p_if = NULL;   
    struct ospf_if *p_next_if = NULL;
    u_int fwd_addr = p_route->fwdaddr;

    /*if no fwdaddr exist,do not check*/
    if (0 == fwd_addr)
    {
        return 0;
    }
    
    /*we set non-zero forwarding address only when nexthop 
    is on one up OSPF interface*/
    
    /*get matched interface according to fwdaddr*/
    p_if = ospf_if_lookup_by_network(p_process, fwd_addr);
    /*interface exist,and state OK,area matched,got it*/
    if (p_if 
       && (OSPF_IFS_DOWN != p_if->state)
       && ((NULL == p_area) || (p_area == p_if->p_area)))
    {
        return fwd_addr;
    }
    /*t5 lsa ,default use 0*/ 
    if (NULL == p_area)
    {
        return 0;
    }
    /*if interface not valid,get the first up interface*/
    for_each_node(&p_area->if_table, p_if, p_next_if)
    {
        if (OSPF_IFS_DOWN != p_if->state)
        {
            return p_if->addr;
        }  
    }   
    return 0;
}

/* section 12.4.4 (page 129-130) */
void 
ospf_external_lsa_build(
            struct ospf_iproute *p_route,
            struct ospf_external_lsa *p_external)
{
    struct ospf_process *p_process = p_route->p_process;
    
    ospf_init_lshdr(&p_external->h, OSPF_LS_AS_EXTERNAL, p_route->dest, p_process->router_id);
    ospf_lsa_option_build(NULL, &p_external->h);
    
    p_external->tag = htonl(p_route->tag);

    /*for PE,should add dn bit for type3,5,7*/
    if (ospf_is_vpn_process(p_process)
        && p_route->vpn_route)
    {        
        ospf_set_option_dn(p_external->h.option);         
    }
                
    if (p_process->route_tag)
    {
        p_external->tag = htonl(p_process->route_tag);
    }

    p_external->h.len = htons(OSPF_ASE_LSA_HLEN);    
    p_external->mask = htonl(p_route->mask);   
    p_external->metric = htonl(p_route->metric);


    /*set maxage for deleted route*/
    if (!p_route->active)
    {
        p_external->h.age = htons(OSPF_MAX_LSAGE);
        ospf_stimer_safe_start(&p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);
    }
    else
    {
        p_route->p_process = p_process;
        /*nssa lsa translate to type5 lsa,not change fwd addr*/
        if (FALSE == p_route->nssa_route)
        {
            p_route->fwdaddr = ospf_forward_addr_get(p_route, NULL);
        }
        p_external->fwdaddr = htonl(p_route->fwdaddr); 
    }
    return;
}

/* section 12.4.4 (page 129-130) */
void 
ospf_external_lsa_originate(struct ospf_iproute *p_route)
{
    struct ospf_process *p_process = p_route->p_process;
    struct ospf_external_lsa  external_lsa;
    u_int8 s_rt[16];
    
    ospf_logx(ospf_debug_lsa, "originate type5 lsa for route %s", ospf_inet_ntoa(s_rt, p_route->dest));
    
    memset(&external_lsa, 0, sizeof(external_lsa));

    ospf_external_lsa_build(p_route, &external_lsa);
    
    ospf_logx(ospf_debug_lsa, "originate type5 lsa for route dest %x adv_id %x",p_route->dest,external_lsa.h.adv_id);
    ospf_local_lsa_install(&p_process->t5_lstable, &external_lsa.h);
    return;
}

void 
ospf_nssa_lsa_build ( 
                  struct ospf_area *p_area ,
                  struct ospf_iproute *p_route,
                  struct ospf_nssa_lsa *p_nssa)
{
    struct ospf_process *p_process = p_area->p_process;
    u_int pbit = FALSE;
    u_int fa = 0;

    ospf_init_lshdr(&p_nssa->h, OSPF_LS_TYPE_7, p_route->dest, p_process->router_id);

    p_nssa->h.len = htons(OSPF_ASE_LSA_HLEN);
    p_nssa->h.option = 0;
    
    /*decide translate bit:
      abr,not use translate bit,else obtain translate bit from
      redistribute control
    */

    #if 0
    pbit = p_process->abr ? FALSE : !(p_route->no_translate);
	#else
	
    //printf("11 ospf_nssa_lsa_build p_process->abr:%d,asbr:%d,no_translate:%d\n",p_process->abr,p_process->asbr,p_route->no_translate);
	/*asbr且无骨干区域则置nssa标志位*/
    pbit = (p_process->asbr)&&(FALSE == ospf_backbone_nbr_exsit(p_process)) ?  !(p_route->no_translate) : FALSE;
  //  printf("23 ospf_nssa_lsa_build pbit:%d,dest=%x，fwdaddr=%x,if_unit=%x,p_process->router_id=%x,process_id=%d\n",
  //      pbit,p_route->dest,p_route->fwdaddr,p_route->if_unit,p_process->router_id,p_process->process_id);
    #endif
    if (pbit)
    {
        /*set pbit*/
        ospf_set_option_nssa(p_nssa->h.option);

        /*select forward address*/
        p_route->p_process = p_process;
        fa = ospf_forward_addr_get(p_route, p_area);
    //    printf("44 ospf_nssa_lsa_build fa:%x\n",fa);
    }
    p_nssa->tag = htonl(p_route->tag);
    /*for PE,should add dn bit for type3,5,7*/
    if (ospf_is_vpn_process(p_process)
        && p_route->vpn_route)
    {        
        ospf_set_option_dn(p_nssa->h.option); 

        if (p_process->route_tag)
        {
            p_nssa->tag = htonl(p_process->route_tag);
        }
    }

    p_nssa->fwdaddr = htonl(fa);  
    p_nssa->mask = htonl(p_route->mask); 
    p_nssa->metric = htonl(p_route->metric);
   
    /* check for deleted external route,or invalid faddr*/
    if (!p_route->active || (pbit && (0 != p_route->fwdaddr) && (0 == fa)))
    {
        p_nssa->h.age = htons(OSPF_MAX_LSAGE);
        ospf_stimer_safe_start(&p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);
    }
    return;
}

/*orignate nssa lsa into special area*/
void 
ospf_nssa_lsa_originate (
                 struct ospf_area *p_area ,
                 struct ospf_iproute *p_route) 
{
    struct ospf_nssa_lsa nssa_lsa;
    u_int8 s_rt[16];

    ospf_logx(ospf_debug_lsa, "originate nssa lsa for route %s", ospf_inet_ntoa(s_rt, p_route->dest));
    ospf_logx(ospf_debug_lsa, "ospf_nssa_lsa_originate dest=%x,fwdaddr=%x,mask=%x,active=%d\n",  
		p_route->dest,p_route->fwdaddr,p_route->mask,p_route->active);
	if((p_route->dest == 0)
		&&(p_route->fwdaddr == 0)
		&&(p_route->mask == 0))
	{
	
		if (FALSE == ospf_backbone_full_nbr_exsit(p_area->p_process))
		{
			//printf("ospf_nssa_lsa_originate not area 0,return\n");
			return;
		}
	}

    memset(&nssa_lsa, 0, sizeof(nssa_lsa));
    ospf_nssa_lsa_build(p_area, p_route, &nssa_lsa);   
    
    ospf_local_lsa_install(p_area->ls_table[nssa_lsa.h.type], &nssa_lsa.h);

	if(!ospf_asbr_area_check(p_area->p_process))
    {
		ospf_nssa_external_del(p_area,p_route);
	}
		
    return;
}

/* section 3.4 RFC-1587 */
void 
ospf_nssa_lsa_originate_for_route(struct ospf_iproute *p_route)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next = NULL;
    
    /* originate nssa for each nssa area*/
    for_each_ospf_area(p_route->p_process, p_area, p_next)
    {
        if (p_area->is_nssa)
        {
            ospf_nssa_lsa_originate (p_area, p_route);
        }
    }
    return;
}

/*TE LSA*/
void 
ospf_router_te_lsa_build(
             struct ospf_area *p_area,
             struct ospf_opaque_lsa *p_lsa)
{
    struct ospf_te_routeraddr_tlv *p_tlv = NULL;
    u_int te_ls_id = 0;
    struct ospf_process *p_process = p_area->p_process;

    /*select instance*/
    if (!p_area->te_instance)
    {
    	/*参考华为,te_instance从0开始算*/
      //  p_area->te_instance = ospf_get_te_instance(p_area);
      
    }

    /*get te lsa id*/        
    te_ls_id = (0x01000000 | (p_area->te_instance & 0x00ffffff));
        
    ospf_init_lshdr(&p_lsa->h, OSPF_LS_TYPE_10, te_ls_id, p_process->router_id);
  
    /*length router address tlv:type 2 len 2 value 4*/
    p_lsa->h.len = htons(OSPF_LSA_HLEN + 8);
    
    /*router address tlv body*/ 
    p_tlv = (struct ospf_te_routeraddr_tlv *)p_lsa->data;    
    p_tlv->hdr.type = htons(OSPF_TE_TLV_RTR_ADDR);         
    p_tlv->hdr.len = htons(4);
    p_tlv->value = htonl(p_process->router_id);

    if (!p_area->te_enable)
    {
        p_lsa->h.age = htons(OSPF_MAX_LSAGE);
        ospf_stimer_safe_start(&p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);
    }
    return;
}

void 
ospf_router_te_lsa_originate(struct ospf_area *p_area)
{
    u_int8 lsa_buf[OSPF_MAX_TXBUF];
    struct ospf_opaque_lsa *p_lsa = (struct ospf_opaque_lsa *)lsa_buf;
    
    memset(p_lsa, 0, sizeof(struct ospf_opaque_lsa));
    ospf_router_te_lsa_build(p_area, p_lsa);
    
    ospf_local_lsa_install(p_area->ls_table[p_lsa->h.type], &p_lsa->h);
    return;
}

/*calculate length of body of te link te lsa.Do not include lsa header*/
u_short 
ospf_calculate_te_link_tlv_len(struct ospf_if *p_if)
{
    u_short length = 0;
    
    /*top tlv type and length,4 bytes*/
    length = 4;
    
    /*链路类型/1/1(补3字节)*/
    length += 8; 
    
    /*链路ID/2/4*/        
    length += 8;         
    
    /*本地接口IP地址/3/4×N N为本地地址的数目*/
    length += 8;         
    
    /*远端接口IP地址/4/4×N N为本地地址的数目*/        
    if (OSPF_IFT_BCAST != p_if->type)
    {
        length += 8; 
    }
	/* 参考华为多路可达,添加远端接口Ip*/
	else
	{
        length += 8; 
	}
    
    /*TE开销/5/4*/
    if (0 != p_if->te_cost)
    {
        length += 8; 
    }
    
    /*最大带宽/6/4*/        
    if (0 != p_if->max_bd)
    {
        length += 8; 
        
        /*最大可预留带宽/7/4*/
        length += 8; 
        
        /*未预留带宽/8/32*/
        length += 36;            
    }
    
    /*管理组/9/4*/        
    if (0 != p_if->te_group)
    {
        length += 8; 
    }
    return length;
}

/*build link tlv*/
void 
ospf_build_te_link_tlv(
             u_int8 *p_tlv,
             struct ospf_if *p_if,
             u_short len) 
{
    struct ospf_te_link_tlv *p_link_tlv = NULL;
    struct ospf_te_linktype_tlv *p_type = NULL;
    struct ospf_te_linkid_tlv *p_id = NULL;
    struct ospf_te_localaddr_tlv *p_local_ip = NULL;
    struct ospf_te_color_tlv *p_group = NULL;
    struct ospf_te_maxbd_tlv *p_max_bind = NULL;
    struct ospf_te_unrsvddb_tlv *p_unrsvd_bind = NULL;
    struct ospf_te_maxrsvddb_tlv *p_max_rsvd = NULL;
    struct ospf_te_metric_tlv *p_cost = NULL;
    struct ospf_te_remoteaddr_tlv *p_remote_ip = NULL;
    struct ospf_nbr *p_nbr = NULL;
    u_int link_id = 0;
    u_int i = 0;

    /*top tlv length and type*/
    p_link_tlv = (struct ospf_te_link_tlv *)p_tlv; 
    p_link_tlv->hdr.type = htons(OSPF_TE_TLV_LINK);

    /*do not include top tlv's type and length self*/
    p_link_tlv->hdr.len = htons(len - 4);

	
    p_tlv += 4;
    /*链路类型/1/1(补3字节)*/
    p_type = (struct ospf_te_linktype_tlv *)p_tlv; 
    p_type->hdr.type = htons(OSPF_TE_SUB_LINK_TYPE);
    p_type->hdr.len = htons(1);
    if (OSPF_IFT_PPP == p_if->type)
    {
        p_type->type = OSPF_RTRLINK_PPP; 
    }
    else
    {
        p_type->type = OSPF_RTRLINK_TRANSIT;
    }
    
    p_tlv += 8;        
    /*链路ID/2/4*/
    p_id = (struct ospf_te_linkid_tlv *)p_tlv;
    p_id->hdr.type = htons(OSPF_TE_SUB_LINK_ID);
    p_id->hdr.len = htons(4);
    
    if (OSPF_IFT_PPP == p_if->type)
    {
        p_nbr = ospf_nbr_first(p_if);
        if (NULL != p_nbr)
        {
            link_id = p_nbr->id;
        }
    }
    else
    {
        link_id = p_if->dr;
    }
    p_id->value = htonl(link_id);
    
    p_tlv += 8;  

	/*待修改为mpls te ip*/
    /*本地接口IP地址/3/4×N N为本地地址的数目。*/
    p_local_ip = (struct ospf_te_localaddr_tlv *)p_tlv;
    p_local_ip->hdr.type = htons(OSPF_TE_SUB_LOCAL_ADDRESS);
    p_local_ip->hdr.len = htons(4);    
    p_local_ip->value[0] = htonl(p_if->addr);
    
    p_tlv += 8;
    /*远端接口IP地址/4/4×N N为本地地址的数目。*/
   	if (OSPF_IFT_BCAST != p_if->type)
    {
        p_remote_ip = (struct ospf_te_remoteaddr_tlv *)p_tlv;
        
        p_remote_ip->hdr.type = htons(OSPF_TE_SUB_REMOTE_ADDRESS);
        p_remote_ip->hdr.len = htons(4);

        p_nbr = ospf_nbr_first(p_if);

        p_remote_ip->value[0] = (NULL != p_nbr) ? htonl(p_nbr->addr) : 0;

        p_tlv += 8;          
    } 
	else	/*参照华为修改广播时远端ip为0*/
    {
    
		p_remote_ip = (struct ospf_te_remoteaddr_tlv *)p_tlv;
		p_remote_ip->hdr.type = htons(OSPF_TE_SUB_REMOTE_ADDRESS);
		p_remote_ip->hdr.len = htons(4);
		p_remote_ip->value[0] = 0;
		
        p_tlv += 8;          
    }
    /*TE开销/5/4*/
    if (0 != p_if->te_cost)
    {
        p_cost = (struct ospf_te_metric_tlv *)p_tlv;        
        p_cost->hdr.type = htons(OSPF_TE_SUB_METRIC);
        p_cost->hdr.len = htons(4);        
        p_cost->value = htonl(p_if->te_cost);
        
        p_tlv += 8;                          
    }
    
    /*最大带宽/6/4*/
    /*最大可预留带宽/7/4*/
    /*未预留带宽/8/32*/
    if (0 != p_if->max_bd)
    {
        p_max_bind = (struct ospf_te_maxbd_tlv *)p_tlv;
        
        p_max_bind->hdr.type = htons(OSPF_TE_SUB_MAX_BANDWIDTH);
        p_max_bind->hdr.len = htons(4);        
        p_max_bind->value = htonl(p_if->max_bd);
        
        p_tlv += 8;                 
        
        p_max_rsvd = (struct ospf_te_maxrsvddb_tlv *)p_tlv;
        p_max_rsvd->hdr.type = htons(OSPF_TE_SUB_MAX_RSVD_BANDWIDTH);
        p_max_rsvd->hdr.len = htons(4);
        p_max_rsvd->value = (0 != p_if->max_rsvdbd) ? htonl(p_if->max_rsvdbd) : htonl(p_if->max_bd);

        p_tlv += 8;                 
        
        p_unrsvd_bind = (struct ospf_te_unrsvddb_tlv *)p_tlv;
        p_unrsvd_bind->hdr.type = htons(OSPF_TE_SUB_UNRSVD_BANDWIDTH);
        p_unrsvd_bind->hdr.len = htons(32);
        
        for (i = 0; i < 8; i++)
        {
            p_unrsvd_bind->value[i] = (0 == p_if->unrsvdbd[i]) ? htonl(p_if->max_bd) : htonl(p_if->unrsvdbd[i]);
        }       
        p_tlv += 36;                 
    }
    
    /*管 理组        9        4*/
    if (0 != p_if->te_group)
    {
        p_group = (struct ospf_te_color_tlv *)p_tlv;
        
        p_group->hdr.type = htons(OSPF_TE_SUB_RESOURCE_CLASS_COLOR);
        p_group->hdr.len = htons(4);
        
        p_group->value = htonl(p_if->te_group);
        
        p_tlv += 8;                          
    }
    return;
}

void 
ospf_link_te_lsa_build(
            struct ospf_if *p_if,
            struct ospf_opaque_lsa *p_lsa)
{
    struct ospf_process *p_process = p_if->p_process;
    u_int id = 0;
    
    /*select instance*/
    if (0 == p_if->te_instance)
    {
        p_if->te_instance = ospf_get_te_instance(p_if->p_area);
    }
    /*compute length of tlv*/
    id = (0x01000000 | (p_if->te_instance & 0x00ffffff));
    ospf_init_lshdr(&p_lsa->h, OSPF_LS_TYPE_10, id, p_process->router_id);
    p_lsa->h.len = ospf_calculate_te_link_tlv_len(p_if);

    ospf_build_te_link_tlv(p_lsa->data, p_if, p_lsa->h.len);

    /*length:router address tlv:type 2 len 2 value 4*/
    p_lsa->h.len += OSPF_LSA_HLEN;
    p_lsa->h.len = htons (p_lsa->h.len);

    if (!p_if->p_area->te_enable)
    {
        p_lsa->h.age = htons(OSPF_MAX_LSAGE);
    }        
    return;
}

/*originate link te lsa,it is more complex than router te lsa*/
void 
ospf_link_te_lsa_originate(struct ospf_if *p_if)
{
    u_int8 buf[OSPF_MAX_TXBUF];
    struct ospf_opaque_lsa *p_lsa = NULL;
    
    p_lsa = (struct ospf_opaque_lsa *)buf;
    memset(p_lsa, 0, sizeof(struct ospf_opaque_lsa));

    ospf_link_te_lsa_build(p_if, p_lsa);
    
    /*if disable,try to timeout current value*/
    if (p_if->te_enable)
    {
        ospf_local_lsa_install(p_if->p_area->ls_table[p_lsa->h.type], &p_lsa->h);
    }
    return;
}
#ifdef OSPF_DCN
void
ospf_dcn_lsa_originate(struct ospf_process *p_process,
                       u_int8 *p_buf,
                       u_int buf_len)
{
    struct ospf_opaque_lsa *p_lsa = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;
    u_int8 buf[OSPF_MAX_TXBUF] = {0};
    u_int pad_len;
	int i = 0;
	
    /* pad to 32-bit alignment*/    
    pad_len = ((buf_len+3)/4)*4;
    if (OSPF_MAX_TXBUF < (pad_len + OSPF_LSA_HLEN))
    {
        p_process->uiDcnLsaErrCnt++;
        OSPF_LOG_ERR("%d input len is too long\n", __LINE__);
        ospf_log(ospf_debug_error, "input len is too long");
        return;
    }
    p_lsa = (struct ospf_opaque_lsa *)buf;

    ospf_init_lshdr(&p_lsa->h, OSPF_LS_TYPE_10, OSPF_DCN_LSID, p_process->router_id);
    memcpy(p_lsa->data, p_buf, buf_len);

    p_lsa->h.len = htons(pad_len + OSPF_LSA_HLEN);

    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        ospf_local_lsa_install(p_area->ls_table[p_lsa->h.type], &p_lsa->h);
    }
    p_process->uiDcnLsaCnt++;
    return;
}
#endif
u_int 
ospf_get_te_instance(struct ospf_area *p_area)
{
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;    
    u_int id = p_area->te_instance;

    for_each_node(&p_area->if_table, p_if, p_next_if)
    {
	//	printf("ospf_get_te_instance:ifnet_uint=%x,te_enable:%d,state:%d,te_instance=%x,id=%x\n",p_if->ifnet_uint,p_if->te_enable,p_if->state,p_if->te_instance,id);

		if(p_if->te_enable != TRUE)
		{
			continue;
		}
		//if (p_if->te_instance > id)
		if ((p_if->te_instance > id)&&(p_if->state != OSPF_IFS_DOWN))
        {
            id = p_if->te_instance;
        }
    }
    return (id + 1);
}

/*lsa table and update operation*/

/*when received or originated lsa,we need check if routing table changed*/ 
void 
ospf_update_route_for_lsa(struct ospf_lsa *p_lsa)
{
    struct ospf_process *p_process = p_lsa->p_lstable->p_process;
    u_int type = p_lsa->lshdr->type;
    if (p_process->in_restart)
    {
        ospf_logx(ospf_debug_lsa, "no calculate route in restarting");   
        return;
    }

    ospf_logx(ospf_debug_lsa, "schedule route update for lsa");  

    switch (type) {
        case OSPF_LS_SUMMARY_ASBR:
        case OSPF_LS_SUMMARY_NETWORK:
        case OSPF_LS_AS_EXTERNAL:
        case OSPF_LS_TYPE_7:
             /*if lsa is originated by self,do nothing*/
             if (ntohl(p_lsa->lshdr->adv_id) == p_process->router_id)
             {
                 break;
             }
             /*schedule full calculation*/
             ospf_spf_request(p_process); 
             break;
             
        case OSPF_LS_ROUTER:
        case OSPF_LS_NETWORK:
             /*if current nbr exceed some limit,schedule calculation*/
             if (OSPF_MAX_EXCHANGE_NBR <= ospf_lstcnt(&p_process->nbr_table))
             {
                 ospf_spf_request(p_process); 
                 break;
             }
             /*fast protection*/
             /*if fast spf can be calculated,calculate directly*/
             if (++p_process->fast_spf_count < OSPF_FAST_SPF_LIMIT)
             {
                /*start fast spf limit timer if not running*/
                if (!ospf_timer_active(&p_process->fast_spf_timer))
                {
                    ospf_timer_start(&p_process->fast_spf_timer, OSPF_FAST_SPF_INTERVAL);
                }
                /*stop spf timer is running*/
                ospf_timer_stop(&p_process->spf_timer);
                /*calculate route directly*/
                ospf_route_calculate_full(p_process);
                break;
             }
             /*full request*/
             ospf_spf_request(p_process); 
             break;
             
        default:
             break;
    }
    return;
}

/* section 12.4.4 (page 129-130) */
void 
ospf_local_lsa_install(
                  struct ospf_lstable *p_table,
                  struct ospf_lshdr *p_lshdr)
{
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_process *p_process = p_table->p_process;
    struct ospf_area *p_area = p_table->p_area;
    u_short age = 0;
    u_int8 type = p_lshdr->type;
    u_int different = FALSE; 
    int seqnum = 0;

 //   ospf_logx(ospf_debug_lsa,"%s:%d,p_table %x,p_lshdr %x\n",__FUNCTION__,__LINE__,p_table,p_lshdr);
 //   ospf_ospf_lshdr_print(p_lshdr);
	
    /*increase statistics for try to install*/
    ospf.stat.self_lsa_build[p_lshdr->type]++; 

    /*self router id is not 0*/
    if (0 == p_process->router_id)
    {
        ospf_logx(ospf_debug_lsa, "local router id is 0,do not originate lsa");
  // 	 ospf_logx(ospf_debug_lsa,"%s:%d #return,p_table %x,p_lshdr %x\n",__FUNCTION__,__LINE__,p_table,p_lshdr);
        return;
    }

    /*if not gr lsa,do not install in restart*/
    if ((OSPF_LS_TYPE_9 != p_lshdr->type) && p_process->in_restart)
    {
        ospf_logx(ospf_debug_lsa|ospf_debug_gr, "router in restart,do not originate lsa"); 
//	ospf_logx(ospf_debug_lsa,"%s:%d #return,p_table %x,p_lshdr %x\n",__FUNCTION__,__LINE__,p_table,p_lshdr);
        return;
    }

    
    ospf_lsa_id_conflict_check(p_table, p_lshdr);
      
    /*search current one*/
    p_lsa = ospf_lsa_lookup(p_table, p_lshdr);
    if (OSPF_MAX_LSAGE == ntohs(p_lshdr->age))
    {
        /*purge request,if lsa exist,delete it*/
        if (NULL != p_lsa)
        {
            ospf_lsa_maxage_set(p_lsa);
        }
//	ospf_logx(ospf_debug,"%s:%d #return,p_table %x,p_lshdr %x\n",__FUNCTION__,__LINE__,p_table,p_lshdr);
        //return;
    }
    
    ospf_logx(ospf_debug_lsa, "install and flood local originated LSA");

    /*control summary lsa originate for stub area*/
    /*do not originate asbr summary lsa into stub area*/
    if ((NULL != p_area) && p_area->is_stub && p_area->nosummary)
    {
        if (((OSPF_LS_SUMMARY_NETWORK == type) 
            || (OSPF_LS_SUMMARY_ASBR == type)) 
            && (0 != p_lshdr->id))
        {
             /*totally stub area do not accept any no-default summary lsa*/            
            ospf_logx(ospf_debug_lsa, "ignore non-default summary lsa for stub area");

            if (NULL != p_lsa)
            {
                /*increase statistics for try to lsa changed*/
                ospf.stat.self_lsa_change[p_lshdr->type]++;

                ospf_lsa_maxage_set(p_lsa);
            }
	
//	ospf_logx(ospf_debug_lsa,"%s:%d #return,p_table %x,p_lshdr %x\n",__FUNCTION__,__LINE__,p_table,p_lshdr);
            return;
        }
    }  
    
    /*is lsa body changed?*/ 
    different = p_lsa ? ospf_lsa_changed(p_lshdr, p_lsa->lshdr) : TRUE;
    if (NULL == p_lsa)
    {
        ospf_logx(ospf_debug_lsa, "use init seqnum for new lsa");

        p_lshdr->seqnum = (int)(htonl(OSPF_INIT_LS_SEQNUM));   
    }
    else if (different) 
    {
        ospf_logx(ospf_debug_lsa, "lsa is changed from old instance");
        
        seqnum = ntohl(p_lsa->lshdr->seqnum);
        if (OSPF_MAX_LS_SEQNUM == seqnum)
        {
            ospf_logx(ospf_debug_lsa, "prepare delete maxseq lsa");

            /* section 12.1.6 (page 109-110) */
			/*不删除lsa，直接更新*/
			ospf_lsa_maxage_set(p_lsa);
            ospf_lsa_flood(p_lsa, NULL, NULL);

			#if 0
            ospf_lsa_delete(p_lsa);
            p_lsa = NULL;
			#else
			ospf_lsa_delete_for_install(p_lsa);
			p_lsa->ucFlag = TRUE;
			#endif
            p_lshdr->seqnum = htonl(((u_int)OSPF_INIT_LS_SEQNUM));
        }
        else
        {            
            seqnum = ntohl(p_lsa->lshdr->seqnum);
            p_lshdr->seqnum = ospf_lsa_next_seqnum(htonl(seqnum));
            
            ospf_logx(ospf_debug_lsa, "select new seq %08x ", p_lshdr->seqnum); 
        }
    }
    else
    {
        ospf_logx(ospf_debug_lsa, "lsa unchanged,don't install");
        p_lsa->self_rx_in_restart = FALSE;
		
	//	ospf_logx(ospf_debug_lsa,"%s:%d #return,p_table %x,p_lshdr %x\n",__FUNCTION__,__LINE__,p_table,p_lshdr);
        return;
    }
    
    /*calculate checksum*/
    /*init age*/
    age = p_lshdr->age;
    p_lshdr->age = 0;
    
    p_lshdr->checksum = 0x0000;
    p_lshdr->checksum = ospf_lsa_checksum_calculate(p_lshdr);

    /*recover age*/
    p_lshdr->age = age;

    /*lsa body changed,install it*/    
    if (different)
    {        
        /*increase statistics for try to lsa changed*/
        ospf.stat.self_lsa_change[p_lshdr->type]++;

        p_lsa = ospf_lsa_install(p_table, p_lsa, p_lshdr);
        if (NULL != p_lsa)
        {
        	#if 1
        	p_lsa->ucFlag = FALSE;
			#endif
            p_process->origin_lsa++;
            ospf_trap_origin(p_lsa);     
        }
    }
    
    /*clear graceful restart flag*/
    if (NULL != p_lsa)
    {
        p_lsa->self_rx_in_restart = FALSE;
        /*flood it*/
        ospf_lsa_flood (p_lsa,  NULL, NULL);
    }    
//	ospf_logx(ospf_debug_lsa,"%s:%d #return,p_table %x,p_lshdr %x\n",__FUNCTION__,__LINE__,p_table,p_lshdr);

    return;
}

 void 
ospf_printf_buffer(
           u_int8 *buf,
           u_int len,
           u_int8 *outbuf,
           u_int maxlen)
{
    u_int i;
    u_int8 str[32];
  
    memset(outbuf, 0, maxlen);
    
    for (i = 0 ; i < len ; i++)
    {
        sprintf(str, "%02x ",buf[i]);
        strcat(outbuf, str);
        if (strlen(outbuf) >= (maxlen - 4))
        {
            break;
        }
    }
    return;
}

/*after installing new lsa,call this function if new lsa has different content with old instance,
   if it is just a period refresh,do not call this function*/
void
ospf_process_changed_lsa(
                        struct ospf_lsa *p_lsa,
                        struct ospf_lshdr *p_oldhdr,
                        u_int old_exist)
{
    struct ospf_summary_lsa *p_summary = NULL;
    struct ospf_summary_lsa *p_old_summary = NULL;
    struct ospf_process *p_process = p_lsa->p_lstable->p_process;
    struct ospf_area *p_area = p_lsa->p_lstable->p_area;
    u_int ls_type = p_lsa->lshdr->type;
    u_int self = FALSE;
    
  //ospf_logx(ospf_debug_lsa, "ospf_process_changed_lsa");
    if (TRUE == old_exist)
    {
         /*if old instance exist,and content changed,leave restart state*/
        if (p_process->in_restart)
        {
            if (ospf_debug_gr)
            {         
                u_int8 str[512];
                ospf_logx(ospf_debug_lsa, "lsa changed,exit graceful restart");
                ospf_logx(ospf_debug_lsa, "old lsa");
                ospf_printf_buffer((u_int8*)p_oldhdr, ntohs(p_oldhdr->len), str, 511);
                ospf_logx(ospf_debug_lsa, "%s",str);
                ospf_logx(ospf_debug_lsa, "new lsa");
                ospf_printf_buffer((u_int8*)p_lsa->lshdr, ntohs(p_lsa->lshdr->len), str, 511);
                ospf_logx(ospf_debug_lsa, "%s" ,str);
            }
            ospf_restart_finish(p_process, OSPF_RESTART_TOPLOGY_CHANGED);
        }
    }

    if (p_lsa->lshdr->adv_id == ntohl(p_process->router_id))
    {
        self = TRUE;
    }
    
    /*exit current helper mode for changed non-restart lsa*/
    if ((OSPF_LS_TYPE_9 != ls_type) || (OSPF_GR_LSID != ntohl(p_lsa->lshdr->id)))
    {
        /*exit hepler when at least one neighbor in restarting will accept this lsa*/
        if (TRUE == ospf_restart_helper_affected_by_lsa(p_lsa))
        {
            if (ospf_debug_gr)
            {         
                u_int8 str[512];
                ospf_logx(ospf_debug_lsa, "lsa changed,exit restart helper");
                ospf_logx(ospf_debug_lsa, "old lsa");
                ospf_printf_buffer((u_int8*)p_oldhdr, ntohs(p_oldhdr->len), str, 511);
                ospf_logx(ospf_debug_lsa, "%s",str);
                ospf_logx(ospf_debug_lsa, "new lsa");
                ospf_printf_buffer((u_int8*)p_lsa->lshdr, ntohs(p_lsa->lshdr->len), str, 511);
                ospf_logx(ospf_debug_lsa, "%s",str);
            }
            ospf_restart_helper_finish_all(p_process, OSPF_RESTART_TOPLOGY_CHANGED);
        }
        
        /*if self originated lsa rxd,and self is in restart,set restart rxd flag*/
        if ((p_process->in_restart) && ospf_is_self_lsa(p_process, p_lsa->lshdr))
        {
            p_lsa->self_rx_in_restart = TRUE;
            return;
        }
    }
    else if (p_process->restart_helper && (FALSE == self))
    {
        /*update helper mode for grlsa*/
        ospf_restart_lsa_recv(p_lsa);
    }
    
    /*if not router/networklsa and adv-router is self,do not calculate route*/
    if ((OSPF_LS_ROUTER == ls_type) 
         || (OSPF_LS_NETWORK == ls_type)
         || (FALSE == self))
    {
        ospf_update_route_for_lsa(p_lsa);
    }

    /*translate type 7 lsa into type 5 lsa,may add or delete one*/
    if ((OSPF_LS_TYPE_7 == ls_type) && (FALSE == self))
    {
        /*when rcv type7 lsa,should elect translator immediately*/
        ospf_nssa_translator_elect(p_area);
                 
        ospf_nssa_lsa_translate(p_lsa);

      
        if (0 != htonl(p_lsa->lshdr->id))
        {
            /*for nssa lsa,may recover suppressed self originated lsa*/
            ospf_preferred_nssa_lsa_select(p_area, ospf_lsa_body(p_lsa->lshdr), (TRUE == old_exist) ? (struct ospf_nssa_lsa*)p_oldhdr : NULL);
        }
    }

    /*conflict network*/
    /*if 3,5,7 lsa's mask changed,rebuild whole route.*/
    if (((OSPF_LS_AS_EXTERNAL == ls_type)
         || (OSPF_LS_SUMMARY_NETWORK == ls_type)
         || (OSPF_LS_TYPE_7 == ls_type))
         && (TRUE == old_exist))
    {
        p_summary = (struct ospf_summary_lsa *)p_lsa->lshdr;
        p_old_summary = (struct ospf_summary_lsa *)p_oldhdr;
   
        /*these lsa's format are same*/
        if (p_summary->mask != p_old_summary->mask)
        {
            ospf_spf_request(p_process);
        }
    }

    if ((OSPF_LS_TYPE_10 == ls_type)
        && (OSPF_TE_LSID == (ntohl(p_lsa->lshdr->id)&OSPF_TE_LSID))
        && (FALSE == self))
    {
    	ucLsa_te = 1;
	//	ospf_type10_lsa_to_tedb(p_area,p_lsa);
    }
	
#ifdef OSPF_DCN
    if ((OSPF_LS_TYPE_10 == ls_type)
        && (OSPF_DCN_LSID == ntohl(p_lsa->lshdr->id))
        && (FALSE == self))
    {    
        ospf_dcn_rxmt_add(p_lsa);       
    }
#endif
    return;
}

/* section 13.2 (p. 137) */
struct ospf_lsa *
ospf_lsa_install(
                struct ospf_lstable *p_table,
                struct ospf_lsa *p_old,
                struct ospf_lshdr *p_lshdr)
{
    struct ospf_lsa *p_new = NULL;
    struct ospf_process *p_process = p_table->p_process;
    struct ospf_area *p_area = p_table->p_area;
    struct ospf_lsa_loginfo lsa_log;
    u_int different = FALSE;
    u_int old_exist = FALSE;
    u_int ls_type = p_lshdr->type;
    u_int8 old_buf[256];

    ospf_logx(ospf_debug_lsa, "install lsa in database type %d",ls_type);

    if ((NULL == p_area) 
        && (OSPF_LS_AS_EXTERNAL != ls_type) 
        && (OSPF_LS_TYPE_11 != ls_type))
    {
        ospf_logx(ospf_debug_lsa, "invalid area params");
        return NULL;
    }
    
    /*is difference found*/
	if((NULL != p_old) && (p_old->ucFlag == TRUE))
	{
		different = TRUE;
	}
	else
	{
    	different = p_old ? ospf_lsa_changed(p_lshdr, p_old->lshdr) : TRUE;
	}
        
    /*clear old instance*/
	#if 1
    if ((NULL != p_old) && (p_old->ucFlag == FALSE))
	#else
	if (NULL != p_old)
	#endif
    {  
        /*tag old lsa exist*/
        old_exist = TRUE;

        /*save part of old lsa's buffer,to check route change after installing*/
        memcpy(old_buf, p_old->lshdr, (ntohs(p_old->lshdr->len)) & 0x00ff);
        
        /*if current length enough,do not free old instance,just clear retransmit*/
        if (ntohs(p_old->lshdr->len) >= ntohs(p_lshdr->len))
        {
            ospf_lsa_rxmt_clear(p_old); 
            ospf_lsa_dd_clear(p_old);
        }
        else
        {
        	#if 0
            ospf_lsa_delete(p_old);
            p_old = NULL;
			#else
			ospf_lsa_delete_for_install(p_old);
			p_old->ucFlag = TRUE;
			#endif
        }
    }

    /*check asexternal limit if set*/
    if ((OSPF_DEFAULT_ASE_LIMIT != p_process->overflow_limit) 
        && (OSPF_LS_AS_EXTERNAL == ls_type))
    {
        if ((p_process->extlsa_count + 1) >= p_process->overflow_limit)
        {
            ospf_trap_overflow(p_process);             
            return NULL;
        }
   
        /*0.9*/
        if (((p_process->extlsa_count + 1)/9) >= (p_process->overflow_limit/10))
        {
            ospf_trap_near_overflow(p_process);
        }
    }
    /*add lsa*/
    p_new = ospf_lsa_add(p_old, p_table, p_lshdr);
    if (NULL == p_new)
    {
        ospf_logx(ospf_debug_lsa, "failed to add this lsa");
        return NULL;
    }
    
    /*if lsa changed,check if change affect route and restart*/
    if (different)
    {
        /*record changed not-local lsa */
        if (ntohl(p_lshdr->adv_id) != p_process->router_id)
        {
            ospf.stat.rxd_change_lsa[p_lshdr->type]++;
        } 

        ospf_process_changed_lsa(p_new, (struct ospf_lshdr *)old_buf, old_exist);

        /*add log for important lsa*/
        if ((OSPF_LS_ROUTER == ls_type)
            || (OSPF_LS_NETWORK == ls_type))
        {
            memcpy(lsa_log.lsa_hdr, p_lshdr, sizeof(struct ospf_lshdr));
            log_time_print(lsa_log.time);
            lsa_log.action = p_old ? OSPF_LSA_STAT_CHANGE : OSPF_LSA_STAT_CREATE;
            lsa_log.process_id = p_process->process_id;
            ospf_lsa_log_add(p_process, &lsa_log);
        }
    }
	#if 1
	p_new->ucFlag = FALSE;
	#endif
    return p_new;
}

/*APIs for LSA compare*/
/*decide if lsa changed section 13.2 (p. 137) */
u_int 
ospf_lsa_changed(
              struct ospf_lshdr *p1,
              struct ospf_lshdr *p2) 
{    
    u_short age = 0;
    u_short age2 = 0;
    u_int8 *p_buf1 = NULL;
    u_int8 *p_buf2 = NULL;
        
    age = ntohs(p1->age);
    age2 = ntohs(p2->age);
     
    /* first bullet item :option*/
    if (p1->option != p2->option)
    {
        ospf_logx(ospf_debug_lsa, "option changed");
        return TRUE;
    }
    
    /* second bullet item :age*/
    if ( ((OSPF_MAX_LSAGE <= age) && (OSPF_MAX_LSAGE > age2)) 
        || ((OSPF_MAX_LSAGE > age ) && (OSPF_MAX_LSAGE <= age2 )))
    {
        ospf_logx(ospf_debug_lsa, "age need change,age1=%d,age2=%d", age, age2);
        return TRUE;
    }
    
    /* third bullet item length*/
    if (p1->len != p2->len)
    {
        ospf_logx(ospf_debug_lsa, "length changed");
        return TRUE;
    }
    
    /* fourth bullet item :body*/ 
    p_buf1 = (u_int8*)p1 + OSPF_LSA_HLEN;
    p_buf2= (u_int8*)p2+ OSPF_LSA_HLEN;
    if (memcmp(p_buf1, p_buf2, ntohs(p1->len) - OSPF_LSA_HLEN)) 
    {
        ospf_logx(ospf_debug_lsa, "lsa body changed");
        return TRUE;
    }
    return FALSE;
}

/*decide which lsa is more recently according to lsa header
return value:
new lsa is newer than old lsa,  return  >0
new lsa is older than old lsa,  return   <0
new lsa is same as old lsa,  return   0*/
/* section 13.1 of OSPF specification (page 136) */
int
ospf_lshdr_cmp(
              struct ospf_lshdr *p1,
              struct ospf_lshdr *p2)
{
    int seq1;
    int seq2;
    u_short cks1;
    u_short cks2;
    u_short age1;
    u_short age2;
    u_int difference = 0;

    if ((NULL == p1) || (NULL == p2))
    {
        return ((u_long)p1 > (u_long)p2) ? 1 : -1;
    }     

    seq1 = ntohl(p1->seqnum);
    seq2 = ntohl(p2->seqnum);

    if (seq1 != seq2)
    {
        ospf_logx(ospf_debug_lsa, "lsa seqnum different %08x, %08x", seq1, seq2);
        return (seq1 > seq2) ? 1 : -1;
    }
    
    cks1 = ntohs(p1->checksum);
    cks2 = ntohs(p2->checksum);
    
    if (cks1 != cks2)
    {
        ospf_logx(ospf_debug_lsa, "lsa checksum different %u, %u", cks1, cks2);

        return (cks1 > cks2) ? 1 : -1;
    }

    age1 = ntohs(p1->age);
    age2 = ntohs(p2->age);

    /*if only one of the instances has its LS age field set  to MaxAge, 
    the instance of age MaxAge is considered to be  more recent
    */
    if ((OSPF_MAX_LSAGE <= age1) && (OSPF_MAX_LSAGE > age2))
    {
        ospf_logx(ospf_debug_lsa, "one of lsa age expired %d,%d", age1, age2);
        return 1;
    }   
    
    if ((OSPF_MAX_LSAGE <= age2) && (OSPF_MAX_LSAGE > age1))
    {
        ospf_logx(ospf_debug_lsa, "one of lsa age expired %d,%d", age1, age2);
        return -1;
    }  
    /*Else, if the LS age fields of the two instances differ by more than MaxAgeDiff,
    the instance having the smaller(younger) LS age is considered to be more recent*/
    difference = (age1 > age2) ? ( age1 - age2) : (age2 - age1);
    if (OSPF_MAX_LSAGE_DIFFERENCE <= difference)
    {
        ospf_logx(ospf_debug_lsa, "lsa age different %d,%d", age1, age2);
        return (age1 > age2) ? -1 : 1;
    }
    /*Else, the two instances are considered to be identical.*/
    ospf_logx(ospf_debug_lsa, "two lsa are same instance");
    return 0;
}


void 
ospf_lsa_stat_update(
                   struct ospf_lsa *p_lsa,
                   signed int step)
{
    struct ospf_process *p_process = p_lsa->p_lstable->p_process;
    struct ospf_area *p_area = p_lsa->p_lstable->p_area;
    u_short cks = ntohs(p_lsa->lshdr->checksum);
    
    switch (p_lsa->lshdr->type){
        case OSPF_LS_TYPE_9:
             p_process->rx_opaque_lsa += step;
             p_process->t9lsa_count += step;
             p_process->t9lsa_checksum += (int)(cks*step);        
             break;
            
        case OSPF_LS_TYPE_10:
             p_process->rx_opaque_lsa += step;
             if (NULL != p_area)
             {
                 p_area->t10_lscount += step;
                 p_area->t10_cksum += (int)(cks*step);
             }
             break;
            
        case OSPF_LS_TYPE_11:
             p_process->rx_opaque_lsa += step;
             p_process->t11lsa_count += step;
             p_process->t11lsa_checksum += (int)(cks*step);
             break;
            
        case OSPF_LS_AS_EXTERNAL:
             p_process->rx_lsa += step;
             p_process->extlsa_count += step;
             p_process->extlsa_checksum += (int)(cks*step);
             break;
            
        case OSPF_LS_ROUTER:
        case OSPF_LS_NETWORK:
        case OSPF_LS_SUMMARY_NETWORK:
        case OSPF_LS_SUMMARY_ASBR:
        case OSPF_LS_MULTICAST:
        case OSPF_LS_TYPE_7:
             p_process->rx_lsa += step;
             if (NULL != p_area)
             {
                 p_area->lscount += step;
                 p_area->cksum += (int)(cks*step);
             }
             break;
            
        default:
            break;
    }
    return;
}

/*add lsa to lsa table,may create new memory*/
struct ospf_lsa *
ospf_lsa_add(
            struct ospf_lsa *p_old,
            struct ospf_lstable *p_table,
            struct ospf_lshdr *p_lshdr)
{
    struct ospf_lsa *p_new = NULL;
    struct ospf_process *p_process = p_table->p_process;
    u_int age = 0;
    u_int expire = 0;
    u_int len = ntohs(p_lshdr->len),uiMallocLen = 0;
    u_int now = ospf_sys_ticks();
    u_int uiPreLen = 0, uiNewLen = 0;

    /*if old lsa exist,use this entry directly*/
    if (NULL != p_old)
    {
        p_new = p_old;
        uiPreLen = ntohs(p_new->lshdr->len)/OSPF_LSA_HDR_LEN_MAX;
        uiNewLen = len/OSPF_LSA_HDR_LEN_MAX;
        /*copy content,if input and new are same pointer,do not copy*/
        if (p_new->lshdr != p_lshdr && uiPreLen >= uiNewLen)
        {
            memcpy(p_new->lshdr, p_lshdr, len);
        }
        else if(p_new->lshdr != p_lshdr && uiPreLen < uiNewLen)
        {
            uiMallocLen = OSPF_LSA_HDR_LEN_MAX * (uiNewLen+1)+sizeof(struct ospf_lsa) - sizeof(struct ospf_lshdr);
            p_new = ospf_malloc(uiMallocLen, OSPF_MLSA);
           // printf("p_new = %p len = %d uiPreLen = %d uiNewLen = %d\n",p_new,uiMallocLen,uiPreLen,uiNewLen);
           ospf_logx(ospf_debug_lsa, "p_new = %p len = %d uiPreLen = %d uiNewLen = %d",p_new,uiMallocLen,uiPreLen,uiNewLen);
            if(NULL == p_new)
            {
                return NULL;
            }
            memcpy(p_new,p_old,ntohs(p_old->lshdr->len)+sizeof(struct ospf_lsa)-sizeof(struct ospf_lshdr));
            memcpy(p_new->lshdr, p_lshdr, len);
            ospf_lstdel_free(&p_table->list,p_old,OSPF_MLSA);
            ospf_lstadd(&p_table->list, p_new);
        }
        p_new->expired = FALSE;
        p_new->update_time = p_new->rx_time = now;

        goto UPDATE_AGING_TIMER;
    }

    /*allocate buffer containing lsa control struct and lsa buffer*/
    
   // p_new = ospf_malloc((len + sizeof(struct ospf_lsa) - sizeof(struct ospf_lshdr)), OSPF_MLSA);
    uiMallocLen = OSPF_LSA_HDR_LEN_MAX+sizeof(struct ospf_lsa) - sizeof(struct ospf_lshdr);
    if(len > uiMallocLen)
    {
        uiMallocLen = len+sizeof(struct ospf_lsa) - sizeof(struct ospf_lshdr);
    }
    if(ulOspfmemFlag == 1)
    {
        printf("ospf_lsa_add len=%d,p_lshdr->len=%d,Max = %d,p_old = %x\n",len,p_lshdr->len,uiMallocLen,p_old);
    }
    p_new = ospf_malloc(/*(len + sizeof(struct ospf_lsa) - sizeof(struct ospf_lshdr))+*/uiMallocLen, OSPF_MLSA);
    if (NULL == p_new)
    {
        return NULL;
    }   
    /*copy content*/
    memcpy(p_new->lshdr, p_lshdr, len);
    p_new->p_lstable = p_table;
    p_new->expired = FALSE;
    p_new->update_time = p_new->rx_time = now;
        
    ospf_lstadd(&p_table->list, p_new);

    /*statistics*/     
    ospf_lsa_stat_update(p_new, 1);

	
UPDATE_AGING_TIMER:
 //   printf("\r\nospf_lsa_add:p_old %p p_lsa %p p_lsa->p_lstable %p\n", p_old, p_new, p_new->p_lstable);
    /*update aging timer,decide expired time of this lsa, and compare result with
      aging timer's expired time,if new value is less,restart aging timer
     */    
    age = ntohs(p_new->lshdr->age);
    if (OSPF_MAX_LSAGE <= age)
    {
        expire = 1;
    }
    else if (ntohl(p_new->lshdr->adv_id) == p_process->router_id)
    {
        expire = (age >= OSPF_LS_REFRESH_TIME) ? 1 : (OSPF_LS_REFRESH_TIME - age);
    }
    else
    {
        expire = OSPF_MAX_LSAGE - age;
    }

    /*special process for grestart,we need fast timer*/
    if (OSPF_LS_TYPE_9 == p_new->lshdr->type)
    {
        expire = 1;
    }
    if (OSPF_MAX_LSAGE_DIFFERENCE < expire)
    {
        expire = OSPF_MAX_LSAGE_DIFFERENCE;
    }

    ospf_stimer_safe_start(&p_process->lsa_aging_timer, expire);
    

    /*schedule sync msg for this lsa*/
    if (OSPF_MODE_MASTER == p_process->p_master->work_mode)
    {
        ospf_syn_lsa_send(p_new, TRUE, NULL);        
    }
	p_new->ucFlag = FALSE;
    return p_new;
}

void 
ospf_lsa_delete_for_install(struct ospf_lsa *p_lsa)
{
    struct ospf_lstable *p_table = p_lsa->p_lstable;
    struct ospf_process *p_process = p_table->p_process;
    struct ospf_lsa_loginfo lsa_log;
	struct ospf_area *p_area = NULL;
	
    /*add log for important lsa*/
    if ((OSPF_LS_ROUTER == p_lsa->lshdr->type)
        || (OSPF_LS_NETWORK == p_lsa->lshdr->type))
    {
        memcpy(lsa_log.lsa_hdr, p_lsa->lshdr, sizeof(struct ospf_lshdr));
        log_time_print(lsa_log.time);   
        lsa_log.action = OSPF_LSA_STAT_DELETE;                  
        lsa_log.process_id = p_process->process_id;
        ospf_lsa_log_add(p_process, &lsa_log);
    }    
    /*clear lsa related retransmit node*/
    ospf_lsa_rxmt_clear(p_lsa); 
    ospf_lsa_dd_clear(p_lsa);
	
    /*statistic*/       
    //ospf_lsa_stat_update(p_lsa, -1);
	
    /*send sync msg for lsa directly,no wait*/
    if (OSPF_MODE_MASTER == p_process->p_master->work_mode)
    {
        ospf_syn_lsa_send(p_lsa, FALSE, NULL);
    }
	//memset(p_lsa->lshdr, 'x', OSPF_LSA_HDR_LEN_MAX);
	return ;
}

/*delete lsa from lsa table*/
void 
ospf_lsa_delete(struct ospf_lsa *p_lsa)
{
    struct ospf_lstable *p_table = NULL;
    struct ospf_process *p_process = NULL;
    struct ospf_lsa_loginfo lsa_log;
	struct ospf_area *p_area = NULL;

	p_table = p_lsa->p_lstable;
	p_process = p_table->p_process;
    /*add log for important lsa*/
    if ((OSPF_LS_ROUTER == p_lsa->lshdr->type)
        || (OSPF_LS_NETWORK == p_lsa->lshdr->type))
    {
        memcpy(lsa_log.lsa_hdr, p_lsa->lshdr, sizeof(struct ospf_lshdr));
        log_time_print(lsa_log.time);   
        lsa_log.action = OSPF_LSA_STAT_DELETE;                  
        lsa_log.process_id = p_process->process_id;
        ospf_lsa_log_add(p_process, &lsa_log);
    }    
    /*clear lsa related retransmit node*/
    ospf_lsa_rxmt_clear(p_lsa); 
    ospf_lsa_dd_clear(p_lsa);
	
    /*statistic*/       
    ospf_lsa_stat_update(p_lsa, -1);
	
    /*send sync msg for lsa directly,no wait*/
    if (OSPF_MODE_MASTER == p_process->p_master->work_mode)
    {
        ospf_syn_lsa_send(p_lsa, FALSE, NULL);
    }
//	printf("ospf_lsa_delete:p_lsa %p p_lsa->p_lstable %p enter %s:%d\n", p_lsa, p_lsa->p_lstable, __FUNCTION__, __LINE__);
    /*delete from lsa table and free it*/
	#if 0
	p_area = p_lsa->p_lstable->p_area;
	p_area->ls_table[OSPF_MLSA] = NULL;
	ospf_timer_stop(&p_process->lsa_aging_timer);
	#endif

    ospf_lstdel_free(&p_table->list, p_lsa, OSPF_MLSA);   
//	printf("ospf_lsa_delete:type %d  id %x adv_id %x enter %s:%d\n", 
//		p_lsa->lshdr->type, p_lsa->lshdr->id,p_lsa->lshdr->adv_id, __FUNCTION__, __LINE__);
    return ;
}

 /*delete all lsa in special lsa table*/
void
ospf_lsa_table_flush(struct ospf_lstable *p_table)
{
    struct ospf_conflict_network *p_network = NULL;
    struct ospf_conflict_network *p_next = NULL;
    
    ospf_lstwalkup(&p_table->list, ospf_lsa_delete);

    for_each_node(&p_table->conflict_list, p_network, p_next)
    {
        ospf_lstdel(&p_table->conflict_list, p_network);
        ospf_mfree(p_network, OSPF_MPACKET);
    }
    return;
}

/*delete all lsa of an instance, called when instance is shutdown or restarted*/
void 
ospf_lsa_flush_all(struct ospf_process *p_process)
{
    ospf_lstwalkup(&p_process->lstable_table, ospf_lsa_table_flush);
    return;
}

/*obtain lsas with same id,do not compare other fileds*/
void 
ospf_lsa_lookup_by_id(
           struct ospf_lst *p_table,
           u_int type,
           u_int id,
           u_int mask,
           struct ospf_lsvector *p_vector) 
{
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa min_lsa;
    struct ospf_lsa max_lsa;
    struct ospf_summary_lsa *p_summary = NULL;
    
    memset(p_vector, 0, sizeof(struct ospf_lsvector));
    /*set scan range*/
    min_lsa.lshdr->type = type;
    min_lsa.lshdr->id = htonl(id);
    min_lsa.lshdr->adv_id = 0;

    max_lsa.lshdr->type = type;
    if ((OSPF_LS_NETWORK != type) && (OSPF_LS_ROUTER != type))
    {
        max_lsa.lshdr->id = id | (~mask);
    }
    else
    {
        max_lsa.lshdr->id = id;
    }
    max_lsa.lshdr->id = htonl(max_lsa.lshdr->id);
    
    max_lsa.lshdr->adv_id = OSPF_HOST_NETWORK;

    for_each_node_range(p_table, p_lsa, &min_lsa, &max_lsa)
    {     
        if ((OSPF_LS_NETWORK != type) && (OSPF_LS_ROUTER != type))
        {
            p_summary = (struct ospf_summary_lsa *)p_lsa->lshdr;
            /*mask must match*/
            if (ntohl(p_summary->mask) != mask)
            {
                continue;
            }        
        }
         
        /*insert to vector*/
        if (OSPF_LSVECTOR_LEN > p_vector->count)
        {
            p_vector->p_lsa[p_vector->count++] = p_lsa;
        }
    }
    return;
}

/*search lsa, type and id must be set,if router is set,search directly;else search first one matched*/
struct ospf_lsa *
ospf_lsa_lookup(
              struct ospf_lstable *p_table,
              struct ospf_lshdr *p_lshdr)
{
    struct ospf_lsa search;
    struct ospf_lsa *p_lsa = NULL;

    if (NULL == p_table)
    {
        return NULL;
    }
    search.lshdr->type = p_lshdr->type;
    search.lshdr->id = p_lshdr->id;    
    search.lshdr->adv_id = p_lshdr->adv_id;

    p_lsa = ospf_lstlookup(&p_table->list, &search);
    if (NULL != p_lsa)
    {
        return p_lsa;
    }
    /*if router is zero,we got the first one with same id*/
    if (0 == p_lshdr->adv_id)
    {
        p_lsa = ospf_lstgreater(&p_table->list, &search);
        if ((NULL != p_lsa ) && (p_lsa->lshdr->id == p_lshdr->id))
        {
            return p_lsa;
        }
    }
    return NULL;
}

/*clear all retransmit information of lsa*/
void 
ospf_lsa_rxmt_clear(struct ospf_lsa *p_lsa)
{
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    struct ospf_process *p_process = p_lsa->p_lstable->p_process;

    if (NULL == p_lsa->p_rxmt)
    {
        return;
    }
    
    /*clear all nbr's rxmt*/
    for_each_node(&p_process->nbr_table, p_nbr, p_next_nbr) 
    {   
   //     ospf_logx(ospf_debug,"%s %d \n", __FUNCTION__,__LINE__);
        ospf_nbr_rxmt_delete (p_nbr, p_lsa->p_rxmt);
        /*all rxmt deleted,do not check any more*/ 
        if (NULL == p_lsa->p_rxmt)
        {
            break;
        }
    }    
    return;
}

/*clear all dd information about a lsa*/
void 
ospf_lsa_dd_clear(struct ospf_lsa *p_lsa)
{
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    struct ospf_process *p_process = p_lsa->p_lstable->p_process;

    if ((NULL == p_lsa->p_rxmt) || (0 == p_lsa->p_rxmt->dd_count))
    {
        return;
    }

    /*clear all nbr's dd rxmt*/
    for_each_node(&p_process->nbr_table, p_nbr, p_next_nbr) 
    {   
        ospf_nbr_dd_delete (p_nbr, p_lsa->p_rxmt);
        /*all rxmt deleted,do not check any more*/ 
        if (NULL == p_lsa->p_rxmt)
        {
            break;
        }
    }    
    return;
}

/*process expired lsa,return :indicate expired time,if this lsa is deleted, do not check it anymore
    so, use largest value,else use min value for fastly checking
  */ 
u_int 
ospf_lsa_expired(
            struct ospf_lsa *p_lsa,
            u_int self,
            u_int exchange_or_loading/*current nbr count in exchange or loading state*/)
{
    struct ospf_process *p_process = p_lsa->p_lstable->p_process;
    u_int finish_retransmit = FALSE ;

    /*first timeout,check route change and do flooding*/
    if (!p_lsa->expired)
    {
        /*clear all dd nodes about it*/
        ospf_lsa_dd_clear(p_lsa);

        /*send trap*/
        ospf_trap_maxage(p_lsa); 

        /*set expire flag*/
        p_lsa->expired = TRUE;

        /*update route*/
    //    ospf_logx(ospf_debug,"%s %d \n", __FUNCTION__,__LINE__);
        ospf_update_route_for_lsa(p_lsa);

        /*flood it*/
        ospf_lsa_flood (p_lsa, NULL, NULL);

        /*delete translated nssa lsa*/
        if (OSPF_LS_TYPE_7 == p_lsa->lshdr->type)
        {
            ospf_nssa_lsa_translate(p_lsa);                     
        }
        /*leave any hepler mode for not grlsa*/
        if ((OSPF_LS_TYPE_9 != p_lsa->lshdr->type) 
            || (OSPF_GR_LSID != ntohl(p_lsa->lshdr->id)))
        {
            if (ospf_restart_helper_affected_by_lsa(p_lsa) == TRUE)
            {
                ospf_logx(ospf_debug_lsa, "exit gr helper for non gr lsa");     
 
                ospf_restart_helper_finish_all(p_process, OSPF_RESTART_TOPLOGY_CHANGED);
            }
        }
    }

    /*decide if retransmit is ok*/
    if ((!ospf_lsa_rxmt_exist(p_lsa)) && (! exchange_or_loading))
    {
        finish_retransmit = TRUE;
    }
     
    /*self lsa flused for max-sequence,if no retransmit exist,originate new instance*/
    if (self && p_lsa->maxseq_wait)
    {
        /*if no retransmit need, re-originate it*/
        if (finish_retransmit)
        {
            ospf_lsa_refresh(p_lsa);
        }        
        return OSPF_LS_REFRESH_TIME;        
    }
 
    /*if not self originated lsa,delete it directly,
       else must wait for ack and have no exchange/loading nbr*/
    if (finish_retransmit)
    {
        ospf_logx(ospf_debug_lsa, "delete lsa id %x adv_id %x type %d age %d\n",
        ntohl(p_lsa->lshdr->id),ntohl(p_lsa->lshdr->adv_id),
        p_lsa->lshdr->type,ntohs(p_lsa->lshdr->age)); 
        ospf_lsa_delete(p_lsa);
        return OSPF_MAX_LSAGE;
    }                   
    /*lsa is not deleted,so will check it fastly*/
    return OSPF_MIN_LSAGE_TIME;
}

/*increase lsa's real age and set update time*/
void 
ospf_lsa_age_update(struct ospf_lsa *p_lsa)
{
    u_short age = ntohs(p_lsa->lshdr->age);
    u_int now = ospf_sys_ticks();
    u_int differ = ospf_time_differ(now, p_lsa->update_time);
    u_int differ_s = 0;/*seconds*/
   
    /*printf("now %d,last update %d, differ %d\r\n",now, p_lsa->update_time, differ);*/
    differ_s = differ/OSPF_TICK_PER_SECOND;
 
    age += differ_s;

    if (OSPF_MAX_LSAGE < age)
    {
        age = OSPF_MAX_LSAGE; 
		if(p_lsa->p_lstable != 	NULL)
		{
			if(p_lsa->p_lstable->p_process != 	NULL)
			{
        ospf_stimer_safe_start(&p_lsa->p_lstable->p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);            
			}
			else
			{
				ospf_logx(ospf_debug_lsa,"(p_lsa->p_lstable->p_process != 	NULL)\n");
			}
		}
		else
		{
			ospf_logx(ospf_debug_lsa,"p_lsa->p_lstable != 	NULL\n");
		}
   }
   p_lsa->lshdr->age = htons(age); 
    /*do not set to current time!!*/
    if (now >= (differ % OSPF_TICK_PER_SECOND))
    {
        p_lsa->update_time = now - (differ % OSPF_TICK_PER_SECOND);
    }
    else
    {
        p_lsa->update_time = 0;
    }    
    return;
}

/*unique function used to age all type of lsa*/
u_int 
ospf_lsa_age_check(
            struct ospf_lsa *p_lsa,
            u_int exchange_or_loading)
{
    struct ospf_process *p_process = NULL;
    u_int self = FALSE;
    u_int expire = OSPF_MAX_LSAGE;
    u_short age = 0;

	if(NULL == p_lsa->p_lstable)
	{
		return 0xffffffff;
	}
	p_process = p_lsa->p_lstable->p_process;
    /*add age according to current aging interval*/
    ospf_lsa_age_update(p_lsa);
    
    /*new age*/
    age = ntohs(p_lsa->lshdr->age);

    /*increase age*/
    if (OSPF_MAX_LSAGE > age)
    {
        /*if local node not in restarting,and there be lsa in gr state,fast age it*/
        if ( !p_process->in_restart && p_lsa->self_rx_in_restart)
        {
            age += OSPF_LS_REFRESH_TIME; 
            p_lsa->lshdr->age = htons(age);
        }
    }      
    
    /*decide if lsa is originated from self*/
    self = ospf_is_self_lsa(p_process, p_lsa->lshdr);    
    
    /*flood max age lsa or delete it*/
    if ( OSPF_MAX_LSAGE <= age)
    {
        expire = ospf_lsa_expired(p_lsa, self, exchange_or_loading);
    }
    else if (self && (OSPF_LS_REFRESH_TIME <= age ))
    {
        /* section 12.4, item (1) - (page 114-115) */
        if (!p_lsa->self_rx_in_restart)
        {
            ospf_lsa_refresh(p_lsa);
            expire = OSPF_LS_REFRESH_TIME;
        }
        else if (!p_process->in_restart)
        {
            expire = OSPF_MIN_LSAGE_TIME;
        }
    }
    else 
    {
        /*get expired time*/
        expire = self ? (OSPF_LS_REFRESH_TIME - age) : (OSPF_MAX_LSAGE - age);
    }    
    return expire;
}

/*check lsa aging timer for all of lsa*/ 
void 
ospf_lsa_table_timer_expired(struct ospf_process *p_process)
{
    struct ospf_lstable *p_table = NULL;
    struct ospf_lstable *p_nexttable = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next = NULL;
    u_int expire = 0;
    u_int min = OSPF_MAX_LSAGE_DIFFERENCE;
    u_int exchange_or_loading = FALSE;
    u_int8 state_mask[4] = {0};

    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    ospf_set_context(p_process);
    /*set state mask,check exchange or loading nbr*/
    BIT_LST_SET(state_mask, OSPF_NS_EXCHANGE);
    BIT_LST_SET(state_mask, OSPF_NS_LOADING);

    /*check if there be any nbr in exchange or loading state*/
    exchange_or_loading = ospf_nbr_count_in_state(p_process, state_mask);

    for_each_node(&p_process->lstable_table, p_table, p_nexttable)
    {
        for_each_node(&p_table->list, p_lsa, p_next)
        {
            expire = ospf_lsa_age_check(p_lsa, exchange_or_loading);
			if(0xffffffff == expire)
			{
				continue;
			}
            if (expire < min)
            {
                min = expire;
            }
        }
        /* check conflict network*/
        if (ospf_lstcnt(&p_table->conflict_list))
        {
            ospf_conflict_network_table_update(p_table);
        }
    }

    /*restart timer according to lsa expired time*/
    ospf_stimer_start(&p_process->lsa_aging_timer, min);
    return;
}

/*set lsa's age to maxage,and restart lsa aging timer*/
void 
ospf_lsa_maxage_set(struct ospf_lsa *p_lsa) 
{
    void *pData=NULL;
    
    if((p_lsa == NULL)||(p_lsa->lshdr == NULL)||
        (p_lsa->p_lstable == NULL)||
        (p_lsa->p_lstable->p_process == NULL))
    {
        ospf_logx(ospf_debug_lsa,"ospf_lsa_maxage_set p_lsa == 	NULL\n");
        return;
    }

    pData = &p_lsa->p_lstable->p_process->lsa_aging_timer;
    if(pData == NULL)  
    {
        ospf_logx(ospf_debug_lsa,"ospf_lsa_maxage_set (pData == NULL)\n");
        return;
    }
    p_lsa->lshdr->age = htons(OSPF_MAX_LSAGE);
    ospf_stimer_safe_start(&p_lsa->p_lstable->p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);   
    return;
}

/*tp avoid same type5*/
int ospf_as_external_search(struct ospf_process *p_process,struct ospf_lshdr *p_lshdr)

{
    struct ospf_lsa *p_lsa = NULL;    
		
    p_lsa = ospf_lsa_lookup(&p_process->t5_lstable, p_lshdr);
    if (p_lsa)
	{
		return OK;
	}
	
	return ERR;

}
/*NSSA Translate*/
/*Translate one type 7 lsa into type 5 lsa*/
void 
ospf_nssa_lsa_translate(struct ospf_lsa *p_lsa)
{
    struct ospf_area *p_area = p_lsa->p_lstable->p_area;
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_lsa *p_best = NULL;
    struct ospf_nssa_lsa *p_nssa = NULL;
    struct ospf_iproute route;         
    struct ospf_lsvector vector;
    u_int i = 0;
    u_int self = FALSE;
    u_int cost;
    u_int faddr;
    u_int dest = ntohl(p_lsa->lshdr->id);
    u_int mask = 0; 
	struct ospf_lshdr lshdr;	
	int ret = 0;

    p_nssa = ((struct ospf_nssa_lsa *)p_lsa->lshdr);
    mask = ntohl(p_nssa->mask);
  
	ospf_logx(ospf_debug_lsa, "ospf translate nssa lsa :	%x/%x,adv_id=%x,age=%d", dest, mask,p_lsa->lshdr->adv_id,ntohs(p_nssa->h.age));  

	ospf_logx(ospf_debug_lsa, "areaid=%d,is nssa=%d,translator=%d", p_area->id, p_area->is_nssa, p_area->nssa_translator);  
  	/*if self is not nssa translator,and the nssa wait timer 
    is not running,do nothing*/ 
    if ((!p_area->is_nssa) 
        || ((!p_area->nssa_translator) && !ospf_timer_active(&p_area->nssa_timer)))
    {
        return;
    }

    /*first,decide if need remove more special lsa*/
    ospf_get_nssa_lsa_with_id(p_process, dest, mask, &vector);
    
    /*deicde best nssa lsa from all of nssa area*/
    p_best = ospf_best_lsa_for_translate(dest, mask, &vector);

   /*decide if any local nssa exist, if exist local nssa,it come from redistribute,so don't
   flush type5 lsa originated by redistribute*/
    for (i = 0 ; i < vector.count; i++)
    {
        if ((NULL != vector.p_lsa[i])
            && (p_process->router_id == ntohl(vector.p_lsa[i]->lshdr->id)))
        {
            self = TRUE;
            return;
        }
    }

    ospf_logx(ospf_debug_lsa, "p_best=%x,p_lsa=%x,self=%d", p_best, p_lsa,self);  
    
    /*no lsa to translate and no self nssa lsa,delete it*/
	
	/*delete external lsa.*/
    if ((NULL == p_best)
		||((p_best == p_lsa)&&(ntohs(p_nssa->h.age) == OSPF_MAX_LSAGE)))
    {
        ospf_flush_external_lsa(p_process, dest, mask);
        //ospf_stimer_start(&p_process->asbr_range_update_timer, 5);
        /*end*/
		ospf_logx(ospf_debug_lsa, "ospf_flush_external_lsa dest=%x,mask=%x", dest, mask);  
		return;
    }
	
	lshdr.type = OSPF_LS_AS_EXTERNAL;
	lshdr.id = htonl(dest);
	lshdr.adv_id = p_lsa->lshdr->adv_id;
	/*查询type5表中是否已包含该路由，避免重复下发*/
	ret = ospf_as_external_search(p_process,&lshdr);
    ospf_logx(ospf_debug_lsa, "dest=%x,adv_id=%x,ret=%d,age=%d", lshdr.id, lshdr.adv_id,ret,ntohs(p_nssa->h.age));  

	/*不存在该条路由且LSA老化时间不为3600时才添加*/
	if((ERR == ret)&&(ntohs(p_nssa->h.age) != OSPF_MAX_LSAGE))
    {
    
        /*if self nssa not exist,originate external lsa*/    
        p_nssa = ((struct ospf_nssa_lsa *)p_best->lshdr);
        cost = ntohl(p_nssa->metric);
        faddr = ntohl(p_nssa->fwdaddr);
        ospf_build_external_route(&route, dest, mask, cost, faddr);    
        route.active = TRUE;
        route.p_process = p_process;
        route.nssa_route = TRUE;
        
        ospf_external_lsa_originate(&route);
    }
    //ospf_stimer_start(&p_process->asbr_range_update_timer, 5);
    /*end*/
    return;
}

/*for special dest.mask,select best nssa lsa to be translated.
there may be multiple nssa lsa with same dest and mask,in this case,we must
select one.*/ 
struct ospf_lsa *
ospf_best_lsa_for_translate(
                       u_int dest,
                       u_int mask,
                       struct ospf_lsvector *p_vector)
{
    struct ospf_lsa *p_best = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_nssa_lsa *p_nssa = NULL;
    struct ospf_range *p_range = NULL;       
    u_int best_metric = 0xffffffff;/*max value*/
    u_int i = 0;

    ospf_logx(0, "ospf_best_lsa_for_translate count=%d", p_vector->count);  

    for (i = 0; i < p_vector->count; i++)       
    {
        p_lsa = p_vector->p_lsa[i];
        if (NULL == p_lsa)
        {
            continue;
        }
        /*ignore local ,P=0,or age invalid*/       
       // if (ntohs(p_lsa->lshdr->age) >= OSPF_MAX_LSAGE)
        if (ntohs(p_lsa->lshdr->age) > OSPF_MAX_LSAGE)
        {
        
			ospf_logx(0, "ospf_best_lsa_for_translate age=%d", ntohs(p_lsa->lshdr->age));	
            continue;
        }
        if (!ospf_option_nssa(p_lsa->lshdr->option))
        {
        
			ospf_logx(0, "ospf_best_lsa_for_translate option=%d", p_lsa->lshdr->option);	
            continue;
        }
        p_nssa = (struct ospf_nssa_lsa *)p_lsa->lshdr;
        /*fwdaddr 0 do not translate*/
        if ((0 == p_nssa->fwdaddr)&&(0 == dest))
        {
			ospf_logx(0, "ospf_best_lsa_for_translate fwdaddr=%x", p_nssa->fwdaddr);	
            continue;
        }
        /*self is translator ,or wait timer not expired,accept it*/
        p_area = p_lsa->p_lstable->p_area;
        if ((!p_area->nssa_translator) && !ospf_timer_active(&p_area->nssa_timer))
        {
        
			ospf_logx(0, "ospf_best_lsa_for_translate nssa_translator=%d", p_area->nssa_translator);	
            continue;
        }
           
        /*if area range covered it,do not translate*/
        p_range = ospf_range_match(p_area, OSPF_LS_TYPE_7, dest, mask, NULL);
        if ((NULL != p_range) && (!p_range->isdown))
        {
            /*if range has shorter mask or already has aggregated lsa,ignore*/
            if ((p_range->network != dest) 
                || (p_range->mask < mask)
                || (ospf_nssa_range_active(p_range)))
            {
                p_range->need_check = TRUE;
                ospf_timer_try_start(&p_area->p_process->range_update_timer, 5);
                continue;
            }
        }
        /*compare metric,less metric prefered*/
        if ((NULL == p_best) || (best_metric > ntohl(p_nssa->metric)))
        {
            p_best = p_lsa;          
            best_metric = ntohl(p_nssa->metric);
			ospf_logx(0, "ospf_best_lsa_for_translate p_best=%x", p_best);	
        }
    }
    return p_best;
}

/*get lsa vector for same dest and mask's nssa lsa,search in all areas*/
void 
ospf_get_nssa_lsa_with_id(
                    struct ospf_process *p_process, 
                    u_int dest,
                    u_int mask, 
                    struct ospf_lsvector *p_vector)
{
    struct ospf_lsvector vector;/*temp list*/
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next = NULL;    
    u_int i = 0;
    
    p_vector->count = 0;
    
    for_each_ospf_area(p_process, p_area, p_next)
    {
        /*get lsa in this area*/
        ospf_lsa_lookup_by_id(&p_area->ls_table[OSPF_LS_TYPE_7]->list, OSPF_LS_TYPE_7, dest, mask, &vector) ;
         
         /*copy to output list*/
        for (i = 0; i < vector.count; i++)              
        {
            p_vector->p_lsa[p_vector->count++] = vector.p_lsa[i];
            if (OSPF_LSVECTOR_LEN <= p_vector->count )
            {
                return;
            }
        }
    } 
    return;
}



u_short 
ospf_lsa_checksum_calculate(struct ospf_lshdr *p_lsa)
{
    long c0 = 0;
    long c1 = 0;
    u_int8 *p_data = (u_int8 *) p_lsa;
    u_int length = ntohs(p_lsa->len);
    u_int bytes = 0;
    u_int i = 0;
    u_int k = 0;
    u_short result = 0;
    u_short age = p_lsa->age;

    p_lsa->age = 0;
    
    bytes = length & OSPF_MOD_MASK;

    for (i = bytes & OSPF_MASK_FOR_UNEVEN_BITS; i > 0x00000000L; --i)
    {
        c0 = (long) (c0 + (*p_data++));
        c1 = (long) (c1 + c0);
    }

    bytes >>= OSPF_INLINED_SHIFT;
    while (bytes-- > 0)
    {
        for (i = 0x00000008; i > 0x00000000L; --i)
        {
            c0 = (long) (c0 + (*p_data++));
            c1 = (long) (c1 + c0);
        }
    }
    /*
     * Now process the remainder in 4096 chunks, with a mod beforehand to avoid overflow.
     */
    bytes = length >> OSPF_LOG2_OF_NUMBER_OF_ITERATIONS;

    for (; bytes > 0x00000000L; --bytes)
    {
        if (p_data != (u_int8 *) p_lsa)
        {
            c0 %= OSPF_MODULUS;
            c1 %= OSPF_MODULUS;
        }

        for (k = OSPF_NUMBER_OF_INLINE_ITERATIONS; 
             k > 0x00000000L; 
             --k)
        {
            for (i = 0x00000008; i > 0x00000000L; --i)
            {
                c0 = (long) (c0 + (*p_data++));
                c1 = (long) (c1 + c0);
            }
        }
    }

   /* do modulus of c0 now to avoid overflow below */
    c0 %= OSPF_MODULUS;                                                        
    c1 = (long) ((c1 - ((long) (((u_int8 *) p_lsa + length) - (u_int8 *)&p_lsa->checksum) * c0)) % OSPF_MODULUS);
    if (c1 <= 0x00000000L) 
    {
        c1 += OSPF_MODULUS;
    }

    c0 = (long) (OSPF_MODULUS - c1 - c0);
    if (c0 <= 0x00000000L)
    {
        c0 += OSPF_MODULUS;
    }

    result = (u_short) ((c0 << OSPF_FINAL_CHECKSUM_SHIFT) | c1);
    p_lsa->age = age; 
    return htons(result);
}

/*get lsa table according to rxd interface and lsa type*/
struct ospf_lstable *
ospf_lstable_lookup(
             struct ospf_if *p_if,
             u_int8 type)
{
    struct ospf_process *p_process = p_if->p_process;
    struct ospf_area *p_area = p_if->p_area;   
   
    switch (type) {
        case OSPF_LS_AS_EXTERNAL:
             return &p_process->t5_lstable;
             
        case OSPF_LS_TYPE_11:
             return &p_process->t11_lstable;
             
        case OSPF_LS_TYPE_9:
             return &p_if->opaque_lstable;
             
        default:
   //  	printf("%s:%d: type=%d,ls_table=%p.\n",__FUNCTION__,__LINE__,type,p_area->ls_table[type]);
             return p_area->ls_table[type];
    }
    return NULL;
}

/*fill scope according to instance,area, if and type*/
struct ospf_lstable *
ospf_lsa_scope_to_lstable(
            struct ospf_process *p_process,
            struct ospf_area *p_area,
            struct ospf_if *p_if,
            u_int lstype)
{
    if ((OSPF_LS_TYPE_9 == lstype) && p_if)
    {
        return &p_if->opaque_lstable;
    }
    else if ((OSPF_LS_AS_EXTERNAL != lstype)
             && (OSPF_LS_TYPE_11 > lstype)
             && p_area
             && p_area->ls_table[lstype])
    {
        return p_area->ls_table[lstype];
    }
    else
    {
        return ((OSPF_LS_AS_EXTERNAL == lstype) && p_process) ? &p_process->t5_lstable : &p_process->t11_lstable;
    }
    return NULL;
}

/*force to originate all self originated lsa,called when restart&helper finished*/
void 
ospf_lsa_force_refresh(struct ospf_process *p_process)
{   
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_nextlsa = NULL;
    struct ospf_lstable *p_table = NULL;
    struct ospf_lstable *p_next = NULL;

    /*set self lsa's age to refresh interval*/
    for_each_node(&p_process->lstable_table, p_table, p_next)
    {
        for_each_node(&p_table->list, p_lsa, p_nextlsa)
        {
            if ((OSPF_LS_REFRESH_TIME > ntohs(p_lsa->lshdr->age)) 
                && (p_process->router_id == ntohl(p_lsa->lshdr->adv_id)))
            {
                p_lsa->lshdr->age = htons(OSPF_LS_REFRESH_TIME);
                p_lsa->update_time = ospf_sys_ticks();
            }
        }
    }
    /*start lsa aging check timer using short interval*/
    ospf_stimer_safe_start(&p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);
    return;
}

/*check lsa's header,decide if valid,called when db&upate input*/
u_int 
ospf_lshdr_is_valid(struct ospf_lshdr *p_lshdr)
{
    /*router id must non-zero*/
    if (0 == p_lshdr->adv_id)
    {
        ospf_logx(ospf_debug_lsa, "invalid zero router in lsa");
        return FALSE;
    }
    
    /*special case for router lsa,it's id and router must be same,testing from ixia*/
    if ((OSPF_LS_ROUTER == p_lshdr->type)
          && (p_lshdr->id != p_lshdr->adv_id))
    {
        ospf_logx(ospf_debug_lsa, "router lsa has different id and router id");
        return FALSE;
    }

    /*lsa type 1,2,4,id must non-zero*/
    if ((OSPF_LS_ROUTER == p_lshdr->type)
        || (OSPF_LS_NETWORK == p_lshdr->type)
        || (OSPF_LS_SUMMARY_ASBR == p_lshdr->type))
    {
        if (0 == p_lshdr->id)
        {
            ospf_logx(ospf_debug_lsa, "invalid zero id of lsa");
            return FALSE;
        }
    }
    if ((OSPF_LS_ROUTER > p_lshdr->type)
        ||((OSPF_LS_TYPE_11 < p_lshdr->type)))
    {
         ospf_logx(ospf_debug_lsa, "invalid lstype");
         return FALSE;
    }
    return TRUE;
}

/*conflict network*/
int 
ospf_conflict_network_cmp(
            struct ospf_conflict_network *p1,
            struct ospf_conflict_network *p2)
{
    OSPF_KEY_CMP(p1, p2, network);    
    return 0;
}

struct ospf_conflict_network *  
ospf_conflict_network_lookup(
            struct ospf_lstable *p_table,
            u_int network)
{
    struct ospf_conflict_network ospf_confilict_net;  
     
    ospf_confilict_net.network = network;
    return ospf_lstlookup(&p_table->conflict_list, &ospf_confilict_net);
}   

struct ospf_conflict_network * 
ospf_conflict_network_add(
                struct ospf_lstable *p_table,
                u_int network)
{
    struct ospf_conflict_network *p_conflict_net = NULL;

    p_conflict_net = ospf_conflict_network_lookup(p_table, network);
    if (NULL == p_conflict_net)
    {    
        p_conflict_net = ospf_malloc(sizeof(struct ospf_conflict_network), OSPF_MPACKET);
        if (NULL == p_conflict_net)
        {
            return NULL;
        }
        p_conflict_net->network = network;        
        ospf_lstadd(&p_table->conflict_list, p_conflict_net);
    }
    return p_conflict_net;
}

void 
ospf_conflict_network_delete(
                struct ospf_lstable *p_table,
                u_int network)
{
    struct ospf_conflict_network *p_conflict_net = NULL;

    p_conflict_net = ospf_conflict_network_lookup(p_table, network);

    if (NULL != p_conflict_net)
    {
        ospf_lstdel(&p_table->conflict_list, p_conflict_net);
        ospf_mfree(p_conflict_net, OSPF_MPACKET);
    }
    return;
}

/*check if there are conflict lsa id for new orginated lsa,if so,change lsa id*/
void 
ospf_lsa_id_conflict_check (
                  struct ospf_lstable *p_table,
                  struct ospf_lshdr *p_lshdr)
{     
    struct ospf_summary_lsa *p_summary = NULL;
    struct ospf_summary_lsa new_summary;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_other_lsa = NULL;     
    u_int network = ntohl(p_lshdr->id);
    u_int mask = 0;
    u_int other_mask = 0;
    u_int flush = FALSE;
    u_int i = 0;
     
    /*only type3,5,7 need*/
    if ((OSPF_LS_SUMMARY_ASBR != p_lshdr->type)
        && (OSPF_LS_AS_EXTERNAL != p_lshdr->type)
        && (OSPF_LS_TYPE_7 != p_lshdr->type))
    {
        return;
    }

    /*if lsid is not network,do not check it*/
    p_summary = (struct ospf_summary_lsa *)p_lshdr;
    mask = ntohl(p_summary->mask);

    if (network != (network & mask))
    {
        return;
    }

    /*decide lsa operation,if age is max,will delete this lsa*/
    if (OSPF_MAX_LSAGE <= ntohs(p_lshdr->age))
    {
        flush = TRUE;
    }
    
    p_lsa = ospf_lsa_lookup(p_table, p_lshdr);
    /*if lsa with network is null,do nothing*/
    if (NULL == p_lsa)
    {
        return;
    }        

    p_summary = (struct ospf_summary_lsa *)p_lsa->lshdr;
    other_mask = ntohl(p_summary->mask);

    /*lsa add operation*/
    if (FALSE == flush)
    {
        /*mask is same as input,do not more work need*/
        if (other_mask == mask)
        {
            return;
        }
        /*mask is not same,conflict detected,add conflict mask*/
        ospf_conflict_network_add(p_table, network);
        
        /*modify lsid to network+hostbit,we will originate new lsa*/
        p_lshdr->id = htonl(network | (~mask));
        return;
    }
    else/*lsa delete operation*/
    {
        /*search conflict table,if not exist,do nothing*/        
        if (NULL == ospf_conflict_network_lookup(p_table, network))
        {
            return;
        }
        /*if lsa's mask different*/
        if (other_mask != mask)
        {
            /*modify input lsa's id to network+hostbit*/
            p_lshdr->id = htonl(network|(~mask));
            return;
        }

        /*mask same*/
        /*get another mask in conflict table,lsa must exist*/
        memcpy(&new_summary, p_lshdr, sizeof(new_summary));        
        other_mask = 0;
        /*ignore all 1's mask*/
        for (i = 1 ; i < 32 ; i++)
        {            
            other_mask = OSPF_HOST_MASK << i;
            if ((mask != other_mask) && ((other_mask & network) == network))
            {                
                new_summary.h.id = htonl(network|(~other_mask));

                p_other_lsa = ospf_lsa_lookup(p_table, &new_summary.h);
                if (NULL != p_other_lsa)
                {
                    break;
                }
            }
        }
         
        /*other lsa not exist, do nothing*/
        if (NULL == p_other_lsa)
        {
            return;
        }

        /*replace current lsa body by this other lsa,but do not change header*/
        memcpy(p_lsa->lshdr + 1, p_other_lsa->lshdr + 1, ntohs(p_other_lsa->lshdr->len) - OSPF_LSA_HLEN);

        /*sechdule to originate it*/
        ospf_lsa_refresh(p_lsa);

        /*delete the other lsa*/
        p_lshdr->id = p_other_lsa->lshdr->id;
    }
    return; 
}

/*conflict network checking timer*/
void
ospf_conflict_network_update(
           struct ospf_lstable *p_table,
           struct ospf_conflict_network *p_conflict)
{
    struct ospf_process *p_process = p_table->p_process;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_normal_lsa = NULL;
    struct ospf_lsa *p_mask_lsa[32];
    struct ospf_external_lsa summary;
    u_int i = 0;
    u_int lscount = 0;
    u_int mask = 0;
 
    memset(&summary, 0, sizeof(summary));
    memset(p_mask_lsa, 0, sizeof(p_mask_lsa));
 
    /*lsa table is empty,delete conflict network*/
    p_lsa = ospf_lstfirst(&p_table->list);
    if (NULL == p_lsa)
    {
        ospf_conflict_network_delete(p_table, p_conflict->network);
        return;
    }
 
    summary.h.type = p_lsa->lshdr->type;
    summary.h.adv_id = htonl(p_process->router_id);
 
    /*search lsa with network*/
    summary.h.id = htonl(p_conflict->network);
    p_normal_lsa = ospf_lsa_lookup(p_table, &summary.h);
    
    /*search all possible lsa with this network*/
    /*ignore all 1's mask*/
    for (i = 1 ; i < 32 ; i++)
    {
        mask = OSPF_HOST_MASK << i;
        if ((mask & p_conflict->network) == p_conflict->network)
        {
            summary.h.id = htonl(p_conflict->network | (~mask));
            p_lsa = ospf_lsa_lookup(p_table, &summary.h);
            if (p_lsa)
            {
                p_mask_lsa[lscount++] = p_lsa;               
            }
            if (lscount > 1)
            {
                break;
            }
        }
    }
 
    /*if not any detailed lsa exist,delete this entry*/
    if (0 == lscount)
    {
        ospf_conflict_network_delete(p_table, p_conflict->network);
        return;
    }
 
    /*if normal lsa exist,do nothing*/
    if (p_normal_lsa)
    {
        return;
    }
 
    /*normal lsa not exist,select a detailed lsa to replace*/
    /*copy lsa's body to new buffer,and set lsid to network*/
    memcpy(&summary, p_mask_lsa[0]->lshdr, sizeof(summary));
    summary.h.id = htonl(p_conflict->network);
    ospf_local_lsa_install(p_table, &summary.h);
 
    /*try to age current lsa*/
    ospf_lsa_maxage_set(p_mask_lsa[0]);
    
    /*if only one detailed lsa exist,delete thie entry*/
    if (1 == lscount)
    {
        ospf_conflict_network_delete(p_table, p_conflict->network);
    }
    return;
}

void
ospf_conflict_network_table_update(struct ospf_lstable *p_table)
{
   struct ospf_conflict_network *p_conflict = NULL;
   struct ospf_conflict_network *p_next = NULL;

   for_each_node(&p_table->conflict_list, p_conflict, p_next)
   {
       ospf_conflict_network_update(p_table, p_conflict);
   }
   return;
}

void
ospf_lsa_check_timer_expired(struct ospf_process *p_process)
{
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next_lsa = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    u_int originate;
    u_int count = 0;

    if (p_process->in_restart == TRUE)
    {
        ospf_stimer_start(&p_process->lsa_check_timer, OSPF_LSA_CHECK_TIME);
        return;
    }
    
    for_each_node(&p_process->area_table, p_area, p_next_area)
    {
        originate = TRUE;
        for_each_router_lsa(p_area, p_lsa, p_next_lsa)
        {
           if (ospf_is_self_lsa(p_process, p_lsa->lshdr))
           {
               originate = FALSE;
           }
        }
        if (originate)
        {
            ospf.stat.check_router_lsa_count++;
            ospf_router_lsa_originate(p_area);
        }
    }
    
    for_each_ospf_if(p_process, p_if, p_next_if)
    {
        /*ignore non-dr interface*/
        if (p_if->addr != p_if->dr)
        {
            continue;
        }
        /*ignore interface without full nbr*/
        count = 0;
        for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
        {
            if ((OSPF_NS_FULL == p_nbr->state) || p_nbr->in_restart)
            {
                count++;
                break;
            }
        }
        if (0 == count)
        {
            continue;
        }
        originate = TRUE;
        for_each_node(&p_if->p_area->ls_table[OSPF_LS_NETWORK]->list, p_lsa, p_next_lsa)
        {
            if (!ospf_is_self_lsa(p_process, p_lsa->lshdr))
            {
                continue;
            }
            if (p_if->addr == ntohl(p_lsa->lshdr->id))
            {
                originate = FALSE;
            }
        }
        if (originate)
        {
            ospf.stat.check_network_lsa_count++;
            ospf_network_lsa_originate(p_if);
        }
    }
    ospf_logx(ospf_debug_lsa, "lsa check timer expired");
    
    ospf_stimer_start(&p_process->lsa_check_timer, OSPF_LSA_CHECK_TIME);
    
    return;
}
void ospf_ospf_lshdr_print(struct ospf_lshdr *p_lshdr)
{
	ospf_logx(ospf_debug_lsa,"##ospf_ospf_lshdr_print p_lshdr 0x%x:##\n",p_lshdr);
	ospf_logx(ospf_debug_lsa,"##age %d,option %d,type %d,id 0x%x,adv_id 0x%x,seqnum %d,checksum %d,len %d##\n",
		ntohs(p_lshdr->age),p_lshdr->option,p_lshdr->type,p_lshdr->id,p_lshdr->adv_id,p_lshdr->seqnum,
		p_lshdr->checksum,p_lshdr->len);
	
	ospf_logx(1,"##ospf_ospf_lshdr_print p_lshdr 0x%x:##\n",p_lshdr);
	ospf_logx(1,"##age %d,option %d,type %d,id 0x%x,adv_id 0x%x,seqnum %d,checksum %d,len %d##\n",
		ntohs(p_lshdr->age),p_lshdr->option,p_lshdr->type,p_lshdr->id,p_lshdr->adv_id,p_lshdr->seqnum,
		p_lshdr->checksum,p_lshdr->len);
}
void ospf_router_link_print(struct ospf_router_link *p_link)
{
	ospf_logx(ospf_debug_lsa,"##ospf_router_link_print p_link 0x%x:##\n",p_link);
	ospf_logx(ospf_debug_lsa,"##  id 0x%x, data 0x%x, type %d,tos_count :%d,tos0_metric:%d ##\n", 
		p_link->id,p_link->data,p_link->type,p_link->tos_count,p_link->tos0_metric);


}

void ospf_router_lsa_print(struct ospf_router_lsa *p_router)
{
	ospf_logx(ospf_debug_lsa,"##ospf_router_lsa_print p_router 0x%x:##\n",p_router);
	ospf_ospf_lshdr_print(&p_router->h);
	ospf_logx(ospf_debug_lsa,"##flag %d,rsvd %d,link_count %d##\n",p_router->flag,p_router->rsvd,p_router->link_count);
	ospf_router_link_print(p_router->link);
}


/*ucCostFlg --1 max cost;0 normal cost*/
void 
ospf_router_hold_cost_lsa(struct ospf_if *p_if, u_char ucCostFlg)
{
    u_int8 buf[OSPF_MAX_TXBUF];
    struct ospf_router_lsa *p_router = NULL;
    struct ospf_area *p_area = p_if->p_area;
    struct ospf_process *p_process = p_if->p_process;
    u_int if_count = (OSPF_MAX_TXBUF - OSPF_ROUTER_LSA_HLEN)/OSPF_ROUTER_LINK_LEN;
    u_int dynamic_buffer = FALSE;
    u_int now = ospf_sys_ticks();
    u_int8 state_mask[4] = {0};
    
    ospf_logx(ospf_debug_lsa, "originate router lsa for area %u", p_area->id);

    /*set state mask,check exchange or loading nbr*/
    BIT_LST_SET(state_mask, OSPF_NS_EXCHANGE);
    BIT_LST_SET(state_mask, OSPF_NS_LOADING);

    p_area->last_routelsa_time = now;

    /*big packet:if lsa length exceed current buffer, allocate larger buffer*/
    /*sub 10 for safe.*/
    if (ospf_lstcnt(&p_process->if_table) >= (if_count - 10))
    {
        u_int ulLen = 0;

        ulLen = (OSPF_BIGPKT_BUF/sizeof(struct ospf_router_lsa))*sizeof(struct ospf_router_lsa);
        dynamic_buffer = TRUE;

        p_router = ospf_malloc(OSPF_BIGPKT_BUF, OSPF_MPACKET);
        if (NULL == p_router)
        {
            return;
        }
    }
    else
    {
        p_router = (struct ospf_router_lsa *)buf;
    }

    memset(p_router, 0, sizeof(struct ospf_router_lsa));
    ospf_router_lsa_build(p_area, p_router);
	
   // ospf_router_lsa_print(p_router);//新加打印
    ospf_local_lsa_install(p_area->ls_table[p_router->h.type], &p_router->h);

    /*big packet*/
    if (TRUE == dynamic_buffer)
    {
        ospf_mfree(p_router, OSPF_MPACKET);
    }
    return;
}




