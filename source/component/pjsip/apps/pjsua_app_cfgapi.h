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
#ifndef __PJAPP_CFG_H__
#define __PJAPP_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "pjsua_app_common.h"


/********************************* SUA ***************************************/
extern int  pjapp_cfg_nameserver_add(pjsua_app_config *cfg, char *lval);
extern int  pjapp_cfg_nameserver_del(pjsua_app_config *cfg, char *lval);
extern int  pjapp_cfg_loose_route(pjsua_app_config *cfg, pj_bool_t enable);
extern int  pjapp_cfg_outbound_proxy_add(pjsua_app_config *cfg, char *server, int port);
extern int  pjapp_cfg_outbound_proxy_del(pjsua_app_config *cfg, char *server, int port);
extern int  pjapp_cfg_stun_domain(pjsua_app_config *cfg, char *lval);
extern int  pjapp_cfg_stun_host(pjsua_app_config *cfg, char *lval);
extern int  pjapp_cfg_stunserver_add(pjsua_app_config *cfg, char *lval);
extern int  pjapp_cfg_stunserver_del(pjsua_app_config *cfg, char *lval);
extern int  pjapp_cfg_stun_ipv6(pjsua_app_config *cfg, pj_bool_t enable);
extern int  pjapp_cfg_100real(pjsua_app_config *cfg, int val);
extern int  pjapp_cfg_auth_username_add(pjsua_app_config *cfg, char *username, char *password, char *realm, char *scheme);
extern int  pjapp_cfg_auth_username_del(pjsua_app_config *cfg, char *username);
extern int  pjapp_cfg_user_agent(pjsua_app_config *cfg, char *lval);
extern int  pjapp_cfg_srtp_option(pjsua_app_config *cfg, int val);
extern int  pjapp_cfg_srtp_secure_signaling(pjsua_app_config *cfg, int val);
extern int  pjapp_cfg_srtp_keying(pjsua_app_config *cfg, int type1, int type2);
extern int  pjapp_cfg_srtp_crypto_add(pjsua_app_config *cfg, char *name, char *keystr);
extern int  pjapp_cfg_srtp_crypto_del(pjsua_app_config *cfg, char *name);
extern int  pjapp_cfg_upnp_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int  pjapp_cfg_user_upnp_interface(pjsua_app_config *cfg, char *lval);

/*account*/
extern pjsua_acc_config *pjapp_account_acc_lookup(char *user);
extern int pjapp_account_add(pjsua_app_config *cfg, char *sip_user);
extern int pjapp_account_del(pjsua_app_config *cfg, char *sip_user);
extern int pjapp_account_priority(pjsua_app_config *cfg, char *user, int priority);
extern int pjapp_account_register_server(pjsua_app_config *cfg, char *user, char * server, int port);
extern int pjapp_account_mwi(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_mwi_expires(pjsua_app_config *cfg, char *user, int mwi_expires);
extern int pjapp_account_publish(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_unpublish_wait(pjsua_app_config *cfg, char *user, int val);
extern int pjapp_account_auth_preference_algorithm(pjsua_app_config *cfg, char *user, char* algorithm);
extern int pjapp_account_100rel(pjsua_app_config *cfg, char *user, pj_bool_t enable);

extern int pjapp_account_proxy_add(pjsua_app_config *cfg, char *user, char * server, int port);
extern int pjapp_account_proxy_del(pjsua_app_config *cfg, char *user, char * server, int port);

extern int pjapp_account_register_timeout(pjsua_app_config *cfg, char *user, int lval);
extern int pjapp_account_register_refresh_delay(pjsua_app_config *cfg, char *user, int lval);
extern int pjapp_account_unregister_timeout(pjsua_app_config *cfg, char *user, int lval);
extern int pjapp_account_auth_username_add(pjsua_app_config *cfg, char *user, char *username, char *password, char *realm, char * scheme);
extern int pjapp_account_auth_username_del(pjsua_app_config *cfg, char *user, char *username);

extern int pjapp_account_rfc5626_instance_id(pjsua_app_config *cfg, char *user, char *lval);
extern int pjapp_account_rfc5626_register_id(pjsua_app_config *cfg, char *user, char *lval);

extern int pjapp_account_rfc5626_instance_id(pjsua_app_config *cfg, char *user, char *lval);
extern int pjapp_account_rfc5626_register_id(pjsua_app_config *cfg, char *user, char *lval);
extern int pjapp_account_video_auto_show(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_video_auto_transmit(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_video_record_bandwidth(pjsua_app_config *cfg, char *user, int bandwidth);
extern int pjapp_account_nat64(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_media_ipv6(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_stun(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_media_stun(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_upnp(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_media_upnp(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_loopback(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_media_loopback(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_ice_rtcp(pjsua_app_config *cfg, char *user, pj_bool_t enable, pj_bool_t rtcp_enable);
extern int pjapp_account_turn_enable(pjsua_app_config *cfg, char *user, pj_bool_t enable);
extern int pjapp_account_turn_server(pjsua_app_config *cfg, char *user, char *lval);
extern int pjapp_account_turn_proto(pjsua_app_config *cfg, char *user, int proto);
extern int pjapp_account_turn_auth(pjsua_app_config *cfg, char *user, char *username, char *password, char *realm);
extern int pjapp_account_turn_auth_tlsfilstlist(pjsua_app_config *cfg, char *user, char *filstlist);
extern int pjapp_account_turn_auth_cert_file(pjsua_app_config *cfg, char *user, char *filstlist);
extern int pjapp_account_turn_auth_privkey_file(pjsua_app_config *cfg, char *user, char *filstlist);
extern int pjapp_account_turn_auth_keystrlist(pjsua_app_config *cfg, char *user, char *filstlist);
extern int pjapp_account_turn_auth_cert_key(pjsua_app_config *cfg, char *user, char *filstlist);
extern int pjapp_account_turn_auth_privkey(pjsua_app_config *cfg, char *user, char *filstlist);
extern int pjapp_account_turn_auth_password(pjsua_app_config *cfg, char *user, char *filstlist);

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)

extern int pjapp_account_srtp_enable(pjsua_app_config *cfg, char *user,  int type);
extern int pjapp_account_srtp_secure(pjsua_app_config *cfg, char *user, int type);

extern int pjapp_account_srtp_keying(pjsua_app_config *cfg, char *user, int type1, int type2);
extern int pjapp_account_srtp_crypto_add(pjsua_app_config *cfg, char *user, char* name, char* keystr);
extern int pjapp_account_srtp_crypto_del(pjsua_app_config *cfg, char *user, char* name);
#endif

extern int pjapp_account_register_retry_interval(pjsua_app_config *cfg, char *user, int interval);
extern int pjapp_account_register_use_proxy(pjsua_app_config *cfg, char *user, int value);

/********************************* transport ***************************************/
extern int pjapp_transport_local_port(pjsua_app_config *cfg, pjapp_transport_proto_t type, char *address, int port, int port_range);
extern int pjapp_transport_public(pjsua_app_config *cfg, pjapp_transport_proto_t type, char *address);
extern int pjapp_transport_randomize_port_enable(pjsua_app_config *cfg, pjapp_transport_proto_t type, pj_bool_t enable);
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
extern int pjapp_transport_tls_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_transport_tls_ca_file(pjsua_app_config *cfg, char * lval);
extern int pjapp_transport_tls_cert_file(pjsua_app_config *cfg, char * lval);
extern int pjapp_transport_tls_priv_file(pjsua_app_config *cfg, char * lval);
extern int pjapp_transport_tls_password(pjsua_app_config *cfg, char * lval);
extern int pjapp_transport_tls_verify_server(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_transport_tls_verify_client(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_transport_tls_neg_timeout(pjsua_app_config *cfg, int value);
extern int pjapp_transport_tls_cipher(pjsua_app_config *cfg, char * lval);
#endif
/********************************* media ***************************************/
extern int pjapp_media_clock_rate(pjsua_app_config *cfg, int lval);
extern int pjapp_media_sound_clock_rate(pjsua_app_config *cfg, int lval);
extern int pjapp_media_audio_frame_ptime(pjsua_app_config *cfg, int value);
extern int pjapp_media_audio_ptime(pjsua_app_config *cfg, int value);
extern int pjapp_media_vad(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_media_quality(pjsua_app_config *cfg, int value);
extern int pjapp_media_ilbc_mode(pjsua_app_config *cfg, int value);
extern int pjapp_media_rx_drop_pct(pjsua_app_config *cfg, int value);
extern int pjapp_media_tx_drop_pct(pjsua_app_config *cfg, int value);
extern int pjapp_media_echo_options(pjsua_app_config *cfg, int value);
extern int pjapp_media_echo_tial(pjsua_app_config *cfg, int value);
extern int pjapp_media_record_latency(pjsua_app_config *cfg, int value);
extern int pjapp_media_play_latency(pjsua_app_config *cfg, int value);
extern int pjapp_media_ice_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_media_ice_rtcp(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_media_turn_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_media_turn_server(pjsua_app_config *cfg, char *lval);
extern int pjapp_media_turn_proto(pjsua_app_config *cfg, int proto);
extern int pjapp_media_turn_auth(pjsua_app_config *cfg, char *username, char *password, char *realm);
extern int pjapp_media_turn_auth_tlsfilstlist(pjsua_app_config *cfg, char *filstlist);
extern int pjapp_media_turn_auth_cert_file(pjsua_app_config *cfg, char *filstlist);
extern int pjapp_media_turn_auth_privkey_file(pjsua_app_config *cfg, char *filstlist);
extern int pjapp_media_turn_auth_keystrlist(pjsua_app_config *cfg, char *filstlist);
extern int pjapp_media_turn_auth_cert_key(pjsua_app_config *cfg, char *filstlist);
extern int pjapp_media_turn_auth_privkey(pjsua_app_config *cfg, char *filstlist);
extern int pjapp_media_turn_auth_password(pjsua_app_config *cfg, char *filstlist);
extern int pjapp_media_auto_close_time(pjsua_app_config *cfg, int val);
extern int pjapp_media_video_preview_native(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_media_smart_media_update(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_media_rtcp_sdes_bye(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_media_jb_max_size(pjsua_app_config *cfg, int value);
/*********************************** buddy ****************************************/
extern int pjapp_buddy_add(pjsua_app_config *cfg, char *uri,  pj_bool_t enable);
extern int pjapp_buddy_del(pjsua_app_config *cfg, char *uri);

/********************************* global ***************************************/
extern int pjapp_global_ipv6(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_refersub(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_qos_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_tcp_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_udp_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_tls_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_srtp_keying_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_codec_add(pjsua_app_config *cfg, char *codec);
extern int pjapp_global_codec_del(pjsua_app_config *cfg, char *codec);
extern int pjapp_global_audio_null(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_wavfile_add(pjsua_app_config *cfg, char *name);
extern int pjapp_global_wavfile_del(pjsua_app_config *cfg, char *name);
extern int pjapp_global_tones_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_tones_add(pjsua_app_config *cfg, int f1, int f2, int on, int off);
extern int pjapp_global_tones_del(pjsua_app_config *cfg, int f1, int f2, int on, int off);
extern int pjapp_global_auto_play(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_auto_play_hangup(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_auto_rec(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_auto_loop(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_auto_config(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_rec_file(pjsua_app_config *cfg, char *lval);
extern int pjapp_global_auto_answer(pjsua_app_config *cfg, int value);
extern int pjapp_global_duration(pjsua_app_config *cfg, int value);
extern int pjapp_global_micspeaker(pjsua_app_config *cfg, float mic, float speaker);
extern int pjapp_global_avifile_add(pjsua_app_config *cfg, char *name);
extern int pjapp_global_avifile_del(pjsua_app_config *cfg, char *name);
extern int pjapp_global_avi_default(pjsua_app_config *cfg, int val);
extern int pjapp_global_aviauto_play(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_video_auto_transmit(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_video_auto_show(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_capture_dev(pjsua_app_config *cfg, int value);
extern int pjapp_global_playback_dev(pjsua_app_config *cfg, int value);
extern int pjapp_global_capture_devname(pjsua_app_config *cfg, char* name);
extern int pjapp_global_playback_devname(pjsua_app_config *cfg, char* name);
extern int pjapp_global_capture_lat(pjsua_app_config *cfg, int value);
extern int pjapp_global_playback_lat(pjsua_app_config *cfg, int value);
extern int pjapp_global_vcapture_dev(pjsua_app_config *cfg, int value);
extern int pjapp_global_vrender_dev(pjsua_app_config *cfg, int value);
extern int pjapp_global_vcapture_devname(pjsua_app_config *cfg, char* name);
extern int pjapp_global_vrender_devname(pjsua_app_config *cfg, char* name);
extern int pjapp_global_cli_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_global_cli_telnet_port(pjsua_app_config *cfg, int port);
extern int pjapp_global_cli_console(pjsua_app_config *cfg, pj_bool_t enable);
/********************************* transport ***************************************/
/** Extern variable declaration **/
extern int pjapp_cfg_log_file(pjsua_app_config *cfg, char *logfile);
extern int pjapp_cfg_log_level(pjsua_app_config *cfg, int level);
extern int pjapp_cfg_app_log_level(pjsua_app_config *cfg, int level);
extern int pjapp_cfg_log_option(pjsua_app_config *cfg, int option, pj_bool_t enable);
extern int pjapp_cfg_log_color(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjapp_cfg_log_light_bg(pjsua_app_config *cfg, pj_bool_t enable);


extern pj_bool_t pjapp_global_isenable(void);
extern int pjapp_global_set_api(pj_bool_t enable);
extern int pjapp_config_default_setting(pjsua_app_config *appcfg);
extern int pjapp_cfg_log_config(pjsua_app_config *cfg);
extern int pjapp_config_init(void);

extern int pjapp_cmd_init(void);

#ifdef __cplusplus
}
#endif

#endif	/* __PJAPP_CFG_H__ */

