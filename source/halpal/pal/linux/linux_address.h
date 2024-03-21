
#ifndef __LINUX_NLADDRESS_H__
#define __LINUX_NLADDRESS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "if.h"

int linux_ioctl_if_set_prefix(struct interface *ifp, struct connected *ifc);
int linux_ioctl_if_unset_prefix(struct interface *ifp, struct connected *ifc);

int linux_ioctl_if_set_dst_prefix(struct interface *ifp, struct connected *ifc);
int linux_ioctl_if_unset_dst_prefix(struct interface *ifp, struct connected *ifc);

#ifdef ZPL_BUILD_IPV6
int linux_ioctl_if_prefix_add_ipv6(struct interface *ifp, struct connected *ifc, zpl_bool sec);
int linux_ioctl_if_prefix_delete_ipv6(struct interface *ifp, struct connected *ifc, zpl_bool sec);
#endif


#ifdef __cplusplus
}
#endif

#endif /* __LINUX_NLADDRESS_H__ */
