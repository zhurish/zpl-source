
#ifndef __LINUX_ETH_H__
#define __LINUX_ETH_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_NSM_VLANETH
extern int linux_ioctl_eth_create (struct interface *ifp);
extern int linux_ioctl_eth_destroy (struct interface *ifp);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_VLANETH_H__ */
