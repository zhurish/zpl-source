/*
 * nsm_dos.h
 *
 *  Created on: May 8, 2018
 *      Author: zhurish
 */

#ifndef __NSM_DOS_H__
#define __NSM_DOS_H__

typedef enum dos_type_e
{
	DOS_IP_LAN_DRIP = 1,
	TCP_BLAT_DROP,
	UDP_BLAT_DROP,
	TCP_NULLSCAN_DROP,
	TCP_XMASSCAN_DROP,
	TCP_SYNFINSCAN_DROP,
	TCP_SYNERROR_DROP,

	TCP_SHORTHDR_DROP,
	TCP_FRAGERROR_DROP,
	ICMPv4_FRAGMENT_DROP,
	ICMPv6_FRAGMENT_DROP,

	ICMPv4_LONGPING_DROP,
	ICMPv6_LONGPING_DROP,

}dos_type_en;



typedef struct Gl2dos_s
{
	void		*mutex;
	BOOL		enable;

	BOOL		icmpv6_longping;
	BOOL		icmpv4_longping;
	BOOL		icmpv6_fragment;
	BOOL		icmpv4_fragment;

	BOOL		tcp_fragerror;
	BOOL		tcp_shorthdr;
	BOOL		tcp_synerror;
	BOOL		tcp_synfinscan;
	BOOL		tcp_xmasscan;
	BOOL		tcp_nullscan;

	BOOL		udp_blat;
	BOOL		tcp_blat;
	BOOL		ip_lan_drip;

	u_int		tcp_hdr_min;
	u_int		icmpv4_max;
	u_int		icmpv6_max;

}Gl2dos_t;


int nsm_dos_init();
int nsm_dos_exit();

int nsm_dos_enable(void);
BOOL nsm_dos_is_enable(void);
int nsm_dos_enable_api(dos_type_en type);
int nsm_dos_disable_api(dos_type_en type);


int nsm_dos_tcp_hdr_min(u_int size);
int nsm_dos_icmpv4_max(u_int size);
int nsm_dos_icmpv6_max(u_int size);

typedef int (*l2dos_cb)(Gl2dos_t *, void *);

int nsm_dos_callback_api(l2dos_cb, void *);


#endif /* __NSM_DOS_H__ */
