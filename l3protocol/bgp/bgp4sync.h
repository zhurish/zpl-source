

#include "bgp4_api.h"
#include "bgp4main.h"
#include "bgp4peer.h"
#include "bgp4path.h"
#ifdef NEW_BGP_WANTED
#ifndef  BGP4SYNC_H
#define   BGP4SYNC_H
#ifdef __cplusplus
extern "C" {
#endif

/*for synchronization*/
#if !defined(WIN32)


#else


#endif

#define BGP4_INIT_SYNC_DELAY_TIME 60

enum{
       BGP4_MODE_MASTER = 1,
       BGP4_MODE_SLAVE,
       BGP4_MODE_OTHER,
};

enum{
       BGP4_SYNC_PEER = 1,
       BGP4_SYNC_PDU,
       BGP4_SYNC_FRAGMENT_PDU,
       BGP4_SYNC_ORF
};


#define BGP4_SYNC_HDRLEN 4
#define BGP4_SYNC_BUFSIZE    5000/*BGP4_MAX_MSG_LEN + BGP4_SYNC_HDRLEN*/
#define BGP4_SYNC_MAX_UPDATE_SIZE 1280
#define BGP4_SYNC_MAX_ROUTE 300

/*4 bytes sync msg header*/
typedef struct bgp4SyncMsgHdr
{
    /*sync msg type*/
    u_char type;

    /*add or delete table,1:add,0:del*/
    u_char sync_action;

    /*sync msg length,not include header*/
    u_short len; 
}tBGP4_SYNC_MSG_HDR;

/*peer*/
typedef struct bgp4SyncPeer
{
    u_int vrf_id;
    
    tBGP4_ADDR remote_ip;/*index*/
    
    u_int router_id;
    
    u_int af; 
    
    u_int reset_af;   
        
    u_int neg_hold_interval;
    
    u_int neg_keep_interval;
    
    u_char refresh_enable;
    
    u_char in_restart;
    
    u_char reset_enable;
    
    u_char state;/*FSM STATE*/
    
    u_char version;
    
    u_char event;/*sync event*/
    
    u_short reset_time;

    /*rxd end-of-rib flag for each afi/safi*/
    u_int rxd_rib_end;
    
    /*statstic*/
    u_int established_transitions;

    u_int peer_if_unit;

    u_int uptime;

    u_int rx_updatetime;

    u_int rx_update;

    u_int tx_update;

    u_int rx_msg;

    u_int tx_msg;

    u_short last_errcode;

    u_short last_subcode;
    
    u_int orf_send;

    u_int orf_recv;
}tBGP4_SYNC_PEER;

/*peer local orf*/
typedef struct bgp4SyncPeerOrf
{
    u_int vrf_id;
    
    tBGP4_ADDR remote_ip;/*index*/
    
    u_int seq; 

    u_char match;

    u_char minlen;

    u_char maxlen;

    u_char rowstatus;

    tBGP4_ADDR prefix;
}tBGP4_SYNC_PEER_ORF;

/*route update msg*/
typedef struct bgp4SyncPdu
{
    u_int vrf_id;
    
    tBGP4_ADDR remote_ip;/*peer index*/

    u_int msg_len;

    u_char msg[BGP4_MAX_MSG_LEN];
}tBGP4_SYNC_PDU;

/*fragment pdu msg*/
typedef struct bgp4SyncFragPdu
{
    u_int vrf_id;
    
    tBGP4_ADDR remote_ip;/*peer index*/

    /*id of a full msg*/
    u_short msg_id;

    /*fragment id,from 0--to 4*/
    u_char fragment_id;

    u_char rsvd;

    /*payload length of this fragment*/
    u_short fragment_len;

    u_short rsvd2;
    
    u_char msg[BGP4_MAX_MSG_LEN];
}tBGP4_SYNC_FRAGMENT_PDU;

/*route update msg*/
typedef struct bgp4SyncRouteImport
{
    u_int msg_len;

    u_char import_msg[1024];
}tBGP4_SYNC_RT_IMPORT;

u_int bgp4_get_workmode(void);
u_int bgp4_sync_peer_update_send(struct bgp4_peer *p_peer, u_int count, struct bgp4_route **pp_route);
void bgp4_sync_init_send(void);
void bgp4_sync_recv(octetstring *p_octet);
void bgp4_sync_peer_send(tBGP4_PEER* p_peer);
void bgp4_sync_fsm_send(tBGP4_PEER* p_peer);
void bgp4_sync_pdu_send(tBGP4_PEER *p_peer,u_char *p_pdu,u_int pdu_len);
void bgp4_send_sync_route_import(u_char *p_pdu,u_int pdu_len);
void bgp4_sync_init_update_send(void);
void bgp4_sync_become_master(void);
void bgp4_sync_fragment_pdu_send(tBGP4_PEER *p_peer, u_char *p_pdu, u_int len);
void bgp4_sync_fragment_pdu_clear(void);

#ifdef __cplusplus
}
#endif    
#endif

#else
#ifndef  BGP4SYNC_H
#define   BGP4SYNC_H
#ifdef __cplusplus
      extern "C" {
     #endif
/*for synchronization*/
#if !defined(WIN32)


#else


#endif

enum{
       BGP4_MODE_MASTER = 1,
       BGP4_MODE_SLAVE,
       BGP4_MODE_OTHER,
};

enum{
       BGP4_SYNC_PEER = 1,
    BGP4_SYNC_PEER_FSM,
       BGP4_SYNC_ROUTE_UPDATE,
       BGP4_SYNC_ROUTE_IMPORT,
};


#define BGP4_SYNC_HDRLEN 4
#define BGP4_SYNC_BUFSIZE   BGP4_MAX_MSG_LEN + BGP4_SYNC_HDRLEN
#define BGP4_SYNC_MAX_UPDATE_SIZE BGP4_MAX_MSG_LEN


/*4 bytes sync msg header*/
typedef struct bgp4SyncMsgHdr
{
    /*sync msg type*/
        u_char type;
    
        /*add or delete table,1:add,0:del*/
        u_char sync_action;
    
        /*sync msg length,not include header*/
        u_short len; 
}tBGP4_SYNC_MSG_HDR;

/*peer*/
typedef struct bgp4SyncPeer
{
    u_int vrf_id;
    tBGP4_ADDR remote_ip;/*index*/
    
    u_int router_id;
    u_int af; 
    u_int reset_af; 
    u_int capability;        
        u_int neg_hold_interval;
        u_int neg_keep_interval;
        
    u_char refresh;
    u_char reset_bit;
    u_char reset_enable;
    u_char state;/*FSM STATE*/
    
    u_char version;
    u_char event;/*sync event*/
    u_short reset_time;

    u_int rib_end_af;/*end-of-rib flag for each afi/safi*/

    u_char  send_open_option;/*0-should not send open with option para,or otherwise*/
        u_char  unsupport_capability;/*1-has unsupport cap,should not restart,or otherwise*/
    u_char  reset_role ;/*current role in gr,normal-0,restarting-1,recving-2*/    
        u_char  revd;

    u_int  send_capability;
    
    /*statstic*/
        u_int established_transitions;
        u_int peer_if_unit;
        u_int uptime;
        u_int rx_updatetime;
        u_int rx_update;
        u_int tx_update;
        u_int rx_msg;
        u_int tx_msg;
        u_short last_errcode;
        u_short last_subcode;
    
}tBGP4_SYNC_PEER;

/*PEER FSM*/
typedef struct bgp4SyncFsm
{
    u_int vrf_id;

    tBGP4_ADDR remote_ip;/*peer index*/

    u_char state;/*FSM STATE*/

    u_char event;/*FSM EVENT*/

    u_short rsvd;

    u_int peer_if_unit;/*port unit number*/

}tBGP4_SYNC_FSM;

/*route update msg*/
typedef struct bgp4SyncPdu
{
    u_int vrf_id;
    
    tBGP4_ADDR remote_ip;/*peer index*/

    u_int msg_len;

    u_char msg[BGP4_MAX_MSG_LEN];
}tBGP4_SYNC_PDU;

/*route update msg*/
typedef struct bgp4SyncRouteImport
{
    u_int msg_len;

    u_char import_msg[1024];
}tBGP4_SYNC_RT_IMPORT;

void bgp4_tcp_sync_add(tBGP4_VPN_INSTANCE* p_instance,u_int af,u_char* p_laddr,u_char* p_faddr,u_int lport,u_int fport,int sockfd);
void bgp4_tcp_sync_del(tBGP4_VPN_INSTANCE* p_instance,u_int af,u_char* p_laddr,u_char* p_faddr,u_int lport,u_int fport,int sockfd);
u_int bgp4_get_workmode();
void bgp4_send_sync_all();
void bgp4_recv_sync_all(octetstring *p_octet);
void bgp4_send_sync_peer(tBGP4_PEER* p_peer,u_int action);
void bgp4_send_sync_fsm(tBGP4_PEER* p_peer);
void bgp4_send_sync_pdu(tBGP4_PEER *p_peer,u_char *p_pdu,u_int pdu_len);
void bgp4_send_sync_route_import(u_char *p_pdu,u_int pdu_len);
void bgp4_send_sync_route_all();
void bgp4_rebuild_sync_pdu(tBGP4_LIST* p_flist,tBGP4_LIST* p_wlist,tBGP4_PEER* p_src_peer);
void bgp4_card_up_sync(void * buf);
void bgp4_slot_up_sync(void * buf);
void bgp4_subsys_up_sync(void * buf);
void bgp4_card_role_change(void);


extern void bgp4_peer_tcp_addr_fill(tBGP4_PEER *p_peer);
extern u_char * bgp4_printf_addr(tBGP4_ADDR *p, u_char *str);
extern void bgp4_debug_packet(u_char *p_msg,u_short len);
u_short bgp4_attribute_hdr_fill(u_char * p_msg, u_char type,u_short len);

#ifdef __cplusplus
     }
     #endif    
#endif

#endif
