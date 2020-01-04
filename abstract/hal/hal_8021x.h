/*
 * hal_8021x.h
 *
 *  Created on: May 7, 2018
 *      Author: zhurish
 */

#ifndef __HAL_8021X_H__
#define __HAL_8021X_H__




typedef struct sdk_8021x_s
{
	//8021x
	int (*sdk_8021x_enable_cb) (void *, ifindex_t, BOOL);
	int (*sdk_8021x_state_cb) (void *, ifindex_t, u_int);
	int (*sdk_8021x_mode_cb) (void *, ifindex_t, u_int);
	int (*sdk_8021x_auth_bypass_cb) (void *, ifindex_t, u_int);

/*	int (*sdk_8021x_dstmac_cb) (ifindex_t, u_char *mac);
	int (*sdk_8021x_address_cb) (u_int index, u_int address, u_int mask);
	int (*sdk_8021x_mult_address_cb) (u_int index, BOOL);
	int (*sdk_8021x_mode_cb) (u_int index, BOOL);*/

	void *sdk_driver;
}sdk_8021x_t;


extern int hal_8021x_interface_enable(ifindex_t ifindex, BOOL enable);

extern int hal_8021x_state(ifindex_t ifindex, u_int value);

extern int hal_8021x_mode(ifindex_t ifindex, u_int value);

extern int hal_8021x_auth_bypass(ifindex_t ifindex, u_int value);


#endif /* __HAL_8021X_H__ */
