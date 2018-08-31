/* ospf_spf.c  spf calculation */

#include "ospf.h"

void ospf_spf_nexthop_get(struct ospf_spf_vertex * p_node);
void ospf_spf_examine_router_vertex(struct ospf_spf_vertex * p_vertex_V);
void ospf_spf_examine_network_vertex(struct ospf_spf_vertex * p_vertex_V);
void ospf_spf_parent_set(struct ospf_spf_vertex * p_parent, u_int ifaddr, struct ospf_spf_vertex * p_node);
struct ospf_route * ospf_intra_route_calculate(u_int type, u_int dest, u_int mask, struct ospf_spf_vertex * p_vertex, u_int local_route);

/************************************************************
  compare spf node for lookup
  rule:type+id
*/
int 
ospf_spf_lookup_cmp(
                  struct ospf_spf_vertex *p1, 
                  struct ospf_spf_vertex *p2)
{
    OSPF_KEY_CMP(p1, p2, id);
    OSPF_KEY_CMP(p1, p2, type);
    return 0; 
}

/*compare spf node using cost*/
int 
ospf_spf_cost_lookup_cmp(
                  struct ospf_spf_vertex *p1, 
                  struct ospf_spf_vertex *p2)
{
    OSPF_KEY_CMP(p1, p2, cost);
    
    /*lager type prefered*/
    OSPF_KEY_CMP(p2, p1, type);
 
    OSPF_KEY_CMP(p1, p2, id);
 
    return 0; 
}

void 
ospf_spf_table_init(struct ospf_area *p_area)
{
    ospf_lstinit(&p_area->spf_table, ospf_spf_lookup_cmp);
    ospf_lstinit2(&p_area->candidate_table, ospf_spf_cost_lookup_cmp, mbroffset(struct ospf_spf_vertex, cost_node));
    return;
}

/*api:lookup spf node in a table*/
struct ospf_spf_vertex *
ospf_spf_lookup(
              struct ospf_lst *p_table, 
              u_int id,
              u_int type)
{
    struct ospf_spf_vertex search;
    search.type = type;
    search.id = id;
    return ospf_lstlookup(p_table, &search);
}

/*just create new now,add to spf and candidate list*/
struct ospf_spf_vertex *
ospf_spf_node_create(
                 struct ospf_area *p_area,
                 u_int type,
                 u_int id,
                 u_int cost)
{
    struct ospf_spf_vertex *p_vertex = ospf_malloc2(OSPF_MSPF);
    
    if (NULL != p_vertex)
    {
        p_vertex->id = id;
        p_vertex->type = type;    
        p_vertex->p_area = p_area;
        p_vertex->cost = cost;
        /*add to both spf table and candidate table*/
        ospf_lstadd(&p_area->spf_table, p_vertex);
        ospf_lstadd(&p_area->candidate_table, p_vertex);
    }
    return p_vertex;
}

/*free spf node and related nexthop list.it must be cleared from list*/
void 
ospf_spf_node_delete(struct ospf_spf_vertex *p_node)
{
    ospf_mfree(p_node, OSPF_MSPF);
    return;
}

void 
ospf_spf_table_flush(struct ospf_area *p_area)
{
    struct ospf_spf_vertex *p_vertex = NULL;
    struct ospf_spf_vertex *p_next = NULL;   
    
    /*delete node in sort table, do not free memory*/
    for_each_node(&p_area->candidate_table, p_vertex, p_next)
    {
       ospf_lstdel(&p_area->candidate_table, p_vertex);
    }
    /*clear spf table and release spf node*/ 
    for_each_node(&p_area->spf_table, p_vertex, p_next)
    {
       ospf_lstdel(&p_area->spf_table, p_vertex);
       ospf_spf_node_delete(p_vertex);
    }
    return;
}

/*this function check if a stub link can be add to spf tree,if so ,add it
*/ 
struct ospf_route * 
ospf_spf_stub_route_add(
                     struct ospf_router_link *p_link,
                     struct ospf_spf_vertex *p_parent) 
{
    struct ospf_spf_vertex node;
    struct ospf_area *p_area = p_parent->p_area;
    struct ospf_if *p_if = NULL;
    u_int cost = p_parent->cost + ntohs(p_link->tos0_metric);
    u_int dest = ntohl(p_link->id);
    u_int mask = ntohl(p_link->data);
    u_int ifip = 0;
    u_int8 dstr[20];
    u_int8 mstr[20];
    u_int local_route = FALSE;
    
    /* section 16.1, stage 2, item (1) (p 154) */
    ospf_logx(ospf_debug_spf, "add stub link %s/%s", ospf_inet_ntoa(dstr, dest), ospf_inet_ntoa(mstr, mask));

    /*get nexthop for stub node,for local stub node,must decide local output interface*/
    memset (&node, 0, sizeof(node));
    node.id = dest;
    node.cost = cost;
    node.p_area = p_area;
    node.type = OSPF_LS_NETWORK;/*local stub node have no nexthop,so use network*/
    /*parent is root,so decide local interface address*/
    if (p_parent->id == p_area->p_process->router_id)
    {
	//	printf("%s %d  ****ospf_if_lookup_by_network*******\n", __FUNCTION__,__LINE__);
        p_if = ospf_if_lookup_by_network(p_area->p_process, dest);
        if (NULL !=p_if)    
        {
            ifip = p_if->addr;
        }
    }
    /*link parent and this node*/
    ospf_spf_parent_set(p_parent, ifip, &node);
 //    printf("%s %d  *****p_parent->id 0x%x, ifip 0x%x******\n", __FUNCTION__,__LINE__,p_parent->id, ifip);

    /*calculate nexthop*/
    ospf_spf_nexthop_get(&node);

    /*parent is root,it is local route */
    if (p_parent->id == p_area->p_process->router_id)
    {
         local_route = TRUE;
    }
    else
    {
        node.p_lsa = p_parent->p_lsa; /*caoyong modify 2018-3-9 for display stub route advRouter*/
    }
    return ospf_intra_route_calculate(OSPF_ROUTE_NETWORK, dest, mask, &node, local_route);
}

/*check if one link exist in a lsa*/
void * 
ospf_lsa_link_lookup (
              struct ospf_lsa *p_lsa,
              struct ospf_spf_vertex *p_parent)
{
    struct ospf_router_link *p_rlink = NULL;
    struct ospf_router_lsa *p_router = ospf_lsa_body(p_lsa->lshdr);
    struct ospf_network_lsa *p_network = ospf_lsa_body(p_lsa->lshdr);
    u_int id = p_parent->id;
    u_int *p_nlink = NULL;

    /*router lsa*/
    if (OSPF_LS_ROUTER == p_lsa->lshdr->type)
    {              
        for_each_router_link(p_router, p_rlink)       
        {
            if (OSPF_LS_ROUTER == p_parent->type)/*p2p*/
            {
                if ((OSPF_RTRLINK_VLINK != p_rlink->type) && (OSPF_RTRLINK_PPP != p_rlink->type))
                {
                    continue;
                }
            }
            else if (OSPF_LS_NETWORK == p_parent->type)/*transit*/
            {
                if (OSPF_RTRLINK_TRANSIT != p_rlink->type)
                {
                    continue;
                }
            }
            if (id == ntohl(p_rlink->id))
            {
                return (void*)p_rlink;
            }
        }            
    }
    else/*network lsa*/
    {
        for_each_network_link(p_network, p_nlink)
        {
            if (ntohl(*p_nlink) == id)
            {
                return (void*)p_nlink;
            } 
        }
    }
    return NULL;
}

/*calculate route after spf running*/
void 
ospf_intra_route_calculate_for_area(struct ospf_area *p_area)
{
    struct ospf_spf_vertex *p_node = NULL;  
    struct ospf_spf_vertex *p_next = NULL;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_router_link *p_link = NULL;
    struct ospf_router_lsa *p_router = NULL;
    struct ospf_network_lsa *p_network = NULL;
    struct ospf_route *p_route = NULL;
    u_int mask = 0;
    u_int8 dest[20];
    u_int local_route = FALSE;
    u_int current = 0;

    ospf_logx(ospf_debug_spf, "ospf_intra_route_calculate_for_area calculate intra route for area %u", p_area->id);   

    /*scan for all spf vertex*/
    for_each_node(&p_area->spf_table, p_node, p_next)
    {        
        local_route = FALSE;
        if (ospf_debug_spf)
        {
            ospf_logx(ospf_debug_spf, "ospf_intra_route_calculate_for_area check spf node %s,type %s", 
            		ospf_inet_ntoa(dest, p_node->id),
                (OSPF_LS_ROUTER == p_node->type) ? "Router" : "Network");
            if (NULL != p_node->p_nexthop)
            {
                ospf_logx(ospf_debug_spf, "nexthop %s", ospf_inet_ntoa(dest, p_node->p_nexthop->gateway[0].addr));
            }           
        }        

        /*obtain database directly*/    
        p_lsa = p_node->p_lsa;
        if (NULL == p_lsa)
        {
            ospf_logx(ospf_debug_spf, "no lsa matching this node,skip"); 
            continue;
        }

        /*abr/asbr/network*/
        if (OSPF_LS_ROUTER == p_node->type)
        {
            p_router = ospf_lsa_body(p_lsa->lshdr); 

            ospf_logx(ospf_debug_spf, "router lsa flag %x", p_router->flag); 

            if (ospf_router_flag_abr(p_router->flag) && (p_area->p_process->router_id != p_node->id))                
            {
                ospf_intra_route_calculate(OSPF_ROUTE_ABR, p_node->id, 0, p_node, FALSE);
	//	printf("%s %d  *****OSPF_ROUTE_ABR******\n", __FUNCTION__,__LINE__);
            }            

            if (ospf_router_flag_asbr(p_router->flag) && (p_area->p_process->router_id != p_node->id) && (p_area->is_stub!= TRUE))                
            {
                ospf_intra_route_calculate(OSPF_ROUTE_ASBR, p_node->id, 0, p_node, FALSE);
//		printf("%s %d  *****OSPF_ROUTE_ASBR******\n", __FUNCTION__,__LINE__);
            }
            /*stub link*/
            for_each_router_link(p_router, p_link)
            {
                 /*only consider stub link*/
                if (OSPF_RTRLINK_STUB == p_link->type)
                {
                    p_route = ospf_spf_stub_route_add(p_link, p_node); 
					
			if(NULL != p_route)
			{
				current = p_area->p_process->current_route;
				p_route->path[current].intra_type = OSPF_RTRLINK_STUB;
			}

                    #if 0 
                    /*parent is root,it is local route*/
                    if ((NULL !=p_route) && (p_node->id == p_area->p_process->router_id))
                    {
                        p_route->local_route = TRUE;
                    }
                    #endif
                }
            }
        }
        else 
        {
            p_network = ospf_lsa_body(p_lsa->lshdr);
            mask = ntohl(p_network->mask);
           
            if (p_node->parent[0].p_node->id == p_area->p_process->router_id)
            {
                local_route = TRUE;
            }  
            p_route = ospf_intra_route_calculate(OSPF_ROUTE_NETWORK, p_node->id & mask, mask, p_node, local_route);   
	     if(NULL != p_route)
	     {
			current = p_area->p_process->current_route;
			p_route->path[current].intra_type = OSPF_RTRLINK_TRANSIT;
	     }
 #if 0 
            /*network's parent is root, it is local route,only consider the first parent......may be changed*/
            if ((NULL != p_route) && (p_node->parent[0].p_node->id == p_area->p_process->router_id))
            {
                p_route->local_route = TRUE;
            }   
            #endif
        }
    }    
    return;
}
/*get nexthop for directly connected node*/
void 
ospf_spf_direct_nexthop_get(
                    struct ospf_spf_vertex *p_node,
                    struct ospf_nexthop *p_nexthop,
                    u_int ifaddr)
{            
    struct ospf_area *p_area = p_node->p_area;
    struct ospf_process *p_process = p_area->p_process; 
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next = NULL;    
    
    ospf_logx(ospf_debug_spf, "parent is root");
    
    /*in any case,output interface must exist,and has same area.if not same area
       it may be a virtual link,do not calculate nexthop here*/
//		printf("%s %d  ****ospf_if_lookup*******\n", __FUNCTION__,__LINE__);
    p_if = ospf_if_lookup(p_process, ifaddr);
//	printf("%s %d	****ifaddr 0x%x*******\n", __FUNCTION__,__LINE__,ifaddr);
//	printf("%s %d  ****p_if 0x%x*******\n", __FUNCTION__,__LINE__,p_if);
	if (p_if != NULL)
	{
//		printf("%s %d  ****addr 0x%x, ifnet_uint 0x%x*******\n", __FUNCTION__,__LINE__,p_if->addr, p_if->ifnet_uint);
	}

	
    if ((NULL == p_if) || (p_area != p_if->p_area))
    {    
        /*20120724 shamlink:try to get shamlink*/
        for_each_node(&p_process->shamlink_if_table, p_if, p_next_if)
        {

            if ((ifaddr == p_if->addr) && (p_area == p_if->p_area))
            {
                p_nbr = ospf_lstfirst(&p_if->nbr_table);
                if (p_nbr && (p_nbr->id == p_node->id))
                {
      //          	  printf("%s %d  **** addr 0x%x uint 0x%x*******\n", __FUNCTION__,__LINE__,
	//		p_nbr->addr, p_if->ifnet_uint);
                    p_nexthop->gateway[p_nexthop->count].if_uint = p_if->ifnet_uint;
                    p_nexthop->gateway[p_nexthop->count].addr = p_nbr->addr;
                    p_nexthop->gateway[p_nexthop->count].type = OSPF_NEXTHOP_SHAMLINK;
                    p_nexthop->count++;
                    break;
                }   
            }
        }
        if (NULL == p_if)
        {
            ospf_logx(ospf_debug_spf, "nexthop interface not found by %x", ifaddr);
        }
        return;
   }
             
   /*record local address as nexthop*/
   p_nexthop->gateway[p_nexthop->count].addr = ifaddr;
   p_nexthop->gateway[p_nexthop->count].if_uint = p_if->ifnet_uint;
 
   /*router node need real nexthop address,so get matched nbr,use nbr's address as nexthop*/            
   if (OSPF_LS_ROUTER == p_node->type)
   {
       ospf_logx(ospf_debug_spf, "select nexthop for directly nbr");
                
       for_each_ospf_nbr(p_if, p_nbr, p_next)
       {
//	   ospf_logx(ospf_debug_spf, "nexthop_get p_nbr->id=0x%x,p_node->id=0x%x.\r\n",p_nbr->id,p_node->id);
           if (p_nbr->id == p_node->id)
           {
               ospf_logx(ospf_debug_spf, "nbr got,set nexthop %x", p_nbr->addr);
               
               p_nexthop->gateway[p_nexthop->count].addr = p_nbr->addr;
               p_nexthop->gateway[p_nexthop->count].if_uint = p_if->ifnet_uint;         
               break;
           }
       }
   }
   p_nexthop->count++;
   return;        
}

/*change te tunnel head-end node's nexthop to tail-end node
*   FALSE no need to change nexhop 
*   TRUE this node is te tunnel head-end node ,change the nexthop
************************************************************************/
u_int
ospf_spf_te_tunnel_nexthop_get(
              struct ospf_process *p_process,
              struct ospf_spf_vertex *p_node,
              struct ospf_nexthop *nexthop)
{
    struct ospf_te_tunnel *p_te_tunnel = NULL;
    u_int8 addr[16];
    
    /*te tunnel node must be router node*/
    if (OSPF_LS_NETWORK == p_node->type)
    {   
        return FALSE;
    }
    p_te_tunnel = ospf_te_tunnel_lookup(p_process, p_node->id);
    if (NULL == p_te_tunnel)
    {
        return FALSE; 
    }
    /* te teunnel node must be active,have shortcut ability 
              and area id is 0xffffffff or same as node's area id*/
    if ((TRUE == p_te_tunnel->active)
        && (TRUE == p_te_tunnel->shortcut)
        && ((0xffffffff == p_te_tunnel->area)
            || (p_node->p_area->id == p_te_tunnel->area)))   
    {
         /*change nexthop to te tunnel tail-end node*/
        nexthop->gateway[0].type = OSPF_NEXTHOP_TETUNNEL;
        nexthop->gateway[0].addr = p_te_tunnel->addr_out;
        nexthop->gateway[0].if_uint = p_te_tunnel->if_unit;
        p_te_tunnel->area = p_node->p_area->id;
        nexthop->count++;

        ospf_logx(ospf_debug_spf, "change nexthop to tail-end node,gateway type %d,addr %s,if unit %d,area %d",
            nexthop->gateway[0].type, ospf_inet_ntoa(addr,nexthop->gateway[0].addr), nexthop->gateway[0].if_uint, p_te_tunnel->area);
        return TRUE;
    }
    return FALSE;
}

/* section 16.1.1 (page 155-156) */
/*calculate nexthop for vertex*/ 
void 
ospf_spf_nexthop_get (struct ospf_spf_vertex *p_node)
{
    struct ospf_area *p_area = p_node->p_area;
    struct ospf_process *p_process = p_area->p_process; 
    struct ospf_spf_vertex *p_parent;
    struct ospf_nbr *p_nbr;
    struct ospf_nbr *p_next;    
    struct ospf_if *p_if = NULL;
    struct ospf_nexthop nexthop;
    u_int i = 0,uiAddr = 0,uiIfindx = 0;
    u_int8 str[32];
    u_int8 linkstr[32];
    
    ospf_logx(ospf_debug_spf, "decide nexthop of spf node, type=%s,id=%s", 
        (OSPF_LS_ROUTER == p_node->type) ? "Router" : "Network",
        		ospf_inet_ntoa(str, p_node->id));
    
    memset(&nexthop, 0, sizeof(nexthop));

    /*root have no nexthop*/
    if ((OSPF_LS_ROUTER == p_node->type) && (p_process->router_id == p_node->id))
    {
//	printf("%s %d id p_node->id 0x%x, router_id 0x%x***********\n", __FUNCTION__,__LINE__,
//	p_node->id, p_process->router_id);
        return;
    }
    
    /*scan for all parent nodes*/
    for (i = 0 ; i < OSPF_MAX_SPF_PARENT ; i++)
    {
        p_parent = p_node->parent[i].p_node;
        if (NULL == p_parent)
        {
            continue;
        }
        ospf_logx(ospf_debug_spf, "check parent,type %s,id %s,link %s", 
            (OSPF_LS_ROUTER == p_parent->type) ? "Router" : "Network",
            		ospf_inet_ntoa(str, p_parent->id),
					ospf_inet_ntoa(linkstr, p_node->parent[i].link));

        /*prevent exceed ecmp limit*/ 
        if (OSPF_ECMP_COUNT <= nexthop.count)
        {
            break;
        }

	//	printf("%s %d id parent->id 0x%x, router_id 0x%x***********\n", __FUNCTION__,__LINE__,
	//		p_parent->id, p_process->router_id);
        /*if parent is root,decide real output interface*/
        if ((OSPF_LS_ROUTER == p_parent->type) 
            && (p_parent->id == p_process->router_id))
        {   
        #ifdef OSPF_DCN
            /*for unnumber:linkdata--ifunit*/
	//	printf("%s %d  *****ospf_if_lookup_unnumber******\n", __FUNCTION__,__LINE__);
	//	printf("%s %d  *****link 0x%x******\n", __FUNCTION__,__LINE__,p_node->parent[i].link);
       	  if(OSPF_DCN_PROCESS == p_process->process_id)
	    { 
			if (ospf_if_lookup_forDcnCreat(p_process, p_process->router_id, p_node->parent[i].link))
			{
		//		printf("%s %d  *****link 0x%x******\n", __FUNCTION__,__LINE__,p_node->parent[i].link);
				/* nexthop address must not NULL, will used in route calculate 
				fill nexthop address to nbr id */               
				nexthop.gateway[nexthop.count].addr = p_node->id;
				nexthop.gateway[nexthop.count].if_uint = p_node->parent[i].link;
				nexthop.gateway[nexthop.count].type = OSPF_NEXTHOP_UNNUMBER;
	//			printf("%s %d	*****count=%d,addr 0x%x,if_uint0x=%x.******\n", __FUNCTION__,__LINE__,nexthop.count, 
	//			nexthop.gateway[nexthop.count].addr,nexthop.gateway[nexthop.count].if_uint);
				nexthop.count++;
			    }
			else
			{
		//		printf("%s %d  *****link 0x%x******\n", __FUNCTION__,__LINE__,p_node->parent[i].link);
				ospf_spf_direct_nexthop_get(p_node, &nexthop, p_node->parent[i].link);
			}
        }
	  else
	  {
		//  printf("%s %d  *****link 0x%x******\n", __FUNCTION__,__LINE__,p_node->parent[i].link);
		  ospf_spf_direct_nexthop_get(p_node, &nexthop, p_node->parent[i].link);
	  }
         #else       
            ospf_spf_direct_nexthop_get(p_node, &nexthop, p_node->parent[i].link);
         #endif
        }
        else/*parent is not root,copy directly*/
        {
            ospf_logx(ospf_debug_spf, "accept nexthop of parent");

            /*if not a te tunnel head-tail node, merge nexthop*/
            if (FALSE == ospf_spf_te_tunnel_nexthop_get(p_process, p_node, &nexthop))
            {
                ospf_nexthop_merge(&nexthop, p_parent->p_nexthop);
            }
        }
    }
//	printf("%s %d  *****p_node->type 0x%x******\n", __FUNCTION__,__LINE__, p_node->type);
    if (OSPF_LS_ROUTER != p_node->type)
    {
        p_node->p_nexthop = ospf_nexthop_add(p_process, &nexthop);
//	printf("%s %d  ****p_node->p_nexthop 0x%x*******\n", __FUNCTION__,__LINE__,p_node->p_nexthop);
        return;
    }
    
    /*set nbr address for unresolved nexthop,only for router node*/
//	printf("%s %d  *****nexthop.count %d******\n", __FUNCTION__,__LINE__, nexthop.count);
    for (i = 0 ; i < nexthop.count; i++)
    {
//		printf("%s %d  *****p_process->process_id %ld,p_process->router_id 0x%x, if_uint 0x%x******\n", __FUNCTION__,__LINE__, 
	//		p_process->process_id,p_process->router_id,nexthop.gateway[i].if_uint);
        uiIfindx = nexthop.gateway[i].if_uint;
#ifdef OSPF_DCN
	 //	printf("%s %d	*****i=%d,addr 0x%x,if_uint0x=%x,******\n", __FUNCTION__,__LINE__, 
	// 	i,nexthop.gateway[i].addr,nexthop.gateway[i].if_uint);
		if(OSPF_DCN_PROCESS == p_process->process_id)
	        { 

            uiAddr = p_process->router_id;
            p_if = ospf_if_lookup_forDcnCreat(p_process, uiAddr,uiIfindx);
            if (NULL == p_if)    
            {
                uiAddr = nexthop.gateway[i].addr;
                p_if = ospf_if_lookup_forDcnCreat(p_process, uiAddr,uiIfindx);
                if (NULL != p_if)    
                {
              //      printf("8888 %s %d  p_if:%x,uiIfindx:%x,ulDcnflag:%d\n", __FUNCTION__,__LINE__,p_if,p_if->ifnet_uint,p_if->ulDcnflag);
                    if (OSPF_DCN_FLAG == p_if->ulDcnflag)    
                    {
                        p_if = NULL;
                    }
                }

            }
		}
		else
		{
			p_if = ospf_if_lookup(p_process, nexthop.gateway[i].addr);
		}
#else		
        		p_if = ospf_if_lookup(p_process, nexthop.gateway[i].addr);
#endif
	//	 printf("%s %d	****p_if 0x%x*******\n", __FUNCTION__,__LINE__,p_if);
		 if (p_if != NULL)
		 {
			 //printf("%s %d	****addr 0x%x, ifnet_uint 0x%x*******\n", __FUNCTION__,__LINE__,p_if->addr, p_if->ifnet_uint);
		 }
        if (NULL != p_if)    
        {
            for_each_ospf_nbr(p_if, p_nbr, p_next)
            {
                if (p_nbr->id == p_node->id)
                {
                    nexthop.gateway[i].addr = p_nbr->addr;
	//	   printf("%s %d  ****p_nbr->id:%d,p_node->id:%d,p_nbr->addr :%x*******\n", __FUNCTION__,__LINE__,p_nbr->id,p_node->id,p_nbr->addr);
                    break;
                }
            }
        }
    }
    p_node->p_nexthop = ospf_nexthop_add(p_process, &nexthop);
//	printf("%s %d  ****p_node->p_nexthop 0x%x*******\n", __FUNCTION__,__LINE__,p_node->p_nexthop);

    return;
}

/*set parent for spf node,if parent is null,clear all parent
   if parent is router,there must be routerlink,record linkdata
  */
void 
ospf_spf_parent_set(
               struct ospf_spf_vertex *p_parent,
               u_int ifaddr,
               struct ospf_spf_vertex *p_node)
{
    u_int i;

    /*set new parent,do not care about duplicate*/
    for (i = 0 ; i < OSPF_MAX_SPF_PARENT ; i++)
    {
        if (NULL == p_node->parent[i].p_node)
        {
            p_node->parent[i].p_node = p_parent;
            p_node->parent[i].link = ifaddr;
//	   printf("%s %d  *****link 0x%x, p_parent 0x%x******\n", __FUNCTION__,__LINE__,p_node->parent[i].link, p_parent);
            break;
        }
    }
    return;
}

/* Run Dijkstra on this area *//* section 16.1 of OSPF specification (page 150) */
void 
ospf_spf_calculate (struct ospf_area *p_area)
{       
    struct ospf_spf_vertex *p_node = NULL;
    struct ospf_process *p_process = p_area->p_process;
    struct ospf_lshdr lshdr;
    u_int8 vid[20];
    
    ospf_logx(ospf_debug_spf, "do spf for area %u", p_area->id);

    /*free old spf result*/
    ospf_spf_table_flush(p_area);

    p_area->transit = FALSE;
    p_area->abr_count = 0;
    p_area->asbr_count = 0;    
    p_area->spf_run++;
    
    /* section 16.1, first stage (page 151-154) */
    ospf_spf_node_create(p_area, OSPF_LS_ROUTER, p_process->router_id, 0);
    ospf_logx(ospf_debug_spf, "ospf_spf_calculate router_id= 0x%x.", p_process->router_id);

    while (1)
    {
         /*get nearest node,if candidate is empty, spf finished*/
        p_node = ospf_lstfirst(&p_area->candidate_table);
        if (NULL == p_node)
        {
            ospf_logx(ospf_debug_spf, "spf finished");        
            break;
        }

        /*remove from cost-candidate table*/
        ospf_lstdel(&p_area->candidate_table, p_node);

   //     ospf_logx(ospf_debug_spf, "ospf_spf_calculate");        
        /*obtain nexthop for new node*/
        ospf_spf_nexthop_get(p_node);
        
        ospf_logx(ospf_debug_spf, "get nearest node %s,type %d", ospf_inet_ntoa(vid,p_node->id),p_node->type);

        /*check vertex newly added to spf table*/
        /*if lsa is empty,search again*/
        if (NULL == p_node->p_lsa) 
        {
           lshdr.type = p_node->type;
           lshdr.id = htonl(p_node->id);
           /*do not care about router id for network lsa*/
           lshdr.adv_id = (OSPF_LS_ROUTER == p_node->type) ? htonl(p_node->id) : 0;

           p_node->p_lsa = ospf_lsa_lookup(p_area->ls_table[lshdr.type], &lshdr);
        }

        /*check new spf node's lsa*/
        if ((NULL != p_node->p_lsa)
            && (OSPF_MAX_LSAGE > ntohs(p_node->p_lsa->lshdr->age)))
        { 
            /*lsa type must be router or network*/
            if (OSPF_LS_ROUTER == p_node->p_lsa->lshdr->type)
            {
                ospf_spf_examine_router_vertex(p_node);
            }
            else
            {
                ospf_spf_examine_network_vertex(p_node); 
            }
        }
    }     
    /* section 16.1, second stage (page 154-155) */
    ospf_intra_route_calculate_for_area(p_area);     
    return;
}

void 
ospf_spf_candidate_node_update(
                    struct ospf_spf_vertex *p_parent,
                    u_int id,
                    u_int type,
                    u_int link_cost,
                    u_int link_data)
{
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_lsa *p_back_lsa = NULL;
    struct ospf_spf_vertex *p_child = NULL;
    struct ospf_area *p_area = p_parent->p_area;
    struct ospf_lsvector vector;
    u_int cost = p_parent->cost + link_cost;
   /* uint32_t cost = (0xFFFF == link_cost) ? OSPF_METRIC_INFINITY : (p_parent->cost + link_cost); 
*/
    u_int i = 0;
    u_int8 idstr[32];
    u_int8 linkstr[32];
    
    ospf_logx(ospf_debug_spf, "update spf candidate,type %s,id %s",
        (OSPF_LS_ROUTER == type) ? "Router" : "Network", 
        		ospf_inet_ntoa(idstr, id));

    ospf_logx(ospf_debug_spf, "parent type %s,id %s,link %s",
        (OSPF_LS_ROUTER == p_parent->type) ? "Router" : "Network",
        		ospf_inet_ntoa(idstr, p_parent->id),
				ospf_inet_ntoa(linkstr, link_data));
    
     /* section 16.1, item (2)(c) page (152) link node already in spf tree,do nothing*/
    p_child = ospf_spf_lookup(&p_area->spf_table, id, type);
    if ((NULL != p_child) && (p_child->cost < cost))
    {
        ospf_logx(ospf_debug_spf, "lsa in spf tree or has better cost");  
        return;
    }
  //  printf("%s %d  *****p_child 0x%x******\n", __FUNCTION__,__LINE__,p_child);

    /* section 16.1, item (2)(b) page (152) */
    if (NULL != p_child)
    {
        p_lsa = p_child->p_lsa;
    }
  //  printf("%s %d  *****p_lsa 0x%x******\n", __FUNCTION__,__LINE__,p_lsa);
    if (NULL == p_lsa)
    {
        ospf_lsa_lookup_by_id(&p_area->ls_table[type]->list, type, id, 0, &vector);
//	printf("%s %d  *****type 0x%x, id 0x%x,vector 0x%x******\n", __FUNCTION__,__LINE__,type, id,vector);
    }
    else
    {
        vector.count = 1;
        vector.p_lsa[0] = p_lsa;
//	printf("%s %d  *****vector.p_lsa[0] 0x%x******\n", __FUNCTION__,__LINE__,vector.p_lsa[0]);
    }
  //   printf("%s %d  *****vector.count %d******\n", __FUNCTION__,__LINE__,vector.count);

    /*select back lsa,there may be multiple candidate lsa,so check all*/
    for (i = 0; i < vector.count ; i++)
    {
        if (NULL == vector.p_lsa[i])
        {
//	    printf("%s %d  ***vector.p_lsa[%d] 0x%x******\n", __FUNCTION__,__LINE__,i,vector.p_lsa[i]);
            continue;
			
        }
        
        ospf_lsa_age_update(vector.p_lsa[i]);
        if (OSPF_MAX_LSAGE <= ntohs(vector.p_lsa[i]->lshdr->age))
        {
            ospf_logx(ospf_debug_spf, "lsa has max age");        
            continue;
        }
   
        if (NULL == ospf_lsa_link_lookup(vector.p_lsa[i], p_parent))
        {
            ospf_logx(ospf_debug_spf, "lsa has no back link");
            continue;
        }
        p_back_lsa = vector.p_lsa[i];
        break;
    }
    
    if (NULL == p_back_lsa)
    {
        ospf_logx(ospf_debug_spf, "lsa not found");        
        return;//
    }

    /*create new candidate node*/
    if (NULL == p_child)
    {    
        ospf_logx(ospf_debug_spf, "create candidate ");
        
        p_child = ospf_spf_node_create(p_area, type, id, cost);
        if (NULL == p_child)
        {
            return;
        }
        p_child->p_lsa = p_back_lsa;
        ospf_spf_parent_set(p_parent, link_data, p_child); 

//	printf("%s %d  *****p_parent->id 0x%x, link_data 0x%x,p_child 0x%x******\n", __FUNCTION__,__LINE__,p_parent->id, link_data,p_child);
        return;
    }

    /*candidate exist,update better instance*/
    /*compare cost*/
    /* section 16.1, item (2)(d), first bullet item, page (152) */
    if (cost > p_child->cost)                                
    {
        ospf_logx(ospf_debug_spf, "new cost greater,ignore it");
        return;
    } 
    
    /* section 16.1, item (2)(d), second bullet item, page (152),merge*/
    if (cost == p_child->cost)                        
    {
        ospf_logx(ospf_debug_spf, "new cost equal to exist one,merge it"); 
        ospf_spf_parent_set(p_parent, link_data, p_child);
//	printf("%s %d  *****p_parent->id 0x%x, link_data 0x%x,p_child 0x%x******\n", __FUNCTION__,__LINE__,p_parent->id, link_data,p_child);
        return;
    }

    /*replace*/
    /* section 16.1, item (2)(d), third bullet item, page (152) */
    ospf_logx(ospf_debug_spf, "new cost less than exist one,replace it");

    /*remove from cost table*/
    ospf_lstdel(&p_area->candidate_table, p_child);

    p_child->p_lsa = p_back_lsa;
    p_child->cost = cost;

    /*clear current parent nodes*/    
    memset(p_child->parent, 0, sizeof(p_child->parent));

    /*insert new parent*/
    ospf_spf_parent_set(p_parent, link_data, p_child);
//	printf("%s %d  *****p_parent->id 0x%x, link_data 0x%x,p_child 0x%x******\n", __FUNCTION__,__LINE__,p_parent->id, link_data,p_child);

    /*add back to cost table*/
    ospf_lstadd(&p_area->candidate_table, p_child);
    return;
}

/*examine the router lsa associated with the vertex*/
void 
ospf_spf_examine_router_vertex (struct ospf_spf_vertex *p_vertex)
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
        	ospf_inet_ntoa(lid, ntohl(p_link->id));
        	ospf_inet_ntoa(ldata, ntohl(p_link->data));
            ospf_logx(ospf_debug_spf, "ospf_spf_examine check link type %d,id %s,data %s", p_link->type, lid, ldata);
        }
        
        /* skip stub nets for now, they will be added later */                  
        /* section 16.1, item (2)(a) page (151) */
        if (OSPF_RTRLINK_STUB == p_link->type)        
        {
            ospf_logx(ospf_debug_spf, "ospf_spf_examine skip stub link currently");
            continue; 
        }
        
        /* section 16.1, item (2)(d), third bullet item, page (152) */
        /*this is new candidate node*/
        type = (OSPF_RTRLINK_TRANSIT == p_link->type) ? OSPF_LS_NETWORK : OSPF_LS_ROUTER;
        ospf_spf_candidate_node_update(p_vertex, ntohl(p_link->id), type, ntohs(p_link->tos0_metric), ntohl(p_link->data));
//	printf("%s %d  *****p_vertex 0x%x,p_link->id 0x%x, type %d,tos0_metric %d,p_link->data 0x%x******\n", 
//		__FUNCTION__,__LINE__,p_vertex, ntohl(p_link->id), type, ntohs(p_link->tos0_metric), ntohl(p_link->data));
    }
    return;
}

/*examine the network lsa associated with the vertex*/
void 
ospf_spf_examine_network_vertex (struct ospf_spf_vertex *p_vertex)
{
    struct ospf_network_lsa *p_network = ospf_lsa_body(p_vertex->p_lsa->lshdr);
    u_int *p_link = NULL;
    u_int8 attachstr[20];

    /* section 16.1, item (2)(d), third bullet item, page (152) */   
    for_each_network_link(p_network, p_link)
    {
    	ospf_inet_ntoa(attachstr, *p_link);
        ospf_logx(ospf_debug_spf, "check attached router %s", attachstr);

        ospf_spf_candidate_node_update(p_vertex, ntohl(*p_link), OSPF_LS_ROUTER, 0, 0);
//	printf("%s %d  *****p_vertex 0x%x,p_link 0x%x, type %d******\n", 
//			__FUNCTION__,__LINE__,p_vertex, ntohl(*p_link), OSPF_LS_ROUTER);
    }    
    return;
}

/*add an intra area route from spf vertx*/
struct ospf_route * 
ospf_intra_route_calculate(
                 u_int type,
                 u_int dest,
                 u_int mask,
                 struct ospf_spf_vertex *p_vertex,
                 u_int local_route)
{
    struct ospf_route route;
    struct ospf_route *p_route = NULL;
    struct ospf_area *p_area = p_vertex->p_area;
    struct ospf_process *p_process = p_area->p_process;
    u_int current = p_process->current_route;
    u_int8 dstr[32];
    u_int8 mstr[32];
    
    ospf_logx(ospf_debug_spf, "calculate intra %s route %s/%s",
        (OSPF_ROUTE_ABR == type) ? "ABR" : ((OSPF_ROUTE_ASBR == type) ? "ASBR" : "network"),
        		ospf_inet_ntoa(dstr, dest), ospf_inet_ntoa(mstr, mask));
     #if 0
    /* the destination is unreachable */
    if (ospf_invalid_metric(p_vertex->cost))
    {
        ospf_logx(ospf_debug_spf, "cost too large,add failed");
        return NULL;
    }
    #endif

    /*build compared roue*/
    memset(&route, 0, sizeof(route));    
    route.type = type; 
    route.dest = dest;
    route.mask = (OSPF_ROUTE_NETWORK == type) ? mask : OSPF_HOST_MASK;
    route.path[current].cost = p_vertex->cost;
    route.path[current].p_area = p_area;    
    route.path[current].p_nexthop = p_vertex->p_nexthop;
    route.path[current].type = OSPF_PATH_INTRA;
    /*20131018*/
    route.path[current].local_route = local_route;
    #if 1
    if(p_vertex && p_vertex->p_lsa && local_route == FALSE)
    {
        route.path[current].adv_id = htonl(p_vertex->p_lsa->lshdr[0].adv_id);
    }
    else
    {
        route.path[current].adv_id = p_area->p_process->router_id;
    }
    #endif
    if (OSPF_ROUTE_NETWORK == type)
    {
        p_route = ospf_route_add(p_process, &p_process->route_table, &route);
    }
    else  if (OSPF_ROUTE_ABR == type)
    {
        ospf_logx(ospf_debug_spf, "add abr route:%x, areaid=%d", route.dest, p_area->id);

        p_route = ospf_route_add(p_process, &p_area->abr_table, &route);
    }
    else
    {
        ospf_logx(ospf_debug_spf, "add asbr route:%x, areaid=%d", route.dest, p_area->id);

        p_route = ospf_route_add(p_process, &p_area->asbr_table, &route);
    }
    return p_route;
}
