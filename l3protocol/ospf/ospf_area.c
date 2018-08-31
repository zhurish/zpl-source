/* ospf_area.c --- contain functions about area operation, 
    include area,area range,nssa/stub operation*/

#include "ospf.h"

void ospf_abr_state_update(struct ospf_process * p_process);
void ospf_spf_table_init(struct ospf_area *p_area); 
extern void ospf_summary_lsa_build(struct ospf_area * p_area, struct ospf_route * p_route, struct ospf_summary_lsa * p_summary, u_int need_flush);
void ospf_T5_lsa_build(struct ospf_asbr_range *p_range);
void ospf_T7_lsa_build(struct ospf_area *p_area, struct ospf_asbr_range *p_range);
int ospf_nssa_area_exist(struct ospf_process *p_process);
u_int ospf_abr_state_get(struct ospf_process *p_process);
int ospf_asbr_area_check(struct ospf_process *p_process);


/*sort and cmp function for area table operation*/
int
ospf_area_lookup_cmp(
                 struct ospf_area *p1, 
                 struct ospf_area *p2)
{
    OSPF_KEY_CMP(p1, p2, id);
    return 0;
}

/*nm access compare function*/
int 
ospf_area_nm_cmp(
                 struct ospf_area *p1, 
                 struct ospf_area *p2)
{
    /*process id*/ 
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
    return ospf_area_lookup_cmp(p1, p2);
}

/*lookup area according to id*/
struct ospf_area * 
ospf_area_lookup(
                struct ospf_process *p_process, 
                u_int area_id) 
{
    struct ospf_area area;
    area.id = area_id;
    return ospf_lstlookup(&p_process->area_table, &area);    
}

/*init process's area table*/
void 
ospf_area_table_init(struct ospf_process *p_process)
{
    ospf_lstinit(&p_process->area_table, ospf_area_lookup_cmp);
    return;
}

/*Area range related ,sort and cmp function*/
int 
ospf_range_lookup_cmp(
                 struct ospf_range *p1, 
                 struct ospf_range *p2)
{
    OSPF_KEY_CMP(p1, p2, lstype); 
    OSPF_KEY_CMP(p1, p2, network); 
    OSPF_KEY_CMP(p1, p2, mask); 
    return 0;
}

/*cmp function for nm access*/
int
ospf_range_nm_cmp(
                 struct ospf_range *p1, 
                 struct ospf_range *p2)
{
    int rc;
    /*compare area first*/
    rc = ospf_area_nm_cmp(p1->p_area, p2->p_area);
    if (rc)
    {
        return rc;
    }
    return ospf_range_lookup_cmp(p1, p2);
}

/*ospf nssa area lookup*/
struct ospf_area *ospf_nssa_area_lookup(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL ;
    struct ospf_area *p_areaNext = NULL ;
    for_each_node(&p_process->area_table, p_area, p_areaNext)
    {
        if(p_area->is_nssa)
        {
            return p_area;
        }
    }
    return NULL;
}
/*end*/

/*api :search exact area range entry according to 
  area id,network and mask*/
struct ospf_range *
ospf_range_lookup(
          struct ospf_area *p_area, 
          u_int8 ls_type, 
          u_int network,
          u_int mask) 
{
    struct ospf_range range;
    
    range.network = network;
    range.mask = mask;
    range.lstype = ls_type;
    return ospf_lstlookup(&p_area->range_table, &range);
}

/*create area structure*/
struct ospf_area *
ospf_area_create(
                struct ospf_process *p_process, 
                u_int areaid)
{
    struct ospf_area *p_area = ospf_malloc2(OSPF_MAREA);
    u_int type = 0;
    u_int tos = 0;

    if(ospf_lstcnt(&ospf.nm.area_table) >= AREACOUNTMAX)
    {
	 printf("Limit the area to 512.\n");
	 return NULL;
    }

    memset(p_area, 0, sizeof(struct ospf_area));
    p_area->id = areaid;
    p_area->p_process = p_process;
#ifdef OSPF_SNMP
    p_area->state = SNMP_ACTIVE;
#endif
    /*init table in the area*/
    ospf_lstinit(&p_area->range_table, ospf_range_lookup_cmp);
    ospf_lstinit2(&p_area->if_table, ospf_if_lookup_cmp, mbroffset(struct ospf_if, area_node));
        
    ospf_spf_table_init(p_area);
    ospf_route_table_init(&p_area->abr_table);
    ospf_route_table_init(&p_area->asbr_table);
#ifdef OSPF_FRR
    ospf_lstinit(&p_area->backup_spf_table, ospf_backup_spf_vertex_cmp);   
    ospf_lstinit(&p_area->backup_abr_table, ospf_backup_route_cmp);
    ospf_lstinit(&p_area->backup_asbr_table, ospf_backup_route_cmp);
#endif

    /*init timer of area*/
    ospf_timer_init(&p_area->nssa_timer, p_area, ospf_nssa_wait_timeout, p_process);
    ospf_timer_init(&p_area->delete_timer, p_area, ospf_area_delete_event_handler, p_process);
    ospf_timer_init(&p_area->transit_range_timer, p_area, ospf_range_for_transit_area, p_process);
    
    /* initialize lsa table of area*/
    for (type = OSPF_LS_ROUTER; OSPF_LS_TYPE_10 >= type; ++type)
    { 
        /*ignore unsupported lsa type*/
        if ((OSPF_LS_AS_EXTERNAL == type)
            || (OSPF_LS_MULTICAST == type)
            || (OSPF_LS_TYPE_8 == type)
            || (OSPF_LS_TYPE_9 == type))
        {
            continue;
        }
        /*create lsa table and init it*/
        p_area->ls_table[type] = ospf_malloc2(OSPF_MLSTABLE);
        if (NULL != p_area->ls_table[type])
        {
            ospf_lsa_table_init(p_area->ls_table[type], type, p_process, p_area, NULL);
            ospf_lstadd(&ospf.nm.area_lsa_table, p_area->ls_table[type]);
            ospf_lstadd(&ospf.lsatable_table, p_area->ls_table[type]);
        }
    }
 
    /*init nssa related params*/
    p_area->nssa_always_translate = FALSE;
    p_area->nssa_wait_time = OSPF_DEFAULT_ROUTER_DEAD_INTERVAL; 
    
    /*set backbone*/
    if (OSPF_BACKBONE == p_area->id)
    {
        p_process->p_backbone = p_area;
        p_process->p_backbone->Vlinkcfg = 0;
    }
    /*if it is the first area,redistribute all external routes*/
    if (0 == ospf_lstcnt(&p_process->area_table)) 
    { 
        ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
        p_process->import_update = TRUE;
    }      

    /*stub metric type is internal*/
    for (tos = 0; OSPF_MAX_TOS > tos; tos++) 
    {    
        p_area->stub_default_cost[tos].type = OSPF_STUB_METRICTYPE_INTERNAL;
    }

    /*insert into area table*/
    ospf_lstadd(&p_process->area_table, p_area);

    /*insert into nm table*/
    ospf_lstadd(&ospf.nm.area_table, p_area);
    /*will send sync msg to backup card*/
    //ospf_sync_event_set(p_area->syn_flag);
    p_area->syn_flag = TRUE;
    
    /*bring up area,do not care about interface*/
    ospf_area_up(p_area);
    
    return p_area;
}

/*destroy an Area*/
void 
ospf_area_delete(struct ospf_area *p_area)
{
    struct ospf_process *p_process = p_area->p_process;
    u_int type = 0;
   
    /*delete spf&candidate tree*/
    ospf_spf_table_flush(p_area);
#ifdef OSPF_FRR
    /*delete backup spf tree*/
    ospf_backup_spf_table_flush(p_area);
#endif
    /*clear range*/  
    ospf_flush_range(p_area);

    /*clear backbone*/
    if (p_process->p_backbone == p_area)
    {
        p_process->p_backbone = NULL;
    }

    /*send sync msg directly,do not wait*/
    if (OSPF_MODE_MASTER == p_process->p_master->work_mode)
    {    
        ospf_syn_area_send(p_area, FALSE, NULL);
    }

    /*delete lsa&lsa table*/
    for (type = OSPF_LS_ROUTER; OSPF_LS_TYPE_10 >= type; ++type)                                                    
    {
        if (NULL == p_area->ls_table[type])
        {
            continue;
        }
        ospf_lsa_table_shutdown(p_area->ls_table[type]);
        
        ospf_lstdel(&ospf.nm.area_lsa_table, p_area->ls_table[type]);
        ospf_lstdel(&ospf.lsatable_table, p_area->ls_table[type]);
    
        ospf_mfree(p_area->ls_table[type], OSPF_MLSTABLE);
        p_area->ls_table[type] = NULL;
    }
    /*remove from nm table*/
    ospf_lstdel(&ospf.nm.area_table, p_area);

    /*stop timer*/
    ospf_timer_stop(&p_area->nssa_timer);
    ospf_timer_stop(&p_area->delete_timer);
    ospf_timer_stop(&p_area->transit_range_timer);

    /*delete and free it*/
    ospf_lstdel_free(&p_process->area_table, p_area, OSPF_MAREA);   

    /*update abr state*/
    ospf_abr_state_update(p_process);
    return;
}

/*decide if some interface use a special area*/
u_int 
ospf_area_if_exist(struct ospf_area *p_area)
{
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_process *p_process = p_area->p_process;

    /*if some interface in this area,or vlink use it as transit area*/
    for_each_ospf_if(p_process, p_if, p_next_if)     
    {
        if ((p_if->p_transit_area == p_area) || (p_if->p_area == p_area))
        {
            ospf_logx(ospf_debug, "some interface use area,do not delete it");
            
            return TRUE;
        }
    }
    return FALSE;
}

/*area delete schedule event,in most case,area is deleted directly
   ,if there is large route table,system update may spend a longer
   period,we do not wait here,but start a timer to check if all system update finished
   ,if so,area is deleted*/
void
ospf_area_delete_event_handler(struct ospf_area *p_area)
{
    struct ospf_process *p_process = p_area->p_process;

    /*area may be reused during waiting for deletion,so do not delete
     area if it is used.*/
    if ((OK == ospf_network_match_area(p_process, p_area->id))
        || (TRUE == ospf_area_if_exist(p_area)))
    {
        return;
    }

   /*shutdown area's dynamic resource*/
    ospf_area_down(p_area);
#ifdef OSPF_DCN
    if(p_process->process_id == OSPF_DCN_PROCESS)
    {
        ospf_logx(ospf_debug_dcn,"%d,wait export=%d,send route msg=%d.\r\n",__LINE__,p_process->wait_export,p_process->send_routemsg);   
        p_process->wait_export = 0;
        p_process->send_routemsg = 0;
    }
#endif
    /*some sys update to be done,wait for finish*/
    if (p_process->wait_export || p_process->send_routemsg)
    {
        ospf_timer_start(&p_area->delete_timer, 2);
        return;
    }

    /*force to calculate ip route*/
    ospf_route_calculate_full(p_process);
#ifdef OSPF_DCN
    if(p_process->process_id == OSPF_DCN_PROCESS)
    {
        p_process->wait_export = 0;
        p_process->send_routemsg = 0;
    }
#endif
    /*if sys update not finished,wait for finish*/
    if (p_process->wait_export || p_process->send_routemsg)
    {
        ospf_timer_start(&p_area->delete_timer, 2);
        return;
    }

    /*all route update finished,delete area*/
    ospf_area_delete(p_area);
    return;
}

/*area up,one active interface appear..*/
void 
ospf_area_up(struct ospf_area *p_area)
{
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_iproute default_route;
  
    /*update abr status.  */
    ospf_abr_state_update(p_process);
  
    /*abr state may changed,so schedule sync msg for process*/
    ospf_sync_event_set(p_process->syn_flag);
    
    /*self is abr*/
    if (p_process->abr)
    {
        /*generate summary lsa for this new area*/
        ospf_summary_lsa_originate_for_area(p_area, !p_area->p_process->abr);
   
        /*originate default NSSA lsa /type 3 lsafor all of NSSA area*/
        if (p_area->is_nssa && (!p_area->nosummary))
        {                
            memset(&default_route, 0, sizeof(default_route));
            if (ospf_backbone_full_nbr_exsit(p_process))
            {
                /*use newly configured metric*/
                default_route.metric = ospf_extmetric(p_area->nssa_default_cost, p_area->nssa_cost_type);
                default_route.active = TRUE;
            }
            else
            {
                /*use newly configured metric*/
                default_route.metric = ospf_extmetric(OSPF_METRIC_INFINITY-1, 1);
                default_route.active = TRUE;
            }
                  
            ospf_nssa_lsa_originate(p_area, &default_route);                  
        }
   
        if (p_area->is_stub || (p_area->is_nssa && p_area->nosummary))
        {
            ospf_originate_summary_default_lsa(p_area, FALSE);
        }
    }
           
    /*if area enable te, try to regenerate type 10 lsa*/
    if (p_area->te_enable)
    {
        ospf_router_te_lsa_originate(p_area);
    } 
    
    /*schedule external route import*/
    ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
    p_process->import_update = TRUE;
  
    /*check all interface state*/
    ospf_lstwalkup(&p_process->if_table, ospf_if_state_update);
    
    /*regenerate all of router lsa if necessary*/
    ospf_lstwalkup(&p_process->area_table, ospf_router_lsa_originate);
    
    return;
}

/*area shutdown*/
void 
ospf_area_down(struct ospf_area *p_area)
{
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_range *p_range = NULL;
    struct ospf_range *p_next_range = NULL;
    u_int8 type = 0;
    int iRet = 0;

    /*bring down all interfaces*/
    for_each_node(&p_area->if_table, p_if, p_next_if)
    {
        ospf_ism(p_if, OSPF_IFE_DOWN);
    }  
    
    /*delete translated external lsa*/
    if (p_area->nssa_translator)
    {
        ospf_nssa_translator_down(p_area);
    }                       

    /*clear database of this area*/
    for (type = OSPF_LS_ROUTER; OSPF_LS_TYPE_10 >= type; ++type)                                                    
    {
        if (p_area->ls_table[type])
        {
            ospf_lsa_table_flush(p_area->ls_table[type]);
        }
    } 
    
    iRet = ospf_nssa_only_exist_exceptOwn(p_area);
    if(iRet == OK)
    {
        ospf_lsa_table_flush(&p_area->p_process->t5_lstable);
    }
    
    /*schedule external route import*/
    ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
    p_process->import_update = TRUE;
     
    /*clear  range-lsa originate for other area*/
    for_each_ospf_range(p_area, p_range, p_next_range)
    {
        ospf_originate_range_lsa(p_range, TRUE);
    }
    /*request calculate*/
    ospf_spf_request(p_process);
    return;
}

int ospf_nssa_only_exist_exceptOwn(struct ospf_area *pstArea)
{   
    struct ospf_area *p_Area =NULL;
    struct ospf_area *pstAreaFirst = NULL;
    struct ospf_area *pstAreaNext = NULL;
    int iCount = 0;
    
    for_each_ospf_area(pstArea->p_process, pstAreaFirst, pstAreaNext)
    {
        if(iCount == 2)
        {
            return ERR;
        }
        
        if(pstAreaFirst->id != pstArea->id)
        {
            p_Area = pstAreaFirst;
        }
        iCount++;
    }
    if((iCount == 2) && (p_Area->is_nssa == TRUE))
    {
        return OK;
    }

    return ERR;


}


/*check if router is an area border router
  RETURNS: TRUE or FALSE*/
u_int 
ospf_abr_state_get(struct ospf_process *p_process)
{
    if(NULL == p_process->p_backbone)
    {
        return FALSE;
    }
    /*if area's count >= 2,it is abr*/
    if (2 <= ospf_lstcnt(&p_process->area_table))
    {
        return TRUE;
    }
#if 1	
    if (ospf_is_vpn_process(p_process))
    {
        return TRUE;
    }
#endif	
    return FALSE;
}

/*update instance's abr state,if changed do some action*/
void 
ospf_abr_state_update(struct ospf_process *p_process) 
{
    struct ospf_area *p_area = NULL;        
    struct ospf_area *p_next = NULL;
    struct ospf_range *p_range = NULL;
    struct ospf_range *p_next_range = NULL;
    struct ospf_iproute default_route;
    u_int old_abr = p_process->abr;

    /*get new abr state*/
    p_process->abr = ospf_abr_state_get(p_process);  

    /*no change,do nothing*/
    if (old_abr == p_process->abr)
    {
        return;
    }
    /*router lsa flag need change*/
    ospf_lstwalkup(&p_process->area_table, ospf_router_lsa_originate);
    
    /*check for each area*/
    for_each_ospf_area(p_process, p_area, p_next)
    {
        /*we need update summary lsa for area,originate or delete*/
        ospf_summary_lsa_originate_for_area(p_area, !p_area->p_process->abr);

        /*try to translate t7 lsa into extern lsa,
          originate default NSSA lsa */
        if (p_area->is_nssa)
        {
            /*abr will elect translator, non-abr will 
              clear translated external lsa*/
            if (p_process->abr)
            {
                ospf_nssa_translator_elect(p_area);
            }       
            else if (p_area->nssa_translator)
            {
                ospf_nssa_translator_down(p_area);
            }                       
    
            /*if import summary enable,originate default type7 lsa*/ 
            if (!p_area->nosummary)
            {
                memset(&default_route, 0, sizeof(default_route));       
                if (ospf_backbone_full_nbr_exsit(p_process))
                {
                    /*use newly configured metric*/
                    default_route.metric = ospf_extmetric(p_area->nssa_default_cost, p_area->nssa_cost_type);
                    default_route.active = p_process->abr;
                }
                else
                {
                    /*use newly configured metric*/
                    default_route.metric = ospf_extmetric(OSPF_METRIC_INFINITY-1, 1);
                    default_route.active = p_process->abr;
                }
                     
                ospf_nssa_lsa_originate(p_area, &default_route);  
            }
        }
        /*clear range lsa if change from abr to non-abr*/
        if (!p_process->abr)
        {
            for_each_ospf_range(p_area,p_range, p_next_range)
            {
                ospf_originate_range_lsa(p_range, TRUE);
            }
        }
    }
    return;
}

/*address range related*/

/*create a range*/
struct ospf_range *
ospf_range_create(
                 struct ospf_area *p_area,
                 u_int lstype,
                 u_int dest,
                 u_int mask)
{
    struct ospf_range *p_range = NULL;
    struct ospf_range *p_loop_range = NULL;
    struct ospf_range *p_next_range = NULL;
    
    for_each_node(&ospf.nm.range_table, p_loop_range, p_next_range)
    {
        if ((p_loop_range->lstype == lstype)
            && (p_loop_range->network == dest)
            && (p_loop_range->mask == mask)
            && (p_loop_range->p_area->id == p_area->id))
        {
            ospf_log(ospf_debug, "range already exist");
            return NULL;
        }
    }
    p_range = ospf_malloc2(OSPF_MRANGE);
    if (NULL != p_range)
    {
        p_range->p_area = p_area;
        p_range->lstype = lstype;
        p_range->network = dest;
        p_range->mask = mask;

        /*add to area's range table*/
        ospf_lstadd(&p_area->range_table, p_range);

        /*add to nm range table*/
        ospf_lstadd(&ospf.nm.range_table, p_range);
    }
    return p_range;
}

/*delete a range*/
void 
ospf_range_delete(struct ospf_range *p_range)
{     
    /*remove from nm table*/
    ospf_lstdel(&ospf.nm.range_table, p_range);

    /*remove from area range table,and free memory*/
    ospf_lstdel_free(&(p_range->p_area->range_table), p_range, OSPF_MRANGE) ;
    return;
}

/*type 5 range delete*/
void 
ospf_T5_range_delete(struct ospf_asbr_range *p_range)
{     
    /*remove from nm table*/
    ospf_lstdel(&ospf.nm.asbr_range_table, p_range);

    /*remove from area range table,and free memory*/
    ospf_lstdel_free(&(p_range->p_process->asbr_range_table), p_range, OSPF_MASBRRANGE) ;
    return;
}
/*end*/

/*get best matched range for dest*/
struct ospf_range *
ospf_range_match(
               struct ospf_area *p_area,
               u_int lstype,
               u_int dest,
               u_int mask,
               struct ospf_range *p_range_exclued/*range to be exlcuded in matching*/)
{
    struct ospf_range *p_range = NULL; 
    struct ospf_range *p_best = NULL;         
    struct ospf_range *p_next = NULL;

    /*scan for all range*/
    for_each_ospf_range(p_area, p_range, p_next)
    {
        /*ignore excluded range ,or with different type*/
        if ((p_range->lstype != lstype) || (p_range == p_range_exclued))
        {
            continue;
        }
        /*route must be sub-network of range*/
        if (!ospf_netmatch(dest, p_range->network, p_range->mask)
            || (mask < p_range->mask))
        {
            continue;
        }        
        /*select range with longest mask*/
        if ((NULL == p_best ) || (p_range->mask > p_best->mask))
        {
            p_best = p_range;
        }               
    }
    return p_best;
}

 void
ospf_originate_summary_range_lsa(
                     struct ospf_range *p_range,
                     u_int need_flush)
{
    struct ospf_route route;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_area_next = NULL;     
    struct ospf_process *p_process = p_range->p_area->p_process;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_summary_lsa *p_summary = NULL;
    struct ospf_summary_lsa summary;
    struct ospf_lshdr lshdr;
    u_int current = p_process->current_route;
   
    memset(&summary, 0, sizeof(summary));
    memset(&route, 0, sizeof(struct ospf_route));
       
    route.type = OSPF_ROUTE_NETWORK;
    route.dest = p_range->network;
    route.mask = p_range->mask;
    route.path[current].cost = p_range->cost;
    
    for_each_ospf_area(p_process, p_area, p_area_next)
    {
        /*check if target area valid*/
        if ((p_area == p_range->p_area) || p_area->nosummary)
        {
            continue;
        }
        /*do not originate range lsa for transit area*/
        if ((FALSE == need_flush) && (TRUE == p_area->transit))
        {
            continue;
        }
        /*special for delete,mask must same*/ 
        if (TRUE == need_flush)
        {
            lshdr.type = OSPF_LS_SUMMARY_NETWORK;
            lshdr.id = htonl(p_range->network);
            lshdr.adv_id = htonl(p_process->router_id);
            
            p_lsa = ospf_lsa_lookup(p_area->ls_table[lshdr.type], &lshdr);
            if (p_lsa)
            {
                /*delete only exit lsa with same mask as range*/
                p_summary = (struct ospf_summary_lsa*)p_lsa->lshdr;
                if (ntohl(p_summary->mask) != p_range->mask)
                {
                    return;
                }
            }
        }
        ospf_summary_lsa_build(p_area, &route, &summary, need_flush);    
        ospf_local_lsa_install(p_area->ls_table[summary.h.type], &summary.h);
    }
    return;
}

 void
ospf_originate_nssa_range_lsa(
                       struct ospf_range *p_range, 
                       u_int need_flush)    
{
    struct ospf_process *p_process = p_range->p_area->p_process;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_external_lsa *p_external = NULL;
    struct ospf_lshdr lshdr;       
    struct ospf_iproute iproute;  
    
    /*special for deletion,mask must same*/ 
    if (TRUE == need_flush)
    {
        lshdr.type = OSPF_LS_AS_EXTERNAL;
        lshdr.id = htonl(p_range->network);
        lshdr.adv_id = htonl(p_process->router_id);
        p_lsa = ospf_lsa_lookup(&p_process->t5_lstable, &lshdr);
        if (p_lsa)
        {
            /*delete only exit lsa with same mask as range*/
            p_external = (struct ospf_external_lsa*)p_lsa->lshdr;
            if (ntohl(p_external->mask) != p_range->mask)
            {
                return;
            }
        }
    }
    ospf_build_external_route(&iproute, p_range->network, p_range->mask, p_range->cost, 0);
    iproute.active = !need_flush;
    iproute.p_process = p_process;
    ospf_external_lsa_originate(&iproute);
    return;
}

/*originate lsa for area range entry*/
void 
ospf_originate_range_lsa(
                       struct ospf_range *p_range, 
                       u_int need_flush)
{
    /*case of summary range*/
    if (OSPF_LS_SUMMARY_NETWORK == p_range->lstype)
    {
        ospf_originate_summary_range_lsa(p_range, need_flush);
    }
    else/*type 7 lsa*/
    {
        ospf_originate_nssa_range_lsa(p_range, need_flush);
    }
    return;
}

/*check intra-area route when range up*/
void
ospf_check_intra_route_when_range_up(
                    struct ospf_range *p_range,
                    struct ospf_route *p_route)
{
    struct ospf_process *p_process = p_range->p_area->p_process;
    struct ospf_range *p_best_range = NULL;
    struct ospf_path *p_path = NULL;
    struct ospf_area *p_area= NULL;
    struct ospf_area *p_area_next = NULL;
    u_int current = p_process->current_route;
    
    p_path = &p_route->path[current];
    
    /*ignore invalid path type,or not matched route*/
    if ((OSPF_PATH_INTRA != p_path->type) 
        || (p_range->p_area != p_path->p_area) 
        || !ospf_netmatch(p_route->dest, p_range->network, p_range->mask)
        || (p_route->mask < p_range->mask))
    {
        return;
    }
 
    /*get older matched range for route*/          
    p_best_range = ospf_range_match(p_range->p_area, p_range->lstype, p_route->dest, p_route->mask, p_range);
 
    /*there is better range for this route, do nothing*/
    if ((NULL != p_best_range) && (p_best_range->mask > p_range->mask)) 
    {
        ospf_logx(ospf_debug, "exit better range for route");
        return;
    }
 
    /*i'm best range, update cost and matched count*/
    p_range->rtcount++;
 
    /*select larger cost*/
    if (p_range->cost < p_path->cost)
    {
        p_range->cost = p_path->cost;
    }
        
    if (NULL == p_best_range)
    {
        /*no range found,will delete type3 lsa for this route*/
        for_each_ospf_area(p_process, p_area, p_area_next)
        {
            /*do not delete summary lsa for route:route's area is backbone,
             and target area is transit*/
            if ((OSPF_BACKBONE == p_path->p_area->id) && (TRUE == p_area->transit))
            {
                continue;
            }
            ospf_summary_lsa_originate(p_route, p_area, TRUE);  
        }             
    }      
    else if (p_best_range->advertise)
    {         
        /*current range's summary lsa may be changed,so update it*/
        ospf_timer_try_start(&p_process->range_update_timer, 5);
    }       
    return;
}

/*range enable*/
void 
ospf_range_up(struct ospf_range *p_range)
{
    struct ospf_process *p_process = p_range->p_area->p_process;
    struct ospf_route *p_route = NULL;
    struct ospf_route min_route;
    struct ospf_route max_route;
    
    /*if not abr,do nothing*/
    if (!p_process->abr)
    {
        ospf_logx(ospf_debug, "i am not abr,do nothing with range");
        return;
    }
 
    /*nssa range,nothing now*/
    if (OSPF_LS_TYPE_7 == p_range->lstype)
    {
        ospf_logx(ospf_debug, "nssa range");
        return;
    }
 
    /*reinit count and cost*/
    p_range->rtcount = 0;
    p_range->cost = 0;
 
    /*set scan range*/
    min_route.type = OSPF_ROUTE_NETWORK;
    min_route.dest = p_range->network;
    min_route.mask = 0;
 
    max_route.type = OSPF_ROUTE_NETWORK;
    max_route.dest = p_range->network | (~p_range->mask);
    max_route.mask = OSPF_HOST_MASK;
 
    /*scan routes under the range*/
    for_each_node_range(&p_process->route_table, p_route, &min_route, &max_route)
    {
        ospf_check_intra_route_when_range_up(p_range, p_route);
    }
      
    /*all matched route's inter-lsa cleared,decide if update summarylsa of the range*/
    if (p_range->advertise && (0 != p_range->rtcount))
    {
        ospf_originate_range_lsa(p_range, FALSE);
    }
    return;
}

 u_int 
ospf_check_intra_route_when_range_down(
                    struct ospf_range *p_range,
                    struct ospf_route *p_route)
{
    struct ospf_process *p_process = p_range->p_area->p_process;
    struct ospf_range *p_best_range =NULL;
    struct ospf_path *p_path = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_area_next = NULL;    
    u_int current = p_process->current_route;

    p_path = &p_route->path[current];
    
    /*ignore invalid path type,or not matched route*/
    if ((OSPF_PATH_INTRA != p_path->type) 
         || (p_range->p_area != p_path->p_area) 
         || !ospf_netmatch(p_route->dest, p_range->network, p_range->mask)
         || (p_route->mask < p_range->mask))
    {
        return FALSE;
    }
       
    /*range must be the best one*/
    p_best_range = ospf_range_match(p_range->p_area, p_range->lstype, p_route->dest, p_route->mask, NULL);
    if (p_best_range != p_range)
    {
        return FALSE;
    }
    /*decide if other range exist when range down*/     
    p_best_range = ospf_range_match(p_range->p_area, p_range->lstype, p_route->dest, p_route->mask, p_range);    
    if (NULL == p_best_range)
    {     
        /*no range exist,recover this route*/
        for_each_ospf_area(p_process, p_area, p_area_next)
        {
            ospf_summary_lsa_originate(p_route, p_area, FALSE);        
        }
    }
    else
    {
        /*will update rest range*/
        ospf_timer_try_start(&p_process->range_update_timer, 5);
    }
    return TRUE;
}

/*range disable */
void 
ospf_range_down(struct ospf_range *p_range)
{
    struct ospf_process *p_process = p_range->p_area->p_process;
    struct ospf_route *p_route = NULL;
    struct ospf_route min_route;
    struct ospf_route max_route;
    u_int same = FALSE;
    u_int affect = FALSE;
    
    p_range->isdown = TRUE;
    
    /*if not abr,do nothing*/
    if (!p_process->abr)
    {
        ospf_logx(ospf_debug, "i am not abr,do nothing with range");
        return;
    }
 
    /*nssa range,nothing now*/
    if (OSPF_LS_TYPE_7 == p_range->lstype)
    {
        ospf_logx(ospf_debug,  "nssa range");
        return;
    }
    
    /*set scan range*/
    min_route.type = OSPF_ROUTE_NETWORK;
    min_route.dest = p_range->network;
    min_route.mask = 0;
 
    max_route.type = OSPF_ROUTE_NETWORK;
    max_route.dest = p_range->network | (~p_range->mask);
    max_route.mask = OSPF_HOST_MASK;
 
    for_each_node_range(&p_process->route_table, p_route, &min_route, &max_route)
    {
        affect = ospf_check_intra_route_when_range_down(p_range, p_route);
        /*if this range has not been affected,do nothing*/
        if (FALSE == affect)
        {
            continue;
        }
        
         /*record if route same as range,this is a special case*/
        if ((!same) 
            && (p_range->network == p_route->dest)
            && (p_range->mask == p_route->mask))
        {
            same = TRUE;
        }
    }
   
    /*if same route not exist,delete summary lsa for this range*/
    if ((!same) && p_range->advertise)
    {
        ospf_originate_range_lsa(p_range, TRUE);
    } 
    return;
}

 void
ospf_check_intra_route_for_range(
                struct ospf_range *p_range,
                struct ospf_route *p_route)
{
    struct ospf_process *p_process = p_range->p_area->p_process;
    struct ospf_path *p_path = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_area_next = NULL;    
    u_int current = p_process->current_route;

    p_path = &p_route->path[current];
    
    /*ignore invalid path type,or not matched route*/
    if ((OSPF_PATH_INTRA != p_path->type) 
          || (p_range->p_area != p_path->p_area) 
          || !ospf_netmatch(p_route->dest, p_range->network, p_range->mask)
          || (p_route->mask < p_range->mask))
    {
        return;
    }
    /*this range is not the best range,fulsh the range lsa*/
    if (ospf_range_match(p_range->p_area, p_range->lstype,
         p_route->dest, p_route->mask, NULL) != p_range)
    {              
        return;
    }
         
    /*i'm best range, update cost and matched count*/
    p_range->rtcount++;
    if (p_range->cost < p_path->cost)
    {
        p_range->cost = p_path->cost;
    }

    /*route is same as range,do not delete it*/
    if (p_route->dest == p_range->network)
    {              
        return;
    }

    /*delete summary lsa for route*/
    for_each_ospf_area(p_process, p_area, p_area_next)
    {
         /*do not delete summary lsa for route:route's area is backbone,
         and target area is transit*/
        if ((OSPF_BACKBONE == p_path->p_area->id) && (TRUE == p_area->transit))
        {
            continue;
        }

        ospf_summary_lsa_originate(p_route, p_area, TRUE);      
    } 
    return;    
}

/*update range state if used*/
void 
ospf_range_update(struct ospf_range *p_range)
{
    struct ospf_process *p_process = p_range->p_area->p_process;
    struct ospf_route *p_route = NULL;
    struct ospf_route min_route;
    struct ospf_route max_route;
 
    ospf_logx(ospf_debug, "update range %x/%x", p_range->network, p_range->mask);
    
    /*if not abr,do nothing*/
    if (!p_process->abr)
    {
        ospf_logx(ospf_debug, "i am not abr,do nothing with range");
        return;
    }
 
    p_range->rtcount = 0;
    p_range->cost = 0;
 
    min_route.type = OSPF_ROUTE_NETWORK;
    min_route.dest = p_range->network;
    min_route.mask = 0;
 
    max_route.type = OSPF_ROUTE_NETWORK;
    max_route.dest = p_range->network | (~p_range->mask);
    max_route.mask = OSPF_HOST_MASK;

    /*scan routes covered by range*/
    for_each_node_range(&p_process->route_table, p_route, &min_route, &max_route)
    {
        ospf_check_intra_route_for_range(p_range, p_route);
    }
    
    /*now ,all matched route's inter-lsa cleared,
     decide if update summarylsa of the range*/
    if (p_range->advertise) 
    {
        ospf_originate_range_lsa(p_range, p_range->rtcount ? FALSE : TRUE);
    }
    return;
}
void
ospf_range_for_transit_area(struct ospf_area *p_transit_area)
{
    struct ospf_range *p_range = NULL;
    struct ospf_range *p_next_range = NULL;
    struct ospf_area *p_bone_area = ospf_area_lookup(p_transit_area->p_process, 0);
    struct ospf_route route;
    
    if (NULL == p_bone_area)
    {
        return ;
    }
    /*if spf caculate not finish ,wait. transit bit is set in spf*/
    if (ospf_timer_active(&p_transit_area->p_process->spf_timer)) 
    {
       ospf_stimer_start(&p_transit_area->transit_range_timer, 1);
    }
    /*vlink is creat*/
    if(p_transit_area->transit)
    {
        for_each_node(&p_bone_area->range_table, p_range, p_next_range)
        {
            memset(&route, 0, sizeof(route));

            route.dest = p_range->network;
            route.mask = p_range->mask;
            route.type = OSPF_LS_SUMMARY_NETWORK;
            /*flush range lsa in transit lsa*/
            ospf_summary_lsa_originate(&route, p_transit_area, TRUE);     

            /*reoriginate summary lsa for transit lsa*/
            ospf_summary_lsa_originate_for_area(p_transit_area, FALSE);
        }
    }
}
/*nssa range related*/

/*decide if nssa range will be updated*/
u_int 
ospf_get_covered_lsa_for_nssa_range(
                      struct ospf_range *p_range, 
                      struct ospf_lsvector *p_vector)
{
    struct ospf_process *p_process = p_range->p_area->p_process;
    struct ospf_lsa *p_lsa = NULL;       
    struct ospf_nssa_lsa *p_nssa = NULL;
    struct ospf_lst *p_list = &p_range->p_area->ls_table[OSPF_LS_TYPE_7]->list;
    struct ospf_lsa minlsa;
    struct ospf_lsa maxlsa;
    u_int id = 0;
    u_int mask = 0;
    u_int adv_id = 0;
    u_int same = FALSE;

    p_vector->count = 0;    
    
    minlsa.lshdr->type = OSPF_LS_TYPE_7;
    minlsa.lshdr->id = p_range->network;
    minlsa.lshdr->adv_id = 0;

    maxlsa.lshdr->type = OSPF_LS_TYPE_7;
    maxlsa.lshdr->id = p_range->network | (~p_range->mask);
    maxlsa.lshdr->adv_id = OSPF_HOST_NETWORK;

    /*scan all lsa under range*/
    for_each_node_range(p_list, p_lsa, &minlsa, &maxlsa)    
    {
        p_nssa = (struct ospf_nssa_lsa *)p_lsa->lshdr;
        id = ntohl(p_lsa->lshdr->id);
        mask = ntohl(p_nssa->mask);
        adv_id = ntohl(p_lsa->lshdr->adv_id);
        
       /*ignore lsa not match range or invalid*/
        if (OSPF_MAX_LSAGE <= ntohs(p_lsa->lshdr->age) 
            || (!ospf_netmatch(p_range->network, id , p_range->mask))
            || (mask < p_range->mask)
            || adv_id == p_process->router_id)
        {
            continue;
        }    
        
        p_vector->p_lsa[p_vector->count++] = p_lsa;

        /*tag if route is same as range*/
        if ((p_range->network == id) && (p_range->mask == mask))
        {
            same = TRUE;
        }     
    }

    /*only one nssa lsa match range,and same to range, do not aggregtae*/
    if (same && (1 == p_vector->count))
    {
        p_vector->count = 0;
    }
    return p_vector->count;
}

/*called when a nssa range up*/
void 
ospf_nssa_range_up(struct ospf_range *p_range)
{
    struct ospf_lsa *p_lsa = NULL;                 
    struct ospf_range *p_best_range = NULL;
    struct ospf_nssa_lsa *p_nssa = NULL;
    struct ospf_lsvector range_vector;
    u_int id = 0;
    u_int mask = 0;
    u_int lscost = 0;
    u_int i = 0;

    /*ignore non-nssa area,and non-translator*/
    if ((!p_range->p_area->is_nssa) 
        || (!p_range->p_area->nssa_translator))
    {
        return;
    }

    /*if no lsa covered,do nothing*/
    if (0 == ospf_get_covered_lsa_for_nssa_range(p_range, &range_vector))
    {
        return;
    }    

    p_range->rtcount = 0;  
    /*scan all covered lsa*/
    for (i= 0, p_lsa = range_vector.p_lsa[i]; 
         i < range_vector.count; 
         i++ ,p_lsa = range_vector.p_lsa[i])
    {
        p_nssa = (struct ospf_nssa_lsa *)p_lsa->lshdr;
        id = ntohl(p_lsa->lshdr->id);
        mask = ntohl(p_nssa->mask);

        /*check if other range matched*/
        p_best_range = ospf_range_match(p_range->p_area, OSPF_LS_TYPE_7, id, mask, p_range);
        /*there is better range exist,do nothing*/
        if ((NULL != p_best_range) && (p_best_range->mask > p_range->mask)) 
        {
            ospf_logx(ospf_debug, "exit better range for route");
            continue;
        }
        
        /*i'm best range, update cost and matched count*/
        p_range->rtcount++;
        
        /*select larger cost*/
        if (lscost < ntohl(p_nssa->metric))
        {
            lscost = ntohl(p_nssa->metric);
        }
          
        /*p_best_range is not the best range,so delete the lsa orinigate by p_best_range*/
        if ((NULL != p_best_range) && p_best_range->advertise)
        {
            p_best_range->need_check = TRUE;
            ospf_timer_try_start(&p_range->p_area->p_process->range_update_timer, 5);
        }
        else if (NULL == p_best_range)
        { 
            /*no range mathced,recover translated lsa*/
            ospf_nssa_lsa_translate(p_lsa);
        }
    }

    /*decide range cost*/
    p_range->cost = (ospf_ase2_metric(lscost)) ? (lscost + 1) : lscost;

    /*originate external lsa for range*/
    if (p_range->advertise && (0 != p_range->rtcount))
    {  
        ospf_originate_range_lsa(p_range, FALSE);
    }  
    return;
}

/*called when a nssa range is deleted*/
void 
ospf_nssa_range_down(struct ospf_range *p_range)
{
    struct ospf_lsa *p_lsa = NULL;       
    struct ospf_lsa *p_next_lsdb = NULL;           
    struct ospf_range *p_best_range = NULL;
    struct ospf_nssa_lsa *p_nssa = NULL;
    struct ospf_process *p_process = p_range->p_area->p_process; 
    u_int id = 0;
    u_int mask = 0;
    u_int adv_id = 0;
    u_int same = FALSE;
    
    if (!p_range->p_area->is_nssa || !p_range->p_area->nssa_translator)
    {  
        return;
    }
    
    for_each_nssa_lsa(p_range->p_area, p_lsa, p_next_lsdb)
    {
        p_nssa = (struct ospf_nssa_lsa *)p_lsa->lshdr;
        id = ntohl(p_lsa->lshdr->id);
        mask = ntohl(p_nssa->mask);
        adv_id = ntohl(p_lsa->lshdr->adv_id);
        
        /*ignore lsa not match range*/
        if (OSPF_MAX_LSAGE <= ntohs(p_lsa->lshdr->age) 
            || (!ospf_netmatch(p_range->network, id , p_range->mask))
            || (mask < p_range->mask)
            || (adv_id == p_process->router_id))
        {
            continue;
        }    
        /*tag if lsa same as range*/      
        if ((p_range->network == id) && (p_range->mask == mask))
        {
            same = TRUE;
        }
        /*if current range is not matched,ignore*/
        p_best_range = ospf_range_match(p_range->p_area, OSPF_LS_TYPE_7, id, mask, NULL);
        if (p_best_range != p_range) 
        {             
            continue;
        }

        /*decide if other range exist when range down*/
        p_best_range = ospf_range_match(p_range->p_area, OSPF_LS_TYPE_7, id, mask, p_range);
        if (NULL == p_best_range)
        {     
            /*no range exist,recover external lsa*/
            ospf_nssa_lsa_translate(p_lsa);
        }
        else
        {
            /*will update rest range*/
            p_best_range->need_check = TRUE;
            ospf_timer_try_start(&p_process->range_update_timer, 5);
        }
    }
    /*delete external lsa for range*/
    if ((!same) && p_range->advertise)
    {
        ospf_originate_range_lsa(p_range, TRUE);
    } 
    return;
}

u_int 
ospf_get_covered_lsa_for_nssa_range_asbr(
                      struct ospf_asbr_range *p_range, 
                      struct ospf_area *p_nssa_area,
                      struct ospf_lsvector *p_vector)
{
    struct ospf_process *p_process = p_range->p_process;
    struct ospf_lsa *p_lsa = NULL;       
    struct ospf_lsa *p_lsaNext = NULL;       
    struct ospf_nssa_lsa *p_nssa = NULL;
    struct ospf_lst *p_list = &p_nssa_area->ls_table[OSPF_LS_TYPE_7]->list;
    u_int id = 0;
    u_int mask = 0;
    u_int adv_id = 0;
    u_int same = FALSE;
    struct ospf_iproute *p_route;
    struct ospf_iproute *p_next_route;
    int iFlg = 0;

    p_vector->count = 0;    

       
    /*scan all lsa under range*/
  //  for_each_node_range(p_list, p_lsa, &minlsa, &maxlsa)    
    for_each_node(p_list, p_lsa,p_lsaNext )
    {
        if(p_lsa->lshdr->type != OSPF_LS_TYPE_7)
        {
            continue;
        }
        
        p_nssa = (struct ospf_nssa_lsa *)p_lsa->lshdr;
        id = ntohl(p_lsa->lshdr->id);
        mask = ntohl(p_nssa->mask);
        adv_id = ntohl(p_lsa->lshdr->adv_id);

        #if 0
       /*ignore lsa not match range or invalid*/
        if (OSPF_MAX_LSAGE <= ntohs(p_lsa->lshdr->age) 
            || (!ospf_netmatch(p_range->dest, id , p_range->mask))
            || (mask < p_range->mask))
            //|| (adv_id == p_process->router_id))
        #else
        /*ignore lsa not match range or invalid*/
        if (OSPF_MAX_LSAGE <= ntohs(p_lsa->lshdr->age) 
            || (!ospf_netmatch(p_range->dest, id , p_range->mask))
            || (mask < p_range->mask)
            || (adv_id != p_process->router_id))
        #endif
        {
            continue;
        }    
        
        p_vector->p_lsa[p_vector->count++] = p_lsa;

        /*tag if route is same as range*/
        if ((p_range->dest == id) && (p_range->mask == mask))
        {
            same = TRUE;
        } 

    }
    /*only one nssa lsa match range,and same to range, do not aggregtae*/
    if (same && (1 == p_vector->count))
    {
        for_each_node(&p_process->import_table,p_route, p_next_route)
        {
            if(ospf_netmatch(p_range->dest, p_route->dest, p_range->mask))
            {
                iFlg = 1;
                break;
            }
        }
        if(!iFlg)
        {
            //ospf_lsa_delete(p_vector->p_lsa);
            p_vector->p_lsa[0]->lshdr->age = htons(OSPF_MAX_LSAGE);
            p_vector->count = 0;
            ospf_stimer_safe_start(&p_process->lsa_aging_timer, OSPF_MIN_LSAGE_TIME);
        }
    }
    return p_vector->count;
}


/*type-7 lsa的聚合*/
void 
ospf_asbr_range_nssa_update(
                    struct ospf_area *p_nssa_area, 
                    struct ospf_asbr_range *p_range)
{
    struct ospf_lsa *p_lsa = NULL;                
    struct ospf_nssa_lsa *p_nssa = NULL;
    struct ospf_nssa_lsa stNssa ;
    struct ospf_lsvector range_vector;
    u_int id = 0;
    u_int mask = 0;
    u_int i = 0;
    u_int lscost = 0;
    struct ospf_process *p_process = NULL;     
    
    memset(&stNssa, 0, sizeof(struct ospf_nssa_lsa));

    if(p_range == NULL)
    {
        return;
    }
	p_process = p_range->p_process;
    /*ignore non-nssa area*/
    if (!p_nssa_area->is_nssa)
    { 
        return;
    }
#if 0
    /*if nssa waiting timer is running,do not update for nssa range*/
    if (ospf_timer_active(&p_range->p_area->nssa_timer))
    { 
        return;
    }
#endif
    /*delete external lsa for range,if self is not translator or there is no lsa covered*/
    if (0 == ospf_get_covered_lsa_for_nssa_range_asbr(p_range,p_nssa_area,&range_vector))
    { 
        //ospf_originate_range_lsa(p_range, TRUE);
        return;
    }    

    /*check lsa covered*/
    p_range->rtcount = 0;
    for (i = 0, p_lsa = range_vector.p_lsa[i]; 
        i < range_vector.count; 
        i++ ,p_lsa = range_vector.p_lsa[i])
    {    
        p_nssa = (struct ospf_nssa_lsa *)p_lsa->lshdr;
        id = ntohl(p_lsa->lshdr->id);
        mask = ntohl(p_nssa->mask);

        p_nssa->h.age = htons(OSPF_MAX_LSAGE);
 
        /*i'm best range, update cost and matched count*/
        p_range->rtcount++;
        if (lscost < ntohl(p_nssa->metric))
        {
            lscost = ntohl(p_nssa->metric); 
        }

        if(p_process->abr)
        {
         //   ospf_nssa_lsa_translate(p_lsa);
        }
        
    }
    
    /*now ,all matched route's inter-lsa cleared,decide if update summarylsa
       of the range*/
    p_range->cost = (ospf_ase2_metric(lscost)) ? (lscost + 1) : lscost;

    /*如果宣告，生成type-7 lsa*/
    if (p_range->advertise)
    {
        ospf_T7_lsa_build(p_nssa_area,p_range);
    }

    /*启动定时器，计算lsa*/
    ospf_stimer_safe_start(&p_process->lsa_aging_timer,OSPF_MIN_LSAGE_TIME);
    //ospf_lsa_table_timer_expired(p_process);
    return;
}
u_int 
ospf_get_covered_lsa_for_range_asbr_T5(
                      struct ospf_asbr_range *p_range, 
                      struct ospf_lsvector *p_vector)
{
    struct ospf_process *p_process = p_range->p_process;
    struct ospf_lsa *p_lsa = NULL;       
    struct ospf_lsa *p_lsaNext = NULL;       
    struct ospf_external_lsa *p_external = NULL;
    struct ospf_lst *p_list = &p_range->p_process->t5_lstable.list;
    u_int id = 0;
    u_int mask = 0;
    u_int adv_id = 0;
    u_int same = FALSE;
    int iFlg = 1;

    p_vector->count = 0;    

    /*scan all lsa under range*/
    for_each_node(p_list, p_lsa,p_lsaNext )
    {
        if(p_lsa->lshdr->type != OSPF_LS_AS_EXTERNAL)
        {
            continue;
        }
        
        p_external = (struct ospf_external_lsa *)p_lsa->lshdr;
        id = ntohl(p_lsa->lshdr->id);
        mask = ntohl(p_external->mask);
        adv_id = ntohl(p_lsa->lshdr->adv_id);
        
       /*ignore lsa not match range or invalid*/
        if (OSPF_MAX_LSAGE <= ntohs(p_lsa->lshdr->age) 
            || (!ospf_netmatch(p_range->dest, id , p_range->mask))
            || (mask < p_range->mask)
            || (adv_id != p_process->router_id))
        {
            continue;
        }    
        
        p_vector->p_lsa[p_vector->count++] = p_lsa;

        /*tag if route is same as range*/
        if ((p_range->dest == id) && (p_range->mask == mask))
        {
            same = TRUE;
        } 
        
    }
    /*only one nssa lsa match range,and same to range, do not aggregtae*/
    if (same && (1 == p_vector->count))
    {
        /*如果type7中没有匹配的lsa，则表明type5中可能存在匹配的lsa，
                所剩的lsa为聚合后的lsa*/
        struct ospf_area *p_area_nssa = NULL;
        struct ospf_area *p_areanext = NULL;
        struct ospf_lsvector range_vector;

        for_each_node(&p_process->area_table, p_area_nssa, p_areanext)
        {
            if(p_area_nssa->is_nssa)
            {
                if (ospf_get_covered_lsa_for_nssa_range_asbr(p_range, p_area_nssa, &range_vector) == 0)
                {
                    iFlg = 0;
                    break;
                }
            }
        }
        if(!iFlg)
        {
            //ospf_lsa_delete(p_vector->p_lsa);
            p_vector->p_lsa[0]->lshdr->age = htons(OSPF_MAX_LSAGE);
            p_vector->count = 0;
            ospf_stimer_safe_start(&p_range->p_process->lsa_aging_timer,OSPF_MIN_LSAGE_TIME);
        }
    }

   return p_vector->count;
}

void 
ospf_range_update_asbr_T5(struct ospf_asbr_range *p_range)
{
    struct ospf_lsa *p_lsa = NULL;                
    struct ospf_external_lsa *p_external = NULL;
    struct ospf_external_lsa stexternal ;
    struct ospf_lsvector range_vector;
    u_int id = 0;
    u_int mask = 0;
    u_int i = 0;
    u_int lscost = 0;
	struct ospf_area *p_area_nssa = NULL;

    memset(&stexternal, 0, sizeof(struct ospf_external_lsa));

    if(p_range == NULL)
    {
        return;
    }
    
    
#if 0
    /*if nssa waiting timer is running,do not update for nssa range*/
    if (ospf_timer_active(&p_range->p_area->nssa_timer))
    { 
        return;
    }
#endif
    /*delete external lsa for range,if self is not translator or there is no lsa covered*/
    if (//(!p_range->p_area->nssa_translator)
         (0 == ospf_get_covered_lsa_for_range_asbr_T5(p_range, &range_vector)))
    { 
        //ospf_originate_range_lsa(p_range, TRUE);
        return;
    }    
    #if 0/*ASBR也会对自己的type5进行聚合*/
    if(!p_range->p_process->asbr)
    {
        return;
    }
    #endif
    /*check lsa covered*/
    p_range->rtcount = 0;
    for (i = 0, p_lsa = range_vector.p_lsa[i]; 
        i < range_vector.count; 
        i++ ,p_lsa = range_vector.p_lsa[i])
    {    
        p_external = (struct ospf_external_lsa *)p_lsa->lshdr;
        id = ntohl(p_lsa->lshdr->id);
        mask = ntohl(p_external->mask);

        p_external->h.age = htons(OSPF_MAX_LSAGE);
 
        /*i'm best range, update cost and matched count*/
        p_range->rtcount++;
        if (lscost < ntohl(p_external->metric))
        {
            lscost = ntohl(p_external->metric); 
        }
        
    }
    
    /*now ,all matched route's inter-lsa cleared,decide if update summarylsa
       of the range*/
    p_range->cost = (ospf_ase2_metric(lscost)) ? (lscost + 1) : lscost;

    p_area_nssa = ospf_nssa_area_lookup(p_range->p_process);
    /*宣告，生成lsa*/
    if(p_range->advertise)
    {
        ospf_T5_lsa_build(p_range);
       
    }
    /*启动定时器，计算lsa*/
    ospf_stimer_safe_start(&p_range->p_process->lsa_aging_timer,OSPF_MIN_LSAGE_TIME);
    //ospf_lsa_table_timer_expired(p_process);
    return;
}

/*asbr summery operation*/
struct ospf_asbr_range *ospf_asbr_range_lookup(
                struct ospf_process *p_process,
                u_int8 ls_type, 
                u_int dest, 
                u_int mask)
{
    struct ospf_asbr_range range;
    
    range.dest = dest;
    range.mask = mask;
    //range.lstype = ls_type;
    return ospf_lstlookup(&p_process->asbr_range_table, &range);
}


struct ospf_asbr_range *ospf_asbr_range_create(
                     struct ospf_process *p_process,
                     u_int dest,
                     u_int mask)
{
    struct ospf_asbr_range *p_asbr_range = NULL;
    struct ospf_asbr_range *p_loop_range = NULL;
    struct ospf_asbr_range *p_next_range = NULL;

    for_each_node(&p_process->asbr_range_table, p_loop_range, p_next_range)
    {
        if ((p_loop_range->dest == dest)
            && (p_loop_range->mask == mask))
        {
            ospf_log(ospf_debug, "asbr range already exist");
            return NULL;
        }
    }
    p_asbr_range = ospf_malloc2(OSPF_MASBRRANGE);
    if (NULL != p_asbr_range)
    {
        p_asbr_range->p_process= p_process;
        p_asbr_range->dest = dest;
        p_asbr_range->mask = mask;
        p_asbr_range->advertise = TRUE;
        p_asbr_range->cost = 0;
        /*add to process's range table*/
        ospf_lstadd(&p_process->asbr_range_table, p_asbr_range);

        /*add to nm range table*/
        ospf_lstadd(&ospf.nm.asbr_range_table, p_asbr_range);
    }
    return p_asbr_range;
}
/*end*/

/*check route matched to a nssa range*/
void 
ospf_nssa_range_update(struct ospf_range *p_range)
{
    struct ospf_lsa *p_lsa = NULL;                
    struct ospf_nssa_lsa *p_nssa = NULL;
    struct ospf_lsvector range_vector;
    u_int id = 0;
    u_int mask = 0;
    u_int i = 0;
    u_int lscost = 0;

    /*ignore non-nssa area*/
    if (!p_range->p_area->is_nssa)
    { 
        return;
    }

    /*if nssa waiting timer is running,do not update for nssa range*/
    if (ospf_timer_active(&p_range->p_area->nssa_timer))
    { 
        return;
    }

    /*delete external lsa for range,if self is not translator or there is no lsa covered*/
    if ((!p_range->p_area->nssa_translator)
        || (0 == ospf_get_covered_lsa_for_nssa_range(p_range, &range_vector)))
    { 
        ospf_originate_range_lsa(p_range, TRUE);
        return;
    }    

    /*check lsa covered*/
    p_range->rtcount = 0;
    for (i = 0, p_lsa = range_vector.p_lsa[i]; 
        i < range_vector.count; 
        i++ ,p_lsa = range_vector.p_lsa[i])
    {    
        p_nssa = (struct ospf_nssa_lsa *)p_lsa->lshdr;
        id = ntohl(p_lsa->lshdr->id);
        mask = ntohl(p_nssa->mask);

        /*this range is not the best range,fulsh the range lsa*/
        if (ospf_range_match(p_range->p_area, OSPF_LS_TYPE_7, id, mask, NULL) != p_range)
        {
            continue;
        }

        /*i'm best range, update cost and matched count*/
        p_range->rtcount++;
        if (lscost < ntohl(p_nssa->metric))
        {
            lscost = ntohl(p_nssa->metric); 
        }
        ospf_nssa_lsa_translate(p_lsa);
    }

    /*now ,all matched route's inter-lsa cleared,decide if update summarylsa
       of the range*/
    p_range->cost = (ospf_ase2_metric(lscost)) ? (lscost + 1) : lscost;
    
    if (p_range->advertise)
    { 
        ospf_originate_range_lsa(p_range, p_range->rtcount ? FALSE : TRUE);
    }
    return;
}

/*decide if some nssa lsa matched to this range*/ 
u_int 
ospf_nssa_range_active(struct ospf_range *p_range)
{
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_nssa_lsa *p_nssa = NULL;
    struct ospf_lsa minlsa;
    struct ospf_lsa maxlsa;
    
    minlsa.lshdr->type = OSPF_LS_TYPE_7;
    minlsa.lshdr->id = p_range->network;
    minlsa.lshdr->adv_id = 0 ;

    maxlsa.lshdr->type = OSPF_LS_TYPE_7;
    maxlsa.lshdr->id = p_range->network | (~p_range->mask);
    maxlsa.lshdr->adv_id = OSPF_HOST_NETWORK;

    for_each_node_range(&p_range->p_area->ls_table[OSPF_LS_TYPE_7]->list, p_lsa, &minlsa, &maxlsa)    
    {
        /*ignore expired lsa*/
        if (OSPF_MAX_LSAGE <= ntohs(p_lsa->lshdr->age))
        {
            continue;
        }
        p_nssa = ospf_lsa_body(p_lsa->lshdr);

        /*must be the best one range*/
        if (p_range 
            != ospf_range_match(p_range->p_area, OSPF_LS_TYPE_7, 
                 ntohl(p_nssa->h.id) , ntohl(p_nssa->mask), NULL))
        {
            continue;
        }
        /*ignore same mask*/
        if (p_range->mask == ntohl(p_nssa->mask))
        {
            continue;
        }
        return TRUE;
    }
    return FALSE;
}

/*translate all nssa lsa Section 4.1 RFC-1587 */
void 
ospf_nssa_translator_up (struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;    
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next_lsdb = NULL;
    
    /*must be abr*/ 
    if (!p_process->abr)
    {
        return;
    }        
    
    /*check each nssa area*/
    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        if ((!p_area->is_nssa) || (!p_area->nssa_translator))
        {
            continue;
        }
        /*check each nssa lsa*/
        for_each_nssa_lsa(p_area, p_lsa, p_next_lsdb)
        {
            /*ignore local lsa*/                
            if (ntohl(p_lsa->lshdr->adv_id) != p_process->router_id) 
            {
                /*translate into external lsa*/
                ospf_nssa_lsa_translate(p_lsa);
            }
        }                           
    }
    return;
}

/*nssa translator down*/
void 
ospf_nssa_translator_down(struct ospf_area *p_area)
{
    struct ospf_lsa *p_lsa = NULL;       
    struct ospf_lsa *p_best = NULL;       
    struct ospf_lsa *p_next_lsdb = NULL;           
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_nssa_lsa *p_nssa = NULL;
    struct ospf_range *p_range = NULL;
    struct ospf_range *p_nextrange = NULL;
    struct ospf_lsvector vector;
    u_int other_lsa = FALSE;
    u_int id = 0;
    u_int mask = 0;
    u_int adv_id = 0;
    u_int i;
    
    if (!p_area->is_nssa)
    {
        return;
    }
    
    /*for each received nssa lsa in this area,try to search same nssa lsa in other area,
    if not found,delete translated external lsa*/          
    for_each_nssa_lsa(p_area, p_lsa, p_next_lsdb)
    {
        p_nssa = (struct ospf_nssa_lsa *)p_lsa->lshdr;
        /*ignore self originate lsa*/
        adv_id = ntohl(p_lsa->lshdr->adv_id);
        if (adv_id == p_process->router_id)
        {
            continue;
        }
        other_lsa = FALSE;
        id = ntohl(p_lsa->lshdr->id);
        mask = ntohl(p_nssa->mask);
         
        ospf_get_nssa_lsa_with_id(p_process, id, mask, &vector);
 
       /*decide if any local nssa exist*/
        for (i = 0 ; i < vector.count; i++)
        {
            if (vector.p_lsa[i] && (p_process->router_id == ntohl(vector.p_lsa[i]->lshdr->id)))
            {
                other_lsa = TRUE;
                break;
            }
        }
        /*loca lsa exist,do not delete external lsa--it is imported local,not translated*/
        if (other_lsa)
        {
            continue;
        }
        
         /*deicde best nssa lsa from all of nssa area*/
        p_best = ospf_best_lsa_for_translate(id, mask, &vector);
        if (p_best == p_lsa)
        {
            /*if other nssa lsa not exist,delete external lsa*/
            ospf_flush_external_lsa(p_process, id, mask);
        }
    }                 
    
    /*similar operation for range*/
    for_each_node(&p_area->range_table, p_range, p_nextrange)
    {
        if (OSPF_LS_TYPE_7 == p_range->lstype)
        {
            p_range->isdown = TRUE;
            ospf_nssa_range_down(p_range);
            p_range->isdown = FALSE;
        }
    }    
    /*do not act as translator*/
    p_area->nssa_translator = FALSE;   
    return;
}

/*decide if a router lsa is prefered then self in translator election*/
 u_int 
ospf_is_better_translator(
                struct ospf_area *p_area,
                struct ospf_lsa *p_lsa)
{
    struct ospf_process *p_process = p_area->p_process;     
    struct ospf_router_lsa *p_router = NULL;
    struct ospf_route *p_abr_route = NULL;
    u_int current = p_process->current_route;
    u_int adv_id = 0;
    u_int abr = 0;
    u_int asbr = 0;
    u_int translate = 0;
   
    p_router = (struct ospf_router_lsa *)p_lsa->lshdr;
    adv_id = ntohl(p_lsa->lshdr->id); 
    abr = ospf_router_flag_abr(p_router->flag);
    asbr = ospf_router_flag_asbr(p_router->flag); 
    translate = ospf_router_flag_translator(p_router->flag);
           
    ospf_logx(ospf_debug, "check router lsa %x,abr %d,asbr %d,translate %d, age %d", 
           adv_id, abr, asbr, translate, ntohs(p_lsa->lshdr->age));  
   
    /*translator must be abr&asbr,ignore local*/
    if (!abr || !asbr || (adv_id == p_process->router_id))
    {
        return FALSE;
    }
    
    /*ignore expired lsa*/
    if (OSPF_MAX_LSAGE <= ntohs(p_lsa->lshdr->age))
    {
        return FALSE;
    }
   
    /*abr route must exist*/
    p_abr_route = ospf_abr_lookup(p_area, adv_id);
    if ((NULL == p_abr_route) || (OSPF_PATH_INVALID == p_abr_route->path[current].type))
    {
        ospf_logx(ospf_debug, " no route to abr ");  
        return FALSE;
    }
   
    /*compare priority for nssa translator
     translator-always is prefered,it is indicated by translator bit, or
     larger router id is prefered*/
    if (translate || (adv_id > p_process->router_id))
    {
        return TRUE;
    }
    return FALSE;   
}

/*elect translator in a nssa area*/
void 
ospf_nssa_translator_elect(struct ospf_area *p_area)
{
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_next = NULL;
    struct ospf_process *p_process = p_area->p_process;     
    u_int8 new_state = TRUE;/*default is translator*/

    ospf_logx(ospf_debug, "ospf elect nssa translator, current %d", p_area->nssa_translator);  
    
    /*must be nssa area*/
    if (!p_area->is_nssa)
    { 
        return;
    }

    /*self is not abr,can not act as translator*/
    if (!p_process->abr)
    {
        new_state = FALSE;
    }/*need elect*/
    else if (!p_area->nssa_always_translate)
    {
        /*check all of route lsa in this area*/
        for_each_router_lsa(p_area, p_lsa, p_next)
        {
            /*if any learnt lsa is better than self,can not be translator*/
            if (TRUE == ospf_is_better_translator(p_area, p_lsa))
            {
                new_state = FALSE;
                break;
            }
        }                                                
    }
    /*check if state changed,if not ,do nothing*/        
    if (new_state == p_area->nssa_translator)
    {
        ospf_logx(ospf_debug, "translator not change");  
        return;
    }
    
    p_area->nssa_translator = new_state;
    
    /*change to translator true,perform translate action*/
    if (p_area->nssa_translator)
    {
        ospf_logx(ospf_debug, " self is translator");  
        ospf_nssa_translator_up(p_process);
    }
    else
    {
        ospf_logx(ospf_debug, " self is not nssa  translator,start wait timer %d", p_area->nssa_wait_time);  
        /*restart nssa timeout event if election failed*/
        ospf_stimer_start(&p_area->nssa_timer, p_area->nssa_wait_time);
    }    
    /*regenerate route lsa for this area*/ 
    ospf_router_lsa_originate(p_area);

    /*send trap*/
    ospf_trap_translator_state(p_area);
    return;
}

/*wait timeout,delete all translated aggregated lsa*/
void 
ospf_nssa_wait_timeout(struct ospf_area *p_area)
{
    struct ospf_range *p_range;
    struct ospf_range *p_nextrange;    

    /*ignored in backup card*/
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    ospf_set_context(p_area->p_process);

    /*ignored when area is non-nssa or is translator winner*/
    if (p_area->nssa_translator || !p_area->is_nssa)
    {
        return;
    }
    
    ospf_logx(ospf_debug, "nssa translator wait timer expired for area %d", p_area->id);

    /*delete all external lsa for range*/
    for_each_node(&p_area->range_table, p_range, p_nextrange)
    {
        if ((OSPF_LS_TYPE_7 == p_range->lstype) && p_range->advertise)
        {
            ospf_originate_range_lsa(p_range, TRUE);
        }
    }
    return;
}

/*check if a nssa lsa is better than self-originated instance.
Called when receive more recent lsa,or lsa maxage exceed*/ 
void 
ospf_preferred_nssa_lsa_select(
                        struct ospf_area *p_area,
                        struct ospf_nssa_lsa *p_new,
                        struct ospf_nssa_lsa *p_old)
{
    struct ospf_nssa_lsa *p_current = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_lshdr lshdr;
    u_int adv_id = 0;
    u_short age = 0;         
        
    adv_id = ntohl(p_new->h.adv_id); 
    
    /*self orignated ,ignore*/    
    if (adv_id == p_process->router_id)
    {
        return;
    }
    
    /*if body changed or age exceed, reoriginate self lsa*/ 
    age = ntohs(p_new->h.age);
    
    if ((OSPF_MAX_LSAGE <= age) 
        || (p_old && ((p_new->metric != p_old->metric)
        || (p_new->fwdaddr != p_old->fwdaddr))))
    {                                          
        /*for simple,redistribute all of routes*/
        ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
        p_process->import_update = TRUE;
        return;
    } 
    
    /*clear less priority lsa*/    
    lshdr.type = OSPF_LS_TYPE_7;
    lshdr.id = p_new->h.id;
    lshdr.adv_id = htonl(p_area->p_process->router_id);

    p_lsa = ospf_lsa_lookup(p_area->ls_table[lshdr.type], &lshdr);
    if (NULL == p_lsa)
    {
        return;
    }

    /*if body not same,do nothing*/    
    p_current = (struct ospf_nssa_lsa *)p_lsa->lshdr;
    if ((p_new->metric != p_current->metric)
       || (p_new->fwdaddr != p_current->fwdaddr))         
    {
        return;
    }
    
    /*check priority*/
    /*local translate bit set,do nothing*/
    if (ospf_option_nssa(p_current->h.option) > ospf_option_nssa(p_new->h.option))
    {
        return;                       
    }
    
    /*larger routerid prefered*/
    if (p_process->router_id > adv_id)
    {
        return;
    }
    /*delete self originated lsa*/    
    ospf_lsa_maxage_set(p_lsa);
    return;
}

void 
ospf_backbone_status_update(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;        
    struct ospf_area *p_next = NULL;        
    struct ospf_iproute default_route;

    if (!p_process->abr)
    {
        return;
    }
    
    /*router lsa flag need change*/
    ospf_lstwalkup(&p_process->area_table, ospf_router_lsa_originate);
    
    /*check for each area*/
    for_each_ospf_area(p_process, p_area, p_next)
    {
        if (p_area->is_nssa && (!p_area->nosummary))
        {
            memset(&default_route, 0, sizeof(default_route));
            if (ospf_backbone_full_nbr_exsit(p_process))
            {
                /*use newly configured metric*/
                default_route.metric = ospf_extmetric(p_area->nssa_default_cost, p_area->nssa_cost_type);
                default_route.active = p_process->abr;
            }
            else
            {
                /*use newly configured metric*/
                default_route.metric = ospf_extmetric(OSPF_METRIC_INFINITY-1, 1);
                default_route.active = p_process->abr;
            }
                 
            ospf_nssa_lsa_originate(p_area, &default_route);  
        }
    }

    return;
}

/*update type 5 lsa by asbr range */
void ospf_asbr_T5_range_down(struct ospf_asbr_range *p_range)
{
    struct ospf_lsa *p_lsa = NULL;                
    struct ospf_external_lsa *p_external = NULL;
    struct ospf_external_lsa stexternal ;
    struct ospf_lsvector range_vector;
    u_int id = 0;
    u_int mask = 0;
    u_int i = 0;
    u_int lscost = 0;
    struct ospf_iproute *p_route;
    struct ospf_iproute *p_next_route;

    memset(&stexternal, 0, sizeof(struct ospf_external_lsa));
    if(!p_range->p_process->asbr)
    {
        return;
    }
	/*import route re-update*/
    for_each_node(&p_range->p_process->import_table,p_route, p_next_route)
    {
        if(ospf_netmatch(p_range->dest, p_route->dest, p_range->mask))
        {
            if(!ospf_nssa_area_exist(p_range->p_process) || ospf_asbr_area_check(p_range->p_process))
            {
                ospf_external_lsa_originate(p_route);
                continue;
            }
        }
    }
    ospf_stimer_safe_start(&p_range->p_process->lsa_aging_timer,OSPF_MIN_LSAGE_TIME);
    /*delete external lsa for range,if self is not translator or there is no lsa covered*/
    if ((0 == ospf_get_covered_lsa_for_range_asbr_T5(p_range, &range_vector)))
    { 
        //ospf_originate_range_lsa(p_range, TRUE);
        return;
    }    

    /*check lsa covered*/
    p_range->rtcount = 0;
    for (i = 0, p_lsa = range_vector.p_lsa[i]; 
        i < range_vector.count; 
        i++ ,p_lsa = range_vector.p_lsa[i])
    {    
        p_external = (struct ospf_nssa_lsa *)p_lsa->lshdr;
        id = ntohl(p_lsa->lshdr->id);
        mask = ntohl(p_external->mask);

        if((p_range->dest == id) && (p_range->mask == mask))
        {
            p_external->h.age = htons(OSPF_MAX_LSAGE);
        }
        else
        {
            p_external->h.age = 1;
        }
        /*i'm best range, update cost and matched count*/
        p_range->rtcount++;
        if (lscost < ntohl(p_external->metric))
        {
            lscost = ntohl(p_external->metric); 
        }

    }

    ospf_stimer_safe_start(&p_range->p_process->lsa_aging_timer,OSPF_MIN_LSAGE_TIME);
    //ospf_lsa_table_timer_expired(p_process);
    return;
}
/*update type 7 lsa by asbr range */
void 
ospf_asbr_T7_range_down(
                    struct ospf_area *p_nssa_area, 
                    struct ospf_asbr_range *p_range)
{
    struct ospf_lsa *p_lsa = NULL;                
    struct ospf_nssa_lsa *p_nssa = NULL;
    struct ospf_lsvector range_vector;
    u_int id = 0;
    u_int mask = 0;
    u_int i = 0;
    u_int lscost = 0;
    struct ospf_iproute *p_route;
    struct ospf_iproute *p_next_route;

    
    if (!p_nssa_area->is_nssa)
    { 
        return;
    }

    /*if nssa waiting timer is running,do not update for nssa range*/
    if (ospf_timer_active(&p_nssa_area->nssa_timer))
    { 
        return;
    }
    /*import_table re-update*/
    for_each_node(&p_nssa_area->p_process->import_table,p_route, p_next_route)
    {
        if(ospf_netmatch(p_range->dest, p_route->dest, p_range->mask))
        {
            ospf_nssa_lsa_originate(p_nssa_area,p_route);
        }
    }


    /*delete external lsa for range,if self is not translator or there is no lsa covered*/
    if (0 == ospf_get_covered_lsa_for_nssa_range_asbr(p_range,p_nssa_area, &range_vector))
    { 
        //ospf_originate_range_lsa(p_range, TRUE);
        return;
    }    

    /*check lsa covered*/
    p_range->rtcount = 0;
    for (i = 0, p_lsa = range_vector.p_lsa[i]; 
        i < range_vector.count; 
        i++ ,p_lsa = range_vector.p_lsa[i])
    {    
        p_nssa = (struct ospf_nssa_lsa *)p_lsa->lshdr;
        id = ntohl(p_lsa->lshdr->id);
        mask = ntohl(p_nssa->mask);

        if((p_range->dest== id) && (p_range->mask == mask))
        {
            p_nssa->h.age = htons(OSPF_MAX_LSAGE);
        }
        else
        {
            p_nssa->h.age = 1;
        }
 
        /*i'm best range, update cost and matched count*/
        p_range->rtcount++;
        if (lscost < ntohl(p_nssa->metric))
        {
            lscost = ntohl(p_nssa->metric); 
        }
    }

     ospf_stimer_safe_start(&p_nssa_area->p_process->lsa_aging_timer,OSPF_MIN_LSAGE_TIME);
    //ospf_lsa_table_timer_expired(p_process);
    return;
}

/*asbr lsa build for type 5*/
void 
ospf_T5_lsa_build(struct ospf_asbr_range *p_range)
{
    struct ospf_external_lsa stexternal ;
    //if(ospf_asbr_area_check(p_range->p_process))
    {
        ospf_init_lshdr(&stexternal.h, OSPF_LS_AS_EXTERNAL, p_range->dest,p_range->p_process->router_id);

        stexternal.h.len = htons(OSPF_ASE_LSA_HLEN);
        ospf_set_option_external(stexternal.h.option);
        //stexternal.h.option = 0;
        stexternal.mask = ntohl(p_range->mask);
        stexternal.metric = p_range->cost & (~OSPF_ASE_EBIT);
        stexternal.fwdaddr = 0;//ntohl(p_range->dest);
        stexternal.h.age = htons(OSPF_MIN_LSAGE_TIME);
        ospf_local_lsa_install(&p_range->p_process->t5_lstable, &stexternal.h);
        
    }
}
/*asbr lsa build for type 7*/
void 
ospf_T7_lsa_build(struct ospf_area *p_area, struct ospf_asbr_range *p_range)
{
    struct ospf_nssa_lsa stNssa ;
    struct ospf_process *p_process = p_range->p_process;     
    u_int pbit = FALSE;

    memset(&stNssa, 0, sizeof(struct ospf_nssa_lsa));
    
    ospf_init_lshdr(&stNssa.h, OSPF_LS_TYPE_7, p_range->dest,p_process->router_id);

    stNssa.h.len = htons(OSPF_ASE_LSA_HLEN);
    stNssa.h.option = 0;
    //ospf_lsa_option_build(p_range->p_area,&stNssa.h);
    
	pbit = (ospf_asbr_area_check(p_process)) ?  TRUE : FALSE;
    if (pbit)
    {
        /*set pbit*/
        ospf_set_option_nssa(stNssa.h.option);
    }
    
    stNssa.mask = ntohl(p_range->mask);
    stNssa.fwdaddr = 0;//ntohl(p_range->dest);
    stNssa.metric = p_range->cost & (~OSPF_ASE_EBIT);
    stNssa.h.age = htons(OSPF_MIN_LSAGE_TIME);
    ospf_local_lsa_install(p_area->ls_table[OSPF_LS_TYPE_7], &stNssa.h);
   
    return;
}
/*nssa 区域判断*/
int ospf_nssa_area_exist(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL ;
    struct ospf_area *p_areaNext = NULL ;
    for_each_node(&p_process->area_table, p_area, p_areaNext)
    {
        if(p_area->is_nssa)
        {
            return TRUE;
        }
    }
    return FALSE;
}
/*普通区域判断*/
int ospf_normal_area_check(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next = NULL;
    for_each_node(&p_process->area_table, p_area, p_next)
    {
        if((!p_area->is_stub) && (!p_area->is_nssa))
        {
            return TRUE;
        }
    }
    return FALSE;
}
/*多区域判断*/
int ospf_asbr_area_check(struct ospf_process *p_process)
{
    int normal = ospf_normal_area_check(p_process);
    int nssa = ospf_nssa_area_exist(p_process);
    int areacnt = ospf_lstcnt(&p_process->area_table);

    if(areacnt >= 2)
    {
        if(nssa && normal)
        {
            return TRUE;
        }
    }
    return FALSE;
}
 
