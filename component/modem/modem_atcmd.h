/*
 * modem_atcmd.h
 *
 *  Created on: Jul 25, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_ATCMD_H__
#define __MODEM_ATCMD_H__

#ifdef __cplusplus
extern "C" {
#endif

//#define __MODEM_CMD_DEBUG
#ifdef __MODEM_CMD_DEBUG
#define MODEM_CMD_DEBUG(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_CMD_WARN(fmt,...)		modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#define MODEM_CMD_ERROR(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#else
#define MODEM_CMD_DEBUG(fmt,...)
#define MODEM_CMD_WARN(fmt,...)
#define MODEM_CMD_ERROR(fmt,...)
#endif


#define MODEM_TIMEOUT( n )		((n))


/*
 * 查看AT 是否打开
 */
extern int modem_atcmd_isopen(modem_client_t *);
/*
 * 打开AT 回显
 */
extern int modem_atcmd_echo(modem_client_t *);

/*
 * 获取厂商标识
 */
extern int modem_factory_atcmd_get(modem_client_t *);

/*
 * 获取模块标识
 */
extern int modem_product_atcmd_get(modem_client_t *);

/*
 * AT+CGSN
 * 获取模块序列号
 */
extern int modem_product_id_atcmd_get(modem_client_t *);

/*
 * AT+CGMR
 * 获取版本信息
 */
extern int modem_version_atcmd_get(modem_client_t *);

/*
 * AT+CSQ
 * 获取信号信息
 */
extern int modem_signal_atcmd_get(modem_client_t *);

//#define modem_IMEI_atcmd_get modem_serial_number_atcmd_get
extern int modem_serial_number_atcmd_get(modem_client_t *client);
extern int modem_IMEI_atcmd_get(modem_client_t *client);

extern int modem_nwcell_atcmd_set(modem_client_t *client, ospl_bool enable);

extern int modem_echo_atcmd_set(modem_client_t *client, ospl_bool enable);
extern int modem_swreset_atcmd_set(modem_client_t *client);
extern int modem_reboot_atcmd_set(modem_client_t *client);
extern int modem_save_atcmd_set(modem_client_t *client);
extern int modem_open_atcmd_set(modem_client_t *client);
extern int modem_activity_atcmd_get(modem_client_t *client);
extern int modem_gprsattach_atcmd_set(modem_client_t *client);
extern int modem_nwscanmode_atcmd_set(modem_client_t *client);
extern int modem_nwscanseq_atcmd_set(modem_client_t *client);
extern int modem_roamservice_atcmd_set(modem_client_t *client);
extern int modem_servicedomain_atcmd_set(modem_client_t *client);
extern int modem_nwband_atcmd_set(modem_client_t *client);
extern int modem_hsdpacat_atcmd_set(modem_client_t *client);
extern int modem_hsupacat_atcmd_set(modem_client_t *client);
extern int modem_sgsn_atcmd_set(modem_client_t *client);
extern int modem_tdscsq_atcmd_set(modem_client_t *client);
extern int modem_nwpdp_atcmd_set(modem_client_t *client);
extern int modem_nwpdp_atcmd_enable(modem_client_t *client, ospl_bool enable);
extern int modem_nwinfo_atcmd_get(modem_client_t *client);
extern int modem_nwaddr_atcmd_get(modem_client_t *client);
extern int modem_nwservingcell_atcmd_get(modem_client_t *client);
extern int modem_nwreq_addr_atcmd_get(modem_client_t *client);
extern int modem_IMSI_atcmd_get(modem_client_t *client);
extern int modem_CCID_atcmd_get(modem_client_t *client);
extern int modem_cell_information_atcmd_set(modem_client_t *client);
extern int modem_cell_information_atcmd_get(modem_client_t *client);

extern int modem_operator_atcmd_get(modem_client_t *client);



#ifdef __cplusplus
}
#endif

#endif /* __MODEM_ATCMD_H__ */
