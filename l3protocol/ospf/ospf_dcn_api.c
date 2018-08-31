

#include "ospf.h"
#include "ospf_nm.h"
#include "ospf_dcn.h"

#ifdef OSPF_DCN
extern OSPF_DCN_T stDcnCfg;
extern OSPF_DCN_T stOspfDcn;


int dcn_vlan_change_check(u_int uiVlan)
{
    int iRet = VOS_ERR;
    u_int uiIfIndex = 0;
    u_int uiBindVlan = 0;
    u_int uiSlot = 0, uiPort = 0;
    
    iRet = zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_FIRST_IFINDEX, &uiIfIndex);
    while(iRet == VOS_OK)
    {
        uiBindVlan = 0;
        if(IFINDEX_TO_TYPE(uiIfIndex) == IFINDEX_TYPE_VPIF || IFINDEX_TO_TYPE(uiIfIndex) == IFINDEX_TYPE_ETH_VTRUNK)
        {
            zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_IF_BIND_VLAN, &uiBindVlan);
            if(uiVlan == uiBindVlan)
            {
               uiSlot = VPORT_IF_IFINDEX_TO_SLOT(uiIfIndex);
               uiPort = VPORT_IF_IFINDEX_TO_PORT(uiIfIndex);
               if((uiSlot > 0 && uiSlot < MAX_SLOT_NUM) && (uiPort < SLOT_MAX_PORT_NUM && uiPort > 0))
               {
                   if(stOspfDcn.uiUsePort[uiSlot-1][uiPort-1] == OSPF_DCN_PORT_ENABLE)
                   {
                        return VOS_ERR;
                   }
               }
            }
        }

        iRet = zebra_if_get_api(uiIfIndex, ZEBRA_IF_GET_NEXT_IFINDEX, &uiIfIndex);
    }

    return VOS_OK;
}

STATUS 
dcnSetApi(
         void *index, 
         u_int cmd, 
         void *var)
{
    int iRet = 0;
    int iFlg = 0;
    u_int lval = (*(u_int*)var),uiOldVlan = 0;
    STATUS rc = OK;
    u_char ucDcnEn = 0,ucPortEn = 0,ucInitFlg = 0;
    u_int uiIfindex = (*(u_int*)index);
    //DCN_PORT_OVERLAY_CFG *pstOverCfg = (DCN_PORT_OVERLAY_CFG *)index;
    int iLen = 0;
    u_int auiIndex[3] = {0};
    u_char ucComfirm = 0;
    u_int uiDcnEnFlg = 1;
    u_int uiSecFlg = 1;
   
    
    if((index == NULL)||(var == NULL))
    {
		return ERR;
	}
    ucInitFlg = ospf_dcn_get_init();
    ucDcnEn = ospf_dcn_get_globale_enable();
    
    if((ucInitFlg == 1)
        &&(ucDcnEn != OSPF_DCN_ENABLE)
        &&(cmd != DCN_GBL_EN))
	{
		return ERR;
	}
    /*debug not need take sem*/
    switch (cmd) 
    {
        case DCN_NE_BANDWIDTH:
            
            if((lval != DCN_BANDWIDTH_64)&&(lval != DCN_BANDWIDTH_128)
                &&(lval != DCN_BANDWIDTH_192)&&(lval != DCN_BANDWIDTH_256)
                &&(lval != DCN_BANDWIDTH_512)&&(lval != DCN_BANDWIDTH_DEF)
                &&(lval != DCN_BANDWIDTH_2048))
            {
                return ERR;
            }

			rc = ospf_dcn_modify_all_eth_bandwidth(lval);
			break;
        case DCN_NE_ID:
            
            rc = ospf_dcn_neid_set(lval);
            break;

        case DCN_NE_IP:            
            if(stOspfDcn.ulDeviceIp != lval)
			{
			    stOspfDcn.ulDeviceIp = lval;
				rc = ospf_dcn_modify_ip(lval,stOspfDcn.ulDeviceMask);
				stOspfDcn.ucNeidReleIp = 0;
				ospf_dcn_lldp_init();
			}
			break;
            
        case DCN_NE_IP_MASK:
			if(lval != stOspfDcn.ulDeviceMask)
			{
				stOspfDcn.ulDeviceMask = lval;
			}
			break;   
        case DCN_IMPORT_LOCALROUTE:
            rc = ospf_dcn_local_manage_cfg(uiIfindex,lval);
            break;

        case DCN_OVERLAY_MANAGE:
            //rc = ospf_dcn_port_overlay_mng_cfg(pstOverCfg->uiIfindx,pstOverCfg->uiIpaddr,pstOverCfg->uiMasklen,lval);
            break;

        case DCN_PORT_EN:
            ucPortEn = lval;
            rc = ospf_dcn_one_interface_enable(uiIfindex,ucPortEn);
			break;
           
        case DCN_GBL_VLAN:
            if(dcn_config_load_done() == VOS_TRUE)
            {
                ospf_dcn_vlan_get(&uiOldVlan);
    			if(lval != uiOldVlan)
    			{
    				rc = ospf_dcn_global_vlan_modify(lval);
    				ospf_dcn_lldp_init();
    			}
			}
			else
			{
			    stOspfDcn.uiVlanId = lval;
			}
			break;
            
        case DCN_GBL_EN:
            if(ucDcnEn != lval)
            {
                if(lval == ENABLE)//创建dcn
                {
                    rc = ospf_dcn_create();
                    stOspfDcn.ucDcnEnflag = OSPF_DCN_ENABLE;
                }
                else if(lval == DISABLE)//删除dcn
                {
                    rc = ospf_dcn_delete();
                    stOspfDcn.ucDcnEnflag = OSPF_DCN_DISABLE;
                }
                else
                {
                    return ERR;
                }
            }
		    break; 
		case DCN_IF_DEL_IF_DCN_PORT:
            rc = dcn_interface_delete_if_dcn_port(uiIfindex);
            break;    
        default:
            rc = ERR;
            break;
    }

    return rc;
}


STATUS 
dcnGetApi(
         void *index, 
         u_int cmd, 
         void *var)
{
    u_int *lval = (u_int*)var;
    u_int uiOldVlan = 0;
    STATUS rc = OK;
    u_char ucDcnEn = 0,ucPortEn = 0;
    u_int uiIfindex = (*(u_int*)index),uiPort = 0;
    DCN_PORT_OVERLAY_CFG *pstOverMng = (DCN_PORT_OVERLAY_CFG *)var;
    tOSPF_LSA_NET *pstPortMng = (tOSPF_LSA_NET*)var;
    u_int uiLogicPort = 0;
    u_int ulIpAddr = 0,ulMasklen = 0;

    if((index == NULL)||(var == NULL))
    {
		return ERR;
	}    

    switch (cmd) 
    {
        case DCN_NE_BANDWIDTH:
			*lval = stOspfDcn.uiBandwidth;
			break;
        case DCN_NE_ID:
			*lval = stOspfDcn.uiNeid;
            break;

        case DCN_NE_IP:
            *lval = stOspfDcn.ulDeviceIp;
			break;
            
        case DCN_NE_IP_MASK:
            *lval = stOspfDcn.ulDeviceMask;
			break;
            
        case DCN_IMPORT_LOCALROUTE:
            rc = ospf_dcn_local_manage_get(uiIfindex,pstPortMng);
            break;

        case DCN_OVERLAY_MANAGE:
            if(VOS_ERR == if_index_to_logic_port(uiIfindex, &uiPort))
            {
                return ERR;
            }

            rc = ospf_dcn_lookup_master_port(uiPort,&ulIpAddr,&ulMasklen);
            if(rc == OK)
            {
                pstOverMng->uiIfindx = uiIfindex;
                pstOverMng->uiIpaddr = ulIpAddr;
                pstOverMng->uiMasklen = ulMasklen;
            }

            break;

        case DCN_PORT_EN:
            rc = ospf_dcn_get_port_enable_status(uiIfindex);
            if(rc != ERR)
            {
                *lval = rc;
                rc = OK;
            }
            break;

            
        case DCN_GBL_VLAN:
            rc = ospf_dcn_vlan_get(&uiOldVlan);
            if(rc == OK)
            {
			    *lval = uiOldVlan;
            }
			break;
            
        case DCN_GBL_EN:
            *lval = ospf_dcn_get_globale_enable();
			break;
            
        case DCN_NE_DEVICE_TYPE :
            memcpy(var,stOspfDcn.ucDevType,strlen(stOspfDcn.ucDevType));
			break;

        case DCN_NE_COMPANY_NAME :
            memcpy(var,stOspfDcn.ucDevName,strlen(stOspfDcn.ucDevName));
			break;
            
        case DCN_NE_DEVICE_MAC:
            memcpy(var,stOspfDcn.ucDevMac,MAC_ADDR_LEN);
			break;

        case DCN_NE_TYPE:
            *lval = stOspfDcn.uiNeType;
            break;

        case DCN_NE_PORT:
            *lval = stOspfDcn.uiNePort;
            break;
        case DCN_NE_CHANGE_FLAG:
            *lval = stOspfDcn.ucChangeFlag;
            break;

        case DCN_INITFLAG: 
            *lval = stOspfDcn.ucInitflag;
            break;
        case DCN_NE_IP_RELATION_FLAG:
            *lval = stOspfDcn.ucNeidReleIp;
            break;
        case DCN_VLAN_CHECK:
            rc = dcn_vlan_change_check(*lval);
            break;
        case DCN_IF_VLAN_CHECK:
            rc = dcn_interface_vlan_check(*lval, uiIfindex);
            break;
        case DCN_IF_INTERFACE_CHECK:
            rc = dcn_interface_exist_check(uiIfindex);
            break;
        default:
            return ERR;
    }
    
    return rc;
}

#endif

