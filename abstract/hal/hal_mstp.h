/*
 * hal_mstp.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_MSTP_H__
#define __HAL_MSTP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum hal_port_stp_state_e {
    HAL_PORT_STP_DISABLE = 1,
    HAL_PORT_STP_BLOCK,
    HAL_PORT_STP_LISTEN,
    HAL_PORT_STP_LEARN,
    HAL_PORT_STP_FORWARD
} hal_port_stp_state_t;


typedef struct sdk_mstp_s
{
	int (*sdk_mstp_enable_cb) (void *, ospl_bool);
	int (*sdk_mstp_age_cb) (void *, ospl_uint32);
	int (*sdk_mstp_bypass_cb) (void *, ospl_bool, ospl_uint32);
	int (*sdk_mstp_state_cb) (void *, ifindex_t, ospl_uint32, hal_port_stp_state_t);
	int (*sdk_mstp_vlan_cb) (void *, vlan_t,  ospl_uint32);
	int (*sdk_stp_state_cb) (void *, ifindex_t,  hal_port_stp_state_t);

	void *sdk_driver;
}sdk_mstp_t;


int hal_mstp_enable(ospl_bool enable);
int hal_mstp_age(ospl_uint32 age);
int hal_mstp_bypass(ospl_bool enable, ospl_uint32 type);
int hal_mstp_state(ifindex_t ifindex, ospl_uint32 mstp, hal_port_stp_state_t state);
int hal_stp_state(ifindex_t ifindex, hal_port_stp_state_t state);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_MSTP_H__ */
