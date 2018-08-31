/* ospf_nbr.c - OSPF API for neighbor operation,containing lookup,dbsummary,request,retransmit...*/

#include "ospf.h"
#include "ospf_nm.h"

void ospf_nbr_rxmt_list_flush (struct ospf_nbr *p_nbr);
void ospf_nbr_slavehold_timeout(struct ospf_nbr *p_nbr);
void ospf_nbr_dd_timeout(struct ospf_nbr *p_nbr);
void ospf_nbr_request_timeout(struct ospf_nbr *p_nbr);
void ospf_nbr_retransmit_timeout(struct ospf_nbr *p_nbr);
void ospf_nbr_error_timeout(struct ospf_nbr *p_nbr);
void ospf_nbr_reject_timeout(struct ospf_nbr *p_nbr);

extern int log_time_print (u_int8 *buf);

/*neighbor struct operation*/
/*lookup neighbor according to internal index*/
struct ospf_nbr *
ospf_nbr_lookup_by_index(
     struct ospf_process *p_process,
     u_int idx)
{
    struct ospf_nbr nbr;
    nbr.index = idx;
    return ospf_lstlookup(&p_process->nbr_table, &nbr);
}
/*vlink peer state*/
u_int
ospf_vif_nbr_state_get(struct ospf_if *p_if, u_int nbr, u_int *lval)
{
    struct ospf_nbr *p_nbr=NULL, *p_next=NULL;
    for_each_node(&p_if->nbr_table, p_nbr, p_next)
    {
        if(p_nbr->id == nbr)
        {
            *lval = p_nbr->state;
            return 0;
        }
    }
    return -1;
}


/*select new index for a new created neighbor*/
u_int 
ospf_nbr_index_allocate(struct ospf_process *p_process)
{
    u_int i = 0;
 
    for (i = 1 ; i < OSPF_MAX_NBR_INDEX ; i++)
    {
        if (!ospf_nbr_lookup_by_index(p_process, i))
        {
           return i;
        }
    }
    return 0;
}

/*create nbr from hello msg*/
struct ospf_nbr *
ospf_nbr_create(
            struct ospf_if *p_if, 
            u_int source)
{
    struct ospf_nbr *p_nbr = NULL,*p_next_nbr = NULL; 
    struct ospf_nbr search_nbr;
	int iFlag = 0;

    ospf_logx(ospf_debug_nbr, "create new neighbor:%d",__LINE__);  
  	//ospf_logx(ospf_debug_nbr,"#%s:%d,source=%x.\r\n",__FUNCTION__,__LINE__,source);

/*˫��������*/
#ifdef OSPF_DCN
	/*20140115 modify for vrrp, two interface rcv same nbr*/
    memset(&search_nbr, 0, sizeof(search_nbr));
    search_nbr.p_if = p_if;
    search_nbr.addr = source;
    p_nbr = ospf_lstlookup(&ospf.nm.nbr_table, &search_nbr);
   // printf("#%s:%d,source=%x,p_nbr=%x.\r\n",__FUNCTION__,__LINE__,source,p_nbr);
    if ((NULL != p_nbr)&&(p_nbr->p_if->type != OSPF_IFT_P2MP))
    {
	//	ospf_logx(1," ifnet_uint:%x, addr:%x,id:%x\n",p_nbr->p_if->ifnet_uint,p_nbr->addr,p_nbr->id);
		if(p_if->p_process->ulDcnFlag != OSPF_DCN_FLAG)
		{
	        if((p_nbr->p_if->ifnet_uint != p_if->ifnet_uint)
	            &&((p_nbr->p_if->link_up == FALSE)
	                ||(DCN_RECV_OK != ospf_dcn_get_rx_pkt_flag(p_nbr->p_if->ifnet_uint,p_nbr->p_if->addr))))
	        {
	            ospf_logx(ospf_debug_nbr,"nbr already create in other interface, delete nbr \n");
	          //  ospf_logx(1,"ospf nbr already create in other interface, delete nbr \n");
	            ospf_nsm(p_nbr, OSPF_NE_LL_DOWN);
	            ospf_nbr_delete(p_nbr);
	            p_nbr = NULL; 
	        }
	        else
	        {
				ospf_logx(ospf_debug_nbr,"nbr already create in other interface, create nbr failed \n");
	          //  ospf_logx(1,"ospf nbr already create in other interface, create nbr failed \n");
				iFlag = 1;
	        }
		}
		else
		{
		
			ospf_logx(0,"p_nbr:ifnet_uint:%x,  p_if:ifnet_uint:%x, addr:%x,id:%x\n",
				p_nbr->p_if->ifnet_uint,p_if->ifnet_uint,p_nbr->addr,p_nbr->id);
			if(p_nbr->p_if->ifnet_uint == p_if->ifnet_uint)
			{
				ospf_logx(ospf_debug_nbr,"nbr already create in other interface, create nbr failed \n");
	            ospf_logx(0,"dcn nbr already create in other interface, create nbr failed \n");
				iFlag = 1;
	        }
			
		}
    }
#endif
	for_each_node(&ospf.nm.nbr_table, p_nbr, p_next_nbr)
    {   
        if (NULL == p_nbr)
        {
            continue;
        }
        if ((NULL == p_nbr->p_if) || (p_nbr->p_if->type == OSPF_IFT_P2MP))
        {
            continue;
        }
		if(p_nbr->addr != source)
		{
            continue;
        }
        if(p_nbr->p_if->p_process->process_id != p_if->p_process->process_id)
        {
            continue;
        }
        #ifdef OSPF_DCN
		if(p_if->p_process->ulDcnFlag != OSPF_DCN_FLAG)/*caoyong�޸ģ�������dcn���,����ulDcnFlag*/
		#else
		if(1)
		#endif
		{
	        if((p_nbr->p_if->ifnet_uint != p_if->ifnet_uint)
	            &&((p_nbr->p_if->link_up == FALSE)))
	                //||(DCN_RECV_OK != ospf_dcn_get_rx_pkt_flag(p_nbr->p_if->ifnet_uint,p_nbr->p_if->addr))))
	        {
	            ospf_logx(ospf_debug_nbr,"nbr already create in other interface, delete nbr \n");
	            ospf_nsm(p_nbr, OSPF_NE_LL_DOWN);
	            ospf_nbr_delete(p_nbr);
	            p_nbr = NULL; 
	        }
	        else
	        {
				ospf_logx(ospf_debug_nbr,"nbr already create in other interface, create nbr failed \n");
				return p_nbr;
	        }
		}
		else
		{
		
			ospf_logx(0,"p_nbr:ifnet_uint:%x,  p_if:ifnet_uint:%x, addr:%x,id:%x\n",
				p_nbr->p_if->ifnet_uint,p_if->ifnet_uint,p_nbr->addr,p_nbr->id);
			if(p_nbr->p_if->ifnet_uint == p_if->ifnet_uint)
			{
				ospf_logx(ospf_debug_nbr,"nbr already create in other interface, create nbr failed \n");
				return p_nbr;
	        }
			
		}

    }

    #ifdef OSPF_DCN
	/*ɾ��DCN�ӿ������޹��ھӣ�overlayģ�ͳ���*/
	if((p_if->p_process->ulDcnFlag == OSPF_DCN_FLAG)
		&&(p_if->uiOverlayflag == 0)) 
	{
		p_nbr = ospf_lstfirst(&p_if->nbr_table);
		if (NULL != p_nbr)
		{
			ospf_nsm(p_nbr, OSPF_NE_LL_DOWN);
			ospf_nbr_delete(p_nbr);
			p_nbr = NULL; 

		}
	}
    #endif
	
    p_nbr = ospf_malloc2(OSPF_MNBR);
    if (NULL == p_nbr)
    {
        return NULL;
    }
    
    memset(p_nbr, 0, sizeof(struct ospf_nbr));
    p_nbr->addr = source;
    p_nbr->p_if = p_if;
//	ospf_logx(ospf_debug_nbr,"%s %d p_nbr=0x%p, p_nbr->addr 0x%x, p_nbr->p_if 0x%x,p_if=0x%p, p_if->addr 0x%x,p_if->ifnet_uint=0x%x.\n", __FUNCTION__,__LINE__,p_nbr, p_nbr->addr, p_nbr->p_if,p_if, p_if->addr,p_if->ifnet_uint);
    p_nbr->state = OSPF_NS_DOWN;
    p_nbr->restart_exitreason = OSPF_RESTART_NONE;      
       
    if (OSPF_IFT_VLINK == p_if->type)
    {
        p_nbr->id = p_if->nbr;
    }
    /*select new index for this new neighbor,do not consider failure now*/
    p_nbr->index = ospf_nbr_index_allocate(p_if->p_process);
    if (0 == p_nbr->index)
    { 
    	
		//ospf_logx(ospf_debug_nbr,"#%s:%d,source=%x.\r\n",__FUNCTION__,__LINE__,source);
		ospf_logx(ospf_debug_nbr, "create new neighbor source=%x",source); 
        ospf_mfree(p_nbr, OSPF_MNBR);
        return NULL;
    }
	
	//ospf_logx(ospf_debug,"#%s:%d,source=%x.\r\n",__FUNCTION__,__LINE__,source);
   // ospf_logx(ospf_debug_nbr, "create new neighbor:%d",__LINE__); 
    /*add to process nbr table*/
    ospf_lstadd(&p_if->p_process->nbr_table, p_nbr); 
	//ospf_logx(ospf_debug,"#%s:%d,source=%x.\r\n",__FUNCTION__,__LINE__,source);
   // ospf_logx(ospf_debug_nbr, "create new neighbor:%d",__LINE__);  
    /*add to if nbr table*/
    ospf_lstadd(&p_if->nbr_table, p_nbr);
	//ospf_logx(ospf_debug,"#%s:%d,source=%x.\r\n",__FUNCTION__,__LINE__,source);
    ospf_logx(ospf_debug_nbr, "create new neighbor:%d",__LINE__); 
    /*if not vnbr,add to nm nbr table*/
    if (OSPF_IFT_VLINK != p_if->type)
    {
    	//ospf_logx(ospf_debug,"#%s:%d,source=%x.\r\n",__FUNCTION__,__LINE__,source);
		ospf_logx(ospf_debug_nbr, "create new neighbor:%d",__LINE__); 
        ospf_lstadd(&ospf.nm.nbr_table, p_nbr);
    }
    /*init timer of nbr*/    
    ospf_timer_init(&p_nbr->slavehold_timer, p_nbr, ospf_nbr_slavehold_timeout, p_if->p_process); 
    ospf_timer_init(&p_nbr->dd_timer, p_nbr, ospf_nbr_dd_timeout, p_if->p_process); 
    ospf_timer_init(&p_nbr->request_timer, p_nbr, ospf_nbr_request_timeout, p_if->p_process); 
    ospf_timer_init(&p_nbr->lsrxmt_timer, p_nbr, ospf_nbr_retransmit_timeout, p_if->p_process); 
    ospf_timer_init(&p_nbr->restart_timer, p_nbr, ospf_restart_helper_timeout, p_if->p_process); 
    ospf_timer_init(&p_nbr->hold_timer, p_nbr, ospf_nbr_inactive_timer, p_if->p_process); 
    ospf_timer_init(&p_nbr->lsa_error_timer, p_nbr, ospf_nbr_error_timeout, p_if->p_process); 
    ospf_timer_init(&p_nbr->reject_nbr_timer, p_nbr, ospf_nbr_reject_timeout, p_if->p_process);
#ifdef HAVE_BFD
    ospf_timer_init(&p_nbr->bfd_timer, p_nbr, ospf_nbr_bfd_timeout, p_if->p_process); 
#endif
    /*start nbr rxmt timer for safe*/
    ospf_timer_start(&p_nbr->lsrxmt_timer, ospf_rand(p_nbr->p_if->rxmt_interval * OSPF_TICK_PER_SECOND));

#ifdef HAVE_BFD
    /*bind bfd*/
    if ((p_if->bfd_enable == OSPF_BFD_GBL_ENABLE) 
        || (p_if->bfd_enable == OSPF_BFD_IF_ENABLE))
    {
        ospf_bind_bfd(p_nbr);
    }
#endif
    if (OSPF_IFT_NBMA == p_if->type)
    {               
        ospf_nsm( p_nbr, OSPF_NE_START);  
    }
    if (OSPF_MODE_MASTER == ospf.work_mode)
    {
        ospf_syn_nbr_send(p_nbr, TRUE, NULL);
    }
    return p_nbr;
}

/*delete neighbor*/
void 
ospf_nbr_delete (struct ospf_nbr* p_nbr)
{
	ospf_logx(ospf_debug_nbr," ospf_nbr_delete strat \r\n");
    if (OSPF_MODE_MASTER == ospf.work_mode)
    {
        ospf_syn_nbr_send(p_nbr, FALSE, NULL);
    }
#ifdef HAVE_BFD
    /*unbind bfd*/
    if(p_nbr->p_if->p_process->ulDcnFlag != OSPF_DCN_FLAG)
    {
        if((p_nbr->p_if->bfd_enable == OSPF_BFD_IF_ENABLE)
            || (p_nbr->p_if->bfd_enable == OSPF_BFD_GBL_ENABLE))
        {
            ospf_unbind_bfd(p_nbr);
        }
    }
#endif
#if 0
	if ((p_nbr->p_if->nbr_table.avl.avl_root == NULL) ||
		(p_nbr->p_if->p_process->nbr_table.avl.avl_root == NULL) ||
		(ospf.nm.nbr_table.avl.avl_root == NULL))
	{
		printf("%s%d p_nbr->p_if->nbr_table.avl.avl_root == NULL", __FUNCTION__,__LINE__);
		//return;
	}
	else
	{
		
	
#endif
	    /*remove from tables*/
	    ospf_lstdel(&p_nbr->p_if->nbr_table, p_nbr);

	    ospf_lstdel(&p_nbr->p_if->p_process->nbr_table, p_nbr);

	    if (OSPF_IFT_VLINK != p_nbr->p_if->type)
	    {
			/*��ֹɾ�ھӶδ���*/
			if(ospf_lstlookup(&ospf.nm.nbr_table, p_nbr) != NULL)
			{
				ospf_lstdel(&ospf.nm.nbr_table, p_nbr);
			}
	    }
	//	ospf_logx(ospf_debug_nbr,"%s %d p_nbr=0x%p, p_nbr->addr 0x%x, p_nbr->p_if 0x%x,p_if->addr 0x%x,p_if->ifnet_uint=0x%x.\n", __FUNCTION__,__LINE__,p_nbr, p_nbr->addr, p_nbr->p_if,p_nbr->p_if->addr,p_nbr->p_if->ifnet_uint);
	//	ospf_logx(ospf_debug_nbr,"ospf_nbr_delete:p_nbr->p_if->nbr_table.avl.avl_root 0x%x\n", p_nbr->p_if->nbr_table.avl.avl_root);
	//	ospf_logx(ospf_debug_nbr,"p_nbr->p_if->p_process->nbr_table.avl.avl_root 0x%x\n", p_nbr->p_if->p_process->nbr_table.avl.avl_root);
	//	ospf_logx(ospf_debug_nbr,"ospf.nm.nbr_table.avl.avl_root 0x%x\n", ospf.nm.nbr_table.avl.avl_root);
	//}
    /*delete retransmit information*/
    ospf_nbr_rxmt_list_flush(p_nbr); 

    /*delete database summary information*/    
    ospf_nbr_dd_list_flush(p_nbr); 

    /*delete request information*/    
    ospf_nbr_request_list_flush(p_nbr);  

    /*stop all timers*/
    ospf_timer_stop(&p_nbr->lsrxmt_timer);
    ospf_timer_stop(&p_nbr->dd_timer);
    ospf_timer_stop(&p_nbr->request_timer);
    ospf_timer_stop(&p_nbr->hold_timer);
    ospf_timer_stop(&p_nbr->slavehold_timer);        
    ospf_timer_stop(&p_nbr->restart_timer);      
    ospf_timer_stop(&p_nbr->lsa_error_timer);        
    ospf_timer_stop(&p_nbr->reject_nbr_timer); 
#ifdef HAVE_BFD
    ospf_timer_stop(&p_nbr->bfd_timer); 
#endif
    
    ospf_mfree(p_nbr, OSPF_MNBR);
	ospf_logx(ospf_debug_nbr," ospf_nbr_delete end \r\n");
    return;
}

/*nbr compare ,just use address,not id*/
int 
ospf_nbr_lookup_cmp(
                struct ospf_nbr *p1, 
                struct ospf_nbr *p2)
{
    OSPF_KEY_CMP(p1, p2, addr);
    return 0;
}

int 
ospf_nbr_nm_cmp(
                struct ospf_nbr *p1, 
                struct ospf_nbr *p2)
{
    OSPF_KEY_CMP(p1, p2, p_if->p_process->process_id);
    OSPF_KEY_CMP(p1, p2, addr);    
    return 0;
}

void 
ospf_nbr_table_init(struct ospf_if *p_if)
{
    ospf_lstinit(&p_if->nbr_table, ospf_nbr_lookup_cmp);
    return;
}

/*nbr compare using local index*/
int 
ospf_nbr_index_lookup_cmp(
      struct ospf_nbr *p1, 
      struct ospf_nbr*p2)
{
    OSPF_KEY_CMP(p1, p2, index);
    return 0;
}

void 
ospf_process_nbr_table_init(struct ospf_process *p_process)
{
    ospf_lstinit2(&p_process->nbr_table, ospf_nbr_index_lookup_cmp, mbroffset(struct ospf_nbr, index_node));
    return;
}

/*get ospf neighbor on normal interface*/
struct ospf_nbr* 
ospf_nbr_lookup(
              struct ospf_if *p_if,
              u_int ip_address)
{
    struct ospf_nbr search;
    search.addr = ip_address;
    return ospf_lstlookup(&p_if->nbr_table, &search); 
}

/*request related*/
/*compare REQUEST route for lookup*/
int 
ospf_request_lookup_cmp(
     struct ospf_request_node *p1, 
     struct ospf_request_node *p2)
{
    /*��֤��ɢ��Χ��ͬ*/
    OSPF_KEY_CMP(p1, p2, p_lstable);
 
    /*compare id and advid,type,use network byte order*/    
    OSPF_KEY_CMP(p1, p2, ls_hdr.type);
    OSPF_KEY_HOST_CMP(p1, p2, ls_hdr.id);
    OSPF_KEY_HOST_CMP(p1, p2, ls_hdr.adv_id);
    return 0;
}

struct ospf_request_node *
ospf_request_lookup(
            struct ospf_lstable *p_lstable,
            struct ospf_lshdr *p_lshdr) 
{
    struct ospf_request_node req; 

    req.p_lstable = p_lstable;
    req.ls_hdr.type = p_lshdr->type;
    req.ls_hdr.id = p_lshdr->id;
    req.ls_hdr.adv_id = p_lshdr->adv_id;
    return ospf_lstlookup(&p_lstable->p_process->req_table, &req);
}

/*Request list*/
/*when receive database packet,if databse is more recent ,call this 
function to add database to neighbor request list.if request node
is already added,ignore or update
Called by ospf_parse_database_packet
Input params already checked
*/
void 
ospf_request_add(
            struct ospf_nbr *p_nbr,
            struct ospf_lshdr *p_lshdr)
{
    struct ospf_request_node *p_request = NULL;
    struct ospf_lstable *p_table = ospf_lstable_lookup(p_nbr->p_if, p_lshdr->type);
    
    ospf_logx(ospf_debug_nbr, "accept rcvd lsa,add request");  
    
    /*if same lsa exist:same type,id,advid,do not add more,but may change it*/    
    p_request = ospf_request_lookup(p_table, p_lshdr);
    if (NULL != p_request) 
    {       
        /*select newer lsa as requested one*/
        if (p_nbr != p_request->p_nbr)
        {
            if ((p_table == p_request->p_lstable)
                && (0 <= ospf_lshdr_cmp (&p_request->ls_hdr, p_lshdr)))
            {
                ospf_logx(ospf_debug_nbr, "do not add duplicate request node");
                return;
            }
            else
            {
                ospf_request_delete(p_request);
            }     
        }/*select larger checksum*/
        else 
        {
            if (ntohs(p_lshdr->checksum) > ntohs(p_request->ls_hdr.checksum))
            {
                p_request->ls_hdr.checksum = p_lshdr->checksum;
                p_request->ls_hdr.age = p_lshdr->age;
            }
            return;
        }
    }
    /*create new request node*/
    p_request = ospf_malloc2(OSPF_MREQUEST);
    if (NULL != p_request)
    {
        p_request->p_nbr = p_nbr;
        p_request->p_lstable = p_table;
        p_nbr->req_count++;
        memcpy(&p_request->ls_hdr, p_lshdr, sizeof(struct ospf_lshdr));
        ospf_lstadd(&p_nbr->p_if->p_process->req_table, p_request);
    }
    return;
}    

/*remove a request node from neighbor's request list,and free 
this node,trigger neighbor event.
before calling this function,request node and neighbor have
existed relationship,so we do not check any more
*/
void 
ospf_request_delete(struct ospf_request_node *p_req)
{
    struct ospf_nbr *p_nbr = p_req->p_nbr;
   
    p_nbr->lsa_reply++;
    p_nbr->req_count--;
    ospf_lstdel_free(&p_nbr->p_if->p_process->req_table, p_req, OSPF_MREQUEST);

    /*exchange done:no more request exist*/ 
    if ((OSPF_NS_LOADING == p_nbr->state)&& (0 == p_nbr->req_count))
    {
        /*schedule nsm ,do not perform here*/
        ospf_timer_start(&p_nbr->request_timer, 1);
    }
    return;
}

/*ospf_nbr_dd_list_build - initializes summary table for nbr exchange*/
STATUS 
ospf_nbr_dd_list_build(struct ospf_nbr *p_nbr)
{
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next = NULL;
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_lstable *lslist[OSPF_LS_TYPE_11 + 1] = {NULL};
    struct ospf_process *p_process = p_nbr->p_if->p_process;
    u_int count = 0;
    u_int i = 0;
    u_int opaque_support = FALSE;
    u_int8 nbr[16];


#ifdef OSPF_DCN
	if(p_process->ulDcnFlag == OSPF_DCN_FLAG)
	{
		p_process->opaque = TRUE;
	}
#endif

    /*decide opaque capability*/
    if (p_process->opaque && p_nbr->opaque_enable)
    {
        opaque_support = TRUE;
    }
    /*clear old information*/ 
    ospf_nbr_request_list_flush(p_nbr);
    ospf_nbr_dd_list_flush(p_nbr);    

    /*scan for area lsa*/
    for (i = OSPF_LS_ROUTER;  i <= OSPF_LS_TYPE_10;  ++i)  
    {
        if (NULL == p_if->p_area->ls_table[i])
        {
            continue;
        }
        /*ignore nssa lsa in non-area,it it impossible in fact*/
        if ((OSPF_LS_TYPE_7 == i) && (!p_if->p_area->is_nssa))
        {
            continue;
        }
        /*ignore unsupported opaque lsa*/
        if ((FALSE == opaque_support) && (OSPF_LS_TYPE_10 == i))
        {
            continue;
        }       
        lslist[count++] = p_if->p_area->ls_table[i];
    }

    /*add external/t11 lsa table*/
    if ((!p_if->p_area->is_stub) 
        && (!p_if->p_area->is_nssa)
        && (OSPF_IFT_VLINK != p_if->type))
    {
        lslist[count++] = &p_process->t5_lstable;
        if (opaque_support)
        {
            lslist[count++] = &p_process->t11_lstable;
        }
    }

    /*opaque 9 lsa*/
    if (opaque_support)
    {
        lslist[count++] = &p_if->opaque_lstable;
    }

    /*scan for all lsa selected*/
    for (i = 0 ; i < count ; i++)
    {
        for_each_node(&lslist[i]->list, p_lsa, p_next)
        {
            /*if database's age exceed max age,add to retransmit list
              if interface is in fast dd mode and lsa is not self,add retransmit also
             */
         //    ospf_logx(ospf_debug_nbr,"ospf_nbr_dd_list_build ntohs(p_lsa->lshdr->age)%d.\r\n",ntohs(p_lsa->lshdr->age));
            if ((OSPF_MAX_LSAGE <= ntohs(p_lsa->lshdr->age))
                || (p_if->fast_dd_enable && (!ospf_is_self_lsa(p_process, p_lsa->lshdr))))
            {                
                /*add failed,schedule neighbor deleting*/
                if (NULL == ospf_nbr_rxmt_add(p_nbr, p_lsa))        
                {
                    p_nbr->force_down = TRUE;
                    return ERR;
                }
            }
            else
            {           
                if (NULL == ospf_nbr_dd_add(p_nbr, p_lsa))
                {
                    return ERR;
                }          
            }
        }
    }
    
    if (ospf_debug)
    {
        ospf_logx(ospf_debug_nbr, "bulid dbsummary for %s,total dd count %d", ospf_inet_ntoa(nbr, p_nbr->addr), p_nbr->dd_count);
    }
    return OK;
}

/*LS retransmit lisr*/

/**add lsa to nbr's retransmit list*/
struct ospf_retransmit * 
ospf_nbr_rxmt_add(
                  struct ospf_nbr *p_nbr,
                  struct ospf_lsa *p_lsa)
{
    struct ospf_retransmit *p_rxmt = NULL;
    struct ospf_process *p_process = p_nbr->p_if->p_process;

    /*already add to retransmit,do not add again*/
//	ospf_logx(ospf_debug_nbr,"ospf_nbr_rxmt_add,p_rxmt=%p.\r\n",p_lsa->p_rxmt);
    if (ospf_nbr_rxmt_isset(p_lsa->p_rxmt, p_nbr))
    {
        ospf_logx(ospf_debug_nbr, "duplicate retransmit");
        return p_lsa->p_rxmt; 
    }

    /*if retransmit node already exist,but neighbor not on retransmit list,add it*/
    if (NULL == p_lsa->p_rxmt)
    {
        p_rxmt = ospf_malloc2(OSPF_MRXMT);
        if (NULL == p_rxmt)
        {
            return NULL; 
        }
        p_rxmt->p_lsa = p_lsa;
        p_lsa->p_rxmt = p_rxmt;
        ospf_lstadd_unsort(&p_process->rxmt_table, p_rxmt);
    }
    /*set nbr bits*/
    BIT_LST_SET(p_lsa->p_rxmt->rxmt_bits, p_nbr->index);
    ++p_lsa->p_rxmt->rxmt_count;   
    ++p_nbr->rxmt_count;

    /*restart retransmit timer*/
    if (p_nbr->p_if->rxmt_interval)
    {     
        /*if nbr count exceed limit,start timer directly.*/
        if (OSPF_MAX_EXCHANGE_NBR <= ospf_lstcnt(&p_process->nbr_table))
        {
           /* ospf_timer_start(&p_nbr->lsrxmt_timer, ospf_rand(p_nbr->p_if->rxmt_interval * OSPF_TICK_PER_SECOND));
            */
            p_nbr->rxmt_timer_delay = TRUE;
        }
        else
        {
            ospf_timer_try_start(&p_nbr->lsrxmt_timer, ospf_rand(p_nbr->p_if->rxmt_interval * OSPF_TICK_PER_SECOND));
        }
    }
    else/*short value*/
    {
        ospf_timer_try_start(&p_nbr->lsrxmt_timer, 2);
    }
    /*if lsa rxmt add from 0 to 1, send sync msg now*/
    if (1 == p_lsa->p_rxmt->rxmt_count)
    {
        ospf_syn_lsa_rxmt_send(p_lsa, TRUE, NULL);
    }
    return p_lsa->p_rxmt; 
}

/*delete nbr retransmit node*/
void 
ospf_nbr_rxmt_delete(
                  struct ospf_nbr *p_nbr, 
                  struct ospf_retransmit *p_rxmt)
{ 
    struct ospf_process *p_process = p_nbr->p_if->p_process;

    /*no retransmit exist,do nothing*/
    if (!ospf_nbr_rxmt_isset(p_rxmt, p_nbr))
    {
        return;
    }

    /*clear retransmit bit and count*/
    BIT_LST_CLR(p_rxmt->rxmt_bits, p_nbr->index);
	if(0 != p_rxmt->rxmt_count)
	{
		ospf_logx(0,"ospf_nbr_rxmt_delete:%s:%d\n", __LINE__);
    	--p_rxmt->rxmt_count;
	}
	if(0 != p_nbr->rxmt_count)
	{
		ospf_logx(0,"ospf_nbr_rxmt_delete:%s:%d\n", __LINE__);
    	--p_nbr->rxmt_count;
	}


    /*send sync msg for retransmit*/
    if (0 == p_rxmt->rxmt_count)
    {
        ospf_syn_lsa_rxmt_send(p_rxmt->p_lsa, FALSE, NULL);
    }
    
    /*if this retransmit node is not in any retransmit flags,delete it*/
    if ((0 == p_rxmt->dd_count) && (0 == p_rxmt->rxmt_count))
    {
        if (NULL != p_rxmt->p_lsa)
        {
            p_rxmt->p_lsa->p_rxmt = NULL;
            p_rxmt->p_lsa = NULL;
        }
        ospf_lstdel_unsort(&p_process->rxmt_table, p_rxmt);
        ospf_mfree(p_rxmt, OSPF_MRXMT);
    }
    return;
}

/*delete all retransmit of nbr*/
void 
ospf_nbr_rxmt_list_flush(struct ospf_nbr *p_nbr)
{  
    struct ospf_retransmit *p_rxmt = NULL;
    struct ospf_retransmit *p_next = NULL;
    struct ospf_process *p_process = p_nbr->p_if->p_process;

    for_each_node(&p_process->rxmt_table, p_rxmt, p_next)
    {
 //       ospf_logx(ospf_debug_nbr,"%s %d \n", __FUNCTION__,__LINE__);
        ospf_nbr_rxmt_delete(p_nbr, p_rxmt);
    }
    return;
}

/*Neighbor database table*/
/**add lsa to nbr's dd list*/
struct ospf_retransmit * 
ospf_nbr_dd_add(
          struct ospf_nbr *p_nbr,
          struct ospf_lsa *p_lsa)
{
    struct ospf_retransmit *p_dd = NULL;
    struct ospf_process *p_process = p_nbr->p_if->p_process;

    /*prevent duplicated add*/
    if (ospf_nbr_dd_isset(p_lsa->p_rxmt, p_nbr))
    {
        ospf_logx(ospf_debug_nbr, "duplicate dd set");
        return p_lsa->p_rxmt; 
    }

    /*if dd node already exist,but neighbor not on dd list,add it*/
    if (NULL == p_lsa->p_rxmt)
    {
         p_dd = ospf_malloc2(OSPF_MRXMT);
         if (NULL == p_dd)
         {
             return NULL; 
         }
         p_dd->p_lsa = p_lsa;
         p_lsa->p_rxmt = p_dd;
         ospf_lstadd_unsort(&p_process->rxmt_table, p_dd);
    }
    /*set nbr bits*/
    BIT_LST_SET(p_lsa->p_rxmt->dd_bits, p_nbr->index);
    ++p_lsa->p_rxmt->dd_count;   
    ++p_nbr->dd_count;

    return p_lsa->p_rxmt; 
}

/*clear nbr dd node*/
void 
ospf_nbr_dd_delete(
             struct ospf_nbr *p_nbr, 
             struct ospf_retransmit *p_rxmt)
{ 
    struct ospf_process *p_process = p_nbr->p_if->p_process;

    /*no dd exist,do nothing*/
    if (!ospf_nbr_dd_isset(p_rxmt, p_nbr))
    {
        return;
    }
    /*clear database bit and count*/
    BIT_LST_CLR(p_rxmt->dd_bits, p_nbr->index);
    --p_rxmt->dd_count;
    --p_nbr->dd_count;

    /*if this retransmit node is not in any retransmit flags,delete it*/
    if ((0 == p_rxmt->dd_count) && (0 == p_rxmt->rxmt_count))
    {
        if (NULL != p_rxmt->p_lsa)
        {
            p_rxmt->p_lsa->p_rxmt = NULL;		 
            p_rxmt->p_lsa = NULL;
        }
        ospf_lstdel_unsort(&p_process->rxmt_table, p_rxmt);
        ospf_mfree(p_rxmt, OSPF_MRXMT);
    }
    return;
}

/*delete all dd of nbr*/
void 
ospf_nbr_dd_list_flush(struct ospf_nbr *p_nbr)
{  
    struct ospf_retransmit *p_rxmt = NULL;
    struct ospf_retransmit *p_next = NULL;
    struct ospf_process *p_process = p_nbr->p_if->p_process;

    for_each_node(&p_process->rxmt_table, p_rxmt, p_next)
    {
        ospf_nbr_dd_delete(p_nbr, p_rxmt);
    }
    p_nbr->dd_count = 0;

    /*clear dd message buffer*/
    ospf_delete_last_send_dd(p_nbr);

    /*clear next dd hdr*/
    memset(p_nbr->next_dd_hdr, 0, sizeof(p_nbr->next_dd_hdr));
    return;
}

/*neighbor state machine*/
 
/*establish protocol adj,enter exstart state*/
void 
ospf_nbr_adj_establish(struct ospf_nbr *p_nbr)
{
    /*ospf_logx(1,"neighbor %s enter exstart state",ospf_inet_ntoa(nbr_str,p_nbr->addr));*/

    p_nbr->state = OSPF_NS_EXSTART;
    
    /*select nbr's dd seqnum*/
    if (0 == p_nbr->dd_seqnum)
    {
        p_nbr->dd_seqnum = os_system_tick();
//		ospf_logx(ospf_debug_nbr,"%s,%d p_nbr->dd_seqnum=%d\n",__FUNCTION__,__LINE__,p_nbr->dd_seqnum);
    }
    else
    {
        ++p_nbr->dd_seqnum;
//		ospf_logx(ospf_debug_nbr,"%s,%d p_nbr->dd_seqnum=%d\n",__FUNCTION__,__LINE__,p_nbr->dd_seqnum);
    }
    
    /*set i/m/ms flag*/
    p_nbr->dd_flag = 0;
    ospf_set_dd_flag_init(p_nbr->dd_flag);
    ospf_set_dd_flag_more(p_nbr->dd_flag);
    ospf_set_dd_flag_master(p_nbr->dd_flag);
    p_nbr->dd_state = OSPF_CLEAR_MODE;
    p_nbr->empty_dd_need = FALSE;
    p_nbr->empty_dd_rcvd = FALSE;

    /*send init database packet*/
    ospf_initial_dd_output (p_nbr);    

    return;
}

/*clear adjacency related resource*/
void 
ospf_nbr_adj_clear(
                    struct ospf_nbr *p_nbr,
                    u_int old_state) 
{
    struct ospf_if *p_if = p_nbr->p_if;

    /*clear lsa retransmit related information*/
//	ospf_logx(ospf_debug_nbr,"%s,%d p_nbr->dd_state=0x%x,  p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_state,p_nbr->dd_seqnum);
    ospf_nbr_rxmt_list_flush(p_nbr);
    ospf_nbr_dd_list_flush(p_nbr);
    ospf_nbr_request_list_flush(p_nbr);

    /*rebuild router lsa when nbr changed from FULL state*/           
    if (OSPF_NS_FULL == old_state)
    {
        ospf_router_lsa_originate(p_if->p_area);
        /*schedule interface DR election*/
        if (OSPF_IFS_DR <= p_if->state)
        {
            p_if->nbrchange = TRUE;
        }
    }    
    return;
}

/* section 10.3, State: Any (p. 81-82) */
void 
ospf_nsm_hello_rxd (struct ospf_nbr *p_nbr)
{
    struct ospf_if *p_if = p_nbr->p_if;
            
    /* reset inactivity timer */
    ospf_stimer_start(&p_nbr->hold_timer, p_if->dead_interval);
    
   /*set NBMA state*/
    if ( (OSPF_NS_ATTEMPT ==  p_nbr->state)
        || (OSPF_NS_DOWN == p_nbr->state))
    {
        p_nbr->state = OSPF_NS_INIT;
    }
    return;
}

/*execute when neighbor start event is received*/
/* section 10.3, State: Down (p. 81) */
void 
ospf_nsm_start (struct ospf_nbr *p_nbr)
{
    /* this routine is called for OSPF_NBMA networks only - section 10.2, Event: Start (p. 79) */
    struct ospf_if *p_if = p_nbr->p_if;
  
    if (OSPF_IFT_NBMA == p_if->type)
    {
        ospf_stimer_start(&p_if->hello_timer, 1);
        ospf_stimer_start(&p_if->poll_timer, 1);
        
        ospf_stimer_start(&p_nbr->hold_timer, p_if->dead_interval);
        p_nbr->state = OSPF_NS_ATTEMPT;
    }
    return; 
}

/*check if a nbr can be an adj,OK----can be adj, ERR---can not be adj*/
u_int 
ospf_nbr_adj_ok(struct ospf_nbr *p_nbr)
{
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_process *p_process = p_if->p_process;
    u_int8 state_mask[4] = {0};
    
    BIT_LST_SET(state_mask, OSPF_NS_EXSTART);
    BIT_LST_SET(state_mask, OSPF_NS_EXCHANGE);
    BIT_LST_SET(state_mask, OSPF_NS_LOADING);

    /*if self is master,check if current exchanging nbr's count exceed limit,if so,reject this adj*/
    if (p_nbr->id < p_process->router_id)
    {
        /*check nbr's count in this process*/
        if (OSPF_MAX_EXCHANGE_NBR <= ospf_nbr_count_in_state(p_process, state_mask))
        {
            ospf_logx(ospf_debug, "nbr count instate falied count = %d", ospf_nbr_count_in_state(p_process, state_mask));
            return FALSE;
        }
        /*check nbr's count in all process*/
        if (OSPF_MAX_GLOBAL_EXSTART_NBR <= ospf_nbr_count_in_state(NULL, state_mask))
        {
             ospf_logx(ospf_debug, "nbr global count instate falied count = %d", ospf_nbr_count_in_state(NULL, state_mask));
            return FALSE;
        }
    }
   /*
    * section 10.4 (p. 87) - Routers connected by point-to-point 
    networks, Point-to-MultiPoint networks and virtual links 
    always become adjacent.  On broadcast and OSPF_NBMA networks, 
    all routers become adjacent to both the Designated Router and 
    the Backup Designated
    * Router.
    */
    switch (p_if->type) {
        case OSPF_IFT_PPP:
        case OSPF_IFT_VLINK:
        case OSPF_IFT_P2MP:
        case OSPF_IFT_SHAMLINK:  
             return TRUE;
             break;

        case OSPF_IFT_BCAST:
        case OSPF_IFT_NBMA:
             if ((p_if->dr == p_nbr->addr)
                 || (p_if->bdr == p_nbr->addr)
                 || (p_if->dr == p_if->addr)
                 || (p_if->bdr == p_if->addr))
             {             
                 /*if nbr's dr/bdr not different from self,do not establish adj,wait for dr election*/
                 /*  for test center,hello pkt dr,bdr is 0 
 
                 if ((p_if->dr != p_nbr->dr) || (p_if->bdr != p_nbr->bdr))
                 {
                     return FALSE;
                 }  */          
                 return TRUE;
             }
             break;
            
        default:
             break;
    }
    return FALSE;
}

/* section 10.3, State: Init (p. 82) */
void 
ospf_nsm_2way_rxd(struct ospf_nbr *p_nbr)
{
    u_int old_state = p_nbr->state;
    
    /*establish adj or remain 2way state*/
    if (TRUE == ospf_nbr_adj_ok(p_nbr))
    {
        ospf_nbr_adj_establish (p_nbr);
    }
    else
    {
        p_nbr->state = OSPF_NS_2WAY;
    }
    /*if old state is less than 2way,trigger ifnbrchange event*/
    if (OSPF_NS_2WAY > old_state)
    {
        ospf_ism(p_nbr->p_if, OSPF_IFE_NBR_CHANGE);
    }
    return;
}

/* section 10.3, State: ExStart (p. 83) */
void 
ospf_nsm_negdone(struct ospf_nbr *p_nbr)
{
    u_int8 nbr_str[32];

    /*if overload timer is running, do not establish it*/
    if (ospf_timer_active(&p_nbr->p_if->p_process->db_overload_timer))
    {
        return;
    }

    ospf_logx(ospf_debug_nbr, "%sneighbor %s enter exchange state",
        (OSPF_IFT_VLINK == p_nbr->p_if->type) ? "virtual " : "",
        ospf_inet_ntoa(nbr_str,p_nbr->addr));

    /*build database table for nbr,and enter exchange state*/
    if (OK == ospf_nbr_dd_list_build (p_nbr))
    {
        p_nbr->state = OSPF_NS_EXCHANGE;        
    }
    else
    {
        ospf_logx(ospf_debug_nbr, "failed to build dd list");

        /*dbd allocate failed,start overload timer,1 seconds?*/
        ospf_stimer_start(&p_nbr->p_if->p_process->db_overload_timer, 1);
    }
    return;
}

/* section 10.3, State: Loading (p. 84) */
void 
ospf_nsm_load_done(struct ospf_nbr *p_nbr)
{
    u_int8 nbr_str[20];
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_process *p_process = p_if->p_process;
	struct ospf_nbr *p_exstart_nbr = NULL;
	struct ospf_nbr *p_next = NULL;
    u_int8 state_mask[4] = {0};
    
    BIT_LST_SET(state_mask, OSPF_NS_LOADING);
    BIT_LST_SET(state_mask, OSPF_NS_FULL);

    vty_out_to_all_terminal("%s ospf neighbor %s loading done,enter full state ",
        (OSPF_IFT_VLINK == p_nbr->p_if->type) ? "virtual " : "",
        ospf_inet_ntoa(nbr_str, p_nbr->addr));

    /*enter full state*/
    p_nbr->state = OSPF_NS_FULL;

    /*record time enter FULL state*/
    p_nbr->full_time = os_system_tick();

    /*if some lsa is in request list,do not check
     restart finish*/
    if (p_process->restart_enable 
        && p_process->in_restart
        && (ospf_nbr_count_in_state(p_process, state_mask) == p_process->gr_nbr_count))
    {
        /*decide if restart successfully finished or aborted*/
        if (TRUE == ospf_restart_topo_changed(p_process))
        {
            ospf_restart_finish(p_process, OSPF_RESTART_TOPLOGY_CHANGED);
        }
        else if (TRUE == ospf_restart_completed(p_process))
        {
            ospf_restart_finish(p_process, OSPF_RESTART_COMPLETED);
        }
    }        
    /* section 12.4, item (4) (page 115) */
    ospf_router_lsa_originate(p_if->p_area);
    /* section 12.4, item (4) (page 115) */   
    if (OSPF_IFS_DR == p_if->state)
    {
        ospf_network_lsa_originate (p_if);
    }   
    /*send init dd for all nbr in exstart state,for fast establish*/
    for_each_node(&p_process->nbr_table, p_exstart_nbr, p_next)
    {
        if (OSPF_NS_EXSTART == p_exstart_nbr->state)
        {
            ospf_initial_dd_output(p_exstart_nbr);
        }
    }
    return;
}

/* section 10.3, State: Exchange (p. 83) */
void 
ospf_nsm_exchangedone (struct ospf_nbr *p_nbr)
{
    /**no request need, enter full state*/
    if (0 == p_nbr->req_count)
    {
        ospf_nsm_load_done(p_nbr);
    }
    else
    {
        /*else enter loading state*/
        p_nbr->state = OSPF_NS_LOADING;
    }
    
    /* section 10.8, (page 94) *//* wait deadinterval before freeing last ddpacket */
    if (OSPF_SLAVE == p_nbr->dd_state )          
    {
        ospf_stimer_start(&p_nbr->slavehold_timer, p_nbr->p_if->dead_interval);
        p_nbr->dd_state = OSPF_SLAVE_HOLD;      
    }
    else 
    {
        ospf_nbr_dd_list_flush (p_nbr);
    }    

    
    ospf_if_hold_cost_start(p_nbr->p_if, OSPF_LDP_INIT_MSG);
    return;
}

/* section 10.3, State: 2-Way or greater (p. 86) */
void 
ospf_nsm_1way_rxd (struct ospf_nbr *p_nbr)
{
    u_int old_state = p_nbr->state;
    u_int8 nbr_str[20];
    struct ospf_if *p_if = p_nbr->p_if;

//   	ospf_logx(ospf_debug_nbr,"#%s:%d\r\n",__FUNCTION__,__LINE__);
  if (OSPF_NS_FULL == old_state)
    {            
        ospf_logx(ospf_debug_nbr, "%sneighbor %s return init state",
            (OSPF_IFT_VLINK == p_nbr->p_if->type) ? "virtual " : "",
            ospf_inet_ntoa(nbr_str, p_nbr->addr));
    }
    
    p_nbr->state = OSPF_NS_INIT;
    p_nbr->dd_state = OSPF_CLEAR_MODE;
   /* p_nbr->dd_seqnum = 0;*/
    p_nbr->dr = 0;
    p_nbr->bdr = 0;
    
//	ospf_logx(ospf_debug_nbr,"%s,%d p_nbr->dd_state=0x%x,  p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_state,p_nbr->dd_seqnum);
    ospf_nbr_adj_clear(p_nbr, old_state);
    
    /* Section 10.3 (Introduction): When a neighbor's state change, it may be
    * necessary to rerun the designated router election algorithm. This is determined
    * whether the interface NeighborChange event is generated.
    * Section 9.2: NeighborChange event should be executed if (item number 2) there
    * is no longer bidirection communication with a neighbor. Here we have tear
    * down the adjacency with the neighbor. So the interface state machine needs to
    * be executed. Otherwise, WindNet OSPF will never detect the scenario where the
    * neighbor has shutdown and restart again (before the router dead interval expires).
    */
    if ((OSPF_IFS_DROTHER == p_if->state)
        ||(OSPF_IFS_DR == p_if->state)
        ||(OSPF_IFS_BDR == p_if->state))
    {
        ospf_ism(p_if, OSPF_IFE_NBR_CHANGE);
    }

    /*if neighbor is not in restarting,leave any hepler mode*/
    if (!p_nbr->in_restart)
    { 
        ospf_logx(ospf_debug_nbr, "1-way event,exit gr helper");     

        ospf_restart_helper_finish_all(p_if->p_process, OSPF_RESTART_TOPLOGY_CHANGED);
    }
    return;
}

/* section 10.3, State: Any (p. 85-86) */
void 
ospf_nsm_down (struct ospf_nbr *p_nbr)
{
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_process *p_process = p_if->p_process;
    u_int old_state = p_nbr->state;
    u_int8 nbr_str[20];
    u_int8 state_mask[4] = {0};

    if (OSPF_NS_FULL == old_state)
    {   
        vty_out_to_all_terminal("%s ospf neighbor %s shutdown ", 
            (OSPF_IFT_VLINK == p_nbr->p_if->type) ? "virtual " : "",
            ospf_inet_ntoa(nbr_str, p_nbr->addr));
    }
    ospf_timer_stop(&p_nbr->hold_timer);
    
    p_nbr->state = OSPF_NS_DOWN;

    memset(p_nbr->auth_seqnum, 0, sizeof(p_nbr->auth_seqnum));
//	ospf_logx(ospf_debug_nbr,"%s,%d p_nbr->dd_state=0x%x,  p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_state,p_nbr->dd_seqnum);
    ospf_nbr_adj_clear(p_nbr, old_state);

    if (OSPF_NS_INIT < old_state)
    {
        ospf_ism (p_if, OSPF_IFE_NBR_CHANGE);
    }
        
    if (!p_process->in_restart)
    {
        if (p_nbr->addr == p_if->dr)
        {
            p_if->dr = 0;
            p_nbr->dr = 0;
        }
        
        if (p_nbr->addr == p_if->bdr)
        {
            p_if->bdr = 0;
            p_nbr->bdr = 0;
        }
    }

    /*set state mask,check full nbr*/
    BIT_LST_SET(state_mask, OSPF_NS_FULL);

    /*if no full nbr on this iface, delete self network LSA*/
    if (ospf_if_nbr_count_in_state(p_if, state_mask) == 0) 
    { 
        ospf_network_lsa_originate(p_if);
    }

    /*check if delete lsa from any nbr.*/
    ospf_nbr_lsa_flush(p_process);        
    return;
}

/*record nsm output string*/
void 
ospf_log_nsm(
          struct ospf_nbr *p_nbr,
          u_int state,
          u_int event)
{
    u_int8 s_ifa[16];
    u_int8 *s_state[9] = {"","Down","Attempt","Init", "2-way","ExStart", "Exchange", "Loading","Full"};	
    u_int8 *s_event[] = {"hello-rcvd","Start","2-way","neg-done","exchange-done","bad-request","loading-done","Adj-OK","SeqMismatch","1-way","KillNeighbor","Expired","LowlayerDown"}; 

    ospf_inet_ntoa(s_ifa, p_nbr->addr);

    ospf_logx(ospf_debug_nbr,"nbr-fsm,%snbr=%s,state=%s,event=%s", 
        (OSPF_IFT_VLINK == p_nbr->p_if->type) ? "virtual " : "", s_ifa,s_state[state],s_event[event]);
    return;
}

void 
ospf_nbr_log_add(
            struct ospf_nbr *p_nbr,
            u_int lsat_state,
            u_int event)
{
    struct ospf_nbr_loginfo *p_stat = NULL;
    
    if (OSPF_STAT_MAX_NUM <= ospf_lstcnt(&ospf.log_table.nbr_table))
    {
        p_stat = ospf_lstfirst(&ospf.log_table.nbr_table);
        ospf_lstdel_unsort(&ospf.log_table.nbr_table, p_stat);
    }
    else
    {
        p_stat = ospf_malloc(sizeof(struct ospf_nbr_loginfo), OSPF_MSTAT);
    }
    if (NULL == p_stat)
    {
        return;
    }    

    p_stat->process_id = p_nbr->p_if->p_process->process_id;
    p_stat->nbr_addr = p_nbr->addr;
    p_stat->state = p_nbr->state;
    p_stat->last_state = lsat_state;
    p_stat->event = event;
    log_time_print(p_stat->time);
   
    ospf_lstadd_unsort(&ospf.log_table.nbr_table, p_stat);
    return;   
}

/*nbr state machine*/
void 
ospf_nsm (
       struct ospf_nbr *p_nbr,
       u_int event)
{
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_process *p_process = p_if->p_process;
    u_int  old_state = p_nbr->state;
    u_int log_display = 0;
    u_int8 statemask[4] = {0};
    
	//ospf_logx(ospf_debug_nbr," ospf_nsm strat \r\n");
    /*hello event will display in hello debug control*/
    if (ospf_debug_nbr && ((OSPF_NE_HELLO != event) && (OSPF_NE_2WAY != event)))
    {
        log_display++;
    }
    else if (ospf_debug_hello && ((OSPF_NE_HELLO == event) || (OSPF_NE_2WAY == event)))
    {
        log_display++;
    }

    if (log_display)
    {
        ospf_log_nsm(p_nbr, old_state, event);
    }
    
    p_if->stat->nbr_event[event]++;
    switch (event) {
        case OSPF_NE_HELLO: 
             ospf_nsm_hello_rxd(p_nbr);
             break;

        case OSPF_NE_START:
             if (OSPF_NS_DOWN == old_state)
             {
                 ospf_nsm_start(p_nbr);
             }
             break;

        case OSPF_NE_2WAY:
             if ((OSPF_NS_INIT == old_state)  
                 || (OSPF_NS_2WAY == old_state))
             {
                 ospf_nsm_2way_rxd(p_nbr);
             }
             break;

        case OSPF_NE_NEGDONE:
             if (OSPF_NS_EXSTART == old_state)
             {
                 ospf_nsm_negdone(p_nbr);
             }
             break;

        case OSPF_NE_EXCHANGE_DONE:
             if (OSPF_NS_EXCHANGE == old_state)
             {
                 ospf_nsm_exchangedone(p_nbr);
             }
             break;

        case OSPF_NE_BAD_REQUEST:
        case OSPF_NE_SEQ_MISMATCH:
		//	printf("%s %d OSPF_NE_SEQ_MISMATCH \n", __FUNCTION__,__LINE__);
             if (OSPF_NS_EXCHANGE <= old_state)
             {
                p_nbr->state = OSPF_NS_EXSTART;

                ospf_nbr_adj_clear(p_nbr, old_state);
                ospf_nbr_adj_establish (p_nbr);
             }
             break;

        case OSPF_NE_LOADDONE:
             if (OSPF_NS_LOADING == old_state)
             {
                 ospf_nsm_load_done(p_nbr);
             }
             break;

        case OSPF_NE_ADJOK:
             if (OSPF_NS_2WAY == old_state)
             {
                 if (TRUE == ospf_nbr_adj_ok(p_nbr))
                 {
                     ospf_nbr_adj_establish (p_nbr);
                 }
             }
             else if (OSPF_NS_EXSTART <= old_state)
             {
                 if (FALSE == ospf_nbr_adj_ok(p_nbr))
                 {
                     p_nbr->state = OSPF_NS_2WAY;
                     p_nbr->dd_state = OSPF_CLEAR_MODE;
                     /*p_nbr->dd_seqnum = 0x00000000L;*/
 
			//		 printf("%s,%d p_nbr->dd_state=0x%x,  p_nbr->dd_seqnum=0x%x\n",__FUNCTION__,__LINE__,p_nbr->dd_state,p_nbr->dd_seqnum);
                     ospf_nbr_adj_clear(p_nbr, old_state);
                 }
             }
             break;
           
        case OSPF_NE_1WAY:
             if (OSPF_NS_2WAY <= old_state)    
             {
                 ospf_nsm_1way_rxd(p_nbr);
             }
             break;

        case OSPF_NE_KILL:
        case OSPF_NE_INACTIVITY_TIMER:
        case OSPF_NE_LL_DOWN:
             ospf_nsm_down(p_nbr);
             break;

        default:
             break;
    }

    if (old_state == p_nbr->state)
    {
        return;
    }

    if ((OSPF_NS_FULL == old_state) && (OSPF_NS_FULL != p_nbr->state))
    {
        ospf_logx(0, "process %d, ifindex 0x%x,neighbor old state %d to %d",
            p_process->process_id,p_if->ifnet_uint, old_state, p_nbr->state);
    }
    
    p_process->nbr_count[p_nbr->state]++;
    ospf.stat.nbr_count[p_nbr->state]++;
    p_process->nbr_count[old_state]--;
    ospf.stat.nbr_count[old_state]--;
    
    ospf_stimer_start(&ospf.nbr_state_update_timer, 2);
    /*some nbr's state changed*/
    p_process->p_master->nbr_changed = TRUE;
    
    ++p_nbr->events;
    
    if (OSPF_IFT_VLINK == p_if->type)
    {
        ospf_trap_vnbrstate(old_state, p_nbr);
    }
    else 
    {
        ospf_trap_nbrstate(old_state, p_nbr);
    }
    
    ospf_nbr_log_add(p_nbr, old_state, event);

    /*when nbr state change back to EXSTART, limit EXSTART nbr count*/
    if ((old_state > OSPF_NS_EXSTART) && (p_nbr->state == OSPF_NS_EXSTART))   
    {        
        BIT_LST_SET(statemask, OSPF_NS_EXSTART);
        if (OSPF_MAX_TWOWAY_NBR_COUNT <= ospf_nbr_count_in_state(NULL, statemask))
        {
            ospf_timer_start(&p_nbr->hold_timer, 0);
        }
    }
    /*if nbr on vlink switch to or from FULL state,rebuild router lsa of transit area*/
    if ((OSPF_IFT_VLINK == p_if->type)
        && ((OSPF_NS_FULL == old_state) || (OSPF_NS_FULL == p_nbr->state)))
    {
        ospf_router_lsa_originate(p_if->p_transit_area);
        ospf_stimer_start(&p_if->p_transit_area->transit_range_timer, 2);
    }
#ifdef OSPF_MASTER_SLAVE_SYNC 
    if (OSPF_MODE_MASTER != p_process->p_master->work_mode)
    {
	//	ospf_logx(ospf_debug_nbr," ospf_nsm end (OSPF_MODE_MASTER != p_process->p_master->work_mode)\r\n");
        return;
    }
    /*send sync msg to backup card*/
    ospf_sync_event_set(p_nbr->syn_flag);

    /*log for exchange->lower state*/
    if (ospf_debug_nbr
        && (OSPF_NS_EXCHANGE == old_state)
        && (OSPF_NS_EXCHANGE > p_nbr->state))
    {
        ospf_logx(ospf_debug_nbr, "neighbor state bring down to %d, event %d", p_nbr->state, event);
    }
	ospf_logx(ospf_debug_nbr," ospf_nsm end \r\n");
#endif
   
    if ((p_process->abr) 
        && (((OSPF_NS_FULL == old_state) && (OSPF_NS_FULL != p_nbr->state))
        || ((OSPF_NS_FULL != old_state) && (OSPF_NS_FULL == p_nbr->state))))
    {
        ospf_backbone_status_update(p_process);
    }
    return;
}

/*delete all lsa advertised by nbr,used for most conformance testing
check if we can delete all remote lsa learnt from some nbr,
 if so,delete these lsa.*/
void 
ospf_nbr_lsa_flush(struct ospf_process *p_process)
{
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_nextlsa = NULL;
    struct ospf_lstable *p_table = NULL;
    struct ospf_lstable *p_next_table = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_nextarea = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_nextif = NULL;
    u_int flush = FALSE;
    u_int8 state_mask[4] = {0};

    /*set state mask,check exstart/exchange/loading/full or in restart*/
    BIT_LST_SET(state_mask, OSPF_NS_FULL);
    BIT_LST_SET(state_mask, OSPF_NS_RESTART);
    BIT_LST_SET(state_mask, OSPF_NS_EXSTART);
    BIT_LST_SET(state_mask, OSPF_NS_EXCHANGE);
    BIT_LST_SET(state_mask, OSPF_NS_LOADING);
    
    /*scan all area*/
    for_each_node(&p_process->area_table, p_area, p_nextarea)
    {
        flush = TRUE;
        /*check all interface in this area,if some nbr state 
         is exstart/exchange/loading/full,do not delete any lsa in this area*/
        for_each_node(&p_process->if_table, p_if, p_nextif)
        {
            if ((p_if->p_area == p_area) 
                && ospf_if_nbr_count_in_state(p_if, state_mask))
            {
                flush = FALSE;
                break;
            }
        }
        /*if flush enable,flush expected lsa*/
        if (!flush) 
        {
            continue;
        }
        for_each_node(&p_process->lstable_table, p_table, p_next_table)
        {
            /*lsa table scope must be this area*/
            if (p_table->p_area != p_area)
            {
                continue;
            }
            
            for_each_node(&p_table->list, p_lsa, p_nextlsa)
            {
                if (ntohl(p_lsa->lshdr->adv_id) != p_process->router_id)
                {
                    ospf_lsa_maxage_set(p_lsa);
                }
            }
        }
    }

    /*check lsa in as flooding scope*/
    for_each_node(&p_process->if_table, p_if, p_nextif)
    {
        if (ospf_if_nbr_count_in_state(p_if, state_mask))
        {
            return;
        }
    }

    /*clear any remote lsa in process*/
    for_each_node(&p_process->lstable_table, p_table, p_next_table)
    {
        for_each_node(&p_table->list, p_lsa, p_nextlsa)
        {
            if (ntohl(p_lsa->lshdr->adv_id) != p_process->router_id)
            {
                ospf_lsa_maxage_set(p_lsa);
            }
        }
    }
    return;
}

/*neighbor's inactive timer expired*/
void 
ospf_nbr_inactive_timer(struct ospf_nbr *p_nbr)
{
    struct ospf_if *p_if = p_nbr->p_if;
    u_int now = os_system_tick();
    u_int last_rx = p_nbr->rcv_pkt_time;
    
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    p_nbr->rcv_pkt_time = 0;
    
    /* section 10, (page 73-74) */
    /*if neighbor is in GR state,do not expiration it*/
    if (p_nbr->in_restart) 
    {
        ospf_stimer_start(&p_nbr->hold_timer, p_if->dead_interval);
        return;
    }
    if (now > last_rx)
    {
        /*have rcvd pkt during death time */
        if (now - last_rx < p_if->dead_interval)
        {
            ospf_stimer_start(&p_nbr->hold_timer, p_if->dead_interval - (now - last_rx));
            return;
        }
    }
    else/*time reversed,restart timer*/
    {
         ospf_stimer_start(&p_nbr->hold_timer, p_if->dead_interval); 
         return;
    }
    
    ospf_logx(ospf_debug_nbr, "nbr inactive timer expired");     
    ospf_nsm(p_nbr, OSPF_NE_INACTIVITY_TIMER);
    ospf_timer_stop(&p_nbr->hold_timer);

    /*delete nbr for normal interface,if nbr is to be rejected in a preriod,do not delete it*/
    if (((OSPF_IFT_BCAST == p_if->type) || (OSPF_IFT_PPP == p_if->type))  
        && !ospf_timer_active(&p_nbr->reject_nbr_timer))
    {
    #ifdef OSPF_FRR
        ospf_backup_route_switch(p_nbr);
    #endif
        ospf_nbr_delete (p_nbr);
    }          

    /*rebuild router lsa for related area*/
    ospf_router_lsa_originate(p_if->p_area);

    /*neighbor removed,do not act as hepler*/
    ospf_logx(ospf_debug_nbr, "nbr inactive timer expired,exit gr helper");     

    ospf_restart_helper_finish_all(p_if->p_process, OSPF_RESTART_TOPLOGY_CHANGED);
    
    return;
}
void
ospf_nbr_state_update_timeout(void) 
{
    struct ospf_nbr *p_nbr = NULL; 
    struct ospf_nbr *p_nextnbr = NULL;
    struct ospf_process *p_scan = NULL;
    struct ospf_process *p_next = NULL;
    
    memset(&ospf.stat.nbr_count, 0, sizeof(ospf.stat.nbr_count));
    
    for_each_ospf_process(p_scan, p_next)
    {
        memset(&p_scan->nbr_count, 0, sizeof(p_scan->nbr_count));
        for_each_node(&p_scan->nbr_table, p_nbr, p_nextnbr)    
        {
            if (!p_nbr->in_restart)
            {
                p_scan->nbr_count[p_nbr->state]++;
                ospf.stat.nbr_count[p_nbr->state]++;
            }
            else
            {
                p_scan->nbr_count[OSPF_NS_RESTART]++;
                ospf.stat.nbr_count[OSPF_NS_RESTART]++;
            }
        }
    }
    return;
}

u_int 
ospf_nbr_count_in_state( 
                      struct ospf_process *p_process,
                      uint8_t *state_mask)
{
    u_int i = 0;
    u_int count = 0;
    
    /*get count directly if state not changed*/
    for (i = 0 ; i < OSPF_NS_MAX ; i++)
    {
        if (BIT_LST_TST(state_mask, i))
        {
            if (p_process)
            {
                count += p_process->nbr_count[i];
            }
            else
            {
                count += ospf.stat.nbr_count[i];
            } 
        }
    }
    return count;
    
    
}

/*get neighbor count in special state:for special interface*/
u_int 
ospf_if_nbr_count_in_state( 
                  struct ospf_if *p_if, 
                  u_int8 *state_mask)
{
    struct ospf_nbr *p_nbr = NULL;   
    struct ospf_nbr *p_nextbr = NULL;   
    u_int count = 0;
    
    for_each_ospf_nbr(p_if, p_nbr, p_nextbr)
    {
        if (BIT_LST_TST(state_mask, p_nbr->state))
        {
            count++;
        }
        if (BIT_LST_TST(state_mask, OSPF_NS_RESTART) && (p_nbr->in_restart == TRUE))
        {
            count++;
        }
    }
    return count;
}

/*decide if area has any nbr in FULL state*/
u_int
ospf_area_full_nbr_exist(struct ospf_area *p_area)
{
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_nbr *p_nbr = NULL;   
    struct ospf_nbr *p_nextbr = NULL;

    for_each_node(&p_process->nbr_table, p_nbr, p_nextbr)
    {
        if ((OSPF_NS_FULL == p_nbr->state) && (p_nbr->p_if->p_area == p_area))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*after dd exchange,slave will retain the last database packet for a period
  ,it delete the saved packet when this timer expired*/
void 
ospf_nbr_slavehold_timeout(struct ospf_nbr *p_nbr)
{
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    if (OSPF_SLAVE_HOLD == p_nbr->dd_state) 
    {
        p_nbr->dd_state = OSPF_CLEAR_MODE;
        ospf_nbr_dd_list_flush(p_nbr);
    } 
    return;
}

/*database retransmit timer expired*/
void 
ospf_nbr_dd_timeout(struct ospf_nbr *p_nbr)
{
    struct ospf_process *p_process = p_nbr->p_if->p_process;
    u_int8 statemask[4] = {0};
    
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    /*exstart state:restart init database packet*/
    if (OSPF_NS_EXSTART == p_nbr->state)
    {
        /*check current nbr's count, if exceed limit,do not send init dd packet*/
        BIT_LST_SET(statemask, OSPF_NS_EXCHANGE);
        /*BIT_LST_SET(statemask, OSPF_NS_LOADING);*//*do not limit loading number*/
        if ((OSPF_MAX_EXCHANGE_NBR <= ospf_nbr_count_in_state(p_process, statemask))
            || (OSPF_MAX_GLOBAL_EXCHANGE_NBR <= ospf_nbr_count_in_state(NULL, statemask)))
        {
            ospf_timer_start(&p_nbr->dd_timer, 5);
            return;
        }    
        p_nbr->p_if->stat->init_dd_rxmt++;
        ospf_initial_dd_output(p_nbr);
    }/*master and in exhange state:restart current database packet*/
    else if ((OSPF_NS_EXCHANGE == p_nbr->state) 
            && (OSPF_MASTER == p_nbr->dd_state)) 
    {
        p_nbr->p_if->stat->dd_rxmt++;
        ospf_stimer_start(&p_process->ratelimit_timer, OSPF_RATELIMIT_INTERVAL);
        ospf_dd_output(p_nbr);
    }
    return ;
}

/*nbr request timer expire,send lsa request packet*/
void 
ospf_nbr_request_timeout(struct ospf_nbr *p_nbr)
{
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    if ((OSPF_NS_EXCHANGE <= p_nbr->state) 
        && (ospf_lstcnt(&p_nbr->p_if->p_process->req_table)))
    {
        /*request retransmit detected,some congestion appear,so start a ratelimit
           timer to slow request speed*/
        if (p_nbr->lsa_reply < p_nbr->lsa_requested)
        {
            p_nbr->p_if->stat->req_rxmt++;
            ospf_stimer_start(&p_nbr->p_if->p_process->ratelimit_timer, OSPF_RATELIMIT_INTERVAL);
        }
        /*send request packet*/
        ospf_request_output(p_nbr);
    }

    /*call nsm loading done:no more request exist*/ 
    if ((OSPF_NS_LOADING == p_nbr->state) && (0 == p_nbr->req_count))
    {
        ospf_nsm(p_nbr, OSPF_NE_LOADDONE);
    }
    return;
}

/*nbr lsa retransmit timer expired*/
void 
ospf_nbr_retransmit_timeout(struct ospf_nbr *p_nbr)
{
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    /*don't rxmt at once when exsit too may nbr*/
    if (p_nbr->rxmt_timer_delay)
    {
        ospf_timer_start(&p_nbr->lsrxmt_timer, ospf_rand(p_nbr->p_if->rxmt_interval * OSPF_TICK_PER_SECOND));
        p_nbr->rxmt_timer_delay = FALSE;
        return;
    }
    /*send unicast update to nbr*/
    if ((OSPF_NS_EXCHANGE <= p_nbr->state) && (0 < p_nbr->rxmt_count))
    {
        ospf_lsa_retransmit (p_nbr);
    }
    /*restart retransmit timer*/
    if (p_nbr->p_if->rxmt_interval)
    {
        if (!ospf_timer_active(&p_nbr->p_if->p_process->rxmtlimit_timer))
        { 
            ospf_timer_start(&p_nbr->lsrxmt_timer, ospf_rand(p_nbr->p_if->rxmt_interval * OSPF_TICK_PER_SECOND));
        }
        else
        {
            ospf_timer_start(&p_nbr->lsrxmt_timer, 1);
        }
    }
    else/*short value*/
    {
        ospf_timer_start(&p_nbr->lsrxmt_timer, 2);
    }
    return;
}

/*if too many errored lsa rxd from nbr in a period,delete this nbr*/
void 
ospf_nbr_error_timeout(struct ospf_nbr *p_nbr)
{
    u_int8 nbr_str[20];
    
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    if (p_nbr->lsa_error_cnt > OSPF_LSA_ERROR_MAX_CNT)
    {
        ospf_logx(ospf_debug_nbr, "too many errored lsa rxd from nbr %s, shutdown neighbr", ospf_inet_ntoa(nbr_str, p_nbr->addr));

        ospf_nsm(p_nbr, OSPF_NE_KILL);

        ospf_stimer_start(&p_nbr->reject_nbr_timer, (p_nbr->p_if->dead_interval)*2);
    }
    p_nbr->lsa_error_cnt = 0;
    return;
}

/*reject nbr establish in a period*/
void 
ospf_nbr_reject_timeout(struct ospf_nbr *p_nbr)
{
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    /*start normal hold timer of nbr*/
    ospf_stimer_start(&p_nbr->hold_timer, p_nbr->p_if->dead_interval);
    return;
}

/*scheduled bfd checking timer,try to bind bfd session when timer expired*/
#ifdef HAVE_BFD
void
ospf_nbr_bfd_timeout(struct ospf_nbr *p_nbr)
{
    if (((p_nbr->p_if->bfd_enable == OSPF_BFD_GBL_ENABLE)
        || (p_nbr->p_if->bfd_enable == OSPF_BFD_IF_ENABLE))
            && (0 == p_nbr->bfd_discribe))
    {
        ospf_bind_bfd(p_nbr);
    }
    return;
}
#endif


u_int 
ospf_backbone_full_nbr_exsit(struct ospf_process *p_process)
{
	struct ospf_area *p_area = NULL;

    p_area = ospf_area_lookup(p_process, OSPF_BACKBONE);
	if (p_area == NULL)
		return FALSE;
	
    return ospf_area_full_nbr_exist(p_area);

}
/*decide if area has any nbr */
u_int
ospf_area_nbr_exist(struct ospf_area *p_area)
{
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_nbr *p_nbr = NULL;   
    struct ospf_nbr *p_nextbr = NULL;

    for_each_node(&p_process->nbr_table, p_nbr, p_nextbr)
    {
        if ((OSPF_NS_DOWN != p_nbr->state) && (p_nbr->p_if->p_area == p_area))
        {
            return TRUE;
        }
    }
    return FALSE;
}



u_int 
ospf_backbone_nbr_exsit(struct ospf_process *p_process)
{
	struct ospf_area *p_area = NULL;

    p_area = ospf_area_lookup(p_process, OSPF_BACKBONE);
	if (p_area == NULL)
		return FALSE;
	
    return ospf_area_nbr_exist(p_area);

}

void 
ospf_lsa_t5_uninstall(
                  struct ospf_lstable *p_table,
                  struct ospf_lshdr *p_lshdr)
{
    struct ospf_lsa *p_lsa = NULL;

	/*search current one*/
	p_lsa = ospf_lsa_lookup(p_table, p_lshdr);
	if (OSPF_MAX_LSAGE == ntohs(p_lshdr->age))
	{
		/*purge request,if lsa exist,delete it*/
		if (NULL != p_lsa)
		{
			ospf_lsa_maxage_set(p_lsa);
		}
		return;
	}

}
/*nssa����type 5ת��Ϊtype7��ɾ��type5����*/
int ospf_nssa_external_del(
                 struct ospf_area *p_area ,
                 struct ospf_iproute *p_route)
{
	struct ospf_process *p_process = p_area->p_process;
    struct ospf_iproute *p_route_tmp = NULL;
    struct ospf_iproute *p_next_route = NULL;
	struct ospf_external_lsa  external_lsa;
	
	memset(&external_lsa, 0, sizeof(external_lsa));
	if(TRUE == ospf_backbone_nbr_exsit(p_process))
	{
		return OK;
	}
	if (TRUE == p_area->is_nssa)
	{
		for_each_node(&p_process->import_table, p_route_tmp, p_next_route)
		{
			if(p_route_tmp != NULL)
			{
				if((p_route->dest == p_route_tmp->dest)
					&&(p_route->fwdaddr == p_route_tmp->fwdaddr)
					&&(p_route->mask == p_route_tmp->mask)
					&&(p_route->active == TRUE))
				{
					
					ospf_logx(0, "ospf_nssa_external_del dest=%x,fwdaddr=%x,mask=%x,active=%d\n",	
						p_route->dest,p_route->fwdaddr,p_route->mask,p_route->active);
					ospf_init_lshdr(&external_lsa.h, OSPF_LS_AS_EXTERNAL, p_route->dest, p_process->router_id);
					external_lsa.h.len = htons(OSPF_ASE_LSA_HLEN);	 
					external_lsa.h.age = htons(OSPF_MAX_LSAGE);
					ospf_lsa_t5_uninstall(&p_process->t5_lstable, &external_lsa.h);
					return OK;
				}
			}
		}
	}
	
	return OK;
}

void ospf_if_to_nbr(struct ospf_if *p_if)
    {
        struct ospf_nbr *p_nbr = NULL;   
        struct ospf_nbr *p_nextbr = NULL;
        
        for_each_ospf_nbr(p_if, p_nbr, p_nextbr)
        {
            ospf_dd_output(p_nbr);
        }
    }
  



