
#ifndef __LINUX_NLROUTE_H__
#define __LINUX_NLROUTE_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "hal_include.h"
extern int linux_netlink_route_rib_add (void *, hal_route_param_t *param);
extern int linux_netlink_route_rib_del (void *, hal_route_param_t *param);


#ifdef __cplusplus
}
#endif

#endif /* __LINUX_NLROUTE_H__ */
