/*
 * kernel_driver.h
 *
 *  Created on: 2019年9月21日
 *      Author: zhurish
 */

#ifndef __KERNEL_DRIVER_H__
#define __KERNEL_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef ZPL_KERNEL_SORF_FORWARDING
#ifdef ZPL_NSM_VLANETH
extern int _ipkernel_linux_create (nsm_vlaneth_t *kifp);
extern int _ipkernel_linux_destroy (nsm_vlaneth_t *kifp);
extern int _ipkernel_linux_change (nsm_vlaneth_t *kifp, vlan_t vlan);
#endif

#ifdef ZPL_NSM_TUNNEL
extern int _ipkernel_tunnel_create(nsm_tunnel_t *tunnel);
extern int _ipkernel_tunnel_delete(nsm_tunnel_t *tunnel);
extern int _ipkernel_tunnel_change(nsm_tunnel_t *tunnel);
#endif
#ifdef ZPL_NSM_BRIDGE
extern int _ipkernel_bridge_create(nsm_bridge_t *br);
extern int _ipkernel_bridge_delete(nsm_bridge_t *br);
extern int _ipkernel_bridge_add_interface(nsm_bridge_t *br, ifindex_t ifindex);
extern int _ipkernel_bridge_del_interface(nsm_bridge_t *br, ifindex_t ifindex);
extern int _ipkernel_bridge_list_interface(nsm_bridge_t *br, ifindex_t ifindex[]);
extern int _ipkernel_bridge_check_interface(char *br, ifindex_t ifindex);
#endif
#ifdef ZPL_NSM_TRUNK
extern int _ipkernel_bond_create(struct interface *ifp);
extern int _ipkernel_bond_delete(struct interface *ifp);
int _if_bond_test();
#endif

int _ipkernel_create(struct interface *ifp);
int _ipkernel_destroy(struct interface *ifp);
int _ipkernel_change(struct interface *ifp);
int _ipkernel_set_vlan(struct interface *ifp, vlan_t vlan);

#endif

int _ipkernel_ipforward (void);
int _ipkernel_ipforward_on (void);
int _ipkernel_ipforward_off (void); 
int _ipkernel_ipforward_ipv6 (void);
int _ipkernel_ipforward_ipv6_on (void);
int _ipkernel_ipforward_ipv6_off (void);


/*
 * 端口映射
 */
extern int _ipkernel_firewall_portmap_rule_set(firewall_t *rule, zpl_action action);
/*
 * 端口开放
 */
extern int _ipkernel_firewall_port_filter_rule_set(firewall_t *rule, zpl_action action);
extern int _ipkernel_firewall_mangle_rule_set(firewall_t *rule, zpl_action action);
extern int _ipkernel_firewall_raw_rule_set(firewall_t *rule, zpl_action action);
extern int _ipkernel_firewall_snat_rule_set(firewall_t *rule, zpl_action action);
extern int _ipkernel_firewall_dnat_rule_set(firewall_t *rule, zpl_action action);

extern int _ipkernel_vrf_enable(vrf_id_t vrf_id);
extern int _ipkernel_vrf_disable(vrf_id_t vrf_id);
extern zpl_socket_t _kernel_vrf_socket(int domain, zpl_uint32 type, zpl_uint16 protocol, vrf_id_t vrf_id);

extern int ip_ifp_stack_init(void);
extern int ip_arp_stack_init(void);
extern int kernel_driver_init(void);


#ifdef __cplusplus
}
#endif


#endif /* __KERNEL_DRIVER_H__ */
