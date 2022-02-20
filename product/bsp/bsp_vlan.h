/*
 * bsp_vlan.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __BSP_VLAN_H__
#define __BSP_VLAN_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sdk_vlan_s
{
    int    (*sdk_vlan_enable)(void *, zpl_bool);
    int    (*sdk_vlan_create)(void *, zpl_bool, vlan_t);
    int    (*sdk_vlan_batch_create)(void *, zpl_bool, vlan_t, vlan_t);

    int    (*sdk_vlan_untag_port)(void *, zpl_bool, zpl_phyport_t, vlan_t);
    int    (*sdk_vlan_tag_port)(void *, zpl_bool, zpl_phyport_t, vlan_t);
    int    (*sdk_port_native_vlan)(void *, zpl_bool, zpl_phyport_t, vlan_t);

    int    (*sdk_port_allowed_tag_vlan)(void *, zpl_bool, zpl_phyport_t, vlan_t);
    int    (*sdk_port_allowed_tag_batch_vlan)(void *, zpl_bool, zpl_phyport_t, vlan_t, vlan_t);

    int    (*sdk_port_pvid_vlan)(void *, zpl_bool, zpl_phyport_t, vlan_t);

    int    (*sdk_vlan_stp_state)(void *, vlan_t, zpl_phyport_t, int stp_state);

    int    (*sdk_vlan_mstp_instance)(void *, zpl_bool, vlan_t, zpl_index_t mstpid);
    int    (*sdk_vlan_translate)(void *, zpl_bool, zpl_phyport_t, vlan_t, vlan_t, int);

    int    (*sdk_port_qinq_vlan)(void *, zpl_bool, zpl_phyport_t);
    int    (*sdk_port_qinq_tpid)(void *, vlan_t);

    int    (*sdk_vlan_port)(void *, vlan_t, zpl_phyport_t);

}sdk_vlan_t;

extern sdk_vlan_t sdk_vlan;
extern int bsp_vlan_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_VLAN_H__ */
