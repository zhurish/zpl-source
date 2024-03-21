
#ifndef __LINUX_ARP_H__
#define __LINUX_ARP_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "if.h"
#include "prefix.h"
extern int kernel_arp_del(struct interface *ifp, struct prefix *address);
extern int kernel_arp_set(struct interface *ifp, struct prefix *address, zpl_uint8 *mac);
extern int kernel_arp_get(struct interface *ifp, struct prefix *address, zpl_uint8 *mac);
extern int kernel_arp_gratuitousarp_enable(int enable);

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_ARP_H__ */
