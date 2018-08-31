#include "ospf.h"

#ifdef OSPF_FRR

/*simple implement of frr*/



void 
ospf_backup_old_route_clear(
              struct ospf_process *p_process,
              struct ospf_lst *p_list)
{
    struct ospf_backup_route *p_route = NULL;
    struct ospf_backup_route *p_next_route = NULL;
    u_int current = p_process->backup_current_route;
    u_int old = p_process->backup_old_route;

    for_each_node(p_list, p_route, p_next_route)
    {
        p_route->path[old].nexthop = 0;
        p_route->path[old].ifunit = 0;
        
        /*if new path is null, delete it*/
        if (0 == p_route->path[current].nexthop)
        {
            ospf_lstdel_free(p_list, p_route, OSPF_MBACKROUTE);
        }        
    }
    return;
}

#define ospf_save_backup_old_routes(ins) do{(ins)->backup_current_route = (ins)->backup_old_route;(ins)->backup_old_route = 1 - (ins)->backup_current_route ;}while(0)

void
ospf_frr_disable_process(struct ospf_process *p_process)
{
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;
    struct ospf_backup_route *p_back_route = NULL;
    struct ospf_backup_route *p_next_back_route = NULL;

    /*flush abr asbr,spf*/
    for_each_ospf_area(p_process, p_area, p_next_area)
    {
        ospf_backup_route_table_flush(&p_area->backup_abr_table);
        ospf_backup_route_table_flush(&p_area->backup_asbr_table);
        ospf_backup_spf_table_flush(p_area);
    } 

    for_each_node(&p_process->backup_route_table, p_back_route, p_next_back_route)
    {
        p_back_route->path[p_process->backup_current_route].nexthop = 0;
    }
    /*export route*/
    p_process->backup_wait_export = TRUE;
    p_process->backup_last_export_network = OSPF_HOST_NETWORK;

    ospf_backup_route_export(p_process);
    return;
}

/*timer expired for frr calculate*/
void
ospf_frr_timer_expired(struct ospf_process *p_process)
{
    struct ospf_route *p_route = NULL;
    struct ospf_route *p_next = NULL;

    ospf_logx(ospf_debug_frr, "start backup calculate");

    /*if backup route is waiting, calculate later*/
    if (TRUE == p_process->backup_wait_export)
    {
        p_process->backup_spf_called_count++;
        ospf_stimer_start(&p_process->frr_timer, OSPF_FRR_INTERVAL);
        return;
    }
    
    ospf_save_backup_old_routes(p_process);
          
    if (FALSE == p_process->frr_enable)
    {
        ospf_frr_disable_process(p_process);
        return;
    }   

    /*increase spf running times*/
    p_process->backup_spf_running_count++;
    
    /*calculate backup spf table for all areas*/
    ospf_lstwalkup(&p_process->area_table, ospf_area_backup_spf_calculate);

    /*calculate back route table for all network route*/
    for_each_node(&p_process->route_table, p_route, p_next)
    {
        ospf_logx(ospf_debug_frr, "calculate backup for network route :%x/%x", p_route->dest, p_route->mask);

        ospf_backup_route_calculate(p_process, p_route);
    }

    p_process->backup_wait_export = TRUE;
    p_process->backup_last_export_network = OSPF_HOST_NETWORK;

    ospf_backup_route_export(p_process);
    ospf_logx(ospf_debug_frr, "backup calculate finished");

    return;
}

/*calculate backup spf table for an area*/
void
ospf_area_backup_spf_calculate(struct ospf_area *p_area)
{
    struct ospf_spf_vertex *p_vertex = NULL;
    struct ospf_spf_vertex *p_next = NULL;
    struct ospf_route *p_route = NULL;
    struct ospf_route *p_next_route = NULL;
    u_int8 str[32];

    ospf_logx(ospf_debug_frr, "calculate backup spf for area %s", ospf_printf_area(str, p_area->id));

    ospf_backup_route_table_flush(&p_area->backup_abr_table);
    ospf_backup_route_table_flush(&p_area->backup_asbr_table);
    ospf_backup_spf_table_flush(p_area);
    /*scan for all spf vertex,calculate back path for it*/
    for_each_node(&p_area->spf_table, p_vertex, p_next)
    {
        ospf_backup_spf_calculate(p_area, p_vertex);       
    }
    
    for_each_node(&p_area->abr_table, p_route, p_next_route)
    {
        ospf_logx(ospf_debug_frr, "calculate backup for abr route :%x/%x", p_route->dest, p_route->mask);
        ospf_backup_route_calculate(p_area->p_process, p_route);
    }
    
    /*calculate asbr backup route from type4 lsa*/
    for_each_node(&p_area->asbr_table, p_route, p_next_route)
    {
        ospf_logx(ospf_debug_frr, "calculate backup for asbr route :%x/%x", p_route->dest, p_route->mask);
        ospf_backup_route_calculate(p_area->p_process, p_route);
    }
    return;
}

u_int32 
ospf_backup_verify_connect_vertex(
              struct ospf_area *p_area,
              struct ospf_spf_vertex *p_vertex,
              uint32_t *if_unit,
              uint32_t *nexthop)
{
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_router_lsa *p_router = NULL;
    struct ospf_router_link *p_router_link = NULL;
    struct ospf_network_lsa *p_network = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    uint32_t *p_link = NULL;
    struct ospf_lsvector vector;
    u_int32 i = 0;
    
    p_router = ospf_lsa_body(p_vertex->p_lsa->lshdr);
    for_each_router_link(p_router, p_router_link)
    {
        if (OSPF_RTRLINK_TRANSIT != p_router_link->type)        
        { 
            continue;
        }
        ospf_lsa_lookup_by_id(&p_area->ls_table[OSPF_LS_NETWORK]->list, OSPF_LS_NETWORK, ntohl(p_router_link->id), 0, &vector);
        for (i = 0; i < vector.count; i++)
        {
            p_network = ospf_lsa_body(vector.p_lsa[i]->lshdr);
                                                                   
            for_each_network_link(p_network, p_link)
            {
                if (ntohl(*p_link) != p_process->router_id)
                {
                    continue;
                }
                p_if = ospf_if_lookup_by_network(p_process, ntohl(p_router_link->id));
                if (p_if)
                {
                    *if_unit = p_if->ifnet_uint;
                    for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
                    {
                        if (p_nbr->id == p_vertex->id)
                        {
                            *nexthop = p_nbr->addr;         
                            break;
                        }
                    }
                    return TRUE;
                }                                                                            
            }                    
        }                  
    } 
    return FALSE;
}
/*calculate backup path for a special spf node
        root-----backup
         |                  \
         |                   \
         nbr---------dest
 */
void
ospf_backup_spf_calculate(
                       struct ospf_area *p_area,
                       struct ospf_spf_vertex *p_dest)
{
    struct ospf_spf_vertex *p_vertex = NULL;
    struct ospf_spf_vertex *p_next = NULL;
    struct ospf_spf_vertex *p_parent = NULL;
    struct ospf_spf_vertex *p_current_nbr = NULL;
    struct ospf_spf_vertex *p_root = NULL;    
    struct ospf_backup_spf_vertex backup;
    struct ospf_backup_spf_vertex backup_best;
    struct ospf_process *p_process = p_area->p_process;
    u_int cost_nbr_dest = 0;
    u_int candidate = 0;
    u_int8 dstr[32];
    u_int i = 0;
    u_int if_uint = 0;
    u_int nexthop = 0;


    ospf_logx(ospf_debug_frr, "calculate backup spf %s,type %d", ospf_inet_ntoa(dstr, p_dest->id), p_dest->type);
    
    /*ignore root node*/
    if ((OSPF_LS_ROUTER == p_dest->type) && (p_process->router_id == p_dest->id))
    {
        ospf_logx(ospf_debug_frr, "ignore root node");
        return;
    }

   /*ignore network connected to root*/
    if (OSPF_LS_ROUTER != p_dest->type)
    {
        p_parent = p_dest->parent[0].p_node;
        if ((OSPF_LS_ROUTER == p_parent->type) && (p_parent->id == p_process->router_id))
        {
            ospf_logx(ospf_debug_frr, "ignore network node connected to root");
            return;
        }
    }
    
    /*for safe*/
    if (NULL == p_dest->p_nexthop)
    {
        return;
    }
    
    /*ignore node with multiple nexthops*/
    if (1 < p_dest->p_nexthop->count)
    {
        ospf_logx(ospf_debug_frr, "ignore node with ecmp");
        return;
    }

    memset(&backup, 0, sizeof(backup));
    memset(&backup_best, 0, sizeof(backup_best)); 
    backup_best.cost = 0xffffffff;
    backup_best.id = p_dest->id;
    backup_best.type = p_dest->type;
    /*decide directly connected node for this dest*/
    for_each_node(&p_area->spf_table, p_vertex, p_next)
    {
         /*ignore not connected*/       
        p_parent = p_vertex->parent[0].p_node;
        if ((NULL == p_parent)
            || (OSPF_LS_ROUTER != p_parent->type) 
            || (p_parent->id != p_process->router_id))
        {
            continue;
        }
        /*get expected one with same nexthop if*/
        if (p_vertex->p_nexthop->gateway[0].if_uint 
            != p_dest->p_nexthop->gateway[0].if_uint)
        {
            continue;
        }
        p_current_nbr = p_vertex;
    }
    if (NULL == p_current_nbr)
    {
        return;
    }
    
    /*calculate cost from current nbr to dest:dest's cost-nbr'cost*/
    cost_nbr_dest = p_dest->cost - p_current_nbr->cost;
    
    ospf_logx(ospf_debug_frr, "current nexthop %s,type %d", ospf_inet_ntoa(dstr, p_current_nbr->id), p_current_nbr->type);

    /*calculate path from other connected node to this dest*/
    for_each_node(&p_area->spf_table, p_vertex, p_next)
    {
        /*ignore dest self*/
        if (p_vertex == p_dest)
        {
            continue;
        }

        /*ignore root node*/
        if ((OSPF_LS_ROUTER == p_vertex->type) 
            && (p_process->router_id == p_vertex->id))
        {
            continue;
        }
        /*consider connected router,network,or connected by network*/

        /*test:ignore network node*/
        if (OSPF_LS_ROUTER != p_vertex->type)
        {
            continue;
        }

        candidate = ospf_backup_verify_connect_vertex(p_area, p_vertex,&if_uint, &nexthop);

        if (FALSE == candidate)
        {
            continue;
        }

    
        /*ignore node with same nexthop interface*/
        if ((if_uint == p_dest->p_nexthop->gateway[0].if_uint)
            && (nexthop == p_dest->p_nexthop->gateway[0].addr))
        {
            continue;
        }

        /*ignore current nexthop*/
        if (p_vertex == p_current_nbr)
        {
            continue;
        }
        
        ospf_logx(ospf_debug_frr, "check candidate nexthop %s,type %d", ospf_inet_ntoa(dstr, p_vertex->id), p_vertex->type);

        /*calculate cost from nbr to dest*/
        backup.cost = ospf_part_spf_calculate(p_area, p_vertex, p_dest);

        /*calculate cost from nbr to root,root must be the first one*/
        p_root = ospf_spf_lookup(&p_area->spf_table, p_process->router_id, OSPF_LS_ROUTER);
        backup.cost_root = ospf_part_spf_calculate(p_area, p_vertex, p_root);

        /*calculate cost from nbr to node's current nbr*/
        backup.cost_nexthop = ospf_part_spf_calculate(p_area, p_vertex, p_current_nbr);

        ospf_logx(ospf_debug_frr, "get cost %d from backup to dest", backup.cost);
        ospf_logx(ospf_debug_frr, "get cost %d from backup to root", backup.cost_root);
        ospf_logx(ospf_debug_frr, "get cost %d from backup to nexthop", backup.cost_nexthop);        
        /*ignore path with loop*/ 
        if (backup.cost >= (backup.cost_root + p_dest->cost))
        {
            ospf_logx(ospf_debug_frr, "backup nexthop has loop,ignore it");
            continue;
        }
        /*get path with min cost*/ 
        if (backup.cost < backup_best.cost)
        {
            backup_best.cost = backup.cost;
            backup_best.cost_root = backup.cost_root;
            backup_best.cost_nexthop = backup.cost_nexthop;
            backup_best.ifunit = if_uint;
            backup_best.nexthop = nexthop;
            backup_best.cost_total = backup_best.cost + p_vertex->cost;
            ospf_logx(ospf_debug_frr, "set backup nexthop active");
        }
    }

    /*create backup node if exist*/
    if (0xffffffff != backup_best.cost)
    {
        backup_best.main_nhop = p_dest->p_nexthop->gateway[0].addr;
        ospf_logx(ospf_debug_frr, "create backup path for dest");
        ospf_backup_spf_vertex_create(p_area, &backup_best);
    }
    else
    {
        ospf_logx(ospf_debug_frr, "no backup path for dest");
    }
    return;
}

int 
ospf_backup_spf_vertex_cmp(
          struct ospf_backup_spf_vertex *p1,
          struct ospf_backup_spf_vertex *p2)
{
    OSPF_KEY_CMP(p1, p2, id);
    OSPF_KEY_CMP(p1, p2, type);
    return 0; 
}  

void 
ospf_backup_spf_table_flush(struct ospf_area *p_area)
{
    struct ospf_backup_spf_vertex *p_vertex = NULL;
    struct ospf_backup_spf_vertex *p_next = NULL;   
    
    /*clear spf table and release spf node*/ 
    for_each_node(&p_area->backup_spf_table, p_vertex, p_next)
    {
        ospf_lstdel(&p_area->backup_spf_table, p_vertex);
        ospf_mfree(p_vertex, OSPF_MBACKSPF);
    }
    return;
}

void
ospf_backup_spf_vertex_create(
           struct ospf_area *p_area,
           struct ospf_backup_spf_vertex *p_backup)
{
    struct ospf_backup_spf_vertex *p_vertex = ospf_malloc(sizeof(*p_vertex), OSPF_MBACKSPF);
    if (p_vertex)
    {
        memcpy(p_vertex, p_backup, sizeof(struct ospf_backup_spf_vertex));
        ospf_lstadd(&p_area->backup_spf_table, p_vertex);
    }
    return;
}

/*calculate part spf path from special source to special dest*/
u_int
ospf_part_spf_calculate(
                       struct ospf_area *p_area,
                       struct ospf_spf_vertex *p_src,
                       struct ospf_spf_vertex *p_dest)
{
    struct ospf_spf_vertex *p_vertex = NULL;
    struct ospf_spf_vertex *p_next = NULL;
    struct ospf_lst spf_list;
    struct ospf_lst candidate_list;
    struct ospf_lshdr lshdr;
    u_int cost = 0xffffffff;

    ospf_lstinit(&spf_list, ospf_spf_lookup_cmp);
    ospf_lstinit2(&candidate_list, ospf_spf_cost_lookup_cmp, mbroffset(struct ospf_spf_vertex, cost_node));

    /*start spf calculate from source*/
    p_vertex = ospf_malloc2(OSPF_MSPF);
    if (NULL == p_vertex)
    {
        return cost;
    }
    
    p_vertex->id = p_src->id;
    p_vertex->type = p_src->type;    
    p_vertex->p_area = p_area;
    p_vertex->cost = 0;
    /*add to both spf table and candidate table*/
    ospf_lstadd(&spf_list, p_vertex);
    ospf_lstadd(&candidate_list, p_vertex);

    while (1)
    {
         /*get nearest node,if candidate is empty, spf finished*/
        p_vertex = ospf_lstfirst(&candidate_list);
        if (NULL == p_vertex)
        {
            ospf_logx(ospf_debug_frr, "spf finished");        
            break;
        }

        /*if expected node got,stop calculate*/
        if ((p_vertex->id == p_dest->id) && (p_vertex->type == p_dest->type))
        {
            cost = p_vertex->cost;
            break;
        }
        /*remove from cost-candidate table*/
        ospf_lstdel(&candidate_list, p_vertex);

        /*if lsa is empty,search again*/
        if (NULL == p_vertex->p_lsa) 
        {
            lshdr.type = p_vertex->type;
            lshdr.id = htonl(p_vertex->id);
            /*do not care about router id for network lsa*/
            lshdr.adv_id = (OSPF_LS_ROUTER == p_vertex->type) ? htonl(p_vertex->id) : 0;
 
            p_vertex->p_lsa = ospf_lsa_lookup(p_area->ls_table[lshdr.type], &lshdr);
        }
        if ((NULL != p_vertex->p_lsa)
            && (OSPF_MAX_LSAGE > ntohs(p_vertex->p_lsa->lshdr->age)))
        { 
            /*lsa type must be router or network*/
            if (OSPF_LS_ROUTER == p_vertex->p_lsa->lshdr->type)
            {
                ospf_backup_spf_examine_router_vertex(&spf_list, &candidate_list, p_vertex);
            }
            else
            {
                ospf_backup_spf_examine_network_vertex(&spf_list, &candidate_list, p_vertex); 
            }
        }
    }

    /*clear spf list*/
    /*delete node in sort table, do not free memory*/
    for_each_node(&candidate_list, p_vertex, p_next)
    {
        ospf_lstdel(&candidate_list, p_vertex);
    }
    /*clear spf table and release spf node*/ 
    for_each_node(&spf_list, p_vertex, p_next)
    {
        ospf_lstdel(&spf_list, p_vertex);
        ospf_spf_node_delete(p_vertex);
    }
    return cost;
}

void 
ospf_backup_spf_examine_network_vertex (
             struct ospf_lst *p_spf_list,
             struct ospf_lst *p_candidate_list,
             struct ospf_spf_vertex *p_vertex)
{
    struct ospf_network_lsa *p_network = ospf_lsa_body(p_vertex->p_lsa->lshdr);
    u_int *p_link = NULL;
    u_int8 attachstr[20];

    /* section 16.1, item (2)(d), third bullet item, page (152) */   
    for_each_network_link(p_network, p_link)
    {
        ospf_logx(ospf_debug_frr, "check attached router %s", ospf_inet_ntoa(attachstr, *p_link));

        ospf_backup_spf_candidate_node_update(p_spf_list, p_candidate_list, p_vertex, ntohl(*p_link), OSPF_LS_ROUTER, 0, 0);
    }    
    return;
}

void 
ospf_backup_spf_examine_router_vertex(
             struct ospf_lst *p_spf_list,
             struct ospf_lst *p_candidate_list,
             struct ospf_spf_vertex *p_vertex)
{
    struct ospf_router_link *p_link = NULL;
    struct ospf_area *p_area = p_vertex->p_area;
    struct ospf_router_lsa *p_router = ospf_lsa_body(p_vertex->p_lsa->lshdr);
    u_int8 type = 0;
    u_int8 lid[20];
    u_int8 ldata[20];
    
    /*virtual link up*/    
    if (ospf_router_flag_vlink(p_router->flag))
    {
        p_area->transit = TRUE; 
    }
    
    /*check each link in this lsa*/
    for_each_router_link(p_router, p_link)
    {
        if (ospf_debug_spf)
        {
            ospf_inet_ntoa(lid, p_link->id);
            ospf_inet_ntoa(ldata, p_link->data);
            ospf_logx(ospf_debug_frr, "ospf_backup_spf check link %d,id %s,data %s", p_link->type, lid, ldata);
        }
        
        /* skip stub nets for now, they will be added later */                  
        /* section 16.1, item (2)(a) page (151) */
        if (OSPF_RTRLINK_STUB == p_link->type)        
        {
            ospf_logx(ospf_debug_frr, "ospf_backup_spf skip stub link currently");
            continue; 
        }
        
        /* section 16.1, item (2)(d), third bullet item, page (152) */
        /*this is new candidate node*/
        type = (OSPF_RTRLINK_TRANSIT == p_link->type) ? OSPF_LS_NETWORK : OSPF_LS_ROUTER;
        ospf_backup_spf_candidate_node_update(p_spf_list, p_candidate_list, p_vertex, ntohl(p_link->id), type, ntohs(p_link->tos0_metric), ntohl(p_link->data));
    }
    return;
}

void 
ospf_backup_spf_candidate_node_update(
                    struct ospf_lst *p_spf_list,
                    struct ospf_lst *p_candidate_list,
                    struct ospf_spf_vertex *p_parent,
                    u_int id,
                    u_int type,
                    u_int link_cost,
                    u_int link_data)
{
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_spf_vertex *p_child = NULL;
    struct ospf_area *p_area = p_parent->p_area;
    struct ospf_lsvector vector;
    u_int cost = (0xFFFF == link_cost) ? OSPF_METRIC_INFINITY : (p_parent->cost + link_cost); 
    u_int i = 0;
    
    ospf_logx(ospf_debug_frr, "update candidate,type=%d,id=%x,parent: type %d/id %x/link_data %x",
        type, id, p_parent->type, p_parent->id, link_data);
    
     /* section 16.1, item (2)(c) page (152) link node already in spf tree,do nothing*/
    p_child = ospf_spf_lookup(p_spf_list, id, type);
    if ((NULL != p_child) && (p_child->cost < cost))
    {
        ospf_logx(ospf_debug_frr, "lsa in spf tree or has better cost");  
        return;
    }

    /* section 16.1, item (2)(b) page (152) */
    if (NULL != p_child)
    {
        p_lsa = p_child->p_lsa;
    }
    
    if (NULL == p_lsa)
    {
        ospf_lsa_lookup_by_id(&p_area->ls_table[type]->list, type, id, 0, &vector);
    }
    else
    {
        vector.count = 1;
        vector.p_lsa[0] = p_lsa;
    }

    for (i = 0; i < vector.count ; i++)
    {
        if (vector.p_lsa[i])
        {
            ospf_lsa_age_update(vector.p_lsa[i]);
            if (OSPF_MAX_LSAGE <= ntohs(vector.p_lsa[i]->lshdr->age))
            {
                 ospf_logx(ospf_debug_frr, "lsa has max age");        
                 continue;
            }
   
            if (NULL == ospf_lsa_link_lookup(vector.p_lsa[i], p_parent))
            {
                 ospf_logx(ospf_debug_frr, "lsa has no back link");
                 continue;
            }
            p_lsa = vector.p_lsa[i];
            break;
        }
    }
    
    if (NULL == p_lsa)
    {
        ospf_logx(ospf_debug_frr, "lsa not found");        
        return;
    }

    if (NULL == p_child)
    {    
        ospf_logx(ospf_debug_frr, "create candidate ");
        
        p_child = ospf_malloc2(OSPF_MSPF);
        if (NULL == p_child)
        {
            return;
        }
        p_child->id = id;
        p_child->type = type;    
        p_child->p_area = p_area;
        p_child->cost = cost;
        /*add to both spf table and candidate table*/
        ospf_lstadd(p_spf_list, p_child);
        ospf_lstadd(p_candidate_list, p_child);
        p_child->p_lsa = p_lsa;
        ospf_spf_parent_set(p_parent, link_data, p_child); 
//	ospf_logx(ospf_debug_frr,"%s %d  *****p_child :id 0x%x, type %d,p_area 0x%x,cost %d,p_lsa 0x%x******\n", __FUNCTION__,__LINE__,
//		p_child->id,p_child->type,p_child->p_area,p_child->cost,p_child->p_lsa);
        return;
    }

    /*compare cost*/
    /* section 16.1, item (2)(d), first bullet item, page (152) */
    if (cost > p_child->cost)                                
    {
        ospf_logx(ospf_debug_frr, "new cost greater,ignore it");
        return;
    } 
    
    /* section 16.1, item (2)(d), second bullet item, page (152),merge*/
    if (cost == p_child->cost)                        
    {
        ospf_logx(ospf_debug_frr, "new cost equal to exist one,merge it"); 
        ospf_spf_parent_set(p_parent, link_data, p_child);
//	ospf_logx(ospf_debug_frr,"%s %d  *****p_parent->id 0x%x, link_data 0x%x,p_child 0x%x******\n", __FUNCTION__,__LINE__,p_parent->id, link_data,p_child);
        return;
    }

    /*replace*/
    /* section 16.1, item (2)(d), third bullet item, page (152) */
    ospf_logx(ospf_debug_frr, "new cost less than exist one,replace it");

    /*remove from cost table*/
    ospf_lstdel(p_candidate_list, p_child);

    p_child->p_lsa = p_lsa;
    p_child->cost = cost;

    /*clear current parent nodes*/    
    memset(p_child->parent, 0, sizeof(p_child->parent));

    /*insert new parent*/
    ospf_spf_parent_set(p_parent, link_data, p_child);
 //    ospf_logx(ospf_debug_frr,"%s %d  *****p_parent->id 0x%x, link_data 0x%x,p_child 0x%x******\n", __FUNCTION__,__LINE__,p_parent->id, link_data,p_child);

    /*add back to cost table*/
    ospf_lstadd(p_candidate_list, p_child);
    return;
}

int 
ospf_backup_route_cmp(
        struct ospf_backup_route *p1,
        struct ospf_backup_route *p2)
{
    OSPF_KEY_CMP(p1, p2, type);
    OSPF_KEY_CMP(p1, p2, dest);
    /*mask:only for network route*/
    if (OSPF_ROUTE_NETWORK == p1->type)
    {
        OSPF_KEY_CMP(p1, p2, mask);
    }
    OSPF_KEY_CMP(p1, p2, nexthop);
    return 0; 
}                                

struct ospf_backup_route*
ospf_backup_route_add(
                   struct ospf_process *p_process,
                   struct ospf_lst *p_table,
                   u_int type,
                   u_int dest,
                   u_int mask, 
                   u_int cost,
                   u_int ifunit,
                   u_int nexthop,
                   u_int main_nhop)
{
    struct ospf_backup_route *p_route = NULL;
    struct ospf_backup_route search_route;
    u_int current = p_process->backup_current_route;
    
    search_route.type = type;
    search_route.dest = dest;
    search_route.mask = mask;
    search_route.nexthop = main_nhop;
    p_route = ospf_lstlookup(p_table, &search_route);
    if (NULL == p_route)
    {
        p_route = ospf_malloc(sizeof(struct ospf_backup_route), OSPF_MBACKROUTE);
        if (NULL == p_route)
        {
            return NULL;
        }
        p_route->type = type;
        p_route->dest = dest;
        p_route->mask = mask;
        p_route->path[current].cost = cost;
        p_route->path[current].ifunit = ifunit;
        p_route->path[current].nexthop = nexthop;
        p_route->nexthop = main_nhop;
        ospf_lstadd(p_table, p_route);
        ospf_logx(ospf_debug_frr, "backup route add type%d, %x/%x,cost%d, nexthop%x", type, dest, mask, cost, nexthop);
        return p_route;
    }

    /*route exist, but path invalid,add path*/
    if (0 == p_route->path[current].nexthop)
    {
       /*add path*/
        p_route->path[current].cost = cost;
        p_route->path[current].ifunit = ifunit;
        p_route->path[current].nexthop = nexthop;
        p_route->nexthop = main_nhop;
        ospf_logx(ospf_debug_frr, "backup route update type%d, %x/%x,cost%d, nexthop%x", type, dest, mask, cost, nexthop);
    }
    return p_route;
}

struct ospf_backup_route*
ospf_backup_abr_lookup(
         struct ospf_area *p_area, 
         u_int abr)
{
    struct ospf_backup_route search;
    
    search.type = OSPF_ROUTE_ABR;
    search.dest = abr;
    return ospf_lstlookup(&p_area->backup_abr_table, &search);
}

struct ospf_backup_route*
ospf_backup_asbr_lookup(
         struct ospf_area *p_area, 
         u_int asbr)
{
    struct ospf_backup_route search;
    
    search.type = OSPF_ROUTE_ASBR;
    search.dest = asbr;
    return ospf_lstlookup(&p_area->backup_asbr_table, &search);
}

/*clear all backup path before backup calculate*/
void
ospf_backup_route_table_flush(struct ospf_lst *p_route_table)
{
    struct ospf_backup_route *p_route = NULL;
    struct ospf_backup_route *p_next = NULL;

    for_each_node(p_route_table, p_route, p_next)
    {
        ospf_lstdel(p_route_table, p_route);
        ospf_mfree(p_route, OSPF_MBACKROUTE);
    }
    return;
}


void ospf_ecmp_backup_router_add(
            struct ospf_process *pstProcess,
            struct ospf_route *pstRoute)
{
    struct ospf_area *p_area = NULL;
    u_int uiCurrent = pstProcess->current_route;
    u_short usNHopCount = 0, i = 0, j = 0;
    u_int uiDest = 0, uiMask = 0, uiCost = 0,uiMaxIfIndex = 0;
    struct ospf_nexthop_info *pstMainHop = NULL;
    struct ospf_nexthop_info *pstNextHop = NULL;
    struct ospf_path *pstCurPath = &pstRoute->path[uiCurrent];

    usNHopCount = pstCurPath->p_nexthop->count;
    if(usNHopCount <= 1)
    {
        return;
    }
    
    uiDest = pstRoute->dest;
    uiMask = pstRoute->mask;
    uiCost = pstCurPath->cost;

    /*�ȼ�·����ifindex�ϴ������ѡΪ����·��*/
    for (i = 0; i < usNHopCount; i++)
    {
        pstMainHop = &pstCurPath->p_nexthop->gateway[i];
        for (j = 0; j < usNHopCount; j++)
        {
            if(j == i)
            {
                continue;
            }
            if(uiMaxIfIndex < pstCurPath->p_nexthop->gateway[j].if_uint)
            {
                pstNextHop = &pstCurPath->p_nexthop->gateway[j];
            }
        }
        ospf_logx(ospf_debug_frr,"ecmp backup route add");
        ospf_logx(ospf_debug_frr,"dest:%x/%x,cost:%d,nexthop:0x%x mainhop:0x%x",
            uiDest,uiMask,uiCost,pstNextHop->addr,pstMainHop->addr);
        ospf_backup_route_add(pstProcess, &pstProcess->backup_route_table, 
            OSPF_ROUTE_NETWORK, uiDest, uiMask, uiCost, pstNextHop->if_uint,
            pstNextHop->addr, pstMainHop->addr);
    }
}


/*calculate backup path for an intra area route*/
void
ospf_backup_intra_route_calculate(
             struct ospf_process *p_process,
             struct ospf_route *p_route)
{
    struct ospf_backup_spf_vertex *p_vertex = NULL;
    struct ospf_backup_spf_vertex *p_next_vertex = NULL;
    struct ospf_spf_vertex *p_spf = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_router_lsa *p_router = NULL;
    struct ospf_router_link *p_link = NULL;
    struct ospf_network_lsa *p_network = NULL;
    u_int dest = 0;
    u_int mask = 0;
    u_int cost = 0;
    u_int current = p_process->current_route;
    u_int main_nhop = 0;

    ospf_logx(ospf_debug_frr, "backup intra route calulate");

    /*decide area of route,backup path must be in the same path*/
    p_area = p_route->path[current].p_area;
    if (NULL == p_area)
    {
        ospf_logx(ospf_debug_frr, "area is NULL, ignore");
        return;
    }

    /*scan for all backup spf node in this area*/
    for_each_node(&p_area->backup_spf_table, p_vertex, p_next_vertex)
    {
        /*get matched spf node*/
        p_spf = ospf_spf_lookup(&p_area->spf_table, p_vertex->id, p_vertex->type);
        if ((NULL == p_spf) || (NULL == p_spf->p_lsa))
        {
            continue;
        }

        /*check all link of lsa,if dest&mask is matched to route,create backup route*/
        if (OSPF_LS_ROUTER == p_vertex->type)
        {
            p_router = ospf_lsa_body(p_spf->p_lsa->lshdr); 
            if (ospf_router_flag_abr(p_router->flag) && (p_process->router_id != p_vertex->id))                
            {
                main_nhop = p_vertex->main_nhop;
                ospf_backup_route_add(p_process, &p_area->backup_abr_table, OSPF_ROUTE_ABR, p_vertex->id, OSPF_HOST_MASK, p_vertex->cost_total, p_vertex->ifunit, p_vertex->nexthop, main_nhop);
            }            

            if (ospf_router_flag_asbr(p_router->flag) && (p_process->router_id != p_vertex->id) && (p_area->is_stub!= TRUE))                
            {
                main_nhop = p_vertex->main_nhop;
                ospf_backup_route_add(p_process, &p_area->backup_asbr_table, OSPF_ROUTE_ASBR, p_vertex->id, OSPF_HOST_MASK, p_vertex->cost_total, p_vertex->ifunit, p_vertex->nexthop, main_nhop);                            
            }
            /*stub link*/
            for_each_router_link(p_router, p_link)
            {
                 /*only consider stub link*/
                if (OSPF_RTRLINK_STUB == p_link->type)
                {
                    cost = p_vertex->cost + ntohs(p_link->tos0_metric);
                    mask = ntohl(p_link->data);
                    dest = ntohl(p_link->id) & mask;
                    if ((dest == p_route->dest) && (mask == p_route->mask))
                    {
                        main_nhop = p_vertex->main_nhop;
                        ospf_backup_route_add(p_process, &p_process->backup_route_table, OSPF_ROUTE_NETWORK, dest, mask, cost, p_vertex->ifunit, p_vertex->nexthop, main_nhop);
                    }
                }
            }
        }
        else 
        {
            p_network = ospf_lsa_body(p_spf->p_lsa->lshdr);
            mask = ntohl(p_network->mask);            
            dest = ntohl(p_network->h.id) & mask;
            if ((dest == p_route->dest) && (mask == p_route->mask))
            {
                main_nhop = p_vertex->main_nhop;
                ospf_backup_route_add(p_process, &p_process->backup_route_table, OSPF_ROUTE_NETWORK, dest, mask, p_vertex->cost_total, p_vertex->ifunit, p_vertex->nexthop, main_nhop);
            }
        }
    }
    return;
}

/*calculate backup route for inter-area route,backup path must belong to the same area*/
void
ospf_backup_inter_route_calculate(
                             struct ospf_process *p_process,
                             struct ospf_route *p_route)
{
    struct ospf_area *p_area = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_nextlsa = NULL;
    struct ospf_summary_lsa *p_summary = NULL;
    struct ospf_backup_route *p_abr = NULL;
    struct ospf_backup_path *p_path = NULL;
    struct ospf_nexthop_info *p_nexthop_info = NULL;
    u_int dest;
    u_int mask;
    u_int cost;
    u_int abr;
    u_int current = p_process->current_route;
    u_int main_nhop = 0;
    
    /*get backup route's area*/
    p_area = p_route->path[current].p_area;
    if (NULL == p_area)
    {
        return;
    }
    p_nexthop_info = &p_route->path[current].p_nexthop->gateway[0];
    /*scan for all summary network lsa in this area,calculate backup path*/
    if (OSPF_ROUTE_NETWORK == p_route->type)
    {
        for_each_area_lsa(p_area, OSPF_LS_SUMMARY_NETWORK, p_lsa, p_nextlsa)
        {            
            p_summary = ospf_lsa_body(p_lsa->lshdr);
            
            mask = ntohl(p_summary->mask);
            dest = ntohl(p_summary->h.id) & mask;
            cost = ntohl(p_summary->metric) & OSPF_METRIC_INFINITY;
            abr = ntohl(p_summary->h.adv_id);
   
            /*network must same*/
            if ((mask != p_route->mask) || (dest != p_route->dest))
            {
                continue;
            }
   
            /*ignore invalid lsa*/
            if ((ospf_invalid_metric(cost)) 
             || (OSPF_MAX_LSAGE <= ntohs(p_summary->h.age)) 
             || (abr == p_process->router_id))     
            {
                continue;
            }
   
            /*get backup route for abr*/
            p_abr = ospf_backup_abr_lookup(p_area, abr);
            if (NULL == p_abr)
            {
                continue;
            }
            p_path = &p_abr->path[p_process->backup_current_route];
            if ((p_path->ifunit == p_nexthop_info->if_uint)
                && (p_path->nexthop == p_nexthop_info->addr))
            {
                continue;
            }
            /*create backup route from backup abr,only one nexthop need*/
            main_nhop = p_nexthop_info->addr;
            ospf_backup_route_add(p_process, &p_process->backup_route_table, OSPF_ROUTE_NETWORK,dest, mask, cost+p_path->cost, p_path->ifunit, p_path->nexthop, main_nhop);
            break;
        } 
    }
    
    if (OSPF_ROUTE_ASBR == p_route->type)
    {
        for_each_area_lsa(p_area, OSPF_LS_SUMMARY_ASBR, p_lsa, p_nextlsa)
        {            
            p_summary = ospf_lsa_body(p_lsa->lshdr);
            
            mask = OSPF_HOST_MASK;
            dest = ntohl(p_summary->h.id) & mask;
            cost = ntohl(p_summary->metric) & OSPF_METRIC_INFINITY;
            abr = ntohl(p_summary->h.adv_id);
   
            /*asbr destination can not be self*/
            if (dest == p_process->router_id)
            {
                continue;
            }
   
            /*ignore invalid lsa*/
            if ((ospf_invalid_metric(cost)) 
             || (OSPF_MAX_LSAGE <= ntohs(p_summary->h.age)) 
             || (abr == p_process->router_id))     
            {
                continue;
            }
   
            /*get backup route for abr*/
            p_abr = ospf_backup_abr_lookup(p_area, abr);
            if (NULL == p_abr)
            {
                continue;
            }
            p_path = &p_abr->path[p_process->backup_current_route];
            if ((p_path->ifunit == p_nexthop_info->if_uint)
                && (p_path->nexthop == p_nexthop_info->addr))
            {
                continue;
            }
            /*create backup route from backup abr,only one nexthop need*/
            main_nhop = p_nexthop_info->addr;
            ospf_backup_route_add(p_process, &p_area->backup_asbr_table, OSPF_ROUTE_ASBR,dest, mask, cost + p_path->cost, p_path->ifunit, p_path->nexthop, main_nhop);
            break;
        } 
    }
    return;
}

/*calculate backup route for external-area route,backup path must belong to the same area*/
void
ospf_backup_external_route_calculate(
                             struct ospf_process *p_process,
                             struct ospf_route *p_route)
{
    struct ospf_area *p_area = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_nextlsa = NULL;  
    struct ospf_external_lsa  *p_external = NULL;
    struct ospf_backup_route *p_asbr = NULL;
    struct ospf_backup_path *p_path = NULL; 
    struct ospf_lsvector lsvector;
    struct ospf_path current_path;
    struct ospf_nexthop_info *p_nexthop_info;
    u_int metric = 0;
    u_int mask = 0;
    u_int faddr = 0;
    u_int dest = 0;
    u_int asbr_id = 0;
    u_short age = 0;
    u_int i = 0;
    u_int current = p_process->current_route;
    u_int main_nhop = 0;
    
    /*get backup route's area,if area not exist, try to decide it according to nexthop*/
    p_area = p_route->path[current].p_area;
    if (NULL == p_area)
    {
        for_each_node(&p_process->normal_if_table, p_if, p_next_if)
        {
            if (p_if->ifnet_uint == p_route->path[current].p_nexthop->gateway[0].if_uint)
            {
                p_area = p_if->p_area;
                break;
            }
        }
    }
    if (NULL == p_area)
    {
        return;
    }

    ospf_lsa_lookup_by_id(&p_process->t5_lstable.list, OSPF_LS_AS_EXTERNAL, p_route->dest,p_route->mask, &lsvector);
    for (i = 0; i<lsvector.count; i++)
    {
        p_lsa = lsvector.p_lsa[i];
        p_external = ospf_lsa_body(p_lsa->lshdr);
        
        metric = ntohl(p_external->metric) & OSPF_METRIC_INFINITY;            
        age = ntohs(p_external->h.age);
        asbr_id = ntohl(p_external->h.adv_id);
        mask = ntohl(p_external->mask);
        dest = ntohl(p_external->h.id) & mask;
        faddr = ntohl(p_external->fwdaddr);
        if ((ospf_invalid_metric(metric)) 
            || (OSPF_MAX_LSAGE <= age) 
            || (asbr_id == p_process->router_id))
        { 
            continue;
        }

        /*get backup asbr route,asbr route may self be inter-area route,in this case,path must
         belong to the same area*/
        p_asbr = ospf_backup_asbr_lookup(p_area, asbr_id);
        if (p_asbr)
        {
            p_path = &p_asbr->path[p_process->backup_current_route];  
            p_nexthop_info = &p_route->path[current].p_nexthop->gateway[0];
        }
        if ((NULL == p_asbr)
            || ((p_path->ifunit == p_nexthop_info->if_uint)&& (p_path->nexthop == p_nexthop_info->addr)))
        {
            /*when External lsa with same dest ,different advid  
            r1--r--r2, r1 and r2 originate the same dest external lsa,but not ECMP(cost different)*/
            struct ospf_route *p_old_asbr = NULL;
 
            p_old_asbr = ospf_asbr_lookup(p_process, p_area, asbr_id);
            if (NULL == p_old_asbr)
            {
                continue;
            }
            memset(&current_path, 0, sizeof(current_path));
            if (ospf_ase2_metric(ntohl(p_external->metric)))
            {
                current_path.type = OSPF_PATH_ASE2;
                current_path.cost = p_old_asbr->path[current].cost;
                current_path.cost2 = ntohl(p_external->metric);
            }
            else
            {
                current_path.type = OSPF_PATH_ASE;
                current_path.cost = ntohl(p_external->metric)+p_old_asbr->path[current].cost;
            }

            if ((p_old_asbr->path[current].p_nexthop->gateway[0].addr != p_route->path[current].p_nexthop->gateway[0].addr)
               && (0 > ospf_path_cmp(&current_path, &p_route->path[current])))
            {
                 main_nhop = p_route->path[current].p_nexthop->gateway[0].addr;
                 ospf_backup_route_add(p_process, &p_process->backup_route_table ,OSPF_ROUTE_NETWORK, dest, mask, current_path.cost, 
                     p_old_asbr->path[current].p_nexthop->gateway[0].if_uint, p_old_asbr->path[current].p_nexthop->gateway[0].addr,
                     main_nhop);
                 return;
            }

            continue;
        }
        /*create backup route*/
        main_nhop = p_route->path[current].p_nexthop->gateway[0].addr;
        ospf_backup_route_add(p_process, &p_process->backup_route_table ,
            OSPF_ROUTE_NETWORK, dest, mask, metric + p_path->cost,
            p_path->ifunit, p_path->nexthop, main_nhop);
        return;
    }

    /*check all nssa lsa in this area*/
    for_each_nssa_lsa(p_area, p_lsa, p_nextlsa)
    {
        p_external = ospf_lsa_body(p_lsa->lshdr);
        metric = ntohl(p_external->metric) & OSPF_METRIC_INFINITY;    
        age = ntohs(p_external->h.age);
        asbr_id = ntohl(p_external->h.adv_id);
        mask = ntohl(p_external->mask);
        dest = ntohl(p_external->h.id) & mask;
        faddr = ntohl(p_external->fwdaddr);

        /*dest and mask must same*/
        if ((dest != p_route->dest) || (mask != p_route->mask))
        {
            continue;
        }
        
        if ((ospf_invalid_metric(metric)) 
            || (OSPF_MAX_LSAGE <= age) 
            || (asbr_id == p_process->router_id))
        {                
            continue;
        }

        /*get backup asbr route,asbr route must be intra-area*/
        p_asbr = ospf_backup_asbr_lookup(p_area, asbr_id);
        if (NULL == p_asbr)
        {                
            continue;
        }
        p_path = &p_asbr->path[p_process->backup_current_route];                    
        /*create backup route*/
        main_nhop = p_route->path[current].p_nexthop->gateway[0].addr;
        ospf_backup_route_add(p_process, &p_process->backup_route_table,
            OSPF_ROUTE_NETWORK, dest, mask, metric + p_path->cost,
            p_path->ifunit, p_path->nexthop, main_nhop);
        break;
    }
    return;
}

/*calculate back path for a special route*/
void
ospf_backup_route_calculate(
                         struct ospf_process *p_process,
                         struct ospf_route *p_route)
{
    u_int current = p_process->current_route;
    int i = 0;
    
    /*ignore not protected route*/
    if ((NULL == p_route->path[current].p_nexthop)
        //|| (1 < p_route->path[current].p_nexthop->count)
       /* || (TRUE == p_route->local_route)*/)
    {
        ospf_logx(ospf_debug_frr, "ignore protect route");
        return;
    }
    if (1 < p_route->path[current].p_nexthop->count)
    {
        ospf_logx(ospf_debug_frr,"�ȼ�·��count%d", p_route->path[current].p_nexthop->count);
        ospf_logx(ospf_debug_frr,"dest:0x%x, mask:0x%x", p_route->dest, p_route->mask);
        for(i = 0; i < p_route->path[current].p_nexthop->count; i++)
        {
            ospf_logx(ospf_debug_frr,"%d next hop:0x%x", i+1, p_route->path[current].p_nexthop->gateway[i].addr);
        }
        ospf_logx(ospf_debug_frr,"type 0x%x\n", p_route->path[current].type);
        if(OSPF_ROUTE_NETWORK == p_route->type)
        {
            ospf_ecmp_backup_router_add(p_process, p_route);
        }
        return;
    }

    /*calculate backup route for different type*/
    switch (p_route->path[current].type) {
        case OSPF_PATH_INTRA:
             ospf_backup_intra_route_calculate(p_process, p_route);
             break;
                
        case OSPF_PATH_INTER:
             ospf_backup_inter_route_calculate(p_process, p_route);
             break;

        default:
             ospf_backup_external_route_calculate(p_process, p_route);
             break;
    }
    return;
}

STATUS
ospf_backup_sys_route_update(
                         struct ospf_process *p_process,
                         struct ospf_backup_route *p_route)
{
    struct ospf_backup_path *p_old = NULL;
    struct ospf_backup_path *p_new = NULL;
    struct ospf_route *p_primary_route = NULL;
    struct ospf_iproute iproute;
    u_int not_update = FALSE;
    u_int old = p_process->backup_old_route;
    u_int current = p_process->backup_current_route;
    u_char ucDbg = ospf_debug_frr;


    p_old = &p_route->path[old];
    p_new = &p_route->path[current];
    ospf_logx(ucDbg, "backup route export:dest %x/%x/%d,ifunit%d,nexthop%x", p_route->dest, p_route->mask, p_route->type,
        p_new->ifunit, p_new->nexthop);

    /*if route not changed,do nothing*/
    if ((p_new->cost == p_old->cost)
        && (p_new->nexthop == p_old->nexthop)
        && (p_new->ifunit == p_old->ifunit))
    {
        return OK;
    }

    memset(&iproute, 0, sizeof(iproute));
    iproute.p_process = p_process;
    iproute.dest = htonl(p_route->dest);
    iproute.mask = htonl(p_route->mask);
    iproute.proto = M2_ipRouteProto_ospf;
    iproute.fwdaddr = htonl(p_route->nexthop);

    /*update ip route for each changed nexthop*/

    /*delete old nexthop*/
    if (p_old->nexthop)
    {   
        /* del route is uesd in primary route,not delete*/
        p_primary_route = ospf_route_lookup(&p_process->route_table, OSPF_ROUTE_NETWORK, p_route->dest, p_route->mask);
        if (p_primary_route
            && p_primary_route->path[p_process->current_route].p_nexthop
            && ospf_nexthop_exist(p_primary_route->path[p_process->current_route].p_nexthop, p_old->nexthop))
        {
            not_update = TRUE;
            ospf.stat.backup_not_del++;                
        }

        if (FALSE == not_update)
        {
            /*update ip route for each changed nexthop*/
            //iproute.fwdaddr = p_old->nexthop; 
            iproute.bak_hop= htonl(p_old->nexthop); 
            iproute.if_unit = p_old->ifunit; 
            iproute.active = FALSE;
            iproute.metric = 0; 
            iproute.backup_route = TRUE;
            ospf_logx(ucDbg, "del back router:");
            ospf_logx(ucDbg, "dest:0x%x, mask:0x%x, primary hop:0x%x, back hop:0x%x",
                iproute.dest, iproute.mask, iproute.fwdaddr, iproute.bak_hop);

            #if 0
            if (OK != ospf_rtsock_route_msg_insert(&iproute))
            {
                return ERR;
            }
            #endif
        }
        /*reset old path after update route*/
        p_old->nexthop = 0;
        p_old->ifunit = 0;        
    }
    /*add new path*/
    if (p_new->nexthop)
    {   
        //iproute.fwdaddr = p_new->nexthop; 
        iproute.bak_hop= htonl(p_new->nexthop); 
        iproute.if_unit = p_new->ifunit; 
        iproute.active = TRUE;
        iproute.metric = p_new->cost; 
        iproute.backup_route = TRUE;
        ospf_logx(ucDbg,"add back router:");
        ospf_logx(ucDbg,"dest:0x%x, mask:0x%x, primary hop:0x%x, back hop:0x%x",
            iproute.dest, iproute.mask, iproute.fwdaddr, iproute.bak_hop);

        #if 0
        /*if rtmsg need,send rtmsg,return failed if not ok*/
        if (OK != ospf_rtsock_route_msg_insert(&iproute))
        {
            return ERR;
        }
        #endif
    }
    return OK;
}

#define OSPF_BACKUP_EXPORT_TIME_LIMIT 3
void
ospf_backup_route_export(struct ospf_process *p_process)
{
    struct ospf_backup_route last_route;
    struct ospf_backup_route *p_route = NULL;
    struct ospf_backup_route *p_sucess = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_area *p_next_area = NULL;
    STATUS rc = ERR;
    u_int now;
    u_int start;

    ospf_logx(ospf_debug_frr, "backup route export");
    /*if main route is waiting,backup route wait*/
    if (TRUE == p_process->wait_export)
    {
        p_process->backup_wait_export = TRUE;
        ospf_timer_start(&p_process->backup_ipupdate_timer, 1);
        return;
    }
    memset(&last_route, 0, sizeof(last_route));
    if (OSPF_HOST_NETWORK != p_process->backup_last_export_network)
    {
        last_route.type = OSPF_ROUTE_NETWORK;
        last_route.dest = p_process->backup_last_export_network;
        last_route.mask = p_process->backup_last_export_mask;
    }
    start = ospf_sys_ticks();             

    for_each_node_greater(&p_process->backup_route_table, p_route, &last_route)
    {
        rc = ospf_backup_sys_route_update(p_process,p_route);
        if (OK != rc)
        {
            if (p_sucess)
            {
                p_process->backup_last_export_network = p_sucess->dest;
                p_process->backup_last_export_mask = p_sucess->mask;
            }
            p_process->backup_wait_export = TRUE;
            ospf_timer_start(&p_process->backup_ipupdate_timer, 1);
            return;
        }
        p_sucess = p_route;
        /*if do not wait for finish,we will stop update if exceed timer limit*/
        /*if time limit exceed,stop update, we will restart from the next route*/
        now = ospf_sys_ticks();             
        if (OSPF_BACKUP_EXPORT_TIME_LIMIT < ospf_time_differ(now, start))
        {
            p_process->last_export_network = p_route->dest;
            p_process->last_export_mask = p_route->mask;
            p_process->backup_wait_export = TRUE;
            ospf_timer_start(&p_process->backup_ipupdate_timer, 1);            
            return;
        } 
    }

    /*all update finished,prepare to delete unused routes*/
    ospf_backup_old_route_clear(p_process, &p_process->backup_route_table);
    
    for_each_ospf_area(p_process, p_area, p_next_area)
    {        
        ospf_backup_old_route_clear(p_process, &p_area->backup_abr_table);
        ospf_backup_old_route_clear(p_process, &p_area->backup_asbr_table);
    }
    p_process->backup_wait_export = FALSE;
    return;
}


int ospf_backup_route_switch(struct ospf_nbr* p_nbr)
{
    struct ospf_process *p_process = p_nbr->p_if->p_process;
    struct ospf_backup_route *pstBRouteFrist = NULL;
    struct ospf_backup_route *pstBRouteNext = NULL;

    for_each_node(&p_process->backup_route_table, pstBRouteFrist, pstBRouteNext)
    {
        if (p_nbr->addr == pstBRouteFrist->nexthop)
        {
            //printf("type:0x%x, dest:0x%x, mask:0x%x, main_hop:0x%x\n",
            //    pstBRouteFrist->type, pstBRouteFrist->dest, pstBRouteFrist->mask,
            //    pstBRouteFrist->nexthop);
        }
    }

    return OK;
}


int ospf_backup_route_if_switch(u_int uiProId, u_int uiIfIndex)
{
    struct ospf_nbr *pstNbr = NULL;
    struct ospf_nbr *pstNextNbr = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_nprocess = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_nextif = NULL;

	for_each_node(&ospf.process_table, p_process, p_nprocess)
	{
        if(p_process->process_id != uiProId)
        {
            continue;
        }
        for_each_node(&p_process->nbr_table, pstNbr, pstNextNbr)
        {
            if (uiIfIndex == pstNbr->p_if->ifnet_uint)
            {
                break;
            }
        }
	}

    if (pstNbr != NULL)
    {
        ospf_backup_route_switch(pstNbr);
    }

    return OK;
}

#endif
