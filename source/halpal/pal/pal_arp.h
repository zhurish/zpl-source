/*
 * pal_arp.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __PAL_ARP_H__
#define __PAL_ARP_H__

#ifdef __cplusplus
extern "C" {
#endif

//ip arp
#ifdef ZPL_NSM_ARP
extern int pal_interface_arp_add(struct interface *ifp, struct prefix *address, zpl_uint8 *mac);
extern int pal_interface_arp_delete(struct interface *ifp, struct prefix *address);
extern int pal_interface_arp_request(struct interface *ifp, struct prefix *address);
extern int pal_arp_gratuitousarp_enable(zpl_bool enable);
extern int pal_arp_ttl(zpl_uint32 ttl);
extern int pal_arp_age_timeout(zpl_uint32 timeout);
extern int pal_arp_retry_interval(zpl_uint32 interval);
extern int pal_arp_stack_init(void);
#endif


#ifdef __cplusplus
}
#endif


#endif /* __PAL_ARP_H__ */
