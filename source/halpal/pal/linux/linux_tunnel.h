
#ifndef __LINUX_TUNNEL_H__
#define __LINUX_TUNNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_NSM_TUNNEL
extern int linux_ioctl_tunnel_create(struct interface *ifp);
extern int linux_ioctl_tunnel_delete(struct interface *ifp);
extern int linux_ioctl_tunnel_change(struct interface *ifp);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_TUNNEL_H__ */
