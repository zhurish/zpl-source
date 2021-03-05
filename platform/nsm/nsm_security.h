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
	ospl_bool	dos_enable;

	ospl_bool	ip_lan_drip;		//IPDA = IPSA
	ospl_bool	tcp_blat_drop;		//DPort = SPort in a TCP header
	ospl_bool	udp_blat_drop;		//DPort = SPort in a UDP header
	ospl_bool	tcp_null_scan_drop;		//Seq_Num = 0 and all TCP_FLAGs = 0 in a TCP header
	ospl_bool	tcp_xmas_scan_drop;		//Seq_Num = 0, FIN = 1, URG = 1, and PSH = 1 in a TCP header

	ospl_bool	tcp_synfin_scan_drop;
	ospl_bool	tcp_synerr_scan_drop;
	ospl_bool	tcp_ospl_int16_hdr_drop;
	ospl_bool	tcp_fragerr_scan_drop;

	ospl_bool	icmpv4_fragment_drop;
	ospl_bool	icmpv6_fragment_drop;

	ospl_bool	icmpv4_longping_drop;
	ospl_bool	icmpv6_longping_drop;

	ospl_uint32	tcp_hdr_min_size;
	ospl_uint32	icmpv4_max_size;
	ospl_uint32	icmpv6_max_size;
	ospl_bool	learn_disable;

	//Jumbo frame
	ospl_bool	jumbo_enable[PHY_PORT_MAX];
	ospl_uint32	jumbo_frame_size;


}nsm_security_t;



extern int nsm_security_init();
extern int nsm_security_exit();

extern void cmd_security_init();


 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_SECURITY_H_ */
