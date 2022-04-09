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
    HAL_PORT_PAUSE,
    HAL_PORT_JUMBO,
	HAL_PORT_LOOP,
	HAL_PORT_LEARNING,
	HAL_PORT_SWLEARNING,
	HAL_PORT_PROTECTED,
    HAL_PORT_VRF,
    HAL_PORT_MODE,
	HAL_PORT_MAC,
	HAL_PORT_IPADDR,
};

typedef struct hal_port_param_s
{
	zpl_uint32 value;
	mac_t mac[NSM_MAC_MAX];
}hal_port_param_t;


extern int hal_port_up(ifindex_t ifindex);
extern int hal_port_down(ifindex_t ifindex);

extern int hal_port_speed_set(ifindex_t ifindex, nsm_speed_en value);
extern int hal_port_duplex_set(ifindex_t ifindex, nsm_duplex_en value);

extern int hal_port_loop_set(ifindex_t ifindex, zpl_bool value);

extern int hal_port_jumbo_set(ifindex_t ifindex, zpl_bool enable);

extern int hal_port_enable_set(ifindex_t ifindex, zpl_bool enable);


extern zpl_bool hal_port_state_get(ifindex_t ifindex);
extern nsm_speed_en hal_port_speed_get(ifindex_t ifindex);
extern nsm_duplex_en hal_port_duplex_get(ifindex_t ifindex);

extern int hal_port_flow_set(ifindex_t ifindex, zpl_bool tx, zpl_bool rx);
extern int hal_port_learning_set(ifindex_t ifindex, zpl_bool enable);

extern int hal_port_software_learning_set(ifindex_t ifindex, zpl_bool enable);

extern int hal_port_protected_set(ifindex_t ifindex, zpl_bool enable);

extern int hal_port_mac_set(ifindex_t ifindex, zpl_uint8 *cp, zpl_bool secondry);
extern int hal_port_mtu_set(ifindex_t ifindex, zpl_uint32 value);
extern int hal_port_vrf_set(ifindex_t ifindex, vrf_id_t value);
extern int hal_port_mode_set(ifindex_t ifindex, if_mode_t value);

/* 
 * Enable or disable transmission of pause frames and honoring received
 * pause frames on a port.
 */
extern int hal_port_pause_set(
    ifindex_t ifindex, 
    zpl_bool pause_tx, 
    zpl_bool pause_rx);


#if 0
/* Enable or disable BPDU processing on a port. */
extern int hal_port_bpdu_enable_set(
    ifindex_t ifindex, 
    zpl_bool enable);

/* Configure a port for egress rate shaping. */
extern int hal_port_rate_egress_set(
    ifindex_t ifindex, 
    zpl_uint32 kbits_sec, 
    zpl_uint32 kbits_burst);
/* Configure a port fifos for egress rate shaping. */
extern int hal_port_rate_egress_traffic_set(
    ifindex_t ifindex, 
    zpl_uint32 traffic_types, 
    zpl_uint32 kbits_sec, 
    zpl_uint32 kbits_burst);
/* Configure a port for ingress rate policing. */
extern int hal_port_rate_ingress_set(
    ifindex_t ifindex, 
    zpl_uint32 kbits_sec, 
    zpl_uint32 kbits_burst);



/* Set the default tag protocol ID (TPID) for the specified port. */
extern int hal_port_tpid_set(
    ifindex_t ifindex, 
    vlan_t tpid);
/* Get or set various features at the port level. */
extern int hal_port_control_set(
    ifindex_t ifindex, 
    zpl_uint32 type, 
    int value);


/* Set the Policer ID accociated for the specified physical port. */
extern int hal_port_policer_set(
    ifindex_t ifindex, 
    zpl_uint32 policer_id);

#endif

#ifdef __cplusplus
}
#endif


#endif /* ABSTRACT_HAL_HAL_PORT_H_ */