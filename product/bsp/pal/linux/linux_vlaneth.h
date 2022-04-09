
#ifndef __LINUX_VLANETH_H__
#define __LINUX_VLANETH_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ZPL_NSM_VLANETH
extern int _ipkernel_vlaneth_create (nsm_vlaneth_t *kifp);
extern int _ipkernel_vlaneth_destroy (nsm_vlaneth_t *kifp);
extern int _ipkernel_vlaneth_change (nsm_vlaneth_t *kifp, vlan_t vlan);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LINUX_VLANETH_H__ */
