/* $Id: pjsua_app_config.h 5022 2015-03-25 03:41:21Z nanang $ */
/*
 * Copyright (C) 2008-2013 Teluu Inc. (http://www.teluu.com)
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __PJSUA_APP_CONFIG_H__
#define __PJSUA_APP_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <pjlib.h>
#include <zplos_include.h>
#include <log.h>
enum
{
	OPT_CONFIG_FILE = 127,
	OPT_LOG_FILE,
	OPT_LOG_LEVEL,
	OPT_APP_LOG_LEVEL,
	OPT_LOG_APPEND,
	OPT_COLOR,
	OPT_NO_COLOR,
	OPT_LIGHT_BG,
	OPT_NO_STDERR,
	OPT_HELP,
	OPT_VERSION,
	OPT_NULL_AUDIO,
	OPT_SND_AUTO_CLOSE,
	OPT_LOCAL_PORT,
	OPT_IP_ADDR,
	OPT_PROXY,
	OPT_OUTBOUND_PROXY,
	OPT_REGISTRAR,
	OPT_REG_TIMEOUT,
	OPT_PUBLISH,
	OPT_ID,
	OPT_CONTACT,
	OPT_BOUND_ADDR,
	OPT_CONTACT_PARAMS,
	OPT_CONTACT_URI_PARAMS,
	OPT_100REL,
	OPT_USE_IMS,
	OPT_REALM,
	OPT_USERNAME,
	OPT_PASSWORD,
	OPT_REG_RETRY_INTERVAL,
	OPT_REG_USE_PROXY,
	OPT_MWI,
	OPT_NAMESERVER,
	OPT_STUN_SRV,
	OPT_OUTB_RID,
	OPT_ADD_BUDDY,
	OPT_OFFER_X_MS_MSG,
	OPT_NO_PRESENCE,
	OPT_AUTO_ANSWER,
	OPT_AUTO_PLAY,
	OPT_AUTO_PLAY_HANGUP,
	OPT_AUTO_LOOP,
	OPT_AUTO_CONF,
	OPT_CLOCK_RATE,
	OPT_SND_CLOCK_RATE,
	OPT_STEREO,
	OPT_USE_ICE,
	OPT_ICE_REGULAR,
	OPT_USE_SRTP,
	OPT_SRTP_SECURE,
	OPT_USE_TURN,
	OPT_ICE_MAX_HOSTS,
	OPT_ICE_NO_RTCP,
	OPT_TURN_SRV,
	OPT_TURN_TCP,
	OPT_TURN_USER,
	OPT_TURN_PASSWD,
	OPT_RTCP_MUX,
	OPT_SRTP_KEYING,
	OPT_PLAY_FILE,
	OPT_PLAY_TONE,
	OPT_RTP_PORT,
	OPT_ADD_CODEC,
	OPT_ILBC_MODE,
	OPT_REC_FILE,
	OPT_AUTO_REC,
	OPT_COMPLEXITY,
	OPT_QUALITY,
	OPT_PTIME,
	OPT_NO_VAD,
	OPT_RX_DROP_PCT,
	OPT_TX_DROP_PCT,
	OPT_EC_TAIL,
	OPT_EC_OPT,
	OPT_NEXT_ACCOUNT,
	OPT_NEXT_CRED,
	OPT_MAX_CALLS,
	OPT_DURATION,
	OPT_NO_TCP,
	OPT_NO_UDP,
	OPT_THREAD_CNT,
	OPT_NOREFERSUB,
	OPT_ACCEPT_REDIRECT,
	OPT_USE_TLS,
	OPT_TLS_CA_FILE,
	OPT_TLS_CERT_FILE,
	OPT_TLS_PRIV_FILE,
	OPT_TLS_PASSWORD,
	OPT_TLS_VERIFY_SERVER,
	OPT_TLS_VERIFY_CLIENT,
	OPT_TLS_NEG_TIMEOUT,
	OPT_TLS_CIPHER,
	OPT_CAPTURE_DEV,
	OPT_PLAYBACK_DEV,
	OPT_CAPTURE_LAT,
	OPT_PLAYBACK_LAT,
	OPT_NO_TONES,
	OPT_JB_MAX_SIZE,
	OPT_STDOUT_REFRESH,
	OPT_STDOUT_REFRESH_TEXT,
	OPT_IPV6,
	OPT_QOS,
#ifdef _IONBF /**/
	OPT_STDOUT_NO_BUF,
#endif /**/
	OPT_AUTO_UPDATE_NAT,
	OPT_USE_COMPACT_FORM,
	OPT_DIS_CODEC,
	OPT_DISABLE_STUN,
	OPT_NO_FORCE_LR,
	OPT_TIMER,
	OPT_TIMER_SE,
	OPT_TIMER_MIN_SE,
	OPT_VIDEO,
	OPT_EXTRA_AUDIO,
	OPT_VCAPTURE_DEV,
	OPT_VRENDER_DEV,
	OPT_PLAY_AVI,
	OPT_AUTO_PLAY_AVI,
	OPT_USE_CLI,
	OPT_CLI_TELNET_PORT,
	OPT_DISABLE_CLI_CONSOLE
};


extern int pl_pjsip_log_file(pjsua_app_config *cfg, char *logfile);
extern int pl_pjsip_log_level(pjsua_app_config *cfg, int level);
extern int pl_pjsip_app_log_level(pjsua_app_config *cfg, int level);
extern int pl_pjsip_log_option(pjsua_app_config *cfg, int option, zpl_bool enable);
extern int pl_pjsip_log_color(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_log_light_bg(pjsua_app_config *cfg, zpl_bool enable);

extern int pl_pjsip_null_audio(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_clock_rate(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_snd_clock_rate(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_stereo(pjsua_app_config *cfg);
extern int pl_pjsip_local_port(pjsua_app_config *cfg, zpl_int32 lval);
extern int pl_pjsip_public_address(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_bound_address(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_no_udp(pjsua_app_config *cfg);
extern int pl_pjsip_no_tcp(pjsua_app_config *cfg);
extern int pl_pjsip_norefersub(pjsua_app_config *cfg);
extern int pl_pjsip_proxy(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_outbound_proxy(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_registrar(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_register_timeout(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_publish(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_mwi(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_100rel(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_session_timer(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_session_timer_expiration(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_session_timer_expiration_min(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_outbound_reg_id(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_ims(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_url_id(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_contact(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_contact_params(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_contact_uri_params(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_update_nat(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_stun(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_compact_form(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_accept_redirect(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_no_force_lr(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_next_account(pjsua_app_config *cfg);
extern int pl_pjsip_username(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_scheme(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_realm(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_password(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_reg_retry_interval(pjsua_app_config *cfg, int interval);
extern int pl_pjsip_reg_use_proxy(pjsua_app_config *cfg, int value);
extern int pl_pjsip_credential(pjsua_app_config *cfg, int credential);
extern int pl_pjsip_nameserver(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_stunserver(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_buddy_list(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_auto_play(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_auto_play_hangup(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_auto_rec(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_auto_loop(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_auto_config(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_play_file(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_play_tone(pjsua_app_config *cfg, int f1, int f2, int on, int off);
extern int pl_pjsip_rec_file(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_ice_enable(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_regular_enable(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_turn_enable(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_ice_max_hosts(pjsua_app_config *cfg, int maxnum);
extern int pl_pjsip_ice_nortcp_enable(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_turn_srv(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_turn_tcp(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_turn_user(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_turn_password(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_rtcp_mux(pjsua_app_config *cfg, zpl_bool enable);
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
extern int pl_pjsip_srtp_enable(pjsua_app_config *cfg, zpl_uint32 type);
extern int pl_pjsip_srtp_secure(pjsua_app_config *cfg, zpl_uint32 type);
extern int pl_pjsip_srtp_keying(pjsua_app_config *cfg, zpl_uint32 type);
#endif

extern int pl_pjsip_rtp_port(pjsua_app_config *cfg, int port);
extern int pl_pjsip_dis_codec(pjsua_app_config *cfg, char *lval);
extern int pl_pjsip_add_codec(pjsua_app_config *cfg, char *lval);
extern int pl_pjsip_duration(pjsua_app_config *cfg, int value);
extern int pl_pjsip_thread_cnt(pjsua_app_config *cfg, int value);
extern int pl_pjsip_ptime(pjsua_app_config *cfg, int value);
extern int pl_pjsip_novad(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_ec_tial(pjsua_app_config *cfg, int value);
extern int pl_pjsip_ec_options(pjsua_app_config *cfg, int value);
extern int pl_pjsip_quality(pjsua_app_config *cfg, int value);
extern int pl_pjsip_ilbc_mode(pjsua_app_config *cfg, int value);
extern int pl_pjsip_rx_drop_pct(pjsua_app_config *cfg, int value);
extern int pl_pjsip_tx_drop_pct(pjsua_app_config *cfg, int value);
extern int pl_pjsip_auto_answer(pjsua_app_config *cfg, int value);
extern int pl_pjsip_max_calls(pjsua_app_config *cfg, int value);
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
extern int pl_pjsip_tls_enable(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_tls_ca_file(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_tls_cert_file(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_tls_priv_file(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_tls_password(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_tls_verify_server(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_tls_verify_client(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_tls_neg_timeout(pjsua_app_config *cfg, int value);
extern int pl_pjsip_tls_cipher(pjsua_app_config *cfg, char * lval);
#endif

extern int pl_pjsip_capture_dev(pjsua_app_config *cfg, int value);

extern int pl_pjsip_capture_lat(pjsua_app_config *cfg, int value);
extern int pl_pjsip_playback_dev(pjsua_app_config *cfg, int value);
extern int pl_pjsip_playback_lat(pjsua_app_config *cfg, int value);
extern int pl_pjsip_snd_auto_close(pjsua_app_config *cfg, int value);
extern int pl_pjsip_no_tones(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_jb_max_size(pjsua_app_config *cfg, int value);

#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
extern int pl_pjsip_ipv6_enable(pjsua_app_config *cfg, zpl_bool enable);
#endif

extern int pl_pjsip_qos_enable(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_video_enable(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_extra_audio(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_vcapture_dev(pjsua_app_config *cfg, int value);
extern int pl_pjsip_vrender_dev(pjsua_app_config *cfg, int value);
extern int pl_pjsip_play_avi(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_auto_play_avi(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_cli_enable(pjsua_app_config *cfg, zpl_bool enable);
extern int pl_pjsip_cli_telnet_port(pjsua_app_config *cfg, int port);
extern int pl_pjsip_cli_console(pjsua_app_config *cfg, zpl_bool enable);
/* Set default config. */
extern void pjsip_default_config(void);
extern int load_config(void);
extern int pjsip_load_config(void);


//extern char   *stdout_refresh_text;

#ifdef __cplusplus
}
#endif

#endif	/* __PJSUA_APP_CONFIG_H__ */

