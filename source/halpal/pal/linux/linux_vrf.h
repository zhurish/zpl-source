
#ifndef __LINUX_VRF_H__
#define __LINUX_VRF_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_LIBNL_MODULE
int rtnl_ipvrf_create(int id, char *name);
int rtnl_ipvrf_add_dev(char *dev, char *name);
#endif
extern int linux_ioctl_vrf_enable(struct ip_vrf *vrf);
extern int linux_ioctl_vrf_disable(struct ip_vrf *vrf);
extern int linux_ioctl_vrf_set(struct interface *ifp, struct ip_vrf *vrf);
extern zpl_socket_t linux_vrf_socket(int domain, zpl_uint32 type, zpl_uint16 protocol, vrf_id_t vrf_id);


#ifdef __cplusplus
}
#endif

#endif /* __LINUX_VRF_H__ */
