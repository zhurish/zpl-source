#ifndef OSPF_DCN_H
#define  OSPF_DCN_H

#ifdef __cplusplus
extern "C" {
#endif
#include "ospf_main.h"

#ifdef OSPF_DCN
#define OSPF_DCN_LOG_INF(format, ...) zlog(MTYPE_OSPF,LOG_INFO, format"(%s:%d)", ## __VA_ARGS__, __FILE__, __LINE__)      
#define OSPF_DCN_LOG_DBG(format, ...) zlog(MTYPE_OSPF, LOG_DEBUG, format"(%s:%d)", ## __VA_ARGS__,__FILE__, __LINE__)
#define OSPF_DCN_LOG_WARNING(format, ...) zlog(MTYPE_OSPF, LOG_WARNING, format"(%s:%d)", ## __VA_ARGS__,__FILE__, __LINE__)
#define OSPF_DCN_LOG_ERR(format,...) zlog(MTYPE_OSPF, LOG_ERR, format"(%s:%d)", ## __VA_ARGS__, __FILE__, __LINE__)
#define OSPF_DCN_LOG_NOTICE(format,...) zlog(MTYPE_OSPF, LOG_NOTICE, format"(%s:%d)", ## __VA_ARGS__, __FILE__, __LINE__)

//#define  OSPF_DCN		  1
#define  OSPF_DCN_FLAG    1
#define  OSPF_DCN_NOFLAG  0
#define  OSPF_DCN_PROCESS 65535
#define  OSPF_DCN_VPN_NAME    "dcn_vpn"
#define  OSPF_DCN_LSID 0xcaffee00


#define OSPF_DCNLOOPBACK_IFINDEX 		63
#define OSPF_DCN_MASK_LEN			 	32
#define OSPF_DCN_OVERLAY_MASK_LEN		24
#define OSPF_DCN_OVERLAY_IPMASK         0xffffff00


#define OSPF_DCN_AREA_ID         		0
#define DCN_VLAN_DEF  					4094	//dcn 使用默认vlan

#define OSPF_DCNDEF_IPADDR              0x86000000
#define OSPF_DCNDEF_IPMASK              0xffffffff
#define OSPF_DCNDEF_NEID                0x123456
#define OSPF_DCN_SUBPORT 4094
#define OSPF_DCN_NEID_RELEVANCE_NEIP    1
#define DCN_DEL_VLAN  0
#define DCN_ADD_VLAN  1

#define DCN_GBL_FLG  1
#define DCN_PORT_FLG  0


#define DCN_BANDWIDTH_64  64
#define DCN_BANDWIDTH_128  128
#define DCN_BANDWIDTH_192  192
#define DCN_BANDWIDTH_256  256
#define DCN_BANDWIDTH_512  512
#define DCN_BANDWIDTH_DEF  1024
#define DCN_BANDWIDTH_2048  2048
#define		DCN_RECV_TICK			4			//接受异常限定次数

typedef enum {  
        DCN_REMOTE_NE_ID = 0,   /*远端网元id*/
        DCN_REMOTE_NE_IP,       /*远端网元ip*/
        DCN_REMOTE_NE_MAC,      /*远端网元mac地址*/
        DCN_REMOTE_NE_TYPE,     /*远端网元设备类型*/
        DCN_REMOTE_NE_COMPANY,    /*远端网元厂商*/
        DCN_REMOTE_NE_IPV6,     /*远端网元ipv6*/
}DCN_REMOTE_NE_CMD_E;


typedef enum {  
        DCN_NE_ID = 0,
        DCN_NE_DEVICE_TYPE,
        DCN_NE_COMPANY_NAME,
        DCN_NE_DEVICE_MAC,
        DCN_NE_TYPE,
        DCN_NE_IP,
        DCN_NE_IP_MASK,
        DCN_NE_PORT,
        DCN_NE_CHANGE_FLAG,
        DCN_NE_ENABLE_FLAG,
        DCN_NE_VLAN_ID,
        DCN_NE_BANDWIDTH,
        DCN_IMPORT_LOCALROUTE,
        DCN_OVERLAY_MANAGE,
        DCN_PORT_EN,
        DCN_GBL_VLAN,
        DCN_GBL_EN,
        DCN_INITFLAG,
        DCN_NE_IP_RELATION_FLAG,
        DCN_VLAN_CHECK,
        DCN_IF_VLAN_CHECK,
        DCN_IF_INTERFACE_CHECK,
        DCN_IF_DEL_IF_DCN_PORT,
}DCN_NE_CMD_E;


enum
{
    DCN_SLOT_INIT = 0,
	DCN_SLOT_MASTER,
    DCN_SLOT_SLAVER,
    
};
enum {  
        DCN_BAND_CREAT = 1, /*创建端口带宽*/
        DCN_BAND_DEL        /*删除端口带宽限制*/
};

enum {  
        DCN_RECV_OK = 1,    /*dcn报文接收成功*/
        DCN_RECV_ERR        /*未接收到dcn报文*/
};

enum {  
        DCN_IPCHG_CLOSE = 0,    /*IPCHG_EXSTART*/
        DCN_IPCHG_EXSTART = 1,    /*IPCHG_EXSTART*/
        DCN_IPCHG_START        /*IPCHG_START*/
};
enum {  
        DCN_IPCHG_CNTINIT = 0,    /*IPCHG_EXSTART*/
        DCN_IPCHG_CNTSTART = 1,    /*IPCHG_EXSTART*/
        DCN_IPCHG_CNTEND        /*IPCHG_START*/
};


typedef struct {
   u_long ulAddr;   
   u_long ulIfIndex; 
   u_char ucRecvFlag;//端口接收标志位
   u_char ucRecvCnt;//端口接收报文超时计数
   u_char ucSorMac[MAC_ADDR_LEN];//端口接收报文超时计数
}OSPF_RX_PKT_T;
enum {  
        DCN_OVERLAY_NET_ADD = 2,    
        DCN_OVERLAY_NET_DEL,        
};

enum {  
        DCN_OVERLAY_MASTER_ADD = 1,    /*master*/
        DCN_OVERLAY_SLAVER_ADD,         /*slaver*/
        DCN_OVERLAY_DEL,                 /*delete*/
};
typedef struct {
   u_long ulIfindex;      //Overlay 管理接口
   u_char ucMngflag;		//接口是否为master标志位
   u_long ulAddr;      //Overlay 管理ip
   u_long ulMasklen;   //Overlay 管理mask
}OSPF_DCN_OVERLAY_CFG_T;

typedef struct {
   u_char ucOverlayNum;		//接口是否启动overlay模型标志位
   OSPF_DCN_OVERLAY_CFG_T stOverlayConfig[MAX_PORT_NUM];//  port
}OSPF_DCN_OVERLAY_T;

typedef struct {
  /*DCN 全局变量*/
   u_char ucDcnEnflag;	//DCN全局使能标志位
   u_char ucChangeFlag;		//DCN接口进入标志位,仅DCN配置时可进入
   u_char ucInitflag;		//DCN数据初始化完成标志位
   u_char ucNeidReleIp;
   pthread_mutex_t lock_sem;
   struct timespec stDcnStartTime;
   struct timespec stDcnStopTime;

   
   /*MIB结点数据*/
   u_int uiNeid;   	//网元ID
   u_char ucDevType[128];	//设备类型
   u_char ucDevName[128];	//厂商标识
   u_char ucDevMac[6];	//MAC地址
   u_int uiNeType;	//网络类型IPV4 、IPV6
   u_int ulDeviceIp;	//DCN IP
   u_int ulDeviceMask;	//DCN 掩码
   u_int uiNePort;	//DCN TCP端口
   u_int uiVlanId;	//Vlan Id
   u_int uiBandwidth;	//Ethernet bandwidth
   /*DCN 使用接口*/
   u_int uiUsePort[MAX_SLOT_NUM][SLOT_MAX_PORT_NUM];//slot  port
   u_int uiLagPort[LAG_GROUP_NUM];
   OSPF_DCN_OVERLAY_CFG_T stOverlayCfg[MAX_PORT_NUM];// port
   OSPF_RX_PKT_T stOspfRxPkt[100];//port
   u_short  usIpChgStart;
}OSPF_DCN_T;

#define OSPF_ARRAY_LEN(array)    (sizeof(array) / sizeof(array[0]))
struct ospf_dcn_rxmt_node
{
    /*node in rtm_dcnd_fail_table*/
    struct ospf_lstnode node; 

    u_int rtm_type;
    
    struct ospf_lshdr lshdr;

    uint8_t body[256];
};

#define RTM_DCN_DEL  0
#define RTM_DCN_ADD  1

#define DCN_VLAN_EN  		1//VLAN创建成功
#define DCN_VLAN_DIS  		0//VLAN未创建

typedef struct {
   u_char ucDevType[128];
   u_char ucCompany[128];
   u_char ucDevMac[8];
   u_int uiNeid;   
   u_long ulNeIp;
   u_char ulNeIpv6[16];
}OSPF_DCN_NBR_T;

typedef struct {
   u_int uiCardType;	//板卡类型
   u_int uiPortStart;   //端口起始号
   u_int uiPortEnd;   //端口结束号
}OSPF_DCN_CARD_CTL_T;

typedef struct {
   u_int uiPoStart;   //端口起始号
   u_int uiPoEnd;   //端口结束号
}OSPF_DCN_PORT_RANGE_T;


typedef struct {
   u_int uiPortNum;   //端口号
   u_char ucEnable;	//使能状态
}OSPF_DCN_PORT_ENABLE_T;

enum {  
	OSPF_DCN_DISABLE = 0,		//DCN使能关闭	0
	OSPF_DCN_ENABLE,	//DCN使能开启1
	OSPF_DCN_PORT_ENABLE,	//DCN端口使能开启2,端口恢复时赋值
	OSPF_DCN_PORT_DISABLE	//DCN端口使能关闭3,端口恢复时赋值
};

enum {  
    DCN_PORT_LOCAL_MNG_DEL = 0,		//DCN端口取消引入lsa
	DCN_PORT_LOCAL_MNG_ADD = 1,	//DCN端口引入lsa
};



int ospf_dcn_create();
int ospf_dcn_delete();
int ospf_dcn_set_eth_bandwidth(u_int uiLogicPort, u_int uiWidth, u_int uiVlan, u_int uiMode);
u_int ospf_dcn_modify_all_eth_bandwidth(u_int uiWidth);
u_int ospf_dcn_modify_ip(u_int ulIp,u_int ulMask);
int ospf_dcn_get_flag();
int ospf_dcn_get_api(u_int uiCmd,void *pValue);
int ospf_dcn_set_default();
int ospf_DcnLoginInterface(u_int uiLpbkId);
int ospf_dcn_init_data(void);
int ospf_dcn_delete_data();
int ospf_dcn_set_api(u_int uiCmd,void *pValue);
void ospf_dcn_change_all_network(tOSPF_NETWORK_INDEX *p_index);
int ospf_dcn_addr_update(u_int uiPo);
int ospf_dcn_setSubIntf_Mod(u_long ulIfIndx,u_char ucEnmode);
int ospf_dcn_peer_update(u_int uiIfIndex,u_long ulPeerIp);
int ospf_dcn_slot_remove(u_int uiSlot);
int ospf_dcn_slot_init(u_int uiSlot);
int ospf_dcn_neid_set(u_int uiNeid);
int ospf_dcn_lpbk_ip_check(u_int uiIfindx);
int ospf_dcn_get_first_route(u_long *pIpAddr);
int ospf_dcn_get_next_route(u_long *pFirstIpAddr,u_long *pNextIpAddr);
int ospf_dcn_RouteTab_get_api(u_long IpAddr,void *pbuf,u_int mib_num);
int ospf_dcn_get_bandwidth(u_int *puiBand);
int ospf_dcn_set_bandwidth(u_int uiBand);
int ospf_dcn_rx_pkt_addr_get(u_long ulIfIndex,u_long ulAddr,u_char *pucMac);
u_long ospf_dcn_get_rx_pkt_addr(u_long ulIfIndex,u_long *pulAddr);
void ospf_dcn_recv_count();
u_int ospf_dcn_change_nbr(tOSPF_NETWORK_INDEX *p_index,u_char ucPort);
int ospf_dcn_overlay_cfg_save(OSPF_DCN_OVERLAY_T *pstOverlay);
int ospf_dcn_set_overlay(u_char ucPort,u_long ulIpaddr,u_long ulMasklen,u_char ucMode);



u_int ospf_dcn_rxmt_cmp(struct ospf_dcn_rxmt_node * p1, struct ospf_dcn_rxmt_node * p2);
struct ospf_dcn_rxmt_node *ospf_dcn_rxmt_lookup(struct ospf_process * p_process, struct ospf_lshdr * p_hdr);
struct ospf_dcn_rxmt_node* ospf_dcn_rxmt_add(struct ospf_lsa *p_lsa);
void ospf_dcn_rxmt_del(struct ospf_process * p_process, struct ospf_dcn_rxmt_node * p_msg);
void ospf_rtmsg_dcn_lsa_send(struct ospf_process *p_process);
void ospf_dcn_if_set(u_int ulPrid,u_int ulIp);
void dcn_print(u_int8 *ucbuf,u_int len);
void ospf_display_dcn(struct vty *vty) ;
void ospf_dcn_get_type_10(struct ospf_dcn_rxmt_node *p_node,OSPF_DCN_NBR_T *pstNbr);
//void ospf_dcn_get_type_10_lsa_hdr(struct ospf_lshdr *p_lshdr,uint8_t* ubuf);
int ospf_dcn_create_by_ifindex(u_int uiIndx);
int ospf_dcn_delete_port_by_ifindex(u_int uiIndx);
int ospf_DcnPort_Creat_all();
int ospf_dcn_enable_port(u_long ulIfIndx,u_char ucEnmode);
int ospf_dcn_get_init(void);
int ospf_dcn_set_init_flag(u_char ucData);
int ospf_getDcnLogin(void);
int ospf_setDcnLogin(u_char ucData);
void ospf_dcn_if_update(struct ospf_if *p_if);
void ospf_dcn_upate_if();
int ospf_dcn_port_get_first(u_int *pulIfIndex);
int ospf_dcn_port_get_next(u_int *pulIfIndex, u_int *pulNextIfIndex);
int ospf_dcn_Creat_vpif_interface(unsigned int uiIfIndex);
int ospf_dcn_Del_vpif_interface(unsigned int uiIfIndex);
u_int ospf_dcn_vpif_lport_subid_to_index(u_int uiLportNo, u_int uiSubid);
int ospf_DcnPortVlan_api(u_long ulIfIndx,u_long ulVlan,u_char ucVmode);
int ospf_dcn_set_dcn_mode_by_ifindex(u_long ulIfIndex, u_long ulDcnMode);
int ospf_dcn_delete_by_ifindex(u_int ulIp,u_char ucPort);
int ospf_dcn_set_port_enable(u_char ucSlot,u_char ucPort,u_char ucData);
int ospf_dcn_vlan_get(u_int *uiVlan);
int ospf_dcn_vlan_set(u_int uiVlanid);
int ospf_dcn_Creat_vpif_interface(unsigned int uiIfIndex);
int ospf_dcn_vlan_hw_creat(u_int uiVlanid);
int ospf_dcn_lpbk_interface_creat(u_int uilpid);
int ospf_dcn_port_vpif_create(u_int uiLportNo);

int ospf_dcn_check_by_ifindex(u_long ulIfIndx);
int ospf_dcn_remove_port_by_ifindex(u_int uiIndx);
void ospf_dcn_if_neid_update(u_int uiNeid);
int ospf_dcn_vlan_to_default();
int ospf_set_acl(u_int ulIfIndx, u_int uiMode);
int ospf_dcn_delete_acl();
int ospf_dcn_overlay_ipaddr_install(u_int uiLport,u_long ulAddr,u_long ulMasklen,u_char ucMode);
int ospf_dcn_overlay_network_get(u_int ulNetWorkIp,u_int ulNetWorkMask);
int ospf_dcn_overlay_network_set(u_int ulNetWorkIp,u_int ulNetWorkMask);
int ospf_dcn_overlay_network_del(u_int ulNetWorkIp,u_int ulNetWorkMask);
int ospf_dcn_overlay_ipaddr_uninstall(u_int uiLport,u_long ulAddr,u_long ulMasklen,u_char ucMode);
int lldp_overlay_ipaddr_aging(u_int uiLport);
int ospf_dcn_lookup_overlay_port(u_int uiPort);
int ospf_dcn_lookup_master_port(u_int uiPort,u_int *puiMasIp,u_int *puiMasMasklen);
int ospf_dcn_lookup_slave_port(u_int uiPort,u_int *puiSlaveIp,u_int *puiSlaveMasklen);
int ospf_hello_overlay_check (struct ospf_if *p_if,u_int uiNbrAddr,u_int uiNbrRouid);
int ospf_dcn_overlay_network_timer( struct ospf_process *p_process,u_int uiflag);
void ospf_dcn_overlay_network_timer_pro(struct ospf_process *p_process);

int ospf_dcn_local_manage_cfg(u_int uiIfIndex,u_char ucMode);
int ospf_dcn_local_manage_get(u_int uiIfIndex,tOSPF_LSA_NET *pstLsaMng);

void ospf_dcn_debug_print(struct vty *vty);

int ospf_overlay_if_master_check (struct ospf_if *p_if);
int ospf_dcn_del_overlay_config(u_long ulLPortNum);
void ospf_dcn_modify_overlay_vlan(u_long ulPort);
void lag_dcn_destroy_and_create(); 
u_long ospf_dcn_get_ip();
void ospf_dcn_set_globale_enable(u_char ucData);
int ospf_dcn_add_vlan_port(u_int uiSlot,u_int uiVlanid,u_char cmd);
int ospf_dcn_create_network(tOSPF_NETWORK_INDEX *p_Index, u_char ucPort);
int ospf_dcn_global_vlan_modify(u_int uiVlanId);
int ospf_dcn_get_port_enable_status(u_int ulIfIndx);
STATUS dcnSetApi(void *index, u_int cmd, void *var);
#endif

#ifdef __cplusplus
}
#endif
#endif
