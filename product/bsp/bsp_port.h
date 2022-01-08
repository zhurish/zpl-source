/*
 * bsp_port.h
 *
 *  Created on: Jan 21, 2018
 *      Author: zhurish
 */

#ifndef __BSP_PORT_H__
#define __BSP_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sdk_port_s
{
	int (*sdk_port_enable_cb)(void *, zpl_phyport_t, zpl_bool);

	int (*sdk_port_link_cb)(void *, zpl_phyport_t, zpl_bool);

	int (*sdk_port_speed_cb)(void *, zpl_phyport_t, zpl_uint32);
	int (*sdk_port_duplex_cb)(void *, zpl_phyport_t, zpl_uint32);
	int (*sdk_port_flow_cb)(void *, zpl_phyport_t, zpl_uint32);
	int (*sdk_port_jumbo_cb)(void *, zpl_phyport_t, zpl_bool);

	zpl_bool (*sdk_port_state_get_cb)(void *, zpl_phyport_t);
	zpl_uint32 (*sdk_port_speed_get_cb)(void *, zpl_phyport_t);
	zpl_uint32 (*sdk_port_duplex_get_cb)(void *, zpl_phyport_t);


	int (*sdk_port_loop_cb)(void *, zpl_phyport_t, zpl_uint32);
	int (*sdk_port_8021x_cb)(void *, zpl_phyport_t, zpl_uint32);

	int (*sdk_port_learning_enable_cb)(void *, zpl_phyport_t, zpl_bool);
	int (*sdk_port_swlearning_enable_cb)(void *, zpl_phyport_t, zpl_bool);

	int (*sdk_port_protected_enable_cb)(void *, zpl_phyport_t, zpl_bool);
	int (*sdk_port_wan_enable_cb)(void *, zpl_phyport_t, zpl_bool);

	int (*sdk_port_mac_cb)(void *, zpl_phyport_t, zpl_uint8 *, zpl_bool);
	int (*sdk_port_mtu_cb)(void *, zpl_phyport_t, zpl_uint32);
//	int (*sdk_port_metric_cb)(void *, zpl_phyport_t, zpl_uint32);
	int (*sdk_port_vrf_cb)(void *, zpl_phyport_t, zpl_uint32);

	int (*sdk_port_mode_cb)(void *, zpl_phyport_t, zpl_uint32);

}sdk_port_t;


extern sdk_port_t sdk_port;
extern int bsp_port_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);


#ifdef __cplusplus
}
#endif


#endif /* __BSP_PORT_H__ */
