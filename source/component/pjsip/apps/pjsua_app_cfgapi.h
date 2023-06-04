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
#ifndef __PJSUA_APP_CFG_H__
#define __PJSUA_APP_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "pjsua_app_common.h"



/** Extern variable declaration **/
extern int pl_pjsip_log_file(pjsua_app_config *cfg, char *logfile);
extern int pl_pjsip_log_level(pjsua_app_config *cfg, int level);
extern int pl_pjsip_app_log_level(pjsua_app_config *cfg, int level);
extern int pl_pjsip_log_option(pjsua_app_config *cfg, int option, pj_bool_t enable);
extern int pl_pjsip_log_color(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_log_light_bg(pjsua_app_config *cfg, pj_bool_t enable);

extern int pl_pjsip_null_audio(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_clock_rate(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_snd_clock_rate(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_stereo(pjsua_app_config *cfg);
extern int pl_pjsip_local_port(pjsua_app_config *cfg, pj_int32_t lval);
extern int pl_pjsip_public_address(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_bound_address(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_no_udp(pjsua_app_config *cfg);
extern int pl_pjsip_no_tcp(pjsua_app_config *cfg);
extern int pl_pjsip_norefersub(pjsua_app_config *cfg);
extern int pl_pjsip_proxy(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_outbound_proxy(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_registrar(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_register_timeout(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_publish(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_mwi(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_100rel(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_session_timer(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_session_timer_expiration(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_session_timer_expiration_min(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_outbound_reg_id(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_ims(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_url_id(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_contact(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_contact_params(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_contact_uri_params(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_update_nat(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_stun(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_compact_form(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_accept_redirect(pjsua_app_config *cfg, int lval);
extern int pl_pjsip_no_force_lr(pjsua_app_config *cfg, pj_bool_t enable);
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
extern int pl_pjsip_auto_play(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_auto_play_hangup(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_auto_rec(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_auto_loop(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_auto_config(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_play_file(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_play_tone(pjsua_app_config *cfg, int f1, int f2, int on, int off);
extern int pl_pjsip_rec_file(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_ice_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_regular_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_turn_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_ice_max_hosts(pjsua_app_config *cfg, int maxnum);
extern int pl_pjsip_ice_nortcp_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_turn_srv(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_turn_tcp(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_turn_user(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_turn_password(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_rtcp_mux(pjsua_app_config *cfg, pj_bool_t enable);
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
extern int pl_pjsip_srtp_enable(pjsua_app_config *cfg, pj_uint32_t type);
extern int pl_pjsip_srtp_secure(pjsua_app_config *cfg, pj_uint32_t type);
extern int pl_pjsip_srtp_keying(pjsua_app_config *cfg, pj_uint32_t type);
#endif

extern int pl_pjsip_rtp_port(pjsua_app_config *cfg, int port);
extern int pl_pjsip_dis_codec(pjsua_app_config *cfg, char *lval);
extern int pl_pjsip_add_codec(pjsua_app_config *cfg, char *lval);
extern int pl_pjsip_duration(pjsua_app_config *cfg, int value);
extern int pl_pjsip_thread_cnt(pjsua_app_config *cfg, int value);
extern int pl_pjsip_ptime(pjsua_app_config *cfg, int value);
extern int pl_pjsip_novad(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_ec_tial(pjsua_app_config *cfg, int value);
extern int pl_pjsip_ec_options(pjsua_app_config *cfg, int value);
extern int pl_pjsip_quality(pjsua_app_config *cfg, int value);
extern int pl_pjsip_ilbc_mode(pjsua_app_config *cfg, int value);
extern int pl_pjsip_rx_drop_pct(pjsua_app_config *cfg, int value);
extern int pl_pjsip_tx_drop_pct(pjsua_app_config *cfg, int value);
extern int pl_pjsip_auto_answer(pjsua_app_config *cfg, int value);
extern int pl_pjsip_max_calls(pjsua_app_config *cfg, int value);
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
extern int pl_pjsip_tls_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_tls_ca_file(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_tls_cert_file(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_tls_priv_file(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_tls_password(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_tls_verify_server(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_tls_verify_client(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_tls_neg_timeout(pjsua_app_config *cfg, int value);
extern int pl_pjsip_tls_cipher(pjsua_app_config *cfg, char * lval);
#endif

extern int pl_pjsip_capture_dev(pjsua_app_config *cfg, int value);

extern int pl_pjsip_capture_lat(pjsua_app_config *cfg, int value);
extern int pl_pjsip_playback_dev(pjsua_app_config *cfg, int value);
extern int pl_pjsip_playback_lat(pjsua_app_config *cfg, int value);
extern int pl_pjsip_snd_auto_close(pjsua_app_config *cfg, int value);
extern int pl_pjsip_no_tones(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_jb_max_size(pjsua_app_config *cfg, int value);

#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
extern int pl_pjsip_ipv6_enable(pjsua_app_config *cfg, pj_bool_t enable);
#endif

extern int pl_pjsip_qos_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_video_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_extra_audio(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_vcapture_dev(pjsua_app_config *cfg, int value);
extern int pl_pjsip_vrender_dev(pjsua_app_config *cfg, int value);
extern int pl_pjsip_play_avi(pjsua_app_config *cfg, char * lval);
extern int pl_pjsip_auto_play_avi(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_cli_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pl_pjsip_cli_telnet_port(pjsua_app_config *cfg, int port);
extern int pl_pjsip_cli_console(pjsua_app_config *cfg, pj_bool_t enable);
/* Set default config. */
extern void pjsip_default_config(pjsua_app_config *cfg);
//extern int load_config(void);
extern int pjsip_load_config(pjsua_app_config *cfg);
extern int pjsip_log_config(pjsua_app_config *cfg);

#ifdef __cplusplus
}
#endif

#endif	/* __PJSUA_APP_CFG_H__ */

