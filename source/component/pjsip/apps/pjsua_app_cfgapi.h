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
extern int pjsua_app_cfg_log_file(pjsua_app_config *cfg, char *logfile);
extern int pjsua_app_cfg_log_level(pjsua_app_config *cfg, int level);
extern int pjsua_app_cfg_app_log_level(pjsua_app_config *cfg, int level);
extern int pjsua_app_cfg_log_option(pjsua_app_config *cfg, int option, pj_bool_t enable);
extern int pjsua_app_cfg_log_color(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_log_light_bg(pjsua_app_config *cfg, pj_bool_t enable);

extern int pjsua_app_cfg_null_audio(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_clock_rate(pjsua_app_config *cfg, int lval);
extern int pjsua_app_cfg_snd_clock_rate(pjsua_app_config *cfg, int lval);
extern int pjsua_app_cfg_stereo(pjsua_app_config *cfg);
extern int pjsua_app_cfg_local_port(pjsua_app_config *cfg, pj_int32_t lval);
extern int pjsua_app_cfg_public_address(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_bound_address(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_no_udp(pjsua_app_config *cfg);
extern int pjsua_app_cfg_no_tcp(pjsua_app_config *cfg);
extern int pjsua_app_cfg_norefersub(pjsua_app_config *cfg);
extern int pjsua_app_cfg_proxy(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_outbound_proxy(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_registrar(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_register_timeout(pjsua_app_config *cfg, int lval);
extern int pjsua_app_cfg_publish(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_mwi(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_100rel(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_session_timer(pjsua_app_config *cfg, int lval);
extern int pjsua_app_cfg_session_timer_expiration(pjsua_app_config *cfg, int lval);
extern int pjsua_app_cfg_session_timer_expiration_min(pjsua_app_config *cfg, int lval);
extern int pjsua_app_cfg_outbound_reg_id(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_ims(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_url_id(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_contact(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_contact_params(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_contact_uri_params(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_update_nat(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_stun(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_compact_form(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_accept_redirect(pjsua_app_config *cfg, int lval);
extern int pjsua_app_cfg_no_force_lr(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_next_account(pjsua_app_config *cfg);
extern int pjsua_app_cfg_username(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_scheme(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_realm(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_password(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_reg_retry_interval(pjsua_app_config *cfg, int interval);
extern int pjsua_app_cfg_reg_use_proxy(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_credential(pjsua_app_config *cfg, int credential);
extern int pjsua_app_cfg_nameserver(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_stunserver(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_buddy_list(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_auto_play(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_auto_play_hangup(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_auto_rec(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_auto_loop(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_auto_config(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_play_file(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_play_tone(pjsua_app_config *cfg, int f1, int f2, int on, int off);
extern int pjsua_app_cfg_rec_file(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_ice_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_regular_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_turn_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_ice_max_hosts(pjsua_app_config *cfg, int maxnum);
extern int pjsua_app_cfg_ice_nortcp_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_turn_srv(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_turn_tcp(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_turn_user(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_turn_password(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_rtcp_mux(pjsua_app_config *cfg, pj_bool_t enable);
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
extern int pjsua_app_cfg_srtp_enable(pjsua_app_config *cfg, pj_uint32_t type);
extern int pjsua_app_cfg_srtp_secure(pjsua_app_config *cfg, pj_uint32_t type);
extern int pjsua_app_cfg_srtp_keying(pjsua_app_config *cfg, pj_uint32_t type);
#endif

extern int pjsua_app_cfg_rtp_port(pjsua_app_config *cfg, int port);
extern int pjsua_app_cfg_dis_codec(pjsua_app_config *cfg, char *lval);
extern int pjsua_app_cfg_add_codec(pjsua_app_config *cfg, char *lval);
extern int pjsua_app_cfg_duration(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_thread_cnt(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_ptime(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_novad(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_ec_tial(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_ec_options(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_quality(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_ilbc_mode(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_rx_drop_pct(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_tx_drop_pct(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_auto_answer(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_max_calls(pjsua_app_config *cfg, int value);
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
extern int pjsua_app_cfg_tls_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_tls_ca_file(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_tls_cert_file(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_tls_priv_file(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_tls_password(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_tls_verify_server(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_tls_verify_client(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_tls_neg_timeout(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_tls_cipher(pjsua_app_config *cfg, char * lval);
#endif

extern int pjsua_app_cfg_capture_dev(pjsua_app_config *cfg, int value);

extern int pjsua_app_cfg_capture_lat(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_playback_dev(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_playback_lat(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_snd_auto_close(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_no_tones(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_jb_max_size(pjsua_app_config *cfg, int value);

#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
extern int pjsua_app_cfg_ipv6_enable(pjsua_app_config *cfg, pj_bool_t enable);
#endif

extern int pjsua_app_cfg_qos_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_video_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_extra_audio(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_vcapture_dev(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_vrender_dev(pjsua_app_config *cfg, int value);
extern int pjsua_app_cfg_play_avi(pjsua_app_config *cfg, char * lval);
extern int pjsua_app_cfg_auto_play_avi(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_cli_enable(pjsua_app_config *cfg, pj_bool_t enable);
extern int pjsua_app_cfg_cli_telnet_port(pjsua_app_config *cfg, int port);
extern int pjsua_app_cfg_cli_console(pjsua_app_config *cfg, pj_bool_t enable);

extern void pjsua_app_cfg_default_config(pjsua_app_config *cfg);

extern int pjsua_app_cfg_load_config(pjsua_app_config *cfg);
extern int pjsua_app_cfg_log_config(pjsua_app_config *cfg);

#ifdef __cplusplus
}
#endif

#endif	/* __PJSUA_APP_CFG_H__ */

