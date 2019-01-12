/*
 * hal_vlan.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef ABSTRACT_HAL_HAL_VLAN_H_
#define ABSTRACT_HAL_HAL_VLAN_H_

typedef struct sdk_vlan_s
{
    int    (*sdk_vlan_enable)(BOOL);
    int    (*sdk_vlan_create)(vlan_t);
    int    (*sdk_vlan_delete)(vlan_t);
    int    (*sdk_vlan_batch_create)(vlan_t, vlan_t);
    int    (*sdk_vlan_batch_delete)(vlan_t, vlan_t);

    int    (*sdk_vlan_add_untag_port)(ifindex_t, vlan_t);
    int    (*sdk_vlan_del_untag_port)(ifindex_t, vlan_t);
    int    (*sdk_vlan_add_tag_port)(ifindex_t, vlan_t);
    int    (*sdk_vlan_del_tag_port)(ifindex_t, vlan_t);

    int    (*sdk_port_set_native_vlan)(ifindex_t, vlan_t);
    int    (*sdk_port_unset_native_vlan)(ifindex_t, vlan_t);

    int    (*sdk_port_add_allowed_tag_vlan)(ifindex_t, vlan_t);
    int    (*sdk_port_del_allowed_tag_vlan)(ifindex_t, vlan_t);

    int    (*sdk_port_add_allowed_tag_batch_vlan)(ifindex_t, vlan_t, vlan_t);
    int    (*sdk_port_del_allowed_tag_batch_vlan)(ifindex_t, vlan_t, vlan_t);

    int    (*sdk_port_set_pvid_vlan)(ifindex_t, vlan_t);
    int    (*sdk_port_unset_pvid_vlan)(ifindex_t, vlan_t);


#if 0
    /* vlan_mode_set            */  drv_vlan_mode_set,
    /* vlan_mode_get            */  drv_vlan_mode_get,
    /* port_vlan_pvid_set       */  drv_port_vlan_pvid_set,
    /* port_vlan_pvid_get       */  drv_port_vlan_pvid_get,
    /* port_vlan_set            */  drv_port_vlan_set,
    /* port_vlan_get            */  drv_port_vlan_get,
    /* vlan_property_set        */  drv_bcm53115_vlan_prop_set,
    /* vlan_property_get        */  drv_bcm53115_vlan_prop_get,
    /* vlan_prop_port_enable_set */ drv_bcm53115_vlan_prop_port_enable_set,
    /* vlan_prop_port_enable_get */ drv_bcm53115_vlan_prop_port_enable_get,
    /* vlan_vt_set              */  drv_bcm53115_vlan_vt_set,
    /* vlan_vt_get              */  drv_bcm53115_vlan_vt_get,
    /* vlan_vt_add              */  drv_bcm53115_vlan_vt_add,
    /* vlan_vt_delete           */  drv_bcm53115_vlan_vt_delete,
    /* vlan_vt_delete_all       */  drv_bcm53115_vlan_vt_delete_all,
#endif

}sdk_vlan_t;


extern sdk_vlan_t sdk_vlan;

extern int hal_vlan_enable(BOOL enable);
extern int hal_vlan_create(vlan_t vlan);
extern int hal_vlan_destroy(vlan_t vlan);

extern int hal_vlan_add_untag_port(ifindex_t ifindex, vlan_t vlan);
extern int hal_vlan_del_untag_port(ifindex_t ifindex, vlan_t vlan);

extern int hal_vlan_add_tag_port(ifindex_t ifindex, vlan_t vlan);
extern int hal_vlan_del_tag_port(ifindex_t ifindex, vlan_t vlan);

extern int hal_port_add_native_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_del_native_vlan(ifindex_t ifindex, vlan_t vlan);

extern int hal_port_add_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_del_allowed_tag_vlan(ifindex_t ifindex, vlan_t vlan);

extern int hal_port_add_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end);
extern int hal_port_del_allowed_tag_batch_vlan(ifindex_t ifindex, vlan_t start, vlan_t end);

extern int hal_port_set_vlan(ifindex_t ifindex, vlan_t vlan);
extern int hal_port_unset_vlan(ifindex_t ifindex, vlan_t vlan);

#endif /* ABSTRACT_HAL_HAL_VLAN_H_ */
