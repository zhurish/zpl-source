/*
 * x5_b_ctl.h
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
 */

#ifndef __X5_B_MGT_X5_B_CTL_H__
#define __X5_B_MGT_X5_B_CTL_H__


extern int x5b_app_factory_set(x5_b_factory_data_t *data);

extern int x5b_app_start_call(BOOL start, x5_b_room_position_t *room);

extern int x5b_app_call_result_api(int res);
extern int x5b_app_open_result_api(int res);
extern int x5b_app_open_door_api(int res);

#endif /* __X5_B_MGT_X5_B_CTL_H__ */
