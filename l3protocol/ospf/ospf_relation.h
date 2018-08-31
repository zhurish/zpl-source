
#if !defined (_OSPF_RELATION_H_)
#define _OSPF_RELATION_H_
#include "rib.h"

#define LLDP_MNG_NONE 0

/*LDP*/
typedef enum
{
    LDP_MSG_SESSION_STATE_DOWN,
    LDP_MSG_SESSION_STATE_UP,
    LDP_MSG_SESSION_STATE_INIT,
}LDP_MSG_TYPE_E;

/*LLDP*/
typedef enum
{
    LLDP_PORT_MASTER_IP_MASK,
    LLDP_MANAGE_ADDR,
    LLDP_PORT_MASTER_CFG,
    LLDP_PORT_EN,
    LLDP_PORT_MASTER_IP_ADDR,
    LLDP_REMOTE_PORT_NUM,
    LLDP_REMOTE_MANAGE_ADDR,
    LLDP_REMOTE_DELETE_NBR_BY_LOGIC_PORT,
    LLDP_REMOTE_GET_MAC_BY_IP,
}LLDP_DCN_CMD_E;

#define LLDP_PORT_DISABLE 1
#define LLDP_PORT_ENABLE 2

#define L3_IF_DCN_MODE_NONE   0
#define L3_IF_DCN_MODE_P2P    1

#define SYS_API_DCN_EN    1
/*PCL*/
typedef enum
{
    PCL_TYPE_PLIST = 0,
    PCL_TYPE_RMAP,
}PCL_CMD_E;

typedef struct next_hop
{
    u_char *address;
}next_hop_t;

typedef struct pcl_cfg_info
{
    struct next_hop nexthop;
    struct prefix *prefix;
    u_int uiCost;
    u_char ucPref;  
}pcl_cfg_info_t;



#define ISIS_GET_PROCESS_ID_BY_IFINDEX 1

typedef struct
{
    /* process ID */
    u_int process_id;

    /*stanard protocol*/
    u_int8 proto;
    u_int16 vrid;

    /*dest of route*/
    u_int dest;

    /*mask of route*/
    u_int mask;

    /*cost of route*/
    u_int metric;

    /*nexthop of route*/
    u_int fwdaddr;

    /*nexthop interface unit*/
    u_int if_unit;

    /*true :add route, false :delete route*/
    u_int8 active : 1;

}ospf_import_route_t;


/*other*/
/*
typedef struct
{
    u_char ucProtoType;
    u_char type;
    u_int uiIfIndex;
    u_int uiNextHopAddr;
    u_int uiDestAddr;
    u_int uiProcessId;
    u_int uiVrfId;
    u_int uiSubCode;
    u_int uiMetric;
    u_int uiIPMaskLen;
}ROUTE_MSG_INFO_T;

typedef struct
{
    u_int uiProto;
    u_int uiProtoProId;
    u_int uiVrfId;
    u_int uiMetric;
    u_int uiDest;
    u_int uiMaskLen;
    u_int uiState;
    u_int uiAf;
}L3_IMPORT_ROUTE_INFO_T;

typedef struct
{
    u_int uiVrf;
    u_int uiIp;
}ARP_INFO_INDEX_T;

typedef struct
{
    u_short usLagIndex;
}LAG_GROUP_T;
*/

/*
typedef struct
{
    u_int uiIfindx;
    u_int uiIpaddr;
    u_int uiMasklen;
}DCN_PORT_OVERLAY_CFG;
*/


enum{
       OSPF_RMAP_PERMIT = 1,
	   OSPF_RMAP_DENYMATCH,
	   OSPF_RMAP_NOMATCH,
	   OSPF_RMAP_NO_ENTRY,
};

enum{
	OSPF_PREFIX_PERMIT = 1,
	OSPF_PREFIX_DENY,
	OSPF_PREFIX_NOMATCH,
	OSPF_PREFIX_NO_ENTRY,
};

enum
{
    SYS_API_BASE_DEVICE_ID = 1,
    SYS_API_BASE_PROVIDER,
    SYS_API_BASE_MAC_ADDR,
    SYS_API_GBL_CARD_NAME,
};

enum
{
    LLDP_OVERLAY_MASTER_IPADDR_MNG = 1,
};

enum
{
    MPLS_PORT_API_ADD = 1,
};

int lldp_dcn_rem_nbr_del();

int arp_refresh_by_lldp();

int uspGetApi(u_int uiIndex, u_int uiCmd, void *pValue);

int mpls_port_set_api(u_int uiIndex, u_int uiCmd, void *pValue);

int lldp_overlay_slave_clean_api(u_int uiIfIndex, u_int uiMask);

int ospf_dcn_route_check(u_int uiIp);

int lldp_rem_del_batch_by_if_index(u_int ulIfIndex);

void ospf_msg_send(u_int uiMsgType, void *pBuf);

u_int ospf_zebra_route_translate_to_m2_route(u_int uiZebraRouteType);

u_int ospf_m2_route_translate_to_zebra_route(u_int uiM2RouteType);

//int ospf_add_route_to_system(ZEBRA_ROUTE_CONFIG_INFO_T *pstRouteInfo);

//int ospf_delete_route_to_system(ZEBRA_ROUTE_CONFIG_INFO_T *pstRouteInfo);

//int ospf_get_exist_if_ip(u_int uiIndex, u_int uiVrf, u_int *pIpAddr);

//u_int ospf_get_ldp_session_state_by_ifindex(u_int uiVrfId, u_int uiIfIndex);

#endif
