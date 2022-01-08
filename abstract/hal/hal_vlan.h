/*
 * hal_vlan.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef ABSTRACT_HAL_HAL_VLAN_H_
#define ABSTRACT_HAL_HAL_VLAN_H_

#ifdef __cplusplus
extern "C" {
#endif

enum hal_vlan_cmd 
{
    HAL_VLAN_NONE,
	HAL_VLAN,
	HAL_VLAN_CREATE,
	HAL_VLAN_DELETE,
	HAL_VLAN_RANGE_CREATE,
    HAL_VLAN_RANGE_DELETE,
    //PORT
    HAL_VLAN_UNTAG,
    HAL_VLAN_TAG,
    HAL_VLAN_NATIVE,
    HAL_VLAN_ALLOWE,
    HAL_VLAN_RANGE_ALLOWE,
    HAL_VLAN_PVID,
    HAL_VLAN_MAX,
};

typedef struct hal_vlan_param_s
{
    zpl_bool set;
	zpl_bool enable;
	vlan_t vlan;
	vlan_t vlan_end;
}hal_vlan_param_t;

extern int hal_vlan_enable(zpl_bool enable);
extern int hal_vlan_create(vlan_t vlan);
extern int hal_vlan_destroy(vlan_t vlan);

extern int hal_vlan_batch_create(vlan_t *vlan, int num);
extern int hal_vlan_batch_destroy(vlan_t *vlan, int num);

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

#ifdef __cplusplus
}
#endif

#endif /* ABSTRACT_HAL_HAL_VLAN_H_ */
