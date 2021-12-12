/*
 * nsm_security.h
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */

#ifndef __NSM_SECURITY_H_
#define __NSM_SECURITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "nsm_vlan.h"

typedef struct nsm_security_s
{
	ifindex_t	ifindex;
	//DOS Prevent
	zpl_bool	dos_enable;

	zpl_bool	ip_lan_drip;		//IPDA = IPSA
	zpl_bool	tcp_blat_drop;		//DPort = SPort in a TCP header
	zpl_bool	udp_blat_drop;		//DPort = SPort in a UDP header
	zpl_bool	tcp_null_scan_drop;		//Seq_Num = 0 and all TCP_FLAGs = 0 in a TCP header
	zpl_bool	tcp_xmas_scan_drop;		//Seq_Num = 0, FIN = 1, URG = 1, and PSH = 1 in a TCP header

	zpl_bool	tcp_synfin_scan_drop;
	zpl_bool	tcp_synerr_scan_drop;
	zpl_bool	tcp_zpl_int16_hdr_drop;
	zpl_bool	tcp_fragerr_scan_drop;

	zpl_bool	icmpv4_fragment_drop;
	zpl_bool	icmpv6_fragment_drop;

	zpl_bool	icmpv4_longping_drop;
	zpl_bool	icmpv6_longping_drop;

	zpl_uint32	tcp_hdr_min_size;
	zpl_uint32	icmpv4_max_size;
	zpl_uint32	icmpv6_max_size;
	zpl_bool	learn_disable;

	//Jumbo frame
	zpl_bool	jumbo_enable[PHY_PORT_MAX];
	zpl_uint32	jumbo_frame_size;


}nsm_security_t;



extern int nsm_security_init();
extern int nsm_security_exit();
extern int nsm_security_interface_create_api(struct interface *ifp);
extern int nsm_security_interface_del_api(struct interface *ifp);
extern void cmd_security_init();
#ifdef ZPL_SHELL_MODULE
extern int nsm_security_interface_write_config(struct vty *vty, struct interface *ifp);
#endif
 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_SECURITY_H_ */
