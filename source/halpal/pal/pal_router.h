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

#include "prefix.h"
#include "if.h"
#include "nsm_rib.h"
//route
extern int pal_create_vrf(struct ip_vrf *vrf);
extern int pal_delete_vrf(struct ip_vrf *vrf);
extern int pal_route_rib_add(zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num);
extern int pal_route_rib_del(zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num);


#ifdef __cplusplus
}
#endif


#endif /* __PAL_ROUTER_H__ */
