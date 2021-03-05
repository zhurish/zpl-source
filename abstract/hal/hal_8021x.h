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


typedef struct sdk_8021x_s
{
	//8021x
	int (*sdk_8021x_enable_cb) (void *, ifindex_t, ospl_bool);
	int (*sdk_8021x_state_cb) (void *, ifindex_t, ospl_uint32);
	int (*sdk_8021x_mode_cb) (void *, ifindex_t, ospl_uint32);
	int (*sdk_8021x_auth_bypass_cb) (void *, ifindex_t, ospl_uint32);

/*	int (*sdk_8021x_dstmac_cb) (ifindex_t, ospl_uchar *mac);
	int (*sdk_8021x_address_cb) (ospl_uint32 index, ospl_uint32 address, ospl_uint32 mask);
	int (*sdk_8021x_mult_address_cb) (ospl_uint32 index, ospl_bool);
	int (*sdk_8021x_mode_cb) (ospl_uint32 index, ospl_bool);*/

	void *sdk_driver;
}sdk_8021x_t;


extern int hal_8021x_interface_enable(ifindex_t ifindex, ospl_bool enable);

extern int hal_8021x_state(ifindex_t ifindex, ospl_uint32 value);

extern int hal_8021x_mode(ifindex_t ifindex, ospl_uint32 value);

extern int hal_8021x_auth_bypass(ifindex_t ifindex, ospl_uint32 value);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_8021X_H__ */
