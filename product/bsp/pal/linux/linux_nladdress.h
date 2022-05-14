
#ifndef __LINUX_NLADDRESS_H__
#define __LINUX_NLADDRESS_H__

#ifdef __cplusplus
extern "C" {
#endif


int _ipkernel_if_set_prefix(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec);
int _ipkernel_if_unset_prefix(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec);

int _ipkernel_if_set_dst_prefix(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec);
int _ipkernel_if_unset_dst_prefix(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec);

#ifdef ZPL_BUILD_IPV6
int _ipkernel_if_prefix_add_ipv6(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec);
int _ipkernel_if_prefix_delete_ipv6(void *d, ifindex_t kifindex, zpl_family_t family, zpl_uint8 prelen, union g_addr ifc, zpl_bool sec);
#endif


#ifdef __cplusplus
}
#endif

#endif /* __LINUX_NLADDRESS_H__ */
