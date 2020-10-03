/*
 * pjsip_cfg.c
 *
 *  Created on: Feb 2, 2019
 *      Author: zhurish
 */

/* $Id: main.c 4752 2014-02-19 08:57:22Z ming $ */
/*
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ua program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "pjsua_app.h"
#include "pjsip_main.h"
#include "pjsip_app_api.h"
#include "pjsip_jsoncfg.h"
#include "pjsip_cfg.h"

#define THIS_FILE "pjsip_cfg.c"

//#define PJ_STR_STRDUP(n)	if((n).ptr) return strdup((n).ptr); else return NULL;
#define PJ_STR_STRDUP(n) pj_str_strdup(n)

static char *pj_str_strdup(pj_str_t str)
{
	if (str.ptr && str.slen > 0)
		return strdup(str.ptr);
	return NULL;
}

static int pjsip_ua_config_default(pjsip_ua_config_t *ua)
{
	if (ua)
	{
		unsigned int i;
		pjsua_config ua_cfg;
		pjsua_config_default(&ua_cfg);

		ua->maxCalls = ua_cfg.max_calls;
		ua->threadCnt = ua_cfg.thread_cnt;
		ua->userAgent = PJ_STR_STRDUP(ua_cfg.user_agent);
		ua->nameserverCnt = ua->stunServerCnt = ua->outboundProxiesCnt = 0;

		for (i = 0; i < ua_cfg.nameserver_count && i < PJSIP_NAMESERVER_MAX; ++i)
		{
			if (ua_cfg.nameserver[i].ptr)
			{
				ua->nameserver[ua->nameserverCnt++] = strdup(ua_cfg.nameserver[i].ptr);
			}
		}

		for (i = 0; i < ua_cfg.stun_srv_cnt && i < PJSIP_STUNSERVER_MAX; ++i)
		{
			if (ua_cfg.stun_srv[i].ptr)
			{
				ua->stunServer[ua->stunServerCnt++] = strdup(ua_cfg.nameserver[i].ptr);
			}
		}
		for (i = 0; i < ua_cfg.outbound_proxy_cnt && i < PJSIP_OUTBOUND_PROXY_MAX; ++i)
		{
			if (ua_cfg.outbound_proxy[i].ptr)
			{
				ua->outboundProxies[ua->outboundProxiesCnt++] = strdup(ua_cfg.outbound_proxy[i].ptr);
			}
		}

		ua->stunTryIpv6 = (ua_cfg.stun_try_ipv6);
		ua->stunIgnoreFailure = (ua_cfg.stun_ignore_failure);
		ua->natTypeInSdp = ua_cfg.nat_type_in_sdp;
		ua->mwiUnsolicitedEnabled = (ua_cfg.enable_unsolicited_mwi);
		return OK;
	}
	return ERROR;
}

static pjsip_ua_config_t *pjsip_ua_config_new()
{
	pjsip_ua_config_t *ua = malloc(sizeof(pjsip_ua_config_t));
	if (ua)
	{
		memset(ua, 0, sizeof(pjsip_ua_config_t));
		return ua;
	}
	return NULL;
}

static int pjsip_ua_config_destroy(pjsip_ua_config_t *ua)
{
	if (ua)
	{
		unsigned int i;
		if (ua->userAgent)
			free(ua->userAgent);

		for (i = 0; i < ua->nameserverCnt; ++i)
		{
			if (ua->nameserver[i])
			{
				free(ua->nameserver[i]);
			}
		}

		for (i = 0; i < ua->stunServerCnt; ++i)
		{
			if (ua->stunServer[i])
			{
				free(ua->stunServer[i]);
			}
		}
		for (i = 0; i < ua->outboundProxiesCnt; ++i)
		{
			if (ua->outboundProxies[i])
			{
				free(ua->outboundProxies[i]);
			}
		}
		free(ua);
		ua = NULL;
	}
	return OK;
}

//log
static int pjsip_log_config_default(pjsip_log_config_t *ua)
{
	if (ua)
	{
		pjsua_logging_config lc;
		pjsua_logging_config_default(&lc);

		ua->msgLogging = lc.msg_logging;
		ua->level = lc.level;
		ua->consoleLevel = lc.console_level;
		ua->decor = lc.decor;
		ua->filename = PJ_STR_STRDUP(lc.log_filename);
		ua->fileFlags = lc.log_file_flags;
		//ua->writer = NULL;

		return OK;
	}
	return ERROR;
}

static pjsip_log_config_t *pjsip_log_config_new()
{
	pjsip_log_config_t *ua = malloc(sizeof(pjsip_log_config_t));
	if (ua)
	{
		memset(ua, 0, sizeof(pjsip_log_config_t));
		return ua;
	}
	return NULL;
}

static int pjsip_log_config_destroy(pjsip_log_config_t *ua)
{
	if (ua)
	{
		if (ua->filename)
			free(ua->filename);
		free(ua);
		ua = NULL;
	}
	return OK;
}

//media
static int pjsip_media_config_default(pjsip_media_config_t *ua)
{
	if (ua)
	{
		pjsua_media_config mc;
		pjsua_media_config_default(&mc);
		ua->clockRate = mc.clock_rate;
		ua->sndClockRate = mc.snd_clock_rate;
		ua->channelCount = mc.channel_count;
		ua->audioFramePtime = mc.audio_frame_ptime;
		ua->maxMediaPorts = mc.max_media_ports;
		ua->hasIoqueue = (mc.has_ioqueue);
		ua->threadCnt = mc.thread_cnt;
		ua->quality = mc.quality;
		ua->ptime = mc.ptime;
		ua->noVad = (mc.no_vad);
		ua->ilbcMode = mc.ilbc_mode;
		ua->txDropPct = mc.tx_drop_pct;
		ua->rxDropPct = mc.rx_drop_pct;
		ua->ecOptions = mc.ec_options;
		ua->ecTailLen = mc.ec_tail_len;
		ua->sndRecLatency = mc.snd_rec_latency;
		ua->sndPlayLatency = mc.snd_play_latency;
		ua->jbInit = mc.jb_init;
		ua->jbMinPre = mc.jb_min_pre;
		ua->jbMaxPre = mc.jb_max_pre;
		ua->jbMax = mc.jb_max;
		ua->sndAutoCloseTime = mc.snd_auto_close_time;
		ua->vidPreviewEnableNative = (mc.vid_preview_enable_native);
		return OK;
	}
	return ERROR;
}

static pjsip_media_config_t *pjsip_media_config_new()
{
	pjsip_media_config_t *ua = malloc(sizeof(pjsip_media_config_t));
	if (ua)
	{
		memset(ua, 0, sizeof(pjsip_media_config_t));
		return ua;
	}
	return NULL;
}

static int pjsip_media_config_destroy(pjsip_media_config_t *ua)
{
	if (ua)
	{
		free(ua);
		ua = NULL;
	}
	return OK;
}

//tls
static int pjsip_tls_config_default(pjsip_tls_config_t *ua)
{
	if (ua)
	{
		pjsip_tls_setting ts;
		pjsip_tls_setting_default(&ts);
		ua->CaListFile = PJ_STR_STRDUP(ts.ca_list_file);
		ua->certFile = PJ_STR_STRDUP(ts.cert_file);
		ua->privKeyFile = PJ_STR_STRDUP(ts.privkey_file);
		ua->password = PJ_STR_STRDUP(ts.password);
		ua->CaBuf = PJ_STR_STRDUP(ts.ca_buf);
		ua->certBuf = PJ_STR_STRDUP(ts.cert_buf);
		ua->privKeyBuf = PJ_STR_STRDUP(ts.privkey_buf);
		ua->method = (pjsip_ssl_method)ts.method;
		ua->proto = ts.proto;
		// The following will only work if sizeof(enum)==sizeof(int)
		//pj_assert(sizeof(ts.ciphers[0]) == sizeof(int));
		//ua->ciphers 	= IntVector(ts.ciphers, ts.ciphers+ts.ciphers_num);
		if (ts.ciphers_num > 0)
		{
			ua->ciphers = malloc(sizeof(pj_ssl_cipher) * ts.ciphers_num);
			if (ua->ciphers)
			{
				memcpy(ua->ciphers, ts.ciphers, sizeof(pj_ssl_cipher) * ts.ciphers_num);
				ua->ciphersCnt = ts.ciphers_num;
			}
		}
		ua->verifyServer = (ts.verify_server);
		ua->verifyClient = (ts.verify_client);
		ua->requireClientCert = (ts.require_client_cert);
		ua->msecTimeout = PJ_TIME_VAL_MSEC(ts.timeout);
		ua->qosType = ts.qos_type;
		ua->qosParams = ts.qos_params;
		ua->qosIgnoreError = (ts.qos_ignore_error);
		return OK;
	}
	return ERROR;
}

static pjsip_tls_config_t *pjsip_tls_config_new()
{
	pjsip_tls_config_t *ua = malloc(sizeof(pjsip_tls_config_t));
	if (ua)
	{
		memset(ua, 0, sizeof(pjsip_tls_config_t));
		return ua;
	}
	return NULL;
}

static int pjsip_tls_config_destroy(pjsip_tls_config_t *ua)
{
	if (ua)
	{
		if (ua->CaListFile)
			free(ua->CaListFile);
		if (ua->certFile)
			free(ua->certFile);
		if (ua->privKeyFile)
			free(ua->privKeyFile);
		if (ua->password)
			free(ua->password);

		if (ua->CaBuf)
			free(ua->CaBuf);
		if (ua->certBuf)
			free(ua->certBuf);
		if (ua->privKeyBuf)
			free(ua->privKeyBuf);

		if (ua->ciphers)
			free(ua->ciphers);

		free(ua);
		ua = NULL;
	}
	return OK;
}

//transport
static int pjsip_transport_config_default(pjsip_transport_config_t *ua)
{
	if (ua)
	{
		pjsua_transport_config tc;
		pjsua_transport_config_default(&tc);
		ua->port = tc.port;
		ua->portRange = tc.port_range;
		ua->publicAddress = PJ_STR_STRDUP(tc.public_addr);
		ua->boundAddress = PJ_STR_STRDUP(tc.bound_addr);
		//ua->tlsConfig.fromPj(tc.tls_setting);
		ua->tlsConfig = pjsip_tls_config_new();
		if (ua->tlsConfig)
		{
			pjsip_tls_config_default(ua->tlsConfig);
		}
		ua->qosType = tc.qos_type;
		ua->qosParams = tc.qos_params;
		return OK;
	}
	return ERROR;
}

static pjsip_transport_config_t *pjsip_transport_config_new()
{
	pjsip_transport_config_t *ua = malloc(sizeof(pjsip_transport_config_t));
	if (ua)
	{
		memset(ua, 0, sizeof(pjsip_transport_config_t));
		return ua;
	}
	return NULL;
}

static int pjsip_transport_config_destroy(pjsip_transport_config_t *ua)
{
	if (ua)
	{
		if (ua->publicAddress)
			free(ua->publicAddress);

		if (ua->boundAddress)
			free(ua->boundAddress);
		if (ua->tlsConfig)
		{
			pjsip_tls_config_destroy(ua->tlsConfig);
			ua->tlsConfig = NULL;
		}
		free(ua);
		ua = NULL;
	}
	return OK;
}

//srtp crypto
static int pjsip_srtp_crypto_default(pjsip_srtp_crypto_t *ua, pjmedia_srtp_crypto *opt)
{
	if (ua && opt)
	{
		ua->key = PJ_STR_STRDUP(opt->key);
		ua->name = PJ_STR_STRDUP(opt->name);
		ua->flags = opt->flags;
		return OK;
	}
	return ERROR;
}

static int pjsip_srtp_crypto_destroy(pjsip_srtp_crypto_t *ua)
{
	if (ua)
	{
		if (ua->key)
			free(ua->key);

		if (ua->name)
			free(ua->name);
		ua = NULL;
	}
	return OK;
}

//srtp option
static int pjsip_srtp_option_default(pjsip_srtp_option_t *ua)
{
	if (ua)
	{
		pjsua_srtp_opt opt;
		pjsua_srtp_opt_default(&opt);
		if (opt.crypto_count > 0)
		{
			ua->cryptos = malloc(opt.crypto_count * sizeof(struct pjsip_srtp_crypto));
			if (ua->cryptos)
			{
				ua->cryptos_num = opt.crypto_count;
				for (unsigned i = 0; i < opt.crypto_count; ++i)
				{
					pjsip_srtp_crypto_default(&ua->cryptos[i], NULL);
				}
			}
		}
		if (opt.keying_count > 0)
		{
			ua->keyings = malloc(sizeof(int) * opt.keying_count);
			if (ua->keyings)
			{
				ua->keying_num = opt.keying_count;
				for (unsigned i = 0; i < opt.keying_count; ++i)
				{
					ua->keyings[i] = (opt.keying[i]);
				}
			}
		}
		return OK;
	}
	return ERROR;
}

static pjsip_srtp_option_t *pjsip_srtp_option_new()
{
	pjsip_srtp_option_t *ua = malloc(sizeof(pjsip_srtp_option_t));
	if (ua)
	{
		memset(ua, 0, sizeof(pjsip_srtp_option_t));
		return ua;
	}
	return NULL;
}

static int pjsip_srtp_option_destroy(pjsip_srtp_option_t *ua)
{
	if (ua)
	{
		if (ua->cryptos)
		{
			int i = 0;
			for (i = 0; i < ua->cryptos_num; ++i)
			{
				pjsip_srtp_crypto_destroy(&ua->cryptos[i]);
			}
			free(ua->cryptos);
		}

		if (ua->keyings)
			free(ua->keyings);
		free(ua);
		ua = NULL;
	}
	return OK;
}

//pjsip_rtcp_fb_config
static int pjsip_rtcp_fb_config_default(pjsip_rtcp_fb_config_t *ua)
{
	if (ua)
	{
		pjmedia_rtcp_fb_setting setting;
		pjmedia_rtcp_fb_setting_default(&setting);

		ua->dontUseAvpf = (setting.dont_use_avpf);
		/*	    ua->caps.clear();
	    for (unsigned i = 0; i < setting.cap_count; ++i) {
		RtcpFbCap cap;
		cap.fromPj(prm.caps[i]);
		this->caps.push_back(cap);
	    }*/
		return OK;
	}
	return ERROR;
}

static pjsip_rtcp_fb_config_t *pjsip_rtcp_fb_config_new()
{
	pjsip_rtcp_fb_config_t *ua = malloc(sizeof(pjsip_rtcp_fb_config_t));
	if (ua)
	{
		memset(ua, 0, sizeof(pjsip_rtcp_fb_config_t));
		return ua;
	}
	return NULL;
}

static int pjsip_rtcp_fb_config_destroy(pjsip_rtcp_fb_config_t *ua)
{
	if (ua)
	{
		free(ua);
		ua = NULL;
	}
	return OK;
}

//account
static int pjsip_account_reg_config_destroy(pjsip_account_reg_config_t *ua)
{
	if (ua)
	{
		if (ua->registrarUri)
			free(ua->registrarUri);

		if (ua->contactParams)
			free(ua->contactParams);
		free(ua);
		ua = NULL;
	}
	return OK;
}

static int pjsip_account_sip_config_destroy(pjsip_account_sip_config_t *ua)
{
	if (ua)
	{
		if (ua->authCreds)
			free(ua->authCreds);
		if (ua->proxies)
			free(ua->proxies);

		if (ua->contactForced)
			free(ua->contactForced);
		if (ua->contactParams)
			free(ua->contactParams);
		if (ua->contactUriParams)
			free(ua->contactUriParams);
		if (ua->authInitialAlgorithm)
			free(ua->authInitialAlgorithm);

		free(ua);
		ua = NULL;
	}
	return OK;
}

static int pjsip_account_nat_config_destroy(pjsip_account_nat_config_t *ua)
{
	if (ua)
	{
		if (ua->turnServer)
			free(ua->turnServer);
		if (ua->turnUserName)
			free(ua->turnUserName);

		if (ua->turnPassword)
			free(ua->turnPassword);
		if (ua->sipOutboundInstanceId)
			free(ua->sipOutboundInstanceId);
		if (ua->sipOutboundRegId)
			free(ua->sipOutboundRegId);
		if (ua->udpKaData)
			free(ua->udpKaData);

		free(ua);
		ua = NULL;
	}
	return OK;
}

static int pjsip_account_config_default(pjsip_account_config_t *ua)
{
	if (ua && ua->regConfig && ua->sipConfig && ua->callConfig &&
		ua->presConfig && ua->mwiConfig && ua->natConfig &&
		ua->mediaConfig && ua->videoConfig && ua->ipChangeConfig)
	{
		int i = 0;
		pjsua_acc_config acc_cfg;
		pjsua_media_config med_cfg;
		pjsua_acc_config_default(&acc_cfg);
		pjsua_media_config_default(&med_cfg);

		//const pjsip_hdr *hdr;
		//unsigned i;

		// Global
		ua->priority = acc_cfg.priority;
		ua->idUri = PJ_STR_STRDUP(acc_cfg.id);

		// AccountRegConfig
		ua->regConfig->registrarUri = PJ_STR_STRDUP(acc_cfg.reg_uri);
		ua->regConfig->registerOnAdd = (acc_cfg.register_on_acc_add != 0);
		ua->regConfig->timeoutSec = acc_cfg.reg_timeout;
		ua->regConfig->retryIntervalSec = acc_cfg.reg_retry_interval;
		ua->regConfig->firstRetryIntervalSec = acc_cfg.reg_first_retry_interval;
		ua->regConfig->randomRetryIntervalSec = acc_cfg.reg_retry_random_interval;
		ua->regConfig->delayBeforeRefreshSec = acc_cfg.reg_delay_before_refresh;
		ua->regConfig->dropCallsOnFail = (acc_cfg.drop_calls_on_reg_fail);
		ua->regConfig->unregWaitMsec = acc_cfg.unreg_timeout;
		ua->regConfig->proxyUse = acc_cfg.reg_use_proxy;
		ua->regConfig->contactParams = PJ_STR_STRDUP(acc_cfg.reg_contact_params);
		//ua->regConfig->headers.clear();
		//hdr = acc_cfg.reg_hdr_list.next;
		/*	    while (hdr != &acc_cfg.reg_hdr_list) {
		SipHeader new_hdr;
		new_hdr.fromPj(hdr);

		ua->regConfig->headers.push_back(new_hdr);

		hdr = hdr->next;
	    }*/

		// AccountSipConfig
		if (acc_cfg.cred_count)
		{
			ua->sipConfig->authCreds = malloc(acc_cfg.cred_count * sizeof(pjsip_auth_cred_info_t));
			memset(ua->sipConfig->authCreds, 0, acc_cfg.cred_count * sizeof(pjsip_auth_cred_info_t));
		}
		else
			ua->sipConfig->authCreds = NULL;
		if (ua->sipConfig->authCreds)
		{
			for (i = 0; i < acc_cfg.cred_count; ++i)
			{
				const pjsip_cred_info *src = &acc_cfg.cred_info[i];

				ua->sipConfig->authCreds[i].realm = PJ_STR_STRDUP(src->realm);
				ua->sipConfig->authCreds[i].scheme = PJ_STR_STRDUP(src->scheme);
				ua->sipConfig->authCreds[i].username = PJ_STR_STRDUP(src->username);
				ua->sipConfig->authCreds[i].dataType = src->data_type;
				ua->sipConfig->authCreds[i].data = PJ_STR_STRDUP(src->data);
				ua->sipConfig->authCreds[i].akaK = PJ_STR_STRDUP(src->ext.aka.k);
				ua->sipConfig->authCreds[i].akaOp = PJ_STR_STRDUP(src->ext.aka.op);
				ua->sipConfig->authCreds[i].akaAmf = PJ_STR_STRDUP(src->ext.aka.amf);
			}
		}
		/*
		if (acc_cfg.proxy_cnt)
		{
			ua->sipConfig->proxies = malloc(acc_cfg.proxy_cnt * sizeof(char));
			memset(ua->sipConfig->proxies, 0, acc_cfg.proxy_cnt * sizeof(char));
		}
		else
			ua->sipConfig->proxies = NULL;
		*/	
		if (ua->sipConfig->proxies)
		{
			for (i = 0; i < acc_cfg.proxy_cnt; ++i)
			{
				ua->sipConfig->proxies[i] = (PJ_STR_STRDUP(acc_cfg.proxy[i]));
			}
		}
		ua->sipConfig->contactForced = PJ_STR_STRDUP(acc_cfg.force_contact);
		ua->sipConfig->contactParams = PJ_STR_STRDUP(acc_cfg.contact_params);
		ua->sipConfig->contactUriParams = PJ_STR_STRDUP(acc_cfg.contact_uri_params);
		ua->sipConfig->authInitialEmpty = (acc_cfg.auth_pref.initial_auth);
		ua->sipConfig->authInitialAlgorithm = PJ_STR_STRDUP(acc_cfg.auth_pref.algorithm);
		ua->sipConfig->transportId = acc_cfg.transport_id;

		// AccountCallConfig
		ua->callConfig->holdType = acc_cfg.call_hold_type;
		ua->callConfig->prackUse = acc_cfg.require_100rel;
		ua->callConfig->timerUse = acc_cfg.use_timer;
		ua->callConfig->timerMinSESec = acc_cfg.timer_setting.min_se;
		ua->callConfig->timerSessExpiresSec = acc_cfg.timer_setting.sess_expires;

		// AccountPresConfig
		/*	    ua->presConfig->headers.clear();
	    hdr = acc_cfg.sub_hdr_list.next;
	    while (hdr != &acc_cfg.sub_hdr_list) {
		SipHeader new_hdr;
		new_hdr.fromPj(hdr);
		ua->presConfig->headers.push_back(new_hdr);
		hdr = hdr->next;
	    }*/
		ua->presConfig->publishEnabled = (acc_cfg.publish_enabled);
		ua->presConfig->publishQueue = (acc_cfg.publish_opt.queue_request);
		ua->presConfig->publishShutdownWaitMsec = acc_cfg.unpublish_max_wait_time_msec;
		ua->presConfig->pidfTupleId = PJ_STR_STRDUP(acc_cfg.pidf_tuple_id);

		// AccountMwiConfig
		ua->mwiConfig->enabled = (acc_cfg.mwi_enabled);
		ua->mwiConfig->expirationSec = acc_cfg.mwi_expires;

		// AccountNatConfig
		ua->natConfig->sipStunUse = acc_cfg.sip_stun_use;
		ua->natConfig->mediaStunUse = acc_cfg.media_stun_use;
		ua->natConfig->nat64Opt = acc_cfg.nat64_opt;
		if (acc_cfg.ice_cfg_use == PJSUA_ICE_CONFIG_USE_CUSTOM)
		{
			ua->natConfig->iceEnabled = (acc_cfg.ice_cfg.enable_ice);
			ua->natConfig->iceMaxHostCands = acc_cfg.ice_cfg.ice_max_host_cands;
			ua->natConfig->iceAggressiveNomination = (acc_cfg.ice_cfg.ice_opt.aggressive);
			ua->natConfig->iceNominatedCheckDelayMsec = acc_cfg.ice_cfg.ice_opt.nominated_check_delay;
			ua->natConfig->iceWaitNominationTimeoutMsec = acc_cfg.ice_cfg.ice_opt.controlled_agent_want_nom_timeout;
			ua->natConfig->iceNoRtcp = (acc_cfg.ice_cfg.ice_no_rtcp);
			ua->natConfig->iceAlwaysUpdate = (acc_cfg.ice_cfg.ice_always_update);
		}
		else
		{
			ua->natConfig->iceEnabled = (med_cfg.enable_ice);
			ua->natConfig->iceMaxHostCands = med_cfg.ice_max_host_cands;
			ua->natConfig->iceAggressiveNomination = (med_cfg.ice_opt.aggressive);
			ua->natConfig->iceNominatedCheckDelayMsec = med_cfg.ice_opt.nominated_check_delay;
			ua->natConfig->iceWaitNominationTimeoutMsec = med_cfg.ice_opt.controlled_agent_want_nom_timeout;
			ua->natConfig->iceNoRtcp = (med_cfg.ice_no_rtcp);
			ua->natConfig->iceAlwaysUpdate = (med_cfg.ice_always_update);
		}

		if (acc_cfg.turn_cfg_use == PJSUA_TURN_CONFIG_USE_CUSTOM)
		{
			ua->natConfig->turnEnabled = (acc_cfg.turn_cfg.enable_turn);
			ua->natConfig->turnServer = PJ_STR_STRDUP(acc_cfg.turn_cfg.turn_server);
			ua->natConfig->turnConnType = acc_cfg.turn_cfg.turn_conn_type;
			ua->natConfig->turnUserName = PJ_STR_STRDUP(acc_cfg.turn_cfg.turn_auth_cred.data.static_cred.username);
			ua->natConfig->turnPasswordType = acc_cfg.turn_cfg.turn_auth_cred.data.static_cred.data_type;
			ua->natConfig->turnPassword = PJ_STR_STRDUP(acc_cfg.turn_cfg.turn_auth_cred.data.static_cred.data);
		}
		else
		{
			ua->natConfig->turnEnabled = (med_cfg.enable_turn);
			ua->natConfig->turnServer = PJ_STR_STRDUP(med_cfg.turn_server);
			ua->natConfig->turnConnType = med_cfg.turn_conn_type;
			ua->natConfig->turnUserName = PJ_STR_STRDUP(med_cfg.turn_auth_cred.data.static_cred.username);
			ua->natConfig->turnPasswordType = med_cfg.turn_auth_cred.data.static_cred.data_type;
			ua->natConfig->turnPassword = PJ_STR_STRDUP(med_cfg.turn_auth_cred.data.static_cred.data);
		}
		ua->natConfig->contactRewriteUse = acc_cfg.allow_contact_rewrite;
		ua->natConfig->contactRewriteMethod = acc_cfg.contact_rewrite_method;
		ua->natConfig->contactUseSrcPort = acc_cfg.contact_use_src_port;
		ua->natConfig->viaRewriteUse = acc_cfg.allow_via_rewrite;
		ua->natConfig->sdpNatRewriteUse = acc_cfg.allow_sdp_nat_rewrite;
		ua->natConfig->sipOutboundUse = acc_cfg.use_rfc5626;
		ua->natConfig->sipOutboundInstanceId = PJ_STR_STRDUP(acc_cfg.rfc5626_instance_id);
		ua->natConfig->sipOutboundRegId = PJ_STR_STRDUP(acc_cfg.rfc5626_reg_id);
		ua->natConfig->udpKaIntervalSec = acc_cfg.ka_interval;
		ua->natConfig->udpKaData = PJ_STR_STRDUP(acc_cfg.ka_data);

		// AccountMediaConfig
		if (ua->mediaConfig->transportConfig == NULL)
			ua->mediaConfig->transportConfig = pjsip_transport_config_new();
		if (ua->mediaConfig->transportConfig)
			pjsip_transport_config_default(ua->mediaConfig->transportConfig);
		ua->mediaConfig->lockCodecEnabled = (acc_cfg.lock_codec);
#if defined(PJMEDIA_STREAM_ENABLE_KA) && (PJMEDIA_STREAM_ENABLE_KA != 0)
		ua->mediaConfig->streamKaEnabled = (acc_cfg.use_stream_ka);
#else
		ua->mediaConfig->streamKaEnabled = false;
#endif
		ua->mediaConfig->srtpUse = acc_cfg.use_srtp;
		ua->mediaConfig->srtpSecureSignaling = acc_cfg.srtp_secure_signaling;
		//ua->mediaConfig->srtpOpt.fromPj(acc_cfg.srtp_opt);
		if (ua->mediaConfig->srtpOpt == NULL)
			ua->mediaConfig->srtpOpt = pjsip_srtp_option_new();
		if (ua->mediaConfig->srtpOpt)
			pjsip_srtp_option_default(ua->mediaConfig->srtpOpt);

		ua->mediaConfig->ipv6Use = acc_cfg.ipv6_media_use;
		ua->mediaConfig->rtcpMuxEnabled = (acc_cfg.enable_rtcp_mux);

		//ua->mediaConfig->rtcpFbConfig.fromPj(acc_cfg.rtcp_fb_cfg);
		if (ua->mediaConfig->rtcpFbConfig == NULL)
			ua->mediaConfig->rtcpFbConfig = pjsip_rtcp_fb_config_new();
		if (ua->mediaConfig->rtcpFbConfig)
			pjsip_rtcp_fb_config_default(ua->mediaConfig->rtcpFbConfig);

		// AccountVideoConfig
		ua->videoConfig->autoShowIncoming = (acc_cfg.vid_in_auto_show);
		ua->videoConfig->autoTransmitOutgoing = (acc_cfg.vid_out_auto_transmit);
		ua->videoConfig->windowFlags = acc_cfg.vid_wnd_flags;
		ua->videoConfig->defaultCaptureDevice = acc_cfg.vid_cap_dev;
		ua->videoConfig->defaultRenderDevice = acc_cfg.vid_rend_dev;
		ua->videoConfig->rateControlMethod = acc_cfg.vid_stream_rc_cfg.method;
		ua->videoConfig->rateControlBandwidth = acc_cfg.vid_stream_rc_cfg.bandwidth;
		ua->videoConfig->startKeyframeCount = acc_cfg.vid_stream_sk_cfg.count;
		ua->videoConfig->startKeyframeInterval = acc_cfg.vid_stream_sk_cfg.interval;

		// AccountIpChangeConfig
		ua->ipChangeConfig->shutdownTp = (acc_cfg.ip_change_cfg.shutdown_tp);
		ua->ipChangeConfig->hangupCalls = (acc_cfg.ip_change_cfg.hangup_calls);
		ua->ipChangeConfig->reinviteFlags = acc_cfg.ip_change_cfg.reinvite_flags;

		return OK;
	}
	return ERROR;
}

/*pjsip_account_config_t * pjsip_account_config_new()
{
	pjsip_account_config_t *ua = malloc(sizeof(pjsip_account_config_t));
	if(ua)
	{
		memset(ua, 0, sizeof(pjsip_account_config_t));
		return ua;
	}
	return NULL;
}*/

int pjsip_account_config_init(pjsip_account_config_t *ua)
{
	if (ua)
	{
		if (ua->regConfig == NULL)
		{
			ua->regConfig = malloc(sizeof(pjsip_account_reg_config_t));
			if (ua->regConfig == NULL)
			{
				return ERROR;
			}
		}
		if (ua->sipConfig == NULL)
		{
			ua->sipConfig = malloc(sizeof(pjsip_account_sip_config_t));
			if (ua->sipConfig == NULL)
			{
				free(ua->regConfig);
				ua->regConfig = NULL;
				return ERROR;
			}
		}
		if (ua->callConfig == NULL)
		{
			ua->callConfig = malloc(sizeof(pjsip_account_call_config_t));
			if (ua->callConfig == NULL)
			{
				free(ua->regConfig);
				ua->regConfig = NULL;
				free(ua->sipConfig);
				ua->sipConfig = NULL;
				return ERROR;
			}
		}
		if (ua->presConfig == NULL)
		{
			ua->presConfig = malloc(sizeof(pjsip_account_pres_config_t));
			if (ua->presConfig == NULL)
			{
				free(ua->regConfig);
				ua->regConfig = NULL;
				free(ua->sipConfig);
				ua->sipConfig = NULL;
				free(ua->callConfig);
				ua->callConfig = NULL;
				return ERROR;
			}
		}
		if (ua->mwiConfig == NULL)
		{
			ua->mwiConfig = malloc(sizeof(pjsip_account_mwi_config_t));
			if (ua->mwiConfig == NULL)
			{
				free(ua->regConfig);
				ua->regConfig = NULL;
				free(ua->sipConfig);
				ua->sipConfig = NULL;
				free(ua->callConfig);
				ua->callConfig = NULL;
				free(ua->presConfig);
				ua->presConfig = NULL;
				return ERROR;
			}
		}

		if (ua->natConfig == NULL)
		{
			ua->natConfig = malloc(sizeof(pjsip_account_nat_config_t));
			if (ua->natConfig == NULL)
			{
				free(ua->regConfig);
				ua->regConfig = NULL;
				free(ua->sipConfig);
				ua->sipConfig = NULL;
				free(ua->callConfig);
				ua->callConfig = NULL;
				free(ua->presConfig);
				ua->presConfig = NULL;
				free(ua->mwiConfig);
				ua->mwiConfig = NULL;
				return ERROR;
			}
		}

		if (ua->mediaConfig == NULL)
		{
			ua->mediaConfig = malloc(sizeof(pjsip_account_media_config_t));
			if (ua->mediaConfig == NULL)
			{
				free(ua->regConfig);
				ua->regConfig = NULL;
				free(ua->sipConfig);
				ua->sipConfig = NULL;
				free(ua->callConfig);
				ua->callConfig = NULL;
				free(ua->presConfig);
				ua->presConfig = NULL;
				free(ua->mwiConfig);
				ua->mwiConfig = NULL;
				free(ua->natConfig);
				ua->natConfig = NULL;
				return ERROR;
			}
		}
		if (ua->videoConfig == NULL)
		{
			ua->videoConfig = malloc(sizeof(pjsip_account_video_config_t));
			if (ua->videoConfig == NULL)
			{
				free(ua->regConfig);
				ua->regConfig = NULL;
				free(ua->sipConfig);
				ua->sipConfig = NULL;
				free(ua->callConfig);
				ua->callConfig = NULL;
				free(ua->presConfig);
				ua->presConfig = NULL;
				free(ua->mwiConfig);
				ua->mwiConfig = NULL;
				free(ua->natConfig);
				ua->natConfig = NULL;
				free(ua->mediaConfig);
				ua->mediaConfig = NULL;
				return ERROR;
			}
		}
		if (ua->ipChangeConfig == NULL)
		{
			ua->ipChangeConfig = malloc(sizeof(pjsip_account_IpChange_config_t));
			if (ua->ipChangeConfig == NULL)
			{
				free(ua->regConfig);
				ua->regConfig = NULL;
				free(ua->sipConfig);
				ua->sipConfig = NULL;
				free(ua->callConfig);
				ua->callConfig = NULL;
				free(ua->presConfig);
				ua->presConfig = NULL;
				free(ua->mwiConfig);
				ua->mwiConfig = NULL;
				free(ua->natConfig);
				ua->natConfig = NULL;
				free(ua->videoConfig);
				ua->videoConfig = NULL;
				free(ua->mediaConfig);
				ua->mediaConfig = NULL;
				return ERROR;
			}
		}
		pjsip_account_config_default(ua);
		return OK;
	}
	return ERROR;
}

int pjsip_account_config_destroy(pjsip_account_config_t *ua)
{
	if (ua)
	{
		if (ua->regConfig != NULL)
		{
			pjsip_account_reg_config_destroy(ua->regConfig);
			free(ua->regConfig);
			ua->regConfig = NULL;
		}
		if (ua->sipConfig != NULL)
		{
			pjsip_account_sip_config_destroy(ua->sipConfig);
			free(ua->sipConfig);
			ua->sipConfig = NULL;
		}
		if (ua->callConfig != NULL)
		{
			free(ua->callConfig);
			ua->callConfig = NULL;
		}
		if (ua->presConfig != NULL)
		{
			if (ua->presConfig->pidfTupleId)
				free(ua->presConfig->pidfTupleId);
			free(ua->presConfig);
			ua->presConfig = NULL;
		}
		if (ua->mwiConfig != NULL)
		{
			free(ua->mwiConfig);
			ua->mwiConfig = NULL;
		}
		if (ua->natConfig != NULL)
		{
			pjsip_account_nat_config_destroy(ua->natConfig);
			free(ua->natConfig);
			ua->natConfig = NULL;
		}
		if (ua->videoConfig != NULL)
		{
			free(ua->videoConfig);
			ua->videoConfig = NULL;
		}
		if (ua->mediaConfig != NULL)
		{
			if (ua->mediaConfig->transportConfig)
				pjsip_transport_config_destroy(ua->mediaConfig->transportConfig);

			if (ua->mediaConfig->srtpOpt)
				pjsip_srtp_option_destroy(ua->mediaConfig->srtpOpt);

			if (ua->mediaConfig->rtcpFbConfig)
				pjsip_rtcp_fb_config_destroy(ua->mediaConfig->rtcpFbConfig);

			free(ua->mediaConfig);
			ua->mediaConfig = NULL;
		}
		if (ua->idUri)
			free(ua->idUri);
		ua->idUri = NULL;
	}
	return OK;
}

int pjsip_endpoint_config_init(pjsip_ep_config_t *ua)
{
	if (ua)
	{
		if (ua->uaConfig == NULL)
		{
			ua->uaConfig = pjsip_ua_config_new();
			if (ua->uaConfig)
			{
				pjsip_ua_config_default(ua->uaConfig);
			}
			else
			{
				return ERROR;
			}
		}
		if (ua->logConfig == NULL)
		{
			ua->logConfig = pjsip_log_config_new();
			if (ua->logConfig)
			{
				pjsip_log_config_default(ua->logConfig);
			}
			else
			{
				pjsip_ua_config_destroy(ua->uaConfig);
				return ERROR;
			}
		}
		if (ua->medConfig == NULL)
		{
			ua->medConfig = pjsip_media_config_new();
			if (ua->medConfig)
			{
				pjsip_media_config_default(ua->medConfig);
			}
			else
			{
				pjsip_log_config_destroy(ua->logConfig);
				pjsip_ua_config_destroy(ua->uaConfig);
				return ERROR;
			}
		}
		return OK;
	}
	return ERROR;
}

int pjsip_endpoint_config_destroy(pjsip_ep_config_t *ua)
{
	if (ua)
	{
		if (ua->medConfig != NULL)
		{
			pjsip_media_config_destroy(ua->medConfig);
		}
		if (ua->logConfig != NULL)
		{
			pjsip_log_config_destroy(ua->logConfig);
		}
		if (ua->uaConfig != NULL)
		{
			pjsip_ua_config_destroy(ua->uaConfig);
		}
		return OK;
	}
	return ERROR;
}
