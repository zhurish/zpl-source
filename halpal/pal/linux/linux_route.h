
#ifndef __LINUX_NLROUTE_H__
#define __LINUX_NLROUTE_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int librtnl_route_rib_add (zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num);
extern int librtnl_route_rib_del (zpl_uint8 processid, safi_t safi, struct prefix *p,
						struct rib *rib, zpl_uint8 num);

extern int librtnl_route_rib_action (struct prefix *p, struct rib *old, struct rib *new);

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_NLROUTE_H__ */
