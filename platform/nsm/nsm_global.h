
#ifndef __NSM_GLOBAL_H__
#define __NSM_GLOBAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NSM_GLOBAL_JUMBO_MAX		    9720
#define NSM_GLOBAL_JUMBO_DEFAULT		8192
#define NSM_GLOBAL_FORWARD_DEFAULT	            zpl_true
#define NSM_GLOBAL_UNICAST_FLOOD_DEFAULT		zpl_false
#define NSM_GLOBAL_MULTICAST_FLOOD_DEFAULT	    zpl_false
#define NSM_GLOBAL_MULTICAST_LEARNING_DEFAULT	zpl_false
#define NSM_GLOBAL_BPDU_DEFAULT	                zpl_false

#ifdef ZPL_NSM_IGMP
enum nsm_proto_action
{
	NSM_PKT_PROXY,
	NSM_PKT_SNOOPING,
};

enum nsm_snoop_proto_type 
{
    NSM_SNOOP_PROTO_IGMP = 1,
	NSM_SNOOP_PROTO_IGMPQRY,
	NSM_SNOOP_PROTO_IGMPUNKNOW,
	NSM_SNOOP_PROTO_MLD,
	NSM_SNOOP_PROTO_MLDQRY,
	NSM_SNOOP_PROTO_ARP,
	NSM_SNOOP_PROTO_RARP,
	NSM_SNOOP_PROTO_DHCP,
};

typedef struct nsm_snoop_proto_s
{
    /* snooping */
    zpl_bool        igmp_snoop;
    zpl_bool        igmp_proxy;
    zpl_bool        igmpqry_snoop;
    zpl_bool        igmpqry_proxy;
    zpl_bool        igmpunknow_snoop;
    zpl_bool        igmpunknow_proxy;
    zpl_bool        mld_snoop;
    zpl_bool        mld_proxy;
    zpl_bool        mldqry_snoop;
    zpl_bool        mldqry_proxy;
    zpl_bool        arp_snoop;
    zpl_bool        rarp_snoop;
    zpl_bool        dhcp_snoop;
}nsm_snoop_proto_t;
#endif
typedef struct nsm_global_s
{
    void	        *mutex;
	vlan_t		    qinq_tpid;
	zpl_uint32		global_jumbo_size;
    zpl_bool        switch_forward;

    zpl_bool        unicast_flood;
    zpl_bool        multicast_flood;
    zpl_bool        multicast_learning;
    zpl_bool        bpdu_enable;
#ifdef ZPL_NSM_IGMP
    nsm_snoop_proto_t snoop_proto;
#endif
}nsm_global_t;



extern int nsm_global_jumbo_size_set(zpl_uint32 value);
extern int nsm_global_jumbo_size_get(zpl_uint32 *value);

extern int nsm_global_switch_forward_set(zpl_bool value);
extern int nsm_global_switch_forward_get(zpl_bool *value);

extern int nsm_global_multicast_flood_set(zpl_bool value);
extern int nsm_global_multicast_flood_get(zpl_bool *value);

extern int nsm_global_unicast_flood_set(zpl_bool value);
extern int nsm_global_unicast_flood_get(zpl_bool *value);

extern int nsm_global_multicast_learning_set(zpl_bool value);
extern int nsm_global_multicast_learning_get(zpl_bool *value);

//全局使能接收BPDU报文
extern int nsm_global_bpdu_set(zpl_bool value);
extern int nsm_global_bpdu_get(zpl_bool *value);

#ifdef ZPL_NSM_IGMP
extern int nsm_snooping_proto_set(enum nsm_snoop_proto_type, enum nsm_proto_action, zpl_bool value);
extern int nsm_snooping_proto_get(enum nsm_snoop_proto_type, enum nsm_proto_action, zpl_bool *value);
#endif

extern int nsm_global_init(void);
extern int nsm_global_exit(void);
extern int nsm_global_start(void);

extern void cmd_global_init(void);


#ifdef __cplusplus
}
#endif

#endif /* __NSM_GLOBAL_H__ */
