
#ifndef __LINUX_VLAN_H__
#define __LINUX_VLAN_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "if.h"
#include "nsm_vlan.h"
#ifdef ZPL_NSM_VLANETH
extern int linux_ioctl_vlan_create (struct interface *ifp);
extern int linux_ioctl_vlan_destroy (struct interface *ifp);
extern int linux_ioctl_vlan_change (struct interface *ifp, vlan_t vlan);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_VLANETH_H__ */
