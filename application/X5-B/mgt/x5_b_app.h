/*
 * x5_b_app.h
 *
 *  Created on: 2019年3月22日
 *      Author: DELL
 */

#ifndef __X5_B_APP_H__
#define __X5_B_APP_H__

#include "x5_b_global.h"

#define X5B_APP_TEST_DEBUG
#define X5B_APP_TIMESYNC_C

#define X5B_APP_LOCAL_PORT_DEFAULT			9527
#define X5B_APP_REMOTE_PORT_DEFAULT			9527
#define X5B_APP_TLV_DEFAULT				1024
#define X5B_APP_BUF_DEFAULT				1024+128
#define X5B_APP_INTERVAL_DEFAULT		10
#define X5B_APP_INTERVAL_CNT_DEFAULT	6
#define X5B_APP_WAITING_TIMEOUT 		5000

#define X5B_APP_VERSION_MAX				128

#define X5B_APP_STATIC_IP_ENABLE

#ifdef X5B_APP_STATIC_IP_ENABLE
#define X5B_APP_A_IP_DEFAULT		"10.10.10.101"
#define X5B_APP_C_IP_DEFAULT		"10.10.10.100"
#endif

#define X5_B_ESP32_DEBUG_EVENT		0X01
#define X5_B_ESP32_DEBUG_HEX		0X02
#define X5_B_ESP32_DEBUG_RECV		0X04
#define X5_B_ESP32_DEBUG_SEND		0X08
#define X5_B_ESP32_DEBUG_UPDATE		0X10
#define X5_B_ESP32_DEBUG_TIME		0X20
#define X5_B_ESP32_DEBUG_WEB		0X40
#define X5_B_ESP32_DEBUG_MSG		0X80
#define X5_B_ESP32_DEBUG_STATE		0X100
#define X5_B_ESP32_DEBUG_UCI		0X200

#define X5_B_ESP32_DEBUG(n)		(X5_B_ESP32_DEBUG_ ## n & x5b_app_mgt->debug)
#define X5_B_ESP32_DEBUG_ON(n)	(x5b_app_mgt->debug |= (X5_B_ESP32_DEBUG_ ## n ))
#define X5_B_ESP32_DEBUG_OFF(n)	(x5b_app_mgt->debug &= ~(X5_B_ESP32_DEBUG_ ## n ))



typedef struct x5b_app_statistics_s
{
	u_int32 rx_packet;
	u_int32 tx_packet;
} x5b_app_statistics_t;


typedef struct x5b_app_mgt_node_s
{
	u_int32		id;				//module ID
	void		*t_thread;
	BOOL		reg_state;		//register state
	u_int8 		keep_cnt;			//>= 3: OK, else ERROR;
#define X5B_APP_MGT_STATE_OK(n)		((n)->state)>=0
	u_int8		interval;		//keepalive interval

	//BOOL 		msg_sync;		//注册同步
	u_int32		address;		//module address
	//char		*remote_address;
	u_int16		remote_port;	//module udp port
	u_int32		remote_timestmp;

	char		sbuf[X5B_APP_BUF_DEFAULT];
	int			slen;
	int			offset;
	u_int8		seqnum;

	x5b_app_statistics_t statistics;
	char		version[X5B_APP_VERSION_MAX];
	char		ioversion[X5B_APP_VERSION_MAX];
	//u_int8		face_snyc;
	u_int8		wiggins;
	void		*priv;
	//char		*face_path;
}x5b_app_mgt_node_t;


typedef struct wan_state_s
{
	u_int32 link_phy;
	//u_int32 link;
	u_int32 address;
	void	*t_thread;
	u_int8	interval;
}wan_state_t;


typedef struct x5b_app_mgt_s
{
	BOOL	enable;
	int		task_id;
	void	*master;
	int		r_fd;
	int		w_fd;
	void	*r_thread;

	void	*reset_thread;
	void	*t_thread;

	char	*local_address;
	u_int16	local_port;

	x5b_app_mgt_node_t	app_a;
	x5b_app_mgt_node_t	app_c;
	x5b_app_mgt_node_t	*app;

	u_int32	fromid;		//module ID from remote
	char	buf[X5B_APP_BUF_DEFAULT];
	int		len;
	u_int8	seqnum;		//seqnum of recv MSG

	u_int8	ack_seqnum;		//应答报文中的ACK

	u_int32	debug;
	BOOL	not_debug;

	BOOL	sync_ack;		//停等接收，一问一答

	BOOL	time_sync;		//时间是否已经同步

	BOOL	regsync;		//两个都注册上
	u_int16 wait_timeout;
	void	*mutex;

	struct sockaddr_in from;

	BOOL	upgrade;		//upgrate A(STM32/ESP32)
	char	up_buf[16];
	u_int8	up_buf_len;

	wan_state_t wan_state;	//wan接口的状态

	void	*report_thread;//自动上报
	u_int32	report_event;
	void	*k_thread;		//keepalive

}x5b_app_mgt_t;




extern x5b_app_mgt_t *x5b_app_mgt;


extern int x5b_app_module_init(char *local, u_int16 port);
extern int x5b_app_module_exit();
extern int x5b_app_module_task_init();
extern int x5b_app_module_task_exit();



/*extern int x5b_app_update_mode_init(x5b_app_mgt_t *app);
extern int x5b_app_update_mode_exit(x5b_app_mgt_t *app);*/
extern int x5b_app_update_mode_enable(x5b_app_mgt_t *app, BOOL enable, int to);
extern int x5b_app_read_eloop_reload(x5b_app_mgt_t *mgt);
/*
 * raw
 */
extern int x5b_app_make_update(x5b_app_mgt_t *mgt, int to);
extern int x5b_app_crc_make(x5b_app_mgt_t *mgt);
extern int x5b_app_hdr_make(x5b_app_mgt_t *mgt);
extern int x5b_app_send_msg(x5b_app_mgt_t *mgt);
extern int x5b_app_read_msg_timeout(x5b_app_mgt_t *mgt, int timeout_ms, char *output, int outlen);
extern int x5b_app_read_handle(x5b_app_mgt_t *mgt);
/*
 *
 */
//extern int x5b_app_mode_load(x5b_app_mgt_t *mgt);

extern int x5b_app_local_address_set_api(char *address);
extern int x5b_app_local_port_set_api(u_int16 port);
extern int x5b_app_port_set_api(int to, u_int16 port);
extern int x5b_app_interval_set_api(int to, u_int8 interval);



extern int x5b_app_rtc_request(x5b_app_mgt_t *app, int to);
extern int x5b_app_IP_address_api(x5b_app_mgt_t *app, u_int32 address, int to);

extern int x5b_app_keepalive_send(x5b_app_mgt_t *mgt, int res, int to);

extern int x5b_app_sync_web_time(x5b_app_mgt_t *mgt, int to);
extern int x5b_app_version_request(x5b_app_mgt_t *app, int to);

//WAN接口状态上报
extern int x5b_app_network_port_status_api(x5b_app_mgt_t *app, int res, int to);

extern int x5b_app_reboot_request(x5b_app_mgt_t *app, int to, BOOL reset);


/*
 * for CLI
 */
extern void * x5b_app_tmp();
extern int x5b_app_free();
extern int x5b_app_show_config(struct vty *vty);
extern int x5b_app_show_debug(struct vty *vty);
extern int x5b_app_show_state(struct vty *vty);

#ifdef X5B_APP_TEST_DEBUG
extern int x5b_app_test_register(int to, int p);
extern int x5b_app_test_call(u_int16 num);
#endif

extern int x5b_app_debug_proc();


#include "x5_b_call.h"
#include "x5_b_io.h"

#endif /* __X5_B_APP_H__ */
