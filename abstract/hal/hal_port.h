/*
 * hal_port.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef ABSTRACT_HAL_HAL_PORT_H_
#define ABSTRACT_HAL_HAL_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif


enum hal_port_cmd 
{
    HAL_PORT_NONE,
	HAL_PORT,
	HAL_PORT_LINK,
	HAL_PORT_SPEED,
	HAL_PORT_DUPLEX,
	HAL_PORT_FLOW,
    HAL_PORT_JUMBO,
	HAL_PORT_LOOP,
	HAL_PORT_8021X,
	HAL_PORT_LEARNING,
	HAL_PORT_SWLEARNING,
	HAL_PORT_PROTECTED,
	HAL_PORT_WAN,
	HAL_PORT_MAC,
	HAL_PORT_MTU,
	HAL_PORT_METRIC,
	HAL_PORT_VRF,
	HAL_PORT_MODE,
	HAL_PORT_IPADDR,
};


extern int hal_port_up(ifindex_t ifindex);
extern int hal_port_down(ifindex_t ifindex);

extern int hal_port_speed_set(ifindex_t ifindex, zpl_uint32 value);
extern int hal_port_duplex_set(ifindex_t ifindex, zpl_uint32 value);

extern int hal_port_loop_set(ifindex_t ifindex, zpl_uint32 value);
extern int hal_port_8021x_set(ifindex_t ifindex, zpl_uint32 value);

extern int hal_port_jumbo_set(ifindex_t ifindex, zpl_bool enable);

extern int hal_port_enable_set(ifindex_t ifindex, zpl_bool enable);
extern zpl_bool hal_port_state_get(ifindex_t ifindex);
extern zpl_uint32 hal_port_speed_get(ifindex_t ifindex);
extern zpl_uint32 hal_port_duplex_get(ifindex_t ifindex);
extern int hal_port_flow_set(ifindex_t ifindex, zpl_uint32 value);
extern int hal_port_learning_set(ifindex_t ifindex, zpl_bool enable);

extern int hal_port_software_learning_set(ifindex_t ifindex, zpl_bool enable);

extern int hal_port_protected_set(ifindex_t ifindex, zpl_bool enable);
extern int hal_port_wan_set(ifindex_t ifindex, zpl_bool enable);

extern int hal_port_mac_set(ifindex_t ifindex, zpl_uint8 *cp, zpl_bool secondry);
extern int hal_port_mtu_set(ifindex_t ifindex, zpl_uint32 value);
extern int hal_port_vrf_set(ifindex_t ifindex, zpl_uint32 value);
extern int hal_port_mode_set(ifindex_t ifindex, zpl_uint32 value);

extern int hal_port_metric_set(ifindex_t ifindex, zpl_uint32 value);

#ifdef __cplusplus
}
#endif


#endif /* ABSTRACT_HAL_HAL_PORT_H_ */
