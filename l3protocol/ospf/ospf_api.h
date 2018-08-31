/* ospf_area.h */

#if !defined (_OSPF_API_H_)
#define _OSPF_API_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "ospf.h"

#define OSPF_AUTH_SIMPLE_LEN    8
#define OSPF_AUTH_MD5_LEN       16
#define OSPF_AUTH_KEY_MAX_LEN   64

#define OSPF_VPN_NAME_MAX_LEN   30

#ifdef HAVE_BFD
#define OSPF_BFD_SES_NUM_PER_IF 16
#define OSPF_BFD_SES_MINRX_DEF 10
#define OSPF_BFD_SES_MINTX_DEF 10
#define OSPF_BFD_SES_DETMUL_DEF 3


enum{
    OSPF_BFD_DISABLE = 0,
    OSPF_BFD_GBL_ENABLE,
    OSPF_BFD_IF_ENABLE,
    OSPF_BFD_STATIC,
    OSPF_BFD_BLOCK,
};
#endif

enum {  
        OSPF_IF_IPADDR,/*ospfIfIpAddress*/
        OSPF_IF_IPADDRLESSIF,/*ospfAddressLessIf*/
        OSPF_IF_AREAID,/*ospfIfAreaId*/
        OSPF_IF_TYPE,/*ospfIfType*/
        OSPF_IF_ADMINSTATUS ,/*ospfIfAdminStat*/
        OSPF_IF_PRIORITY,/*ospfIfRtrPriority*/
        OSPF_IF_TRANSITDELAY,/*ospfIfTransitDelay*/
        OSPF_IF_RETRANSMITINTERVAL,/*ospfIfRetransInterval*/
        OSPF_IF_HELLOINTERVAL,/*ospfIfHelloInterval*/
        OSPF_IF_DEADINTERVAL,/*ospfIfRtrDeadInterval*/
        OSPF_IF_POLLINTERVAL,/*ospfIfPollInterval*/
        OSPF_IF_STATE,/*ospfIfState*/
        OSPF_IF_DRADDR,/*ospfIfDesignatedRouter*/
        OSPF_IF_BDRADDR,/*ospfIfBackupDesignatedRouter*/
        OSPF_IF_EVENT,/*ospfIfEvents*/
        OSPF_IF_AUTHKEY,/*ospfIfAuthKey */
        OSPF_IF_CIPHERKEY,
        OSPF_IF_AUTHDIS,/*ospfIfAuthDis*/
        OSPF_IF_STATUS,/*ospfIfStatus*/
        OSPF_IF_MCASTFWDING,/*ospfIfMulticastForwarding*/
        OSPF_IF_DEMAND,/*ospfIfDemand*/
        OSPF_IF_AUTHTYPE,/*ospfIfAuthType*/
        OSPF_IF_LSCOUNT,
        OSPF_IF_LSCHECKSUM,
        OSPF_IF_DRID,
        OSPF_IF_BDRID,
        OSPF_IF_PASSIVE,
        OSPF_IF_MTU,
        OSPF_IF_TEADMINGROUP ,
        OSPF_IF_TECOST,
        OSPF_IF_TEENABLE,
        OSPF_IF_TEMAXBINDWIDTH ,
        OSPF_IF_TEMAXRSVDBINDWIDTH ,
        OSPF_IF_STATICCOST,
        OSPF_IF_AUTHKEYID,
#ifdef HAVE_BFD
        OSPF_IF_BFD,
        OSPF_IF_BFD_MIN_RX_INTERVAL,
        OSPF_IF_BFD_MIN_TX_INTERVAL,
        OSPF_IF_BFD_DETECT_MUL,
        OSPF_IF_BFD_COUNT,
#endif
        OSPF_IF_FLOODGROUP,
        OSPF_IF_FASTDDEXCHANGE,
        OSPF_IF_UNIT,
        OSPF_IF_MTUIGNORE,
        OSPF_IF_COST,
        OSPF_IF_SAVE_KEY,
        OSPF_IF_LDP_SYNC,
        OSPF_IF_HOLD_DOWN,
        OSPF_IF_HOLD_COST,
        OSPF_IF_LDP_SYNC_STATE,
        OSPF_IF_DCN_ADD,
        OSPF_IF_DCN_DELETE,
        OSPF_IF_VRID
   };


enum {  
        MIB_OSPF_NODE_PROCESS = 0,
        MIB_OSPF_NODE_AREA,
        MIB_OSPF_NODE_NEST,
        MIB_OSPF_NODE_MASK
};


enum {
    OSPF_NBR_IPADDR ,
    OSPF_NBR_IPADDRLESSINDEX  ,
    OSPF_NBR_ROUTERID ,
    OSPF_NBR_OPTION  ,
    OSPF_NBR_PRIORITY ,
    OSPF_NBR_STATE  ,
    OSPF_NBR_EVENT ,
    OSPF_NBR_RETRANSQLEN  ,
    OSPF_NBR_STATUS  ,
    OSPF_NBR_NBMAPERMANENCE  ,
    OSPF_NBR_HELLOSUPRESS ,
    OSPF_NBR_RESTARTHELPERSTATUS,
    OSPF_NBR_RESTARTHELPERAGE,
    OSPF_NBR_RESTARTHELPEREXITREASON,
    OSPF_NBR_DEADTIMER,
    OSPF_NBR_UPTIME,
#ifdef HAVE_BFD
    OSPF_NBR_BFDSESSION,
#endif
    OSPF_NBR_IFUNIT,
    OSPF_NBR_DD_STATE,
    OSPF_NBR_DR,
    OSPF_NBR_BDR,
    OSPF_NBR_IFNAME,
    OSPF_NBR_AREAID,
    OSPF_NBR_IFIPADDR,
    OSPF_NBR_RXMTINTERVAL,
    OSPF_NBR_AUTHSEQNUM,
    OSPF_NBR_MTU,
    OSPF_NBR_RESTARTSTATUS
    };

/*route */
enum {
      OSPF_ROUTE_TYPE,
      OSPF_ROUTE_MASK,        
      OSPF_ROUTE_DEST,
      OSPF_ROUTE_NEXTHOP ,
      OSPF_ROUTE_PATHTYPE,
      OSPF_ROUTE_BACKUPNEXTHOP,
      OSPF_ROUTE_COST,
      OSPF_ROUTE_COST2,
      OSPF_ROUTE_AREA,
      OSPF_ROUTE_EXT_TYPE,
      OSPF_ROUTE_TAG,
      OSPF_ROUTE_ADV_ID,
      OSPF_ROUTE_INTRA,
      OSPF_ROUTE_INTERFACENAME,
      OSPF_ROUTE_IMPORTASEXTERNAL,
      OSPF_ROUTE_NEXTHOP_IFINDEX,
};




enum {  
        OSPF_RULE_CREAT = 1, /*OSPF创建ACL规则*/
        OSPF_RULE__DEL        /*OSPF删除ACL规则*/
};

enum{
    OSPF_DCN_IF_CNT = 1,
    OSPF_DCN_IF_ERR_CNT,
    OSPF_DCN_LSA_CNT,
    OSPF_DCN_LSA_ERR_CNT,
    OSPF_DCN_NEID_CNT,
    OSPF_DCN_NEID_ERR_CNT,
};

typedef struct {
   u_int process_id;   
   u_int nbr_ip;
}tOSPFMIB_NBR_INDEX;

typedef struct {
   u_int process_id;   
   u_int route_dest;
   u_int route_mask;
}tOSPFMIB_ROUTE_INDEX;




int ospf_getFirstProcessMib(u_int **pulIndex);
int ospf_getNextProcessMib(u_int *pulIndx, u_int **pulNextIndex);
int ospf_SetCost(u_int ulIfunit, u_int costValue);
int ospf_GetCost(u_int ulIfunit, u_int *costValue);
int ospf_getif(u_int ulIfunit);
int ospf_getAuthKeyId(u_int ulIfunit,u_int *pAuthKeyid);
int ospf_setAuthKeyId(u_int ulIfunit,u_int *pKeyid);
int ospf_getAuthKey(u_int ulIfunit ,u_char *pKeyStr);
int ospf_setAuthKey(u_int ulIfunit ,u_char *pKeyStr,u_int uKeylen);
int ospf_getHelloInterval(u_int ulIfunit,u_int *uValue);
int ospf_setHelloInterval(u_int ulIfunit,u_int *pValue);
int Mib_ospf_GetInterface(u_int ulIfunit,u_int *uValue,u_int mib_num);
int Mib_ospf_SetInterface(u_int ulIfunit,u_int *pValue,u_int mib_num);
int Mib_ospf_NodeCreatArea_Pro(tOSPF_AREA_INDEX *pstIndx);
int Mib_ospf_setRowStatus(tOSPF_NETWORK_INDEX *pstIndx,u_int ulData);
int Mib_ospf_NodeSet(u_int ulCmd,u_int ulData);
int Mib_ospf_NodeCreatData(tOSPF_NETWORK_INDEX *pstIndx);
int Mib_ospf_Nodedelete(u_int ulPrid);
int getFirstNode(tOSPF_NETWORK_INDEX *p_index);
int getNextNode(tOSPF_NETWORK_INDEX *p_index,tOSPF_NETWORK_INDEX *p_Nextindex);
int Mib_ospf_getRowStatus(tOSPF_NETWORK_INDEX *pstIndx,u_int *ulData);
int getFirstNeighbor(tOSPFMIB_NBR_INDEX *p_index);
int getNextNeighbor(tOSPFMIB_NBR_INDEX *p_index,tOSPFMIB_NBR_INDEX *p_Nextindex);
int Mib_ospf_GetNbrApi(tOSPFMIB_NBR_INDEX *pstIndx,void *vValue,u_int mib_num);
int ospf_get_AreaId(u_int ulProid,u_int ulDest,u_int ulMask,u_int *ulAreaid);
void Creat_ospf_IpaddrApi(u_int ulIfIndex,u_int ipaddr);
int getFirstRoute(tOSPFMIB_ROUTE_INDEX *p_index);
int getNextRoute(tOSPFMIB_ROUTE_INDEX *p_index,tOSPFMIB_ROUTE_INDEX *p_Nextindex);
int Mib_RouteGetApi(tOSPFMIB_ROUTE_INDEX *p_index,u_int cmd,void *var);
int ospfGetPortLinkStates(u_int ulIfIndex);
void ospf_lsa_update(u_int uiProcessId);
#ifdef HAVE_BFD
int Bfd_ospf_nbr_del_api(struct ospf_nbr *p_nbr);
#endif
void ospf_update_route_for_pre_chg(u_int ulProId, u_int ulCmd);
#ifdef OSPF_MASTER_SLAVE_SYNC
int ospf_master_slave_state_get(u_int *puiState);
int ospf_master_slave_state_set(u_int uiState);
int ospf_dyn_data_send();
int ospf_master_slave_dyn_data_sync_recv(int iModid, u_char *pBuf, int iLen);
#endif
int ospf_mpls_lsr_id_adv(u_int uiProcessId,u_int uiIpaddr,u_int uiCost,u_char ucMode);
int ospf_te_if_set_api(uint32_t index,int32_t cmd, void* var);
u_int ospf_router_lsa_lsrid_fill(struct ospf_router_link *p_link,struct ospf_area *p_area);
int ospf_lsrid_refresh(u_char ucMode);
u_int ospf_vfindextolpindex(u_int ulVfIndex);
int ospf_IntfIpAddr_get(u_long ulIfIndex,struct prefix *pstPrefx);
STATUS ospfRouteGetApi(tOSPF_ROUTE_INDEX *pstRouteIndex,u_int cmd, void *var,u_int ulIndex);
STATUS ospfRouteGetFirst(tOSPF_ROUTE_INDEX *pstRouteIndex,u_int ulIndex);
STATUS ospfRouteGetNext(tOSPF_ROUTE_INDEX *pstRuIndex,tOSPF_ROUTE_INDEX *pstNeRuIndex,u_int ulIndex);
BOOL ospf_get_cfg_state();

#endif

