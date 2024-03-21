
#ifndef __LINUX_NLIFACE_H__
#define __LINUX_NLIFACE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "if.h"
extern int ip_ifp_stack_init(void);

extern int librtnl_create_interface(struct interface *ifp);
extern int librtnl_destroy_interface(struct interface *ifp);

int iplink_test(void);

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_ARP_H__ */
