/* ospf_memory.c memory support for ospf */

#include "ospf.h"


 struct ospf_memory *p_memory = NULL;
extern u_long ulOspfmemFlag;
/*memory zone managment*/
/*compare of zone,use the laster buffer pointer as key*/
 int
ospf_mzone_cmp(
    struct ospf_mzone *p1,
    struct ospf_mzone *p2)
{
    u_long end1 = (u_long)p1->buf + p1->size;
    u_long end2 = (u_long)p2->buf + p2->size;
    if (end1 != end2)
    {
        return (end1 > end2) ? 1 : -1;
    }
    return 0;
}

/*create a zone*/
 struct ospf_mzone *
ospf_mzone_create(
     u_int8 *buf,
     u_int size)
{
    u_int i;
    if ((buf == NULL) || (size == 0))
    {
        return NULL;
    }
    
    for (i = 0 ; i < OSPF_MAX_MZONE; i++)
    {
        if (p_memory->zone[i].buf == NULL)
        {
            p_memory->zone[i].buf = buf;
            p_memory->zone[i].size = size;
            
            ospf_lstadd(&p_memory->zone_list, &p_memory->zone[i]);
            return &p_memory->zone[i];
        }
    }
    return NULL;
}

/*release a zone*/
 void
ospf_mzone_free(struct ospf_mzone *p)
{
    if (p->buf == NULL)
    {
        return;
    }
    ospf_lstdel(&p_memory->zone_list, p);
    p->buf = NULL;
    p->size = 0;
    return;
}

/*decide matched zone for special pointer,use when release memory*/
 struct ospf_mzone *
ospf_mzone_match(void *p)
{
    struct ospf_mzone search;
    /*get zone which pointer is larger then input pointer*/
    search.buf = p;
    search.size = 0;
    return ospf_lstgreater(&p_memory->zone_list, &search);
}

/*construct zone list for special memory size,we may allocate multiple zone for a large memory request*/
 void
ospf_mzone_list_create(u_int total)
{
    struct ospf_mzone *p_zone = NULL;
    u_int8 *buf = NULL;
    u_int zone_size = 0;
    u_int allocate = 0;
    u_int rest = 0;
    if (total == 0)
    {
        return;
    }
    
    /*decide closet zone size,from the min size to max size*/
    for (zone_size = OSPF_MIN_MZONE_SIZE;
           zone_size <= OSPF_MAX_MZONE_SIZE; 
           zone_size = (zone_size * 2))
    {
        /*if zone size is larger then request one,stop*/
        if ((zone_size * 2) > total)
        {
            break;
        }
    }
    if (total < OSPF_MIN_MZONE_SIZE)
    {
        zone_size = total;
    }

	/*allocate buffer using zone size,if full length allocated,or memory is not enough,stopped*/
    while (allocate < total)
    {
        /*allocate system buffer according to expected zone length*/
        buf = (void *)(ospf.malloc)(__FILE__, __LINE__,MTYPE_OSPF_TOP, zone_size);
        if (buf == NULL)
        {
            p_memory->zone_allocate_failed++;
            /*no buffer for this size,we may decrease zone size,but can not lower than min size*/
            if (zone_size == OSPF_MIN_MZONE_SIZE)
            {
                break;
            }
            /*select a lower size*/
            zone_size = zone_size/2;
            continue;
        }
        memset(buf, 0, zone_size);
        /*allocate zone for this memory*/
        p_zone = ospf_mzone_create(buf, zone_size);
        if (p_zone == NULL)
        {
            p_memory->zone_allocate_failed++;
            ospf.free(MTYPE_OSPF_TOP,buf);
            buf = NULL;
            break;
        }
   
        /*zone allocate sucessfully,increase allocate length*/
        allocate += zone_size;
   
        /*get rest length to be allocated*/
        rest = total - allocate;
        /*if zone size larger than rest length,use a shorter zone*/
        while ((zone_size > rest) && (zone_size > OSPF_MIN_MZONE_SIZE))
        {
            zone_size = zone_size/2;
        }
        if (rest < OSPF_MIN_MZONE_SIZE)
        {
            zone_size = rest;
        }		
    }
	
    return;
}

/*called when ospf shutdown,and free all memory*/
 void
ospf_mzone_list_clear(void)
{
    struct ospf_mzone *p_zone = NULL;
    struct ospf_mzone *p_next = NULL;
    for_each_node(&p_memory->zone_list, p_zone, p_next)
    {
        ospf.free(MTYPE_OSPF_TOP,p_zone->buf);
        ospf_mzone_free(p_zone);
    }
    return;
}
void
ospf_memory_check_timeout(void)
{
    struct ospf_mzone *p_zone = NULL;
    struct ospf_mzone *p_next_zone = NULL;
    struct ospf_mpage *page = NULL;
    u_int used = FALSE;
    u_int page_count = 0;
    u_int i = 0;
    
    for_each_node(&p_memory->zone_list, p_zone, p_next_zone)
    {
        used = FALSE;
        if (p_zone->expand == TRUE)
        {
            page = (struct ospf_mpage *)p_zone->buf;
            page_count = p_zone->size/OSPF_PAGE_SIZE;
            
            for (i = 0 ; i < page_count ; i++)
            {    
                /*this mzone have page used ,can't free*/
                if (page->free != page->total)
                {
                     used = TRUE;
                     break;
                }
                page = (struct ospf_mpage *)((u_int8 *)page + OSPF_PAGE_SIZE);
            }
            /*page all free ,can free this mzone*/
            if (used == FALSE)
            {
                page = (struct ospf_mpage *)p_zone->buf;
                for (i = 0 ; i < page_count ; i++)
                {    
                    if (page->p_mtable)
                    {
                        ospf_lstdel_unsort(&page->p_mtable->page_list, page);
                        page->p_mtable->page_free++; 
                        page->p_mtable = NULL;
                    }
                    else
                    {
                        ospf_lstdel_unsort(&p_memory->free_page_list, page);  
                    }
                    page->total = 0;
                    page->free = 0;
                    page->p_free = NULL;  
                    page = (struct ospf_mpage *)((u_int8 *)page + OSPF_PAGE_SIZE);
                }
                ospf.free(MTYPE_OSPF_TOP,p_zone->buf);
                ospf_mzone_free(p_zone);
            }

        }
    }
    ospf_stimer_start(&p_memory->mzone_timer, OSPF_MEMORY_TIMER_INTERVAL);
    return ;
}
 void
ospf_memory_expand(void)
{
    struct ospf_mzone *p_zone = NULL;
    struct ospf_mpage *page = NULL;
    u_int page_count = 0;    
    u_int i = 0;
    u_int8 *buf = NULL;
    u_int expand_size = OSPF_EXPAND_MBLOCK_SIZE;

    /*expand memory will not exceed the total_len/4*/
    /*if ((p_memory->expand_len != 0)
        && (p_memory->expand_len+expand_size) > p_memory->total_len/4)
    {
         return;
    }*/
    buf = (u_int8 *)(ospf.malloc)(__FILE__, __LINE__,MTYPE_OSPF_TOP, expand_size);
    if (NULL == buf)
    {
        return;
    }
    p_zone = ospf_mzone_create(buf, expand_size);
    if (NULL == p_zone)
    {
        p_memory->zone_allocate_failed++;
        ospf.free(MTYPE_OSPF_TOP,buf);
        buf = NULL;
        return;
    }
    p_zone->expand = TRUE;
    memset(buf, 0, expand_size);
    
    /*construct free page list*/
    page = (struct ospf_mpage *)buf;
    page_count = expand_size/OSPF_PAGE_SIZE;
    
    for (i = 0 ; i < page_count ; i++)
    {
        ospf_lstadd_unsort(&p_memory->free_page_list, page);
        page = (struct ospf_mpage *)((u_int8 *)page + OSPF_PAGE_SIZE);
    }
    p_memory->expand_len += expand_size;
    ospf_stimer_start(&p_memory->mzone_timer, OSPF_MEMORY_TIMER_INTERVAL);
    return;
}

void 
ospf_set_init_num(
                  u_int max_process,
                  u_int max_lsa,
                  u_int max_route,
                  u_int max_if,
                  u_int max_area,
                  u_int max_nbr)
{
    u_int i = 0;
    /*create memory information*/
    if (NULL == p_memory)
    {
        p_memory = (struct ospf_memory *)XMALLOC(MTYPE_OSPF_TOP,sizeof(struct ospf_memory));
        if (NULL == p_memory)
        {
            ospf_logx(ospf_debug,"\n\rmemory init failed");
            return;
        }
    }

    memset(p_memory, 0, sizeof(struct ospf_memory));
    
    /*adjust input value,we have min value*/
    if (max_process < OSPF_MAX_PROCESS_NUM)
    {
        max_process = OSPF_MAX_PROCESS_NUM;
    }
    if (max_if < OSPF_MAX_IF_NUM)
    {
        max_if = OSPF_MAX_IF_NUM;
    }
    
    if (max_area < OSPF_MAX_AREA_NUM)
    {
        max_area = OSPF_MAX_AREA_NUM;
    }
    
    if (max_nbr < OSPF_MAX_NBR_NUM)
    {
        max_nbr = OSPF_MAX_NBR_NUM;
    }
    
    if (max_lsa < OSPF_MAX_LSA_NUM)
    {
        max_lsa = OSPF_MAX_LSA_NUM;
    }
    
    if (max_route < OSPF_MAX_ROUTE_NUM)
    {
        max_route = OSPF_MAX_ROUTE_NUM;
    }
    p_memory->max_process = max_process;
    p_memory->max_lsa = max_lsa;
    p_memory->max_route = max_route;
    p_memory->max_if = max_if;
    p_memory->max_area = max_area;
    p_memory->max_nbr = max_nbr;
    for (i = 0 ; i < OSPF_MAX_MEMPOOL; i++)
    {
        ospf_lstinit(&p_memory->table[i].page_list, NULL);
        ospf_lstinit(&p_memory->table[i].full_page_list, NULL);
    }
    ospf_lstinit(&p_memory->free_page_list, NULL);
    ospf_lstinit(&p_memory->zone_list, ospf_mzone_cmp);
    ospf_timer_init(&p_memory->mzone_timer, NULL, ospf_memory_check_timeout, NULL);
    return;
}

/*create memory for ospf running,must be called before ospf startup*/
void 
ospf_init_memory(void)
{


    struct ospf_mpage *p_page = NULL;
    struct ospf_mzone *p_zone = NULL;
    struct ospf_mzone *p_next = NULL;
    u_int page_count = 0;
    u_int i;
    u_int max_process = p_memory->max_process;
    u_int max_lsa = p_memory->max_lsa;
    u_int max_route = p_memory->max_route;
    u_int max_if = p_memory->max_if;
    u_int max_area = p_memory->max_area;
    u_int max_nbr = p_memory->max_nbr;


    /*reset statistics*/
    memset(p_memory->stat, 0, sizeof(struct ospf_mstat) * OSPF_MTYPE_MAX);

    p_memory->zone_allocate_failed = 0;
    
    /*init unit length for each pool*/
    p_memory->table[0].size = 48;
    p_memory->table[1].size = 72;
    p_memory->table[2].size = 128;
    p_memory->table[3].size = 256;
    p_memory->table[4].size = 600;
    p_memory->table[5].size = 1024;
    p_memory->table[6].size = OSPF_DEFAULT_IP_MTU + 100;    
    p_memory->table[7].size = OSPF_MAX_TXBUF;
    p_memory->table[8].size = OSPF_PAGE_SIZE - sizeof(struct ospf_mpage);
//	ospf_logx(ospf_debug,"ospf_init_memory##%s:%d\r\n",__FUNCTION__,__LINE__);
    p_memory->total_len = 0;
    
    /*init memory config,set length and count*/
    memset(p_memory->config, 0, sizeof(struct ospf_memconfig) * OSPF_MTYPE_MAX);
    
    p_memory->config[OSPF_MNETWORK].len = sizeof(struct ospf_network);
    p_memory->config[OSPF_MNETWORK].count = max_if;

    p_memory->config[OSPF_MIF].len = sizeof (struct ospf_if);
    p_memory->config[OSPF_MIF].count = max_if;
    
    p_memory->config[OSPF_MAREA].len = sizeof (struct ospf_area);
    p_memory->config[OSPF_MAREA].count = max_area;
      
    p_memory->config[OSPF_MSPF].len = sizeof (struct ospf_spf_vertex);
    p_memory->config[OSPF_MSPF].count = OSPF_MAX_SPF_NUM;
    
    p_memory->config[OSPF_MIPROUTE].len = sizeof (struct ospf_iproute);
    p_memory->config[OSPF_MIPROUTE].count = (max_route / 40);
    
    p_memory->config[OSPF_MRXMT].len = sizeof (struct ospf_retransmit);  
    p_memory->config[OSPF_MRXMT].count = max_lsa; 
    
    p_memory->config[OSPF_MROUTE].len = sizeof (struct  ospf_route);  
    p_memory->config[OSPF_MROUTE].count = /*max_route*2*/max_route + max_route/10; 
	//ospf_logx(ospf_debug,"ospf_init_memory##%s:%d\r\n",__FUNCTION__,__LINE__);
    p_memory->config[OSPF_MNBR].len = sizeof (struct ospf_nbr);
    p_memory->config[OSPF_MNBR].count = max_nbr; 
    
    p_memory->config[OSPF_MRANGE].len = sizeof (struct ospf_range); 
    p_memory->config[OSPF_MRANGE].count = OSPF_MAX_RANGE_NUM;
     
    p_memory->config[OSPF_MREQUEST].len = sizeof (struct ospf_request_node);
    p_memory->config[OSPF_MREQUEST].count = (max_lsa / 2) + (max_lsa / 4);
      
    p_memory->config[OSPF_MREDISTRIBUTE].len = sizeof(struct ospf_redistribute); 
    p_memory->config[OSPF_MREDISTRIBUTE].count = OSPF_MAX_REDISTRIBUTE_NUM;
                 
    p_memory->config[OSPF_MTEROUTER].len = sizeof(struct ospf_terouter);
    p_memory->config[OSPF_MTEROUTER].count = OSPF_MAX_TEROUTER_NUM;
	//ospf_logx(ospf_debug,"ospf_init_memory##%s:%d\r\n",__FUNCTION__,__LINE__);
    p_memory->config[OSPF_MTELINK].len = sizeof(struct ospf_telink);
    p_memory->config[OSPF_MTELINK].count = OSPF_MAX_TELINK_NUM;
   
    p_memory->config[OSPF_MINSTANCE].len = sizeof(struct ospf_process);
    p_memory->config[OSPF_MINSTANCE].count = max_process;
      
    p_memory->config[OSPF_MPOLICY].len = sizeof(struct ospf_policy);
    p_memory->config[OSPF_MPOLICY].count = OSPF_MAX_POLICY_NUM;
    
    p_memory->config[OSPF_MDBD].len = OSPF_DEFAULT_IP_MTU + 100; 
    p_memory->config[OSPF_MDBD].count = max_nbr;

    p_memory->config[OSPF_MLSTABLE].len = sizeof(struct ospf_lstable);
    p_memory->config[OSPF_MLSTABLE].count = max_area * OSPF_LS_MAX;
    p_memory->config[OSPF_MTETUNNEL].len = sizeof(struct ospf_te_tunnel);
    p_memory->config[OSPF_MTETUNNEL].count = OSPF_MAX_TETUNNEL_NUM;
	//ospf_logx(ospf_debug,"ospf_init_memory##%s:%d\r\n",__FUNCTION__,__LINE__);

#ifdef OSPF_DCN
    p_memory->config[OSPF_MDCN].len = sizeof(struct ospf_dcn_rxmt_node); 
    p_memory->config[OSPF_MDCN].count = OSPF_MAX_DCN_NUM;
#endif    

    p_memory->config[OSPF_MASBRRANGE].len = sizeof(struct ospf_asbr_range); 
    p_memory->config[OSPF_MASBRRANGE].count = OSPF_MAX_ASBRRANGE_NUM;

    /*set start mempool index incease count of each mempool*/
    for (i = 0 ; i < OSPF_MTYPE_MAX; i++)
    {   
	//	ospf_logx(ospf_debug,"ospf_init_memory##%s:%d\r\n",__FUNCTION__,__LINE__);
        ospf_increase_memory(p_memory->config[i].len, p_memory->config[i].count);
    }
	//ospf_logx(ospf_debug,"ospf_init_memory##%s:%d\r\n",__FUNCTION__,__LINE__);
        
    /*LSA liner buffer,use external lsa as length*/
    ospf_increase_memory(sizeof(struct ospf_lsa) + 16, max_lsa);     
    ospf_increase_memory(OSPF_MAX_TXBUF, 10);  
	//ospf_logx(ospf_debug,"ospf_init_memory##%s:%d\r\n",__FUNCTION__,__LINE__);

    /*aligment according to page*/
    p_memory->total_len = ((p_memory->total_len / OSPF_PAGE_SIZE) + 1) * OSPF_PAGE_SIZE;

//	ospf_logx(ospf_debug,"ospf_init_memory##%s:%d\r\n",__FUNCTION__,__LINE__);
    /*construct memory zone list*/
    if (p_memory->total_len > OSPF_MEMORY_INIT_SPLIT_SIZE)        
    {
		ospf_mzone_list_create(p_memory->total_len/4);
    }
    else
    {
        ospf_mzone_list_create(p_memory->total_len);
		
    }
    /*split all memory into pages*/
    for_each_node(&p_memory->zone_list, p_zone, p_next)
    {
        /*construct free page list*/
        p_page = (struct ospf_mpage *)p_zone->buf;
        page_count = p_zone->size/OSPF_PAGE_SIZE;

        for (i = 0 ; i < page_count ; i++)
        {
            ospf_lstadd_unsort(&p_memory->free_page_list, p_page);
            p_page = (struct ospf_mpage *)((u_int8 *)p_page + OSPF_PAGE_SIZE);
        }
    }
    return;
}

/*delete all memory table*/
void 
ospf_destory_memory(void)
{
    struct ospf_mpage *p_page = NULL;
    struct ospf_mpage *p_next = NULL;
    u_int i = 0;

    /*clear all page list*/
    for (i = 0 ; i < OSPF_MAX_MEMPOOL; i++)
    {
        for_each_node(&p_memory->table[i].page_list, p_page, p_next)
        {
            ospf_lstdel_unsort(&p_memory->table[i].page_list, p_page);
            p_memory->table[i].page_free++;
        }
        for_each_node(&p_memory->table[i].full_page_list, p_page, p_next)
        {
            ospf_lstdel_unsort(&p_memory->table[i].full_page_list, p_page);
            p_memory->table[i].page_free++;
        }
    }
 
    /*clear free page list*/
    for_each_node(&p_memory->free_page_list, p_page, p_next)
    {
        ospf_lstdel_unsort(&p_memory->free_page_list, p_page);
    }
    /*free global buffer*/
    ospf_mzone_list_clear();
    return;
}

void
ospf_mem_garbage_collect(void)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;

    return;
    for_each_ospf_process(p_process, p_next_process)
    {
        ospf_spf_log_clear(p_process);
        ospf_nbr_log_clear(p_process);
        ospf_lsa_log_clear(p_process);
    }   
    return;
}

 struct ospf_mpage *
ospf_page_alloc(struct ospf_mtable *p_table)
{
    struct ospf_mhdr *p_hdr = NULL;
    struct ospf_mhdr *p_next = NULL;
    struct ospf_mpage *p_page = NULL;
    struct ospf_mpage *p_nextpage = NULL;
    u_int total = 0;
    u_int i;

    if (!p_table->size)
    {
        return NULL;
    }
    total = (u_short)((OSPF_PAGE_SIZE - sizeof(struct ospf_mpage)) / p_table->size);
    if (total < 1)
    {
        return NULL;
    }

    /*no free object,try to get pages from free list*/
    p_page = ospf_lstfirst(&p_memory->free_page_list);
    if (!p_page)
    {
        /*try to get free page from other table*/
        for (i = 0 ; i < OSPF_MAX_MEMPOOL; i++)
        {
            for_each_node(&p_memory->table[i].page_list, p_page, p_nextpage)
            {
                if (p_page->total == p_page->free)
                {
                    ospf_lstdel_unsort(&p_memory->table[i].page_list, p_page);
                    ospf_lstadd_unsort(&p_memory->free_page_list, p_page);
                    p_page->p_mtable->page_free++;
                    p_page->p_mtable = NULL;
                }
            }
        }      
    }
    p_page = ospf_lstfirst(&p_memory->free_page_list);
    if (!p_page)
    {
        /*allocate burst buffer */ 
        ospf_memory_expand();
        p_page = ospf_lstfirst(&p_memory->free_page_list);
        if (NULL == p_page)
        {
            return NULL;
        }       
    }
    if (ospf_lstcnt(&p_memory->free_page_list) < 3)
    {
        ospf_mem_garbage_collect();
    }
    ospf_lstdel_unsort(&p_memory->free_page_list, p_page);
    ospf_lstadd_unsort(&p_table->page_list, p_page);
    p_page->total = total;
    
    p_hdr = (struct ospf_mhdr *)(p_page + 1);
    p_page->p_free = p_hdr;
    p_hdr->p_next = NULL;
    p_page->free = p_page->total;    
    
    p_page->p_mtable = p_table;

    for (i = 0 ; i < (p_page->total - 1); i++)
    {
        p_next = (void*)(((u_long)p_hdr) + p_table->size);
        p_hdr->p_next = p_next;
        p_next->p_next = NULL;
        p_hdr = p_next;
    }
    p_table->page_alloc++;
    return p_page;
}

/*ospf_malloc:allocate one buffer unit from buffer table*/ 
void *
ospf_malloc(
    u_int size, 
    u_short type)
{
    struct ospf_mtable *p_table = NULL;
    struct ospf_mpage *p_page = NULL;
    u_int i;
    u_int zero_len = 0;
    u_int8 *p_buf = NULL;
    struct ospf_lst *p_tmp = NULL;
    void *p_vid = NULL;
    
    if(ulOspfmemFlag == 1)
    {
         vty_out_to_all_terminal("ospf_malloc %d:size = %d,type = %d",__LINE__,size,type);
    }
    p_memory->stat[type].allocate++;
    p_memory->histroy_stat[type].allocate++;

    /*input is 0, so use length in memconfig*/
    if (0 == size)
    {
        size = p_memory->config[type].len;
        if (0 == size)
        {
            ospf_logx(ospf_debug,"\n\rospfv2 invalid null size need,type %d", type);
            goto FAIL;
        }
    }

    /*for large buffer,do not init all buffer,just header*/
    if (((OSPF_MPACKET == type) || (OSPF_MDBD == type)) && (128 < size ))
    {
        zero_len = 128;
    }
    else
    {
        zero_len = size;
    }

    /*get matched memory table*/
    for (i = 0 ; i < OSPF_MAX_MEMPOOL; i++)
    {
        if (p_memory->table[i].size >= size)
        {
            p_table = &p_memory->table[i];
            break;
        }
    }
    
    /*no matched pool, do nothing*/
    if (!p_table)
    {
        goto FAIL;
    }

    if(ulOspfmemFlag == 1)
    {
        vty_out_to_all_terminal("ospf_malloc %d:zero_len = %d,page_list = %x,",__LINE__,zero_len,&p_table->page_list);
        p_tmp = &p_table->page_list;
        if(p_tmp != NULL)
        {
            vty_out_to_all_terminal("ospf_malloc %d:p_tmp = %x",__LINE__,p_tmp);
            if(p_tmp->p_first == NULL)
            {
                vty_out_to_all_terminal("(p_tmp->p_first == NULL)");
               // printf("ospf_malloc %d:p_first = %x\r\n",__LINE__,p_tmp->p_first);
               // goto FAIL;

            }
            else
            {
                vty_out_to_all_terminal("ospf_malloc %d:p_first = %x",__LINE__,p_tmp->p_first);
            }
            
            p_vid = &(p_tmp->avl);
            if(( p_vid == NULL)&&(p_tmp->p_first == NULL))
            {
                 vty_out_to_all_terminal("(( p_vid == NULL)&&(p_tmp->p_first == NULL))");
                 goto FAIL;
              // printf("ospf_malloc %d:avl = %x,\r\n",__LINE__,p_vid);
            }
            if( p_vid != NULL)
            {
                 vty_out_to_all_terminal("ospf_malloc %d:avl = %x,",__LINE__,p_vid);
            }
        }
    }
    /*get first page,if not exist,try to allocate a page from free list*/
    p_page = ospf_lstfirst(&p_table->page_list);
    if(ulOspfmemFlag == 1)
    {
        vty_out_to_all_terminal("ospf_malloc %d:zero_len = %d,p_page = %x,",__LINE__,zero_len,p_page);
    }
    if (!p_page)
    {
        p_page = ospf_page_alloc(p_table); 
        if(ulOspfmemFlag == 1)
        {
            vty_out_to_all_terminal("ospf_malloc %d:p_table = %x,p_page = %x,",__LINE__,p_table,p_page);
        }
        if (!p_page)
        {
            goto FAIL;
        }
    }
    if(ulOspfmemFlag == 1)
    {
        vty_out_to_all_terminal("ospf_malloc %d: p_free = %x ,free = %d",__LINE__,p_page->p_free,p_page->free);

        if(p_page->p_free == NULL)
         {

            vty_out_to_all_terminal("ospf_malloc %d:(p_page->p_free == NULL) p_free = %x ,free = %d",__LINE__,p_page->free);
            goto FAIL;
         }
        // if(p_page->p_free->p_next == NULL)
         {
          //  printf("ospf_malloc %d: (p_page->p_free->p_next == NULL) p_free = %x ,free = %d\r\n",__LINE__,p_page->p_free,p_page->free);
         }
    }
    
    p_buf = (void *)p_page->p_free;
    p_page->p_free = p_page->p_free->p_next;
    p_page->free--;
    memset(p_buf, 0, zero_len);

    /*if no free memory,add to full list*/
    if (!p_page->p_free)
    {
        ospf_lstdel_unsort(&p_table->page_list, p_page);
        ospf_lstadd_unsort(&p_table->full_page_list, p_page);
    }
    return p_buf;        
FAIL:
    p_memory->stat[type].fail++;
    p_memory->histroy_stat[type].fail++;
    return NULL;
}

/*release one buffer unit,insert it back into free list*/
void 
ospf_mfree(
    void *ptr, 
    u_int type)
{   
    struct ospf_mhdr *p_hdr = (struct ospf_mhdr *)ptr;
    struct ospf_mpage *p_page = NULL;
    struct ospf_mzone *p_zone = NULL;
    u_long offset = 0;
    
    p_memory->stat[type].del++;
    p_memory->histroy_stat[type].del++;

    /*decide zone it belong to*/
    p_zone = ospf_mzone_match(ptr);
    if (p_zone == NULL)
    {
       ospf_logx(ospf_debug,"no memory zone matched to free buffer %x\n\r",(int)ptr);
       return;
    }
    
    /*decide page*/
    offset = ((u_long)ptr-(u_long)p_zone->buf)/OSPF_PAGE_SIZE;
    p_page = (struct ospf_mpage *)(p_zone->buf + offset*OSPF_PAGE_SIZE);

  //  memset(p_hdr, 'x', p_page->p_mtable->size);
    memset(p_hdr, 0, p_page->p_mtable->size);

    /*add to free list*/
    p_hdr->p_next = p_page->p_free;
    p_page->p_free = p_hdr;
    p_page->free++;

    if (p_page->p_mtable)
    {
         /*if free count is 1,add to page list*/
         if (p_page->free == 1)
         {
             ospf_lstdel_unsort(&p_page->p_mtable->full_page_list, p_page);
             ospf_lstadd_unsort(&p_page->p_mtable->page_list, p_page);
         }
    }
    return;
}

#define MCURRENT_CNT(type) ((int)(p_memory->stat[type].allocate - p_memory->stat[type].fail - p_memory->stat[type].del))

void 
ospf_verify_memory(void)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    struct ospf_lsa_fragment *p_frag = NULL;
    struct ospf_lsa_fragment *p_nextfrag = NULL;
    int count[OSPF_MTYPE_MAX] = {0};
    int type;
    int i;


    if (OK != ospf_semtake_timeout())
    {
        return;
    }
  
    count[OSPF_MINSTANCE] += ospf_lstcnt(&ospf.process_table);
    count[OSPF_MSTAT] += ospf_lstcnt(&ospf.log_table.nbr_table);
    count[OSPF_MSTAT] += ospf_lstcnt(&ospf.log_table.lsa_table);
    count[OSPF_MSTAT] += ospf_lstcnt(&ospf.log_table.spf_table);
#ifdef HAVE_BFD
    count[OSPF_MSTAT] += ospf_lstcnt(&ospf.log_table.bfd_table);
#endif
    count[OSPF_MPACKET] += ospf_lstcnt(&ospf.syn_control_table);
    
    for_each_ospf_process(p_process, p_next_process)
    {
        count[OSPF_MPOLICY] += ospf_lstcnt(&p_process->filter_policy_table);
        count[OSPF_MPOLICY] +=ospf_lstcnt(&p_process->redis_policy_table);
        
        count[OSPF_MNEXTHOP] += ospf_lstcnt(&p_process->nexthop_table);
              
        count[OSPF_MTEROUTER] += ospf_lstcnt(&p_process->te_router_table);
                 
        count[OSPF_MREDISTRIBUTE] += ospf_lstcnt(&p_process->redistribute_config_table);
    
        count[OSPF_MNETWORK] += ospf_lstcnt(&p_process->network_table);
      
        count[OSPF_MAREA] += ospf_lstcnt(&p_process->area_table);
        
        count[OSPF_MIF] += ospf_lstcnt(&p_process->if_table);
        
        count[OSPF_MIPROUTE] += ospf_lstcnt(&p_process->filtered_route_table);
        count[OSPF_MIPROUTE] += ospf_lstcnt(&p_process->import_table);
    
        count[OSPF_MROUTE] += ospf_lstcnt(&p_process->route_table);
    
        count[OSPF_MNBR] += ospf_lstcnt(&p_process->nbr_table);
    
        count[OSPF_MRXMT] += ospf_lstcnt(&p_process->rxmt_table); 
    
        count[OSPF_MLSA] += ospf_lstcnt(&p_process->t5_lstable.list); 
        count[OSPF_MLSA] += ospf_lstcnt(&p_process->t11_lstable.list);

        count[OSPF_MPACKET] += ospf_lstcnt(&p_process->t5_lstable.conflict_list);
     
        count[OSPF_MPACKET] += ospf_lstcnt(&p_process->t11_lstable.conflict_list);
        
        if (p_process->p_rtmsg)
        {
            count[OSPF_MPACKET]++;
        }

        count[OSPF_MREQUEST] += ospf_lstcnt(&p_process->req_table);

        count[OSPF_MPACKET] += ospf_lstcnt(&p_process->fragment_table);
        
#ifdef OSPF_FRR
        count[OSPF_MBACKROUTE] += ospf_lstcnt(&p_process->backup_route_table);
#endif
        count[OSPF_MTETUNNEL] += ospf_lstcnt(&p_process->te_tunnel_table);

#ifdef OSPF_DCN
        count[OSPF_MDCN] += ospf_lstcnt(&p_process->dcn_rtm_tx_table);
#endif		 

        
        count[OSPF_MASBRRANGE] += ospf_lstcnt(&p_process->asbr_range_table);

        for_each_node(&p_process->fragment_table, p_frag, p_nextfrag) 
        {
            for (i = 0 ; i < 64; i++)
            {
                if (p_frag->fragment[i].p_buf)
                {
                    count[OSPF_MPACKET]++;
                }
            }
        }
        
        for_each_ospf_area(p_process, p_area, p_next_area)
        {
            count[OSPF_MSPF] += ospf_lstcnt(&p_area->spf_table);
            
            count[OSPF_MROUTE] += ospf_lstcnt(&p_area->asbr_table);
            count[OSPF_MROUTE] += ospf_lstcnt(&p_area->abr_table);
    
            count[OSPF_MRANGE] += ospf_lstcnt(&p_area->range_table);
    
            for (type = OSPF_LS_ROUTER; type <= OSPF_LS_TYPE_10; type++) 
            {
                if (p_area->ls_table[type])
                {
                    count[OSPF_MLSA] += ospf_lstcnt(&p_area->ls_table[type]->list);             
                    count[OSPF_MPACKET] += ospf_lstcnt(&p_area->ls_table[type]->conflict_list);
                    count[OSPF_MLSTABLE]++;              
                }
            }            
        }
    
        for_each_node(&p_process->nbr_table, p_nbr, p_next_nbr)
        {
            if (p_nbr->p_last_dd)
            {
                count[OSPF_MDBD]++;
            }
        }
    
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
            count[OSPF_MLSA] += ospf_lstcnt(&p_if->opaque_lstable.list);
            
            count[OSPF_MPACKET] += ospf_lstcnt(&p_if->opaque_lstable.conflict_list);
                
            if (p_if->update.p_msg)
            {
                count[OSPF_MPACKET]++;
            }
            if (p_if->delay_ack.p_msg)
            {
                count[OSPF_MPACKET]++;
            }
            if (p_if->p_direct_ack
                && p_if->p_direct_ack->p_msg)
            {
                count[OSPF_MPACKET]++;
            }
            if (p_if->stat)
            {
                count[OSPF_MSTAT]++;
            }
        }
    }

    /*result*/
    for (i = 0 ; i < OSPF_MTYPE_MAX ; i++)
    {
        if (MCURRENT_CNT(i) != count[i])
        {
            ospf_logx(ospf_debug,"\n\r Memory Use Error,type=%d,current=%d,calculate=%d\n\r", i, MCURRENT_CNT(i), count[i]);
        }
    }
    ospf_semgive();

	return;
}

#define _MSTAT(st, len, desc) do{\
    int current = st.allocate - st.fail - st.del;\
    /* if (ospf.stat.name.allocate)*/\
   ospf_logx(1," %-10d%-10u%-10u%-10u%-10u%-8u%s\n\r",i++,(unsigned int)st.allocate,\
                           (unsigned int)st.fail,(unsigned int)st.del,current, (unsigned int)(len),(desc));\
   free_count += st.del;\
}while(0)

/*calculate total used memory*/
 u_int
ospf_memory_used(void)
{
    struct ospf_mpage *p_page = NULL;
    struct ospf_mpage *p_next = NULL;
    struct ospf_mtable *p_table;
    u_int used = 0;
    u_int i;

	
    ospf_semtake_forever();

    for (i = 0; i < OSPF_MAX_MEMPOOL; i++)        
    {
        p_table = &p_memory->table[i];

        for_each_node(&p_table->page_list, p_page, p_next)
        {
            used += (p_page->total - p_page->free) * p_table->size;
        }
        used += ospf_lstcnt(&p_table->full_page_list) * OSPF_PAGE_SIZE;
    }
	
    ospf_semgive();

    return used;
}

#define OSPF_MSTAT(type, len, desc) _MSTAT(p_memory->stat[type], len, desc)
#define OSPF_HISTROY_MSTAT(type, len, desc) _MSTAT(p_memory->histroy_stat[type], len, desc)

void 
ospf_display_memory_statistics(void)
{
    struct ospf_mtable *p_table;
    struct ospf_mpage *p_page = NULL;
    struct ospf_mpage *p_next = NULL;
    struct ospf_mzone *p_zone = NULL;
    u_int allocate = 0;
    int free_count = 0;
    int total = 0;
    int i ;
    int emptypage = 0;
    
    if (NULL == p_memory)
    {
        ospf_logx(1,"ospf not running\r\n");
        return;
    }
    ospf_logx(1,"\n\r Memory Statistics\n\r");
    ospf_logx(1," Configuration Params:\n\r");
    ospf_logx(1,"  Process:%d\n\r",(int)p_memory->max_process);
    ospf_logx(1,"  LSA:%d\n\r",(int)p_memory->max_lsa);
    ospf_logx(1,"  Route:%d\n\r",(int)p_memory->max_route);
    ospf_logx(1,"  Interface:%d\n\r",(int)p_memory->max_if);
    ospf_logx(1,"  Neighbor:%d\n\r",(int)p_memory->max_nbr);
    ospf_logx(1,"  Area:%d\n\r",(int)p_memory->max_area);
    ospf_logx(1,"  Memory Table Size %d,Zone Count %d,Total expected %d bytes\n\r", 
        sizeof(struct ospf_memory), (int)ospf_lstcnt(&p_memory->zone_list), (int)p_memory->total_len);    

    ospf_logx(1," Page size %d,Empty page count %d\n\r", OSPF_PAGE_SIZE, (int)ospf_lstcnt(&p_memory->free_page_list));  

    ospf_logx(1,"\n\r Memory Zone Table\n\r");
    ospf_logx(1," %-5s%-16s%-s\n\r","id","size","buffer");
    for (i = 0 ; i < OSPF_MAX_MZONE ; i++)
    {
        p_zone = &p_memory->zone[i];
        if (p_zone->buf)
        {
            ospf_logx(1," %-5d%-16d%x\n\r",(int)i,(int)p_zone->size,(int)p_zone->buf);
            allocate += p_zone->size;
        }
    }
    ospf_logx(1,"\n\r Total Allocate memory zone length %d bytes, failed %d\n\r", (int)allocate, (int)p_memory->zone_allocate_failed);

    if (allocate)
    {
        int used = ospf_memory_used();
        ospf_logx(1," Total Used Memory %dbytes, Percent %d%%\n\r", (int)used, (int)(((used>>10)*100)/(allocate>>10)));
    }
    
    ospf_logx(1,"\n\r Memory Page Table\n\r");    
    ospf_logx(1," %-5s%-6s%-6s%-10s%-10s%-10s%-10s%-10s%-10s\n\r",
          "id","size","page","fullpage","emptypage","allocpage","freepage","total","avaliable");

    for (i = 0;i < OSPF_MAX_MEMPOOL;i++)        
    {
        p_table = &p_memory->table[i];

        free_count = 0;
        total = 0;
        for_each_node(&p_table->page_list, p_page, p_next)
        {
            free_count += p_page->free;
            total += p_page->total;
        }
        for_each_node(&p_table->full_page_list, p_page, p_next)
        {
            total += p_page->total;
        }
        emptypage = 0;
        for_each_node(&p_table->page_list, p_page, p_next)
        {
            if (p_page->free == p_page->total)
            {
                emptypage++;
            }
        }
        ospf_logx(1," %-5d%-6d%-6d%-10d%-10d%-10d%-10d%-10d%-10d\n\r",
        i+1,
        (int)p_table->size,
        (int)ospf_lstcnt(&p_table->page_list),
        (int)ospf_lstcnt(&p_table->full_page_list),
         emptypage,
        (int)p_table->page_alloc,(int)p_table->page_free, total, free_count); 
    }

    i = 0;
    free_count = 0;
    ospf_logx(1,"\n\rDetailed Memory Operation\n\r");        
    ospf_logx(1," %-10s%-10s%-10s%-10s%-10s%-8s%s\n\r","Index","Add","Fail","Free","Current","Size","Description");
 
    OSPF_MSTAT(OSPF_MPACKET, OSPF_MAX_TXBUF, "Packet");
    OSPF_MSTAT(OSPF_MINSTANCE,sizeof (struct ospf_process),"Instance");
    OSPF_MSTAT(OSPF_MAREA,sizeof (struct ospf_area),"Area");
    OSPF_MSTAT(OSPF_MSPF,sizeof (struct ospf_spf_vertex),"SPF Node");
    OSPF_MSTAT(OSPF_MIF, sizeof (struct ospf_if),"Interface");
    OSPF_MSTAT(OSPF_MIPROUTE,sizeof (struct ospf_iproute),"External Route");
    OSPF_MSTAT(OSPF_MRXMT,sizeof (struct ospf_retransmit),"Retransmit Node");
    OSPF_MSTAT(OSPF_MDBD, OSPF_DEFAULT_IP_MTU+100,"DD summary packet");
    OSPF_MSTAT(OSPF_MLSA, sizeof (struct ospf_lsa),"LSA");
    OSPF_MSTAT(OSPF_MROUTE, sizeof (struct ospf_route),"Route Entry");
    OSPF_MSTAT(OSPF_MNEXTHOP, sizeof (struct ospf_nexthop),"Nexthop");
    OSPF_MSTAT(OSPF_MPOLICY, sizeof (struct ospf_policy),"Route Policy");
    OSPF_MSTAT(OSPF_MNBR, sizeof (struct ospf_nbr),"Neighbor");
    OSPF_MSTAT(OSPF_MRANGE, sizeof (struct ospf_range),"Address Range");
    OSPF_MSTAT(OSPF_MREQUEST, sizeof (struct ospf_request_node),"Request Node");
    OSPF_MSTAT(OSPF_MNETWORK, sizeof(struct ospf_network),"Network Config");
    OSPF_MSTAT(OSPF_MREDISTRIBUTE, sizeof(struct ospf_redistribute), "Redistribue Config");
    OSPF_MSTAT(OSPF_MLSTABLE, sizeof (struct ospf_lstable),"Lsa Table");
    OSPF_MSTAT(OSPF_MSTAT, sizeof (struct ospf_if_stat),"Stat");
 #ifdef OSPF_FRR
    OSPF_MSTAT(OSPF_MBACKSPF, sizeof (struct ospf_spf_vertex),"Bckup Spf");
    OSPF_MSTAT(OSPF_MBACKROUTE, sizeof (struct ospf_route),"Backup Route");
 #endif
    OSPF_MSTAT(OSPF_MTETUNNEL, sizeof (struct ospf_te_tunnel),"TE Tunnel");
 
#ifdef OSPF_DCN
    OSPF_MSTAT(OSPF_MDCN, sizeof (struct ospf_dcn_rxmt_node),"Dcn rxmt");
#endif
    
    OSPF_MSTAT(OSPF_MASBRRANGE, sizeof(struct ospf_asbr_range),"asbr range");
    i = 0 ;
    ospf_logx(1,"\n\rDetailed Total Memory Operation:\n\r");
    ospf_logx(1," %-10s%-10s%-10s%-10s%-10s%-8s%s\n\r","Index","Add","Fail","Free","Current","Size","Description");
     
    OSPF_HISTROY_MSTAT(OSPF_MPACKET, OSPF_MAX_TXBUF, "Packet");
    OSPF_HISTROY_MSTAT(OSPF_MINSTANCE,sizeof (struct ospf_process),"Instance");
    OSPF_HISTROY_MSTAT(OSPF_MAREA,sizeof (struct ospf_area),"Area");
    OSPF_HISTROY_MSTAT(OSPF_MSPF,sizeof (struct ospf_spf_vertex),"SPF Node");
    OSPF_HISTROY_MSTAT(OSPF_MIF, sizeof (struct ospf_if),"Interface");
    OSPF_HISTROY_MSTAT(OSPF_MIPROUTE,sizeof (struct ospf_iproute),"External Route");
    OSPF_HISTROY_MSTAT(OSPF_MRXMT,sizeof (struct ospf_retransmit),"Retransmit Node");
    OSPF_HISTROY_MSTAT(OSPF_MDBD, OSPF_DEFAULT_IP_MTU+100,"DD summary packet");
    OSPF_HISTROY_MSTAT(OSPF_MLSA, sizeof (struct ospf_lsa),"LSA");
    OSPF_HISTROY_MSTAT(OSPF_MROUTE, sizeof (struct ospf_route),"Route Entry");
    OSPF_HISTROY_MSTAT(OSPF_MNEXTHOP, sizeof (struct ospf_nexthop),"Nexthop");
    OSPF_HISTROY_MSTAT(OSPF_MPOLICY, sizeof (struct ospf_policy),"Route Policy");
    OSPF_HISTROY_MSTAT(OSPF_MNBR, sizeof (struct ospf_nbr),"Neighbor");
    OSPF_HISTROY_MSTAT(OSPF_MRANGE, sizeof (struct ospf_range),"Address Range");
    OSPF_HISTROY_MSTAT(OSPF_MREQUEST, sizeof (struct ospf_request_node),"Request Node");
    OSPF_HISTROY_MSTAT(OSPF_MNETWORK, sizeof(struct ospf_network),"Network Config");
    OSPF_HISTROY_MSTAT(OSPF_MREDISTRIBUTE, sizeof(struct ospf_redistribute), "Redistribue Config");
    OSPF_HISTROY_MSTAT(OSPF_MLSTABLE, sizeof (struct ospf_lstable),"Lsa Table");
    OSPF_HISTROY_MSTAT(OSPF_MSTAT, sizeof (struct ospf_if_stat),"Stat");
 #ifdef OSPF_FRR
    OSPF_HISTROY_MSTAT(OSPF_MBACKSPF, sizeof (struct ospf_spf_vertex),"Bckup Spf");
    OSPF_HISTROY_MSTAT(OSPF_MBACKROUTE, sizeof (struct ospf_route),"Backup Route");
 #endif
    OSPF_HISTROY_MSTAT(OSPF_MTETUNNEL, sizeof (struct ospf_te_tunnel),"TE Tunnel");
 
#ifdef OSPF_DCN
    OSPF_HISTROY_MSTAT(OSPF_MDCN, sizeof (struct ospf_dcn_rxmt_node),"Dcn rxmt");
#endif
    ospf_verify_memory();
    return;
}

