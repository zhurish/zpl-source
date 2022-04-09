/*
 * web_sip_html.c
 *
 *  Created on: 2019年8月22日
 *      Author: DELL
 */

#define HAS_BOOL 1
#include "zplos_include.h"
#include "vty.h"
#include "if.h"

#include "buffer.h"
#include "command.h"
#include "if_name.h"
#include "linklist.h"
#include "log.h"
#include "zmemory.h"
#include "prefix.h"
#include "sockunion.h"
#include "str.h"
#include "table.h"
#include "vector.h"
#include "nsm_ip_vrf.h"
#include "nsm_interface.h"
#include "nsm_client.h"

#include "web_util.h"
#include "web_jst.h"
#include "web_app.h"
#include "web_api.h"


//#define ZPL_PJSIP_MODULE
#ifdef ZPL_PJSIP_MODULE

//#include <pjlib-util/cli_socket.h>
#include "pjsua_app_common.h"
#include "pjsua_app_config.h"
#include "pjsip_app_api.h"


static int web_sip_config_get(char *buf)
{
	zpl_uint8 vol = 0;
	zassert(pl_pjsip != NULL);

	if(pl_pjsip->mutex)
		os_mutex_lock(pl_pjsip->mutex, OS_WAIT_FOREVER);

	//memset(buf, 0, sizeof(buf));
	strcat(buf, "\"response\":");
	strcat(buf, "\"OK\",");

	strcat(buf, "\"sip_enable\":");
	strcat(buf, "true,");

	strcat(buf, "\"interface\":\"");
	if(web_type_get() == WEB_TYPE_HOME_WIFI)
	{
		if(pl_pjsip->sip_source_interface ==ifname2ifindex("ethernet 0/0/2"))
		{
			strcat(buf, "wan");
		}
		else if(pl_pjsip->sip_source_interface ==ifname2ifindex("brigde 0/0/1"))
		{
			strcat(buf, "lan");
		}
	}
	else
		strcat(buf, ifindex2ifname(pl_pjsip->sip_source_interface));
	strcat(buf, "\",");

	strcat(buf, "\"local_port\":\"");
	strcat(buf, itoa(pl_pjsip->sip_local.sip_port, 10));
	strcat(buf, "\",");

	strcat(buf, "\"rtp_port\":\"");
	strcat(buf, itoa(pl_pjsip->sip_rtp_port, 10));
	strcat(buf, "\",");

	strcat(buf, "\"proto\":\"");
	strcat(buf, pl_pjsip_transport_name(pl_pjsip->proto));
	strcat(buf, "\",");

	strcat(buf, "\"dtmf\":\"");
	strcat(buf, pl_pjsip_dtmf_name(pl_pjsip->dtmf));
	strcat(buf, "\",");

	strcat(buf, "\"codec\":\"");
	strcat(buf, pl_pjsip->sip_codec.payload_name);
	strcat(buf, "\",");

	strcat(buf, "\"sip_realm\":\"");
	strcat(buf, pl_pjsip->sip_realm);
	strcat(buf, "\",");

	strcat(buf, "\"sip_expires\":\"");
	strcat(buf, itoa(pl_pjsip->sip_expires, 10));
	strcat(buf, "\",");

	if(strlen(pl_pjsip->sip_user.sip_user))
	{
/*
		strcat(buf, "\"sip_user_reg\":\"");
		strcat(buf, "true");
*/

		strcat(buf, "\"sip_user\":\"");
		strcat(buf, pl_pjsip->sip_user.sip_user);
		strcat(buf, "\",");
	}
	if(strlen(pl_pjsip->sip_user.sip_phone))
	{
		strcat(buf, "\"sip_number\":\"");
		strcat(buf, pl_pjsip->sip_user.sip_phone);
		strcat(buf, "\",");
	}
	if(strlen(pl_pjsip->sip_user.sip_password))
	{
		strcat(buf, "\"sip_password\":\"");
		strcat(buf, pl_pjsip->sip_user.sip_password);
		strcat(buf, "\",");
	}
	strcat(buf, "\"sip_100_rel\":\"");
	strcat(buf, pl_pjsip->sip_100_rel ? "true":"false");
	strcat(buf, "\",");

	if(strlen(pl_pjsip->sip_server.sip_address))
	{
		strcat(buf, "\"serverip\":\"");
		strcat(buf, pl_pjsip->sip_server.sip_address);
		strcat(buf, "\",");

		strcat(buf, "\"serverport\":\"");
		strcat(buf, itoa(pl_pjsip->sip_server.sip_port, 10));
		strcat(buf, "\",");
	}

	if(strlen(pl_pjsip->sip_server_sec.sip_address))
	{
		//strcat(buf, "\"sip_backup\":\"");
		//strcat(buf, "true");

		strcat(buf, "\"serverip_backup\":\"");
		strcat(buf, pl_pjsip->sip_server_sec.sip_address);
		strcat(buf, "\",");

		strcat(buf, "\"serverport_backup\":\"");
		strcat(buf, itoa(pl_pjsip->sip_server_sec.sip_port, 10));
		strcat(buf, "\",");
	}

	if(strlen(pl_pjsip->sip_proxy.sip_address))
	{
		//strcat(buf, "\"sip_proxy\":\"");
		//strcat(buf, "true");

		strcat(buf, "\"proxy_server\":\"");
		strcat(buf, pl_pjsip->sip_proxy.sip_address);
		strcat(buf, "\",");

		strcat(buf, "\"proxy_port\":\"");
		strcat(buf, itoa(pl_pjsip->sip_proxy.sip_port, 10));
		strcat(buf, "\",");
	}

	if(strlen(pl_pjsip->sip_proxy_sec.sip_address))
	{
		//strcat(buf, "\"sip_proxy_backup\":\"");
		//strcat(buf, "true");

		strcat(buf, "\"proxy_server_backup\":\"");
		strcat(buf, pl_pjsip->sip_proxy_sec.sip_address);
		strcat(buf, "\",");

		strcat(buf, "\"proxy_port_backup\":\"");
		strcat(buf, itoa(pl_pjsip->sip_proxy_sec.sip_port, 10));
		strcat(buf, "\",");
	}

	if(strlen(pl_pjsip->sip_stun_server.sip_address))
	{
		//strcat(buf, "\"sip_proxy\":\"");
		//strcat(buf, "true");

		strcat(buf, "\"stun_server\":\"");
		strcat(buf, pl_pjsip->sip_stun_server.sip_address);
		strcat(buf, "\",");

		strcat(buf, "\"stun_port\":\"");
		strcat(buf, itoa(pl_pjsip->sip_stun_server.sip_port, 10));
		strcat(buf, "\",");

		if(strlen(pl_pjsip->sip_turn_user))
		{
			strcat(buf, "\"stun_user\":\"");
			strcat(buf, pl_pjsip->sip_turn_user);
			strcat(buf, "\",");
		}
		if(strlen(pl_pjsip->sip_turn_password))
		{
			strcat(buf, "\"stun_password\":\"");
			strcat(buf, pl_pjsip->sip_turn_password);
			strcat(buf, "\",");
		}
	}

	if(strlen(pl_pjsip->sip_outbound.sip_address))
	{
		//strcat(buf, "\"sip_proxy_backup\":\"");
		//strcat(buf, "true");

		strcat(buf, "\"outbound_server\":\"");
		strcat(buf, pl_pjsip->sip_outbound.sip_address);
		strcat(buf, "\",");

		strcat(buf, "\"outbound_port\":\"");
		strcat(buf, itoa(pl_pjsip->sip_outbound.sip_port, 10));
		strcat(buf, "\",");
	}

	if(strlen(pl_pjsip->sip_nameserver.sip_address))
	{
		//strcat(buf, "\"sip_proxy_backup\":\"");
		//strcat(buf, "true");

		strcat(buf, "\"nameserver_server\":\"");
		strcat(buf, pl_pjsip->sip_nameserver.sip_address);
		strcat(buf, "\",");

		strcat(buf, "\"nameserver_port\":\"");
		strcat(buf, itoa(pl_pjsip->sip_nameserver.sip_port, 10));
		strcat(buf, "\",");
	}

	for(vol = 0; vol < PJSIP_CODEC_MAX; vol++)
	{
		if(pl_pjsip->codec[vol].is_active &&
				strlen(pl_pjsip->codec[vol].payload_name)/* &&
				pl_pjsip->codec[vol].payload*/)
		{
			//codec_cmdname(pl_pjsip->codec[i].payload)
			if(pl_pjsip->codec[vol].payload == codec_payload_index("pcmu"))
			//if(strstr(pl_pjsip->codec[vol].payload_name, "pcmu"))
				strcat(buf, "\"pcmu_8000\":true,");
			else if(pl_pjsip->codec[vol].payload == codec_payload_index("pcma"))
			//else if(strstr(pl_pjsip->codec[vol].payload_name, "pcma"))
				strcat(buf, "\"pcma_8000\":true,");
			else if(pl_pjsip->codec[vol].payload == codec_payload_index("gsm"))
			//else if(strstr(pl_pjsip->codec[vol].payload_name, "gsm"))
				strcat(buf, "\"gsm_8000\":true,");
			else if(pl_pjsip->codec[vol].payload == codec_payload_index("g722"))
			//else if(strstr(pl_pjsip->codec[vol].payload_name, "g722"))
				strcat(buf, "\"g722_8000\":true,");
			else if(pl_pjsip->codec[vol].payload == codec_payload_index("speex-nb"))
			//else if(strstr(pl_pjsip->codec[vol].payload_name, "speex-nb"))
				strcat(buf, "\"speex_8000\":true,");
			else if(pl_pjsip->codec[vol].payload == codec_payload_index("ilbc"))
			//else if(strstr(pl_pjsip->codec[vol].payload_name, "ilbc"))
				strcat(buf, "\"ilbc_8000\":true,");
		}
	}
	voip_playback_volume_out_get_api(&vol);
	strcat(buf, "\"out_vol\":\"");
	strcat(buf, itoa(vol, 10));
	strcat(buf, "\",");

	voip_capture_volume_adc_get_api(&vol);
	strcat(buf, "\"in_vol\":\"");
	strcat(buf, itoa(vol, 10));
	strcat(buf, "\"");

	if(pl_pjsip->mutex)
		os_mutex_unlock(pl_pjsip->mutex);
	return OK;
}

static int web_sip_config_set(Webs *wp)
{
	char *strval = NULL;
	//zpl_int8 *cbuf = NULL;
	//zpl_int32 ival = 0;
	zassert(pl_pjsip != NULL);

	if(pl_pjsip->mutex)
		os_mutex_lock(pl_pjsip->mutex, OS_WAIT_FOREVER);
	strval = webs_get_var(wp, T("interface"), T(""));
	if (NULL == strval)
	{
		if(pl_pjsip->mutex)
			os_mutex_unlock(pl_pjsip->mutex);
		return web_return_text_plain(wp, ERROR);
	}
	if(web_type_get() == WEB_TYPE_HOME_WIFI)
	{
		if(strstr(strval, "wan"))
			pl_pjsip->sip_source_interface = ifname2ifindex("ethernet 0/0/2");
		if(strstr(strval, "lan"))
			pl_pjsip->sip_source_interface = ifname2ifindex("brigde 0/0/1");
	}
	else
		pl_pjsip->sip_source_interface = ifname2ifindex(strval);

	strval = webs_get_var(wp, T("local_port"), T(""));
	if (NULL == strval)
	{
		if(pl_pjsip->mutex)
			os_mutex_unlock(pl_pjsip->mutex);
		return web_return_text_plain(wp, ERROR);
	}
	pl_pjsip->sip_local.sip_port = atoi(strval);

	strval = webs_get_var(wp, T("serverip"), T(""));
	if (NULL == strval)
	{
		if(pl_pjsip->mutex)
			os_mutex_unlock(pl_pjsip->mutex);
		return web_return_text_plain(wp, ERROR);
	}
	memset(pl_pjsip->sip_server.sip_address, 0, sizeof(pl_pjsip->sip_server.sip_address));
	strcpy(pl_pjsip->sip_server.sip_address, strval);

	strval = webs_get_var(wp, T("serverport"), T(""));
	if (NULL == strval)
	{
		if(pl_pjsip->mutex)
			os_mutex_unlock(pl_pjsip->mutex);
		return web_return_text_plain(wp, ERROR);
	}
	pl_pjsip->sip_server.sip_port = atoi(strval);

	strval = webs_get_var(wp, T("sip_user"), T(""));
	if (NULL != strval)
	{
		memset(pl_pjsip->sip_user.sip_user, 0, sizeof(pl_pjsip->sip_user.sip_user));
		strcpy(pl_pjsip->sip_user.sip_user, strval);
	}

	strval = webs_get_var(wp, T("sip_number"), T(""));
	if (NULL == strval)
	{
		if(pl_pjsip->mutex)
			os_mutex_unlock(pl_pjsip->mutex);
		return web_return_text_plain(wp, ERROR);
	}
	memset(pl_pjsip->sip_user.sip_phone, 0, sizeof(pl_pjsip->sip_user.sip_phone));
	strcpy(pl_pjsip->sip_user.sip_phone, strval);

	strval = webs_get_var(wp, T("sip_password"), T(""));
	if (NULL != strval)
	{
		memset(pl_pjsip->sip_user.sip_password, 0, sizeof(pl_pjsip->sip_user.sip_password));
		strcpy(pl_pjsip->sip_user.sip_password, strval);
	}

	strval = webs_get_var(wp, T("sip_realm"), T(""));
	if (NULL != strval)
	{
		memset(pl_pjsip->sip_realm, 0, sizeof(pl_pjsip->sip_realm));
		strcpy(pl_pjsip->sip_realm, strval);
	}

	strval = webs_get_var(wp, T("sip_expires"), T(""));
	if (NULL != strval)
	{
		pl_pjsip->sip_expires = atoi(strval);
	}

	strval = webs_get_var(wp, T("proto"), T(""));
	if (NULL != strval)
	{
		if(strstr(strval, "UDP"))
			pl_pjsip->proto = PJSIP_PROTO_UDP;
		else if(strstr(strval, "TCP"))
			pl_pjsip->proto = PJSIP_PROTO_TCP;
		else if(strstr(strval, "TLS"))
			pl_pjsip->proto = PJSIP_PROTO_TLS;
		else if(strstr(strval, "DTLS"))
			pl_pjsip->proto = PJSIP_PROTO_DTLS;
	}

	strval = webs_get_var(wp, T("dtmf"), T(""));
	if (NULL != strval)
	{
		if(strstr(strval, "SIP-INFO"))
			pl_pjsip->dtmf = PJSIP_DTMF_INFO;
		else if(strstr(strval, "RFC"))
			pl_pjsip->dtmf = PJSIP_DTMF_RFC2833;
		else if(strstr(strval, "IN"))
			pl_pjsip->dtmf = PJSIP_DTMF_INBAND;
	}

	strval = webs_get_var(wp, T("codec"), T(""));
	if (NULL != strval)
	{
		if(pl_pjsip->mutex)
			os_mutex_unlock(pl_pjsip->mutex);
		pl_pjsip_codec_default_set_api(strval);
		if(pl_pjsip->mutex)
			os_mutex_lock(pl_pjsip->mutex, OS_WAIT_FOREVER);
		//pl_pjsip_codec_default_set_api(char * sip_codec, int indx)
	}

	strval = webs_get_var(wp, T("rtp_port"), T(""));
	if (NULL != strval)
	{
		pl_pjsip->sip_rtp_port = atoi(strval);
	}

	strval = webs_get_var(wp, T("serverip_backup"), T(""));
	if (NULL != strval)
	{
		memset(pl_pjsip->sip_server_sec.sip_address, 0, sizeof(pl_pjsip->sip_server_sec.sip_address));
		strcpy(pl_pjsip->sip_server_sec.sip_address, strval);
	}
	strval = webs_get_var(wp, T("serverport_backup"), T(""));
	if (NULL != strval)
	{
		pl_pjsip->sip_server_sec.sip_port = atoi(strval);
	}

	strval = webs_get_var(wp, T("proxy_server"), T(""));
	if (NULL != strval)
	{
		memset(pl_pjsip->sip_proxy.sip_address, 0, sizeof(pl_pjsip->sip_proxy.sip_address));
		strcpy(pl_pjsip->sip_proxy.sip_address, strval);
	}
	strval = webs_get_var(wp, T("proxy_port"), T(""));
	if (NULL != strval)
	{
		pl_pjsip->sip_proxy.sip_port = atoi(strval);
	}
	strval = webs_get_var(wp, T("proxy_server_backup"), T(""));
	if (NULL != strval)
	{
		memset(pl_pjsip->sip_proxy_sec.sip_address, 0, sizeof(pl_pjsip->sip_proxy_sec.sip_address));
		strcpy(pl_pjsip->sip_proxy_sec.sip_address, strval);
	}
	strval = webs_get_var(wp, T("proxy_port_backup"), T(""));
	if (NULL != strval)
	{
		pl_pjsip->sip_proxy_sec.sip_port = atoi(strval);
	}

	strval = webs_get_var(wp, T("sip_100_rel"), T(""));
	if (NULL != strval)
	{
		pl_pjsip->sip_100_rel = zpl_true;
	}

	if(pl_pjsip->mutex)
		os_mutex_unlock(pl_pjsip->mutex);
	strval = webs_get_var(wp, T("pcmu_8000"), T(""));
	if (NULL != strval)
	{
		pl_pjsip_discodec_del_api("pcmu");
		pl_pjsip_codec_add_api("pcmu");

/*		pl_pjsip->codec[0].is_active = zpl_true;
		pl_pjsip->codec[0].payload = voip_sip_payload_index("PCMU");
		memset(pl_pjsip->codec[0].payload_name, 0, sizeof(pl_pjsip->codec[0].payload_name));
		strcpy(pl_pjsip->codec[0].payload_name,"PCMU");*/
	}
	else
	{
		pl_pjsip_codec_del_api("pcmu");
		pl_pjsip_discodec_add_api("pcmu");
	}
	strval = webs_get_var(wp, T("pcma_8000"), T(""));
	if (NULL != strval)
	{
		pl_pjsip_discodec_del_api("pcma");
		pl_pjsip_codec_add_api("pcma");
/*		pl_pjsip->codec[0].is_active = zpl_true;
		pl_pjsip->codec[0].payload = voip_sip_payload_index("PCMA");
		memset(pl_pjsip->codec[0].payload_name, 0, sizeof(pl_pjsip->codec[0].payload_name));
		strcpy(pl_pjsip->codec[0].payload_name,"PCMA");*/
	}
	else
	{
		pl_pjsip_codec_del_api("pcma");
		pl_pjsip_discodec_add_api("pcma");
	}
	strval = webs_get_var(wp, T("gsm_8000"), T(""));
	if (NULL != strval)
	{
		pl_pjsip_discodec_del_api("GSM");
		pl_pjsip_codec_add_api("GSM");
/*		pl_pjsip->codec[0].is_active = zpl_true;
		pl_pjsip->codec[0].payload = voip_sip_payload_index("GSM");
		memset(pl_pjsip->codec[0].payload_name, 0, sizeof(pl_pjsip->codec[0].payload_name));
		strcpy(pl_pjsip->codec[0].payload_name,"GSM");*/
	}
	else
	{
		pl_pjsip_codec_del_api("GSM");
		pl_pjsip_discodec_add_api("GSM");
	}
	strval = webs_get_var(wp, T("g722_8000"), T(""));
	if (NULL != strval)
	{
		pl_pjsip_discodec_del_api("G722");
		pl_pjsip_codec_add_api("G722");
/*		pl_pjsip->codec[0].is_active = zpl_true;
		pl_pjsip->codec[0].payload = voip_sip_payload_index("G722");
		memset(pl_pjsip->codec[0].payload_name, 0, sizeof(pl_pjsip->codec[0].payload_name));
		strcpy(pl_pjsip->codec[0].payload_name,"G722");*/
	}
	else
	{
		pl_pjsip_codec_del_api("G722");
		pl_pjsip_discodec_add_api("G722");
	}
	strval = webs_get_var(wp, T("speex_8000"), T(""));
	if (NULL != strval)
	{
		pl_pjsip_discodec_del_api("speex-nb");
		pl_pjsip_codec_add_api("speex-nb");
/*		pl_pjsip->codec[0].is_active = zpl_true;
		pl_pjsip->codec[0].payload = voip_sip_payload_index("SPEEX-NB");
		memset(pl_pjsip->codec[0].payload_name, 0, sizeof(pl_pjsip->codec[0].payload_name));
		strcpy(pl_pjsip->codec[0].payload_name,"SPEEX-NB");*/
	}
	else
	{
		pl_pjsip_codec_del_api("speex-nb");
		pl_pjsip_discodec_add_api("speex-nb");
	}
	strval = webs_get_var(wp, T("ilbc_8000"), T(""));
	if (NULL != strval)
	{
		pl_pjsip_discodec_del_api("ilbc");
		pl_pjsip_codec_add_api("ilbc");
/*		pl_pjsip->codec[0].is_active = zpl_true;
		pl_pjsip->codec[0].payload = voip_sip_payload_index("iLBC");
		memset(pl_pjsip->codec[0].payload_name, 0, sizeof(pl_pjsip->codec[0].payload_name));
		strcpy(pl_pjsip->codec[0].payload_name,"iLBC");*/
	}
	else
	{
		pl_pjsip_codec_del_api("ilbc");
		pl_pjsip_discodec_add_api("ilbc");
	}

	if(pl_pjsip->mutex)
		os_mutex_lock(pl_pjsip->mutex, OS_WAIT_FOREVER);

	strval = webs_get_var(wp, T("stun_server"), T(""));
	if (NULL != strval)
	{
		memset(pl_pjsip->sip_stun_server.sip_address, 0, sizeof(pl_pjsip->sip_stun_server.sip_address));
		strcpy(pl_pjsip->sip_stun_server.sip_address, strval);
	}
	strval = webs_get_var(wp, T("stun_port"), T(""));
	if (NULL != strval)
	{
		pl_pjsip->sip_stun_server.sip_port = atoi(strval);
	}
	strval = webs_get_var(wp, T("stun_user"), T(""));
	if (NULL != strval)
	{
		memset(pl_pjsip->sip_turn_user, 0, sizeof(pl_pjsip->sip_turn_user));
		strcpy(pl_pjsip->sip_turn_user, strval);
	}
	strval = webs_get_var(wp, T("stun_password"), T(""));
	if (NULL != strval)
	{
		memset(pl_pjsip->sip_turn_password, 0, sizeof(pl_pjsip->sip_turn_password));
		strcpy(pl_pjsip->sip_turn_password, strval);
	}

	strval = webs_get_var(wp, T("outbound_server"), T(""));
	if (NULL != strval)
	{
		memset(pl_pjsip->sip_outbound.sip_address, 0, sizeof(pl_pjsip->sip_outbound.sip_address));
		strcpy(pl_pjsip->sip_outbound.sip_address, strval);
	}
	strval = webs_get_var(wp, T("outbound_port"), T(""));
	if (NULL != strval)
	{
		pl_pjsip->sip_outbound.sip_port = atoi(strval);
	}

	strval = webs_get_var(wp, T("nameserver_server"), T(""));
	if (NULL != strval)
	{
		memset(pl_pjsip->sip_nameserver.sip_address, 0, sizeof(pl_pjsip->sip_nameserver.sip_address));
		strcpy(pl_pjsip->sip_nameserver.sip_address, strval);
	}
	strval = webs_get_var(wp, T("nameserver_port"), T(""));
	if (NULL != strval)
	{
		pl_pjsip->sip_nameserver.sip_port = atoi(strval);
	}

	if(pl_pjsip->mutex)
		os_mutex_unlock(pl_pjsip->mutex);
	return OK;
}

static int web_sip_set(Webs *wp, char *path, char *query)
{
	char *strval = NULL;
	char buf[1024];
	strval = webs_get_var(wp, T("ACTION"), T(""));
	if (NULL == strval)
	{
		return web_return_text_plain(wp, ERROR);
	}
	if(strstr(strval ,"GET"))
	{
		websSetStatus(wp, 200);
		websWriteHeaders(wp, -1, 0);
		websWriteHeader(wp, "Content-Type", "application/json");
		websWriteEndHeaders(wp);

		memset(buf, 0, sizeof(buf));
		web_sip_config_get(buf);

		websWrite(wp, "{%s}",buf);

		websDone(wp);
		return OK;
	}

	if(web_sip_config_set(wp) == OK)
	{
		_WEB_DBG_TRAP("%s: ------_pjsip->proto = %d\r\n", __func__,pl_pjsip->proto);
#ifdef ZPL_OPENWRT_UCI
		voip_uci_sip_config_save(pl_pjsip);
		voip_stream_config_save(pl_pjsip);
#endif
		pjsua_app_restart();
		//vty_execute_shell("write memory");
	}

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "application/json");
	websWriteEndHeaders(wp);

	memset(buf, 0, sizeof(buf));
	web_sip_config_get(buf);

	websWrite(wp, "{%s}",buf);

	websDone(wp);

	return OK;
}

static int web_sipstate(Webs *wp, char *path, char *query)
{
	char sipregcnt = 0, sipurl[20], sipreg[20];
	pl_pjsip_t *sip = pl_pjsip;
	zassert(pl_pjsip != NULL);

	websSetStatus(wp, 200);
	websWriteHeaders(wp, -1, 0);
	websWriteHeader(wp, "Content-Type", "text/plain");
	websWriteEndHeaders(wp);
	websWrite(wp, "%s", "[");

	if(pl_pjsip->mutex)
		os_mutex_lock(pl_pjsip->mutex, OS_WAIT_FOREVER);

	if(strlen(sip->sip_user.sip_user) || strlen(sip->sip_user.sip_phone))
	{
		memset(sipurl, 0, sizeof(sipurl));
		memset(sipreg, 0, sizeof(sipreg));

		if(strlen(sip->sip_server.sip_address))
		{
			snprintf(sipurl, sizeof(sipurl), "%s:%d", sip->sip_server.sip_address,
					sip->sip_server.sip_port);
		}
		//snprintf(sippro, sizeof(sippro), "sip");
		if(sip->sip_user.sip_state == PJSIP_STATE_UNREGISTER)
			snprintf(sipreg, sizeof(sipreg), "unregister");
		else if(sip->sip_user.sip_state == PJSIP_STATE_REGISTER_FAILED)
			snprintf(sipreg, sizeof(sipreg), "failed");
		else if(sip->sip_user.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			snprintf(sipreg, sizeof(sipreg), "register");
		else
			snprintf(sipreg, sizeof(sipreg), "unknown");

		websWrite(wp, "{\"sip_number\":\"%s\", \"serverip\":\"%s\", \
				\"dtmf\":\"%s\", \"codec\":\"%s\", \"state\":\"%s\"}",
				sip->sip_user.sip_user, sipurl, pl_pjsip_dtmf_name(sip->dtmf),
				sip->sip_codec.payload_name, sipreg);
		sipregcnt++;
	}
	if(strlen(sip->sip_user_sec.sip_user) || strlen(sip->sip_user_sec.sip_phone))
	{
		memset(sipurl, 0, sizeof(sipurl));
		memset(sipreg, 0, sizeof(sipreg));

		if(strlen(sip->sip_server_sec.sip_address))
		{
			snprintf(sipurl, sizeof(sipurl), "%s:%d", sip->sip_server_sec.sip_address,
					sip->sip_server_sec.sip_port);
		}
		//snprintf(sippro, sizeof(sippro), "sip");
		if(sip->sip_user_sec.sip_state == PJSIP_STATE_UNREGISTER)
			snprintf(sipreg, sizeof(sipreg), "unregister");
		else if(sip->sip_user_sec.sip_state == PJSIP_STATE_REGISTER_FAILED)
			snprintf(sipreg, sizeof(sipreg), "failed");
		else if(sip->sip_user_sec.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			snprintf(sipreg, sizeof(sipreg), "register");
		else
			snprintf(sipreg, sizeof(sipreg), "unknown");

		if(sipregcnt)
			websWrite(wp, "%s", ",");
		websWrite(wp, "{\"sip_number\":\"%s\", \"serverip\":\"%s\", \
				\"dtmf\":\"%s\", \"codec\":\"%s\", \"state\":\"%s\"}",
				sip->sip_user.sip_user, sipurl, pl_pjsip_dtmf_name(sip->dtmf),
				sip->sip_codec.payload_name, sipreg);
	}
	if(pl_pjsip->mutex)
		os_mutex_unlock(pl_pjsip->mutex);

	websWrite(wp, "%s", "]");
	websDone(wp);
	return 0;
}


static int web_sip_register(Webs *wp, void *p)
{
	char *btnid = NULL;
	btnid = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL == btnid)
	{
		return ERROR;//web_return_text_plain(wp, ERROR);
	}
	if(strstr(btnid, "register"))
	{
		if(!pl_pjsip_isregister_api())
			voip_app_sip_register_start(zpl_true);
	}
	if(strstr(btnid, "unregister"))
	{
		if(pl_pjsip_isregister_api())
			voip_app_sip_register_start(zpl_false);
	}
	return web_return_text_plain(wp, OK);
	//return OK;
}

static int web_volume_change(Webs *wp, void *p)
{
	char *btnid = NULL;
	char *value = NULL;
	btnid = webs_get_var(wp, T("BTNID"), T(""));
	if (NULL == btnid)
	{
		_WEB_DBG_TRAP("%s: can not get BTNID\r\n", __func__);
		return ERROR;//web_return_text_plain(wp, ERROR);
	}
	if(strstr(btnid, "out_"))
	{
		value = webs_get_var(wp, T("volume"), T(""));
		if (NULL != value)
			voip_playback_volume_out_set_api(atoi(value));
		else
		{
			_WEB_DBG_TRAP("%s: can not get out volume\r\n", __func__);
			return ERROR;//web_return_text_plain(wp, ERROR);
		}
		return web_return_text_plain(wp, OK);
	}
	if(strstr(btnid, "in_"))
	{
		value = webs_get_var(wp, T("volume"), T(""));
		if (NULL != value)
			voip_capture_volume_adc_set_api(atoi(value));
		else
		{
			_WEB_DBG_TRAP("%s: can not get in volume\r\n", __func__);
			return ERROR;//web_return_text_plain(wp, ERROR);
		}
		return web_return_text_plain(wp, OK);
	}
	return ERROR;//web_return_text_plain(wp, OK);
}
#endif


int web_sip_app(void)
{
#ifdef ZPL_PJSIP_MODULE
	websFormDefine("setsip", web_sip_set);
	websFormDefine("sipstate", web_sipstate);

	web_button_add_hook("sip", "register", web_sip_register, NULL);//断开连接
	web_button_add_hook("sip", "unregister", web_sip_register, NULL);//扫描附近wifi

	web_button_add_hook("volume", "out_vol", web_volume_change, NULL);//断开连接
	web_button_add_hook("volume", "in_vol", web_volume_change, NULL);//扫描附近wifi
#endif
	return 0;
}




