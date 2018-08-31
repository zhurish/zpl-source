/*
 * hal_mstp.h
 *
 *  Created on: May 6, 2018
 *      Author: zhurish
 */

#ifndef __HAL_MSTP_H__
#define __HAL_MSTP_H__


typedef struct sdk_mstp_s
{
	int (*sdk_mstp_enable_cb) (BOOL);
	int (*sdk_mstp_age_cb) (int);
	int (*sdk_mstp_bypass_cb) (BOOL, int);
	int (*sdk_mstp_state_cb) (ifindex_t, int, int);
	int (*sdk_stp_state_cb) (ifindex_t,  int);
}sdk_mstp_t;


int hal_mstp_enable(BOOL enable);
int hal_mstp_age(int age);
int hal_mstp_bypass(BOOL enable, int type);
int hal_mstp_state(ifindex_t ifindex, int mstp, int state);
int hal_stp_state(ifindex_t ifindex, int state);

#endif /* __HAL_MSTP_H__ */
