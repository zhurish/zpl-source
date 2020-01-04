/*
 * hal_mstp.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_MSTP_H__
#define __HAL_MSTP_H__


typedef enum hal_port_stp_state_e {
    HAL_PORT_STP_DISABLE = 1,
    HAL_PORT_STP_BLOCK,
    HAL_PORT_STP_LISTEN,
    HAL_PORT_STP_LEARN,
    HAL_PORT_STP_FORWARD
} hal_port_stp_state_t;


typedef struct sdk_mstp_s
{
	int (*sdk_mstp_enable_cb) (void *, BOOL);
	int (*sdk_mstp_age_cb) (void *, int);
	int (*sdk_mstp_bypass_cb) (void *, BOOL, int);
	int (*sdk_mstp_state_cb) (void *, ifindex_t, int, hal_port_stp_state_t);
	int (*sdk_mstp_vlan_cb) (void *, vlan_t,  int);
	int (*sdk_stp_state_cb) (void *, ifindex_t,  hal_port_stp_state_t);

	void *sdk_driver;
}sdk_mstp_t;


int hal_mstp_enable(BOOL enable);
int hal_mstp_age(int age);
int hal_mstp_bypass(BOOL enable, int type);
int hal_mstp_state(ifindex_t ifindex, int mstp, hal_port_stp_state_t state);
int hal_stp_state(ifindex_t ifindex, hal_port_stp_state_t state);

#endif /* __HAL_MSTP_H__ */
