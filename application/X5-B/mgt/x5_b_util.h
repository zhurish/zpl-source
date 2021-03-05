/*
 * x5_b_util.h
 *
 *  Created on: 2019年5月20日
 *      Author: DELL
 */

#ifndef __X5_B_UTIL_H__
#define __X5_B_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	X5B_KEEPALIVE_EV,
	X5B_TIMER_EV,
	X5B_READ_EV,
	X5B_WRITE_EV,
	X5B_RESET_EV,
	X5B_REPORT_EV,
#ifdef X5B_APP_TCP_ENABLE
	X5B_TCP_ACCEPT_EV,
	X5B_TCP_READ_EV,
	X5B_TCP_WRITE_EV,
#endif
} x5_b_event_t;

int x5b_app_statistics(x5b_app_mgt_t *mgt, int tx, int from);
ospl_uint16 Data_CRC16Check ( ospl_uint8 * data, ospl_uint16 leng );

int x5b_app_hex_debug(x5b_app_mgt_t *mgt, char *hdr, int rx);
int x5b_app_event_inactive(x5b_app_mgt_t *mgt, x5_b_event_t ev, int who);
int x5b_app_event_active(x5b_app_mgt_t *mgt, x5_b_event_t ev, int who, int value);

int x5b_app_read_msg_timeout(x5b_app_mgt_t *app, int timeout_ms, char *output, int outlen);
int x5b_app_read_chk_handle(x5b_app_mgt_t *mgt);

int x5b_app_send_msg_without_ack(x5b_app_mgt_t *mgt);
int x5b_app_send_msg(x5b_app_mgt_t *mgt);

int x5b_app_socket_init(x5b_app_mgt_t *mgt);
int x5b_app_socket_exit(x5b_app_mgt_t *mgt);

#ifdef __cplusplus
}
#endif

#endif /* __X5_B_UTIL_H__ */
