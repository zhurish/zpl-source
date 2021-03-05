/*
 * modem_client.h
 *
 *  Created on: Jul 24, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_CLIENT_H__
#define __MODEM_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zebra.h"
#include "prefix.h"
#include "os_list.h"
#include "modem.h"
#include "modem_bitmap.h"
#include "modem_driver.h"


#define ATCMD_RESPONSE_MAX	4096
#define ATCMD_REQUEST_MAX	512

typedef struct atcmd_response
{
	char buf[ATCMD_RESPONSE_MAX];
	ospl_size_t len;
}atcmd_response_t;

typedef struct atcmd_request
{
	char buf[ATCMD_REQUEST_MAX];
	ospl_size_t len;
}atcmd_request_t;

typedef struct modem_client_s
{
	NODE		node;
	ospl_uint32			bus;
	ospl_uint32			device;
	ospl_uint32			vendor;
	ospl_uint32			product;
	char		module_name[MODEM_PRODUCT_NAME_MAX];//EC25E 模块名称

	ospl_bool		active;

	//ospl_uint32			event;

	ospl_bool		echo;
	ospl_bool		echoold;
	ospl_bool		init;
	/*
	 * AT+CGMI
	 */
	char		factory_name[MODEM_PRODUCT_NAME_MAX];//厂商标识 Quectel
	/*
	 * AT+CGMM
	 */
	char		product_name[MODEM_PRODUCT_NAME_MAX];//EC25E 模块名称
	/*
	 * AT+CGMR
	 */
	char		product_iden[MODEM_STRING_MAX];//EC25E 标识 <revision>

	char		version[MODEM_STRING_MAX]; //版本信息
	/*
	 * AT+CGSN -> IMEI
	 */
	char				serial_number[MODEM_STRING_MAX];//生产序列号 (IMEI)
	/*
	 * AT+GSN  -> IMEI
	 */
	char				IMEI_number[MODEM_STRING_MAX];//生产序列号 (IMEI)

	/*
	 * AT+CPAS
	 */
	modem_activity_en	activity;



	/*
	 * 天线 信息
	 */
	ospl_uint8 				signal;		//信号强度
	ospl_uint8				bit_error;	//误码率
	modem_signal_state	signal_state;

	/*
	 * AT+CCID  SIM ID
	 */
	char				CCID_number[MODEM_STRING_MAX];//生产序列号 (CCID)
	/*
	 * AT+CIMI
	 */

	/*
	 * AT+CIMI:460077689452351  OK
	 * 国际移动用户识别码 IMSI=MCC+MNC+MSIN
	 * MCC： 移动国家码 中国 460
	 * MNC：移动网络码
	 */
	char				IMSI_number[MODEM_STRING_MAX];

	ospl_bool 				pin_lock;		//USIM 锁状态
	modem_cpin_en		cpin_status;	//USIM 状态
	char				CPIN[MODEM_STRING_MAX];//生产序列号 (CCID)

	//AT+COPS？ Operator Selection

	ospl_uint32					operator;		//当前注册运营商 (oper)
	nw_network_state	nw_state;		//网络状态标识 (stat)
	nw_rat_state		rat_state;		//无线接入技术 (Act) Wireless access technology


	//AT+CREG Network Registration status
	nw_register_type    nw_register_type;		//注册网络类型(for setup)
	nw_register_state	nw_register_state;		//当前注册网络类型(状态)


	modem_bitmap_t		hw_state;

	char				nw_act[MODEM_STRING_MAX];//当前无线接入技术
	char				nw_band[MODEM_STRING_MAX];//当前频段
	ospl_uint32					nw_channel;


	ospl_uint8				LAC[MODEM_STRING_MAX];	//小区信息
	ospl_uint8				CI[MODEM_STRING_MAX];	//基站信息

	struct prefix		prefix;


	void			*serial;
	void 			*modem;

	struct tty_com 	*attty;
	struct tty_com 	*pppd;

	struct modem_driver *driver;

	atcmd_request_t	 *atcmd;
	atcmd_response_t *response;
	ospl_bool			bSms;

}modem_client_t;


typedef struct modem_client_list_s
{
	LIST	*list;
	void	*mutex;

}modem_client_list_t;










typedef int (*modem_client_cb)(modem_client_t *, void *);


extern int modem_client_init(void);
extern int modem_client_exit(void);

extern const char * modem_client_product_name(modem_client_t *);

extern modem_client_t * modem_client_alloc(ospl_uint32 vendor, ospl_uint32 product);
extern int modem_client_free(modem_client_t *);

extern int modem_client_add_api(modem_client_t *client);


extern int modem_client_register_api(modem_client_t *client);
//extern int modem_client_unregister_api(modem_client_t *client);


extern int modem_client_del_api(modem_client_t *client);
extern int modem_client_del_by_product_api(ospl_uint32 vendor, ospl_uint32 product);
extern int modem_client_del_by_product_name_api(char *name);

extern modem_client_t * modem_client_lookup_api(ospl_uint32 vendor, ospl_uint32 product);
extern modem_client_t * modem_client_lookup_by_name_api(char *name);

extern int modem_client_callback_api(modem_client_cb cb, void *pVoid);



#ifdef __cplusplus
}
#endif


#endif /* __MODEM_CLIENT_H__ */
