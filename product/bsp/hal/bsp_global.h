/*
 * bsp_global.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_GLOBAL_H__
#define __BSP_GLOBAL_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct sdk_global_s
{
	int (*sdk_jumbo_size_cb) (void * , zpl_uint32 );
	int (*sdk_switch_manege_cb) (void *, zpl_bool);
	int (*sdk_switch_forward_cb) (void *, zpl_bool);
	int (*sdk_multicast_flood_cb) (void *, zpl_bool);
	int (*sdk_unicast_flood_cb) (void *, zpl_bool);
	int (*sdk_multicast_learning_cb) (void *, zpl_bool);
	int (*sdk_bpdu_enable_cb) (void *, zpl_bool);//全局使能接收BPDU报文
	int (*sdk_aging_time_cb) (void *, zpl_uint32);
	int (*sdk_wan_port_cb)(void *, zpl_phyport_t, zpl_bool);	
}sdk_global_t;

#ifdef ZPL_NSM_IGMP
typedef struct sdk_snooping_s
{
	int (*sdk_mldqry_snoop_cb) (void * , zpl_uint32, zpl_bool );
	int (*sdk_mld_snoop_cb) (void *, zpl_uint32, zpl_bool);
	int (*sdk_igmpunknow_snoop_cb) (void *, zpl_uint32, zpl_bool);
	int (*sdk_igmpqry_snoop_cb) (void *, zpl_uint32, zpl_bool);
	int (*sdk_igmp_snoop_cb) (void *, zpl_uint32, zpl_bool);
	int (*sdk_igmp_ipcheck_cb) (void *, zpl_bool);
	int (*sdk_arp_snoop_cb) (void *, zpl_bool);//全局使能接收BPDU报文
	int (*sdk_rarp_snoop_cb) (void *, zpl_bool);
	int (*sdk_dhcp_snoop_cb) (void *, zpl_bool);
}sdk_snooping_t;
#endif


extern sdk_global_t sdk_global;

#ifdef ZPL_NSM_IGMP
extern sdk_snooping_t sdk_snooping;
#endif

extern int bsp_global_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);
extern int bsp_snooping_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_MIRROR_H__ */
