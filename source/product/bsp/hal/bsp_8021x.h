/*
 * bsp_8021x.h
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */

#ifndef __BSP_8021X_H__
#define __BSP_8021X_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct sdk_8021x_s
{
	//8021x
	/*全局使能*/
	int (*sdk_8021x_enable_cb) (void *, zpl_bool);

	/*端口认证状态*/
	int (*sdk_8021x_port_state_cb) (void *, zpl_phyport_t , zpl_int32);
	/*端口认证模式*/
	int (*sdk_8021x_port_mode_cb) (void *, zpl_phyport_t , zpl_int32);

	int (*sdk_8021x_auth_dmac_cb) (void *, zpl_phyport_t, mac_t *);


}sdk_8021x_t;

extern sdk_8021x_t sdk_8021x_cb;

extern int bsp_8021x_module_handle(struct hal_client *client, zpl_uint32 cmd, zpl_uint32 subcmd, void *driver);



#ifdef __cplusplus
}
#endif
#endif /* __BSP_8021X_H__ */
