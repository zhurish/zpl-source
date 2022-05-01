/*
 * pal_router.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __PAL_ROUTER_H__
#define __PAL_ROUTER_H__

#ifdef __cplusplus
extern "C" {
#endif

//route
extern int apal_create_vrf(struct ip_vrf *vrf);
extern int apal_delete_vrf(struct ip_vrf *vrf);
extern int apal_iproute_rib_action(struct prefix *p, struct rib *old, struct rib *new);


#ifdef __cplusplus
}
#endif


#endif /* __PAL_ROUTER_H__ */
