/*
 * x5_b_ctl.h
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
 */

#ifndef __X5_B_MGT_X5_B_CTL_H__
#define __X5_B_MGT_X5_B_CTL_H__

extern int x5b_app_local_mac_address_get(u_int8 *address);
extern int x5b_app_local_address_get(u_int32 *address);
extern int x5b_app_factory_set(x5b_app_factory_t *data);

extern int x5b_app_rtc_tm_set(int timesp);

extern int x5b_app_start_call(BOOL start, x5b_app_call_t *call);
extern int x5b_app_start_call_phone(BOOL start, char *call);
extern int x5b_app_stop_call(BOOL start, x5b_app_call_t *call);

extern int x5b_app_local_network_info_get(x5b_app_netinfo_t *info);
extern int x5b_app_local_register_info_get(x5b_app_phone_register_ack_t *info);
extern int x5b_app_network_port_status_get(x5b_app_mgt_t *app);

extern int x5b_app_call_room_param_get(void *data, u_int8 *building,
		u_int8 *unit, u_int16 *room);

extern int x5b_app_a_thlog_log(char *format);


extern int x5b_app_network_event_init(x5b_app_mgt_t *app);
extern int x5b_app_network_event_exit(x5b_app_mgt_t *app);

extern int x5b_app_AC_state_load(x5b_app_mgt_t *mgt);
extern int x5b_app_AC_state_save(x5b_app_mgt_t *mgt);

extern int x5b_app_timezone_offset_api(char * res);

int inet64_to_mac(u_int64 value, u_int8 *dst);
u_int64 mac_to_inet64(u_int8 *dst);
//extern int x5b_app_rtc_time_get(int to, rtcTimeType *ti);

#endif /* __X5_B_MGT_X5_B_CTL_H__ */
