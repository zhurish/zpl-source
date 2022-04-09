
#ifndef __LINUX_VRF_H__
#define __LINUX_VRF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vrf.h"

extern int _ipkernel_vrf_enable(struct ip_vrf *vrf);
extern int _ipkernel_vrf_disable(struct ip_vrf *vrf);
extern zpl_socket_t _kernel_vrf_socket(int domain, zpl_uint32 type, zpl_uint16 protocol, vrf_id_t vrf_id);


#ifdef __cplusplus
}
#endif

#endif /* __LINUX_VRF_H__ */
