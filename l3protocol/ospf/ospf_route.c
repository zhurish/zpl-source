/* ospf_route.c contain api for routing table operation*/

#include "ospf.h"

void ospf_transit_area_examine (struct ospf_process *p_process);
void ospf_transit_area_route_calculate(struct ospf_lsa *p_lsa);
void ospf_abr_route_change_check(struct ospf_process *p_process);
void ospf_asbr_route_change_check(struct ospf_process *p_process);
struct ospf_area *ospf_inter_calculate_area_get(struct ospf_process * p_process);
extern int log_time_print (u_int8 *buf);
u_int g_ospf_loopback_control = 1;
void 
ospf_spf_log_add(
            struct ospf_process *p_process, 
            struct ospf_spf_loginfo *p_log)
{
    struct ospf_spf_loginfo *p_new = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next = NULL;
    
    if (OSPF_STAT_MAX_NUM <= ospf_lstcnt(&ospf.log_table.spf_table))
    {
        p_new = ospf_lstfirst(&ospf.log_table.spf_table);
        ospf_lstdel_unsort(&ospf.log_table.spf_table, p_new);
    }
    else
    {
        p_new = ospf_malloc(sizeof(struct ospf_spf_loginfo), OSPF_MSTAT);
    }
    if (NULL == p_new)
    {
        return;
    }    
    memcpy(p_new, p_log, sizeof(struct ospf_spf_loginfo));
    p_new->process_id = p_process->process_id;
    for_each_node(&p_process->area_table, p_area, p_next)
    {
        p_new->abr_num += ospf_lstcnt(&p_area->abr_table);
        p_new->asbr_num += ospf_lstcnt(&p_area->asbr_table);
    }
    p_new->route_num = ospf_lstcnt(&p_process->route_table);
    p_new->spf_called = p_process->spf_called_count - p_process->last_spf_called_count;
    p_process->last_spf_called_count = p_process->spf_called_count;
    ospf_lstadd_unsort(&ospf.log_table.spf_table, p_new);
    return;   
}

 /*route compare*/  
int 
ospf_route_lookup_cmp(
          struct ospf_route *p1, 
          struct ospf_route *p2)
{
    /*type*/
    OSPF_KEY_CMP( p1, p2, type);
    /*dest*/
    OSPF_KEY_CMP(p1, p2, dest);
    /*mask:only for network route*/
    if (OSPF_ROUTE_NETWORK == p1->type)
    {
        OSPF_KEY_CMP(p1, p2, mask);
    }
    return 0;
}

void 
ospf_route_table_init(struct ospf_lst *p_list)
{
    ospf_lstinit(p_list, ospf_route_lookup_cmp);
    return;
}
/*等价路由的负载均衡*/
void ospf_clear_addr(struct ospf_nexthop_info *gateway)
{
    int i = 0;
    for(i = 0 ; i < OSPF_ECMP_COUNT; i++)
    {
        if(gateway[i].addr == 0)
        {
            break;
        }
        gateway[i].addr = 0; 
    }
}
/*count > ecmp时，只会将ecmp个数的nexthop下底层,多余的路由会被过滤*/
int ospf_max_ecmp_check(struct ospf_nexthop *pstNexthop , int iMaxecmp)
{
    int i = 0;
    int rc = 0;

    if(iMaxecmp < pstNexthop->count)
    {
        for(i = iMaxecmp ; i < pstNexthop->count; i++)
        {
            pstNexthop->gateway[i].addr = 0;
        }
        pstNexthop->count = iMaxecmp;
        rc = 1;
    }
    return rc;
}

/*等价路由下一跳权重查找*/
int  ospf_ecmp_nhop_check(struct ospf_nexthop *pstNexthop, struct ospf_nhop_weight *pstNhopw)
{
    int i = 0, j = 0;
    int num = 0;
    int rc = FALSE;
    u_long minweight = 0;
    int flag = FALSE;
    struct ospf_nhop_weight sttmp[OSPF_ECMP_COUNT] = {0};
    struct ospf_nhop_weight *pstNhopwTmp = pstNhopw;
    struct ospf_nhop_weight stTemp = {0};
    struct ospf_nexthop_info stTempNextHop[OSPF_ECMP_COUNT] = {0};

    
    if(pstNexthop == NULL)
    {
        return;
    }

    memcpy(stTempNextHop, pstNexthop->gateway, sizeof(pstNexthop->gateway));
    memset(pstNexthop->gateway, 0, sizeof(pstNexthop->gateway));
    /*等价路由中是否配置了nexthop weight */
    for(i = 0; i < pstNexthop->count; i++)
    {
        while(pstNhopwTmp->flag)
        {
            if(pstNhopwTmp->addr == stTempNextHop[i].addr)
            {
                flag = TRUE;
                break;
            }
            pstNhopwTmp++;
        }
        if(flag)
        {
            flag = FALSE;
            sttmp[num].addr = pstNhopwTmp->addr;
            sttmp[num].weight = pstNhopwTmp->weight;
            sttmp[num].flag = pstNhopwTmp->flag;
            num++;
        }
        else
        {
            memcpy(&pstNexthop->gateway[pstNexthop->count-1-i+num], &stTempNextHop[i], sizeof(struct ospf_nexthop_info));
        }
        
        pstNhopwTmp = pstNhopw;
    }

    for(i = 0; i < num; i++)
    {
        for(j = i+1; j < num; j++)
        {
            if(sttmp[i].weight > sttmp[j].weight)
            {
                memcpy(&stTemp, &sttmp[i], sizeof(struct ospf_nhop_weight));
                memcpy(&sttmp[i], &sttmp[j], sizeof(struct ospf_nhop_weight));
                memcpy(&sttmp[j], &stTemp, sizeof(struct ospf_nhop_weight));
            }
        }
    }


    for(i = 0; i < num; i++)
    {
        for(j = 0; j < pstNexthop->count; j++)
        {
            if(sttmp[i].addr == stTempNextHop[j].addr)
            {
                break;
            }
        }
        if(j < pstNexthop->count)
        {
            memcpy(&pstNexthop->gateway[i], &stTempNextHop[j], sizeof(struct ospf_nexthop_info));
        }

    }
    #if 0
    for(i = 0; i < pstNexthop->count; i++)
    {
        printf("next hop = %x\n", pstNexthop->gateway[i].addr);
    }
    #endif
    rc = TRUE; 
    
    return rc;
}


/*common function to update-create a route
   input route containing all information for create and update
 */
struct ospf_route *
ospf_route_add(
         struct ospf_process *p_process,
         struct ospf_lst *p_list, 
         struct ospf_route *p_route)
{
    struct ospf_route *p_current = NULL;
    struct ospf_route *p_new = NULL;
    struct ospf_nexthop nexthop;
    u_int current = p_process->current_route;
    int rc; 
	
   // printf("%s %d	  ****p_route:0x%x*******\n", __FUNCTION__,__LINE__,p_route);
  //  ospf_ospf_route_print(p_route);
//    printf("%s %d	  ****current: %d,p_nexthop:0x%x*******\n", __FUNCTION__,__LINE__,current,p_route->path[current].p_nexthop);
    if (NULL == p_route->path[current].p_nexthop)
    {
        ospf_logx(ospf_debug_route, "route have no nexthop,stop");
        return NULL;
    }
    
    if ((((p_route->dest & 0xff000000) == 0x7f000000)
        && g_ospf_loopback_control)
     || (p_route->dest >= 0xe0000000))
    {
        ospf_logx(ospf_debug_route|ospf_debug_error, "route have invalid dest addr:lookback addr or muticast sddr,stop");
        return NULL;
    }
 
    /*lookup current route node*/
    p_current = ospf_lstlookup(p_list, p_route);
  //  printf("%s %d	 ****p_current:0x%x*******\n", __FUNCTION__,__LINE__,p_current);
 //   ospf_ospf_route_print(p_current);
    /*no route exist,create one*/
    if (NULL == p_current)
    {
        ospf_logx(ospf_debug_route, "create new route");
        p_new = ospf_malloc2(OSPF_MROUTE);
//	printf("%s %d	 ****p_new:0x%x*******\n", __FUNCTION__,__LINE__,p_new);
//	ospf_ospf_route_print(p_new);
        if (NULL == p_new)
        {           
//	   printf("%s %d  ****(NULL == p_new)*******\n", __FUNCTION__,__LINE__);
            return NULL; 
        }
        /*copy from input*/
        p_new->dest = p_route->dest;
        p_new->mask = p_route->mask;
        p_new->type = p_route->type;
      /*  p_new->local_route = p_route->local_route;  20131018*/
 
        ospf_lstadd(p_list, p_new); 
        p_current = p_new;
//	printf("%s %d	 ****p_current:0x%x*******\n", __FUNCTION__,__LINE__,p_current);
    }
  //  printf("%s %d	 ****p_current:0x%x*******\n", __FUNCTION__,__LINE__,p_current);
  //  ospf_ospf_route_print(p_current);

//	printf("%s %d	 ****type:0x%x*******\n", __FUNCTION__,__LINE__,p_current->path[current].type);
    /*route exist, but path invalid,add path*/
    if (OSPF_PATH_INVALID == p_current->path[current].type)
    {
        /*add path*/
        memcpy(&p_current->path[current], &p_route->path[current], sizeof(struct ospf_path));
//	printf("%s %d	 ****p_area:0x%x*******\n", __FUNCTION__,__LINE__,p_current->path[current].p_area);
        if (NULL != p_current->path[current].p_area)
        {
	//	printf("%s %d	 ****type:0x%x*******\n", __FUNCTION__,__LINE__,p_current->type);
            if (OSPF_ROUTE_ABR == p_current->type)
            {
                p_current->path[current].p_area->abr_count++;
            }
            else if (OSPF_ROUTE_ASBR == p_current->type)
            {
                p_current->path[current].p_area->asbr_count++;
            }
        }
//	printf("%s %d	 ****type:0x%x*******\n", __FUNCTION__,__LINE__,p_current->type);
        /*decide max &min mask of routing table,for fast lookup*/
        if (OSPF_ROUTE_NETWORK == p_current->type)
        {
//		printf("%s %d	 ****mask:0x%x,max_mask 0x%x,min_mask 0x%x*******\n", __FUNCTION__,__LINE__,p_current->mask,p_process->max_mask,p_process->min_mask);
            if (p_current->mask > p_process->max_mask)
            {
                p_process->max_mask = p_current->mask;
            }                
            if (p_current->mask < p_process->min_mask)
            {
                p_process->min_mask = p_current->mask;
            }
        }
        p_current->path_checked = FALSE;
        p_current->summary_checked = FALSE;
//		printf("%s %d  ****(OSPF_PATH_INVALID == p_current->path[current].type)*******\n", __FUNCTION__,__LINE__);
        return p_current;
    }
 
    /*compare path*/
    /*current route exist,comapre cost and path*/
    rc = ospf_path_cmp(&p_current->path[current], &p_route->path[current]);
  //  printf("%s %d	 ****rc %d*******\n", __FUNCTION__,__LINE__,rc);
    /*current is better,ignore*/
    if (0 < rc)
    {
        ospf_logx(ospf_debug_route, "current route is better,ignore");
        return p_current;
    }
    else if (0 > rc)
    {
        ospf_logx(ospf_debug_route, "new route is better, replace it");
 
        /*new cost is better,replace it*/
        memcpy(&p_current->path[current], &p_route->path[current], sizeof(struct ospf_path));
    }
    else
    {  
        ospf_logx(ospf_debug_route, "same cost,merge nexthop");
        /*same cost,add nexthop*/
        memset(&nexthop, 0, sizeof(nexthop));
        ospf_nexthop_merge(&nexthop, p_current->path[current].p_nexthop);
        ospf_nexthop_merge(&nexthop, p_route->path[current].p_nexthop);
 
        /*add new nexthop*/
        p_current->path[current].p_nexthop = ospf_nexthop_add(p_process, &nexthop);
    }   
//	printf("%s %d  ****p_current 0x%x*******\n", __FUNCTION__,__LINE__,p_current);
    return p_current;
}

/*free a route node*/
void 
ospf_route_delete(struct ospf_route *p_route)
{
    /*do not care about nexthop*/ 
    ospf_mfree(p_route, OSPF_MROUTE);
    return;
}

/*compare path cost of two ospf routes*/
int 
ospf_path_cmp(
         struct ospf_path *p1,
         struct ospf_path *p2)
{
    if ((NULL == p1)||(NULL == p2))
    {
        return 1;
    }
    
    /*path type,less value is prefered*/
    if (p1->type != p2->type)
    {
        OSPF_KEY_CMP(p2, p1, type);
    }
    /*20131018 local route is perfered*/
    if (p1->local_route != p2->local_route)
    {
        return p1->local_route ? 1: -1;
    }
    /*same pathtype,compare cost,less cost is prefered*/
    if (OSPF_PATH_ASE2 == p1->type)
    {
        OSPF_KEY_CMP(p2, p1, cost2);
        OSPF_KEY_CMP(p2, p1, cost);
    }
    else
    {
        u_int cost1 = p1->cost + p1->cost2;
        u_int cost2 = p2->cost + p2->cost2;
        if (cost1 != cost2)
        {
            return (cost1 < cost2) ? 1 : -1;
        }
    }   
    return 0;
}

/*LOOKUP functions*/

/***************************************************************
  lookup for a route entry in a special routing table.used for route update
  after calculating
*/
struct  ospf_route *
ospf_route_lookup(
            struct ospf_lst *p_table,
            u_int type,
            u_int dest,
            u_int mask)
{
    struct ospf_route search;
    search.dest = dest;
    search.mask = mask;
    search.type = type;
    return ospf_lstlookup(p_table, &search);
}

/*
 select the route with the least cost; when there are multiple least cost route
 the route whose associated area has	the largest OSPF Area ID is chosen.
*/
int
ospf_asbr_priority_cmp(
          struct ospf_process *p_process,
          struct ospf_route *r1,
          struct ospf_route *r2)
{
    struct ospf_path *p1 = &r1->path[p_process->current_route];
    struct ospf_path *p2 = &r2->path[p_process->current_route];
    u_int transit_intra1 = ((OSPF_BACKBONE != p1->p_area->id) && (OSPF_PATH_INTRA == p1->type)) ? 1 : 0;
    u_int transit_intra2 = ((OSPF_BACKBONE != p2->p_area->id) && (OSPF_PATH_INTRA == p2->type)) ? 1 : 0;   
    
    /*rfc1583_compatibility disable:
      Intra-area paths using non-backbone areas are always themost preferred.
      The other paths, intra-area backbone paths and inter area paths,
      are of equal preference
     */
    if (p_process->rfc1583_compatibility == FALSE)
    {
        if (transit_intra1 != transit_intra2)
        {
            return (transit_intra1 > transit_intra2) ? 1 : -1;
        }
    }
    
    /*less cost is prefered*/
    if (p1->cost != p2->cost)
    {
        OSPF_KEY_CMP(p2, p1, cost);
    }
 
    /*cost same,large area preferd*/
    OSPF_KEY_CMP(p1, p2, p_area->id);
    return 0;
}

/*lookup asbr route*/
struct ospf_route *
ospf_asbr_lookup(
           struct ospf_process *p_process,
           struct ospf_area *p_area,
           u_int asbr)
{
    struct ospf_area *p_next = NULL;
    struct ospf_route *p_route = NULL;
    struct ospf_route *p_best = NULL;
    struct ospf_route *p_routenext = NULL;
    u_int current = p_process->current_route;
    
    /*area not null,search in this area*/   
    if (NULL != p_area)
    {
        return ospf_route_lookup(&p_area->asbr_table, OSPF_ROUTE_ASBR, asbr, 0);
    }
   
    /*no area,search route in any area,and select best one*/
    for_each_ospf_area(p_process, p_area, p_next)
    {
        p_route = ospf_route_lookup(&p_area->asbr_table, OSPF_ROUTE_ASBR, asbr, 0);
        if ((NULL == p_route)
            || (NULL == p_route->path[current].p_nexthop))
        {
            continue;
        }

        if (NULL == p_best)
        {
            p_best = p_route;
            continue;
        }
        
        if (0 < ospf_asbr_priority_cmp(p_process, p_route, p_best))
        {
            p_best = p_route;
        }
    }
    #if 1
    /*undo rfc1583 compatibility 去使能，在多个asbr中找出best*/
    if (p_process->rfc1583_compatibility == FALSE)
    {
        for_each_ospf_area(p_process, p_area, p_next)
        {
            p_route = ospf_lstfirst(&p_area->asbr_table);
            while(p_route)
            {
                if(p_route->type == OSPF_ROUTE_ASBR)
                {
                    if((p_best && p_best->path[p_process->current_route].p_area) && p_route->path[p_process->current_route].p_area)
                    {
                        if (0 < ospf_asbr_priority_cmp(p_process, p_route, p_best))
                        {
                            p_best = p_route;
                        }
                    }
                }
                p_routenext = ospf_lstnext(&p_area->asbr_table, p_route);
                p_route = p_routenext;
            }
       }
   }
   #endif
    return p_best;
}

/*nexthop operation*/
int 
ospf_nexthop_cmp (
          struct ospf_nexthop *p1,
          struct ospf_nexthop *p2) 
{
    int rc = 0;

    if ((NULL == p1) || (NULL == p2))
    {
        return p1 ? 1 : -1;
    }

    /*normal case,pointer same*/
    if (p1 == p2)
    {
        return 0;
    }
    
    /*count checking*/
    OSPF_KEY_CMP(p1, p2, count);

    /*compare all nexthops*/
    if (p1->count)
    {
        rc = memcmp(p1->gateway, p2->gateway, sizeof(struct ospf_nexthop_info)*p1->count);
        if (0 != rc)
        {
            return (rc > 0) ? 1 : -1;
        }
    }
    return 0;
}

void 
ospf_nexthop_table_init(struct ospf_process *p_process)
{
    ospf_lstinit(&p_process->nexthop_table, ospf_nexthop_cmp);
    return;
}

/*if a nexthop table is empty,all nexthops in table are empty*/
u_int 
ospf_nexthop_empty(struct ospf_nexthop *p_nexthop)
{
    u_int i = 0;
    
    if ((NULL == p_nexthop) || (0 == p_nexthop->count))
    {
        return TRUE;
    }
    
    for (i = 0 ; i < p_nexthop->count; i++)
    {
        if (0 != p_nexthop->gateway[i].addr)
        {
            return FALSE;
        }
    }
    return TRUE;
}

/*merge nexthop from src to dest*/
void 
ospf_nexthop_merge(
            struct ospf_nexthop *p_dest, 
            struct ospf_nexthop *p_src)
{
    u_int d = 0;
    u_int s = 0;

    if ((NULL == p_dest) || (NULL == p_src))
    {
        return;
    }
    
    for (s = 0 ; s < p_src->count; s++)
    {
        if (0 == p_src->gateway[s].addr)
        {
            continue;
        }
        if (TRUE == ospf_nexthop_exist(p_dest, p_src->gateway[s].addr))
        {
            continue;
        }
        for (d = 0 ; d < OSPF_ECMP_COUNT ; d++)
        {        
            if (0 == p_dest->gateway[d].addr)
            {
                p_dest->gateway[d].addr = p_src->gateway[s].addr ;
                p_dest->gateway[d].if_uint = p_src->gateway[s].if_uint ;
                p_dest->gateway[d].type = p_src->gateway[s].type ;
                p_dest->count++;
                break;
            }
        }
    }
    return;
}

/*decide if a nexthop exist*/
u_int 
ospf_nexthop_exist(
                    struct ospf_nexthop *p_nhop,
                    u_int gateway)
{
    u_int i = 0;
    
    if (0 == gateway)
    {
        return FALSE;
    }
    for (i = 0 ; i < p_nhop->count; i++)
    {
        if (p_nhop->gateway[i].addr == gateway)
        {
             return TRUE;
        }
    }
    return FALSE;
}

/*decide if a nexthop exist*/
u_int 
ospf_nexthop_ifuint_exist(
                    struct ospf_nexthop *p_nhop,
                    u_int uiIndx)
{
    u_int i = 0;
    
    if (0 == uiIndx)
    {
        return FALSE;
    }
    for (i = 0 ; i < p_nhop->count; i++)
    {
        if (p_nhop->gateway[i].if_uint == uiIndx)
        {
             return TRUE;
        }
    }
    return FALSE;
}

/*search ospf nexthop in table*/
struct ospf_nexthop *
ospf_nexthop_lookup(
                 struct ospf_process *p_process,
                 struct ospf_nexthop *p_search)
{
    return ospf_lstlookup(&p_process->nexthop_table, p_search);
}

/*search and add nexthop in global nexthop table,do not modify reference here*/
struct ospf_nexthop *
ospf_nexthop_add(
               struct ospf_process *p_process,
               struct ospf_nexthop *p_nexthop)
{
    struct ospf_nexthop *p_new = NULL;
    u_int i = 0,uiMlen = 0;

    p_new = ospf_nexthop_lookup(p_process, p_nexthop);
    if ((NULL != p_new)&&(p_new->count != 0))
    {
//	printf("%s %d  *p_new:%x,count:%d,addr:0x%x,if_uint:0x%x,type:%d*\n", __FUNCTION__,__LINE__, 
//		p_new,p_new->count,p_new->gateway[p_new->count].addr,p_new->gateway[p_new->count].if_uint,p_new->gateway[p_new->count].type);
        return p_new;
    }
    else
    {
        /*create new one,length is variable*/ 
        uiMlen = (sizeof(struct ospf_lstnode) + 8 +(p_nexthop->count)*sizeof(struct ospf_nexthop_info));
        u_int ulLen = 0;

        ulLen = (uiMlen/sizeof(struct ospf_nexthop))*sizeof(struct ospf_nexthop);
      //  printf("ospf_nexthop_add ulLen=%d,uiMlen=%d,lst = %d\n",ulLen,uiMlen,sizeof(struct ospf_nexthop));

        p_new = ospf_malloc(uiMlen, OSPF_MNEXTHOP);
       // p_new = ospf_malloc(ulLen, OSPF_MNEXTHOP);
    }
    if (NULL != p_new)
    {       
        /*copy nexthop one by one,do not use struct copy*/
        for (i = 0 ; i < p_nexthop->count; i++)
        {
            if (p_nexthop->gateway[i].addr)
            {
                p_new->gateway[p_new->count].addr = p_nexthop->gateway[i].addr;
                p_new->gateway[p_new->count].if_uint = p_nexthop->gateway[i].if_uint;
                p_new->gateway[p_new->count].type = p_nexthop->gateway[i].type ;             
                p_new->count++;
//		printf("%s %d  p_new:%x,count:%d,addr:0x%x,if_uint:0x%x,type:%d*\n", __FUNCTION__,__LINE__, 
//			p_new,p_new->count,p_new->gateway[p_new->count].addr,p_new->gateway[p_new->count].if_uint,p_new->gateway[p_new->count].type);
            }
        }
        /*add to list*/
        ospf_lstadd(&p_process->nexthop_table, p_new);
    }
    return p_new;
}

/********************************************************
   decide if a special nexthop in list belong to the area.   
   used for deciding if summary lsa can be originated when 
   vlink exist
*/
u_int 
ospf_nexhop_in_the_area (
                 struct ospf_nexthop *p_nexthop,
                 struct ospf_area *p_area)
{
    struct ospf_if *p_if = NULL;
    u_int i = 0;

    if (NULL == p_nexthop)
    {
        return FALSE;
    }
    
    /*if one nexthop in list has outgoing interface in this area,this is true*/
    for (i = 0 ; i < p_nexthop->count; i++)
    {
        p_if = ospf_if_lookup_by_network(p_area->p_process, p_nexthop->gateway[i].addr) ;
        if ((NULL != p_if) && (p_if->p_area == p_area))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*route calculation*/
  
/**full spf calculation request**/
void 
ospf_spf_request(struct ospf_process *p_process)
{
    u_int ticks = ospf_sys_ticks(); 
    
    ospf_logx(ospf_debug_route, "prepare spf calculate");    

    /*stop frr timer if scheduled*/
  #ifdef OSPF_FRR
    if (ospf_timer_active(&p_process->frr_timer))  
    {
        ospf_timer_stop(&p_process->frr_timer);
    }
  #endif
    
    if (p_process->in_restart)
    {
        ospf_logx(ospf_debug_route, "schedule route build exit for in grace start state");    
        return;
    }
        
    /*update total spf called counter*/
    p_process->spf_called_count ++;

    /*first calling,start timer....*/        
    if (!ospf_timer_active(&p_process->spf_timer)) 
    {
        /*it is mstimer*/
        ospf_timer_start(&p_process->spf_timer, p_process->spf_interval);
        p_process->spf_start_time = ticks;
        p_process->spf_delay_interval = 1;/*100ms*/
        return;
    }

    /* use longer timer when nbr exceed limit*/
    if (OSPF_MAX_EXCHANGE_NBR <= ospf_lstcnt(&p_process->nbr_table))
    {
        if ((OSPF_MAX_SPF_INTERVAL + (3 * ospf_lstcnt(&p_process->nbr_table)))
            > ospf_time_differ(ticks, p_process->spf_start_time))
        {
            ospf_timer_start(&p_process->spf_timer, 
                  (p_process->spf_interval + ospf_lstcnt(&p_process->nbr_table)));
        }
        return;
    }
    
    /*if time interval between current and first called 
    time is less than max interval,reset timer*/
    if (OSPF_MAX_SPF_INTERVAL > ospf_time_differ(ticks, p_process->spf_start_time))
    {
        p_process->spf_delay_interval = p_process->spf_delay_interval << 1;
        /*delay most 3 second*/
        if (30 < p_process->spf_delay_interval)
        {
            p_process->spf_delay_interval = 30;
        }
        ospf_timer_start(&p_process->spf_timer, (p_process->spf_interval + p_process->spf_delay_interval));
    }
    return;
}
#define OSPF_LARGE_ROUTE_NUM 2000000 /*200w*/
/* section 16 of OSPF specification (page 148-149) */
/*full calculation of route*/
void 
ospf_route_calculate_full (struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;    
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    struct ospf_spf_loginfo stat;
    uint32_t type = 0;
    uint32_t now = 0 ;
    uint32_t last_hello = 0 ;
    uint32_t large_route = FALSE;
    uint32_t hello_interval = 65535;


    ospf_set_context(p_process);
    ospf_logx(ospf_debug_route, "start building routing table");
    
    if (p_process->in_restart)
    {
        ospf_logx(ospf_debug_route, "node is in restarting ,do not calculate routing table");
        return;
    }


    if (p_process->wait_export) 
    {
        ospf_logx(ospf_debug_route, "some routes is in wating,do not calculate new routes");
        
        ospf_spf_request(p_process);
        return;
    }
    memset(&stat, 0, sizeof(stat));
    /*increase spf running times*/
    p_process->spf_running_count++;

    /*store old routing table*/
    ospf_save_old_routes(p_process);

    now = ospf_sys_ticks();
    log_time_print(stat.spf_start_time);
    stat.calculate_period = now;
    stat.spf_period = now;
    /*spf calculate for all araes*/
    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        ospf_spf_calculate(p_area);
    }

    /*cacluate spf running timer*/
    now = ospf_sys_ticks();
    stat.spf_period = ospf_time_differ(now, stat.spf_period);
    stat.ia_period = now;

    /* section 16, item (2)  */
    p_area = ospf_inter_calculate_area_get(p_process);
    if (NULL != p_area)
    {
        ospf_logx(ospf_debug_route, "calculate inter routes for area %u", p_area->id);
    
        /*type 3 and type 4 lsa*/
        for (type = OSPF_LS_SUMMARY_NETWORK; type <= OSPF_LS_SUMMARY_ASBR; type++)
        { 
            for_each_area_lsa(p_area, type, p_lsa, p_next)
            {            
                ospf_inter_route_calculate(p_lsa);
            }  
        }    
    }

//	printf("%s %d  *****p_process->abr = %x******\n", __FUNCTION__,__LINE__,p_process->abr);
    /* section 16, item (4)  */
    if (p_process->abr)
    {
        ospf_transit_area_examine(p_process);
    }

    /*cacluate inter-area-route running timer*/
    now = ospf_sys_ticks();
    stat.ia_period = ospf_time_differ(now, stat.ia_period);
    stat.ase_period = now;
    last_hello = now;

    if (OSPF_LARGE_ROUTE_NUM < ospf_lstcnt(&p_process->t5_lstable.list))
    {
        large_route = TRUE;
        for_each_ospf_if(p_process, p_if, p_next_if)
        {
            if (p_if->hello_interval < hello_interval)
            {
                hello_interval = p_if->hello_interval;
            }
        }
    }
    /* section 16, item (5) */
    if (ospf_as_external_route_calculate_enable(p_process))
    {
        for_each_external_lsa(p_process, p_lsa, p_next)
        {
            ospf_external_route_calculate(p_lsa);

            if (large_route)
            {
                now = ospf_sys_ticks();
                if (hello_interval * OSPF_TICK_PER_SECOND < ospf_time_differ(now, last_hello))
                {
                    last_hello = now;
                    ospf_lstwalkup(&p_process->if_table, ospf_if_hello_timeout);
                 
                }
            }
        }
    }

    if (large_route) 
    {
         for_each_ospf_if(p_process, p_if, p_next_if)
         {
             for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
             {
                 ospf_stimer_start(&p_nbr->hold_timer, p_if->dead_interval);
             }
         }
    }
    /*  Calculate type-7 As external routes*/
    for_each_ospf_area(p_process, p_area, p_next_area)
    {
    
//	printf("%s %d  ***p_area->is_nssa %d********\n", __FUNCTION__,__LINE__,p_area->is_nssa);
        if (!p_area->is_nssa)
        {
//		printf("%s %d  ****(!p_area->is_nssa)*******\n", __FUNCTION__,__LINE__);
            continue;
        }
        
        /*elect translator */
        ospf_nssa_translator_elect(p_area);
        
        for_each_nssa_lsa(p_area, p_lsa, p_next)
        {
            ospf_external_route_calculate(p_lsa);
        }
    }

    /*cacluate external-route running timer*/
    now = ospf_sys_ticks();
    stat.ase_period = ospf_time_differ(now, stat.ase_period);

    /* section 16.7 (page 163-164) */
//	printf("%s %d  ***********\n", __FUNCTION__,__LINE__);

    /*check abr route's change*/
    ospf_abr_route_change_check(p_process);

    /*check asbr route's change,and update summary lsa*/
    ospf_asbr_route_change_check(p_process);    
        
    /*set wait export flag*/
    p_process->wait_export = TRUE;
    p_process->last_export_network = OSPF_HOST_NETWORK;
//	printf("%s %d  ***********\n", __FUNCTION__,__LINE__);

    /*check network route change,and update summary lsa and system route*/
    ospf_network_route_change_check(p_process);

    /*if network's number is large,set this timer to check change in next loop*/
    ospf_timer_start(&p_process->ipupdate_timer, 1);

    /*stats:cacluate route update timer*/
    now = ospf_sys_ticks();    
    
    /*update max spf running time*/
    stat.calculate_period = ospf_time_differ(now, stat.calculate_period);

    /*record this calculation*/
    ospf_spf_log_add(p_process, &stat);
    
    if (stat.calculate_period > p_process->max_spf_time)
    {
        p_process->max_spf_time = stat.calculate_period;
    }
    /*check vif state*/
    ospf_lstwalkup(&p_process->virtual_if_table, ospf_vif_state_update);

    /*prepare check range state*/
    ospf_timer_try_start(&p_process->range_update_timer, 5);
  //   printf("%s %d  ***********\n", __FUNCTION__,__LINE__);
     return;
}
/*decide if the there is intra asbr route in any area*/
u_int 
ospf_intra_asbr_route_exist(
               struct ospf_process *p_process,
               uint32_t dest)
{
    struct ospf_route search;
    struct ospf_route *p_route = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next = NULL;    
    uint32_t current = p_process->current_route;
    
    memset(&search, 0, sizeof(search));
    search.type = OSPF_ROUTE_ASBR;
    search.dest = dest;

    for_each_ospf_area(p_process, p_area, p_next)
    {
        p_route = ospf_lstlookup(&p_area->asbr_table, &search);
        if (p_route
            && (OSPF_PATH_INTRA == p_route->path[current].type))
        {
            return TRUE;
        }
    }
    return FALSE;
}
/*select area for inter-calculate:if abr,select backbone, else select the first active area*/
struct ospf_area *
ospf_inter_calculate_area_get(struct ospf_process *p_process)
{
    return ospf_lstfirst(&p_process->area_table);
}

/*calculate inter area routes for a single summary LSA*/
/* section 16.2 of OSPF specification (page 156) */
void 
ospf_inter_route_calculate (struct ospf_lsa *p_lsa)
{
    struct ospf_range *p_range = NULL;
    struct ospf_route *p_abr_route = NULL;
    struct ospf_summary_lsa *p_summary = ospf_lsa_body(p_lsa->lshdr);
    struct ospf_area *p_area = p_lsa->p_lstable->p_area;
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_route route;
    u_int current = p_process->current_route;
    u_int type = 0;
    u_int cost =0;
    u_int abr = 0;
    u_int dest = 0;
    u_int mask = 0;
    u_int8 abr_str[20]; 
    u_int8 route_str[20]; 
        
    if (ospf_debug_spf)   
    {
    	ospf_inet_ntoa(abr_str, ntohl(p_summary->h.adv_id));
    	ospf_inet_ntoa(route_str, ntohl(p_summary->h.id));
        ospf_logx(ospf_debug_route, "calculate summary route,t=%d,id=%s,adv=%s",
        p_summary->h.type, route_str, abr_str);
    }
    if (OSPF_LS_SUMMARY_NETWORK == p_summary->h.type)
    {
        type = OSPF_ROUTE_NETWORK;
        mask = ntohl(p_summary->mask);
    }
    else
    {
        type = OSPF_ROUTE_ASBR;
        mask = OSPF_HOST_MASK;
    }
    cost = ntohl(p_summary->metric) & OSPF_METRIC_INFINITY;
    abr = ntohl(p_summary->h.adv_id) ;
    dest = ntohl(p_summary->h.id) & mask;

    /*ignored lsa:invalid cost,maxage,and self originated*/
    /* section 16.2, items (1 & 2) (page 157) */
    if ((ospf_invalid_metric(cost)) 
        || (OSPF_MAX_LSAGE <= ntohs(p_summary->h.age)) 
        || (abr == p_process->router_id))     
    {
        ospf_logx(ospf_debug_route, "do not calculate for invalid summary lsa");
        return;
    }

    /*asbr destination can not be self*/
    if ((OSPF_ROUTE_ASBR == type) && (dest == p_process->router_id))
    {
        ospf_logx(ospf_debug_route, "do not calculate asbr route for self");            
        return;
    }
    /*PE don't caculate for type3 lsa with dn bit set*/
    if ((ospf_is_vpn_process(p_process)) 
        && (p_process->vpn_loop_prevent)
        && (OSPF_LS_SUMMARY_NETWORK == p_summary->h.type)
        && (ospf_option_dn(p_summary->h.option)))
    {
        ospf_logx(ospf_debug_route, "do not calculate route for  type3 lsa with dn bit set");            
        return;
    }
	/*if this is asbr summary lsa,and there is same intra asbr route in other
     area,do not calculate it.20150130*/
    if (OSPF_ROUTE_ASBR == type)
    {
        if (TRUE == ospf_intra_asbr_route_exist(p_process, dest))
        {
            ospf_logx(ospf_debug_route, "do not calculate inter asbr route,some intra route exist");
            return;
        }
    }
    /*do not calculate route for aggregated route,it is a discarded route*/
    /* section 16.2, item (3) (page 157) */
    if (OSPF_ROUTE_NETWORK == type)
    {
        p_range = ospf_range_lookup(p_area, p_summary->h.type, dest, mask) ;
        if ((NULL != p_range) && (0 < p_range->rtcount))
        { 
             ospf_logx(ospf_debug_route, "ignore route because address range limit"); 
             return; 
        }
    }

    /*lookup abr route*/    
    /* section 16.2, item (4) (page 157) */
    p_abr_route = ospf_abr_lookup(p_area, abr); 
    if ((NULL == p_abr_route)
        || (NULL == p_abr_route->path[current].p_nexthop))
    {
        ospf_logx(ospf_debug_route, "no route to abr,stop route calculation");         
        return; 
    }
    
    memset(&route, 0, sizeof(route));
    route.type = type;
    route.dest = dest;
    route.mask = mask;
    route.path[current].type = OSPF_PATH_INTER;
    route.path[current].cost = (ntohl(p_summary->metric) & OSPF_METRIC_INFINITY) + p_abr_route->path[current].cost;
    route.path[current].p_area = p_abr_route->path[current].p_area;
    route.path[current].p_nexthop = p_abr_route->path[current].p_nexthop;
    route.path[current].adv_id = abr;
    
    if (OSPF_ROUTE_NETWORK == type)
    {
        ospf_route_add(p_process, &p_process->route_table, &route);
    }
    else
    {
        ospf_logx(ospf_debug_route, "add asbr route:%x", route.dest);

        ospf_route_add(p_process, &p_area->asbr_table, &route);
    }
    return;
}

/*clear a path*/
#define ospf_path_clear(x) do{\
    (x)->p_nexthop = NULL;\
    (x)->p_area = NULL;\
    (x)->type = OSPF_PATH_INVALID;\
}while(0)

/* section 16.3 of OSPF specification (page 158) */
void 
ospf_transit_area_examine (struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;    
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next_lsdb = NULL;
    struct ospf_route *p_route = NULL;
    struct ospf_route *p_next = NULL;
    u_int i = 0;
    u_int transit_check = 0;
    u_int current = p_process->current_route;
        
    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        /*only transit area considered*/
        if (!p_area->transit)
        {
            continue;
        }
        transit_check ++;
        ospf_logx(ospf_debug_route, "examine transit area %d", p_area->id);

        /*type 3 and type 4 lsa*/
        for (i = OSPF_LS_SUMMARY_NETWORK; i <= OSPF_LS_SUMMARY_ASBR; i++)
        {
            for_each_area_lsa(p_area, i, p_lsa, p_next_lsdb)
            {
                ospf_lsa_age_update(p_lsa);
                ospf_transit_area_route_calculate(p_lsa);
            }                                
        } 
    }
    
    /*clear any routes without real nexthop*/
    if (!transit_check)
    {
        return;
    }

    for_each_node(&p_process->route_table, p_route, p_next)
    {        
        if (TRUE == ospf_nexthop_empty(p_route->path[current].p_nexthop))
        {
            ospf_path_clear(&p_route->path[current]);
        }
    }
    
    for_each_ospf_area(p_process, p_area, p_next_area)
    {
       /*abr*/
        for_each_node(&p_area->abr_table, p_route, p_next)
        {   
	//	   printf("%s %d  *****current = %x******\n", __FUNCTION__,__LINE__,current);
            if (TRUE == ospf_nexthop_empty(p_route->path[current].p_nexthop))
            {
	//	printf("%s %d  *****p_route = %x******\n", __FUNCTION__,__LINE__,p_route);
                ospf_lstdel(&p_area->abr_table, p_route);
                ospf_route_delete(p_route);
            }     
        }
        /*asbr*/
        for_each_node(&p_area->asbr_table, p_route, p_next)
        {
            if (TRUE == ospf_nexthop_empty(p_route->path[current].p_nexthop))
            {
                ospf_lstdel(&p_area->asbr_table, p_route);
                ospf_route_delete(p_route);
            }
        }
    }
    return;
}

/* section 16.3 of OSPF specification (page 158) */
void 
ospf_transit_area_route_calculate (struct ospf_lsa *p_lsa)
{
    struct ospf_route *p_route = NULL;
    struct ospf_route *p_abr_route = NULL;
    struct ospf_summary_lsa *p_summary = ospf_lsa_body(p_lsa->lshdr);
    struct ospf_area *p_area = p_lsa->p_lstable->p_area;
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_route route;
    u_int current = p_process->current_route;    
    u_int rttype = 0;
    u_int mask = 0;
    u_int lscost = 0;
    u_int dest = 0;
    u_int abr = 0;
    u_short age = 0;

    lscost = ntohl(p_summary->metric) & OSPF_METRIC_INFINITY;
    age = ntohs(p_summary->h.age); 
    abr = ntohl(p_summary->h.adv_id);

    /* section 16.3, items (1 & 2) (page 158) */
    if ((ospf_invalid_metric(lscost))
        || (OSPF_MAX_LSAGE <= age)
        || (abr == p_process->router_id))
    {
        return;  
    }

    rttype = (OSPF_LS_SUMMARY_ASBR != p_lsa->lshdr->type) ? OSPF_ROUTE_NETWORK : OSPF_ROUTE_ASBR ;
    mask = (OSPF_LS_SUMMARY_ASBR != p_lsa->lshdr->type) ? ntohl(p_summary->mask) : OSPF_HOST_MASK;
    dest = ntohl(p_summary->h.id) & mask; 
    
    if (OSPF_ROUTE_NETWORK == rttype)
    {
        p_route = ospf_network_route_lookup(p_process, dest, mask);
    }
    else
    {
        p_route = ospf_asbr_lookup(p_process, NULL, dest);  
    }

    /*route must exist in backbone*/
    if ((NULL == p_route) || (p_process->p_backbone != p_route->path[current].p_area))
    {
        ospf_logx(ospf_debug_route, "route is not in backbone area");

        return;
    }
    /*abr route must exist*/    
    p_abr_route = ospf_abr_lookup(p_area, abr);
    if ((NULL == p_abr_route) || (NULL == p_abr_route->path[current].p_nexthop))
    {
        ospf_logx(ospf_debug_route, "no abr route for %x", abr);

        return;
    }
    
    /*install route,area and path not changed*/
    memset(&route, 0, sizeof(route));
    route.type = p_route->type;
    route.mask = mask;
    route.dest = dest;
    route.path[current].p_area = p_route->path[current].p_area;
    route.path[current].type = p_route->path[current].type;    
    route.path[current].cost = lscost + p_abr_route->path[current].cost;
    route.path[current].p_nexthop = p_abr_route->path[current].p_nexthop;
    route.path[current].adv_id = abr;

    if (OSPF_ROUTE_NETWORK == rttype)    
    {
        ospf_route_add(p_process, &p_process->route_table, &route);
    }
    else
    {
        ospf_logx(ospf_debug_route, "add asbr route:%x, areaid=%d", route.dest, p_area->id);

        ospf_route_add(p_process, &p_area->asbr_table, &route);
    }        
    return;
}

/*decide if we can calculate ase route,must have a active transit area*/ 
u_int 
ospf_as_external_route_calculate_enable(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next = NULL;  
    u_int asbr_exist = FALSE;
    /*if no asbr route exist,do not calculate external route*/
    for_each_ospf_area(p_process, p_area, p_next)
    {
        if (ospf_lstcnt(&p_area->asbr_table))
        {
            asbr_exist = TRUE;
            break;
        }
    }
    if (FALSE == asbr_exist)
    {
        return FALSE;
    }
    for_each_ospf_area(p_process, p_area, p_next)
    {
        if (!p_area->is_nssa && !p_area->is_stub)
        {
            return TRUE;
        }
    }
    return FALSE; 
}

/*get route for a forward address*/
struct  ospf_route *
ospf_fwd_route_lookup(
               struct ospf_process *p_process,
               u_int dest, 
               struct ospf_area *p_area)
{
    struct ospf_route *p_route = NULL;
    u_int network = 0;
    u_int mask = 0;
    u_int current = p_process->current_route;    

    /*not exact search,we search from max mask to min mask*/  
    for (mask = p_process->max_mask; 
         mask && (mask >= p_process->min_mask);
         mask = mask << 1)
    {
        network = dest & mask;

        p_route = ospf_route_lookup(&p_process->route_table, OSPF_ROUTE_NETWORK, network, mask);

        /*must be an intra/inter area path*/
        if ((NULL != p_route)
            && ((OSPF_PATH_INTRA == p_route->path[current].type) 
                || (OSPF_PATH_INTER == p_route->path[current].type)))
        {    
            /*if area not null, this is for type7 route,so it is must be intra area*/
            if (NULL != p_area)
            {
                if ((OSPF_PATH_INTRA != p_route->path[current].type)
                    || (p_area != p_route->path[current].p_area ))
                {
                    continue;
                }
            }
            return p_route;
        }          
    }
    return NULL;
}

/*calculate external route for ase and nssa lsa*/
void 
ospf_external_route_calculate(struct ospf_lsa *p_lsa)
{
    struct ospf_route *p_parent = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_external_lsa  *p_external = ospf_lsa_body(p_lsa->lshdr);
    struct ospf_process *p_process = p_lsa->p_lstable->p_process;
    struct ospf_area *p_area = p_lsa->p_lstable->p_area;
    struct ospf_route route;
    struct ospf_nexthop nexthop;
    u_int current = p_process->current_route;    
    u_int metric = 0;
    u_int mask = 0;
    u_int faddr = 0;
    u_int dest = 0;
    u_int asbr_id = 0;
    u_short age = 0;
    u_int i;
    u_int8 dstr[32];
    u_int8 mstr[32];
    u_int8 asbrstr[32];
    u_int8 fastr[32];
    
    /*nssa lsa must belong to special area*/
    if ((OSPF_LS_TYPE_7 == p_external->h.type) && (NULL == p_area))
    {
        return;
    }

    metric = ntohl(p_external->metric) & OSPF_METRIC_INFINITY;    
    age = ntohs(p_external->h.age);
    asbr_id = ntohl(p_external->h.adv_id);
    mask = ntohl(p_external->mask) ;
    dest = ntohl(p_external->h.id) & mask;
    faddr = ntohl(p_external->fwdaddr) ;

    ospf_logx(ospf_debug_route, "calculate external route %s/%s", 
    		ospf_inet_ntoa(dstr, dest),
			ospf_inet_ntoa(mstr, mask));
    ospf_logx(ospf_debug_route, "asbr %s,fwdaddr %s", 
    		ospf_inet_ntoa(asbrstr, asbr_id),
			ospf_inet_ntoa(fastr, faddr));
    
    /* section 3.5, items (1) RFC-1587*/
    if ((ospf_invalid_metric(metric)) 
        || (OSPF_MAX_LSAGE <= age) 
        || (asbr_id == p_process->router_id))
    {                
        return;                                     
    }
    /*PE don't caculate for type3,5,7 lsa with dn bit set*/
    if (ospf_is_vpn_process(p_process) && (p_process->vpn_loop_prevent)) 
    {
        if (ospf_option_dn(p_external->h.option))
        {
            ospf_logx(ospf_debug_route, "do not cal route for lsa with dn bit set");            
            return;
        }
    }
    if (ospf_is_vpn_process(p_process) 
        && (p_process->route_tag) 
        && (p_process->route_tag == ntohl(p_external->tag)))
    {
        ospf_logx(ospf_debug_route, "do not cal route for lsa with route tag");            
        return;
    }
    
    /* ASBR is unreachable */
    p_parent = ospf_asbr_lookup(p_process, p_area, asbr_id);
    if ((NULL == p_parent) || (NULL == p_parent->path[current].p_nexthop) )
    {                                 
        ospf_logx(ospf_debug_route, "no asbr route");
        return; 
    }    

    /*check area&path for nssa*/
    if (OSPF_LS_TYPE_7 == p_external->h.type)
    {
        /*route to asbr must be intra area*/        
        if ( (OSPF_PATH_INTRA != p_parent->path[current].type)
            ||(p_area != p_parent->path[current].p_area) )        
        {
            ospf_logx(ospf_debug_route, "asbr route not in area %d,ignore", p_area->id);            
            return;
        }
     
        /*default route,for abr,ignore route when p bit
        is 0,or import summary is false*/
        if ( (0 == dest) && (0 == mask) 
			&& p_process->abr 
			&& (ospf_backbone_full_nbr_exsit(p_process)))
        {
            if (!ospf_option_nssa(p_external->h.option) || p_area->nosummary)
            {                        
                return; 
            }
        }
    }

    /*route for fwding address*/
    if (0 != faddr)
    {
//		printf("%s %d  ****p_process 0x%x ,faddr 0x%x*******\n", __FUNCTION__,__LINE__,p_process,faddr);
        if (ospf_if_lookup(p_process, faddr)) 
        {
            ospf_logx(ospf_debug_route, "ignore external route with self nexthop");
	//		printf("%s %d  ****ospf_if_lookup return*******\n", __FUNCTION__,__LINE__);
            return;
        }
    
        p_parent = ospf_fwd_route_lookup(p_process, faddr, p_area);
        if (NULL == p_parent)
        {
            ospf_logx(ospf_debug_route, "no fwding route to %x,stop", faddr);
            return;
        }
    }
    
    /* section 3.5, item (5) RFC-1587 */
    if (ospf_nexthop_empty(p_parent->path[current].p_nexthop) == TRUE)
    {
        return;
    }
    
    /* section 3.5, item (3) RFC-1587*/           
    memset(&route, 0, sizeof(route));
    memset(&nexthop, 0, sizeof(nexthop));
    route.type = OSPF_ROUTE_NETWORK;
    route.mask = mask;
    route.dest = dest;
    route.path[current].p_area = p_parent->path[current].p_area;
    route.path[current].adv_id = asbr_id;
    /*set cost*/    
    if (ospf_ase2_metric(ntohl(p_external->metric)))
    {
        route.path[current].type = OSPF_PATH_ASE2;
        route.path[current].cost = p_parent->path[current].cost;
        route.path[current].cost2 = metric;
    }
    else
    {
        route.path[current].type = OSPF_PATH_ASE;
        route.path[current].cost = p_parent->path[current].cost + metric;
    }
    
    route.path[current].tag =ntohl(p_external->tag);
    if (0 == p_external->fwdaddr)
    {
        /*have no faddr,use parent's nexthop directly*/
        route.path[current].p_nexthop = p_parent->path[current].p_nexthop ;
    }
    else
    {
        /*adjust nexthop for local connected forwarding address,local 
          route has no nexthop,must change to faddr*/
        memcpy(&nexthop, p_parent->path[current].p_nexthop, sizeof(nexthop));

        for (i = 0 ; i < nexthop.count; i++)
        {
//			printf("%s %d  ****p_process 0x%x ,addr 0x%x*******\n", __FUNCTION__,__LINE__,p_process,nexthop.gateway[i].addr);
            p_if = ospf_if_lookup(p_process, nexthop.gateway[i].addr); 
	 //			printf("%s %d, p_if = %p\n", __FUNCTION__,__LINE__, p_if);
				if (p_if != NULL)
				{
	//				printf("%s %d, p_if->addr = 0x%x, p_if->ifnet_uint = 0x%x\n", __FUNCTION__,__LINE__, p_if->addr, p_if->ifnet_uint);
				}
		//	printf("%s %d  ****ospf_if_lookup addr 0x%x uint 0x%x*******\n", __FUNCTION__,__LINE__,
		//		nexthop.gateway[i].addr, nexthop.gateway[i].if_uint);
            if ((NULL != p_if )
                && ((NULL == p_area) || (p_if->p_area == p_area))
                && ospf_netmatch(p_external->fwdaddr, p_if->addr, p_if->mask))
            {
                nexthop.gateway[i].addr = p_external->fwdaddr;
                nexthop.gateway[i].if_uint = p_if->ifnet_uint;
		//		printf("%s %d  ****ospf_if_lookup addr 0x%x uint 0x%x*******\n", __FUNCTION__,__LINE__,
		//		nexthop.gateway[i].addr, nexthop.gateway[i].if_uint);
                break;
            }
        }
        /*add this nexthop to system,do not add reference,*/
        route.path[current].p_nexthop = ospf_nexthop_add(p_process, &nexthop);
    }
    ospf_route_add(p_process, &p_process->route_table, &route);
    return;
}

/*check route change after calculating*/

/*compare route path,update related checking flag*/
void 
ospf_route_path_change_check(
               struct ospf_process *p_process,
               struct ospf_route *p_route)
{
    struct ospf_path *p_newpath = NULL;
    struct ospf_path *p_oldpath = NULL;
    u_int current = p_process->current_route;
    u_int old = p_process->old_route;
    u_int gateway = 0;
    u_int i = 0;
    
    p_oldpath = &p_route->path[old];
    p_newpath = &p_route->path[current];

    /*path cost/type/area changed,do not compare nexthop*/
    if (0 != ospf_path_cmp(p_oldpath, p_newpath))
    {  
        p_route->path_change = TRUE;
	//    printf("%s:%d\r\n",__FUNCTION__,__LINE__);    
        return;        
    }

    /*path cost/type/area not change,compare nexthop*/
    /*check old nexthop*/
    if ((OSPF_PATH_INVALID != p_oldpath->type) 
         && (NULL != p_oldpath->p_nexthop))
    {
        for (i = 0 ; i < p_oldpath->p_nexthop->count; i++)
        {        
            gateway = p_oldpath->p_nexthop->gateway[i].addr;

            /*ignore invalid nexthop,do not update it*/
            if (0 == gateway)
            {                                        
                continue;
            }
                
            /*ignore same nexthop as new path*/
            if ((NULL != p_newpath->p_nexthop)
                && (TRUE == ospf_nexthop_exist(p_newpath->p_nexthop , gateway)))
            {
                BIT_LST_SET(p_oldpath->msg_send, i);
                continue;
            }
            /*route path changed*/
	//		printf("%s:%d\r\n",__FUNCTION__,__LINE__);	  
            p_route->path_change = TRUE;
        }
    }

    /*check new nexthop*/
    if ((OSPF_PATH_INVALID != p_newpath->type)
         && (NULL != p_newpath->p_nexthop))
    {
        for (i = 0 ; i < p_newpath->p_nexthop->count; i++)
        {
            gateway = p_newpath->p_nexthop->gateway[i].addr;

            /*ignore invalid nexthop ,do not update it*/
            if (0 == gateway)                                  
            {
                continue;
            }
            /*if exist in old path,do not add*/
            if ((NULL != p_oldpath->p_nexthop )
                && (TRUE == ospf_nexthop_exist(p_oldpath->p_nexthop , gateway) ))
            {                
                BIT_LST_SET(p_newpath->msg_send, i);
                continue;
            }
	//		printf("%s:%d\r\n",__FUNCTION__,__LINE__);	  
            /*route path changed*/
            p_route->path_change = TRUE;
        }
    }
    return;
}

/*return OK or error,OK-all update finished,ERR-some update failed*/
STATUS 
ospf_sys_route_update(
               struct ospf_process *p_process,
               struct ospf_route *p_route)
{
    struct ospf_iproute iproute;
    struct ospf_nexthop nexthoptmp = {0};
    struct ospf_path *p_new = NULL;
    struct ospf_path *p_old = NULL;
    u_int current = p_process->current_route;
    u_int old = p_process->old_route;
    u_int i = 0;
    int iMaxecmp = p_process->max_ecmp;
    struct ospf_nhop_weight *pstNhopw = p_process->nhopw;
    int iNhopflg = FALSE;
    int iMaxecmpflg = FALSE;

	if ((NULL == p_route) 
        || (OSPF_ROUTE_NETWORK != p_route->type)
      /* 20131018 || (TRUE == p_route->local_route)*/)
	{
	    /*reset check flag for delete*/
		if (p_route)
		{
		    p_route->path_checked = FALSE;
		    p_route->summary_checked = FALSE;
		}
#ifdef OSPF_DCN
		if(p_process->process_id != OSPF_DCN_PROCESS)
		{
			return OK;
		}
#endif
	}
	/*等价路由负载均衡处理*/
	if (p_route && (p_route->path[current].p_nexthop))
	{
        if(OSPF_ROUTE_NETWORK == p_route->type)
        {
            if(p_route->path[current].p_nexthop->count > 1)
            {
                if((p_route->path[old].p_nexthop == NULL) || (p_route->path[current].p_nexthop == p_route->path[old].p_nexthop))
                {
                    memcpy(&nexthoptmp, p_route->path[current].p_nexthop, sizeof(nexthoptmp));
                }
                if(p_process->nhopw->flag)
                {
                    iNhopflg = ospf_ecmp_nhop_check(p_route->path[current].p_nexthop, pstNhopw);
                }
                iMaxecmpflg = ospf_max_ecmp_check(p_route->path[current].p_nexthop, iMaxecmp);
                    
               if(p_route->path[old].p_nexthop != NULL)
                {
                    if((nexthoptmp.count != 0) && (nexthoptmp.gateway->addr !=0))
                	{
	                    if(p_route->path[current].p_nexthop != p_route->path[old].p_nexthop)
	                    {
	                        memcpy(p_route->path[old].p_nexthop, &nexthoptmp, sizeof(nexthoptmp)); 
	                    }
	                    else 
	                    {
	                        p_route->path[old].p_nexthop = &nexthoptmp;
	                    }
                	}
            	}
        	}
        }
    }
   
  //	printf("ospf_sys_route_update path_checked=%d,path_change=%d,\r\n",p_route->path_checked,p_route->path_change);
  	ospf_logx(ospf_debug_route,"ospf_sys_route_update path_checked=%d,path_change=%d,\r\n",p_route->path_checked,p_route->path_change);
    /*compare route path if necessary*/
    if (FALSE == p_route->path_checked)
    {
        p_route->path_checked = TRUE;
        p_route->path_change = FALSE;

        /*suppose all nexthop update need*/
        memset(p_route->path[old].msg_send, 0, sizeof(p_route->path[old].msg_send));
        memset(p_route->path[current].msg_send, 0, sizeof(p_route->path[current].msg_send));

        ospf_route_path_change_check(p_process, p_route);
    }
    /*if route not changed,do nothing*/
    if (FALSE == p_route->path_change)
    {
        /*reset check flag for delete*/
        p_route->path_checked = FALSE;
        p_route->summary_checked = FALSE;
//	printf("%s,%d:ospf_sys_route_update path_checked=%d,path_change=%d,\r\n",__FUNCTION__,__LINE__,p_route->path_checked,p_route->path_change);
        return OK;
    }

    memset(&iproute, 0, sizeof(iproute));
    iproute.p_process = p_process;
    iproute.dest = p_route->dest;
    iproute.mask = p_route->mask;
    iproute.proto = M2_ipRouteProto_ospf; 

    p_old = &p_route->path[old];
    p_new = &p_route->path[current];
    
    /*update ip route for each changed nexthop*/
	
  	ospf_logx(ospf_debug_route,"p_old local_route=%d,p_new local_route=%d\r\n",p_old->local_route,p_new->local_route);
    /*delete old nexthop*/
   if (!p_old->local_route)
    {
		ospf_logx(ospf_debug_route,"p_old %d,type:%d,p_nexthop:0x%x\r\n",__LINE__,p_old->type,p_old->p_nexthop);    
        if ((OSPF_PATH_INVALID != p_old->type) && p_old->p_nexthop)
        {
 //			printf("%s:%d count:%d\r\n",__FUNCTION__,__LINE__,p_old->p_nexthop->count);    
			
 			ospf_logx(ospf_debug_route,"%d\r\n",__LINE__);    
           /*update ip route for each changed nexthop*/
            for (i = 0 ; i < p_old->p_nexthop->count; i++)
            {  
#ifdef OSPF_DCN
            	if(p_process->process_id != OSPF_DCN_PROCESS)
#endif
		        {
    
                    /*if both flag are set,ignore it*/            
                    if (BIT_LST_TST(p_old->msg_send, i))
                    {
                       // printf("%s:%d,del continue addr=0x%x,if_uint=0x%x,type=%d,path_type=%d\r\n",
                        //    __FUNCTION__,__LINE__,p_old->p_nexthop->gateway[i].addr,
                        //    p_old->p_nexthop->gateway[i].if_uint,
                        //    p_old->p_nexthop->gateway[i].type,p_old->type);   
                        ospf_logx(ospf_debug_route,"%d continue\r\n",__LINE__);    
                        continue;
                    }
                }
             
                iproute.fwdaddr = p_old->p_nexthop->gateway[i].addr; 
                iproute.if_unit = p_old->p_nexthop->gateway[i].if_uint; 
                iproute.nexthop_type = p_old->p_nexthop->gateway[i].type;
                iproute.active = FALSE;
                iproute.path_type = p_old->type;
                iproute.metric = 0; 
    
//		printf("%s:%d\r\n",__FUNCTION__,__LINE__);	
                /*if can not be exported,ignore it*/
                if (FALSE == ospf_sys_route_verify(&iproute))
                {
                    BIT_LST_SET(p_old->msg_send, i);
                    continue;
                }
//		printf("%s:%d\r\n",__FUNCTION__,__LINE__);	
                /*nexthop not change,only cost change*/
                if (p_new->p_nexthop
                    && (TRUE == ospf_nexthop_exist(p_new->p_nexthop , iproute.fwdaddr)))
                {
                   iproute.cost_change = TRUE; 
                }
		//printf("ospf_rtsock_route_msg_insert %s:%d\r\n",__FUNCTION__,__LINE__);
		ospf_logx(ospf_debug_route,"ospf_rtsock_route_msg_insert %d\r\n",__LINE__);
                /*insert failed,stop processing*/
                if (OK != ospf_rtsock_route_msg_insert(&iproute))
                {
 //		ospf_logx(ospf_debug_route,"%s:%d\r\n",__FUNCTION__,__LINE__);    
                    return ERR;
                }
                /*set flag,indicating msg send ok*/
                BIT_LST_SET(p_old->msg_send, i);
            }
        }
    }

  //  ospf_logx(ospf_debug_route,"%s:%d,p_new->local_route :%d\r\n",__FUNCTION__,__LINE__,p_new->local_route);	
    /*add new path*/
           /*20131018*/
    if (!p_new->local_route)		
    {  
	//	printf("p_new %s:%d\r\n",__FUNCTION__,__LINE__);    
		ospf_logx(ospf_debug_route,"%d\r\n",__LINE__);    
        if ((OSPF_PATH_INVALID != p_new->type) && p_new->p_nexthop)
        {
		//	printf("%s:%d\r\n",__FUNCTION__,__LINE__);    
			ospf_logx(ospf_debug_route,"%d\r\n",__LINE__);    
            for (i = 0 ; i < p_new->p_nexthop->count; i++)
            {  
#ifdef OSPF_DCN
	            if(p_process->process_id != OSPF_DCN_PROCESS)
#endif
		        {
                    /*if both flag are set,ignore it*/            
    	            if (BIT_LST_TST(p_new->msg_send, i))
    	            {
    				//	printf("%s:%d,add continue addr=0x%x,if_uint=0x%x,type=%d,path_type=%d\r\n",
                      //      __FUNCTION__,__LINE__,p_new->p_nexthop->gateway[i].addr,
                      //      p_new->p_nexthop->gateway[i].if_uint,
                      //      p_new->p_nexthop->gateway[i].type,p_new->type);    
    					ospf_logx(ospf_debug_route,"%d\r\n",__LINE__);
    	                continue;
    	            }
                }
    
                iproute.fwdaddr = p_new->p_nexthop->gateway[i].addr; 
                iproute.if_unit = p_new->p_nexthop->gateway[i].if_uint; 
                iproute.nexthop_type = p_new->p_nexthop->gateway[i].type;
                iproute.active = TRUE;
                iproute.path_type = p_new->type;
                #if 0
                iproute.metric = p_new->cost; 
                #else
                iproute.metric = (OSPF_PATH_ASE2 == p_new->type) ? p_new->cost2 : p_new->cost; 
                #endif
    
                /*if can not be exported,ignore it*/
                if (FALSE == ospf_sys_route_verify(&iproute))
                {
			//printf("%s:%d\r\n",__FUNCTION__,__LINE__);    
			ospf_logx(ospf_debug_route,"%d\r\n",__LINE__);    
                    BIT_LST_SET(p_new->msg_send, i);
                    continue;
                }
     	//			printf("%s:%d\r\n",__FUNCTION__,__LINE__);    
 	//			printf("ospf_rtsock_route_msg_insert %s:%d\r\n",__FUNCTION__,__LINE__);
 				ospf_logx(ospf_debug_route,"%d\r\n",__LINE__);    
 				ospf_logx(ospf_debug_route,"ospf_rtsock_route_msg_insert %d\r\n",__LINE__);
              /*if rtmsg need,send rtmsg,return failed if not ok*/
                if (OK != ospf_rtsock_route_msg_insert(&iproute))
                {
	//	ospf_logx(ospf_debug_route,"%s:%d\r\n",__FUNCTION__,__LINE__);	
                    return ERR;
                }
                /*set flag,indicating msg send ok*/
                BIT_LST_SET(p_new->msg_send, i);
            }
        }
    }

    /*reset check flag for delete*/
    p_route->path_checked = FALSE;
    p_route->summary_checked = FALSE;
  //  printf("%s %d  *****ospf_sys_route_update ERR******\n", __FUNCTION__,__LINE__);
    return OK;
}

/*clear old path of route,if new path is null,delete route self*/
void 
ospf_old_route_clear(
              struct ospf_process *p_process,
              struct ospf_lst *p_list,
              struct ospf_route *p_route)
{
    u_int old = p_process->old_route;
    u_int current = p_process->current_route;

//	printf("%s %d  *****old = %d,type = %d,current = %d,type = %d,******\n", __FUNCTION__,__LINE__,
//		old,p_route->path[old].type,current,p_route->path[current].type);

    /*clear old path of route*/
    if (OSPF_PATH_INVALID != p_route->path[old].type)
    {
        ospf_path_clear(&p_route->path[old]); 
    }
    /*if new path is null, delete it*/
    if (OSPF_PATH_INVALID == p_route->path[current].type)
    {
#ifdef OSPF_DCN
		if(p_process->process_id == OSPF_DCN_PROCESS)
		{
			ospf_sys_route_update(p_process,p_route);
		}
#endif
        ospf_lstdel_free(p_list, p_route, OSPF_MROUTE);
    }/*set nexthop's active flag*/
    else if (p_route->path[current].p_nexthop)
    {
        p_route->path[current].p_nexthop->active = TRUE;
//	printf("%s %d  ***********\n", __FUNCTION__,__LINE__);
//	ospf_ospf_route_print(p_route);
    }
//	printf("%s %d  *****p_route %x******\n", __FUNCTION__,__LINE__,p_route);
	//ospf_ospf_route_print(p_route);
    return;
}

/*check all change for abr routes,just delete unused one*/
void 
ospf_abr_route_change_check(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next = NULL;
    struct ospf_route *p_route = NULL;
    struct ospf_route *p_nextroute = NULL;

    /*scan area*/
    for_each_ospf_area(p_process, p_area, p_next)
    {
        for_each_node(&p_area->abr_table, p_route, p_nextroute)
        {
//		printf("%s %d  *****p_route %x******\n", __FUNCTION__,__LINE__,p_route);
//		ospf_ospf_route_print(p_route);
		ospf_old_route_clear(p_process, &p_area->abr_table, p_route);
        }
    }
    return;
}

/*check all change for abr routes,update summary lsa,and delete unused one*/
void 
ospf_asbr_route_change_check(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next = NULL;
    struct ospf_route *p_route = NULL;
    struct ospf_route *p_nextroute = NULL;

    /*scan area*/
    for_each_ospf_area(p_process, p_area, p_next)
    {
        for_each_node(&p_area->asbr_table, p_route, p_nextroute)
        {
            ospf_summary_lsa_update(p_process, p_route);
            
            ospf_old_route_clear(p_process, &p_area->asbr_table, p_route);
        }
    }
    return;
}

/*check route change after calculating*/
void 
ospf_summary_lsa_update(
               struct ospf_process *p_process,
               struct ospf_route *p_route)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next = NULL;    
    struct ospf_route dummy;
    struct ospf_iproute iproute;
    struct ospf_iproute *p_iproute = NULL;
    u_int current = p_process->current_route;
    u_int old = p_process->old_route;
    int path_cmp = 0;
    u_int flush;
    
    if ((NULL == p_route) || (OSPF_ROUTE_ABR == p_route->type))
    {
        return;
    }

    path_cmp = ospf_path_cmp(&p_route->path[old], &p_route->path[current]);
    /*if no change,do nothing,do not care about nexthop,but area*/
    if ((0 == path_cmp) && (p_route->path[old].p_area == p_route->path[current].p_area))
    {
        return;
    }

    memset(&dummy, 0, sizeof(dummy));             
    dummy.dest = p_route->dest;
    dummy.mask = p_route->mask;
    dummy.type = p_route->type;
    memcpy(&dummy.path[current], &p_route->path[old], sizeof(struct ospf_path));
    
    for_each_ospf_area(p_process, p_area, p_next)
    {
        /*flush old lsa if new lsa not need*/
        if ((OSPF_PATH_INVALID != p_route->path[old].type)
            && ((OSPF_PATH_INVALID == p_route->path[current].type) || !p_process->abr))
        {
            flush = TRUE;
            /*vpn process:check if can redistribute from bgp, if not,delete it*/
            if (ospf_is_vpn_process(p_process)
                && (BIT_LST_TST(p_process->reditribute_flag, M2_ipRouteProto_bgp)))
            {
                memset(&iproute, 0, sizeof(iproute));
                iproute.dest = dummy.dest;
                iproute.mask = dummy.mask;
                iproute.proto = M2_ipRouteProto_bgp;
                                    
                p_iproute = ospf_lstlookup(&p_process->import_table, &iproute);
                if (p_iproute && p_iproute->vpn_internal)
                {
                    flush = FALSE;
                }
            }
            if (TRUE == flush)
            {
                ospf_summary_lsa_originate(&dummy, p_area, TRUE);
            }

        }
        /*build new lsa*/ 
        if ((OSPF_PATH_INVALID != p_route->path[current].type) && p_process->abr)
        {
            ospf_summary_lsa_originate(p_route, p_area, FALSE);
        }
    }
    return;    
}

/*api used for construct an ospf route according to dest mask,nexthop,and curent index
   only used for update process
*/
void 
ospf_update_route_construct(
                   struct ospf_route *p_route,
                   u_int dest,
                   u_int mask,
                   u_int cost,
                   u_int cost2,
                   struct ospf_nexthop *p_nexthop,
                   u_int current/*path index*/)
{
    u_int old = 1 - current;
    
    memset(p_route, 0, sizeof(*p_route));
    
    p_route->dest = dest;
    p_route->mask = mask;
    p_route->type = OSPF_ROUTE_NETWORK;
  /*  p_route->local_route = FALSE*/
 
    /*old path do not used*/
    p_route->path[old].type = OSPF_PATH_INVALID;
          
    p_route->path[current].type = OSPF_PATH_ASE;
    p_route->path[current].cost = cost;
    p_route->path[current].cost2 = cost2;
 
    /*set nexthop directly,do not increase reference*/
    p_route->path[current].p_nexthop = p_nexthop;
    return;
}

/*20130517 fast protection:we can calculate directly without waiting in special case*/
void
ospf_fast_spf_timer_expired(struct ospf_process *process)
{
    if (process->fast_spf_count)
    {
       process->fast_spf_count = 0;
       ospf_timer_start(&process->fast_spf_timer, OSPF_FAST_SPF_INTERVAL);
    }
    return;
}

void ospf_ospf_route_print(struct ospf_route*p_route)
{
	struct ospf_nexthop *p_Nxthop = NULL;
	int i = 0,j = 0;
	if(p_route == NULL)
	{
		return;
	}
	vty_out_to_all_terminal("##ospf_ospf_route_print p_route 0x%x:##",p_route);
	vty_out_to_all_terminal("##node 0x%x,dest %x,mask %x,type %d,path_checked  %d,path_change %d,summary_checked %d,path %x##",
		p_route->node,p_route->dest,p_route->mask,p_route->type,p_route->path_checked,p_route->path_change,
		p_route->summary_checked,p_route->path);
	for(i = 0;i<2;i++)
	{
	
		vty_out_to_all_terminal("## i = %d, cost = %d,cost2 = %d,local_route =%d,type = %d##",
			i,p_route->path[i].cost,p_route->path[i].cost2,p_route->path[i].local_route,p_route->path[i].type);
		p_Nxthop = p_route->path[i].p_nexthop;
		if(p_Nxthop == NULL)
		{
			continue;
		}
		vty_out_to_all_terminal("##nexthop node 0x%x,count %d,active %d##",p_Nxthop->node,p_Nxthop->count,p_Nxthop->active);
		for (j = 0 ; j < OSPF_ECMP_COUNT ; j++)
		{
            if (0 != p_Nxthop->gateway[j].addr)
            {
         
        		vty_out_to_all_terminal("## %d:nexthop addr= %x,if_uint = %x,type= %d##",
        		j,
                p_Nxthop->gateway[j].addr,
                p_Nxthop->gateway[j].if_uint,
                p_Nxthop->gateway[j].type);
            }

		}
	}

}


void ospf_ospf_nexthop_print(struct ospf_nexthop *p_Nxthop)
{
	int i = 0,j = 0;
    
	if(p_Nxthop == NULL)
	{
		vty_out_to_all_terminal("##ospf_ospf_nexthop_print (p_Nxthop == NULL):##");
		return;
	}
	vty_out_to_all_terminal("ospf_ospf_nexthop_print ##nexthop  0x%x,count %d,active %d##",p_Nxthop,p_Nxthop->count,p_Nxthop->active);
	for (j = 0 ; j < OSPF_ECMP_COUNT ; j++)
	{
        if (0 != p_Nxthop->gateway[j].addr)
        {
     
    		vty_out_to_all_terminal("## %d:nexthop addr= %x,if_uint = %x,type= %d##",
    		j,
            p_Nxthop->gateway[j].addr,
            p_Nxthop->gateway[j].if_uint,
            p_Nxthop->gateway[j].type);
        }

	}


}
