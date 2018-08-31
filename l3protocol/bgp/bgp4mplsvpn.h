#include "plateform.h"
#include "bgp4path.h"
#ifdef NEW_BGP_WANTED
#ifndef  BGP4MPLSVPN_H
#define   BGP4MPLSVPN_H
#ifdef __cplusplus
 extern "C" {
#endif

#ifdef BGP_VPLS_WANTED

#endif
typedef struct
{
    u_char dest_type;
    u_char next_hop_type;
    u_char route_direction;
    u_char dest[16];
    u_char next_hop[16];     /*tunnel dest*/
    u_char tunnel_source[16];/*tunnel source*/
    u_int mask;
    u_int vrf_id;
    u_int vc_label; /*out_label*/
    u_int uiInLabel; /*in_label*/
    u_int uiIndex;
}tMplsVpnRouteEntry;



#define RTM_VPLSVCLABEL_ADD 1
#define RTM_VPLSVCLABEL_DEL 2


#define MPLSL3VPN_ADD_ROUTE 1
#define MPLSL3VPN_DEL_ROUTE 2

#define MAX_LOCAL_VRF 128


typedef enum
{
    MPLSL3VPN_ROUTE_LOCAL,
    MPLSL3VPN_ROUTE_REMOTE,
    MPLS_6PE_LOCAL_ROUTE,
    MPLS_6PE_REMOTE_ROUTE,
}BGP4_VPN_ROUTE_DIRECTION_E;

typedef enum
{
    MPLSL3VPN_GET_LOCALVRF,
    MPLSL3VPN_GET_REMOTEVRF,
    MPLSL3VPN_GET_VIDANDRD,
    MPLSL3VPN_GET_EXPORTRT,
    MPLSL3VPN_GET_RD_BY_VRFID,
}BGP4_VPN_MSG_TYPE_E;

typedef enum
{
    BGP4_AF_IP = 1,    /*caoyongÐÞ¸Ä201711-13*/
    BGP4_AF_IP6,
    BGP4_AF_L2VPN = 196,
    BGP4_AF_VPLS = 25
}BGP4_AFI_E;

enum {
	BGP4_VPN_IMPORT_RT_ADD=1,
	BGP4_VPN_EXPORT_RT_ADD,
	BGP4_VPN_IMPORT_RT_DEL,
	BGP4_VPN_EXPORT_RT_DEL
};

/*RD type*/
enum{
   BGP4_RD_TYPE_AS,
   BGP4_RD_TYPE_IP, 
   BGP4_RD_TYPE_LONG_AS,
};

/*vpls encapsulation type*/
#define PWTYPE_ETHERNET_TAGGED_MODE 0x0004
#define PWTYPE_ETHERNET 0x0005

enum
{
    BGP4_L2VPNVPLS_ADD = 1,
    BGP4_L2VPNVPLS_DEL,
    BGP4_L2VPNVPLS_CHANGE
};

enum 
{
    BGP4_L2VPN_OFFSET = 1,
    BGP4_L2VPN_SITEID,
    BGP4_L2VPN_LABELRANGE, 
    BGP4_L2VPN_MTU,
    BGP4_L2VPN_ENCAPTYPE, 
    BGP4_L2VPN_CTRLWORD,
    BGP4_L2VPN_RD
};

enum {
    BGP4_L2VPN_IMPORT_RT_ADD=1,
    BGP4_L2VPN_EXPORT_RT_ADD,
    BGP4_L2VPN_IMPORT_RT_DEL,
    BGP4_L2VPN_EXPORT_RT_DEL
};

typedef struct 
{
    u_char vrfRT[BGP4_VPN_RT_LEN];
    u_int vrfId;
}tBgpL3VpnInfo;

typedef struct 
{
    u_char inRT[BGP4_VPN_RT_LEN];
    u_int type;
    u_int inVrfId;
    u_int maxNum;
    u_int count;
    tBgpL3VpnInfo *outData;
}tBgpLookupL3Vpn;

typedef struct
{
    u_char  nexthop[24];
    u_short in_label;
    u_short out_label;
    u_int vsi;
}MPLS_BGP_VPLS;

struct l2vpnVcLabel_msg
{
   u_short in_label;
   u_short out_label;
   u_int vpnId;
};


struct l2vpnVcLabel_msghdr
{
   u_char  rtm_version;
   u_char  rtm_type;
   u_short rtm_msglen;
   u_int cnt;
};

struct vplsInstanceIndexInfohdr
{
    u_int cnt ;
    u_int *vplsInstanceIndex;
};

#define BGP4_ROUTE_INFO_MAX_NUM 15
typedef struct
{
    char rt[BGP4_VPN_RT_LEN];
}BGP4_ROUTE_INFO_T;

struct vplsRtInfohdr 
{
   BGP4_ROUTE_INFO_T rtInfo[BGP4_ROUTE_INFO_MAX_NUM];
};


struct l2vpn_msg
{
    u_char ctrlWord;
    u_char encapType;
    u_short mtu;
    u_short offset;
    u_short labelRange;
    u_short siteId;
    u_int vpnId;
};


u_int bgp4_vpn_upe_peer_count_update(tBGP4_VPN_INSTANCE *p_instance);
u_int bgp4_route_dest_vpls_get(tBGP4_ROUTE *p_route, u_int *p_vrid);
u_int bgp4_route_dest_vrf_get(tBGP4_ROUTE * p_route, u_int * p_vrf);
u_int bgp4_vrf_route_export_enable(tBGP4_ROUTE *p_route, tBGP4_VPN_INSTANCE *p_instance);
int bgp4_sys_mpls_route_update(tBGP4_ROUTE* p_route);
int bgp4_init_private_instance(void);
void bgp4_mplsvpn_local_vpn_init(void);
void bgp4_rtsock_import_rtarget_add(tBGP4_VPN_INSTANCE *p_instance);
void bgp4_rtsock_import_rtarget_delete(tBGP4_VPN_INSTANCE *p_instance);
void bgp4_rtsock_export_rtarget_add(tBGP4_VPN_INSTANCE *p_instance);
void bgp4_rtsock_export_rtarget_delete(tBGP4_VPN_INSTANCE *p_instance);
void bgp4_vpls_addr_get(tBGP4_ADDR *p_addr, tBGP4_VPLS_ADDR *p_vpls_addr);
void bgp4_vpls_addr_set(tBGP4_ADDR *p_addr, u_char *rd, tBGP4_VPLS_ADDR *p_vpls_addr);
void bgp4_vpls_local_route_fill(tBGP4_VPN_INSTANCE *p_instance, tBGP4_ROUTE *p_route);
void bgp4_vrf_route_export_check(tBGP4_ROUTE *p_route, u_int feasible);
void bgp4_vpls_add_event_process(struct l2vpn_msg *p_msg);
void bgp4_vpls_del_event_process(struct l2vpn_msg *p_msg);
void bgp4_vpls_update_event_process(struct l2vpn_msg *p_msg);
void bgp4_vrf_route_translate(tBGP4_ROUTE * p_route, tBGP4_ROUTE * p_dest, u_int feasible);
void bgp4_upe_default_route_update(tBGP4_VPN_INSTANCE *p_instance);
void bgp4_vpn_route_in_label_get(tBGP4_ROUTE *p_route);
STATUS bgp4_sys_vpls_xc_add(tBGP4_ROUTE *p_route);
STATUS bgp4_sys_vpls_xc_delete(tBGP4_ROUTE *p_route);
STATUS bgp4_sys_vpls_msg_send(tBGP4_ROUTE *p_route, u_int active);
int bgp4_init_one_private_instance(u_int uiVrfId);
extern int mplsLookupL3Vpn(tBgpLookupL3Vpn *pL3VpnInfo);

#ifdef __cplusplus
}
#endif
#endif

#else

#ifndef  BGP4MPLSVPN_H
#define   BGP4MPLSVPN_H
#ifdef __cplusplus
      extern "C" {
     #endif

enum {
    BGP4_VPN_IMPORT_RT_ADD=1,
    BGP4_VPN_EXPORT_RT_ADD,
    BGP4_VPN_IMPORT_RT_DEL,
    BGP4_VPN_EXPORT_RT_DEL

}BGP_VPN_RT_CHANGE;


u_int bgp4_mpls_vpn_get_vrf_id_list(tBGP4_ROUTE*p_route,
                                    u_int action,
                                    tBGP4_VPN_INSTANCE* p_src_instance,
                                    u_int* direction,
                                    u_int vrf_id_array[BGP4_MAX_VRF_ID]);
tBGP4_ROUTE* bgp4_mpls_rebuild_local_vpn_route(u_int src_vrf_id,u_int direction,tBGP4_VPN_INSTANCE* p_instance,tBGP4_ROUTE* p_origin_route);
int bgp4_mpls_vpn_route_notify(tBGP4_ROUTE* p_route);
tBGP4_VPN_INSTANCE* bgp4_mplsvpn_add(u_int vrf_id);
void bgp4_mplsvpn_delete(tBGP4_VPN_INSTANCE* p_instance);
void bgp4_mplsvpn_change(tBGP4_VPN_INSTANCE* p_instance,u_char change_type);
void bgp4_mplsvpn_local_vpn_init(void);
u_char* bgp4_translate_vpn_RD(tBGP4_ROUTE *p_route ,octetstring* p_rd_str);
u_int bgp4_translate_vpn_label(u_int vpn_label);
int bgp4_init_private_instance();


#ifdef __cplusplus
     }
     #endif
#endif

#endif
