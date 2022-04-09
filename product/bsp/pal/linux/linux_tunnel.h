
#ifndef __LINUX_TUNNEL_H__
#define __LINUX_TUNNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_NSM_TUNNEL
extern int _ipkernel_tunnel_create(nsm_tunnel_t *tunnel);
extern int _ipkernel_tunnel_delete(nsm_tunnel_t *tunnel);
extern int _ipkernel_tunnel_change(nsm_tunnel_t *tunnel);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_TUNNEL_H__ */
