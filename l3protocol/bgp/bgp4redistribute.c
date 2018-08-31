
#include "bgp4redistribute.h"

#ifdef NEW_BGP_WANTED

#include "bgp4com.h"

 /*check if a route can be redistributed,only applied to ipv4 unicast*/
 int   
 bgp4_redistribute_is_permitted(
       tBGP4_VPN_INSTANCE *p_instance,
       tBGP4_ROUTE *p_route)
 {
    u_int policy = 0;
    /*if protocol not connected,but there has same local route,ignore*/
    if (p_route->dest.afi == BGP4_PF_IPUCAST)
    {
        if (p_route->dest.prefixlen > 32)
        {
            bgp4_log(BGP_DEBUG_RT|BGP_DEBUG_ERROR,"The prefix len cannot be greater than 32.");
            return FALSE;
        }
    }
    
    if (p_route->dest.afi == BGP4_PF_IP6UCAST)
    {
        if ((p_route->dest.ip[0] == 0xfe)
            && (p_route->dest.ip[1] == 0x80))
        {
            bgp4_log(BGP_DEBUG_RT|BGP_DEBUG_ERROR,"IPV6 local link address should be filtered");
            return FALSE;
        }
        if (p_route->dest.ip[0] == 0xff)
        {
            bgp4_log(BGP_DEBUG_RT|BGP_DEBUG_ERROR,"IPV6 muti-address should be filtered");
            return FALSE;
        }
        if (p_route->dest.prefixlen > 128)
        {
            bgp4_log(BGP_DEBUG_RT|BGP_DEBUG_ERROR,"The IPV6 prefix len  be greater than 128,filtered",
                        p_route->dest.prefixlen);
            return FALSE;
        }
    }

    policy = bgp4_redistribute_policy_apply(p_instance, p_route);
    
    return (policy == BGP4_ROUTE_PERMIT) ? TRUE : FALSE;
}

void 
bgp4_network_up(tBGP4_NETWORK *p_network)
{
    tBGP4_PATH  path;
    tBGP4_ROUTE route; 

    memset(&route, 0, sizeof(route));
    memset(&path, 0, sizeof(path));
    bgp4_path_init(&path);

    memcpy(&route.dest, &p_network->net, sizeof(tBGP4_ADDR));
    path.af = route.dest.afi;
    route.proto = M2_ipRouteProto_other;
    
    route.p_path = &path;
    path.p_instance = p_network->p_instance;
    path.origin_vrf = p_network->p_instance->vrf;
    path.origin = BGP4_ORIGIN_IGP;
    
    bgp4_link_path(&path, &route);

    bgp4_redistribute_route(p_network->p_instance, &route, TRUE);
    return;
}

void 
bgp4_network_down(tBGP4_NETWORK *p_network)
{
    tBGP4_PATH  path;
    tBGP4_ROUTE route;

    memset(&route, 0, sizeof(route));
    memset(&path, 0, sizeof(path));
    bgp4_path_init(&path);

    memcpy(&route.dest, &p_network->net, sizeof(tBGP4_ADDR));
    path.af = route.dest.afi;
    route.proto = M2_ipRouteProto_other;
    
    route.p_path = &path;
    path.p_instance = p_network->p_instance;
    path.origin_vrf = p_network->p_instance->vrf;

    bgp4_link_path(&path, &route);

    bgp4_redistribute_route(p_network->p_instance, &route, FALSE);
    return;
}

int
bgp4_network_lookup_cmp(
    tBGP4_NETWORK *p1,
    tBGP4_NETWORK *p2)
{
    return bgp4_prefixcmp(&p1->net, &p2->net);
}

tBGP4_NETWORK *
bgp4_network_lookup(
      tBGP4_VPN_INSTANCE *p_instance,
      tBGP4_ADDR *p_dest)
{
    tBGP4_NETWORK network;
    memcpy(&network.net, p_dest, sizeof(tBGP4_ADDR));
    return bgp4_avl_lookup(&p_instance->network_table, &network);
}

tBGP4_NETWORK * 
bgp4_network_add(
      tBGP4_VPN_INSTANCE *p_instance,
      tBGP4_ADDR *p_dest)
{
    tBGP4_NETWORK *p_network = NULL;
    u_char net_str[64] = {0};
    p_network = bgp4_network_lookup(p_instance, p_dest) ;
    if (p_network)
    {
        return p_network;
    }

    p_network = bgp4_malloc(sizeof(tBGP4_NETWORK), MEM_BGP_NETWORK);
    if (!p_network)
    {
        return NULL;
    }
    
    memcpy(&p_network->net, p_dest, sizeof(tBGP4_ADDR));
    p_network->p_instance = p_instance;
    
    /*sorted by ipaddress*/
    bgp4_avl_add(&p_instance->network_table, p_network);

    bgp4_log(BGP_DEBUG_EVT,"add a net %s from vpn instance %d",
                bgp4_printf_addr(&p_network->net,net_str),
                p_instance->vrf);

    return p_network;
}

void 
bgp4_network_delete(tBGP4_NETWORK *p_network)
{
    tBGP4_VPN_INSTANCE *p_instance = p_network->p_instance;
    
    bgp4_network_down(p_network);

    bgp4_avl_delete(&p_instance->network_table, p_network);
    bgp4_free(p_network, MEM_BGP_NETWORK);
    return ;
}

void 
bgp4_network_delete_all(tBGP4_VPN_INSTANCE* p_instance)
{
    bgp4_avl_walkup(&p_instance->network_table, bgp4_network_delete);
    return;
}

/*import route from system table,containing mp support*/
void
bgp4_redistribute_route( 
     tBGP4_VPN_INSTANCE *p_instance,
     tBGP4_ROUTE *p_route, 
     u_int new_route/*TRUE:new route,FALSE:deleted route*/)
{
    /*set instance for safe*/
    p_route->p_path->p_instance = p_instance;
    
    /*set route's delete flag if necessary*/
    p_route->is_deleted = (new_route == TRUE) ? FALSE : TRUE;

    /*update rib input table of all dest instance*/
    bgp4_vrf_route_export_check(p_route, new_route);
    
    /*update rib input table of instance*/
    bgp4_rib_in_table_update(p_route);
    return;
}

#else
#include "bgp4com.h"

/*rtsock callback when route add.only applied to ipv4 unicast*/
void bgp4_rtsock_route_change(u_int vrf_id,tBGP4_ROUTE *p_route, u_int add)
{
    u_int tag = 0 ;
    tBGP4_VPN_INSTANCE* p_instance = NULL;

    p_instance = bgp4_vpn_instance_lookup(vrf_id);
    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 rtsock route change input instance %d does not exist",vrf_id);
        return;
    }

    /*ignore bgp route change orignated from bgp self*/
    if (p_route->proto == M2_ipRouteProto_bgp)
    {
        return ;
    }

    /*get self id*/
    if ((p_route->proto == M2_ipRouteProto_local) && (gBgp4.router_id == 0))
    {
        bgp4_get_max_ifipaddr(&gBgp4.router_id);
    }

    if (((tag&0xf0000000) == 0xA0000000)||
            (((tag&0xf0000000) == 0xE0000000)&&
            ((tag&0x0000ffff) == gBgp4.asnum))||
            ((tag&0x030000000) == 0x30000000))
    {
        return;
    }
    if ((tag&0x80000000) == 0)
    {
        p_route->p_path->origin = BGP4_ORIGIN_EGP;
    }
    if ((tag&0x80000000) == 0x80000000)
    {
        p_route->p_path->origin = BGP4_ORIGIN_EGP;
    }
    if ((tag&0xf0000000) == 0x90000000)
    {
        p_route->p_path->origin = BGP4_ORIGIN_EGP;
    }
    if ((tag&0xf0000000) == 0xc0000000)
    {

    }

    {
        u_char addrstr[64];
        u_char addrstr2[64];

        if(p_route->dest.afi==BGP4_PF_IPUCAST)
        {
            bgp4_log(BGP_DEBUG_CMN,1,"process redistribute route %s,nexthop %s",
                            bgp4_printf_route(p_route,addrstr),
                            bgp4_printf_addr(&p_route->p_path->nexthop, addrstr2));
        }
#ifdef BGP_IPV6_WANTED
        else if(p_route->dest.afi==BGP4_PF_IP6UCAST)
        {
            bgp4_log(BGP_DEBUG_CMN,1,"process redistribute route %s,nexthop %s",
                            bgp4_printf_route(p_route,addrstr),
                            bgp4_printf_addr(&p_route->p_path->nexthop_global, addrstr2));
        }
#endif
    }

    p_route->p_path->p_instance = p_instance;
    /*redistribute*/
    /*if can not be redistribute,stop*/
    if (add == FALSE /*route del should not check*/|| bgp4_redistribute_check_route(p_instance,p_route) >0)
    {
        bgp4_import_route(p_instance,p_route, add);
    }
    return ;
}

 /*check if a route can be redistributed,only applied to ipv4 unicast*/
 int   bgp4_redistribute_check_route(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_input)
 {
    u_int action=0;
    u_int ipv6_local_flag = 0xfe800000;

    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 redistribute check route input instance does not exist");
        return FALSE;
    }

    /*if protocol not connected,but there has same local route,ignore*/
    if(p_input->dest.afi==BGP4_PF_IPUCAST)
    {
        if (p_input->dest.prefixlen > 32)
        {
            bgp4_log(BGP_DEBUG_RT,1,"bgp4 redistribute check route prefix len overflow (len>32)");
            return FALSE;
        }
/*      mask = bgp4_len2mask(p_input->dest.prefixlen);

        if ((p_input->proto != M2_ipRouteProto_local) &&
                bgpIfaddrMatch(bgp_ip4(p_input->dest.ip), mask))
        {
            return FALSE ;
        }
*/
    }
    
#if 0
    /*network exist,do not add more*/
    if (bgp4_network_lookup(p_instance,&p_input->dest) != NULL)
    {
        bgp4_log(BGP_DEBUG_RT,1,"[bgp4_redistribute_check_route]:such route is advertised by NETWORK");
        return  FALSE;
    }
#endif

    if(p_input->dest.afi==BGP4_PF_IP6UCAST)
    {
        if(!memcmp(p_input->dest.ip,&ipv6_local_flag,2))
        {
            bgp4_log(BGP_DEBUG_RT,1,"bgp4 redistribute check route IPV6 link local address should be filtered");
            return FALSE;
        }
        if(p_input->dest.ip[0] == 0xff)
        {
            bgp4_log(BGP_DEBUG_RT,1,"bgp4 redistribute check route IPV6 muti address should be filtered");
            return FALSE;
        }
        if(p_input->dest.prefixlen  > 128)
        {
            bgp4_log(BGP_DEBUG_RT,1,"bgp4 redistribute check route IPV6 prefix length %d overflow,filtered",
                        p_input->dest.prefixlen);
            return FALSE;
        }
    }

    if(bgp4_check_import_policy(p_instance,p_input) == BGP4_ROUTE_PERMIT)
    {
        action = TRUE;
    }

    bgp4_log(BGP_DEBUG_RT,1,"bgp4 redistribute check route route proto %d,af %d,result %d(0:deny,1:permit)",
                p_input->proto,p_input->dest.afi,action);


    return action;
 }

INT1 bgp4_network_up(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR* p_net)
{
    tBGP4_PATH  path;
    tBGP4_ROUTE route;
    u_int mask = 0;

    if(p_instance == NULL || p_net == NULL)
    {
        return FALSE;
    }

    memset(&route, 0, sizeof(route));
    memset(&path, 0, sizeof(path));
    bgp4_lstnodeinit(&path.node);
    bgp4_lstinit(&path.aspath_list);
    bgp4_lstinit(&path.route_list);

    memcpy(&route.dest,p_net,sizeof(tBGP4_ADDR));
    path.afi = bgp4_index_to_afi(route.dest.afi);
    path.safi = bgp4_index_to_safi(route.dest.afi);
    route.proto = M2_ipRouteProto_other ;
    
    route.p_path = &path;
    path.p_instance = p_instance;

    return(bgp4_import_route(p_instance,&route, TRUE));
}

INT1 bgp4_network_down(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR* p_net)
{
    tBGP4_PATH  path;
    tBGP4_ROUTE route;

    if(p_instance == NULL || p_net == NULL)
    {
        return FALSE;
    }

    memset(&route, 0, sizeof(route));
    memset(&path, 0, sizeof(path));
    bgp4_lstnodeinit(&path.node);
    bgp4_lstinit(&path.aspath_list);
    bgp4_lstinit(&path.route_list);

    memcpy(&route.dest,p_net,sizeof(tBGP4_ADDR));
    path.afi = bgp4_index_to_afi(route.dest.afi);
    path.safi = bgp4_index_to_safi(route.dest.afi);
    route.proto = M2_ipRouteProto_other ;
    
    route.p_path = &path;
    path.p_instance = p_instance;

    return (bgp4_import_route(p_instance,&route, FALSE));
}

tBGP4_NETWORK *bgp4_network_lookup(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR* p_net)
{
    tBGP4_NETWORK *p_network = NULL;

    if(p_instance == NULL)
    {
        return NULL;
    }

    LST_LOOP(&p_instance->network, p_network, node, tBGP4_NETWORK)
    {
        if(bgp4_prefixcmp(p_net,&p_network->net) == 0)
        {
            return p_network;
        }
    }

    return NULL;
}

tBGP4_NETWORK * bgp4_add_network(tBGP4_VPN_INSTANCE* p_instance,tBGP4_ADDR* p_net)
{
    tBGP4_NETWORK *p_network = NULL;
    tBGP4_NETWORK *p_temp = NULL;
    tBGP4_NETWORK *p_prev = NULL;
    u_char net_str[64] = {0};

    if(p_instance == NULL)
    {
        return NULL;
    }

    p_network = bgp4_network_lookup(p_instance,p_net) ;
    if (p_network)
    {
        return p_network;
    }

    p_network = (tBGP4_NETWORK*)bgp4_malloc(sizeof(tBGP4_NETWORK), MEM_BGP_NETWORK);
    if (!p_network)
    {
        return NULL;
    }


    bgp4_lstnodeinit(&p_network->node);

    memcpy(&p_network->net,p_net,sizeof(tBGP4_ADDR));

    /*sorted by ipaddress*/
    LST_LOOP(&p_instance->network, p_temp, node, tBGP4_NETWORK)
    {
        if (bgp4_prefixcmp(&p_temp->net, &p_network->net) > 0)
        {
            break;
        }
        else
        {
            p_prev = p_temp;
        }
    }
    if (p_prev)
    {
        bgp4_lstinsert(&p_instance->network, &p_prev->node, &p_network->node);
    }
    else
    {
        bgp4_lstadd(&p_instance->network,  &p_network->node);
    }
    bgp4_log(BGP_DEBUG_EVT,1,"add a net %s from instance %d",
                bgp4_printf_addr(&p_network->net,net_str),
                p_instance->instance_id);

    return p_network;
}

void bgp4_delete_network(tBGP4_VPN_INSTANCE* p_instance,tBGP4_NETWORK *p_network)
{
    bgp4_lstdelete(&p_instance->network, &p_network->node);
    bgp4_free(p_network, MEM_BGP_NETWORK);
    return ;
}

void bgp4_delete_all_network(tBGP4_VPN_INSTANCE* p_instance)
{
    tBGP4_NETWORK *p_network = NULL;
    tBGP4_NETWORK *p_next = NULL;

    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_EVT,1,"bgp4 delete all network, instance is null");
        return ;
    }

    LST_LOOP_SAFE(&p_instance->network, p_network, p_next, node, tBGP4_NETWORK)
    {
        bgp4_network_down(p_instance,&p_network->net);
        bgp4_delete_network(p_instance,p_network);
    }

    return;
}

static int bgp4_update_import_route(
        u_int input_vrf_id,
        tBGP4_VPN_INSTANCE* p_dst_instance,
        tBGP4_ROUTE *p_input,
        tBGP4_ROUTE *p_rebuild_route,
        u_int action)
 {
    u_int af = 0;
    u_int i = 0;
    tBGP4_ROUTE_VECTOR vector;
    tBGP4_RIB* p_root = NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_current = NULL;
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_new_best = NULL;
    tBGP4_PATH * p_new_path = NULL;
    tBGP4_PATH *p_path = p_input->p_path ;
    u_char rt_str[64] = {0};
    u_char addr_str[64] = {0};
    u_char addr_str2[64] = {0};
    u_char zerobit[BGP4_MAX_PEER_ID/8] = {0};

    p_root = &p_dst_instance->rib;

    af = p_rebuild_route->dest.afi;

    /*lookup current route in rib*/
    p_current = bgp4_rib_lookup(&p_dst_instance->rib, p_rebuild_route);
    if(p_current)
    {
            u_char str1[64] = {0};
            u_char str2[64] = {0};
            bgp4_log(1,1,"\r\nbgp current route %s,nexthop %s",
                    bgp4_printf_route(p_current,str1),
                    bgp4_printf_addr(&p_current->p_path->nexthop,str2));
    }

    bgp4_rib_lookup_vector(p_root, &p_input->dest, &vector);

    if (action == TRUE)
    {
        /*get current best*/
        p_best = bgp4_best_route(&p_dst_instance->rib, &p_input->dest);
        
        /*if route exist,do nothing*/
        if (p_current)
        {
            if(p_current->is_deleted == TRUE)/*complete last deletion right now*/
            {
                bgp4_log(BGP_DEBUG_CMN,0,"Such import route %s is to be deleted,complete deletion now!",
                        bgp4_printf_route(p_current,rt_str));
                bgp4_force_ip_rib_update(p_dst_instance,p_current);
                bgp4_rib_delete(&p_dst_instance->rib, p_current);

            }
            else
            {
                bgp4_log(BGP_DEBUG_CMN,0,"Such import route %s has already exist in rib,do nothing!",
                        bgp4_printf_route(p_current,rt_str));
                return TRUE;
            }
        }

        /*no route,create imput route*/
        if(p_input == p_rebuild_route)/*no rebuilt*/
        {
            p_route = bgp4_creat_route();

            if (!p_route)
            {
                bgp4_log(BGP_DEBUG_CMN,0,"bgp4 malloc failed in bgp4 import route!");
                return FALSE;
            }

            memcpy(&p_route->dest, &p_input->dest, sizeof(p_input->dest));
            p_route->proto = p_input->proto;

            /*allocate path info*/
            p_new_path = bgp4_path_lookup(p_path);

            if (!p_new_path)
            {
                p_new_path = bgp4_add_path(p_route->dest.afi);
                if (!p_new_path)
                {
                    bgp4_log(BGP_DEBUG_CMN,0,"bgp4 add path failed in bgp4 importroute!");
                    bgp4_free(p_route, MEM_BGP_ROUTE);
                    return FALSE;
                }
                bgp4_path_copy(p_new_path, p_path);
                p_new_path->src_instance_id = input_vrf_id;
                p_new_path->p_instance = p_dst_instance;
            }

            /*link route and path info*/
            bgp4_link_path (p_new_path, p_route);

        }
        else/*rebuilt route need not to be created again*/
        {
            p_route = p_rebuild_route;
        }

        /*clear route's delete and withdraw flag*/
        p_route->is_deleted = FALSE ;
        p_route->ip_action = BGP4_IP_ACTION_NONE ;
        memset(p_route->withdraw_bits, 0, sizeof(p_route->withdraw_bits));
        memset(p_route->update_bits, 0, sizeof(p_route->update_bits));

        /*add to rib*/
        bgp4_rib_add(&p_dst_instance->rib, p_route);

        p_new_best = bgp4_best_route(&p_dst_instance->rib, &p_route->dest);

        if (p_new_best == p_route)
        {
            u_int flag = 0;
            for(i=0;i<vector.count;i++)
            {
                if(memcmp(vector.p_route[i]->withdraw_bits,zerobit,sizeof(vector.p_route[i]->withdraw_bits)))
                {                   
                    memset(vector.p_route[i]->withdraw_bits, 0, sizeof(vector.p_route[i]->withdraw_bits));
                    flag = 1;   
                    break;
                }

                if(memcmp(vector.p_route[i]->update_bits,zerobit,sizeof(vector.p_route[i]->update_bits)))
                {
                    flag = 1;
                    break;
                }
            }           
            /*send update to all other peer*/
            if(flag == 0)
                bgp4_schedule_rib_update(p_dst_instance, p_best, p_route, NULL);
            if(input_vrf_id != p_dst_instance->instance_id)
            {
                /*BGP answer for updating other vrf*/
                bgp4_schedule_ip_update(p_route,BGP4_IP_ACTION_ADD);
            }

            /*delete old route from kernal*/
            if (p_best)
            {
                bgp4_schedule_ip_update(p_best, BGP4_IP_ACTION_DELETE);
            }
        }
        else
        {
            bgp4_printf_addr(&p_route->dest,addr_str);
            if(p_new_best)
            {
                bgp4_printf_addr(&p_new_best->dest,addr_str2);
            }
            bgp4_log(BGP_DEBUG_EVT,1,"import route %s is not the best one,do not send it out!,the best one in rib is %s",addr_str,addr_str2);
        }
    }
    else /*delete*/
    {
        /*no such route,do nothing*/
        if (p_current == NULL)
        {
            bgp4_log(BGP_DEBUG_CMN,0,"Such import route %s  nexthop %s has not exist in rib,do nothing!",
                        bgp4_printf_route(p_rebuild_route,addr_str),
                        bgp4_printf_addr(&p_rebuild_route->p_path->nexthop,addr_str2));
            return TRUE;
        }

        p_best = bgp4_best_route(&p_dst_instance->rib, &p_rebuild_route->dest);/*get best route before delete*/

        p_current->is_deleted = TRUE ;

        p_new_best = bgp4_best_route(&p_dst_instance->rib, &p_rebuild_route->dest);

        /*update route only when best changed*/
        if (p_new_best != p_best)
        {
            u_int flag = 0;
            for(i=0;i<vector.count;i++)
            {
                if(memcmp(vector.p_route[i]->update_bits,zerobit,sizeof(vector.p_route[i]->update_bits)))
                {
                    memset(vector.p_route[i]->update_bits, 0, sizeof(vector.p_route[i]->update_bits));
                    flag = 1;   
                    break;
                }
                
                if(memcmp(vector.p_route[i]->withdraw_bits,zerobit,sizeof(vector.p_route[i]->withdraw_bits)))
                {
                    flag = 1;
                    break;
                }
            }           
            /*send update to all other peer*/
            if(flag == 0)           
                bgp4_schedule_rib_update(p_dst_instance,p_best,p_new_best,NULL);/*new best is null,send withdraw to all peer,aggregate considered here*/
            if(input_vrf_id && p_dst_instance->instance_id && input_vrf_id != p_dst_instance->instance_id)
            {
                /*BGP answer for updating other vrf*/

                /*prepare to delete ip route of old best*/
                bgp4_schedule_ip_update(p_best, BGP4_IP_ACTION_DELETE);
            }
            if (p_new_best)
            {
                /*update ip table,add newbest's ip route*/
                bgp4_schedule_ip_update(p_new_best, BGP4_IP_ACTION_ADD);
            }
        }
        /*rebuild route should release*/
        if(p_rebuild_route != p_input)
        {
            bgp4_release_route(p_rebuild_route);
        }

    }

    return TRUE;
 }

/*import route from system table,containing mp support*/
int   bgp4_import_route( tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE *p_input, u_int action)
{
    u_char addr_str[64] = {0};
    u_int input_vrf_id = 0;
    u_int vrf_id_array[BGP4_MAX_VRF_ID] = {0};
    u_int vrf_num = 1;
    u_int i = 0;
    u_int direction = 0;
    tBGP4_VPN_INSTANCE* p_dst_instance = NULL;
    tBGP4_ROUTE *p_rebuild_route = NULL;

    if(p_instance == NULL || p_input == NULL)
    {
        bgp4_log(BGP_DEBUG_CMN,0,"Such import instance does not  exist!");
        return FALSE;
    }

    input_vrf_id = p_instance->instance_id;


    vrf_num = bgp4_mpls_vpn_get_vrf_id_list(p_input,1,p_instance,&direction,vrf_id_array);

    for(i = 0;i < vrf_num;i++)
    {

        p_dst_instance = bgp4_vpn_instance_lookup(vrf_id_array[i]);
        if(p_dst_instance == NULL)
        {
            continue;
        }

        /*6pe route should update in common way*/
        if(direction == MPLS_6PE_LOCAL_ROUTE)
        {
            if(bgp4_update_import_route(input_vrf_id,p_dst_instance,p_input,p_input,action) == FALSE)
            {
                return FALSE;
            }
        }

        p_rebuild_route = NULL;


        p_rebuild_route = bgp4_mpls_rebuild_local_vpn_route(input_vrf_id,direction,p_dst_instance,p_input);


        if(p_rebuild_route == NULL)
        {
            bgp4_log(BGP_DEBUG_EVT,0,"rebuild import route failed");
            continue;
        }

        bgp4_log(BGP_DEBUG_EVT,1,"input instance %d route %s to dest instance %d",
                    input_vrf_id,
                    bgp4_printf_route(p_rebuild_route,addr_str),
                    p_dst_instance->instance_id);

        if(bgp4_update_import_route(input_vrf_id,p_dst_instance,p_input,p_rebuild_route,action) == FALSE)
        {
            return FALSE;
        }


    }

    return (TRUE);
}

#endif
