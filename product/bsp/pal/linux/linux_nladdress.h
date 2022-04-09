
#ifndef __LINUX_NLADDRESS_H__
#define __LINUX_NLADDRESS_H__

#ifdef __cplusplus
extern "C" {
#endif


int _ipkernel_if_set_prefix(struct interface *ifp, struct connected *ifc);
int _ipkernel_if_unset_prefix(struct interface *ifp, struct connected *ifc);

int _ipkernel_if_set_dst_prefix(struct interface *ifp, struct connected *ifc);
int _ipkernel_if_unset_dst_prefix(struct interface *ifp, struct connected *ifc);

#ifdef ZPL_BUILD_IPV6
int _ipkernel_if_prefix_add_ipv6(struct interface *ifp, struct connected *ifc, int sec);
int _ipkernel_if_prefix_delete_ipv6(struct interface *ifp, struct connected *ifc, int sec);
#endif


#ifdef __cplusplus
}
#endif

#endif /* __LINUX_NLADDRESS_H__ */
