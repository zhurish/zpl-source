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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "pjsua_app_common.h"
#include "pjsua_app_config.h"
#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"


#define THIS_FILE "pjapp_config_t.c"

static int pjsua_media_cli_write_config(pjsua_media_config *media_cfg, struct vty *vty, pj_bool_t detail, pj_bool_t bwrt)
{
    vty_out(vty, " ipsip media clock-rate %d%s", media_cfg->clock_rate, VTY_NEWLINE);
    vty_out(vty, " ipsip media sound-clock-rate %d%s", media_cfg->snd_clock_rate, VTY_NEWLINE);
    vty_out(vty, " ipsip media channel count %d%s", media_cfg->channel_count, VTY_NEWLINE);
    vty_out(vty, " ipsip media audio frame ptime %d%s", media_cfg->audio_frame_ptime, VTY_NEWLINE);
    vty_out(vty, " ipsip media port size %d%s", media_cfg->max_media_ports, VTY_NEWLINE);
    vty_out(vty, " ipsip media quality %d%s", media_cfg->quality, VTY_NEWLINE);
    vty_out(vty, " ipsip media ptime %d%s", media_cfg->ptime, VTY_NEWLINE);

    vty_out(vty, " ipsip media vad %s%s", media_cfg->no_vad ? "disable" : "enable", VTY_NEWLINE);
    vty_out(vty, " ipsip media ilbc-mode frame %d%s", media_cfg->ilbc_mode, VTY_NEWLINE);
    vty_out(vty, " ipsip media tx-drop-pct %d%s", media_cfg->tx_drop_pct, VTY_NEWLINE);
    vty_out(vty, " ipsip media rx-drop-pct %d%s", media_cfg->rx_drop_pct, VTY_NEWLINE);

	if(media_cfg->ec_options == PJMEDIA_ECHO_DEFAULT)
		vty_out(vty, " ipsip media echo canceller algorithm default%s", VTY_NEWLINE);
	else if(media_cfg->ec_options == PJMEDIA_ECHO_SPEEX)
		vty_out(vty, " ipsip media echo canceller algorithm speex%s", VTY_NEWLINE);
	else if(media_cfg->ec_options == PJMEDIA_ECHO_SIMPLE)
		vty_out(vty, " ipsip media echo canceller algorithm simple%s", VTY_NEWLINE);
	else if(media_cfg->ec_options == PJMEDIA_ECHO_WEBRTC)
		vty_out(vty, " ipsip media echo canceller algorithm webrtc%s", VTY_NEWLINE);

    //vty_out(vty, " ipsip media echo option %d%s", media_cfg->ec_options, VTY_NEWLINE);
    vty_out(vty, " ipsip media echo-tail-len %d%s", media_cfg->ec_tail_len, VTY_NEWLINE);

    vty_out(vty, " ipsip media sound record latency %d%s", media_cfg->snd_rec_latency, VTY_NEWLINE);
    vty_out(vty, " ipsip media sound play latency %d%s", media_cfg->snd_play_latency, VTY_NEWLINE);

    vty_out(vty, " ipsip media ice %s%s", media_cfg->enable_ice ? "enable" : "disable", VTY_NEWLINE);
    vty_out(vty, " ipsip media ice host cands max %d%s", media_cfg->ice_max_host_cands, VTY_NEWLINE);
    //vty_out(vty, " ipsip media ice option %x%s", media_cfg->ice_opt, VTY_NEWLINE);
    vty_out(vty, " ipsip media ice rtcp %s%s", media_cfg->ice_no_rtcp ? "disable" : "enable", VTY_NEWLINE);

    vty_out(vty, " ipsip media turn %s%s", media_cfg->enable_turn ? "enable" : "disable", VTY_NEWLINE);
    if (media_cfg->enable_turn)
    {
        if (media_cfg->turn_server.ptr)
            vty_out(vty, " ipsip media turn server %s%s", media_cfg->turn_server.ptr, VTY_NEWLINE);

        if (media_cfg->turn_conn_type == PJ_TURN_TP_UDP)
            vty_out(vty, " ipsip media turn udp %s", VTY_NEWLINE);
        if (media_cfg->turn_conn_type == PJ_TURN_TP_TCP)
            vty_out(vty, " ipsip media turn tcp%s", VTY_NEWLINE);
        if (media_cfg->turn_conn_type == PJ_TURN_TP_TLS)
            vty_out(vty, " ipsip media turn tls%s", VTY_NEWLINE);

        if (media_cfg->turn_auth_cred.type == PJ_STUN_AUTH_CRED_DYNAMIC)
            vty_out(vty, " ipsip media turn auth dynamic%s", VTY_NEWLINE);
        else
        {
            vty_out(vty, " ipsip media turn auth static%s", VTY_NEWLINE);
            if (media_cfg->turn_auth_cred.data.static_cred.realm.ptr)
            {
                vty_out(vty, " ipsip media turn auth realm %s%s", media_cfg->turn_auth_cred.data.static_cred.realm.ptr, VTY_NEWLINE);
            }
            if (media_cfg->turn_auth_cred.data.static_cred.username.ptr)
            {
                vty_out(vty, " ipsip media turn auth username %s%s", media_cfg->turn_auth_cred.data.static_cred.username.ptr, VTY_NEWLINE);
            }
            if (media_cfg->turn_auth_cred.data.static_cred.data_type == PJ_STUN_PASSWD_HASHED &&
                media_cfg->turn_auth_cred.data.static_cred.data.ptr)
            {
                vty_out(vty, " ipsip media turn auth hashed %s%s", media_cfg->turn_auth_cred.data.static_cred.data.ptr, VTY_NEWLINE);
            }
            if (media_cfg->turn_auth_cred.data.static_cred.data_type == PJ_STUN_PASSWD_PLAIN &&
                media_cfg->turn_auth_cred.data.static_cred.data.ptr)
            {
                vty_out(vty, " ipsip media turn auth password %s%s", media_cfg->turn_auth_cred.data.static_cred.data.ptr, VTY_NEWLINE);
            }
            if (media_cfg->turn_auth_cred.data.static_cred.nonce.ptr)
            {
                vty_out(vty, " ipsip media turn auth nonce %s%s", media_cfg->turn_auth_cred.data.static_cred.nonce.ptr, VTY_NEWLINE);
            }
        }
        if (media_cfg->turn_tls_setting.ca_list_file.ptr)
        {
            vty_out(vty, " ipsip media turn tls certificate-list %s%s", media_cfg->turn_tls_setting.ca_list_file.ptr, VTY_NEWLINE);
        }
        if (media_cfg->turn_tls_setting.cert_file.ptr)
        {
            vty_out(vty, " ipsip media turn tls certificate %s%s", media_cfg->turn_tls_setting.cert_file.ptr, VTY_NEWLINE);
        }
        if (media_cfg->turn_tls_setting.privkey_file.ptr)
        {
            vty_out(vty, " ipsip media turn tls privkeyfile %s%s", media_cfg->turn_tls_setting.privkey_file.ptr, VTY_NEWLINE);
        }
        if (media_cfg->turn_tls_setting.ca_buf.ptr)
        {
            vty_out(vty, " ipsip media turn tls certificate-list-key %s%s", media_cfg->turn_tls_setting.ca_buf.ptr, VTY_NEWLINE);
        }
        if (media_cfg->turn_tls_setting.cert_buf.ptr)
        {
            vty_out(vty, " ipsip media turn tls certificate-key %s%s", media_cfg->turn_tls_setting.cert_buf.ptr, VTY_NEWLINE);
        }
        if (media_cfg->turn_tls_setting.privkey_buf.ptr)
        {
            vty_out(vty, " ipsip media turn tls priv-key %s%s", media_cfg->turn_tls_setting.privkey_buf.ptr, VTY_NEWLINE);
        }
        if (media_cfg->turn_tls_setting.password.ptr)
        {
            vty_out(vty, " ipsip media turn tls password %s%s", media_cfg->turn_tls_setting.password.ptr, VTY_NEWLINE);
        }
        if (media_cfg->turn_tls_setting.ssock_param.proto)
        {
            vty_out(vty, " ipsip media turn tls sock proto %d%s", media_cfg->turn_tls_setting.ssock_param.proto, VTY_NEWLINE);
        }
        if (media_cfg->turn_tls_setting.ssock_param.sigalgs.ptr)
        {
            vty_out(vty, " ipsip media turn tls sock sigalgs %s%s", media_cfg->turn_tls_setting.ssock_param.sigalgs.ptr, VTY_NEWLINE);
        }
        if (media_cfg->turn_tls_setting.ssock_param.server_name.ptr)
        {
            vty_out(vty, " ipsip media turn tls sock server-name %s%s", media_cfg->turn_tls_setting.ssock_param.server_name.ptr, VTY_NEWLINE);
        }
    }
    vty_out(vty, " ipsip media sound auto close time %d%s", media_cfg->snd_auto_close_time, VTY_NEWLINE);
    vty_out(vty, " ipsip media video preview native %s%s", media_cfg->vid_preview_enable_native ? "enable" : "disable", VTY_NEWLINE);
    vty_out(vty, " ipsip media smart update %s%s", media_cfg->no_smart_media_update ? "enable" : "disable", VTY_NEWLINE);
    vty_out(vty, " ipsip media rtcp sded-bye %s%s", media_cfg->no_rtcp_sdes_bye ? "enable" : "disable", VTY_NEWLINE);
    return 0;
}

static int pjsua_sua_cli_write_config(pjsua_config *cfg, struct vty *vty, pj_bool_t detail, pj_bool_t bwrt)
{
    int i = 0, flag = 0;
    //vty_out(vty, " ipsip sua call max %d%s", cfg->max_calls, VTY_NEWLINE);
    //vty_out(vty, " ipsip sua thread max %d%s", cfg->thread_cnt, VTY_NEWLINE);
    //vty_out(vty, " ipsip sua nameserver max %d%s", cfg->nameserver_count, VTY_NEWLINE);
    for (i = 0; i < 4; i++)
    {
        if (cfg->nameserver[i].ptr)
            vty_out(vty, " ipsip sua nameserver %s%s", cfg->nameserver[i].ptr, VTY_NEWLINE);
    }
    vty_out(vty, " ipsip sua loose-route %s%s", cfg->force_lr ? "enable" : "disable", VTY_NEWLINE);
    //vty_out(vty, " ipsip sua outbound-proxy max %d%s", cfg->outbound_proxy_cnt, VTY_NEWLINE);
    for (i = 0; i < 4; i++)
    {
        if (cfg->outbound_proxy[i].ptr)
            vty_out(vty, " ipsip sua outbound-proxy %s%s", cfg->outbound_proxy[i].ptr, VTY_NEWLINE);
    }
    if (cfg->stun_domain.ptr)
        vty_out(vty, " ipsip sua stun domain %s%s", cfg->stun_domain.ptr, VTY_NEWLINE);
    if (cfg->stun_host.ptr)
        vty_out(vty, " ipsip sua stun host %s%s", cfg->stun_host.ptr, VTY_NEWLINE);

    //vty_out(vty, " ipsip sua stun-server max %d%s", cfg->stun_srv_cnt, VTY_NEWLINE);
    for (i = 0; i < 8; i++)
    {
        if (cfg->stun_srv[i].ptr)
            vty_out(vty, " ipsip sua stun-server %s%s", cfg->stun_srv[i].ptr, VTY_NEWLINE);
    }
    vty_out(vty, " ipsip sua stun-ipv6 %s%s", cfg->stun_try_ipv6 ? "enable" : "disable", VTY_NEWLINE);

    if (cfg->require_100rel == PJSUA_100REL_MANDATORY)
        vty_out(vty, " ipsip sua 100real mandatory %s", VTY_NEWLINE);
    if (cfg->require_100rel == PJSUA_100REL_OPTIONAL)
        vty_out(vty, " ipsip sua 100real optional %s", VTY_NEWLINE);

    //vty_out(vty, " ipsip sua credentials max %d%s", cfg->cred_count, VTY_NEWLINE);
    for (i = 0; i < PJSUA_ACC_MAX_PROXIES; i++)
    {
        if (cfg->cred_info[i].username.ptr)
        {
            flag = 1;
            vty_out(vty, " ipsip sua username %s", cfg->cred_info[i].username.ptr);
        }
        if (cfg->cred_info[i].data.ptr && cfg->cred_info[i].data_type == 0)
        {
            flag = 1;
            vty_out(vty, " password %s",cfg->cred_info[i].data.ptr);
        }
        if (cfg->cred_info[i].realm.ptr)
        {
            flag = 1;
            vty_out(vty, " realm %s", cfg->cred_info[i].realm.ptr);
        }
        if (cfg->cred_info[i].scheme.ptr)
        {
            flag = 1;
            vty_out(vty, " scheme %s", cfg->cred_info[i].scheme.ptr);
        }
        if(flag)
        {
            vty_out(vty, "%s",  VTY_NEWLINE);    
            flag = 0;
        }
    }
    if (cfg->user_agent.ptr)
        vty_out(vty, " ipsip sua user-agent %s%s", cfg->user_agent.ptr, VTY_NEWLINE);

    if (cfg->use_srtp == PJMEDIA_SRTP_MANDATORY)
        vty_out(vty, " ipsip sua srtp mandatory %s", VTY_NEWLINE);
    if (cfg->use_srtp == PJMEDIA_SRTP_OPTIONAL)
        vty_out(vty, " ipsip sua srtp optional %s", VTY_NEWLINE);

    if (cfg->srtp_secure_signaling == 0)
        vty_out(vty, " ipsip sua secure signaling disable%s", VTY_NEWLINE);
    if (cfg->srtp_secure_signaling == 1)
        vty_out(vty, " ipsip sua secure signaling over tls%s", VTY_NEWLINE);
    if (cfg->srtp_secure_signaling == 2)
        vty_out(vty, " ipsip sua secure signaling endtoend%s", VTY_NEWLINE);

    vty_out(vty, " ipsip sua srtp optional dup-offer %s%s", cfg->srtp_optional_dup_offer ? "enable" : "disable", VTY_NEWLINE);

    //vty_out(vty, " ipsip sua srtp crypto max %d%s", cfg->srtp_opt.crypto_count, VTY_NEWLINE);
    for (i = 0; i < PJMEDIA_SRTP_MAX_CRYPTOS; i++)
    {
        if (cfg->srtp_opt.crypto[i].name.ptr && cfg->srtp_opt.crypto[i].key.ptr)
            vty_out(vty, " ipsip sua srtp crypto name %s key %s%s", cfg->srtp_opt.crypto[i].name.ptr, cfg->srtp_opt.crypto[i].key.ptr, VTY_NEWLINE);
    }
    //vty_out(vty, " ipsip sua srtp key max %d%s", cfg->srtp_opt.keying_count, VTY_NEWLINE);
    for (i = 0; i < PJMEDIA_SRTP_KEYINGS_COUNT; i++)
    {
        if (cfg->srtp_opt.keying[i] == PJMEDIA_SRTP_KEYING_SDES)
            vty_out(vty, " ipsip sua srtp key sdes%s", VTY_NEWLINE);
        if (cfg->srtp_opt.keying[i] == PJMEDIA_SRTP_KEYING_DTLS_SRTP)
            vty_out(vty, " ipsip sua srtp key dtls%s", VTY_NEWLINE);
    }
    //vty_out(vty, " ipsip sua upnp %s%s", cfg->enable_upnp ? "enable" : "disable", VTY_NEWLINE);
    if (cfg->upnp_if_name.ptr)
        vty_out(vty, " ipsip sua upnp interface %s%s", cfg->upnp_if_name.ptr, VTY_NEWLINE);
    return 0;
}

static int pjsua_transport_cli_write_config(pjsua_transport_config *cfg, char *hdr, struct vty *vty, pj_bool_t detail, pj_bool_t bwrt)
{
    vty_out(vty, " ipsip transport %s port %d%s", hdr, cfg->port, VTY_NEWLINE);
    if (cfg->port_range)
        vty_out(vty, " ipsip transport %s port %d-%d%s", hdr, cfg->port, cfg->port_range, VTY_NEWLINE);

    vty_out(vty, " ipsip transport %s randomize %s%s", hdr, cfg->randomize_port ? "enable" : "disable", VTY_NEWLINE);
    if (cfg->public_addr.ptr)
        vty_out(vty, " ipsip transport %s public address %s%s", hdr, cfg->public_addr.ptr, VTY_NEWLINE);

    if (cfg->bound_addr.ptr)
        vty_out(vty, " ipsip transport %s bound address %s%s", hdr, cfg->bound_addr.ptr, VTY_NEWLINE);

    if (cfg->tls_setting.ca_list_file.ptr)
    {
        vty_out(vty, " ipsip transport %s tls certificate-list %s%s", hdr, cfg->tls_setting.ca_list_file.ptr, VTY_NEWLINE);
    }
    if (cfg->tls_setting.cert_file.ptr)
    {
        vty_out(vty, " ipsip transport %s tls certificate %s%s", hdr, cfg->tls_setting.cert_file.ptr, VTY_NEWLINE);
    }
    if (cfg->tls_setting.privkey_file.ptr)
    {
        vty_out(vty, " ipsip transport %s tls privkeyfile %s%s", hdr, cfg->tls_setting.privkey_file.ptr, VTY_NEWLINE);
    }
    if (cfg->tls_setting.ca_buf.ptr)
    {
        vty_out(vty, " ipsip transport %s tls certificate-list-key %s%s", hdr, cfg->tls_setting.ca_buf.ptr, VTY_NEWLINE);
    }
    if (cfg->tls_setting.cert_buf.ptr)
    {
        vty_out(vty, " ipsip transport %s tls certificate-key %s%s", hdr, cfg->tls_setting.cert_buf.ptr, VTY_NEWLINE);
    }
    if (cfg->tls_setting.privkey_buf.ptr)
    {
        vty_out(vty, " ipsip transport %s tls priv-key %s%s", hdr, cfg->tls_setting.privkey_buf.ptr, VTY_NEWLINE);
    }
    if (cfg->tls_setting.password.ptr)
    {
        vty_out(vty, " ipsip transport %s tls password %s%s", hdr, cfg->tls_setting.password.ptr, VTY_NEWLINE);
    }
    return 0;
}

static int pjsua_account_cli_write_config(pjsua_acc_config *cfg, struct vty *vty, pj_bool_t detail, pj_bool_t bwrt)
{
    int i = 0, flag = 0;
    pjapp_username_t *userdata = cfg->user_data;
    if (cfg && userdata)
    {
        vty_out(vty, " ipsip account %s%s", userdata->sip_phone, VTY_NEWLINE);

        vty_out(vty, " ipsip account %s priority %d%s", userdata->sip_phone, cfg->priority, VTY_NEWLINE);
        if (cfg->reg_uri.ptr)
        {
            vty_out(vty, " ipsip account %s register-server %s port %d%s", userdata->sip_phone, userdata->register_svr.sip_address, userdata->register_svr.sip_port, VTY_NEWLINE);
        }

        vty_out(vty, " ipsip account %s mwi %s%s", userdata->sip_phone, cfg->mwi_enabled ? "enable" : "disable", VTY_NEWLINE);
        vty_out(vty, " ipsip account %s mwi expires %d%s", userdata->sip_phone, cfg->mwi_expires, VTY_NEWLINE);
        vty_out(vty, " ipsip account %s publish %s%s", userdata->sip_phone, cfg->publish_enabled ? "enable" : "disable", VTY_NEWLINE);

        vty_out(vty, " ipsip account %s unpublish wait %d%s", userdata->sip_phone, cfg->unpublish_max_wait_time_msec, VTY_NEWLINE);

        //vty_out(vty, " ipsip account %s auth preference initail %s%s", userdata->sip_phone, cfg->auth_pref.initial_auth ? "enable" : "disable", VTY_NEWLINE);
        if (cfg->auth_pref.algorithm.ptr)
        {
            vty_out(vty, " ipsip account %s auth preference algorithm %s%s", userdata->sip_phone, cfg->auth_pref.algorithm.ptr, VTY_NEWLINE);
        }

        if (cfg->require_100rel == PJSUA_100REL_MANDATORY)
            vty_out(vty, " ipsip account %s 100rel mandatory %s", userdata->sip_phone, VTY_NEWLINE);
        if (cfg->require_100rel == PJSUA_100REL_OPTIONAL)
            vty_out(vty, " ipsip account %s 100rel optional %s", userdata->sip_phone, VTY_NEWLINE);

        //vty_out(vty, " ipsip account %s proxy max %d%s", userdata->sip_phone, cfg->proxy_cnt, VTY_NEWLINE);
        for (i = 0; i < PJSUA_ACC_MAX_PROXIES; i++)
        {
            if (cfg->proxy[i].ptr)
            {
                vty_out(vty, " ipsip account %s proxy %s%s", userdata->sip_phone, cfg->proxy[i].ptr, VTY_NEWLINE);
            }
        }
        vty_out(vty, " ipsip account %s register timeout %d%s", userdata->sip_phone, cfg->reg_timeout, VTY_NEWLINE);
        vty_out(vty, " ipsip account %s register refresh delay %d%s", userdata->sip_phone, cfg->reg_delay_before_refresh, VTY_NEWLINE);
        vty_out(vty, " ipsip account %s unregister timeout %d%s", userdata->sip_phone, cfg->unreg_timeout, VTY_NEWLINE);

        //vty_out(vty, " ipsip account %s credentials max %d%s", userdata->sip_phone, cfg->cred_count, VTY_NEWLINE);
        for (i = 0; i < PJSUA_ACC_MAX_PROXIES; i++)
        {
            if (cfg->cred_info[i].username.ptr)
            {
                vty_out(vty, " ipsip account %s username %s", userdata->sip_phone, cfg->cred_info[i].username.ptr);
                flag = 1;
            }
            if (cfg->cred_info[i].data.ptr && cfg->cred_info[i].data_type == 0)
            {
                vty_out(vty, " password %s",cfg->cred_info[i].data.ptr);
                flag = 1;
            }
            if (cfg->cred_info[i].scheme.ptr)
            {
                vty_out(vty, "  scheme %s", cfg->cred_info[i].scheme.ptr);
                flag = 1;
            }
            if (cfg->cred_info[i].realm.ptr)
            {
                vty_out(vty, " realm %s", cfg->cred_info[i].realm.ptr);
                flag = 1;
            }
            if(flag)
            {
                vty_out(vty, "%s", VTY_NEWLINE);
                flag = 0;
            }
        }

        if (cfg->rfc5626_instance_id.ptr)
            vty_out(vty, " ipsip account %s rfc5626-instance-id %s%s", userdata->sip_phone, cfg->rfc5626_instance_id.ptr, VTY_NEWLINE);

        if (cfg->rfc5626_reg_id.ptr)
            vty_out(vty, " ipsip account %s rfc5626-register-id %s%s", userdata->sip_phone, cfg->rfc5626_reg_id.ptr, VTY_NEWLINE);
 
        vty_out(vty, " ipsip account %s video auto show %s%s", userdata->sip_phone, cfg->vid_in_auto_show ? "enable" : "disable", VTY_NEWLINE);
        vty_out(vty, " ipsip account %s video auto transmit %s%s", userdata->sip_phone, cfg->vid_out_auto_transmit ? "enable" : "disable", VTY_NEWLINE);

        vty_out(vty, " ipsip account %s video record bandwidth %d%s", userdata->sip_phone, cfg->vid_stream_rc_cfg.bandwidth, VTY_NEWLINE);
        vty_out(vty, " ipsip account %s video keyframe interval %d%s", userdata->sip_phone, cfg->vid_stream_sk_cfg.interval, VTY_NEWLINE);

        vty_out(vty, " ipsip account %s nat64 %s%s", userdata->sip_phone, cfg->nat64_opt ? "enable" : "disable", VTY_NEWLINE);
        vty_out(vty, " ipsip account %s media ipv6 %s%s", userdata->sip_phone, cfg->ipv6_media_use ? "enable" : "disable", VTY_NEWLINE);

        vty_out(vty, " ipsip account %s sip stun %s%s", userdata->sip_phone, (cfg->sip_stun_use == PJSUA_STUN_USE_DISABLED) ? "disable" : " ", VTY_NEWLINE);

        vty_out(vty, " ipsip account %s media stun %s%s", userdata->sip_phone, (cfg->media_stun_use == PJSUA_STUN_USE_DISABLED) ? "disable" : " ", VTY_NEWLINE);

        vty_out(vty, " ipsip account %s sip upnp %s%s", userdata->sip_phone, (cfg->sip_upnp_use == PJSUA_UPNP_USE_DISABLED) ? "disable" : " ", VTY_NEWLINE);

        vty_out(vty, " ipsip account %s media upnp %s%s", userdata->sip_phone, (cfg->media_upnp_use == PJSUA_UPNP_USE_DISABLED) ? "disable" : " ", VTY_NEWLINE);

        vty_out(vty, " ipsip account %s media loopback %s%s", userdata->sip_phone, cfg->use_loop_med_tp ? "enable" : "disable", VTY_NEWLINE);
        vty_out(vty, " ipsip account %s loopback %s%s", userdata->sip_phone, cfg->enable_loopback ? "enable" : "disable", VTY_NEWLINE);

        //vty_out(vty, " ipsip account %s ice %s%s", userdata->sip_phone, (cfg->ice_cfg_use == PJSUA_ICE_CONFIG_USE_CUSTOM) ? "disable" : " ", VTY_NEWLINE);
        if (cfg->ice_cfg_use == PJSUA_ICE_CONFIG_USE_CUSTOM)
        {
            vty_out(vty, " ipsip account %s ice %s%s", userdata->sip_phone, cfg->ice_cfg.enable_ice ? "enable" : "disable", VTY_NEWLINE);
            vty_out(vty, " ipsip account %s ice host cands max %d%s", userdata->sip_phone, cfg->ice_cfg.ice_max_host_cands, VTY_NEWLINE);
            // vty_out(vty, " ipsip account %s ice option %s%s", userdata->sip_phone, cfg->ice_cfg.ice_opt ?"enable":"disable", VTY_NEWLINE);
            vty_out(vty, " ipsip account %s ice rtcp %s%s", userdata->sip_phone, cfg->ice_cfg.ice_no_rtcp ? "disable" : "enable", VTY_NEWLINE);
        }

        //vty_out(vty, " ipsip account %s turn %s%s", userdata->sip_phone, (cfg->turn_cfg_use == PJSUA_TURN_CONFIG_USE_CUSTOM) ? "disable" : " ", VTY_NEWLINE);
        if (cfg->turn_cfg_use == PJSUA_TURN_CONFIG_USE_CUSTOM)
        {
            vty_out(vty, " ipsip account %s turn %s%s", userdata->sip_phone, cfg->turn_cfg.enable_turn ? "enable" : "disable", VTY_NEWLINE);
            if (cfg->turn_cfg.enable_turn)
            {
                if (cfg->turn_cfg.turn_server.ptr)
                {
                    // sip:%s:%d
                    vty_out(vty, " ipsip account %s turn server %s%s", userdata->sip_phone, cfg->turn_cfg.turn_server.ptr, VTY_NEWLINE);
                }
                if (cfg->turn_cfg.turn_conn_type == PJ_TURN_TP_UDP)
                    vty_out(vty, " ipsip account %s turn udp %s", userdata->sip_phone, VTY_NEWLINE);
                if (cfg->turn_cfg.turn_conn_type == PJ_TURN_TP_TCP)
                    vty_out(vty, " ipsip account %s turn tcp%s", userdata->sip_phone, VTY_NEWLINE);
                if (cfg->turn_cfg.turn_conn_type == PJ_TURN_TP_TLS)
                    vty_out(vty, " ipsip account %s turn tls%s", userdata->sip_phone, VTY_NEWLINE);

                if (cfg->turn_cfg.turn_auth_cred.type == PJ_STUN_AUTH_CRED_DYNAMIC)
                    vty_out(vty, " ipsip account %s turn auth dynamic%s", userdata->sip_phone, VTY_NEWLINE);
                else
                {
                    //vty_out(vty, " ipsip account %s turn auth static%s", userdata->sip_phone, VTY_NEWLINE);
                    if (cfg->turn_cfg.turn_auth_cred.data.static_cred.username.ptr)
                    {
                        vty_out(vty, " ipsip account %s turn auth username %s", userdata->sip_phone, cfg->turn_cfg.turn_auth_cred.data.static_cred.username.ptr);
                        if (cfg->turn_cfg.turn_auth_cred.data.static_cred.data_type == PJ_STUN_PASSWD_PLAIN &&
                            cfg->turn_cfg.turn_auth_cred.data.static_cred.data.ptr)
                        {
                            flag = 1;
                            vty_out(vty, " password %s", cfg->turn_cfg.turn_auth_cred.data.static_cred.data.ptr);
                        }
                        if (cfg->turn_cfg.turn_auth_cred.data.static_cred.realm.ptr)
                        {
                            flag = 1;
                            vty_out(vty, " realm %s", cfg->turn_cfg.turn_auth_cred.data.static_cred.realm.ptr);
                        }
                        if(flag)
                        {
                            vty_out(vty, "%s", VTY_NEWLINE);
                            flag = 0;
                        }
                    }
/*
                        if (cfg->turn_cfg.turn_auth_cred.data.static_cred.data_type == PJ_STUN_PASSWD_HASHED &&
                            cfg->turn_cfg.turn_auth_cred.data.static_cred.data.ptr)
                        {
                            vty_out(vty, " ipsip account %s turn auth hashed %s%s", userdata->sip_phone, cfg->turn_cfg.turn_auth_cred.data.static_cred.data.ptr, VTY_NEWLINE);
                        }

                        if (cfg->turn_cfg.turn_auth_cred.data.static_cred.nonce.ptr)
                        {
                            vty_out(vty, " ipsip account %s turn auth nonce %s%s", userdata->sip_phone, cfg->turn_cfg.turn_auth_cred.data.static_cred.nonce.ptr, VTY_NEWLINE);
                        }*/

                }
                if (cfg->turn_cfg.turn_tls_setting.ca_list_file.ptr)
                {
                    vty_out(vty, " ipsip account %s turn tls certificate-list %s%s", userdata->sip_phone, cfg->turn_cfg.turn_tls_setting.ca_list_file.ptr, VTY_NEWLINE);
                }
                if (cfg->turn_cfg.turn_tls_setting.cert_file.ptr)
                {
                    vty_out(vty, " ipsip account %s turn tls certificate %s%s", userdata->sip_phone, cfg->turn_cfg.turn_tls_setting.cert_file.ptr, VTY_NEWLINE);
                }
                if (cfg->turn_cfg.turn_tls_setting.privkey_file.ptr)
                {
                    vty_out(vty, " ipsip account %s turn tls privkeyfile %s%s", userdata->sip_phone, cfg->turn_cfg.turn_tls_setting.privkey_file.ptr, VTY_NEWLINE);
                }
                if (cfg->turn_cfg.turn_tls_setting.ca_buf.ptr)
                {
                    vty_out(vty, " ipsip account %s turn tls certificate-list-key %s%s", userdata->sip_phone, cfg->turn_cfg.turn_tls_setting.ca_buf.ptr, VTY_NEWLINE);
                }
                if (cfg->turn_cfg.turn_tls_setting.cert_buf.ptr)
                {
                    vty_out(vty, " ipsip account %s turn tls certificate-key %s%s", userdata->sip_phone, cfg->turn_cfg.turn_tls_setting.cert_buf.ptr, VTY_NEWLINE);
                }
                if (cfg->turn_cfg.turn_tls_setting.privkey_buf.ptr)
                {
                    vty_out(vty, " ipsip account %s turn tls priv-key %s%s", userdata->sip_phone, cfg->turn_cfg.turn_tls_setting.privkey_buf.ptr, VTY_NEWLINE);
                }
                if (cfg->turn_cfg.turn_tls_setting.password.ptr)
                {
                    vty_out(vty, " ipsip account %s turn tls password %s%s", userdata->sip_phone, cfg->turn_cfg.turn_tls_setting.password.ptr, VTY_NEWLINE);
                }
                if (cfg->turn_cfg.turn_tls_setting.ssock_param.proto)
                {
                    vty_out(vty, " ipsip account %s turn tls sock proto %d%s", userdata->sip_phone, cfg->turn_cfg.turn_tls_setting.ssock_param.proto, VTY_NEWLINE);
                }
                if (cfg->turn_cfg.turn_tls_setting.ssock_param.sigalgs.ptr)
                {
                    vty_out(vty, " ipsip account %s turn tls sock sigalgs %s%s", userdata->sip_phone, cfg->turn_cfg.turn_tls_setting.ssock_param.sigalgs.ptr, VTY_NEWLINE);
                }
                if (cfg->turn_cfg.turn_tls_setting.ssock_param.server_name.ptr)
                {
                    vty_out(vty, " ipsip account %s turn tls sock server-name %s%s", userdata->sip_phone, cfg->turn_cfg.turn_tls_setting.ssock_param.server_name.ptr, VTY_NEWLINE);
                }
            }
        }

        if (cfg->use_srtp == PJMEDIA_SRTP_MANDATORY)
            vty_out(vty, " ipsip account %s srtp mandatory %s", userdata->sip_phone, VTY_NEWLINE);
        if (cfg->use_srtp == PJMEDIA_SRTP_OPTIONAL)
            vty_out(vty, " ipsip account %s srtp optional %s", userdata->sip_phone, VTY_NEWLINE);

        if (cfg->srtp_secure_signaling == 0)
            vty_out(vty, " ipsip account %s secure signaling disable%s", userdata->sip_phone, VTY_NEWLINE);
        if (cfg->srtp_secure_signaling == 1)
            vty_out(vty, " ipsip account %s secure signaling over tls%s", userdata->sip_phone, VTY_NEWLINE);
        if (cfg->srtp_secure_signaling == 2)
            vty_out(vty, " ipsip account %s secure signaling endtoend%s", userdata->sip_phone, VTY_NEWLINE);

        //vty_out(vty, " ipsip account %s srtp optional dup-offer %s%s", userdata->sip_phone, cfg->srtp_optional_dup_offer ? "enable" : "disable", VTY_NEWLINE);

        //vty_out(vty, " ipsip account %s srtp crypto max %d%s", userdata->sip_phone, cfg->srtp_opt.crypto_count, VTY_NEWLINE);
        for (i = 0; i < PJMEDIA_SRTP_MAX_CRYPTOS; i++)
        {
            if (cfg->srtp_opt.crypto[i].name.ptr && cfg->srtp_opt.crypto[i].key.ptr)
                vty_out(vty, " ipsip account %s srtp crypto name %s key %s%s", userdata->sip_phone, 
                    cfg->srtp_opt.crypto[i].name.ptr, cfg->srtp_opt.crypto[i].key.ptr, VTY_NEWLINE);
        }

        //vty_out(vty, " ipsip account %s srtp key max %d%s", userdata->sip_phone, cfg->srtp_opt.keying_count, VTY_NEWLINE);
        for (i = 0; i < PJMEDIA_SRTP_KEYINGS_COUNT; i++)
        {
            if (cfg->srtp_opt.keying[i] == PJMEDIA_SRTP_KEYING_SDES)
                vty_out(vty, " ipsip account %s srtp key sdes%s", userdata->sip_phone, VTY_NEWLINE);
            else if (cfg->srtp_opt.keying[i] == PJMEDIA_SRTP_KEYING_DTLS_SRTP)
                vty_out(vty, " ipsip account %s srtp key dtls%s", userdata->sip_phone, VTY_NEWLINE);
        }
        vty_out(vty, " ipsip account %s register retry interval %d%s", userdata->sip_phone, cfg->reg_retry_interval, VTY_NEWLINE);
        //vty_out(vty, " ipsip account %s register proxy %x%s", userdata->sip_phone, cfg->reg_use_proxy, VTY_NEWLINE);
    }
    return 0;
}

static int pjsua_buddy_cli_write_config(pjsua_buddy_config *cfg, struct vty *vty, pj_bool_t detail, pj_bool_t bwrt)
{
    if (cfg->uri.ptr)
    {
        vty_out(vty, " ipsip buddy id %s%s", cfg->uri.ptr, VTY_NEWLINE);
        vty_out(vty, " ipsip buddy %s subscribe %s%s", cfg->uri.ptr, cfg->subscribe ? "enable" : "disable", VTY_NEWLINE);
    }
    return 0;
}

static int pjsua_app_misc_cli_write_config(pjsua_app_config *cfg, struct vty *vty, pj_bool_t detail, pj_bool_t bwrt)
{
    int i = 0;
    vty_out(vty, " ipsip global ipv6 %s%s", cfg->ipv6 ? "enable" : "disable", VTY_NEWLINE);
    vty_out(vty, " ipsip global refersub %s%s", cfg->no_refersub ? "disable" : "enable", VTY_NEWLINE);
    vty_out(vty, " ipsip global qos %s%s", cfg->enable_qos ? "enable" : "disable", VTY_NEWLINE);
    vty_out(vty, " ipsip global tcp %s%s", cfg->no_tcp ? "disable" : "enable", VTY_NEWLINE);
    vty_out(vty, " ipsip global udp %s%s", cfg->no_udp ? "disable" : "enable", VTY_NEWLINE);
    vty_out(vty, " ipsip global tls %s%s", (!cfg->use_tls) ? "disable" : "enable", VTY_NEWLINE);
    vty_out(vty, " ipsip global srtp keying %s%s", (!cfg->srtp_keying) ? "disable" : "enable", VTY_NEWLINE);

    for (i = 0; i < cfg->codec_cnt; i++)
    {
        if (cfg->codec_arg[i].ptr)
            vty_out(vty, " ipsip global codec %d %s%s", i, cfg->codec_arg[i].ptr, VTY_NEWLINE);
    }
    for (i = 0; i < cfg->codec_dis_cnt; i++)
    {
        if (cfg->codec_dis[i].ptr)
            vty_out(vty, " ipsip global codec %d %s disable%s", i, cfg->codec_dis[i].ptr, VTY_NEWLINE);
    }
    vty_out(vty, " ipsip global audio-null %s%s", (cfg->null_audio) ? "enable" : "disable", VTY_NEWLINE);

    for (i = 0; i < cfg->wav_count; i++)
    {
        if (cfg->wav_files[i].ptr)
            vty_out(vty, " ipsip global wav %d %s%s", i, cfg->wav_files[i].ptr, VTY_NEWLINE);
    }
    vty_out(vty, " ipsip global tones %s%s", (cfg->no_tones) ? "disable" : "enable", VTY_NEWLINE);

    for (i = 0; i < cfg->tone_count; i++)
    {
        vty_out(vty, " ipsip global tone %d freq1 %d%s", i, cfg->tones[i].freq1, VTY_NEWLINE);
        vty_out(vty, " ipsip global tone %d freq2 %d%s", i, cfg->tones[i].freq2, VTY_NEWLINE);
        vty_out(vty, " ipsip global tone %d on_msec %d%s", i, cfg->tones[i].on_msec, VTY_NEWLINE);
        vty_out(vty, " ipsip global tone %d off_msec %d%s", i, cfg->tones[i].off_msec, VTY_NEWLINE);
        vty_out(vty, " ipsip global tone %d volume %d%s", i, cfg->tones[i].volume, VTY_NEWLINE);
    }

    vty_out(vty, " ipsip global audio auto play %s%s", (cfg->auto_play) ? "enable" : "disable", VTY_NEWLINE);
    vty_out(vty, " ipsip global audio auto hangup %s%s", (cfg->auto_play_hangup) ? "enable" : "disable", VTY_NEWLINE);

    vty_out(vty, " ipsip global audio auto loop %s%s", (cfg->auto_loop) ? "enable" : "disable", VTY_NEWLINE);

    vty_out(vty, " ipsip global audio auto configure %s%s", (cfg->auto_conf) ? "enable" : "disable", VTY_NEWLINE);
    vty_out(vty, " ipsip global audio auto record %s%s", (cfg->auto_rec) ? "enable" : "disable", VTY_NEWLINE);

    if (cfg->rec_file.ptr)
        vty_out(vty, " ipsip global record file %s%s", cfg->rec_file.ptr, VTY_NEWLINE);

    vty_out(vty, " ipsip global auto answer %d%s", cfg->auto_answer, VTY_NEWLINE);
    vty_out(vty, " ipsip global duration %d%s", cfg->duration, VTY_NEWLINE);

    vty_out(vty, " ipsip global mic level %.2f speaker level %.2f%s", cfg->mic_level, cfg->speaker_level, VTY_NEWLINE);

    for (i = 0; i < cfg->avi_cnt; i++)
    {
        if (cfg->avi[i].path.ptr)
            vty_out(vty, " ipsip global avi %d file %s%s", i, cfg->avi[i].path.ptr, VTY_NEWLINE);
    }
    vty_out(vty, " ipsip global avi default idx %d%s", cfg->avi_def_idx, VTY_NEWLINE);
    vty_out(vty, " ipsip global avi auto play %s%s", (cfg->avi_auto_play) ? "enable" : "disable", VTY_NEWLINE);

    vty_out(vty, " ipsip global video auto transmit %s%s", (cfg->vid.out_auto_transmit) ? "enable" : "disable", VTY_NEWLINE);
    vty_out(vty, " ipsip global video auto show %s%s", (cfg->vid.in_auto_show) ? "enable" : "disable", VTY_NEWLINE);

    vty_out(vty, " ipsip global video devcnt %d%s", cfg->vid.vid_cnt, VTY_NEWLINE);
    vty_out(vty, " ipsip global video vcapture_dev %d%s", cfg->vid.vcapture_dev, VTY_NEWLINE);
    vty_out(vty, " ipsip global video vrender_dev %d%s", cfg->vid.vrender_dev, VTY_NEWLINE);
    if (strlen(cfg->vid.vcapture_dev_name))
        vty_out(vty, " ipsip global video vcapture_dev_name %s%s", cfg->vid.vcapture_dev_name, VTY_NEWLINE);
    if (strlen(cfg->vid.vrender_dev_name))
        vty_out(vty, " ipsip global video vrender_dev_name %s%s", cfg->vid.vrender_dev_name, VTY_NEWLINE);

    vty_out(vty, " ipsip global sound devcnt %d%s", cfg->aud_cnt, VTY_NEWLINE);
    vty_out(vty, " ipsip global sound capture_dev %d%s", cfg->capture_dev, VTY_NEWLINE);
    vty_out(vty, " ipsip global sound playback_dev %d%s", cfg->playback_dev, VTY_NEWLINE);
    vty_out(vty, " ipsip global sound capture_lat %d%s", cfg->capture_lat, VTY_NEWLINE);
    vty_out(vty, " ipsip global sound playback_lat %d%s", cfg->playback_lat, VTY_NEWLINE);
    if (strlen(cfg->capture_dev_name))
        vty_out(vty, " ipsip global sound capture_dev_name %s%s", cfg->capture_dev_name, VTY_NEWLINE);
    if (strlen(cfg->playback_dev_name))
        vty_out(vty, " ipsip global sound playback_dev_name %s%s", cfg->playback_dev_name, VTY_NEWLINE);
    return 0;
}

static int pjsua_app_cli_write_config(pjsua_app_config *appcfg, struct vty *vty, pj_bool_t detail, pj_bool_t bwrt)
{
    int i = 0;
    pjsua_sua_cli_write_config(&appcfg->cfg, vty, detail, bwrt);

    pjsua_transport_cli_write_config(&appcfg->udp_cfg, "udp", vty, detail, bwrt);

    pjsua_transport_cli_write_config(&appcfg->rtp_cfg, "rtp", vty, detail, bwrt);

    for (i = 0; i < appcfg->acc_cnt; i++)
        pjsua_account_cli_write_config(&appcfg->acc_cfg[i], vty, detail, bwrt);

    for (i = 0; i < appcfg->buddy_cnt; i++)
        pjsua_buddy_cli_write_config(&appcfg->buddy_cfg[i], vty, detail, bwrt);

    pjsua_media_cli_write_config(&appcfg->media_cfg, vty, detail, bwrt);

    pjsua_app_misc_cli_write_config(appcfg, vty, detail, bwrt);

    return 0;
}

int pjapp_global_cli_write_config(void *p, pj_bool_t detail, pj_bool_t bwrt)
{
    struct vty *vty = p;
    // pj_bool_t           global_enable;//全局使能
    // pjsua_call_setting  call_opt;
    if(_pjAppCfg.global_enable)
    {
        vty_out(vty, " ipsip global %s%s", (_pjAppCfg.global_enable) ? "enable" : "disable", VTY_NEWLINE);
        pjsua_app_cli_write_config(&_pjAppCfg, vty, detail, bwrt);
    }
    return 0;
}
