
#include "bgp4_api.h"
#include "bgp4com.h"
#ifdef NEW_BGP_WANTED
#ifndef  BGP4MSGH_H
#define   BGP4MSGH_H
#ifdef __cplusplus
 extern "C" {
#endif
#define BGP4_VERSION_4 0x04

/* BGP Message lengths */
#define BGP4_MARKER_LEN 16
#define BGP4_HLEN 19
#define BGP4_MIN_MSG_LEN BGP4_HLEN
#define BGP4_MAX_MSG_LEN 4096 
#define BGP4_KEEPALIVE_LEN 19
#define BGP4_OPEN_HLEN 29
#define BGP4_UPDATE_HLEN 23
#define BGP4_NOTIFY_HLEN 21
#define BGP4_ROUTE_REFRESH_LEN 23

#define BGP4_VPLS_NLRI_LEN 17

/*max pdu length sended in 1S,640K/S*/
#define BGP4_MAX_FLOOD_SIZE (64 * 1024)/**10*/

#define BGP4_MAX_UNKOWN_OPT_LEN 1024

#define BGP4_OPEN_OPTION_CAPABILITY 2

/*  AS Path Segment Type values  */
enum {
    BGP_ASPATH_SET = 1 ,
    BGP_ASPATH_SEQ,
    BGP_ASPATH_CONFED_SEQ,
    BGP_ASPATH_CONFED_SET
};

/*  Origin Type values  */
enum {
  BGP4_ORIGIN_IGP = 0 ,
  BGP4_ORIGIN_EGP = 0x01,
  BGP4_ORIGIN_INCOMPLETE = 0x02
};
/*ORF type*/
enum {
   BGP4_ORF_TYPE_COMMUNITY = 2,
   BGP4_ORF_TYPE_EXTCOMMUNITY = 3,
   BGP4_ORF_TYPE_IPPREFIX = 64
};

/*orf send/recv flag*/
#define BGP4_ORF_SEND_FLAG   0x02
#define BGP4_ORF_RECV_FLAG   0x01

/*Orf when to refresh flag*/
enum {
    BGP4_ORF_FRESH_IMMEDIATE = 1,
    BGP4_ORF_FRESH_DEFER = 2
};

/*orf action*/
enum {
    BGP4_ORF_ACTION_ADD = 0,
    BGP4_ORF_ACTION_REMOVE,
    BGP4_ORF_MATCH_DENY,
    BGP4_ORF_MATCH_PERMIT,
    BGP4_ORF_ACTION_REMOVE_ALL
};

/*Capability type in Open*/
enum {
    BGP4_CAP_MULTI_PROTOCOL = 1,/*multi protocol */
    BGP4_CAP_ROUTE_REFRESH = 2,/*rt refresh*/
    BGP4_CAP_ORF = 3,/*draft-ietf-idr-route-filter-10.txt*/
    BGP4_CAP_4BYTE_AS = 65,/*4bytes as number.a test value.*/
    BGP4_CAP_GRACEFUL_RESTART = 64/*graceful restart*/
};
/* BGP4 Notification Codes */
enum {
    BGP4_MSG_HDR_ERR = 0x01,
    BGP4_OPEN_MSG_ERR,
    BGP4_UPDATE_MSG_ERR,
    BGP4_HOLD_TMR_EXPIRED_ERR,
    BGP4_FSM_ERR,
    BGP4_CEASE,
    BGP4_NOTIFY_MSG_ERR
};

enum {
 /* BGP4 Notification SubCodes */
            BGP4_NO_SUBCODE = 0x00,
/* Message Header Notification Subcodes */
            BGP4_CONN_NOT_SYNC = 0x01,
            BGP4_BAD_MSG_LEN,
            BGP4_BAD_MSG_TYPE,

/* OPEN message Notification Subcodes */
            BGP4_UNSUPPORTED_VER_NO = 0x01,
            BGP4_AS_UNACCEPTABLE,
            BGP4_BGPID_INCORRECT,
            BGP4_OPT_PARM_UNRECOGNIZED,
            BGP4_AUTH_PROC_FAILED,
            BGP4_HOLD_TMR_UNACCEPTABLE,
            BGP4_UNSUPPORTED_CAPABILITY,
/* UPDATE message Notification Subcode */
            BGP4_MALFORMED_ATTR_LIST = 0x01,
            BGP4_UNRECOGNISED_WELLKNOWN_ATTR,
            BGP4_MISSING_WELLKNOWN_ATTR,
            BGP4_ATTR_FLAG_ERR,
            BGP4_ATTR_LEN_ERR,
            BGP4_INVALID_ORIGIN,
            BGP4_AS_ROUTING_LOOP,
            BGP4_INVALID_NEXTHOP,
            BGP4_OPTIONAL_ATTR_ERR,
            BGP4_INVALID_NLRI,
            BGP4_MALFORMED_AS_PATH,
/* error code cease*/
            BGP4_OVER_MAX_PREFIX = 0x01,
            BGP4_ADMINISTRATIVE_SHUTDOWN,
            BGP4_PEER_NOT_CONFIGURED,
            BGP4_ADMINISTRATIVE_RESET,
            BGP4_CONNECTION_REJECTED,
            BGP4_OTHER_CONFIGURATION_CHANGE,
            BGP4_CONNECTION_COLLISION_RESOLUTION,
            BGP4_OUT_OF_RESOURCES
};

/*attribute type*/
enum {
           BGP4_ATTR_ORIGIN = 0x01,
           BGP4_ATTR_PATH = 0x02,
           BGP4_ATTR_NEXT_HOP = 0x03,
           BGP4_ATTR_MED = 0x04,
           BGP4_ATTR_LOCAL_PREF = 0x05,
           BGP4_ATTR_ATOMIC_AGGR = 0x06,
           BGP4_ATTR_AGGREGATOR = 0x07,
           BGP4_ATTR_COMMUNITY = 0x08,
/*add by cheng to implement route refletor*/
           BGP4_ATTR_ORIGINATOR = 0x09,
           BGP4_ATTR_CLUSTERLIST = 0x0a,
/*end add*/
           BGP4_ATTR_UNKNOWN = 0x0b ,/*not change */
           BGP4_ATTR_MP_NLRI = 0x0e,
           BGP4_ATTR_MP_UNREACH_NLRI = 0x0f,
           BGP4_ATTR_EXT_COMMUNITY = 0x10,
/*4bytes as externsion,just test value .*/
           BGP4_ATTR_NEW_PATH = 0x11,
           BGP4_ATTR_NEW_AGGREGATOR = 0x12
} ;

/* Attribute Length */
enum {
   BGP4_ORIGIN_LEN = 0x01,
   BGP4_NEXTHOP_LEN = 0x04 ,
   BGP4_MED_LEN = 0x04 ,
   BGP4_LPREF_LEN = 0x04 ,
   BGP4_ATOMIC_LEN = 0x00 ,
   BGP4_AGGR_LEN = 0x06 ,
   BGP4_ORIGINATOR_LEN = 0x04 
};

/* BGP Message Types */
enum {
   BGP4_MSG_OPEN = 0x01,  
   BGP4_MSG_UPDATE, 
   BGP4_MSG_NOTIFY, 
   BGP4_MSG_KEEPALIVE, 
   BGP4_MSG_REFRESH 
};

/*  Attribute Flags  */
#define BGP4_ATTR_FLAG_OPT 0x80
#define BGP4_ATTR_FLAG_TRANSIT 0x40
#define BGP4_ATTR_FLAG_PARTIAL 0x20 
#define BGP4_ATTR_FLAG_EXT 0x10

/*community*/
enum {
    BGP4_COMMUNITY_NOEXPORT = 0xFFFFFF01,
    BGP4_COMMUNITY_NOADV = 0xFFFFFF02,
    BGP4_COMMUNITY_NOEXPORT_SUSPEND = 0xFFFFFF03
};

#define BGP4_GR_RESET_FLAG 0x8000
#define BGP4_GR_FORWARDING_FLAG 0x80

#define BGP4_SELECTION_DEFERRALTIME 300

enum {
    BGP4_GR_ROLE_NORMAL = 0,
    BGP4_GR_ROLE_RESTART,
    BGP4_GR_ROLE_RECIEVE 
};

#define BGP4_AS_TRANS 23456

#pragma pack (1)
typedef struct bgp4_msghdr {
    u_char marker[BGP4_MARKER_LEN];
    
    u_short len;
    
    u_char type;
} tBGP4_MSGHDR;
#pragma pack ()

#pragma pack (1)
typedef struct bgp4_openmsg {
    u_char version;
    
    u_short asnum;
    
    u_short holdtime;
    
    u_int bgp_id;
    
    u_char opt_len;
} tBGP4_OPENMSG;
#pragma pack ()

#pragma pack (1)
typedef struct bgp4_notify {
    u_char code;
    
    u_char sub_code;
} tBGP4_NOTIFYMSG;
#pragma pack ()

/*AFI and SAFI id in a struct.in host order*/
typedef struct bgp4_af{
    u_short afi;

    u_char rsvd;

    u_char safi;
}tBGP4_AFID;

/*rxd update msg using internal struct*/
typedef struct bgp4_update_packet{
    /*sending peer*/
    struct bgp4_peer *p_peer;

    /*address family in this update*/
    u_int af;
    
    /*pointer to origin buf,used in sending*/
    u_char *p_buf;
    
    /*pointer to msg,pdu hdr is skipped*/
    u_char *p_msg;

    /*total msg length*/
    u_short len;

    /*withdraw route length*/
    u_short withdraw_len;

    /*attribute length*/
    u_short attribute_len;

    u_int withdraw_count;
    
    u_int feasible_count;
    
    /*mpbgp nexthop*/
    u_char mp_nexthop[64];

    /*bytes of nexthop length*/
    u_int mp_nexthop_len;

    /*mpbgp path attribute.do not include mp-reach and mp-unreach*/
    u_char mp_path[2048];

    /*length of */
    u_int mp_path_len;

    /*mp-unreach nlri filled,do not include option hdr
      and nexthop*/    
    u_char *p_mp_unreach;
    
    /*total length of mp_nlri*/
    u_int mp_unreach_len;

    /*mp-unreach nlri filled,do not include option hdr
      and nexthop*/    
    u_char *p_mp_reach;
    
    /*total length of mp_nlri*/
    u_int mp_reach_len;

}tBGP4_UPDATE_PACKET; 

/*rxd update msg using internal struct*/
typedef struct bgp4_rxupdate_packet{
    struct bgp4_peer *p_peer;

    /*pointer to msg,pdu hdr is skipped*/
    u_char *p_msg;

    /*total msg length*/
    u_short len;

    /*withdraw route length*/
    u_short withdraw_len;

    /*attribute length*/
    u_short attribute_len;

    /*NLRI length*/
    u_short nlri_len;
    
    /*pointer to withdraw route*/
    u_char *p_withdraw;

    /*pointer to path attribute*/
    u_char *p_attribute;

    /*pointer to nlri*/
    u_char *p_nlri;    

    /*pointer to each option,indexed by type*/
    u_char *p_option[256];

    /*hdr length*/
    u_char option_hlen[256];

    /*payload length*/
    u_short option_len[256];    
}tBGP4_RXUPDATE_PACKET; 

typedef struct bgp4_restart_capability{
    u_short afi;
       
    u_char safi;

    u_char flag;    
}tBGP4_RESTART_AFID;

typedef struct bgp4_rxopen_packet{
    u_char *p_pdu;

    u_char refresh_enable;

    u_char version;

    u_char restart_enable;

    u_char in_restart;
    
    u_int as;

    u_short hold_time;

    u_short restart_time;
    
    u_int router_id;
    
    u_int mpbgp_capability;

    u_int restart_mpbgp_capability;

    u_int as4_enable;

    /*include all orf send capability*/
    u_int orf_send_capability;

    /*include all orf recv capability*/
    u_int orf_recv_capability;
}tBGP4_RXOPEN_PACKET;

#define bgp4_init_msg_hdr(p_msg, type) do {\
    u_int *p_mark = (u_int *)(p_msg);\
    p_mark[0] = p_mark[1] = p_mark[2] = p_mark[3] = 0xFFFFFFFF ;\
    (p_msg)[BGP4_MARKER_LEN + 2] = (type);}while(0)

#define bgp4_update_packet_init(x) do\
{\
    (x)->p_msg = (x)->p_buf + BGP4_HLEN + 2;\
    (x)->len = BGP4_HLEN + 4;\
    (x)->withdraw_len = 0;\
    (x)->attribute_len = 0;\
    (x)->withdraw_count = 0;\
    (x)->feasible_count = 0;\
    (x)->mp_nexthop_len = 0;\
    (x)->mp_path_len = 0;\
    (x)->mp_reach_len = 0;\
    (x)->mp_unreach_len = 0;\
}while(0)
struct bgp4_route;
void bgp4_keepalive_output(struct bgp4_peer *p_peer);
void bgp4_open_output(struct bgp4_peer * p_peer);
void bgp4_notify_output(struct bgp4_peer *p_peer);
void bgp4_refresh_output(struct bgp4_peer * p_peer, u_int af_flag, avl_tree_t * p_orf_table);
u_int bgp4_update_output(tBGP4_UPDATE_PACKET *p_packet);
STATUS bgp4_update_packet_withdraw_insert(tBGP4_UPDATE_PACKET *p_packet, struct bgp4_route *p_route);
STATUS bgp4_update_packet_feasible_insert(tBGP4_UPDATE_PACKET *p_packet, struct bgp4_route *p_route);
STATUS bgp4_update_packet_mpunreach_insert(tBGP4_UPDATE_PACKET *p_packet, struct bgp4_route *p_route);
STATUS bgp4_update_packet_mpreach_insert(tBGP4_UPDATE_PACKET *p_packet, struct bgp4_route *p_route);

#ifdef __cplusplus
}
#endif  

#endif

#else

#ifndef  BGP4MSGH_H
#define   BGP4MSGH_H
#ifdef __cplusplus
      extern "C" {
     #endif
#define BGP4_VERSION_4 0x04

/* BGP Message lengths */
#define BGP4_MARKER_LEN 16
#define BGP4_HLEN              19
#define BGP4_MIN_MSG_LEN BGP4_HLEN
#define BGP4_MAX_MSG_LEN 4096 
#define BGP4_KEEPALIVE_LEN 19
#define BGP4_OPEN_HLEN 29
#define BGP4_UPDATE_HLEN 23
#define BGP4_NOTIFY_HLEN 21
#define BGP4_ROUTE_REFRESH_LEN 23

/*  AS Path Segment Type values  */
enum {
    BGP_ASPATH_SET = 1 ,
    BGP_ASPATH_SEQ,
    BGP_ASPATH_CONFED_SEQ,
    BGP_ASPATH_CONFED_SET
};

/*  Origin Type values  */
enum {
  BGP4_ORIGIN_IGP = 0 ,
  BGP4_ORIGIN_EGP = 0x01,
  BGP4_ORIGIN_INCOMPLETE = 0x02
};
/*ORF type*/
enum {
           BGP4_ORF_TYPE_COMMUNITY = 2,
           BGP4_ORF_TYPE_EXTCOMMUNITY = 3
};

/*orf send/recv flag*/
#define BGP4_ORF_SEND_FLAG   0x02
#define BGP4_ORF_RECV_FLAG   0x01

/*Orf when to refresh flag*/
enum {
           BGP4_ORF_FRESH_IMMEDIATE = 1,
           BGP4_ORF_FRESH_DEFER = 2
};

/*orf action*/
enum {
           BGP4_ORF_ACTION_ADD = 0,
           BGP4_ORF_ACTION_REMOVE,
           BGP4_ORF_MATCH_DENY,
           BGP4_ORF_MATCH_PERMIT,
           BGP4_ORF_ACTION_REMOVE_ALL
};

/*Capability type in Open*/
enum {
           BGP4_CAP_MULTI_PROTOCOL = 1,/*multi protocol */
           BGP4_CAP_ROUTE_REFRESH = 2,/*rt refresh*/
           BGP4_CAP_ORF = 3,/*draft-ietf-idr-route-filter-10.txt*/
           BGP4_CAP_4BYTE_AS = 6,/*4bytes as number.a test value.*/
           BGP4_CAP_GRACEFUL_RESTART = 64/*graceful restart*/
};
/* BGP4 Notification Codes */
 enum {
            BGP4_MSG_HDR_ERR = 0x01,
            BGP4_OPEN_MSG_ERR,
            BGP4_UPDATE_MSG_ERR,
            BGP4_HOLD_TMR_EXPIRED_ERR,
            BGP4_FSM_ERR,
            BGP4_CEASE,
            BGP4_NOTIFY_MSG_ERR
    };

enum {
    /* BGP4 Notification SubCodes */
            BGP4_NO_SUBCODE = 0x00,
/* Message Header Notification Subcodes */
            BGP4_CONN_NOT_SYNC = 0x01,
            BGP4_BAD_MSG_LEN,
            BGP4_BAD_MSG_TYPE,

/* OPEN message Notification Subcodes */
            BGP4_UNSUPPORTED_VER_NO = 0x01,
            BGP4_AS_UNACCEPTABLE,
            BGP4_BGPID_INCORRECT,
            BGP4_OPT_PARM_UNRECOGNIZED,
            BGP4_AUTH_PROC_FAILED,
            BGP4_HOLD_TMR_UNACCEPTABLE,
            BGP4_UNSUPPORTED_CAPABILITY,
/* UPDATE message Notification Subcode */
            BGP4_MALFORMED_ATTR_LIST = 0x01,
            BGP4_UNRECOGNISED_WELLKNOWN_ATTR,
            BGP4_MISSING_WELLKNOWN_ATTR,
            BGP4_ATTR_FLAG_ERR,
            BGP4_ATTR_LEN_ERR,
            BGP4_INVALID_ORIGIN,
            BGP4_AS_ROUTING_LOOP,
            BGP4_INVALID_NEXTHOP,
            BGP4_OPTIONAL_ATTR_ERR,
            BGP4_INVALID_NLRI,
            BGP4_MALFORMED_AS_PATH,
/* error code cease*/
            BGP4_OVER_MAX_PREFIX = 0x01,
            BGP4_ADMINISTRATIVE_SHUTDOWN,
            BGP4_PEER_NOT_CONFIGURED,
            BGP4_ADMINISTRATIVE_RESET,
            BGP4_CONNECTION_REJECTED,
            BGP4_OTHER_CONFIGURATION_CHANGE,
            BGP4_CONNECTION_COLLISION_RESOLUTION,
            BGP4_OUT_OF_RESOURCES,
            BGP4_CEASE_ERROR_MAX_PREFIX = 20
};

/*attribute type*/
enum {
           BGP4_ATTR_ORIGIN = 0x01,
           BGP4_ATTR_PATH = 0x02,
           BGP4_ATTR_NEXT_HOP = 0x03,
           BGP4_ATTR_MED = 0x04,
           BGP4_ATTR_LOCAL_PREF = 0x05,
           BGP4_ATTR_ATOMIC_AGGR = 0x06,
           BGP4_ATTR_AGGREGATOR = 0x07,
           BGP4_ATTR_COMMUNITY = 0x08,
/*add by cheng to implement route refletor*/
           BGP4_ATTR_ORIGINATOR = 0x09,
           BGP4_ATTR_CLUSTERLIST = 0x0a,
/*end add*/
           BGP4_ATTR_UNKNOWN = 0x0b ,/*not change */
           BGP4_ATTR_MP_NLRI = 0x0e,
           BGP4_ATTR_MP_UNREACH_NLRI = 0x0f,
           BGP4_ATTR_EXT_COMMUNITY = 0x10,
/*4bytes as externsion,just test value .*/
           BGP4_ATTR_NEW_PATH = 0x11,
           BGP4_ATTR_NEW_AGGREGATOR = 0x12
} ;

/* Attribute Length */
enum {
           BGP4_ORIGIN_LEN = 0x01,
           BGP4_NEXTHOP_LEN = 0x04 ,
           BGP4_MED_LEN = 0x04 ,
           BGP4_LPREF_LEN = 0x04 ,
           BGP4_ATOMIC_LEN = 0x00 ,
           BGP4_AGGR_LEN = 0x06 ,
           BGP4_ORIGINATOR_LEN = 0x04 
};

/* BGP Message Types */
enum {
           BGP4_MSG_OPEN = 0x01,  
           BGP4_MSG_UPDATE, 
           BGP4_MSG_NOTIFY, 
           BGP4_MSG_KEEPALIVE, 
           BGP4_MSG_REFRESH 
};

/*  Attribute Flags  */
#define BGP4_ATTR_FLAG_OPT           0x80
#define BGP4_ATTR_FLAG_TRANSIT         0x40
#define BGP4_ATTR_FLAG_PARTIAL            0x20 
#define BGP4_ATTR_FLAG_EXT            0x10

/*attribute flag check*/
#define BGP4_ATTR_FLAG_CHECK(x, y) ((x) & (BGP4_ATTR_FLAG_##y))

/*community*/
enum {
            BGP4_COMMUNITY_NOEXPORT = 0xFFFFFF01,
            BGP4_COMMUNITY_NOADV = 0xFFFFFF02,
            BGP4_COMMUNITY_NOEXPORT_SUSPEND = 0xFFFFFF03
};

#define BGP4_GR_RESET_FLAG 0x8000
#define BGP4_GR_FORWARDING_FLAG 0x80
#define BGP4_SELECTION_DEFERRALTIME 300

enum {
            BGP4_GR_ROLE_NORMAL = 0,
            BGP4_GR_ROLE_RESTART,
            BGP4_GR_ROLE_RECIEVE 
};

#if !defined(WIN32)
/* These things should be packed */
typedef struct bgp4_msghdr {
    u_char           marker[BGP4_MARKER_LEN];
    u_short           len;
    u_char           type;
} __attribute__ ((packed)) tBGP4_MSGHDR;
#else
#pragma pack (1)
typedef struct bgp4_msghdr {
    u_char           marker[BGP4_MARKER_LEN];
    u_short           len;
    u_char           type;
} tBGP4_MSGHDR;
#pragma pack ()
#endif
#if !defined(WIN32)
typedef struct bgp4_openmsg {
    u_char           version;
    u_short           asnum;
    u_short           holdtime;
    u_int           bgp_id;
    u_char           opt_len;
} __attribute__ ((packed)) tBGP4_OPENMSG;
#else
#pragma pack (1)
typedef struct bgp4_openmsg {
    u_char           version;
    u_short           asnum;
    u_short           holdtime;
    u_int           bgp_id;
    u_char           opt_len;
} tBGP4_OPENMSG;
#pragma pack ()
#endif
#if !defined(WIN32)
typedef struct bgp4_notify {
    u_char           code;
    u_char           sub_code;
} __attribute__ ((packed)) tBGP4_NOTIFYMSG;
#else
#pragma pack (1)
typedef struct bgp4_notify {
    u_char           code;
    u_char           sub_code;
} tBGP4_NOTIFYMSG;
#pragma pack ()
#endif

#define bgp4_init_msg_hdr(p_msg, type) do {\
    u_int *p_mark = (u_int *)(p_msg);\
    p_mark[0] = p_mark[1] = p_mark[2] = p_mark[3] = 0xFFFFFFFF ;\
    (p_msg)[BGP4_MARKER_LEN + 2] = (type);}while(0)

#define bgp4_init_update(x) do\
{\
    (x)->p_msg = (x)->buf + BGP4_HLEN + 2;\
    (x)->len = BGP4_HLEN + 4;\
    (x)->with_len = (x)->attr_len = (x)->with_count = (x)->fea_count = 0;\
}while(0)

u_short bgp4_index_to_afi(u_int flag);
u_short bgp4_index_to_safi(u_int flag);
u_int bgp4_afi_to_index(u_short afi, u_short safi);
int bgp4_is_end_of_rib(u_char *p_msg,u_short len,u_short *p_afi,u_char *p_safi);
int bgp4_nlri_extract(tBGP4_PATH * p_path, u_char * p_buf, int len, avl_tree_t * p_list);
int bgp4_open_capablity_extract(tBGP4_RXOPEN_PACKET *p_peer,u_char *p_opt, u_short opt_len);
void bgp4_schedule_all_init_update();
extern int sendBgpEstablishedTrap(int addrType,char* peerid,
                               int state, char *LastError);

#ifdef __cplusplus
     }
     #endif  
#endif


#endif
