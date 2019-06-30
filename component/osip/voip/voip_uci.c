/*
 * voip_uci.c
 *
 *  Created on: 2019年3月12日
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
#include "voip_util.h"
#include "voip_app.h"
#include "voip_sip.h"
#include "voip_event.h"
#include "voip_stream.h"
#include "voip_uci.h"


BOOL voip_global_enabled()
{
	int enable = 0;
#ifdef PL_OPENWRT_UCI
	os_uci_get_integer("product.global.voip", &enable);
#endif
	return (BOOL)enable;
}


int voip_status_register_api(BOOL reg)
{
#ifdef PL_OPENWRT_UCI
	os_uci_set_integer("voipconfig.status.regstate", reg);
	os_uci_save_config("voipconfig");
#endif
	return OK;
}

int voip_status_register_main_api(BOOL reg)
{
#ifdef PL_OPENWRT_UCI
	if(voip_sip_multiuser_get_api())
		os_uci_set_integer("voipconfig.status.regmain", reg);
	else
		os_uci_set_integer("voipconfig.status.regmain", 1);
	os_uci_save_config("voipconfig");
#endif
	return OK;
}

int voip_status_talk_api(BOOL reg)
{
#ifdef PL_OPENWRT_UCI
	os_uci_set_integer("voipconfig.status.voiptalk", reg);
	os_uci_save_config("voipconfig");
#endif
	return OK;
}

int voip_status_enable_api(BOOL reg)
{
#ifdef PL_OPENWRT_UCI
	os_uci_set_integer("voipconfig.status.enable", reg);
	os_uci_save_config("voipconfig");
#endif
	return OK;
}

int voip_status_clear_api()
{
#ifdef PL_OPENWRT_UCI
	os_uci_set_integer("voipconfig.testing.callstate", 0);
	//os_uci_save_config("voipconfig");
	if(voip_sip_multiuser_get_api())
		os_uci_set_integer("voipconfig.status.regmain", 0);
	os_uci_set_integer("voipconfig.status.voiptalk", 0);
#endif
	voip_status_enable_api(TRUE);
	voip_status_register_api(FALSE);
	return OK;
}

#ifdef PL_OPENWRT_UCI

static int voip_uci_sip_config_load_address(voip_sip_t *sip)
{
	char tmp[128];
	int	 value = 0, ret = ERROR;
	zassert(sip != NULL);
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("voipconfig.sip.sip_source_interface", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_source_interface = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
	{
		if(strncasecmp(tmp, "wan", 3) == 0)
		{
			sip->sip_local_address = 0;
			sip->sip_source_interface = if_ifindex_make("ethernet 0/0/2", NULL);
		}
		else if(strncasecmp(tmp, "lan", 3) == 0)
		{
			sip->sip_local_address = 0;
			sip->sip_source_interface = if_ifindex_make("brigde 0/0/1", NULL);
		}
	}
	ret = os_uci_get_integer("voipconfig.sip.sip_local_port", &value);
	if(ret == OK)
		sip->sip_local_port = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_local_port = %d", sip->sip_local_port);
#endif

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_address("voipconfig.sip.sip_server", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_server = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
		sip->sip_server = ntohl(inet_addr(tmp));

	ret = os_uci_get_integer("voipconfig.sip.sip_port", &value);
	if(ret == OK)
		sip->sip_port = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_port = %d", sip->sip_port);
#endif
	if(sip->sip_active_standby == FALSE)
		return OK;
	//sec
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_address("voipconfig.sip.sip_server_sec", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_server_sec = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
		sip->sip_server_sec = ntohl(inet_addr(tmp));

	ret = os_uci_get_integer("voipconfig.sip.sip_port_sec", &value);
	if(ret == OK)
		sip->sip_port_sec = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_port_sec = %d", sip->sip_port_sec);
#endif
	return OK;
}

static int voip_uci_sip_config_load_proxy_address(voip_sip_t *sip)
{
	char tmp[128];
	int	 value = 0, ret = ERROR;
	zassert(sip != NULL);

	if(sip->sip_proxy_enable == FALSE)
		return OK;
	memset(tmp, 0, sizeof(tmp));

	ret = os_uci_get_address("voipconfig.sip.sip_proxy_server", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_proxy_server = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
		sip->sip_proxy_server = ntohl(inet_addr(tmp));

	ret = os_uci_get_integer("voipconfig.sip.sip_proxy_port", &value);
	if(ret == OK)
		sip->sip_proxy_port = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_proxy_port = %d", sip->sip_port);
#endif

	//sec
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_address("voipconfig.sip.sip_proxy_server_sec", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_proxy_server_sec = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
		sip->sip_proxy_server_sec = ntohl(inet_addr(tmp));

	ret = os_uci_get_integer("voipconfig.sip.sip_proxy_port_sec", &value);
	if(ret == OK)
		sip->sip_proxy_port_sec = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_proxy_port_sec = %d", sip->sip_port_sec);
#endif
	return OK;
}

static int voip_uci_sip_config_load_username(voip_sip_t *sip)
{
	char tmp[128];
	int	 ret = ERROR;
	zassert(sip != NULL);
	memset(tmp, 0, sizeof(tmp));

	ret = os_uci_get_string("voipconfig.sip.localphone", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->localphone = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
	{
		memset(sip->sip_local_number, 0, sizeof(sip->sip_local_number));
		strncpy(sip->sip_local_number, tmp, MIN(sizeof(sip->sip_local_number), strlen(tmp)));
	}

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("voipconfig.sip.username", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->username = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
	{
		memset(sip->sip_user, 0, sizeof(sip->sip_user));
		if(!all_space(tmp))
			strncpy(sip->sip_user, tmp, MIN(sizeof(sip->sip_user), strlen(tmp)));
	}

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("voipconfig.sip.password", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->password = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
	{
		memset(sip->sip_password, 0, sizeof(sip->sip_password));
		strncpy(sip->sip_password, tmp, MIN(sizeof(sip->sip_password), strlen(tmp)));
	}

	if(sip->sip_multi_user == FALSE)
		return OK;

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("voipconfig.sip.localphone_sec", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->localphone_sec = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
	{
		memset(sip->sip_local_number_alias, 0, sizeof(sip->sip_local_number_alias));
		strncpy(sip->sip_local_number_alias, tmp, MIN(sizeof(sip->sip_local_number_alias), strlen(tmp)));
	}

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("voipconfig.sip.username_sec", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->username_sec = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
	{
		memset(sip->sip_user_alias, 0, sizeof(sip->sip_user_alias));
		if(!all_space(tmp))
			strncpy(sip->sip_user_alias, tmp, MIN(sizeof(sip->sip_user_alias), strlen(tmp)));
	}

	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("voipconfig.sip.password_sec", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->password_sec = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
	{
		memset(sip->sip_password_alias, 0, sizeof(sip->sip_password_alias));
		strncpy(sip->sip_password_alias, tmp, MIN(sizeof(sip->sip_password_alias), strlen(tmp)));
	}
	return OK;
}


static int voip_uci_sip_config_load_misc(voip_sip_t *sip)
{
	char tmp[128];
	int	 value = 0, ret = ERROR;
	zassert(sip != NULL);
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("voipconfig.sip.proto", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->proto = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
	{
		if(strncasecmp(tmp, "UDP", 3) == 0)
			sip->proto = SIP_PROTO_UDP;
		else if(strncasecmp(tmp, "tcp", 3) == 0)
			sip->proto = SIP_PROTO_TCP;
		else if(strncasecmp(tmp, "DTLS", 3) == 0)
			sip->proto = SIP_PROTO_DTLS;
		else if(strncasecmp(tmp, "TLS", 3) == 0)
			sip->proto = SIP_PROTO_TLS;
	}
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("voipconfig.sip.payload", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->payload = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
	{
		if(voip_sip_payload_index(tmp) >= 0)
		{
			//voip_stream_payload_api(tmp, -1);
			sip->payload = voip_sip_payload_index(tmp);
			memset(sip->payload_name, 0, sizeof(sip->payload_name));
			strcpy(sip->payload_name, strlwr(tmp));
		}
	}
	memset(tmp, 0, sizeof(tmp));
	ret = os_uci_get_string("voipconfig.sip.dtmf", tmp);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->dtmf = %s", tmp);
#endif
	if(ret == OK && strlen(tmp) >= 1)
	{
		if(strstr(tmp, "2833"))
			sip->dtmf = VOIP_SIP_RFC2833;
		else if(strstr(tmp, "info") || strstr(tmp, "INFO"))
			sip->dtmf = VOIP_SIP_INFO;
		else if(strstr(tmp, "INBAND") || strstr(tmp, "inband"))
			sip->dtmf = VOIP_SIP_INBAND;
	}

	ret = os_uci_get_integer("voipconfig.sip.registerinterval", &value);
	if(ret == OK)
		sip->sip_register_interval = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_register_interval = %d", sip->sip_register_interval);
#endif
	if(sip->sip_keepalive == FALSE)
		return OK;
	ret = os_uci_get_integer("voipconfig.sip.sip_keepalive_interval", &value);
	if(ret == OK)
		sip->sip_keepalive_interval = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_keepalive_interval = %d", sip->sip_keepalive_interval);
#endif

	return OK;
}


int voip_uci_sip_config_load(voip_sip_t *sip)
{
	int	 value = 0, ret = ERROR;
	zassert(sip != NULL);

	if(sip->mutex)
		os_mutex_lock(sip->mutex, OS_WAIT_FOREVER);

	ret = os_uci_get_integer("voipconfig.sip.enable", &value);
	if(ret == OK)
		sip->sip_enable = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "sip->sip_enable = %d", sip->sip_enable);
#endif
	ret = os_uci_get_integer("voipconfig.sip.sip_active_standby", &value);
	if(ret == OK)
		sip->sip_active_standby = value;

	ret = os_uci_get_integer("voipconfig.sip.sip_multi_user", &value);
	if(ret == OK)
		sip->sip_multi_user = value;

	ret = os_uci_get_integer("voipconfig.sip.sip_keepalive", &value);
	if(ret == OK)
		sip->sip_keepalive = value;

	ret = os_uci_get_integer("voipconfig.sip.sip_proxy_enable", &value);
	if(ret == OK)
		sip->sip_proxy_enable = value;

	voip_uci_sip_config_load_address(sip);
	voip_uci_sip_config_load_proxy_address(sip);
	voip_uci_sip_config_load_username(sip);
	voip_uci_sip_config_load_misc(sip);
	if(sip->mutex)
		os_mutex_unlock(sip->mutex);
	return OK;
}

//voipconfig.sip.localport
int voip_uci_sip_config_save(voip_sip_t *sip)
{
	zassert(sip != NULL);
	if(sip->mutex)
		os_mutex_lock(sip->mutex, OS_WAIT_FOREVER);
	os_uci_set_integer("voipconfig.sip.enable", sip->sip_enable);

	os_uci_set_integer("voipconfig.sip.sip_active_standby", sip->sip_active_standby);
	os_uci_set_integer("voipconfig.sip.sip_multi_user", sip->sip_multi_user);
	os_uci_set_integer("voipconfig.sip.sip_proxy_enable", sip->sip_proxy_enable);
	os_uci_set_integer("voipconfig.sip.sip_keepalive", sip->sip_keepalive);

	if(sip->sip_keepalive)
		os_uci_set_integer("voipconfig.sip.sip_keepalive_interval", sip->sip_keepalive_interval);

	if(sip->sip_source_interface == if_ifindex_make("ethernet 0/0/2", NULL))
		os_uci_set_string("voipconfig.sip.sip_source_interface", "wan");
	else if(sip->sip_source_interface == if_ifindex_make("brigde 0/0/1", NULL))
		os_uci_set_string("voipconfig.sip.sip_source_interface", "lan");

	if(sip->sip_local_port)
		os_uci_set_integer("voipconfig.sip.sip_local_port", sip->sip_local_port);


	if(strlen(sip->sip_local_number))
	{
		os_uci_set_string("voipconfig.sip.localphone", sip->sip_local_number);
	}

	if(strlen(sip->sip_user))
	{
		os_uci_set_string("voipconfig.sip.username", sip->sip_user);
	}

	if(strlen(sip->sip_password))
	{
		os_uci_set_string("voipconfig.sip.password", sip->sip_password);
	}
	if(sip->sip_multi_user)
	{
		if(strlen(sip->sip_local_number_alias))
		{
			os_uci_set_string("voipconfig.sip.localphone_sec", sip->sip_local_number_alias);
		}

		if(strlen(sip->sip_user_alias))
		{
			os_uci_set_string("voipconfig.sip.username_sec", sip->sip_user_alias);
		}

		if(strlen(sip->sip_password_alias))
		{
			os_uci_set_string("voipconfig.sip.password_sec", sip->sip_password_alias);
		}
	}

	if(sip->sip_server)
		os_uci_set_string("voipconfig.sip.sip_server", inet_address(sip->sip_server));

	if(sip->sip_port)
		os_uci_set_integer("voipconfig.sip.sip_port", sip->sip_port);


	if(sip->sip_active_standby)
	{
		if(sip->sip_server_sec)
			os_uci_set_string("voipconfig.sip.sip_server_sec", inet_address(sip->sip_server_sec));

		if(sip->sip_port_sec)
			os_uci_set_integer("voipconfig.sip.sip_port_sec", sip->sip_port_sec);
	}

	if(sip->sip_proxy_enable)
	{
		if(sip->sip_proxy_server)
			os_uci_set_string("voipconfig.sip.sip_proxy_server", inet_address(sip->sip_proxy_server));

		if(sip->sip_proxy_port)
			os_uci_set_integer("voipconfig.sip.sip_proxy_port", sip->sip_proxy_port);

		if(sip->sip_proxy_server_sec)
			os_uci_set_string("voipconfig.sip.sip_proxy_server_sec", inet_address(sip->sip_proxy_server_sec));

		if(sip->sip_proxy_port_sec)
			os_uci_set_integer("voipconfig.sip.sip_proxy_port_sec", sip->sip_proxy_port_sec);
	}

	os_uci_set_integer("voipconfig.sip.registerinterval", sip->sip_register_interval);

	if(sip->proto == SIP_PROTO_UDP)
		os_uci_set_string("voipconfig.sip.proto", "UDP");
	else if(sip->proto == SIP_PROTO_TCP)
		os_uci_set_string("voipconfig.sip.proto", "TCP");
	else if(sip->proto == SIP_PROTO_TLS)
		os_uci_set_string("voipconfig.sip.proto", "TLS");
	else if(sip->proto == SIP_PROTO_DTLS)
		os_uci_set_string("voipconfig.sip.proto", "DTLS");

	if(strlen(sip->payload_name) && sip->payload)
		os_uci_set_string("voipconfig.sip.payload", voip_sip_payload_name(sip->payload));

	if(sip->dtmf == VOIP_SIP_RFC2833)
		os_uci_set_string("voipconfig.sip.dtmf", "RFC2833");
	else if(sip->dtmf == VOIP_SIP_INFO)
		os_uci_set_string("voipconfig.sip.dtmf", "SIP-INFO");
	else if(sip->dtmf == VOIP_SIP_INBAND)
		os_uci_set_string("voipconfig.sip.dtmf", "inband");

	os_uci_save_config("voipconfig");
	if(sip->mutex)
		os_mutex_unlock(sip->mutex);
	return OK;
}


static int _voip_stream_config_load(voip_stream_t *voip)
{
	int	 value = 0;
	float	 floatvalue = 0.0;
	zassert(voip != NULL);
	os_uci_get_integer("voipconfig.voip.enable", &value);
	voip->enable = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->enable = %d", voip->enable);
#endif
	os_uci_get_integer("voipconfig.voip.localport", &value);
	voip->l_rtp_port = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->rtp_local_port = %d", voip->l_rtp_port);
#endif
	os_uci_get_integer("voipconfig.voip.dtmf_echo", &value);
	voip->play_dtmf = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->play_dtmf = %d", voip->play_dtmf);
#endif
	os_uci_get_integer("voipconfig.voip.disbale_rtcp", &value);
	voip->rtcp = value?FALSE:TRUE;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->rtcp = %d", voip->rtcp);
#endif
	os_uci_get_integer("voipconfig.voip.disbale_avpf", &value);
	voip->avpf = value?FALSE:TRUE;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->avpf = %d", voip->avpf);
#endif

	os_uci_get_integer("voipconfig.voip.bitrate", &value);
	voip->bitrate = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->bitrate = %d", voip->bitrate);
#endif

	os_uci_get_integer("voipconfig.voip.jitter", &value);
	voip->jitter = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->jitter = %d", voip->jitter);
#endif

	os_uci_get_integer("voipconfig.voip.ec_canceller", &value);
	voip->echo_canceller = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->echo_canceller = %d", voip->echo_canceller);
#endif

	os_uci_get_integer("voipconfig.voip.ec_tail", &value);
	voip->ec_tail = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->ec_tail = %d", voip->ec_tail);
#endif
	os_uci_get_integer("voipconfig.voip.ec_delay", &value);
	voip->ec_delay = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->ec_delay = %d", voip->ec_delay);
#endif

	os_uci_get_integer("voipconfig.voip.ec_framesize", &value);
	voip->ec_framesize = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->ec_framesize = %d", voip->ec_framesize);
#endif

	os_uci_get_integer("voipconfig.voip.ec_limiter", &value);
	voip->echo_limiter = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->echo_limiter = %d", voip->echo_limiter);
#endif

	os_uci_get_float("voipconfig.voip.el_force", &floatvalue);
	voip->el_force = floatvalue;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->el_force = %f", voip->el_force);
#endif

	os_uci_get_float("voipconfig.voip.el_speed", &floatvalue);
	voip->el_speed = floatvalue;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->el_speed = %f", voip->el_speed);
#endif

	os_uci_get_float("voipconfig.voip.el_thres", &floatvalue);
	voip->el_thres = floatvalue;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->el_thres = %f", voip->el_thres);
#endif

	os_uci_get_float("voipconfig.voip.el_transmit", &floatvalue);
	voip->el_transmit = floatvalue;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->el_transmit = %f", voip->el_transmit);
#endif

	os_uci_get_integer("voipconfig.voip.el_sustain", &value);
	voip->el_sustain = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->el_sustain = %d", voip->el_sustain);
#endif

	os_uci_get_integer("voipconfig.voip.noise_gate", &value);
	voip->noise_gate = value;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->noise_gate = %d", voip->noise_gate);
#endif

	os_uci_get_float("voipconfig.voip.ng_floorgain", &floatvalue);
	voip->ng_floorgain = floatvalue;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->ng_floorgain = %f", voip->ng_floorgain);
#endif
	os_uci_get_float("voipconfig.voip.ng_threshold", &floatvalue);
	voip->ng_threshold = floatvalue;
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->ng_threshold = %f", voip->ng_threshold);
#endif

	os_uci_get_integer("voipconfig.voip.volume", &value);
	//voip->volume = value;
	voip_playback_volume_out_set_api(value);
#ifdef OSIP_LOAD_DEBUG
	zlog_debug(ZLOG_VOIP, "voip->volume = %d", value);
#endif

	return OK;
}
//voipconfig.voip.localport
static int voip_stream_config_save(voip_stream_t *voip)
{
	u_int8	 volume = 0;
	zassert(voip != NULL);
	os_uci_set_integer("voipconfig.voip.enable", voip->enable);
	os_uci_set_integer("voipconfig.voip.localport", voip->l_rtp_port);
	os_uci_set_integer("voipconfig.voip.dtmf_echo", voip->play_dtmf);
	os_uci_set_integer("voipconfig.voip.disbale_rtcp", voip->rtcp);
	os_uci_set_integer("voipconfig.voip.disbale_avpf", voip->avpf);
	os_uci_set_integer("voipconfig.voip.bitrate", voip->bitrate);
	os_uci_set_integer("voipconfig.voip.jitter", voip->jitter);

	os_uci_set_integer("voipconfig.voip.ec_canceller", voip->echo_canceller);
		os_uci_set_integer("voipconfig.voip.ec_tail", voip->ec_tail);
		os_uci_set_integer("voipconfig.voip.ec_delay", voip->ec_delay);
		os_uci_set_integer("voipconfig.voip.ec_framesize", voip->ec_framesize);

	os_uci_set_integer("voipconfig.voip.ec_limiter", voip->echo_limiter);
		os_uci_set_float("voipconfig.voip.el_force", voip->el_force);
		os_uci_set_float("voipconfig.voip.el_speed", voip->el_speed);
		os_uci_set_float("voipconfig.voip.el_thres", voip->el_thres);
		os_uci_set_float("voipconfig.voip.el_transmit", voip->el_transmit);
		os_uci_set_integer("voipconfig.voip.el_sustain", voip->el_sustain);


	os_uci_set_integer("voipconfig.voip.noise_gate", voip->noise_gate);
		os_uci_set_float("voipconfig.voip.ng_floorgain", voip->ng_floorgain);
		os_uci_set_float("voipconfig.voip.ng_threshold", voip->ng_threshold);

	voip_playback_volume_out_get_api(&volume);
	os_uci_set_integer("voipconfig.voip.volume", volume);

	os_uci_save_config("voipconfig");
	return OK;
}


static int voip_stream_config_update_thread(voip_event_t *event)
{
	zassert(event != NULL);
	voip_stream_t *voip = event->pVoid;
	zassert(voip != NULL);
	voip->t_event = NULL;
	if(voip_stream_config_save(voip) == OK)
		;//vty_execute_shell("write memory");
	return OK;
}

int voip_stream_config_update_api(voip_stream_t *voip)
{
	zassert(voip != NULL);
	{
		if(voip->t_event != NULL)
			voip_event_cancel(voip->t_event);
		voip->t_event = voip_event_timer_add(voip_stream_config_update_thread, voip, NULL, 0, 5);
	}
	return OK;
}

int voip_stream_config_load(voip_stream_t *voip)
{
	return _voip_stream_config_load(voip);
}



static int voip_stream_restart_job(void *p)
{
	zassert(voip_stream != NULL);
	//if(strstr(buf, "voip"))
	{
		_voip_stream_config_load(voip_stream);
		vty_execute_shell("write memory");
	}
	return OK;
}


static int voip_sip_restart_job(void *p)
{
	zassert(sip_config != NULL);
	//if(strstr(buf, "sip"))
	{
		zlog_debug(ZLOG_VOIP, "OSIP Reload");
		//voip_sip_config_load(sip_config);
		voip_uci_sip_config_load(sip_config);
		//zlog_debug(ZLOG_VOIP, "OSIP Restart");
		voip_osip_restart();
		//zlog_debug(ZLOG_VOIP, "OSIP Save Config");
		vty_execute_shell("write memory");
	}
	return OK;
}



/*
 * call testing
 */
static int voip_ubus_call_enable(BOOL start)
{
	int	 ret = ERROR;
	if(start)
	{
		char tmp[128];
		memset(tmp, 0, sizeof(tmp));
		/*
		 * @2003
		 * @2003:2005
		 *
		 * 0:1:304
		 */
		ret = os_uci_get_string("voipconfig.testing.callnum", tmp);
		if(ret == OK && strlen(tmp) >= 1)
		{
#ifdef VOIP_STREAM_UNIT_TESTING
			if(strstr(tmp, "@test"))
			{
				x5b_app_A_unit_test_set_api(TRUE);
				super_system("echo sadadas > /tmp/app/unit-test");
				sync();
				x5b_unit_test_init();
				return OK;
			}
#endif
			u_int8 building = 0, unit = 0;
			u_int16 room = atoi(tmp);
/*			char phonelist[128];
			memset(phonelist, 0, sizeof(phonelist));*/

/*			if(voip_app_call_spilt_from_web(tmp, &building,
					&unit, &room, phonelist, sizeof(phonelist)) == OK)*/
			{
				ret = voip_app_start_call_event_cli_web(APP_CALL_ID_WEB, building, unit,
						room/*, NULL, phonelist*/, NULL);
				if(ret == OK)
				{
					os_uci_set_integer("voipconfig.testing.callstate", 1);
					os_uci_set_integer("voipconfig.status.voiptalk", 1);
				}
				else
				{
					os_uci_set_integer("voipconfig.testing.callstate", 0);
					os_uci_set_integer("voipconfig.status.voiptalk", 0);
				}
				os_uci_save_config("voipconfig");
			}
			//else
			if(ret != OK)
				return ERROR;
		}
	}
	else
	{
		if(voip_app_call_event_current())
			ret = voip_app_stop_call_event_cli_web(voip_app_call_event_current());
		if(ret == OK)
			os_uci_set_integer("voipconfig.testing.callstate", 0);
		else
			os_uci_set_integer("voipconfig.testing.callstate", 0);
		os_uci_set_integer("voipconfig.status.voiptalk", 0);
		os_uci_save_config("voipconfig");
	}
	return ret;
}
#endif


static int tcpdump_capture_start(char *name)
{
	char cap_cmd[512];
	char ifname[16];
	char tmp[32];
	char filler[256];
	u_int32 ifindex = 0;
	sip_transport_t proto;
	u_int16 port = 0;

	voip_sip_source_interface_get_api(&ifindex);
	memset(ifname, 0, sizeof(ifname));
	if(ifindex && ifindex == if_ifindex_make("ethernet 0/0/2", NULL))
		snprintf(ifname, sizeof(ifname), "%s", "eth0.2");
	else if(ifindex && ifindex == if_ifindex_make("brigde 0/0/1", NULL))
		snprintf(ifname, sizeof(ifname), "%s", "br-lan");

	voip_sip_local_port_get_api(&port);
	voip_sip_transport_proto_get_api(&proto);
	memset(filler, 0, sizeof(filler));
	if(proto == SIP_PROTO_UDP)
	{
		snprintf(filler, sizeof(filler), "udp port %d", port);
	}
	else
	{
		snprintf(filler, sizeof(filler), "tcp port %d", port);
	}
	port = voip_stream_local_port_get_api();
	memset(tmp, 0, sizeof(tmp));
	snprintf(tmp, sizeof(tmp), " or udp port %d", port);
	strcat(filler, tmp);

	memset(tmp, 0, sizeof(tmp));
	snprintf(tmp, sizeof(tmp), " or udp port %d", port + 1);
	strcat(filler, tmp);
	/*
	 * tcpdump -i eth0.2 udp port 5060 or udp port 5551 or udp port 5550 -w /tmp/app/tmp/aa.pcap
	 */
	snprintf(cap_cmd, sizeof(cap_cmd), "tcpdump -i %s %s -w /tmp/app/tmp/%s > /dev/null &", ifname, filler, name);
	super_system(cap_cmd);

	//zlog_debug(ZLOG_VOIP, "----------------%s:%s", __func__, cap_cmd);
	return OK;
}

static int tcpdump_capture_stop(void)
{
/*	pid_t pid = name2pid("tcpdump");
	if(pid > 0)
	{
		zlog_debug(ZLOG_VOIP, "----------------%s:%d", __func__, pid);
		kill(pid, -9);
	}
	else*/
	{
		//zlog_debug(ZLOG_VOIP, "----------------%s:killall -9 tcpdump", __func__);
		super_system("killall -9 tcpdump");
	}
	return OK;
}


#ifdef PL_OPENWRT_UCI
static int voip_ubus_capture_enable(BOOL start)
{
	static u_int8 capture_enable = 0;
	static char prefix_filename[32];
	int	 ret = ERROR;
	if(start)
	{
		int value = 0;
		char tmp[128];
		memset(tmp, 0, sizeof(tmp));
		ret = os_uci_get_integer("voipconfig.testing.capture_enable", &value);
		if(value == 1 && capture_enable == 0)
		{
			int level = 0;
			struct tm *tm = NULL;
			char log_filename[128];
			time_t ttt = 0;
			capture_enable = 1;
			ret = os_uci_get_string("voipconfig.testing.log_level", tmp);
			if(ret == OK && strlen(tmp) >= 1)
			{
				if(strstr(tmp, "Debug"))
					level = LOG_DEBUG;
				else if(strstr(tmp, "Information"))
					level = LOG_INFO;
				else if(strstr(tmp, "Notifications"))
					level = LOG_NOTICE;
				else if(strstr(tmp, "Warnings"))
					level = LOG_WARNING;
			}
			else
				level = LOG_DEBUG;


			ttt = os_time(NULL);
			tm = gmtime(&ttt);
			memset(log_filename, 0, sizeof(log_filename));
			memset(prefix_filename, 0, sizeof(prefix_filename));
			strftime(prefix_filename, sizeof(prefix_filename), "%Y-%m-%d_%H-%M-%S", tm);
			snprintf(log_filename, sizeof(log_filename), "%s-capture.log", prefix_filename);

			zlog_testing_file(log_filename);
			zlog_testing_priority(level);
			zlog_testing_enable(TRUE);

			ret = os_uci_get_integer("voipconfig.testing.log_mod_sip", &value);
			if(ret == OK && value == 1)
				voip_osip_set_log_level(level, 1);

			ret = os_uci_get_integer("voipconfig.testing.log_mod_app", &value);
			if(ret == OK && value == 1)
				voip_app_debug_set_api(0X100f);

			ret = os_uci_get_integer("voipconfig.testing.log_mod_media", &value);
			if(ret == OK && value == 1)
			{
/*
				if(level == LOG_DEBUG)
					voip_stream_debug_set_api(TRUE, "debug");
				if(level == LOG_WARNING)
					voip_stream_debug_set_api(TRUE, "warning");
				if(level == LOG_NOTICE)
					voip_stream_debug_set_api(TRUE, "trace");
				if(level == LOG_INFO)
					voip_stream_debug_set_api(TRUE, "message");
*/
				//zlog_debug(ZLOG_VOIP, "----------------%s:%s", __func__, prefix_filename);
				memset(log_filename, 0, sizeof(log_filename));
				snprintf(log_filename, sizeof(log_filename), "%s-capture.pcap", prefix_filename);
				tcpdump_capture_start(log_filename);
			}
		}
	}
	else
	{
		int value = 0;
		ret = os_uci_get_integer("voipconfig.testing.capture_enable", &value);
		if(value == 1 && capture_enable == 1)
		{
			capture_enable = 0;
			char log_cmd[128];
			voip_osip_set_log_level(LOG_WARNING, 1);
			voip_app_debug_set_api(0);
#ifdef PL_VOIP_MEDIASTREAM
			bctbx_get_log_level_mask(BCTBX_LOG_DOMAIN);
			bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_ERROR);
#endif
			//voip_stream_debug_set_api(TRUE, "warning");

			zlog_testing_enable(FALSE);

			if(strlen(prefix_filename))
			{
				//zlog_debug(ZLOG_VOIP, "----------------%s:%s", __func__, prefix_filename);
				memset(log_cmd, 0, sizeof(log_cmd));
				snprintf(log_cmd, sizeof(log_cmd), "mv /tmp/app/log/%s-capture.log /tmp/app/tmp/ > /dev/null", prefix_filename);
				super_system(log_cmd);
				tcpdump_capture_stop();
				os_uci_set_integer("voipconfig.testing.capture_enable", 0);
				os_uci_save_config("voipconfig");

				memset(log_cmd, 0, sizeof(log_cmd));
				snprintf(log_cmd, sizeof(log_cmd), "%s.tar.gz", prefix_filename);
				os_uci_set_string("voipconfig.testing.capturefile", log_cmd);
				os_uci_save_config("voipconfig");
/*
				memset(log_cmd, 0, sizeof(log_cmd));
				snprintf(log_cmd, sizeof(log_cmd), "cd /tmp/app/ && tar -zcf %s.tar.gz tmp > /dev/null", prefix_filename);
				super_system(log_cmd);

				memset(log_cmd, 0, sizeof(log_cmd));
				snprintf(log_cmd, sizeof(log_cmd), "%s.tar.gz", prefix_filename);

				os_uci_set_string("voipconfig.testing.capturefile", log_cmd);
				os_uci_save_config("voipconfig");
*/
				memset(prefix_filename, 0, sizeof(prefix_filename));
			}
		}
	}
	return OK;
}



/*struct ubus_tmp
{
	char buf[512];
	int len
};*/

static int voip_sip_restart_cb(void *buf)
{
	//return voip_sip_restart_job(NULL);
	os_job_add(voip_sip_restart_job, NULL);
	return OK;
}

static int voip_stream_restart_cb(void *buf)
{
	os_job_add(voip_stream_restart_job, NULL);
	return OK;
}

static int voip_ubus_uci_action(char *buf, int len)
{
/*
	struct ubus_tmp *uci_tmp = malloc(sizeof(struct ubus_tmp));
	if(!uci_tmp)
		return ERROR;
	memset(uci_tmp, 0, sizeof(struct ubus_tmp));
	strncpy(uci_tmp->buf, buf, MIN(len, sizeof(uci_tmp->buf)))
	uci_tmp->len = len;
*/
	if(strstr(buf, "sip"))
	{
		if(strstr(buf + 3, "restart"))
			os_time_create_once(voip_sip_restart_cb, NULL, 1500);
			//ret = voip_sip_restart_cb(NULL);
	}
	else if(strstr(buf, "voip"))
	{
		if(strstr(buf + 4, "restart"))
			os_time_create_once(voip_stream_restart_cb, NULL, 1500);
			//ret = voip_stream_restart_cb(NULL);
	}
	return OK;
}

int voip_ubus_uci_update_cb(char *buf, int len)
{
	int ret = 0;
	if(strstr(buf, "sip"))
	{
		ret = voip_ubus_uci_action(buf, len);
/*		if(strstr(buf + 3, "restart"))
			ret = voip_sip_restart_cb(NULL);*/
	}
	else if(strstr(buf, "voip"))
	{
		if(strstr(buf + 4, "restart"))
			ret = voip_ubus_uci_action(buf, len);
			//ret = voip_stream_restart_cb(NULL);

		else if(strstr(buf + 4, "start-call"))
		{
			ret = voip_ubus_call_enable(TRUE);
		}
		else if(strstr(buf + 4, "stop-call"))
			ret = voip_ubus_call_enable(FALSE);

		else if(strstr(buf + 4, "start-capture"))
			ret = voip_ubus_capture_enable(TRUE);
		else if(strstr(buf + 4, "stop-capture"))
			ret = voip_ubus_capture_enable(FALSE);
	}
	else if(strstr(buf, "dbase"))
	{
		voip_ubus_dbase_sync(2);
		if(strstr(buf, "add"))
			ret = voip_ubus_dbase_sync(1);
		else if(strstr(buf, "delete"))
			ret = voip_ubus_dbase_sync(-1);
		else if(strstr(buf, "select"))
			ret = voip_ubus_dbase_sync(2);
	}
	return ret;
}

#endif
