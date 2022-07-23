
#ifndef __LINUX_FIREWALLD_H__
#define __LINUX_FIREWALLD_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_NSM_FIREWALLD
/*
 * 端口映射
 */
extern int linux_ioctl_firewall_portmap_rule_set(firewall_t *rule, zpl_action action);
/*
 * 端口开放
 */
extern int linux_ioctl_firewall_port_filter_rule_set(firewall_t *rule, zpl_action action);
extern int linux_ioctl_firewall_mangle_rule_set(firewall_t *rule, zpl_action action);
extern int linux_ioctl_firewall_raw_rule_set(firewall_t *rule, zpl_action action);
extern int linux_ioctl_firewall_snat_rule_set(firewall_t *rule, zpl_action action);
extern int linux_ioctl_firewall_dnat_rule_set(firewall_t *rule, zpl_action action);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_FIREWALLD_H__ */
