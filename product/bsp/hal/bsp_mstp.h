/*
 * bsp_mstp.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __BSP_MSTP_H__
#define __BSP_MSTP_H__

#ifdef __cplusplus
extern "C" {
#endif
#ifndef ZPL_SDK_USER
typedef enum stp_state_s
{
	STP_DISABLE = 0,
	STP_LISTENING,
	STP_LEARNING,
	STP_FORWARDING,
	STP_BLOCKING,
}stp_state_t;

typedef enum hal_port_stp_state_e {
    HAL_PORT_STP_DISABLE = 1,
    HAL_PORT_STP_BLOCK,
    HAL_PORT_STP_LISTEN,
    HAL_PORT_STP_LEARN,
    HAL_PORT_STP_FORWARD
} hal_port_stp_state_t;

enum hal_mstp_cmd 
{
    HAL_MSTP_NONE,
	HAL_MSTP_ENABLE,
    HAL_MSTP_CREATE,
    HAL_MSTP_ADD_VLAN,
    HAL_MSTP_DEL_VLAN,
	HAL_MSTP_STATE,
    HAL_STP_STATE,
};

typedef struct hal_mstp_param_s
{
    zpl_bool enable;
	zpl_uint32 value;
	zpl_uint32 type;
	hal_port_stp_state_t state;
}hal_mstp_param_t;
#endif
typedef struct sdk_mstp_s
{
	int (*sdk_mstp_enable_cb) (void *, zpl_bool);
	int (*sdk_mstp_create) (void *, zpl_index_t);
	int (*sdk_mstp_add_vlan) (void *, zpl_index_t , vlan_t );
	int (*sdk_mstp_del_vlan) (void *, zpl_index_t , vlan_t );
	int (*sdk_mstp_aging_cb) (void *, zpl_index_t );
	int (*sdk_mstp_state) (void *, zpl_index_t, zpl_phyport_t, zpl_uint32);
	int (*sdk_stp_state_cb) (void *, zpl_index_t, zpl_phyport_t, zpl_uint32);
}sdk_mstp_t;

extern sdk_mstp_t sdk_mstp;
extern int bsp_mstp_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_MSTP_H__ */
