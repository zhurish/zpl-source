

#include "ospf.h"
#include "ospf_api.h"
#include "ospf_nm.h"
#include "ospf_dcn.h"
//#include "lldpd_api.h"
//#include "eth_trunk_api.h"

#ifdef  OSPF_DCN

OSPF_DCN_CARD_CTL_T stDcnCardCtl[]=
{
	{0x0100,		0,	9},
	{0x0107,		0,	7},
	{0x0101,		0,	5},
	{0x0106,		0,	5},

}; 

extern OSPF_DCN_T stDcnCfg;
OSPF_DCN_T stOspfDcn;
extern tOSPF_LSA_MANAGE stOspfLsaManage;
extern u_int uiCfgLoadFlag;

int ospf_dcn_one_interface_enable(u_int uiIfIndex, u_char ucPortEn);

int ospf_dcn_global_init()
{
    int i = 0, j = 0;
    
    memset(&stOspfDcn, 0, sizeof(stOspfDcn));
    stOspfDcn.uiNeid = OSPF_DCNDEF_NEID;
    stOspfDcn.ulDeviceIp = OSPF_DCNDEF_IPADDR + OSPF_DCNDEF_NEID;
    stOspfDcn.ulDeviceMask = OSPF_DCNDEF_IPMASK;
    stOspfDcn.ucNeidReleIp = OSPF_DCN_NEID_RELEVANCE_NEIP;
    stOspfDcn.uiVlanId = DCN_VLAN_DEF;

    for(i = 0; i < MAX_SLOT_NUM; i++)
    {
        for(j = 0; j < SLOT_MAX_PORT_NUM; j++)
        {
            stOspfDcn.uiUsePort[i][j] = OSPF_DCN_PORT_ENABLE;
        }
    }

    for(i = 0; i < LAG_GROUP_NUM; i++)
    {
        stOspfDcn.uiLagPort[i] = OSPF_DCN_PORT_ENABLE;
    }

    //stOspfDcn.uiUsePort[0][0] = OSPF_DCN_PORT_ENABLE;
    
    memcpy(stOspfDcn.ucDevName, "jiubo", 5);
    memcpy(stOspfDcn.ucDevType, "ipran_u3", 8);
}

int dcn_vlan_used_num(u_int uiVlan)
{
    int iRet = VOS_ERR;
    u_int uiIfIndex = 0;
    u_int uiBindVlan = 0;
    u_int uiCount = 0;
    
    iRet = zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_FIRST_IFINDEX, &uiIfIndex);
    while(iRet == VOS_OK)
    {
        uiBindVlan = 0;
        zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_IF_BIND_VLAN, &uiBindVlan);
        if(uiBindVlan == uiVlan)
        {
            uiCount++;
        }
        iRet = zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_NEXT_IFINDEX, &uiIfIndex);
    }

    return uiCount;
}

int dcn_interface_vlan_check(u_int uiVlan, u_int uiCuIfIndex)
{
    int iRet = VOS_ERR;
    u_int uiIfIndex = 0, uiMainIfIndex = 0;
    u_int uiBindVlan = 0;
    u_int uiSlot = 0, uiPort = 0;
    
    iRet = zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_FIRST_IFINDEX, &uiIfIndex);
    while(iRet == VOS_OK)
    {
        uiBindVlan = 0;
        uiMainIfIndex = 0;
        zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_IF_BIND_VLAN, &uiBindVlan);
        if(uiVlan == uiBindVlan)
        {
            if(IFINDEX_TYPE_ETH_VTRUNK == IFINDEX_TO_TYPE(uiIfIndex))
            {
                if_vlag_ifindex_to_lag_ifindex(uiIfIndex, &uiMainIfIndex);
                if(uiCuIfIndex == uiMainIfIndex)
                {
                    return VOS_ERR;
                }
            }
            else if(IFINDEX_TYPE_VPIF == IFINDEX_TO_TYPE(uiIfIndex))
            {
                if_vport_ifindex_to_ifindex(uiIfIndex, &uiMainIfIndex);
                if(uiCuIfIndex == uiMainIfIndex)
                {
                    return VOS_ERR;
                }
            }
        }

        iRet = zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_NEXT_IFINDEX, &uiIfIndex);
    }

    return VOS_OK;
}

int dcn_interface_exist_check(u_int uiMainIfIndex)
{
    octetstring stOctetInfo = {0};
    u_char ucIfName[32] = {0};
    u_int uiSubIfIndex = 0;
    u_int uiLogicPort = 0;
    u_int uiAdminSatet = 0;
    u_int uiType = IFINDEX_TO_TYPE(uiMainIfIndex);

    if(uiType != IFINDEX_TYPE_ETH_TRUNK)
    {
        if_index_to_logic_port(uiMainIfIndex, &uiLogicPort);
    
    }
    else
    {
        uiLogicPort = VPORT_IF_IFINDEX_TO_PORT(uiMainIfIndex);
    }
    
    if((uiType != IFINDEX_TYPE_ETH_TRUNK) && VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLogicPort, stOspfDcn.uiVlanId, &uiSubIfIndex))
    {
        return VOS_ERR;
    }

    if((uiType == IFINDEX_TYPE_ETH_TRUNK) && if_vlag_logic_port_subid_to_index(uiLogicPort, stOspfDcn.uiVlanId, &uiSubIfIndex))
    {
        return VOS_ERR;
    }

    stOctetInfo.pucBuf = ucIfName;
    stOctetInfo.len = sizeof(ucIfName);
    if(VOS_OK == zebra_if_get_api(uiSubIfIndex, ZEBRA_IF_ADMIN_STATE, &uiAdminSatet))
    {
        memset(ucIfName, 0, sizeof(ucIfName));
        stOctetInfo.len = sizeof(ucIfName);
        zebra_if_get_api(uiMainIfIndex, ZEBRA_IF_NAME, &stOctetInfo);
        vty_out_to_all_terminal("Error:dcn sub interface %s.%d is exist, please delete the sub interface and then enable dcn on the intereface manually.", ucIfName,stOspfDcn.uiVlanId); 
        return VOS_ERR;        
    }

    return VOS_OK;
}

int dcn_interface_delete_if_dcn_port(u_int uiIfIndex)
{
    u_int uiMainIfIndex = 0;
    u_int uiLogicPort = 0;
    u_int uiDcnSubIndex = 0;
	u_int uiType = IFINDEX_TO_TYPE(uiIfIndex);
    int iRet = VOS_ERR;
    
    if(uiType != IFINDEX_TYPE_ETH_TRUNK && uiType != IFINDEX_TYPE_ETH_VTRUNK)
    {
        uiMainIfIndex = VPORT_IF_IFINDEX_TO_MAIN_IFINDEX(uiIfIndex);
    }
    else
    {
        if_vlag_ifindex_to_lag_ifindex(uiIfIndex, &uiMainIfIndex);
    }

    if(ospf_dcn_get_port_enable_status(uiMainIfIndex) == OSPF_DCN_PORT_ENABLE)
    {
        if(uiType != IFINDEX_TYPE_ETH_TRUNK && uiType != IFINDEX_TYPE_ETH_VTRUNK)
        {
            if_index_to_logic_port(uiMainIfIndex, &uiLogicPort);
            if_vport_if_logic_port_subid_to_index(uiLogicPort, stOspfDcn.uiVlanId, &uiDcnSubIndex);
        }
        else
        {
            uiLogicPort = VPORT_IF_IFINDEX_TO_PORT(uiMainIfIndex);
            if_vlag_logic_port_subid_to_index(uiLogicPort, stOspfDcn.uiVlanId, &uiDcnSubIndex);
        }

        if(uiIfIndex == uiDcnSubIndex)
        {
            ospf_logx(ospf_debug_dcn, "interface index %x is dcn interface\n",uiIfIndex);
            iRet = ospf_dcn_one_interface_enable(uiMainIfIndex, OSPF_DCN_PORT_DISABLE);
        }
    }

    return iRet;
}

u_int dcn_config_load_done()
{
    return (uiCfgLoadFlag == 0) ? VOS_FALSE : VOS_TRUE;
}

int ospf_dcn_get_bandwidth(u_int *puiBand)
{
	int iRtv = 0;

    *puiBand = stOspfDcn.uiBandwidth;
	iRtv =ospf_dcn_get_globale_enable();
	if(iRtv != OSPF_DCN_ENABLE)
	{
        OSPF_DCN_LOG_WARNING("DCN function has not been used!");
		return ERR;
	}	

   	return OK;
}

int ospf_dcn_set_bandwidth(u_int uiBand)
{
	int iRtv = 0;

	iRtv =ospf_dcn_get_globale_enable();
	if(iRtv != OSPF_DCN_ENABLE)
	{
        OSPF_DCN_LOG_WARNING("DCN function has not been used!");
		return ERR;
	}	
    stOspfDcn.uiBandwidth = uiBand;    
    return OK;
}

u_int ospf_dcn_modify_ip(u_int ulIp,u_int ulMask)
{
	u_int ulValue=0;
	tOSPF_NETWORK_INDEX	stOspfNetWorkIndex;
	int iRtv = 0;
	octetstring stOct;
	u_char aucMac[MAC_ADDR_LEN] = {0};
	u_long ulLoIp = 0xc0a80301; /*默认ip地址为192.mac[5].3.1*/
	u_long	ulLbIp = 0;
	u_int ulNetWorkMask = 0;
	u_char ucIpAddr[4];
	u_long	ulMaskIp = 0;
	long	ldcnIp = ulIp;
	u_int ulns = OSPF_DCN_PROCESS;
	u_int uiRouterFlg = 0;

	/*先删除dcn网络*/
	stOspfNetWorkIndex.area = 0;
	stOspfNetWorkIndex.mask = 0;
	stOspfNetWorkIndex.network = 0;
	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
	ulValue = FALSE;

	if(ospfNetworkSetApi(&stOspfNetWorkIndex,OSPF_NETWORK_STATUS,&ulValue)!=OK)
	{
	     ospf_logx(ospf_debug_dcn,"Failed to configure network:%d \r\n",__LINE__);
	     OSPF_DCN_LOG_WARNING("Failed to configure network,ulLbIp:%x.",ulLbIp);
	}

    ospf_dcn_loopback_ip_set(ulIp, ulMask);

    ospf_dcn_interface_ip_set();

    vos_pthread_delay(100);

    ospfSetApi(OSPF_DCN_PROCESS, OSPF_GBL_RESET, &ulValue);

    vos_pthread_delay(300);

    ulValue = TRUE;
	if(ospfNetworkSetApi(&stOspfNetWorkIndex,OSPF_NETWORK_STATUS,&ulValue)!=OK)
	{
	     ospf_logx(ospf_debug_dcn,"Failed to configure network:%d \r\n",__LINE__);
	     OSPF_DCN_LOG_WARNING("Failed to configure network,ulLbIp:%x.",ulLbIp);
	}

    #if 0
	stOspfNetWorkIndex.network = ulIp;

	ospf_dcn_change_all_network(&stOspfNetWorkIndex);

	lag_dcn_destroy_and_create();	
	ospf_dcn_if_set(OSPF_DCN_PROCESS,ulIp);
    #endif
    ospf_dcn_lsa_update();
    
	return OK;	
}

int ospf_dcn_get_flag()
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;

  	for_each_ospf_process(p_process, p_next_process)  
	{
		if(p_process->process_id == OSPF_DCN_PROCESS)
		{
			for_each_ospf_if(p_process, p_if, p_next_if)
			{
				if (OSPF_DCN_FLAG == p_if->ulDcnflag)
				{
					ospf_logx(ospf_debug_dcn,"ospf dcn get flag ulDcnflag=%d.\r\n",p_if->ulDcnflag);
					OSPF_DCN_LOG_WARNING("ospf dcn get flag ulDcnflag=%d.",p_if->ulDcnflag);
					return OK;
				}
			}
		}
	}

   	return ERR;
}

u_int ospf_dcn_change_nbr(tOSPF_NETWORK_INDEX *p_index,u_char ucPort)
{
 	u_int if_unit = 0;
	u_int addr = 0,unit = 0;
	u_int mask = 0;
    u_long ulIp = 0;
    int iRet = VOS_OK;
    
	struct ospf_if *p_if = NULL;
	struct ospf_process *p_process = NULL;

	p_process = ospf_get_nm_process(p_index);

	if (NULL == p_process)
	{
		return iRet;
	}

    addr = p_index->network;
    mask = p_index->mask;

    /*ignore invalid interfaces*/     

    unit = ospf_dcn_vpif_lport_subid_to_index(ucPort,OSPF_DCN_SUBPORT);//dcn 子接口索引

    p_if = ospf_if_lookup_forDcnCreat(p_process, addr,unit);
   // printf("#%s:%d,ucPort = %d,addr=%x,p_if=%x,unit=%x.\r\n",__FUNCTION__,__LINE__,ucPort,addr,p_if,unit);

    if(NULL != p_if)
    {
        iRet = ospf_get_rem_man_addr(p_if->ifnet_uint, &ulIp);
      //  printf("#%s:%d,iRet = %d,ulIp=%x,unit=%x.\r\n",__FUNCTION__,__LINE__,iRet,ulIp,p_if->ifnet_uint);
        if((iRet == OK)&&(ulIp != 0))
		{
            ospf_dcn_creat_nbr(p_if,ulIp);
        }
    }

    return iRet;
}

/*修改所有接口IP*/
void ospf_dcn_change_all_network(tOSPF_NETWORK_INDEX *p_index)
{
	int i = 0,iRtv = 0;
	OSPF_DCN_PORT_RANGE_T *pstPort = NULL;
	u_char ucStart = 0,ucEnd = 0;
	u_int ulIfIndx = 0;
    
	iRtv = ospf_dcn_create_process();
    if(iRtv == ERR)
	{
		return ERR;
	}
    
    for(i = 0; i < MAX_PORT_NUM; i++)
    {        
        if(VOS_ERR == if_logic_port_to_index(i,&ulIfIndx))
        {
            continue;
        }
        /*接口被overlay使用*/
        if(OK == ospf_dcn_lookup_overlay_port(i))
		{
			continue;
		}
		iRtv = ospf_dcn_get_port_enable_status(ulIfIndx);//接口
		if((iRtv !=  OSPF_DCN_PORT_DISABLE)&&(iRtv !=  ERR))
		{
			ospf_dcn_create_network(p_index,i);//修改ospf网络IP   
			ospf_dcn_change_nbr(p_index,i);
		}
    }
    lldp_dcn_rem_nbr_del();/*清dcn nbr*/


}

int ospf_dcn_lag_process_creat()
{
    int iRtv = 0;
	u_long ulValue = 0;
	u_long ulns = 0;
	u_long ulVal = 0;
	u_long ulLbIp = 0;

	ospfGetApi(OSPF_DCN_PROCESS,OSPF_GBL_ADMIN,&ulValue);
	
	if(ulValue != ENABLE)
	{
		ulValue = ENABLE; 
		
		if(ospfSetApi(OSPF_DCN_PROCESS,OSPF_GBL_ADMIN ,&ulValue) != OK)
		{
			ospf_logx(ospf_debug_dcn," Failed to ENABLE ospf :%d \r\n",__LINE__);
			OSPF_DCN_LOG_WARNING(" Failed to ENABLE ospf.");
		}
	}
	
	ulns = OSPF_DCN_PROCESS;

#ifdef OSPF_VPN
	ulVal = OSPF_DCN_VRID;
#else
	iRtv = zebra_if_ospf_get_api(ulns,ZEBRA_IF_CREATE_VPN_OSPF,&ulVal);
	iRtv = zebra_if_ospf_get_api(ulns,ZEBRA_IF_GET_VRF_BY_OSPF_ID,&ulVal);
#endif

	if(ospfSetApi(ulns,OSPF_GBL_VRID,&ulVal)!=OK)
	{
		ospf_logx(ospf_debug_dcn,"Failed to set vrid:%d \r\n",__LINE__);
		OSPF_DCN_LOG_WARNING("Failed to set vrid.");
	}
    
	ulLbIp = stOspfDcn.ulDeviceIp;

	if(ospfSetApi(OSPF_DCN_PROCESS,OSPF_GBL_ROUTERID,&ulLbIp) != OK)
	{
		ospf_logx(ospf_debug_dcn,"Failed to disable ospf:%d \r\n",__LINE__);
		OSPF_DCN_LOG_WARNING("Failed to disable ospf.");
	}
	iRtv = ospf_dcn_create_process();

    return iRtv;
}


/*根据逻辑端口号创建网络*/

void ospf_dcn_create_lag_port(u_long ulLbIp,u_int unit)
{
	struct ospf_area *p_area = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;    
	struct ospf_network *p_network = NULL;    
	STATUS rc = OK;
	STATUS ifrc = OK;
	u_int if_unit = 0;
	u_int addr = 0;//,unit = 0;
	u_int mask = 0;
	tOSPF_NETWORK_INDEX *p_index,stOspfNetWorkIndex;

    if(ERR == ospf_dcn_lag_process_creat())
    {
        return ;
    }

	stOspfNetWorkIndex.area = 0;
	stOspfNetWorkIndex.mask = OSPF_DCNDEF_IPMASK;
	stOspfNetWorkIndex.network = ulLbIp;
	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
	p_index = &stOspfNetWorkIndex;

	p_process = ospf_get_nm_process(p_index);

	if (NULL == p_process)
	{
		zlog_info(MTYPE_OSPF,"ospf dcn create network failed ,process:%x\n", p_process);
		OSPF_DCN_LOG_WARNING("ospf dcn create network failed ,Lb ip:%x", ulLbIp);
		return;
	}

    p_area = ospf_area_lookup(p_process,  p_index->area);
    if (NULL == p_area)
    {
		zlog_info(MTYPE_OSPF,"ospf dcn create network failed ,p_area:%x\n", p_area);
		OSPF_DCN_LOG_WARNING("ospf dcn create network failed.");
	    return;
    }
    addr = p_index->network;
    mask = p_index->mask;

    /*ignore invalid interfaces*/     
	
	//unit = ospf_dcn_vpif_lport_subid_to_index(ucPort,OSPF_DCN_SUBPORT);//dcn 子接口索引

    p_if = ospf_if_lookup_forDcnCreat(p_process, addr,unit);
	if (p_if != NULL)
	{
	    ospf_logx(ospf_debug,"%d, p_if->addr = 0x%x, p_if->ifnet_uint = 0x%x\n",__LINE__, p_if->addr, p_if->ifnet_uint);
	    OSPF_DCN_LOG_WARNING("The p_if has been create p_if->addr = %x, p_if->ifnet_uint = %x",p_if->addr, p_if->ifnet_uint);
	}
    ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS  p_if=%p.\r\n",p_if);
  /*if interface exist,and interface has area configured,do nothing*/
     if (NULL != p_if) 
     {                       
         if (NULL == p_if->p_area)
         {
            ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS NULL == p_if->p_area.\r\n");
            p_if->p_area = p_area;        
          //  p_if->state = OSPF_IFS_DOWN;
            ///ospf_lstadd_unsort(&p_area->if_table, p_if);
            ospf_lstadd(&p_area->if_table, p_if);
         }
     }
     else
     {    
         /*create interface with area*/                          
         ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS real_if_cr.\r\n");
         ospf_Dcn_real_if_create(p_process,unit,addr,mask, p_area);
     }
}



/*根据逻辑端口号创建网络*/

int ospf_dcn_create_network(tOSPF_NETWORK_INDEX *p_index,u_char ucPort)
{
	struct ospf_area *p_area = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;    
	struct ospf_network *p_network = NULL;    
	STATUS rc = OK;
	STATUS ifrc = OK;
	u_int if_unit = 0;
	u_int addr = 0,unit = 0;
	u_int mask = 0;

	p_process = ospf_get_nm_process(p_index);

	if (NULL == p_process)
	{
		zlog_info(MTYPE_OSPF,"ospf_dcn_create_network failed ,p_process:%x\n", p_process);
		OSPF_DCN_LOG_WARNING("ospf_dcn_create_network failed,p_process is null");
		return ERR;
	}

    p_area = ospf_area_lookup(p_process,  p_index->area);
    if (NULL == p_area)
    {
		zlog_info(MTYPE_OSPF,"ospf_dcn_create_network failed ,p_area:%x\n", p_area);
		OSPF_DCN_LOG_WARNING("ospf_dcn_create_network failed,p_area is null");
	    return ERR;
    }
    addr = p_index->network;
    mask = p_index->mask;

    /*ignore invalid interfaces*/     
	
	unit = ospf_dcn_vpif_lport_subid_to_index(ucPort,OSPF_DCN_SUBPORT);//dcn 子接口索引

     p_if = ospf_if_lookup_forDcnCreat(p_process, addr,unit);
	if (p_if != NULL)
	{
	    ospf_logx(ospf_debug,"%d, p_if->addr = 0x%x, p_if->ifnet_uint = 0x%x\n",__LINE__, p_if->addr, p_if->ifnet_uint);
	    OSPF_DCN_LOG_WARNING("The p_if has been create p_if->addr = %x, p_if->ifnet_uint = %x",p_if->addr, p_if->ifnet_uint);
	}
    ospf_logx(ospf_debug, "  ospf interface =%p.\r\n",p_if);
  /*if interface exist,and interface has area configured,do nothing*/
     if (NULL != p_if) 
     {                       
         if (NULL == p_if->p_area)
         {
            ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS NULL == p_if->p_area.\r\n");
            p_if->p_area = p_area;        
          //  p_if->state = OSPF_IFS_DOWN;
            //ospf_lstadd_unsort(&p_area->if_table, p_if);
            ospf_lstadd(&p_area->if_table, p_if);
         }
     }
     else
     {    
         /*create interface with area*/                          
         ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS real_if_cr.\r\n");
         ospf_Dcn_real_if_create(p_process,unit,addr,mask, p_area);
     }

     return OK;
}


/*更新子接口Peer IP*/
int ospf_dcn_peer_update(u_int uiIfIndex,u_long ulPeerIp)
{
	int ret = 0;
    	char szIfName[INTERFACE_NAMSIZ] = {0};
	char szCmd[128] = {0};
	char szIp[32] = {0};
		
	ip_int_to_ip_str(ulPeerIp, szIp);
	zebra_if_get_api(uiIfIndex,ZEBRA_IF_GET_IF_NAME_BY_IF, szIfName);

	if((szIfName[0] != '\0')&&(szIfName[0] != 0)&&(0 != ulPeerIp))
	{
		sprintf(szCmd, "ifconfig %s pointopoint %s", szIfName, szIp);
		system(szCmd);
	}

}


/*更新LLDP及DCN子接口IP*/
int ospf_dcn_addr_update(u_int uiPo)
{
	u_int uiIfIndex = 0;
	u_long ulLbIp = 0;
	int ret = 0;
	u_long ulPeerIp = 0;

	ulLbIp= stOspfDcn.ulDeviceIp;
	uiIfIndex = ospf_dcn_vpif_lport_subid_to_index(uiPo,OSPF_DCN_SUBPORT);
    ospf_logx(ospf_debug_dcn,"ospf_dcn_addr_update uiSub:0x%x,ulLbIp:0x%x\n",uiPo,ulLbIp);
//	ospf_set_loc_man_addr(uiSub,ulLbIp);  //更新lldp地址
	
	zebra_if_ospf_get_api(uiIfIndex, ZEBRA_IF_GET_DCN_PEER_IP, &ulPeerIp);//获取peer IP
	ospf_dcn_peer_update(uiIfIndex,ulPeerIp); //更新Peer ip
}


/*默认创建loopback 31及所有子接口*/
int ospf_dcn_Intf_hw_init(void)
{
	u_int uiVlan = 0,uiLport = 0;
	int ret = 0;
	struct interface *ifp  = NULL;
	
	ospf_dcn_lpbk_interface_creat(OSPF_DCNLOOPBACK_IFINDEX);

	ospf_dcn_vlan_hw_creat(DCN_VLAN_DEF);

	/*创建所有子接口*/
	#if 0
    for(uiLport=0; uiLport<MAX_PORT_NUM; uiLport++)
    {
		if(INVALID == uspIfLogicPortToIndex(uiLport))
		{
			continue;
		}
        if(ERR ==  ospf_dcn_port_vpif_create(uiLport))
        {
            zlog_info(MTYPE_OSPF,"ospf_dcn_port_vpif_create uiLport: %d err!\n",uiLport);
        }
    }
    #endif
}




int ospf_dcn_slot_init(u_int uiSlot)
{
    u_char ucSlot = 0;
    int i,iRtv = 0;
    u_int uiLport;
    u_int uiPport = 0;
    u_long ulLagId = 0;
    u_long uiIfIndex = 0;
    u_int uiDevType = 0;
    u_int uiLagFlag = 0;
    u_int uiBand = 0,uiVlan = 0;
    u_int uiMode = 0;
    //LAG_GROUP_T *pstLagGroup = NULL;
    
	#if 0
	uspGetApi(uiSlot, SYS_API_BASE_TYPE, &uiDevType);
	if((CARD_TYPE_MIN >= uiDevType)|| (CARD_TYPE_MAX <= uiDevType))
	{	
		return;
	}
	#endif
	
	for(i = 1; i <= SLOT_MAX_PORT_NUM; i++)
	{
	    if(stOspfDcn.uiUsePort[uiSlot-1][i-1] != OSPF_DCN_PORT_ENABLE)
	    {
	        continue;
	    }
	    
		uiLport = LOGIC_PORT_GENERATE(uiSlot, i);

        if(VOS_ERR == if_logic_port_to_index(uiLport,&uiIfIndex))
        {
            continue;
        }

        if(NULL != eth_trunk_get_by_logic_port(uiLport))
        {
            #if 0
        	ucSlot = IFINDEX_to_SLOT(uiIfIndex);
		    ospf_dcn_set_port_enable(ucSlot,uiLport,OSPF_DCN_PORT_ENABLE);
		    #endif
		    stOspfDcn.uiUsePort[uiSlot-1][i-1] = OSPF_DCN_PORT_DISABLE;
		    ospf_logx(ospf_debug_dcn,"The port is in trunk group, disable dcn\r\n");
		    continue;
    	}

        zebra_if_get_api(uiIfIndex, ZEBRA_IF_L2_L3_MODE, &uiMode);
        if(uiMode == IF_L2_MODE)
        {
            ospf_logx(ospf_debug_dcn,"The port is l2mode, disable dcn\r\n");
            stOspfDcn.uiUsePort[uiSlot-1][i-1] = OSPF_DCN_PORT_DISABLE;
            continue;
        }

        if(VOS_TRUE == dcn_config_load_done() && VOS_ERR == dcn_interface_vlan_check(stOspfDcn.uiVlanId, uiIfIndex))
        {
            ospf_logx(ospf_debug_dcn,"dcn vlan has been used in the subinterface, disable dcn\r\n");
            stOspfDcn.uiUsePort[uiSlot-1][i-1] = OSPF_DCN_PORT_DISABLE;
            continue;
        }

        if(VOS_TRUE == dcn_config_load_done() && VOS_ERR == dcn_interface_exist_check(uiIfIndex))
        {
            stOspfDcn.uiUsePort[uiSlot-1][i-1] = OSPF_DCN_PORT_DISABLE;
            continue;
        }
        
        if(VOS_ERR == ospf_dcn_port_vpif_create(uiLport))
        {
            zlog_info(MTYPE_OSPF,"ospf_dcn_port_vpif_create uiLport: %d err!\n",uiLport);
            ospf_logx(ospf_debug_dcn,"Failed to create dcn v port\r\n",__LINE__);
            continue;
        }
        #if 0
        if(uiSlot >= 0)
        {
            iRtv = ospf_dcn_get_globale_enable();
            if(iRtv == OSPF_DCN_ENABLE)
            {
                /*DCN开启*/
                ospf_dcn_set_dcn_mode_by_ifindex(uiIfIndex,L3_IF_DCN_MODE_P2P);
                ospf_dcn_create_by_ifindex(uiIfIndex);
                ospf_dcn_addr_update(i);	
                /*获取dcn 带宽,并设置*/
                ospf_dcn_get_bandwidth(&uiBand);
                ospf_dcn_vlan_get(&uiVlan);
                ospf_dcn_set_eth_bandwidth(i, uiBand,uiVlan,DCN_BAND_CREAT);
            }
        }
        #endif
    }
    #if 0
    if((uiVlan != 0)&&(uiVlan < 4095))
    {
    	ospf_dcn_add_vlan_port(uiSlot,uiVlan,DCN_ADD_VLAN);
    }
    #endif
}

int ospf_dcn_slot_remove(u_int uiSlot)
{
    int i;
    u_int uiDevType = 0;
    u_int uiLport = 0,uiBand = 0,uiVlan = 0;
    u_long uiIfIndex = 0,ulLbIp = 0;

	for(i = 1; i <= SLOT_MAX_PORT_NUM; i++)
	{
	    if(stOspfDcn.uiUsePort[uiSlot-1][i-1] != OSPF_DCN_PORT_ENABLE)
	    {
	        continue;
	    }
	    
		uiLport = LOGIC_PORT_GENERATE(uiSlot, i);
        if(VOS_OK != if_logic_port_to_index(uiLport, &uiIfIndex))
        {
            continue;
        }
        #if 0
        /*删除dcn接口网络*/
        ospf_dcn_set_dcn_mode_by_ifindex(uiIfIndex,L3_IF_DCN_MODE_NONE);
        ospf_dcn_remove_port_by_ifindex(uiIfIndex);

        /*取消dcn 带宽限制*/
        ospf_dcn_get_bandwidth(&uiBand);
        ospf_dcn_vlan_get(&uiVlan);
        ospf_dcn_set_eth_bandwidth(i, uiBand,uiVlan,DCN_BAND_DEL);
        #endif
        if(ERR ==  ospf_dcn_port_vpif_destroy(uiLport))
        {
            ospf_logx(ospf_debug_dcn,"ospf dcn port vpif create uiLport: %d err!\n",uiLport);
        }
        
    }

    return OK;    
}

int ospf_dcn_trunk_remove()
{
    u_int uiLagId = 0;
    ETH_TRUNK_GROUP_T stEthTrunkGroup;
    int iRet = 0; 

    /*聚合口*/
    for(uiLagId=1;uiLagId <= LAG_GROUP_NUM;uiLagId++)
    {
        memset(&stEthTrunkGroup, 0, sizeof(stEthTrunkGroup));
        iRet = eth_trunk_get_api(uiLagId, ETH_TRUNK_ROW_STATUS, &stEthTrunkGroup);
        if(iRet == VOS_OK)
        {
            if(stOspfDcn.uiLagPort[uiLagId-1] != OSPF_DCN_PORT_DISABLE)
            {
                ospf_dcn_port_vpif_destroy(uiLagId + MAX_PORT_NUM);
            }
        }
    }

    return VOS_OK;
}

int ospf_dcn_slot_init_all()
{
	u_int uiSlot = 0;

	for(uiSlot = 1;uiSlot <= MAX_SLOT_NUM; uiSlot++)
	{
		ospf_dcn_slot_init(uiSlot);
	}

	return VOS_OK;
}

int ospf_trunk_init_all()
{
    u_int uiLagId = 0;
    u_int uiIfIndex = 0, uiMode = 0;
    ETH_TRUNK_GROUP_T stEthTrunkGroup = {0};
    int iRet = 0;

    /*聚合口*/
    for(uiLagId=1;uiLagId <= LAG_GROUP_NUM;uiLagId++)
    {
        memset(&stEthTrunkGroup, 0, sizeof(stEthTrunkGroup));
        iRet = eth_trunk_get_api(uiLagId, ETH_TRUNK_ROW_STATUS, &stEthTrunkGroup);
        if(iRet == VOS_OK)
        {
            if(stOspfDcn.uiLagPort[uiLagId-1] == OSPF_DCN_PORT_DISABLE)
            {
                continue;
            }
            if_lag_id_to_index(1, uiLagId, &uiIfIndex);
            iRet = zebra_if_get_api(uiIfIndex, ZEBRA_IF_L2_L3_MODE, &uiMode);
            if(iRet == VOS_ERR || uiMode == IF_L2_MODE)
            {
                ospf_logx(ospf_debug_dcn,"The port is not exist or is l2mode, disable dcn\r\n");
                stOspfDcn.uiLagPort[uiLagId-1] = OSPF_DCN_PORT_DISABLE;
                continue;
            }

            if(VOS_TRUE == dcn_config_load_done() && VOS_ERR == dcn_interface_vlan_check(stOspfDcn.uiVlanId, uiIfIndex))
            {
                ospf_logx(ospf_debug_dcn,"dcn vlan has been used in the subinterface, disable dcn\r\n");
                stOspfDcn.uiLagPort[uiLagId-1]  = OSPF_DCN_PORT_DISABLE;
                continue;
            }

            if(VOS_TRUE == dcn_config_load_done() && VOS_ERR == dcn_interface_exist_check(uiIfIndex))
            {
                stOspfDcn.uiLagPort[uiLagId-1] = OSPF_DCN_PORT_DISABLE;
                continue;
            }
            ospf_dcn_port_vpif_create(uiLagId + MAX_PORT_NUM);
        }
        else
        {
            stOspfDcn.uiLagPort[uiLagId-1] = OSPF_DCN_PORT_DISABLE;
        }
    }

    return VOS_OK;
}

int ospf_dcn_setSubIntf_Mod(u_long ulIfIndx,u_char ucEnmode)
{
	struct interface *ifp  = NULL;

	ifp = if_lookup_by_index(ulIfIndx);
	if(NULL == ifp)
	{
		return ERR;
	}
	
	if(ucEnmode == OSPF_DCN_PORT_ENABLE)	
	{
		SET_FLAG (ifp->status, INTERFACE_ACTIVE);
	}
	else
	{
		UNSET_FLAG (ifp->status, INTERFACE_ACTIVE);
	}
	
	return OK;
}

int ospf_dcn_neid_set(u_int uiNeid)
{
    int i = 0,ret = 0;
    
    if(uiNeid == stOspfDcn.uiNeid)
    {
        return OK;
    }
    
    stOspfDcn.uiNeid = uiNeid;

    if(stOspfDcn.ucDcnEnflag == OSPF_DCN_ENABLE)
    {
        if(stOspfDcn.ucNeidReleIp == OSPF_DCN_NEID_RELEVANCE_NEIP)
        {
            stOspfDcn.ulDeviceIp = OSPF_DCNDEF_IPADDR + stOspfDcn.uiNeid;

            ospf_dcn_modify_ip(stOspfDcn.ulDeviceIp, stOspfDcn.ulDeviceMask);
        }
    }
    else
    {
        if(stOspfDcn.ucNeidReleIp == OSPF_DCN_NEID_RELEVANCE_NEIP)
        {
            stOspfDcn.ulDeviceIp = OSPF_DCNDEF_IPADDR + stOspfDcn.uiNeid;
        }
    }
    
    return OK;
}
#endif


#ifdef OSPF_DCN

u_long ospf_dcn_get_ip()
{
    return stOspfDcn.ulDeviceIp;	
}

/*loopback 31上电加载配置时检测，若已配置默认dcn ip则先删除*/
int ospf_dcn_lpbk_ip_check(u_int uiIfindx)
{
	u_long ulIP = 0;
    struct prefix_ipv4 stIp; 
    int ret = 0;
    struct prefix stPrefx;
	
    memset(&stPrefx,0,sizeof(struct prefix));
    memset(&stIp,0,sizeof(struct prefix_ipv4));

    ulIP = stOspfDcn.ulDeviceIp;
    if(ERR == ospf_IntfIpAddr_get(uiIfindx,&stPrefx))
    {
        return OK;
    }
    if(stPrefx.u.prefix4.s_addr == ntohl(ulIP))
    {
        ret = ospf_dcn_ipaddr_uninstall(0,IFINDEX_TYPE_LOOPBACK_IF);
        if(ret != VOS_ERR_NO_ERROR)
        {
	  	    zlog_info(MTYPE_OSPF," ospf_dcn_ipaddr_uninstall: 0x%x error!\n",uiIfindx);
            OSPF_DCN_LOG_WARNING("ospf_dcn_ipaddr_uninstall ip:%x error!.",uiIfindx);
        }
        return ret;
        
    }
	
}


int ospf_dcn_network_creat(tOSPF_NETWORK_INDEX *pstOspfNet)
{
	int value_l = 0;

	value_l = TRUE;
	if(ospfNetworkSetApi(pstOspfNet,OSPF_NETWORK_STATUS,&value_l) != OK)
	{
		 ospf_logx(ospf_debug_dcn,"	%%ospf_dcn_network_creat: Failed to configure network \n");
         OSPF_DCN_LOG_WARNING("ospf_dcn_network_creat: Failed to configure network!");
		 return ERR;
	}	
   // ospf_restart_lsa_originate();
    return OK;
}

int ospf_dcn_network_delete(tOSPF_NETWORK_INDEX *pstOspfNet)
{
	int value_l = 0;

	value_l = FALSE;
	if(ospfNetworkSetApi(pstOspfNet,OSPF_NETWORK_STATUS,&value_l)!=OK)
	{
		 ospf_logx(ospf_debug_dcn,"	%%ospf_dcn_network_delete: Failed to delete network \n");
         OSPF_DCN_LOG_WARNING("Failed to delete network.");
		 return ERR;
	}	
    return OK;
}


int ospf_dcn_get_first_route(u_long *pIpAddr)
{
	long value_l=0;
	int flag = 1,ret = 0,rc = OK;
	long valLen=sizeof(long);
	tOSPF_LSDB_INDEX stOspfLsdbIndex,stOspfLsdbNextIndex;
    OSPF_DCN_NBR_T stNbr;
    char value_c[500]= {0};
	octetstring octet = {0};
    
	octet.pucBuf=value_c;
	octet.len=sizeof(value_c);
    
	if(pIpAddr == NULL)
	{
		return ERR;
	}

	ret = ospf_dcn_get_lsa_frist(&stOspfLsdbIndex);
	if(OK != ret) 
	{
        OSPF_DCN_LOG_WARNING("Failed to get first dcn lsa.");
	    return ERR;
	}

	while(OK == ret)
	{
		ret = ospf_dcn_get_lsa_next(&stOspfLsdbIndex,&stOspfLsdbNextIndex);
	
		if((stOspfLsdbIndex.process_id == OSPF_DCN_PROCESS) &&
		    (stOspfLsdbIndex.area == 0)&&
		    (stOspfLsdbIndex.type == 10))
		{
            memset(value_c,0,sizeof(value_c));
        	if((ospfLsdbGetApi(&stOspfLsdbIndex,OSPF_LSDB_ADVERTISEMENT,&octet)==OK)
                &&(octet.len > 36))                
        	{

                ospf_dcn_get_type_10(octet.pucBuf+OSPF_LSA_HLEN,&stNbr);
                
                /*异常ip过滤*/
            	if(ntohl(stNbr.ulNeIp) == 0)		
                {
                    stOspfLsdbIndex = stOspfLsdbNextIndex;
            		continue;
            	}
                
                /*本设备ip过滤*/
            	if(ntohl(stNbr.ulNeIp) == ospf_dcn_get_ip())		
                {
                    stOspfLsdbIndex = stOspfLsdbNextIndex;
            		continue;
            	}
                #if 0
                /*过滤未老化路由*/
                if(ERR == ospf_dcn_route_check(ntohl(stNbr.ulNeIp)))		
                {
                    stOspfLsdbIndex = stOspfLsdbNextIndex;
            		continue;
            	}
            	#endif
                *pIpAddr = ntohl(stNbr.ulNeIp);
                return OK;

        	}

		}
		stOspfLsdbIndex = stOspfLsdbNextIndex;
    }


	return ERR;
}




int ospf_dcn_get_next_route(u_long *pFirstIpAddr,u_long *pNextIpAddr)
{
	int rc  = OK,ret = 0;
	tOSPF_LSDB_INDEX stOspfLsdbIndex,stOspfLsdbNextIndex;
	u_int ulFlag =0;
    OSPF_DCN_NBR_T stNbr;
    char value_c[500]= {0};
	octetstring octet = {0};
	octet.pucBuf=value_c;
	octet.len=sizeof(value_c);
    if((pFirstIpAddr == NULL)||(pNextIpAddr == NULL))
	{
		return ERR;
	}
	ret = ospf_dcn_get_lsa_frist(&stOspfLsdbIndex);
	if(OK != ret) 
	{
        OSPF_DCN_LOG_WARNING("Failed to get first dcn lsa.");
	    return ERR;
	}

	while(OK == ret)
	{
		ret = ospf_dcn_get_lsa_next(&stOspfLsdbIndex,&stOspfLsdbNextIndex);
	
		if((stOspfLsdbIndex.process_id == OSPF_DCN_PROCESS) &&
		    (stOspfLsdbIndex.area == 0)&&
		    (stOspfLsdbIndex.type == 10))
		{
            memset(value_c,0,sizeof(value_c));
        	if((ospfLsdbGetApi(&stOspfLsdbIndex,OSPF_LSDB_ADVERTISEMENT,&octet)==OK)
                &&(octet.len > 36))
        	{
                ospf_dcn_get_type_10(octet.pucBuf+OSPF_LSA_HLEN,&stNbr);
                
                /*异常ip过滤*/
            	if(ntohl(stNbr.ulNeIp) == 0)		
                {
                    stOspfLsdbIndex = stOspfLsdbNextIndex;
            		continue;
            	}                

                /*本设备ip过滤*/
            	if(ntohl(stNbr.ulNeIp) == ospf_dcn_get_ip())		
                {
                    stOspfLsdbIndex = stOspfLsdbNextIndex;
            		continue;
            	}
#if 0
                /*过滤未老化路由*/
                if(ERR == ospf_dcn_route_check(ntohl(stNbr.ulNeIp)))		
                {
                    stOspfLsdbIndex = stOspfLsdbNextIndex;
            		continue;
            	}
#endif
                if(ntohl(stNbr.ulNeIp) == *pFirstIpAddr)
                {
                    ulFlag = 1;
                    stOspfLsdbIndex = stOspfLsdbNextIndex;
                    continue;
                }
                if(ulFlag == 1)
                {
                    *pNextIpAddr = ntohl(stNbr.ulNeIp);
	                return OK;
                }
        	}

		}
		stOspfLsdbIndex = stOspfLsdbNextIndex;
    }
    
	return ERR;
}


int ospf_dcn_route_get_api(OSPF_DCN_NBR_T *pstNbr,void *vValue,u_int mib_num)
{
	int ret = OK;
	if(vValue == NULL)
	{
		return  ERR;
	}
	u_long *pVal = (u_long *)vValue;

	switch(mib_num)
	{
        case DCN_REMOTE_NE_ID:
		{
			*pVal = pstNbr->uiNeid;
			break;
		}
        case DCN_REMOTE_NE_TYPE:
		{
			memcpy(vValue,pstNbr->ucDevType,strlen(pstNbr->ucDevType));
			break;
		}
        case DCN_REMOTE_NE_COMPANY:
		{
			memcpy(vValue,pstNbr->ucCompany,strlen(pstNbr->ucCompany));
			break;
		}
        case DCN_REMOTE_NE_MAC:
		{
			memcpy(vValue,pstNbr->ucDevMac,MAC_ADDR_LEN);
			break;
		}

        case DCN_REMOTE_NE_IP:
		{
			*pVal = pstNbr->ulNeIp;
			break;
		}

		case DCN_REMOTE_NE_IPV6:
		{
			memcpy(vValue,pstNbr->ulNeIpv6,sizeof(pstNbr->ulNeIpv6));
			break;
		}
		default:
	        return ERR;
	}
	return OK;
    
}

int ospf_dcn_RouteTab_get_api(u_long IpAddr,void *pbuf,u_int mib_num)
{
	long value_l=0;
	int flag = 1,ret = 0,rc = OK;
	long valLen=sizeof(long);
	tOSPF_LSDB_INDEX stOspfLsdbIndex,stOspfLsdbNextIndex;
    OSPF_DCN_NBR_T stNbr;
    char value_c[500]= {0};
	octetstring octet = {0};
	octet.pucBuf=value_c;
	octet.len=sizeof(value_c);
    

	ret = ospf_dcn_get_lsa_frist(&stOspfLsdbIndex);
	if(OK != ret) 
	{
        OSPF_DCN_LOG_WARNING("Failed to get first dcn lsa.");
	    return ERR;
	}

	while(OK == ret)
	{
		ret = ospf_dcn_get_lsa_next(&stOspfLsdbIndex,&stOspfLsdbNextIndex);
		memset(value_c,0,sizeof(value_c));
    	if((ospfLsdbGetApi(&stOspfLsdbIndex,OSPF_LSDB_ADVERTISEMENT,&octet)==OK)
            &&(octet.len > 36))
    	{

            ospf_dcn_get_type_10(octet.pucBuf+OSPF_LSA_HLEN,&stNbr);

            /*异常ip过滤*/
        	if(ntohl(stNbr.ulNeIp) == 0)		
            {
                stOspfLsdbIndex = stOspfLsdbNextIndex;
        		continue;
        	}  

            
            /*本设备ip过滤*/
        	if(ntohl(stNbr.ulNeIp) == ospf_dcn_get_ip())		
            {
                stOspfLsdbIndex = stOspfLsdbNextIndex;
        		continue;
        	}

            /*过滤未老化路由*/
            if(ERR == ospf_dcn_route_check(ntohl(stNbr.ulNeIp)))		
            {
                stOspfLsdbIndex = stOspfLsdbNextIndex;
        		continue;
        	}
            
            if(ntohl(stNbr.ulNeIp) == IpAddr)		
            {
        		ret = ospf_dcn_route_get_api(&stNbr,pbuf,mib_num);
                return ret;
        	}

    	}

		stOspfLsdbIndex = stOspfLsdbNextIndex;
    }


	return ERR;
}
int ospf_dcn_route_add(tOSPF_LSA_NET *pstRout)
{
    int i = 0;
    
    for(i = 0;i < 100;i++)
    {
        if((stOspfLsaManage.stNet[i].uiIfIndx == pstRout->uiIfIndx)
            ||(stOspfLsaManage.stNet[i].uiIp == pstRout->uiIp))

        {
            return ERR;
        }
        if(stOspfLsaManage.ucEn != OSPF_LSA_MNG_EN)
        {
            stOspfLsaManage.ucEn = OSPF_LSA_MNG_EN;
        }
        if(stOspfLsaManage.stNet[i].uiIp != 0)
        {
            continue;
        }
        if(pstRout->uiIfIndx != 0)
        {
            stOspfLsaManage.stNet[i].uiIfIndx = pstRout->uiIfIndx;
        }
		stOspfLsaManage.stNet[i].uiIp = pstRout->uiIp;
		stOspfLsaManage.stNet[i].uiMask= pstRout->uiMask;
        stOspfLsaManage.uiCnt++;

        return OK;
        
    }

}

int ospf_dcn_route_del(tOSPF_LSA_NET *pstRout)
{
    int i = 0,ret = 0;
    tOSPF_NETWORK_INDEX stOspfNet;
    memset(&stOspfNet,0,sizeof(tOSPF_NETWORK_INDEX));

    if(stOspfLsaManage.ucEn != OSPF_LSA_MNG_EN)
    {
        return ERR;
    }

    for(i = 0;i < 100;i++)
    {
        if(((stOspfLsaManage.stNet[i].uiIfIndx == pstRout->uiIfIndx)
            &&(pstRout->uiIfIndx != 0))
            ||((stOspfLsaManage.stNet[i].uiIp == pstRout->uiIp)
            &&(pstRout->uiIp != 0)))

        {
            stOspfNet.process_id = OSPF_DCN_PROCESS;
            stOspfNet.area = 0;
            stOspfNet.mask = OSPF_DCNDEF_IPMASK;
            stOspfNet.network= stOspfLsaManage.stNet[i].uiIp;
            ret = ospf_dcn_network_delete(&stOspfNet);  
            
            memset(&stOspfLsaManage.stNet[i],0,sizeof(tOSPF_LSA_NET));
            stOspfLsaManage.uiCnt--;
            return ret;
        }

    }
    return ERR;


}


int ospf_dcn_lookup_route(u_int uiIpaddr,u_int uiIfindex)
{
    int i = 0,ret = 0;



    if(stOspfLsaManage.ucEn != OSPF_LSA_MNG_EN)
    {
        return ERR;
    }
    if((uiIfindex == 0)||(uiIpaddr == 0))
    {
        return ERR;
    }
    for(i = 0;i < 100;i++)
    {
        if((stOspfLsaManage.stNet[i].uiIfIndx == uiIfindex)
            &&(stOspfLsaManage.stNet[i].uiIp == uiIpaddr))

        {
            return OK;
        }

    }
    return ERR;


}

u_long ospf_dcn_get_rx_pkt_addr(u_long ulIfIndex,u_long *pulAddr)
{
    int i = 0,ret = 0;

    *pulAddr = 0;
    for(i = 0;i < 100;i++)
    {
        if(stOspfDcn.stOspfRxPkt[i].ulIfIndex == ulIfIndex)
        {
            *pulAddr = stOspfDcn.stOspfRxPkt[i].ulAddr;
            return ret;
        }

    }
    return ERR;
}

int ospf_dcn_get_rx_pkt_flag(u_long ulIfIndex,u_long ulIpAddr)
{
    int i = 0,ret = 0;


    for(i = 0;i < 100;i++)
    {
        if((stOspfDcn.stOspfRxPkt[i].ulIfIndex == ulIfIndex)
            &&(stOspfDcn.stOspfRxPkt[i].ulAddr == ulIpAddr))
        {
		#if 0
            printf("ospf_dcn_get_rx_pkt_flag :i=%d,ulIfIndex = 0x%x,ulAddr = 0x%x ,ucRecvFlag = %d.\n",
                i,stDcnDefData.stOspfRxPkt[i].ulIfIndex,
                stDcnDefData.stOspfRxPkt[i].ulAddr,stDcnDefData.stOspfRxPkt[i].ucRecvFlag);
		#endif
            return stOspfDcn.stOspfRxPkt[i].ucRecvFlag;
        }

    }
    return ERR;
}

int ospf_dcn_rx_pkt_addr_get(u_long ulIfIndex,u_long ulAddr,u_char *pucMac)
{
    int i = 0,ret = 0;

    for(i = 0;i < 100;i++)
    {
        if(stOspfDcn.stOspfRxPkt[i].ulIfIndex == ulIfIndex)
        {
            stOspfDcn.stOspfRxPkt[i].ulAddr = ulAddr;
            stOspfDcn.stOspfRxPkt[i].ucRecvFlag = DCN_RECV_OK;
            memcpy(stOspfDcn.stOspfRxPkt[i].ucSorMac, pucMac, MAC_ADDR_LEN);

            return ret;
        }

    }
    
    for(i = 0;i < 100;i++)
    {
        if(stOspfDcn.stOspfRxPkt[i].ulIfIndex == 0)
        {
            stOspfDcn.stOspfRxPkt[i].ulIfIndex = ulIfIndex;
            stOspfDcn.stOspfRxPkt[i].ulAddr = ulAddr;
            stOspfDcn.stOspfRxPkt[i].ucRecvFlag = DCN_RECV_OK;
            memcpy(stOspfDcn.stOspfRxPkt[i].ucSorMac, pucMac, MAC_ADDR_LEN);

            return ret;
        }
     }
   return ERR;


}




/*****************************************************************************
 函 数 名  : ospf_dcn_recv_count
 功能描述  :接收报文异常计数
 输入参数  : 无
 
 输出参数  : 无
 返 回 值  : 无

 修改历史      :
 *****************************************************************************/

void ospf_dcn_recv_count()
{
	int i=0;

	for(i=0;i<100;i++)
	{
        if(stOspfDcn.stOspfRxPkt[i].ulIfIndex == 0)
		{	
			continue;
		}
		if(stOspfDcn.stOspfRxPkt[i].ucRecvFlag== DCN_RECV_OK)
		{           
			stOspfDcn.stOspfRxPkt[i].ucRecvCnt= 0;
            stOspfDcn.stOspfRxPkt[i].ucRecvFlag = DCN_RECV_ERR;
		}
		else
		{
			stOspfDcn.stOspfRxPkt[i].ucRecvCnt++;
            stOspfDcn.stOspfRxPkt[i].ucRecvFlag = DCN_RECV_ERR;
		}
		
		if(stOspfDcn.stOspfRxPkt[i].ucRecvCnt >= DCN_RECV_TICK)//4s内端口未接收到报文则清空数据
		{   
			stOspfDcn.stOspfRxPkt[i].ucRecvCnt = 0;
            stOspfDcn.stOspfRxPkt[i].ulAddr = 0;
            stOspfDcn.stOspfRxPkt[i].ucRecvFlag = DCN_RECV_ERR;
            
		}
	}


}


void ospf_dcn_set_ip_change_flag(u_short usFlag)
{
    stOspfDcn.usIpChgStart = usFlag;
}

u_short ospf_dcn_get_ip_change_flag()
{
    return stOspfDcn.usIpChgStart;
}

int ospf_dcn_vpn_create(u_int *pVpnId)
{
    u_char ucVpnName[L3VPN_NAME_LEN];
    L3VPN_ROWSTATUS_E eumRowstaus = L3VPN_VALID;
    int iRet = 0;
    L3VPN_INSTANCE_T stInstanceEntry = {0};

    memset(ucVpnName,0,L3VPN_NAME_LEN);
    memcpy(ucVpnName, OSPF_DCN_VPN_NAME, sizeof(OSPF_DCN_VPN_NAME));

    iRet = l3vpn_get_api(ucVpnName,L3VPN_ROW_SATUS, &stInstanceEntry);
    if(iRet == VOS_OK)
    {
        l3vpn_get_api(ucVpnName, L3VPN_ID_VALUE, pVpnId);
        ospf_logx(ospf_debug_dcn,"Info:dcn vpn is exist, maybe config-save is excuting\r\n");
        return iRet;
    }
    
    iRet = l3vpn_set_api(ucVpnName,L3VPN_ROW_SATUS,&eumRowstaus);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Failed to create dcn vpn\r\n");
    }

    eumRowstaus = L3VPN_FAMILY_ENTRY_VALID;
    iRet = l3vpn_family_entry_set_api(ucVpnName, L3VPN_FAMILY_IPV4, &eumRowstaus);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Failed to enable ipv4 family\r\n");
    }

    iRet = l3vpn_get_api(ucVpnName, L3VPN_ID_VALUE, pVpnId);
    
    return iRet;
}

int ospf_dcn_vpn_delete()
{
    u_char ucVpnName[L3VPN_NAME_LEN];
    L3VPN_ROWSTATUS_E eumRowstaus = L3VPN_INVALID;
    int iRet = 0;

    memset(ucVpnName,0,L3VPN_NAME_LEN);
    memcpy(ucVpnName, OSPF_DCN_VPN_NAME, sizeof(OSPF_DCN_VPN_NAME));

    eumRowstaus = L3VPN_FAMILY_ENTRY_INVALID;
    iRet = l3vpn_family_entry_set_api(ucVpnName, L3VPN_FAMILY_IPV4, &eumRowstaus);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Failed to disable ipv4 family\r\n");
    }

    iRet = l3vpn_set_api(ucVpnName,L3VPN_REFERENCE_CLEAR,&eumRowstaus);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Failed to clear vpn reference\r\n");
    }
    
    eumRowstaus = L3VPN_INVALID;
    iRet = l3vpn_set_api(ucVpnName,L3VPN_ROW_SATUS,&eumRowstaus);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Failed to create dcn vpn\r\n");
    }
    
    return iRet;
}

int ospf_dcn_bind_vpn(u_int uiIfIndex)
{
    u_char ucVpnName[L3VPN_NAME_LEN];
    int iRet = 0;
    L3VPN_ROWSTATUS_E eumRowstaus = L3VPN_VALID;

    memset(ucVpnName,0,L3VPN_NAME_LEN);
    memcpy(ucVpnName, OSPF_DCN_VPN_NAME, sizeof(OSPF_DCN_VPN_NAME));

    iRet = l3vpn_binding_interface_set_api(ucVpnName,eumRowstaus,&uiIfIndex);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Failed to bind dcn vpn index = %d\r\n",uiIfIndex); 
    }

    return iRet;
}

int ospf_dcn_disable_bind_vpn(u_int uiIfIndex)
{
    u_char ucVpnName[L3VPN_NAME_LEN];
    int iRet = 0;
    L3VPN_ROWSTATUS_E eumRowstaus = L3VPN_INVALID;

    memset(ucVpnName,0,L3VPN_NAME_LEN);
    memcpy(ucVpnName, OSPF_DCN_VPN_NAME, sizeof(OSPF_DCN_VPN_NAME));

    iRet = l3vpn_binding_interface_set_api(ucVpnName,eumRowstaus,&uiIfIndex);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Failed to disable bind dcn vpn index = %d\r\n",uiIfIndex); 
    }

    return iRet;
}

int ospf_dcn_loopback_interface_create()
{
    int iRet = 0;
    struct prefix_ipv4 stPrefixIp;
    struct in_addr stMask; 
    u_int uiIfIndex = 0;
    u_int uiIpAddr = 0;
    u_int uiVlaue = ZEBRA_IF_CREATE;

    if_loopback_id_to_index(OSPF_DCNLOOPBACK_IFINDEX, &uiIfIndex);

    iRet = zebra_if_set_api(uiIfIndex,ZEBRA_IF_ROW_STATUS,&uiVlaue);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"create loopback interface failed\r\n");
        return iRet;        
    }
    
    iRet = ospf_dcn_bind_vpn(uiIfIndex);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Dcn loopback bind vpn failed\r\n");
        return iRet;
    }

    uiIpAddr = htonl(stOspfDcn.ulDeviceIp);
    stPrefixIp.family = AF_INET;
    memcpy(&stPrefixIp.prefix, &uiIpAddr, 4);
    memcpy(&stMask, &stOspfDcn.ulDeviceMask, 4);
    stPrefixIp.prefixlen = ip_masklen(stMask);
    
    iRet = zebra_if_set_api(uiIfIndex,ZEBRA_IF_IPADRESS_SET,&stPrefixIp);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Set loopback ip address failed\r\n");
    }

    return iRet;
}

int ospf_dcn_loopback_interface_delete()
{
    int iRet = 0; 
    u_int uiIfIndex = 0;
    u_int uiValue = ZEBRA_IF_INVALID;
    struct prefix_ipv4 stPrefixIp = {0};

    if_loopback_id_to_index(OSPF_DCNLOOPBACK_IFINDEX, &uiIfIndex);

    iRet = ospf_dcn_disable_bind_vpn(uiIfIndex);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Dcn loopback delete bind vpn failed\r\n");
        return iRet;
    }

    memset(&stPrefixIp,0,sizeof(struct prefix_ipv4));
    iRet = zebra_if_get_api(uiIfIndex,ZEBRA_IF_IPADRESS_GET,&stPrefixIp);
    if(iRet == VOS_OK)
    {
        iRet = zebra_if_set_api(uiIfIndex,ZEBRA_IF_IPADRESS_DELETE,&stPrefixIp);
        if(iRet != VOS_OK)
        {
            ospf_logx(ospf_debug_dcn, "Loopback interface delete ip failed");
            return iRet;
        }
    }
    
    iRet = zebra_if_set_api(uiIfIndex,ZEBRA_IF_ROW_STATUS,&uiValue);
    if(iRet == VOS_ERR)
    {
        ospf_logx(ospf_debug_dcn,"Delete ospf dcn interface failed\r\n");
    }

    return iRet;
}

int ospf_dcn_loopback_ip_set(u_int uiIp, u_int uiMask)
{
    int iRet = 0;
    struct prefix_ipv4 stPrefixIp;
    struct in_addr stMask; 
    u_int uiIfIndex = 0;
    u_int uiIpAddr = 0;

    if_loopback_id_to_index(OSPF_DCNLOOPBACK_IFINDEX, &uiIfIndex);

    uiIpAddr = htonl(uiIp);
    stPrefixIp.family = AF_INET;
    memcpy(&stPrefixIp.prefix, &uiIpAddr, 4);
    uiMask = htonl(uiMask);
    memcpy(&stMask, &uiMask, 4);
    stPrefixIp.prefixlen = ip_masklen(stMask);
    
    iRet = zebra_if_set_api(uiIfIndex,ZEBRA_IF_IPADRESS_SET,&stPrefixIp);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Set loopback ip address failed\r\n");
    }

    return iRet;

}

int ospf_dcn_port_vpif_ip_set(u_int uiLportNo)
{
    int iRet = 0;
    struct prefix_ipv4 stPrefixIp;
    struct in_addr stMask;
    u_int uiIpAddr = 0;
    u_int uiVpIfIndex = 0;
    u_int uiLoopbackIndex = 0;

    if(uiLportNo > MAX_PORT_NUM)
    {
        return ERR;
    }

    if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLportNo,stOspfDcn.uiVlanId,&uiVpIfIndex))
    {
        return ERR;
    }

    memset(&stPrefixIp, 0, sizeof(stPrefixIp)); 
    uiIpAddr = htonl(stOspfDcn.ulDeviceIp);
    stPrefixIp.family = AF_INET;
    memcpy(&stPrefixIp.prefix, &uiIpAddr, 4);
    memcpy(&stMask, &stOspfDcn.ulDeviceMask, 4);
    stPrefixIp.prefixlen = ip_masklen(stMask);

    iRet = zebra_if_set_api(uiVpIfIndex,ZEBRA_IF_SET_UNNUMBER,&stPrefixIp);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Set ip address unnumber failed\r\n");
    }

    if_loopback_id_to_index(OSPF_DCN_LOOPBACK_INDEX, &uiLoopbackIndex);
    zebra_if_set_api(uiVpIfIndex, ZEBRA_IF_UNNUMBER_IFINDEX,&uiLoopbackIndex);
    
    return iRet;
}

int ospf_dcn_slot_set(u_int uiSlot)
{
    int i;
    u_int uiDevType = 0;
    u_int uiLport = 0,uiBand = 0,uiVlan = 0;
    u_long uiIfIndex = 0,ulLbIp = 0;

	for(i = 1; i <= SLOT_MAX_PORT_NUM; i++)
	{
	    if(stOspfDcn.uiUsePort[uiSlot-1][i-1] == OSPF_DCN_PORT_DISABLE)
	    {
	        continue;
	    }
	    
		uiLport = LOGIC_PORT_GENERATE(uiSlot, i);
        if(VOS_OK != if_logic_port_to_index(uiLport, &uiIfIndex))
        {
            continue;
        }
        #if 0
        /*删除dcn接口网络*/
        ospf_dcn_set_dcn_mode_by_ifindex(uiIfIndex,L3_IF_DCN_MODE_NONE);
        ospf_dcn_remove_port_by_ifindex(uiIfIndex);

        /*取消dcn 带宽限制*/
        ospf_dcn_get_bandwidth(&uiBand);
        ospf_dcn_vlan_get(&uiVlan);
        ospf_dcn_set_eth_bandwidth(i, uiBand,uiVlan,DCN_BAND_DEL);
        #endif
        if(ERR ==  ospf_dcn_port_vpif_ip_set(uiLport))
        {
            ospf_logx(ospf_debug_dcn,"ospf dcn port vpif ip set uiLport: %d err!\n",uiLport);
        }
        
    }

    return OK;    
}

int ospf_dcn_interface_ip_set()
{
    u_int uiSlot = 0;

    for(uiSlot = 1;uiSlot <= MAX_SLOT_NUM; uiSlot++)
    {
    	ospf_dcn_slot_set(uiSlot);
    }	   
}

int ospf_dcn_interface_create()
{
    u_int uiLagId = 0;
    u_int uiIfIndex = 0, uiMode = 0;
    ETH_TRUNK_GROUP_T stEthTrunkGroup = {0};
    int iRet = 0;
    
    ospf_dcn_loopback_interface_create();

    ospf_dcn_slot_init_all();

    ospf_trunk_init_all();

    return VOS_OK;
}

int ospf_dcn_interface_delete()
{
    u_int uiSlot = 0;
    u_int uiLagId = 0;
    u_int uiIfindex = 0;
    ETH_TRUNK_GROUP_T stEthTrunkGroup;
    int iRet = 0; 

    for(uiSlot = 1;uiSlot <= MAX_SLOT_NUM; uiSlot++)
	{
		ospf_dcn_slot_remove(uiSlot);
	}

	ospf_dcn_trunk_remove();
	
    ospf_dcn_loopback_interface_delete();

	return VOS_OK;
}

int ospf_dcn_vlan_create(u_int uiVlanId)
{
    VLAN_ROWSTATUS_E eumRowstaus;
    
    eumRowstaus = VLAN_VALID;
    if(VOS_OK != vlan_set_api(uiVlanId,VLAN_ROW_STATUS,&eumRowstaus))
    {
        ospf_logx(ospf_debug_dcn,"Failed to create dcn vlan %d\r\n",uiVlanId); 
        return VOS_ERR;
    }
    eumRowstaus = VLAN_ACTIVE;
    if(VOS_OK != vlan_set_api(uiVlanId,VLAN_ROW_STATUS,&eumRowstaus))
    {
        ospf_logx(ospf_debug_dcn,"Failed to active dcn vlan %d\r\n",uiVlanId); 
        return VOS_ERR;
    }

    return VOS_OK;
}

int ospf_dcn_vlan_delete(u_int uiVlanId)
{
    VLAN_ROWSTATUS_E eumRowstaus;
    
    eumRowstaus = VLAN_INVALID;
    if(VOS_OK != vlan_set_api(uiVlanId,VLAN_ROW_STATUS,&eumRowstaus))
    {
        ospf_logx(ospf_debug_dcn,"Failed to delete dcn vlan %d\r\n",uiVlanId); 
        return VOS_ERR;
    }

    return VOS_OK;
}

int ospf_dcn_vlan_dot1q_enable(u_int uiIfIndex, u_int uiVlanId)
{
    u_int uiBindvlan = 0;
    u_int uiIfMainIndex = 0;
    u_int uiCarryIfIndex = 0;
    u_int iRet = 0;
    u_int uiTrunkId = 0;
    LINK_TYPE_E ucLinkType;  
    //u_int uiTrunkMemberIfindex[ETH_TRUNK_GROUP_MAX_PORT_NUM];
    //int i;
    ETH_TRUNK_PORT_INDEX_T stTrunkPortIndex;
    IF_TERMINATION_SUBIF_ST stTermin = {0};
     
    if(TRUE != vlan_exit(uiVlanId))
    {
        ospf_logx(ospf_debug_dcn, "Error: create vlan %s first.", uiVlanId);
        return VOS_ERR;
    }

    if(VOS_OK != zebra_if_get_api(uiIfIndex,ZEBRA_IF_GET_IF_BIND_VLAN,&uiBindvlan))
    {
        ospf_logx(ospf_debug_dcn, "Error: The interface existence check failed.");
        return VOS_ERR;
    }

    if(0 != uiBindvlan)
    {
        ospf_logx(ospf_debug_dcn, "Error: The interface has been bind with vlan %d");
        return VOS_ERR;
    }
    if(VOS_OK != zebra_if_get_api(uiIfIndex,ZEBRA_IF_GET_TERMIF_VLAN,&stTermin))
    {
        ospf_logx(ospf_debug_dcn, "Error: zebra if get api ZEBRA_IF_GET_TERMIF_VLAN");
        return VOS_ERR;
    }
    if(stTermin.uiCtrlVlan != 0)
    {
        ospf_logx(ospf_debug_dcn, "Error: Failed to configure the vlan-type because the control VLAN exists.");
        return VOS_ERR;
    }
    #if 0
    if(uiBindvlan && uiBindvlan != uiVlanId)
    {

        if(mpls_l2vc_lookup_api(L2VC_STATIC_LOOKUP_CMD, MPLS_L2VC_IFINDEX_SUB_CMD, &uiIfIndex))
        {
            vty_out (vty, "Error: This interface is bound to a VLL. Please unbind this interface first.%s", VTY_NEWLINE);
            return CMD_WARNING;
        }

    }
    #endif
    if(VOS_OK != zebra_if_set_api(uiIfIndex,ZEBRA_IF_QOT1Q_VLAN_ID,&uiVlanId))
    {
        ospf_logx(ospf_debug_dcn, "Error: vlan-type dot1q %d", uiVlanId);
        return VOS_ERR;
    }
    
    /*在以太网子接口视图下执行命令vlan-type dot1q则表示该VLAN已被该子接口独占
  VLAN不能再以802.1P方式配置在其他子接口上。*/
    if(IFINDEX_TO_TYPE(uiIfIndex) == IFINDEX_TYPE_VPIF)
    {
        iRet = if_vport_ifindex_to_ifindex(uiIfIndex, &uiCarryIfIndex);
        if(iRet == VOS_OK && uiCarryIfIndex != 0)
        {
            iRet = vlan_get_api(uiVlanId,VLAN_ALLOW,&uiCarryIfIndex);
            if(iRet == VOS_OK)
            {
                ospf_logx(ospf_debug_dcn, "Error: The specified VLAN ID has been used by the other interface.");
                return VOS_ERR;
            }
            iRet = port_get_api(uiCarryIfIndex,PORT_LINK_TYPE,&ucLinkType);
            if(iRet != VOS_OK)
            {
                ospf_logx(ospf_debug_dcn, "port get  failed");
            }

            if(ucLinkType == LINK_TYPE_TRUNK)
            {
                iRet |= vlan_set_api(uiVlanId,VLAN_ADD_TRUNK_PORT,&uiCarryIfIndex); 
                iRet |= zebra_if_set_api(uiIfIndex,ZEBRA_IF_SET_IF_BIND_VLAN,&uiVlanId);
            }
            else if(ucLinkType == LINK_TYPE_HYBRID)
            {
                iRet = vlan_set_api(uiVlanId,VLAN_ADD_HYBRID_PORT,&uiCarryIfIndex); 
                iRet |= zebra_if_set_api(uiIfIndex,ZEBRA_IF_SET_IF_BIND_VLAN,&uiVlanId);
            }
            else
            {
                 ospf_logx(ospf_debug_dcn, "vlan-type cmd not support in this port failed%s");
            }
        }
              
    }
    else if(IFINDEX_TO_TYPE(uiIfIndex) == IFINDEX_TYPE_ETH_VTRUNK)
    {
        /*Resolve the aggregation sub-interface to display VLAN encapsulation. add by ctf 2018/2/3*/
         iRet = if_vlag_ifindex_to_lag_ifindex(uiIfIndex,&uiIfMainIndex);
        if(iRet == VOS_OK)
     {
               iRet = if_index_to_lag_id(uiIfMainIndex,&uiTrunkId);
        }
    /* else
     {
               iRet = if_index_to_lag_id(uiIfIndex,&uiTrunkId);
     }*/
    
       // iRet = if_index_to_lag_id(uiIfIndex,&uiTrunkId);
        if(iRet != VOS_OK)
        {
            return iRet;
        }
        memset(&stTrunkPortIndex,0,sizeof(stTrunkPortIndex));
        stTrunkPortIndex.usTrunkId = uiTrunkId;
        for(iRet =eth_trunk_group_port_get_api(&stTrunkPortIndex,ETH_TRUNK_ID_GET_GROUP_FIRST_PORT,&stTrunkPortIndex);
            VOS_OK == iRet;
            iRet =eth_trunk_group_port_get_api(&stTrunkPortIndex,ETH_TRUNK_ID_GET_GROUP_NEXT_PORT,&stTrunkPortIndex))

        {
            if(stTrunkPortIndex.ulTrunkIfIndex != 0)
            {
                   iRet = vlan_get_api(uiVlanId,VLAN_ALLOW,&stTrunkPortIndex.ulTrunkIfIndex);
           if(iRet == VOS_OK)
           {
            ospf_logx(ospf_debug_dcn, "Error: The specified VLAN ID has been used by the other interface");
            return VOS_ERR;
           }               
                iRet = vlan_set_api(uiVlanId,VLAN_ADD_HYBRID_PORT,&stTrunkPortIndex.ulTrunkIfIndex); 
                if(iRet != VOS_OK)
                {
                    ospf_logx(ospf_debug_dcn,"Error: port link-type set failed.");
                    return iRet;
                }
            }
        }
  #if 0
        iRet = eth_trunk_get_api(uiTrunkId,ETH_TRUNK_GET_MEMBER, uiTrunkMemberIfindex);
        for(i = 0;i< ETH_TRUNK_GROUP_MAX_PORT_NUM;i++)
        {
            if(uiTrunkMemberIfindex[i] != 0)
            {
                iRet = vlan_set_api(uiVlanId,VLAN_ADD_HYBRID_PORT,&uiTrunkMemberIfindex[i]); 
            }
         if(iRet == VOS_OK)
        mpls_interface_vlan_change_api(uiIfIndex, (u_short)uiVlanId);
        }
 #endif
        iRet |= zebra_if_set_api(uiIfIndex,ZEBRA_IF_SET_IF_BIND_VLAN,&uiVlanId);
    
    }
    else
    {
        ospf_logx(ospf_debug_dcn, "Error:not support in this port.");
    }
    #if 0
    /*add by chtf  2018/3/15 for mantis 0000866*/
    u_int uiL3VpnId = 0;
    if(1)
    {
       
    if(VOS_OK != zebra_if_get_api(uiIfIndex,ZEBRA_IF_VRF_ID_GET,&uiL3VpnId))            
    {
        vty_out(vty, "Error: fail to get l3vpn id %s", VTY_NEWLINE);
        return CMD_WARNING;
    }
    
    if(0 != uiL3VpnId)
    {
       iRet = zebra_if_set_api(uiIfIndex, ZEBRA_IF_VRF_ID_ADD, &uiL3VpnId);
       if(VOS_OK != iRet)
       {
        vty_out(vty, "Error: fail to change if vrf vlan %s", VTY_NEWLINE);
        return CMD_WARNING;    
       }
    }
    }
    #endif
    /*end add by chtf  2018/3/15*/
    #if 0
    //start zhurish
       if(IFINDEX_TO_TYPE(uiIfIndex) == IFINDEX_TYPE_VPIF || IFINDEX_TO_TYPE(uiIfIndex) == IFINDEX_TYPE_ETH_VTRUNK)
        interface_vlan_sync_mpls(uiIfIndex, (u_short)uiVlanId,  uiL3VpnId,  MPLS_EVENT_ADD);
    //end zhurish 
    #endif
    return VOS_OK;
}

int ospf_dcn_vlan_dot1q_disable(u_int uiIfIndex, u_int uiVlanId)
{
    u_int uiBindvlan = 0;
    u_int iRet = 0;
    u_int uiIfMainIndex = 0;
    u_int uiCarryIfIndex = 0;
    u_int uiVlanZero = 0;   
    u_int uiIpset = 0;
    u_int uiTrunkId = 0;
    LINK_TYPE_E ucLinkType;
    ETH_TRUNK_PORT_INDEX_T stTrunkPortIndex;
    //u_int uiTrunkMemberIfindex[ETH_TRUNK_GROUP_MAX_PORT_NUM];
    //int i;
    
    if(uiVlanId < MIN_VLAN_ID || uiVlanId > MAX_VLAN_ID)
    {
        ospf_logx(ospf_debug_dcn,"vlan range must be %d-%d",MIN_VLAN_ID,MAX_VLAN_ID);
        return VOS_ERR;
    }

    if(VOS_OK != zebra_if_get_api(uiIfIndex,ZEBRA_IF_GET_IF_BIND_VLAN,&uiBindvlan))
    {
        ospf_logx(ospf_debug_dcn, "Error: The interface existence check failed.");
        return VOS_ERR;
    }
    
    if(uiVlanId == uiBindvlan)
    {
        if(VOS_OK != zebra_if_get_api(uiIfIndex,ZEBRA_IF_GET_IF_IP_IS_SET_OR_NOT,&uiIpset))
        {
            ospf_logx(ospf_debug_dcn, "Error: The interface existence check failed.");
            return VOS_ERR;
        }

        if(uiIpset == 1)
        {
            ospf_logx(ospf_debug_dcn, "Error: undo ip address  first");
            return VOS_ERR;
        }
#if 0
    if(mpls_l2vc_lookup_api(L2VC_STATIC_LOOKUP_CMD, MPLS_L2VC_IFINDEX_SUB_CMD, &uiIfIndex))
    {
        vty_out (vty, "Error: This interface is bound to a VLL. Please unbind this interface first.%s", VTY_NEWLINE);
        return CMD_WARNING;
    }
#endif
        if(VOS_OK != zebra_if_set_api(uiIfIndex,ZEBRA_IF_QOT1Q_VLAN_ID,&uiVlanZero))
        {
            ospf_logx(ospf_debug_dcn, "Error: undo vlan-type dot1q %d", uiVlanId);
            return VOS_ERR;
        }
        if(IFINDEX_TO_TYPE(uiIfIndex) == IFINDEX_TYPE_VPIF)
        {
            iRet |= if_vport_ifindex_to_ifindex(uiIfIndex, &uiCarryIfIndex);
            iRet |= port_get_api(uiCarryIfIndex,PORT_LINK_TYPE,&ucLinkType);
            if(iRet != VOS_OK)
            {
                ospf_logx(ospf_debug_dcn, "port_get_api failed");
            }
            if(ucLinkType == LINK_TYPE_TRUNK)
            {
                iRet |= vlan_set_api(uiBindvlan,VLAN_DEL_TRUNK_PORT,&uiCarryIfIndex); 
            }
            else if(ucLinkType == LINK_TYPE_HYBRID)
            {
                iRet |= vlan_set_api(uiBindvlan,VLAN_DEL_HYBRID_PORT,&uiCarryIfIndex); 
            }
            else
            {
                ospf_logx(ospf_debug_dcn, "vlan-type cmd not support in this port failed");
            }
        }
        else if(IFINDEX_TO_TYPE(uiIfIndex) == IFINDEX_TYPE_ETH_VTRUNK)
        {

          /*Resolve the aggregation sub-interface to display VLAN encapsulation. add by ctf 2018/2/5*/  
          iRet = if_vlag_ifindex_to_lag_ifindex(uiIfIndex,&uiIfMainIndex);
          if(iRet == VOS_OK)
         {
          iRet = if_index_to_lag_id(uiIfMainIndex,&uiTrunkId);
         }
         else
         {
             iRet = if_index_to_lag_id(uiIfIndex,&uiTrunkId);
         }

            if(iRet != VOS_OK)
            {
                return iRet;
            }/*modify by lrf 2018/3/6modify by lrf 2018/3/5:eth_trunk子接口下vlan配置 删除，不成功问题*/
          memset(&stTrunkPortIndex,0,sizeof(stTrunkPortIndex));
             stTrunkPortIndex.usTrunkId = uiTrunkId;
        for(iRet =eth_trunk_group_port_get_api(&stTrunkPortIndex,ETH_TRUNK_ID_GET_GROUP_FIRST_PORT,&stTrunkPortIndex);
            VOS_OK == iRet;
            iRet =eth_trunk_group_port_get_api(&stTrunkPortIndex,ETH_TRUNK_ID_GET_GROUP_NEXT_PORT,&stTrunkPortIndex))

        {
            if(stTrunkPortIndex.ulTrunkIfIndex != 0)
            {                          
                 iRet = vlan_set_api(uiVlanId,VLAN_DEL_HYBRID_PORT,&stTrunkPortIndex.ulTrunkIfIndex);
               
            }
        
        }
#if 0
            iRet = eth_trunk_get_api(uiTrunkId,ETH_TRUNK_GET_MEMBER, uiTrunkMemberIfindex);
            for(i = 0;i< ETH_TRUNK_GROUP_MAX_PORT_NUM;i++)
            {
                if(uiTrunkMemberIfindex[i] != 0)
                {
                    iRet = vlan_set_api(uiVlanId,VLAN_DEL_HYBRID_PORT,&uiTrunkMemberIfindex[i]); 
                }
            }
#endif			
        }
        else
        {
            ospf_logx(ospf_debug_dcn, "Error:not support in this port.");
        }
        uiBindvlan = 0;
        zebra_if_set_api(uiIfIndex,ZEBRA_IF_SET_IF_BIND_VLAN,&uiBindvlan);
    }
    else
    {
        ospf_logx(ospf_debug_dcn, "Error: undo vlan-type dot1q %d not exit.", uiVlanId);
    }
    #if 0
    //start zhurish
       if(IFINDEX_TO_TYPE(uiIfIndex) == IFINDEX_TYPE_VPIF || IFINDEX_TO_TYPE(uiIfIndex) == IFINDEX_TYPE_ETH_VTRUNK)
        interface_vlan_sync_mpls(uiIfIndex, (u_short)uiBindvlan,  0,  MPLS_EVENT_DEL);
    //end zhurish 
    #endif
    return VOS_OK;
}

int ospf_dcn_ip_unnumber_cfg(u_int uiIfIndex, u_char ucEnable)
{
    int iRet = 0;
    struct prefix_ipv4 stPrefixIp;
    struct in_addr stMask;
    u_int uiIpAddr = 0;
    u_int uiLoopbackIndex = 0;
    
    memset(&stPrefixIp, 0, sizeof(struct prefix_ipv4));
    if(ENABLE == ucEnable)
    {
        uiIpAddr = htonl(stOspfDcn.ulDeviceIp);
        stPrefixIp.family = AF_INET;
        memcpy(&stPrefixIp.prefix, &uiIpAddr, 4);
        memcpy(&stMask, &stOspfDcn.ulDeviceMask, 4);
        stPrefixIp.prefixlen = ip_masklen(stMask);
    }
    iRet = zebra_if_set_api(uiIfIndex,ZEBRA_IF_SET_UNNUMBER,&stPrefixIp);
    if(iRet != VOS_OK)
    {
        ospf_logx(ospf_debug_dcn,"Set ip address unnumber failed\r\n");
    }
    
    if_loopback_id_to_index(OSPF_DCN_LOOPBACK_INDEX, &uiLoopbackIndex);
    zebra_if_set_api(uiIfIndex, ZEBRA_IF_UNNUMBER_IFINDEX,&uiLoopbackIndex);

    return iRet;
}

int ospf_dcn_one_interface_delete(u_int uiIfIndex)
{
    int iRet = 0;
    u_int uiValue = ZEBRA_IF_INVALID;
    
    iRet = zebra_if_set_api(uiIfIndex,ZEBRA_IF_ROW_STATUS,&uiValue);
    if(iRet == VOS_ERR)
    {
        ospf_logx(ospf_debug_dcn,"Delete ospf dcn interface failed\r\n");
    }

    return iRet;
}

int ospf_dcn_one_interface_enable(u_int uiIfIndex, u_char ucPortEn)
{
    u_int uiSlot = IFINDEX_TO_SLOT(uiIfIndex);
    u_int uiType = IFINDEX_TO_TYPE(uiIfIndex);
    u_int uiInPort = IFINDEX_TO_INTF_PORT_NO(uiIfIndex);
    u_int uiLogicPort = 0;
    tOSPF_IFINDEX stIfIndex = {0};
    int iRet = 0;

    if(uiType != IFINDEX_TYPE_ETH_TRUNK)
    {
        if_index_to_logic_port(uiIfIndex, &uiLogicPort);
    
    }
    else
    {
        uiLogicPort = VPORT_IF_IFINDEX_TO_PORT(uiIfIndex);
    }

    if((uiSlot < 1 || uiSlot > MAX_SLOT_NUM) && (uiType != IFINDEX_TYPE_ETH_TRUNK))
    {
        vty_out_to_all_terminal("Error:if slot error");
        return VOS_ERR;
    }

    if((uiLogicPort < 1 || uiLogicPort > MAX_PORT_NUM) && (uiType != IFINDEX_TYPE_ETH_TRUNK))
    {
        vty_out_to_all_terminal("Error:if logic port error");
        return VOS_ERR;
    }

    if((uiLogicPort < 1 || uiLogicPort > LAG_GROUP_NUM) && (uiType == IFINDEX_TYPE_ETH_TRUNK))
    {
        vty_out_to_all_terminal("Error:eth if logic port error");
        return VOS_ERR;
    }
	
    if(uiInPort < 1 || uiInPort > SLOT_MAX_PORT_NUM)
    {
        vty_out_to_all_terminal("Error:port number in one slot is error");
        return VOS_ERR;
    }

    if((uiType != IFINDEX_TYPE_ETH_TRUNK) && (stOspfDcn.uiUsePort[uiSlot-1][uiInPort-1] == ucPortEn))
    {
        return VOS_OK;
    }

    if((uiType == IFINDEX_TYPE_ETH_TRUNK) && (stOspfDcn.uiLagPort[uiLogicPort-1] == ucPortEn))
    {
        return VOS_OK;
    }
    if(uiType != IFINDEX_TYPE_ETH_TRUNK)
    {
        stOspfDcn.uiUsePort[uiSlot-1][uiInPort-1] = ucPortEn;
    }
    else
    {
        stOspfDcn.uiLagPort[uiLogicPort-1] = ucPortEn;
    }


    if(stOspfDcn.ucDcnEnflag == OSPF_DCN_ENABLE)
    {
        stIfIndex.process_id = OSPF_DCN_PROCESS;
	    if(uiType != IFINDEX_TYPE_ETH_TRUNK)
	    {
	        if_vport_if_logic_port_subid_to_index(uiLogicPort,stOspfDcn.uiVlanId,&stIfIndex.addrlessif);
	    }
	    else
	    {
	        if_vlag_logic_port_subid_to_index(uiLogicPort, stOspfDcn.uiVlanId,&stIfIndex.addrlessif);
	        uiLogicPort = MAX_PORT_NUM + uiLogicPort;
	    }

        stIfIndex.ipaddr = 0;
        if(ucPortEn == OSPF_DCN_PORT_ENABLE)
        {
            ospf_dcn_port_vpif_create(uiLogicPort);
            iRet = ospfIfSetApi(&stIfIndex, OSPF_IF_DCN_ADD, &uiLogicPort);
            if(iRet != VOS_OK)
            {
                ospf_logx(ospf_debug_dcn,"add dcn if failed\r\n");
            }
        }
        else if(ucPortEn == OSPF_DCN_PORT_DISABLE)
        {
            iRet = ospfIfSetApi(&stIfIndex, OSPF_IF_DCN_DELETE, &uiLogicPort);
            if(iRet != VOS_OK)
            {
                ospf_logx(ospf_debug_dcn,"delete dcn if failed\r\n");
            }
            ospf_dcn_port_vpif_destroy(uiLogicPort);
        }
    }

    return VOS_OK;
}

int ospf_dcn_global_vlan_modify(u_int uiVlanId)
{
    tOSPF_NETWORK_INDEX	stOspfNetWorkIndex = {0};
    u_int uiValue = 0;
    u_int uiSlot = 0;
    
    if(stOspfDcn.ucDcnEnflag == ENABLE)
    {
        /*先删除dcn网络*/
    	stOspfNetWorkIndex.area = 0;
    	stOspfNetWorkIndex.mask = 0;
    	stOspfNetWorkIndex.network = 0;
    	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
    	uiValue = FALSE;

    	if(ospfNetworkSetApi(&stOspfNetWorkIndex,OSPF_NETWORK_STATUS,&uiValue)!=OK)
    	{
    	     ospf_logx(ospf_debug_dcn,"Failed to configure network:%d \r\n",__LINE__);
    	}
    	
    	for(uiSlot = 1;uiSlot <= MAX_SLOT_NUM; uiSlot++)
    	{
    		ospf_dcn_slot_remove(uiSlot);
    	}

        ospf_dcn_trunk_remove();

        if(dcn_vlan_used_num(stOspfDcn.uiVlanId) == 0)
        {
    	    ospf_dcn_vlan_delete(stOspfDcn.uiVlanId);
    	}
    	else
    	{
            ospf_logx(ospf_debug_dcn,"dcn do not delete vlan %d because the vlan is used\r\n",stOspfDcn.uiVlanId);
    	}

    	stOspfDcn.uiVlanId = uiVlanId;

    	ospf_dcn_vlan_create(stOspfDcn.uiVlanId);

    	ospf_dcn_slot_init_all();

        ospf_trunk_init_all();

    	uiValue = TRUE;

    	if(ospfNetworkSetApi(&stOspfNetWorkIndex,OSPF_NETWORK_STATUS,&uiValue)!=OK)
    	{
    	     ospf_logx(ospf_debug_dcn,"Failed to configure network:%d \r\n",__LINE__);
    	}
    }
    
    stOspfDcn.uiVlanId = uiVlanId;

    return VOS_OK;
}

int ospf_dcn_process_default_cfg()
{
    tOSPF_NETWORK_INDEX	stOspfNetWorkIndex = {0};
    tOSPF_IFINDEX  stIfIndex = {0};
    tOSPF_IFINDEX stNextIndex = {0};
    u_int uiValue = 0;
    int iRet = 0;

    uiValue = TRUE;
    ospfSetApi(OSPF_DCN_PROCESS, OSPF_GBL_OPAQUE, &uiValue);

    vos_pthread_delay(200);

    ospfSetApi(OSPF_DCN_PROCESS, OSPF_GBL_LOOPPREVENT, &uiValue);

    stOspfNetWorkIndex.network = 0;
    stOspfNetWorkIndex.mask = 0;
    stOspfNetWorkIndex.area = 0;
    stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;

    vos_pthread_delay(100);

    ospfNetworkSetApi(&stOspfNetWorkIndex, OSPF_NETWORK_STATUS, &uiValue);

    ospf_dcn_lsa_update();
    
    return VOS_OK;
}

int ospf_dcn_lsa_update()
{
#ifdef OSPF_DCN
	tOSPF_NETWORK_INDEX network_index;
	uint32_t offset = 0;
	uint32_t count = 0;
	uint8_t buf[512] = {0};
    octetstring octetStat;
    char szcDevStr[64] = {0};
	u_int ulid = 0,ulIp = 0;
	struct ospf_process *p_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    u_short usVarLen = 0x20;

	/*
	厂家标示：	Type：0x8000； Length：标示名称长度
	设备型号：	Type：0x8001； Length：型号名称长度
	设备桥MAC： 	Type：0x8002； Length：6
	NEID:			Type：0x8003； Length：4
	NEIP IPV4： 	Type：0x8004； Length：4
	NEIP IPV6： 	Type：0x8005； Length：16
	*/

	memset(buf,0,512);
	offset = 0;

    memset(szcDevStr, 0, sizeof(szcDevStr));
	memcpy(szcDevStr, stOspfDcn.ucDevName, strlen(stOspfDcn.ucDevName));
	
	/*provider*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x00;
	buf[offset++] = (usVarLen & 0xff00)>>8;
	buf[offset++] = usVarLen & 0xff;
	memcpy(buf+offset,szcDevStr,usVarLen);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] pvoider %s len %d", \
		buf+offset,usVarLen);
		
	offset += usVarLen;//strlen(szcDevStr);

    memset(szcDevStr, 0, sizeof(szcDevStr));
	memcpy(szcDevStr, stOspfDcn.ucDevType, strlen(stOspfDcn.ucDevType));
    usVarLen = 16;

	/*sys descipter*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x01;
    
	buf[offset++] = (usVarLen& 0xff00)>>8;
	buf[offset++] = usVarLen & 0xff;
	
	memcpy(buf+offset,szcDevStr,usVarLen);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] device %s len %d", \
		buf+offset,usVarLen);
	offset += usVarLen;

    memset(szcDevStr, 0, sizeof(szcDevStr));
    memcpy(szcDevStr, stOspfDcn.ucDevMac, strlen(stOspfDcn.ucDevMac));
    usVarLen = 6;

	/*sys mac*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x02;

	buf[offset++] = (usVarLen& 0xff00)>>8;
	buf[offset++] = usVarLen & 0xff;
	memcpy(buf+offset,szcDevStr,usVarLen);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] mac %02x:%02x:%02x:%02x:%02x:%02x len %d", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3],buf[offset+4],buf[offset+5],6);
	offset += usVarLen;

	/*neid*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x03;
	buf[offset++] = 0;
	buf[offset++] = 4;
  	ulid = htonl(stOspfDcn.uiNeid);
	memcpy(buf+offset,&ulid,4);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] neid %d.%d.%d.%d len 4", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3]);
	offset += 4;

  	ulid = htonl(stOspfDcn.ulDeviceIp);
	/*neip*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x04;
	buf[offset++] = 0;
	buf[offset++] = 4;
	memcpy(buf+offset,&ulid,4);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] neipv4 %d.%d.%d.%d len 4", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3]);
	offset += 4;

	/*neip*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x05;
	buf[offset++] = 0;
	buf[offset++] = 16;
//	memse(buf+offset,&ulid,4);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] neipv6 %d.%d.%d.%d len 4", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3]);
	offset += 16;
	
	octetStat.pucBuf = buf;
	octetStat.len  = offset;

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] deliver dcn lsa to ospf,total len %d",offset);

    if(ospfSetApi(OSPF_DCN_PROCESS,OSPF_GBL_DCNLSA,&octetStat)!=OK)
    {
        ospf_logx(ospf_debug_dcn,"\n\r [dcn] set ospf dcn lsa failed");
    }

    #if 0
    p_process = ospf_process_lookup(&ospf, OSPF_DCN_PROCESS);

    if(p_process != NULL)
    {
    	p_process->uiDcnNeidCnt++;
    	if(ospfSetApi(OSPF_DCN_PROCESS,OSPF_GBL_DCNLSA,&octetStat)!=OK)
    	{
    	    p_process->uiDcnNeidErrCnt++;
    		ospf_logx(ospf_debug_dcn,"\n\r [dcn] set ospf dcn lsa failed");
    		OSPF_DCN_LOG_WARNING("Failed to set ospf dcn lsa.");
    	}
	}
	#endif
	
#endif

}

int ospf_dcn_lldp_init()
{
    int iRet = VOS_OK;
    u_char ucVpnName[L3VPN_NAME_LEN];
    u_int uiVrfId = 0;

    memset(ucVpnName,0,L3VPN_NAME_LEN);
    memcpy(ucVpnName, OSPF_DCN_VPN_NAME, sizeof(OSPF_DCN_VPN_NAME));

    lldpd_set_api(LLDPD_DCN_MANAGE_ADDR, &stOspfDcn.ulDeviceIp, NULL);

    lldpd_set_api(LLDPD_DCN_MANAGE_VLAN, &stOspfDcn.uiVlanId, NULL);

    l3vpn_get_api(ucVpnName, L3VPN_ID_VALUE, &uiVrfId);
    lldpd_set_api(LLDPD_DCN_MANAGE_VRF, &uiVrfId, NULL);

    return iRet;
}

/*DCN创建函数*/
int ospf_dcn_create()
{
	u_long ulValue=0;
	tOSPF_NETWORK_INDEX	stOspfNetWorkIndex;
	int iRtv = 0;
	u_int state = 0;
    octetstring stOct;
	u_int uiLbIp = 0;
	u_int ulNetWorkMask = 0;
	u_char ucIpAddr[4];
	u_long	ulMaskIp = 0;
	u_int uiVpnId=0;
	u_long ulns=0;
	u_long ulLagId = 0;
	u_int uiIfindex= 0;
	u_int uiRouteId = OSPF_NO_SET_VAL;
	
    iRtv =ospf_dcn_get_globale_enable();
	if(iRtv == OSPF_DCN_ENABLE)
    {
        return OK;
    }

    ospf_dcn_vpn_create(&uiVpnId);

    if(dcn_config_load_done() == VOS_TRUE)
    {
        ospf_dcn_vlan_create(stOspfDcn.uiVlanId);
    }
    
    ospf_dcn_interface_create();
    
    uiLbIp = stOspfDcn.ulDeviceIp;

    ospf_dcn_lldp_init();
    
	ospf_dcn_get_data();

    ospf_dcn_lldp_init();

	ospfGetApi(OSPF_DCN_PROCESS,OSPF_GBL_ADMIN,&ulValue);
	
	if(ulValue != ENABLE)
	{
		ulValue = ENABLE; 
		
		if(ospfSetApi(OSPF_DCN_PROCESS,OSPF_GBL_ADMIN ,&ulValue)!=OK)
		{
			ospf_logx(ospf_debug_dcn," Failed to ENABLE ospf :%d \r\n",__LINE__);
			OSPF_DCN_LOG_WARNING(" Failed to ENABLE ospf.");
		}
	}

	if(ospfSetApi(OSPF_DCN_PROCESS,OSPF_GBL_VRID,&uiVpnId)!=OK)
	{
		ospf_logx(ospf_debug_dcn," %%Failed to set vrid:%d \r\n",__LINE__);
        OSPF_DCN_LOG_WARNING("Failed to set vrid.");
	}

    ulValue = ENABLE;

	if(ospfSetApi(OSPF_DCN_PROCESS,OSPF_GBL_ROUTERID,&uiRouteId)!=OK)
	{
		ospf_logx(ospf_debug_dcn," %%Failed to disable ospf:%d \r\n",__LINE__);
        OSPF_DCN_LOG_WARNING("Failed to disable ospf.");
	}

    ospf_dcn_process_default_cfg();
    
	//ospf_dcn_initSuptAddrList();
    ospf_dcn_set_bandwidth(DCN_BANDWIDTH_DEF);    /*更新带宽*/

	ospf_dcn_if_set(OSPF_DCN_PROCESS,uiLbIp);
    
    #if 0
    for(ulLagId=1;ulLagId <= LAG_GROUP_NUM;ulLagId++)
    {
        if(VOS_ERR == if_vlag_logic_port_subid_to_index(ulLagId,0,&uiIfindex))
        {
            continue;
        }
     
    	ospf_dcn_create_lag_port(ulLbIp,uiIfindex);
    }
    #endif
    
    //arp_refresh_by_lldp();
    
	return iRtv;
}

int ospf_dcn_delete()
{
	u_int ulValue=0,state = 0;
	u_int ulLbIndex = OSPF_DCNLOOPBACK_IFINDEX,ulLbIfIndex = 0,ulIfIndx = 0;
	char szCmd[256] = {0};
	struct prefix stPrefx;
	char buf[INET6_ADDRSTRLEN] = {0};
	int ret = 0;
	u_char ucIndex = 0;
	memset(&stPrefx,0,sizeof(stPrefx));
  
    ret =ospf_dcn_get_globale_enable();
	if(ret != OSPF_DCN_ENABLE)
    {
        return OK;
    }   


	ret = ospfGetApi(OSPF_DCN_PROCESS,OSPF_GBL_ADMIN,&ulValue);
	if((ret == OK)&&(ulValue!=ENABLE))
	{
        return OK;
	}

       lldpd_set_api(LLDPD_DCN_UNDO, NULL, NULL);

	ulValue=DISABLE; 
	
	if(ospfSetApi(OSPF_DCN_PROCESS,OSPF_GBL_ADMIN ,&ulValue)!=OK)
	{
		ospf_logx(ospf_debug_dcn," %%Failed to disable ospf:%d \r\n",__LINE__);
		OSPF_DCN_LOG_WARNING("Failed to disable ospf.");
		return ERR;
	}
    vos_pthread_delay(300);//100-1S

#if 0     
   /*删除lpbk 31地址，所有dcn子接口恢复至默认vlan 4094，创建但不安装ip地址,删除dcn network*/
	ret = ospf_dcn_vlan_to_default();
	if(ret != OK)
	{
        zlog_info(MTYPE_OSPF,"ospf_dcn_vlan_to_def err!\n");
        return ERR;
	}
#endif
    ospf_dcn_interface_delete();

    vos_pthread_delay(100);
    
    ospf_dcn_vpn_delete();

    if(dcn_vlan_used_num(stOspfDcn.uiVlanId) == 0)
    {
        ospf_dcn_vlan_delete(stOspfDcn.uiVlanId);
    }
    else
    {
        ospf_logx(ospf_debug_dcn,"dcn do not delete vlan %d because the vlan is used\r\n",stOspfDcn.uiVlanId);
    }

    /*清空DCN数据,并将dcn ip恢复为默认值*/
	ospf_dcn_delete_data();

    memset(&stOspfLsaManage, 0, sizeof(stOspfLsaManage));   /*清空路由引入数据*/

    //ospf_dcn_delete_acl();

	return ret;	
}

/*uiLogicPort:逻辑端口号，uiWidth带宽值，uiVlan:vlan值，uiMode:带宽规则添加或删除*/
int ospf_dcn_set_eth_bandwidth(u_int uiLogicPort, u_int uiWidth, u_int uiVlan, u_int uiMode)
{
#ifdef HW
	int ret = 0,iRuleid = 0;
	int i;
	u_int ulPhyPort = 0,uiVlanId = 0;
	SDK_ACL_NEW_T stAcl;
	u_long ulIfIndx;
    
    MemZero(&stAcl, sizeof(stAcl));

    return OK;  /*该函数先注掉*/

    if ((MIN_VLAN_ID > uiVlan) || (MAX_VLAN_ID < uiVlan))
    {
        return ERR;
    }
  //  printf("ospf_dcn_set_eth_bandwidth uiLogicPort = %d,uiWidth = %d,uiVlan = %d,uiMode = %d\n",
   //     uiLogicPort,uiWidth,uiVlan,uiMode);

    memset(&stAcl,0,sizeof(SDK_ACL_NEW_T));
	ulPhyPort = uspPortLogicPortToPhyPort(uiLogicPort);

	iRuleid = uspSdkAclGetRule(ulPhyPort);
//	printf("ospf_dcn_set_eth_bandwidth uiLogicPort = %d,rule id:%d\n",uiLogicPort, iRuleid);
	if (iRuleid != ERR)
	{
	    /*创建规则*/
        if(DCN_BAND_CREAT == uiMode)
        {
    		stAcl.ruleId = iRuleid;
    		stAcl.type = SDK_ACL_GROUP_PRO_IN;
    		//stAcl.inport = ulPhyPort;
   		    stAcl.inport_wild = 0xffff;
            stAcl.s_vlan = uiVlan;
    		stAcl.s_vlan_wild= 0xfff;
    	//	stAcl.ip_protocol_type = 0x59;
    		stAcl.ip_protocol_type_wild = 0xff;

            ret = uspSdkAclSetApi(0, ulPhyPort, SDK_SET_ACL_RULE_ADD, &stAcl);	

    		stAcl.car.cir = uiWidth;
    	    stAcl.car.eir = uiWidth;
    	    stAcl.car.mode = SDK_ACL_CAR_BLIND;
            stAcl.car.cbs = 1;//暂时任意配置
            stAcl.car.ebs = 1;//暂时任意配置
            
		    ret |= uspSdkAclSetApi(0, ulPhyPort, SDK_SET_CAR_RULE_ADD, &stAcl);//限速配置
          //  printf("ospf_dcn_set_eth_bandwidth DCN_BAND_CREAT :ret = %d\n",ret);
        }
        else
        {
            ret = uspSdkAclSetApi(0, ulPhyPort, SDK_SET_ACL_RULE_DEL, &iRuleid);	
		    ret |= uspSdkAclSetApi(0, ulPhyPort, SDK_SET_CAR_RULE_DEL, &iRuleid);//限速配置

          //  printf("ospf_dcn_set_eth_bandwidth DCN_BAND_DEL :ret = %d\n",ret);

        }
	}
#endif
	return OK;	
}

u_int ospf_dcn_modify_all_eth_bandwidth(u_int uiWidth)
{
	u_int ulIfIndx = 0,uiOldBand = 0,uiVlan = 0;
	int ret = 0,i;
    
    ret = ospf_dcn_get_bandwidth(&uiOldBand);
    if(ret == ERR)
    {
        return ERR;
    }
    
    if(uiOldBand == uiWidth) 
    {
        return OK;
    }
    ospf_dcn_vlan_get(&uiVlan);
    for(i = 0; i < MAX_PORT_NUM; i++)
    {
        if(VOS_OK != if_logic_port_to_index(i, &ulIfIndx))
        {
            continue;
        }
        
		ret = ospf_dcn_get_port_enable_status(ulIfIndx);//接口
		if((ret !=  OSPF_DCN_PORT_DISABLE)&&(ret !=  ERR))
        {

            ret = ospf_dcn_set_eth_bandwidth(i, uiOldBand,uiVlan,DCN_BAND_DEL);
            ret |= ospf_dcn_set_eth_bandwidth(i, uiWidth,uiVlan,DCN_BAND_CREAT);
    		if (ERR == ret)
    		{
                ospf_logx(ospf_debug_dcn,"ospf_dcn_modify_all_eth_bandwidth:failed to configure EthWidth:%d \r\n",__LINE__);
                OSPF_DCN_LOG_WARNING("ospf_dcn_modify_all_eth_bandwidth:failed to configure EthWidth.");
    			continue;
    		}
        }
    }
    ret = ospf_dcn_set_bandwidth(uiWidth);
    //printf("ospf_dcn_modify_all_eth_bandwidth uiOldBand = %d,uiWidth = %d,ret = %d\n",uiOldBand,uiWidth,ret);

	return OK;
}


int ospf_dcn_get_data(void)
{
	int ret = 0;
	u_int ulLen = 0;

	//ulLen = sizeof(stDcnCfg)- sizeof(u_char)*3;
	/*防止后期维护添加变量，长度实时求取*/
	ulLen = sizeof(stDcnCfg) - ((char *)&stDcnCfg.uiNeid - (char *)&stDcnCfg.ucDcnEnflag);
	ospf_dcn_set_globale_enable((u_char)OSPF_DCN_ENABLE);
	ospf_dcn_set_init_flag(1);
	stOspfDcn.ucDcnEnflag = OSPF_DCN_ENABLE;

}
int ospf_dcn_change_data(u_long ulIpAddr,u_long ulIpMask)
{
	int ret = OK;

	if(ulIpAddr != stOspfDcn.ulDeviceIp)
	{
		stOspfDcn.ulDeviceIp = ulIpAddr;
//		ret = dev_hw_set_ip_env(ulIpAddr,ENV_NAME_DCN_IP_ADDR);
	}
	if(ulIpMask != stOspfDcn.ulDeviceMask)
	{
		stOspfDcn.ulDeviceMask = ulIpMask;
	}
	return ret;
}

int ospf_dcn_delete_data()
{
	int ret = OK;
	u_char aucMac[MAC_ADDR_LEN] = {0};
	u_int uiLoIp = OSPF_DCNDEF_IPADDR;
	u_int uiLbIp = 0;
	u_int uiNeId = 0;

	uiNeId = stOspfDcn.uiNeid;
	
#if 0
	dcn_set_api(DCN_NE_DEVICE_MAC,aucMac);
    //ulLoIp += (aucMac[3]<<16)+(aucMac[4]<<8)+aucMac[5];
    (0 == aucMac[5])?(ulLoIp += (aucMac[3]<<16)+(aucMac[4]<<8)+aucMac[5]+1):(ulLoIp += (aucMac[3]<<16)+(aucMac[4]<<8)+aucMac[5]);

	ulLbIp = ulLoIp;		
//	ret = dev_hw_set_ip_env(ulLbIp,ENV_NAME_DCN_IP_ADDR);
#endif
    ospf_dcn_global_init();
    
    stOspfDcn.uiNeid = uiNeId;
	stOspfDcn.ulDeviceIp = OSPF_DCNDEF_IPADDR + uiNeId;
	
	return ret;
}

int ospf_dcn_init_data(void)
{
	int ret = 0;
	

	memset(&stDcnCfg, 0, sizeof(stDcnCfg));
	ret = ospf_dcn_set_default();	
	
	stOspfDcn.uiVlanId = DCN_VLAN_DEF;	//配置默认vlan

	ospf_dcn_Intf_hw_init();
	
    /*TODO:设置组播组成员的最大个数*/

	return ret;
}

int ospf_DcnLoginInterface(u_int uiLpbkId)
{
	int ret = 0;
	u_char ucIndex = 0;
	
	ret = ospf_dcn_get_globale_enable();
	if((ret == OSPF_DCN_ENABLE)&&(uiLpbkId == OSPF_DCNLOOPBACK_IFINDEX))
	{
		return ERR;//若接口已被DCN使用则无法进入子接口
	}	

	return OK;
}
int ospf_dcn_set_default()
{
	int ret = 0;


	ret = ospf_dcn_get_api(DCN_NE_ID,&stOspfDcn.uiNeid);
	ret |= ospf_dcn_get_api(DCN_NE_DEVICE_TYPE,stOspfDcn.ucDevType);
	ret |= ospf_dcn_get_api(DCN_NE_COMPANY_NAME,stOspfDcn.ucDevName);
	ret |= ospf_dcn_get_api(DCN_NE_DEVICE_MAC,stOspfDcn.ucDevMac);
	ret |= ospf_dcn_get_api(DCN_NE_TYPE,&stOspfDcn.uiNeType);
	ret |= ospf_dcn_get_api(DCN_NE_IP,&stOspfDcn.ulDeviceIp);
	ret |= ospf_dcn_get_api(DCN_NE_IP_MASK,&stOspfDcn.ulDeviceMask);
	ret |= ospf_dcn_get_api(DCN_NE_PORT,&stOspfDcn.uiNePort);

	return ret;
}


int ospf_dcn_get_api(u_int uiCmd,void *pValue)
{
	if(pValue == NULL)
	{
		return ERR;
	}

	int ret = OK;
	u_char ucbuf[64]={0},ucIpAddr[4] = {0};
	octetstring octetStat;
	u_long *pVal = (u_long *)pValue,ulData = 0;
	octetStat.pucBuf = ucbuf;
	octetStat.len = sizeof(ucbuf);
	u_char aucMac[MAC_ADDR_LEN] = {0};
	u_long ulLoIp = OSPF_DCNDEF_IPADDR; /*默认ip地址为134.mac[5].0.1*/
	u_long ulLbIp = 0;
    int iSlot = 0;

    iSlot = uspGetSlot();
	switch(uiCmd)
	{
	        case DCN_NE_ID:
		{
            ret = uspGetApi(iSlot, SYS_API_BASE_DEVICE_ID, pValue);
			break;
		}
	        case DCN_NE_DEVICE_TYPE:
		{
        
            ret = uspGetApi(0, SYS_API_GBL_CARD_NAME, &octetStat);
            if(OK == ret)
            {
                memcpy(pValue,ucbuf,strlen(ucbuf));
            }
			break;
		}
	        case DCN_NE_COMPANY_NAME:
		{
			ret = uspGetApi(iSlot, SYS_API_BASE_PROVIDER, &octetStat);//设备厂家
			if(OK == ret)
			{
				memcpy(pValue,octetStat.pucBuf,octetStat.len);
			}
			break;
		}
	        case DCN_NE_DEVICE_MAC:
		{
			ret = uspGetApi(iSlot, SYS_API_BASE_MAC_ADDR, &octetStat);//MAC
			if(OK == ret)
			{
				memcpy(pValue,octetStat.pucBuf,octetStat.len);
			}
			break;
		}
	        case DCN_NE_TYPE:
		{
			*pVal = AF_INET;
			break;
		}
	        case DCN_NE_IP:
		{
		//	ret = dev_hw_get_ip_env(&ulLbIp,ENV_NAME_DCN_IP_ADDR);
			//if((ERR == ret)||(ulLbIp == 0)||(ulLbIp == OSPF_DCNDEF_IPMASK))
			if(ulLbIp == 0)
			{
                memcpy(aucMac,stOspfDcn.ucDevMac,6);
				(0 == aucMac[5])?(ulLoIp += (aucMac[3]<<16)+(aucMac[4]<<8)+aucMac[5]+1):(ulLoIp += (aucMac[3]<<16)+(aucMac[4]<<8)+aucMac[5]);
				
				ulLbIp = ulLoIp;		
		//		ret = dev_hw_set_ip_env(ulLbIp,ENV_NAME_DCN_IP_ADDR);
			}
			*pVal = ulLbIp;
			ret = OK;
			break;
		}
	        case DCN_NE_IP_MASK:
		{
			*pVal = OSPF_DCNDEF_IPMASK;
			break;
		}
	        case DCN_NE_PORT:
		{
			break;
		}
		default:
		{
			ret = ERR;
			break;
		}
	}
	
	return ret;
}



#ifdef OSPF_DCN
u_int
ospf_dcn_rxmt_cmp(struct ospf_dcn_rxmt_node *p1,
                  struct ospf_dcn_rxmt_node *p2)
{

    struct ospf_lshdr *h1 = &p1->lshdr;
    struct ospf_lshdr *h2 = &p2->lshdr;
        
    OSPF_KEY_CMP(h1, h2, type);
    /*compare id and router with host order*/
    OSPF_KEY_HOST_CMP(h1, h2, id);
    OSPF_KEY_HOST_CMP(h1, h2, adv_id);

    return 0;
}

struct ospf_dcn_rxmt_node*
ospf_dcn_rxmt_add(struct ospf_lsa *p_lsa)
{
    struct ospf_dcn_rxmt_node *p_msg;

    p_msg = ospf_dcn_rxmt_lookup(p_lsa->p_lstable->p_process, p_lsa->lshdr);
    if (NULL == p_msg)
    {
    p_msg = ospf_malloc2(OSPF_MDCN);
        
    }
    if (NULL == p_msg)
    {
        return NULL;
    }
    if (OSPF_MAX_LSAGE == ntohs(p_lsa->lshdr->age))
    {
        p_msg->rtm_type = RTM_DCN_DEL;
    }
    else
    {
        p_msg->rtm_type = RTM_DCN_ADD;
    }
   
    memcpy(&p_msg->lshdr, p_lsa->lshdr, sizeof(struct ospf_lshdr));

    memcpy(p_msg->body, (uint8_t*)(p_lsa->lshdr + 1), ntohs(p_lsa->lshdr->len) - OSPF_LSA_HLEN);

    ospf_lstadd(&p_lsa->p_lstable->p_process->dcn_rtm_tx_table, p_msg);

    //ospf_timer_start(&p_lsa->p_lstable->p_process->dcn_rtm_tx_timer, 1);

    return p_msg;
}

void
ospf_dcn_rxmt_del(struct ospf_process *p_process,
                  struct ospf_dcn_rxmt_node *p_msg)
{  

    ospf_lstdel(&p_process->dcn_rtm_tx_table, p_msg);

    ospf_mfree(p_msg, OSPF_MDCN);

    return;
}
struct ospf_dcn_rxmt_node *
ospf_dcn_rxmt_lookup(struct ospf_process *p_process,
                     struct ospf_lshdr *p_hdr)
{
    struct ospf_dcn_rxmt_node key;

    memcpy(&key.lshdr, p_hdr, sizeof(struct ospf_lshdr));

    return ospf_lstlookup(&p_process->dcn_rtm_tx_table, &key);
}

/*
	|rtm_msglen        |rtm_version|rtm_type |
	------------------------------------------
	|    len              |    TLV            |  ---len:lsa body len,include padding len
    ------------------------------------------
    |               TLV....                   |
    -------------------------------------------
    |......         len    ...TLV             | 
    --------------------------------------------
    |    TLV....

--can fill more than one lsa,now only fill one lsa*/
void
ospf_rtmsg_dcn_lsa_send(struct ospf_process *p_process)
{
    struct ospf_dcn_rxmt_node *p_node = NULL;
    struct ospf_dcn_rxmt_node *p_next_node = NULL;
  //  struct rtmsg_generalhdr *p_rtmsg = NULL;
    uint8_t buf[1024] = {0};
    uint8_t *p_data = buf;
    uint16_t len;
    uint32_t s = ospf.rtsock;

    ospf_logx(ospf_debug_dcn,"ospf dcn lsa notify send");   

    for_each_node(&p_process->dcn_rtm_tx_table, p_node, p_next_node)
    {

        len = ntohs(p_node->lshdr.len) - OSPF_LSA_HLEN ;

        *(uint16_t*)p_data = htons(len);
        /*fill lsa body*/
        p_data += 2;
        memcpy(p_data, p_node->body, len);
  
    }   
        ospf_logx(ospf_debug_dcn,"ospf dcn lsa notify send end");   

}
void
ospf_display_dcn(struct vty *vty)    
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
    struct ospf_dcn_rxmt_node *p_node = NULL;
    struct ospf_dcn_rxmt_node *p_next_node = NULL;
	char acLinkId[32] = {0};
	char acRouterId[32] = {0};
	u_long ulLinkId = 0;
	u_long ulAdvId = 0;
	
   if (ospf_semtake_timeout() == ERR)
    {
        ospf_logx(ospf_debug_dcn," OSPF is busy now,call this command late");    
        return  ;      
    }   
    for_each_node(&ospf.process_table, p_process, p_next_process)
    {
        vty_out(vty, "-------------------------process: %d--------------------------%s", p_process->process_id,VTY_NEWLINE);
        
        for_each_node(&p_process->dcn_rtm_tx_table, p_node, p_next_node)
        {
            memset(acLinkId, 0, sizeof(acLinkId));
            memset(acRouterId, 0, sizeof(acRouterId));
            ulLinkId = ntohl(p_node->lshdr.id);
            ulAdvId = ntohl(p_node->lshdr.adv_id);
            ospf_inet_ntoa(acLinkId,ulLinkId);
            ospf_inet_ntoa(acRouterId,ulAdvId);
            vty_out(vty,"%s\tfor each node %x, rtm type=%d,%s/%s/%d %s", VTY_NEWLINE,p_node, p_node->rtm_type,
                acRouterId, acLinkId, p_node->lshdr.type,VTY_NEWLINE);  
        }

    }
    vty_out(vty, "%s",VTY_NEWLINE);
    ospf_semgive();
    return ;
}
#endif
void ospf_dcn_if_neid_update(u_int uiNeid)
{
#ifdef OSPF_DCN
	tOSPF_NETWORK_INDEX network_index;
	uint32_t offset = 0;
	uint32_t count = 0;
	uint8_t buf[512] = {0};
    octetstring octetStat;
    char szcDevStr[64] = {0};
	u_int ulid = 0,ulIp = 0;
	struct ospf_process *p_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    u_short usVarLen = 0x20;

    octetStat.pucBuf = szcDevStr;

	/*
	厂家标示：	Type：0x8000； Length：标示名称长度
	设备型号：	Type：0x8001； Length：型号名称长度
	设备桥MAC： 	Type：0x8002； Length：6
	NEID:			Type：0x8003； Length：4
	NEIP IPV4： 	Type：0x8004； Length：4
	NEIP IPV6： 	Type：0x8005； Length：16
	*/

	memset(buf,0,512);
	offset = 0;

    memset(szcDevStr, 0, sizeof(szcDevStr));
	
	memcpy(szcDevStr,stOspfDcn.ucDevName,128);
	/*provider*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x00;
    /*长度四字节对其*/
    octetStat.len = usVarLen;
    //octetStat.len = ((octetStat.len+3)/4)*4;
    
	buf[offset++] = (octetStat.len& 0xff00)>>8;
	buf[offset++] = octetStat.len & 0xff;
	//memcpy(buf+offset,octetStat.pBuf,octetStat.len);
	memcpy(buf+offset,szcDevStr,32);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] pvoider %s len %d", \
		buf+offset,octetStat.len);
	offset += 32;//strlen(szcDevStr);

    memset(szcDevStr, 0, sizeof(szcDevStr));
	memcpy(szcDevStr, stOspfDcn.ucDevType,128);

	/*sys descipter*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x01;

    /*长度四字节对其*/
    octetStat.len = usVarLen;
    //octetStat.len = ((octetStat.len+3)/4)*4;
    
	buf[offset++] = (octetStat.len& 0xff00)>>8;
	buf[offset++] = octetStat.len & 0xff;
	
	memcpy(buf+offset,szcDevStr,32);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] device %s len %d", \
		buf+offset,octetStat.len);
	offset += 32;//strlen(szcDevStr);

    memset(szcDevStr, 0, sizeof(szcDevStr));
    memcpy(szcDevStr, stOspfDcn.ucDevMac,6);

	/*sys mac*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x02;

    /*长度四字节对其*/
    octetStat.len = ((6+3)/4)*4;
    
	buf[offset++] = (octetStat.len& 0xff00)>>8;
	buf[offset++] = octetStat.len & 0xff;
	memcpy(buf+offset,szcDevStr,6);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] mac %02x:%02x:%02x:%02x:%02x:%02x len %d", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3],buf[offset+4],buf[offset+5],6);
	offset += octetStat.len;

	//dcn_set_api((void *)&ulid,DCN_NE_ID);
    ulid = uiNeid;
	/*neid*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x03;
	buf[offset++] = 0;
	buf[offset++] = 4;
  	ulid = htonl(ulid);
	memcpy(buf+offset,&ulid,4);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] neid %d.%d.%d.%d len 4", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3]);
	offset += 4;

  //  dev_sys_ip_get_api(1,SYS_API_IP_ADDR ,(void *)&ulid);
  	//ulid = htonl(ulIp);
	ulIp = stOspfDcn.ulDeviceIp;
  	ulid = htonl(ulIp);

	/*neip*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x04;
	buf[offset++] = 0;
	buf[offset++] = 4;
	memcpy(buf+offset,&ulid,4);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] neip %d.%d.%d.%d len 4", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3]);
	offset += 4;

	/*neip*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x05;
	buf[offset++] = 0;
	buf[offset++] = 16;
//	memse(buf+offset,&ulid,4);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] neip %d.%d.%d.%d len 4", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3]);
	offset += 16;
	
	octetStat.pucBuf = buf;
	octetStat.len  = offset;

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] deliver dcn lsa to ospf,total len %d",offset);

    p_process = ospf_process_lookup(&ospf, OSPF_DCN_PROCESS);


	if(NULL != p_process)
	{
		for_each_ospf_if(p_process, p_if, p_next_if)
	    {

	        if (ulIp == p_if->addr)
	        {
	        	p_if->ulDcnflag = OSPF_DCN_FLAG;

	        }
	    }
	}
	p_process->uiDcnNeidCnt++;
	if(ospfSetApi(OSPF_DCN_PROCESS,OSPF_GBL_DCNLSA,&octetStat)!=OK)
	{
	    p_process->uiDcnNeidErrCnt++;
		ospf_logx(ospf_debug_dcn,"\n\r [dcn] set ospf dcn lsa failed");
		OSPF_DCN_LOG_WARNING("Failed to set ospf dcn lsa.");
	}
	
#endif

}
void ospf_dcn_if_set(u_int ulPrid,u_int ulIp)
{
#ifdef OSPF_DCN
	tOSPF_NETWORK_INDEX network_index;
	uint32_t offset = 0;
	uint32_t count = 0;
	uint8_t buf[512] = {0};
    octetstring octetStat;
    char szcDevStr[64] = {0};
	u_int ulid = 0;
	struct ospf_process *p_process = NULL;
    struct ospf_if *p_if = NULL;
    struct ospf_if *p_next_if = NULL;
    u_short usVarLen = 0x20;

    OSPF_LOG_WARN("%d ulPrid %d ulIp 0x%x\n",__LINE__, ulPrid, ulIp);

    octetStat.pucBuf = szcDevStr;

	/*
	厂家标示：	Type：0x8000； Length：标示名称长度
	设备型号：	Type：0x8001； Length：型号名称长度
	设备桥MAC： 	Type：0x8002； Length：6
	NEID:			Type：0x8003； Length：4
	NEIP IPV4： 	Type：0x8004； Length：4
	NEIP IPV6： 	Type：0x8005； Length：16
	*/

	memset(buf,0,sizeof(buf));
	offset = 0;

    memset(szcDevStr, 0, sizeof(szcDevStr));
	memcpy(szcDevStr, stOspfDcn.ucDevName, 128);
	/*provider*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x00;
    /*长度四字节对其*/
    octetStat.len = usVarLen;
    //octetStat.len = ((octetStat.len+3)/4)*4;
    
	buf[offset++] = (octetStat.len& 0xff00)>>8;
	buf[offset++] = octetStat.len & 0xff;
	//memcpy(buf+offset,octetStat.pBuf,octetStat.len);
	memcpy(buf+offset,szcDevStr,32);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] pvoider %s len %d", \
		buf+offset,octetStat.len);
	offset += 32;//strlen(szcDevStr);

    memset(szcDevStr, 0, sizeof(szcDevStr));
    memcpy(szcDevStr, stOspfDcn.ucDevType, 128);

	/*sys descipter*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x01;

    /*长度四字节对其*/
    octetStat.len = usVarLen;
    //octetStat.len = ((octetStat.len+3)/4)*4;
    
	buf[offset++] = (octetStat.len& 0xff00)>>8;
	buf[offset++] = octetStat.len & 0xff;
	
	memcpy(buf+offset,szcDevStr,32);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] device %s len %d", \
		buf+offset,octetStat.len);
	offset += 32;//strlen(szcDevStr);

    memset(szcDevStr, 0, sizeof(szcDevStr));
    memcpy(szcDevStr, stOspfDcn.ucDevMac, 6);

	/*sys mac*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x02;

    /*长度四字节对其*/
    octetStat.len = ((6+3)/4)*4;
    
	buf[offset++] = (octetStat.len& 0xff00)>>8;
	buf[offset++] = octetStat.len & 0xff;
	memcpy(buf+offset,szcDevStr,6);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] mac %02x:%02x:%02x:%02x:%02x:%02x len %d", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3],buf[offset+4],buf[offset+5],6);
	offset += octetStat.len;

    ulid = stOspfDcn.uiNeid;

	/*neid*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x03;
	buf[offset++] = 0;
	buf[offset++] = 4;
  	ulid = htonl(ulid);
	memcpy(buf+offset,&ulid,4);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] neid %d.%d.%d.%d len 4", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3]);
	offset += 4;

  //  dev_sys_ip_get_api(1,SYS_API_IP_ADDR ,(void *)&ulid);
  	ulid = htonl(ulIp);

	/*neip*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x04;
	buf[offset++] = 0;
	buf[offset++] = 4;
	memcpy(buf+offset,&ulid,4);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] neip %d.%d.%d.%d len 4", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3]);
	offset += 4;

	/*neip*/
	buf[offset++] = 0x80;
	buf[offset++] = 0x05;
	buf[offset++] = 0;
	buf[offset++] = 16;
//	memse(buf+offset,&ulid,4);

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] neip %d.%d.%d.%d len 4", \
		buf[offset],buf[offset+1],buf[offset+2],buf[offset+3]);
	offset += 16;
	
	octetStat.pucBuf = buf;
	octetStat.len  = offset;

	ospf_logx(ospf_debug_dcn,"\n\r [dcn] deliver dcn lsa to ospf,total len %d",offset);
    #if 0
    p_process = ospf_process_lookup(&ospf, ulPrid);

	if(NULL != p_process)
	{
		for_each_ospf_if(p_process, p_if, p_next_if)
	    {

	        if (ulIp == p_if->addr)
	        {
	        	p_if->ulDcnflag = OSPF_DCN_FLAG;

	        }
	    }
	}
    #endif
	//p_process->uiDcnIfCnt++;
	if(ospfSetApi(ulPrid,OSPF_GBL_DCNLSA,&octetStat)!=OK)
	{
	    //p_process->uiDcnIfErrCnt++;
        OSPF_LOG_WARN("%d ulPrid %d ulIp 0x%x\n",__LINE__, ulPrid, ulIp);
		ospf_logx(ospf_debug_dcn,"\n\r [dcn] set ospf dcn lsa failed");
        OSPF_DCN_LOG_WARNING("Failed to set dcn lsa,ulPrid:%d ulIp:%x", ulPrid, ulIp);
	}
#endif

}

void ospf_dcn_print(u_int8 *ucbuf,u_int len)
{
	int i = 0;
	
	if(ucbuf == NULL)
	{
		return;
	}
	
	vty_out_to_all_terminal("========ospf_dcn_print=============");
	for(i = 0;i<len;i++)
	{
		vty_out_to_all_terminal("0x%x  ",ucbuf[i]);
		if((i+1)%16 == 0)
		{
			vty_out_to_all_terminal("\n");
		}
	}
	vty_out_to_all_terminal("\n");
}


void ospf_dcn_get_type_10(struct ospf_dcn_rxmt_node *p_node,OSPF_DCN_NBR_T *pstNbr)
{
	u_int sub_type = 0;
	u_int sub_len = 0;
	u_int sub_tem = 0;
	u_char  ucbuf[256] = {0};
	u_char  ucbuftem[256] = {0};
	u_int dcn_offset = 0;
	u_int offset = 256;
	u_int dcn_len = 256;
	OSPF_DCN_NBR_T stDcnNbr;
	u_long ulOffset = 0,len = 0;
	if(NULL == p_node)
	{
		return;
	}
	memset(&stDcnNbr,0,sizeof(OSPF_DCN_NBR_T));
	memcpy(ucbuf,p_node->body,256);
	
	while(dcn_offset < offset)
	{

		sub_type = (ucbuf[dcn_offset]<<8) + ucbuf[dcn_offset+1];
		dcn_offset +=2;
		sub_len = (ucbuf[dcn_offset]<<8) + ucbuf[dcn_offset+1];
		dcn_offset +=2;
		
		memset(ucbuftem,0,sizeof(ucbuftem));
		memcpy(ucbuftem,&ucbuf[dcn_offset],sub_len);
		sub_tem = ((sub_len+3)/4)*4;
		dcn_offset += sub_tem;

		switch(sub_type)
		{
			case 0x8000:
				ospf_logx(ospf_debug_dcn,"\n\r [dcn] provider: %s", ucbuftem);
				memcpy(stDcnNbr.ucCompany,ucbuftem,sub_len);
				break;
			case 0x8001:
				ospf_logx(ospf_debug_dcn,"\n\r [dcn] device: %s", ucbuftem);
				memcpy(stDcnNbr.ucDevType,ucbuftem,sub_len);
				break;
			case 0x8002:
				ospf_logx(ospf_debug_dcn,"\n\r [dcn] mac: %02x:%02x:%02x:%02x:%02x:%02x",    
					ucbuftem[0],ucbuftem[1],ucbuftem[2],ucbuftem[3],ucbuftem[4],ucbuftem[5]);
				memcpy(stDcnNbr.ucDevMac,ucbuftem,6);
				break;
			case 0x8003:
				ospf_logx(ospf_debug_dcn,"\n\r [dcn] neid: %d.%d.%d.%d",
					ucbuftem[0],ucbuftem[1],ucbuftem[2],ucbuftem[3]);
				memcpy(&stDcnNbr.uiNeid,ucbuftem,sub_len);
				break;
			case 0x8004:
				ospf_logx(ospf_debug_dcn,"\n\r [dcn] neip: %d.%d.%d.%d",
					ucbuftem[0],ucbuftem[1],ucbuftem[2],ucbuftem[3]);
				memcpy(&stDcnNbr.ulNeIp,ucbuftem,sub_len);
				break;
			case 0x8005:
				memcpy(stDcnNbr.ulNeIpv6,ucbuftem,sub_len);
				break;
			default:
				//dcn_offset = dcn_len;
				break;
		}
		
		
	}
	memcpy(pstNbr,&stDcnNbr,sizeof(OSPF_DCN_NBR_T));
}


void ospf_dcn_display_nbr(struct vty *vty)    
{
	int flag = 1,rc  = OK,ret = 0;
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next_process = NULL;
	struct ospf_dcn_rxmt_node *p_node = NULL;
	struct ospf_dcn_rxmt_node *p_next_node = NULL;
	u_char  ucbuf[256] = {0},ucIpAddr[16] = {0},ucNeId[16] = {0},ucMac[20] = {0};
	OSPF_DCN_NBR_T stNbr;
	

	vty_out(vty,"Network Element Info %s",VTY_NEWLINE);	
	vty_out(vty," %-16s%-16s%-20s%-20s%-20s%-16s%s",
	"NEID","NEIP","MAC ADDRESS","DEVICETYPE","COMPANY","NEIPv6",VTY_NEWLINE);	
	
	for_each_node(&ospf.process_table, p_process, p_next_process)
	{
	        for_each_node(&p_process->dcn_rtm_tx_table, p_node, p_next_node)
	        {
	        		if(p_node != NULL)
        			{
		        		ospf_dcn_get_type_10(p_node,&stNbr);
						//stNbr.ulNeIp = ntohl(stNbr.ulNeIp);
						inet_ntop(AF_INET,&stNbr.ulNeIp, ucIpAddr, sizeof(ucIpAddr));
						inet_ntop(AF_INET,&stNbr.uiNeid, ucNeId, sizeof(ucNeId));
						
						sprintf(ucMac,"%02x:%02x:%02x:%02x:%02x:%02x",    
							stNbr.ucDevMac[0],stNbr.ucDevMac[1],stNbr.ucDevMac[2],
							stNbr.ucDevMac[3],stNbr.ucDevMac[4],stNbr.ucDevMac[5]);
						vty_out(vty," %-16s%-16s%-20s%-20s%-20s%-16s%s", 
							ucNeId,ucIpAddr,ucMac,stNbr.ucDevType,stNbr.ucCompany,
							"--",VTY_NEWLINE);  
        			}

	        }
	}
}


void ospf_dcn_get_type_10_lsa_hdr(struct ospf_lshdr *p_lshdr,uint8_t* ubuf)
{
	struct ospf_process *p_process = NULL;
	struct ospf_process *p_next_process = NULL;
	struct ospf_dcn_rxmt_node *p_node = NULL;
	struct ospf_dcn_rxmt_node *p_next_node = NULL;

	for_each_node(&ospf.process_table, p_process, p_next_process)
	{
	        for_each_node(&p_process->dcn_rtm_tx_table, p_node, p_next_node)
	        {
			memcpy(p_lshdr,&p_node->lshdr,sizeof(p_node->lshdr));
			memcpy(ubuf,p_node->body,sizeof(p_node->body));
		}

	}

}


//设置子接口dcn使能状态
int ospf_dcn_set_port_enable(u_char ucSlot,u_char ucPort,u_char ucData)
{
	if((ucSlot >MAX_SLOT_NUM)||(ucPort >MAX_PORT_NUM))
	{
		return ERR;
	}
    if(NULL != eth_trunk_get_by_logic_port(ucPort))
    {
        return ERR;
    }
	stDcnCfg.uiUsePort[ucSlot][ucPort] = ucData;
    return OK;

}
#if 0
//加载子接口dcn使能状态
int ospf_dcn_set_port_enable(u_char ucSlot,u_char ucPort,u_char ucData)
{
	if((ucSlot >MAX_SLOT_NUM)||(ucPort >MAX_PORT_NUM))
	{
		return ERR;
	}
	stOspfDcn.uiUsePort[ucSlot][ucPort] = ucData;
    return OK;

}
#endif


//接口调用
int ospf_dcn_get_port_enable_status(u_int ulIfIndx)
{
	u_int uiSlot = 0,uiPort = 0, uiType = 0, uiInPort = 0;
	char buf[100] = {0};
	int iRtv = 0;

	uiSlot = IFINDEX_TO_SLOT(ulIfIndx);
	uiType = IFINDEX_TO_TYPE(ulIfIndx);
    uiInPort = IFINDEX_TO_INTF_PORT_NO(ulIfIndx);
    
	if(uiType != IFINDEX_TYPE_ETH_TRUNK)
	{
	    if_index_to_logic_port(ulIfIndx, &uiPort);
	}
	else
	{
	    uiPort = VPORT_IF_IFINDEX_TO_PORT(ulIfIndx);
	}

    
	if(stOspfDcn.ucDcnEnflag != OSPF_DCN_ENABLE)
	{
		return ERR;
	}	 

	if((uiType != IFINDEX_TYPE_ETH_TRUNK) && ((uiSlot > MAX_SLOT_NUM)||(uiPort > MAX_PORT_NUM)))
	{
		return ERR;
	}

    if((uiType == IFINDEX_TYPE_ETH_TRUNK) && (uiPort > LAG_GROUP_NUM || uiPort < 1))
    {
        return ERR;
    }

    if(uiType != IFINDEX_TYPE_ETH_TRUNK)
    {
        return stOspfDcn.uiUsePort[uiSlot-1][uiInPort-1];
    }
    else
    {
        return stOspfDcn.uiLagPort[uiPort-1];
    }
}



//设置子接口dcn使能状态
int ospf_dcn_set_overlay(u_char ucPort,u_long ulIpaddr,u_long ulMasklen,u_char ucMode)
{
    u_int ulIndx = 0;
    
	if(ucPort > MAX_PORT_NUM)
	{
		return ERR;
	}
	if(VOS_OK != if_logic_port_to_index(ucPort, &ulIndx))
	{
	    return ERR;
	}
    switch(ucMode)
    {
        case DCN_OVERLAY_MASTER_ADD:
        case DCN_OVERLAY_SLAVER_ADD:
            stDcnCfg.stOverlayCfg[ucPort].ucMngflag = ucMode;
            stDcnCfg.stOverlayCfg[ucPort].ulIfindex = ulIndx;
            stDcnCfg.stOverlayCfg[ucPort].ulAddr = ulIpaddr;
        	stDcnCfg.stOverlayCfg[ucPort].ulMasklen = ulMasklen;
            break;
        case DCN_OVERLAY_DEL:
            memset(&stDcnCfg.stOverlayCfg[ucPort],0,sizeof(OSPF_DCN_OVERLAY_CFG_T));
            break;
        default:
            return ERR;
    }
            
    return OK;

}

//接口调用
int ospf_dcn_get_overlay_port_config(u_char ucPort, OSPF_DCN_OVERLAY_CFG_T *pstOverCfg)
{
	char buf[100] = {0};
	int iRtv = 0;
    
    if(pstOverCfg == NULL)
    {
        return ERR;
    }

    iRtv = ospf_dcn_get_globale_enable();
    if(iRtv != OSPF_DCN_ENABLE)
    {
        return ERR;
    }	 

	if(ucPort > MAX_PORT_NUM)
	{
		return ERR;
	}
    if(stDcnCfg.stOverlayCfg[ucPort].ucMngflag == DCN_OVERLAY_MASTER_ADD)
    {
        memcpy(pstOverCfg,&stDcnCfg.stOverlayCfg[ucPort],sizeof(OSPF_DCN_OVERLAY_CFG_T));
        return OK;
    }
    return ERR;
}
#endif
/*查找dcn端口是否位于overlay中*/
int ospf_dcn_lookup_overlay_port(u_int uiPort)
{
#ifdef OSPF_DCN /*caoyong delete 2017.9.20*/
	char buf[100] = {0};
	int iRtv = 0,iflag = 0;

    iRtv = ospf_dcn_get_globale_enable();
    if(iRtv != OSPF_DCN_ENABLE)
    {
        return ERR;
    }	 
    iflag = stDcnCfg.stOverlayCfg[uiPort].ucMngflag;
    if((stDcnCfg.stOverlayCfg[uiPort].ulAddr != 0)
        &&((iflag == DCN_OVERLAY_MASTER_ADD)
        ||(iflag == DCN_OVERLAY_SLAVER_ADD)))
    {
        return OK;
    }
#endif
    return ERR;
}
#ifdef OSPF_DCN 
/*查找dcn端口是否位于overlay中,且为master*/
int ospf_dcn_lookup_master_port(u_int uiPort,u_int *puiMasIp,u_int *puiMasMasklen)
{
	char buf[100] = {0};
	int iRtv = 0,iflag = 0;

    iRtv = ospf_dcn_get_globale_enable();
    if(iRtv != OSPF_DCN_ENABLE)
    {
        return ERR;
    }	 
    iflag = stDcnCfg.stOverlayCfg[uiPort].ucMngflag;
    if((stDcnCfg.stOverlayCfg[uiPort].ulAddr != 0)
        &&(iflag == DCN_OVERLAY_MASTER_ADD))
    {
        *puiMasIp = stDcnCfg.stOverlayCfg[uiPort].ulAddr;
        *puiMasMasklen = stDcnCfg.stOverlayCfg[uiPort].ulMasklen;
        return OK;
    }
    return ERR;
}
#endif

/*查找dcn端口是否位于overlay中,且为slaver*/
int ospf_dcn_lookup_slave_port(u_int uiPort,u_int *puiSlaveIp,u_int *puiSlaveMasklen)
{
#ifdef OSPF_DCN
	char buf[100] = {0};
	int iRtv = 0,iflag = 0;

    iRtv = ospf_dcn_get_globale_enable();
    if(iRtv != OSPF_DCN_ENABLE)
    {
        return ERR;
    }	 
    iflag = stDcnCfg.stOverlayCfg[uiPort].ucMngflag;
    if((stDcnCfg.stOverlayCfg[uiPort].ulAddr != 0)
        &&(iflag == DCN_OVERLAY_SLAVER_ADD))
    {
        *puiSlaveIp = stDcnCfg.stOverlayCfg[uiPort].ulAddr;
        *puiSlaveMasklen = stDcnCfg.stOverlayCfg[uiPort].ulMasklen;
        return OK;
    }
    return ERR;
#endif
}
#ifdef OSPF_DCN
int ospf_dcn_get_port_enable_by_ifindex(u_long ulIfIndx)
{
	int iRet = 0;
	
	iRet = ospf_dcn_get_port_enable_status(ulIfIndx);//接口调用
	if(iRet == OSPF_DCN_PORT_ENABLE)
	{
		return OK;
	}
	else
	{
	    return ERR;
	}
}
#if 0
int ospf_dcn_check_by_ifindex(u_long ulIfIndx)
{
	int iRet = ERR,iValue = 0;
	
	iValue = ospf_dcn_get_port_enable_status(ulIfIndx);
	if(iValue == OSPF_DCN_PORT_ENABLE)
	{
		return OK;
	}
	return ERR;
}
#endif
int ospf_dcn_get_globale_enable(void)
{
	return stOspfDcn.ucDcnEnflag;
}
void ospf_dcn_set_globale_enable(u_char ucData)
{
	stOspfDcn.ucDcnEnflag = ucData;
}

int ospf_dcn_get_init(void)
{
	return stOspfDcn.ucInitflag;
}
int ospf_dcn_set_init_flag(u_char ucData)
{
	stOspfDcn.ucInitflag = ucData;

}






/*仅创建ospf网络*/
//uiIndx-接口索引,例如ge0/0/1:0x20001
int ospf_dcn_create_by_ifindex(u_int uiIndx)
{
	int iRtv = 0,iRet = 0,iExt = 0,iVlan = 0;
	u_long ulIfIndx = 0,ulValue = 0,ulLbIp = 0;
	u_char ucPortRole = 0;
	char szName[100] = {0};
	u_char ucSlot = 0,ucPort = 0,ucSubPort = OSPF_DCN_SUBPORT;
	struct prefix stPrefx;

	
	ulLbIp = stOspfDcn.ulDeviceIp;
	ucSlot = IFINDEX_TO_SLOT(uiIndx);
	
	if(VOS_ERR == if_index_to_logic_port(uiIndx, &ucPort))
    {
        return ERR;
    }
	if (ucSlot > MAX_SLOT_NUM)
	{	    
	    OSPF_DCN_LOG_WARNING("ospf_dcn_create_by_ifindex slot or lport is invalid.");
		return ERR;
	}
	
	iRtv = ospf_dcn_get_port_enable_status(uiIndx);//接口
	
	ospf_logx(ospf_debug_dcn,"ospf_dcn_create_by_ifindex:  uiIndx:%ld,ucPort:%d\n",uiIndx,ucPort);
	if((iRtv !=  OSPF_DCN_PORT_DISABLE)&&(iRtv !=  ERR))
	{
		iRet = ospf_dcn_delete_by_ifindex(ulLbIp,ucPort);
		ospf_dcn_set_port_enable(ucSlot,ucPort,OSPF_DCN_PORT_ENABLE);
        iRet = ospf_set_acl(uiIndx,OSPF_RULE_CREAT);   /*添加UNI、BRG端口ACL规则*/
    //    printf("ospf_dcn_create_by_ifindex OSPF_RULE_CREAT ucPort=%d,iRet=%d\n",ucPort,iRet);

    }
    else
    {
        iRet = ospf_set_acl(uiIndx,OSPF_RULE__DEL);   /*删除UNI、BRG端口ACL规则*/
     //   printf("ospf_dcn_create_by_ifindex OSPF_RULE__DEL ucPort=%d,iRet=%d\n",ucPort,iRet);
    }
    
	iRtv = ospf_dcn_get_port_enable_status(uiIndx);//接口
	
	if(iRtv != OSPF_DCN_PORT_ENABLE)
	{
		//获取接口名称
	
        iRet = if_name_get_by_index(uiIndx,szName);
		if(OK != iRet)
		{
			zlog_info(MTYPE_OSPF,"Failed to enable DCN %s.%d\n", szName,ucSubPort);
            OSPF_DCN_LOG_WARNING("Failed to enable DCN ifname=%s,ucSubPort=%d",szName,ucSubPort);
		}
        return iRet;

	}

	return OK;
}

int ospf_dcn_delete_by_ifindex(u_int ulIp,u_char ucPort)
{
	u_long ulValue=0;
	tOSPF_NETWORK_INDEX	stOspfNetWorkIndex;
	int iRtv = 0;
	octetstring stOct;
	u_char aucMac[MAC_ADDR_LEN] = {0};
	u_long ulLoIp = 0xc0a80301; /*默认ip地址为192.mac[5].3.1*/
	u_int ulNetWorkMask = 0;
	u_char ucIpAddr[4];
	u_long	ulMaskIp = 0;
	long	ldcnIp = ulIp;
	u_int ulns = OSPF_DCN_PROCESS;

	if(ulIp == 0)
	{
		zlog_info(MTYPE_OSPF,"ospf_dcn_delete_by_ifindex failed ,ulIp:%x\n", ulIp);
        OSPF_DCN_LOG_WARNING("ospf_dcn_delete_by_ifindex failed ,ulIp:%x",ulIp);
		return ERR;
	}

	ulNetWorkMask = OSPF_DCNDEF_IPMASK;
	stOspfNetWorkIndex.area = 0;
	stOspfNetWorkIndex.mask = ulNetWorkMask;
	stOspfNetWorkIndex.network = ulIp;
	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
	
	ospf_dcn_create_network(&stOspfNetWorkIndex,ucPort);
	return OK;

}


/*删除单个接口网络,ulIfIndx-接口索引，如131073*/
int ospf_dcn_delete_by_ifindex_ip(u_long ulIfIndx,u_int ulIp)
{

	struct ospf_if *p_if ;
	struct ospf_if *p_next_if;
	struct ospf_process *p_process = NULL;
	tOSPF_NETWORK_INDEX	stNetIndex;
	u_long ulSubIndx = 0;
	u_char ucPo = 0;//逻辑端口号
	
	if(ulIp == 0)
	{
	    OSPF_DCN_LOG_WARNING("ospf_dcn_delete_by_ifindex_ip IP is 0.");
		return ERR;
	}
	if(VOS_ERR == if_index_to_logic_port(ulIfIndx, &ucPo))
    {
        return ERR;
    }
	ulSubIndx = ospf_dcn_vpif_lport_subid_to_index(ucPo,OSPF_DCN_SUBPORT);//根据逻辑端口号得到子接口索引

	stNetIndex.area = 0;
	stNetIndex.mask = OSPF_DCNDEF_IPMASK;
	stNetIndex.network = ulIp;
	stNetIndex.process_id = OSPF_DCN_PROCESS;

    p_process = ospf_get_nm_process(&stNetIndex);
	if(p_process == NULL)	
	{
	    OSPF_DCN_LOG_WARNING("Failed to get nm process,p_process is null.");
		return OK;
	}
	for_each_ospf_if(p_process, p_if, p_next_if)	
	{
		  /*area  must match*/
		  if ((NULL != p_if )&&(p_if->ifnet_uint == ulSubIndx)
		  	&&(p_if->addr == ulIp))	  
		  {
			  ospf_if_delete(p_if);
			  return OK;
		  }
	}
	return ERR;


}

/*根据逻辑端口号创建网络*/

void ospf_dcn_creat_overlay_if(tOSPF_NETWORK_INDEX *p_index,u_char ucPort)
{
	struct ospf_area *p_area = NULL;
	struct ospf_process *p_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;    
	struct ospf_network *p_network = NULL;    
	STATUS rc = OK;
	STATUS ifrc = OK;
	u_int if_unit = 0;
	u_int addr = 0,unit = 0;
	u_int mask = 0;

	p_process = ospf_get_nm_process(p_index);
	if(NULL == p_process)
	{
	    OSPF_DCN_LOG_WARNING("Failed to get nm process,p_process is null.");
		return;
	}

    p_area = ospf_area_lookup(p_process,  p_index->area);
    if(NULL == p_area)
    {
	    OSPF_DCN_LOG_WARNING("Failed to get nm process,p_process is null.");
	    return;
    }
    addr = p_index->network;
    mask = p_index->mask;

    /*ignore invalid interfaces*/     
	
	unit = ospf_dcn_vpif_lport_subid_to_index(ucPort,OSPF_DCN_SUBPORT);//dcn 子接口索引

    p_if = ospf_if_lookup_forDcnCreat(p_process, addr,unit);
	if (p_if != NULL)
	{
	    ospf_logx(ospf_debug,"%d, p_if->addr = 0x%x, p_if->ifnet_uint = 0x%x\n",__LINE__, p_if->addr, p_if->ifnet_uint);
	    OSPF_DCN_LOG_WARNING("The p_if is not null, p_if->addr = %x, p_if->ifnet_uint = %x.",p_if->addr, p_if->ifnet_uint);
	}
    ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS  p_if=%p.\r\n",p_if);
  /*if interface exist,and interface has area configured,do nothing*/
     if (NULL != p_if) 
     {                       
         if (NULL == p_if->p_area)
         {
            ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS NULL == p_if->p_area.\r\n");
            p_if->p_area = p_area;        
          //  p_if->state = OSPF_IFS_DOWN;
            //ospf_lstadd_unsort(&p_area->if_table, p_if);
            ospf_lstadd(&p_area->if_table, p_if);
         }
     }
     else
     {    
         /*create interface with area*/                          
         ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS real_if_cr.\r\n");
         ospf_real_if_create(p_process,addr,p_area);
     }
}

int ospf_dcn_create_overlay_by_ip_logic_port(u_int ulIp,u_char ucPort)
{
	u_long ulValue=0;
	tOSPF_NETWORK_INDEX	stOspfNetWorkIndex;
	int iRtv = 0;
	octetstring stOct;
	u_char aucMac[MAC_ADDR_LEN] = {0};
	u_long ulLoIp = 0xc0a80301; /*默认ip地址为192.mac[5].3.1*/
	u_int ulNetWorkMask = 0;
	u_char ucIpAddr[4];
	u_long	ulMaskIp = 0;
	long	ldcnIp = ulIp;
	u_int ulns = OSPF_DCN_PROCESS;


	stOspfNetWorkIndex.area = 0;
	stOspfNetWorkIndex.mask = OSPF_DCN_OVERLAY_IPMASK;
	stOspfNetWorkIndex.network = ulIp;
	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
	
	ospf_dcn_creat_overlay_if(&stOspfNetWorkIndex,ucPort);
	return OK;


}


/*删除单个接口网络,ulIfIndx-接口索引，如131073*/
int ospf_dcn_del_overlay_by_ifindex_ip(u_long ulIfIndx,u_int ulIp)
{

	struct ospf_if *p_if ;
	struct ospf_if *p_next_if;
	struct ospf_process *p_process = NULL;
	tOSPF_NETWORK_INDEX	stNetIndex;
	u_long ulSubIndx = 0;
	u_char ucPo = 0;//逻辑端口号
	
	if(VOS_ERR == if_index_to_logic_port(ulIfIndx, &ucPo))
    {
        return ERR;
    }
	ulSubIndx = ospf_dcn_vpif_lport_subid_to_index(ucPo,OSPF_DCN_SUBPORT);//根据逻辑端口号得到子接口索引

	stNetIndex.area = 0;
	stNetIndex.mask = OSPF_DCN_OVERLAY_IPMASK;
	stNetIndex.network = ulIp;
	stNetIndex.process_id = OSPF_DCN_PROCESS;

    p_process = ospf_get_nm_process(&stNetIndex);
	if(p_process == NULL)	
	{
	    OSPF_DCN_LOG_WARNING("ospf_dcn_del_overlay_by_ifindex_ip p_process is null.");
		return OK;
	}
	for_each_ospf_if(p_process, p_if, p_next_if)	
	{
		  /*area  must match*/
		  if ((NULL != p_if )&&(p_if->ifnet_uint == ulSubIndx)
		  	&&(p_if->addr == ulIp))	  
		  {
			  ospf_if_delete(p_if);
			  return OK;
		  }
	}


}


int ospf_dcn_remove_port_by_ifindex(u_int uiIndx)
{
	int iRtv = 0,iRet = 0,iVlan = 4094;
	u_long ulIfIndx = 0,ulValue = 0,ulExt = 0,ulPortIdx = 0,ulLbIp = 0;
	u_char ucPortRole = 0;
	char szCmd[256] = {0};
	u_char ucSlot = 0,ucPort = 0,ucSubPort = OSPF_DCN_SUBPORT;
    u_char ucL3Flag = 0;

	
	ulLbIp = stOspfDcn.ulDeviceIp;
	iRet = ospf_dcn_delete_by_ifindex_ip(uiIndx,ulLbIp);
	ucSlot = IFINDEX_TO_SLOT(uiIndx);
    if(VOS_ERR == if_index_to_logic_port(uiIndx, &ucPort))
    {
        return ERR;
    }
    iRet |= ospf_set_acl(uiIndx,OSPF_RULE__DEL);   /*删除UNI、BRG端口ACL规则*/
  //  printf("ospf_dcn_remove_port_by_ifindex  OSPF_RULE__DEL ucPort=%d,iRet=%d\n",ucPort,iRet);

	ospf_dcn_set_port_enable(ucSlot,ucPort,OSPF_DCN_PORT_DISABLE);

	return iRet;
}

int ospf_dcn_delete_port_by_ifindex(u_int uiIndx)
{
	int iRtv = 0,iRet = 0,iVlan = 4094;
	u_long ulIfIndx = 0,ulValue = 0,ulExt = 0,ulPortIdx = 0,ulLbIp = 0;
	u_char ucPortRole = 0;
	char szCmd[256] = {0};
	u_char ucSlot = 0,ucPort = 0,ucSubPort = OSPF_DCN_SUBPORT;
    u_char ucL3Flag = 0;

	ulLbIp = stOspfDcn.ulDeviceIp;
	iRet = ospf_dcn_delete_by_ifindex_ip(uiIndx,ulLbIp);
	ucSlot = IFINDEX_TO_SLOT(uiIndx);
    if(VOS_ERR == if_index_to_logic_port(uiIndx, &ucPort))
    {
        return ERR;
    }
    iRet |= ospf_set_acl(uiIndx,OSPF_RULE__DEL);   /*删除UNI、BRG端口ACL规则*/
  //  printf("ospf_dcn_delete_port_by_ifindex  OSPF_RULE__DEL ucPort=%d,iRet=%d\n",ucPort,iRet);

	ospf_dcn_set_port_enable(ucSlot,ucPort,OSPF_DCN_PORT_DISABLE);
    ospf_logx(ospf_debug_dcn,"ospf_dcn_delete_port_by_ifindex iRet:%d\n", iRet);
	return iRet;
}



int ospf_dcn_enable_port(u_long ulIfIndx,u_char ucEnmode)
{
	int i = 0,iRtv = 0;
	u_long ulValue = 0,ulExt = 0,ulLbIp = 0;
	u_char ucPortRole = 0;
	u_char ucSlot = 0,ucPort = 0;
	char szCmd[256] = {0};
	
	ucSlot = IFINDEX_TO_SLOT(ulIfIndx);
	if(VOS_ERR == if_index_to_logic_port(ulIfIndx, &ucPort))
    {
        return ERR;
    }

    ulLbIp = stOspfDcn.ulDeviceIp;
	
	iRtv = ospf_dcn_get_init();
//	printf("#%d:iRtv = %d\n",__LINE__,iRtv);
	if(iRtv != 1)
	{
		//ospf_dcn_set_port_enable(ucSlot,ucPort,ucEnmode);
		iRtv = OK;
	}
	else
	{
		iRtv =ospf_dcn_get_globale_enable();
		if(iRtv != OSPF_DCN_ENABLE)
		{
			return ERR;
		}		
		
		ospf_dcn_setSubIntf_Mod(ulIfIndx,ucEnmode);
		if(ucEnmode == OSPF_DCN_PORT_ENABLE)	
		{
			if(stDcnCfg.uiUsePort[ucSlot][ucPort] != OSPF_DCN_PORT_ENABLE)
			{
				ospf_dcn_set_port_enable(ucSlot,ucPort,ucEnmode);
				iRtv = ospf_dcn_create_by_ifindex(ulIfIndx);
				if(iRtv == ERR)
				{
                    OSPF_DCN_LOG_WARNING("ospf_dcn_create_by_ifindex ulIfIndx=%x.",ulIfIndx);
					return ERR;
				}
				ospf_dcn_addr_update(ucPort);
				ospf_dcn_upate_if();
			}

		}
		else if(ucEnmode == OSPF_DCN_PORT_DISABLE)	
		{
			if(stDcnCfg.uiUsePort[ucSlot][ucPort] == OSPF_DCN_PORT_ENABLE)
			{
				iRtv = ospf_dcn_delete_port_by_ifindex(ulIfIndx);
			}
		}
	}
	return iRtv;
}



int ospf_getDcnLogin(void)
{
	return stDcnCfg.ucChangeFlag;
}
int ospf_setDcnLogin(u_char ucData)
{
	stDcnCfg.ucChangeFlag = ucData;

}

void ospf_dcn_if_update(struct ospf_if *p_if)
{
	u_int flag = 0;
  
	ospf_logx(ospf_debug_dcn,"#%d,type=%d.\r\n",__LINE__,p_if->type);
	
	ospf_if_mcast_group_set(p_if, IP_ADD_MEMBERSHIP);//dai test
		

	ospf_logx(ospf_debug_dcn,"#%d,type=%d.link_up=%d.\r\n",__LINE__,p_if->type,p_if->link_up);

    return ;
}

void ospf_dcn_upate_if()
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;

    for_each_ospf_process(p_process, p_next_process)          
    {
        /*for safe,update all interface state in this process*/
        ospf_lstwalkup(&p_process->if_table, ospf_dcn_if_update);
    }
}



/*修改overlay vlan时调用，解决改完vlan后socket收包问题*/
void ospf_dcn_modify_overlay_vlan(u_long ulPort)
{
    struct ospf_process *p_process = NULL;
    struct ospf_process *p_next_process = NULL;
	struct ospf_if start_if;  
	struct ospf_if *p_if = NULL;

	for_each_node_noless(&ospf.real_if_table, p_if, &start_if)
    {
    	if(VOS_OK != if_vport_ifindex_to_logic_port(p_if->ifnet_uint,&ulPort))
		{
			continue;
		}
        if((p_if->ulDcnflag != OSPF_DCN_FLAG)
			&&(p_if->p_process->process_id == OSPF_DCN_PROCESS))
		{
			ospf_if_mcast_group_set(p_if, IP_ADD_MEMBERSHIP);
			return;
		}

	}
	return;
}

int ospf_dcn_init()
{
	int iRtv = 0,iValue = 0;
	u_long ulLbIp = 0;
	u_long ulDcnTime = 0;
	

	ospf_dcn_set_init_flag(1);

    iRtv =ospf_dcn_get_globale_enable();
	if(iRtv == OSPF_DCN_ENABLE)
    {
        return OK;
    }
	//iRtv = dev_hw_get_dcn_en_state(&iValue);
	if((iRtv != OK)||(iValue != 0))
	{	
		iRtv = ospf_dcn_create();
	}
    return iRtv;
}

int ospf_dcn_port_get_first(u_int *pulIfIndex)
{
	u_int ulIfIndx = 0;
	u_char ucTyp = 0;
	u_int uiSubPort = 0;
	
	if(OK != zebra_if_get_api(0, ZEBRA_IF_GET_FIRST_IFINDEX, pulIfIndex))
	{
		return ERR;
	}
	
	ulIfIndx = *pulIfIndex;

	ucTyp = IFINDEX_TO_TYPE(ulIfIndx);
	uiSubPort = uspVpIfToSubId(ulIfIndx);
	if((OSPF_DCN_SUBPORT == uiSubPort)&&(ucTyp == IFINDEX_TYPE_VPIF))
	{
		return OK;
	}
	return ERR;
}

int ospf_dcn_port_check(u_int ulIfIndx)
{
	u_char ucTyp = 0;
	u_int uiSubPort = 0;

	ucTyp = IFINDEX_TO_TYPE(ulIfIndx);
	uiSubPort = uspVpIfToSubId(ulIfIndx);

	if((OSPF_DCN_SUBPORT == uiSubPort)&&(ucTyp == IFINDEX_TYPE_VPIF))
	{
		return OK;
	}
	return ERR;
}

int ospf_dcn_port_get_next(u_int *pulIfIndex, u_int *pulNextIfIndex)
{
	u_int ulIfIndx = 0,uiIndex;
	u_char ucTyp =0,ucSubPort=0;
	if(OK != zebra_if_get_api(*pulIfIndex, ZEBRA_IF_GET_NEXT_IFINDEX, pulNextIfIndex))
	{
		return ERR;
	}
	while(OK !=  ospf_dcn_port_check(*pulNextIfIndex))
	{
		uiIndex = *pulNextIfIndex;
		if(OK != zebra_if_get_api(uiIndex,ZEBRA_IF_GET_NEXT_IFINDEX,pulNextIfIndex))
		{
			return ERR;
		}
	}
	return OK;
}

/*将子接口移至vlan中，但不安装ip*/
int ospf_dcn_port_vpif_vlan_create(u_int uiLportNo)
{
    u_long ulVlanid,ulLbIp = 0;
    u_long ulVpIfIndex;
    u_char ucRowStatus;
    struct prefix_ipv4 stIp; 
    struct interface *ifp = NULL;
    int ret = 0;
    
    ulLbIp = stOspfDcn.ulDeviceIp;
    ulVlanid = stOspfDcn.uiVlanId;
    
    if(uiLportNo > MAX_PORT_NUM)
    {
        return ERR;
    }

    if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLportNo,OSPF_DCN_SUBPORT,&ulVpIfIndex))
    {
        return ERR;
    }
        
    zebra_if_get_api(ulVpIfIndex,ZEBRA_IF_GET_IF_BY_IF_INDEX,ifp);
	if(NULL == ifp)
	{
        zlog_info(MTYPE_OSPF," if_get_by_index: Ifindex=0x%x failed!\n",ulVpIfIndex);
        OSPF_DCN_LOG_WARNING(" if_get_by_index: Ifindex=%x failed!",ulVpIfIndex);
        return ERR;
	}
    
    /*vlan hw set*/
    #if 0
	if(ERR == l3if_encap_add (ifp, ulVlanid))
	{
        zlog_info(MTYPE_OSPF," l3if_encap_add: Ifindex=0x%x ,ulVlanid=%d failed!\n",ulVpIfIndex,ulVlanid);
        OSPF_DCN_LOG_WARNING(" l3if_encap_add: Ifindex=%x ,ulVlanid=%d failed!",ulVpIfIndex,ulVlanid);
		return ERR;
	}
    #endif

    return OK;
}


/*安装ip地址，lpbk 31及dcn子接口调用*/
int ospf_dcn_ipaddr_install(u_int uiLportNo,u_int uiPortType,u_long ulIpaddr,u_int uiMasklen)
{
    u_long ulVpIfIndex = 0;
    struct prefix_ipv4 stIp; 
    struct prefix stPrefx;
    int iRtv = 0;
    
    /*delete  old ip*/
    if(IFINDEX_TYPE_VPIF == uiPortType)
    {
        if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLportNo,OSPF_DCN_SUBPORT,&ulVpIfIndex))
        {
            return ERR;
        }
    }
    else if(IFINDEX_TYPE_LOOPBACK_IF == uiPortType)
    {
        if(VOS_ERR == if_loopback_id_to_index(OSPF_DCNLOOPBACK_IFINDEX,&ulVpIfIndex))
        {
            return ERR;
        }
    }
    else if(IFINDEX_TYPE_ETH_VTRUNK == uiPortType)
    {
        if(VOS_ERR == if_vlag_logic_port_subid_to_index(uiLportNo,OSPF_DCN_SUBPORT,&ulVpIfIndex))
        {
            return ERR;
        }
    }
    else
    {
        return ERR;
    }
    memset(&stPrefx,0,sizeof(struct prefix));
    /*ip hw set*/
	stIp.family = AF_INET;
	stIp.prefixlen = uiMasklen;
	stIp.prefix.s_addr = ntohl(ulIpaddr);
	iRtv = ip_address_install(ulVpIfIndex, stIp,0,0);
    if(iRtv != VOS_ERR_NO_ERROR)
    {
  	    ospf_logx(ospf_debug_if,"%d ip_address_install: Ifindex=0x%x,Addr=0x%x failed!\n",__LINE__,ulVpIfIndex,ulIpaddr);
  	    zlog_info(MTYPE_OSPF," ip_address_install: Ifindex=0x%x,Addr=0x%x failed!\n",ulVpIfIndex,ulIpaddr);
  	    OSPF_DCN_LOG_WARNING(" ip_address_install: Ifindex=%x,Addr=%x failed!",ulVpIfIndex,ulIpaddr);
    } 

    return iRtv;
}


/*移除ip地址，lpbk 31及dcn子接口调用*/
int ospf_dcn_ipaddr_uninstall(u_int uiLportNo,u_int uiPortType)
{
    u_long ulVpIfIndex = 0;
    struct prefix_ipv4 stIp; 
    struct prefix stPrefx;
    int iRtv = 0;
    
    /*delete  old ip*/
    if(IFINDEX_TYPE_VPIF == uiPortType)
    {
        if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLportNo,OSPF_DCN_SUBPORT,&ulVpIfIndex))
        {
            return ERR;
        }
    }
    else if(IFINDEX_TYPE_LOOPBACK_IF == uiPortType)
    {
        if(VOS_ERR == if_loopback_id_to_index(OSPF_DCNLOOPBACK_IFINDEX,&ulVpIfIndex))
        {
            return ERR;
        }
    }
    else if(IFINDEX_TYPE_ETH_VTRUNK == uiPortType)
    {
        if(VOS_ERR == if_vlag_logic_port_subid_to_index(uiLportNo,OSPF_DCN_SUBPORT,&ulVpIfIndex))
        {
            return ERR;
        }
    }
    else
    {
        return ERR;
    }
    memset(&stPrefx,0,sizeof(struct prefix));

    if(OK == ospf_IntfIpAddr_get(ulVpIfIndex,&stPrefx))
    {
        stIp.prefix = stPrefx.u.prefix4;
        
    	/*ip hw set*/
    	stIp.family = stPrefx.family;
    	stIp.prefixlen = stPrefx.prefixlen;
        
    	iRtv = ip_address_uninstall(ulVpIfIndex, stIp,0,0);
        if(iRtv != VOS_ERR_NO_ERROR)
        {
            zlog_info(MTYPE_OSPF," ip_address_uninstall: Ifindex=0x%x,Addr=0x%x failed!\n",ulVpIfIndex,stIp);
            OSPF_DCN_LOG_WARNING(" ip_address_uninstall: Ifindex=%x,Ip=%x failed!\n",ulVpIfIndex,stIp);
        } 
    }
    return iRtv;
}

int ospf_dcn_lag_ipaddr_install(u_long ulVpIfIndex,u_int uiPortType,u_long ulIpaddr,u_int uiMasklen)
{
    struct prefix_ipv4 stIp; 
    struct prefix stPrefx;
    int iRtv = 0;
    
    memset(&stPrefx,0,sizeof(struct prefix));
    /*ip hw set*/
	stIp.family = AF_INET;
	stIp.prefixlen = uiMasklen;
	stIp.prefix.s_addr = ntohl(ulIpaddr);
	iRtv = ip_address_install(ulVpIfIndex, stIp,0,0);
    if(iRtv != VOS_ERR_NO_ERROR)
    {
  	    zlog_info(MTYPE_OSPF," ip_address_install: Ifindex=0x%x,Addr=0x%x failed!\n",ulVpIfIndex,ulIpaddr);
        OSPF_DCN_LOG_WARNING(" ip_lag_address_uninstall: Ifindex=%x,Ip=%x failed!",ulVpIfIndex,stIp);
    } 

    return iRtv;
}




int ospf_dcn_port_vpif_create(u_int uiLportNo)
{
    u_long ulVlanid,ulLbIp = 0;
    u_long ulVpIfIndex = 0,ulIpAddr = 0,ulMasklen = 0;
    u_char ucRowStatus;
    struct prefix_ipv4 stIp; 
    struct interface *ifp = NULL;
    int ret = 0;
    u_int uiValue = ZEBRA_IF_CREATE;
    
    //dcn_get_api(DCN_NE_IP,&ulLbIp);	/*获取dcn ip*/
    //dcn_get_api(DCN_NE_VLAN_ID,&ulVlanid);	/*获取dcn vlan*/

    if(uiLportNo > MAX_PORT_NUM + LAG_GROUP_NUM)
    {
        return ERR;
    }

    if(uiLportNo <=  MAX_PORT_NUM)
    {
        if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLportNo,stOspfDcn.uiVlanId,&ulVpIfIndex))
        {
            return ERR;
        }
    }
    else
    {
        uiLportNo = uiLportNo - MAX_PORT_NUM;
        if(VOS_ERR == if_vlag_logic_port_subid_to_index(uiLportNo, stOspfDcn.uiVlanId, &ulVpIfIndex))
        {
            return ERR;
        }
    }
    
    if(zebra_if_set_api(ulVpIfIndex,ZEBRA_IF_ROW_STATUS,&uiValue) != VOS_OK)
	{
        zlog_info(MTYPE_OSPF," if_get_by_index: Ifindex=0x%x failed!\n",ulVpIfIndex);
        OSPF_DCN_LOG_WARNING("if_get_by_index: Ifindex=%x failed!",ulVpIfIndex);
        ospf_logx(ospf_debug_dcn,"Failed to create interface index = %d\r\n", ulVpIfIndex, __LINE__);
        return ERR;
	}

	if(dcn_config_load_done() == VOS_TRUE)
	{
        ospf_dcn_vlan_dot1q_enable(ulVpIfIndex, stOspfDcn.uiVlanId);
        ospf_dcn_bind_vpn(ulVpIfIndex);
    }

    ospf_dcn_ip_unnumber_cfg(ulVpIfIndex, ENABLE);

    /*vlan hw set*/
    #if 0
	if(ERR == l3if_encap_add (ifp, ulVlanid))
	{
        zlog_info(MTYPE_OSPF," l3if_encap_add: Ifindex=0x%x ,ulVlanid=%d failed!\n",ulVpIfIndex,ulVlanid);
        OSPF_DCN_LOG_WARNING(" l3if_encap_add: Ifindex=%x ,ulVlanid=%d failed!\n",ulVpIfIndex,ulVlanid);
		return ERR;
	}
    #endif

    #if 0
    /*查询接口是否为overlay master*/
    ret = ospf_dcn_lookup_master_port(uiLportNo,&ulIpAddr,&ulMasklen);
    if(ret == OK)
    {	
        ulIpAddr = (ulIpAddr & OSPF_DCN_OVERLAY_IPMASK)+LLDP_OVERLAY_MASTER_IPADDR_MNG;
        ret = ospf_dcn_overlay_ipaddr_install(uiLportNo,ulIpAddr,ulMasklen,DCN_OVERLAY_MASTER_ADD); 
        if(ret != OK)
        {
            zlog_info(MTYPE_OSPF," ospf_dcn_overlay_ipaddr_install: uiLportNo=%d ,ulVlanid=%d failed!\n",uiLportNo,ulLbIp);
            OSPF_DCN_LOG_WARNING(" ospf_dcn_overlay_ipaddr_install: uiLportNo=%d ,Ip=%d failed!",uiLportNo,ulIpAddr);
        }
    }
    else
    {
        /*ip hw set*/
        ret = ospf_dcn_ipaddr_install(uiLportNo,IFINDEX_TYPE_VPIF,ulLbIp,OSPF_DCN_MASK_LEN);
        if(ret != OK)
        {
            zlog_info(MTYPE_OSPF," ospf_dcn_ipaddr_install: uiLportNo=%d ,ulVlanid=%d failed!\n",uiLportNo,ulLbIp);
            OSPF_DCN_LOG_WARNING(" ospf_dcn_ipaddr_install: uiLportNo=%d ,Ip=%d failed!",uiLportNo,ulLbIp);
        }
    }
    #endif
    return OK;
}

int ospf_dcn_port_vpif_destroy(u_int uiLportNo)
{
    u_long ulVlanid;
    u_long ulVpIfIndex;
    struct interface *ifp = NULL;

    if(uiLportNo > MAX_PORT_NUM + LAG_GROUP_NUM)
    {
        return ERR;
    }

    if(uiLportNo <=  MAX_PORT_NUM)
    {
        if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLportNo,stOspfDcn.uiVlanId,&ulVpIfIndex))
        {
            return ERR;
        }
    }
    else
    {
        uiLportNo = uiLportNo - MAX_PORT_NUM;
        if(VOS_ERR == if_vlag_logic_port_subid_to_index(uiLportNo, stOspfDcn.uiVlanId, &ulVpIfIndex))
        {
            return ERR;
        }
    }


    ospf_dcn_ip_unnumber_cfg(ulVpIfIndex, DISABLE);

    ospf_dcn_disable_bind_vpn(ulVpIfIndex);

    ospf_dcn_vlan_dot1q_disable(ulVpIfIndex, stOspfDcn.uiVlanId);

    ospf_dcn_one_interface_delete(ulVpIfIndex);
    
    return OK;
}

#ifdef OSPF_DCN
void ospf_dcn_lag_vpif_create(u_short usLagId)
{
	u_long ulVlanid,ulLbIp = 0;
    u_long ulIpAddr = 0,ulMasklen = 0;
    u_long uiIfindex = 0;
    u_char ucRowStatus;
    ZEBRA_IF_INDEX_T stIndx; 
    struct prefix_ipv4 addr_prefix;
    struct interface *ifp = NULL;
    int ret = 0;
    u_int uiMaskLen = 32;
    u_char ucRow = ZEBRA_IF_CREATE;
    u_int uiVlan = 0;
	
    if(VOS_ERR == if_vlag_logic_port_subid_to_index(usLagId,OSPF_DCN_SUBPORT,&uiIfindex))
    {
        return ;
    }

    ulLbIp = stOspfDcn.ulDeviceIp;
    ulVlanid = stOspfDcn.uiVlanId;

	zebra_if_get_api(uiIfindex,ZEBRA_IF_GET_IF_BY_IF_INDEX,ifp);
	if(NULL == ifp)
	{
        zlog_info(MTYPE_OSPF," if_get_by_index: Ifindex=0x%x failed!\n",uiIfindex);
        return ERR;
	}
	
	/*采用api配置虚拟子接口*/
    if(OK != zebra_if_set_api(uiIfindex, ZEBRA_IF_ROW_STATUS, &ucRow))
    {
        return CMD_WARNING;
    }
    
    /*vlan hw set*/
    zebra_if_get_api(uiIfindex,ZEBRA_IF_GET_IF_BY_IF_INDEX,ifp);
	if(NULL == ifp)
	{
        zlog_info(MTYPE_OSPF," if_get_by_index: Ifindex=0x%x failed!\n",uiIfindex);
        return ERR;
	}
	#if 0
	if(ERR == l3if_encap_add (ifp, ulVlanid))
	{
        zlog_info(MTYPE_OSPF," l3if_encap_add: Ifindex=0x%x ,ulVlanid=%d failed!\n",uiIfindex,ulVlanid);
		return ERR;
	}
    #endif

    addr_prefix.family = AF_INET;
    addr_prefix.prefixlen = uiMaskLen;
    addr_prefix.prefix.s_addr = ntohl(ulLbIp);
    if(OK != zebra_if_set_api(uiIfindex,ZEBRA_IF_IPADRESS_SET,&addr_prefix))
	{
        //vty_out (vty, "%% Create Ipv4 address failed%s", VTY_NEWLINE);
		return ERR;
	}
    uiVlan = 1;
    if(VOS_OK != zebra_if_set_api(uiIfindex,ZEBRA_IF_PHY_IF_VLAN_ID,&uiVlan))
    {
        return VOS_ERR;
    }
    /*查询接口是否为overlay master*/
    #if 0
    ret = ospf_dcn_lookup_master_port(uiLportNo,&ulIpAddr,&ulMasklen);
    if(ret == OK)
    {	
        ulIpAddr = (ulIpAddr & OSPF_DCN_OVERLAY_IPMASK)+LLDP_OVERLAY_MASTER_IPADDR_MNG;
        ret = ospf_dcn_overlay_ipaddr_install(uiLportNo,ulIpAddr,ulMasklen,DCN_OVERLAY_MASTER_ADD); 
        if(ret != OK)
        {
            zlog_info(MTYPE_OSPF," ospf_dcn_overlay_ipaddr_install: uiLportNo=%d ,ulVlanid=%d failed!\n",uiLportNo,ulLbIp);
        }
    }
    else
    {
        /*ip hw set*/
        ret = ospf_dcn_lag_ipaddr_install(uiIfindex,IFINDEX_TYPE_ETH_VTRUNK,ulLbIp,OSPF_DCN_MASK_LEN);
        if(ret != OK)
        {
            zlog_info(MTYPE_OSPF," ospf_dcn_ipaddr_install: ulVlanid=%d failed!\n",ulLbIp);
        }
    }
    #endif
    return OK;
}


#endif

void ospf_dcn_if_del(u_int unit)
{
	struct ospf_process *p_process = NULL;
	struct ospf_if *p_if = NULL;
	struct ospf_if *p_next_if = NULL;    
    	
	tOSPF_NETWORK_INDEX *p_index,stOspfNetWorkIndex;
	

	stOspfNetWorkIndex.area = 0;
	stOspfNetWorkIndex.mask = OSPF_DCNDEF_IPMASK;
	stOspfNetWorkIndex.network = 0;
	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
	p_index = &stOspfNetWorkIndex;

	
    p_process = ospf_get_nm_process(p_index);
    for_each_ospf_if(p_process, p_if, p_next_if)
    {
        if (unit == p_if->ifnet_uint)
        {
            ospf_logx(ospf_debug,"ospf_updatIfDel if_delete ifindex=0x%x.\r\n",unit);
            ospf_if_delete(p_if);
        }
    }
        
}
#if 0
int ospf_dcn_lag_vpif_create(u_short usLagId)
{
    u_long ulVlanid,ulLbIp = 0;
    u_long ulVpIfIndex = 0,ulIpAddr = 0,ulMasklen = 0;
    u_int uiIfindex = 0;
    u_char ucRowStatus;
    struct prefix_ipv4 stIp; 
    struct interface *ifp = NULL;
    int ret = 0;
    #ifdef OSPF_DCN
    dcn_set_api(&ulLbIp,DCN_NE_IP);	/*获取dcn ip*/
    dcn_set_api(&ulVlanid,DCN_NE_VLAN_ID);	/*获取dcn vlan*/

    if(uiLportNo > MAX_PORT_NUM)
    {
        return ERR;
    }

	if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLportNo,OSPF_DCN_SUBPORT,&ulVpIfIndex))
    {
        return ERR;
    }
	#endif
	if(VOS_ERR == if_vlag_logic_port_subid_to_index(usLagId,OSPF_DCN_SUBPORT,&uiIfindex))
    {
        OSPF_DCN_LOG_WARNING(" dev_vlag_lport_subid_to_index: usLagId=%d failed!",usLagId);
        return ;
    }

	zebra_if_get_api(uiIfindex,ZEBRA_IF_GET_IF_BY_IF_INDEX,ifp);
	if(NULL == ifp)
	{
        zlog_info(MTYPE_OSPF," if_get_by_index: Ifindex=0x%x failed!\n",uiIfindex);
        OSPF_DCN_LOG_WARNING(" if_get_by_index: Ifindex=%x failed!",uiIfindex);
        return ERR;
	}
	
    dcn_set_api(DCN_NE_IP,&ulLbIp);	/*获取dcn ip*/
    dcn_set_api(DCN_NE_VLAN_ID,&ulVlanid);	/*获取dcn vlan*/
    
    /*vlan hw set*/
    #if 0
	if(ERR == l3if_encap_add (ifp, ulVlanid))
	{
        zlog_info(MTYPE_OSPF," l3if_encap_add: Ifindex=0x%x ,ulVlanid=%d failed!\n",uiIfindex,ulVlanid);
        OSPF_DCN_LOG_WARNING(" l3if_encap_add: Ifindex=%x ,ulVlanid=%d failed!",uiIfindex,ulVlanid);
		return ERR;
	}
	#endif
    #if 0
    /*查询接口是否为overlay master*/
    ret = ospf_dcn_lookup_master_port(uiLportNo,&ulIpAddr,&ulMasklen);
    if(ret == OK)
    {	
        ulIpAddr = (ulIpAddr & OSPF_DCN_OVERLAY_IPMASK)+LLDP_OVERLAY_MASTER_IPADDR_MNG;
        ret = ospf_dcn_overlay_ipaddr_install(uiLportNo,ulIpAddr,ulMasklen,DCN_OVERLAY_MASTER_ADD); 
        if(ret != OK)
        {
            zlog_info(MTYPE_OSPF," ospf_dcn_overlay_ipaddr_install: uiLportNo=%d ,ulVlanid=%d failed!\n",uiLportNo,ulLbIp);
        }
    }
    else
    #endif
    {
        /*ip hw set*/
        ret = ospf_dcn_ipaddr_install(usLagId,IFINDEX_TYPE_ETH_VTRUNK,ulLbIp,OSPF_DCN_MASK_LEN);
        if(ret != OK)
        {
            zlog_info(MTYPE_OSPF," ospf_dcn_ipaddr_install: usLagId=%d ,ulVlanid=%d failed!\n",usLagId,ulLbIp);
            OSPF_DCN_LOG_WARNING(" ospf_dcn_ipaddr_install: usLagId=%d ,ulVlanid=%d failed!",usLagId,ulLbIp);
        }
    }

    //l3if_new_hook(ifp);
    #if 0
    if (!if_is_up(ifp))
    {
        if_set_flags (ifp, IFF_UP | IFF_RUNNING);
        l3if_refresh (ifp);
    }
    #endif
    SET_FLAG (ifp->status, INTERFACE_ACTIVE);
    if_unset_flags (ifp, IFF_UP | IFF_RUNNING);
    l3if_refresh (ifp);  

	ospf_dcn_create_lag_port(ulLbIp,uiIfindex);
	
    return OK;
}
#endif

int ospf_dcn_lag_vpif_destory(u_short usLagId)
{
	u_long ulVlanid;
    u_long uiIfindex;
    struct interface *ifp = NULL;

	if(VOS_ERR == if_vlag_logic_port_subid_to_index(usLagId,OSPF_DCN_SUBPORT,&uiIfindex))
    {
        return ERR;
    }
    lldp_rem_del_batch_by_if_index(uiIfindex);
    ulVlanid = stOspfDcn.uiVlanId;

    ifp = if_lookup_by_index(uiIfindex);
    if(NULL == ifp)
    {
        zlog_info(MTYPE_OSPF," if_lookup_by_index: Ifindex=0x%x failed!\n",uiIfindex);
        OSPF_DCN_LOG_WARNING(" if_lookup_by_index: Ifindex=%x failed!",uiIfindex);
        return ERR;
    }
    /*ip delete need ?*/
    
    if_delete( ifp);

    ospf_dcn_if_del(uiIfindex);
    return OK;
}





int ospf_dcn_vlan_hw_creat(u_int uiVlanid)
{
#if 0
    hwSetApi(0, SDK_SET_VLAN_CRT, &uiVlanid);
#endif    
}

int ospf_dcn_add_vlan_port(u_int uiSlot,u_int uiVlanid,u_char cmd)
{
	int i = 0;
	u_int uiLport = 0;
	u_int uiPport = 0;
	u_int uiIfIndex = 0;
	u_long ulPorts[3] = {0x0};
	u_char aucPorts[128] = {0};
	char szcPrintBuf[1024] = {0};
	char *pcPrt = szcPrintBuf;
	u_int uiDevType = 0;

	
	switch(cmd)
	{
		case DCN_ADD_VLAN:
		{
			for(i = 1; i <= SLOT_MAX_PORT_NUM; i++)
			{	
				uiLport = LOGIC_PORT_GENERATE(uiSlot, i);
				if(VOS_OK != if_logic_port_to_index(uiLport,&uiIfIndex))
				{
				    continue;
				}

				if_logic_port_to_phy_port(uiLport, &uiPport);
		        if(uiPport < 32)
		        {
		        	ulPorts[0] |= (0x1 << uiPport);
		        }
		        else if((31 < uiPport)&&(64 > uiPport))
		        {
		        	ulPorts[1] |= (0x1 << (uiPport-32));
		        }
		        else if(63 < uiPport)
		        {	
		        	ulPorts[2] |= (0x1 << (uiPport-64));
		        }
		    }
			sprintf(pcPrt, "vlan add %d pbm=0x%x",uiVlanid,ulPorts[0]);
			//sh_process_command(0, szcPrintBuf);
			sprintf(pcPrt, "vlan add %d pbm=0x%x00000000",uiVlanid,ulPorts[1]);
			//sh_process_command(0, szcPrintBuf);
			sprintf(pcPrt, "vlan add %d pbm=0x%x0000000000000000",uiVlanid,ulPorts[2]);
			//sh_process_command(0, szcPrintBuf);
		}
		break;
		case DCN_DEL_VLAN:
		{
			for(i = 1; i <= SLOT_MAX_PORT_NUM; i++)
			{	
				uiLport = LOGIC_PORT_GENERATE(uiSlot, i);
			    if(VOS_OK != if_logic_port_to_index(uiLport,&uiIfIndex))
				{
				    continue;
				}
				
		        if_logic_port_to_phy_port(uiLport, &uiPport);
		        if(uiPport < 32)
		        {
		        	ulPorts[0] |= (0x1 << uiPport);
		        }
		        else if((31 < uiPport)&&(64 > uiPport))
		        {
		        	ulPorts[1] |= (0x1 << (uiPport-32));
		        }
		        else if(63 < uiPport)
		        {	
		        	ulPorts[2] |= (0x1 << (uiPport-64));
		        }
		    }
			sprintf(pcPrt, "vlan remove %d pbm=0x%x",uiVlanid,ulPorts[0]);
			//sh_process_command(0, szcPrintBuf);
			sprintf(pcPrt, "vlan remove %d pbm=0x%x00000000",uiVlanid,ulPorts[1]);
			//sh_process_command(0, szcPrintBuf);
			sprintf(pcPrt, "vlan remove %d pbm=0x%x0000000000000000",uiVlanid,ulPorts[2]);
			//sh_process_command(0, szcPrintBuf);
		}
		break;
		default:
		break;
	}
}


/*轮询vlan是否被使用*/
int ospf_dcn_vlan_check(u_int uiVlanid)
{
    u_char ucRowStatus;
    int ret = 0,i = 0;
    
    ret = vlan_get_api(uiVlanid, VLAN_EXIST, NULL);

    return ret;
}


/*关闭dcn全局使能时调用，删除lpbk 31地址，所有dcn子接口恢复至默认vlan 4094，但不安装ip地址*/
int ospf_dcn_vlan_to_default()
{
    int iRtv = 0,iflag = 0;
    u_int uiSlot = 0;
    u_int uiLport = 0,uiBand = 0;
    u_int uiVlanOld = 0,ulIfindex = 0;
    u_char portlist[PORTLIST_LENS] = VLAN_HW_BMP;
    u_char ucPortRole = 0;
    u_long ulVpIfIndex = 0;
    struct prefix_ipv4 stIp; 
    struct prefix stPrefx;
    //SDK_VLAN_T stVlanSet = {0};
    u_long ulPeerIp[MAX_PORT_NUM] = {0},ulGblVlan = 0,ulLbIp = 0;
    u_int uiVlanid = DCN_VLAN_DEF;

    ulLbIp = stOspfDcn.ulDeviceIp;
    ulGblVlan = stOspfDcn.uiVlanId;
    
    ospf_dcn_get_bandwidth(&uiBand);

    if(uiVlanid == ulGblVlan)
    {
        iflag = 1;    /*vlan未改变*/
    }
    /*删除dcn 网络*/
   // ospf_dcn_delete_network(ulLbIp);

    uiVlanOld = ulGblVlan;

	/*vlan被更改*/
    if(iflag != 1)
    {
        /*create new vlan and set all ports*/
        vlan_set_api(uiVlanid, VLAN_ROW_STATUS, VLAN_VALID);
        for(uiSlot = 1;uiSlot <= MAX_SLOT_NUM; uiSlot++)
		{
			ospf_dcn_add_vlan_port(uiSlot,uiVlanid,DCN_ADD_VLAN);
		}
		#if 0
        stVlanSet.lvid = uiVlanid;
        MEM_COPY(stVlanSet.allMeb, portlist, PORTLIST_LENS);
        hwSetApi(0, SDK_SET_VLAN_SET_MBR, &stVlanSet);
		#endif
		
        /*delete all vpif*/
        for(uiLport=0; uiLport<MAX_PORT_NUM; uiLport++)
        {
            if(VOS_OK != if_logic_port_to_index(uiLport, &ulIfindex))
            {
                continue;
            }
            ospf_dcn_del_overlay_config(uiLport);
          #if 0  
             if(OSPF_DCN_ENABLE == ospf_dcn_get_port_enable_status(ulIfindex))
            {
                iRtv = ospf_set_acl(ulIfindex,OSPF_RULE__DEL);   /*删除UNI、BRG端口ACL规则*/
            //    printf("ospf_dcn_vlan_to_def OSPF_RULE__DEL uiLport=%d,iRtv=%d\n",uiLport,iRtv);
            }
           #endif

            
            if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLport,OSPF_DCN_SUBPORT,&ulVpIfIndex))
            {
                continue;
            }
            /*取消dcn 子接口带宽限制*/
            ospf_dcn_set_eth_bandwidth(uiLport, uiBand,uiVlanOld,DCN_BAND_DEL);
            if(ERR ==  ospf_dcn_port_vpif_destroy(uiLport))
            {
                OSPF_DCN_LOG_WARNING(" ospf_dcn_port_vpif_destroy uiLport=%d failed!",uiLport);
            }

        }


        /*create new vpif*/

    	ospf_dcn_set_global_vlan(uiVlanid);	//配置vlan

        for(uiLport=0; uiLport<MAX_PORT_NUM; uiLport++)
        {
            if(VOS_OK != if_logic_port_to_index(uiLport, &ulIfindex))
            {
                continue;
            }

            if(ERR ==  ospf_dcn_port_vpif_vlan_create(uiLport))
            {
                zlog_info(MTYPE_OSPF,"ospf_dcn_port_vpif_create uiLport: %d err!\n",uiLport);
                OSPF_DCN_LOG_WARNING(" ospf_dcn_port_vpif_destroy uiLport=%d failed!",uiLport);
            }

        }

        /*delete old vlan*/
        vlan_set_api(uiVlanOld,VLAN_ROW_STATUS,VLAN_INVALID);
    }
    else
    {
        /*uninstall all vpif ip*/
        for(uiLport=0; uiLport<MAX_PORT_NUM; uiLport++)
        {
            if(VOS_OK != if_logic_port_to_index(uiLport, &ulIfindex))
            {
                continue;
            }

            ospf_dcn_del_overlay_config(uiLport);

          #if 0  
             if(OSPF_DCN_ENABLE == ospf_dcn_get_port_enable_status(ulIfindex))
            {
                iRtv = ospf_set_acl(ulIfindex,OSPF_RULE__DEL);   /*删除UNI、BRG端口ACL规则*/
            //    printf("ospf_dcn_vlan_to_def OSPF_RULE__DEL uiLport=%d,iRtv=%d\n",uiLport,iRtv);
            }
           #endif
            if(ERR ==  ospf_dcn_ipaddr_uninstall(uiLport,IFINDEX_TYPE_VPIF))
            {
                zlog_info(MTYPE_OSPF,"ospf_dcn_ipaddr_uninstall uiLport: %d err!\n",uiLport);
                OSPF_DCN_LOG_WARNING(" ospf_dcn_ipaddr_uninstall uiLport:%d err!\n",uiLport);
            }

        }

    }
    /*delete loopback old ip*/

    iRtv = ospf_dcn_ipaddr_uninstall(0,IFINDEX_TYPE_LOOPBACK_IF);
    if(ERR ==  iRtv)
    {
        zlog_info(MTYPE_OSPF,"ospf_dcn_ipaddr_uninstall loopback31 err!\n");
        OSPF_DCN_LOG_WARNING(" ospf_dcn_ipaddr_uninstall loopback31 err!");
    }   
	/*清lldp 邻居，重新触发*/
    lldp_rem_nbr_del_all();

    return OK;
}

int ospf_dcn_vlan_set(u_int uiVlanid)
{
    int iRtv = 0,iflag = 0;
    u_int uiLport = 0,uiBand = 0;
    u_int uiSlot = 0;
    u_int uiLagFlag = 0;
    u_int uiVlanOld,ulIfindex = 0;
    u_char portlist[PORTLIST_LENS] = VLAN_HW_BMP;
    u_char ucPortRole = 0,ucFlag = 0;
    u_long ulIpAddr = 0,ulMasklen = 0;
    //SDK_VLAN_T stVlanSet = {0};
    u_long ulPeerIp[MAX_PORT_NUM] = {0},ulVpIfIndex = 0,ulGblVlan = 0,ulLbIp = 0;

    
    if(ERR == ospf_dcn_vlan_check(uiVlanid))
    {
        return ERR;
    }

    ulLbIp = stOspfDcn.ulDeviceIp;
    ulGblVlan = stOspfDcn.uiVlanId;
    
    ospf_dcn_get_bandwidth(&uiBand);

    if(uiVlanid == ulGblVlan)
    {
        return OK;
    }
    ospf_dcn_delete_network(ulLbIp);

    uiVlanOld = ulGblVlan;
    
    /*create new vlan and set all ports*/
    vlan_set_api(uiVlanid, VLAN_ROW_STATUS, VLAN_VALID);
	for(uiSlot = 1;uiSlot <= MAX_SLOT_NUM; uiSlot++)
	{
		ospf_dcn_add_vlan_port(uiSlot,uiVlanid,DCN_ADD_VLAN);
	}	
    #if 0
    stVlanSet.lvid = uiVlanid;
    MEM_COPY(stVlanSet.allMeb, portlist, PORTLIST_LENS);
    hwSetApi(0, SDK_SET_VLAN_SET_MBR, &stVlanSet);
	#endif
         /*delete all vpif*/
     for(uiLport=0; uiLport<MAX_PORT_NUM; uiLport++)
     {
         if(VOS_OK != if_logic_port_to_index(uiLport, &ulIfindex))
         {
             continue;
         }

        if(OSPF_DCN_PORT_ENABLE == ospf_dcn_get_port_enable_status(ulIfindex))
        {
            iRtv = ospf_set_acl(ulIfindex,OSPF_RULE__DEL);   /*删除UNI、BRG端口ACL规则*/
       //     printf("ospf_dcn_vlan_to_def  OSPF_RULE__DEL ucPort=%d,iRet=%d\n",uiLport,iRtv);

        }
            		
        if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLport,OSPF_DCN_SUBPORT,&ulVpIfIndex))
        {
            continue;
        }
		zebra_if_ospf_get_api(ulVpIfIndex, ZEBRA_IF_GET_DCN_PEER_IP, &ulPeerIp[uiLport]);//获取peer IP
        /*取消dcn 子接口带宽限制*/
        ospf_dcn_set_eth_bandwidth(uiLport, uiBand,uiVlanOld,DCN_BAND_DEL);
		
		/*删除over-lay 从设备p_if*/
		iRtv = ospf_dcn_lookup_slave_port(uiLport,&ulIpAddr,&ulMasklen);
		if(OK == iRtv)
		{
			ospf_dcn_delete_by_ifindex_ip(ulIfindex,ulIpAddr); 		
		}

		if(ERR ==  ospf_dcn_port_vpif_destroy(uiLport))
		{
		  	zlog_info(MTYPE_OSPF,"ospf_dcn_port_vpif_destroy uiLport: %d err!\n",uiLport);
            OSPF_DCN_LOG_WARNING(" ospf_dcn_port_vpif_destroy uiLport:%d err!",uiLport);
		}


     }


    /*create new vpif*/

	ospf_dcn_set_global_vlan(uiVlanid);	//配置vlan
    iRtv = ospf_dcn_create_process();
    if(iRtv == OK)
	{
        iflag = 1;
    }
    for(uiLport=0; uiLport<MAX_PORT_NUM; uiLport++)
    {
        if(VOS_OK != if_logic_port_to_index(uiLport, &ulIfindex))
        {
            continue;
        }

        if(ERR ==  ospf_dcn_port_vpif_create(uiLport))
        {
            zlog_info(MTYPE_OSPF,"ospf_dcn_port_vpif_create uiLport: %d err!\n",uiLport);
            OSPF_DCN_LOG_WARNING(" ospf_dcn_port_vpif_create uiLport: %d err!",uiLport);
        }

    	ospf_dcn_set_dcn_mode_by_ifindex(ulIfindex,L3_IF_DCN_MODE_P2P); //重新配置p2p模式
    	
    	if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLport,OSPF_DCN_SUBPORT,&ulVpIfIndex))
        {
            continue;
        }
    	ospf_dcn_peer_update(ulVpIfIndex,ulPeerIp[uiLport]); 	//重新配置Peer ip
    	
    	/*添加 overlay模型master处理*/
		iRtv = ospf_dcn_lookup_master_port(uiLport,&ulIpAddr,&ulMasklen);
		if(iRtv == OK)
		{
			/*overlay 接口创建规则*/
			ospf_set_acl(ulIfindex,OSPF_RULE_CREAT);	/*添加UNI、BRG端口ACL规则*/
			ospf_dcn_modify_overlay_vlan(uiLport);
		}
		else if(iflag == 1)
		{
			ospf_dcn_create_by_ifindex(ulIfindex);	//重新创建ospf网络
		}
        
        /*添加dcn 子接口带宽限制*/
        ospf_dcn_set_eth_bandwidth(uiLport, uiBand,uiVlanid,DCN_BAND_CREAT);

        /* UNI 、BRG新增acl规则 */
        
    	if(OK == ospf_dcn_check_port_role(ulIfindex))

        {
            iRtv = mpls_port_set_api(ulIfindex,MPLS_PORT_API_ADD,&ulIfindex);
    		ospf_logx(ospf_debug_dcn,"%d,mpls_port_set_api ifindex %d iRtv:%d\n",__LINE__,ulIfindex,iRtv);
        }
    }


        /*delete old vlan*/
    vlan_set_api(uiVlanOld, VLAN_ROW_STATUS, VLAN_INVALID);

    /*加入是lag的dcn，则删除并创建lag点0的子接口*/    
    lag_dcn_destroy_and_create();
        
	/*清dcn 邻居，重新触发*/
     lldp_dcn_rem_nbr_del();
	 ospf_dcn_if_set(OSPF_DCN_PROCESS,ulLbIp);


    return OK;
}

int ospf_dcn_vlan_get(u_int *uiVlan)
{
	int iRtv = 0;

    *uiVlan = stOspfDcn.uiVlanId;

    return OK;
}


/*修改IP调用函数*/
int ospf_dcn_ip_set(u_int uiIPAddr)
{
    u_int uiLport,uiFlag = 0, uiIfindex = 0;
    int ret = 0;
    u_long ulVpIfIndex;
    struct prefix_ipv4 stIp; 
    struct prefix stPrefx;
    u_long ulVlanid = 0;
    struct interface *ifp = NULL;
	
    memset(&stPrefx,0,sizeof(struct prefix));
    memset(&stIp,0,sizeof(struct prefix_ipv4));

    if(uiIPAddr == stDcnCfg.ulDeviceIp)
    {
        return OK;
    }

	/*是否设置过IP，若有则先删除*/
    if(0 != stDcnCfg.ulDeviceIp)
    {
		/*delete all subInterface old ip*/
		for(uiLport=0; uiLport<MAX_PORT_NUM; uiLport++)
		{
		
            if(VOS_OK != if_logic_port_to_index(uiLport, &uiIfindex))
            {
                continue;
            }

            /*接口被overlay使用*/
            if(OK == ospf_dcn_lookup_overlay_port(uiLport))
			{
				continue;
			}
            
            ret = ospf_dcn_ipaddr_uninstall(uiLport,IFINDEX_TYPE_VPIF);

    	    if(ret != VOS_ERR_NO_ERROR)
            {
    	  	    OSPF_DCN_LOG_WARNING("ospf_dcn_ipaddr_uninstall: uiLport=%d error!",uiLport);
            } 
  
		}
        ret = ospf_dcn_ipaddr_uninstall(0,IFINDEX_TYPE_LOOPBACK_IF);
	    if(ret != VOS_ERR_NO_ERROR)
        {
            OSPF_DCN_LOG_WARNING("ospf_dcn_ipaddr_uninstall: loopback error!");
        } 

    }

    /*create new ip*/   

	if(uiIPAddr != 0)
	{
	
		/*delete loopback old ip*/
		/*TODO:caoyong*/
		//ulVpIfIndex = dev_if_lbid_to_index(OSPF_DCNLOOPBACK_IFINDEX);
		
		if(OK == ospf_IntfIpAddr_get(ulVpIfIndex,&stPrefx))
		{
			stIp.prefix = stPrefx.u.prefix4;
			if(stIp.prefix.s_addr == ntohl(uiIPAddr))
			{
				uiFlag = 1;
			}
			else
			{
                ret = ospf_dcn_ipaddr_uninstall(0,IFINDEX_TYPE_LOOPBACK_IF);
				if(ret != VOS_ERR_NO_ERROR)
				{
                    zlog_info(MTYPE_OSPF,"ospf_dcn_ipaddr_uninstall: loopback error!\n");
                    OSPF_DCN_LOG_WARNING( "ospf_dcn_ipaddr_uninstall: loopback error!");
                } 
		
			}
		
		}
		
		if(uiFlag != 1)
		{
			/*lpbk ip set*/
            ret = ospf_dcn_ipaddr_install(0,IFINDEX_TYPE_LOOPBACK_IF,uiIPAddr,OSPF_DCN_MASK_LEN);
			if(ret != VOS_ERR_NO_ERROR)
			{
    	  	    zlog_info(MTYPE_OSPF,"ospf_dcn_ipaddr_install loopback: uiIPAddr=0x%x error!\n",uiIPAddr);
    	  	    OSPF_DCN_LOG_WARNING("ospf_dcn_ipaddr_install loopback: uiIPAddr=0x%x error!\n",uiIPAddr);
            } 
		}
	    /*create subInterface ip*/
	    for(uiLport=0; uiLport<MAX_PORT_NUM; uiLport++)
	    {
            if(VOS_OK != if_logic_port_to_index(uiLport, &uiIfindex))
            {
                continue;
            }

            /*接口被overlay使用*/
            if(OK == ospf_dcn_lookup_overlay_port(uiLport))
			{
				continue;
			}
            /*caoyong deleted 2017.9.15*/   
            /*TODO:以后修改基于lag的dcn*/
            #if 0
			if(NULL != eth_trunk_get_by_logic_port(uiLport))
			{
                continue;
			}
			#endif
        	if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLport,OSPF_DCN_SUBPORT,&ulVpIfIndex))
            {
                continue;
            }
            ulVlanid = stOspfDcn.uiVlanId;
            zebra_if_get_api(ulVpIfIndex,ZEBRA_IF_GET_IF_BY_IF_INDEX,ifp);
            if(NULL == ifp)
            {
                zlog_info(MTYPE_OSPF," if_get_by_index: Ifindex=0x%x failed!\n",ulVpIfIndex);
                OSPF_DCN_LOG_WARNING(" if_get_by_index: VpIfindex=%x,uiLport=%d failed!",ulVpIfIndex,uiLport);
                return ERR;
            }
            
            /*vlan hw set*/
            #if 0
            if(ERR == l3if_encap_add (ifp, ulVlanid))
            {
                zlog_info(MTYPE_OSPF," l3if_encap_add: Ifindex=0x%x ,ulVlanid=%d failed!\n",ulVpIfIndex,ulVlanid);
                OSPF_DCN_LOG_WARNING(" l3if_encap_add: VpIfindex=%x,uiLport=%d,ulVlanid=%d failed!",ulVpIfIndex,uiLport,ulVlanid);
                return ERR;
            }
            #endif
            ret = ospf_dcn_ipaddr_install(uiLport,IFINDEX_TYPE_VPIF,uiIPAddr,OSPF_DCN_MASK_LEN);
            if(ret != VOS_ERR_NO_ERROR)
            {
    	  	    zlog_info(MTYPE_OSPF,"ospf_dcn_ipaddr_install uiLport=%d, uiIPAddr=0x%x error!\n",uiLport,uiIPAddr);
    	  	    OSPF_DCN_LOG_WARNING("ospf_dcn_ipaddr_install uiLport=%d, uiIPAddr=%x error!",uiLport,uiIPAddr);
            }
	    }
        
	}
    return OK;
}

void lag_dcn_destroy_and_create()  
{
    u_long ulLPortNum = 0;

    /*caoyong deleted 2017.9.15*/
    /*TODO:以后修改基于lag的dcn*/
    #ifdef OSPF_DCN
	ETH_TRUNK_GROUP_T *pstTrunkGroup = NULL;
    
	for(ulLPortNum = 0; ulLPortNum < MAX_PORT_NUM; ulLPortNum++)
	{
        pstTrunkGroup = eth_trunk_get_by_logic_port(ulLPortNum);
        if(!pstTrunkGroup)
        {
            continue;
        }
        
        if(OK == ospf_dcn_lag_vpif_destory(pstTrunkGroup->usTrunkId))
        {
            ospf_dcn_lag_vpif_create(pstTrunkGroup->usTrunkId);
        }       
    }
    #endif
}

int ospf_dcn_lpbk_interface_creat(u_int uilpid)
{
    struct interface *ifp  = NULL;
    u_int uiIfIndex = 0;
    struct prefix_ipv4 stIp; 
    u_long ulLbIp = 0;
    u_char ucLpDesc[50] = "DCN_loopback_interface";
    int ret = 0;
    memset(&stIp,0,sizeof(struct prefix_ipv4));

	if(VOS_ERR == if_loopback_id_to_index(uilpid,&uiIfIndex))
    {
        return ERR;
    }
	if(if_loopback_id_to_index(uilpid,&uiIfIndex) != VOS_OK)
	{
        return ERR;
	}
	
    ulLbIp = stOspfDcn.ulDeviceIp;

	zebra_if_get_api(uiIfIndex,ZEBRA_IF_GET_IF_BY_IF_INDEX,ifp);
	if(NULL == ifp)
	{
	    zlog_info(MTYPE_OSPF," if_get_by_index: Ifindex=0x%x failed!\n",uiIfIndex);
	    OSPF_DCN_LOG_WARNING(" if_get_by_index: Ifindex=%x failed!",uiIfIndex);
		return ERR;
	}
	if (ifp->ulIfIndex == IFINDEX_INTERNAL)
	/* Is this really necessary?  Shouldn't status be initialized to 0
	   in that case? */
		UNSET_FLAG (ifp->status, INTERFACE_ACTIVE);

    /*ip set*/
    ret = ospf_dcn_ipaddr_install(0,IFINDEX_TYPE_LOOPBACK_IF,ulLbIp,OSPF_DCN_MASK_LEN);
    
	if (ifp->desc)
	  free(ifp->desc);
	ifp->desc = XSTRDUP(MTYPE_INTERFACE, ucLpDesc);

	return ret;
}

int ospf_dcn_Creat_vpif_interface(unsigned int uiIfIndex)
{
	struct interface *ifp  = NULL;

	
	zebra_if_get_api(uiIfIndex,ZEBRA_IF_GET_IF_BY_IF_INDEX,ifp);
	if(NULL == ifp)
	{
		return ERR;
	}
	if (ifp->ulIfIndex == IFINDEX_INTERNAL)
	/* Is this really necessary?  Shouldn't status be initialized to 0
	   in that case? */
		UNSET_FLAG (ifp->status, INTERFACE_ACTIVE);

	return OK;
}
int ospf_dcn_Del_vpif_interface(unsigned int uiIfIndex)
{
	struct interface *ifp  = NULL;

	ifp = if_lookup_by_index(uiIfIndex);
	if(ifp != NULL)
	{
		if_delete(ifp);
	}

}


u_int ospf_dcn_vpif_lport_subid_to_index(u_int uiLportNo, u_int uiSubid)
{
	if (MAX_PORT_NUM <= uiLportNo)
	{
	    return IFINDEX_INTERNAL;
	}

	if(MAX_SUBID_NUM <= uiSubid)
	{
	    return IFINDEX_INTERNAL;
	}   

	return VPORT_IF_IFINDEX_GENERATE(IFINDEX_TYPE_VPIF, uiLportNo, uiSubid);
}

/*修改全局vlan值*/
int ospf_dcn_set_global_vlan(u_long ulVlan)
{
 	int i= 0,iRtv = 0;
	u_long ulIfIndex = 0;
		
	iRtv = ospf_dcn_get_globale_enable();
	if(iRtv != OSPF_DCN_ENABLE)
	{
		return ERR;
	}	
	stOspfDcn.uiVlanId = ulVlan;
	return OK;
}

int ospf_dcn_create_process()
{
	tOSPF_NETWORK_INDEX	stOspfNetWorkIndex,*p_index = NULL;
    u_long ulLbIp = 0,ulValue = 0;

    struct ospf_area *p_area = NULL;
	struct ospf_process *p_process = NULL;

    ulLbIp = stOspfDcn.ulDeviceIp;

	stOspfNetWorkIndex.area = 0;
	stOspfNetWorkIndex.mask = OSPF_DCNDEF_IPMASK;
	stOspfNetWorkIndex.network = ulLbIp;
	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
	ulValue = TRUE;
    p_index = &stOspfNetWorkIndex;
    
	p_process = ospf_get_nm_process(p_index);
	if (NULL == p_process)
	{
        OSPF_DCN_LOG_WARNING("ospf_get_nm_process process is null.");
		return ERR;
	}
	/*dcn opaque使能默认开启*/
	p_process->opaque = TRUE;
	ospf_logx(ospf_debug, "ospf_dcn_create_network \r\n");
    if (NULL == ospf_network_add(p_process, p_index->network, p_index->mask, p_index->area))
    {
        ospf_logx(ospf_debug, "OSPF_NETWORK_STATUS NULL == ospf\r\n");
        OSPF_DCN_LOG_WARNING("OSPF_NETWORK_STATUS NULL == ospf.");
        return ERR;
    }
               
     p_area = ospf_area_lookup(p_process,  p_index->area);
     if (NULL == p_area)
     {
         p_area = ospf_area_create(p_process, p_index->area);
     }
  	if(ospfSetApi(OSPF_DCN_PROCESS,OSPF_GBL_ROUTERID,&ulLbIp)!=OK)
  	{
  		ospf_logx(ospf_debug_dcn," %%Failed to disable ospf:%d \r\n",__LINE__);
  		OSPF_DCN_LOG_WARNING("Failed to disable ospf,ulLbIp=%x.",ulLbIp);          		
  	}
#ifndef OSPF_VPN
	ospf_sys_netmaskbingvrf(p_process->process_id,p_index->network,p_index->mask);
#endif
    return OK;

}

/*****************************************************************************
 函 数 名  : ospf_dcn_initSuptAddrList
 功能描述  :获取端口接收数据标志位值
 输入参数  : uchPort - 端口索引
 输出参数  : 无
 返 回 值  : 端口标志位值或NULL

 修改历史      :
*****************************************************************************/
int ospf_dcn_initSuptAddrList()
{
	int uiSlot = 0,iRet = 0,iRtv = 0;
	int uiCardType = 0;
	u_long ulIfIndx = 0,uLport = 0,ulSubIndx = 0;
	u_int uiVlan = 0;

    iRtv =ospf_dcn_get_flag();
	if(iRtv == OK)
    {
        return OK;
    }  

    /*创建dcn network*/
	iRtv = ospf_dcn_create_process();
    if(iRtv == ERR)
	{
	    OSPF_DCN_LOG_WARNING("Failed to create dcn process net.");
		return ERR;
	}

	for(uLport = 0; uLport < MAX_PORT_NUM; uLport++)
	{
	    if(VOS_OK != if_logic_port_to_index(uLport, &ulIfIndx))
	    {
	        continue;
	    }

        iRet =ospf_dcn_set_dcn_mode_by_ifindex(ulIfIndx,L3_IF_DCN_MODE_P2P);
        ospf_dcn_create_by_ifindex(ulIfIndx);    /*创建单个dcn if网络*/
        ospf_dcn_addr_update(uLport);
        ospf_dcn_vlan_get(&uiVlan);
        /*配置dcn 子接口带宽默认值*/
        ospf_dcn_set_eth_bandwidth(uLport, DCN_BANDWIDTH_DEF,uiVlan,DCN_BAND_CREAT);


	}

    return OK;
}


/*根据端口索引配DCN MODE,ulIfIndex:接口索引*/
int ospf_dcn_set_dcn_mode_by_ifindex(u_long ulIfIndex, u_long ulDcnMode)
{    
	u_char ucPortRole = 0,ucPort = 0;
	int i = 0,ret = 0;
	u_int uiSdkIndex = 0,ulLagIfIndex = 0,uiDcnMod = 0;
	#if 0
	if(OK != uspIfPortGetApi(ulIfIndex, PORT_API_PORT_ROLE, &ucPortRole))
	{
		ospf_logx(ospf_debug_dcn,"%s:%d,get ifindex %d role err\n", __FUNCTION__,__LINE__,ulIfIndex);
		OSPF_DCN_LOG_WARNING("get ifindex=%x role err.",ulIfIndex);
		return ERR;
	}
	#endif
	if(VOS_ERR == if_index_to_logic_port(ulIfIndex, &ucPort))
    {
        return ERR;
    }

	uiSdkIndex = ospf_dcn_vpif_lport_subid_to_index(ucPort,OSPF_DCN_SUBPORT);//子接口索引,例如ge0/0/1.1:0x160201
	if (uiSdkIndex != IFINDEX_INTERNAL)
	{
		zebra_if_ospf_get_api(uiSdkIndex, ZEBRA_IF_GET_DCN_MODE, &uiDcnMod);
		if(uiDcnMod != ulDcnMode)
		{
			ret = zebra_if_ospf_get_api(uiSdkIndex, ZEBRA_IF_GET_DCN_MODE, &ulDcnMode);
			ospf_logx(ospf_debug_dcn,"%d,ret= %d,uiSdkIndex = 0x%x, ulDcnMode=%d\n",__LINE__,ret,uiSdkIndex,ulDcnMode);
			
			return ret;
		}
	}
}



int ospf_dcn_delete_network(u_int ulIp)
{
    int value_l = 0;
    tOSPF_NETWORK_INDEX	stOspfNetWorkIndex = {0};

	stOspfNetWorkIndex.area = 0;
	stOspfNetWorkIndex.mask = OSPF_DCNDEF_IPMASK;
	stOspfNetWorkIndex.network = ulIp;
	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
    value_l = FALSE;
	if(ospfNetworkSetApi(&stOspfNetWorkIndex,OSPF_NETWORK_STATUS,&value_l) != OK)
	{
		 ospf_logx(ospf_debug_dcn,"	ospf_dcn_delete_network err\r\n");
		 OSPF_DCN_LOG_WARNING("ospf_dcn_delete_network err ulIp=%x.",ulIp);
		 return CMD_WARNING;
	}	

}
#if 0
int ospf_dcn_create_network(u_int ulIp)
{
    int value_l = 0;
    tOSPF_NETWORK_INDEX	stOspfNetWorkIndex = {0};

	stOspfNetWorkIndex.area = 0;
	stOspfNetWorkIndex.mask = OSPF_DCNDEF_IPMASK;
	stOspfNetWorkIndex.network = ulIp;
	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
    value_l = TRUE;

	ospf_dcn_change_all_network(&stOspfNetWorkIndex);

}
#endif

/*根据端口索引校验是否支持DCN功能，
目前仅支持光口及电口*/
int ospf_dcn_check_by_ifindex(u_long ulIfIndx)
{
	u_int uiLport = 0;
	int ret = 0;
	
	if(VOS_ERR == if_index_to_logic_port(ulIfIndx, &uiLport))
    {
        return ERR;
    }

	return OK;
}

/*添加接口role校验，仅校验UNI及BRG口*/
int ospf_dcn_check_port_role(u_long ulIfIndx)
{
    u_char ucPortRole = 0;


	if(OK != port_get_api(ulIfIndx, PORT_API_PORT_ROLE, &ucPortRole))
	{
		ospf_logx(ospf_debug_dcn,"%d,get ifindex %d role err\n",__LINE__,ulIfIndx);
		OSPF_DCN_LOG_WARNING("get ifindex=%x role err.",ulIfIndx);
		return ERR;
	}
    if((PORT_ROLE_UNI == ucPortRole)||(PORT_ROLE_BRIDGE == ucPortRole))
    {
        return OK;
    }
    return ERR;

}


int ospf_set_acl(u_int ulIfIndx, u_int uiMode)
{
	int ret = 0,iRuleid = 0;
	int i;
	u_int uilport = 0;

    
    ret = ospf_dcn_check_port_role(ulIfIndx);
    if (ret != OK)
    {
        return OK;   /*非UNI及BRG口直接返回成功*/
    }

    ret = ospf_dcn_get_port_enable_status(ulIfIndx);//接口调用
	if((ret != OSPF_DCN_PORT_ENABLE)&&(OSPF_RULE_CREAT == uiMode))
	{
        return OK;   /*接口使能未开启直接返回成功*/
	}

    if(VOS_ERR == if_index_to_logic_port(ulIfIndx, &uilport))
    {
        OSPF_DCN_LOG_WARNING("if_index_to_logic_port lport is invaild ulIfIndx=%x.",ulIfIndx);
        return ERR;
    }

#ifdef HW

    /*创建规则*/
    if(OSPF_RULE_CREAT == uiMode)
    {
        ret = port_acl_ospf_uni_in_add(uilport);
    //    printf("ospf_set_acl OSPF_RULE_CREAT%d :ret = %d\n",__LINE__,ret);
    }
    else
    {
        ret = port_acl_ospf_in_del(uilport);
     //   printf("ospf_set_acl OSPF_RULE__DEL%d :ret = %d\n",__LINE__,ret);
    }
#endif

	return ret;	
}

int ospf_dcn_delete_acl()
{
    u_int uiLport = 0,ulIfindex = 0;
    int iRtv = 0;
    for(uiLport=1; uiLport<MAX_PORT_NUM+1; uiLport++)
    {
        if(VOS_OK != if_logic_port_to_index(uiLport, &ulIfindex))
        {
            continue;
        }

        iRtv = ospf_set_acl(ulIfindex,OSPF_RULE__DEL);   /*删除UNI、BRG端口ACL规则*/
      //  printf("ospf_dcn_delete_acl OSPF_RULE__DEL uiLport=%d,iRtv=%d\n",uiLport,iRtv);
    }
    return iRtv;
}

int ospf_dcn_overlay_ipaddr_install(u_int uiLport,u_long ulAddr,u_long ulMasklen,u_char ucMode)
{
    u_int ulIfindex = 0;
    u_int ulIpAddr = 0;
    int iRtv = 0,iFlag = 0;
    u_int ulMask = 0,ulLbIp = 0,ulVpIfIndex = 0;
    struct prefix stPrefx;

    masklen2ip(ulMasklen, &ulMask);
    ulMask = ntohl(ulMask);

    memset(&stPrefx,0,sizeof(struct prefix));
    if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLport,OSPF_DCN_SUBPORT,&ulVpIfIndex))
    {
        return ERR;
    }
    iRtv = if_logic_port_to_index(uiLport, &ulIfindex);
    if(iRtv != VOS_OK)
    {
        return iRtv;
    }
    
    iRtv = ospf_IntfIpAddr_get(ulVpIfIndex,&stPrefx);
    ulIpAddr = (u_long)htonl(stPrefx.u.prefix4.s_addr);   

	/*接口ip地址存在*/
    if (OK == iRtv)
	{
    	/*ip地址相同不下发*/
		if(ulIpAddr == ulAddr)
		{
	    	if(ERR == ospf_dcn_overlay_network_get(ulAddr,ulMask))
			{
				iRtv = ospf_dcn_overlay_network_set(ulAddr,ulMask);
			}
			return iRtv;
    	}
		
		/*若新ip与原接口ip位于同一网段内，则直接删p_if*/
		else if((ulIpAddr&ulMask) == (ulAddr&ulMask))
		{
			ospf_dcn_delete_by_ifindex_ip(ulIfindex,ulIpAddr);
		//	printf("22 ospf_dcn_delete_by_ifindex_ip: ulIfindex=%x,ulIpAddr=%x\n",ulIfindex,ulIpAddr);
			iFlag = 1;
		}
		else 	/*若接口使能dcn，则直接删p_if*/
		{
			iRtv = ospf_dcn_get_port_enable_status(ulIfindex);//接口
			if(iRtv == OSPF_DCN_PORT_ENABLE) 
			{
				/*删除p_if */
				iRtv = ospf_dcn_delete_by_ifindex_ip(ulIfindex,ulIpAddr);
				iFlag = 1;
			}   
		}
	}


    iRtv = ospf_dcn_ipaddr_uninstall(uiLport,IFINDEX_TYPE_VPIF);   
 //   printf("22 ospf_dcn_overlay_ipaddr_install: ospf_dcn_ipaddr_uninstall,iRtv=%d\n",iRtv);

    if(iRtv == OK)
    {
        iRtv = ospf_dcn_ipaddr_install(uiLport,IFINDEX_TYPE_VPIF,ulAddr,ulMasklen);

  //      printf("ospf_dcn_overlay_ipaddr_install: ospf_dcn_ipaddr_install,iRtv=%d\n",iRtv);
		if(iRtv == OK)
        {
            iRtv = ospf_dcn_set_overlay(uiLport,ulAddr,ulMasklen,ucMode);
        }
        if(OK == iRtv)
        {
			if(ERR == ospf_dcn_overlay_network_get(ulAddr,ulMask))
			{
				iRtv = ospf_dcn_overlay_network_set(ulAddr,ulMask);
				if(iRtv != OK)
		        {
		            iRtv = ospf_dcn_set_overlay(uiLport,0,0,DCN_OVERLAY_DEL);
		        }
			}
			else
			{
				/*网络已存在，仅ip改变，创建p_if*/
				if(iFlag == 1)
				{
					/*创建overlay p_if*/
					ospf_dcn_create_overlay_by_ip_logic_port(ulAddr,uiLport);
				}

			}

     //       printf("ospf_dcn_overlay_ipaddr_install: ospf_dcn_overlay_network_set,iRtv=%d\n",iRtv);

        }


    }

    return iRtv;
}


int ospf_dcn_overlay_ipaddr_uninstall(u_int uiLport,u_long ulAddr,u_long ulMasklen,u_char ucMode)
{
    u_int ulIfindex = 0;
    int iRtv = 0,iRet = 0;
    u_int ulMask = 0,ulLbIp = 0,ulVpIfIndex = 0;
    u_int ulIpAddr = 0;
    struct prefix stPrefx;
    
    memset(&stPrefx,0,sizeof(struct prefix));
    
    if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLport,OSPF_DCN_SUBPORT,&ulVpIfIndex))
    {
        return ERR;
    }
    
    iRtv = if_logic_port_to_index(uiLport, &ulIfindex);
    if(iRtv != VOS_OK)
    {
        return iRtv;
    }
    
    ulLbIp = stOspfDcn.ulDeviceIp;
    iRtv = ospf_IntfIpAddr_get(ulVpIfIndex,&stPrefx);
    ulIpAddr = (u_long)htonl(stPrefx.u.prefix4.s_addr);    
    /*ip地址不存在或与要删除的不一致直接返回*/
    if((OK != iRtv)
        ||(ulIpAddr != ulAddr))
    {
        return OK;
    }

    iRtv = ospf_dcn_ipaddr_uninstall(uiLport,IFINDEX_TYPE_VPIF);
    if(iRtv == OK)
    {
        masklen2ip(ulMasklen, &ulMask);
        ulMask = ntohl(ulMask);
		
		/*删除overlay network*/
		if(OK == ospf_dcn_overlay_network_get(ulAddr,ulMask))
		{
			/*先删除p_if,再通过定时器触发删除network*/
			ospf_dcn_delete_by_ifindex_ip(ulIfindex,ulIpAddr);
        	iRtv = ospf_dcn_overlay_network_del(ulAddr,ulMask);
		}
        iRtv = ospf_dcn_ipaddr_install(uiLport,IFINDEX_TYPE_VPIF,ulLbIp,OSPF_DCN_MASK_LEN);
        if(iRtv != OK)
        {
            return iRtv;
        }
        iRtv = ospf_dcn_set_overlay(uiLport,0,0,ucMode);

        if(iRtv == OK)
        {
            iRet = ospf_dcn_get_port_enable_status(ulIfindex);//接口
			if(iRet == OSPF_DCN_PORT_ENABLE) 
            {
               // iRtv = ospf_dcn_create_by_ifindex(ulIfindex);
               
			    ulLbIp = stOspfDcn.ulDeviceIp;
				iRtv = ospf_dcn_delete_by_ifindex(ulLbIp,uiLport);

            }   
            else
            {
                return OK;
            }
        }

    }

    return iRtv;
}


/*lldp邻居删除时overlay接口还原状态*/
int lldp_overlay_ipaddr_aging(u_int uiLport)
{
    u_int ulIfindex = 0;
    int iRtv = 0;
    u_int ulMask = 0,ulLbIp = 0,ulVpIfIndex = 0;
    u_int ulIpAddr = 0;
    struct prefix stPrefx;
    
    memset(&stPrefx,0,sizeof(struct prefix));
    if(VOS_ERR == if_vport_if_logic_port_subid_to_index(uiLport,OSPF_DCN_SUBPORT,&ulVpIfIndex))
    {
        return ERR;
    }
    iRtv = if_logic_port_to_index(uiLport, &ulIfindex);
    if(iRtv != VOS_OK)
    {
        return iRtv;
    }
    
    ulLbIp = stOspfDcn.ulDeviceIp;

    iRtv = ospf_IntfIpAddr_get(ulVpIfIndex,&stPrefx);
    ulIpAddr = (u_long)htonl(stPrefx.u.prefix4.s_addr);  
    
    /*ip地址不存在或与dcn ip 一致直接返回*/
    if((OK != iRtv)
        ||(ulIpAddr == ulLbIp))
    {
        return OK;
    }

    iRtv = ospf_dcn_ipaddr_uninstall(uiLport,IFINDEX_TYPE_VPIF);
    if(iRtv == OK)
    {
        masklen2ip(OSPF_DCN_OVERLAY_MASK_LEN, &ulMask);
        ulMask = ntohl(ulMask);
        ulIpAddr &= ulMask;
        iRtv = ospf_dcn_overlay_network_del(ulIpAddr,ulMask);/*删除overlay network*/
        iRtv = ospf_dcn_ipaddr_install(uiLport,IFINDEX_TYPE_VPIF,ulLbIp,OSPF_DCN_MASK_LEN);
        if(iRtv != OK)
        {
            return iRtv;
        }
    	iRtv = ospf_dcn_get_port_enable_status(ulIfindex);//接口
    	if(iRtv !=  OSPF_DCN_PORT_DISABLE) 
        {
            iRtv = ospf_dcn_create_by_ifindex(ulIfindex);
        }   
        else
        {
            return OK;
        }
    }

    return iRtv;
}
/*查询overlay模型network配置*/
int ospf_dcn_overlay_network_get(u_int ulNetWorkIp,u_int ulNetWorkMask)
{
    int value_l = 0,ret = OK;
	struct ospf_network search;
    struct ospf_process start_process;
    struct ospf_network *p_network = NULL;


    start_process.process_id = OSPF_DCN_PROCESS;
    search.p_process = &start_process;
    search.area_id = OSPF_DCN_AREA_ID;
    search.dest = (ulNetWorkIp)&(ulNetWorkMask);
    search.mask = ulNetWorkMask;

    p_network = ospf_lstlookup(&ospf.nm.network_table, &search);
    if (NULL == p_network)
	{
	    OSPF_DCN_LOG_WARNING("Failed to lookup p_network.");
		ret = ERR;
	}
	return ret;

	
}

/*配置overlay模型network*/
int ospf_dcn_overlay_network_set(u_int ulNetWorkIp,u_int ulNetWorkMask)
{
    int value_l = 0;
    tOSPF_NETWORK_INDEX stOspfNetWorkIndex = {0};
	struct ospf_process *p_process = NULL;


	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
	
    p_process = ospf_get_nm_process(&stOspfNetWorkIndex);
    if (NULL == p_process)
	{
	    OSPF_DCN_LOG_WARNING("Failed to get nm process,process is null.");
		return ERR;
	}
	p_process->uiOverlayNet = (ulNetWorkIp)&(ulNetWorkMask);

	ospf_dcn_overlay_network_timer(p_process,DCN_OVERLAY_NET_ADD);

    return OK;

	
}



/*删除overlay模型network配置*/
int ospf_dcn_overlay_network_del(u_int ulNetWorkIp,u_int ulNetWorkMask)
{
    int value_l = 0;
    tOSPF_NETWORK_INDEX stOspfNetWorkIndex = {0};
	struct ospf_process *p_process = NULL;


	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
	
    p_process = ospf_get_nm_process(&stOspfNetWorkIndex);
    if (NULL == p_process)
	{
	    OSPF_DCN_LOG_WARNING("Failed to get nm process,process is null.");
		return ERR;
	}
	p_process->uiOverlayNet = (ulNetWorkIp)&(ulNetWorkMask);

	ospf_dcn_overlay_network_timer(p_process,DCN_OVERLAY_NET_DEL);

    return OK;
}
int ospf_dcn_overlay_cfg_save(OSPF_DCN_OVERLAY_T *pstOverlay)
{
    u_long uLport = 0,ulIfIndx = 0;
    OSPF_DCN_OVERLAY_CFG_T *pstCfg = NULL;
    int ret = 0;
    OSPF_DCN_OVERLAY_T stOverStu;

    memset(&stOverStu,0,sizeof(OSPF_DCN_OVERLAY_T));
    pstCfg = stOverStu.stOverlayConfig;
	for(uLport = 0; uLport < MAX_PORT_NUM; uLport++)
	{
	    if(VOS_OK != if_logic_port_to_index(uLport, &ulIfIndx))
	    {
	        continue;
	    }
	    
        ret = ospf_dcn_get_overlay_port_config(uLport,pstCfg);
        if(ret == OK)
        {
            pstCfg++;
            stOverStu.ucOverlayNum++;
        }
    }
    if(stOverStu.ucOverlayNum != 0)
    {
        memcpy(pstOverlay,&stOverStu,sizeof(OSPF_DCN_OVERLAY_T));
        return OK;
    }
    return ERR;
}

int ospf_dcn_local_manage_cfg(u_int uiIfIndex,u_char ucMode)
{
	u_int ulNetWorkIp = 0;
	u_int ulNetWorkMask = 0;
	u_int i = 0;
	struct prefix stPrefx;
	tOSPF_NETWORK_INDEX stOspfNet;
    tOSPF_LSA_NET stRout;
    int ret = 0;
    
    if(DCN_PORT_LOCAL_MNG_ADD == ucMode)	
    {
    	if(OK == ospf_IntfIpAddr_get(uiIfIndex,&stPrefx))
    	{
    	 	ulNetWorkIp = ntohl(stPrefx.u.prefix4.s_addr);
    	 	masklen2ip(stPrefx.prefixlen, &ulNetWorkMask);
    		ulNetWorkMask = ntohl(ulNetWorkMask);
        }
    	if(ulNetWorkIp && ulNetWorkMask)
    	{
    	    memset(&stRout,0,sizeof(tOSPF_LSA_NET));
            
            stRout.uiIfIndx = uiIfIndex;
    		stRout.uiIp = ulNetWorkIp;
    		stRout.uiMask= ulNetWorkMask;
            ret = ospf_dcn_route_add(&stRout);
            if(ret != OK)
            {
                OSPF_DCN_LOG_WARNING("ospf_dcn_route_add err.");
                return ERR;
            }
            
            stOspfNet.process_id = OSPF_DCN_PROCESS;
            stOspfNet.area = 0;
            stOspfNet.mask = OSPF_DCNDEF_IPMASK;
            stOspfNet.network= ulNetWorkIp;
            ret = ospf_dcn_network_creat(&stOspfNet);  
            if(ret != OK)
            {
                OSPF_DCN_LOG_WARNING("ospf_dcn_network_creat err.");
                return ERR;
            }
            ospf_lsa_update(OSPF_DCN_PROCESS);

    	}
        else
        {
            return ERR;
        }     
    }
    else
    {

    	if(stOspfLsaManage.ucEn != OSPF_LSA_MNG_EN)
    	{
            return ERR;
        }
        memset(&stRout,0,sizeof(tOSPF_LSA_NET));
        
        stRout.uiIfIndx = uiIfIndex;
        ret = ospf_dcn_route_del(&stRout);

        if(ret == OK)
        {
            ospf_lsa_update(OSPF_DCN_PROCESS);
        }

    }
    return ret;
}

int ospf_dcn_local_manage_get(u_int uiIfIndex,tOSPF_LSA_NET *pstLsaMng)
{
	u_long ulNetWorkIp = 0;
	u_long ulNetWorkMask = 0;
	u_int i = 0;
	struct prefix stPrefx;
	tOSPF_NETWORK_INDEX stOspfNet;
    tOSPF_LSA_NET stRout;
    int ret = 0;

    if (stOspfLsaManage.ucEn != OSPF_LSA_MNG_EN)
	{
	    return ERR;
    }
	for(i = 0;i < stOspfLsaManage.uiCnt;i++)
	{
        if(stOspfLsaManage.stNet[i].uiIfIndx == 0)
        {
            continue;
        }
        if((stOspfLsaManage.stNet[i].uiIfIndx == uiIfIndex)
            &&(stOspfLsaManage.stNet[i].uiIp && stOspfLsaManage.stNet[i].uiMask))
        {
            memcpy(pstLsaMng,&stOspfLsaManage.stNet[i],sizeof(tOSPF_LSA_NET));
            return OK;        
        }

    }
    return ERR;

}

/*overlay模型中hello报文非master报文过滤处理.*/
int ospf_hello_overlay_check (struct ospf_if *p_if,u_int uiNbrAddr,u_int uiNbrRouid)
{
#if 0
    u_int uiLportNo = 0;
    u_long ulIpAddr = 0,ulMasklen = 0,uiMasterIp = 0;
    int ret = 0,iflag = 0;
    struct ospf_nbr *p_nbr = NULL;
    struct ospf_nbr *p_next = NULL;

    if(VOS_ERR== if_vport_ifindex_to_logic_port(p_if->ifnet_uint,&uiLportNo))
    {
        return ERR;
    }


    iflag = stDcnCfg.stOverlayCfg[uiLportNo].ucMngflag;
	
	//printf("00 ospf_hello_overlay_check:ifnet_uint=%x,uiNbrAddr=%x,iflag=%d\n",p_if->ifnet_uint,uiNbrAddr,iflag);
	switch(iflag)
	{
		/*slaver过滤其他非master ip报文*/
		case DCN_OVERLAY_SLAVER_ADD:
		{
	        uiMasterIp = (uiNbrAddr & OSPF_DCN_OVERLAY_IPMASK)+LLDP_OVERLAY_MASTER_IPADDR_MNG;
	     //   printf("22 ospf_hello_overlay_check:uiMasterIp=%x, uiNbrAddr=%x\n",uiMasterIp ,uiNbrAddr);
	        if(uiMasterIp != uiNbrAddr)
	        {
	            return ERR;
	        }
	        else
	        {
	            for_each_ospf_nbr(p_if, p_nbr, p_next)
	            {
	                if(p_nbr == NULL)
	                {
	                    continue;
	                }
					

	             //   printf("33 interface nbr :%x\n",p_nbr->addr);
	             
					/*删除slaver接口上非overlay网段所有邻居*/
	                if((p_nbr->addr & 0xffffff00)!=(uiNbrAddr & 0xffffff00))
	                {
	             //       ospf_logx(1,"interface already in overlay slave mode , delete nbr :%x\n",p_nbr->addr);
	                    ospf_nsm(p_nbr, OSPF_NE_LL_DOWN);
	                    ospf_nbr_delete(p_nbr);

	                }
	            }

	        }
			break;
    	}
		
		/*当slaver 网元ip地址改变时，master删除网元旧ip邻居*/
		case DCN_OVERLAY_MASTER_ADD:
		{
			for_each_ospf_nbr(p_if, p_nbr, p_next)
            {
                if(p_nbr == NULL)
                {
                    continue;
                }
				
				//printf("33 ospf_hello_overlay_check:p_nbr->id=%x, addr=%x,uiNbrRouid=%x,uiNbrAddr=%x\n",p_nbr->id,p_nbr->addr,uiNbrRouid,uiNbrAddr);
				/*若接口邻居已存在该网元，但ip改变，则删除老nbr*/
				if((p_nbr->id == uiNbrRouid)
					&&(p_nbr->addr != uiNbrAddr))
                {
                    ospf_nsm(p_nbr, OSPF_NE_LL_DOWN);
					//printf("ospf_hello_overlay_check:p_nbr all already exist,delete old nbr:%x\n",p_nbr);
                    ospf_nbr_delete(p_nbr);
                }
				
            }
			break;
		}
		default:
			break;

	}
#endif    
    return OK;
}


int ospf_overlay_if_master_check (struct ospf_if *p_if)
{
	u_int uiLportNo = 0;
	u_long ulIpAddr = 0,ulMasklen = 0,uiMasterIp = 0;
	int ret = 0,iflag = 0;
	struct ospf_nbr *p_nbr = NULL;
	struct ospf_nbr *p_next = NULL;

	if(VOS_ERR== if_vport_ifindex_to_logic_port(p_if->ifnet_uint,&uiLportNo))
    {
        return ERR;
    }
    
	if((p_if->p_process->process_id != OSPF_DCN_PROCESS)
		||(p_if->ulDcnflag == OSPF_DCN_FLAG))
	{
		return ERR;
	}
	/*slaver过滤*/

	iflag = stDcnCfg.stOverlayCfg[uiLportNo].ucMngflag;
	if(iflag == DCN_OVERLAY_MASTER_ADD)
	{
		return OK;
	}
	
	return ERR;
}

int ospf_dcn_overlay_network_timer( struct ospf_process *p_process,u_int uiflag)
{
	p_process->overlay_update = uiflag;
    ospf_stimer_start(&p_process->overlay_timer, 1);
}
#endif
/*overlay模型network处理定时器*/
void ospf_dcn_overlay_network_timer_pro(struct ospf_process *p_process)
{
#ifdef OSPF_DCN /*caoyong delete 2017.9.20*/
    int value_l = 0;
	u_int uiFlag = 0;
    tOSPF_NETWORK_INDEX stOspfNetWorkIndex = {0};
	
	stOspfNetWorkIndex.area = OSPF_DCN_AREA_ID;
	stOspfNetWorkIndex.mask = OSPF_DCN_OVERLAY_IPMASK;
	stOspfNetWorkIndex.network = p_process->uiOverlayNet;
	stOspfNetWorkIndex.process_id = OSPF_DCN_PROCESS;
	
	uiFlag = p_process->overlay_update;
	if(uiFlag == DCN_OVERLAY_NET_ADD)
	{
		value_l = TRUE;
	}
	else if(uiFlag == DCN_OVERLAY_NET_DEL)
	{
		value_l = FALSE;
	}
	else
	{
		return;
	}
	
//	printf(" ospf_dcn_overlay_network_timer_pro :value_l=%d,ip = %x,mask = %x\n",value_l,stOspfNetWorkIndex.network,stOspfNetWorkIndex.mask);
	if(ospfNetworkSetApi(&stOspfNetWorkIndex,OSPF_NETWORK_STATUS,&value_l)!=OK)
	{
		 zlog_info(MTYPE_OSPF,"	ospf_dcn_overlay_network_timer_pro :failed to configure network,value_l=%d,ip = %x,mask = %x\n",value_l,stOspfNetWorkIndex.network,stOspfNetWorkIndex.mask);
	}	
    return ;
#endif
}
#ifdef OSPF_DCN
void ospf_dcn_debug_print(struct vty *vty)
{   
    int i,j;
    vty_out(vty,"%sstDcnCfg:%s",VTY_NEWLINE,VTY_NEWLINE);
    vty_out(vty,"ucDcnEnflag=%d,ucChangeFlag=%d,ucInitflag=%d%s",
        stDcnCfg.ucDcnEnflag,stDcnCfg.ucChangeFlag,stDcnCfg.ucInitflag,VTY_NEWLINE);
    
    vty_out(vty,"uiNeid=%d,ucDevType=%s,ucDevName=%s,uiNeType=%d,ulDeviceIp=%x,ulDeviceMask=%x,uiNePort=%d,uiVlanId=%d,uiBandwidth=%d%s",
        stDcnCfg.uiNeid,stDcnCfg.ucDevType,stDcnCfg.ucDevName,
        stDcnCfg.uiNeType,stDcnCfg.ulDeviceIp,stDcnCfg.ulDeviceMask,
        stDcnCfg.uiNePort,stDcnCfg.uiVlanId,stDcnCfg.uiBandwidth,VTY_NEWLINE);
    for(i=0;i<16;i++)
        for(j=0;i<64;j++)
            if(stDcnCfg.uiUsePort[i][j] != 0)
            {
                vty_out(vty,"stDcnCfg.uiUsePort:%d%s",stDcnCfg.uiUsePort[i][j],VTY_NEWLINE);
            }

    for(i=0;i<MAX_PORT_NUM;i++)
    {   
        if(stDcnCfg.stOverlayCfg[i].ucMngflag != 0)
        {
            vty_out(vty,"stOverlayCfg:i=%d,ucMngflag=%d,ulIfindex=%x,ulAddr=%x,ulMasklen=%d%s",
                i,stDcnCfg.stOverlayCfg[i].ucMngflag,
                stDcnCfg.stOverlayCfg[i].ulIfindex,
                stDcnCfg.stOverlayCfg[i].ulAddr,
                stDcnCfg.stOverlayCfg[i].ulMasklen,VTY_NEWLINE);
        }

    }
    
    for(i=0;i<100;i++)
    {   
        if(stDcnCfg.stOspfRxPkt[i].ulIfIndex != 0)
        {
            vty_out(vty,"stOspfRxPkt:i=%d,ucRecvFlag=%d,ucRecvCnt=%d,ulIfIndex=%x,ulAddr=%x%s",
                i,stDcnCfg.stOspfRxPkt[i].ucRecvFlag,
                stDcnCfg.stOspfRxPkt[i].ucRecvCnt,
                stDcnCfg.stOspfRxPkt[i].ulIfIndex,
                stDcnCfg.stOspfRxPkt[i].ulAddr,VTY_NEWLINE);
        }

    }
    
    for(i=0;i<MAX_PORT_NUM;i++)
    {   
        if(stDcnCfg.stOverlayCfg[i].ucMngflag != 0)
        {
            vty_out(vty,"stOverlayCfg:i=%d,ucMngflag=%d,ulIfindex=%x,ulAddr=%x,ulMasklen=%x%s",
                i,stDcnCfg.stOverlayCfg[i].ucMngflag,
                stDcnCfg.stOverlayCfg[i].ulIfindex,
                stDcnCfg.stOverlayCfg[i].ulAddr,
                stDcnCfg.stOverlayCfg[i].ulMasklen,VTY_NEWLINE);
        }

    }
}

/*undo dcn时删overlay相关配置*/
int ospf_dcn_del_overlay_config(u_long ulLPortNum)
{
    int ret = 0;
    u_int uiIpaddr = 0,uiMasklen = 0;    
    u_long ulIfIndex = 0,ulMask = 0,ulIpAddr = 0,ulLldpEn=0;

    
    /*过滤非overlay接口*/
    ret = ospf_dcn_lookup_overlay_port(ulLPortNum);
    if(OK != ret)
    {
        return OK;
    }

    if(VOS_OK != if_logic_port_to_index(ulLPortNum,&ulIfIndex))
    {
        return ERR;
    }
    
    /*删除主设备overlay相关配置*/
    ret = ospf_dcn_lookup_master_port(ulLPortNum,&uiIpaddr,&uiMasklen);
    if(OK == ret)
    {
        ret = ospf_dcn_overlay_ipaddr_uninstall(ulLPortNum,uiIpaddr,uiMasklen,DCN_OVERLAY_DEL);
        if(ret != OK)
        {
            zlog_info(MTYPE_OSPF,"ospf_dcn_overlay_ipaddr_uninstall failed!uiPort:%d,ulIpAddr:%x\n",ulLPortNum,ulIpAddr);
            OSPF_DCN_LOG_WARNING("ospf_dcn_overlay_ipaddr_uninstall failed!uiPort:%d,ulIpAddr:%x.",ulLPortNum,ulIpAddr);
            return ERR;
        }
        
        /*设置管理掩码*/
        ulMask = 0;

    	ret |= lldp_set_api(ulIfIndex,LLDP_PORT_MASTER_IP_MASK,&ulMask);
    	if(ret != OK)
    	{
    		zlog_info(MTYPE_OSPF,"lldp over-lay master ip address set failed!\n");
            OSPF_DCN_LOG_WARNING("lldp over-lay master ip address set failed!");
            return ERR;
        }        
        lldp_overlay_slave_clean_api(ulIfIndex,ulMask);
		
        ulIpAddr = 0;
        /*设置管理ip地址*/
    	ret = lldp_set_api(ulIfIndex,LLDP_PORT_MASTER_IP_ADDR,&ulIpAddr);
    	if(ret != OK)
    	{
    		zlog_info(MTYPE_OSPF,"lldp over-lay master ip address set failed!\n");
            OSPF_DCN_LOG_WARNING("lldp over-lay master ip address set failed!");
            return ERR;
        }
    	ulLldpEn = LLDP_MNG_NONE;
    	ret = lldp_set_api(ulIfIndex,LLDP_PORT_MASTER_CFG,&ulLldpEn);
    	if(ret != OK)
    	{
    		zlog_info(MTYPE_OSPF,"lldp over-lay master disable set failed!\n");
            OSPF_DCN_LOG_WARNING("lldp over-lay master disable set failed!");
            return ERR;
        }
        
    }
    
    /*删除从设备overlay相关配置*/
    ret = ospf_dcn_lookup_slave_port(ulLPortNum,&uiIpaddr,&uiMasklen);
    if(OK == ret)
    {
    	ulLldpEn = LLDP_MNG_NONE;
    	ret = lldp_set_api(ulIfIndex,LLDP_PORT_MASTER_CFG,&ulLldpEn);
    	if(OK != ret)
    	{
    		zlog_info(MTYPE_OSPF,"lldp over-lay master disable set failed!\n");
            OSPF_DCN_LOG_WARNING("lldp over-lay master disable set failed!");
            return ERR;
        }
        ret = ospf_dcn_overlay_ipaddr_uninstall(ulLPortNum,uiIpaddr,OSPF_DCN_OVERLAY_MASK_LEN,DCN_OVERLAY_DEL);
        if(OK != ret)
        {
            zlog_info(MTYPE_OSPF,"ospf_dcn_overlay_ipaddr_uninstall failed!uiPort:%d,ulIpAddr:%x\n",ulLPortNum,ulIpAddr);
            OSPF_DCN_LOG_WARNING("ospf_dcn_overlay_ipaddr_uninstall failed!uiPort:%d,ulIpAddr:%x.",ulLPortNum,ulIpAddr);
            return ERR;
        }
    }

    return OK;
}


int ospf_dcn_get_lsa_frist(tOSPF_LSDB_INDEX *p_index)

{
    struct ospf_process * p_process = NULL;
    struct ospf_area * p_area = NULL;
    struct ospf_lsa *p_lsa = NULL;
 	struct ospf_lsa *p_next_lsa = NULL;
    int iRet = OK;

    ospf_semtake_try();

    p_process = ospf_prcess_search(OSPF_DCN_PROCESS);
    if (p_process == NULL)
    {
        iRet = ERR;
        goto END;
    }

    p_area = ospf_lstfirst(&p_process->area_table);
    if (p_area == NULL)
    {
        iRet = ERR;
        goto END;
    }
    
    for_each_node(&p_area->ls_table[OSPF_LS_TYPE_10]->list, p_lsa, p_next_lsa) 
    {
        /*get the first*/
        if (NULL != p_lsa)  
        {
            if (OSPF_DCN_LSID != ntohl(p_lsa->lshdr->id))
            {
                continue;
            }

            p_index->process_id = OSPF_DCN_PROCESS;
            p_index->area = 0;
            p_index->type = OSPF_LS_TYPE_10;
            p_index->id = ntohl(p_lsa->lshdr->id);
            p_index->adv = ntohl(p_lsa->lshdr->adv_id);

            iRet = OK;            
            break;
        }  
    }
END:
    ospf_semgive();

    return iRet;
}



int ospf_dcn_get_lsa_next(tOSPF_LSDB_INDEX *p_index, tOSPF_LSDB_INDEX *p_next_index)
{
    struct ospf_process * p_process = NULL;
    struct ospf_area * p_area = NULL;
    struct ospf_lsa ospf_lsa = {0};
    struct ospf_lsa *p_lsa = NULL;
    int iRet = OK;

    ospf_semtake_try();

    p_process = ospf_prcess_search(OSPF_DCN_PROCESS);
    if (p_process == NULL)
    {
        iRet = ERR;
        goto END;
    }

    p_area = ospf_lstfirst(&p_process->area_table);
    if (p_area == NULL)
    {
        iRet = ERR;
        goto END;
    }

    ospf_lsa.lshdr->type = p_index->type;
    ospf_lsa.lshdr->id = ntohl(p_index->id);
    ospf_lsa.lshdr->adv_id = ntohl(p_index->adv);
    p_lsa = ospf_lstgreater(&p_area->ls_table[OSPF_LS_TYPE_10]->list, &ospf_lsa);
    if (p_lsa == NULL)
    {
        iRet = ERR;
        goto END;
    }

    p_next_index->process_id = OSPF_DCN_PROCESS;
    p_next_index->area = 0;
    p_next_index->type = OSPF_LS_TYPE_10;
    p_next_index->id = ntohl(p_lsa->lshdr->id);
    p_next_index->adv = ntohl(p_lsa->lshdr->adv_id);
END:
    ospf_semgive();

    return iRet;
}
#endif
