#include "bgp4mplsvpn.h"
#include "bgp4_api.h"
#ifdef NEW_BGP_WANTED
/*****************************************************************************
  bgp4mplsvpn.c :for processing MPLS VPN route  only
******************************************************************************/

#include "bgp4com.h"
extern tBGP4_GLOBAL gBgp4 ;
#ifndef BGP4_VPN_WANTED
#define mpls_l3vpn_get_label(x,y) 0
#define mpls_l3vpn_label_route_notify(x,y) VOS_ERR
#endif

/*export a route into target instance,may add or delete route in
  dest vrf*/
void
bgp4_vrf_route_export(
        tBGP4_ROUTE *p_route,
        tBGP4_VPN_INSTANCE *p_instance,
        u_int feasible)
{
    tBGP4_ROUTE new_route;
    tBGP4_PATH new_path;

    /*orign and target are same,only 6pe supported*/
    if ((p_instance == p_route->p_path->p_instance)
        && (p_route->dest.afi != BGP4_PF_IP6UCAST)
        && (p_route->dest.afi != BGP4_PF_IP6LABEL))
    {
        return;
    }

    /*6pe checking:must be enabled*/
    if ((feasible == TRUE)
        && (p_instance == p_route->p_path->p_instance)
        && (p_route->dest.afi == BGP4_PF_IP6UCAST)
        && (p_instance->v6pe_count == 0))
    {
        return;
    }
    
    memset(&new_route, 0, sizeof(new_route));
    memset(&new_path, 0, sizeof(new_path));
    bgp4_path_init(&new_path);
    bgp4_path_copy(&new_path, p_route->p_path);
    new_path.p_instance = p_instance;
    bgp4_link_path(&new_path, &new_route);
    
    bgp4_vrf_route_translate(p_route, &new_route, feasible);
    
    new_route.is_deleted = !feasible;
    
    bgp4_rib_in_table_update(&new_route);
    
    bgp4_path_clear(&new_path);
    return;
}

/*translate normal route from vpn-instance into
  vpn format route in public or other vpn instance*/
void
bgp4_vrf_route_translate(
        tBGP4_ROUTE *p_route,
        tBGP4_ROUTE *p_dest,
        u_int feasible)
{
    tBGP4_VPN_INSTANCE *p_instance_from = p_route->p_path->p_instance;
    tBGP4_VPN_INSTANCE *p_instance = p_dest->p_path->p_instance;
    tBGP4_PATH *p_path = p_route->p_path;
    tBGP4_PATH *p_path_dest = p_dest->p_path;
    tBgpLookupL3Vpn lookup_msg;
    int rc = TRUE;
    u_int i = 0;
    u_int rtlen = BGP4_VPN_RT_LEN;

    /*case:export local vpn route to another vpn instance*/
    if (p_path->origin_vrf 
        && (p_path->origin_vrf == p_instance_from->vrf)
        && (p_instance != p_instance_from)
        && (p_instance->vrf != 0))
    {
        /*remain route format*/
        memcpy(&p_dest->dest, &p_route->dest, sizeof(tBGP4_ADDR));
    }

    /*case:export local vpn route to public instance,it will
      be translated into ipvpn format,and obatin label
      ,rd,rt attribute*/
    if (p_path->origin_vrf 
        && (p_path->origin_vrf == p_instance_from->vrf)
        && (p_instance != p_instance_from)
        && (p_instance->vrf == 0))
    {
        /*send to remote pe using public instance*/
         /*fill rd of vpn instance*/  
         memcpy(p_dest->dest.ip, p_instance_from->rd, BGP4_VPN_RD_LEN);

         /*fill dest*/  
         memcpy(p_dest->dest.ip + BGP4_VPN_RD_LEN, p_route->dest.ip, 16);

         p_dest->dest.prefixlen = p_route->dest.prefixlen + 8*BGP4_VPN_RD_LEN;
         p_dest->dest.afi = (p_route->dest.afi == BGP4_PF_IPUCAST) ? BGP4_PF_IPVPN : BGP4_PF_IP6VPN;

         /*put assigned label on new route*/       
         //p_dest->in_label = mpls_l3vpn_get_label(p_instance_from->vrf, 0);
         /*move 4bits shift*/
         //p_dest->in_label = p_dest->in_label >> 4;
         p_dest->proto = p_route->proto;

         /*only get target attribute for feasible route*/
         if (feasible)
         {
             bgp_label_handle(BGP4_GET_MPLS_LABEL, &p_dest->in_label);
             /*get route target attribute*/
             memset(&lookup_msg, 0, sizeof(tBgpLookupL3Vpn));
             lookup_msg.type = MPLSL3VPN_GET_EXPORTRT;
             lookup_msg.inVrfId = p_instance_from->vrf;
             lookup_msg.maxNum = BGP4_MAX_ECOMMNUITY;
             lookup_msg.outData = (tBgpL3VpnInfo *)bgp4_malloc(sizeof(tBgpL3VpnInfo)*BGP4_MAX_ECOMMNUITY, MEM_BGP_BUF);
             if(NULL == lookup_msg.outData)
             {
                bgp4_log(BGP_DEBUG_MPLS_VPN,"outdata of lookup msg  is null\n");
                return;
             }
             rc = mplsLookupL3Vpn(&lookup_msg);
             if (rc == VOS_ERR)
             {
                 bgp4_free(lookup_msg.outData, MEM_BGP_BUF);
                 return;
             }
             /*copy ext-community*/
             if (lookup_msg.count)
             {
                 if (p_path_dest->p_ecommunity)
                 {
                     bgp4_free(p_path_dest->p_ecommunity, MEM_BGP_BUF);
                 }
                 
                 p_path_dest->p_ecommunity = 
                          bgp4_malloc(lookup_msg.count * rtlen, MEM_BGP_BUF);
                 if (p_path_dest->p_ecommunity)
                 {
                     for (i = 0; i < lookup_msg.count; i++)
                     {
                         memcpy(p_path_dest->p_ecommunity + (i * rtlen), lookup_msg.outData[i].vrfRT, rtlen);
                     }
                     p_path_dest->excommunity_len = lookup_msg.count * rtlen;
                 }
            }
            bgp4_free(lookup_msg.outData, MEM_BGP_BUF);
        }
    }

    /*case:route is ipvpn route in public instance,exported
      into special vrf*/
    if ((p_path->origin_vrf == 0)
        && (p_path->origin_vrf == p_instance_from->vrf)
        && (p_instance != p_instance_from)
        && (p_instance->vrf != 0)
        /*&& (p_route->out_label != 0)*/)
    {
        /*route is learnt from remote pe,wille be exported to vpn*/
        memcpy(p_dest->dest.ip, p_route->dest.ip + BGP4_VPN_RD_LEN, 16);
        p_dest->dest.prefixlen = p_route->dest.prefixlen - 8*BGP4_VPN_RD_LEN;
        p_dest->dest.afi = (p_route->dest.afi == BGP4_PF_IPVPN) ? BGP4_PF_IPUCAST: BGP4_PF_IP6UCAST;
        p_dest->out_label = p_route->out_label;
        p_dest->in_label = p_route->in_label;
    }

    /*case:route is local ipv6 route,exported to public route as
      6pe*/
    if ((p_path->origin_vrf == 0)
        && (p_path->origin_vrf == p_instance_from->vrf)
        && (p_instance == p_instance_from)
        && (p_route->dest.afi == BGP4_PF_IP6UCAST))
    {
        memcpy(&p_dest->dest, &p_route->dest, sizeof(tBGP4_ADDR));
        p_dest->dest.afi = BGP4_PF_IP6LABEL;
        
        /*put assigned label to new route*/
        //p_dest->in_label = mpls_l3vpn_get_label(p_instance_from->vrf, 0);
        bgp_label_handle(BGP4_GET_MPLS_LABEL, &p_dest->in_label);
        /*move 4bits shift less*/
        //p_dest->in_label = p_dest->in_label >> 4;
    }

    /*case:route is ipv6 route with label,change to normal route*/
    if ((p_path->origin_vrf == 0)
        && (p_path->origin_vrf == p_instance_from->vrf)
        && (p_instance == p_instance_from)
        && (p_route->dest.afi == BGP4_PF_IP6LABEL))
    {
        memcpy(&p_dest->dest, &p_route->dest, sizeof(tBGP4_ADDR));
        p_dest->dest.afi = BGP4_PF_IP6UCAST;
        p_dest->out_label = p_route->out_label;
        p_dest->in_label = p_route->in_label;
    }
    p_dest->proto = p_route->proto;

    p_path_dest->af = p_dest->dest.afi;
    return;
}

/*update system vpn route with label*/
int 
bgp4_sys_mpls_route_update(tBGP4_ROUTE *p_route)
{
    tMplsVpnRouteEntry vpn_route;
    u_int vrf = p_route->p_path->p_instance->vrf;
    u_int afi = 0;
    u_char action = 0;
    int rc = 0;
    u_char rt_str[64];
    u_char nstr[64];
    
    /*only remote route with label considered*/
    if (p_route->out_label == 0)
    {
        return VOS_OK;
    }

    /*vpn routes int common vrf need not be notified*/
    if ((p_route->dest.afi != BGP4_PF_IPUCAST)
        && (p_route->dest.afi != BGP4_PF_IP6UCAST))
    {
        return VOS_OK;
    }

    if (p_route->system_selected == TRUE)
    {
        action = MPLSL3VPN_ADD_ROUTE;
        gbgp4.stat.mpls_route.add++;
    }
    else
    {
        action = MPLSL3VPN_DEL_ROUTE;
        gbgp4.stat.mpls_route.delete++;
    }

    bgp4_log(BGP_DEBUG_MPLS_VPN, "%s vpn route %s,vrf %d",
            (action == MPLSL3VPN_ADD_ROUTE) ? "add" : "delete",
            bgp4_printf_route(p_route,rt_str),vrf);

    memset(&vpn_route, 0, sizeof(vpn_route));
    #if 0
    typedef struct
    {
        u_char dest_type;
        u_char next_hop_type;
        u_char route_direction;
        u_char dest[24];
        u_char next_hop[24];
        u_int mask;
    }tMplsVpnRouteEntry;
    #endif
    /*fill route dest (not include RD)*/
    afi = bgp4_index_to_afi(p_route->dest.afi);
    vpn_route.dest_type = (afi == BGP4_AF_IP) ? AF_INET : AF_INET6;
    memcpy(vpn_route.dest, p_route->dest.ip, 16);
    
    /*fill nexthop.6pe route's vrf is 0,other vpn's vrf must not be 0*/
    if ((afi == BGP4_AF_IP6) || (vrf == 0))
    {
        vpn_route.next_hop_type = AF_INET;/*MPLS ONLY SUPPORT IPV4,SO FAR*/
        memcpy(vpn_route.next_hop, p_route->p_path->nexthop.ip + 12, 4);
    }
    else
    {
        vpn_route.next_hop_type = vpn_route.dest_type;
        memcpy(vpn_route.next_hop, p_route->p_path->nexthop.ip, 16);
        if(p_route->p_path->p_peer)
        {
            memcpy(vpn_route.tunnel_source, p_route->p_path->p_peer->local_ip.ip,4);
        }
    }

    if (afi == BGP4_AF_IP)
    {
        vpn_route.mask = bgp4_len2mask(p_route->dest.prefixlen);
        
        vpn_route.route_direction = MPLSL3VPN_ROUTE_REMOTE;
    }
    else
    {
        vpn_route.mask = p_route->dest.prefixlen;
        if (vrf != 0)
        {
            vpn_route.route_direction = MPLSL3VPN_ROUTE_REMOTE;
        }
        else
        {
            vpn_route.route_direction = MPLS_6PE_REMOTE_ROUTE;
        }
    }

    vpn_route.vrf_id = vrf;

    vpn_route.vc_label = p_route->out_label/*(p_route->out_label << 4) | 0x00000001*/;
    if(p_route->p_path->p_peer)
    {
        vpn_route.uiIndex = p_route->p_path->p_peer->if_unit;
    }
    
    if (vpn_route.dest_type == AF_INET)
    {
        bgp4_log(BGP_DEBUG_MPLS_VPN,
                    "vpn route type %d,nexthop %#x, tunnel source %#x, label %d out Index %x",
                    vpn_route.route_direction,
                    bgp_ip4(vpn_route.next_hop),
                    bgp_ip4(vpn_route.tunnel_source),
                    vpn_route.vc_label, vpn_route.uiIndex);
    }
    else
    {
        if (vpn_route.next_hop_type == AF_INET6)
        {
            inet_ntop(AF_INET6, vpn_route.next_hop, nstr, 64);
        }
        else
        {
            sprintf(nstr, "%d.%d.%d.%d", 
                    vpn_route.next_hop[0],
                    vpn_route.next_hop[1],
                    vpn_route.next_hop[2],
                    vpn_route.next_hop[3]);
        }
        bgp4_log(BGP_DEBUG_MPLS_VPN,
                    "vpn route type %d,nexthop %s,label %d",
                    vpn_route.route_direction,
                    nstr,
                    vpn_route.vc_label);
    }

    //rc = mpls_l3vpn_label_route_notify(&vpn_route, action);
    rc = bgp4_l3vpn_label_set(vpn_route, action);
    if (rc == VOS_OK)
    {
        return VOS_OK;
    }
    
    bgp4_log(BGP_DEBUG_MPLS_VPN|BGP_DEBUG_ERROR, "vpn update failed %d", rc);

    if (action == MPLSL3VPN_ADD_ROUTE)
    {
        gbgp4.stat.mpls_route.fail++;
        gbgp4.stat.mpls_route.add_error = rc;
    }
    else
    {
        gbgp4.stat.mpls_route.delete_fail++;
        gbgp4.stat.mpls_route.delete_error = rc;
    }
    #if 0
    /*retry notification in such cases below only*/
    if (rc == ETIME || rc == EUSP_NOPROCESS) /*待确认修改错误码判断*/
    {
        return VOS_ERR;
    }
    #endif
    return VOS_OK;
}

/*decide route's target vrf,it will be exported to these vrf*/
u_int 
bgp4_route_dest_vrf_get(
             tBGP4_ROUTE *p_route,
             u_int *p_vrf)
{
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_PATH *p_path = NULL; 
    tBgpLookupL3Vpn lookup_msg = {0};
    u_int safi = bgp4_index_to_safi(p_route->p_path->af);
    u_int count = 0;
    int rc = VOS_ERR;
    u_int i = 0;
    u_int k = 0;
    
    /*any routes should be distributed in self instance*/
    p_vrf[count++] = p_instance->vrf;

    /*6PE route's target vrf only be self original vrf*/
    if (p_route->dest.afi == BGP4_PF_IP6LABEL)
    {
        return count;
    }
    
    if (p_instance->vrf != 0)
    {
        memset(&lookup_msg, 0, sizeof(tBgpLookupL3Vpn));
        lookup_msg.type = MPLSL3VPN_GET_LOCALVRF;
        lookup_msg.inVrfId = p_instance->vrf;
        lookup_msg.maxNum = BGP4_MAX_VRF_ID;
        lookup_msg.outData = (tBgpL3VpnInfo *)bgp4_malloc(sizeof(tBgpL3VpnInfo)*BGP4_MAX_VRF_ID, MEM_BGP_BUF);

        rc = mplsLookupL3Vpn(&lookup_msg);
        for (i = 0; i < lookup_msg.count; i++)
        {
            p_vrf[count++] = lookup_msg.outData[i].vrfId;
        }
        
        /*public instance should always be the last one*/
        p_vrf[count++] = 0;
        
        bgp4_free(lookup_msg.outData, MEM_BGP_BUF);
    }
    else if (safi == BGP4_SAF_VLBL)/*mpls vpn,from remote PE,public instance*/
    {
        p_path = p_route->p_path;

        memset(&lookup_msg, 0, sizeof(tBgpLookupL3Vpn));
        lookup_msg.outData = (tBgpL3VpnInfo *)bgp4_malloc(sizeof(tBgpL3VpnInfo)*BGP4_MAX_VRF_ID, MEM_BGP_BUF);
        /*check ext-community in path,get matched vrf*/
        for (i = 0; p_path->p_ecommunity && (i < p_path->excommunity_len/BGP4_VPN_RT_LEN); i++)
        {
            memcpy(lookup_msg.inRT, p_path->p_ecommunity + (i*BGP4_VPN_RT_LEN),BGP4_VPN_RT_LEN);
            lookup_msg.type = MPLSL3VPN_GET_REMOTEVRF;
            lookup_msg.maxNum = BGP4_MAX_VRF_ID;
            lookup_msg.count = 0;
            
            /*must be route target extend community*/
            if (lookup_msg.inRT[1] != 2)
            {
                continue;
            }
            rc = mplsLookupL3Vpn(&lookup_msg);
            if ((lookup_msg.count == 0) || (rc == VOS_ERR))
            {
                gbgp4.stat.vrf_not_exist++;
                continue;
            }

            for (k = 0; k < lookup_msg.count; k++)
            {
                if (count <= BGP4_MAX_VRF_ID)
                {
                    p_vrf[count++] = lookup_msg.outData[k].vrfId;
                }
            }
        }
        bgp4_free(lookup_msg.outData, MEM_BGP_BUF);        
    }
    return count;
}

/*import rt add,vpn routes may be increased*/
void 
bgp4_rtsock_import_rtarget_add(tBGP4_VPN_INSTANCE *p_vpn)
{
    tBGP4_VPN_INSTANCE *p_public = NULL;
    tBGP4_ROUTE *p_route = NULL;
    u_int af = BGP4_PF_IPVPN;
    
    gbgp4.stat.import_rt_add++;
    p_public = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, 0);
    if (p_public == NULL)
    {
        bgp4_log(BGP_DEBUG_MPLS_VPN, "public instance is null!");
        return;
    }

    if (p_public->rib[af] == NULL)
    {
        return;
    }

    /*scan all vpn route in public instance*/
    bgp4_avl_for_each(&p_public->rib[af]->rib_table, p_route)
    {
        /*decide if route can be exported to this instance*/
        if (p_route->p_path->origin_vrf == p_vpn->vrf)
        {
            continue;
        }
        if (bgp4_vrf_route_export_enable(p_route, p_vpn) == TRUE)
        {
            bgp4_vrf_route_export(p_route, p_vpn, TRUE);
        }
    }
    return;
}

/*loop such instance rib,send update to PEs using renewed RTs && local other vpn instance routes may increased*/
void 
bgp4_rtsock_export_rtarget_add(tBGP4_VPN_INSTANCE *p_vpn)
{
    tBGP4_ROUTE *p_route = NULL;
    u_int af = BGP4_PF_IPUCAST;
    
    gbgp4.stat.export_rt_add++;

    if (p_vpn->rib[af] == NULL)
    {
        return;
    }

    /*refresh local vpn instance routes to remote and local target instances*/
    bgp4_avl_for_each(&p_vpn->rib[af]->rib_table, p_route)
    {
        /*only original routes should be refreshed*/
        if (p_route->p_path->origin_vrf != p_vpn->vrf)
        {
            continue;
        }
        bgp4_vrf_route_export_check(p_route, TRUE);
    }
    return;
}

/*loop such instance rib,send update to PEs using renewed RTs && local other vpn instance routes may decreased*/
void 
bgp4_rtsock_export_rtarget_delete(tBGP4_VPN_INSTANCE *p_vpn)
{
    tBGP4_VPN_INSTANCE *p_dest = NULL;
    tBGP4_ROUTE *p_route = NULL;
    u_int af = BGP4_PF_IPUCAST;
    
    gbgp4.stat.export_rt_delete++;

    if (p_vpn->rib[af] == NULL)
    {
        return;
    }

    /*refresh local vpn instance routes to remote and local target instances*/
    bgp4_avl_for_each(&p_vpn->rib[af]->rib_table, p_route)
    {
        /*only original routes should be refreshed*/
        if (p_route->p_path->origin_vrf != p_vpn->vrf)
        {
            continue;
        }
        bgp4_vrf_route_export_check(p_route, TRUE);

        bgp4_instance_for_each(p_dest)
        {
            if ((p_dest == p_vpn) 
                || (p_dest->vrf == 0))
            {
                continue;
            }
            if (bgp4_vrf_route_export_enable(p_route, p_dest) == FALSE)
            {
                bgp4_vrf_route_export(p_route, p_dest, FALSE);
            }
        }
    }
    return;
}

void 
bgp4_rtsock_import_rtarget_delete(tBGP4_VPN_INSTANCE *p_vpn)
{
    tBGP4_VPN_INSTANCE *p_public_instance = NULL;
    tBGP4_ROUTE *p_route = NULL;
    u_int af = BGP4_PF_IPVPN;
    
    gbgp4.stat.import_rt_add++;
    p_public_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, 0);
    if (p_public_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_MPLS_VPN, "public instance is null!");
        return;
    }

    if (p_public_instance->rib[af] == NULL)
    {
        return;
    }

    /*scan all vpn route in public instance*/
    bgp4_avl_for_each(&p_public_instance->rib[af]->rib_table, p_route)
    {
        /*decide if route can be exported to this instance*/
        if (p_route->p_path->origin_vrf == p_vpn->vrf)
        {
            continue;
        }
        if (bgp4_vrf_route_export_enable(p_route, p_vpn) == FALSE)
        {
            bgp4_vrf_route_export(p_route, p_vpn, FALSE);
        }
    }
    return;
}

int 
bgp4_init_private_instance(void)
{
    tBgpLookupL3Vpn lookup_msg;
    tBGP4_VPN_INSTANCE *p_instance;
    u_int i = 0;
    u_int vrf = 0;

    memset(&lookup_msg,0,sizeof(tBgpLookupL3Vpn));
    lookup_msg.type = MPLSL3VPN_GET_VIDANDRD;
    lookup_msg.maxNum = MAX_LOCAL_VRF;
    lookup_msg.outData = (tBgpL3VpnInfo *)bgp4_malloc(sizeof(tBgpL3VpnInfo)*BGP4_MAX_VRF_ID, MEM_BGP_BUF);

    if (mplsLookupL3Vpn(&lookup_msg) == VOS_ERR)
    {
        bgp4_free(lookup_msg.outData, MEM_BGP_BUF);
        return VOS_ERR;
    }
    for (i = 0; i < lookup_msg.count; i++)
    {
        vrf = lookup_msg.outData[i].vrfId;
        p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, vrf);
        if (p_instance == NULL)
        {
            p_instance = bgp4_vpn_instance_create(BGP4_INSTANCE_IP, vrf);
            if (p_instance)
            {
                /*copy vrf rd*/
                memcpy(p_instance->rd, lookup_msg.outData[i].vrfRT, sizeof(p_instance->rd));
                
                /*newly created instance only need refresh remote vpn routes from PEs*/
                bgp4_rtsock_import_rtarget_add(p_instance);
            }
        }
    }
    bgp4_free(lookup_msg.outData, MEM_BGP_BUF);
    return VOS_OK;
}

int 
bgp4_init_one_private_instance(u_int uiVrfId)
{
    tBgpLookupL3Vpn lookup_msg;
    tBGP4_VPN_INSTANCE *p_instance;
    u_int i = 0;
    u_int vrf = 0;

    memset(&lookup_msg,0,sizeof(tBgpLookupL3Vpn));
    lookup_msg.type = MPLSL3VPN_GET_RD_BY_VRFID;
    lookup_msg.inVrfId = uiVrfId;
    lookup_msg.maxNum = MAX_LOCAL_VRF;
    lookup_msg.outData = (tBgpL3VpnInfo *)bgp4_malloc(sizeof(tBgpL3VpnInfo), MEM_BGP_BUF);

    if (mplsLookupL3Vpn(&lookup_msg) == VOS_ERR)
    {
        bgp4_free(lookup_msg.outData, MEM_BGP_BUF);
        return VOS_ERR;
    }

    p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, uiVrfId);
    if (p_instance == NULL)
    {
        p_instance = bgp4_vpn_instance_create(BGP4_INSTANCE_IP, uiVrfId);
    }
    else
    {
        bgp4_free(lookup_msg.outData, MEM_BGP_BUF);
        return VOS_OK;        
    }

    if (p_instance)
    {
        /*copy vrf rd*/
        memcpy(p_instance->rd, lookup_msg.outData[0].vrfRT, sizeof(p_instance->rd));
        p_instance->enable_state = 1;
        /*newly created instance only need refresh remote vpn routes from PEs*/
        bgp4_rtsock_import_rtarget_add(p_instance);
    }

    bgp4_free(lookup_msg.outData, MEM_BGP_BUF);
    return VOS_OK;
}

#ifdef BGP_VPLS_WANTED 
void
bgp4_vpls_route_export(
        tBGP4_ROUTE *p_route,
        tBGP4_VPN_INSTANCE *p_instance,
        u_int feasible)
{
    tBGP4_ROUTE newroute;
    tBGP4_PATH path;

    memset(&newroute, 0, sizeof(newroute));
    /*copy dest*/
    memcpy(&newroute.dest, &p_route->dest, sizeof(p_route->dest));
    newroute.proto = p_route->proto;
    newroute.is_deleted = p_route->is_deleted;
    
    /*build a path*/
    memset(&path, 0, sizeof(path));
    bgp4_path_init(&path);
    
    /*accept fields from source route*/
    bgp4_path_copy(&path, p_route->p_path);

    /*change vrf instance*/
    path.p_instance = p_instance;
    path.origin_vrf = p_route->p_path->p_instance->vrf;
    path.af = BGP4_PF_L2VPLS;
    /*no nbr exist*/
    path.p_peer = NULL;

    bgp4_link_path(&path, &newroute);

    newroute.is_deleted = !feasible;
    
    bgp4_rib_in_table_update(&newroute);
    
    bgp4_path_clear(&path);
    return;
}

/*translate vpls route address into vpls address*/
void
bgp4_vpls_addr_get(
         tBGP4_ADDR *p_addr,
         tBGP4_VPLS_ADDR *p_vpls_addr)
{
    u_char *p_buf = p_addr->ip;
    /*   +------------------------------------+
      |  Length (2 octets)                 |--00 01 == 17
      +------------------------------------+
      |  Route Distinguisher  (8 octets)   | == 00 00 00 64 00 00 00 01 ---RD
      +------------------------------------+
      |  VE ID (2 octets)                  | 00 02 ID为2
      +------------------------------------+
      |  VE Block Offset (2 octets)        |00 00 Offset为0
      +------------------------------------+
      |  VE Block Size (2 octets)          |00 0a BLOCK
      +------------------------------------+
      |  Label Base (3 octets)             |18 6a 11 ---和普通标记封装一样，都是增加了4比特偏移
      +------------------------------------+*/
   memset(p_vpls_addr, 0, sizeof(tBGP4_VPLS_ADDR));
    
   /*skip rd*/
   p_buf += BGP4_VPN_RD_LEN;

   /*get veid*/
   bgp4_get_2bytes(p_vpls_addr->ve_id, p_buf);
   p_buf += 2;

   /*get offset*/
   bgp4_get_2bytes(p_vpls_addr->ve_block_offset, p_buf);
   p_buf += 2;

   /*get size*/   
   bgp4_get_2bytes(p_vpls_addr->ve_block_size, p_buf);
   p_buf += 2;

   /*get label*/
   p_vpls_addr->label_base = bgp4_label_extract(p_buf);
   
   return;
}

/*translate vpls address into route nlri*/
void
bgp4_vpls_addr_set(
         tBGP4_ADDR *p_addr,
         u_char *rd,
         tBGP4_VPLS_ADDR *p_vpls_addr)
{
    u_char *p_buf = p_addr->ip;
    /*   +------------------------------------+
      |  Length (2 octets)                 |--00 01 == 17
      +------------------------------------+
      |  Route Distinguisher  (8 octets)   | == 00 00 00 64 00 00 00 01 ---RD
      +------------------------------------+
      |  VE ID (2 octets)                  | 00 02 ID为2
      +------------------------------------+
      |  VE Block Offset (2 octets)        |00 00 Offset为0
      +------------------------------------+
      |  VE Block Size (2 octets)          |00 0a BLOCK
      +------------------------------------+
      |  Label Base (3 octets)             |18 6a 11 ---和普通标记封装一样，都是增加了4比特偏移
      +------------------------------------+*/
   memset(p_addr, 0, sizeof(tBGP4_ADDR));

   p_addr->afi = BGP4_PF_L2VPLS;
   
   /*copy rd*/
   memcpy(p_buf, rd, BGP4_VPN_RD_LEN);
   p_buf += BGP4_VPN_RD_LEN;

   /*get veid*/
   bgp4_fill_2bytes(p_buf, p_vpls_addr->ve_id);
   p_buf += 2;

   /*get offset*/
   bgp4_fill_2bytes(p_buf, p_vpls_addr->ve_block_offset);
   p_buf += 2;

   /*get size*/   
   bgp4_fill_2bytes(p_buf, p_vpls_addr->ve_block_size);
   p_buf += 2;

   /*get label*/
   bgp4_label_fill(p_buf, p_vpls_addr->label_base);
   return;
}

void
bgp4_sys_vpls_xc_build(
             tBGP4_ROUTE *p_route,
             MPLS_BGP_VPLS *p_xc)
{
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_ROUTE *p_local_route = NULL;
    tBGP4_VPLS_ADDR vpls;
    
    /*get label information*/
    bgp4_vpls_addr_get(&p_route->dest, &vpls);
    
    memset(p_xc, 0, sizeof(*p_xc));
    p_xc->vsi = p_instance->vrf;
    p_xc->in_label = p_instance->local_nlri.label_base + 
                   p_instance->local_nlri.ve_id - 
                   p_instance->local_nlri.ve_block_offset;
    
    p_xc->out_label = vpls.label_base + vpls.ve_id - vpls.ve_block_offset;
    
    p_xc->next_hop_type = AF_INET;
    memcpy(&p_xc->nexthop, p_route->p_path->nexthop.ip, 4);
    
    return;
}

/*add system route for vpls*/
STATUS
bgp4_sys_vpls_xc_add(tBGP4_ROUTE *p_route)
{
    MPLS_BGP_VPLS msg;
    if (p_route->p_path->p_instance->vrf == 0)
    {
        return VOS_OK;
    }
    
    bgp4_sys_vpls_xc_build(p_route, &msg);
    return mpls_bgp_vpls_add(&msg, p_route);
}

STATUS
bgp4_sys_vpls_xc_delete(tBGP4_ROUTE *p_route)
{
    MPLS_BGP_VPLS msg;
    if (p_route->p_path->p_instance->vrf == 0)
    {
        return VOS_OK;
    }
    
    bgp4_sys_vpls_xc_build(p_route, &msg);
    return mpls_bgp_vpls_del(&msg, p_route);
}

/*send route msg for vpls*/
STATUS 
bgp4_sys_vpls_msg_send(
            tBGP4_ROUTE *p_route,
            u_int active)
{
    MPLS_BGP_VPLS xc;
    struct l2vpnVcLabel_msghdr *p_hdr = NULL;
    struct l2vpnVcLabel_msg *p_msg = NULL;
    u_char buf[RT_MSG_MAXLEN] = {0};
    u_int len = 0;
    int error = 0;
    
    if (p_route->p_path->p_instance->vrf == 0)
    {
        return VOS_OK;
    }
    
    bgp4_sys_vpls_xc_build(p_route, &xc);

    p_hdr = (struct l2vpnVcLabel_msghdr*)buf;
    p_hdr->rtm_msglen = sizeof(struct l2vpnVcLabel_msghdr) + sizeof(struct l2vpnVcLabel_msg);
    p_hdr->rtm_version = RTM_VERSION;
    p_hdr->rtm_type = active ? RTM_VPLSVCLABEL_ADD : RTM_VPLSVCLABEL_DEL;
    p_hdr->cnt = 1;

    p_msg = (struct l2vpnVcLabel_msg *)(p_hdr + 1);
    
    p_msg->vpnId = xc.vsi;
    p_msg->in_label = xc.in_label;
    p_msg->out_label = xc.out_label;

    len = p_hdr->rtm_msglen;
    
    p_hdr->rtm_msglen = htons(len);
    gbgp4.stat.sys_route.add++;
    gbgp4.stat.sys_route.msg_total_len += len;

    if ((gbgp4.rtsock > 0) 
        && (send(gbgp4.rtsock, buf, len, 0) < 0))
    {
        gbgp4.stat.sys_route.fail++;
        
        error = errnoGet();
        if ((error == ERTMSGNOBUF) || (error == ENOBUFS))
        {
            return VOS_ERR;
        }
    }
    return VOS_OK;
}

/*get target vpls list for learnt route
  for local route,it will be exported to public instance
  for remote route,it will be exported to matched vpls instance
*/
u_int
bgp4_route_dest_vpls_get(
            tBGP4_ROUTE *p_route,
            u_int *p_vrid)
{
    tBGP4_VPN_INSTANCE *p_instance = p_route->p_path->p_instance;
    tBGP4_ECOMMUNITY *p_ecom = NULL;
    struct vplsInstanceIndexInfohdr *p_msg = NULL;
    u_int ecom_count = 0;
    u_int vrcount = 0;
    u_int i = 0;
    u_int rc = 0;
    u_char buf[1024] = {0};
    
    if (p_instance->vrf != 0)
    {
        p_vrid[0] = 0;
        return 1;
    }

    if ((p_route->p_path->p_ecommunity == NULL) 
        || (p_route->p_path->excommunity_len == 0))
    {
        return 0;
    }
    p_ecom = (tBGP4_ECOMMUNITY *)p_route->p_path->p_ecommunity;
    ecom_count = p_route->p_path->excommunity_len/sizeof(tBGP4_ECOMMUNITY);

    for (i = 0; i < ecom_count ; i++, p_ecom++)
    {
        /*only consider route target*/
        if (p_ecom->val[1] != 2)
        {
            continue;
        }
        /*get matched vrid*/
        mplsVplsRtLookUpInstance(MPLSVPLS_RT_IMPORT, (u_char *)p_ecom, buf, sizeof(buf));
        p_msg = (struct vplsInstanceIndexInfohdr*)buf;

        for (rc = 0; rc < p_msg->cnt; rc++)
        {
            p_vrid[vrcount++] = p_msg->vplsInstanceIndex[rc];
        }
    }    
    return vrcount;
}

/*build local route of vpls*/
void
bgp4_vpls_local_route_fill(
         tBGP4_VPN_INSTANCE *p_instance,
         tBGP4_ROUTE *p_route)
{
    tBGP4_PATH *p_path = p_route->p_path;
    struct vplsRtInfohdr *p_hdr = NULL;
    u_char *p_fill = NULL;
    u_char ecom[512];
    u_char buf[1024] = {0};      
    u_int i = 0;
    
    /*fill NLRI*/
    bgp4_vpls_addr_set(&p_route->dest, p_instance->rd, &p_instance->local_nlri);
    p_route->proto = M2_ipRouteProto_local;

    /*fill path attribute*/
    p_path->p_instance = p_instance;
    p_path->origin_vrf = p_instance->vrf;
    p_path->af = BGP4_PF_L2VPLS;
    p_path->origin = BGP4_ORIGIN_INCOMPLETE;

    /*set extend community attribute*/
    memset(ecom, 0, sizeof(ecom));
    
    /*route target obtained from vpls instance*/  
    mplsVplsInstanceLookUpRt(MPLSVPLS_RT_EXPORT, p_instance->vrf, buf, sizeof(buf));

    p_hdr = (struct vplsRtInfohdr*)buf;
    p_fill = ecom;
    for (i = 0; (i < p_hdr->cnt) && (i < 15); i++)
    {
        bgp4_put_string(p_fill, &p_hdr->rtInfo[i].rt, BGP4_VPN_RT_LEN);
    }

   
    /*insert vpls community
     +------------------------------------+
      | Extended community type (2 octets) | 80 0a
      +------------------------------------+
      |  Encaps Type (1 octet)             | 4????
      +------------------------------------+
      |  Control Flags (1 octet)           |  
      +------------------------------------+
      |  Layer-2 MTU (2 octet)             |
      +------------------------------------+
      |  Reserved (2 octets)               |
      +------------------------------------+
    */
    i++;
    
    *p_fill = 0x80;
    p_fill++;
    
    *p_fill = 0x0a;
    p_fill++;

    *p_fill = p_instance->encapsulation;
    p_fill++;

    *p_fill = p_instance->control_word;
    p_fill++;

    bgp4_fill_2bytes(p_fill, p_instance->mtu);
    p_fill += 2;

    bgp4_fill_2bytes(p_fill, 0);

    p_path->p_ecommunity = bgp4_malloc(i * BGP4_VPN_RT_LEN, MEM_BGP_BUF);
    if (p_path->p_ecommunity)
    {
        memcpy(p_path->p_ecommunity, ecom, i * BGP4_VPN_RT_LEN);
        p_path->excommunity_len = i * BGP4_VPN_RT_LEN;
    }
    return;
}

/*process vpls add event*/
void
bgp4_vpls_add_event_process(struct l2vpn_msg *p_msg)
{
    tBGP4_VPN_INSTANCE *p_vpls =NULL;
    tBGP4_VPN_INSTANCE *p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, 0);
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_newroute = NULL;
    tBGP4_ROUTE route;
    tBGP4_PATH path;
    u_int vrf = p_msg->vpnId;
   
    if (vrf == 0)
    {
        return;
    }
    bgp4_log(BGP_DEBUG_EVT, "vpls %d add detected", vrf);
    
    p_vpls = bgp4_vpn_instance_lookup(BGP4_INSTANCE_VPLS, vrf);
    if (p_vpls == NULL)
    {
        gbgp4.stat.l2vpn_add++;
        p_vpls = bgp4_vpn_instance_create(BGP4_INSTANCE_VPLS, vrf);
    }
    if (p_vpls == NULL)
    {
        return;
    }
    p_vpls->local_nlri.ve_id = p_msg->siteId;
    p_vpls->local_nlri.ve_block_offset = p_msg->offset;
    p_vpls->local_nlri.ve_block_size = p_msg->labelRange;
    mpls_bgp_vpls_alloc_label_block(p_msg->labelRange, &p_vpls->local_nlri.label_base);
    p_vpls->mtu = p_msg->mtu;  
    p_vpls->encapsulation = p_msg->encapType;
    p_vpls->control_word = p_msg->ctrlWord;

    /*copy rd of vpls*/
    memcpy(p_vpls->rd, p_msg->rd, BGP4_VPN_RD_LEN);
    /*build a vpls route,insert into vpls table*/
    memset(&route, 0, sizeof(route));
    memset(&path, 0, sizeof(path));
    bgp4_path_init(&path);
    bgp4_link_path(&path, &route);
    path.af = BGP4_PF_L2VPLS;
    
    bgp4_vpls_local_route_fill(p_vpls, &route);

    /*insert into public instance*/
    bgp4_vrf_route_export_check(&route, TRUE);    

    /*update rib table*/
    bgp4_rib_in_table_update(&route);

    bgp4_path_clear(&path);

    if (p_instance->rib[BGP4_PF_L2VPLS] == NULL)
    {
        return;
    }
    
    /*check all vpls route in public instance,try to add again*/
    bgp4_avl_for_each(&p_instance->rib[BGP4_PF_L2VPLS]->rib_table, p_route)
    {
        /*ignore local route*/
        if (p_route->p_path->origin_vrf == p_vpls->vrf)
        {
            continue;
        }

        if (bgp4_vrf_route_export_enable(p_route, p_vpls) == TRUE)
        {
            bgp4_vpls_route_export(p_route, p_vpls, TRUE);
        }
    }
    return;
}

void
bgp4_vpls_del_event_process(struct l2vpn_msg *p_msg)
{
    tBGP4_VPN_INSTANCE *p_vpls =NULL;
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE delete_route;
    tBGP4_ROUTE *p_newroute = NULL;
    u_int vrf = p_msg->vpnId;
    p_vpls = bgp4_vpn_instance_lookup(BGP4_INSTANCE_VPLS, vrf);
    if (p_vpls == NULL)
    {
        return;
    }
    bgp4_log(BGP_DEBUG_EVT, "vpls %d delete detected", vrf);

    if (p_vpls->rib[BGP4_PF_L2VPLS] == NULL)
    {
        return;
    }
    /*remove all route in vpls*/
    bgp4_avl_for_each(&p_vpls->rib[BGP4_PF_L2VPLS]->rib_table, p_route)
    {
        if (p_route->is_deleted == TRUE)
        {
            continue;
        }

        /*build route to be deleted*/
        memset(&delete_route, 0, sizeof(delete_route));
        memcpy(&delete_route.dest, &p_route->dest, sizeof(tBGP4_ADDR));
        delete_route.proto = p_route->proto;
        delete_route.is_deleted = TRUE;
        delete_route.p_path = p_route->p_path;
        
        /*notify delete to public instance*/
        if (p_route->p_path->origin_vrf == p_vpls->vrf)
        {
            bgp4_vrf_route_export_check(p_route, FALSE);
        }
        bgp4_rib_in_table_update(&delete_route);
    }
    bgp4_vpn_instance_delete_timeout(p_vpls);
    return;
}

void
bgp4_vpls_update_event_process(struct l2vpn_msg *p_msg)
{
    tBGP4_VPN_INSTANCE *p_vpls =NULL;
    tBGP4_VPN_INSTANCE *p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, 0);
    tBGP4_ROUTE *p_route = NULL;
    tBGP4_ROUTE *p_newroute = NULL;
    tBGP4_ROUTE *p_current = NULL;
    tBGP4_ROUTE route;
    tBGP4_PATH path;
    u_int vrf = p_msg->vpnId;
    
    vrf = p_msg->vpnId;
    p_vpls = bgp4_vpn_instance_lookup(BGP4_INSTANCE_VPLS, vrf);
    if (p_vpls == NULL)
    {
        return;
    }
    bgp4_log(BGP_DEBUG_EVT, "vpls %d param change detected", vrf);

    /*originate new local route instance*/
    /*build a vpls route,insert into vpls table*/
    memset(&route, 0, sizeof(route));
    memset(&path, 0, sizeof(path));
    bgp4_path_init(&path);
    bgp4_link_path(&path, &route);
    
    bgp4_vpls_local_route_fill(p_vpls, &route);
    
    /*insert into public instance*/
    bgp4_vrf_route_export_check(&route, TRUE);
    
    /*update rib table*/
    bgp4_rib_in_table_update(&route);

    bgp4_path_clear(&path);

    if (p_instance->rib[BGP4_PF_L2VPLS] == NULL)
    {
        return;
    }
    if (p_vpls->rib[BGP4_PF_L2VPLS] == NULL)
    {
        return;
    }

    /*check all vpls route in public instance,try to add or delete
     according to change*/
    bgp4_avl_for_each(&p_instance->rib[BGP4_PF_L2VPLS]->rib_table, p_route)
    {
        /*ignore local route*/
        if (p_route->p_path->origin_vrf == p_vpls->vrf)
        {
            continue;
        }
        
        p_current = bgp4_avl_lookup(&p_vpls->rib[BGP4_PF_L2VPLS]->rib_table, p_route);
        if (bgp4_vrf_route_export_enable(p_route, p_vpls) == TRUE)
        {
            if (p_current == NULL)
            {
                bgp4_vpls_route_export(p_route, p_vpls, TRUE);
            }
        }
        else if (p_current != NULL)
        {
            bgp4_vpls_route_export(p_route, p_vpls, FALSE);
        }
    }
    return;
}

#endif

/*decide if vpn route filtered by range*/
STATUS
bgp4_vrf_route_export_check_by_range(
              tBGP4_ROUTE *p_route,
              tBGP4_ROUTE *p_current,
              u_int feasible/*TRUE:it is a feasible route,
                              FALSE:it is a withdraw route*/)
{
    tBGP4_VPN_INSTANCE *p_srcinstance = p_route->p_path->p_instance;
    tBGP4_RANGE *p_range = NULL;
    
    p_range = bgp4_range_match(p_srcinstance, p_route);
    if (p_range == NULL)
    {
        return VOS_OK;
    }

    if (p_range->summaryonly == FALSE)
    {
        return VOS_OK;
    }
        
    if (p_current && (bgp4_range_match(p_srcinstance, p_current) != p_range))
    {
        return VOS_OK;
    }

    if ((p_range->matched_route == 1) && p_current)
    {
        return VOS_OK;
    }
    if ((p_range->matched_route == 0) && (p_current == NULL))
    {
        return VOS_OK;
    }

    /*schedule update for range*/
    p_range->update_need = TRUE;
    if (!bgp4_timer_is_active(&p_srcinstance->range_update_timer))
    {
        bgp4_timer_start(&p_srcinstance->range_update_timer, 1);
    }

    if (feasible == TRUE)
    {
        return VOS_ERR;
    }
    return VOS_OK;
}
/*for a given route,export it into vrf which will accept it*/
void 
bgp4_vrf_route_export_check(
              tBGP4_ROUTE *p_route,
              u_int feasible/*TRUE:it is a feasible route,
                              FALSE:it is a withdraw route*/)
{
    tBGP4_VPN_INSTANCE *p_instance = NULL;
    tBGP4_VPN_INSTANCE *p_srcinstance = p_route->p_path->p_instance;
    tBGP4_ROUTE *p_current = NULL;
    u_int vrf_id[BGP4_MAX_VRF_ID] = {0};
    u_int vrf_num = 1;
    u_int old_vrf_id[BGP4_MAX_VRF_ID] = {0};
    u_int old_vrf_num = 0;
    u_int old_id = 0;
    u_int old_exist = 0;
    u_int i = 0;
    /*ignore null rib*/
    if (p_srcinstance->rib[p_route->dest.afi] == NULL)
    {
        return;
    }

    /*if there is only one instance,and route is ipv4,do not check vrf*/
    if ((p_route->dest.afi == BGP4_PF_IPUCAST) 
        && (bgp4_avl_count(&gbgp4.instance_table) == 1))
    {
        return;
    }
    
    /*get current route if no path attribute for withdraw*/
    p_current = bgp4_avl_lookup(&p_srcinstance->rib[p_route->dest.afi]->rib_table, p_route);
    /*  vpn aggregate:if route learnt from vpn instance,and there is
      range configured,do not export to other vrf*/
    if (((p_route->dest.afi == BGP4_PF_IPUCAST) 
          || (p_route->dest.afi == BGP4_PF_IP6UCAST))
       && (p_srcinstance->vrf != 0)
       && (p_srcinstance->vrf == p_route->p_path->origin_vrf)
       && (p_route->summary_route == FALSE))
    {
        if (bgp4_vrf_route_export_check_by_range(p_route, p_current, feasible) != VOS_OK)
        {
            return;
        }
    }
    if (feasible == FALSE)
    {
        if (p_current == NULL)
        {
            return;
        }
        p_route = p_current;
    }
    else
    {
        /*route target may change,so we must decide if any old vrf
          route to be deleted*/
        if (p_current)
        {
            #ifdef BGP_VPLS_WANTED 
            if (p_route->dest.afi == BGP4_PF_L2VPLS)
            {
                old_vrf_num = bgp4_route_dest_vpls_get(p_current, old_vrf_id);
            }
            else
            #endif
            {
                old_vrf_num = bgp4_route_dest_vrf_get(p_current, old_vrf_id);
            }
        }
    }
#ifdef BGP_VPLS_WANTED 
    /*vpls case,search vpls instance*/
    if (p_route->dest.afi == BGP4_PF_L2VPLS)
    {
        vrf_num = bgp4_route_dest_vpls_get(p_route, vrf_id);
        for (i = 0; i < vrf_num; i++)
        {
            p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_VPLS, vrf_id[i]);
            if (p_instance)
            {
                bgp4_vpls_route_export(p_route, p_instance, feasible);
            }            
        }

        /*delete old route for unused vrf*/
        if (feasible && p_current)
        {
            /*if any old vrf not existed in new vrf,delete*/
            for (old_id = 0 ; old_id < old_vrf_num ; old_id++)
            {
                old_exist = 0;
                for (i = 0; i < vrf_num; i++)
                {
                    if (vrf_id[i] == old_vrf_id[old_id])
                    {
                        old_exist = 1;
                        break;
                    }
                }
                if (old_exist)
                {
                    continue;
                }
                p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_VPLS, old_vrf_id[old_id]);
                if (p_instance)
                {
                    bgp4_vpls_route_export(p_route, p_instance, FALSE);
                }
            }
        }
        return;
    }
#endif
    vrf_num = bgp4_route_dest_vrf_get(p_route, vrf_id);
    for (i = 0; i < vrf_num; i++)
    {
        p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, vrf_id[i]);
        if (p_instance)
        {
            /*UPE label assign:if route is vpn route,and there is upe peer
             and upe label not allocated*/
            if (vrf_id[i] 
                && ((p_route->dest.afi == BGP4_PF_IPVPN) 
                    || (p_route->dest.afi == BGP4_PF_IP6VPN))
                && (p_route->upe_label == 0)
                && (p_instance->upe_count != 0))
            {
                bgp_label_handle(BGP4_GET_MPLS_LABEL, &p_route->upe_label);
                /*put assigned label to new route*/
                //p_route->upe_label = mpls_l3vpn_get_label(vrf_id[i] , 0);
                /*move 4bits shift less*/
                //p_route->upe_label = p_route->upe_label >> 4;
            }

            bgp4_vrf_route_export(p_route, p_instance, feasible);
        }
    }
    
    /*delete old route for unused vrf*/
    if (feasible && p_current)
    {
        /*if any old vrf not existed in new vrf,delete*/
        for (old_id = 0 ; old_id < old_vrf_num ; old_id++)
        {
            old_exist = 0;
            for (i = 0; i < vrf_num; i++)
            {
                if (vrf_id[i] == old_vrf_id[old_id])
                {
                    old_exist = 1;
                    break;
                }
            }
            if (old_exist)
            {
                continue;
            }
            p_instance = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, old_vrf_id[old_id]);
            if (p_instance)
            {
                bgp4_vrf_route_export(p_route, p_instance, FALSE);
            }
        }
    }
    return;
}

/*decide if a route can be exported to special vrf*/
u_int
bgp4_vrf_route_export_enable(
              tBGP4_ROUTE *p_route,
              tBGP4_VPN_INSTANCE *p_instance)
{
    u_int vrf_id[BGP4_MAX_VRF_ID] = {0};
    u_int vrf_num = 1;
    u_int i = 0;
    
#ifdef BGP_VPLS_WANTED 
    /*vpls case,search vpls instance*/
    if (p_route->dest.afi == BGP4_PF_L2VPLS)
    {
        vrf_num = bgp4_route_dest_vpls_get(p_route, vrf_id);
        for (i = 0; i < vrf_num; i++)
        {
            if (p_instance->vrf == vrf_id[i])
            {
                return TRUE;
            }
        }
        return FALSE;
    }
#endif
    vrf_num = bgp4_route_dest_vrf_get(p_route, vrf_id);
    for (i = 0; i < vrf_num; i++)
    {
        if (p_instance->vrf == vrf_id[i])
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*calculate upe peer count*/
u_int
bgp4_vpn_upe_peer_count_update(tBGP4_VPN_INSTANCE *p_instance)
{
    tBGP4_PEER *p_peer = NULL;
    tBGP4_VPN_INSTANCE *p_vrf_instance = NULL;
    
    p_instance->upe_count = 0;
    bgp4_avl_for_each(&p_instance->peer_table, p_peer)
    {
        if (p_peer->upe_enable)
        {
            p_instance->upe_count++;
        }
    }

    /*update vrf's default route*/
    bgp4_avl_for_each(&gbgp4.instance_table, p_vrf_instance)
    {
        bgp4_upe_default_route_update(p_vrf_instance);
    }
    return p_instance->upe_count;
}

void
bgp4_upe_default_route_update(tBGP4_VPN_INSTANCE *p_instance)
{
    tBGP4_VPN_INSTANCE *p_public = bgp4_vpn_instance_lookup(BGP4_INSTANCE_IP, 0);
    tBGP4_ROUTE route;
    tBGP4_PATH path;
    if ((p_public == NULL) || (p_public == p_instance))
    {
        return;
    }

    if (p_instance->type != BGP4_INSTANCE_IP)
    {
        return;
    }

    /*build default route*/
    memset(&route, 0, sizeof(route));
    route.dest.afi = BGP4_PF_IPUCAST;
    route.proto = M2_ipRouteProto_icmp;/*special protocol*/
    route.is_deleted = p_public->upe_count ? FALSE : TRUE;
    
    memset(&path, 0, sizeof(path));
    bgp4_path_init(&path);
    bgp4_link_path(&path, &route);

    path.origin = BGP4_ORIGIN_IGP;
    path.af = BGP4_PF_IPUCAST;
    path.p_instance = p_instance;
    path.origin_vrf = p_instance->vrf;
    
    bgp4_vrf_route_export_check(&route, route.is_deleted ? FALSE : TRUE);

    bgp4_rib_in_table_update(&route);

    bgp4_path_clear(&path);
    return;    
}
void
bgp4_vpn_route_in_label_get(tBGP4_ROUTE *p_route)
{
    u_int vrf_id[BGP4_MAX_VRF_ID] = {0};
    u_int i = 0;
    u_int vrf_count = bgp4_route_dest_vrf_get(p_route, vrf_id);

    for (i = 0; i < vrf_count ; i++)
    {
        if (vrf_id[i] != 0)
        {
            bgp_label_handle(BGP4_GET_MPLS_LABEL, &p_route->in_label);
            //p_route->in_label = mpls_l3vpn_get_label(vrf_id[i], 0);
            if (p_route->in_label)
            {
                //p_route->in_label = p_route->in_label >> 4;
                break;
            }
        }
    }
    return;
}
#else
/*****************************************************************************
  bgp4mplsvpn.c :for processing MPLS VPN route  only
******************************************************************************/

#include "bgp4com.h"

#ifndef BGP4_VPN_WANTED
#define mpls_l3vpn_get_label(x,y) VOS_ERR
#define mpls_l3vpn_label_route_notify(x,y) VOS_ERR
#endif

u_char* bgp4_translate_vpn_RD(tBGP4_ROUTE *p_route ,octetstring* p_rd_str)
{
    u_short rd_type = 0;
    u_short as_num = 0;
    u_int as4_num = 0;
    u_short assigned_num = 0;
    u_int assigned4_num = 0;
    u_char ip_str[32] = {0};

    if(p_route == NULL || p_rd_str->pucBuf == NULL ||
        (p_route->dest.afi != BGP4_PF_IPVPN && p_route->dest.afi != BGP4_PF_IP6VPN))
    {
        return NULL;
    }

    if(p_rd_str->len < 32)
    {
        return NULL;
    }

    memcpy(&rd_type,p_route->dest.ip,2);
    memset(p_rd_str->pucBuf,0,p_rd_str->len);
    switch(rd_type)
    {
        /*ASN(2 BYTES) + ASSIGNED VALUE(4 BYTES)*/
        case 0:
        {
            memcpy(&as_num,p_route->dest.ip+2,2);
            memcpy(&assigned4_num,p_route->dest.ip+4,4);

            sprintf(p_rd_str->pucBuf,"%d:%d",as_num,assigned4_num);
            p_rd_str->len = strlen(p_rd_str->pucBuf);
            break;
        }
        /*IP ADDR(4 BYTES) + ASSIGNED VALUE(2 BYTES)*/
        case 1:
        {
            inet_ntoa_1(ip_str,bgp_ip4(p_route->dest.ip+2));
            memcpy(&assigned_num,p_route->dest.ip+6,2);

            sprintf(p_rd_str->pucBuf,"%s:%d",ip_str,assigned_num);
            p_rd_str->len = strlen(p_rd_str->pucBuf);
            break;
        }
        /*ASN(4 BYTES) + ASSIGNED VALUE(2 BYTES)*/
        case 2:
        {
            memcpy(&as4_num,p_route->dest.ip+2,4);
            memcpy(&assigned_num,p_route->dest.ip+6,2);

            sprintf(p_rd_str->pucBuf,"%d:%d",as4_num,assigned_num);
            p_rd_str->len = strlen(p_rd_str->pucBuf);
            break;
        }

    }
    return p_rd_str->pucBuf;
}

u_int bgp4_translate_vpn_label(u_int vpn_label)
{
    return (vpn_label>>4);
}

void bgp4_fill_vpn_route_entry(tBGP4_ROUTE *p_route,tBGP4_PATH *p_path,
                                u_int vpn_route_direction,tMplsVpnRouteEntry* vpn_route_info,u_int vrf_id)
{

    u_int afi = 0;

    if(p_route == NULL ||p_path == NULL || vpn_route_info== NULL)
    {
        return;
    }

    memset(vpn_route_info,0,sizeof(tMplsVpnRouteEntry));
    
    /*fill route dest (not include RD)*/
    afi = bgp4_index_to_afi(p_route->dest.afi);
    vpn_route_info->dest_type = (afi == BGP4_AF_IP) ? AF_INET : AF_INET6;
    memcpy(vpn_route_info->dest,p_route->dest.ip,16);

    /*fill nexthop*/
    if(vpn_route_direction == MPLS_6PE_REMOTE_ROUTE ||
        (vpn_route_direction == MPLSL3VPN_ROUTE_REMOTE && afi == BGP4_AF_IP6)/*IPV6 VPN REMOTE*/)
    {
        vpn_route_info->next_hop_type = AF_INET;/*MPLS ONLY SUPPORT IPV4,SO FAR*/
        memcpy(vpn_route_info->next_hop,p_route->p_path->nexthop_global.ip + 12,4);
    }
    else
    {
        vpn_route_info->next_hop_type = vpn_route_info->dest_type;
        if(vpn_route_info->next_hop_type == AF_INET)
        {
            memcpy(vpn_route_info->next_hop,p_route->p_path->nexthop.ip,4);
        }
        else
        {
            memcpy(vpn_route_info->next_hop,p_route->p_path->nexthop_global.ip,16);/*global or local?*/
        }
    }

    if(afi == BGP4_AF_IP)
    {
        vpn_route_info->mask = bgp4_len2mask(p_route->dest.prefixlen);
    }
    else
    {
        vpn_route_info->mask = p_route->dest.prefixlen;
    }

    vpn_route_info->route_direction = vpn_route_direction;
    vpn_route_info->vrf_id = vrf_id;

    vpn_route_info->vc_label = p_route->vpn_label;

    if(vpn_route_info->dest_type == AF_INET)
    {
        bgp4_log(BGP_DEBUG_MPLS_VPN,1,
                    "vrf id %d,route direction %d,route %#x,nexthop %#x,mask %#x,label %d",
                    vpn_route_info->vrf_id,
                    vpn_route_info->route_direction,
                    bgp_ip4(vpn_route_info->dest),
                    bgp_ip4(vpn_route_info->next_hop),
                    vpn_route_info->mask,
                    vpn_route_info->vc_label);
    }
    else
    {
        u_char dest_addr[64];
        u_char nexthop_addr[64];
        if(vpn_route_info->next_hop_type == AF_INET6)
        {
            inet_ntop(AF_INET6,vpn_route_info->next_hop, nexthop_addr, 64);
        }
        else
        {
            sprintf(nexthop_addr,"%d.%d.%d.%d",vpn_route_info->next_hop[0],
                                            vpn_route_info->next_hop[1],
                                            vpn_route_info->next_hop[2],
                                            vpn_route_info->next_hop[3]);
        }
        bgp4_log(BGP_DEBUG_MPLS_VPN,1,
                    "vrf id %d,route direction %d,route %s,nexthop %s,mask %d,label %d",
                    vpn_route_info->vrf_id,
                    vpn_route_info->route_direction,
                    inet_ntop(AF_INET6,vpn_route_info->dest, dest_addr, 64),
                    nexthop_addr,
                    vpn_route_info->mask,
                    vpn_route_info->vc_label);
    }


    return;
}
tBGP4_ROUTE* bgp4_mpls_rebuild_local_vpn_route(u_int src_vrf_id,
                            u_int direction,
                            tBGP4_VPN_INSTANCE* p_instance,
                            tBGP4_ROUTE* p_origin_route)
{
    tBGP4_ROUTE* p_new_route = NULL;
    tBGP4_VPN_INSTANCE* p_temp_instance = NULL;
    u_char vpn_rd[BGP4_VPN_RD_LEN] = {0};
    tBGP4_PATH* p_old_path = NULL;
    tBGP4_PATH p_temp_path = {0};
    tBGP4_PATH* p_rebuild_path = NULL;
    tBgpLookupL3Vpn lookup_msg = {0};
    u_int dst_vrf_id = 0;
    int rc = TRUE;
    u_int rt_num = 0;
    u_int i = 0;

    if(gBgp4.max_vpn_instance == 0)
    {
        return p_origin_route;
    }

    if(p_origin_route == NULL || p_instance == NULL)
    {
        return NULL;
    }

     dst_vrf_id = p_instance->instance_id;

    if(src_vrf_id == dst_vrf_id
            && direction != MPLS_6PE_LOCAL_ROUTE
            && direction != MPLS_6PE_REMOTE_ROUTE)
    {
        /*record origin route direction,such route no need rebuilding*/
        p_origin_route->route_direction = direction;
        return p_origin_route;
    }

    p_new_route = bgp4_creat_route();
    if(p_new_route == NULL)
    {
        bgp4_log(BGP_DEBUG_CMN,0,"bgp4 creat route failed in rebuild local vpn route!");
        return NULL;
    }

    switch(direction)
    {
        case MPLSL3VPN_ROUTE_LOCAL:
        {
            if(dst_vrf_id == 0)/*rebulid vpn label route,send to remote,src vrf id must be no-zero*/
            {
                memset(vpn_rd,0,8);
                p_temp_instance = bgp4_vpn_instance_lookup(src_vrf_id);
                if(p_temp_instance == NULL)
                {
                    bgp4_log(BGP_DEBUG_CMN,0,"Get rd failed rebuild local vpn route!");
                    bgp4_free(p_new_route, MEM_BGP_ROUTE);
                    return NULL;
                }
                memcpy(vpn_rd,p_temp_instance->rd,BGP4_VPN_RD_LEN);
                bgp4_log(BGP_DEBUG_MPLS_VPN,1,"source vrf id %d ,mpls l3vpn get rd %d.%d.%d.%d.%d.%d.%d.%d",src_vrf_id,vpn_rd[0],vpn_rd[1],vpn_rd[2],
                            vpn_rd[3],vpn_rd[4],vpn_rd[5],vpn_rd[6],vpn_rd[7]);

                memcpy(p_new_route->dest.ip,vpn_rd,BGP4_VPN_RD_LEN);
                memcpy(p_new_route->dest.ip+BGP4_VPN_RD_LEN,p_origin_route->dest.ip,16);
                p_new_route->dest.prefixlen = p_origin_route->dest.prefixlen + 8*BGP4_VPN_RD_LEN;
                p_new_route->dest.afi = (p_origin_route->dest.afi == BGP4_PF_IPUCAST) ? BGP4_PF_IPVPN: BGP4_PF_IP6VPN;

                /*put assigned label on new route*/
                p_new_route->vpn_label = mpls_l3vpn_get_label(src_vrf_id,0);
                p_new_route->proto = p_origin_route->proto;
                bgp4_log(BGP_DEBUG_MPLS_VPN,1,"source vrf %d,mpls l3vpn get label return %d",
                            src_vrf_id,p_new_route->vpn_label);

            }
            else/*rebuild private route,distribute to other vpns*/
            {
                memcpy(&p_new_route->dest,&p_origin_route->dest,sizeof(tBGP4_ADDR));
            }
            break;
        }
        case MPLSL3VPN_ROUTE_REMOTE:
        {
            if(dst_vrf_id)/*rebulid vpn private route,src vrf id must be zero*/
            {
                memcpy(p_new_route->dest.ip,p_origin_route->dest.ip+BGP4_VPN_RD_LEN,16);
                p_new_route->dest.prefixlen = p_origin_route->dest.prefixlen - 8*BGP4_VPN_RD_LEN;
                p_new_route->dest.afi = (p_origin_route->dest.afi == BGP4_PF_IPVPN) ? BGP4_PF_IPUCAST: BGP4_PF_IP6UCAST;
                p_new_route->vpn_label = p_origin_route->vpn_label;
            }
            else/*IMPOSSIBLE*/
            {
                bgp4_free(p_new_route, MEM_BGP_ROUTE);
                return p_origin_route;
            }
            break;

        }
        case MPLS_6PE_LOCAL_ROUTE:/*rebulid ipv6 label route*/
        {
            memcpy(&p_new_route->dest,&p_origin_route->dest,sizeof(tBGP4_ADDR));
            p_new_route->dest.afi = BGP4_PF_IP6LABEL;

            /*put assigned label to new route*/
            p_new_route->vpn_label = mpls_l3vpn_get_label(src_vrf_id,0);
            bgp4_log(BGP_DEBUG_MPLS_VPN,1,"source vrf %d,mpls l3vpn get label return %d",
                            src_vrf_id,p_new_route->vpn_label);

            break;

        }
        case MPLS_6PE_REMOTE_ROUTE:/*rebuild ipv6 common route*/
        {
            memcpy(&p_new_route->dest,&p_origin_route->dest,sizeof(tBGP4_ADDR));
            p_new_route->dest.afi = BGP4_PF_IP6UCAST;
            break;
        }
        default:/*IMPOSSIBLE*/
        {
            bgp4_free(p_new_route, MEM_BGP_ROUTE);
            return p_origin_route;
        }
    }

    p_new_route->proto = p_origin_route->proto;

    p_new_route->route_direction = direction;

    p_old_path = p_origin_route->p_path;

    memset(&p_temp_path,0,sizeof(tBGP4_PATH)) ;
    bgp4_lstinit(&p_temp_path.aspath_list);
    bgp4_lstinit(&p_temp_path.route_list);

    bgp4_path_copy(&p_temp_path, p_old_path);

    p_temp_path.p_instance = p_instance;/*record new instance*/
    p_temp_path.src_instance_id = src_vrf_id;
    p_temp_path.p_peer = p_old_path->p_peer;
    p_temp_path.afi = bgp4_index_to_afi(p_new_route->dest.afi);
    p_temp_path.safi = bgp4_index_to_safi(p_new_route->dest.afi);

    /*put ex RT on local vpn route path before sent to remote PE*/
    if(p_new_route->route_direction == MPLSL3VPN_ROUTE_LOCAL)
    {
        tBgpL3VpnInfo ex_rt[BGP4_MAX_ECOMMNUITY];
        memset(ex_rt,0,sizeof(ex_rt));
        memset(&lookup_msg,0,sizeof(tBgpLookupL3Vpn));
        lookup_msg.type = MPLSL3VPN_GET_EXPORTRT;
        lookup_msg.inVrfId = src_vrf_id;
        lookup_msg.maxNum = BGP4_MAX_ECOMMNUITY;
        lookup_msg.outData = ex_rt;

        rc = mplsLookupL3Vpn(&lookup_msg);

        rt_num = lookup_msg.count;

        if(rc == VOS_ERR || rt_num > BGP4_MAX_ECOMMNUITY)
        {
            bgp4_log(BGP_DEBUG_EVT,1,"mpls lookup l3vpn return num %d exceed",rt_num);
            bgp4_free(p_new_route, MEM_BGP_ROUTE);
            /*free temp path's as path list*/
            bgp4_apath_list_free(&p_temp_path);

            if (p_temp_path.p_unkown != NULL)
            {
                bgp4_free((void*)p_temp_path.p_unkown, MEM_BGP_BUF);
            }
            return NULL;
        }
        bgp4_log(BGP_DEBUG_MPLS_VPN,1,"mpls lookup l3vpn return %d",rt_num);

        for(i = 0;i <rt_num;i++)
        {
            memcpy(p_temp_path.ecommunity[i].val,ex_rt[i].vrfRT,BGP4_VPN_RT_LEN);
            bgp4_log(BGP_DEBUG_MPLS_VPN,1,
                    "bgp route->bgp path->ecommunity %d.%d.%d.%d.%d.%d.%d.%d",
                    lookup_msg.outData[i].vrfRT[0],
                    lookup_msg.outData[i].vrfRT[1],
                    lookup_msg.outData[i].vrfRT[2],
                    lookup_msg.outData[i].vrfRT[3],
                    lookup_msg.outData[i].vrfRT[4],
                    lookup_msg.outData[i].vrfRT[5],
                    lookup_msg.outData[i].vrfRT[6],
                    lookup_msg.outData[i].vrfRT[7]);
        }

        p_temp_path.flags.excommunity_count = rt_num;

    }


    /*check if the rebuilt path exist in such instance*/
    p_rebuild_path = bgp4_path_lookup(&p_temp_path);

    /*if no exist,rebuild a new path*/
    if(!p_rebuild_path)
    {
        p_rebuild_path = bgp4_add_path(p_new_route->dest.afi);
        if (!p_rebuild_path)
        {
            bgp4_log(BGP_DEBUG_MPLS_VPN,1,"\r\nbgp4 add path failed in rebuild local vpn route!");
            bgp4_free(p_new_route, MEM_BGP_ROUTE);
            /*free temp path's as path list*/
            bgp4_apath_list_free(&p_temp_path);

            if (p_temp_path.p_unkown != NULL)
            {
                bgp4_free((void*)p_temp_path.p_unkown, MEM_BGP_BUF);
            }
            return NULL;
        }
        bgp4_path_copy(p_rebuild_path, &p_temp_path);
        p_rebuild_path->src_instance_id = src_vrf_id;
        p_rebuild_path->p_peer = p_temp_path.p_peer;
    }

    /*link route and path info*/
    bgp4_link_path (p_rebuild_path, p_new_route);

    {
        u_char addr_str[64] = {0};
        bgp4_log(BGP_DEBUG_MPLS_VPN,1,"\r\n rebuild route %s,direction %d(NEW)/%d(OLD)",
            addr_str,
            p_new_route->route_direction,
            direction);
    }

    /*free temp path's as path list*/
    bgp4_apath_list_free(&p_temp_path);

    if (p_temp_path.p_unkown != NULL)
    {
        bgp4_free((void*)p_temp_path.p_unkown, MEM_BGP_BUF);
    }

    return p_new_route;

}
int bgp4_mpls_vpn_route_notify(tBGP4_ROUTE* p_route)
{
    u_int src_vrf_id = 0;
    u_int dst_vrf_id = 0;
    u_int direction = 0;
    u_char action = 0;
    tMplsVpnRouteEntry label_route_info = {0};
    int rc = 0;

    if(gBgp4.max_vpn_instance == 0)
    {
        return TRUE;
    }

    if(p_route == NULL)
    {
        return TRUE;
    }

    src_vrf_id = p_route->p_path->src_instance_id;
    dst_vrf_id = p_route->p_path->p_instance->instance_id;
    direction = p_route->route_direction;

    if(direction != MPLSL3VPN_ROUTE_REMOTE&&
        direction != MPLS_6PE_REMOTE_ROUTE)
    {
        return TRUE;
    }

    /*vpn routes int common vrf need not be notified*/
    if(direction == MPLSL3VPN_ROUTE_REMOTE &&
        dst_vrf_id == 0)
    {
        return TRUE;
    }

    if(p_route->ip_action == BGP4_IP_ACTION_ADD)
    {
        action = MPLSL3VPN_ADD_ROUTE;
        gBgp4.stat.mpls_add_notify++;
    }
    else
    {
        action = MPLSL3VPN_DEL_ROUTE;
        gBgp4.stat.mpls_del_notify++;
    }

    {
        u_char rt_str[64];
        bgp4_log(BGP_DEBUG_MPLS_VPN,1,"bgp4 mpls vpn route notify,route %s,action %d,direction %d,source vrf id %d,dest vrf id %d",
                bgp4_printf_route(p_route,rt_str),
                action,
                direction,
                src_vrf_id,
                dst_vrf_id);

    }


    bgp4_fill_vpn_route_entry(p_route,p_route->p_path,direction,&label_route_info,dst_vrf_id);
    rc = mpls_l3vpn_label_route_notify(&label_route_info,action);
    if(rc != VOS_OK)
    {
        bgp4_log(BGP_DEBUG_MPLS_VPN,1,"mpls l3vpn label route notify return ERROR,error NO %d!",rc);

        if(action == MPLSL3VPN_ADD_ROUTE)
        {
            gBgp4.stat.mpls_add_notify_failed++;
            gBgp4.stat.mpls_add_notify_error = rc;
        }
        else
        {
            gBgp4.stat.mpls_del_notify_failed++;
            gBgp4.stat.mpls_del_notify_error = rc;
        }

        /*retry notification in such cases below only*/
        if(rc == ETIME || rc == EUSP_NOPROCESS)
        {
            return FALSE;
        }

    }

    return TRUE;


}
u_int bgp4_mpls_vpn_get_vrf_id_list(tBGP4_ROUTE*p_route,
                                    u_int action,
                                    tBGP4_VPN_INSTANCE* p_src_instance,
                                    u_int* direction,
                                    u_int vrf_id_array[BGP4_MAX_VRF_ID])
{
    tBGP4_PATH* p_path = NULL;
    tBgpLookupL3Vpn lookup_msg = {0};
    u_int route_afi = 0;
    u_int route_safi = 0;
    u_int vrf_num = 0;
    u_int input_vrf_id = 0;
    int rc = VOS_ERR;
    u_int i = 0;
    u_int k = 0;

    if(p_route == NULL || p_src_instance == NULL)
    {
        return 0;
    }

    input_vrf_id = p_src_instance->instance_id;

    route_afi = p_route->p_path->afi;

    route_safi = p_route->p_path->safi;

    vrf_id_array[0] = input_vrf_id;/*any routes should be distributed in self instance*/

    vrf_num ++;/*including self instance*/


    if(gBgp4.max_vpn_instance == 0)
    {
        return vrf_num;
    }


    if(input_vrf_id != 0)/*vpn,local CE,in private instance*/
    {
        tBgpL3VpnInfo vrf_id_list[BGP4_MAX_VRF_ID];
        memset(vrf_id_list,0,sizeof(vrf_id_list));
        memset(&lookup_msg,0,sizeof(tBgpLookupL3Vpn));

        lookup_msg.type = MPLSL3VPN_GET_LOCALVRF;
        lookup_msg.inVrfId = input_vrf_id;
        lookup_msg.maxNum = BGP4_MAX_VRF_ID;
        lookup_msg.outData = vrf_id_list;

        rc = mplsLookupL3Vpn(&lookup_msg);

        if(rc == VOS_ERR)
        {
            bgp4_log(BGP_DEBUG_MPLS_VPN,1,
                    "mpls lookup l3vpn return ERROR");

        }
        else if(lookup_msg.count > (BGP4_MAX_VRF_ID-2))
        {
            bgp4_log(BGP_DEBUG_MPLS_VPN,1,
                    "mpls lookup l3vpn vrf num %d exceed",
                    lookup_msg.count);
        }
        else
        {
            vrf_num += lookup_msg.count;
            bgp4_log(BGP_DEBUG_MPLS_VPN,1,
                    "mpls l3vpn lookup vrf local return vrf num %d",
                    lookup_msg.count);
        }

        for(i=0;i<lookup_msg.count;i++)
        {
            u_int vrf_id = 0;
            vrf_id = vrf_id_list[i].vrfId;
            vrf_id_array[i+1] = vrf_id;
        }

        /*public instance should always be the one*/
        vrf_id_array[vrf_num] = 0;
        vrf_num++;
        *direction = MPLSL3VPN_ROUTE_LOCAL;

    }
    else if(route_safi == BGP4_SAF_VLBL)/*mpls vpn,from remote PE,public instance*/
    {
        u_int total_vrf_num = 0;
        *direction = MPLSL3VPN_ROUTE_REMOTE;

        if((route_afi == BGP4_AF_IP && gBgp4.rt_matching_vpnv4 == 0) ||
                (route_afi == BGP4_AF_IP6 && gBgp4.rt_matching_vpnv6 == 0))
        {
            /*no enable vpn rt matching,only record in public instance for propagation*/
            return vrf_num;
        }

        if(action)/*feasible routes*/
        {
            p_path = p_route->p_path;

        }
        else/*withdraw routes with no path*/
        {
            tBGP4_ROUTE* p_old_route = NULL;
            u_char rt_str[64];
            p_old_route = bgp4_rib_lookup(&p_src_instance->rib,p_route);
            if(p_old_route)
            {
                p_path = p_old_route->p_path;
                bgp4_log(BGP_DEBUG_MPLS_VPN,1,"find an old route %s ,its path ,ex rt num %d",
                    bgp4_printf_route(p_route,rt_str),
                    p_path->flags.excommunity_count);

            }
            else
            {
                bgp4_log(BGP_DEBUG_MPLS_VPN,1,"withdraw route %s,no find in instance %d rib",
                        bgp4_printf_route(p_route,rt_str),
                        input_vrf_id);
                return 0;
            }
        }

        for(i = 0;i<p_path->flags.excommunity_count;i++)
        {
            tBgpL3VpnInfo vrf_id_list[BGP4_MAX_VRF_ID];
            memset(vrf_id_list,0,sizeof(vrf_id_list));
            memset(&lookup_msg,0,sizeof(tBgpLookupL3Vpn));
            memcpy(lookup_msg.inRT,p_path->ecommunity[i].val,BGP4_VPN_RT_LEN);
            lookup_msg.type = MPLSL3VPN_GET_REMOTEVRF;
            lookup_msg.maxNum = BGP4_MAX_VRF_ID;
            lookup_msg.outData = vrf_id_list;
            /*must be route target extend community*/
            if (lookup_msg.inRT[1] != 2)
            {
                continue;
            }
            
            rc = mplsLookupL3Vpn(&lookup_msg);

            if(rc == VOS_ERR)
            {
                bgp4_log(BGP_DEBUG_MPLS_VPN,1,
                    "mpls lookup l3vpn return ERROR");
                vrf_num = 0;
            }
            else if((total_vrf_num+lookup_msg.count) > (BGP4_MAX_VRF_ID - 1))
            {
                bgp4_log(BGP_DEBUG_MPLS_VPN,1,
                    "mpls lookup l3vpn vrf num %d exceed",
                    lookup_msg.count);
                vrf_num = 0;
            }
            else
            {
                if(lookup_msg.count > 0)
                {
                    vrf_num += lookup_msg.count;
                }
                else
                {
                    vrf_num = 0;
                }
                bgp4_log(BGP_DEBUG_MPLS_VPN,1,
                    "mpls l3vpn lookup vrf remote return vrf num %d",
                    lookup_msg.count);
            }

            if((lookup_msg.count == 0)||(rc == VOS_ERR))
            {
                gBgp4.stat.vpn_remote_vrf_no_find_count++;
            }

            for(k=0;k<lookup_msg.count;k++)
            {
                u_int vrf_id = 0;
                vrf_id = vrf_id_list[k].vrfId;
                vrf_id_array[k+total_vrf_num+1] = vrf_id;
            }

            total_vrf_num+=vrf_num;
        }

    }
    else if(gBgp4.if_support_6pe && route_afi == BGP4_AF_IP6)/*in public instance*/
    {
        if(route_safi == BGP4_SAF_LBL)/*remote 6PE routes*/
        {
            *direction = MPLS_6PE_REMOTE_ROUTE;
        }
        else if(route_safi == BGP4_SAF_UCAST)/*common ipv6 routes*/
        {
            /*save BGP4_SAF_LBL routes for initial update when 6PEs up*/
            *direction = MPLS_6PE_LOCAL_ROUTE;
        }
    }
    else
    {
        /*do nothing*/
    }

    bgp4_log(BGP_DEBUG_MPLS_VPN,1,
                "input vrf %d,direction %d,vrf num %d",
                input_vrf_id,
                *direction,
                vrf_num);

    return vrf_num;

}

/*import rt add,vpn routes may be increased*/
void bgp4_vpn_import_rt_add(tBGP4_VPN_INSTANCE* p_dst_instance)
{
    tBGP4_VPN_INSTANCE* p_public_instance = NULL;
    tBGP4_PEER *p_peer = NULL;
    tBGP4_ROUTE  *p_scanroute = NULL;
    tBGP4_ROUTE  *p_next = NULL;
    u_int vrf_num = 0;
    u_int direction = 0;
    u_int vrf_id_array[BGP4_MAX_VRF_ID] = {0};
    u_int i = 0;
    tBGP4_VPN_INSTANCE* p_src_instance = NULL;
    tBGP4_ROUTE  * p_new_route = NULL;
    u_int input_vrf_id = 0;

    if(gBgp4.max_vpn_instance == 0)
    {
        return;
    }

    gBgp4.vpn_import_rt_add_count ++;


    p_public_instance = bgp4_vpn_instance_lookup(0);

    if(p_public_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_MPLS_VPN,1,"common instance instance null!");
        return;
    }

    /*remote vpn routes may be increased,refresh PEs (in pub instance) to recalculate remote vpn routes*/
    LST_LOOP(&p_public_instance->peer_list, p_peer, node, tBGP4_PEER)
    {
        /*send route refresh to all PE peers ,if such capability is not supported,restart peer*/
        if(bgp4_af_support(p_peer,BGP4_PF_IPVPN)
            && p_peer->state == BGP4_NS_ESTABLISH)
        {
            if(bgp4_send_refresh(p_peer,BGP4_PF_IPVPN) == VOS_ERR)
            {
                bgp4_fsm(p_peer,BGP4_EVENT_STOP);
                bgp4_fsm(p_peer,BGP4_EVENT_START);
            }
        }

        if(bgp4_af_support(p_peer,BGP4_PF_IP6VPN)
            && p_peer->state == BGP4_NS_ESTABLISH)
        {
            if(bgp4_send_refresh(p_peer,BGP4_PF_IP6VPN) == VOS_ERR)
            {
                bgp4_fsm(p_peer,BGP4_EVENT_STOP);
                bgp4_fsm(p_peer,BGP4_EVENT_START);
            }
        }
    }


    /*other vpn instance vpn routes may be increased,update other local vpn routes in such instance's rib*/
    LST_LOOP(&gBgp4.vpn_instance_list,p_src_instance,node,tBGP4_VPN_INSTANCE)
    {
        /*pub instance need not be considered*/
        if(p_src_instance->instance_id == 0)
        {
            continue;
        }

        input_vrf_id = p_src_instance->instance_id;



        RIB_LOOP(&p_src_instance->rib, p_scanroute, p_next)
        {

            /*only original routes should be refreshed*/
            if(p_scanroute->p_path->src_instance_id != p_src_instance->instance_id)
            {
                continue;
            }

            vrf_num = bgp4_mpls_vpn_get_vrf_id_list(p_scanroute,1,p_src_instance,&direction,vrf_id_array);

            for(i = 0;i < vrf_num;i++)
            {
                /*only newly target instance should be updated*/
                if(bgp4_vpn_instance_lookup(vrf_id_array[i]) != p_dst_instance)
                {
                    continue;
                }

                p_new_route = bgp4_mpls_rebuild_local_vpn_route(input_vrf_id,direction,p_dst_instance,p_scanroute);

                if(p_new_route == NULL)
                {
                    continue;
                }

                bgp4_gr_update_deferral_route(p_new_route);

                /*recalculate routes origin from such instance,schedule rib & ip operation*/
                bgp4_update_new_route(NULL, p_dst_instance, p_new_route);

                if(p_new_route != p_scanroute &&
                    p_new_route->refer== 0)
                {
                    bgp4_release_route(p_new_route);
                }

            }

        }

    }
    return;
}

/*loop such instance rib,send update to PEs using renewed RTs && local other vpn instance routes may increased*/
void bgp4_vpn_export_rt_add(tBGP4_VPN_INSTANCE* p_src_instance)
{
    tBGP4_ROUTE  *p_scanroute = NULL;
    tBGP4_ROUTE  *p_next = NULL;
    u_int vrf_num = 0;
    u_int direction = 0;
    u_int vrf_id_array[BGP4_MAX_VRF_ID] = {0};
    u_int i = 0;
    tBGP4_VPN_INSTANCE* p_dst_instance = NULL;
    tBGP4_ROUTE  * p_new_route = NULL;
    u_int input_vrf_id = p_src_instance->instance_id;

    if(gBgp4.max_vpn_instance == 0)
    {
        return;
    }

    gBgp4.vpn_export_rt_add_count ++;

    /*refresh local vpn instance routes to remote and local target instances*/
    RIB_LOOP(&p_src_instance->rib, p_scanroute, p_next)
    {
        /*only local vpn routes should be refreshed*/
        if(p_scanroute->route_direction != MPLSL3VPN_ROUTE_LOCAL)
        {
            continue;
        }

        /*only original routes should be refreshed*/
        if(p_scanroute->p_path->src_instance_id != p_src_instance->instance_id)
        {
            continue;
        }

        vrf_num = bgp4_mpls_vpn_get_vrf_id_list(p_scanroute,1,p_src_instance,&direction,vrf_id_array);

        for(i = 0;i < vrf_num;i++)
        {
            p_dst_instance = bgp4_vpn_instance_lookup(vrf_id_array[i]);

            if(p_dst_instance == NULL ||
                    p_dst_instance == p_src_instance)
            {
                continue;
            }

            p_new_route = bgp4_mpls_rebuild_local_vpn_route(input_vrf_id,direction,p_dst_instance,p_scanroute);

            if(p_new_route == NULL)
            {
                continue;
            }

            bgp4_gr_update_deferral_route(p_new_route);

            /*recalculate routes origin from such instance,schedule rib & ip operation*/
            bgp4_update_new_route(NULL, p_dst_instance, p_new_route);

            if(p_new_route != p_scanroute &&
                    p_new_route->refer == 0)
            {
                bgp4_release_route(p_new_route);
            }

        }
    }

    return;
}

/*loop such instance rib,send update to PEs using renewed RTs && local other vpn instance routes may decreased*/
void bgp4_vpn_export_rt_del(tBGP4_VPN_INSTANCE* p_src_instance)
{
    tBGP4_ROUTE  *p_scanroute = NULL;
    tBGP4_ROUTE  *p_next = NULL;
    u_int vrf_num = 0;
    u_int vrf_id_array[BGP4_MAX_VRF_ID] = {0};
    u_int direction = 0;
    u_int i = 0;
    tBGP4_ROUTE  * p_new_route = NULL;
    u_int input_vrf_id = p_src_instance->instance_id;
    tBGP4_VPN_INSTANCE* p_dst_instance = NULL;
    u_char found = 0;

    if(gBgp4.max_vpn_instance == 0)
    {
        return;
    }

    gBgp4.vpn_export_rt_del_count ++;

    RIB_LOOP(&p_src_instance->rib, p_scanroute, p_next)
    {
        /*only local vpn routes should be refreshed*/
        if(p_scanroute->route_direction != MPLSL3VPN_ROUTE_LOCAL)
        {
            continue;
        }

        /*only original routes should be refreshed*/
        if(p_scanroute->p_path->src_instance_id != p_src_instance->instance_id)
        {
            continue;
        }

        vrf_num = bgp4_mpls_vpn_get_vrf_id_list(p_scanroute,1,p_src_instance,&direction,vrf_id_array);

        p_dst_instance = NULL;

        /*withdraw routes in vpn instance that is not been the target*/
        LST_LOOP(&gBgp4.vpn_instance_list,p_dst_instance,node,tBGP4_VPN_INSTANCE)
        {
            /*refresh local vpn routes to remote PEs*/
            if(p_dst_instance->instance_id == 0)
            {
                p_new_route = bgp4_mpls_rebuild_local_vpn_route(input_vrf_id,
                                direction,p_dst_instance,p_scanroute);

                if(p_new_route)
                {
                    bgp4_gr_update_deferral_route(p_new_route);

                    bgp4_update_new_route(NULL,p_dst_instance, p_new_route);

                    if(p_new_route != p_scanroute &&
                        p_new_route->refer == 0)
                    {
                        bgp4_release_route(p_new_route);
                    }

                }

                continue;
            }

            for(i = 0;i < vrf_num;i++)
            {
                if(p_dst_instance == bgp4_vpn_instance_lookup(vrf_id_array[i]))
                {
                    found = 1;
                    break;
                }

            }

            /*withdaw routes in instance that is not been the target now,*/
            /*if the instance is not been the target never before,withdrawal will either not become effective*/
            if(found == 0)
            {
                p_new_route = bgp4_mpls_rebuild_local_vpn_route(input_vrf_id,
                                direction,p_dst_instance,p_scanroute);

                if(p_new_route == NULL)
                {
                    continue;
                }

                bgp4_gr_update_deferral_route(p_new_route);

                bgp4_update_withdraw_route(NULL,p_dst_instance, p_new_route);

                if(p_new_route != p_scanroute)/*rebuild withdraw route should be released here*/
                {
                    bgp4_release_route(p_new_route);
                }
            }

        }/*end of LST_LOOP of vpn instance*/
    }/*end of RIB_LOOP*/


    return;
}

void bgp4_vpn_import_rt_del(tBGP4_VPN_INSTANCE* p_dst_instance)
{
    tBGP4_ROUTE  *p_scanroute = NULL;
    tBGP4_ROUTE  *p_next = NULL;
    tBGP4_ROUTE  * p_new_route = NULL;
    u_int vrf_num = 0;
    u_int direction = 0;
    u_int vrf_id_array[BGP4_MAX_VRF_ID] = {0};
    u_int i = 0;
    tBGP4_VPN_INSTANCE* p_src_instance = NULL;
    u_char found = 0;
    u_int input_vrf_id = 0;

    if(gBgp4.max_vpn_instance == 0)
    {
        return;
    }

    gBgp4.vpn_import_rt_del_count ++;

    /*loop other instance if not been the target ,withdraw routes from such src instance*/
    LST_LOOP(&gBgp4.vpn_instance_list,p_src_instance,node,tBGP4_VPN_INSTANCE)
    {
        /*dst instance could not act as src at the same time*/
        if(p_dst_instance == p_src_instance)
        {
            continue;
        }

        input_vrf_id = p_src_instance->instance_id;

        RIB_LOOP(&p_src_instance->rib, p_scanroute, p_next)
        {
            /*local vpn routes in pub instance need not change*/
            if(input_vrf_id == 0 &&
                p_scanroute->route_direction != MPLSL3VPN_ROUTE_REMOTE)
            {
                continue;
            }

            /*only original routes should be refreshed*/
            if(input_vrf_id != 0 &&
                p_scanroute->p_path->src_instance_id != p_src_instance->instance_id)
            {
                continue;
            }

            /*local spread instance may change,re-query mpls*/
            vrf_num = bgp4_mpls_vpn_get_vrf_id_list(p_scanroute,1,p_src_instance,&direction,vrf_id_array);

            for(i = 0;i < vrf_num;i++)
            {
                if(p_dst_instance == bgp4_vpn_instance_lookup(vrf_id_array[i]))
                {
                    found = 1;
                    break;
                }

            }

            /*withdraw routes if such instance is not been the target*/
            if(found == 0)
            {
                p_new_route = bgp4_mpls_rebuild_local_vpn_route(input_vrf_id,direction,p_dst_instance,p_scanroute);

                if(p_new_route == NULL)
                {
                    continue;
                }

                bgp4_gr_update_deferral_route(p_new_route);

                bgp4_update_withdraw_route(NULL,p_dst_instance, p_new_route);

                if(p_new_route != p_scanroute)/*rebuild withdraw route should be released here*/
                {
                    bgp4_release_route(p_new_route);
                }
            }
        }
    }
#if 0
    /*update ip table right away, before other vpn change event get executed*/
    bgp4_rib_walkup(TRUE);
#endif

    /*LOOP remote vpn routes in pub instance ,if rejected by all of the local instance,then delete them*/
    p_src_instance = bgp4_vpn_instance_lookup(0);
    RIB_LOOP(&p_src_instance->rib, p_scanroute, p_next)
    {
        /*local vpn routes in pub instance need not change*/
        if(p_scanroute->route_direction != MPLSL3VPN_ROUTE_REMOTE)
        {
            continue;
        }

        /*local spread instance may change,re-query mpls*/
        vrf_num = bgp4_mpls_vpn_get_vrf_id_list(p_scanroute,1,p_src_instance,&direction,vrf_id_array);

        if(vrf_num == 0)
        {
            bgp4_rib_delete(&p_src_instance->rib,p_scanroute);
        }
    }

    return;
}

tBGP4_VPN_INSTANCE* bgp4_mplsvpn_add(u_int vrf_id)
{
    tBGP4_VPN_INSTANCE* p_add_instance =NULL;

    gBgp4.vpn_instance_add_count ++;

    p_add_instance = bgp4_vpn_instance_lookup(vrf_id);
    if(p_add_instance == NULL)
    {
        p_add_instance = bgp4_vpn_instance_create(vrf_id);
        if(p_add_instance == NULL)
        {
            bgp4_log(BGP_DEBUG_MPLS_VPN,1,"\r\nbgp4 vpn instance create instance %d failed!",
                vrf_id);
            return NULL;
        }

        /*newly created instance only need refresh remote vpn routes from PEs*/
        bgp4_vpn_import_rt_add(p_add_instance);
        bgp4_log(BGP_DEBUG_MPLS_VPN,1,"add vpn instance %d ,send route refresh to PEs!",vrf_id);

    }


    return p_add_instance;
}

void bgp4_mplsvpn_change(tBGP4_VPN_INSTANCE* p_instance,u_char change_type)
{

    if(gBgp4.max_vpn_instance == 0)
    {
        return;
    }

    if(p_instance == NULL)
    {
        bgp4_log(BGP_DEBUG_MPLS_VPN,1,"input instance null!");
        return;
    }

    bgp4_log(BGP_DEBUG_MPLS_VPN,1,"vpn instance %d RT change ,change type %d",
                p_instance->instance_id,change_type);

    switch(change_type)
    {
        /*such case must refresh PE routes,for that remote VPN routes may not in local RIB*/
        case BGP4_VPN_IMPORT_RT_ADD:
        {
            bgp4_log(BGP_DEBUG_MPLS_VPN,1,"vpn instance %d RT change ,BGP4_VPN_IMPORT_RT_ADD",
                        p_instance->instance_id);
            bgp4_vpn_import_rt_add(p_instance);
            break;
        }
        /*such case need not refresh PE routes,for remote VPN routes are all in local RIB*/
        case BGP4_VPN_IMPORT_RT_DEL:
        {
            bgp4_log(BGP_DEBUG_MPLS_VPN,1,"vpn instance %d RT change ,BGP4_VPN_IMPORT_RT_DEL",
                        p_instance->instance_id);
            bgp4_vpn_import_rt_del(p_instance);
            break;
        }
        /*in such case,EX_RT in local VPN routes changed*/
        case BGP4_VPN_EXPORT_RT_ADD:
        {
            bgp4_log(BGP_DEBUG_MPLS_VPN,1,"vpn instance %d RT change ,BGP4_VPN_EXPORT_RT_ADD",
                        p_instance->instance_id);
            bgp4_vpn_export_rt_add(p_instance);
            break;
        }
        case BGP4_VPN_EXPORT_RT_DEL:
        {
            bgp4_log(BGP_DEBUG_MPLS_VPN,1,"vpn instance %d RT change ,BGP4_VPN_EXPORT_RT_DEL",
                        p_instance->instance_id);
            bgp4_vpn_export_rt_del(p_instance);
            break;
        }

        default:
        {
            bgp4_log(BGP_DEBUG_EVT,1,"vpn instance %d RT change type  %d ERROR",
                        p_instance->instance_id,
                        change_type);
            return;
        }
    }

    return;
}


void bgp4_mplsvpn_delete(tBGP4_VPN_INSTANCE* p_instance)
{
    if(gBgp4.max_vpn_instance == 0)
    {
        return;
    }

    gBgp4.vpn_instance_del_count ++;


    bgp4_vpn_instance_release(p_instance);

    bgp4_vpn_instance_del(p_instance);


    return;
}

/*withdraw imported route when vpn instance is released,and notify it to remote PEs*/
void bgp4_withdraw_import_vpn_route(u_int instance_id)
{
    tBGP4_VPN_INSTANCE* p_pub_instance = NULL;
    tBGP4_ROUTE* p_scanroute = NULL;
    tBGP4_ROUTE* p_next = NULL;
    tBGP4_ROUTE *p_best = NULL;
    tBGP4_ROUTE *p_new_best = NULL;
    tBGP4_ADDR ip;
    tBGP4_ROUTE_VECTOR vector;

    p_pub_instance = bgp4_vpn_instance_lookup(0);
    if(p_pub_instance == NULL)
    {
        return;
    }

    RIB_LOOP(&p_pub_instance->rib, p_scanroute, p_next)
    {
        /*only non-bgp routes*/
        if(p_scanroute->p_path->p_peer)
        {
            continue;
        }
        /*only local vpn routes*/
        if(p_scanroute->route_direction != MPLSL3VPN_ROUTE_LOCAL)
        {
            continue;
        }
        /*check the route if is from such vpn*/
        if(p_scanroute->p_path->src_instance_id == instance_id)
        {
            /*avoid reduplicated deletion*/
            if(p_scanroute->is_deleted == TRUE)
            {
                continue;
            }

            memcpy(&ip, &p_scanroute->dest, sizeof(ip));

            bgp4_rib_lookup_vector(&p_pub_instance->rib, &ip, &vector);
            p_best = bgp4_best_route_vector(&vector);
            if (p_best != p_scanroute ||
                p_scanroute->igp_sync_wait == TRUE)
            {
                /*delete directly*/
                bgp4_rib_delete(&p_pub_instance->rib, p_scanroute);
                continue ;
            }

            /*set delete flag*/
            p_scanroute->is_deleted = TRUE ;

            /*get new best route*/
            p_new_best = bgp4_best_route_vector(&vector);

            /*schedule withdraw to remote PEs*/
            bgp4_schedule_rib_update_with_range(p_pub_instance,p_best,p_new_best);
        }
    }

}

int bgp4_init_private_instance()
{
    tBgpLookupL3Vpn lookup_msg;
    tBGP4_VPN_INSTANCE* private_instance;
    u_int i = 0;
    u_int instance_num = 0;
    u_int search_vid = 0;
    u_char search_rd[BGP4_VPN_RD_LEN] = {0};
    tBgpL3VpnInfo instance_info[MAX_LOCAL_VRF];

    memset(&lookup_msg,0,sizeof(tBgpLookupL3Vpn));
    memset(instance_info,0,2048*sizeof(tBgpL3VpnInfo));
    lookup_msg.type = MPLSL3VPN_GET_VIDANDRD;
    lookup_msg.maxNum = MAX_LOCAL_VRF;
    lookup_msg.outData = &instance_info[0];


    if(mplsLookupL3Vpn(&lookup_msg) == VOS_ERR)
    {
        return VOS_ERR;
    }
    else
    {
        instance_num = lookup_msg.count;
        if(instance_num > 0 && instance_num <= MAX_LOCAL_VRF)
        {
            for(i = 0;i < instance_num;i++)
            {
                search_vid = instance_info[i].vrfId;
                private_instance = bgp4_mplsvpn_add(search_vid);
                if(private_instance == NULL)
                {
                    printf("\r\nbgp auto create private instance fail!!!");
                    return VOS_ERR;
                }
                memcpy(private_instance->rd,instance_info[i].vrfRT,BGP4_VPN_RD_LEN);
                bgp4_log(BGP_DEBUG_MPLS_VPN,1,"Add vpn instance %d,get rd %d.%d.%d.%d.%d.%d.%d.%d",
                        private_instance->instance_id,
                        private_instance->rd[0],private_instance->rd[1],private_instance->rd[2],
                        private_instance->rd[3],private_instance->rd[4],private_instance->rd[5],
                        private_instance->rd[6],private_instance->rd[7]);
            }
        }
    }

    return VOS_OK;
}

#endif
