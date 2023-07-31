/*
 * pjsip_app_api.c
 *
 *  Created on: Jun 15, 2019
 *      Author: zhurish
 */
#include "auto_include.h"
#include <zplos_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"

#include "pjsua_app_common.h"
#include "pjsua_app.h"
#include "pjsip_app_api.h"
#include "pjsip_util.h"


pjapp_cfg_t *_pjapp_cfg = NULL;


int pjapp_cfg_source_change(struct interface *ifp, zpl_bool change)
{
	if(_pjapp_cfg && ifp && ifp->ifindex > 0 && ifp->ifindex == _pjapp_cfg->sip_source_interface)
	{
		if(change)
		{
			zpl_uint32 address = voip_get_address(_pjapp_cfg->sip_source_interface);
			if(address)
			{
				memset(_pjapp_cfg->sip_local.sip_address, 0, sizeof(_pjapp_cfg->sip_local.sip_address));
				strcpy(_pjapp_cfg->sip_local.sip_address, inet_address(address));
				_pjapp_cfg->sip_local.state = PJSIP_STATE_CONNECT_LOCAL;
				//pjapp_cfg_bound_address(&_global_config, _pjapp_cfg->sip_local.sip_address);
			}
			//pjsua_app_restart(&app_config);
		}
		else
		{
			memset(_pjapp_cfg->sip_local.sip_address, 0, sizeof(_pjapp_cfg->sip_local.sip_address));
			//pjapp_cfg_bound_address(&_global_config, _pjapp_cfg->sip_local.sip_address);
		}
	}
	return OK;
}

int pjapp_cfg_config_default(pjapp_cfg_t *sip)
{
	zassert(sip != NULL);
	memset(&_global_config, 0, sizeof(_global_config));
	//nsm_hook_install (NSM_HOOK_IFP_CHANGE, pjapp_cfg_source_change);

	sip->sip_enable		= PJSIP_ENABLE_DEFAULT;
	sip->sip_server.sip_port = PJSIP_PORT_DEFAULT;
	sip->sip_server_sec.sip_port = PJSIP_PORT_DEFAULT;

	sip->sip_proxy.sip_port = PJSIP_PROXY_PORT_DEFAULT;
	sip->sip_proxy_sec.sip_port = PJSIP_PROXY_PORT_DEFAULT;

	sip->sip_local.sip_port = PJSIP_PORT_DEFAULT;

	sip->sip_expires = PJSIP_EXPIRES_DEFAULT;
	sip->sip_100_rel = PJSIP_100_REL_DEFAULT;

	sip->dtmf = PJSIP_DTMF_DEFAULT;
	sip->proto = PJSIP_PROTO_DEFAULT;
	sip->pjsip = &_global_config;
/*
	int (*app_dtmf_cb)(int , int);
	void				*mutex;
*/
	sip->sip_proxy_enable = zpl_false;

	//"SIP Account options:"
	strcpy(sip->sip_realm, "*");

	//sip->sip_reg_timeout = PJSUA_REG_INTERVAL;		//Optional registration interval (default %d)
	sip->sip_rereg_delay = PJSUA_REG_RETRY_INTERVAL;		//Optional auto retry registration interval (default %d)
	sip->sip_reg_proxy = PJSIP_REGISTER_ALL;			//Control the use of proxy settings in REGISTER.0=no proxy, 1=outbound only, 2=acc only, 3=all (default)
	sip->sip_publish = zpl_false;			//Send presence PUBLISH for this account
	sip->sip_mwi = zpl_false;				//Subscribe to message summary/waiting indication
	sip->sip_ims_enable = zpl_false;			//Enable 3GPP/IMS related settings on this account
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
	sip->sip_srtp_mode = PJSIP_SRTP_DISABLE;			//Use SRTP?  0:disabled, 1:optional, 2:mandatory,3:optional by duplicating media offer (def:0)
	sip->sip_srtp_secure = PJSIP_SRTP_SEC_TLS;		//SRTP require secure SIP? 0:no, 1:tls, 2:sips (def:1)
	sip->sip_srtp_keying = PJSIP_SRTP_KEYING_SDES;		//SRTP keying method for outgoing SDP offer.0=SDES (default), 1=DTLS
#endif
	sip->sip_timer = PJSIP_TIMER_OPTIONAL;				//Use SIP session timers? (default=1) 0:inactive, 1:optional, 2:mandatory, 3:always"
	sip->sip_timer_sec = PJSIP_SESS_TIMER_DEF_SE;
	sip->sip_outb_rid = 1;			//Set SIP outbound reg-id (default:1)
	sip->sip_auto_update_nat = zpl_true;	//Where N is 0 or 1 to enable/disable SIP traversal behind symmetric NAT (default 1)
	sip->sip_stun_disable = zpl_true;		//Disable STUN for this account


	//Transport Options:
#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
	sip->sip_ipv6_enable = zpl_false;
#endif
	sip->sip_set_qos = zpl_true;
	sip->sip_noudp = zpl_false;
	sip->sip_notcp = zpl_true;

	//pjsip_server_t		sip_nameserver;			//Add the specified nameserver to enable SRV resolution This option can be specified multiple times.
	//pjsip_server_t		sip_outbound;			//Set the URL of global outbound proxy server May be specified multiple times
	//pjsip_server_t		sip_stun_server;		//Set STUN server host or domain. This option may be specified more than once. FORMAT is hostdom[:PORT]

	//TLS Options:
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
	sip->sip_tls_enable = zpl_false;
	/*
	char				sip_tls_ca_file[PJSIP_FILE_MAX];		//Specify TLS CA file (default=none)
	char				sip_tls_cert_file[PJSIP_FILE_MAX];		//Specify TLS certificate file (default=none)
	char				sip_tls_privkey_file[PJSIP_FILE_MAX];	//Specify TLS private key file (default=none)
	char				sip_tls_password[PJSIP_PASSWORD_MAX];	//Specify TLS password to private key file (default=none)
	pjsip_server_t		sip_tls_verify_server;					//Verify server's certificate (default=no)
	pjsip_server_t		sip_tls_verify_client;					//Verify client's certificate (default=no)
	zpl_uint16				sip_neg_timeout;						//Specify TLS negotiation timeout (default=no)
	char				sip_tls_cipher[PJSIP_DATA_MAX];			//Specify prefered TLS cipher (optional).May be specified multiple times
	*/
#endif

	//Audio Options:
	//char				sip_codec[PJSIP_DATA_MAX];
	//char				sip_discodec[PJSIP_DATA_MAX];
	sip->sip_clock_rate = PJSIP_DEFAULT_CLOCK_RATE;
	sip->sip_snd_clock_rate = PJSIP_DEFAULT_CLOCK_RATE;
	sip->sip_stereo = zpl_false;
	sip->sip_audio_null = zpl_false;
	//char				sip_play_file[PJSIP_FILE_MAX];
	//char				sip_play_tone[PJSIP_DATA_MAX];
	sip->sip_auto_play = zpl_false;
	sip->sip_auto_loop = zpl_false;
	sip->sip_auto_conf = zpl_true;



	//strcpy(sip->sip_rec_file, "/tmp/aa.wav");

	sip->sip_quality = PJSUA_DEFAULT_CODEC_QUALITY;							//Specify media quality (0-10, default=2)
	sip->sip_ptime = PJSUA_DEFAULT_AUDIO_FRAME_PTIME;								//Override codec ptime to MSEC (default=specific)
	sip->sip_no_vad = zpl_false;												//Disable VAD/silence detector (default=vad enabled)
	sip->sip_echo_tail = PJSUA_DEFAULT_EC_TAIL_LEN;							//Set echo canceller tail length
	//sip->sip_echo_mode = PJSIP_ECHO_SPEEX;//PJSIP_ECHO_DEFAULT;//PJSIP_ECHO_DISABLE;	//Select echo canceller algorithm (0=default, 1=speex, 2=suppressor, 3=WebRtc)
	//sip->sip_echo_mode = PJSIP_ECHO_SUPPRESSER;
	sip->sip_echo_mode = PJSIP_ECHO_DEFAULT;
	sip->sip_ilbc_mode = PJSUA_DEFAULT_ILBC_MODE;							//Set iLBC codec mode (20 or 30, default is 20)
	sip->sip_capture_lat = PJMEDIA_SND_DEFAULT_REC_LATENCY;						//Audio capture latency, in ms
	sip->sip_playback_lat = PJMEDIA_SND_DEFAULT_PLAY_LATENCY;						//Audio capture latency, in ms
	sip->sip_snd_auto_close = 0;						//Auto close audio device when idle for N secs (default=1)
																//		Specify N=-1 to disable this feature. Specify N=0 for instant close when unused.
	sip->sip_notones = zpl_false;//回铃音							//Disable audible tones
	sip->sip_jb_max_size = -1;						//指定最大值抖动缓冲(帧，默认= 1)Specify jitter buffer maximum size, in frames (default=-1)");
    strcpy(sip->capture_dev_name, "null device");
    strcpy(sip->playback_dev_name, "null device");
#if PJSUA_HAS_VIDEO
	//Video Options:
	sip->sip_video = zpl_true;
	//zpl_uint32				sip_vcapture_dev;
	//zpl_uint32				sip_vrender_dev;
	//char				sip_play_avi[PJSIP_FILE_MAX];
	sip->sip_auto_play_avi = zpl_false;

    strcpy(sip->vcapture_dev_name, "null device");
    strcpy(sip->vrender_dev_name, "null device");		
#endif
	//Media Transport Options:
	sip->sip_ice = zpl_false;				//Enable ICE (default:no)
	sip->sip_ice_regular = zpl_false;		//Use ICE regular nomination (default: aggressive)
	sip->sip_ice_max_host = 5;		//Set maximum number of ICE host candidates
	sip->sip_ice_nortcp = zpl_false;			//Disable RTCP component in ICE (default: no)
	sip->sip_rtp_port = PJSIP_RTP_PORT_DEFAULT;
	sip->sip_rx_drop_pct = 0;		//Drop PCT percent of RX RTP (for pkt lost sim, default: 0)
	sip->sip_tx_drop_pct = 0;		//Drop PCT percent of TX RTP (for pkt lost sim, default: 0)
	sip->sip_turn = zpl_false;
	//zpl_bool				sip_turn;				//Enable TURN relay with ICE (default:no)
	//pjsip_server_t		sip_turn_srv;			//Domain or host name of TURN server (\"NAME:PORT\" format)
	sip->sip_turn_tcp = zpl_false;
	//zpl_bool				sip_turn_tcp;			//Use TCP connection to TURN server (default no)
	//char				sip_turn_user[PJSIP_USERNAME_MAX];
	//char				sip_turn_password[PJSIP_PASSWORD_MAX];
	sip->sip_rtcp_mux = zpl_false;
	//zpl_uint16				sip_rtcp_mux;			//Enable RTP & RTCP multiplexing (default: no)
	//Buddy List (can be more than one):
	//void				*buddy_list;
	//User Agent options:
	sip->sip_auto_answer_code = 200;	//Automatically answer incoming calls with code (e.g. 200)
	sip->sip_max_calls = PJSUA_MAX_CALLS;			//Maximum number of concurrent calls (default:4, max:255)
	sip->sip_thread_max = 1;			//Number of worker threads (default:1)
	sip->sip_duration = PJSUA_APP_NO_LIMIT_DURATION;			//设置最大通话时间（默认是：没有限制）Set maximum call duration (default:no limit)
	sip->sip_norefersub = 1;//转接通话时禁止事件订阅			//Suppress event subscription when transferring calls

	sip->sip_use_compact_form = 0;	//Minimize SIP message size 最小的SIP消息大小
	sip->sip_no_force_lr = 0;		//允许使用严格路由Allow strict-route to be used (i.e. do not force lr)
	sip->sip_accept_redirect = PJSIP_REDIRECT_ACCEPT_REPLACE;	//Specify how to handle call redirect (3xx) response.
												//	0: reject, 1: follow automatically,
												//	2: follow + replace To header (default), 3: ask

/*	strcpy(_pjapp_cfg->sip_codec.payload_name, "PCMU");
	_pjapp_cfg->sip_codec.is_active = zpl_true;

	strcpy(_pjapp_cfg->codec[0].payload_name, "PCMU");
	_pjapp_cfg->codec[0].is_active = zpl_true;
	strcpy(_pjapp_cfg->codec[1].payload_name, "PCMA");
	_pjapp_cfg->codec[1].is_active = zpl_true;
	strcpy(_pjapp_cfg->codec[2].payload_name, "GSM");
	_pjapp_cfg->codec[2].is_active = zpl_true;
	strcpy(_pjapp_cfg->codec[3].payload_name, "G722");
	_pjapp_cfg->codec[3].is_active = zpl_true;

	strcpy(_pjapp_cfg->dicodec[0].payload_name, "speex/8000");
	_pjapp_cfg->dicodec[0].is_active = zpl_true;
	strcpy(_pjapp_cfg->dicodec[1].payload_name, "speex/16000");
	_pjapp_cfg->dicodec[1].is_active = zpl_true;
	strcpy(_pjapp_cfg->dicodec[2].payload_name, "speex/32000");
	_pjapp_cfg->dicodec[2].is_active = zpl_true;
	strcpy(_pjapp_cfg->dicodec[3].payload_name, "iLBC/8000");
	_pjapp_cfg->dicodec[3].is_active = zpl_true;*/

	pjapp_cfg_codec_default_set_api("pcmu");
	pjapp_cfg_codec_add_api("pcmu");
	pjapp_cfg_codec_add_api("pcma");
	//pjapp_cfg_codec_add_api("gsm");
	//pjapp_cfg_codec_add_api("g722");
	//pjapp_cfg_discodec_add_api("speex-nb");
	//pjapp_cfg_discodec_add_api("speex-wb");
	pjapp_cfg_discodec_add_api("ilbc");

#if defined(ZPL_BUILD_ARCH_X86)||defined(ZPL_BUILD_ARCH_X86_64)
	strcpy(sip->sip_user.sip_user, "100");
	strcpy(sip->sip_user.sip_password, "100");
	strcpy(sip->sip_server.sip_address, "192.168.10.102");

	strcpy(sip->sip_local.sip_address, "192.168.10.100");
	sip->sip_local.state = PJSIP_STATE_CONNECT_LOCAL;
#else
	strcpy(sip->sip_user.sip_user, "100");
	strcpy(sip->sip_user.sip_password, "100");
	strcpy(sip->sip_server.sip_address, "192.168.10.102");

	strcpy(sip->sip_local.sip_address, "192.168.10.1");
	sip->sip_local.state = PJSIP_STATE_CONNECT_LOCAL;		
#endif

	sip->debug_level = ZLOG_LEVEL_ERR;
	sip->debug_detail = zpl_false;
/*
	pjapp_cfg_username(&_global_config, "100");//Set authentication username
	pjapp_cfg_password(&_global_config, "100");//Set authentication password
	pjapp_cfg_registrar(&_global_config, "sip:192.168.224.1");//Set the URL of registrar server
	pjapp_cfg_url_id(&_global_config, "sip:100@192.168.224.1");//Set the URL of local ID (used in From header)
*/
	return OK;
}

/***************************************************************************************/
/***************************************************************************************/
char *pjapp_cfg_dtmf_name(pjsip_dtmf_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_DTMF_INFO:
		return "SIP-INFO";
		break;
	case PJSIP_DTMF_RFC2833:
		return "RFC2833";
		break;
	case PJSIP_DTMF_INBAND:
		return "INBAND";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_transport_name(pjsip_transport_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_PROTO_UDP:
		return "UDP";
		break;
	case PJSIP_PROTO_TCP:
		return "TCP";
		break;
	case PJSIP_PROTO_TLS:
		return "TLS";
		break;
	case PJSIP_PROTO_DTLS:
		return "DTLS";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_reg_proxy_name(pjsip_reg_proxy_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_REGISTER_NONE:
		return "NONE";
		break;
	case PJSIP_REGISTER_NO_PROXY:
		return "NO-PROXY";
		break;
	case PJSIP_REGISTER_OUTBOUND_PROXY:
		return "OUTBOUND-PROXY";
		break;
	case PJSIP_REGISTER_ACC_ONLY:
		return "ACC-ONLY";
		break;
	case PJSIP_REGISTER_ALL:
		return "ALL";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_srtp_name(pjsip_srtp_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_SRTP_DISABLE:
		return "DISABLE";
		break;
	case PJSIP_SRTP_OPTIONAL:
		return "OPTIONAL";
		break;
	case PJSIP_SRTP_MANDATORY:
		return "MANDATORY";
		break;
	case PJSIP_SRTP_OPTIONAL_DUP:
		return "OPTIONAL-DUP";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_srtp_sec_name(pjsip_srtp_sec_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_SRTP_SEC_NO:
		return "NO";
		break;
	case PJSIP_SRTP_SEC_TLS:
		return "SEC-TLS";
		break;
	case PJSIP_SRTP_SEC_SIPS:
		return "SEC-SIPS";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_timer_name(pjsip_timer_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_TIMER_INACTIVE:
		return "INACTIVE";
		break;
	case PJSIP_TIMER_OPTIONAL:
		return "OPTIONAL";
		break;
	case PJSIP_TIMER_MANDATORY:
		return "MANDATORY";
		break;
	case PJSIP_TIMER_ALWAYS:
		return "ALWAYS";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_echo_mode_name(pjsip_echo_mode_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_ECHO_DEFAULT:
		return "DEFAULT";
		break;
	case PJSIP_ECHO_SPEEX:
		return "SPEEX";
		break;
	case PJSIP_ECHO_SUPPRESSER:
		return "SUPPRESSER";
		break;
	case PJSIP_ECHO_WEBRTXC:
		return "WEBRTXC";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_accept_redirect_name(pjsip_accept_redirect_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_ACCEPT_REDIRECT_REJECT:
		return "REDIRECT-REJECT";
		break;
	case PJSIP_ACCEPT_REDIRECT_FOLLOW:
		return "REDIRECT-FOLLOW";
		break;
	case PJSIP_ACCEPT_REDIRECT_FOLLOW_REPLACE:
		return "REDIRECT-FOLLOW-REPLACE";
		break;
	case PJSIP_ACCEPT_REDIRECT_ASK:
		return "REDIRECT-ASK";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_register_state_name(pjsip_register_state_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_STATE_UNREGISTER:
		return "UNREGISTER";
		break;
	case PJSIP_STATE_REGISTER_FAILED:
		return "REGISTER-FAILED";
		break;
	case PJSIP_STATE_REGISTER_SUCCESS:
		return "REGISTER-SUCCESS";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_connect_state_name(pjsip_connect_state_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_STATE_UNCONNECT:
		return "UNCONNECT";
		break;
	case PJSIP_STATE_CONNECT_FAILED:
		return "CONNECT-FAILED";
		break;
	case PJSIP_STATE_CONNECT_SUCCESS:
		return "CONNECT-SUCCESS";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

char *pjapp_cfg_call_state_name(pjsip_call_state_t dtmf)
{
	switch(dtmf)
	{
	case PJSIP_STATE_CALL_IDLE:
		return "IDLE";
		break;
	case PJSIP_STATE_CALL_TRYING:
		return "TRYING";
		break;
	case PJSIP_STATE_CALL_RINGING:
		return "RINGING";
		break;
	case PJSIP_STATE_CALL_PICKING:
		return "PICKING";
		break;
	case PJSIP_STATE_CALL_FAILED:
		return "FAILED";
		break;
	case PJSIP_STATE_CALL_SUCCESS:
		return "SUCCESS";
		break;
	case PJSIP_STATE_CALL_CANCELLED:
		return "CANCELLED";
		break;
	case PJSIP_STATE_CALL_CLOSED:
		return "CLOSED";
		break;
	case PJSIP_STATE_CALL_RELEASED:
		return "RELEASED";
		break;
	default:
		return "UNKNOWN";
		break;
	}
}

/***************************************************************************************/
int pjapp_cfg_global_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_enable = enable;
	//voip_pjapp_cfg_update_api(_pjapp_cfg);
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_global_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

zpl_bool pjapp_cfg_global_isenable(void)
{
	zpl_bool enable = zpl_false;
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	enable = _pjapp_cfg->sip_enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return enable;
}

int pjapp_cfg_server_set_api(zpl_int8 *ip, zpl_uint16 port, zpl_bool sec)
{
	zassert(_pjapp_cfg != NULL);
	if (_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if (!sec)
	{
		memset(_pjapp_cfg->sip_server.sip_address, 0,
				sizeof(_pjapp_cfg->sip_server.sip_address));
		if (ip)
		{
			strcpy(_pjapp_cfg->sip_server.sip_address, ip);
			if (strlen(_pjapp_cfg->sip_server_sec.sip_address))
				_pjapp_cfg->sip_server_cnt = 2;
			else
				_pjapp_cfg->sip_server_cnt = 1;
		}
	}
	else
	{
		memset(_pjapp_cfg->sip_server_sec.sip_address, 0,
				sizeof(_pjapp_cfg->sip_server_sec.sip_address));
		if (ip)
		{
			strcpy(_pjapp_cfg->sip_server_sec.sip_address, ip);
			if (strlen(_pjapp_cfg->sip_server.sip_address))
				_pjapp_cfg->sip_server_cnt = 2;
			else
				_pjapp_cfg->sip_server_cnt = 1;
		}
	}
	if (sec)
		_pjapp_cfg->sip_server_sec.sip_port = port ? port : PJSIP_PORT_DEFAULT;
	else
		_pjapp_cfg->sip_server.sip_port = port ? port : PJSIP_PORT_DEFAULT;
	if (_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_server_get_api(zpl_int8 *ip, zpl_uint16 *port, zpl_bool sec)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sec)
	{
		if(ip)
			strcpy(ip, _pjapp_cfg->sip_server_sec.sip_address);
		if(port)
			*port = _pjapp_cfg->sip_server_sec.sip_port;
	}
	else
	{
		if(ip)
			strcpy(ip, _pjapp_cfg->sip_server.sip_address);
		if(port)
			*port = _pjapp_cfg->sip_server.sip_port;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_proxy_set_api(zpl_int8 *ip, zpl_uint16 port, zpl_bool sec)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sec)
	{
		_pjapp_cfg->sip_proxy_enable = zpl_true;
		_pjapp_cfg->sip_proxy_sec.sip_port = port ? port:PJSIP_PORT_DEFAULT;
		memset(_pjapp_cfg->sip_proxy_sec.sip_address, 0, sizeof(_pjapp_cfg->sip_proxy_sec.sip_address));
		if(ip)
		{
			strcpy(_pjapp_cfg->sip_proxy_sec.sip_address, ip);
			if(strlen(_pjapp_cfg->sip_proxy.sip_address))
				_pjapp_cfg->sip_proxy_cnt = 2;
			else
				_pjapp_cfg->sip_proxy_cnt = 1;
		}
	}
	else
	{
		_pjapp_cfg->sip_proxy_enable = zpl_true;
		_pjapp_cfg->sip_proxy_cnt = 1;
		_pjapp_cfg->sip_proxy.sip_port = port ? port:PJSIP_PORT_DEFAULT;
		memset(_pjapp_cfg->sip_proxy.sip_address, 0, sizeof(_pjapp_cfg->sip_proxy.sip_address));
		if(ip)
		{
			strcpy(_pjapp_cfg->sip_proxy.sip_address, ip);
			if(strlen(_pjapp_cfg->sip_proxy_sec.sip_address))
				_pjapp_cfg->sip_proxy_cnt = 2;
			else
				_pjapp_cfg->sip_proxy_cnt = 1;
		}
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_proxy_get_api(zpl_int8 *ip, zpl_uint16 *port, zpl_bool sec)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sec)
	{
		if(ip)
			strcpy(ip, _pjapp_cfg->sip_proxy_sec.sip_address);
		if(port)
			*port = _pjapp_cfg->sip_proxy_sec.sip_port;
	}
	else
	{
		if(ip)
			strcpy(ip, _pjapp_cfg->sip_proxy.sip_address);
		if(port)
			*port = _pjapp_cfg->sip_proxy.sip_port;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}


int pjapp_cfg_local_address_set_api(zpl_int8 *address)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	//_pjapp_cfg->sip_local.sip_port = port ? port:PJSIP_PORT_DEFAULT;
	memset(_pjapp_cfg->sip_local.sip_address, 0, sizeof(_pjapp_cfg->sip_local.sip_address));
	if(address)
	{
		strcpy(_pjapp_cfg->sip_local.sip_address, address);
		_pjapp_cfg->sip_local.state = PJSIP_STATE_CONNECT_LOCAL;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_local_address_get_api(zpl_int8 *address)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(address)
	{
		//memset(_pjapp_cfg->sip_local.sip_address, 0, sizeof(_pjapp_cfg->sip_local.sip_address));
		strcpy(address, _pjapp_cfg->sip_local.sip_address);
		//_pjapp_cfg->sip_local.state = PJSIP_STATE_CONNECT_LOCAL;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_source_interface_set_api(ifindex_t ifindex)
{
	zpl_uint32 address = 0;
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_source_interface = ifindex;
	if(_pjapp_cfg->sip_source_interface)
		address = voip_get_address(_pjapp_cfg->sip_source_interface);
	if(address)
	{
		memset(_pjapp_cfg->sip_local.sip_address, 0, sizeof(_pjapp_cfg->sip_local.sip_address));
		strcpy(_pjapp_cfg->sip_local.sip_address, inet_address(address));
		_pjapp_cfg->sip_local.state = PJSIP_STATE_CONNECT_LOCAL;
	}
	//voip_pjapp_cfg_update_api(_pjapp_cfg);
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_source_interface_get_api(ifindex_t *ifindex)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(ifindex)
		*ifindex = _pjapp_cfg->sip_source_interface;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_local_port_set_api(zpl_uint16 port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_local.sip_port = port ? port:PJSIP_PORT_DEFAULT;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}
int pjapp_cfg_local_port_get_api(zpl_uint16 *port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(port)
		*port = _pjapp_cfg->sip_local.sip_port;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}


int pjapp_cfg_transport_proto_set_api(pjsip_transport_t proto)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->proto = proto;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_transport_proto_get_api(pjsip_transport_t *proto)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(proto)
		*proto = _pjapp_cfg->proto;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}


int pjapp_cfg_dtmf_set_api(pjsip_dtmf_t dtmf)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->dtmf = dtmf;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_dtmf_get_api(pjsip_dtmf_t *dtmf)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(dtmf)
		*dtmf = _pjapp_cfg->dtmf;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_username_set_api(zpl_int8 *user, zpl_int8 *pass, zpl_bool sec)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sec)
	{
		memset(_pjapp_cfg->sip_user_sec.sip_user, 0, sizeof(_pjapp_cfg->sip_user_sec.sip_user));
		memset(_pjapp_cfg->sip_user_sec.sip_password, 0, sizeof(_pjapp_cfg->sip_user_sec.sip_password));
		if(user)
		{
			strcpy(_pjapp_cfg->sip_user_sec.sip_user, user);
			if(strlen(_pjapp_cfg->sip_user.sip_user))
				_pjapp_cfg->sip_user_cnt = 2;
			else
				_pjapp_cfg->sip_user_cnt = 1;
		}
		if(pass)
		{
			strcpy(_pjapp_cfg->sip_user_sec.sip_password, pass);
		}
	}
	else
	{
		memset(_pjapp_cfg->sip_user.sip_user, 0, sizeof(_pjapp_cfg->sip_user.sip_user));
		memset(_pjapp_cfg->sip_user.sip_password, 0, sizeof(_pjapp_cfg->sip_user.sip_password));
		if(user)
		{
			strcpy(_pjapp_cfg->sip_user.sip_user, user);
			if(strlen(_pjapp_cfg->sip_user_sec.sip_user))
				_pjapp_cfg->sip_user_cnt = 2;
			else
				_pjapp_cfg->sip_user_cnt = 1;
		}
		if(pass)
		{
			strcpy(_pjapp_cfg->sip_user.sip_password, pass);
		}
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_username_get_api(zpl_int8 *user, zpl_int8 *pass, zpl_bool sec)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sec)
	{
		if(user)
			strcpy(user, _pjapp_cfg->sip_user_sec.sip_user);
		if(pass)
			strcpy(pass, _pjapp_cfg->sip_user_sec.sip_password);
	}
	else
	{
		if(user)
			strcpy(user, _pjapp_cfg->sip_user.sip_user);
		if(pass)
			strcpy(pass, _pjapp_cfg->sip_user.sip_password);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}


int pjapp_cfg_phonenumber_set_api(zpl_int8 *sip_phone, zpl_bool sec)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sec)
	{
		memset(_pjapp_cfg->sip_user_sec.sip_phone, 0, sizeof(_pjapp_cfg->sip_user_sec.sip_phone));
		if(sip_phone)
		{
			strcpy(_pjapp_cfg->sip_user_sec.sip_phone, sip_phone);
			if(strlen(_pjapp_cfg->sip_user.sip_phone))
				_pjapp_cfg->sip_user_cnt = 2;
			else
				_pjapp_cfg->sip_user_cnt = 1;
		}
	}
	else
	{
		memset(_pjapp_cfg->sip_user.sip_phone, 0, sizeof(_pjapp_cfg->sip_user.sip_phone));
		if(sip_phone)
		{
			strcpy(_pjapp_cfg->sip_user.sip_phone, sip_phone);
			if(strlen(_pjapp_cfg->sip_user_sec.sip_phone))
				_pjapp_cfg->sip_user_cnt = 2;
			else
				_pjapp_cfg->sip_user_cnt = 1;
		}
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_phonenumber_get_api(zpl_int8 *sip_phone, zpl_bool sec)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sec)
	{
		if(sip_phone)
			strcpy(sip_phone, _pjapp_cfg->sip_user_sec.sip_phone);
	}
	else
	{
		if(sip_phone)
			strcpy(sip_phone, _pjapp_cfg->sip_user.sip_phone);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_expires_set_api(zpl_uint16 sip_expires)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_expires = sip_expires;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_expires_get_api(zpl_uint16 *sip_expires)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_expires)
		*sip_expires = _pjapp_cfg->sip_expires;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_100rel_set_api(zpl_bool sip_100_rel)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_100_rel = sip_100_rel;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_100rel_get_api(zpl_bool *sip_100_rel)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_100_rel)
		*sip_100_rel = _pjapp_cfg->sip_100_rel;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_realm_set_api(char *realm)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	//_pjapp_cfg->sip_realm = realm;
	memset(_pjapp_cfg->sip_realm, 0, sizeof(_pjapp_cfg->sip_realm));
	if(realm)
	{
		strcpy(_pjapp_cfg->sip_realm, realm);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_realm_get_api(char *realm)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(realm)
	{
		strcpy(realm, _pjapp_cfg->sip_realm);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}


int pjapp_cfg_reregist_delay_set_api(zpl_uint16 sip_rereg_delay)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_rereg_delay = sip_rereg_delay;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_reregist_delay_get_api(zpl_uint16 *sip_rereg_delay)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_rereg_delay)
		*sip_rereg_delay = _pjapp_cfg->sip_rereg_delay;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_reregister_proxy_set_api(pjsip_reg_proxy_t sip_reg_proxy)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_reg_proxy = sip_reg_proxy;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_reregister_proxy_get_api(pjsip_reg_proxy_t *sip_reg_proxy)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_reg_proxy)
		*sip_reg_proxy = _pjapp_cfg->sip_reg_proxy;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_publish_set_api(zpl_bool sip_publish)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_publish = sip_publish;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_publish_get_api(zpl_bool *sip_publish)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_publish)
		*sip_publish = _pjapp_cfg->sip_publish;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_mwi_set_api(zpl_bool sip_mwi)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_mwi = sip_mwi;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_mwi_get_api(zpl_bool *sip_mwi)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_mwi)
		*sip_mwi = _pjapp_cfg->sip_mwi;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ims_set_api(zpl_bool sip_ims_enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_ims_enable = sip_ims_enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ims_get_api(zpl_bool *sip_ims_enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_ims_enable)
		*sip_ims_enable = _pjapp_cfg->sip_ims_enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_srtp_mode_set_api(pjsip_srtp_t sip_srtp_mode)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_srtp_mode = sip_srtp_mode;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_srtp_mode_get_api(pjsip_srtp_t *sip_srtp_mode)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_srtp_mode)
		*sip_srtp_mode = _pjapp_cfg->sip_srtp_mode;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_srtp_secure_set_api(pjsip_srtp_sec_t sip_srtp_secure)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_srtp_secure = sip_srtp_secure;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_srtp_secure_get_api(pjsip_srtp_sec_t *sip_srtp_secure)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_srtp_secure)
		*sip_srtp_secure = _pjapp_cfg->sip_srtp_secure;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_timer_set_api(pjsip_timer_t sip_timer)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_timer = sip_timer;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_timer_get_api(pjsip_timer_t *sip_timer)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_timer)
		*sip_timer = _pjapp_cfg->sip_timer;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_timer_sec_set_api(zpl_uint16 sip_timer_sec)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_timer_sec = sip_timer_sec;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_timer_sec_get_api(zpl_uint16 *sip_timer_sec)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_timer_sec)
		*sip_timer_sec = _pjapp_cfg->sip_timer_sec;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_outb_rid_set_api(zpl_uint16 sip_outb_rid)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_outb_rid = sip_outb_rid;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_outb_rid_get_api(zpl_uint16 *sip_outb_rid)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_outb_rid)
		*sip_outb_rid = _pjapp_cfg->sip_outb_rid;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_auto_update_nat_set_api(zpl_bool sip_auto_update_nat)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_auto_update_nat = sip_auto_update_nat;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_auto_update_nat_get_api(zpl_bool *sip_auto_update_nat)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_auto_update_nat)
		*sip_auto_update_nat = _pjapp_cfg->sip_auto_update_nat;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_stun_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_stun_disable = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_stun_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_stun_disable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}
/***************************************************************************/
//Transport Options:
int pjapp_cfg_ipv6_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_ipv6_enable = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ipv6_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_ipv6_enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_qos_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_set_qos = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_qos_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_set_qos;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_noudp_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_noudp = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_noudp_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_noudp;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_notcp_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_notcp = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_notcp_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_notcp;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_nameserver_set_api(char * address, zpl_uint16 port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_nameserver.sip_address, 0, sizeof(_pjapp_cfg->sip_nameserver.sip_address));
	if(address)
	{
		strcpy(_pjapp_cfg->sip_nameserver.sip_address, address);
		_pjapp_cfg->sip_nameserver.sip_port = port;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_nameserver_get_api(char * address, zpl_uint16 *port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(address)
	{
		strcpy(address, _pjapp_cfg->sip_nameserver.sip_address);
	}
	if(port)
		*port = _pjapp_cfg->sip_nameserver.sip_port;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_outbound_set_api(char * address, zpl_uint16 port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_outbound.sip_address, 0, sizeof(_pjapp_cfg->sip_outbound.sip_address));
	if(address)
	{
		strcpy(_pjapp_cfg->sip_outbound.sip_address, address);
		_pjapp_cfg->sip_outbound.sip_port = port;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_outbound_get_api(char * address, zpl_uint16 *port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(address)
	{
		strcpy(address, _pjapp_cfg->sip_outbound.sip_address);
	}
	if(port)
		*port = _pjapp_cfg->sip_outbound.sip_port;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_stun_server_set_api(char * address, zpl_uint16 port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_stun_server.sip_address, 0, sizeof(_pjapp_cfg->sip_stun_server.sip_address));
	if(address)
	{
		strcpy(_pjapp_cfg->sip_stun_server.sip_address, address);
		_pjapp_cfg->sip_stun_server.sip_port = port;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_stun_server_get_api(char * address, zpl_uint16 *port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(address)
	{
		strcpy(address, _pjapp_cfg->sip_stun_server.sip_address);
	}
	if(port)
		*port = _pjapp_cfg->sip_stun_server.sip_port;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}
/***************************************************************************/
//TLS Options:
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
int pjapp_cfg_tls_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_tls_enable = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_tls_enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_ca_set_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_tls_ca_file, 0, sizeof(_pjapp_cfg->sip_tls_ca_file));
	if(filename)
	{
		strcpy(_pjapp_cfg->sip_tls_ca_file, filename);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_ca_get_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(filename)
	{
		strcpy(filename, _pjapp_cfg->sip_tls_ca_file);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_cert_set_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_tls_cert_file, 0, sizeof(_pjapp_cfg->sip_tls_cert_file));
	if(filename)
	{
		strcpy(_pjapp_cfg->sip_tls_cert_file, filename);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_cert_get_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(filename)
	{
		strcpy(filename, _pjapp_cfg->sip_tls_cert_file);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}
int pjapp_cfg_tls_privkey_set_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_tls_privkey_file, 0, sizeof(_pjapp_cfg->sip_tls_privkey_file));
	if(filename)
	{
		strcpy(_pjapp_cfg->sip_tls_privkey_file, filename);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_privkey_get_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(filename)
	{
		strcpy(filename, _pjapp_cfg->sip_tls_privkey_file);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_password_set_api(char * password)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_tls_password, 0, sizeof(_pjapp_cfg->sip_tls_password));
	if(password)
	{
		strcpy(_pjapp_cfg->sip_tls_password, password);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_password_get_api(char * password)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(password)
	{
		strcpy(password, _pjapp_cfg->sip_tls_password);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_verify_server_set_api(char * address, zpl_uint16 port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_tls_verify_server.sip_address, 0, sizeof(_pjapp_cfg->sip_tls_verify_server.sip_address));
	if(address)
	{
		strcpy(_pjapp_cfg->sip_tls_verify_server.sip_address, address);
		_pjapp_cfg->sip_tls_verify_server.sip_port = port;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_verify_server_get_api(char * address, zpl_uint16 *port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(address)
	{
		strcpy(address, _pjapp_cfg->sip_tls_verify_server.sip_address);
	}
	if(port)
		*port = _pjapp_cfg->sip_tls_verify_server.sip_port;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_verify_client_set_api(char * address, zpl_uint16 port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_tls_verify_client.sip_address, 0, sizeof(_pjapp_cfg->sip_tls_verify_client.sip_address));
	if(address)
	{
		strcpy(_pjapp_cfg->sip_tls_verify_client.sip_address, address);
		_pjapp_cfg->sip_tls_verify_client.sip_port = port;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_verify_client_get_api(char * address, zpl_uint16 *port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(address)
	{
		strcpy(address, _pjapp_cfg->sip_tls_verify_client.sip_address);
	}
	if(port)
		*port = _pjapp_cfg->sip_tls_verify_client.sip_port;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_cipher_set_api(char * cipher)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_tls_cipher, 0, sizeof(_pjapp_cfg->sip_tls_cipher));
	if(cipher)
	{
		strcpy(_pjapp_cfg->sip_tls_cipher, cipher);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tls_cipher_get_api(char * cipher)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(cipher)
	{
		strcpy(cipher, _pjapp_cfg->sip_tls_cipher);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}
#endif

int pjapp_cfg_neg_timeout_set_api(zpl_uint16 sip_neg_timeout)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_neg_timeout = sip_neg_timeout;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_neg_timeout_get_api(zpl_uint16 *sip_neg_timeout)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_neg_timeout)
		*sip_neg_timeout = _pjapp_cfg->sip_neg_timeout;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}
/***************************************************************************/
//Audio Options:

int pjapp_cfg_codec_default_set_api(char * sip_codec)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_codec)
	{
		zpl_uint32 val = codec_payload_index(sip_codec);
		if(codec_payload_name(val))
		{
			memset(_pjapp_cfg->sip_codec.payload_name, 0, sizeof(_pjapp_cfg->sip_codec.payload_name));
			strcpy(_pjapp_cfg->sip_codec.payload_name, codec_payload_name(val));
			_pjapp_cfg->sip_codec.is_active = zpl_true;
			_pjapp_cfg->sip_codec.payload = val;
			if(_pjapp_cfg->mutex)
				os_mutex_unlock(_pjapp_cfg->mutex);
			return OK;
		}
	}
	else
	{
		memset(_pjapp_cfg->sip_codec.payload_name, 0, sizeof(_pjapp_cfg->sip_codec.payload_name));
		_pjapp_cfg->sip_codec.is_active = zpl_false;
		_pjapp_cfg->sip_codec.payload = 0;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_codec_add_api(char * sip_codec)
{
	return pjapp_cfg_payload_name_add_api(sip_codec);
/*	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_codec, 0, sizeof(_pjapp_cfg->sip_codec));
	if(sip_codec)
	{
		strcpy(_pjapp_cfg->sip_codec, sip_codec);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;*/
}

int pjapp_cfg_codec_del_api(char * sip_codec)
{
	return pjapp_cfg_payload_name_del_api(sip_codec);
/*	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_codec)
	{
		strcpy(sip_codec, _pjapp_cfg->sip_codec);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;*/
}

int pjapp_cfg_discodec_add_api(char * sip_discodec)
{
	return pjapp_cfg_dis_payload_name_add_api(sip_discodec);
/*	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_discodec, 0, sizeof(_pjapp_cfg->sip_discodec));
	if(sip_discodec)
	{
		strcpy(_pjapp_cfg->sip_discodec, sip_discodec);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;*/
}

int pjapp_cfg_discodec_del_api(char * sip_discodec)
{
	return pjapp_cfg_dis_payload_name_del_api(sip_discodec);
/*	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_discodec)
	{
		strcpy(sip_discodec, _pjapp_cfg->sip_discodec);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;*/
}

int pjapp_cfg_clock_rate_set_api(zpl_uint16 sip_clock_rate)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_clock_rate = sip_clock_rate;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_clock_rate_get_api(zpl_uint16 *sip_clock_rate)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_clock_rate)
		*sip_clock_rate = _pjapp_cfg->sip_clock_rate;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_snd_clock_rate_set_api(zpl_uint16 sip_snd_clock_rate)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_snd_clock_rate = sip_snd_clock_rate;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_snd_clock_rate_get_api(zpl_uint16 *sip_snd_clock_rate)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_snd_clock_rate)
		*sip_snd_clock_rate = _pjapp_cfg->sip_snd_clock_rate;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_stereo_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_stereo = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_stereo_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_stereo;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_audio_null_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_audio_null = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_audio_null_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_audio_null;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_play_file_set_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_play_file, 0, sizeof(_pjapp_cfg->sip_play_file));
	if(filename)
	{
		strcpy(_pjapp_cfg->sip_play_file, filename);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_play_file_get_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(filename)
	{
		strcpy(filename, _pjapp_cfg->sip_play_file);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_play_tone_set_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_play_tone, 0, sizeof(_pjapp_cfg->sip_play_tone));
	if(filename)
	{
		strcpy(_pjapp_cfg->sip_play_tone, filename);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_play_tone_get_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(filename)
	{
		strcpy(filename, _pjapp_cfg->sip_play_tone);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_auto_play_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_auto_play = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_auto_play_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_auto_play;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_auto_loop_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_auto_loop = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_auto_loop_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_auto_loop;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_auto_conf_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_auto_conf = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_auto_conf_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_auto_conf;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_rec_file_set_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_rec_file, 0, sizeof(_pjapp_cfg->sip_rec_file));
	if(filename)
	{
		strcpy(_pjapp_cfg->sip_rec_file, filename);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_rec_file_get_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(filename)
	{
		strcpy(filename, _pjapp_cfg->sip_rec_file);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_quality_set_api(zpl_uint16 sip_quality)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_quality = sip_quality;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_quality_get_api(zpl_uint16 *sip_quality)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_quality)
		*sip_quality = _pjapp_cfg->sip_quality;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ptime_set_api(zpl_uint16 sip_ptime)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_ptime = sip_ptime;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ptime_get_api(zpl_uint16 *sip_ptime)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_ptime)
		*sip_ptime = _pjapp_cfg->sip_ptime;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_no_vad_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_no_vad = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_no_vad_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_no_vad;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_echo_tail_set_api(zpl_uint16 sip_echo_tail)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_echo_tail = sip_echo_tail;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_echo_tail_get_api(zpl_uint16 *sip_echo_tail)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_echo_tail)
		*sip_echo_tail = _pjapp_cfg->sip_echo_tail;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_echo_mode_set_api(pjsip_echo_mode_t sip_echo_mode)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_echo_mode = sip_echo_mode;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_echo_mode_get_api(pjsip_echo_mode_t *sip_echo_mode)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_echo_mode)
		*sip_echo_mode = _pjapp_cfg->sip_echo_mode;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ilbc_mode_set_api(zpl_uint16 sip_ilbc_mode)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_ilbc_mode = sip_ilbc_mode;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ilbc_mode_get_api(zpl_uint16 *sip_ilbc_mode)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_ilbc_mode)
		*sip_ilbc_mode = _pjapp_cfg->sip_ilbc_mode;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}


int pjapp_cfg_capture_lat_set_api(zpl_uint16 sip_capture_lat)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_capture_lat = sip_capture_lat;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_capture_lat_get_api(zpl_uint16 *sip_capture_lat)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_capture_lat)
		*sip_capture_lat = _pjapp_cfg->sip_capture_lat;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_playback_lat_set_api(zpl_uint16 sip_playback_lat)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_playback_lat = sip_playback_lat;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_playback_lat_get_api(zpl_uint16 *sip_playback_lat)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_playback_lat)
		*sip_playback_lat = _pjapp_cfg->sip_playback_lat;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_auto_close_delay_set_api(zpl_int32 sip_snd_auto_close)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_snd_auto_close = sip_snd_auto_close;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_auto_close_delay_get_api(zpl_int32 *sip_snd_auto_close)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_snd_auto_close)
		*sip_snd_auto_close = _pjapp_cfg->sip_snd_auto_close;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_no_tones_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_notones = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_no_tones_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_notones;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_jb_max_size_set_api(zpl_int32 sip_jb_max_size)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_jb_max_size = sip_jb_max_size;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_jb_max_size_get_api(zpl_int32 *sip_jb_max_size)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip_jb_max_size)
		*sip_jb_max_size = _pjapp_cfg->sip_jb_max_size;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}
/***************************************************************************/
//Video Options:
#if PJSUA_HAS_VIDEO
int pjapp_cfg_video_enable_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_video = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_video_enable_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_video;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_video_play_file_set_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_play_avi, 0, sizeof(_pjapp_cfg->sip_play_avi));
	if(filename)
	{
		strcpy(_pjapp_cfg->sip_play_avi, filename);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_video_play_file_get_api(char * filename)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(filename)
	{
		strcpy(filename, _pjapp_cfg->sip_play_avi);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_video_auto_play_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_auto_play_avi = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_video_auto_play_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_auto_play_avi;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}
#endif
/***************************************************************************/
//Media Transport Options:
int pjapp_cfg_ice_enable_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_ice = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ice_enable_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_ice;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ice_nortcp_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_ice_nortcp = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ice_nortcp_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_ice_nortcp;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ice_regular_set_api(zpl_uint32 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_ice_regular = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ice_regular_get_api(zpl_uint32 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_ice_regular;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ice_max_host_set_api(zpl_uint32 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_ice_max_host = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_ice_max_host_get_api(zpl_uint32 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_ice_max_host;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_rtp_port_set_api(zpl_uint16 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_rtp_port = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_rtp_port_get_api(zpl_uint16 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_rtp_port;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_rx_drop_pct_set_api(zpl_uint16 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_rx_drop_pct = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_rx_drop_pct_get_api(zpl_uint16 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_rx_drop_pct;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tx_drop_pct_set_api(zpl_uint16 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_tx_drop_pct = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_tx_drop_pct_get_api(zpl_uint16 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_tx_drop_pct;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_turn_enable_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_turn = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_turn_enable_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_turn;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}


int pjapp_cfg_turn_server_set_api(char * address, zpl_uint16 port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_turn_srv.sip_address, 0, sizeof(_pjapp_cfg->sip_turn_srv.sip_address));
	if(address)
	{
		strcpy(_pjapp_cfg->sip_turn_srv.sip_address, address);
		_pjapp_cfg->sip_turn_srv.sip_port = port;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_turn_server_get_api(char * address, zpl_uint16 *port)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(address)
	{
		strcpy(address, _pjapp_cfg->sip_turn_srv.sip_address);
	}
	if(port)
		*port = _pjapp_cfg->sip_turn_srv.sip_port;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_turn_tcp_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_turn_tcp = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_turn_tcp_get_api(zpl_bool *enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(enable)
		*enable = _pjapp_cfg->sip_turn_tcp;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_turn_username_set_api(char * username)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_turn_user, 0, sizeof(_pjapp_cfg->sip_turn_user));
	if(username)
	{
		strcpy(_pjapp_cfg->sip_turn_user, username);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_turn_username_get_api(char * username)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(username)
	{
		strcpy(username, _pjapp_cfg->sip_turn_user);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_turn_password_set_api(char * password)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	memset(_pjapp_cfg->sip_turn_password, 0, sizeof(_pjapp_cfg->sip_turn_password));
	if(password)
	{
		strcpy(_pjapp_cfg->sip_turn_password, password);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_turn_password_get_api(char * password)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(password)
	{
		strcpy(password, _pjapp_cfg->sip_turn_password);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_rtcp_mux_set_api(zpl_bool value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_rtcp_mux = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_rtcp_mux_get_api(zpl_bool *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_rtcp_mux;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_srtp_keying_set_api(pjsip_srtp_keying_t value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_srtp_keying = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_srtp_keying_get_api(pjsip_srtp_keying_t *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_srtp_keying;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

//User Agent options:

int pjapp_cfg_auto_answer_code_set_api(zpl_uint16 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_auto_answer_code = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_auto_answer_code_get_api(zpl_uint16 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_auto_answer_code;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_max_calls_set_api(zpl_uint16 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_max_calls = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_max_calls_get_api(zpl_uint16 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_max_calls;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_max_thread_set_api(zpl_uint16 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_thread_max = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_max_thread_get_api(zpl_uint16 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_thread_max;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_duration_set_api(zpl_uint32 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_duration = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_duration_get_api(zpl_uint32 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_duration;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_norefersub_set_api(zpl_uint16 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_norefersub = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_norefersub_get_api(zpl_uint16 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_norefersub;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}


int pjapp_cfg_use_compact_form_set_api(zpl_uint16 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_use_compact_form = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_use_compact_form_get_api(zpl_uint16 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_use_compact_form;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_no_force_lr_set_api(zpl_uint16 value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_no_force_lr = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_no_force_lr_get_api(zpl_uint16 *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_no_force_lr;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_accept_redirect_set_api(pjsip_accept_redirect_t value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_accept_redirect = value;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_accept_redirect_get_api(pjsip_accept_redirect_t *value)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(value)
		*value = _pjapp_cfg->sip_accept_redirect;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
static int pjapp_cfg_url_get_id(char *url, char *id, char *ip, zpl_uint16 *port)
{
	char tmp[128];
	char *p = url, *brk = NULL;
	brk = strstr(p, ":");
	if(brk)
	{
		brk++;
		if(brk)
		{
			memset(tmp, 0, sizeof(tmp));
			sscanf(brk, "%[^@]", tmp);
			if(id)
				strcpy(id, tmp);
		}
	}
	brk = strstr(p, "@");
	if(brk)
	{
		brk++;
		if(brk)
		{
			memset(tmp, 0, sizeof(tmp));
			if(strstr(brk, ":"))
			{
				sscanf(brk, "%[^:]", tmp);
				if(ip)
					strcpy(ip, tmp);

				brk = strstr(brk, ":");
				brk++;
				if(brk)
				{
					if(port)
						*port = atoi(brk);
					return OK;
				}
			}
			else
			{
				if(ip)
					strcpy(ip, tmp);
				return OK;
			}
		}
	}
	return OK;
}
/********************************** debug log ************************************/
int pjapp_cfg_debug_level_set_api(zpl_uint32 level)
{
	/*	static const char *ltexts[] = { "FATAL:", "ERROR:", " WARN:",
				      " INFO:", "DEBUG:", "TRACE:", "DETRC:"};*/
	int inlevel = 0;
	switch(level)
	{
	case ZLOG_LEVEL_TRAP:
		inlevel = 6;
		break;
	case ZLOG_LEVEL_DEBUG:
		inlevel = 4;
		break;
	case ZLOG_LEVEL_INFO:
		inlevel = 3;
		break;
	case ZLOG_LEVEL_NOTICE:
		inlevel = 5;
		break;
	case ZLOG_LEVEL_WARNING:
		inlevel = 2;
		break;
	case ZLOG_LEVEL_ERR:
		inlevel = 1;
		break;
	//case ZLOG_LEVEL_CRIT:
	default:
		inlevel = 0;
		break;
	}
	_pjapp_cfg->debug_level = level;
	pjsua_app_cfg_log_level(&_global_config.app_config, inlevel);
	return OK;
}

int pjapp_cfg_debug_level_get_api(zpl_uint32 *level)
{
/*	int outlevel = pj_log_get_level();
	switch(outlevel)
	{
	case (6):
		outlevel = ZLOG_LEVEL_TRAP;
		break;
	case 4:
		outlevel = ZLOG_LEVEL_DEBUG;
		break;
	case 3:
		outlevel = ZLOG_LEVEL_INFO;
		break;
	case 5:
		outlevel = ZLOG_LEVEL_NOTICE;
		break;
	case 2:
		outlevel = ZLOG_LEVEL_WARNING;
		break;
	case 1:
		outlevel = ZLOG_LEVEL_ERR;
		break;
	case 0:
		outlevel = ZLOG_LEVEL_CRIT;
		break;
	}
	if(level)
		*level = outlevel;*/
	if(level)
		*level = _pjapp_cfg->debug_level;
	return OK;
}
int pjapp_cfg_debug_detail_set_api(zpl_bool enable)
{
	if(enable)
	{
		zpl_uint32 opt = pj_log_get_decor();
		opt |=  PJ_LOG_HAS_LEVEL_TEXT|
				PJ_LOG_HAS_SENDER|
				PJ_LOG_HAS_THREAD_ID|
				PJ_LOG_HAS_INDENT;
		pj_log_set_decor(opt);
	}
	else
	{
		zpl_uint32 opt = pj_log_get_decor();
		opt &= ~(PJ_LOG_HAS_LEVEL_TEXT|
				PJ_LOG_HAS_SENDER|
				PJ_LOG_HAS_THREAD_ID|
				PJ_LOG_HAS_INDENT);
		pj_log_set_decor(opt);
	}
	_pjapp_cfg->debug_detail = enable;
/*	pj_log_set_decor(PJ_LOG_HAS_NEWLINE|PJ_LOG_HAS_INDENT|PJ_LOG_HAS_THREAD_SWC|
			PJ_LOG_HAS_SENDER|PJ_LOG_HAS_THREAD_ID);*/
	return OK;
}
int pjapp_cfg_debug_detail_get_api(zpl_bool *enable)
{
	if(enable)
	{
		*enable = _pjapp_cfg->debug_detail;
/*		zpl_uint32 opt = pj_log_get_decor();
		if(opt & PJ_LOG_HAS_SENDER)
			*enable = zpl_true;
		else
			*enable = zpl_false;*/
	}
	return OK;
}
/***************************************************************************/
/***************************************************************************/
int pjapp_cfg_account_set_api(pjsua_acc_id id, void *p)
{
	pjsua_acc_info *info = p;
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);

	if(info->has_registration)
	{
		pjsip_username_t	user_tmp;
		pjsip_username_t	*sip_user = NULL;
		pjsip_server_t		srv_tmp;
		pjsip_server_t		*sip_srv = NULL;

		memset(&user_tmp, 0, sizeof(pjsip_username_t));
		memset(&srv_tmp, 0, sizeof(pjsip_server_t));
		//sip:100@192.168.0.103:5060
		pjapp_cfg_url_get_id(info->acc_uri.ptr, user_tmp.sip_user, srv_tmp.sip_address, &srv_tmp.sip_port);

		//printf("==============%s============(%s-%s-%d)\r\n", __func__, user_tmp.sip_user, srv_tmp.sip_address, srv_tmp.sip_port);
		//printf("==============%s============(%d:%d)(%s)\r\n", __func__, id, info->id, info->acc_uri.ptr);

		if(strlen(user_tmp.sip_user) && strlen(srv_tmp.sip_address))
		{
			if(strlen(_pjapp_cfg->sip_user.sip_user) &&
					strncmp(user_tmp.sip_user, _pjapp_cfg->sip_user.sip_user, sizeof(user_tmp.sip_user))==0)
			{
				sip_user = &_pjapp_cfg->sip_user;
			}
			else if(strlen(_pjapp_cfg->sip_user_sec.sip_user) &&
					strncmp(user_tmp.sip_user, _pjapp_cfg->sip_user_sec.sip_user, sizeof(user_tmp.sip_user))==0)
			{
				sip_user = &_pjapp_cfg->sip_user_sec;
			}

			if(strlen(_pjapp_cfg->sip_server.sip_address) &&
					strncmp(srv_tmp.sip_address, _pjapp_cfg->sip_server.sip_address, sizeof(srv_tmp.sip_address))==0)
			{
				sip_srv = &_pjapp_cfg->sip_server;
			}
			else if(strlen(_pjapp_cfg->sip_server_sec.sip_address) &&
					strncmp(srv_tmp.sip_address, _pjapp_cfg->sip_server_sec.sip_address, sizeof(srv_tmp.sip_address))==0)
			{
				sip_srv = &_pjapp_cfg->sip_server_sec;
			}
			else if(strlen(_pjapp_cfg->sip_proxy.sip_address) &&
					strncmp(srv_tmp.sip_address, _pjapp_cfg->sip_proxy.sip_address, sizeof(srv_tmp.sip_address))==0)
			{
				sip_srv = &_pjapp_cfg->sip_proxy;
			}
			else if(strlen(_pjapp_cfg->sip_proxy_sec.sip_address) &&
					strncmp(srv_tmp.sip_address, _pjapp_cfg->sip_proxy_sec.sip_address, sizeof(srv_tmp.sip_address))==0)
			{
				sip_srv = &_pjapp_cfg->sip_proxy_sec;
			}
		}

		if(sip_user)
		{
			if(/*info->online_status && */info->expires > 0)
				sip_user->sip_state = PJSIP_STATE_REGISTER_SUCCESS;
			else
				sip_user->sip_state = PJSIP_STATE_REGISTER_FAILED;
			sip_user->id = info->id;

			sip_user->is_default = info->is_default;
			sip_user->is_current = ((id == current_acc)? zpl_true:zpl_false);

			sip_user->register_svr = sip_srv ? sip_srv:NULL;
		}
		if(sip_srv)
		{
			if(!info->online_status)
				sip_srv->state = PJSIP_STATE_CONNECT_FAILED;
			else
			{
				sip_srv->state = PJSIP_STATE_CONNECT_SUCCESS;
				pjsua_acc_set_online_status(id, PJ_TRUE);
			}
		}
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_account_get_api(pjsua_acc_id id, pjsip_username_t *p)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	//pjsua_acc_info info
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}
zpl_bool pjapp_cfg_isregister_api(void)
{
	zpl_bool reg = zpl_false;
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
/*
	pjsip_username_t	sip_user;
	pjsip_username_t	sip_user_sec;
	zpl_uint16				sip_user_cnt;
	pjsip_server_t		sip_server;
	pjsip_server_t		sip_server_sec;
*/
#if 0//ndef ZPL_BUILD_ARCH_X86
	if(x5b_app_port_status_get() == zpl_false)
	{
		if(_pjapp_cfg->mutex)
			os_mutex_unlock(_pjapp_cfg->mutex);
		return reg;
	}
#endif
	if((strlen(_pjapp_cfg->sip_user.sip_phone)||
			strlen(_pjapp_cfg->sip_user.sip_user)) &&
			_pjapp_cfg->sip_user.register_svr &&
			_pjapp_cfg->sip_user.is_current)
	{
		if(_pjapp_cfg->sip_user.sip_state == PJSIP_STATE_REGISTER_SUCCESS/* &&
				_pjapp_cfg->sip_user.register_svr->state*/)
		{
			reg = zpl_true;
		}
	}
	else if((strlen(_pjapp_cfg->sip_user_sec.sip_phone)||
			strlen(_pjapp_cfg->sip_user_sec.sip_user)) &&
			_pjapp_cfg->sip_user_sec.register_svr &&
			_pjapp_cfg->sip_user_sec.is_current)
	{
		if(_pjapp_cfg->sip_user_sec.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
		{
			reg = zpl_true;
		}
	}

	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return reg;
}
/***************************************************************************/
int pjapp_cfg_payload_name_add_api(char * value)
{
	zpl_int32 i = 0, val = 0;
	zassert(value != NULL);
	zassert(_pjapp_cfg != NULL);
	val = codec_payload_index(value);
	if(val < 0 || !codec_payload_name(val))
		return ERROR;
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(_pjapp_cfg->codec[i].is_active && _pjapp_cfg->codec[i].payload == val)
		{
			if(_pjapp_cfg->mutex)
				os_mutex_unlock(_pjapp_cfg->mutex);
			return OK;
		}
	}
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(!_pjapp_cfg->codec[i].is_active)
		{
			memset(_pjapp_cfg->codec[i].payload_name, 0, sizeof(_pjapp_cfg->codec[i].payload_name));
			strcpy(_pjapp_cfg->codec[i].payload_name, codec_payload_name(val));
			_pjapp_cfg->codec[i].payload = val;
			_pjapp_cfg->codec[i].is_active = zpl_true;
			if(_pjapp_cfg->mutex)
				os_mutex_unlock(_pjapp_cfg->mutex);
			return OK;
		}
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return ERROR;
}

int pjapp_cfg_payload_name_del_api(char * value)
{
	zpl_int32 i = 0, val = 0;
	zassert(value != NULL);
	zassert(_pjapp_cfg != NULL);
	val = codec_payload_index(value);
	if(val < 0 || !codec_payload_name(val))
		return ERROR;
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(_pjapp_cfg->codec[i].is_active &&
				_pjapp_cfg->codec[i].payload == val
				/*strcasecmp(_pjapp_cfg->codec[i].payload_name, value) == 0*/	)
		{
			memset(_pjapp_cfg->codec[i].payload_name, 0, sizeof(_pjapp_cfg->codec[i].payload_name));
			_pjapp_cfg->codec[i].is_active = zpl_false;
			_pjapp_cfg->codec[i].payload = 0;
			if(_pjapp_cfg->mutex)
				os_mutex_unlock(_pjapp_cfg->mutex);
			return OK;
		}
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return ERROR;
}

int pjapp_cfg_dis_payload_name_add_api(char * value)
{
	zpl_int32 i = 0, val = 0;
	zassert(value != NULL);
	zassert(_pjapp_cfg != NULL);
	val = codec_payload_index(value);
	if(val < 0 || !codec_payload_name(val))
		return ERROR;
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(_pjapp_cfg->dicodec[i].is_active && _pjapp_cfg->dicodec[i].payload == val)
		{
			if(_pjapp_cfg->mutex)
				os_mutex_unlock(_pjapp_cfg->mutex);
			return OK;
		}
	}
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(!_pjapp_cfg->dicodec[i].is_active)
		{
			memset(_pjapp_cfg->dicodec[i].payload_name, 0, sizeof(_pjapp_cfg->dicodec[i].payload_name));
			//strcpy(_pjapp_cfg->dicodec[i].payload_name, value);
			strcpy(_pjapp_cfg->codec[i].payload_name, codec_payload_name(val));
			_pjapp_cfg->dicodec[i].payload = val;
			_pjapp_cfg->dicodec[i].is_active = zpl_true;
			if(_pjapp_cfg->mutex)
				os_mutex_unlock(_pjapp_cfg->mutex);
			return OK;
		}
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return ERROR;
}

int pjapp_cfg_dis_payload_name_del_api(char * value)
{
	zpl_int32 i = 0, val = 0;
	zassert(value != NULL);
	zassert(_pjapp_cfg != NULL);
	val = codec_payload_index(value);
	if(val < 0 || !codec_payload_name(val))
		return ERROR;
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(_pjapp_cfg->dicodec[i].is_active &&
				_pjapp_cfg->dicodec[i].payload == val
				/*strcasecmp(_pjapp_cfg->dicodec[i].payload_name, value) == 0*/ )
		{
			memset(_pjapp_cfg->dicodec[i].payload_name, 0, sizeof(_pjapp_cfg->dicodec[i].payload_name));
			_pjapp_cfg->dicodec[i].is_active = zpl_false;
			_pjapp_cfg->dicodec[i].payload = 0;
			if(_pjapp_cfg->mutex)
				os_mutex_unlock(_pjapp_cfg->mutex);
			return OK;
		}
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return ERROR;
}
/***************************************************************************/
/***************************************************************************/
int pjapp_cfg_app_add_acc(char *sip_url, char *sip_srv, char *realm,
		char *user, char *pass, pjsua_acc_id *accid)
{
    pjsua_acc_config acc_cfg;
    pj_status_t status;
	char cmd[128], tmp[128];
	memset(cmd, 0, sizeof(cmd));
	memset(tmp, 0, sizeof(tmp));
    pjsua_acc_config_default(&acc_cfg);
	snprintf(cmd, sizeof(cmd), "sip:%s@%s", user, sip_url);
	snprintf(tmp, sizeof(tmp), "sip:%s", sip_srv);
    acc_cfg.id = pj_str(cmd);
    acc_cfg.reg_uri = pj_str(tmp);
    acc_cfg.cred_count = 1;
    acc_cfg.cred_info[0].scheme = pj_str("Digest");
    acc_cfg.cred_info[0].realm = pj_str(realm);
    acc_cfg.cred_info[0].username = pj_str(user);
    acc_cfg.cred_info[0].data_type = 0;
    acc_cfg.cred_info[0].data = pj_str(pass);

    acc_cfg.rtp_cfg = _global_config.app_config.rtp_cfg;
    app_config_init_video(&acc_cfg);

    status = pjsua_acc_add(&acc_cfg, PJ_TRUE, NULL);
    if (status != PJ_SUCCESS) {
    	return ERROR;
    }
    return OK;
}

int pjapp_cfg_app_del_acc(pjsua_acc_id accid)
{
    pj_status_t status;
    if (!pjsua_acc_is_valid(accid))
    {
    	return ERROR;
    }
    status = pjsua_acc_del(accid);
    if (status != PJ_SUCCESS) {
    	return ERROR;
    }
	return ERROR;
}

int pjapp_cfg_app_mod_acc(pjsua_acc_id accid, char *sip_url, char *sip_srv, char *realm,
		char *user, char *pass)
{
	return ERROR;
}

int pjapp_cfg_app_select_acc(pjsua_acc_id accid, zpl_uint32 type)
{
    pj_status_t status;
    if (pjsua_acc_is_valid(accid))
    {
    	status = pjsua_acc_set_default(accid);
        if (status != PJ_SUCCESS) {
        	return ERROR;
        }
        return OK;
    }
    return ERROR;
}

int pjapp_cfg_app_reg_acc(zpl_bool reg)
{
	if(pjsua_acc_is_valid(current_acc))
		return pjsua_acc_set_registration(current_acc, reg);
	return ERROR;
}

int pjapp_cfg_app_list_acc(pjsua_acc_id accid)
{
	pjsua_acc_id acc_ids[16];
	zpl_uint32 count = PJ_ARRAY_SIZE(acc_ids);
	zpl_uint32 i;
	//static const pj_str_t header = { "Account list:\n", 15 };
	pjsua_enum_accs (acc_ids, &count);

	for (i = 0; i < (int) count; ++i)
	{
		char acc_info[80];
		char out_str[160];
		pjsua_acc_info info;

		pjsua_acc_get_info (acc_ids[i], &info);

		if (_global_config.app_config.cbtbl.cli_account_state_get)
			(_global_config.app_config.cbtbl.cli_account_state_get) (acc_ids[i], &info);

		if (!info.has_registration)
		{
			pj_ansi_snprintf (acc_info, sizeof(acc_info), "%.*s",
							  (int) info.status_text.slen,
							  info.status_text.ptr);

		}
		else
		{
			pj_ansi_snprintf (acc_info, sizeof(acc_info),
							  "%d/%.*s (expires=%d)", info.status,
							  (int) info.status_text.slen, info.status_text.ptr,
							  info.expires);

		}

		pj_ansi_snprintf (out_str, sizeof(out_str), " %c[%2d] %.*s: %s\n",
						  (acc_ids[i] == current_acc ? '*' : ' '), acc_ids[i],
						  (int) info.acc_uri.slen, info.acc_uri.ptr, acc_info);
		//pj_cli_sess_write_msg (cval->sess, out_str, pj_ansi_strlen (out_str));

		pj_bzero (out_str, sizeof(out_str));
		pj_ansi_snprintf (out_str, sizeof(out_str),
						  "       Online status: %.*s\n",
						  (int) info.online_status_text.slen,
						  info.online_status_text.ptr);

		//pj_cli_sess_write_msg (cval->sess, out_str, pj_ansi_strlen (out_str));
	}
	return PJ_SUCCESS;
}


int pjapp_cfg_app_start_call(pjsua_acc_id accid, char *num, pjsua_call_id *callid)
{

	pj_str_t call_uri_arg;
	char cmd[512];
	memset(cmd, '\0', sizeof(cmd));
	if(!_pjapp_cfg)
		return ERROR;
	//zlog_debug(MODULE_VOIP, "========%s->os_mutex_lock", __func__);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);

	if(_pjapp_cfg->sip_user.sip_state == PJSIP_STATE_REGISTER_SUCCESS &&
			_pjapp_cfg->sip_user.register_svr &&
			strlen(_pjapp_cfg->sip_user.register_svr->sip_address))
	{
		if(_pjapp_cfg->sip_user.register_svr->sip_port == PJSIP_PORT_DEFAULT)
			snprintf(cmd, sizeof(cmd), "sip:%s@%s",
					num, _pjapp_cfg->sip_user.register_svr->sip_address);
		else
			snprintf(cmd, sizeof(cmd), "sip:%s@%s:%d",
					num,
					_pjapp_cfg->sip_user.register_svr->sip_address,
					_pjapp_cfg->sip_user.register_svr->sip_port);
	}
	else if(_pjapp_cfg->sip_user_sec.sip_state == PJSIP_STATE_REGISTER_SUCCESS &&
			_pjapp_cfg->sip_user_sec.register_svr &&
			strlen(_pjapp_cfg->sip_user_sec.register_svr->sip_address))
	{
		if(_pjapp_cfg->sip_user_sec.register_svr->sip_port == PJSIP_PORT_DEFAULT)
			snprintf(cmd, sizeof(cmd), "sip:%s@%s",
					num, _pjapp_cfg->sip_user_sec.register_svr->sip_address);
		else
			snprintf(cmd, sizeof(cmd), "sip:%s@%s:%d",
					num,
					_pjapp_cfg->sip_user_sec.register_svr->sip_address,
					_pjapp_cfg->sip_user_sec.register_svr->sip_port);
	}
	else
	{
		if(_pjapp_cfg->mutex)
			os_mutex_unlock(_pjapp_cfg->mutex);
		return ERROR;
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	if(_global_config.current_call != PJSUA_INVALID_ID)
		return ERROR;
	//zlog_debug(MODULE_VOIP, "========%s->voip_volume_control_api", __func__);
	//voip_volume_control_api(zpl_true);
	//zlog_debug(MODULE_VOIP, "========%s-> enter pjapp_cfg_app_start_call", __func__);

	//char *pj_call_str = (char *)(cmd + 9);
	call_uri_arg = pj_str(cmd);
	//call_uri_arg = pj_str("sip:1003@192.168.3.254");
/*	pjsua_call_setting_default(&_global_config._global_config.call_opt);
	_global_config.app_config.app_cfg.call_opt.aud_cnt = _global_config.app_config.app_cfg.aud_cnt;
	_global_config.app_config.app_cfg.call_opt.vid_cnt = _global_config.app_config.app_cfg.vid.vid_cnt;*/
	if(pjsua_call_make_call(current_acc/*current_acc*/, &call_uri_arg,
				NULL/*&_global_config.app_config.app_cfg.call_opt*/, NULL, NULL, &_global_config.current_call) == PJ_SUCCESS)
	{
		if(callid)
			*callid = _global_config.current_call;
		//zlog_debug(MODULE_VOIP, "========%s-> level pjapp_cfg_app_start_call", __func__);
		return OK;
	}
/*
 * handle SIGUSR2 nostop noprint
*/
	//zlog_debug(MODULE_VOIP, "========%s-> level pjapp_cfg_app_start_call", __func__);
	//voip_volume_control_api(zpl_false);
	return ERROR;
}

int pjapp_cfg_app_stop_call(pjsua_call_id callid, zpl_bool all)
{
    if (_global_config.current_call == PJSUA_INVALID_ID)
    {
    	return ERROR;
    }
    else
    {
    	int ret = 0;
		if (all)
		{
			pjsua_call_hangup_all();
			//voip_volume_control_api(zpl_false);
			return OK;
		}
		else
		{
			if(callid == PJSUA_INVALID_ID)
				ret = pjsua_call_hangup(callid, 0, NULL, NULL);
			else
				ret = pjsua_call_hangup(_global_config.current_call, 0, NULL, NULL);
			if(ret == PJ_SUCCESS)
			{
				//voip_volume_control_api(zpl_false);
				return OK;
			}
		}
    }
	//voip_volume_control_api(zpl_false);
	return ERROR;
}
/* Make multi call */
#if 0
int pjapp_cfg_app_start_multi_call(pjsua_acc_id accid, char *num, int *callid)
//static pj_status_t cmd_make_multi_call(pj_cli_cmd_val *cval)
{
	struct input_result result;
	char dest[64] = { 0 };
	char out_str[128];
	int i, count;
	pj_str_t tmp = pj_str(dest);

	pj_ansi_snprintf(out_str, sizeof(out_str),
			"(You currently have %d calls)\n", pjsua_call_get_count());

	count = 3;								//(int)pj_strtol(&cval->argv[1]);
	if (count < 1)
		return PJ_SUCCESS;

	pj_strncpy_with_null(&tmp, &cval->argv[2], sizeof(dest));

	/* input destination. */
	get_input_url(tmp.ptr, tmp.slen, cval, &result);
	if (result.nb_result != PJSUA_APP_NO_NB)
	{
		pjsua_buddy_info binfo;
		if (result.nb_result == -1 || result.nb_result == 0)
		{
			/*	    static const pj_str_t err_msg =
			 {"You can't do that with make call!\n", 35};
			 pj_cli_sess_write_msg(cval->sess, err_msg.ptr, err_msg.slen);*/
			return PJ_SUCCESS;
		}
		pjsua_buddy_get_info(result.nb_result - 1, &binfo);
		pj_strncpy(&tmp, &binfo.uri, sizeof(dest));
	}
	else
	{
		tmp = pj_str(result.uri_result);
	}

	for (i = 0; i < count; ++i)
	{
		pj_status_t status;
		status = pjsua_call_make_call(current_acc, &tmp, &_global_config.app_config.app_cfg.call_opt,
				NULL, NULL, NULL);
		if (status != PJ_SUCCESS)
			break;
	}
	return PJ_SUCCESS;
}
#endif
/***************************************************************************/
int pjapp_cfg_app_answer_call(pjsua_call_id callid, zpl_uint32 st_code)
{
	pjsua_call_info call_info;
	if ((st_code < 100) || (st_code > 699))
		return ERROR;
	if (_global_config.current_call != PJSUA_INVALID_ID)
	{
		pjsua_call_get_info(_global_config.current_call, &call_info);
	}
	else
	{
		/* Make compiler happy */
		call_info.role = PJSIP_ROLE_UAC;
		call_info.state = PJSIP_INV_STATE_DISCONNECTED;
	}

	if (_global_config.current_call == PJSUA_INVALID_ID
			|| call_info.role != PJSIP_ROLE_UAS
			|| call_info.state >= PJSIP_INV_STATE_CONNECTING)
	{
		return ERROR;
	}
	else
	{
		char contact[120];
		pj_str_t hname =
		{ "Contact", 7 };
		pj_str_t hvalue;
		pjsip_generic_string_hdr hcontact;

		pjsua_msg_data_init(&_global_config.msg_data);

		if (st_code / 100 == 3)
		{
			/*			 if (cval->argc < 3)
			 {
			 static const pj_str_t err_msg = {"Enter URL to be put in Contact\n",  32};
			 return PJ_SUCCESS;
			 }*/

			hvalue = pj_str(contact);
			pjsip_generic_string_hdr_init2(&hcontact, &hname, &hvalue);

			pj_list_push_back(&_global_config.msg_data.hdr_list, &hcontact);
		}

		/*
		 * Must check again!
		 * Call may have been disconnected while we're waiting for
		 * keyboard input.
		 */
		if (_global_config.current_call == PJSUA_INVALID_ID)
		{
			//static const pj_str_t err_msg =
			//		{ "Call has been disconnected\n", 28 };
			//pj_cli_sess_write_msg(cval->sess, err_msg.ptr, err_msg.slen);
			return ERROR;
		}

		if (pjsua_call_answer2(_global_config.current_call, &_global_config.call_opt,
				st_code, NULL, &_global_config.msg_data) == PJ_SUCCESS)
			return OK;
	}
	return ERROR;
}

int pjapp_cfg_app_hold_call(pjsua_call_id callid)
{
    if (callid != PJSUA_INVALID_ID)
    {
    	if(pjsua_call_set_hold(callid, NULL) == PJ_SUCCESS)
    		return OK;
    }
    else
    {
    	//PJ_LOG(3,(THIS_FILE, "No current call"));
    	return ERROR;
    }
    return PJ_SUCCESS;
}

int pjapp_cfg_app_reinvite_call(pjsua_call_id callid)
{
	if (callid != PJSUA_INVALID_ID)
	{
		/*
		 * re-INVITE
		 */
		_global_config.call_opt.flag |= PJSUA_CALL_UNHOLD;
		if(pjsua_call_reinvite2 (callid, &_global_config.call_opt, NULL) == PJ_SUCCESS)
			return OK;
	}
	else
	{
		//PJ_LOG(3,(THIS_FILE, "No current call"));
		return ERROR;
	}
	return PJ_SUCCESS;
}

int pjapp_cfg_app_dtmf_call(pjsua_call_id callid, zpl_uint32 type, zpl_uint32 code)
{
	if (_global_config.current_call == PJSUA_INVALID_ID)
	{
		return ERROR;
	}
	if (type == 1)
	{
		char body[64];
		zpl_uint32 call = _global_config.current_call;
		pj_status_t status;
		pj_str_t dtmf_digi = pj_str("INFO");
		memset(body, 0, sizeof(body));

		pj_ansi_snprintf(body, sizeof(body), "%c",code);

		dtmf_digi = pj_str(body);

		if (!pjsua_call_has_media(_global_config.current_call))
		{
			//PJ_LOG(3, (THIS_FILE, "Media is not established yet!"));
			return ERROR;
		}
		if (call != _global_config.current_call)
		{
			//static const pj_str_t err_msg =
			//		{ "Call has been disconnected\n", 28 };
			//pj_cli_sess_write_msg(cval->sess, err_msg.ptr, err_msg.slen);
			return ERROR;
		}

		status = pjsua_call_dial_dtmf(_global_config.current_call, &dtmf_digi);
		if (status != PJ_SUCCESS)
		{
			// pjsua_perror(THIS_FILE, "Unable to send DTMF", status);
			return ERROR;
		}
		return PJ_SUCCESS;
	}
	else
	{
		char body[64];
		const pj_str_t SIP_INFO = pj_str("INFO");
		zpl_uint32 call = _global_config.current_call;
		pj_status_t status;

		if (call != _global_config.current_call)
		{
			return ERROR;
		}

		pjsua_msg_data_init(&_global_config.msg_data);
		_global_config.msg_data.content_type = pj_str("application/dtmf-relay");

		pj_ansi_snprintf(body, sizeof(body), "Signal=%c\n"
				"Duration=160", code);

		_global_config.msg_data.msg_body = pj_str(body);

		status = pjsua_call_send_request(_global_config.current_call, &SIP_INFO,
				&_global_config.msg_data);
		if (status != PJ_SUCCESS)
		{
			return ERROR;
		}
		return PJ_SUCCESS;
	}
	return PJ_SUCCESS;
}

int pjapp_cfg_app_select_call(pjsua_call_id callid, zpl_uint32 type)
{
	/*
	 * Cycle next/prev dialog.
	 */
	if (type == 1)
	{
		find_next_call ();
	}
	else
	{
		find_prev_call ();
	}

	if (_global_config.current_call != PJSUA_INVALID_ID)
	{
		pjsua_call_info call_info;

		if(pjsua_call_get_info (_global_config.current_call, &call_info) == PJ_SUCCESS)
			return OK;
	}
	else
	{
		return ERROR;
	}
	return PJ_SUCCESS;
}
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
int pjapp_cfg_multiuser_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->sip_multi_user == enable)
		return OK;
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_multi_user = enable;
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

zpl_bool pjapp_cfg_multiuser_get_api(void)
{
	zassert(_pjapp_cfg != NULL);
	return _pjapp_cfg->sip_multi_user;
}

int pjapp_cfg_active_standby_set_api(zpl_bool enable)
{
	zassert(_pjapp_cfg != NULL);
	if(_pjapp_cfg->sip_active_standby == enable)
		return OK;
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	_pjapp_cfg->sip_active_standby = enable;

	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

zpl_bool pjapp_cfg_active_standby_get_api(void)
{
	zassert(_pjapp_cfg != NULL);
	return _pjapp_cfg->sip_active_standby;
}
/***************************************************************************/
/***************************************************************************/
static int pjapp_cfg_account_options_write_config(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zpl_uint32 i = 0;
	zassert(sip != NULL);
	zassert(vty != NULL);
	//char buftmp[128];
	if (strlen(sip->sip_user.sip_user))
	{
		vty_out(vty, " ip sip username %s%s", sip->sip_user.sip_user,
				VTY_NEWLINE);
	}
/*	if (strlen(sip->sip_user.sip_phone))
	{
		vty_out(vty, " ip sip local-phone %s%s", sip->sip_user.sip_phone,
				VTY_NEWLINE);
	}*/
	if (strlen(sip->sip_user.sip_password))
		vty_out(vty, " ip sip password %s%s", sip->sip_user.sip_password,
				VTY_NEWLINE);

	if (sip->sip_user_cnt > 1)
	{
		if (strlen(sip->sip_user_sec.sip_user))
			vty_out(vty, " ip sip username %s secondary%s",
					sip->sip_user_sec.sip_user, VTY_NEWLINE);

/*
		if (strlen(sip->sip_user_sec.sip_phone))
			vty_out(vty, " ip sip local-phone %s secondary%s",
					sip->sip_user_sec.sip_phone, VTY_NEWLINE);
*/

		if (strlen(sip->sip_user_sec.sip_password))
			vty_out(vty, " ip sip password %s secondary%s",
					sip->sip_user_sec.sip_password, VTY_NEWLINE);
	}

	if (sip->sip_local.sip_port && sip->sip_local.sip_port != PJSIP_PORT_DEFAULT)
		vty_out(vty, " ip sip local-port %d%s", (sip->sip_local.sip_port),
				VTY_NEWLINE);

	if (strlen(sip->sip_local.sip_address) != 0 && sip->sip_local.state == PJSIP_STATE_CONNECT_LOCAL)
		vty_out(vty, " ip sip local-address %s%s", (sip->sip_local.sip_address),
				VTY_NEWLINE);

	if (sip->sip_source_interface != 0)
		vty_out(vty, " ip sip source-interface %s%s",
				ifindex2ifname(sip->sip_source_interface), VTY_NEWLINE);

	if (strlen(sip->sip_server.sip_address))
		vty_out(vty, " ip sip server %s%s", (sip->sip_server.sip_address),
				VTY_NEWLINE);
	if (sip->sip_server.sip_port && sip->sip_server.sip_port != PJSIP_PORT_DEFAULT)
		vty_out(vty, " ip sip server port %d%s", (sip->sip_server.sip_port),
				VTY_NEWLINE);

	if (sip->sip_server_cnt > 1)
	{
		if (strlen(sip->sip_server_sec.sip_address))
			vty_out(vty, " ip sip server %s secondary%s",
					(sip->sip_server_sec.sip_address), VTY_NEWLINE);
		if (sip->sip_server_sec.sip_port && sip->sip_server_sec.sip_port != PJSIP_PORT_DEFAULT)
			vty_out(vty, " ip sip server port %d secondary%s",
					(sip->sip_server_sec.sip_port), VTY_NEWLINE);

	}

	if (sip->sip_proxy_enable)
	{
		//vty_out(vty, " ip sip proxy enable%s", VTY_NEWLINE);

		if (strlen(sip->sip_proxy.sip_address))
			vty_out(vty, " ip sip proxy-server %s%s",
					(sip->sip_proxy.sip_address), VTY_NEWLINE);
		if (sip->sip_proxy.sip_port && sip->sip_proxy.sip_port != PJSIP_PORT_DEFAULT)
			vty_out(vty, " ip sip proxy-server port %d%s",
					(sip->sip_proxy.sip_port), VTY_NEWLINE);

		if (sip->sip_proxy_cnt > 1)
		{
			if (strlen(sip->sip_proxy_sec.sip_address))
				vty_out(vty, " ip sip proxy-server %s secondary%s",
						(sip->sip_proxy_sec.sip_address), VTY_NEWLINE);
			if (sip->sip_proxy_sec.sip_port && sip->sip_proxy_sec.sip_port != PJSIP_PORT_DEFAULT)
				vty_out(vty, " ip sip proxy-server port %d secondary%s",
						(sip->sip_proxy_sec.sip_port), VTY_NEWLINE);
		}
	}

	if (sip->proto == PJSIP_PROTO_UDP)
		vty_out(vty, " ip sip transport udp%s", VTY_NEWLINE);
	else if (sip->proto == PJSIP_PROTO_TCP)
		vty_out(vty, " ip sip transport tcp%s", VTY_NEWLINE);
	else if (sip->proto == PJSIP_PROTO_TLS)
		vty_out(vty, " ip sip transport tls%s", VTY_NEWLINE);
	else if (sip->proto == PJSIP_PROTO_DTLS)
		vty_out(vty, " ip sip transport dtls%s", VTY_NEWLINE);

	if (sip->dtmf == PJSIP_DTMF_INFO)
		vty_out(vty, " ip sip dtmf-type sip-info%s", VTY_NEWLINE);
	else if (sip->dtmf == PJSIP_DTMF_RFC2833)
		vty_out(vty, " ip sip dtmf-type rfc2833%s", VTY_NEWLINE);
	else if (sip->dtmf == PJSIP_DTMF_INBAND)
		vty_out(vty, " ip sip dtmf-type inband%s", VTY_NEWLINE);

	if (sip->sip_expires && sip->sip_expires != PJSIP_EXPIRES_DEFAULT)
		vty_out(vty, " ip sip register-interval %d%s", sip->sip_expires, VTY_NEWLINE);

	if (sip->sip_100_rel != PJSIP_100_REL_DEFAULT)
		vty_out(vty, " ip sip rel-100%s", VTY_NEWLINE);

	if (strlen(sip->sip_realm))
		vty_out(vty, " ip sip realm %s%s", sip->sip_realm, VTY_NEWLINE);

	if (strlen(_pjapp_cfg->sip_codec.payload_name))
		vty_out(vty, " ip sip default codec %s%s", codec_cmdname(_pjapp_cfg->sip_codec.payload), VTY_NEWLINE);

/*
	if (sip->sip_reg_timeout)
		vty_out(vty, " ip sip reg-timeout %d%s", sip->sip_reg_timeout,
				VTY_NEWLINE);
*/
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(_pjapp_cfg->codec[i].is_active)
		{
			//memset(buftmp, 0, sizeof(buftmp));
			//snprintf(buftmp, sizeof(buftmp), "%s", _pjapp_cfg->codec[i].payload_name);
			vty_out(vty, " ip sip codec %s%s", codec_cmdname(_pjapp_cfg->codec[i].payload), VTY_NEWLINE);
		}
	}
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(_pjapp_cfg->dicodec[i].is_active)
		{
			//memset(buftmp, 0, sizeof(buftmp));
			//snprintf(buftmp, sizeof(buftmp), "%s", _pjapp_cfg->dicodec[i].payload_name);
			//vty_out(vty, " ip sip discodec %s%s", codec_cmdname(_pjapp_cfg->dicodec[i].payload), VTY_NEWLINE);
		}
	}

	if (sip->sip_rereg_delay && sip->sip_rereg_delay != PJSUA_REG_RETRY_INTERVAL)
		vty_out(vty, " ip sip rereg-delay %d%s", sip->sip_rereg_delay,
				VTY_NEWLINE);

	if (sip->sip_reg_proxy == PJSIP_REGISTER_NONE)
		vty_out(vty, " ip sip reg-proxy disable%s", VTY_NEWLINE);
	else if (sip->sip_reg_proxy == PJSIP_REGISTER_NO_PROXY)
		vty_out(vty, " ip sip reg-proxy none%s", VTY_NEWLINE);
	else if (sip->sip_reg_proxy == PJSIP_REGISTER_OUTBOUND_PROXY)
		vty_out(vty, " ip sip reg-proxy outbound%s", VTY_NEWLINE);
	else if (sip->sip_reg_proxy == PJSIP_REGISTER_ACC_ONLY)
		vty_out(vty, " ip sip reg-proxy acc%s", VTY_NEWLINE);
	else if (sip->sip_reg_proxy == PJSIP_REGISTER_ALL)
		vty_out(vty, " ip sip reg-proxy all%s", VTY_NEWLINE);

	if (sip->sip_publish)
		vty_out(vty, " ip sip enable publish%s", VTY_NEWLINE);
	if (sip->sip_mwi)
		vty_out(vty, " ip sip enable mwi%s", VTY_NEWLINE);
	if (sip->sip_ims_enable)
		vty_out(vty, " ip sip enable ims%s", VTY_NEWLINE);

	if (sip->sip_srtp_mode == PJSIP_SRTP_DISABLE)
		vty_out(vty, " ip sip srtp mode disabled%s", VTY_NEWLINE);
	else if (sip->sip_srtp_mode == PJSIP_SRTP_OPTIONAL)
		vty_out(vty, " ip sip srtp mode optional%s", VTY_NEWLINE);
	else if (sip->sip_srtp_mode == PJSIP_SRTP_MANDATORY)
		vty_out(vty, " ip sip srtp mode mandatory%s", VTY_NEWLINE);
	else if (sip->sip_srtp_mode == PJSIP_SRTP_OPTIONAL_DUP)
		vty_out(vty, " ip sip srtp mode optional-duplicating%s", VTY_NEWLINE);
	if (sip->sip_srtp_mode != PJSIP_SRTP_DISABLE)
	{
		if (sip->sip_srtp_secure == PJSIP_SRTP_SEC_NO)
			vty_out(vty, " ip sip srtp secure none%s", VTY_NEWLINE);
		else if (sip->sip_srtp_secure == PJSIP_SRTP_SEC_TLS)
			vty_out(vty, " ip sip srtp secure tls%s", VTY_NEWLINE);
		else if (sip->sip_srtp_secure == PJSIP_SRTP_SEC_SIPS)
			vty_out(vty, " ip sip srtp secure sips%s", VTY_NEWLINE);

		if(sip->sip_srtp_keying == PJSIP_SRTP_KEYING_SDES)
			vty_out(vty, " ip sip srtp keying method sdes%s", VTY_NEWLINE);
		else if(sip->sip_srtp_keying == PJSIP_SRTP_KEYING_DTLS)
			vty_out(vty, " ip sip srtp keying method dtls%s", VTY_NEWLINE);
	}
	if (sip->sip_timer == PJSIP_TIMER_INACTIVE)
		vty_out(vty, " ip sip session-timers inactive%s", VTY_NEWLINE);
	else if (sip->sip_timer == PJSIP_TIMER_OPTIONAL)
		vty_out(vty, " ip sip session-timers optional%s", VTY_NEWLINE);
	else if (sip->sip_timer == PJSIP_TIMER_MANDATORY)
		vty_out(vty, " ip sip session-timers mandatory%s", VTY_NEWLINE);
	else if (sip->sip_timer == PJSIP_TIMER_ALWAYS)
		vty_out(vty, " ip sip session-timers always%s", VTY_NEWLINE);

	if (sip->sip_timer_sec && sip->sip_timer_sec != PJSIP_SESS_TIMER_DEF_SE)
		vty_out(vty, " ip sip session-timers expiration-period %d%s",
				sip->sip_timer_sec, VTY_NEWLINE);

/*	if (sip->sip_outb_rid)
		vty_out(vty, " ip sip outbound reg-id %d%s", sip->sip_outb_rid,
				VTY_NEWLINE);*/

	if (sip->sip_auto_update_nat)
		vty_out(vty, " ip sip traversal-behind symmetric nat%s", VTY_NEWLINE);

	if (sip->sip_stun_disable)
		vty_out(vty, " ip sip stun disabled%s", VTY_NEWLINE); //Disable STUN for this account
	return OK;
}

static int pjapp_cfg_transport_options_write_config(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zassert(sip != NULL);
	zassert(vty != NULL);
	//Transport Options:
	if(sip->sip_ipv6_enable)
		vty_out(vty, " ip sip ipv6 enabled%s", VTY_NEWLINE);

	if(sip->sip_set_qos)
		vty_out(vty, " ip sip tagging-qos enabled%s", VTY_NEWLINE);

	if(sip->sip_noudp)
		vty_out(vty, " ip sip transport udp disabled%s", VTY_NEWLINE);
	if(sip->sip_notcp)
		vty_out(vty, " ip sip transport tcp disabled%s", VTY_NEWLINE);

	//Add the specified nameserver to enable SRV resolution This option can be specified multiple times.
	//Set the URL of global outbound proxy server May be specified multiple times
	//Set STUN server host or domain. This option may be specified more than once. FORMAT is hostdom[:PORT]

	if(strlen(sip->sip_nameserver.sip_address))
		vty_out(vty, " ip sip nameserver %s%s", (sip->sip_nameserver.sip_address), VTY_NEWLINE);
	if(sip->sip_nameserver.sip_port && sip->sip_nameserver.sip_port != PJSIP_PORT_DEFAULT)
		vty_out(vty, " ip sip nameserver port %d%s", (sip->sip_nameserver.sip_port), VTY_NEWLINE);

	if(strlen(sip->sip_outbound.sip_address))
		vty_out(vty, " ip sip outbound %s%s", (sip->sip_outbound.sip_address), VTY_NEWLINE);
	if(sip->sip_outbound.sip_port && sip->sip_outbound.sip_port != PJSIP_PORT_DEFAULT)
		vty_out(vty, " ip sip outbound port %d%s", (sip->sip_outbound.sip_port), VTY_NEWLINE);

	if(strlen(sip->sip_stun_server.sip_address))
		vty_out(vty, " ip sip stun-server %s%s", (sip->sip_stun_server.sip_address), VTY_NEWLINE);
	if(sip->sip_stun_server.sip_port && sip->sip_stun_server.sip_port != PJSIP_PORT_DEFAULT)
		vty_out(vty, " ip sip stun-server port %d%s", (sip->sip_stun_server.sip_port), VTY_NEWLINE);
	return OK;
}

static int pjapp_cfg_tls_options_write_config(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zassert(sip != NULL);
	zassert(vty != NULL);
	//TLS Options:
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
	if(sip->sip_tls_enable)
		vty_out(vty, " ip sip tls enable%s", VTY_NEWLINE);
	if(strlen(sip->sip_tls_ca_file))
		vty_out(vty, " ip sip tls-ca-file %s%s", sip->sip_tls_ca_file, VTY_NEWLINE);
	if(strlen(sip->sip_tls_cert_file))
		vty_out(vty, " ip sip tls-cert-file %s%s", sip->sip_tls_cert_file, VTY_NEWLINE);
	if(strlen(sip->sip_tls_privkey_file))
		vty_out(vty, " ip sip tls-private-file %s%s", sip->sip_tls_privkey_file, VTY_NEWLINE);
	if(strlen(sip->sip_tls_password))
		vty_out(vty, " ip sip tls-password %s%s", sip->sip_tls_password, VTY_NEWLINE);

	if(strlen(sip->sip_tls_verify_server.sip_address))
		vty_out(vty, " ip sip verify-server %s%s", (sip->sip_tls_verify_server.sip_address), VTY_NEWLINE);
	if(sip->sip_tls_verify_server.sip_port && sip->sip_tls_verify_server.sip_port != PJSIP_PORT_DEFAULT)
		vty_out(vty, " ip sip verify-server port %d%s", (sip->sip_tls_verify_server.sip_port), VTY_NEWLINE);

	if(strlen(sip->sip_tls_verify_client.sip_address))
		vty_out(vty, " ip sip verify-client %s%s", (sip->sip_tls_verify_client.sip_address), VTY_NEWLINE);
	if(sip->sip_tls_verify_client.sip_port && sip->sip_tls_verify_client.sip_port != PJSIP_PORT_DEFAULT)
		vty_out(vty, " ip sip verify-client port %d%s", (sip->sip_tls_verify_client.sip_port), VTY_NEWLINE);

	if(strlen(sip->sip_tls_cipher))
		vty_out(vty, " ip sip tls-cipher %s%s", sip->sip_tls_cipher, VTY_NEWLINE);
#endif
	if(sip->sip_neg_timeout)
		vty_out(vty, " ip sip negotiation timeout %d%s", sip->sip_neg_timeout, VTY_NEWLINE);

	return OK;
}

static int pjapp_cfg_audio_options_write_config(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zassert(sip != NULL);
	zassert(vty != NULL);
	//Audio Options:
/*	char				sip_codec[PJSIP_DATA_MAX];
	char				sip_discodec[PJSIP_DATA_MAX];*/
	if(sip->sip_clock_rate != PJSIP_DEFAULT_CLOCK_RATE)
		vty_out(vty, " ip sip clock-rate %d%s", sip->sip_clock_rate, VTY_NEWLINE);

	if(sip->sip_snd_clock_rate != PJSIP_DEFAULT_CLOCK_RATE)
		vty_out(vty, " ip sip snd-clock-rate %d%s", sip->sip_snd_clock_rate, VTY_NEWLINE);

/*
	if(sip->sip_stereo)
		vty_out(vty, " ip sip stereo enable%s", VTY_NEWLINE);
	if(sip->sip_audio_null)
		vty_out(vty, " ip sip audio-null%s", VTY_NEWLINE);
*/

	if(strlen(sip->sip_play_file))
		vty_out(vty, " ip sip auto-play-file %s%s", sip->sip_play_file, VTY_NEWLINE);

	if(strlen(sip->sip_play_tone))
		vty_out(vty, " ip sip auto-play-tones %s%s", sip->sip_play_tone, VTY_NEWLINE);

	if(sip->sip_auto_play)
		vty_out(vty, " ip sip auto-play enable%s", VTY_NEWLINE);
	if(sip->sip_auto_loop)
		vty_out(vty, " ip sip auto-loop enable%s", VTY_NEWLINE);
	if(sip->sip_auto_conf)
		vty_out(vty, " ip sip auto-confured enable%s", VTY_NEWLINE);
	if(strlen(sip->sip_rec_file))
		vty_out(vty, " ip sip rec-file %s%s", sip->sip_rec_file, VTY_NEWLINE);
	if(sip->sip_quality != PJSUA_DEFAULT_CODEC_QUALITY)
		vty_out(vty, " ip sip media-quality %d%s", sip->sip_quality, VTY_NEWLINE);
	if(sip->sip_ptime != PJSUA_DEFAULT_AUDIO_FRAME_PTIME)
		vty_out(vty, " ip sip codec-ptime %d%s", sip->sip_ptime, VTY_NEWLINE);

	if(sip->sip_no_vad)
		vty_out(vty, " ip sip vad-silence disabled%s", VTY_NEWLINE);
	if(sip->sip_echo_tail != PJSUA_DEFAULT_EC_TAIL_LEN)
		vty_out(vty, " ip sip echo canceller tail %d%s", sip->sip_echo_tail, VTY_NEWLINE);

	if(sip->sip_echo_mode == PJSIP_ECHO_DEFAULT)
		vty_out(vty, " ip sip echo canceller algorithm default%s", VTY_NEWLINE);
	else if(sip->sip_echo_mode == PJSIP_ECHO_SPEEX)
		vty_out(vty, " ip sip echo canceller algorithm speex%s", VTY_NEWLINE);
	else if(sip->sip_echo_mode == PJSIP_ECHO_SUPPRESSER)
		vty_out(vty, " ip sip echo canceller algorithm suppressor%s", VTY_NEWLINE);
	else if(sip->sip_echo_mode == PJSIP_ECHO_WEBRTXC)
		vty_out(vty, " ip sip echo canceller algorithm webrtc%s", VTY_NEWLINE);
	if(sip->sip_ilbc_mode != PJSUA_DEFAULT_ILBC_MODE)
		vty_out(vty, " ip sip ilbc fps %d%s", sip->sip_ilbc_mode, VTY_NEWLINE);
	//zpl_uint32				sip_capture_dev;
	//zpl_uint32				sip_playback_dev;
#if 0
	if(sip->sip_capture_lat != PJMEDIA_SND_DEFAULT_REC_LATENCY)
		vty_out(vty, " ip sip capture latency %d%s", sip->sip_capture_lat, VTY_NEWLINE);
	if(sip->sip_playback_lat != PJMEDIA_SND_DEFAULT_PLAY_LATENCY)
		vty_out(vty, " ip sip playback latency %d%s", sip->sip_playback_lat, VTY_NEWLINE);
	if(sip->sip_snd_auto_close > 1)
		vty_out(vty, " ip sip auto-close delay %d%s", sip->sip_snd_auto_close, VTY_NEWLINE);

	if(sip->sip_jb_max_size > 0)
		vty_out(vty, " ip sip jitter max-size %d%s", sip->sip_jb_max_size, VTY_NEWLINE);//Specify jitter buffer maximum size, in frames (default=-1)");

	if(sip->sip_notones)
		vty_out(vty, " ip sip audible tones disabled%s", VTY_NEWLINE);
#endif
	return OK;
}

static int pjapp_cfg_video_options_write_config(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
#if PJSUA_HAS_VIDEO
	zassert(sip != NULL);
	zassert(vty != NULL);
	//Video Options:
	if(sip->sip_video)
		vty_out(vty, " ip sip video enable%s", VTY_NEWLINE);
	if(strlen(sip->sip_play_avi))
		vty_out(vty, " ip sip video play-file %s%s", sip->sip_play_avi, VTY_NEWLINE);
	if(sip->sip_auto_play_avi)
		vty_out(vty, " ip sip video auto-play enable%s", VTY_NEWLINE);
#endif
	return OK;
}


static int pjapp_cfg_media_transport_options_write_config(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zassert(sip != NULL);
	zassert(vty != NULL);
	//Media Transport Options:
	if(sip->sip_ice)
		vty_out(vty, " ip sip ice enable%s", VTY_NEWLINE);

	if(sip->sip_ice_regular)
		vty_out(vty, " ip sip ice regular %d%s", sip->sip_ice_regular, VTY_NEWLINE);

	if(sip->sip_ice_max_host)
		vty_out(vty, " ip sip ice max-host %d%s", sip->sip_ice_max_host, VTY_NEWLINE);

	if(sip->sip_ice_nortcp)
		vty_out(vty, " ip sip ice notcp%s", VTY_NEWLINE);

/*
	if(sip->sip_ice_max_host)
		vty_out(vty, " ip sip ice max-host %d%s", sip->sip_ice_max_host, VTY_NEWLINE);
*/

	if(sip->sip_rtp_port != PJSIP_RTP_PORT_DEFAULT)
		vty_out(vty, " ip sip rtp port %d%s", sip->sip_rtp_port, VTY_NEWLINE);

	if(sip->sip_rx_drop_pct)
		vty_out(vty, " ip sip drop rx-rtp %d%s", sip->sip_rx_drop_pct, VTY_NEWLINE);

	if(sip->sip_tx_drop_pct)
		vty_out(vty, " ip sip drop tx-rtp %d%s", sip->sip_rx_drop_pct, VTY_NEWLINE);

	if(sip->sip_turn)
		vty_out(vty, " ip sip turn enable%s", VTY_NEWLINE);

	if(strlen(sip->sip_turn_srv.sip_address))
		vty_out(vty, " ip sip turn-server %s%s", (sip->sip_turn_srv.sip_address), VTY_NEWLINE);
	if(sip->sip_turn_srv.sip_port && sip->sip_turn_srv.sip_port != PJSIP_PORT_DEFAULT)
		vty_out(vty, " ip sip turn-server port %d%s", (sip->sip_turn_srv.sip_port), VTY_NEWLINE);

	if(sip->sip_turn_tcp)
		vty_out(vty, " ip sip turn-tcp enable%s", VTY_NEWLINE);
	if(strlen(sip->sip_turn_user))
		vty_out(vty, " ip sip turn username %s%s", sip->sip_turn_user, VTY_NEWLINE);
	if(strlen(sip->sip_turn_password))
		vty_out(vty, " ip sip turn password %s%s", sip->sip_turn_password, VTY_NEWLINE);

/*	if(sip->sip_rtcp_mux)
		vty_out(vty, " ip sip rtp-multiplexing %d%s", sip->sip_rtcp_mux, VTY_NEWLINE);*/
	return OK;
}

static int pjapp_cfg_user_agent_options_write_config(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zassert(sip != NULL);
	zassert(vty != NULL);
	//Buddy List (can be more than one):
	//void				*buddy_list;
	//User Agent options:
	if(sip->sip_auto_answer_code)
		vty_out(vty, " ip sip auto-answer-code %d%s", sip->sip_auto_answer_code, VTY_NEWLINE);

	if(sip->sip_max_calls != PJSUA_MAX_CALLS)
		vty_out(vty, " ip sip max calls %d%s", sip->sip_max_calls, VTY_NEWLINE);

/*
	if(sip->sip_thread_max)
		vty_out(vty, " ip sip max threads %d%s", sip->sip_thread_max, VTY_NEWLINE);
*/


	if(sip->sip_duration != PJSUA_APP_NO_LIMIT_DURATION)
		vty_out(vty, " ip sip max call duration %d%s", sip->sip_duration, VTY_NEWLINE);
	else
		vty_out(vty, " ip sip max call duration no-limit%s", VTY_NEWLINE);

	if(sip->sip_norefersub)
		vty_out(vty, " ip sip refer subscription disabled%s", VTY_NEWLINE);

/*
	if(sip->sip_use_compact_form)
		vty_out(vty, " ip sip message min-size %d%s", sip->sip_use_compact_form, VTY_NEWLINE);

	if(sip->sip_no_force_lr)
		vty_out(vty, " ip sip force-lr%s", VTY_NEWLINE);
*/

	if(sip->sip_accept_redirect == PJSIP_ACCEPT_REDIRECT_REJECT)
		vty_out(vty, " ip sip redirect method redirect%s", VTY_NEWLINE);
	else if(sip->sip_accept_redirect == PJSIP_ACCEPT_REDIRECT_FOLLOW)
		vty_out(vty, " ip sip redirect method follow%s", VTY_NEWLINE);
	else if(sip->sip_accept_redirect == PJSIP_ACCEPT_REDIRECT_FOLLOW_REPLACE)
		vty_out(vty, " ip sip redirect method follow-replace%s", VTY_NEWLINE);
	else if(sip->sip_accept_redirect == PJSIP_ACCEPT_REDIRECT_ASK)
		vty_out(vty, " ip sip redirect method ask%s", VTY_NEWLINE);
	return OK;
}
/***************************************************************************/
static int pjapp_cfg_account_options_show(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zassert(sip != NULL);
	zassert(vty != NULL);
	char buftmp[4096];
	zpl_uint32 i = 0;
	vty_out(vty, " sip username                 : %s%s",
			strlen(sip->sip_user.sip_user)? sip->sip_user.sip_user:" ", VTY_NEWLINE);
	vty_out(vty, " sip local-phone              : %s%s",
			strlen(sip->sip_user.sip_phone)? sip->sip_user.sip_phone:" ", VTY_NEWLINE);
	vty_out(vty, " sip password                 : %s%s",
			strlen(sip->sip_user.sip_password)? sip->sip_user.sip_password:" ", VTY_NEWLINE);

	vty_out(vty, " sip username                 : %s secondary%s",
			strlen(sip->sip_user_sec.sip_user)? sip->sip_user_sec.sip_user:" ", VTY_NEWLINE);

	vty_out(vty, " sip local-phone              : %s secondary%s",
			strlen(sip->sip_user_sec.sip_phone)? sip->sip_user_sec.sip_phone:" ", VTY_NEWLINE);

	vty_out(vty, " sip password                 : %s secondary%s",
			strlen(sip->sip_user_sec.sip_password)? sip->sip_user_sec.sip_password:" ", VTY_NEWLINE);

	vty_out(vty, " sip local-port               : %d%s", (sip->sip_local.sip_port), VTY_NEWLINE);
	vty_out(vty, " sip local-address            : %s%s", (sip->sip_local.sip_address), VTY_NEWLINE);
	vty_out(vty, " sip source-interface         : %s%s", if_ifname_make(sip->sip_source_interface), VTY_NEWLINE);
	vty_out(vty, " sip server                   : %s%s", (sip->sip_server.sip_address), VTY_NEWLINE);
	vty_out(vty, " sip server port              : %d%s", (sip->sip_server.sip_port), VTY_NEWLINE);
	vty_out(vty, " sip server                   : %s secondary%s",(sip->sip_server_sec.sip_address), VTY_NEWLINE);
	vty_out(vty, " sip server port              : %d secondary%s", (sip->sip_server_sec.sip_port), VTY_NEWLINE);

	vty_out(vty, " sip proxy                    : %s%s", sip->sip_proxy_enable ? "zpl_true":"zpl_false", VTY_NEWLINE);
	vty_out(vty, " sip proxy-server             : %s%s", (sip->sip_proxy.sip_address), VTY_NEWLINE);
	vty_out(vty, " sip proxy-server port        : %d%s", (sip->sip_proxy.sip_port), VTY_NEWLINE);
	vty_out(vty, " sip proxy-server             : %s secondary%s", (sip->sip_proxy_sec.sip_address), VTY_NEWLINE);
	vty_out(vty, " sip proxy-server port        : %d secondary%s", (sip->sip_proxy_sec.sip_port), VTY_NEWLINE);
	if (sip->sip_reg_proxy == PJSIP_REGISTER_NONE)
		vty_out(vty, " sip reg-proxy                : disable%s", VTY_NEWLINE);
	else if (sip->sip_reg_proxy == PJSIP_REGISTER_NO_PROXY)
		vty_out(vty, " sip reg-proxy                : none%s", VTY_NEWLINE);
	else if (sip->sip_reg_proxy == PJSIP_REGISTER_OUTBOUND_PROXY)
		vty_out(vty, " sip reg-proxy                : outbound%s", VTY_NEWLINE);
	else if (sip->sip_reg_proxy == PJSIP_REGISTER_ACC_ONLY)
		vty_out(vty, " sip reg-proxy                : acc%s", VTY_NEWLINE);
	else if (sip->sip_reg_proxy == PJSIP_REGISTER_ALL)
		vty_out(vty, " sip reg-proxy                : all%s", VTY_NEWLINE);

	if(sip->proto == PJSIP_PROTO_UDP)
		vty_out(vty, " sip transport                : udp%s", VTY_NEWLINE);
	else if(sip->proto == PJSIP_PROTO_TCP)
		vty_out(vty, " sip transport                : tcp%s", VTY_NEWLINE);
	else if(sip->proto == PJSIP_PROTO_TLS)
		vty_out(vty, " sip transport                : tls%s", VTY_NEWLINE);
	else if(sip->proto == PJSIP_PROTO_DTLS)
		vty_out(vty, " sip transport                : dtls%s", VTY_NEWLINE);

	if(sip->dtmf == PJSIP_DTMF_INFO)
		vty_out(vty, " sip dtmf-type                : sip-info%s", VTY_NEWLINE);
	else if(sip->dtmf == PJSIP_DTMF_RFC2833)
		vty_out(vty, " sip dtmf-type                : rfc2833%s", VTY_NEWLINE);
	else if(sip->dtmf == PJSIP_DTMF_INBAND)
		vty_out(vty, " sip dtmf-type                : inband%s", VTY_NEWLINE);

	vty_out(vty, " sip default codec            : %s%s", strlwr(_pjapp_cfg->sip_codec.payload_name), VTY_NEWLINE);

	memset(buftmp, 0, sizeof(buftmp));
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(_pjapp_cfg->codec[i].is_active)
		{
			strcat(buftmp, _pjapp_cfg->codec[i].payload_name);
			strcat(buftmp, " ");
		}
	}
	vty_out(vty, " sip codec                    : %s%s", strlwr(buftmp), VTY_NEWLINE);
	memset(buftmp, 0, sizeof(buftmp));
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(_pjapp_cfg->dicodec[i].is_active)
		{
			strcat(buftmp, _pjapp_cfg->dicodec[i].payload_name);
			strcat(buftmp, " ");
		}
	}
	vty_out(vty, " sip discodec                 : %s%s", buftmp, VTY_NEWLINE);
	//vty_out(vty, " sip time-sync        : %s%s", sip->sip_time_sync ? "zpl_true":"zpl_false",VTY_NEWLINE);
	//vty_out(vty, " sip ring             : %d%s", sip->sip_ring ,VTY_NEWLINE);

	vty_out(vty, " sip expires                  : %d%s", sip->sip_expires, VTY_NEWLINE);
	vty_out(vty, " sip publish                  : %s%s", sip->sip_publish? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip mwi                      : %s%s", sip->sip_mwi? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip rel-100                  : %s%s", sip->sip_100_rel ? "zpl_true":"zpl_false", VTY_NEWLINE);
	vty_out(vty, " sip ims                      : %s%s", sip->sip_ims_enable? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip rereg-delay              : %d%s", sip->sip_rereg_delay, VTY_NEWLINE);
	vty_out(vty, " sip realm                    : %s%s", strlen(sip->sip_realm)? sip->sip_realm:" ", VTY_NEWLINE);

	if (sip->sip_srtp_mode == PJSIP_SRTP_DISABLE)
		vty_out(vty, " sip srtp                     : disabled%s", VTY_NEWLINE);
	else if (sip->sip_srtp_mode == PJSIP_SRTP_OPTIONAL)
		vty_out(vty, " sip srtp                     : optional%s", VTY_NEWLINE);
	else if (sip->sip_srtp_mode == PJSIP_SRTP_MANDATORY)
		vty_out(vty, " sip srtp                     : mandatory%s", VTY_NEWLINE);
	else if (sip->sip_srtp_mode == PJSIP_SRTP_OPTIONAL_DUP)
		vty_out(vty, " sip srtp                     : optional-duplicating%s", VTY_NEWLINE);

	if (sip->sip_srtp_secure == PJSIP_SRTP_SEC_NO)
		vty_out(vty, " sip srtp secure              : none%s", VTY_NEWLINE);
	else if (sip->sip_srtp_secure == PJSIP_SRTP_SEC_TLS)
		vty_out(vty, " sip srtp secure              : tls%s", VTY_NEWLINE);
	else if (sip->sip_srtp_secure == PJSIP_SRTP_SEC_SIPS)
		vty_out(vty, " sip srtp secure              : sips%s", VTY_NEWLINE);

	if (sip->sip_timer == PJSIP_TIMER_INACTIVE)
		vty_out(vty, " sip session-timers           : inactive%s", VTY_NEWLINE);
	else if (sip->sip_timer == PJSIP_TIMER_OPTIONAL)
		vty_out(vty, " sip session-timers           : optional%s", VTY_NEWLINE);
	else if (sip->sip_timer == PJSIP_TIMER_MANDATORY)
		vty_out(vty, " sip session-timers           : mandatory%s", VTY_NEWLINE);
	else if (sip->sip_timer == PJSIP_TIMER_ALWAYS)
		vty_out(vty, " sip session-timers           : always%s", VTY_NEWLINE);

	vty_out(vty, " sip session-timers expiration-period: %d%s",
				sip->sip_timer_sec, VTY_NEWLINE);

	vty_out(vty, " sip outbound reg-id          : %d%s", sip->sip_outb_rid,VTY_NEWLINE);

	vty_out(vty, " sip auto update nat          : %s%s", sip->sip_auto_update_nat? "zpl_true":"zpl_false", VTY_NEWLINE);
	vty_out(vty, " sip stun                     : %s%s", sip->sip_stun_disable? "zpl_true":"zpl_false", VTY_NEWLINE); //Disable STUN for this account
	return OK;
}

static int pjapp_cfg_transport_options_show(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zassert(sip != NULL);
	zassert(vty != NULL);
	//Transport Options:
	//vty_out(vty, " sip expires          : %d%s", sip->sip_expires, VTY_NEWLINE);
	vty_out(vty, " sip ipv6                     : %s%s", sip->sip_ipv6_enable? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip tagging-qos              : %s%s", sip->sip_set_qos? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip transport udp            : %s%s", sip->sip_noudp? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip transport tcp            : %s%s", sip->sip_notcp? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip nameserver               : %s%s", (sip->sip_nameserver.sip_address), VTY_NEWLINE);
	vty_out(vty, " sip nameserver port          : %d%s", (sip->sip_nameserver.sip_port), VTY_NEWLINE);
	vty_out(vty, " sip outbound                 : %s%s", (sip->sip_outbound.sip_address), VTY_NEWLINE);
	vty_out(vty, " sip outbound port            : %d%s", (sip->sip_outbound.sip_port), VTY_NEWLINE);
	vty_out(vty, " sip stun-server              : %s%s", (sip->sip_stun_server.sip_address), VTY_NEWLINE);
	vty_out(vty, " sip stun-server port         : %d%s", (sip->sip_stun_server.sip_port), VTY_NEWLINE);
	return OK;
}

static int pjapp_cfg_tls_options_show(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zassert(sip != NULL);
	zassert(vty != NULL);
	//TLS Options:
	vty_out(vty, " sip tls                      : %s%s", sip->sip_tls_enable? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip tls-ca-file              : %s%s", sip->sip_tls_ca_file, VTY_NEWLINE);
	vty_out(vty, " sip tls-cert-file            : %s%s", sip->sip_tls_cert_file, VTY_NEWLINE);
	vty_out(vty, " sip tls-private-file         :%s%s", sip->sip_tls_privkey_file, VTY_NEWLINE);
	vty_out(vty, " sip tls-password             : %s%s", sip->sip_tls_password, VTY_NEWLINE);
	vty_out(vty, " sip verify-server            : %s%s", (sip->sip_tls_verify_server.sip_address), VTY_NEWLINE);
	vty_out(vty, " sip verify-server port       : %d%s", (sip->sip_tls_verify_server.sip_port), VTY_NEWLINE);
	vty_out(vty, " sip verify-client            : %s%s", (sip->sip_tls_verify_client.sip_address), VTY_NEWLINE);
	vty_out(vty, " sip verify-client port       : %d%s", (sip->sip_tls_verify_client.sip_port), VTY_NEWLINE);
	vty_out(vty, " sip tls negotiation timeout  : %d%s", sip->sip_neg_timeout, VTY_NEWLINE);
	vty_out(vty, " sip tls-cipher               : %s%s", sip->sip_tls_cipher, VTY_NEWLINE);
	return OK;
}

static int pjapp_cfg_audio_options_show(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zassert(sip != NULL);
	zassert(vty != NULL);
	//Audio Options:
/*	char				sip_codec[PJSIP_DATA_MAX];
	char				sip_discodec[PJSIP_DATA_MAX];*/
	//vty_out(vty, " sip tls              : %s%s", sip->sip_tls_enable? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip clock-rate               : %d%s", sip->sip_clock_rate, VTY_NEWLINE);
	vty_out(vty, " sip snd-clock-rate           : %d%s", sip->sip_snd_clock_rate, VTY_NEWLINE);
	vty_out(vty, " sip stereo                   : %s%s", sip->sip_stereo? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip audio-null               : %s%s", sip->sip_audio_null? "zpl_true":"zpl_false",VTY_NEWLINE);

	vty_out(vty, " sip play-file                : %s%s", sip->sip_play_file, VTY_NEWLINE);
	vty_out(vty, " sip play-tones               : %s%s", sip->sip_play_tone, VTY_NEWLINE);
	vty_out(vty, " sip auto-play                : %s%s", sip->sip_auto_play? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip auto-loop                : %s%s", sip->sip_auto_loop? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip auto-confured            : %s%s", sip->sip_auto_conf? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip vad-silence              : %s%s", sip->sip_no_vad? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip rec-file                 : %s%s", sip->sip_rec_file, VTY_NEWLINE);
	vty_out(vty, " sip media-quality            : %d%s", sip->sip_quality, VTY_NEWLINE);
	vty_out(vty, " sip codec-ptime              : %d%s", sip->sip_ptime, VTY_NEWLINE);
	vty_out(vty, " sip echo canceller tail      : %d%s", sip->sip_echo_tail, VTY_NEWLINE);

	if(sip->sip_echo_mode == PJSIP_ECHO_DEFAULT)
		vty_out(vty, " sip echo mode                : default%s", VTY_NEWLINE);
	else if(sip->sip_echo_mode == PJSIP_ECHO_SPEEX)
		vty_out(vty, " sip echo mode                : speex%s", VTY_NEWLINE);
	else if(sip->sip_echo_mode == PJSIP_ECHO_SUPPRESSER)
		vty_out(vty, " sip echo mode                : suppressor%s", VTY_NEWLINE);
	else if(sip->sip_echo_mode == PJSIP_ECHO_WEBRTXC)
		vty_out(vty, " sip echo mode                : webrtc%s", VTY_NEWLINE);

	vty_out(vty, " sip ilbc fps                 : %d%s", sip->sip_ilbc_mode, VTY_NEWLINE);
	vty_out(vty, " sip capture latency          : %d%s", sip->sip_capture_lat, VTY_NEWLINE);
	vty_out(vty, " sip playback latency         : %d%s", sip->sip_playback_lat, VTY_NEWLINE);
	vty_out(vty, " sip auto-close delay         : %d%s", sip->sip_snd_auto_close, VTY_NEWLINE);
	vty_out(vty, " sip jitter max-size          : %d%s", sip->sip_jb_max_size, VTY_NEWLINE);//Specify jitter buffer maximum size, in frames (default=-1)");
	vty_out(vty, " sip audible tones            : %s%s", sip->sip_notones? "zpl_true":"zpl_false",VTY_NEWLINE);

	return OK;
}

static int pjapp_cfg_video_options_show(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
#if PJSUA_HAS_VIDEO
	zassert(sip != NULL);
	zassert(vty != NULL);
	//Video Options:
	vty_out(vty, " sip video                    : %s%s", sip->sip_video? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip video play-file          : %s%s", sip->sip_play_avi, VTY_NEWLINE);
	vty_out(vty, " sip video auto-play          : %s%s", sip->sip_auto_play_avi? "zpl_true":"zpl_false",VTY_NEWLINE);
#endif
	return OK;
}


static int pjapp_cfg_media_transport_options_show(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zassert(sip != NULL);
	zassert(vty != NULL);
	//Media Transport Options:

	vty_out(vty, " sip ice                      : %s%s", sip->sip_ice? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip ice regular              : %d%s", sip->sip_ice_regular, VTY_NEWLINE);
	vty_out(vty, " sip ice max-host             : %d%s", sip->sip_ice_max_host, VTY_NEWLINE);
	vty_out(vty, " sip ice notcp                : %s%s", sip->sip_ice_nortcp? "zpl_true":"zpl_false",VTY_NEWLINE);

	vty_out(vty, " sip rtp port                 : %d%s", sip->sip_rtp_port, VTY_NEWLINE);
	vty_out(vty, " sip drop rx-rtp              : %d%s", sip->sip_rx_drop_pct, VTY_NEWLINE);
	vty_out(vty, " sip drop tx-rtp              : %d%s", sip->sip_rx_drop_pct, VTY_NEWLINE);
	vty_out(vty, " sip turn                     : %s%s", sip->sip_turn? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip turn-server              : %s%s", (sip->sip_turn_srv.sip_address), VTY_NEWLINE);
	vty_out(vty, " sip turn-server port         : %d%s", (sip->sip_turn_srv.sip_port), VTY_NEWLINE);
	vty_out(vty, " sip turn-tcp                 : %s%s", sip->sip_turn_tcp? "zpl_true":"zpl_false",VTY_NEWLINE);

	vty_out(vty, " sip turn username            : %s%s", sip->sip_turn_user, VTY_NEWLINE);
	vty_out(vty, " sip turn password            : %s%s", sip->sip_turn_password, VTY_NEWLINE);
	vty_out(vty, " sip rtp-multiplexing         : %s%s", sip->sip_rtcp_mux? "zpl_true":"zpl_false", VTY_NEWLINE);

	if(sip->sip_srtp_keying == PJSIP_SRTP_KEYING_SDES)
		vty_out(vty, " sip srtp keying              : sdes%s", VTY_NEWLINE);
	else if(sip->sip_srtp_keying == PJSIP_SRTP_KEYING_DTLS)
		vty_out(vty, " sip srtp keying              : dtls%s", VTY_NEWLINE);
	return OK;
}

static int pjapp_cfg_user_agent_options_show(pjapp_cfg_t *sip, struct vty *vty, zpl_bool detail, zpl_bool bwrt)
{
	zassert(sip != NULL);
	zassert(vty != NULL);
	//Buddy List (can be more than one):
	//void				*buddy_list;
	//User Agent options:
	vty_out(vty, " sip auto-answer-code         : %d%s", sip->sip_auto_answer_code, VTY_NEWLINE);
	vty_out(vty, " sip max calls                : %d%s", sip->sip_max_calls, VTY_NEWLINE);
	vty_out(vty, " sip max threads              : %d%s", sip->sip_thread_max, VTY_NEWLINE);
	if(sip->sip_duration == PJSUA_APP_NO_LIMIT_DURATION)
		vty_out(vty, " sip max call duration        : no-limit%s", VTY_NEWLINE);
	else
		vty_out(vty, " sip max call duration        : %d%s", sip->sip_duration, VTY_NEWLINE);
	vty_out(vty, " sip refer subscription       : %s%s", sip->sip_norefersub? "zpl_true":"zpl_false",VTY_NEWLINE);
	vty_out(vty, " sip message min-size         : %d%s", sip->sip_use_compact_form, VTY_NEWLINE);
	vty_out(vty, " sip force-lr                 : %s%s", sip->sip_no_force_lr? "zpl_true":"zpl_false",VTY_NEWLINE);
	if(sip->sip_accept_redirect == PJSIP_ACCEPT_REDIRECT_REJECT)
		vty_out(vty, " sip redirect method          : redirect%s", VTY_NEWLINE);
	else if(sip->sip_accept_redirect == PJSIP_ACCEPT_REDIRECT_FOLLOW)
		vty_out(vty, " sip redirect method          : follow%s", VTY_NEWLINE);
	else if(sip->sip_accept_redirect == PJSIP_ACCEPT_REDIRECT_FOLLOW_REPLACE)
		vty_out(vty, " sip redirect method          : follow-replace%s", VTY_NEWLINE);
	else if(sip->sip_accept_redirect == PJSIP_ACCEPT_REDIRECT_ASK)
		vty_out(vty, " sip redirect method          : ask%s", VTY_NEWLINE);
	return OK;
}
/***************************************************************************/
int pjapp_cfg_show_account_state(void *p)
{
	char idtmp[6], sipurl[20], sipreg[20],sippro[10],tmp[10];
	pjapp_cfg_t *sip = _pjapp_cfg;
	struct vty *vty = (struct vty *)p;
	zassert(_pjapp_cfg != NULL);
	zassert(vty != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);

	//pjapp_cfg_app_list_acc(0);
	/*
	 *[ 1] sip:100@192.168.224.129: 200/OK (expires=44)
	       Online status: Online
	 */
	vty_out(vty, " %-2s %-8s %-20s %-8s %-16s %-16s %-16s%s",
			"--", "--------", "--------------------", "--------",
			"----------------", "----------------", "----------------", VTY_NEWLINE);
	vty_out(vty, " %-2s %-8s %-20s %-8s %-16s %-16s %-16s%s",
			"id", "username", "server url", "proto", "reg-state", "is default", "is current", VTY_NEWLINE);

	vty_out(vty, " %-2s %-8s %-20s %-8s %-16s %-16s %-16s%s",
			"--", "--------", "--------------------", "--------",
			"----------------", "----------------", "----------------", VTY_NEWLINE);

	if(strlen(sip->sip_user.sip_user))
	{
		memset(idtmp, 0, sizeof(idtmp));
		memset(sipurl, 0, sizeof(sipurl));
		memset(sipreg, 0, sizeof(sipreg));
		memset(sippro, 0, sizeof(sippro));
		memset(tmp, 0, sizeof(tmp));

		snprintf(idtmp, sizeof(idtmp), "%d", sip->sip_user.id);
		if(sip->sip_user.register_svr)
		{
			snprintf(sipurl, sizeof(sipurl), "%s:%d", strlen(sip->sip_user.register_svr->sip_address)?sip->sip_user.register_svr->sip_address:"null",
					sip->sip_user.register_svr->sip_port);
		}
		snprintf(sippro, sizeof(sippro), "sip");
		if(sip->sip_user.sip_state == PJSIP_STATE_UNREGISTER)
			snprintf(sipreg, sizeof(sipreg), "unregister");
		else if(sip->sip_user.sip_state == PJSIP_STATE_REGISTER_FAILED)
			snprintf(sipreg, sizeof(sipreg), "failed");
		else if(sip->sip_user.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			snprintf(sipreg, sizeof(sipreg), "success");
		else
			snprintf(sipreg, sizeof(sipreg), "unknown");

		vty_out(vty, " %-2s %-8s %-20s %-8s %-16s %-16s %-16s%s",
				idtmp, sip->sip_user.sip_user, sipurl, sippro, sipreg,
				sip->sip_user.is_default ? "yes":"no", sip->sip_user.is_current?"yes":"no", VTY_NEWLINE);
	}
	if(strlen(sip->sip_user_sec.sip_user))
	{
		memset(idtmp, 0, sizeof(idtmp));
		memset(sipurl, 0, sizeof(sipurl));
		memset(sipreg, 0, sizeof(sipreg));
		memset(sippro, 0, sizeof(sippro));
		memset(tmp, 0, sizeof(tmp));

		snprintf(idtmp, sizeof(idtmp), "%d", sip->sip_user_sec.id);
		if(sip->sip_user_sec.register_svr)
		{
			snprintf(sipurl, sizeof(sipurl), "%s:%d", strlen(sip->sip_user_sec.register_svr->sip_address)?sip->sip_user_sec.register_svr->sip_address:"null",
					sip->sip_user_sec.register_svr->sip_port);
		}
		snprintf(sippro, sizeof(sippro), "sip");
		if(sip->sip_user_sec.sip_state == PJSIP_STATE_UNREGISTER)
			snprintf(sipreg, sizeof(sipreg), "unregister");
		else if(sip->sip_user_sec.sip_state == PJSIP_STATE_REGISTER_FAILED)
			snprintf(sipreg, sizeof(sipreg), "failed");
		else if(sip->sip_user_sec.sip_state == PJSIP_STATE_REGISTER_SUCCESS)
			snprintf(sipreg, sizeof(sipreg), "success");
		else
			snprintf(sipreg, sizeof(sipreg), "unknown");

		vty_out(vty, " %-2s %-8s %-20s %-8s %-16s %-16s %-16s%s",
				idtmp, sip->sip_user_sec.sip_user, sipurl, sippro, sipreg,
				sip->sip_user_sec.is_default ? "yes":"no", sip->sip_user_sec.is_current?"yes":"no", VTY_NEWLINE);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}
/***************************************************************************/
int pjapp_cfg_write_config(void *p)
{
	pjapp_cfg_t *sip = _pjapp_cfg;
	struct vty *vty = (struct vty *)p;
	zassert(_pjapp_cfg != NULL);
	zassert(vty != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip->sip_enable)
	{
		//vty_out(vty, "service sip%s", VTY_NEWLINE);

		pjapp_cfg_account_options_write_config(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_transport_options_write_config(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_tls_options_write_config(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_audio_options_write_config(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_video_options_write_config(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_media_transport_options_write_config(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_user_agent_options_write_config(sip, vty, zpl_false, zpl_true);
		//voip_volume_write_config(vty);
		vty_out(vty, "!%s",VTY_NEWLINE);
	}
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

int pjapp_cfg_show_config(void *p, zpl_bool detail)
{
	pjapp_cfg_t *sip = _pjapp_cfg;
	struct vty *vty = (struct vty *)p;
	zassert(_pjapp_cfg != NULL);
	zassert(vty != NULL);
	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);
	if(sip->sip_enable)
	{
		vty_out(vty, "SIP Service :%s", VTY_NEWLINE);
		pjapp_cfg_account_options_show(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_transport_options_show(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_tls_options_show(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_audio_options_show(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_video_options_show(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_media_transport_options_show(sip, vty, zpl_false, zpl_true);
		pjapp_cfg_user_agent_options_show(sip, vty, zpl_false, zpl_true);

		//voip_volume_show_config(vty, zpl_false);
		vty_out(vty, "%s",VTY_NEWLINE);
	}
	else
		vty_out(vty, "SIP Service is not enable%s", VTY_NEWLINE);
	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);
	return OK;
}

