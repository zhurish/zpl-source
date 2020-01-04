/*
 * hal_port.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef ABSTRACT_HAL_HAL_PORT_H_
#define ABSTRACT_HAL_HAL_PORT_H_


typedef struct sdk_port_s
{
	int (*sdk_port_enable_cb)(void *, ifindex_t, BOOL);

	int (*sdk_port_link_cb)(void *, ifindex_t, BOOL);

	int (*sdk_port_speed_cb)(void *, ifindex_t, u_int);
	int (*sdk_port_duplex_cb)(void *, ifindex_t, u_int);
	int (*sdk_port_flow_cb)(void *, ifindex_t, u_int);
	int (*sdk_port_jumbo_cb)(void *, ifindex_t, BOOL);

	BOOL (*sdk_port_state_get_cb)(void *, ifindex_t);
	u_int (*sdk_port_speed_get_cb)(void *, ifindex_t);
	u_int (*sdk_port_duplex_get_cb)(void *, ifindex_t);


	int (*sdk_port_loop_cb)(void *, ifindex_t, u_int);
	int (*sdk_port_8021x_cb)(void *, ifindex_t, u_int);

	int (*sdk_port_learning_enable_cb)(void *, ifindex_t, BOOL);
	int (*sdk_port_swlearning_enable_cb)(void *, ifindex_t, BOOL);

	int (*sdk_port_protected_enable_cb)(void *, ifindex_t, BOOL);
	int (*sdk_port_wan_enable_cb)(void *, ifindex_t, BOOL);
/*
	int (*sdk_port_multicast_cb)(void *, ifindex_t, int);
	int (*sdk_port_bandwidth_cb)(void *, ifindex_t, int);
*/


	int (*sdk_port_mac_cb)(void *, ifindex_t, unsigned char *, BOOL);
	int (*sdk_port_mtu_cb)(void *, ifindex_t, u_int);
//	int (*sdk_port_metric_cb)(void *, ifindex_t, u_int);
	int (*sdk_port_vrf_cb)(void *, ifindex_t, u_int);

	int (*sdk_port_mode_cb)(void *, ifindex_t, u_int);

	void *sdk_driver;

}sdk_port_t;


extern int hal_port_up(ifindex_t ifindex);
extern int hal_port_down(ifindex_t ifindex);

extern int hal_port_speed_set(ifindex_t ifindex, u_int value);
extern int hal_port_duplex_set(ifindex_t ifindex, u_int value);

extern int hal_port_loop_set(ifindex_t ifindex, u_int value);
extern int hal_port_8021x_set(ifindex_t ifindex, u_int value);

extern int hal_port_jumbo_set(ifindex_t ifindex, BOOL enable);

extern int hal_port_enable_set(ifindex_t ifindex, BOOL enable);
extern BOOL hal_port_state_get(ifindex_t ifindex);
extern u_int hal_port_speed_get(ifindex_t ifindex);
extern u_int hal_port_duplex_get(ifindex_t ifindex);
extern int hal_port_flow_set(ifindex_t ifindex, u_int value);
extern int hal_port_learning_set(ifindex_t ifindex, BOOL enable);

extern int hal_port_software_learning_set(ifindex_t ifindex, BOOL enable);

extern int hal_port_protected_set(ifindex_t ifindex, BOOL enable);
extern int hal_port_wan_set(ifindex_t ifindex, BOOL enable);

extern int hal_port_mac_set(ifindex_t ifindex, unsigned char *cp, BOOL secondry);
extern int hal_port_mtu_set(ifindex_t ifindex, u_int value);
extern int hal_port_vrf_set(ifindex_t ifindex, u_int value);
extern int hal_port_mode_set(ifindex_t ifindex, u_int value);




#endif /* ABSTRACT_HAL_HAL_PORT_H_ */
