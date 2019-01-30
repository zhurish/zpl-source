/*
 * x5_b_a.h
 *
 *  Created on: 2018��12��19��
 *      Author: DELL
 */

#ifndef __X5_B_A_H__
#define __X5_B_A_H__


#define X5_B_A_DEBUG

#define X5_B_A_PORT_DEFAULT			9527
#define X5_B_A_BUF_DEFAULT			1024
#define X5_B_A_INTERVAL_DEFAULT		5
#define X5_B_A_HDR_MAKR				0X7E

#define X5_B_A_ADDRESS_DEFAULT		"10.10.10.101"

enum E_OPEN_RESULT
{
	E_OPEN_RESULT_OK,
	E_OPEN_RESULT_FAIL,
};

enum E_OPEN_CMD
{
	E_OPEN_DOOR_OK,
	E_OPEN_DOOR_ACK,
};

enum E_CALL_RESULT
{
	E_CALL_RESULT_UNREGISTER,		//号码未注册
	E_CALL_RESULT_CALLING,			//呼叫中
	E_CALL_RESULT_TALKLING,			//通话中
	E_CALL_RESULT_STOP,				//ͨ挂断
	E_CALL_RESULT_FAIL,				//ͨ拨号失败
};

enum E_CMD
{
	E_CMD_ACK = 0X01,
	E_CMD_RESULT = 0X02,
	E_CMD_START_CALLING = 0X03,
	E_CMD_STOP_CALLING = 0X04,
	E_CMD_ON_LINE = 0X05,
	E_CMD_CALL_RESULT = 0X06,

	E_CMD_DOOR_TYPE = 0X07,
	E_CMD_KEY = 0X08,
	E_CMD_FACTORY_MODE = 0X09,

	E_CMD_OPEN_DOOR = 0X10,

	E_CMD_VERSION = 0X0b,

	E_CMD_KEEPALIVE = 0x0a,
};


enum E_CMD_LEN
{
	E_CMD_ACK_LEN = 4,
	E_CMD_RESULT_LEN = 4,
	E_CMD_START_CALLING_LEN = 32,
	E_CMD_STOP_CALLING_LEN = 4,
	E_CMD_ON_LINE_LEN = 4,
	E_CMD_CALL_RESULT_LEN = 4,

	E_CMD_DOOR_TYPE_LEN = 4,
	E_CMD_KEY_LEN = 4,
	E_CMD_FACTORY_MODE_LEN = 128,

	E_CMD_OPEN_DOOR_LEN = 4,
	E_CMD_VERSION_LEN = 22,
	E_CMD_KEEPALIVE_LEN = 4,
};


#define X5_B_ESP32_DEBUG_EVENT		0X01
#define X5_B_ESP32_DEBUG_HEX		0X02
#define X5_B_ESP32_DEBUG_RECV		0X04
#define X5_B_ESP32_DEBUG_SEND		0X08

#define X5_B_ESP32_DEBUG(n)		(X5_B_ESP32_DEBUG_ ## n & x5_b_a_mgt->debug)
#define X5_B_ESP32_DEBUG_ON(n)	(x5_b_a_mgt->debug |= (X5_B_ESP32_DEBUG_ ## n ))
#define X5_B_ESP32_DEBUG_OFF(n)	(x5_b_a_mgt->debug &= ~(X5_B_ESP32_DEBUG_ ## n ))


#pragma pack(1)
typedef struct x5_b_a_hdr_s
{
	u_int8 makr;
	u_int8 seqnum;
	u_int32 total_len;		//ֻ��TLV�ĳ���
} x5_b_a_hdr_t;
#pragma pack()

typedef struct x5_b_key_val_s
{
	u_int8 keynum;
	u_int32 keyval[512];
} x5_b_key_val_t;

typedef struct x5_b_room_position_s
{
	char data[32];
} x5_b_room_position_t;

typedef struct x5_b_statistics_s
{
	u_int32 rx_packet;
	u_int32 tx_packet;
} x5_b_statistics_t;

typedef struct x5_b_factory_data_s
{
	u_int32 local_address;			//����IP��ַ��IP/DHCP��
	u_int32 sip_server;				//����SIP��������ַ
	u_int32 sip_proxy_server;		//���ô����������ַ
	u_int16	sip_port;				//����SIP�������˿ں�
	u_int16	sip_proxy_port;			//���ô���������˿ں�
	u_int16	sip_local_port;			//���ñ���SIP�������˿ں�
	u_int16	rtp_local_port;			//����RTP�˿�
	//u_int16	rtcp_local_port;		//����RTCP�˿�
	u_int8 	phone_number[6];		//���غ��룬ÿ������λռ��4bit��

} x5_b_factory_data_t;

typedef struct x5_b_version_s
{
	u_int8 hw[11];
	u_int8 sw[11];
} x5_b_version_t;



typedef struct x5_b_a_mgt_s
{
	int		enable;
	int		task_id;
	void	*master;
	int		r_fd;
	int		w_fd;
	void	*r_thread;
	void	*w_thread;
	void	*reset_thread;
	void	*t_thread;
	u_int8 	state;			//>= 3: OK, else ERROR;
	u_int8	interval;
	char	*local_address;
	u_int16	local_port;

	char	*remote_address;
	u_int16	remote_port;

	char	buf[X5_B_A_BUF_DEFAULT];
	int		len;
	u_int8	seqnum;

	char	sbuf[X5_B_A_BUF_DEFAULT];
	int		slen;
	int		offset;
	u_int8	s_seqnum;

	x5_b_key_val_t	keyval;
	x5_b_factory_data_t	fact;
	x5_b_room_position_t room;

	x5_b_statistics_t  statistics;

	u_int8	debug;
}x5_b_a_mgt_t;


extern x5_b_a_mgt_t *x5_b_a_mgt;

extern uint16_t Data_CRC16Check ( uint8_t * data, uint16_t leng );

extern int x5_b_a_call_result_api(x5_b_a_mgt_t *mgt, int res);
extern int x5_b_a_open_result_api(x5_b_a_mgt_t *mgt, int res);

extern int x5_b_a_address_set_api(char *remote);
extern int x5_b_a_local_address_set_api(char *address);
extern int x5_b_a_port_set_api(u_int16 port);
extern int x5_b_a_local_port_set_api(u_int16 port);

extern void * x5_b_a_app_tmp();
extern int x5_b_a_app_free();


extern int x5_b_a_show_config(struct vty *vty);
extern int x5_b_a_show_debug(struct vty *vty);
extern int x5_b_a_show_state(struct vty *vty);

extern int x5_b_a_module_init(char *remote, u_int16 port);
extern int x5_b_a_module_exit();
extern int x5_b_a_module_task_init();
extern int x5_b_a_module_task_exit();

extern int x5_b_a_show_state(struct vty *vty);
extern int x5_b_a_show_config(struct vty *vty);

extern int x5_b_a_open_door_api(x5_b_a_mgt_t *mgt, int res);


#ifdef X5_B_A_DEBUG
extern int call_recv_test();
extern int call_result_test(int res);
extern int open_result_test(int res);
#endif

#endif /* __X5_B_A_H__ */
