
#include "ospf.h"
#include "ospf_te_interface.h"

/*for type_10 te*/
u_char ucLsa_te = 0;
/*compare function of te router node*/
int
ospf_te_router_cmp(
           struct ospf_terouter *p1, 
           struct ospf_terouter *p2)
{
    OSPF_KEY_CMP(p1, p2, router_id); 
    return 0;
}

void 
ospf_te_router_table_init(struct ospf_process *p_process)
{
    ospf_lstinit(&p_process->te_router_table, ospf_te_router_cmp);
    return;
}

/*lookup a te router*/
struct ospf_terouter* 
ospf_te_router_lookup(
                   struct ospf_process *p_process,
                   u_int router_id)
{
    struct ospf_terouter  te_route;
    te_route.router_id = router_id;
    return ospf_lstlookup(&p_process->te_router_table, &te_route);
}

/*create te router*/
struct ospf_terouter*
ospf_te_router_create(
        struct ospf_process *p_process,
        u_int router_id, 
        u_int area_id, 
        u_int addr)
{
    struct ospf_terouter *p_te_route;
    
    p_te_route = ospf_malloc2(OSPF_MTEROUTER);
    if (NULL == p_te_route)
    {
        return NULL;
    } 
    p_te_route->router_id = router_id;
    p_te_route->addr = addr;
    p_te_route->area_id = area_id;
    p_te_route->proto = M2_ipRouteProto_ospf;
    ospf_lstinit(&p_te_route->link_list, NULL);
    
    ospf_lstadd(&p_process->te_router_table, p_te_route);
    return p_te_route;
}

/*must create tTE_ROUTE first,then create te_link*/
struct ospf_telink * 
ospf_te_link_create(
                  struct ospf_process *p_process,
                  u_int router_id)
{
    struct ospf_telink *p_te_link;
    struct ospf_terouter *p_te_route;

    p_te_route = ospf_te_router_lookup(p_process, router_id);
    if (NULL == p_te_route)
    {
        return NULL;
    }
    
    p_te_link = ospf_malloc2(OSPF_MTELINK);
    if (NULL == p_te_link)
    {
        return NULL;
    }
    memset(p_te_link, 0, sizeof(struct ospf_telink ));
    
    p_te_link->p_router = p_te_route;
    ospf_lstadd_unsort(&p_te_route->link_list, p_te_link);
    return p_te_link;
}

u_int 
ospf_te_pading_lenth(u_int len)
{
    return ((len+3)/4)*4;
}

/*get te param from tlv*/
int 
ospf_te_get_param(
               u_int8 *sub_tlv,
               u_int lenth,
               struct ospf_te_param* param)
{
    struct ospf_te_tlv *p_te_hdr = NULL; 
    int link_type_flag = FALSE;
    int link_id_falg = FALSE;
    u_int8 *tlv_value;
    int i;
    u_int offset = 0;
    
    while (4 < lenth )
    {    
        if (NULL != p_te_hdr)
        {
            sub_tlv += offset;
        }
       
        p_te_hdr = (struct ospf_te_tlv*)sub_tlv;
        tlv_value = (u_int8*)(p_te_hdr + 1);
        
         switch(ntohs(p_te_hdr->type)){
             case OSPF_TE_SUB_LINK_TYPE:
                  link_type_flag = TRUE;
                  param->link_type = *(u_int8*)tlv_value;   
                  break;
                 
             case OSPF_TE_SUB_LINK_ID:
                  link_id_falg = TRUE;
                  param->link_id = ntohl(*(u_int*)tlv_value);
                  break;
                 
             case OSPF_TE_SUB_LOCAL_ADDRESS:
                  for (i = 0; i < MAX_ADDR_NUM; i++)
                  {
                      if (0 == param->local_addr[i])
                      {
                          break;
                      }
                  }
                  if (MAX_ADDR_NUM <= i)
                  {
                       continue;
                  }
                  param->local_addr[i] = ntohl(*(u_int*)tlv_value);
                  break;
                 
             case OSPF_TE_SUB_REMOTE_ADDRESS:
                  for (i = 0; i < MAX_ADDR_NUM; i++)
                  {
                      if (0 == param->remoter_addr[i])
                      {
                          break;
                      }
                  }
                  if (MAX_ADDR_NUM <= i)
                  {
                      continue;
                  }
                  param->remoter_addr[i] = ntohl(*(u_int*)tlv_value);
                  break;   
                 
             case OSPF_TE_SUB_METRIC:
                  param->cost = ntohl(*(u_int*)(p_te_hdr + 1));
                  break;   
                 
             case OSPF_TE_SUB_MAX_BANDWIDTH:
                  param->max_band = ntohl(*(u_int*)(p_te_hdr + 1));
                  break;     
                 
             case OSPF_TE_SUB_MAX_RSVD_BANDWIDTH:
                  param->max_reserve_band = ntohl(*(u_int*)(p_te_hdr + 1));
                  break;     
 
             case OSPF_TE_SUB_UNRSVD_BANDWIDTH:
                  for (i = 0 ; i < 8; i++, tlv_value += 4)
                  {
                      param->unreserve_bind[i] = ntohl(*(u_int*)tlv_value);
                  }               
                  break;     
 
             case OSPF_TE_SUB_RESOURCE_CLASS_COLOR:
                  param->group = ntohl(*(u_int*)tlv_value);
                  break;     
                 
             default:
                  break;
        }
        offset = sizeof(struct ospf_te_tlv) + ospf_te_pading_lenth(ntohs(p_te_hdr->len));
        lenth -= offset;
    }
    if ((!link_type_flag)||(!link_id_falg))
    {
        return ERR;
    }
    return OK;
}

int 
ospf_lsa_tedb_remote_add(struct ospf_process *p_process,
									struct ospf_lsa *p_lsa,
									struct ospf_telink *p_te_link)
{
    struct ospf_lsa *p_next = NULL;
    struct ospf_lshdr *p_ls_hdr = NULL;
    struct ospf_te_tlv *p_te_tlv_hdr = NULL;
    struct ospf_te_routeraddr_tlv *p_router_addr_tlv = NULL;
    u_int8 *p_sub_tlv;
    struct ospf_te_param *p_te_param;
	struct ospf_terouter *p_remote_route;
	u_int remote_route_id;
    struct ospf_telink *p_remote_link;
    struct ospf_telink *p_remote_next_link;

	if(NULL == p_lsa)
	{
		return ERR;
	}

    p_ls_hdr = p_lsa->lshdr;
    
    if (OSPF_MAX_LSAGE <= ntohs(p_ls_hdr->age))
    {
        return ERR;
    }
	

	/*if is router self,ignore*/
	if (ntohl(p_ls_hdr->adv_id) == p_te_link->p_router->router_id)
//	if (ntohl(p_ls_hdr->adv_id) == p_process->router_id)
	{
        return OK;
	}

	remote_route_id = ntohl(p_ls_hdr->adv_id);

	p_remote_route = ospf_te_router_lookup(p_process, remote_route_id);
	if (NULL != p_remote_route) 			   
	{		
		/*check if romote router has link point to self*/
		for_each_node(&p_remote_route->link_list, p_remote_link, p_remote_next_link)
		{
			if ((OSPF_IFT_PPP != p_remote_link->te_param.link_type)
			  && (p_te_link->te_param.link_id == p_remote_link->te_param.link_id))
			{
				/*save remote link*/
				p_te_link->p_remote_link = p_remote_link;
				p_remote_link->p_remote_link = p_te_link;
				return OK;
			}				   
		}			 
	}			 


    return ERR;
}

STATUS 
ospf_te_backlink_exist(
            struct ospf_process *p_process,
            struct ospf_telink *p_te_link)
{
    struct ospf_terouter *p_remote_route;
    struct ospf_telink *p_remote_link;
    struct ospf_telink *p_remote_next_link;
    struct ospf_area *p_area = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_network_lsa *p_network = NULL;
    struct ospf_lshdr lshdr;
    u_int remote_route_id;
    u_int *p_link = NULL;
    
    if (OSPF_IFT_PPP == p_te_link->te_param.link_type)
    {
        p_remote_route = ospf_te_router_lookup(p_process, p_te_link->te_param.link_id);
        if (NULL != p_remote_route)
        {       
            for_each_node(&p_remote_route->link_list, p_remote_link, p_remote_next_link)
            {
                if ((OSPF_IFT_PPP == p_remote_link->te_param.link_type)
                  && (p_te_link->te_param.link_id == p_remote_link->te_param.link_id))
                {
                    p_te_link->p_remote_link = p_remote_link;
                    p_remote_link->p_remote_link = p_te_link;
                    return OK;
                }              
            }
        }
    }
    else
    {
        p_area = ospf_area_lookup(p_process, p_te_link->p_router->area_id);
        if (NULL == p_area)
        {
            return ERR;
        }
		#if 0
        lshdr.type = OSPF_LS_NETWORK;
        lshdr.id = htonl(p_te_link->te_param.link_id);
        lshdr.adv_id = htonl(p_te_link->p_router->router_id);
		#else
		
        lshdr.type = OSPF_LS_TYPE_10;	
        lshdr.id = htonl(p_te_link->te_param.lsa_id);
        lshdr.adv_id = htonl(p_te_link->p_router->router_id);
		#endif
        p_lsa = ospf_lsa_lookup(p_area->ls_table[lshdr.type], &lshdr);
        if (NULL == p_lsa)
        {
            return ERR;
        }
		return ospf_lsa_tedb_remote_add(p_process,p_lsa,p_te_link);

		#if 0
        p_network = (struct ospf_network_lsa *)p_lsa->lshdr;
        for_each_network_link(p_network, p_link)
        {
            /*if is router self,ignore*/
            if (*p_link == p_te_link->p_router->area_id)
            {
                continue;
            }
            remote_route_id = *p_link;
    
            p_remote_route = ospf_te_router_lookup(p_process, remote_route_id);
            if (NULL != p_remote_route)                
            {       
                /*check if romote router has link point to self*/
                for_each_node(&p_remote_route->link_list, p_remote_link, p_remote_next_link)
                {
                    if ((OSPF_IFT_PPP != p_remote_link->te_param.link_type)
                      && (p_te_link->te_param.link_id == p_remote_link->te_param.link_id))
                    {
                        /*save remote link*/
                        p_te_link->p_remote_link = p_remote_link;
                        p_remote_link->p_remote_link = p_te_link;
                        return OK;
                    }                  
                }            
            }            
        }    
		#endif
    }
    return ERR;
}

void 
ospf_check_te_router(struct ospf_process *p_process)
{
    struct ospf_terouter *p_te_route;
    struct ospf_terouter *p_next_route;
    struct ospf_telink *p_te_link;
    struct ospf_telink *p_next_link;  
     
    for_each_node(&p_process->te_router_table, p_te_route, p_next_route)
    { 
        for_each_node(&p_te_route->link_list, p_te_link, p_next_link)
        {
            if (ospf_te_backlink_exist(p_process, p_te_link) != OK)
            {
                ospf_lstdel_unsort(&p_te_route->link_list, p_te_link);
                ospf_mfree(p_te_link, OSPF_MTELINK);
            }
        }
        
        if (NULL == ospf_lstfirst(&p_te_route->link_list))
        {
            ospf_lstdel_free(&p_process->te_router_table, p_te_route, OSPF_MTEROUTER);
        }
    }
    return;
}
/*type 10 lsa translate to te database used struct:te_route+te_link*/
void 
ospf_translate_type10_to_tedb(struct ospf_area * p_area)
{
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next = NULL;
    struct ospf_lshdr *p_ls_hdr = NULL;
    struct ospf_te_tlv *p_te_tlv_hdr = NULL;
    struct ospf_te_routeraddr_tlv *p_router_addr_tlv = NULL;
    struct ospf_process *p_process = p_area->p_process;
    u_int8 *p_sub_tlv;
    struct ospf_te_param *p_te_param;
    struct ospf_telink *p_te_link = NULL;

	if(ucLsa_te == 0)
	{
		return;
	}
	ucLsa_te = 0;
    /*first check all ROUTER_ADDR tlv,create te_router;
    te_link must point to special te_route ,so create te_route first*/
    for_each_area_lsa(p_area, OSPF_LS_TYPE_10, p_lsa, p_next)
    {    
        p_ls_hdr = p_lsa->lshdr;
        
        if (OSPF_MAX_LSAGE <= ntohs(p_ls_hdr->age))
        {
            continue;
        }
        p_te_tlv_hdr = (struct ospf_te_tlv*)(p_lsa->lshdr + 1);

        if (OSPF_TE_TLV_RTR_ADDR == ntohs(p_te_tlv_hdr->type))
        {
            p_router_addr_tlv = (struct ospf_te_routeraddr_tlv*)(p_lsa->lshdr + 1);

            /*router addr must >0*/
            if (0 == p_router_addr_tlv->value)
            {
                continue;
            }
            /*if not exit, create*/
            if (NULL == ospf_te_router_lookup(p_process, ntohl(p_ls_hdr->adv_id)))
            {
                ospf_te_router_create(p_process, ntohl(p_ls_hdr->adv_id), p_area->id, ntohl(p_router_addr_tlv->value));
            }
        }
    }

     /*first check all link_te  tlv,create te_link;*/
    for_each_area_lsa(p_area, OSPF_LS_TYPE_10, p_lsa, p_next)
    {
        p_ls_hdr = p_lsa->lshdr;
        if (OSPF_MAX_LSAGE <= p_ls_hdr->age)
        {
            continue;
        }
        p_te_tlv_hdr = (struct ospf_te_tlv*)(p_lsa->lshdr + 1);
        if (OSPF_TE_TLV_LINK == ntohs(p_te_tlv_hdr->type))
        {
			/*if not exit te_route, p_te_link will not create*/
			p_te_link = ospf_te_link_create(p_process, ntohl(p_ls_hdr->adv_id));
			if(NULL == p_te_link)
			{
                continue;
			}
			p_te_param = &p_te_link->te_param;
			p_sub_tlv = (u_int8 *)(p_lsa->lshdr + 1) + sizeof(struct ospf_te_tlv);
			
		//	printf("ospf_type10_lsa_to_tedb p_te_tlv_hdr->h.len:%d\n",ntohs(p_te_tlv_hdr->len));
			ospf_te_get_param(p_sub_tlv, ntohs(p_te_tlv_hdr->len), p_te_param);
			p_te_param->lsa_id = ntohl(p_ls_hdr->id);
			#if 0
			printf("## ospf_te_get_param:link_type:%d,link_id:%x,local_addr:%x,remoter_addr:%x.\n",
				p_te_param->link_type,p_te_param->link_id,
				p_te_param->local_addr[0],p_te_param->remoter_addr[0]);
			
			printf("## ospf_te_get_param:cost:%d,max_band:%ld,max_reserve_band:%ld,group:%d.\n",
				p_te_param->cost,p_te_param->max_band,
				p_te_param->max_reserve_band,p_te_param->group);
			#endif
        }

    }
    /*check if te_route have valid link,if  don't,delete*/
    ospf_check_te_router(p_process);
    return;
}



/*type 10 lsa translate to te database used struct:te_route+te_link*/
void 
ospf_type10_lsa_to_tedb(struct ospf_area * p_area,struct ospf_lsa *p_lsa)
{
   // struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next = NULL;
    struct ospf_lshdr *p_ls_hdr = NULL;
    struct ospf_te_tlv *p_te_tlv_hdr = NULL;
    struct ospf_te_routeraddr_tlv *p_router_addr_tlv = NULL;
    struct ospf_process *p_process = p_area->p_process;
    u_int8 *p_sub_tlv;
    struct ospf_te_param *p_te_param;
    struct ospf_telink *p_te_link = NULL;

	if(NULL == p_lsa)
	{
		return ;
	}
	
    /*first check all ROUTER_ADDR tlv,create te_router;
    te_link must point to special te_route ,so create te_route first*/
  //  for_each_area_lsa(p_area, OSPF_LS_TYPE_10, p_lsa, p_next)
    {    
        p_ls_hdr = p_lsa->lshdr;
        
        if (OSPF_MAX_LSAGE <= ntohs(p_ls_hdr->age))
        {
            return;
        }
        p_te_tlv_hdr = (struct ospf_te_tlv*)(p_lsa->lshdr + 1);

        if (OSPF_TE_TLV_RTR_ADDR == ntohs(p_te_tlv_hdr->type))
        {
            p_router_addr_tlv = (struct ospf_te_routeraddr_tlv*)(p_lsa->lshdr + 1);

            /*router addr must >0*/
            if (0 == p_router_addr_tlv->value)
            {
                return;
            }
            /*if not exit, create*/
            if (NULL == ospf_te_router_lookup(p_process, ntohl(p_ls_hdr->adv_id)))
            {
                ospf_te_router_create(p_process, ntohl(p_ls_hdr->adv_id), p_area->id, ntohl(p_router_addr_tlv->value));
            }
        }
		else if (OSPF_TE_TLV_LINK == ntohs(p_te_tlv_hdr->type))
        {
            /*if not exit te_route, p_te_link will not create*/
            p_te_link = ospf_te_link_create(p_process, ntohl(p_ls_hdr->adv_id));
            if(NULL == p_te_link)
            {
                return;
            }
            p_te_param = &p_te_link->te_param;
            p_sub_tlv = (u_int8 *)(p_lsa->lshdr + 1) + sizeof(struct ospf_te_tlv);
			
			vty_out_to_all_terminal("ospf_type10_lsa_to_tedb p_te_tlv_hdr->h.len:%d",ntohs(p_te_tlv_hdr->len));
            ospf_te_get_param(p_sub_tlv, ntohs(p_te_tlv_hdr->len), p_te_param);
			p_te_param->lsa_id = ntohl(p_ls_hdr->id);
			vty_out_to_all_terminal("## ospf_te_get_param:link_type:%d,link_id:%x,local_addr:%x,remoter_addr:%x.",
				p_te_param->link_type,p_te_param->link_id,
				p_te_param->local_addr[0],p_te_param->remoter_addr[0]);
			
			vty_out_to_all_terminal("## ospf_te_get_param:cost:%d,max_band:%ld,max_reserve_band:%ld,group:%d.",
				p_te_param->cost,p_te_param->max_band,
				p_te_param->max_reserve_band,p_te_param->group);
			
			/*check if te_route have valid link,if don't,delete*/
			ospf_check_te_router(p_process);
        }
    }
    return;
}

