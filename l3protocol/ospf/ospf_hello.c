/* ospf_hello.c - used for sending, receiving and processing Hello packets */
#include "ospf.h"


/***********************************************************************************************************************************/
 u_int ospf_hello_is_valid (struct ospf_hello_msg *p_hello,struct ospf_if *p_if);
 u_int ospf_hello_2way_check (u_int id, struct ospf_hello_msg *p_hello);

void 
ospf_hello_build(
               struct ospf_if *p_if,
               u_int goingdown,
               struct ospf_hello_msg *p_hello)
{
    struct ospf_process *p_process = p_if->p_process;
    u_int *p_hello_nbr = NULL;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    
    memset(p_hello, 0, sizeof(struct ospf_hello_msg));

    /*network mask*/    
    p_hello->mask = (OSPF_IFT_VLINK == p_if->type) ? 0 : htonl (p_if->mask);

    /*hello interval*/    
    p_hello->hello_interval = htons ((u_short) p_if->hello_interval);

    /*options*/    
    p_hello->option = 0;   

    /*E/N bit:
        normal:E=1,N=0
        sutb:E=0,N=0
        nssa:E=0,N=1
     */
    if (p_if->p_area->is_nssa) 
    {        
        ospf_set_option_nssa(p_hello->option);
    }
    else if (!p_if->p_area->is_stub)
    {
        ospf_set_option_external(p_hello->option);
    }

    /*opaque*/
    if (p_process->opaque)
    {
        ospf_set_option_opaque(p_hello->option);//zhurish
    }
    p_hello->priority = p_if->priority;

    /*dead interval*/ 
	
	if(p_if->dead_interval ==  OSPF_DEFAULT_ROUTER_DEAD_INTERVAL)
	{
		p_hello->dead_interval = htonl(p_if->hello_interval*4);
	}
	else
	{
		p_hello->dead_interval = htonl(p_if->dead_interval);
	}
    /*set fixed length of packet*/
    p_hello->h.type = OSPF_PACKET_HELLO;
    p_hello->h.len = OSPF_HELLO_MIN_LEN;
        
    /*fill dr/bdr if this is normal hello*/
    /*fill nbr information for normal hello*/        
    if (!goingdown)
    {
        p_hello->dr = htonl(p_if->dr);
        p_hello->bdr = htonl(p_if->bdr);

        p_hello_nbr = p_hello->nbr;
        for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
        {
            if ((OSPF_NS_DOWN != p_nbr->state) || (p_nbr->in_restart))
            {
                *p_hello_nbr = htonl (p_nbr->id);
                p_hello_nbr ++ ;
                p_hello->h.len += 4;
            }
        }
    }
    return;
}

/*send hello on nbma interface*/
void
ospf_nbma_hello_output(
                struct ospf_if *p_if,
                u_int poll_flag,
                struct ospf_hello_msg *p_hello)
{
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;

    if (0 < p_if->priority)
    {
        if ((p_if->dr == p_if->addr) ||p_if->bdr== p_if->addr)
        {
            for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
            { 
                if ((!poll_flag && (OSPF_NS_DOWN < p_nbr->state))
                    || (poll_flag && (OSPF_NS_DOWN == p_nbr->state)))
                {
                    ospf_logx(ospf_debug_hello,"ospf_nbma hello ,nbr addr %x,hello len %d",p_nbr->addr,p_hello->h.len);
                    ospf_output(p_hello, p_if, p_nbr->addr, p_hello->h.len);
                }                        
            }
        }
        else
        {
            for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
            { 
                if (0 < p_nbr->priority)
                {
                    if ((!poll_flag && (OSPF_NS_DOWN < p_nbr->state))
                        || (poll_flag && (OSPF_NS_DOWN == p_nbr->state)))
                    {
                        ospf_output(p_hello, p_if, p_nbr->addr, p_hello->h.len);
                    }
                }                    
            }
        }
    }
    else
    {
        /*    If the router is not eligible to become Designated Router,
        it must periodically send Hello Packets to both the
        Designated Router and the Backup Designated	Router (if they
        exist)*/
        if (p_if->dr)
        { 
            ospf_output(p_hello, p_if, p_if->dr, p_hello->h.len);
        }
        if (p_if->bdr)
        { 
            ospf_output(p_hello, p_if, p_if->bdr, p_hello->h.len);
        }
    }
    return;
}

/* section 9.5 of OSPF specification (page 71) */
void 
ospf_hello_output (
                struct ospf_if *p_if,
                u_int goingdown,
                u_int poll_flag)
{
    u_int8 buf[OSPF_MAX_TXBUF];
    struct ospf_hello_msg *p_hello = (struct ospf_hello_msg *)buf;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next_nbr = NULL;
    struct ospf_process *p_process = p_if->p_process;
    struct ospf_lshdr lshdr;
    u_int dest = OSPF_ADDR_ALLSPF;/*default address is mcast*/
    u_int8 addrstr[32];
    int iRet = 0;
	
    /*if self is in restarting,and there is no restart lsa,reoriginate it*/
    if (p_process->in_restart)
    {
        memset(&lshdr, 0, sizeof(lshdr));
        lshdr.type = OSPF_LS_TYPE_9;
        lshdr.id = htonl(OSPF_GR_LSID);
        lshdr.adv_id = htonl(p_process->router_id);
        if (NULL == ospf_lsa_lookup(&p_if->opaque_lstable, &lshdr))
        {
            ospf_restart_lsa_originate(p_if);
            /*send update before hello*/
            ospf_update_output(&p_if->update);
            if (p_if->update.p_msg)
            {
                ospf_mfree(p_if->update.p_msg, OSPF_MPACKET);
                p_if->update.p_msg = NULL;
                p_if->update.maxlen = 0;
            }       
        }
    }

    /*hold_down定时器起来时不允许hello报文*/
    if(ospf_timer_active(&p_if->hold_down_timer) == TRUE)
    {
        return;
    }

	if(ospf_if_is_trunk(p_if->ifnet_uint)
	    && (p_if->link_up == FALSE))
	{
        return;
	}
	
    ospf_hello_build(p_if, goingdown, p_hello);
       
    /*unicast hello*//*begin*/
    /*if configured unicast hello,send unicast hello to each learnt neighbor*/
    if (p_if->unicast_hello) 
    {                
        for_each_ospf_nbr(p_if, p_nbr, p_next_nbr)
        { 
            ospf_output(p_hello, p_if, p_nbr->addr, p_hello->h.len);
        }
    }

    /*nbma support*/
    if (OSPF_IFT_NBMA == p_if->type)
    {
        ospf_nbma_hello_output(p_if, poll_flag, p_hello);
        return;
    }
    /*mcast sending*/
	dest = ospf_packet_dest_select(p_if, NULL, OSPF_PACKET_HELLO);
	
    if((p_if->type != OSPF_IFT_VLINK)&&(p_if->type != OSPF_IFT_SHAMLINK))
    {
        iRet = ospfGetPortLinkStates(p_if->ifnet_uint);
        if (ERR == iRet)
    	{
    		ospf_logx(ospf_debug_hello,"(ERR == iRet) p_if->ifnet_index: 0x%x,ifnet_uint: 0x%x,\r\n", p_if->ifnet_index,p_if->ifnet_uint);
    		return;
	    }
    }
    ospf_logx(ospf_debug_hello, "send hello to %s on %s,length %d",
       ospf_inet_ntoa(addrstr, dest), p_if->name, p_hello->h.len);

    ospf_output(p_hello, p_if, dest, p_hello->h.len);
    if (ospf_debug_hello && ospf_debug_msg)
    {
        ospf_log_hello_packet(p_hello);
        ospf_logx(ospf_debug_hello, "");
    }    
    return;
}

 struct ospf_nbr *
ospf_nbr_create_by_hello(
                        struct ospf_if *p_if,
                        struct ospf_hello_msg *p_hello,
                        u_int source)    
{
    struct ospf_nbr *p_nbr = NULL;
    u_int8 str[64];
    
    if (OSPF_IFT_VLINK == p_if->type)
    {
        /*vlink create nbr and update nbr addr*/
        ospf_vif_state_update (p_if);
        p_nbr = ospf_nbr_first(p_if);
    }/*NBMA not create nbr when config peer*/
    else if (OSPF_IFT_NBMA != p_if->type)
    {   
    
        p_nbr = ospf_nbr_create(p_if, source);
    }
    
    if (NULL != p_nbr)
    {
        p_nbr->dr = ntohl(p_hello->dr);  
        p_nbr->bdr = ntohl(p_hello->bdr);  
        p_nbr->priority = p_hello->priority;  
        
        /*add for smart-discover,prepare fast hello*/
        ospf_stimer_start(&p_if->hello_timer, 1);
   
        if (ospf_debug_gr && p_if->p_process->in_restart)
        {
            ospf_logx(ospf_debug_hello, "create nbr %s during restart", ospf_inet_ntoa(str, source));
        }
    } 
    return p_nbr;
}

/*decide if dr&bdr election need when hello rxd*/
 u_int
ospf_hello_is_nbr_changed(
                          u_int new_nbr,
                          struct ospf_nbr *p_nbr,
                          struct ospf_hello_msg *p_hello)
{
    struct ospf_if *p_if = p_nbr->p_if;
    u_int dr = ntohl(p_hello->dr);
    u_int bdr = ntohl(p_hello->bdr);
    u_int8 drstr[32];
    u_int8 bdrstr[32];
    
    /*if priority changed and it is not new neighbor,trigger nbrchange*/
    if (p_nbr->priority != p_hello->priority)
    {
        if (ospf_debug_hello || ospf_debug_gr)
        {
            ospf_logx(ospf_debug_hello, "priority changed in hello"); 
        }
        return TRUE;
    }

    /*if nbr is in restarting,do not check change for dr&bdr*/
    if (p_nbr->in_restart)
    {
        if (ospf_debug_hello || ospf_debug_gr)
        {
           ospf_logx(ospf_debug_hello, "neighbor is in restarting,no check DR/BDR in hello");   
        }
        return FALSE;
    }
    
    /*section 10.5 bullet4 and 5*/
    if ((dr != p_nbr->dr)  || (bdr != p_nbr->bdr))
    {        
        if (ospf_debug_hello || ospf_debug_gr)
        {
            ospf_logx(ospf_debug_hello, "DR/BDR in hello changed");
            ospf_logx(ospf_debug_hello, "rxd dr %s, bdr %s", ospf_inet_ntoa(drstr, dr), ospf_inet_ntoa(bdrstr, bdr));
            ospf_logx(ospf_debug_hello, "current dr %s, bdr %s", ospf_inet_ntoa(drstr, p_nbr->dr), ospf_inet_ntoa(bdrstr, p_nbr->bdr));
        }
        return TRUE;
    }
     /*if it is not new neighbor,and dr/bdr are different from self,do dr election*/
    if ((!new_nbr) 
        && (!p_if->p_process->in_restart) 
        && ((dr != p_if->dr) || (bdr != p_if->bdr)))
    {
        if (ospf_debug_hello || ospf_debug_gr)
        {
            ospf_logx(ospf_debug_hello, "DR/BDR in hello not same as self");
        }
        return TRUE;
    }
    /*no changed*/
    return FALSE;
}

/*process hello packet received
section 10.5 of OSPF specification (page 87) */
void 
ospf_hello_input (
                struct ospf_hdr *p_hdr,
                struct ospf_nbr *p_nbr,
                struct ospf_if *p_if,
                u_int source)
{
    struct ospf_hello_msg *p_hello = (struct ospf_hello_msg *)p_hdr;
    struct ospf_process *p_process = p_if->p_process;
    struct ospf_authinfo *p_auth;
    u_int new_dr  = FALSE;
    u_int new_bdr = FALSE;
    u_int new_nbr = FALSE;
    u_int ism_change = FALSE;    
    u_int8 nbr[16];
    u_int8 statemask[4] = {0};

    //ospf_logx(ospf_debug_hello, "recv hello from %s on %s", ospf_inet_ntoa(nbr, source), p_if->name);
    ospf_logx(ospf_debug_hello,"ospf_hello_input recv hello from %s on %s,p_if->state=%d.\r\n", ospf_inet_ntoa(nbr, source), p_if->name,p_if->state);
   // printf("ospf_hello_input recv hello from %s on %s,p_if->state=%d.\r\n", ospf_inet_ntoa(nbr, source), p_if->name,p_if->state);

    if (ospf_debug_hello && ospf_debug_msg)
    {
        ospf_log_hello_packet(p_hello);
        ospf_logx(ospf_debug_hello, "");
    }
    
    if (!ospf_hello_is_valid (p_hello, p_if))
    {
    	//printf("ospf_hello_input failed to validate hello packet\r\n");
    	ospf_logx(ospf_debug_hello,"ospf_hello_input failed to validate hello packet\r\n");
       // ospf_logx(ospf_debug_hello, "failed to validate hello packet");
        return;
    }

    /*new neighbor */
    if (NULL == p_nbr)
    {
		//	ospf_logx(ospf_debug_hello,"(NULL == p_nbr) creat \r\n");
        /*limit 2way peer count*/        
        BIT_LST_SET(statemask, OSPF_NS_INIT);
        BIT_LST_SET(statemask, OSPF_NS_2WAY);
        BIT_LST_SET(statemask, OSPF_NS_EXCHANGE);
        BIT_LST_SET(statemask, OSPF_NS_EXSTART);
        BIT_LST_SET(statemask, OSPF_NS_LOADING);
        if (OSPF_MAX_TWOWAY_NBR_COUNT <= ospf_nbr_count_in_state(NULL, statemask))
        {
//	ospf_logx(ospf_debug_hello,"%s %d  *****count: %d******\n", 
//		__FUNCTION__,__LINE__,ospf_nbr_count_in_state(NULL, statemask) );
            return;
        }

        /*hold_down定时器起来时不允许hello报文*/
        if(ospf_timer_active(&p_if->hold_down_timer) == TRUE)
        {
            return;
        }

        if(p_if->link_up == TRUE)
        {
            if ((p_if->ucLdpSyncEn == FALSE)
                || ((p_if->ucLdpSyncEn == TRUE)
                    && (ospf_timer_active(&p_if->hold_down_timer) == FALSE)))
            {
                if ((ospf_nbr_search(p_if) == 0) && (p_if->ucHoldCostState == TRUE))
                {
                    ospf_stimer_start(&p_if->hold_cost_timer, p_if->ulHoldCostInterval);
                    p_if->ulOspfSyncState = OSPF_LDP_HOLD_MAX_COST;
                    ospf_logx(ospf_debug_rtm,"ospf ldp sync change to OSPF_LDP_HOLD_MAX_COST %s %d\r\n",__FUNCTION__,__LINE__);
                }
                p_nbr = ospf_nbr_create_by_hello(p_if, p_hello, source);
            }
        }
        if (NULL == p_nbr)
        {
     		ospf_logx(ospf_debug_hello,"ospf_hello_input can not obtain neighbor\r\n");
           	//ospf_logx(ospf_debug_hello, "can not obtain neighbor"); 
            return;
        }      
        new_nbr = TRUE;
    }
    
    /*record field from hello*/
    p_auth = (struct ospf_authinfo *)p_hello->h.auth;
    p_nbr->auth_seqnum[OSPF_PACKET_HELLO] = ntohl(p_auth->seqnum);
    p_nbr->id = ntohl(p_hello->h.router);
    
    /*NBMA:If the router is not eligible to become Designated Router,send response to nbr eligible beacome DR*/
    if ((OSPF_IFT_NBMA == p_if->type) 
        && (0 == p_if->priority)
        && (0 < p_nbr->priority))
    {
        ospf_hello_output(p_if, FALSE, TRUE);
    }
    
    /*check dr/bdr and neighbor fields in hello*/
    ospf_nsm(p_nbr, OSPF_NE_HELLO);
    
//	ospf_logx(ospf_debug,"#%s:%d,111 p_if=0x%x,p_if->addr=0x%x,p_if->ifnet_uint=0x%x.\r\n",__FUNCTION__,__LINE__,p_if,p_if->addr,p_if->ifnet_uint);
//	ospf_logx(ospf_debug,"#%s:%d,111 p_nbr=0x%p,p_nbr->addr=0x%x,p_nbr->p_if=0x%x,p_nbr->p_if->addr=0x%x.\r\n",__FUNCTION__,__LINE__,p_nbr,p_nbr->addr,p_nbr->p_if,p_nbr->p_if->addr);
    /*check 2way relationship*/
    if (FALSE == ospf_hello_2way_check(p_process->router_id, p_hello))
    {
        //ospf_logx(ospf_debug_hello, "self is not appeared in hello"); 
      	ospf_logx(ospf_debug_hello,"ospf_hello_input self is not appeared in hello\r\n");
     // 	printf("ospf_hello_input self is not appeared in hello\r\n");
       
        /*exit graceful restart*/
        if (p_process->in_restart)
        {
        	ospf_logx(ospf_debug_hello,"ospf_hello_input receive 1way hello,exit graceful restart\r\n");
          	//ospf_logx(ospf_debug_gr, "receive 1way hello,exit graceful restart");   
            
            ospf_restart_finish(p_process, OSPF_RESTART_TOPLOGY_CHANGED);
        }
        /*2way check failed,clear adj*/
        ospf_nsm(p_nbr, OSPF_NE_1WAY);
        return;
    }
    
    /*2way received,check dr/bdr*/
    ospf_nsm(p_nbr, OSPF_NE_2WAY);

//	ospf_logx(ospf_debug,"#%s:%d,222 p_if=0x%x,p_if->addr=0x%x,p_if->ifnet_uint=0x%x.\r\n",__FUNCTION__,__LINE__,p_if,p_if->addr,p_if->ifnet_uint);
//	ospf_logx(ospf_debug,"#%s:%d,222 p_nbr=0x%p,p_nbr->addr=0x%x,p_nbr->p_if=0x%x,p_nbr->p_if->addr=0x%x.\r\n",__FUNCTION__,__LINE__,p_nbr,p_nbr->addr,p_nbr->p_if,p_nbr->p_if->addr);
    /*decide if dr&bdr&priority changed.*/
    ism_change = ospf_hello_is_nbr_changed(new_nbr, p_nbr, p_hello);
    if (TRUE == ism_change)
    {
        p_nbr->dr = ntohl(p_hello->dr);  
        p_nbr->bdr = ntohl(p_hello->bdr);  
        p_nbr->priority =  p_hello->priority;  
        /*scedule sync msg for nbr*/
        ospf_sync_event_set(p_nbr->syn_flag);
    }
    
    new_dr = ntohl(p_hello->dr);  
    new_bdr = ntohl(p_hello->bdr);
    
    /*if self is in restarting,record dr/bdr in hello for the first neighbor,
          and change interface state
     */
    /* if the receiving interface is in state Waiting and this neighbor is 
           declaring itself to be backup designated router or is declaring itself 
           to be designated router and there is no backup designated router, then the receiving
           interface's state machine is scheduled with the event BackupSeen
           section 10.5 bullet4 and 5*/
    if (OSPF_IFS_WAITING == p_if->state)
    {
        if (p_process->in_restart && new_nbr)
        {
            p_if->dr = new_dr;
            p_if->bdr = new_bdr;
            ospf_ism (p_if, OSPF_IFE_BACKUP_SEEN);
        }
        else if (((p_nbr->addr == new_bdr)
               || ((p_nbr->addr == new_dr) && (!new_bdr))))  
        {
            ospf_ism (p_if, OSPF_IFE_BACKUP_SEEN);
        }      
    }    
    
    /*if nbrchange event*/
    if (TRUE == ism_change)
    {         
        ospf_ism (p_if, OSPF_IFE_NBR_CHANGE);

        /*non-restarting neighbor changed,leave any hepler mode*/
        if (!p_nbr->in_restart)
        {
            ospf_restart_helper_finish_all(p_process, OSPF_RESTART_TOPLOGY_CHANGED);
        }
        /*exit graceful restart*/
        if (p_process->in_restart)
        {       
   		//	printf("%s:%d\r\n",__FUNCTION__,__LINE__);	 
           ospf_logx(ospf_debug_hello, "hello changed on if %s", ospf_inet_ntoa(nbr, p_if->addr));

            ospf_restart_finish(p_process, OSPF_RESTART_TOPLOGY_CHANGED);
        }
    }
 //	printf("#%s:%d\r\n",__FUNCTION__,__LINE__);
    /*add for bug: nbr state should be two_way,but become FULL. Use this process to avoid */
    if (OSPF_IFS_DROTHER == p_if->state)
    {
//		ospf_logx(ospf_debug_hello,"ospf_hello_input (OSPF_IFS_DROTHER == p_if->state) OSPF_NE_ADJOK\r\n");
        ospf_nsm(p_nbr, OSPF_NE_ADJOK);
    } 
    return;
}

/*This routine will validate the hello packet.*/
u_int 
ospf_hello_is_valid (
                    struct ospf_hello_msg *p_hello,
                    struct ospf_if *p_if)
{
    struct ospf_process *p_process = p_if->p_process;
    u_int mask = 0;
    u_short hello_interval = 0;
    u_int dead_interval = 0,dead_local = 0;
    u_int errtype = 0;
    u_int ebit = 0;
    u_int nbit = 0;    
    
    /*mask match for normal interface*/
    mask = ntohl(p_hello->mask);    
    if ((OSPF_IFT_VLINK != p_if->type) 
          && (OSPF_IFT_PPP != p_if->type) 
          && (OSPF_IFT_SHAMLINK != p_if->type) 
          && (mask != p_if->mask))
    {
    #if 1
        ospf_logx(ospf_debug_hello, "mask mismatch,rcvd %08x", mask); 
        
        p_if->stat->error[OSPF_IFERR_MASK]++;
        p_if->stat->error_data[OSPF_IFERR_MASK] = mask;
        errtype =  ERROR_NETMASK;
        goto SEND_TRAP; 
	#endif
    }

    /*hello interval match*/
    hello_interval = ntohs(p_hello->hello_interval);
    if (hello_interval != p_if->hello_interval)
    {
        ospf_logx(ospf_debug_hello, "hello interrval mismatch,rcvd %d", hello_interval); 
                
        p_if->stat->error[OSPF_IFERR_HELLOTIME]++;
        p_if->stat->error_data[OSPF_IFERR_HELLOTIME] = hello_interval;
        errtype =  ERROR_HELLO_TIME;
        goto SEND_TRAP;  
    }

    /*dead interval match*/

	if(p_if->dead_interval ==  OSPF_DEFAULT_ROUTER_DEAD_INTERVAL)
	{
		dead_local = p_if->hello_interval*4;
	}
	else
	{
		dead_local = p_if->dead_interval;
	}
    dead_interval = ntohl(p_hello->dead_interval);
    if (dead_interval != dead_local)
    {
        ospf_logx(ospf_debug_hello, "dead interrval mismatch,rcvd %d", dead_interval); 
        
        p_if->stat->error[OSPF_IFERR_DEADTIME]++;
        p_if->stat->error_data[OSPF_IFERR_DEADTIME] = dead_interval;
        errtype =  ERROR_DEAD_TIME;
        goto SEND_TRAP; 
    }

   /*E/N bit:
        normal:E=1,N=0
        sutb:E=0,N=0
        nssa:E=0,N=1
     */
    ebit = ospf_option_external(p_hello->option);
    nbit = ospf_option_nssa(p_hello->option);
    
    if (p_if->p_area->is_nssa)
    {
        if (ebit || (!nbit))
        {
            ospf_logx(ospf_debug_hello, "option bit mismatch for nssa "); 
        
            p_if->stat->error[OSPF_IFERR_EBIT]++;
            errtype =  ERROR_OPTION;
            goto SEND_TRAP;
        }
    }
    else if (p_if->p_area->is_stub)
    {
        if (ebit || nbit)
        {
            ospf_logx(ospf_debug_hello, "option bit mismatch for stub"); 
        
            p_if->stat->error[OSPF_IFERR_EBIT]++;
            errtype =  ERROR_OPTION;
            goto SEND_TRAP; 
        }
    }
    else
    {
        if ((!ebit) || nbit)
        {
            ospf_logx(ospf_debug_hello, "option bit mismatch "); 
        
            p_if->stat->error[OSPF_IFERR_EBIT]++;
            errtype =  ERROR_OPTION;
            goto SEND_TRAP;
        }
    }
    return TRUE;

  SEND_TRAP :
    p_process->trap_error = errtype;
    p_process->trap_packet = OSPF_PACKET_HELLO;

    if (OSPF_IFT_VLINK != p_if->type)
    {
        ospf_trap_iferror(errtype, OSPF_PACKET_HELLO, p_process->trap_source, p_if);
    }
    else 
    {
        ospf_trap_viferror(errtype, OSPF_PACKET_HELLO, p_if);
    }
    return FALSE;
}

/*This routine will examine all the neighbors in the hello packet
* and check if the neighbor has been heard from before.*/
u_int 
ospf_hello_2way_check (
             u_int router_id,
             struct ospf_hello_msg *p_hello)
{
    u_int *p_id = p_hello->nbr;
    u_int count = (ntohs(p_hello->h.len) - OSPF_PACKET_HLEN - OSPF_HELLO_HLEN)/OSPF_HELLO_NBR_LEN;
    u_int i = 0;
    
    for (i =  0; i < count ; i++, p_id++)
    {
        if (router_id == ntohl(*p_id))
        {
            return TRUE;
        }
    }
    return FALSE;
}

