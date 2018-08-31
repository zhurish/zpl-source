/*
 * modem_enum.h
 *
 *  Created on: Aug 12, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_ENUM_H__
#define __MODEM_ENUM_H__

typedef enum
{
	MODEM_DIAL_NONE = 0,
	MODEM_DIAL_DHCP,
	MODEM_DIAL_QMI,
	MODEM_DIAL_GOBINET,
	MODEM_DIAL_PPP,

	MODEM_DIAL_MAX,
}modem_dial_type;

typedef enum
{
	MODEM_IPV4 = 0,
	MODEM_IPV6,
	MODEM_BOTH,
}modem_stack_type;

typedef enum modem_activity_s
{
	CPAS_NONE = 0,
	CPAS_READY = 1,
	CPAS_RINGING = 3,
	CPAS_CALL_HOLD = 4,
}modem_activity_en;


typedef enum modem_cpin_s
{
	CPIN_NONE = 0,
	CPIN_READY,
	CPIN_SIM_PIN,
	CPIN_SIM_PUK,
	CPIN_SIM_PIN2,
	CPIN_SIM_PUK2,

	CPIN_PH_NET_PIN,
	CPIN_PH_NET_PUK,

	CPIN_PH_NET_SUB_PIN,
	CPIN_PH_NET_SUB_PUK,

	CPIN_PH_SP_PIN,
	CPIN_PH_SP_PUK,

	CPIN_PH_CORP_PIN,
	CPIN_PH_CORP_PUK,

}modem_cpin_en;


typedef enum
{
	NETWORK_AUTO  = 0,
	NETWORK_GSM,		//2G 移动 联通
	NETWORK_CDMA1X,		//2G 电信

	NETWORK_GPRS,		//2.5G
	NETWORK_HSCSD,		//2.5G
	NETWORK_WAP,		//2.5G
	NETWORK_EDGE,		//2.5G

	NETWORK_TD_SCDMA,	//3G 移动
	NETWORK_WCDMA,		//3G 联通
	NETWORK_EVDO,		//3G 电信 (CDMA2000)

	NETWORK_TD_LTE,		//4G 移动 联通 电信
	NETWORK_FDD_LTE,	//4G 联通 电信

}modem_network_type;


typedef enum //运营商x信息格式
{
	OP_LONG_FORMAT  = 0,	//长字符串
	OP_SHORT_FORMAT,		//短字符串
	OP_DIGIT_FORMAT,		//数字格式
}modem_op_format;

typedef enum //网络状态
{
	NW_STATE_UNKNOWN  = 0,	//未知的
	NW_STATE_USABLE,		//可用的
	NW_STATE_CURRENT,		//当前的
	NW_STATE_PROHIBIT,		//禁止的 prohibit
}nw_network_state;

typedef enum //无线接入技术 Wireless access technology
{
	NW_RAT_GSM  = 0,	//
	NW_RAT_QGSM,		//GSM增强型
	NW_RAT_UTRAN,		//
	NW_RAT_GSM_EGPRS,		//
	NW_RAT_UTRAN_HSDPA,		//
	NW_RAT_UTRAN_HSUPA,		//
	NW_RAT_UTRAN_BOTH,		//
	NW_RAT_E_UTRAN,		//
	NW_RAT_CDMA,		//
}nw_rat_state;


typedef enum //运营商
{
	OPERATOR_NONE  = 0,
	OPERATOR_MOBILE,		//移动
	OPERATOR_UNICOM,		//联通
	OPERATOR_TELECOM,		//电信
	OPERATOR_ATANDT,		//AT&T
}nw_operator_type;


typedef enum //网络注册
{
	REGISTER_NONE  = 0,
	REGISTER_LOCAL,			//本的注册
	REGISTER_SERASH,		//搜索
	REGISTER_UNKNOW,		//未知
	REGISTER_ROAM,			//已注册，处于漫游
}nw_register_state;

typedef enum //网络注册类型
{
	NW_REGISTER_DISABLE  = 0,
	NW_REGISTER_ENABLE,			//
	NW_REGISTER_ENABLE_LOCAL,	//
}nw_register_type;



typedef enum
{
	RES_OK  = 0,
	RES_ERROR = -1,
	RES_TIMEOUT  = -2,
	RES_CLOSE  = -3,
	RES_AGAIN  = -4,
}md_res_en;

typedef enum
{
	MODEM_STATE_SIGNAL_NONE = 0,

	MODEM_STATE_SIGNAL_LOSS,

	MODEM_STATE_SIGNAL_READY,

	MODEM_STATE_BITERR_READY,

	MODEM_STATE_BITERR_HIGH,

	MODEM_STATE_SIGNAL_MAX,

}modem_signal_state;


typedef enum
{
	MODEM_STATE_HW_NONE = 0,

	MODEM_STATE_HW_CFUN,		//FUNC

	/*
	 * QCFG
	 */
	MODEM_STATE_HW_GPRSATTACH,		//gprsattach
	MODEM_STATE_HW_NWSCANMODE,		//nwscanmode
	MODEM_STATE_HW_NWSCANSEQ,		//
	MODEM_STATE_HW_REAMSERVICE,		//
	MODEM_STATE_HW_SERVICEDOMIAN,	//
	MODEM_STATE_HW_BAND,	//
	MODEM_STATE_HW_HSDPACAT,	//
	MODEM_STATE_HW_HSUPACAT,	//
	MODEM_STATE_HW_RRC,	//
	MODEM_STATE_HW_SGSN,	//
	MODEM_STATE_HW_MSC,	//
	MODEM_STATE_HW_PDPDUPLIC,	//
	MODEM_STATE_HW_TDSCSQ,	//

	MODEM_STATE_HW_CLCK,		//PIN UNLOCK
	MODEM_STATE_HW_UNLOCK,		//PIN UNLOCK
	/*
	 * Network Service
	 */
	MODEM_STATE_HW_COPS,		//COPS
	MODEM_STATE_HW_CREG,		//CREG
	MODEM_STATE_HW_CPOL,		//CPOL

	/*
	 * Call Related
	 */
	MODEM_STATE_HW_PDP,			//APN PDP
	/*
	 * Message
	 */
	MODEM_STATE_HW_CSMS,	//
	MODEM_STATE_HW_CMGF,	//
	MODEM_STATE_HW_CSCA,	//
	MODEM_STATE_HW_CPMS,	//

	/*
	 * Packet Domain
	 */
	MODEM_STATE_HW_CGATT,	//
	MODEM_STATE_HW_CGDCONT,		//NWCALL
	MODEM_STATE_HW_CGACT,	//


	MODEM_STATE_HW_MAX,

}modem_hw_state;









#endif /* __MODEM_ENUM_H__ */
