
#ifndef __LINUX_NLROUTE_H__
#define __LINUX_NLROUTE_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int _netlink_route_rib (struct prefix *p, struct rib *old, struct rib *new);



#ifdef __cplusplus
}
#endif

#endif /* __LINUX_NLROUTE_H__ */
