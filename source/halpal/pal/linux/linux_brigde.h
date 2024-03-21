
#ifndef __LINUX_BRIGDE_H__
#define __LINUX_BRIGDE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "if.h"
#include "nsm_bridge.h"

#ifdef ZPL_NSM_BRIDGE
extern int linux_ioctl_bridge_create(struct interface *ifp);
extern int linux_ioctl_bridge_delete(struct interface *ifp);
extern int linux_ioctl_bridge_add_interface(struct interface *ifp, struct interface *sifp);
extern int linux_ioctl_bridge_del_interface(struct interface *ifp, struct interface *sifp);
extern int linux_ioctl_bridge_list_interface(nsm_bridge_t *br, ifindex_t ifindex[]);
extern int linux_ioctl_bridge_check_interface(char *br, ifindex_t ifindex);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_BRIGDE_H__ */
