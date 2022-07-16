
#ifndef __LINUX_BRIGDE_H__
#define __LINUX_BRIGDE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_NSM_BRIDGE
extern int linux_ioctl_bridge_create(nsm_bridge_t *br);
extern int linux_ioctl_bridge_delete(nsm_bridge_t *br);
extern int linux_ioctl_bridge_add_interface(nsm_bridge_t *br, ifindex_t ifindex);
extern int linux_ioctl_bridge_del_interface(nsm_bridge_t *br, ifindex_t ifindex);
extern int linux_ioctl_bridge_list_interface(nsm_bridge_t *br, ifindex_t ifindex[]);
extern int linux_ioctl_bridge_check_interface(char *br, ifindex_t ifindex);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_BRIGDE_H__ */
