/*
 * x5_b_ctl.h
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
 */

#ifndef __X5_B_MGT_X5_B_CTL_H__
#define __X5_B_MGT_X5_B_CTL_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int x5b_app_local_mac_address_get(zpl_uint8 *address);
extern int x5b_app_local_address_get(zpl_uint32 *address);
extern int x5b_app_factory_set(x5b_app_factory_t *data);

extern int x5b_app_rtc_tm_set(int timesp);

extern int x5b_app_start_call(zpl_bool start, x5b_app_call_t *call);
extern int x5b_app_start_call_phone(zpl_bool start, char *call);
extern int x5b_app_start_call_user(zpl_bool start, char *call);
extern int x5b_app_stop_call(zpl_bool start, x5b_app_call_t *call);

extern int x5b_app_local_network_info_get(x5b_app_netinfo_t *info);
extern int x5b_app_local_register_info_get(x5b_app_phone_register_ack_t *info);
extern int x5b_app_network_port_status_get(x5b_app_mgt_t *app);


extern int x5b_app_network_event_init(x5b_app_mgt_t *app);
extern int x5b_app_network_event_exit(x5b_app_mgt_t *app);
extern zpl_bool x5b_app_port_status_get();


extern int x5b_app_timezone_offset_api(char * res);


extern int x5b_app_call_room_param_get(void *data, zpl_uint8 *building,
		zpl_uint8 *unit, zpl_uint16 *room);

#ifdef X5B_APP_IO_LOG
extern int x5b_app_a_thlog_log(char *format);
#endif
extern int x5b_app_c_log_card(char *format);


#ifdef __cplusplus
}
#endif

#endif /* __X5_B_MGT_X5_B_CTL_H__ */
