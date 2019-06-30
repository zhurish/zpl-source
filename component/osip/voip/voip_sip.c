/*
 * voip_sip.c
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
 */


#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"
#include "network.h"
#include "vty.h"

#include "os_util.h"
#include "os_socket.h"
#include "os_uci.h"
#include "uci_ubus.h"

#include "voip_def.h"
#include "voip_uci.h"
#include "voip_state.h"
#include "voip_stream.h"
#include "voip_sip.h"
#include "voip_osip.h"
#include "voip_app.h"
#include "application.h"

voip_sip_t *sip_config = NULL;

//static int voip_sip_ctl_state(struct vty *vty);


static voip_payload_t _voip_payload_table[] =
{
	{"0 PCMU/8000", 			20, "PCMU", 			0},
	{"8 PCMA/8000", 			-1, "PCMA", 			8},
	{"1 l016/8000", 			-1, "L016", 			1},
	{"7 lpc/8000", 				20, "LPC", 				7},
	{"18 G729/8000", 			-1, "G729", 			18},
	{"4 G7231/8000", 			-1, "G7231", 			4},
	{"18 G7221/8000", 			-1, "G7221", 			-1},
	{"18 G726-40/8000", 		-1, "G726-40", 			-1},
	{"18 G726-32/8000", 		-1, "G726-32", 			-1},
	{"18 G726-24/8000", 		-1, "G726-24", 			-1},
	{"18 G726-16/8000", 		-1, "G726-16", 			-1},
	{"18 AAL2-G726-40/8000", 	-1, "AAL2-G726-40", 	-1},
	{"18 AAL2-G726-32/8000", 	-1, "AAL2-G726-32", 	-1},
	{"18 AAL2-G726-24/8000", 	-1, "AAL2-G726-24", 	-1},
	{"18 AAL2-G726-16/8000", 	-1, "AAL2-G726-16", 	-1},

	{"112 speex/8000", 			-1, "SPEEX-NB", 		110},
	{"112 speex/16000", 		-1, "SPEEX-WB", 		111},
	{"112 iLBC/8000", 			30, "iLBC", 			112},
	{"115 l015/8000", 			-1, "L015", 			115},
	{"113 AMR/8000", 			-1, "AMR", 				113},
	{"9 G722/8000", 			-1, "G722", 			9},
	{"105 opus/8000", 			-1, "OPUS", 			105},
	{"114 custom/8000", 		-1, "CUSTOM", 			114},
};

int voip_sip_get_payload_index(void)
{
	zassert(sip_config != NULL);
	//zlog_debug(ZLOG_VOIP, "======:%s PAYLOAD=%d", __func__, sip_config->payload);
	return sip_config->payload;
}

int voip_sip_payload_index(char *name)
{
	int i = 0;
	char tmp[SIP_NUMBER_MAX];
	zassert(name != NULL);
	memset(tmp, 0, sizeof(tmp));
	strcpy(tmp, name);
	for(i = 0; i < array_size(_voip_payload_table); i++)
	{
		if(_voip_payload_table[i].payload >= 0)
		{
			if(strncasecmp(tmp, _voip_payload_table[i].name, SIP_NUMBER_MAX) == 0)
			{
				//zlog_debug(ZLOG_VOIP, "======:%s PAYLOAD=%d %s", __func__, _voip_payload_table[i].payload, tmp);
				return (int)_voip_payload_table[i].payload;
			}
		}
	}
	return -1;
}

char * voip_sip_payload_name(int index)
{
	int i = 0;
	for(i = 0; i < array_size(_voip_payload_table); i++)
	{
		if(_voip_payload_table[i].payload >= 0 && _voip_payload_table[i].payload == index)
		{
			//zlog_debug(ZLOG_VOIP, "======:%s PAYLOAD=%d %s", __func__, _voip_payload_table[i].payload,
			//		_voip_payload_table[i].name);
			return _voip_payload_table[i].name;
		}
	}
	return NULL;
}

char * voip_sip_payload_rtpmap(int index)
{
	int i = 0;
	for(i = 0; i < array_size(_voip_payload_table); i++)
	{
		if(_voip_payload_table[i].payload >= 0 && _voip_payload_table[i].payload == index)
		{
			//zlog_debug(ZLOG_VOIP, "======:%s PAYLOAD=%d %s rtpmap=%s", __func__, _voip_payload_table[i].payload,
			//		_voip_payload_table[i].name, _voip_payload_table[i].rtpmap);
			return _voip_payload_table[i].rtpmap;
		}
	}
	return NULL;
}

int voip_sip_payload_ptime(int index)
{
	int i = 0;
	for(i = 0; i < array_size(_voip_payload_table); i++)
	{
		if(_voip_payload_table[i].payload >= 0 && _voip_payload_table[i].payload == index)
		{
			//zlog_debug(ZLOG_VOIP, "======:%s PAYLOAD=%d %s ptime=%d", __func__, _voip_payload_table[i].payload,
			//		_voip_payload_table[i].name, _voip_payload_table[i].ptime);
			return (int)_voip_payload_table[i].ptime;
		}
	}
	return -1;
}

static int voip_sip_config_default(voip_sip_t *sip)
{
	zassert(sip != NULL);
	sip->sip_enable		= SIP_ENABLE_DEFAULT;

	sip->sip_active_standby = FALSE;
	sip->sip_keepalive = FALSE;				//
	sip->sip_keepalive_interval = 5;
	sip->sip_proxy_enable = FALSE;
	sip->sip_multi_user = FALSE;
	//sip->sip_server;					//SIP��������ַ
	//sip->sip_server_sec;				//SIP���÷�������ַ
	//sip->sip_proxy_server;			//�����������ַ
	//sip->sip_proxy_server_sec;		//��ѡ�����������ַ
	sip->sip_server = ntohl(inet_addr("192.168.2.252"));
	//sip->sip_local_address = ntohl(inet_addr("192.168.2.100"));
	sip->sip_source_interface = if_ifindex_make("ethernet 0/0/2", NULL);

	sip->sip_port		= SIP_PORT_DEFAULT;					//SIP�������˿ں�
	sip->sip_port_sec	= SIP_PORT_SEC_DEFAULT;				//SIP���÷������˿ں�
	sip->sip_proxy_port	= SIP_PROXY_PORT_DEFAULT;				//����������˿ں�
	sip->sip_proxy_port_sec = SIP_PROXY_PORT_SEC_DEFAULT;			//��ѡ����������˿ں�

	sip->sip_local_port = SIP_PORT_DEFAULT;				//���ñ���SIP�������˿ں�


	sip->bUserMain		= TRUE;			//当前是否使用主号码
	sip->bSipMain		= TRUE;			//当前是否使用主服务器
	sip->bRegMain		= FALSE;
	sip->bRegStanby		= FALSE;
	sip->sip_time_sync = SIP_TIME_DEFAULT;				//ʱ���Ƿ�ͬ��

	sip->sip_ring	= SIP_RING_DEFAULT;					//��������

	sip->sip_register_interval = SIP_REGINTER_DEFAULT;		//����ע������

	strcpy(sip->sip_hostpart, SIP_HOSTPART_DEFAULT);				//����hostpart
	sip->sip_interval = SIP_INTERVAL_DEFAULT;				//��������
	sip->sip_100_rel = SIP_100_REL_DEFAULT;				//����100rel�Ƿ�ǿ��ʹ��
	sip->sip_dis_name = SIP_DIS_NAME_DEFAULT;				//����display name
	//sip->sip_local_number[SIP_NUMBER_MAX];	//���ñ��غ���
	//sip->sip_user[SIP_DATA_MAX];
	//sip->sip_password[SIP_DATA_MAX];
	strcpy(sip->sip_local_number, SIP_PHONE_DEFAULT);
	strcpy(sip->sip_user, SIP_USERNAME_DEFAULT);
	strcpy(sip->sip_password, SIP_PASSWORD_DEFAULT);
	//strcpy(sip->sip_realm, SIP_HOSTPART_DEFAULT);					//����realm
	strcpy(sip->sip_dialplan, SIP_DIALPLAN_DEFAULT);				//����dialplan
	sip->sip_encrypt = SIP_ENCRYPT_DEFAULT;				//����ע������
	sip->dtmf = SIP_DTMF_DEFAULT;
	sip->proto = SIP_PROTO_UDP;
	os_memset(sip->payload_name, 0, sizeof(sip->payload_name));
	strcpy(sip->payload_name, "pcmu");
	sip->payload = 0;

	//sip->debug			= 0;

	return OK;
}

int voip_sip_module_init()
{
/*	if(sip_config == NULL)
		sip_config = malloc(sizeof(voip_sip_t));*/
	sip_config = voip_app->voip_sip;
	if(!sip_config)
		return ERROR;
	os_memset(sip_config, 0, sizeof(voip_sip_t));
	if(master_eloop[MODULE_VOIP] == NULL)
		master_eloop[MODULE_VOIP] = eloop_master_module_create(MODULE_VOIP);

	sip_config->mutex = os_mutex_init();
	voip_sip_config_default(sip_config);

#ifdef PL_OPENWRT_UCI
//	uci_ubus_cb_install(voip_uci_sip_update_cb);
#endif
	voip_sip_config_load(sip_config);

	voip_osip_module_init(sip_config);

	return OK;
}

int voip_sip_module_exit()
{
	zassert(sip_config != NULL);
	voip_osip_module_exit();

	if(sip_config->mutex)
		os_mutex_exit(sip_config->mutex);
	os_memset(sip_config, 0, sizeof(voip_sip_t));
	voip_sip_config_default(sip_config);
	return OK;
}

int voip_sip_module_task_init()
{
	zassert(sip_config != NULL);

	voip_osip_module_task_init(sip_config->osip);

	return OK;
}

int voip_sip_module_task_exit()
{
	zassert(sip_config != NULL);

	voip_osip_module_task_exit(sip_config->osip);

	return OK;
}

int voip_sip_dtmf_cb_api(int (*cb)(int, int))
{
	zassert(sip_config != NULL);
	sip_config->app_dtmf_cb = cb;
	return OK;
}

BOOL voip_sip_main_regstate(void)
{
	BOOL regmain = FALSE;
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	regmain = sip_config->bRegMain;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return regmain;
}

BOOL voip_sip_stanby_regstate(void)
{
	BOOL regmain = FALSE;
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	regmain = sip_config->bRegMain;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return regmain;
}

BOOL voip_sip_is_usemain(void)
{
	BOOL regmain = FALSE;
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	regmain = sip_config->bUserMain;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return regmain;
}
BOOL voip_sip_is_sipmain(void)
{
	BOOL regmain = FALSE;
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	regmain = sip_config->bSipMain;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return regmain;
}


int voip_sip_enable(BOOL enable)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_enable = enable;
	voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

BOOL voip_sip_isenable(void)
{
	zassert(sip_config != NULL);
	return sip_config->sip_enable;
}


int voip_sip_server_set_api(u_int32 ip, u_int16 port, BOOL sec)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(sec)
	{
		sip_config->sip_server_sec = ip;
		sip_config->sip_port_sec = port ? port:SIP_PORT_SEC_DEFAULT;
		voip_sip_config_update_api(sip_config);
	}
	else
	{
		sip_config->sip_server = ip;
		sip_config->sip_port = port ? port:SIP_PORT_DEFAULT;
		voip_sip_config_update_api(sip_config);
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_server_get_api(u_int32 *ip, u_int16 *port, BOOL sec)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(sec)
	{
		if(ip)
			*ip = sip_config->sip_server_sec;
		if(port)
			*port = sip_config->sip_port_sec;
	}
	else
	{
		if(ip)
			*ip = sip_config->sip_server;
		if(port)
			*port = sip_config->sip_port;
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_proxy_server_set_api(u_int32 ip, u_int16 port, BOOL sec)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(sec)
	{
		sip_config->sip_proxy_server_sec = ip;
		sip_config->sip_proxy_port_sec = port ? port:SIP_PROXY_PORT_SEC_DEFAULT;
	}
	else
	{
		sip_config->sip_proxy_server = ip;
		sip_config->sip_proxy_port = port ? port:SIP_PROXY_PORT_DEFAULT;
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_proxy_server_get_api(u_int32 *ip, u_int16 *port, BOOL sec)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(sec)
	{
		if(ip)
			*ip = sip_config->sip_proxy_server_sec;
		if(port)
			*port = sip_config->sip_proxy_port_sec;
	}
	else
	{
		if(ip)
			*ip = sip_config->sip_proxy_server;
		if(port)
			*port = sip_config->sip_proxy_port;
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}


int voip_sip_local_address_set_api(u_int32 address)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_local_address = address;
	voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_local_address_get_api(u_int32 *address)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(address)
		*address = sip_config->sip_local_address;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_source_interface_set_api(u_int32 ifindex)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_source_interface = ifindex;
	voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_source_interface_get_api(u_int32 *ifindex)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(ifindex)
		*ifindex = sip_config->sip_source_interface;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_local_port_set_api(u_int16 port)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_local_port = port;
	voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}
int voip_sip_local_port_get_api(u_int16 *port)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(port)
		*port = sip_config->sip_local_port;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}


int voip_sip_transport_proto_set_api(sip_transport_t proto)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->proto = proto;
	voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_transport_proto_get_api(sip_transport_t *proto)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(proto)
		*proto = sip_config->proto;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}


int voip_sip_multiuser_set_api(BOOL enable)
{
	zassert(sip_config != NULL);
	if(sip_config->sip_multi_user == enable)
		return OK;
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_multi_user = enable;
	//voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

BOOL voip_sip_multiuser_get_api()
{
	zassert(sip_config != NULL);
	return sip_config->sip_multi_user;
}

int voip_sip_active_standby_set_api(BOOL enable)
{
	zassert(sip_config != NULL);
	if(sip_config->sip_active_standby == enable)
		return OK;
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_active_standby = enable;
	//voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

BOOL voip_sip_active_standby_get_api()
{
	zassert(sip_config != NULL);
	return sip_config->sip_active_standby;
}

int voip_sip_proxy_enable_set_api(BOOL enable)
{
	zassert(sip_config != NULL);
	if(sip_config->sip_proxy_enable == enable)
		return OK;
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_proxy_enable = enable;
	//voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

BOOL voip_sip_proxy_enable_get_api()
{
	zassert(sip_config != NULL);
	return sip_config->sip_proxy_enable;
}

int voip_sip_keepalive_set_api(BOOL enable)
{
	zassert(sip_config != NULL);
	if(sip_config->sip_keepalive == enable)
		return OK;
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_keepalive = enable;
	//voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

BOOL voip_sip_keepalive_get_api()
{
	zassert(sip_config != NULL);
	return sip_config->sip_keepalive;
}

int voip_sip_keepalive_interval_set_api(u_int16 value)
{
	zassert(sip_config != NULL);
	if(sip_config->sip_keepalive_interval == value)
		return OK;
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_keepalive_interval = value ? value:5;
	//voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_keepalive_interval_get_api()
{
	zassert(sip_config != NULL);
	return sip_config->sip_keepalive_interval;
}

int voip_sip_local_number_set_api(char * value, BOOL secondary)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(secondary)
	{
		if(value)
		{
			memset(sip_config->sip_local_number_alias, 0, sizeof(sip_config->sip_local_number_alias));
			strcpy(sip_config->sip_local_number_alias, value);
		}
		else
			memset(sip_config->sip_local_number_alias, 0, sizeof(sip_config->sip_local_number_alias));
		voip_sip_config_update_api(sip_config);
	}
	else
	{
		if(value)
		{
			memset(sip_config->sip_local_number, 0, sizeof(sip_config->sip_local_number));
			strcpy(sip_config->sip_local_number, value);
		}
		else
			memset(sip_config->sip_local_number, 0, sizeof(sip_config->sip_local_number));
		voip_sip_config_update_api(sip_config);
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_local_number_get_api(char * value, BOOL secondary)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(secondary)
	{
		if(value && strlen(sip_config->sip_local_number_alias))
			strcpy(value, sip_config->sip_local_number_alias);
	}
	else
	{
		if(value && strlen(sip_config->sip_local_number))
			strcpy(value, sip_config->sip_local_number);
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_password_set_api(char * value, BOOL secondary)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(secondary)
	{
		if(value)
		{
			memset(sip_config->sip_password_alias, 0, sizeof(sip_config->sip_password_alias));
			strcpy(sip_config->sip_password_alias, value);
		}
		else
			memset(sip_config->sip_password_alias, 0, sizeof(sip_config->sip_password_alias));
		voip_sip_config_update_api(sip_config);
	}
	else
	{
		if(value)
		{
			memset(sip_config->sip_password, 0, sizeof(sip_config->sip_password));
			strcpy(sip_config->sip_password, value);
		}
		else
			memset(sip_config->sip_password, 0, sizeof(sip_config->sip_password));
		voip_sip_config_update_api(sip_config);
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_password_get_api(char * value, BOOL secondary)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(secondary)
	{
		if(value && strlen(sip_config->sip_password_alias))
			strcpy(value, sip_config->sip_password_alias);
	}
	else
	{
		if(value && strlen(sip_config->sip_password))
			strcpy(value, sip_config->sip_password);
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_user_set_api(char * value, BOOL secondary)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(secondary)
	{
		if(value)
		{
			memset(sip_config->sip_user_alias, 0, sizeof(sip_config->sip_user_alias));
			strcpy(sip_config->sip_user_alias, value);
		}
		else
			memset(sip_config->sip_user_alias, 0, sizeof(sip_config->sip_user_alias));
		voip_sip_config_update_api(sip_config);
	}
	else
	{
		if(value)
		{
			memset(sip_config->sip_user, 0, sizeof(sip_config->sip_user));
			strcpy(sip_config->sip_user, value);
		}
		else
			memset(sip_config->sip_user, 0, sizeof(sip_config->sip_user));
		voip_sip_config_update_api(sip_config);
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_user_get_api(char * value, BOOL secondary)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(secondary)
	{
		if(value && strlen(sip_config->sip_user_alias))
			strcpy(value, sip_config->sip_user_alias);
	}
	else
	{
		if(value && strlen(sip_config->sip_user))
			strcpy(value, sip_config->sip_user);
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}


int voip_sip_payload_name_set_api(char * value)
{
	zassert(value != NULL);
	zassert(sip_config != NULL);
	if(voip_sip_payload_index(value) < 0)
		return ERROR;
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	memset(sip_config->payload_name, 0, sizeof(sip_config->payload_name));
	if(value)
	{
		sip_config->payload = voip_sip_payload_index(value);
		strcpy(sip_config->payload_name, value);
	}
	voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_dtmf_set_api(sip_dtmf_t dtmf)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->dtmf = dtmf;
	voip_sip_config_update_api(sip_config);
	switch(dtmf)
	{
	case VOIP_SIP_INFO:
		voip_sip_dtmf_cb_api(voip_app_dtmf_recv_callback);
		voip_stream_dtmf_cb_api(NULL);
		break;
	case VOIP_SIP_RFC2833:
		voip_sip_dtmf_cb_api(NULL);
		voip_stream_dtmf_cb_api(voip_app_dtmf_recv_callback);
		break;
	case VOIP_SIP_INBAND:
		voip_sip_dtmf_cb_api(voip_app_dtmf_recv_callback);
		voip_stream_dtmf_cb_api(voip_app_dtmf_recv_callback);
		break;
	default:
		voip_sip_dtmf_cb_api(NULL);
		voip_stream_dtmf_cb_api(NULL);
		break;
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}
int voip_sip_dtmf_get_api(sip_dtmf_t *dtmf)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(dtmf)
		*dtmf = sip_config->dtmf;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_realm_set_api(char * value, BOOL secondary)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(secondary)
	{
		if(value)
		{
			memset(sip_config->sip_realm_alias, 0, sizeof(sip_config->sip_realm_alias));
			strcpy(sip_config->sip_realm_alias, value);
		}
		else
			memset(sip_config->sip_realm_alias, 0, sizeof(sip_config->sip_realm_alias));
		voip_sip_config_update_api(sip_config);
	}
	else
	{
		if(value)
		{
			memset(sip_config->sip_realm, 0, sizeof(sip_config->sip_realm));
			strcpy(sip_config->sip_realm, value);
		}
		else
			memset(sip_config->sip_realm, 0, sizeof(sip_config->sip_realm));
		voip_sip_config_update_api(sip_config);
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_realm_get_api(char * value, BOOL secondary)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(secondary)
	{
		if(value && strlen(sip_config->sip_realm_alias))
			strcpy(value, sip_config->sip_realm_alias);
	}
	else
	{
		if(value && strlen(sip_config->sip_realm))
			strcpy(value, sip_config->sip_realm);
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_100_rel_set_api(BOOL value)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_100_rel = value;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

BOOL voip_sip_100_rel_get_api(void)
{
	zassert(sip_config != NULL);
	return sip_config->sip_100_rel;
}

int voip_sip_register_interval_set_api(u_int16 value)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_register_interval = value ? value:SIP_REGINTER_DEFAULT;
	voip_sip_config_update_api(sip_config);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

u_int16 voip_sip_register_interval_get_api(u_int16 value)
{
	zassert(sip_config != NULL);
	return sip_config->sip_register_interval;
}

int voip_sip_interval_set_api(u_int16 value)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_interval = value ? value:SIP_INTERVAL_DEFAULT;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

u_int16 voip_sip_interval_get_api(void)
{
	zassert(sip_config != NULL);
	return sip_config->sip_interval;
}








int voip_sip_time_syne_set_api(BOOL enable)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_time_sync = enable;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

BOOL voip_sip_time_syne_get_api(void)
{
	zassert(sip_config != NULL);
	return sip_config->sip_time_sync;
}

int voip_sip_ring_set_api(u_int16 value)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_ring = value ? value:SIP_RING_DEFAULT;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_hostpart_set_api(char * value)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	memset(sip_config->sip_hostpart, 0, sizeof(sip_config->sip_hostpart));
	if(value)
		strcpy(sip_config->sip_hostpart, value);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_hostpart_get_api(char * value)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(value)
		strcpy(value, sip_config->sip_hostpart);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_display_name_set_api(BOOL value)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_dis_name = value;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

BOOL voip_sip_display_name_get_api(void)
{
	zassert(sip_config != NULL);
	return sip_config->sip_dis_name;
}


int voip_sip_dialplan_set_api(char  *value)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	memset(sip_config->sip_dialplan, 0, sizeof(sip_config->sip_dialplan));
	if(value)
		strcpy(sip_config->sip_dialplan, value);
	//sip_config->sip_dialplan = value;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}
int voip_sip_dialplan_get_api(char * value)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(value)
		strcpy(value, sip_config->sip_dialplan);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_encrypt_set_api(BOOL value)
{
	zassert(sip_config != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	sip_config->sip_encrypt = value;
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

/*int voip_sip_dialplan_set_api(char * value)
{

}
int voip_sip_dialplan_get_api(char * value)
{

}*/


BOOL voip_sip_encrypt_get_api(void)
{
	zassert(sip_config != NULL);
	return sip_config->sip_encrypt;
}

int voip_sip_config_load(voip_sip_t *sip)
{
#ifdef PL_OPENWRT_UCI
	return voip_uci_sip_config_load(sip);
#else
	return OK;
#endif
}

static int voip_sip_config_save(voip_sip_t *sip)
{
#ifdef PL_OPENWRT_UCI
	return voip_uci_sip_config_save(sip);
#else
	return OK;
#endif
}



static int voip_sip_config_update_thread(voip_event_t *event)
{
	zassert(event != NULL);
	//voip_sip_t *sip = event->pVoid;
	voip_sip_t *sip = VOIP_EVENT_ARGV(event);
	zassert(sip != NULL);
	sip->t_event = NULL;
/*	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);*/
	voip_osip_restart();
/*	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);*/
	if(voip_sip_config_save(sip) == OK)
		;//vty_execute_shell("write memory");
	return OK;
	//return ERROR;
}

int voip_sip_config_update_api(voip_sip_t *sip)
{
	zassert(sip != NULL);
	{
		if(sip->t_event != NULL)
			voip_event_cancel(sip->t_event);
		sip->t_event = voip_event_timer_add(voip_sip_config_update_thread, sip, NULL, 0, 5);
	}
	return OK;
}


static int _voip_sip_state(struct vty *vty)
{
	if(!sip_config)
		return ERROR;
	zassert(sip_config != NULL);
	switch(osip_register_state_get(sip_config->osip))
	{
	case SIP_STATE_UNREGISTER:
		vty_out(vty, " sip register state   : %s%s", "unregister", VTY_NEWLINE);
		break;
	case SIP_STATE_REGISTER_FAILED:
		vty_out(vty, " sip register state   : %s%s", "failed", VTY_NEWLINE);
		break;
	case SIP_STATE_REGISTER_SUCCESS:
		vty_out(vty, " sip register state   : %s%s", "success", VTY_NEWLINE);
		break;
	default:
		break;
	}

	switch(voip_app_state_get(voip_app))
	{
	case APP_STATE_TALK_IDLE:
		vty_out(vty, " sip call state       : %s%s", "IDLE", VTY_NEWLINE);
		break;
	case APP_STATE_TALK_FAILED:
		vty_out(vty, " sip call state       : %s%s", "Failed", VTY_NEWLINE);
		break;
	case APP_STATE_TALK_SUCCESS:
		vty_out(vty, " sip call state       : %s%s", "Ringing", VTY_NEWLINE);
		break;
	case APP_STATE_TALK_RUNNING:
		vty_out(vty, " sip call state       : %s%s", "Talking", VTY_NEWLINE);
		break;
	default:
		break;
	}
/*	switch(sip_config->call_error)
	{
	case VOIP_SIP_UNREGISTER:
		vty_out(vty, " sip error state      : %s%s", "unregister", VTY_NEWLINE);
		break;
	case VOIP_SIP_REGISTER_FAILED:
		vty_out(vty, " sip error state      : %s%s", "failed", VTY_NEWLINE);
		break;
	case VOIP_SIP_REGISTER_SUCCESS:
		vty_out(vty, " sip error state      : %s%s", "success", VTY_NEWLINE);
		break;
	default:
		break;
	}*/
	return OK;
}


int voip_sip_write_config(struct vty *vty)
{
	voip_sip_t *sip = sip_config;
	zassert(sip_config != NULL);
	zassert(vty != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(sip->sip_enable)
	{
		vty_out(vty, "service sip%s", VTY_NEWLINE);

		if(sip->sip_multi_user)
			vty_out(vty, " ip sip multiuser enable%s", VTY_NEWLINE);

		if(sip->sip_active_standby)
		{
			vty_out(vty, " ip sip active-standby enable%s", VTY_NEWLINE);
			if(sip->sip_keepalive)
			{
				vty_out(vty, " ip sip keepalive enable%s", VTY_NEWLINE);
				vty_out(vty, " ip sip keepalive-interval %d%s", sip->sip_keepalive_interval, VTY_NEWLINE);
			}
		}
		if(sip->sip_proxy_enable)
			vty_out(vty, " ip sip proxy enable%s", VTY_NEWLINE);

		if(strlen(sip->sip_local_number))
			vty_out(vty, " ip sip local-phone %s%s", sip->sip_local_number, VTY_NEWLINE);

		if(strlen(sip->sip_user))
			vty_out(vty, " ip sip username %s%s", sip->sip_user, VTY_NEWLINE);

		if(strlen(sip->sip_password))
			vty_out(vty, " ip sip password %s%s", sip->sip_password, VTY_NEWLINE);

		if(sip->sip_multi_user)
		{
			if(strlen(sip->sip_local_number_alias))
				vty_out(vty, " ip sip local-phone %s secondary%s", sip->sip_local_number_alias, VTY_NEWLINE);

			if(strlen(sip->sip_user_alias))
				vty_out(vty, " ip sip username %s secondary%s", sip->sip_user_alias, VTY_NEWLINE);

			if(strlen(sip->sip_password_alias))
				vty_out(vty, " ip sip password %s secondary%s", sip->sip_password_alias, VTY_NEWLINE);
		}

		if(sip->sip_local_port != SIP_PORT_DEFAULT)
			vty_out(vty, " ip sip local-port %d%s", (sip->sip_local_port), VTY_NEWLINE);

		if(sip->sip_local_address != 0)
			vty_out(vty, " ip sip local-address %s%s", inet_address(sip->sip_local_address), VTY_NEWLINE);

		if(sip->sip_source_interface != 0)
			vty_out(vty, " ip sip source-interface %s%s", ifindex2ifname(sip->sip_source_interface), VTY_NEWLINE);


		if(sip->sip_server)
			vty_out(vty, " ip sip server %s%s", inet_address(sip->sip_server), VTY_NEWLINE);
		if(sip->sip_port != SIP_PORT_DEFAULT)
			vty_out(vty, " ip sip server port %d%s", (sip->sip_port), VTY_NEWLINE);

		if(sip->sip_active_standby)
		{
			if(sip->sip_server_sec)
				vty_out(vty, " ip sip server %s secondary%s", inet_address(sip->sip_server_sec), VTY_NEWLINE);

			if(sip->sip_port_sec != SIP_PORT_SEC_DEFAULT)
				vty_out(vty, " ip sip server port %d secondary%s", (sip->sip_port_sec), VTY_NEWLINE);
		}

		if(sip->sip_proxy_enable)
		{
			if(sip->sip_proxy_server)
				vty_out(vty, " ip sip proxy-server %s%s", inet_address(sip->sip_proxy_server), VTY_NEWLINE);
			if(sip->sip_proxy_server_sec)
				vty_out(vty, " ip sip proxy-server %s secondary%s", inet_address(sip->sip_proxy_server_sec), VTY_NEWLINE);

			if(sip->sip_proxy_port != SIP_PROXY_PORT_DEFAULT)
				vty_out(vty, " ip sip proxy-server port %d%s", (sip->sip_proxy_port), VTY_NEWLINE);
			if(sip->sip_proxy_port_sec != SIP_PROXY_PORT_SEC_DEFAULT)
				vty_out(vty, " ip sip proxy-server port %d secondary%s", (sip->sip_proxy_port_sec), VTY_NEWLINE);
		}

		if(sip->proto == SIP_PROTO_UDP)
			vty_out(vty, " ip sip transport udp%s", VTY_NEWLINE);
		else if(sip->proto == SIP_PROTO_TCP)
			vty_out(vty, " ip sip transport tcp%s", VTY_NEWLINE);
		else if(sip->proto == SIP_PROTO_TLS)
			vty_out(vty, " ip sip transport tls%s", VTY_NEWLINE);
		else if(sip->proto == SIP_PROTO_DTLS)
			vty_out(vty, " ip sip transport dtls%s", VTY_NEWLINE);

		if(strlen(sip->payload_name))
			vty_out(vty, " ip sip payload %s%s", strlwr(sip->payload_name), VTY_NEWLINE);


		if(sip->dtmf == VOIP_SIP_INFO)
			vty_out(vty, " ip sip dtmf-type sip-info%s", VTY_NEWLINE);
		else if(sip->dtmf == VOIP_SIP_RFC2833)
			vty_out(vty, " ip sip dtmf-type rfc2833%s", VTY_NEWLINE);
		else if(sip->dtmf == VOIP_SIP_INBAND)
			vty_out(vty, " ip sip dtmf-type inband%s", VTY_NEWLINE);
/*
		if(sip->sip_time_sync != SIP_TIME_DEFAULT)
			vty_out(vty, " ip sip time-sync%s", VTY_NEWLINE);

		if(sip->sip_ring != SIP_RING_DEFAULT)
			vty_out(vty, " ip sip ring %d%s", sip->sip_ring, VTY_NEWLINE);
*/

		if(sip->sip_register_interval != SIP_REGINTER_DEFAULT)
			vty_out(vty, " ip sip register-interval %d%s", sip->sip_register_interval, VTY_NEWLINE);

		//if(sip->sip_hostpart != SIP_HOSTPART_DEFAULT)
/*
		if(strlen(sip->sip_hostpart))
			vty_out(vty, " ip sip hostpart %s%s", sip->sip_hostpart, VTY_NEWLINE);

		if(sip->sip_interval != SIP_INTERVAL_DEFAULT)
			vty_out(vty, " ip sip keep-interval %d%s", sip->sip_interval, VTY_NEWLINE);
*/


		if(sip->sip_100_rel != SIP_100_REL_DEFAULT)
			vty_out(vty, " ip sip rel-100%s", VTY_NEWLINE);

/*
		if(sip->sip_dis_name != SIP_DIS_NAME_DEFAULT)
			vty_out(vty, " ip sip display-name %s", VTY_NEWLINE);

		if(sip->sip_encrypt != SIP_ENCRYPT_DEFAULT)
			vty_out(vty, " ip sip encrypt %s", VTY_NEWLINE);
*/

		//if(sip->sip_dialplan != SIP_DIALPLAN_DEFAULT)
/*
		if(strlen(sip->sip_dialplan))
			vty_out(vty, " ip sip dialplan %s%s", sip->sip_dialplan, VTY_NEWLINE);
*/

		if(strlen(sip->sip_realm))
			vty_out(vty, " ip sip realm %s%s", sip->sip_realm, VTY_NEWLINE);

		if(strlen(sip->sip_realm_alias))
			vty_out(vty, " ip sip realm %s secondary%s", sip->sip_realm_alias, VTY_NEWLINE);

		vty_out(vty, "!%s",VTY_NEWLINE);
	}
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}

int voip_sip_show_config(struct vty *vty, BOOL detail)
{
	voip_sip_t *sip = sip_config;
	zassert(sip_config != NULL);
	zassert(vty != NULL);
	if(sip_config->mutex)
		os_mutex_lock(sip_config->mutex, OS_WAIT_FOREVER);
	if(sip->sip_enable)
	{
		vty_out(vty, "SIP Service :%s", VTY_NEWLINE);
		vty_out(vty, " sip multiuser        : %s%s", sip->sip_multi_user ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " sip active-standby   : %s%s", sip->sip_active_standby ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " sip proxy            : %s%s", sip->sip_proxy_enable ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " sip keepalive        : %s%s", sip->sip_keepalive ? "TRUE":"FALSE", VTY_NEWLINE);
		if(sip->sip_keepalive)
			vty_out(vty, " sip keep interval    : %d%s", sip->sip_keepalive_interval, VTY_NEWLINE);

		vty_out(vty, " sip local-phone      : %s%s",
				strlen(sip->sip_local_number)? sip->sip_local_number:" ", VTY_NEWLINE);

		vty_out(vty, " sip username         : %s%s",
				strlen(sip->sip_user)? sip->sip_user:" ", VTY_NEWLINE);

		vty_out(vty, " sip password         : %s%s",
				strlen(sip->sip_password)? sip->sip_password:" ", VTY_NEWLINE);

		vty_out(vty, " sip local-phone      : %s secondary%s",
				strlen(sip->sip_local_number_alias)? sip->sip_local_number_alias:" ", VTY_NEWLINE);

		vty_out(vty, " sip username         : %s secondary%s",
				strlen(sip->sip_user_alias)? sip->sip_user_alias:" ", VTY_NEWLINE);

		vty_out(vty, " sip password         : %s secondary%s",
				strlen(sip->sip_password_alias)? sip->sip_password_alias:" ", VTY_NEWLINE);

		vty_out(vty, " sip local-port       : %d%s", (sip->sip_local_port), VTY_NEWLINE);
		vty_out(vty, " sip local-address    : %s%s", inet_address(sip->sip_local_address), VTY_NEWLINE);
		vty_out(vty, " sip source-interface : %s%s", if_ifname_make(sip->sip_source_interface), VTY_NEWLINE);
		vty_out(vty, " sip server           : %s%s", inet_address(sip->sip_server), VTY_NEWLINE);
		vty_out(vty, " sip server           : %s secondary%s", inet_address(sip->sip_server_sec), VTY_NEWLINE);
		vty_out(vty, " sip proxy-server     : %s%s", inet_address(sip->sip_proxy_server), VTY_NEWLINE);
		vty_out(vty, " sip proxy-server     : %s secondary%s", inet_address(sip->sip_proxy_server_sec), VTY_NEWLINE);
		vty_out(vty, " sip server port      : %d%s", (sip->sip_port), VTY_NEWLINE);
		vty_out(vty, " sip server port      : %d secondary%s", (sip->sip_port_sec), VTY_NEWLINE);

		vty_out(vty, " sip proxy-server port: %d%s", (sip->sip_proxy_port), VTY_NEWLINE);
		vty_out(vty, " sip proxy-server port: %d secondary%s", (sip->sip_proxy_port_sec), VTY_NEWLINE);

		if(sip->proto == SIP_PROTO_UDP)
			vty_out(vty, " sip transport        : udp%s", VTY_NEWLINE);
		else if(sip->proto == SIP_PROTO_TCP)
			vty_out(vty, " sip transport        : tcp%s", VTY_NEWLINE);
		else if(sip->proto == SIP_PROTO_TLS)
			vty_out(vty, " sip transport        : tls%s", VTY_NEWLINE);
		else if(sip->proto == SIP_PROTO_DTLS)
			vty_out(vty, " sip transport        : dtls%s", VTY_NEWLINE);

		if(strlen(sip->payload_name))
			vty_out(vty, " sip payload          : %d %s%s", sip->payload, sip->payload_name, VTY_NEWLINE);

		if(sip->dtmf == VOIP_SIP_INFO)
			vty_out(vty, " sip dtmf-type        : sip-info%s", VTY_NEWLINE);
		else if(sip->dtmf == VOIP_SIP_RFC2833)
			vty_out(vty, " sip dtmf-type        : rfc2833%s", VTY_NEWLINE);
		else if(sip->dtmf == VOIP_SIP_INBAND)
			vty_out(vty, " sip dtmf-type        : inband%s", VTY_NEWLINE);

		//vty_out(vty, " sip time-sync        : %s%s", sip->sip_time_sync ? "TRUE":"FALSE",VTY_NEWLINE);
		//vty_out(vty, " sip ring             : %d%s", sip->sip_ring ,VTY_NEWLINE);

		vty_out(vty, " sip register-interval: %d%s", sip->sip_register_interval, VTY_NEWLINE);
		//vty_out(vty, " sip hostpart         : %s%s", strlen(sip->sip_hostpart)? sip->sip_hostpart:" ",VTY_NEWLINE);
		//vty_out(vty, " sip keep-interval    : %d%s", sip->sip_interval, VTY_NEWLINE);
		vty_out(vty, " sip rel-100          : %s%s", sip->sip_100_rel ? "TRUE":"FALSE", VTY_NEWLINE);
		//vty_out(vty, " sip display-name     : %s%s", sip->sip_dis_name ? "TRUE":"FALSE", VTY_NEWLINE);
		//vty_out(vty, " sip encrypt          : %s%s", sip->sip_encrypt ? "TRUE":"FALSE", VTY_NEWLINE);
		//vty_out(vty, " sip dialplan         : %s%s", strlen(sip->sip_dialplan)? sip->sip_dialplan:" ", VTY_NEWLINE);
		vty_out(vty, " sip realm            : %s%s", strlen(sip->sip_realm)? sip->sip_realm:" ", VTY_NEWLINE);
		vty_out(vty, " sip realm            : %s secondary%s", strlen(sip->sip_realm_alias)? sip->sip_realm_alias:" ", VTY_NEWLINE);

		_voip_sip_state(vty);
	}
	else
		vty_out(vty, "SIP Service is not enable%s", VTY_NEWLINE);
	if(sip_config->mutex)
		os_mutex_unlock(sip_config->mutex);
	return OK;
}


/*
100试呼叫（Trying）
180振铃（Ringing）
181呼叫正在前转（Call is Being Forwarded）
200成功响应（OK）
302临时迁移（Moved Temporarily）
400错误请求（Bad Request）
401未授权（Unauthorized）
403禁止（Forbidden）
404用户不存在（Not Found）
408请求超时（Request Timeout）
480暂时无人接听（Temporarily Unavailable）
486线路忙（Busy Here）
504服务器超时（Server Time-out）
600全忙（Busy Everywhere）
 */
/*
 * event socket (SIP)
 */

















/*
 * SIP state Module
 */
/*
voip_state_t voip_sip_state_get_api()
{
	return VOIP_STATE_CALL_SUCCESS;
}
*/






