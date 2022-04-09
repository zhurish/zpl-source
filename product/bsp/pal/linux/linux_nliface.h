
#ifndef __LINUX_NLIFACE_H__
#define __LINUX_NLIFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int ip_ifp_stack_init(void);

extern int _netlink_create_interface(struct interface *ifp);
extern int _netlink_destroy_interface(struct interface *ifp);


#ifdef __cplusplus
}
#endif

#endif /* __LINUX_ARP_H__ */
