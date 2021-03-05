/*
 * nsm_tunnel.h
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */

#ifndef __NSM_TUNNEL_H__
#define __NSM_TUNNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NSM_TUNNEL_MTU_DEFAULT 1500
#define NSM_TUNNEL_TTL_DEFAULT 250
#define NSM_TUNNEL_TOS_DEFAULT 1



typedef enum
{
	NSM_TUNNEL_NONE = 0,
	NSM_TUNNEL_IPIP,
	NSM_TUNNEL_GRE,
	NSM_TUNNEL_VTI,
	NSM_TUNNEL_SIT, //IPv6-IN-IPv4
	NSM_TUNNEL_IPV4V6,
	NSM_TUNNEL_IPIPV6,
	NSM_TUNNEL_GREV6,
}tunnel_mode;


typedef struct nsm_tunnel_s
{
	struct interface *ifp;
	//int tun_index;				//tunnel接口的隧道ID编号
	tunnel_mode 	mode;			//隧道模式

	struct prefix source;		//ip tunnel change tunnel1 local 192.168.122.1
    struct prefix remote;		//change: ip tunnel change tunnel1 remote 19.1.1.1

	ospl_uchar tun_ttl;		//change: ip tunnel change tunnel0 ttl
	ospl_ushort tun_mtu;		//change: ip link set dev tunnel0 mtu 1400

	ospl_uchar tun_tos;
	ospl_uchar frag_off;

	ospl_bool	ready;
    ospl_bool 	active;				//隧道接口是否激活

}nsm_tunnel_t;
/*
    ospl_uint8 tos;
    ospl_uint16 tot_len;
    ospl_uint16 id;
    ospl_uint16 frag_off;
    ospl_uint8 ttl;
    ospl_uint8 protocol;

struct ip_tunnel_parm {
	ospl_char			name[IFNAMSIZ];
	int			link;
	__be16			i_flags;
	__be16			o_flags;
	__be32			i_key;
	__be32			o_key;
	struct iphdr		iph;
};
*/
extern nsm_tunnel_t * nsm_tunnel_get(struct interface *ifp);
extern int nsm_tunnel_client_init();
extern int nsm_tunnel_client_exit();

extern int nsm_tunnel_mode_set_api(struct interface *ifp, tunnel_mode mode);
extern int nsm_tunnel_mode_get_api(struct interface *ifp, tunnel_mode *mode);
extern int nsm_tunnel_source_set_api(struct interface *ifp, struct prefix  *source);
extern int nsm_tunnel_source_get_api(struct interface *ifp, struct prefix *source);
extern int nsm_tunnel_destination_set_api(struct interface *ifp, struct prefix *dest);
extern int nsm_tunnel_destination_get_api(struct interface *ifp, struct prefix *dest);
extern int nsm_tunnel_ttl_set_api(struct interface *ifp, ospl_uint32 ttl);
extern int nsm_tunnel_ttl_get_api(struct interface *ifp, ospl_uint32 *ttl);
extern int nsm_tunnel_mtu_set_api(struct interface *ifp, ospl_uint32 mtu);
extern int nsm_tunnel_mtu_get_api(struct interface *ifp, ospl_uint32 *mtu);
extern int nsm_tunnel_tos_set_api(struct interface *ifp, ospl_uint32 tos);
extern int nsm_tunnel_tos_get_api(struct interface *ifp, ospl_uint32 *tos);

extern int nsm_tunnel_make_iphdr(nsm_tunnel_t *tunnel, struct iphdr *iph);

extern void cmd_tunnel_init();

 
#ifdef __cplusplus
}
#endif

#endif /* __NSM_TUNNEL_H__ */
