/* $Id: pjsua_app_config.c 5788 2018-05-09 06:58:48Z ming $ */
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
#include <zplos_include.h>
#include <log.h>
#include "pjsua_app_common.h"
#include "pjsip_app_api.h"
#include "pjsua_app_cfgapi.h"

#define THIS_FILE	"pjsua_app_cfgapi.c"

static PJ_DEF(void) pjsua_app_cfg_log_cb(int level, const char *buffer, int len)
{
	zassert(buffer != NULL);
	if(!len)
		return;
/*	static const char *ltexts[] = { "FATAL:", "ERROR:", " WARN:",
			      " INFO:", "DEBUG:", "TRACE:", "DETRC:"};*/
	/* Copy to terminal/file. */
	if (pj_log_get_decor() & PJ_LOG_HAS_COLOR)
	{
#if defined(PJ_TERM_HAS_COLOR) && PJ_TERM_HAS_COLOR != 0
		pj_term_set_color(pj_log_get_color(level));
#else
		PJ_UNUSED_ARG(level);
#endif
		switch (level)
		{
		case (6):
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_TRAP, "%s", buffer);
			break;
		case 4:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_DEBUG, "%s", buffer);
			break;
		case 3:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_INFO, "%s", buffer);
			break;
		case 5:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_NOTICE, "%s", buffer);
			break;
		case 2:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_WARNING, "%s", buffer);
			break;
		case 1:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_ERR, "%s", buffer);
			break;
		case 0:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_CRIT, "%s", buffer);
			break;
/*		case ZLOG_LEVEL_ALERT:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_ALERT + 1, "%s", buffer);
			break;
		case ZLOG_LEVEL_EMERG:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_EMERG + 1, "%s", buffer);
			break;*/
		default:
			break;
		}
		//printf("%s", buffer);
#if defined(PJ_TERM_HAS_COLOR) && PJ_TERM_HAS_COLOR != 0
		/* Set terminal to its default color */
		pj_term_set_color(pj_log_get_color(77));
#endif
	}
	else
	{
		switch (level)
		{
		case (6):
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_TRAP, "%s", buffer);
			break;
		case 4:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_DEBUG, "%s", buffer);
			break;
		case 3:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_INFO, "%s", buffer);
			break;
		case 5:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_NOTICE, "%s", buffer);
			break;
		case 2:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_WARNING, "%s", buffer);
			break;
		case 1:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_ERR, "%s", buffer);
			break;
		case 0:
			zlog_other(MODULE_PJAPP, ZLOG_LEVEL_CRIT, "%s", buffer);
			break;
		default:
			break;
		}
	}
}

int pjsua_app_cfg_log_file(pjsua_app_config *cfg, char *logfile)
{
	zassert(cfg != NULL);
	zassert(logfile != NULL);
	if (cfg->log_cfg.decor)
	{
		close(cfg->log_cfg.decor);
		cfg->log_cfg.decor = -1;
	}
	if (cfg->log_cfg.log_filename.slen)
	{
		free(cfg->log_cfg.log_filename.ptr);
		cfg->log_cfg.log_filename.ptr = NULL;
		cfg->log_cfg.log_filename.slen = 0;
	}
	cfg->log_cfg.log_filename.ptr = strdup(logfile);
	cfg->log_cfg.log_filename.slen = strlen(logfile);
	return OK;
}

int pjsua_app_cfg_log_level(pjsua_app_config *cfg, int level)
{
	zassert(cfg != NULL);
	if (level < 0 || level > 6)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting integer value 0-6 " "for --log-level"));
		return ERROR;
	}
	cfg->log_cfg.level = level;
	pj_log_set_level(level);
	return OK;
}

int pjsua_app_cfg_app_log_level(pjsua_app_config *cfg, int level)
{
	zassert(cfg != NULL);
	if (level < 0 || level > 6)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting integer value 0-6 " "for --log-level"));
		return ERROR;
	}
	cfg->log_cfg.console_level = level;
	return OK;
}

int pjsua_app_cfg_log_option(pjsua_app_config *cfg, int option, pj_bool_t enable)
{
	zassert(cfg != NULL);
	if (enable)
		cfg->log_cfg.log_file_flags |= option;
	else
		cfg->log_cfg.log_file_flags &= ~option;
	return OK;
}

int pjsua_app_cfg_log_color(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	if (enable)
		cfg->log_cfg.decor |= PJ_LOG_HAS_COLOR;
	else
		cfg->log_cfg.decor &= ~PJ_LOG_HAS_COLOR;
	return OK;
}

int pjsua_app_cfg_log_light_bg(pjsua_app_config *cfg, pj_bool_t enable)
{
	pj_log_set_color(1, PJ_TERM_COLOR_R);
	pj_log_set_color(2, PJ_TERM_COLOR_R | PJ_TERM_COLOR_G);
	pj_log_set_color(3, PJ_TERM_COLOR_B | PJ_TERM_COLOR_G);
	pj_log_set_color(4, 0);
	pj_log_set_color(5, 0);
	pj_log_set_color(77, 0);
	return OK;
}

int pjsua_app_cfg_null_audio(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->null_audio = enable;
	return OK;
}

int pjsua_app_cfg_clock_rate(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	if (lval < 8000 || lval > 192000)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting value between " "8000-192000 for conference clock rate"));
		return ERROR;
	}
	cfg->media_cfg.clock_rate = (unsigned) lval;
	return OK;
}

int pjsua_app_cfg_snd_clock_rate(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	if (lval < 8000 || lval > 192000)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting value between " "8000-192000 for sound device clock rate"));
		return ERROR;
	}
	cfg->media_cfg.snd_clock_rate = (unsigned) lval;
	return OK;
}

int pjsua_app_cfg_stereo(pjsua_app_config *cfg)
{
	zassert(cfg != NULL);
	cfg->media_cfg.channel_count = 2;
	return OK;
}

int pjsua_app_cfg_local_port(pjsua_app_config *cfg, pj_int32_t lval)
{
	zassert(cfg != NULL);
	if (lval < 0 || lval > 65535)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting integer value for " "--local-port"));
		return ERROR;
	}
	cfg->udp_cfg.port = (pj_uint16_t) lval;
	return OK;
}

int pjsua_app_cfg_public_address(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->udp_cfg.public_addr.slen)
	{
		free(cfg->udp_cfg.public_addr.ptr);
		cfg->udp_cfg.public_addr.ptr = NULL;
		cfg->udp_cfg.public_addr.slen = 0;
	}
	cfg->udp_cfg.public_addr.ptr = strdup(lval);
	cfg->udp_cfg.public_addr.slen = strlen(lval);

	if (cfg->rtp_cfg.public_addr.slen)
	{
		free(cfg->rtp_cfg.public_addr.ptr);
		cfg->rtp_cfg.public_addr.ptr = NULL;
		cfg->rtp_cfg.public_addr.slen = 0;
	}
	cfg->rtp_cfg.public_addr.ptr = strdup(lval);
	cfg->rtp_cfg.public_addr.slen = strlen(lval);

	return OK;
}

int pjsua_app_cfg_bound_address(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	if (cfg->udp_cfg.bound_addr.slen)
	{
		free(cfg->udp_cfg.bound_addr.ptr);
		cfg->udp_cfg.bound_addr.ptr = NULL;
		cfg->udp_cfg.bound_addr.slen = 0;
	}
	if(lval)
	{
		cfg->udp_cfg.bound_addr.ptr = strdup(lval);
		cfg->udp_cfg.bound_addr.slen = strlen(lval);
	}
	if (cfg->rtp_cfg.bound_addr.slen)
	{
		free(cfg->rtp_cfg.bound_addr.ptr);
		cfg->rtp_cfg.bound_addr.ptr = NULL;
		cfg->rtp_cfg.bound_addr.slen = 0;
	}
	if(lval)
	{
		cfg->rtp_cfg.bound_addr.ptr = strdup(lval);
		cfg->rtp_cfg.bound_addr.slen = strlen(lval);
	}
	return OK;
}

int pjsua_app_cfg_no_udp(pjsua_app_config *cfg)
{
	zassert(cfg != NULL);
	if (cfg->no_tcp && !cfg->use_tls)
	{
		PJ_LOG(1, (THIS_FILE,"Error: cannot disable both TCP and UDP"));
		return ERROR;
	}

	cfg->no_udp = PJ_TRUE;
	return OK;
}

int pjsua_app_cfg_no_tcp(pjsua_app_config *cfg)
{
	zassert(cfg != NULL);
	if (cfg->no_udp && !cfg->use_tls)
	{
		PJ_LOG(1, (THIS_FILE,"Error: cannot disable both TCP and UDP"));
		return ERROR;
	}

	cfg->no_tcp = PJ_TRUE;
	return OK;
}

int pjsua_app_cfg_norefersub(pjsua_app_config *cfg)
{
	zassert(cfg != NULL);
	cfg->no_refersub = PJ_TRUE;
	return OK;
}

int pjsua_app_cfg_proxy(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_sip_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in proxy argument", lval));
		return ERROR;
	}
	//cur_acc->proxy[cur_acc->proxy_cnt++] = pj_str(pj_optarg);
	if (cur_acc->proxy[cur_acc->proxy_cnt].slen)
	{
		free(cur_acc->proxy[cur_acc->proxy_cnt].ptr);
		cur_acc->proxy[cur_acc->proxy_cnt].ptr = NULL;
		cur_acc->proxy[cur_acc->proxy_cnt].slen = 0;
	}
	cur_acc->proxy[cur_acc->proxy_cnt].ptr = strdup(lval);
	cur_acc->proxy[cur_acc->proxy_cnt].slen = strlen(lval);
	cur_acc->proxy_cnt++;
	return OK;
}

int pjsua_app_cfg_outbound_proxy(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	//pjsua_acc_config *cur_acc;
	//cur_acc = cfg->acc_cfg;
	if (pjsua_verify_sip_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in proxy argument", lval));
		return ERROR;
	}
	//cur_acc->proxy[cur_acc->proxy_cnt++] = pj_str(pj_optarg);
	if (cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].slen)
	{
		free(cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].ptr);
		cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].ptr = NULL;
		cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].slen = 0;
	}
	cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].ptr = strdup(lval);
	cfg->cfg.outbound_proxy[cfg->cfg.outbound_proxy_cnt].slen = strlen(lval);
	cfg->cfg.outbound_proxy_cnt++;
	return OK;
}

int pjsua_app_cfg_registrar(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_sip_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in proxy argument", lval));
		return ERROR;
	}
	//cur_acc->proxy[cur_acc->proxy_cnt++] = pj_str(pj_optarg);
	if (cur_acc->reg_uri.slen)
	{
		free(cur_acc->reg_uri.ptr);
		cur_acc->reg_uri.ptr = NULL;
		cur_acc->reg_uri.slen = 0;
	}
	cur_acc->reg_uri.ptr = strdup(lval);
	cur_acc->reg_uri.slen = strlen(lval);
	return OK;
}

int pjsua_app_cfg_register_timeout(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (lval < 1 || lval > 3600)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid value for --reg-timeout " "(expecting 1-3600)"));
		return ERROR;
	}
	cur_acc->reg_timeout = (unsigned) lval;
	return OK;
}

int pjsua_app_cfg_publish(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->publish_enabled = enable;
	return OK;
}

int pjsua_app_cfg_mwi(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->mwi_enabled = enable;
	return OK;
}

int pjsua_app_cfg_100rel(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (enable)
	{
		cur_acc->require_100rel = PJSUA_100REL_MANDATORY;
		cfg->cfg.require_100rel = PJSUA_100REL_MANDATORY;
	}
	else
	{
		cur_acc->require_100rel = PJSUA_100REL_NOT_USED;
		cfg->cfg.require_100rel = PJSUA_100REL_NOT_USED;
	}
	return OK;
}

int pjsua_app_cfg_session_timer(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (lval < 0 || lval > 3)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: expecting integer value 0-3 for --use-timer"));
		return ERROR;
	}
	cur_acc->use_timer = (pjsua_sip_timer_use) lval;
	cfg->cfg.use_timer = (pjsua_sip_timer_use) lval;
	return OK;
}

int pjsua_app_cfg_session_timer_expiration(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (lval < 90)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid value for --timer-se " "(expecting higher than 90)"));
		return ERROR;
	}
	cur_acc->timer_setting.sess_expires = (pjsua_sip_timer_use) lval;
	cfg->cfg.timer_setting.sess_expires = (pjsua_sip_timer_use) lval;
	return OK;
}

int pjsua_app_cfg_session_timer_expiration_min(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (lval < 90)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid value for --timer-se " "(expecting higher than 90)"));
		return ERROR;
	}
	cur_acc->timer_setting.min_se = (pjsua_sip_timer_use) lval;
	cfg->cfg.timer_setting.min_se = (pjsua_sip_timer_use) lval;
	return OK;
}

int pjsua_app_cfg_outbound_reg_id(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	//cur_acc->proxy[cur_acc->proxy_cnt++] = pj_str(pj_optarg);
	if (cur_acc->rfc5626_reg_id.slen)
	{
		free(cur_acc->rfc5626_reg_id.ptr);
		cur_acc->rfc5626_reg_id.ptr = NULL;
		cur_acc->rfc5626_reg_id.slen = 0;
	}
	cur_acc->rfc5626_reg_id.ptr = strdup(lval);
	cur_acc->rfc5626_reg_id.slen = strlen(lval);
	return OK;
}

int pjsua_app_cfg_ims(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->auth_pref.initial_auth = enable;
	return OK;
}

int pjsua_app_cfg_url_id(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in local id argument", lval));
		return ERROR;
	}
	if (cur_acc->id.slen)
	{
		free(cur_acc->id.ptr);
		cur_acc->id.ptr = NULL;
		cur_acc->id.slen = 0;
	}
	cur_acc->id.ptr = strdup(lval);
	cur_acc->id.slen = strlen(lval);
	return OK;
}

int pjsua_app_cfg_contact(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in contact argument", lval));
		return ERROR;
	}
	if (cur_acc->force_contact.slen)
	{
		free(cur_acc->force_contact.ptr);
		cur_acc->force_contact.ptr = NULL;
		cur_acc->force_contact.slen = 0;
	}
	cur_acc->force_contact.ptr = strdup(lval);
	cur_acc->force_contact.slen = strlen(lval);
	return OK;
}

int pjsua_app_cfg_contact_params(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in contact_params argument", lval));
		return ERROR;
	}
	if (cur_acc->contact_params.slen)
	{
		free(cur_acc->contact_params.ptr);
		cur_acc->contact_params.ptr = NULL;
		cur_acc->contact_params.slen = 0;
	}
	cur_acc->contact_params.ptr = strdup(lval);
	cur_acc->contact_params.slen = strlen(lval);
	return OK;
}

int pjsua_app_cfg_contact_uri_params(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (pjsua_verify_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid SIP URL '%s' " "in contact_uri_params argument", lval));
		return ERROR;
	}
	if (cur_acc->contact_uri_params.slen)
	{
		free(cur_acc->contact_uri_params.ptr);
		cur_acc->contact_uri_params.ptr = NULL;
		cur_acc->contact_uri_params.slen = 0;
	}
	cur_acc->contact_uri_params.ptr = strdup(lval);
	cur_acc->contact_uri_params.slen = strlen(lval);
	return OK;
}

int pjsua_app_cfg_update_nat(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->allow_contact_rewrite = enable;
	return OK;
}

int pjsua_app_cfg_stun(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (enable)
	{
		cur_acc->sip_stun_use = PJSUA_STUN_USE_DEFAULT;
		cur_acc->media_stun_use = PJSUA_STUN_USE_DEFAULT;
	}
	else
	{
		cur_acc->sip_stun_use = PJSUA_STUN_USE_DISABLED;
		cur_acc->media_stun_use = PJSUA_STUN_USE_DISABLED;
	}
	return OK;
}

int pjsua_app_cfg_compact_form(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	extern pj_bool_t pjsip_include_allow_hdr_in_dlg;
	extern pj_bool_t pjmedia_add_rtpmap_for_static_pt;
	pjsip_cfg()->endpt.use_compact_form = enable;
	/* do not transmit Allow header */
	pjsip_include_allow_hdr_in_dlg = PJ_FALSE;
	/* Do not include rtpmap for static payload types (<96) */
	pjmedia_add_rtpmap_for_static_pt = PJ_FALSE;
	return OK;
}

int pjsua_app_cfg_accept_redirect(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	cfg->redir_op = lval;
	return OK;
}

int pjsua_app_cfg_no_force_lr(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->cfg.force_lr = enable;
	return OK;
}

int pjsua_app_cfg_next_account(pjsua_app_config *cfg)
{
	zassert(cfg != NULL);
	cfg->acc_cnt++;
	return OK;
}

int pjsua_app_cfg_username(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (cur_acc->cred_info[cur_acc->cred_count].username.slen)
	{
		free(cur_acc->cred_info[cur_acc->cred_count].username.ptr);
		cur_acc->cred_info[cur_acc->cred_count].username.ptr = NULL;
		cur_acc->cred_info[cur_acc->cred_count].username.slen = 0;
	}
	cur_acc->cred_info[cur_acc->cred_count].username.ptr = strdup(lval);
	cur_acc->cred_info[cur_acc->cred_count].username.slen = strlen(lval);

/*	if (cur_acc->cred_info[cur_acc->cred_count].scheme.slen == 0)
	{
		cur_acc->cred_info[cur_acc->cred_count].scheme.ptr = strdup("digest");
		cur_acc->cred_info[cur_acc->cred_count].scheme.slen = strlen("digest");
	}*/
	return OK;
}

int pjsua_app_cfg_scheme(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (cur_acc->cred_info[cur_acc->cred_count].scheme.slen == 0)
	{
		free(cur_acc->cred_info[cur_acc->cred_count].scheme.ptr);
		cur_acc->cred_info[cur_acc->cred_count].scheme.ptr = NULL;
		cur_acc->cred_info[cur_acc->cred_count].scheme.slen = 0;
	}
	cur_acc->cred_info[cur_acc->cred_count].scheme.ptr = strdup("digest");
	cur_acc->cred_info[cur_acc->cred_count].scheme.slen = strlen("digest");
	return OK;
}

int pjsua_app_cfg_realm(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (cur_acc->cred_info[cur_acc->cred_count].realm.slen)
	{
		free(cur_acc->cred_info[cur_acc->cred_count].realm.ptr);
		cur_acc->cred_info[cur_acc->cred_count].realm.ptr = NULL;
		cur_acc->cred_info[cur_acc->cred_count].realm.slen = 0;
	}
	cur_acc->cred_info[cur_acc->cred_count].realm.ptr = strdup(lval);
	cur_acc->cred_info[cur_acc->cred_count].realm.slen = strlen(lval);
	return OK;
}

int pjsua_app_cfg_password(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (cur_acc->cred_info[cur_acc->cred_count].data.slen)
	{
		free(cur_acc->cred_info[cur_acc->cred_count].data.ptr);
		cur_acc->cred_info[cur_acc->cred_count].data.ptr = NULL;
		cur_acc->cred_info[cur_acc->cred_count].data.slen = 0;
	}
	cur_acc->cred_info[cur_acc->cred_count].data_type =
			PJSIP_CRED_DATA_PLAIN_PASSWD;
	cur_acc->cred_info[cur_acc->cred_count].data.ptr = strdup(lval);
	cur_acc->cred_info[cur_acc->cred_count].data.slen = strlen(lval);
#if 0//PJSIP_HAS_DIGEST_AKA_AUTH
	cur_acc->cred_info[cur_acc->cred_count].data_type |= PJSIP_CRED_DATA_EXT_AKA;
	if(cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.slen)
	{
		free(cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.ptr);
		cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.ptr = NULL;
		cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.slen = 0;
	}
	cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.ptr = strdup(lval);
	cur_acc->cred_info[cur_acc->cred_count].ext.aka.k.slen = strlen(lval);
	//cur_acc->cred_info[cur_acc->cred_count].ext.aka.k = pj_str(pj_optarg);
	cur_acc->cred_info[cur_acc->cred_count].ext.aka.cb = &pjsip_auth_create_aka_response;
#endif
	return OK;
}

int pjsua_app_cfg_reg_retry_interval(pjsua_app_config *cfg, int interval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->reg_retry_interval = (unsigned) interval;
	return OK;
}

int pjsua_app_cfg_reg_use_proxy(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (value > 4)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid --reg-use-proxy value '%d'",value));
		return ERROR;
	}
	cur_acc->reg_use_proxy = (unsigned) value;
	return OK;
}

int pjsua_app_cfg_credential(pjsua_app_config *cfg, int credential)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cur_acc->cred_count++;
	return OK;
}

int pjsua_app_cfg_nameserver(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);

	//pjsua_acc_config *cur_acc;
	if (cfg->cfg.nameserver_count > PJ_ARRAY_SIZE(cfg->cfg.nameserver))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
		return ERROR;
	}
	//cfg->cfg.nameserver[cfg->cfg.nameserver_count++] = pj_str(pj_optarg);
	if (cfg->cfg.nameserver[cfg->cfg.nameserver_count].slen)
	{
		free(cfg->cfg.nameserver[cfg->cfg.nameserver_count].ptr);
		cfg->cfg.nameserver[cfg->cfg.nameserver_count].ptr = NULL;
		cfg->cfg.nameserver[cfg->cfg.nameserver_count].slen = 0;
	}
	cfg->cfg.nameserver[cfg->cfg.nameserver_count].ptr = strdup(lval);
	cfg->cfg.nameserver[cfg->cfg.nameserver_count].slen = strlen(lval);
	cfg->cfg.nameserver_count++;
	return OK;
}

int pjsua_app_cfg_stunserver(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	//pjsua_acc_config *cur_acc;
	if (cfg->cfg.stun_srv_cnt == PJ_ARRAY_SIZE(cfg->cfg.stun_srv))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many STUN servers"));
		return ERROR;
	}

	//cfg->cfg.nameserver[cfg->cfg.nameserver_count++] = pj_str(pj_optarg);
	if (cfg->cfg.stun_host.slen)
	{
		free(cfg->cfg.stun_host.ptr);
		cfg->cfg.stun_host.ptr = NULL;
		cfg->cfg.stun_host.slen = 0;
	}
	cfg->cfg.stun_host.ptr = strdup(lval);
	cfg->cfg.stun_host.slen = strlen(lval);
	if (cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].slen)
	{
		free(cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].ptr);
		cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].ptr = NULL;
		cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].slen = 0;
	}
	cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].ptr = strdup(lval);
	cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt].slen = strlen(lval);
	cfg->cfg.stun_srv_cnt++;
	return OK;
}

int pjsua_app_cfg_buddy_list(pjsua_app_config *cfg, char * lval)
{
	//pjsua_acc_config *cur_acc;
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (pjsua_verify_url(lval) != 0)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid URL '%s' in " "--add-buddy option", lval));
		return ERROR;
	}
	if (cfg->buddy_cnt == PJ_ARRAY_SIZE(cfg->buddy_cfg))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	if (cfg->buddy_cfg[cfg->buddy_cnt].uri.slen)
	{
		free(cfg->buddy_cfg[cfg->buddy_cnt].uri.ptr);
		cfg->buddy_cfg[cfg->buddy_cnt].uri.ptr = NULL;
		cfg->buddy_cfg[cfg->buddy_cnt].uri.slen = 0;
	}
	cfg->buddy_cfg[cfg->buddy_cnt].uri.ptr = strdup(lval);
	cfg->buddy_cfg[cfg->buddy_cnt].uri.slen = strlen(lval);
	//cfg->buddy_cfg[cfg->buddy_cnt].uri = pj_str(pj_optarg);
	cfg->buddy_cnt++;
	return OK;
}

int pjsua_app_cfg_auto_play(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	//pjsua_acc_config *cur_acc;
	//cur_acc = cfg->acc_cfg;
	cfg->auto_play = enable;
	return OK;
}

int pjsua_app_cfg_auto_play_hangup(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	//pjsua_acc_config *cur_acc;
	//cur_acc = cfg->acc_cfg;
	cfg->auto_play_hangup = enable;
	return OK;
}

int pjsua_app_cfg_auto_rec(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->auto_rec = enable;
	return OK;
}

int pjsua_app_cfg_auto_loop(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->auto_loop = enable;
	return OK;
}

int pjsua_app_cfg_auto_config(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->auto_conf = enable;
	return OK;
}

int pjsua_app_cfg_play_file(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->wav_files[cfg->wav_count].slen)
	{
		free(cfg->wav_files[cfg->wav_count].ptr);
		cfg->wav_files[cfg->wav_count].ptr = NULL;
		cfg->wav_files[cfg->wav_count].slen = 0;
	}
	cfg->wav_files[cfg->wav_count].ptr = strdup(lval);
	cfg->wav_files[cfg->wav_count].slen = strlen(lval);
	cfg->wav_count++;
	return OK;
}

int pjsua_app_cfg_play_tone(pjsua_app_config *cfg, int f1, int f2, int on, int off)
{
	zassert(cfg != NULL);
	cfg->tones[cfg->tone_count].freq1 = (zpl_int16) f1;
	cfg->tones[cfg->tone_count].freq2 = (zpl_int16) f2;
	cfg->tones[cfg->tone_count].on_msec = (zpl_int16) on;
	cfg->tones[cfg->tone_count].off_msec = (zpl_int16) off;
	++cfg->tone_count;
	return OK;
}

int pjsua_app_cfg_rec_file(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->rec_file.slen)
	{
		free(cfg->rec_file.ptr);
		cfg->rec_file.ptr = NULL;
		cfg->rec_file.slen = 0;
	}
	cfg->auto_rec = PJ_TRUE;
	cfg->rec_file.ptr = strdup(lval);
	cfg->rec_file.slen = strlen(lval);
	return OK;
}

int pjsua_app_cfg_ice_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	//pjsua_acc_config *acfg = &cfg->acc_cfg[i];
	cfg->media_cfg.enable_ice = cur_acc->ice_cfg.enable_ice = enable;
	if(enable)
	{
		cur_acc->ice_cfg_use = PJSUA_ICE_CONFIG_USE_CUSTOM;
	}
	return OK;
}

int pjsua_app_cfg_regular_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->media_cfg.ice_opt.aggressive = cur_acc->ice_cfg.ice_opt.aggressive =
			enable;
	return OK;
}

int pjsua_app_cfg_turn_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->media_cfg.enable_turn = cur_acc->turn_cfg.enable_turn = enable;
	if(enable)
	{
		cur_acc->turn_cfg_use = PJSUA_TURN_CONFIG_USE_CUSTOM;
	}
	return OK;
}

int pjsua_app_cfg_ice_max_hosts(pjsua_app_config *cfg, int maxnum)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->media_cfg.ice_max_host_cands = cur_acc->ice_cfg.ice_max_host_cands =
			maxnum;
	return OK;
}

int pjsua_app_cfg_ice_nortcp_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->media_cfg.ice_no_rtcp = cur_acc->ice_cfg.ice_no_rtcp = enable;
	return OK;
}

int pjsua_app_cfg_turn_srv(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	if (cfg->media_cfg.turn_server.slen)
	{
		free(cfg->media_cfg.turn_server.ptr);
		cfg->media_cfg.turn_server.ptr = NULL;
		cfg->media_cfg.turn_server.slen = 0;
	}
	cfg->media_cfg.turn_server.ptr = strdup(lval);
	cfg->media_cfg.turn_server.slen = strlen(lval);

	if (cur_acc->turn_cfg.turn_server.slen)
	{
		free(cur_acc->turn_cfg.turn_server.ptr);
		cur_acc->turn_cfg.turn_server.ptr = NULL;
		cur_acc->turn_cfg.turn_server.slen = 0;
	}
	cur_acc->turn_cfg.turn_server.ptr = strdup(lval);
	cur_acc->turn_cfg.turn_server.slen = strlen(lval);
	return OK;
}

int pjsua_app_cfg_turn_tcp(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->media_cfg.turn_conn_type = cur_acc->turn_cfg.turn_conn_type =
			PJ_TURN_TP_TCP;
	return OK;
}

int pjsua_app_cfg_turn_user(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;

	pj_str_clr(&cfg->media_cfg.turn_auth_cred.data.static_cred.realm);
	pj_str_clr(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.realm);

	pj_str_set(&cfg->media_cfg.turn_auth_cred.data.static_cred.realm, "*");
	pj_str_set(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.realm, "*");

	pj_str_clr(&cfg->media_cfg.turn_auth_cred.data.static_cred.username);
	pj_str_clr(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.username);

	pj_str_set(&cfg->media_cfg.turn_auth_cred.data.static_cred.username, lval);
	pj_str_set(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.username,
			lval);

	cfg->media_cfg.turn_auth_cred.type = cur_acc->turn_cfg.turn_auth_cred.type =
			PJ_STUN_AUTH_CRED_STATIC;
	return OK;
}

int pjsua_app_cfg_turn_password(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;

	/*    pj_str_clr(&cfg->media_cfg.turn_auth_cred.data.static_cred.realm);
	 pj_str_clr(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.realm);

	 pj_str_set(&cfg->media_cfg.turn_auth_cred.data.static_cred.realm, "*");
	 pj_str_set(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.realm, "*");*/

	pj_str_clr(&cfg->media_cfg.turn_auth_cred.data.static_cred.data);
	pj_str_clr(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data);

	pj_str_set(&cfg->media_cfg.turn_auth_cred.data.static_cred.data, lval);
	pj_str_set(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data, lval);

	cfg->media_cfg.turn_auth_cred.data.static_cred.data_type =
			cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data_type =
					PJ_STUN_PASSWD_PLAIN;
	return OK;
}

int pjsua_app_cfg_rtcp_mux(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;

	cur_acc->enable_rtcp_mux = PJ_TRUE;
	return OK;
}

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
int pjsua_app_cfg_srtp_enable(pjsua_app_config *cfg, pj_uint32_t type)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->cfg.use_srtp = type;    //my_atoi(pj_optarg);
	if (cfg->cfg.use_srtp > 3)
	{
		PJ_LOG(1, (THIS_FILE, "Invalid value for --use-srtp option"));
		return ERROR;
	}
	if ((int) cfg->cfg.use_srtp == 3)
	{
		/* SRTP optional mode with duplicated media offer */
		cfg->cfg.use_srtp = PJMEDIA_SRTP_OPTIONAL;
		cfg->cfg.srtp_optional_dup_offer = PJ_TRUE;
		cur_acc->srtp_optional_dup_offer = PJ_TRUE;
	}
	cur_acc->use_srtp = cfg->cfg.use_srtp;
	return OK;
}

int pjsua_app_cfg_srtp_secure(pjsua_app_config *cfg, pj_uint32_t type)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->cfg.srtp_secure_signaling = type;    //my_atoi(pj_optarg);
	if (/*!pj_isdigit(*pj_optarg) ||*/
	cfg->cfg.srtp_secure_signaling > 2)
	{
		PJ_LOG(1, (THIS_FILE, "Invalid value for --srtp-secure option"));
		return ERROR;
	}
	cur_acc->srtp_secure_signaling = cfg->cfg.srtp_secure_signaling;
	return OK;
}

int pjsua_app_cfg_srtp_keying(pjsua_app_config *cfg, pj_uint32_t type)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = cfg->acc_cfg;
	cfg->srtp_keying = type;    //my_atoi(pj_optarg);
	if (/*!pj_isdigit(*pj_optarg) || */cfg->srtp_keying < 0
			|| cfg->srtp_keying > 1)
	{
		PJ_LOG(1, (THIS_FILE, "Invalid value for --srtp-keying option"));
		return ERROR;
	}
	/* Set SRTP keying to use DTLS over SDES */
	if (cfg->srtp_keying == 1)
	{
		pjsua_srtp_opt *srtp_opt = &cfg->cfg.srtp_opt;
		srtp_opt->keying_count = 2;
		srtp_opt->keying[0] = PJMEDIA_SRTP_KEYING_DTLS_SRTP;
		srtp_opt->keying[1] = PJMEDIA_SRTP_KEYING_SDES;

		cur_acc->srtp_opt.keying_count = 2;
		cur_acc->srtp_opt.keying[0] = PJMEDIA_SRTP_KEYING_DTLS_SRTP;
		cur_acc->srtp_opt.keying[1] = PJMEDIA_SRTP_KEYING_SDES;
	}
	return OK;
}
#endif

int pjsua_app_cfg_rtp_port(pjsua_app_config *cfg, int port)
{
	zassert(cfg != NULL);
	if (port == 0)
	{
		enum
		{
			START_PORT = 4000
		};
		unsigned range;

		range = (65535 - START_PORT - PJSUA_MAX_CALLS * 2);
		cfg->rtp_cfg.port = START_PORT + ((pj_rand() % range) & 0xFFFE);
	}
	else
	{
		if (port < 1 || port > 65535)
		{
			PJ_LOG(1,
					(THIS_FILE, "Error: rtp-port argument value " "(expecting 1-65535"));
			return ERROR;
		}
		cfg->rtp_cfg.port = port;
	}
	return OK;
}

int pjsua_app_cfg_dis_codec(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
/*	pj_str_clr(&cfg->codec_dis[cfg->codec_dis_cnt]);
	pj_str_set(&cfg->codec_dis[cfg->codec_dis_cnt], lval);
	cfg->codec_dis_cnt++;*/
	cfg->codec_dis[cfg->codec_dis_cnt] = pj_str(lval);
	cfg->codec_dis_cnt++;
	//cfg->codec_dis[cfg->codec_dis_cnt++] = inde;
	return OK;
}

int pjsua_app_cfg_add_codec(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
/*	pj_str_clr(&cfg->codec_arg[cfg->codec_cnt]);
	pj_str_set(&cfg->codec_arg[cfg->codec_cnt], lval);
	cfg->codec_cnt++;
*/
	cfg->codec_arg[cfg->codec_cnt] = pj_str(lval);
	cfg->codec_cnt++;
	//cfg->codec_arg[cfg->codec_cnt++] = inde;
	return OK;
}

int pjsua_app_cfg_duration(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->duration = value;
	return OK;
}

int pjsua_app_cfg_thread_cnt(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value > 128)
	{
		PJ_LOG(1, (THIS_FILE, "Error: invalid --thread-cnt option"));
		return ERROR;
	}
	cfg->cfg.thread_cnt = value;
	return OK;
}

int pjsua_app_cfg_ptime(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value < 10 || value > 1000)
	{
		PJ_LOG(1, (THIS_FILE, "Error: invalid --ptime option"));
		return ERROR;
	}
	cfg->media_cfg.ptime = value;
	return OK;
}

int pjsua_app_cfg_novad(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->media_cfg.no_vad = enable;
	return OK;
}

int pjsua_app_cfg_ec_tial(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value > 1000)
	{
		PJ_LOG(1,
				(THIS_FILE, "I think the ec-tail length setting " "is too big"));
		return ERROR;
	}
	cfg->media_cfg.ec_tail_len = value;
	return OK;
}

int pjsua_app_cfg_ec_options(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->media_cfg.ec_options = value;
	return OK;
}

int pjsua_app_cfg_quality(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value > 10)
	{
		PJ_LOG(1, (THIS_FILE, "Error: invalid --quality (expecting 0-10"));
		return ERROR;
	}
	cfg->media_cfg.quality = value;
	return OK;
}

int pjsua_app_cfg_ilbc_mode(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value != 20 && value != 30)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid --ilbc-mode (expecting 20 or 30"));
		return ERROR;
	}
	cfg->media_cfg.ilbc_mode = value;
	return OK;
}

int pjsua_app_cfg_rx_drop_pct(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (cfg->media_cfg.rx_drop_pct > 100)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid --rx-drop-pct (expecting <= 100"));
		return ERROR;
	}
	cfg->media_cfg.rx_drop_pct = value;
	return OK;
}

int pjsua_app_cfg_tx_drop_pct(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (cfg->media_cfg.tx_drop_pct > 100)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid --tx-drop-pct (expecting <= 100"));
		return ERROR;
	}
	cfg->media_cfg.tx_drop_pct = value;
	return OK;
}

int pjsua_app_cfg_auto_answer(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value < 100 || value > 699)
	{
		PJ_LOG(1,
				(THIS_FILE, "Error: invalid code in --auto-answer " "(expecting 100-699"));
		return ERROR;
	}
	cfg->auto_answer = value;
	return OK;
}

int pjsua_app_cfg_max_calls(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value < 1 || value > PJSUA_MAX_CALLS)
	{
		PJ_LOG(1,
				(THIS_FILE,"Error: maximum call setting exceeds " "compile time limit (PJSUA_MAX_CALLS=%d)", PJSUA_MAX_CALLS));
		return ERROR;
	}
	cfg->cfg.max_calls = value;
	return OK;
}

#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
int pjsua_app_cfg_tls_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->use_tls = enable;
	return OK;
}

int pjsua_app_cfg_tls_ca_file(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.ca_list_file);
	pj_str_set(&cfg->udp_cfg.tls_setting.ca_list_file, lval);
	return OK;
}

int pjsua_app_cfg_tls_cert_file(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.cert_file);
	pj_str_set(&cfg->udp_cfg.tls_setting.cert_file, lval);
	return OK;
}

int pjsua_app_cfg_tls_priv_file(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.privkey_file);
	pj_str_set(&cfg->udp_cfg.tls_setting.privkey_file, lval);
	return OK;
}

int pjsua_app_cfg_tls_password(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.password);
	pj_str_set(&cfg->udp_cfg.tls_setting.password, lval);
	return OK;
}

int pjsua_app_cfg_tls_verify_server(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->udp_cfg.tls_setting.verify_server = enable;
	return OK;
}

int pjsua_app_cfg_tls_verify_client(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->udp_cfg.tls_setting.verify_client = enable;
	cfg->udp_cfg.tls_setting.require_client_cert = enable;
	return OK;
}

int pjsua_app_cfg_tls_neg_timeout(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->udp_cfg.tls_setting.timeout.sec = value;
	return OK;
}

int pjsua_app_cfg_tls_cipher(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_ssl_cipher cipher;

	if (pj_ansi_strnicmp(lval, "0x", 2) == 0)
	{
		pj_str_t cipher_st = pj_str(lval + 2);
		cipher = pj_strtoul2(&cipher_st, NULL, 16);
	}
	else
	{
		cipher = atoi(lval);
	}

	if (pj_ssl_cipher_is_supported(cipher))
	{
		static pj_ssl_cipher tls_ciphers[PJ_SSL_SOCK_MAX_CIPHERS];

		tls_ciphers[cfg->udp_cfg.tls_setting.ciphers_num++] = cipher;
		cfg->udp_cfg.tls_setting.ciphers = tls_ciphers;
	}
	else
	{
		pj_ssl_cipher ciphers[PJ_SSL_SOCK_MAX_CIPHERS];
		pj_uint32_t j = 0, ciphers_cnt = 0;

		ciphers_cnt = PJ_ARRAY_SIZE(ciphers);
		pj_ssl_cipher_get_availables(ciphers, &ciphers_cnt);

		PJ_LOG(1,
				(THIS_FILE, "Cipher \"%s\" is not supported by " "TLS/SSL backend.", lval));
		printf("Available TLS/SSL ciphers (%d):\n", ciphers_cnt);
		for (j = 0; j < ciphers_cnt; ++j)
		printf("- 0x%06X: %s\n", ciphers[j],
				pj_ssl_cipher_name(ciphers[j]));
		return -1;
	}
	return OK;
}
#endif

int pjsua_app_cfg_capture_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->capture_dev = value;
	return OK;
}

int pjsua_app_cfg_capture_lat(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->capture_lat = value;
	return OK;
}

int pjsua_app_cfg_playback_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->capture_dev = value;
	return OK;
}

int pjsua_app_cfg_playback_lat(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->playback_lat = value;
	return OK;
}

int pjsua_app_cfg_snd_auto_close(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->media_cfg.snd_auto_close_time = value;
	return OK;
}

int pjsua_app_cfg_no_tones(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->no_tones = enable;
	return OK;
}

int pjsua_app_cfg_jb_max_size(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->media_cfg.jb_max = value;
	return OK;
}

#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
int pjsua_app_cfg_ipv6_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->ipv6 = enable;
	return OK;
}
#endif

int pjsua_app_cfg_qos_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->enable_qos = enable;
	/* Set RTP traffic type to Voice */
	cfg->rtp_cfg.qos_type = PJ_QOS_TYPE_VOICE;
	/* Directly apply DSCP value to SIP traffic. Say lets
	 * set it to CS3 (DSCP 011000). Note that this will not
	 * work on all platforms.
	 */
	cfg->udp_cfg.qos_params.flags = PJ_QOS_PARAM_HAS_DSCP;
	cfg->udp_cfg.qos_params.dscp_val = 0x18;
	return OK;
}

int pjsua_app_cfg_video_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->vid.vid_cnt = 1;
	cfg->vid.in_auto_show = PJ_TRUE;
	cfg->vid.out_auto_transmit = PJ_TRUE;
	return OK;
}

int pjsua_app_cfg_extra_audio(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->aud_cnt++;
	return OK;
}

int pjsua_app_cfg_vcapture_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc;
	cur_acc = cfg->acc_cfg;
	cur_acc->vid_cap_dev = cfg->vid.vcapture_dev = value;
	return OK;
}

int pjsua_app_cfg_vrender_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc;
	cur_acc = cfg->acc_cfg;
	cur_acc->vid_rend_dev = cfg->vid.vrender_dev = value;
	return OK;
}

int pjsua_app_cfg_play_avi(pjsua_app_config *cfg, char * lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc;
	cur_acc = cfg->acc_cfg;

	if (cfg->avi_cnt >= PJSUA_APP_MAX_AVI)
	{
		PJ_LOG(1, (THIS_FILE, "Too many AVIs"));
		return ERROR;
	}

	//cur_acc->vid_rend_dev = cfg->vid.vrender_dev = value;

	pj_str_clr(&cfg->avi[cfg->avi_cnt].path);
	pj_str_set(&cfg->avi[cfg->avi_cnt].path, lval);
	cfg->avi_cnt++;
	return OK;
}

int pjsua_app_cfg_auto_play_avi(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->avi_auto_play = enable;
	return OK;
}

int pjsua_app_cfg_cli_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->use_cli = enable;
	return OK;
}

int pjsua_app_cfg_cli_telnet_port(pjsua_app_config *cfg, int port)
{
	zassert(cfg != NULL);
	cfg->cli_cfg.telnet_cfg.port = port;
	cfg->cli_cfg.cli_fe |= CLI_FE_TELNET;
	return OK;
}

int pjsua_app_cfg_cli_console(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	if (enable)
		cfg->cli_cfg.cli_fe &= (~CLI_FE_CONSOLE);
	else
		cfg->cli_cfg.cli_fe |= (CLI_FE_CONSOLE);
	return OK;
}








/*
 #ifdef _IONBF
 case OPT_STDOUT_NO_BUF:
 setvbuf(stdout, NULL, _IONBF, 0);
 cfg->log_cfg.cb = &log_writer_nobuf;
 break;
 #endif
 */

/* Set default config. */
void pjsua_app_cfg_default_config(pjsua_app_config *cfg)
{
	char tmp[80];
	unsigned i;
	zassert(cfg != NULL);
	pjsua_config_default(&cfg->cfg);
	pj_ansi_sprintf(tmp, "PJSUA v%s %s", pj_get_version(),
			pj_get_sys_info()->info.ptr);
	pj_strdup2_with_null(cfg->pool, &cfg->cfg.user_agent, tmp);

	pjsua_logging_config_default(&cfg->log_cfg);
	cfg->log_cfg.cb = pjsua_app_cfg_log_cb;
	pj_log_set_decor(PJ_LOG_HAS_NEWLINE|PJ_LOG_HAS_CR|PJ_LOG_HAS_INDENT|PJ_LOG_HAS_THREAD_SWC|
			PJ_LOG_HAS_SENDER|PJ_LOG_HAS_THREAD_ID);

	pjsua_media_config_default(&cfg->media_cfg);
	cfg->media_cfg.clock_rate = PJSIP_DEFAULT_CLOCK_RATE;
    cfg->media_cfg.snd_clock_rate = PJSIP_DEFAULT_CLOCK_RATE;


	pjsua_transport_config_default(&cfg->udp_cfg);
	cfg->udp_cfg.port = 5060;
	pjsua_transport_config_default(&cfg->rtp_cfg);
	cfg->rtp_cfg.port = 40000;
	cfg->redir_op = PJSIP_REDIRECT_ACCEPT_REPLACE;
	cfg->duration = PJSUA_APP_NO_LIMIT_DURATION;
	cfg->wav_id = PJSUA_INVALID_ID;
	cfg->rec_id = PJSUA_INVALID_ID;
	cfg->wav_port = PJSUA_INVALID_ID;
	cfg->rec_port = PJSUA_INVALID_ID;
	cfg->speaker_level = 1.0;
	cfg->mic_level = 1.5;
	cfg->capture_dev = PJSUA_INVALID_ID;
	cfg->playback_dev = PJSUA_INVALID_ID;
	cfg->capture_lat = PJMEDIA_SND_DEFAULT_REC_LATENCY;
	cfg->playback_lat = PJMEDIA_SND_DEFAULT_PLAY_LATENCY;
	cfg->ringback_slot = PJSUA_INVALID_ID;
	cfg->ring_slot = PJSUA_INVALID_ID;

	for (i = 0; i < PJ_ARRAY_SIZE(cfg->acc_cfg); ++i)
		pjsua_acc_config_default(&cfg->acc_cfg[i]);

	for (i = 0; i < PJ_ARRAY_SIZE(cfg->buddy_cfg); ++i)
		pjsua_buddy_config_default(&cfg->buddy_cfg[i]);

	cfg->vid.vcapture_dev = PJMEDIA_VID_DEFAULT_CAPTURE_DEV;
	cfg->vid.vrender_dev = PJMEDIA_VID_DEFAULT_RENDER_DEV;
	cfg->aud_cnt = 0;

	cfg->avi_def_idx = PJSUA_INVALID_ID;

	cfg->use_cli = zpl_true;
	//cfg->cli_cfg.cli_fe = CLI_FE_CONSOLE;
	//cfg->cli_cfg.telnet_cfg.port = 0;
	cfg->cli_cfg.cli_fe = CLI_FE_TELNET;
	cfg->cli_cfg.telnet_cfg.port = 2323;
    /* Add UDP transport. */
	cfg->codec_dis_cnt = 0;
	cfg->codec_cnt = 0;
/*
	cfg->codec_dis_cnt = 0;
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("speex/8000");
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("speex/16000");
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("speex/32000");
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("iLBC/8000");

	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("PCMA");
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("GSM");
	cfg->codec_dis[cfg->codec_dis_cnt++] = pj_str("G722");


	cfg->codec_cnt = 0;
	cfg->codec_arg[cfg->codec_cnt++] = pj_str("PCMU");
	//cfg->codec_arg[cfg->codec_cnt++] = pj_str("PCMA");
*/
}

int pjsua_app_cfg_log_config(pjsua_app_config *cfg)
{
	pjsua_app_cfg_log_level(cfg, 6);
	pjsua_app_cfg_app_log_level(cfg, 6);
	pj_log_set_log_func(pjsua_app_cfg_log_cb);
	return OK;
}
int pjsua_app_cfg_load_config(pjsua_app_config *cfg)
{
	int i = 0;
	char buf[128];
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = &cfg->acc_cfg;
	zassert(_pjapp_cfg != NULL);

	if(_pjapp_cfg->mutex)
		os_mutex_lock(_pjapp_cfg->mutex, OS_WAIT_FOREVER);

	pjsua_app_cfg_default_config(cfg);

	//pjsua_app_cfg_debug_level_set_api(_pjapp_cfg->debug_level);
	//pjsua_app_cfg_debug_detail_set_api(_pjapp_cfg->debug_detail);


	cur_acc->cred_count = 0;
	if(strlen(_pjapp_cfg->sip_user.sip_user))
	{
		pjsua_app_cfg_username(cfg, _pjapp_cfg->sip_user.sip_user);//Set authentication username
		if(strlen(_pjapp_cfg->sip_user.sip_password))
		{
			pjsua_app_cfg_password(cfg, _pjapp_cfg->sip_user.sip_password);//Set authentication password
		}
		cfg->acc_cnt = 1;
	}
	if(strlen(_pjapp_cfg->sip_user_sec.sip_user))
	{
		pjsua_app_cfg_username(cfg, _pjapp_cfg->sip_user_sec.sip_user);//Set authentication username

		if(strlen(_pjapp_cfg->sip_user_sec.sip_password))
		{
			pjsua_app_cfg_password(cfg, _pjapp_cfg->sip_user_sec.sip_password);//Set authentication password
		}
		cfg->acc_cnt = 2;
	}

	pjsua_app_cfg_scheme(cfg, "digest");

	if(strlen(_pjapp_cfg->sip_realm))
	{
		pjsua_app_cfg_realm(cfg, _pjapp_cfg->sip_realm);//Set authentication password
	}
	else
		pjsua_app_cfg_realm(cfg, "*");

	if(strlen(_pjapp_cfg->sip_server.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", _pjapp_cfg->sip_server.sip_address, _pjapp_cfg->sip_server.sip_port);
		pjsua_app_cfg_registrar(cfg, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server

		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s@%s:%d", _pjapp_cfg->sip_user.sip_user,
				_pjapp_cfg->sip_server.sip_address, _pjapp_cfg->sip_server.sip_port);
		pjsua_app_cfg_url_id(cfg, buf /*"sip:100@192.168.0.103"*/);//Set the URL of local ID (used in From header)

		cur_acc->cred_count = 1;
	}

	if(strlen(_pjapp_cfg->sip_server_sec.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", _pjapp_cfg->sip_server_sec.sip_address, _pjapp_cfg->sip_server_sec.sip_port);
		pjsua_app_cfg_registrar(cfg, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server

		memset(buf, 0, sizeof(buf));
		if(_pjapp_cfg->sip_user_cnt == 1 && strlen(_pjapp_cfg->sip_user.sip_user))
			snprintf(buf, sizeof(buf), "sip:%s@%s:%d", _pjapp_cfg->sip_user.sip_user,
				_pjapp_cfg->sip_server_sec.sip_address, _pjapp_cfg->sip_server_sec.sip_port);

		if(_pjapp_cfg->sip_user_cnt == 2 && strlen(_pjapp_cfg->sip_user_sec.sip_user))
			snprintf(buf, sizeof(buf), "sip:%s@%s:%d", _pjapp_cfg->sip_user_sec.sip_user,
				_pjapp_cfg->sip_server_sec.sip_address, _pjapp_cfg->sip_server_sec.sip_port);


		pjsua_app_cfg_url_id(cfg, buf /*"sip:100@192.168.0.103"*/);//Set the URL of local ID (used in From header)

		cur_acc->cred_count = 2;
	}
	//pjsua_app_cfg_credential(cfg, 0);
	//pjsua_app_cfg_next_account(cfg);
	if(_pjapp_cfg->sip_local.sip_port)
		pjsua_app_cfg_local_port(cfg, _pjapp_cfg->sip_local.sip_port);

	if(strlen(_pjapp_cfg->sip_proxy.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d",
				_pjapp_cfg->sip_proxy.sip_address, _pjapp_cfg->sip_proxy.sip_port);
		pjsua_app_cfg_proxy(cfg, buf);
	}

	cfg->no_tcp = PJ_TRUE;
	cfg->no_udp = PJ_FALSE;
	switch(_pjapp_cfg->proto)
	{
	case PJSIP_PROTO_UDP:
		cfg->no_tcp = PJ_TRUE;
		//pjsua_app_cfg_ice_nortcp_enable(cfg, zpl_true);
		cfg->no_udp = PJ_FALSE;
		break;
	case PJSIP_PROTO_TCP:
	case PJSIP_PROTO_TLS:
	case PJSIP_PROTO_DTLS:
		cfg->no_udp = PJ_TRUE;
		cfg->no_tcp = PJ_FALSE;
		break;
	default:
		break;
	}
	pjsua_app_cfg_100rel(cfg, _pjapp_cfg->sip_100_rel);
	//"SIP Account options:"
	//
	if(_pjapp_cfg->sip_reg_proxy != PJSIP_REGISTER_NONE)
		pjsua_app_cfg_reg_use_proxy(cfg, _pjapp_cfg->sip_reg_proxy);
	if(_pjapp_cfg->sip_rereg_delay)
		pjsua_app_cfg_reg_retry_interval(cfg, _pjapp_cfg->sip_rereg_delay);
	pjsua_app_cfg_register_timeout(cfg, _pjapp_cfg->sip_expires/*_pjapp_cfg->sip_reg_timeout*/);

	pjsua_app_cfg_publish(cfg, _pjapp_cfg->sip_publish);			//Send presence PUBLISH for this account
	pjsua_app_cfg_mwi(cfg, _pjapp_cfg->sip_mwi);
	pjsua_app_cfg_ims(cfg, _pjapp_cfg->sip_ims_enable);
#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
	if(_pjapp_cfg->sip_srtp_mode != PJSIP_SRTP_DISABLE)
		pjsua_app_cfg_srtp_enable(cfg, _pjapp_cfg->sip_srtp_mode);
	if(_pjapp_cfg->sip_srtp_secure != PJSIP_SRTP_SEC_NO)
	{
		pjsua_app_cfg_srtp_secure(cfg, _pjapp_cfg->sip_srtp_secure);
		pjsua_app_cfg_srtp_keying(cfg, _pjapp_cfg->sip_srtp_keying);
	}
	//pjsip_srtp_t		sip_srtp_mode;			//Use SRTP?  0:disabled, 1:optional, 2:mandatory,3:optional by duplicating media offer (def:0)
	//pjsip_srtp_sec_t	sip_srtp_secure;		//SRTP require secure SIP? 0:no, 1:tls, 2:sips (def:1)
#endif
	if(_pjapp_cfg->sip_timer != PJSIP_TIMER_INACTIVE)
		pjsua_app_cfg_session_timer(cfg, _pjapp_cfg->sip_timer);
	if(_pjapp_cfg->sip_timer_sec)
		pjsua_app_cfg_session_timer_expiration(cfg, _pjapp_cfg->sip_timer_sec);

	//pjsua_app_cfg_outbound_reg_id(cfg, _pjapp_cfg->sip_outb_rid);

	pjsua_app_cfg_update_nat(cfg, _pjapp_cfg->sip_auto_update_nat);
	//zpl_uint16				sip_auto_update_nat;	//Where N is 0 or 1 to enable/disable SIP traversal behind symmetric NAT (default 1)
	pjsua_app_cfg_stun(cfg, _pjapp_cfg->sip_stun_disable);
	//pj_bool_t				sip_stun_disable;		//Disable STUN for this account

	//Transport Options:
#if defined(PJ_HAS_IPV6) && PJ_HAS_IPV6
	pjsua_app_cfg_ipv6_enable(cfg, _pjapp_cfg->sip_ipv6_enable);
#endif
	pjsua_app_cfg_qos_enable(cfg, _pjapp_cfg->sip_set_qos);
	if(_pjapp_cfg->sip_noudp)
		pjsua_app_cfg_no_udp(cfg);
	if(_pjapp_cfg->sip_notcp)
		pjsua_app_cfg_no_tcp(cfg);

	if(strlen(_pjapp_cfg->sip_nameserver.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", _pjapp_cfg->sip_nameserver.sip_address, _pjapp_cfg->sip_nameserver.sip_port);
		pjsua_app_cfg_nameserver(cfg, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server
	}
	if(strlen(_pjapp_cfg->sip_outbound.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", _pjapp_cfg->sip_outbound.sip_address, _pjapp_cfg->sip_outbound.sip_port);
		pjsua_app_cfg_outbound_proxy(cfg, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server
	}
	if(strlen(_pjapp_cfg->sip_stun_server.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", _pjapp_cfg->sip_stun_server.sip_address, _pjapp_cfg->sip_stun_server.sip_port);
		pjsua_app_cfg_stunserver(cfg, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server
	}
	if(strlen(_pjapp_cfg->sip_local.sip_address))
		pjsua_app_cfg_bound_address(cfg, _pjapp_cfg->sip_local.sip_address);
	//TLS Options:
/*
	pj_bool_t				sip_tls_enable;
	char				sip_tls_ca_file[PJSIP_FILE_MAX];		//Specify TLS CA file (default=none)
	char				sip_tls_cert_file[PJSIP_FILE_MAX];		//Specify TLS certificate file (default=none)
	char				sip_tls_privkey_file[PJSIP_FILE_MAX];	//Specify TLS private key file (default=none)
	char				sip_tls_password[PJSIP_PASSWORD_MAX];	//Specify TLS password to private key file (default=none)
	pjsip_server_t		sip_tls_verify_server;					//Verify server's certificate (default=no)
	pjsip_server_t		sip_tls_verify_client;					//Verify client's certificate (default=no)
	zpl_uint16				sip_neg_timeout;						//Specify TLS negotiation timeout (default=no)
	char				sip_tls_cipher[PJSIP_DATA_MAX];			//Specify prefered TLS cipher (optional).May be specified multiple times
*/

	//Audio Options:
	pjsua_app_cfg_clock_rate(cfg, _pjapp_cfg->sip_clock_rate);
	pjsua_app_cfg_snd_clock_rate(cfg, _pjapp_cfg->sip_snd_clock_rate);

	//char				sip_codec[PJSIP_DATA_MAX];
	//char				sip_discodec[PJSIP_DATA_MAX];

	if(_pjapp_cfg->sip_stereo)
		pjsua_app_cfg_stereo(cfg);

	if(strlen(_pjapp_cfg->sip_play_file))
		pjsua_app_cfg_play_file(cfg, _pjapp_cfg->sip_play_file);
	//if(strlen(_pjapp_cfg->sip_play_tone))
	//	pjsua_app_cfg_play_file(cfg, _pjapp_cfg->sip_play_tone);
	if(_pjapp_cfg->sip_auto_play)
		pjsua_app_cfg_auto_play(cfg, _pjapp_cfg->sip_auto_play);
	if(_pjapp_cfg->sip_auto_loop)
		pjsua_app_cfg_auto_loop(cfg, _pjapp_cfg->sip_auto_loop);
	if(_pjapp_cfg->sip_auto_conf)
		pjsua_app_cfg_auto_config(cfg, _pjapp_cfg->sip_auto_conf);

	if(strlen(_pjapp_cfg->sip_rec_file))
	{
		pjsua_app_cfg_rec_file(cfg, _pjapp_cfg->sip_rec_file);
		pjsua_app_cfg_auto_rec(cfg, PJ_TRUE);
	}
	pjsua_app_cfg_quality(cfg, _pjapp_cfg->sip_quality);
	pjsua_app_cfg_ptime(cfg, _pjapp_cfg->sip_ptime);

	if(_pjapp_cfg->sip_no_vad)
		pjsua_app_cfg_novad(cfg, _pjapp_cfg->sip_no_vad);

	if(_pjapp_cfg->sip_echo_mode != PJSIP_ECHO_DISABLE)
	{
		if(_pjapp_cfg->sip_echo_tail)
			pjsua_app_cfg_ec_tial(cfg, _pjapp_cfg->sip_echo_tail);
		pjsua_app_cfg_ec_options(cfg, _pjapp_cfg->sip_echo_mode - PJSIP_ECHO_DISABLE);
	}
	if(_pjapp_cfg->sip_ilbc_mode)
		pjsua_app_cfg_ilbc_mode(cfg, _pjapp_cfg->sip_ilbc_mode);

	if(_pjapp_cfg->sip_capture_lat)
		pjsua_app_cfg_capture_lat(cfg, _pjapp_cfg->sip_capture_lat);

	if(_pjapp_cfg->sip_playback_lat)
		pjsua_app_cfg_playback_lat(cfg, _pjapp_cfg->sip_playback_lat);

	pjsua_app_cfg_snd_auto_close(cfg, _pjapp_cfg->sip_snd_auto_close);
	pjsua_app_cfg_no_tones(cfg, _pjapp_cfg->sip_notones);
	if(_pjapp_cfg->sip_jb_max_size)
		pjsua_app_cfg_jb_max_size(cfg, _pjapp_cfg->sip_jb_max_size);

	if(strlen(_pjapp_cfg->capture_dev_name))
	{
		strcpy(cfg->capture_dev_name, _pjapp_cfg->capture_dev_name);
	}
	if(strlen(_pjapp_cfg->playback_dev_name))
	{
		strcpy(cfg->playback_dev_name, _pjapp_cfg->playback_dev_name);
	}

#if PJSUA_HAS_VIDEO
	if(strlen(_pjapp_cfg->vcapture_dev_name))
	{
		strcpy(cfg->vid.vcapture_dev_name, _pjapp_cfg->vcapture_dev_name);
	}

	if(strlen(_pjapp_cfg->vrender_dev_name))
	{
		strcpy(cfg->vid.vrender_dev_name, _pjapp_cfg->vrender_dev_name);
	}
	//Video Options:
	if(_pjapp_cfg->sip_video)
	{
		pjsua_app_cfg_video_enable(cfg, _pjapp_cfg->sip_video);
		if(strlen(_pjapp_cfg->sip_play_avi))
			pjsua_app_cfg_play_avi(cfg, _pjapp_cfg->sip_play_avi);
		pjsua_app_cfg_auto_play_avi(cfg, _pjapp_cfg->sip_auto_play_avi);
	}
#endif
	//Media Transport Options:
	pjsua_app_cfg_ice_enable(cfg, _pjapp_cfg->sip_ice);
	pjsua_app_cfg_regular_enable(cfg, _pjapp_cfg->sip_ice_regular);
	pjsua_app_cfg_ice_max_hosts(cfg, _pjapp_cfg->sip_ice_regular);
	pjsua_app_cfg_ice_nortcp_enable(cfg, _pjapp_cfg->sip_ice_nortcp);

	if(_pjapp_cfg->sip_rtp_port)
		pjsua_app_cfg_rtp_port(cfg, _pjapp_cfg->sip_rtp_port);

	if(_pjapp_cfg->sip_rx_drop_pct)
		pjsua_app_cfg_rx_drop_pct(cfg, _pjapp_cfg->sip_rx_drop_pct);
	if(_pjapp_cfg->sip_tx_drop_pct)
		pjsua_app_cfg_tx_drop_pct(cfg, _pjapp_cfg->sip_tx_drop_pct);

	pjsua_app_cfg_turn_enable(cfg, _pjapp_cfg->sip_turn);
	if(strlen(_pjapp_cfg->sip_turn_srv.sip_address))
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "sip:%s:%d", _pjapp_cfg->sip_turn_srv.sip_address, _pjapp_cfg->sip_turn_srv.sip_port);
		pjsua_app_cfg_turn_srv(cfg, buf/*"sip:192.168.0.103"*/);//Set the URL of registrar server
	}
	pjsua_app_cfg_turn_tcp(cfg, _pjapp_cfg->sip_turn_tcp);

	if(strlen(_pjapp_cfg->sip_turn_user))
		pjsua_app_cfg_play_avi(cfg, _pjapp_cfg->sip_turn_user);
	if(strlen(_pjapp_cfg->sip_turn_password))
		pjsua_app_cfg_play_avi(cfg, _pjapp_cfg->sip_turn_password);

	pjsua_app_cfg_rtcp_mux(cfg, _pjapp_cfg->sip_rtcp_mux);
	//Buddy List (can be more than one):
	//void				*buddy_list;
	//User Agent options:
	if(_pjapp_cfg->sip_auto_answer_code)
		pjsua_app_cfg_auto_answer(cfg, _pjapp_cfg->sip_auto_answer_code);
	pjsua_app_cfg_max_calls(cfg, _pjapp_cfg->sip_max_calls);
	pjsua_app_cfg_thread_cnt(cfg, _pjapp_cfg->sip_thread_max);
	pjsua_app_cfg_duration(cfg, _pjapp_cfg->sip_duration);
	if(_pjapp_cfg->sip_norefersub)
		pjsua_app_cfg_norefersub(cfg);
	pjsua_app_cfg_compact_form(cfg, _pjapp_cfg->sip_use_compact_form);
	pjsua_app_cfg_no_force_lr(cfg, _pjapp_cfg->sip_no_force_lr);
	pjsua_app_cfg_accept_redirect(cfg, _pjapp_cfg->sip_accept_redirect);

	if(_pjapp_cfg->sip_codec.is_active)
	{
		pjsua_app_cfg_add_codec(cfg, _pjapp_cfg->sip_codec.payload_name);
	}

	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(_pjapp_cfg->codec[i].is_active)
		{
			pjsua_app_cfg_add_codec(cfg, _pjapp_cfg->codec[i].payload_name);
		}
	}
	for(i = 0; i < PJSIP_CODEC_MAX; i++)
	{
		if(_pjapp_cfg->dicodec[i].is_active)
		{
			pjsua_app_cfg_dis_codec(cfg, _pjapp_cfg->dicodec[i].payload_name);
		}
	}

	if(_pjapp_cfg->mutex)
		os_mutex_unlock(_pjapp_cfg->mutex);

	return PJ_SUCCESS;
}


