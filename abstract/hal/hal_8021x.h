/*
 * hal_8021x.h
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */

#ifndef __HAL_8021X_H__
#define __HAL_8021X_H__

#ifdef __cplusplus
extern "C" {
#endif


enum hal_8021x_cmd 
{
    HAL_8021X_NONE,
	HAL_8021X_PORT,
	HAL_8021X_PORT_MAC,
	HAL_8021X_PORT_STATE,
	HAL_8021X_PORT_MODE,
	HAL_8021X_PORT_BYPASS,
};

typedef struct hal_8021x_param_s
{
	union hal_8021x
	{
	zpl_uint8 value;
	mac_t mac[NSM_MAC_MAX];
	}u;

}hal_8021x_param_t;


extern int hal_8021x_interface_enable(ifindex_t ifindex, zpl_bool enable);

extern int hal_8021x_state(ifindex_t ifindex, zpl_uint32 value);

extern int hal_8021x_mode(ifindex_t ifindex, zpl_uint32 value);

extern int hal_8021x_auth_bypass(ifindex_t ifindex, zpl_uint32 value);

extern int hal_8021x_interface_dstmac(ifindex_t ifindex, zpl_uchar *mac);

extern int hal_8021x_auth_addmac(ifindex_t ifindex, mac_t *mac);
extern int hal_8021x_auth_delmac(ifindex_t ifindex, mac_t *mac);
extern int hal_8021x_auth_delallmac(ifindex_t ifindex);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_8021X_H__ */
