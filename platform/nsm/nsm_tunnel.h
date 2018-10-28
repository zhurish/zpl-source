/*
 * nsm_tunnel.h
 *
 *  Created on: Sep 13, 2018
 *      Author: zhurish
 */

#ifndef __NSM_TUNNEL_H__
#define __NSM_TUNNEL_H__

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

	unsigned char tun_ttl;		//change: ip tunnel change tunnel0 ttl
	unsigned short tun_mtu;		//change: ip link set dev tunnel0 mtu 1400

	unsigned char tun_tos;
	unsigned char frag_off;

	BOOL	ready;
    BOOL 	active;				//隧道接口是否激活

}nsm_tunnel_t;
/*
    u_int8_t tos;
    u_int16_t tot_len;
    u_int16_t id;
    u_int16_t frag_off;
    u_int8_t ttl;
    u_int8_t protocol;

struct ip_tunnel_parm {
	char			name[IFNAMSIZ];
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
extern int nsm_tunnel_ttl_set_api(struct interface *ifp, int ttl);
extern int nsm_tunnel_ttl_get_api(struct interface *ifp, int *ttl);
extern int nsm_tunnel_mtu_set_api(struct interface *ifp, int mtu);
extern int nsm_tunnel_mtu_get_api(struct interface *ifp, int *mtu);
extern int nsm_tunnel_tos_set_api(struct interface *ifp, int tos);
extern int nsm_tunnel_tos_get_api(struct interface *ifp, int *tos);

extern int nsm_tunnel_make_iphdr(nsm_tunnel_t *tunnel, struct iphdr *iph);

extern void cmd_tunnel_init();


#endif /* __NSM_TUNNEL_H__ */
