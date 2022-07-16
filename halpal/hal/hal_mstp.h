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


typedef struct hal_mstp_param_s
{
    zpl_bool enable;
	zpl_uint32 value;
	zpl_uint32 type;
	hal_port_stp_state_t state;
}hal_mstp_param_t;

int hal_mstp_enable(zpl_bool enable);
int hal_mstp_create(zpl_index_t id);
int hal_mstp_add_vlan(zpl_index_t id, vlan_t vid);
int hal_mstp_del_vlan(zpl_index_t id, vlan_t vid);
int hal_mstp_state(zpl_index_t id, ifindex_t ifindex, hal_port_stp_state_t state);
int hal_stp_state(zpl_index_t id, ifindex_t ifindex, hal_port_stp_state_t state);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_MSTP_H__ */
