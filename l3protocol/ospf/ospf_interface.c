/* ospf_interface.c - ospf interface related operation,include virtual link*/

#include "ospf.h"
#include "ospf_nm.h"



/*interface table sort and compare,used for both real and virtual interface*/
int 
ospf_if_lookup_cmp(
            struct ospf_if *p1, 
            struct ospf_if *p2)
{
    /*shamlink,vlink and non vlink.shamlink>vlink > non vlink*/
    /*20120724 shamlink*/
    if (p1->type != p2->type)
    {
        if (OSPF_IFT_SHAMLINK == p1->type)
        {
            return 1;
        }
        else if (OSPF_IFT_SHAMLINK== p2->type)
        {
            return -1;
        }
        else if (OSPF_IFT_VLINK == p1->type)
        {
            return 1;
        }
        else if (OSPF_IFT_VLINK == p2->type)
        {
            return -1;
        }
    }
 
    /*vlink,comapare transit id and nbr id*/
    if (OSPF_IFT_VLINK == p1->type)
    {
        /*transit id*/
        if (p1->p_transit_area && p2->p_transit_area)
        {
            OSPF_KEY_CMP(p1, p2, p_transit_area->id);
        }
        /*nbr id*/ 
        OSPF_KEY_CMP(p1, p2, nbr);
    }
    /*shamlink ,compare ifaddr and nbr addr*/
    else if (OSPF_IFT_SHAMLINK == p1->type)
    {
        OSPF_KEY_CMP(p1, p2, addr); /*interface addr*/
        OSPF_KEY_CMP(p1, p2, nbr); /*nbr addr*/
    }
    else
    {
        if ((0 != p1->addr) && (0 != p2->addr))
        {
        	OSPF_KEY_CMP(p1, p2, addr);
        }
#ifdef OSPF_DCN
        /*only when OSPF_DCN_FLAG,cmp unit,for unnumber interface*/
        else if(OSPF_DCN_FLAG == p1->ulDcnflag && OSPF_DCN_FLAG == p2->ulDcnflag)
        {
        	OSPF_KEY_CMP(p1, p2, ifnet_uint);
        }
#endif
        else
        {
                OSPF_KEY_CMP(p1, p2, ifnet_uint);
        }
    }
    return 0;
}

int
ospf_real_if_lookup_cmp(
           struct ospf_if *p1,
           struct ospf_if *p2)
{
    OSPF_KEY_CMP(p1, p2, ifnet_index);
    OSPF_KEY_CMP(p1, p2, addr);
    return 0;
}

int
ospf_if_nm_cmp(
      struct ospf_if *p1,
      struct ospf_if *p2)
{
    if ((p1->p_process != NULL) && (p2->p_process != NULL))
    {
        OSPF_KEY_CMP(p1, p2, p_process->process_id);
    }
    OSPF_KEY_CMP(p1, p2, addr);
    if(p1->addr == 0)
    {
        OSPF_KEY_CMP(p1, p2, ifnet_uint);
    }
#ifdef OSPF_DCN
    if ((OSPF_DCN_FLAG == p1->ulDcnflag) && (OSPF_DCN_FLAG == p2->ulDcnflag))
    {
        OSPF_KEY_CMP(p1, p2, ifnet_uint);
    }
#endif

    return 0;
}

int
ospf_vif_nm_cmp(
       struct ospf_if *p1,
       struct ospf_if *p2)
{
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
    /*transit id*/
    if (p1->p_transit_area && p2->p_transit_area)
    {
        OSPF_KEY_CMP(p1, p2, p_transit_area->id);
    }
    /*nbr id*/ 
    OSPF_KEY_CMP(p1, p2, nbr);
    return 0;
}

int
ospf_shamlink_nm_cmp(
         struct ospf_if *p1,
         struct ospf_if *p2)
{
    OSPF_KEY_CMP(p1, p2, p_process->process_id);
    OSPF_KEY_CMP(p1, p2, addr); /*interface addr*/
    OSPF_KEY_CMP(p1, p2, nbr); /*nbr addr*/
    return 0;
}


struct ospf_if *
ospf_shamlinkif_lookup(
         struct ospf_process *p_process,
         u_int if_addr, 
         u_int nbr_addr) 
{
    struct ospf_if search;
    
    search.type = OSPF_IFT_SHAMLINK;
    search.nbr = nbr_addr;
    search.addr = if_addr;
    return ospf_lstlookup(&(p_process->if_table), &search);
}

/*  ospf_vif_lookup get an ospf virtual interface */
struct ospf_if *
ospf_vif_lookup(
         struct ospf_process *p_process,
         u_int area_id, 
         u_int nbr_id) 
{
    struct ospf_if search;
    
    search.type = OSPF_IFT_VLINK;
    search.nbr = nbr_id;
    search.p_transit_area = ospf_area_lookup(p_process, area_id);
    return ospf_lstlookup(&(p_process->if_table), &search);
}

/*   get an ospf interface */
#ifdef OSPF_DCN
struct ospf_if *
ospf_if_lookup_unnumber(
               struct ospf_process *p_process,
               u_int addr,
               u_int unit)
{
    struct ospf_if search;
    search.type = 0;
    search.addr = addr;
    search.ifnet_uint = unit;
    return ospf_lstlookup(&(p_process->if_table), &search);
}

struct ospf_if *
ospf_if_lookup_dcn(
               struct ospf_process *p_process,
               u_int addr,
               u_int unit)
{
    struct ospf_if search;
    search.type = 0;
    search.addr = addr;
    search.ifnet_uint = unit;
    search.ulDcnflag = OSPF_DCN_FLAG;
    return ospf_lstlookup(&(p_process->if_table), &search);
}

/*
struct ospf_if *
ospf_if_lookup(
               struct ospf_process *p_process,
               uint32_t addr)
{
    struct ospf_if search;
    struct ospf_if *p_if = NULL;

    search.type = OSPF_IFT_BCAST;
    search.addr = addr;
    search.ifnet_uint = 0;
    p_if = ospf_lstgreater(&(p_process->if_table), &search);
    if (p_if && p_if->addr == addr)
    {
        return p_if;
    }
    return NULL;
}*/
#endif
struct ospf_if *
ospf_if_lookup(
               struct ospf_process *p_process,
               uint32_t addr)
{
    struct ospf_if search;
    search.type = OSPF_IFT_BCAST;
    search.addr = addr;
    return ospf_lstlookup(&(p_process->if_table), &search);
}

struct ospf_if *
ospf_if_lookup_by_ifindex(struct ospf_process *p_process, ifindex_t uiIfIndex)
{
    struct ospf_if search;
    struct ospf_if *p_check_if = NULL;
    struct ospf_if *p_next_if = NULL;

    for_each_ospf_if(p_process, p_check_if, p_next_if)
    {
        if ((p_check_if->ifnet_uint == uiIfIndex))
        {
            return p_check_if;
        }                    
    }

    return NULL;
}

/*compare network of interface*/ 
int
ospf_if_network_lookup_cmp(
                struct ospf_if *p_if1, 
                struct ospf_if *p_if2)
{
    u_int net1 = p_if1->addr & p_if1->mask;
    u_int net2 = p_if2->addr & p_if2->mask;
 
    if (net1 != net2)
    {
        return (net1 > net2) ? 1 : -1;
    }
    OSPF_KEY_CMP(p_if1, p_if2,  addr);
    if(p_if1->addr == 0)
    {
        OSPF_KEY_CMP(p_if1, p_if2,  ifnet_uint);
    }
    return 0;
}

/*instance if table init*/
void 
ospf_if_table_init(struct ospf_process *p_process)
{
    ospf_lstinit(&p_process->if_table, ospf_if_lookup_cmp);
    ospf_lstinit2(&p_process->virtual_if_table, ospf_if_lookup_cmp, mbroffset(struct ospf_if, type_node));
    ospf_lstinit2(&p_process->normal_if_table, ospf_if_network_lookup_cmp, mbroffset(struct ospf_if, type_node));
 
    ospf_lstinit2(&p_process->shamlink_if_table, ospf_if_lookup_cmp, mbroffset(struct ospf_if, type_node));
    
    return;
}

/*get interface according to packet source,search in netmask if table*/
struct ospf_if *
ospf_if_lookup_by_network(
       struct ospf_process *p_process, 
       u_int addr) 
{
    struct ospf_if *p_if = NULL;
    struct ospf_if search;
    int i;

    /*check all mask appeared,only valid for first matched instance*/
    for (i = 0 ; i < 32; i++)
    {
        if (0 == p_process->ifmask[i])
        {
            break;
        }
        search.addr = addr & p_process->ifmask[i];
        search.mask = p_process->ifmask[i];
        if (OSPF_HOST_MASK == search.mask)
        {
	//		ospf_logx(ospf_debug_if,"%s %d  ****ospf_if_lookup*******\n", __FUNCTION__,__LINE__);
            p_if = ospf_if_lookup(p_process, addr);
        }
        else
        {
            p_if = ospf_lstgreater(&p_process->normal_if_table, &search);
        }
        if ((NULL != p_if) && ospf_netmatch(p_if->addr, addr, p_if->mask))
        {
            return p_if;
        }
    }
    return NULL;
}


struct ospf_process * ospf_process_lookup_by_ifindex(ifindex_t ifindex)
{
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_check_if = NULL;
    struct ospf_if *p_next_if = NULL;
    struct ospf_process *p_check_process = NULL;
    struct ospf_process *p_next_process = NULL;
    for_each_ospf_process(p_check_process, p_next_process)
    {
        for_each_ospf_if(p_check_process, p_check_if, p_next_if)
        {
            if ((p_check_if->ifnet_uint == ifindex))
            {
                return p_check_process;
            }
        }
    }
    return NULL;
}

/*common function for interface create*/
struct ospf_if *
ospf_if_create(struct ospf_process *p_process)
{
    struct ospf_if *p_if = NULL;
 
    p_if = ospf_malloc2(OSPF_MIF);
    if (NULL == p_if)
    {        
       return NULL;
    }
    memset(p_if, 0, sizeof(struct ospf_if));
    ospf_nbr_table_init(p_if);
    p_if->p_process = p_process;
    p_if->delay_ack.p_if = p_if;
    p_if->update.p_if = p_if;
    p_if->state = OSPF_IFS_DOWN;    
    p_if->mtu = OSPF_DEFAULT_IP_MTU;
	p_if->mtu_ignore = TRUE;
    p_if->maxlen = ospf_if_max_len(p_if);
    p_if->hello_interval = OSPF_DEFAULT_HELLO_INTERVAL;
    p_if->tx_delay = OSPF_DEFAULT_TRANSMIT_DELAY;
    p_if->priority = OSPF_DEFAULT_PRIORITY;
    p_if->configcost = FALSE;/*use auto update*/
    p_if->authtype = OSPF_AUTH_NONE;
    p_if->poll_interval = OSPF_DEFAULT_POLL_INTERVAL;
    p_if->rxmt_interval = OSPF_DEFAULT_RETRANSMIT_INTERVAL;
    p_if->mcast = OSPF_DEFAULT_MCAST_SUPPORT_VALUE;
    p_if->demand = OSPF_DEFAULT_DC_SUPPORT_VALUE;   
    p_if->passive = FALSE;
    p_if->authtype = OSPF_AUTH_NONE;
    p_if->costflag = FALSE;
    p_if->stat = ospf_malloc(sizeof(struct ospf_if_stat), OSPF_MSTAT);
    p_if->ucLdpSyncEn = FALSE;
    p_if->ucHoldCostState = FALSE;
    p_if->ulHoldDownInterval = OSPF_DEFAULT_HOLD_DOWN_TIME;
    p_if->ulHoldCostInterval = OSPF_DEFAULT_HOLD_COST_TIME;
    p_if->ulOspfSyncState = OSPF_LDP_UNENABLE;
    p_if->authdis = OSPF_AUTHDIS_CIPHER;
    if (NULL == p_if->stat)
    {
        ospf_mfree(p_if, OSPF_MIF);
        return NULL;
    }
    memset(p_if->stat, 0, sizeof(struct ospf_if_stat));
    /*schedule interface state checking*/
    ospf_lstwalkup(&p_process->if_table, ospf_if_state_update);

    ospf_sync_event_set(p_if->syn_flag);
    ospf_timer_init(&p_if->hello_timer, p_if, ospf_if_hello_timeout, p_process);
    ospf_timer_init(&p_if->poll_timer, p_if, ospf_if_poll_timeout, p_process);
    ospf_timer_init(&p_if->ack_timer, p_if, ospf_if_send_delayack, p_process);
    ospf_timer_init(&p_if->wait_timer, p_if, ospf_if_wait_timeout, p_process);
    ospf_timer_init(&p_if->flood_timer, p_if, ospf_if_flood_timeout, p_process);
    ospf_timer_init(&p_if->hold_down_timer, p_if, ospf_if_hold_down_timeout, p_process);
    ospf_timer_init(&p_if->hold_cost_timer, p_if, ospf_if_hold_cost_timeout, p_process);

    return p_if;    
}

struct ospf_if *
ospf_real_if_create_by_ifindex(struct ospf_process *p_process, u_int uiIfIndex, struct ospf_area *p_area)                     
{
    struct ospf_if *p_if = NULL;
    u_int unit = uiIfIndex;
    u_int8 name[32]={0};
    ospf_import_route_t stOspfImRoute = {0};

	ospf_logx(ospf_debug_if, "create if interface vrid%d, ifindex %x", p_process->vrid, uiIfIndex);

    if (IF_TYPE_GET(uiIfIndex) != IF_LOOPBACK)
    {
    	/*for normal interface,if it's ifunit exist in any other process,do not create it*/
    	if(ospf_process_lookup_by_ifindex(uiIfIndex) &&
    			ospf_process_lookup_by_ifindex(uiIfIndex) != p_process)
    		return NULL;
    }
    p_if = ospf_if_create(p_process);
    if (NULL == p_if)
    {
        return NULL;
    }
	ospf_logx(ospf_debug_if,"\r\n ospf_real_if_create unit=%x\r\n",unit);
    p_if->ifnet_uint = unit;
    /*obtain interface name from unit*/
    ospf_ifunit_to_if_name(unit, name, sizeof(name)-1);
    sprintf(p_if->name, "%s", name);
    /*calculate cost according to reference rate*/
    ospf_if_cost_calculate(p_process->reference_rate, p_if);

    p_if->link_up = FALSE;
    p_if->addr = 0;
	p_if->mask = 0;
    p_if->ifnet_index = uiIfIndex;
    p_if->p_area = p_area;
    p_if->type = OSPF_IFT_BCAST;
    p_if->dead_interval = OSPF_DEFAULT_ROUTER_DEAD_INTERVAL;
    ospf_sys_ifmtu_get(p_if->ifnet_uint, p_if->addr, &p_if->mtu);
#ifdef HAVE_BFD
    //p_if->bfd_enable = OSPF_BFD_DISABLE;
    p_if->bfd_enable = p_if->p_process->bfd_enable;
    p_if->ulRxMinInterval = OSPF_BFD_SES_MINRX_DEF;
    p_if->ulTxMinInterval = OSPF_BFD_SES_MINTX_DEF;
    p_if->ulDetMulti = OSPF_BFD_SES_DETMUL_DEF;
#endif

    if (!ospf_if_is_loopback(uiIfIndex))
	{
    	p_if->type = OSPF_IFT_PPP;
	}
    /*直连路由引入更新*/
    //stOspfImRoute.uiAf = ZEBRA_ROUTE_ISIS;
    stOspfImRoute.dest = p_if->addr & p_if->mask;
    //uiMask = ntohl(p_if->mask);
    stOspfImRoute.mask = p_if->mask;
    //stOspfImRoute.uiMaskLen = in_mask2len((struct in_addr *)&uiMask);
    stOspfImRoute.proto = M2_ipRouteProto_ospf;
    stOspfImRoute.process_id = p_if->p_process->process_id;
    stOspfImRoute.metric = 0;
    stOspfImRoute.vrid = p_process->vrid;
    stOspfImRoute.active = TRUE;

    ospf_import_route(&stOspfImRoute);
    /*caoyong delete 2017.9.18 isis路由引入由zebra提供统一的接口函数*/
    
	ospf_logx(ospf_debug_if,"\r\n ospf_real_if_create unit=%x,p_if->link_up=%x.\r\n",unit,p_if->link_up);

	ospf_logx(ospf_debug_if,"\r\n ospf_real_if_create p_if->mask=%x.\r\n",p_if->mask);

 
    ospf_logx(ospf_debug_if, "create if interface vrid%d, unit=0x%x,index=0x%x", p_process->vrid, unit,p_if->ifnet_index);
    /*insert real interface table for not-vlink*/
    ospf_lstadd(&p_process->normal_if_table, p_if);
 
    /*add to global real interface table*/
    ospf_lstadd(&ospf.real_if_table, p_if);
 
    /*add to nm interface table*/
    ospf_lstadd(&ospf.nm.if_table, p_if);
    
    /*add to process's interface table*/  
    ospf_lstadd(&p_process->if_table, p_if);   
 
    /*add to area's if table if set*/
    if (p_area)
    {
        //ospf_lstadd_unsort(&p_area->if_table, p_if);  
        ospf_lstadd(&p_area->if_table, p_if);  
    }   
    
    /*record interface mask for fast lookup according to network*/
    ospf_record_mask(p_process, p_if->mask);

    /*add multicast group address*/
    ospf_if_mcast_group_set(p_if, IP_ADD_MEMBERSHIP);

    /*init opaque lsa table*/
    ospf_lsa_table_init(&p_if->opaque_lstable, OSPF_LS_TYPE_9, p_process, p_area, p_if);
 
    /*add to nm lsa table*/
    ospf_lstadd(&ospf.nm.if_lsa_table, &p_if->opaque_lstable);

    /* if interface status is up,and area exist,trigger ism event*/
    if ((TRUE == p_if->link_up) && (NULL != p_if->p_area))
    {
        ospf_ism (p_if, OSPF_IFE_UP);
    }
 
    /*Originate graceful restart lsa for this interface if GR is in progress*/
    if (p_process->in_restart)
    {
        ospf_restart_lsa_originate(p_if); 
    }

    return p_if;
}

/*create normal interface*/
struct ospf_if *
ospf_real_if_create(
                     struct ospf_process *p_process,
                     u_int ifaddr,
                     struct ospf_area *p_area)
{
    struct ospf_if *p_if = NULL;
    u_int8 name[32]={0};
    u_int uiMask = 0, unit = 0;
    ospf_import_route_t stOspfImRoute = {0};

	ospf_logx(ospf_debug_if, "create if interface vrid%d, addr %x", p_process->vrid, ifaddr);
    /*ip interface must exist*/

    /*根据ip地址精确查找索引*/
    if (ERR == ospf_sys_addr2ifunit(p_process->vrid, ifaddr, &unit))
    {
        ospf_logx(ospf_debug_if, "invalid ip interface vrid%d, addr %x,unit%d,ignore", p_process->vrid, ifaddr, unit);
        return NULL;
    }
    if (!ospf_if_is_loopback(unit))
    {
    	/*for normal interface,if it's ifunit exist in any other process,do not create it*/
    	if(ospf_process_lookup_by_ifindex(unit) &&
    			ospf_process_lookup_by_ifindex(unit) != p_process)
    		return NULL;
    }

    p_if = ospf_if_create(p_process);
    if (NULL == p_if)
    {
        return NULL;
    }
    
    ospf_sys_ifmask_get(ifaddr, unit, &p_if->mask);
	p_if->mask = ntohl(p_if->mask);
#if 0
    if(p_process->process_id == OSPF_DCN_PROCESS)
    {
    	ospf_set_loc_man_addr(unit,ifaddr);

		if(ERROR == if_vport_ifindex_to_logic_port(unit,&uiPort))
		{
            return ERROR;
		}

		if(ERROR == if_vport_ifindex_to_subid(unit,&uiSubId))
		{
           return ERROR;
		}
		if(OSPF_DCN_SUBPORT == uiSubId)
		{
			/*slaver不参与选举DR*/
			ret = ospf_dcn_lookup_slave_port(uiPort,&uiIpaddr,&uiMasklen);
		//	printf("\r\n ospf_real_if_create ret=%x,uiIpaddr=%x,ifaddr = %x.\r\n",ret,uiIpaddr,ifaddr);
			if((ret == OK)&&(uiIpaddr == ifaddr))
			{
				p_if->priority = 0;
				p_if->uiOverlayflag = DCN_OVERLAY_SLAVER_ADD;
			//	printf("\r\n ospf_real_if_create unit=%x,ifaddr=%x,priority = %d.\r\n",unit,ifaddr,p_if->priority);
			}
			else
			{
				p_if->uiOverlayflag = DCN_OVERLAY_MASTER_ADD;
			}
		}

    }
#endif
	ospf_logx(ospf_debug_if,"\r\n ospf_real_if_create unit=%x,ifaddr=%x.\r\n",unit,ifaddr);
  //  p_if->mask = 0xffff0000;
	//ospf_sys_ifflag_get(unit, ifaddr, &flag);

    p_if->ifnet_uint = unit;
    
/*zhurish	if(ospf_if_index_to_sys_index(unit,&ulNetIndex) == ERROR)
    {
        return ERROR;
    }
    
	if(IFINDEX_INTERNAL == ulNetIndex)
	{
	   ulNetIndex = 0;
	}*/
	
    /*obtain interface name from unit*/
    ospf_ifunit_to_if_name(unit, name, sizeof(name)-1);

    //zhurish ospf_inet_ntoa(ipstr, ifaddr);

    sprintf(p_if->name, "%s", name);
    /*calculate cost according to reference rate*/

  	/*loopback cost 0*/
    if (!ospf_if_is_loopback(unit))
	{
	    p_if->cost[0] = ospf_if_cost_calculate(p_process->reference_rate, p_if); 
	}
	
    //zhurish p_if->link_up = (flag == OSPF_IFE_UP) ? TRUE : FALSE;
    p_if->link_up = TRUE;
    p_if->addr = ifaddr;
    p_if->ifnet_index = unit;
    p_if->p_area = p_area;
    p_if->type = OSPF_IFT_BCAST;
    p_if->dead_interval = OSPF_DEFAULT_ROUTER_DEAD_INTERVAL;
    ospf_sys_ifmtu_get(p_if->ifnet_uint, p_if->addr, &p_if->mtu);
#ifdef HAVE_BFD
    //p_if->bfd_enable = OSPF_BFD_DISABLE;
    p_if->bfd_enable = p_if->p_process->bfd_enable;
    p_if->ulRxMinInterval = OSPF_BFD_SES_MINRX_DEF;
    p_if->ulTxMinInterval = OSPF_BFD_SES_MINTX_DEF;
    p_if->ulDetMulti = OSPF_BFD_SES_DETMUL_DEF;
#endif

	if (ospf_if_is_loopback(unit))//zhurish
	{
	   p_if->type = OSPF_IFT_PPP;
	}
    /*直连路由引入更新*/
    //stOspfImRoute.uiAf = ZEBRA_ROUTE_ISIS;
    stOspfImRoute.dest = p_if->addr & p_if->mask;
    //uiMask = ntohl(p_if->mask);
    stOspfImRoute.mask = p_if->mask;
    //stOspfImRoute.uiMaskLen = in_mask2len((struct in_addr *)&uiMask);
    stOspfImRoute.proto = M2_ipRouteProto_ospf;
    stOspfImRoute.process_id = p_if->p_process->process_id;
    stOspfImRoute.metric = 0;
    stOspfImRoute.vrid = p_process->vrid;
    stOspfImRoute.active = TRUE;
    //printf("uiDest=%#x uiMaskLen=%#x uiProtoProId=%#x\n", stOspfImRoute.uiDest, stOspfImRoute.uiMaskLen, stOspfImRoute.uiProtoProId);
    ospf_import_route(&stOspfImRoute);
    /*caoyong delete 2017.9.18 isis路由引入由zebra提供统一的接口函数*/

	ospf_logx(ospf_debug_if,"\r\n ospf_real_if_create unit=%x,ifaddr=%x,p_if->link_up=%x.\r\n",unit,ifaddr,p_if->link_up);

	ospf_logx(ospf_debug_if,"\r\n ospf_real_if_create p_if->mask=%x.\r\n",p_if->mask);

 
    ospf_logx(ospf_debug_if, "create if interface vrid%d, addr=0x%x,unit=0x%x", p_process->vrid, ifaddr, unit);
    /*insert real interface table for not-vlink*/
    ospf_lstadd(&p_process->normal_if_table, p_if);
 
    /*add to global real interface table*/
    ospf_lstadd(&ospf.real_if_table, p_if);
 
    /*add to nm interface table*/
    ospf_lstadd(&ospf.nm.if_table, p_if);
    
    /*add to process's interface table*/  
    ospf_lstadd(&p_process->if_table, p_if);   
 
    /*add to area's if table if set*/
    if (p_area)
    {
        ospf_lstadd(&p_area->if_table, p_if);  
    }   
    
    /*record interface mask for fast lookup according to network*/
    ospf_record_mask(p_process, p_if->mask);

    /*add multicast group address*/
    ospf_if_mcast_group_set(p_if, IP_ADD_MEMBERSHIP);

    /*init opaque lsa table*/
    ospf_lsa_table_init(&p_if->opaque_lstable, OSPF_LS_TYPE_9, p_process, p_area, p_if);
 
    /*add to nm lsa table*/
    ospf_lstadd(&ospf.nm.if_lsa_table, &p_if->opaque_lstable);

    /* if interface status is up,and area exist,trigger ism event*/
    if ((TRUE == p_if->link_up) && (NULL != p_if->p_area))
    {
        ospf_ism (p_if, OSPF_IFE_UP);
    }
 
    /*Originate graceful restart lsa for this interface if GR is in progress*/
    if (p_process->in_restart)
    {
        ospf_restart_lsa_originate(p_if); 
    }

    return p_if;
}

#ifdef OSPF_DCN
/*create dcn interface*/
struct ospf_if *
ospf_dcn_real_if_create(
                     struct ospf_process *p_process,
                     u_int unit,
                     u_int ifaddr,
                     u_int ifMask,
                     struct ospf_area *p_area)
{
    struct ospf_if *p_if = NULL;

    u_int8 name[32]={0};

	ospf_logx(ospf_debug_if, "create if interface vrid%d, addr %x", p_process->vrid, ifaddr);
    /*for normal interface,if it's ifunit exist in any other process,do not create it*/
   	if(ospf_process_lookup_by_ifindex(unit) &&
    			ospf_process_lookup_by_ifindex(unit) != p_process)
    		return NULL;
    
    p_if = ospf_if_create(p_process);
    if (NULL == p_if)
    {
        return NULL;
    }

#if 0
    /*get attribute from system*/
    ospf_sys_ifindex_get(unit, ifaddr, &index);
	index = ntohl(index);
    ospf_sys_ifmask_get( ifaddr, unit, &p_if->mask);
	p_if->mask = ntohl(p_if->mask);
	ospf_logx(ospf_debug_if,"\r\n ospf_real_if_create unit=%x,ifaddr=%x.\r\n",unit,ifaddr);
  //  p_if->mask = 0xffff0000;
    ospf_sys_ifflag_get(unit, ifaddr, &flag);
#endif
	//zhurish ospf_set_loc_man_addr(unit,ifaddr);
	p_if->ifnet_uint = unit;
	/*obtain interface name from unit*/
	ospf_ifunit_to_if_name(unit, name, sizeof(name)-1);

	 sprintf(p_if->name, "%s", name);

	/*calculate cost according to reference rate*/
	 if (!ospf_if_is_loopback(unit))
    {
        p_if->cost[0] = ospf_if_cost_calculate(p_process->reference_rate, p_if);
    }
    
	p_if->link_up = TRUE;
	p_if->addr = ifaddr;

	p_if->mask= ifMask;
	p_if->ifnet_index = unit;
	p_if->p_area = p_area;
	p_if->type = OSPF_IFT_PPP;
	p_if->dead_interval = OSPF_DEFAULT_ROUTER_DEAD_INTERVAL;
	
	p_if->ulDcnflag = OSPF_DCN_FLAG;

	ospf_logx(ospf_debug_if,"\r\n ospf_real_if_create unit=%x,ifaddr=%x,p_if->link_up=%x.\r\n",unit,ifaddr,p_if->link_up);

	//	dev_hw_get_ip_env(&ulLbIp,"dcnip");
	#if 0 /*caoyong delete 2017.9.20 */ /*dcn相关*/
	ospf_dcn_set_api(&ulLbIp,DCN_NE_IP);

	ospf_logx(ospf_debug_if,"\r\n ospf_real_if_create ulLbIp=%x,ifaddr=%x.\r\n",ulLbIp,ifaddr);
    #endif
	ospf_logx(ospf_debug_if,"\r\n ospf_real_if_create p_if->mask=%x.\r\n",p_if->mask);

 
    ospf_logx(ospf_debug_if, "create if interface vrid%d, addr=0x%x,unit=0x%x,index=0x%x", p_process->vrid, ifaddr, unit,index);
    /*insert real interface table for not-vlink*/

    /*add to global real interface table*/
    ospf_lstadd(&ospf.real_if_table, p_if);
 
    /*add to nm interface table*/
    ospf_lstadd(&ospf.nm.if_table, p_if);
    
    /*add to process's interface table*/  
    ospf_lstadd(&p_process->if_table, p_if);   
 
    /*add to area's if table if set*/
    if (p_area)
    {
        ospf_lstadd(&p_area->if_table, p_if);  
    }   
    
    /*record interface mask for fast lookup according to network*/
    ospf_record_mask(p_process, p_if->mask);

    /*add multicast group address*/
    ospf_if_mcast_group_set(p_if, IP_ADD_MEMBERSHIP);

    /*init opaque lsa table*/
    ospf_lsa_table_init(&p_if->opaque_lstable, OSPF_LS_TYPE_9, p_process, p_area, p_if);
 
    /*add to nm lsa table*/
    ospf_lstadd(&ospf.nm.if_lsa_table, &p_if->opaque_lstable);

    /* if interface status is up,and area exist,trigger ism event*/
    if ((TRUE == p_if->link_up) && (NULL != p_if->p_area))
    {
        ospf_ism (p_if, OSPF_IFE_UP);
    }
 
    /*Originate graceful restart lsa for this interface if GR is in progress*/
    if (p_process->in_restart)
    {
        ospf_restart_lsa_originate(p_if); 
    }

    return p_if;
}
#endif
/*create virtual interface*/
struct ospf_if *
ospf_virtual_if_create(
                     struct ospf_process *p_process,
                     u_int nbr_id,
                     struct ospf_area *p_area)
{
    struct ospf_if *p_if = NULL;
    u_int8 nbrstr[32];
    
    /*transit area must exist*/
    if (NULL == p_area)
    {
        return NULL;
    }
 
    /* transit area must not be backbone*/
    if (p_area == p_process->p_backbone)
    {
        ospf_logx(ospf_debug_if, "transit area of virtual-link must not be backbone");
        return NULL;
    }
    p_if = ospf_if_create(p_process);
    if (NULL == p_if)
    {
        return NULL;
    }
     /*init for virtual interface*/
    p_if->p_transit_area = p_area;
    p_if->nbr = nbr_id;
    p_if->type = OSPF_IFT_VLINK;
    p_if->cost[0] = 0;
    p_if->addr = 0;
    p_if->mask = 0;
    p_if->ifnet_index = 0;
    p_if->dead_interval = OSPF_DEFAULT_VLINK_DEAD_INTERVAL;
    
    /*create backbone automatic*/  
    if (NULL == p_process->p_backbone)
    {
        ospf_area_create(p_process, OSPF_BACKBONE);
    }
    p_if->p_area = p_process->p_backbone;
    p_if->p_area->Vlinkcfg = 1;
    p_if->p_area->VlinkAreaDis = 0;

    /*build name*/
    sprintf(p_if->name, "VIF-%u-%s", (unsigned int)p_area->id, ospf_inet_ntoa(nbrstr, nbr_id));
    
    /*set vlink configured flag*/
    p_process->vlink_configured = TRUE;
    ospf_lstadd(&p_process->virtual_if_table, p_if);

    ospf_lstadd(&p_process->if_table, p_if);   

    ospf_lstadd(&ospf.nm.vif_table, p_if);

    ospf_lsa_table_init(&p_if->opaque_lstable, OSPF_LS_TYPE_9, p_process, p_if->p_area, p_if);

    ospf_lstadd(&ospf.nm.vif_lsa_table, &p_if->opaque_lstable);

    ospf_lstadd(&p_if->p_area->if_table, p_if);  

    /*Originate graceful restart lsa for this interface if GR is in progress*/
    if (p_process->in_restart)
    {
        ospf_restart_lsa_originate(p_if); 
    }
    ospf_if_state_update(p_if);
    return p_if;
}

/*create shamlink interface*/
struct ospf_if *
ospf_shamlink_if_create(
         struct ospf_process *p_process,
         u_int if_addr,
         u_int nbr_addr)
{
    struct ospf_if *p_if = NULL;
    u_int8 nbrstr[32];
    
    p_if = ospf_if_create(p_process);
    if (NULL == p_if)
    {
        return NULL;
    }
    p_if->addr = if_addr;
    p_if->nbr = nbr_addr;
    p_if->type = OSPF_IFT_SHAMLINK;
    p_if->dead_interval = OSPF_DEFAULT_ROUTER_DEAD_INTERVAL;
    p_if->cost[0] = 0;
    p_if->mask = 0;
    p_if->ifnet_index = 0;
    /*when to nbr addr exit BGP route, interface is up, here just simple to do */
    p_if->link_up = TRUE;   

    /*build name*/
    sprintf(p_if->name, "SHAMIF-%s", ospf_inet_ntoa(nbrstr, nbr_addr));

    /*insert shamlink interface table*/
    ospf_lstadd(&p_process->shamlink_if_table, p_if);

    ospf_lstadd(&p_process->if_table, p_if);   

    ospf_lstadd(&ospf.nm.shamlink_table, p_if);

    ospf_lsa_table_init(&p_if->opaque_lstable, OSPF_LS_TYPE_9, p_process, NULL, p_if);

    /*Originate graceful restart lsa for this interface if GR is in progress*/
    if (p_process->in_restart)
    {
        ospf_restart_lsa_originate(p_if); 
    }
    return p_if;
}
#ifdef OSPF_DCN

/*create unnumber interface*/
struct ospf_if *
ospf_unnumber_if_create(
                     struct ospf_process *p_process,
                     u_int ifunit,
                     struct ospf_area *p_area)
{
    struct ospf_if *p_if = NULL;
    struct ospf_if start_if;
    struct ospf_process *p_check_process = NULL;
    struct ospf_process *p_next_process = NULL;
    uint32_t index = 0;
    uint32_t flag = 0;
    uint8_t ipstr[16];
    uint8_t name[68] = {0};
    u_int uiLoopbackId = 0;
    #if 0
    if (ospf_sys_ifindex_get(ifunit, 0, &index) != OK)
    {
        ospf_logx(ospf_debug_if, "ospf get index by unit %d error", ifunit);
        return;
    }
    #endif
    p_if = ospf_if_create(p_process);
    if (NULL == p_if)
    {
        return NULL;
    }
    /*get attribute from system*/
 
    ospf_sys_ifflag_get(ifunit, 0, &flag);
    
    p_if->ifnet_uint = ifunit;  

    /*obtain interface name from unit*/
    ospf_ifunit_to_if_name(ifunit, name, sizeof(name)-1);

    sprintf(p_if->name, "%s/%d", name, ifunit);

    /*calculate cost according to reference rate*/
	if(if_index_to_loopback_id(p_if->ifnet_uint,&uiLoopbackId) == ERROR)
    {
        p_if->cost[0] = ospf_if_cost_calculate(p_process->reference_rate, p_if);  
    }
	p_if->link_up = (flag == OSPF_IFE_UP) ? TRUE : FALSE;
    p_if->addr = 0;
    p_if->ifnet_index = ifunit;
    p_if->p_area = p_area;
    p_if->type = OSPF_IFT_PPP;
    p_if->dead_interval = OSPF_DEFAULT_ROUTER_DEAD_INTERVAL;

ospf_logx(ospf_debug_if,"ospf if addr=%x, ifunit=%x,index=%d", 
    p_if->addr,p_if->ifnet_uint, p_if->ifnet_index);  

ospf_logx(ospf_debug_if,"ospf if add nm table\r\n"); 
    /*add to nm interface table*/
    ospf_lstadd(&ospf.nm.if_table, p_if);
    
ospf_logx(ospf_debug_if,"ospf if add if table\r\n"); 
    
    /*add to process's interface table*/  
    ospf_lstadd(&p_process->if_table, p_if);   
 
    /*add to area's if table if set*/
    if (p_area)
    {
        ospf_logx(ospf_debug_if,"ospf if add area nm table\r\n"); 

        //ospf_lstadd_unsort(&p_area->if_table, p_if);  
        ospf_lstadd(&p_area->if_table, p_if);  
    }   
 
    /*add multicast group address*/
    ospf_if_mcast_group_set(p_if, IP_ADD_MEMBERSHIP);
 
    /*init opaque lsa table*/
    ospf_lsa_table_init(&p_if->opaque_lstable, OSPF_LS_TYPE_9, p_process, p_area, p_if);

 ospf_logx(ospf_debug_if,"ospf if add opaque table\r\n"); 

    /*add to nm lsa table*/
    ospf_lstadd(&ospf.nm.if_lsa_table, &p_if->opaque_lstable);

 ospf_logx(ospf_debug_if,"ospf if add real if table\r\n"); 

    /*add to global real interface table*/
    ospf_lstadd(&ospf.real_if_table, p_if);
  ospf_logx(ospf_debug_if,"ospf  table end\r\n"); 
   
    /* if interface status is up,and area exist,trigger ism event*/
    if ((TRUE == p_if->link_up) && (NULL != p_if->p_area))
    {
        ospf_ism (p_if, OSPF_IFE_UP);
    }
 
    /*Originate graceful restart lsa for this interface if GR is in progress*/
    if (p_process->in_restart)
    {
        ospf_restart_lsa_originate(p_if); 
    }
    return p_if;
}
#endif    
/*This routine dynamically destroy an OSPF Interface at runtime. */
void 
ospf_if_delete(struct ospf_if *p_if )
{
    struct ospf_process *p_process = p_if->p_process;
    struct ospf_if *p_nextif = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next = NULL;
    u_int iftype = p_if->type;
    ospf_import_route_t stOspfImRoute = {0};
    u_int uiMask = 0;

    /*直连路由引入更新*/
    stOspfImRoute.dest = p_if->addr & p_if->mask;
    //uiMask = ntohl(p_if->mask);
    stOspfImRoute.mask = p_if->mask;
    //stOspfImRoute.uiMaskLen = in_mask2len((struct in_addr *)&uiMask);
    stOspfImRoute.proto = M2_ipRouteProto_ospf;
    stOspfImRoute.process_id = p_if->p_process->process_id;
    stOspfImRoute.metric = 0;
    stOspfImRoute.vrid = p_process->vrid;
    stOspfImRoute.active = FALSE;

    //printf("ospf_if_delete : dest=%#x  id=%#x\n", stOspfImRoute.uiDest, stOspfImRoute.uiProtoProId);
    ospf_import_route(&stOspfImRoute);
    /*caoyong delete 2017.9.18 路由引入由zebra提供统一的接口函数*/
    #if 0
    isis_import_public_func(&stOspfImRoute);
    #endif
    /*send leave hello*/
    if (NULL != p_if->p_area)
    {
        ospf_hello_output (p_if, TRUE, FALSE);
    }

    /*shutdown interface*/
    if (NULL != p_if->p_area)
    {
        ospf_ism(p_if, OSPF_IFE_DOWN);
    }
     /*NBMA nbr only delete in interface delete*/
    if (OSPF_IFT_NBMA == p_if->type)
    {
        for_each_ospf_nbr(p_if, p_nbr, p_next)
        {
            ospf_nsm(p_nbr, OSPF_NE_LL_DOWN);
            ospf_nbr_delete(p_nbr);
        }
    }
    /*delete nbr when delete if. not delete nbr when timeout*/
    ospf_lstwalkup(&p_if->nbr_table, ospf_nbr_delete);
    
    /*delete opaque lsa table*/
    ospf_lsa_table_shutdown(&p_if->opaque_lstable);
    
    /*leave group*/
    if ((OSPF_IFT_VLINK != p_if->type)
        && (OSPF_IFT_SHAMLINK != p_if->type)) 
    {
		ospf_logx(ospf_debug_if,"ospf_if_delete ,DROP_MEMBE=%d.\r\n",IP_DROP_MEMBERSHIP);
        ospf_if_mcast_group_set(p_if, IP_DROP_MEMBERSHIP);
    }
    
    /*clear update and ack buffer*/
    if (NULL != p_if->update.p_msg)
    {
        ospf_mfree(p_if->update.p_msg, OSPF_MPACKET);
    }

    if (NULL != p_if->delay_ack.p_msg)
    {
        ospf_mfree(p_if->delay_ack.p_msg, OSPF_MPACKET);
    }

    /*send sync msg direclty,no wait need*/
    if (OSPF_MODE_MASTER == p_process->p_master->work_mode)
    {
        ospf_syn_if_send(p_if, FALSE, NULL);
    }

    /*remove interface from special tables according to type*/
    if (OSPF_IFT_VLINK == p_if->type)
    {
        ospf_lstdel(&p_process->virtual_if_table, p_if);
        ospf_lstdel(&ospf.nm.vif_table, p_if);
        ospf_lstdel(&ospf.nm.vif_lsa_table, &p_if->opaque_lstable);
    }
    else if (OSPF_IFT_SHAMLINK == p_if->type)
    {
        ospf_lstdel(&p_process->shamlink_if_table, p_if);
        ospf_lstdel(&ospf.nm.shamlink_table, p_if);
    }
    else
    {
#ifdef OSPF_DCN
        if (OSPF_DCN_FLAG == p_if->ulDcnflag)
        {
         //   ospf_lstdel(&p_process->normal_if_table, p_if);        
        }
        else
#endif
        {
            ospf_lstdel(&p_process->normal_if_table, p_if);
        }

        ospf_lstdel(&ospf.real_if_table, p_if);
        ospf_lstdel(&ospf.nm.if_table, p_if);

        ospf_lstdel(&ospf.nm.if_lsa_table, &p_if->opaque_lstable);
    }
    if (NULL != p_if->p_area)
    {
        ospf_lstdel(&p_if->p_area->if_table, p_if);  
    }
    /*stop all timer*/
    ospf_timer_stop(&p_if->hello_timer);  
    ospf_timer_stop(&p_if->wait_timer);  
    ospf_timer_stop(&p_if->ack_timer);  
    ospf_timer_stop(&p_if->poll_timer);  
    ospf_timer_stop(&p_if->flood_timer);
    ospf_timer_stop(&p_if->hold_down_timer);
    ospf_timer_stop(&p_if->hold_cost_timer);
    
    ospf_mfree(p_if->stat, OSPF_MSTAT);
    ospf_lstdel_free(&p_process->if_table, p_if, OSPF_MIF);
    
    /*virtual link:decide if need delete backbone area*/
    if ((NULL != p_process )
        && (OSPF_IFT_VLINK == iftype) 
        && (OK != ospf_network_match_area(p_process, OSPF_BACKBONE)))
    {
        if (FALSE == ospf_area_if_exist(p_process->p_backbone))
        {
            if(p_process->p_backbone->Vlinkcfg == 0)
            {
                ospf_timer_start(&p_process->p_backbone->delete_timer, 2);
            }
            else
            {
                p_process->p_backbone->Vlinkcfg = 0;
            }
        }
    }

    /*update vlink configured flag*/
    p_process->vlink_configured = FALSE;

    /*rebuild interface mask*/
    memset(p_process->ifmask, 0, sizeof(p_process->ifmask));

    for_each_ospf_if(p_process, p_if, p_nextif)
    {
        if (OSPF_IFT_VLINK == p_if->type)
        {
            p_process->vlink_configured = TRUE;
        }
        else if (OSPF_IFT_SHAMLINK != p_if->type)
        {
            ospf_record_mask(p_process, p_if->mask);
        }
    }

    /*instance is in restarting,delete interface will stop restart*/
    if (p_process->in_restart)
    {
        ospf_logx(ospf_debug_if, "if deleted,exit graceful restart");     
          
        ospf_restart_finish(p_process, OSPF_RESTART_TOPLOGY_CHANGED);
    }   
    return;
}

/*MD5 auth will append md string to origin packet,other auth type no need*/
u_int 
ospf_if_auth_len (struct ospf_if *p_if)
{
    /*param check*/
    if (NULL == p_if->p_area)
    {
        return 0;
    }

    /*decide auth type of interface,if auth type is MD5,add 16bytes digest*/
    if ((OSPF_AUTH_MD5 == p_if->authtype)
        || ((OSPF_AUTH_NONE == p_if->authtype)
              && (OSPF_AUTH_MD5 == p_if->p_area->authtype)))
    {
        return (OSPF_MD5_KEY_LEN);
    }
    return 0;
}

/*join or leave membership for special interface*/
void 
ospf_if_mcast_group_set( 
                      struct ospf_if *p_if,
                      u_int join_flag)
{    
    struct ospf_if *p_other_if = NULL;
    struct ospf_if start_if;
    u_int ifindex = 0;
    
	ospf_logx(ospf_debug_if,"ospf_if_mcast_group_set if unit=0x%x, if addr=%#x,join_flag=%d.\r\n", p_if->ifnet_uint, p_if->addr,join_flag);
    /*ensure ip interface exist,if not,do nothing*/
#if 0
    if (ERR == ospf_sys_ifindex_get(p_if->ifnet_uint, p_if->addr, &ifindex))//del?
    {
        ospf_logx(ospf_debug,"if index get error if unit %d, if addr %#x\r\n", p_if->ifnet_uint, p_if->addr);
        return;
    }
#endif
    /*check if other interface with same ip index exist,if so,do nothing*/ 
    start_if.ifnet_index = p_if->ifnet_index;
    start_if.addr = 0;

    ospf_logx(ospf_debug_if,"if index %#x\r\n",p_if->ifnet_index); 
#if 0    
    for_each_node_greater(&ospf.real_if_table, p_other_if, &start_if)
    {
        if (p_other_if->ifnet_index == p_if->ifnet_index)
        {
            if (p_other_if != p_if)
            {
                 printf("if index %d,index %d\r\n",p_other_if->ifnet_index,p_if->ifnet_index);
                 return;
            } 
        }
        else
        {
            break;
        }
    }
    
#endif
     /*join mcast group*/
    if ( (OSPF_IFT_BCAST == p_if->type)
         || (OSPF_IFT_NBMA == p_if->type)
         || (OSPF_IFT_PPP == p_if->type))
    {
        ospf_sys_mcast_set(p_if, join_flag);
    }
    /*enable rx ospf on if*/
    ospf_logx(ospf_debug_if,"prepare set acl");
    ospf_sys_packet_input_set(p_if, join_flag);
    return;
}

/*compare dr priority of two nbrs
   1 nbr1>nb2
   -1 nbr1 < nb2
   0 same
*/ 
int 
ospf_dr_priority_cmp (
               struct ospf_nbr *p1, 
               struct ospf_nbr *p2)
{
    if ((NULL == p1) || (NULL == p2))
    {
        return p1 ? 1 : -1;
    } 

    /*first,compare priority,higher priority prefered*/
    OSPF_KEY_CMP(p1, p2, priority);
    /*same priority,compare id,higer id prefered*/
    OSPF_KEY_CMP(p1, p2, id);
    return 0;     
}

/*calculates the designated router and backup designated router*/
void 
ospf_elect_dr_bdr (struct ospf_if *p_if)
{
    struct ospf_nbr self_nbr;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    struct ospf_process *p_process = p_if->p_process;
    u_int old_dr;
    u_int old_bdr; 
    u_int old_state;

    if (p_process->in_restart)
    {
        /*if we are in restarting,use recorded dr and bdr*/
        p_nbr = ospf_lstfirst(&p_if->nbr_table);
        if (p_nbr)
        {
            p_if->dr = p_nbr->dr;
            p_if->bdr = p_nbr->bdr;            
        }
        if (p_if->dr == p_if->addr)
        {
            p_if->state = OSPF_IFS_DR;
        }
        else if (p_if->bdr == p_if->addr)
        {
            p_if->state = OSPF_IFS_BDR;
        }                         
        else
        {
            p_if->state = OSPF_IFS_DROTHER;
        }
        return;
    }
    
    old_state = p_if->state;
    
    /* step (1) */
    /*store current dr and bdr*/
    old_dr = p_if->dr;
    old_bdr = p_if->bdr;
    /*insert self as nbr*/
    memset(&self_nbr, 0, sizeof(self_nbr));
    self_nbr.priority = p_if->priority;
    self_nbr.addr = p_if->addr;
    self_nbr.id = p_process->router_id;
    self_nbr.state = OSPF_NS_FULL;
    self_nbr.dr = p_if->dr;
    self_nbr.bdr = p_if->bdr;    

    ospf_lstadd(&p_if->nbr_table, &self_nbr);

    /*calculate BDR*//* step (2) */
    ospf_elect_bdr(p_if);   

    self_nbr.dr = p_if->dr;
    self_nbr.bdr = p_if->bdr;    
    
    /*calculate DR*//* step (3) */ 
    ospf_elect_dr(p_if);  

    self_nbr.dr = p_if->dr;
    self_nbr.bdr = p_if->bdr;    
    
    /* step (4) - repeat steps (2) and (3) if necessary */
    /*
  
    ((p_if->addr == p_if->dr) &&  (old_dr != p_if->addr))
        || ((p_if->addr == p_if->bdr) &&  (old_bdr != p_if->addr))*/
    if ( ((old_dr != p_if->dr) && (old_dr == p_if->addr))
        || ((old_bdr != p_if->bdr) && (old_bdr == p_if->addr))
        || ((p_if->addr == p_if->dr) &&  (old_dr != p_if->addr))
        || ((p_if->addr == p_if->bdr) &&  (old_bdr != p_if->addr)))
    {
        ospf_elect_bdr(p_if); 

        self_nbr.dr = p_if->dr;
        self_nbr.bdr = p_if->bdr;    
        
        ospf_elect_dr(p_if);

        self_nbr.dr = p_if->dr;
        self_nbr.bdr = p_if->bdr;    
    }

    /*remove self nbr*/    
    ospf_lstdel(&p_if->nbr_table, &self_nbr);

    /*
    * Step (4) may not be carried out always. It's only assurances 
    that a router won't declare itself as DR as well as BDR
    * But a router can declared another router to be both. In that 
    case Step (4) is may not be carried out
    * So if DR and BDR are one and the same make the BDR NULL
    */
    if ((p_if->dr == p_if->bdr)  && (p_if->dr == p_if->addr))
    {
        p_if->bdr = 0;
    }
    
    /* step (5) */
    if (p_if->dr == p_if->addr)
    {
        p_if->state = OSPF_IFS_DR;                                                                                
    }
    else if (p_if->bdr == p_if->addr)
    {
        p_if->state = OSPF_IFS_BDR;                                                                        
    }
    else
    {		
		if(p_if->type != OSPF_IFT_PPP)
		{
			p_if->state = OSPF_IFS_DROTHER;                               
		}
    }
   /* step (6)  for NBMA: just become DR/BDR,invoking the
	    neighbor event Start for each neighbor having a Router  Priority of	0*/
    if (OSPF_IFT_NBMA == p_if->type
        &&(((p_if->dr == p_if->addr) && (old_dr != p_if->addr))
            ||((p_if->bdr == p_if->addr) && (old_bdr != p_if->addr))))
    {
        for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
        {
            if (0 == p_nbr->priority)
            {
                ospf_nsm(p_nbr, OSPF_NE_START);
            }
        }   
    }
   
    /*DR/BDR changed*//* step (7) */
    if ((p_if->dr != old_dr) || (p_if->bdr != old_bdr))
    {
        for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
        {
            if (OSPF_NS_2WAY <= p_nbr->state)
            {
                ospf_nsm(p_nbr, OSPF_NE_ADJOK);
            }
        }
        /*send hello directly*/
        ospf_logx(ospf_debug_hello, "send hello for dr changed");
        
        ospf_hello_output(p_if, FALSE, FALSE);
    }
    ospf_dr_changed(p_if, old_state, old_dr);
    return;
}

/*calculates the backup designated router*/
void 
ospf_elect_bdr (struct ospf_if *p_if)
{
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;    
    struct ospf_nbr *p_best = NULL;
    u_int declare = FALSE; 
    
    for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
    {
        if (OSPF_NS_2WAY > p_nbr->state)
        {
           /* skip this neighbor since it hasn't established 
            bidirectional communication with this router */
            continue;                
        }
        
        if (0 == p_nbr->priority)
        {
            /* skip this neighbor since it is not eligible to 
            become designated router */
            continue; 
        }
        
        if (p_nbr->dr == p_nbr->addr) 
        {
            /* skip this neighbor since it has declared itself to 
            be the designated router */
            continue;
        }
        
        if (p_nbr->bdr == p_nbr->addr)
        {
            /* neighbor has already declared itself to be backup 
            designated router */
            if (!declare)
            {
                p_best = p_nbr;
                declare = TRUE;
            }
            else if (0 > ospf_dr_priority_cmp (p_best, p_nbr))
            {
                p_best = p_nbr;
            }
        }
        else
        {
            /* neighbor has not declared itself to be backup 
            designated router */
            if (declare)
            {
                /* can't compete, skip it */
                continue;
            }

            if (0 > ospf_dr_priority_cmp (p_best, p_nbr))
            {
                p_best = p_nbr;
            }
        }
    }
	if(p_if->type != OSPF_IFT_PPP)
	{
		p_if->bdr = p_best ? p_best->addr : 0;
	}
	
    return;
}

/*calculates the designated router*/
void 
ospf_elect_dr (struct ospf_if *p_if)
{
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    struct ospf_nbr *p_best = NULL; 
    
    for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
    {
        if (OSPF_NS_2WAY > p_nbr->state)
        {
            /* skip this neighbor since it hasn't established 
            bidirectional communication with this router */
            continue; 
        }
        
        if (0 == p_nbr->priority)
        {
            /* skip this neighbor since it is not eligible 
            to become designated router */
            continue;                
        }
        
        if (p_nbr->dr == p_nbr->addr)    
        {
            /* neighbor has already declared itself to 
            be designated router */
            if (0 > ospf_dr_priority_cmp (p_best, p_nbr))
            {
                p_best = p_nbr;
            }
        }
    }
    
    if (NULL != p_best)
    {
        p_if->dr = p_best->addr;
    }
    else if (0 != p_if->bdr)
    {
        /* assign the designated router to be the same as the newly 
         elected backup designated router */
        p_if->dr = p_if->bdr;
    }
    else
    {
        /*if no dr on interface,set all zero address*/
        p_if->dr = 0;
    }
    return;
}

/*dr changed,rebuild some lsa*/
void 
ospf_dr_changed(
            struct ospf_if *p_if,   
            u_int ostate,
            u_int olddr)
{  
     /*prepare to originate network lsa on this interface if self is DR*/
    if (OSPF_IFS_DR == p_if->state)
    {
        ospf_network_lsa_originate(p_if);
    }

    /*A router that has formerly been the Designated Router for a network,
    but is no longer, should flush the network-LSA that it had previously SPR#75785
    if there is no neighbor on interface,do not need network lsa
     section 12.4.2
    */
    if (((p_if->dr != olddr) && (olddr == p_if->addr)) 
        || (NULL == ospf_nbr_first(p_if))) 
    {
        ospf_network_lsa_originate (p_if);
    }

    /* section 12.4, items (2) & (3) (page 115) */
    if ((p_if->state != ostate) || (p_if->dr != olddr))
    {       
        ospf_router_lsa_originate(p_if->p_area);
    } 
    
    /*add by yp 20100920 for n2x*/
    if ((p_if->dr != olddr) && (TRUE == p_if->te_enable))
    {         
        ospf_link_te_lsa_originate(p_if);
    }
    return;
}

/*ISM API*/

/*execute when interface comes up*/
/* section 9.3, State: Down (p. 66-67) */
void 
ospf_ism_up(struct ospf_if *p_if)
{
    struct ospf_process *p_process = p_if->p_process;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;

    /*restart current dr&bdr*/
    p_if->dr = 0;
    p_if->bdr = 0;
  	ospf_logx(ospf_debug,"#%d\r\n",__LINE__);
   
    /*fast hello*/     
    ospf_timer_start(&p_if->hello_timer, ospf_rand(p_if->hello_interval * OSPF_TICK_PER_SECOND));

    /*start poll timer for NBMA*/
    if (OSPF_IFT_NBMA == p_if->type)
    {
        ospf_stimer_start(&p_if->poll_timer, p_if->poll_interval);
    }

    /*if we are in restarting,do not send hello here*/
    if (!p_process->in_restart)   
    {
        ospf_hello_output (p_if, FALSE, FALSE);
    }

    /*set next state*/
    if ( (OSPF_IFT_PPP == p_if->type)
        || (OSPF_IFT_P2MP == p_if->type) 
        || (OSPF_IFT_VLINK == p_if->type)
        || (OSPF_IFT_SHAMLINK == p_if->type)) 
    {
        p_if->state = OSPF_IFS_PPP;
    }
    else if (!p_if->priority)
    {
        p_if->state = OSPF_IFS_DROTHER;
    }
    else
    {
        p_if->state = OSPF_IFS_WAITING;
        /*waiting for dead interval*/
        ospf_stimer_start(&p_if->wait_timer, p_if->dead_interval);
    }
    /*section9.3 for NBMA:generate the neighbor event Start for each neighbor that is
	also eligible to become Designated Router*/
    if ((OSPF_IFT_NBMA == p_if->type) && (0 < p_if->priority))
    {
        for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
        {
            if (0 < p_nbr->priority)
            {
                ospf_nsm( p_nbr, OSPF_NE_START);
            }
        }
    }
    /* section 12.4, item (2) (page 115) */                  
    ospf_router_lsa_originate(p_if->p_area);
    
    /*if interface enable te,try to regenerate type 10 lsa*/
    if (p_if->te_enable)
    {
        ospf_link_te_lsa_originate(p_if);
    }

    /*interface up can affect nssa lsa get forwarding addr,and so
    can affect originating nssa lsa  */
    ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
    p_process->import_update = TRUE;
    return;
}

/*execute when interface wait timer event is received*/
/* section 9.3, State: Waiting (p. 67) */
void 
ospf_ism_wait_timer(struct ospf_if *p_if)
{
    ospf_timer_stop(&p_if->wait_timer);
    ospf_elect_dr_bdr(p_if);
    return;
}

/*execute when interface backup seen event is received*/
/* section 9.3, State: Waiting (p. 67) */
void 
ospf_ism_backup_seen(struct ospf_if *p_if)
{    
    /*same as waiting*/
    ospf_ism_wait_timer(p_if);
    return;
}

/*execute when interface neighbor change event is received*/
/* section 9.3, State: DR Other, Backup, or DR (p. 68) */
void 
ospf_ism_nbrchange(struct ospf_if *p_if)
{
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    
    p_if->nbrchange = FALSE;
    ospf_timer_stop(&p_if->wait_timer);

    if ((OSPF_IFT_BCAST != p_if->type) && (OSPF_IFT_NBMA != p_if->type))
    {
        return;
    }

    /*add for smart-discover*/
    ospf_logx(ospf_debug_nbrchange, "send hello for nbr changed");

    ospf_stimer_start(&p_if->hello_timer, 1);
    
    
    /*if some neighbor on this interface is in 
     restarting,do not calculate dr/bdr*/
    for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
    {
        if (p_nbr->in_restart)
        {
            return;
        }
    }
    ospf_elect_dr_bdr(p_if);
    return;
}

/*execute when interface down event is received*/
/* section 9.3, State: Any (p. 68) */
void 
ospf_ism_down(struct ospf_if *p_if)
{
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next = NULL;
    struct ospf_process *p_process = p_if->p_process;
  	ospf_logx(ospf_debug,"#%d\r\n",__LINE__);
    
    ospf_timer_stop(&p_if->wait_timer);
   
    /*clear interface dynamic resource*/
    p_if->state = OSPF_IFS_DOWN;
  
    if (!p_process->in_restart)
    {
        p_if->dr = 0;
        p_if->bdr = 0;
    }
    /*delete nbr*/ 
    /*NBMA nbr only delete in interface delete*/   
    if (OSPF_IFT_NBMA != p_if->type)
    {
        for_each_ospf_nbr(p_if, p_nbr, p_next)
        {
            ospf_nsm(p_nbr, OSPF_NE_LL_DOWN);
            ospf_nbr_delete(p_nbr);
        }
    }

    /*rebuild area router lsa*/
    ospf_router_lsa_originate(p_if->p_area);
    
    /*interface up can affect nssa lsa get forwarding addr,and so
    can affect originating nssa lsa  */
    ospf_stimer_start(&p_process->import_timer, OSPF_IMPORT_INTERVAL);
    p_process->import_update = TRUE;
    return;
}


/*Execute interface state machine*/
void 
ospf_ism(
        struct ospf_if *p_if,
        u_int event)
{
    u_int old_state = p_if->state;

    /*do nothing for interface without area*/
    if (NULL == p_if->p_area)
    {
        return;
    }
    if (ospf_debug_if)
    {
        ospf_log_ism(p_if, old_state, event);
    }
    
    p_if->stat->state_change++;
    p_if->stat->if_event[event]++;

    switch (event){
        case OSPF_IFE_UP:        
             if (OSPF_IFS_DOWN == old_state)
             {
                 ospf_ism_up(p_if);
             }              
             break;
        
        case OSPF_IFE_WAIT_TIMER:        
             if (OSPF_IFS_WAITING == old_state)
             {
                 ospf_ism_wait_timer(p_if);
             }
             break;
        
        case OSPF_IFE_BACKUP_SEEN:        
             if (OSPF_IFS_WAITING == old_state)
             {
                 ospf_ism_backup_seen(p_if);
             }
             break;
        
        case OSPF_IFE_NBR_CHANGE:        
             if((OSPF_IFS_DROTHER == old_state)
                 ||(OSPF_IFS_DR == old_state)
                 ||(OSPF_IFS_BDR == old_state))
             {
                 ospf_ism_nbrchange(p_if);
             }
             break;
        
        case OSPF_IFE_LOOP:
             /*not support*/
             break;
             
        case OSPF_IFE_UNLOOP:
             /*not support*/
             break;
             
        case OSPF_IFE_DOWN:        
             if (OSPF_IFS_DOWN != old_state)
             {
                 ospf_ism_down(p_if);
             }
             break;
        
        default:
             break;
    }

    /*send trap when state changed*/
    if (old_state != p_if->state)
    {
        ospf_sync_event_set(p_if->syn_flag);
        if (OSPF_IFT_VLINK == p_if->type)
        {
            ospf_trap_vifstate(old_state, p_if);
        }
        else 
        {
            ospf_trap_ifstate(old_state, p_if);
        }
    }
	
    return;
}

 
 /*check ip interface state if checking is required*/   
 /***********************************************
 ospf_if_state_update
 Because link up/down notfitication may return error when mbuf is empty,
 we create this function to update ospf interface state in ospf task.
 for each non-virtual interface,if link state is down and prototol state is not down,
 trigger an interface down event;
 else if link state is up and protocol state is down,trigger an interface up event.
 Input :
 struct ospf_if *p_if
 Output:
 none
 Return :
 none
 */
void 
ospf_if_state_update(struct ospf_if *p_if)
{
    u_int flag = 0;
	u_int ulNetIndex = 0;
	u_int ulLink = p_if->link_up;
	u_int uiCount = 0;

    ospf_logx(ospf_debug_if,"#%d,type=%d.\r\n",__LINE__,p_if->type);
    /*update virtual link */
    if (OSPF_IFT_VLINK == p_if->type)
    {
        ospf_vif_state_update (p_if);
        return;
    }
    
    /*always UP*/
    if (OSPF_IFT_SHAMLINK == p_if->type)
    {
        p_if->link_up = TRUE;
    }
    /*real interface,get system flag*/
    else
    {
        ospf_sys_ifflag_get(p_if->ifnet_uint, p_if->addr, &flag);
 		ospf_logx(ospf_debug_if,"#%d,type=%d.link_up=%d.\r\n",__LINE__,p_if->type,p_if->link_up);
		p_if->link_up = (flag == OSPF_IFE_UP) ? TRUE : FALSE;
    }
#ifdef OSPF_DCN
	if (OSPF_DCN_FLAG == p_if->ulDcnflag)
	{
		p_if->link_up = TRUE;
	}
#endif
 	ospf_logx(ospf_debug_if,"#%d,flag=%d,ifnet_uint=0x%x,addr=0x%x,link_up=%d.\r\n",__LINE__,flag,p_if->ifnet_uint,p_if->addr,p_if->link_up);

	if(ospf_if_index_to_sys_index(p_if->ifnet_uint,&ulNetIndex) == ERROR)
    {
        return ERROR;
    }
    
	if(IFINDEX_INTERNAL == ulNetIndex)
	{
	 	ulNetIndex = 0;
	}
	p_if->ifnet_index = ulNetIndex;

    if ((ulLink == FALSE)
        && (p_if->link_up == TRUE)
        && (p_if->ucLdpSyncEn == TRUE))
    {
        //zhurish ldpGlobalGetApi(&p_if->ifnet_uint, LDP_GLB_ADJ_COUNT_IFUNIT, &uiCount);
        /*没有ldp邻居时，才能启动hold down*/
        if(uiCount == 0)
        {
            ospf_if_hold_down_start(p_if);
        }
    }

    /*calculate cost*/
    if (!p_if->configcost)
    {
        ospf_if_cost_update(p_if, 0, 0);   
    }
  
    /*do ism according to state*/
    if ((TRUE == p_if->link_up) && (OSPF_IFS_DOWN == p_if->state))
    {
 	    ospf_logx(ospf_debug_if,"#%d,state=%d.link_up=%d.\r\n",__LINE__,p_if->state,p_if->link_up);
        ospf_ism(p_if, OSPF_IFE_UP);
    }
    else if  ((FALSE == p_if->link_up) && (OSPF_IFS_DOWN != p_if->state))
    {
        ospf_ism(p_if, OSPF_IFE_DOWN);
    }
 	ospf_logx(ospf_debug_if,"#%d,type=%d.link_up=%d.\r\n",__LINE__,p_if->type,p_if->link_up);
    return ;
}

/*update metric of one special interface
*
* This routine set one ospf interface's cost,if cost changed,need 
* regenerate router lsa and then rebuild routing table
*/
void 
ospf_if_cost_update(
                struct ospf_if *p_if, 
                u_int tos,
                u_int metrics)
{
    if ((NULL == p_if) || (NULL == p_if->p_area))
    {
        return;
    }
    
    /*tos:1-7 don't affect ospf,set directly*/
    if ((0 != tos) && (OSPF_MAX_TOS > tos))
    {
        p_if->cost[tos] = metrics;
        return;
    }
        
    /*if input cost is 0,mean auto calculated*/         
    if (0 == metrics)
    {
        /*clear auto  flag*/
        p_if->configcost = FALSE;
        
        /*calculate cost*/
        if (!ospf_if_is_loopback(p_if->ifnet_index))
        {
            metrics = ospf_if_cost_calculate(p_if->p_process->reference_rate, p_if);  
        }
    }
    else
    {
        /*set manual flag*/
        p_if->configcost = TRUE;
    }    
    /*cost has no change,do nothing*/
    if (p_if->cost[0] == metrics)
    {
        return;
    }
    p_if->cost[0] = metrics;

    /*schedule sync msg for interface*/
    ospf_sync_event_set(p_if->syn_flag);

    /*regenerate router lsa for this area*/
    ospf_router_lsa_originate(p_if->p_area);
    return;
}

/** This routine calculate ospf cost of one interface 
    according to interface type and reference cost
* according to rfc4750,ospfReferenceBandwidth indicate interface bandwith 
* in kb/s;so 100M ethernet interface has cost:100000000/1000 = 100000;
* it is also the default value
*/
u_short 
ospf_if_cost_calculate(
           u_int rate,/*reference rate in Mb/s*/ 
           struct ospf_if *p_if)
{
    /*speed :Mb/s*/
    u_int speed = 100;/*default is 100M*/
    
    ospf_sys_ifspeed_get(p_if->ifnet_uint, p_if->addr, &speed);
  
    /*if obtained value is  invalid,use default one*/   
    return ((rate < speed) || (!speed)) ? 1 : (rate/speed);
}

/*get vlink's nbr ip address according to routerlsa*/
void 
ospf_vif_nbr_update(
    struct ospf_nbr *p_nbr,
    struct ospf_route *p_route)
{
    struct ospf_lshdr lshdr;
    struct ospf_if *p_if = p_nbr->p_if;
    struct ospf_area *p_transit_area = p_if->p_transit_area;
    struct ospf_lsa *p_lsa = NULL;
    struct ospf_router_link *p_link = NULL;
    struct ospf_router_lsa *p_router = NULL;
    struct ospf_process *p_process = p_if->p_process;
    struct ospf_route *p_link_route = NULL;
    struct ospf_path *p_path = NULL;
    struct ospf_path *p_abr_path = NULL;
    u_int current = p_process->current_route;

    p_abr_path = &p_route->path[current];
    
    /*route exist, decide remote address,scan for all link in abr's router lsa*/
    lshdr.type = OSPF_LS_ROUTER;
    lshdr.id = htonl(p_route->dest);
    lshdr.adv_id = htonl(p_route->dest);
        
    p_lsa = ospf_lsa_lookup(p_transit_area->ls_table[lshdr.type], &lshdr);
    if (NULL == p_lsa)
    {
        return;
    }
    
    p_router = (struct ospf_router_lsa *)p_lsa->lshdr;
    for_each_router_link(p_router, p_link)
    {
        if ((OSPF_RTRLINK_PPP != p_link->type) 
             && (OSPF_RTRLINK_TRANSIT != p_link->type))
        {
            continue;
        }
   
        /*nexthop same, or this is local route*/
        p_link_route = ospf_fwd_route_lookup(p_process, ntohl(p_link->data), p_transit_area);
        if (NULL == p_link_route)
        {
            continue;
        }
   
        p_path = &p_link_route->path[current];

	//	ospf_logx(ospf_debug_if,"%s %d  ****ospf_if_lookup*******\n", __FUNCTION__,__LINE__);
        if ((p_path->p_nexthop->gateway[0].addr 
              == p_abr_path->p_nexthop->gateway[0].addr)
            ||(NULL != ospf_if_lookup(p_process, p_path->p_nexthop->gateway[0].addr)))
        {
            p_nbr->addr = ntohl(p_link->data);
            break;
        }
    }
    return;
}

/*bring up virtual links if configured*/
void 
ospf_vif_state_update (struct ospf_if *p_if)
{
    struct ospf_area *p_transit_area = p_if->p_transit_area;
    struct ospf_route *p_route = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_process *p_process = p_if->p_process;
    struct ospf_if *p_if_source = NULL;
    struct ospf_path *p_abr_path = NULL;
    u_int current = p_process->current_route;
    u_int8 vnbrstr[32];

    /*if neighbor not exist, create one*/
    p_nbr = ospf_nbr_first(p_if);
    if (NULL == p_nbr)
    {
        p_nbr = ospf_nbr_create(p_if, 0);
        if (NULL == p_nbr)
        {
            return;
        }
    }
    ospf_logx(ospf_debug_if, "check virtual link %s,state %d", ospf_inet_ntoa(vnbrstr, p_nbr->id), p_if->state);

    /*abr route must exist*/
    p_route = ospf_abr_lookup(p_transit_area, p_nbr->id);
    if (NULL != p_route)
    {
        p_abr_path = &p_route->path[current];
    }
    
    if ((NULL == p_route)
        || (OSPF_PATH_INVALID == p_abr_path->type)
        || (NULL == p_abr_path->p_nexthop)
        || (p_abr_path->p_area != p_transit_area))
    {
        p_route = NULL;
    }

    /*route exist, decide remote address,scan for all link in abr's router lsa*/
    if (NULL != p_route )
    {        
        ospf_vif_nbr_update(p_nbr, p_route);        
    }

    /*if route not exist,or transit area error,shutdown interface*/
    if ((NULL == p_route)
        || (ospf_invalid_metric(p_abr_path->cost))
        || p_transit_area->is_stub
        || p_transit_area->is_nssa)
    {
        ospf_logx(ospf_debug_if, "abr route or area invalid");
    
        if (OSPF_IFS_DOWN != p_if->state)
        {
            ospf_ism(p_if, OSPF_IFE_DOWN);
        }
        p_if->addr = 0; /* take link down */
    }
    else if (OSPF_IFS_DOWN == p_if->state)
    {
        ospf_logx(ospf_debug_if, "try to bring up virtual link");
        
        /* We have a route and link is down - bring it up */
        p_if->cost[0] = p_abr_path->cost;

        /* Spec appendix C.4 */
        p_if_source = ospf_if_lookup_by_network(p_process, p_abr_path->p_nexthop->gateway[0].addr);
        if (NULL != p_if_source)
        {
            p_if->addr = p_if_source->addr;
        }
        ospf_ism(p_if, OSPF_IFE_UP);
    }
    else
    {
        ospf_logx(ospf_debug_if, "update virtual link cost and address");

        p_if_source = ospf_if_lookup_by_network(p_process, p_abr_path->p_nexthop->gateway[0].addr);
        if ((p_if->cost[0] != p_abr_path->cost) 
            ||((NULL != p_if_source) && (p_if->addr != p_if_source->addr)))
        {
            p_if->cost[0] =  p_abr_path->cost;
            
            /* Spec appendix C.4 */ 
            p_if->addr = p_if_source->addr;
                               
            /*regenerate router lsa of backbone*/
            if (NULL != p_if->p_area)
            {
                ospf_router_lsa_originate(p_if->p_area);
            }
        }
    }          
    return;
}

/*act on interface address change event*/
void 
ospf_if_addr_change(
                 u_int vrid, 
                 u_int if_addr, 
                 int add,
                 u_int ulIfIndex)
{
    struct ospf_network *p_network = NULL;
    struct ospf_area *p_area = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    u_int uiIfEnable = 0;
    struct ospf_network *p_next = NULL;
    u_char ucNetworkMatch = FALSE;
    struct ospf_if p_old_if;

    /*check if network configured,check all instance*/
    for_each_ospf_process(p_process, p_next_process)
    {
       /*ignore invalid process*/
        if (p_process->proto_shutdown)
        {
            continue;
        }
        /*ignore different vrf id*/
        if (p_process->vrid != vrid && add == ZEBRA_INTERFACE_ADDRESS_ADD)
        {
            continue;
        }
#ifdef OSPF_DCN
        if(p_process->process_id == OSPF_DCN_PROCESS)
        {
            continue;
        }
#endif
        ospf_set_context(p_process);
        /*if network exist and new address add,add ospf interface;
           if address is deleted,delete matched ospf interface*/  

        p_if = ospf_if_lookup_by_ifindex(p_process, ulIfIndex);
        ospf_logx(ospf_debug_if, "update ip addr p_if %p ifindex = %x",p_if,ulIfIndex);
        if(add == ZEBRA_INTERFACE_ADDRESS_ADD && p_if && p_if->addr == if_addr)
        {
            continue;
        }
        for_each_node(&p_process->network_table, p_network, p_next)
        {
            if(ospf_netmatch(if_addr, p_network->dest, p_network->mask))
            {
                ucNetworkMatch = TRUE;
                break;
            }
		}
		
        if(NULL != p_if && add == ZEBRA_INTERFACE_ADDRESS_ADD)
        {
            uiIfEnable = p_if->pif_enable;
            p_area = p_if->p_area;
            p_old_if.priority = p_if->priority;
            p_old_if.type = p_if->type;
            p_old_if.tx_delay =  p_if->tx_delay;
            p_old_if.rxmt_interval = p_if->rxmt_interval;
            p_old_if.hello_interval = p_if->hello_interval;
            p_old_if.dead_interval = p_if->dead_interval;
            p_old_if.cost[0] = p_if->cost[0];
            p_old_if.ucLdpSyncEn = p_if->ucLdpSyncEn;
            p_old_if.mtu_ignore = p_if->mtu_ignore;
            p_old_if.ulHoldCostInterval = p_if->ulHoldCostInterval;
            p_old_if.ulHoldDownInterval = p_if->ulHoldDownInterval;

            ospf_if_delete(p_if);                
            if (NULL != p_area)
            {
                if(uiIfEnable == TRUE || ucNetworkMatch)
                {
                    ospf_real_if_create(p_process, if_addr, p_area);

                    if(uiIfEnable == TRUE)
                    {
                        p_if = ospf_if_lookup_by_ifindex(p_process, ulIfIndex);

                        if(NULL != p_if)
                        {
                            p_if->pif_enable = TRUE;
                        }

                        p_if->priority = p_old_if.priority;
                        p_if->type = p_old_if.type;
                        p_if->tx_delay =  p_old_if.tx_delay;
                        p_if->rxmt_interval = p_old_if.rxmt_interval;
                        p_if->hello_interval = p_old_if.hello_interval;
                        p_if->dead_interval = p_old_if.dead_interval;
                        p_if->ucLdpSyncEn = p_old_if.ucLdpSyncEn;
                        p_if->mtu_ignore = p_old_if.mtu_ignore;
                        p_if->ulHoldCostInterval = p_old_if.ulHoldCostInterval;
                        p_if->ulHoldDownInterval = p_old_if.ulHoldDownInterval;
                        p_if->cost[0] = p_old_if.cost[0];
                        ospf_if_cost_update(p_if,0,p_if->cost[0]);
                    }
                }
            }
        }
        else if(NULL != p_if && add == ZEBRA_INTERFACE_ADDRESS_DELETE)
        {
            uiIfEnable = p_if->pif_enable;
            p_area = p_if->p_area;
            ospf_if_delete(p_if); 
            if(uiIfEnable == TRUE)
            {
                ospf_real_if_create_by_ifindex(p_process, ulIfIndex, p_area);
                p_if = ospf_if_lookup_by_ifindex(p_process, ulIfIndex);

                if(NULL != p_if)
                {
                    p_if->pif_enable = TRUE;
                }
            }
         /*when delete if addr, delete route with the addr as nexthop in redistribute list*/
        /*uesed for redistribute connect, forward addr in routedel notify is meaningless*/
            ospf_import_delete_by_nexthop(p_process, if_addr);
        }
        else if(NULL == p_if && add == ZEBRA_INTERFACE_ADDRESS_ADD)
        {
            if(ucNetworkMatch && p_network)
            {
                p_area = ospf_area_lookup(p_process,p_network->area_id);
                ospf_real_if_create(p_process, if_addr, p_area);
            }
        }
        #if 0
        p_if = ospf_if_lookup(p_process, if_addr);
        if (add && (NULL == p_if))
        {
            p_network = ospf_network_lookup(p_process, if_addr, 0);
            if (NULL != p_network)
            {
                /*if area not exist,create one or do nothing ?*/        
                p_area = ospf_area_lookup(p_process, p_network->area_id);
                if (NULL != p_area)
                {
                    ospf_real_if_create(p_process, if_addr, p_area);
                    return;
                }
            }
        }
        else if ((add == ZEBRA_REDISTRIBUTE_TYPE_IF_ADDR_DEL) && (NULL != p_if))
        {
            ospf_if_delete(p_if);
        }
        else if(add && (NULL != p_if))
        {
            #if 0
            ospf_if_delete(p_if);
            p_network = ospf_network_lookup(p_process, if_addr, 0);
            if (NULL != p_network)
            {
                /*if area not exist,create one or do nothing ?*/        
                p_area = ospf_area_lookup(p_process, p_network->area_id);
                if (NULL != p_area)
                {
                    ospf_real_if_create(p_process, if_addr, p_area);
                    return;
                }
            }
            #endif
        }
        #endif
    }    
    return;
}

/*hello timer expired,send hello and restart timer*/
void 
ospf_if_hello_timeout(struct ospf_if *p_if)
{        
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        ospf_logx(ospf_debug_if, "do not send hello in slave mode");

        return;
    }
    ospf_hello_output (p_if, FALSE, FALSE);
    
    ospf_timer_start(&p_if->hello_timer,ospf_rand(p_if->hello_interval * OSPF_TICK_PER_SECOND));
    return;
}

/*poll timer expired,send hello and poll timer*/
void 
ospf_if_poll_timeout(struct ospf_if *p_if)
{        
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    ospf_hello_output (p_if, FALSE, TRUE);
    ospf_stimer_start(&p_if->poll_timer, p_if->poll_interval);
    return;
}

/*scheduled delay ack sending*/
void 
ospf_if_send_delayack(struct ospf_if *p_if)
{
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    /*send ack msg*/
    ospf_ack_output(&p_if->delay_ack);
     /*buffer has no use,release it*/
    if (NULL != p_if->delay_ack.p_msg)
    {
        ospf_mfree(p_if->delay_ack.p_msg, OSPF_MPACKET);
        p_if->delay_ack.p_msg = NULL;
    }
    return;
}

/*interface waiting timer expired,try to do dr election*/
void 
ospf_if_wait_timeout(struct ospf_if *p_if)
{
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }
    ospf_set_context(p_if->p_process);
    ospf_ism(p_if, OSPF_IFE_WAIT_TIMER);
    return;
}

/*scheduled flood timer expired,send flood buffer*/
void
ospf_if_flood_timeout(struct ospf_if *p_if)
{
    if (OSPF_MODE_SLAVE == ospf.work_mode)
    {
        return;
    }

    if (NULL == p_if->p_area)
    {
        return;
    }
    
     /*send lsa update */ 
    ospf_update_output(&p_if->update);

    /*free update buffer*/ 
    if (p_if->update.p_msg)
    {
        ospf_mfree(p_if->update.p_msg, OSPF_MPACKET);
        p_if->update.p_msg = NULL;
        p_if->update.maxlen = 0;
    }       
    return;
}

void
ospf_if_set_packet_block(struct ospf_process *p_process,u_int uiValue)
{
	struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
	
	for_each_ospf_if(p_process, p_if, p_next_if)
    {
		p_if->passive = uiValue;
	}	
}


void ospf_if_hold_down_start(struct ospf_if *p_if)
{
    if ((p_if == NULL)
        || (p_if->ucLdpSyncEn == FALSE))
    {
        return;
    }
    /*hold down timer is start*/
    p_if->ulOspfSyncState = OSPF_LDP_HOLD_DOWN;
    ospf_logx(ospf_debug_rtm,"ospf ldp sync change to OSPF_LDP_HOLD_DOWN %s %d\r\n",__FUNCTION__,__LINE__);
    ospf_stimer_start(&p_if->hold_down_timer, p_if->ulHoldDownInterval);
}


void ospf_if_hold_down_stop(struct ospf_if *p_if)
{
    if ((p_if == NULL)
        || (ospf_timer_active(&p_if->hold_down_timer) == FALSE))
    {
        return;
    }

    ospf_timer_stop(&p_if->hold_down_timer);
}


/*ldp-sync hold-down timer*/
void
ospf_if_hold_down_timeout(struct ospf_if *p_if)
{
    if (p_if == NULL)
    {
        return;
    }
    p_if->ucHoldCostState = TRUE;
    ospf_if_hold_cost_start(p_if, OSPF_LDP_INIT_MSG);
}


void ospf_if_hold_cost_start(struct ospf_if *p_if, u_int uiLdpMsgType)
{
    if ((p_if == NULL)
        || (p_if->ucLdpSyncEn == FALSE)
        || (p_if->ucHoldCostState == FALSE)
        || (p_if->ulOspfSyncState == OSPF_LDP_HOLD_NORMAL_COST))
    {
        return;
    }
    if (ospf_timer_active(&p_if->hold_cost_timer) == FALSE)
    {
        /*hold cost timer is start*/
        p_if->ulOspfSyncState = OSPF_LDP_HOLD_MAX_COST;
        ospf_logx(ospf_debug_rtm,"ospf ldp sync change to OSPF_LDP_HOLD_MAX_COST %s %d\r\n",__FUNCTION__,__LINE__);
        ospf_stimer_start(&p_if->hold_cost_timer, p_if->ulHoldCostInterval);

        p_if->ucHoldCostState = FALSE;
        /*正常流程已经泛洪，而错误流程没有泛洪，所以需要加判定泛洪*/
        if(uiLdpMsgType != OSPF_LDP_UP_MSG)
        {
            ospf_router_lsa_originate(p_if->p_area);
        }
    }
}


void ospf_if_hold_cost_stop(struct ospf_if *p_if)
{
    if ((p_if == NULL)
        || (ospf_timer_active(&p_if->hold_cost_timer) == FALSE))
    {
        return;
    }
    ospf_timer_stop(&p_if->hold_cost_timer);
    p_if->ucHoldCostState = FALSE;
    /*send normal cost packet*/
    ospf_router_lsa_originate(p_if->p_area);
    /********************/
    /*hold cost timer end,mpls ldp state is init*/
    p_if->ulOspfSyncState = OSPF_LDP_HOLD_NORMAL_COST;
    ospf_logx(ospf_debug_rtm,"ospf ldp sync change to OSPF_LDP_HOLD_NORMAL_COST %s %d\r\n",__FUNCTION__,__LINE__);
}


/*ldp-sync  hold-max-cost timer*/
void
ospf_if_hold_cost_timeout(struct ospf_if *p_if)
{
    if (p_if == NULL)
    {
        return;
    }

    if (ospf_nbr_search(p_if) != 0)
    {
        p_if->ulOspfSyncState = OSPF_LDP_HOLD_NORMAL_COST;
        ospf_logx(ospf_debug_rtm,"ospf ldp sync change to OSPF_LDP_HOLD_NORMAL_COST %s %d\r\n",__FUNCTION__,__LINE__);
    }
    /*send normal cost packet*/
    ospf_router_lsa_originate(p_if->p_area);
    /********************/
}


