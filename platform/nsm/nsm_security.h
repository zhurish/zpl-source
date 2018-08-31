/*
 * nsm_security.h
 *
 *  Created on: Apr 19, 2018
 *      Author: zhurish
 */

#ifndef __NSM_SECURITY_H_
#define __NSM_SECURITY_H_

#include "nsm_vlan.h"

typedef struct nsm_security_s
{
	ifindex_t	ifindex;
	//DOS Prevent
	BOOL	dos_enable;

	BOOL	ip_lan_drip;		//IPDA = IPSA
	BOOL	tcp_blat_drop;		//DPort = SPort in a TCP header
	BOOL	udp_blat_drop;		//DPort = SPort in a UDP header
	BOOL	tcp_null_scan_drop;		//Seq_Num = 0 and all TCP_FLAGs = 0 in a TCP header
	BOOL	tcp_xmas_scan_drop;		//Seq_Num = 0, FIN = 1, URG = 1, and PSH = 1 in a TCP header

	BOOL	tcp_synfin_scan_drop;
	BOOL	tcp_synerr_scan_drop;
	BOOL	tcp_short_hdr_drop;
	BOOL	tcp_fragerr_scan_drop;

	BOOL	icmpv4_fragment_drop;
	BOOL	icmpv6_fragment_drop;

	BOOL	icmpv4_longping_drop;
	BOOL	icmpv6_longping_drop;

	u_int	tcp_hdr_min_size;
	u_int	icmpv4_max_size;
	u_int	icmpv6_max_size;
	BOOL	learn_disable;

	//Jumbo frame
	BOOL	jumbo_enable[PHY_PORT_MAX];
	u_int	jumbo_frame_size;


}nsm_security_t;










#endif /* __NSM_SECURITY_H_ */
