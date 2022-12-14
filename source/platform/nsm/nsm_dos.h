/*
 * nsm_dos.h
 *
 *  Created on: May 8, 2018
 *      Author: zhurish
 */

#ifndef __NSM_DOS_H__
#define __NSM_DOS_H__

#ifdef __cplusplus
extern "C" {
#endif

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
	zpl_bool		enable;

	zpl_bool		icmpv6_longping;
	zpl_bool		icmpv4_longping;
	zpl_bool		icmpv6_fragment;
	zpl_bool		icmpv4_fragment;

	zpl_bool		tcp_fragerror;
	zpl_bool		tcp_shorthdr;
	zpl_bool		tcp_synerror;
	zpl_bool		tcp_synfinscan;
	zpl_bool		tcp_xmasscan;
	zpl_bool		tcp_nullscan;

	zpl_bool		udp_blat;
	zpl_bool		tcp_blat;
	zpl_bool		ip_lan_drip;

	zpl_uint32		tcp_hdr_min;
	zpl_uint32		icmpv4_max;
	zpl_uint32		icmpv6_max;

}Gl2dos_t;


int nsm_dos_init(void);
int nsm_dos_exit(void);

int nsm_dos_enable(void);
zpl_bool nsm_dos_is_enable(void);
int nsm_dos_enable_api(dos_type_en type);
int nsm_dos_disable_api(dos_type_en type);


int nsm_dos_tcp_hdr_min(zpl_uint32 size);
int nsm_dos_icmpv4_max(zpl_uint32 size);
int nsm_dos_icmpv6_max(zpl_uint32 size);

typedef int (*l2dos_cb)(Gl2dos_t *, void *);

int nsm_dos_callback_api(l2dos_cb, void *);

void cmd_dos_init (void);


 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_DOS_H__ */
