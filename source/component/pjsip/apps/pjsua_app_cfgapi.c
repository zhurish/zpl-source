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
#include "pjsua_app_common.h"
#include "pjsua_app_cfgapi.h"
#include "auto_include.h"
#include "lib_include.h"
#define THIS_FILE "pjsua_app_cfgapi.c"


static PJ_DEF(void) pjapp_cfg_log_cb(int level, const char *buffer, int len)
{
	zassert(buffer != NULL);
	if (!len)
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
		// printf("%s", buffer);
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

int pjapp_cfg_log_file(pjsua_app_config *cfg, char *logfile)
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
		pj_str_clr(&cfg->log_cfg.log_filename);
	}
	pj_str_set(&cfg->log_cfg.log_filename, logfile);
	return PJ_SUCCESS;
}

int pjapp_cfg_log_level(pjsua_app_config *cfg, int level)
{
	zassert(cfg != NULL);
	if (level < 0 || level > 6)
	{
		PJ_LOG(1,
			   (THIS_FILE, "Error: expecting integer value 0-6 "
						   "for --log-level"));
		return ERROR;
	}
	cfg->log_cfg.level = level;
	pj_log_set_level(level);
	return PJ_SUCCESS;
}

int pjapp_cfg_app_log_level(pjsua_app_config *cfg, int level)
{
	zassert(cfg != NULL);
	if (level < 0 || level > 6)
	{
		PJ_LOG(1,
			   (THIS_FILE, "Error: expecting integer value 0-6 "
						   "for --log-level"));
		return ERROR;
	}
	cfg->log_cfg.console_level = level;
	return PJ_SUCCESS;
}

int pjapp_cfg_log_option(pjsua_app_config *cfg, int option, pj_bool_t enable)
{
	zassert(cfg != NULL);
	if (enable)
		cfg->log_cfg.log_file_flags |= option;
	else
		cfg->log_cfg.log_file_flags &= ~option;
	return PJ_SUCCESS;
}

int pjapp_cfg_log_color(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	if (enable)
		cfg->log_cfg.decor |= PJ_LOG_HAS_COLOR;
	else
		cfg->log_cfg.decor &= ~PJ_LOG_HAS_COLOR;
	return PJ_SUCCESS;
}

int pjapp_cfg_log_light_bg(pjsua_app_config *cfg, pj_bool_t enable)
{
	pj_log_set_color(1, PJ_TERM_COLOR_R);
	pj_log_set_color(2, PJ_TERM_COLOR_R | PJ_TERM_COLOR_G);
	pj_log_set_color(3, PJ_TERM_COLOR_B | PJ_TERM_COLOR_G);
	pj_log_set_color(4, 0);
	pj_log_set_color(5, 0);
	pj_log_set_color(77, 0);
	return PJ_SUCCESS;
}



/*********account********/

pjsua_acc_config *pjapp_account_acc_lookup(char *user)
{
	int j = 0;
	pjsua_app_config *cfg = &_pjAppCfg;
	pjsua_acc_config *cur_acc = NULL;
	pjapp_username_t *userdata = NULL;
	for (j = 0; j < PJ_ARRAY_SIZE(cfg->acc_cfg); j++)
	{
		userdata = cfg->acc_cfg[j].user_data;
		if (userdata && strcmp(userdata->sip_phone, user) == 0)
		{
			cur_acc = &cfg->acc_cfg[j];
		}
	}
	return cur_acc;
}

static pjsua_acc_config *pjapp_account_acc_get(pjsua_app_config *cfg, char *user)
{
	int j = 0;
	pjsua_acc_config *cur_acc = NULL;
	pjapp_username_t *userdata = NULL;
	for (j = 0; j < PJ_ARRAY_SIZE(cfg->acc_cfg); j++)
	{
		userdata = cfg->acc_cfg[j].user_data;
		if (userdata && strcmp(userdata->sip_phone, user) == 0)
		{
			cur_acc = &cfg->acc_cfg[j];
		}
	}
	return cur_acc;
}

int pjapp_account_add(pjsua_app_config *cfg, char *sip_user)
{
	int i = 0;
	zassert(cfg != NULL);
	zassert(sip_user != NULL);
	pjsua_acc_config *cur_acc = NULL;
	pjapp_username_t *userdata = NULL;
	cur_acc = pjapp_account_acc_get(cfg, sip_user);
	if (cur_acc)
	{
		return ERROR;
	}
	if (cfg->acc_cnt > PJ_ARRAY_SIZE(cfg->acc_cfg))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
		return ERROR;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->acc_cfg); i++)
	{
		userdata = cfg->acc_cfg[i].user_data;
		if (userdata && strcmp(userdata->sip_phone, sip_user) == 0)
		{
			PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->acc_cfg); i++)
	{
		userdata = cfg->acc_cfg[i].user_data;
		if (userdata == NULL)
		{
			cfg->acc_cfg[i].user_data = malloc(sizeof(pjapp_username_t));
			if (cfg->acc_cfg[i].user_data)
			{
				userdata = cfg->acc_cfg[i].user_data;
				strcpy(userdata->sip_phone, sip_user);
				cfg->acc_cfg[i].auth_pref.initial_auth = zpl_false;
				cfg->acc_cfg[i].allow_contact_rewrite = zpl_true;
				cfg->acc_cnt++;
				return PJ_SUCCESS;
			}
		}
	}
	return ERROR;
}
int pjapp_account_del(pjsua_app_config *cfg, char *sip_user)
{
	zassert(cfg != NULL);
	zassert(sip_user != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, sip_user);
	if (cur_acc == NULL)
	{
		return ERROR;
	}
	if (cur_acc->user_data)
	{
		free(cur_acc->user_data);
	}
	pj_str_clr(&cur_acc->id);
	cfg->acc_cnt--;
	return PJ_SUCCESS;
}

int pjapp_account_priority(pjsua_app_config *cfg, char *user, int priority)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->priority = priority;
	}
	return PJ_SUCCESS;
}
int pjapp_account_register_server(pjsua_app_config *cfg, char *user, char *server, int port)
{
	int i = 0;
	char sipserver[128];
	pjsua_acc_config *cur_acc = NULL;
	pjapp_username_t *userdata = NULL;
	zassert(cfg != NULL);
	zassert(server != NULL);
	memset(sipserver, 0, sizeof(sipserver));
	snprintf(sipserver, sizeof(sipserver), "sip:%s:%d", server, port);
	PJAPP_GLOBAL_LOCK();
	if (pjsua_verify_sip_url(sipserver) != 0)
	{
		PJAPP_GLOBAL_UNLOCK();
		PJ_LOG(1,
			   (THIS_FILE, "Error: invalid SIP URL '%s' "
						   "in proxy argument",
				sipserver));
		return ERROR;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->acc_cfg); i++)
	{
		userdata = cfg->acc_cfg[i].user_data;
		if (userdata && strcmp(userdata->sip_phone, user) == 0)
		{
			cur_acc = &cfg->acc_cfg[i];
			memset(userdata->register_svr.sip_address, 0, sizeof(userdata->register_svr.sip_address));
			strcpy(userdata->register_svr.sip_address, server);
			userdata->register_svr.sip_port = port;
			pj_str_clr(&cur_acc->reg_uri);
			pj_str_set(&cur_acc->reg_uri, sipserver);

			memset(sipserver, 0, sizeof(sipserver));
			snprintf(sipserver, sizeof(sipserver), "sip:%s@%s:%d", user, server, port);
			if (cur_acc->id.slen)
			{
				pj_str_clr(&cur_acc->id);
			}
			pj_str_set(&cur_acc->id, sipserver);
			PJAPP_GLOBAL_UNLOCK();
			return PJ_SUCCESS;
		}
	}
	PJAPP_GLOBAL_UNLOCK();
	return ERROR;
}
int pjapp_account_mwi(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->mwi_enabled = enable;
		return PJ_SUCCESS;
	}
	return ERROR;
}

int pjapp_account_mwi_expires(pjsua_app_config *cfg, char *user, int mwi_expires)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->mwi_expires = mwi_expires;
		return PJ_SUCCESS;
	}
	return ERROR;
}

int pjapp_account_publish(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->publish_enabled = enable;
		return PJ_SUCCESS;
	}
	return ERROR;
}
int pjapp_account_unpublish_wait(pjsua_app_config *cfg, char *user, int val)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->unpublish_max_wait_time_msec = val;
		return PJ_SUCCESS;
	}
	return ERROR;
}

int pjapp_account_auth_preference_algorithm(pjsua_app_config *cfg, char *user, char* algorithm)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		pj_str_set(&cur_acc->auth_pref.algorithm, algorithm);
		return PJ_SUCCESS;
	}
	return ERROR;
}


int pjapp_account_100rel(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		if (enable)
		{
			cur_acc->require_100rel = PJSUA_100REL_MANDATORY;
		}
		else
		{
			cur_acc->require_100rel = PJSUA_100REL_NOT_USED;
		}
		return PJ_SUCCESS;
	}
	return ERROR;
}

int pjapp_account_proxy_add(pjsua_app_config *cfg, char *user, char *server, int port)
{
	int i = 0;
	char sipserver[128];
	zassert(cfg != NULL);
	zassert(server != NULL);
	pjsua_acc_config *cur_acc = NULL;
	memset(sipserver, 0, sizeof(sipserver));
	snprintf(sipserver, sizeof(sipserver), "sip:%s:%d", server, port);
	PJAPP_GLOBAL_LOCK();
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc == NULL || pjsua_verify_sip_url(sipserver) != 0)
	{
		PJAPP_GLOBAL_UNLOCK();
		PJ_LOG(1,
			   (THIS_FILE, "Error: invalid SIP URL '%s' "
						   "in proxy argument",
				sipserver));
		return ERROR;
	}

	if (cur_acc->proxy_cnt == PJ_ARRAY_SIZE(cur_acc->proxy))
	{
		PJAPP_GLOBAL_UNLOCK();
		PJ_LOG(1, (THIS_FILE, "Error: too many proxy servers"));
		return ERROR;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cur_acc->proxy); i++)
	{
		if (cur_acc->proxy[i].ptr && pj_stricmp2(&cur_acc->proxy[i], sipserver) == 0)
		{
			PJ_LOG(1, (THIS_FILE, "Error: proxy is already exist."));
			PJAPP_GLOBAL_UNLOCK();
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cur_acc->proxy); i++)
	{
		if (cur_acc->proxy[i].ptr == NULL)
		{
			pj_str_set(&cur_acc->proxy[i], sipserver);
			cur_acc->proxy_cnt++;
			PJAPP_GLOBAL_UNLOCK();
			return PJ_SUCCESS;
		}
	}

	PJAPP_GLOBAL_UNLOCK();
	return ERROR;
}
int pjapp_account_proxy_del(pjsua_app_config *cfg, char *user, char *server, int port)
{
	int i = 0;
	char sipserver[128];
	zassert(cfg != NULL);
	zassert(server != NULL);
	pjsua_acc_config *cur_acc = NULL;
	memset(sipserver, 0, sizeof(sipserver));
	snprintf(sipserver, sizeof(sipserver), "sip:%s:%d", server, port);
	PJAPP_GLOBAL_LOCK();
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc == NULL || pjsua_verify_sip_url(sipserver) != 0)
	{
		PJAPP_GLOBAL_UNLOCK();
		PJ_LOG(1,
			   (THIS_FILE, "Error: invalid SIP URL '%s' "
						   "in proxy argument",
				sipserver));
		return ERROR;
	}

	for (i = 0; i < PJ_ARRAY_SIZE(cur_acc->proxy); i++)
	{
		if (cur_acc->proxy[i].ptr && pj_stricmp2(&cur_acc->proxy[i], sipserver) == 0)
		{
			pj_str_clr(&cur_acc->proxy[i]);
			cur_acc->proxy_cnt--;
			PJAPP_GLOBAL_UNLOCK();
			return PJ_SUCCESS;
		}
	}
	PJAPP_GLOBAL_UNLOCK();
	return ERROR;
}



int pjapp_account_register_timeout(pjsua_app_config *cfg, char *user, int lval)
{
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->reg_timeout = (unsigned)lval;
		return PJ_SUCCESS;
	}
	return ERROR;
}
int pjapp_account_register_refresh_delay(pjsua_app_config *cfg, char *user, int lval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->reg_delay_before_refresh = (unsigned)lval;
		return PJ_SUCCESS;
	}
	return ERROR;
}
int pjapp_account_unregister_timeout(pjsua_app_config *cfg, char *user, int lval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->unreg_timeout = (unsigned)lval;
		return PJ_SUCCESS;
	}
	return ERROR;
}



int pjapp_account_auth_username_add(pjsua_app_config *cfg, char *user, char *username, char *password, char *realm, char *scheme)
{
	int i = 0;
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		if (cur_acc->cred_count > PJ_ARRAY_SIZE(cur_acc->cred_info))
		{
			PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
			return ERROR;
		}
		for (i = 0; i < PJ_ARRAY_SIZE(cur_acc->cred_info); i++)
		{
			if (cur_acc->cred_info[i].username.ptr && pj_strcmp2(&cur_acc->cred_info[i].username, username) == 0)
			{
				PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
				return ERROR;
			}
		}
		for (i = 0; i < PJ_ARRAY_SIZE(cur_acc->cred_info); i++)
		{
			if (cur_acc->cred_info[i].username.ptr == NULL)
			{
				pj_str_set(&cur_acc->cred_info[i].username, username);
				if (password)
				{
					pj_str_set(&cur_acc->cred_info[i].data, password);
					cur_acc->cred_info[i].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
				}
				if (scheme)
					pj_str_set(&cur_acc->cred_info[i].scheme, scheme /*"digest"*/);
				if (realm)
					pj_str_set(&cur_acc->cred_info[i].realm, realm);
				cur_acc->cred_count++;
#if 0 // PJSIP_HAS_DIGEST_AKA_AUTH
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
				return PJ_SUCCESS;
			}
		}
	}
	return PJ_SUCCESS;
}
int pjapp_account_auth_username_del(pjsua_app_config *cfg, char *user, char *username)
{
	int i = 0;
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		if (cur_acc->cred_count > PJ_ARRAY_SIZE(cur_acc->cred_info))
		{
			PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
			return ERROR;
		}
		for (i = 0; i < PJ_ARRAY_SIZE(cur_acc->cred_info); i++)
		{
			if (cur_acc->cred_info[i].username.ptr && pj_strcmp2(&cur_acc->cred_info[i].username, username) == 0)
			{
				pj_str_clr(&cur_acc->cred_info[i].username);
				pj_str_clr(&cur_acc->cred_info[i].data);
				cur_acc->cred_info[i].data_type = 0;
				pj_str_clr(&cur_acc->cred_info[i].scheme);
				pj_str_clr(&cur_acc->cred_info[i].realm);
				cur_acc->cred_count--;
				return PJ_SUCCESS;
			}
		}
	}
	return PJ_SUCCESS;
}

int pjapp_account_rfc5626_instance_id(pjsua_app_config *cfg, char *user, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		if (cur_acc->rfc5626_instance_id.slen)
		{
			pj_str_clr(&cur_acc->rfc5626_instance_id);
		}
		pj_str_set(&cur_acc->rfc5626_instance_id, lval);
		return PJ_SUCCESS;
	}
	return ERROR;
}
int pjapp_account_rfc5626_register_id(pjsua_app_config *cfg, char *user, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		if (cur_acc->rfc5626_reg_id.slen)
		{
			pj_str_clr(&cur_acc->rfc5626_reg_id);
		}
		pj_str_set(&cur_acc->rfc5626_reg_id, lval);
		return PJ_SUCCESS;
	}
	return ERROR;
}

int pjapp_account_video_auto_show(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->vid_in_auto_show = enable;
		return PJ_SUCCESS;
	}
	return ERROR;
}
int pjapp_account_video_auto_transmit(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->vid_out_auto_transmit = enable;
		return PJ_SUCCESS;
	}
	return ERROR;
}
int pjapp_account_video_record_bandwidth(pjsua_app_config *cfg, char *user, int bandwidth)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->vid_stream_rc_cfg.bandwidth = bandwidth;
		return PJ_SUCCESS;
	}
	return ERROR;
}
int pjapp_account_nat64(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->nat64_opt = enable;
		return PJ_SUCCESS;
	}
	return ERROR;
}
int pjapp_account_media_ipv6(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->ipv6_media_use = enable;
		return PJ_SUCCESS;
	}
	return ERROR;
}

int pjapp_account_stun(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		if (enable)
		{
			cur_acc->sip_stun_use = PJSUA_STUN_USE_DEFAULT;
		}
		else
		{
			cur_acc->sip_stun_use = PJSUA_STUN_USE_DISABLED;
		}
	}
	return PJ_SUCCESS;
}
int pjapp_account_media_stun(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		if (enable)
		{
			cur_acc->media_stun_use = PJSUA_STUN_USE_DEFAULT;
		}
		else
		{
			cur_acc->media_stun_use = PJSUA_STUN_USE_DISABLED;
		}
	}
	return PJ_SUCCESS;
}
int pjapp_account_upnp(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		if (enable)
		{
			cur_acc->sip_upnp_use = PJSUA_UPNP_USE_DEFAULT;
		}
		else
		{
			cur_acc->sip_upnp_use = PJSUA_UPNP_USE_DISABLED;
		}
	}
	return PJ_SUCCESS;
}
int pjapp_account_media_upnp(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		if (enable)
		{
			cur_acc->media_upnp_use = PJSUA_UPNP_USE_DEFAULT;
		}
		else
		{
			cur_acc->media_upnp_use = PJSUA_UPNP_USE_DISABLED;
		}
	}
	return PJ_SUCCESS;
}
int pjapp_account_loopback(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->use_loop_med_tp = enable;
	}
	return PJ_SUCCESS;
}
int pjapp_account_media_loopback(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->enable_loopback = enable;
	}
	return PJ_SUCCESS;
}
int pjapp_account_ice_rtcp(pjsua_app_config *cfg, char *user, pj_bool_t enable, pj_bool_t rtcp_enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->ice_cfg_use = PJSUA_ICE_CONFIG_USE_CUSTOM;
		cur_acc->ice_cfg.enable_ice = enable;
		cur_acc->ice_cfg.ice_no_rtcp = rtcp_enable;
	}
	return PJ_SUCCESS;
}

int pjapp_account_turn_enable(pjsua_app_config *cfg, char *user, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->turn_cfg.enable_turn = enable;
		cur_acc->turn_cfg_use = PJSUA_TURN_CONFIG_USE_CUSTOM;
	}
	return PJ_SUCCESS;
}
int pjapp_account_turn_server(pjsua_app_config *cfg, char *user, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		if (cur_acc->turn_cfg.turn_server.slen)
		{
			pj_str_clr(&cur_acc->turn_cfg.turn_server);
		}
		pj_str_set(&cur_acc->turn_cfg.turn_server, lval);
	}
	return PJ_SUCCESS;
}

int pjapp_account_turn_proto(pjsua_app_config *cfg, char *user, int proto)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
	cur_acc->turn_cfg.turn_conn_type = proto;
	}
	return PJ_SUCCESS;
}

int pjapp_account_turn_auth(pjsua_app_config *cfg, char *user, char *username, char *password, char *realm)
{
	zassert(cfg != NULL);
	zassert(username != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data_type = PJ_STUN_PASSWD_PLAIN;
		pj_str_set(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.username, username);
		pj_str_set(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.data, password);
		pj_str_set(&cur_acc->turn_cfg.turn_auth_cred.data.static_cred.realm, realm);
		cur_acc->turn_cfg.turn_auth_cred.type = PJ_STUN_AUTH_CRED_STATIC;
	}
	return PJ_SUCCESS;
}
int pjapp_account_turn_auth_tlsfilstlist(pjsua_app_config *cfg, char *user, char *filstlist)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		pj_str_set(&cur_acc->turn_cfg.turn_tls_setting.ca_list_file, filstlist);
	}
	return PJ_SUCCESS;
}
int pjapp_account_turn_auth_cert_file(pjsua_app_config *cfg, char *user, char *filstlist)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		pj_str_set(&cur_acc->turn_cfg.turn_tls_setting.cert_file, filstlist);
	}
	return PJ_SUCCESS;
}
int pjapp_account_turn_auth_privkey_file(pjsua_app_config *cfg, char *user, char *filstlist)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		pj_str_set(&cur_acc->turn_cfg.turn_tls_setting.privkey_file, filstlist);
	}
	return PJ_SUCCESS;
}

int pjapp_account_turn_auth_keystrlist(pjsua_app_config *cfg, char *user, char *filstlist)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		pj_str_set(&cur_acc->turn_cfg.turn_tls_setting.ca_buf, filstlist);
	}
	return PJ_SUCCESS;
}
int pjapp_account_turn_auth_cert_key(pjsua_app_config *cfg, char *user, char *filstlist)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		pj_str_set(&cur_acc->turn_cfg.turn_tls_setting.cert_buf, filstlist);
	}
	return PJ_SUCCESS;
}
int pjapp_account_turn_auth_privkey(pjsua_app_config *cfg, char *user, char *filstlist)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		pj_str_set(&cur_acc->turn_cfg.turn_tls_setting.privkey_buf, filstlist);
	}
	return PJ_SUCCESS;
}
int pjapp_account_turn_auth_password(pjsua_app_config *cfg, char *user, char *filstlist)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		pj_str_set(&cur_acc->turn_cfg.turn_tls_setting.password, filstlist);
	}
	return PJ_SUCCESS;
}

#if defined(PJMEDIA_HAS_SRTP) && (PJMEDIA_HAS_SRTP != 0)
int pjapp_account_srtp_enable(pjsua_app_config *cfg, char *user,  int type)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->use_srtp = type;
		if ((int)cur_acc->use_srtp == PJMEDIA_SRTP_OPTIONAL)
		{
			/* SRTP optional mode with duplicated media offer */
			cur_acc->srtp_optional_dup_offer = PJ_TRUE;
			cur_acc->srtp_optional_dup_offer = PJ_TRUE;
		}
	}
	return PJ_SUCCESS;
}

int pjapp_account_srtp_secure(pjsua_app_config *cfg, char *user, int type)
{
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->srtp_secure_signaling = type;
	}
	return PJ_SUCCESS;
}

int pjapp_account_srtp_keying(pjsua_app_config *cfg, char *user, int type1, int type2)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->srtp_opt.keying_count = 2;
		cur_acc->srtp_opt.keying[0] = type1;//PJMEDIA_SRTP_KEYING_DTLS_SRTP;
		cur_acc->srtp_opt.keying[1] = type2;//PJMEDIA_SRTP_KEYING_SDES;
	}
	return PJ_SUCCESS;
}

int pjapp_account_srtp_crypto_add(pjsua_app_config *cfg, char *user, char* name, char* keystr)
{
	int i = 0;
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
        for (i = 0; i < PJ_ARRAY_SIZE(cur_acc->srtp_opt.crypto); i++)
        {
			if (cur_acc->srtp_opt.crypto[i].name.ptr && pj_strcmp2(&cur_acc->srtp_opt.crypto[i].name, name) == 0)
			{
				return ERROR;
			}
		}
        for (i = 0; i < PJ_ARRAY_SIZE(cur_acc->srtp_opt.crypto); i++)
        {
			if (cur_acc->srtp_opt.crypto[i].name.ptr == NULL)
			{
				pj_str_set(&cur_acc->srtp_opt.crypto[i].name, name);
				pj_str_set(&cur_acc->srtp_opt.crypto[i].key, keystr);
				return PJ_SUCCESS;
			}
		}
	}
	return PJ_SUCCESS;
}
int pjapp_account_srtp_crypto_del(pjsua_app_config *cfg, char *user, char* name)
{
	int i = 0;
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
        for (i = 0; i <  PJ_ARRAY_SIZE(cur_acc->srtp_opt.crypto); i++)
        {
			if (cur_acc->srtp_opt.crypto[i].name.ptr && pj_strcmp2(&cur_acc->srtp_opt.crypto[i].name, name) == 0)
			{
				pj_str_clr(&cur_acc->srtp_opt.crypto[i].name);
				pj_str_clr(&cur_acc->srtp_opt.crypto[i].key);
				return PJ_SUCCESS;
			}
		}
	}
	return PJ_SUCCESS;
}
#endif

int pjapp_account_register_retry_interval(pjsua_app_config *cfg, char *user, int interval)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		cur_acc->reg_retry_interval = (unsigned)interval;
	}
	return PJ_SUCCESS;
}

int pjapp_account_register_use_proxy(pjsua_app_config *cfg, char *user, int value)
{
	zassert(cfg != NULL);
	pjsua_acc_config *cur_acc = NULL;
	cur_acc = pjapp_account_acc_get(cfg, user);
	if (cur_acc)
	{
		if (value > 4)
		{
			PJ_LOG(1,
				   (THIS_FILE, "Error: invalid --reg-use-proxy value '%d'", value));
			return ERROR;
		}
		cur_acc->reg_use_proxy = (unsigned)value;
	}
	return PJ_SUCCESS;
}
/********************************* SUA ***************************************/
int pjapp_cfg_nameserver_add(pjsua_app_config *cfg, char *lval)
{
	int i = 0;
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->cfg.nameserver_count > PJ_ARRAY_SIZE(cfg->cfg.nameserver))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
		return ERROR;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.nameserver); i++)
	{
		if (cfg->cfg.nameserver[i].ptr && pj_stricmp2(&cfg->cfg.nameserver[i], lval) == 0)
		{
			PJ_LOG(1, (THIS_FILE, "Error: nameservers is already exist."));
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.nameserver); i++)
	{
		if (cfg->cfg.nameserver[i].ptr == NULL)
		{
			pj_str_set(&cfg->cfg.nameserver[cfg->cfg.nameserver_count], lval);
			cfg->cfg.nameserver_count++;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_cfg_nameserver_del(pjsua_app_config *cfg, char *lval)
{
	int i = 0;
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->cfg.nameserver_count > PJ_ARRAY_SIZE(cfg->cfg.nameserver))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
		return ERROR;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.nameserver); i++)
	{
		if (cfg->cfg.nameserver[i].ptr && pj_stricmp2(&cfg->cfg.nameserver[i], lval) == 0)
		{
			pj_str_clr(&cfg->cfg.nameserver[i]);
			cfg->cfg.nameserver_count--;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}
int pjapp_cfg_loose_route(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->cfg.force_lr = enable;
	return PJ_SUCCESS;
}
int pjapp_cfg_outbound_proxy_add(pjsua_app_config *cfg, char *server, int port)
{
	int i = 0;
	char sipserver[128];
	zassert(cfg != NULL);
	zassert(server != NULL);
	pjsua_acc_config *cur_acc = NULL;
	memset(sipserver, 0, sizeof(sipserver));
	snprintf(sipserver, sizeof(sipserver), "sip:%s:%d", server, port);
	PJAPP_GLOBAL_LOCK();

	if (pjsua_verify_sip_url(sipserver) != 0)
	{
		PJAPP_GLOBAL_UNLOCK();
		PJ_LOG(1,
			   (THIS_FILE, "Error: invalid SIP URL '%s' "
						   "in proxy argument",
				sipserver));
		return ERROR;
	}
	if (cfg->cfg.outbound_proxy_cnt == PJ_ARRAY_SIZE(cfg->cfg.outbound_proxy))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many STUN servers"));
		return ERROR;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.outbound_proxy); i++)
	{
		if (cfg->cfg.outbound_proxy[i].ptr && pj_stricmp2(&cfg->cfg.outbound_proxy[i], sipserver) == 0)
		{
			PJ_LOG(1, (THIS_FILE, "Error: stunservers is already exist."));
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.outbound_proxy); i++)
	{
		if (cfg->cfg.outbound_proxy[i].ptr == NULL)
		{
			pj_str_set(&cfg->cfg.outbound_proxy[i], sipserver);
			cfg->cfg.outbound_proxy_cnt++;
			return PJ_SUCCESS;
		}
	}
	PJAPP_GLOBAL_UNLOCK();
	return PJ_SUCCESS;
}
int pjapp_cfg_outbound_proxy_del(pjsua_app_config *cfg, char *server, int port)
{
	int i = 0;
	char sipserver[128];
	zassert(cfg != NULL);
	zassert(server != NULL);
	pjsua_acc_config *cur_acc = NULL;
	memset(sipserver, 0, sizeof(sipserver));
	snprintf(sipserver, sizeof(sipserver), "sip:%s:%d", server, port);
	PJAPP_GLOBAL_LOCK();
	if (pjsua_verify_sip_url(sipserver) != 0)
	{
		PJAPP_GLOBAL_UNLOCK();
		PJ_LOG(1,
			   (THIS_FILE, "Error: invalid SIP URL '%s' "
						   "in proxy argument",
				sipserver));
		return ERROR;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.outbound_proxy); i++)
	{
		if (cfg->cfg.outbound_proxy[i].ptr && pj_stricmp2(&cfg->cfg.outbound_proxy[i], sipserver) == 0)
		{
			pj_str_clr(&cfg->cfg.outbound_proxy[i]);
			cfg->cfg.outbound_proxy_cnt--;
			PJAPP_GLOBAL_UNLOCK();
			return PJ_SUCCESS;
		}
	}
	PJAPP_GLOBAL_UNLOCK();
	return ERROR;
}
int pjapp_cfg_stun_domain(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->cfg.stun_domain.slen)
	{
		pj_str_clr(&cfg->cfg.stun_domain);
	}
	pj_str_set(&cfg->cfg.stun_domain, lval);
	return PJ_SUCCESS;
}
int pjapp_cfg_stun_host(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->cfg.stun_host.slen)
	{
		pj_str_clr(&cfg->cfg.stun_host);
	}
	pj_str_set(&cfg->cfg.stun_host, lval);
	return PJ_SUCCESS;
}
int pjapp_cfg_stunserver_add(pjsua_app_config *cfg, char *lval)
{
	int i = 0;
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->cfg.stun_srv_cnt == PJ_ARRAY_SIZE(cfg->cfg.stun_srv))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many STUN servers"));
		return ERROR;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.stun_srv); i++)
	{
		if (cfg->cfg.stun_srv[i].ptr && pj_stricmp2(&cfg->cfg.stun_srv[i], lval) == 0)
		{
			PJ_LOG(1, (THIS_FILE, "Error: stunservers is already exist."));
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.stun_srv); i++)
	{
		if (cfg->cfg.stun_srv[i].ptr == NULL)
		{
			pj_str_set(&cfg->cfg.stun_srv[cfg->cfg.stun_srv_cnt], lval);
			cfg->cfg.stun_srv_cnt++;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}
int pjapp_cfg_stunserver_del(pjsua_app_config *cfg, char *lval)
{
	int i = 0;
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->cfg.stun_srv_cnt > PJ_ARRAY_SIZE(cfg->cfg.stun_srv))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
		return ERROR;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.stun_srv); i++)
	{
		if (cfg->cfg.stun_srv[i].ptr && pj_stricmp2(&cfg->cfg.stun_srv[i], lval) == 0)
		{
			pj_str_clr(&cfg->cfg.stun_srv[i]);
			cfg->cfg.stun_srv_cnt--;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_cfg_stun_ipv6(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->cfg.stun_try_ipv6 = enable;
	return PJ_SUCCESS;
}
int pjapp_cfg_100real(pjsua_app_config *cfg, int val)
{
	zassert(cfg != NULL);
	cfg->cfg.require_100rel = val;
	return PJ_SUCCESS;
}

int pjapp_cfg_auth_username_add(pjsua_app_config *cfg, char *username, char *password, char *realm, char *scheme)
{
	int i = 0;

	if (cfg->cfg.cred_count > PJ_ARRAY_SIZE(cfg->cfg.cred_info))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
		return ERROR;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.cred_info); i++)
	{
		if (cfg->cfg.cred_info[i].username.ptr && pj_strcmp2(&cfg->cfg.cred_info[i].username, username) == 0)
		{
			PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.cred_info); i++)
	{
		if (cfg->cfg.cred_info[i].username.ptr == NULL)
		{
			pj_str_set(&cfg->cfg.cred_info[i].username, username);
			if (password)
			{
				pj_str_set(&cfg->cfg.cred_info[i].data, password);
				cfg->cfg.cred_info[i].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
			}
			if (scheme)
				pj_str_set(&cfg->cfg.cred_info[i].scheme, scheme /*"digest"*/);
			if (realm)
				pj_str_set(&cfg->cfg.cred_info[i].realm, realm);
			cfg->cfg.cred_count++;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_cfg_auth_username_del(pjsua_app_config *cfg, char *username)
{
	int i = 0;

	if (cfg->cfg.cred_count > PJ_ARRAY_SIZE(cfg->cfg.cred_info))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many nameservers"));
		return ERROR;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.cred_info); i++)
	{
		if (cfg->cfg.cred_info[i].username.ptr && pj_strcmp2(&cfg->cfg.cred_info[i].username, username) == 0)
		{
			pj_str_clr(&cfg->cfg.cred_info[i].username);
			pj_str_clr(&cfg->cfg.cred_info[i].data);
			cfg->cfg.cred_info[i].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
			pj_str_clr(&cfg->cfg.cred_info[i].scheme /*"digest"*/);
			pj_str_clr(&cfg->cfg.cred_info[i].realm);
			cfg->cfg.cred_count--;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_cfg_user_agent(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->cfg.user_agent.slen)
	{
		pj_str_clr(&cfg->cfg.user_agent);
	}
	pj_str_set(&cfg->cfg.user_agent, lval);
	return PJ_SUCCESS;
}

int pjapp_cfg_srtp_option(pjsua_app_config *cfg, int val)
{
	zassert(cfg != NULL);
	cfg->cfg.use_srtp = val;
	return PJ_SUCCESS;
}
int pjapp_cfg_srtp_secure_signaling(pjsua_app_config *cfg, int val)
{
	zassert(cfg != NULL);
	cfg->cfg.srtp_secure_signaling = val;
	return PJ_SUCCESS;
}

int pjapp_cfg_srtp_keying(pjsua_app_config *cfg, int type1, int type2)
{
	zassert(cfg != NULL);
	cfg->cfg.srtp_opt.keying_count = 2;
	cfg->cfg.srtp_opt.keying[0] = type1; // PJMEDIA_SRTP_KEYING_DTLS_SRTP;
	cfg->cfg.srtp_opt.keying[1] = type2; // PJMEDIA_SRTP_KEYING_SDES;
	return PJ_SUCCESS;
}

int pjapp_cfg_srtp_crypto_add(pjsua_app_config *cfg, char *name, char *keystr)
{
	int i = 0;
	zassert(cfg != NULL);

	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.srtp_opt.crypto); i++)
	{
		if (cfg->cfg.srtp_opt.crypto[i].name.ptr && pj_strcmp2(&cfg->cfg.srtp_opt.crypto[i].name, name) == 0)
		{
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.srtp_opt.crypto); i++)
	{
		if (cfg->cfg.srtp_opt.crypto[i].name.ptr == NULL)
		{
			pj_str_set(&cfg->cfg.srtp_opt.crypto[i].name, name);
			pj_str_set(&cfg->cfg.srtp_opt.crypto[i].key, keystr);
			return PJ_SUCCESS;
		}
	}
	return PJ_SUCCESS;
}
int pjapp_cfg_srtp_crypto_del(pjsua_app_config *cfg, char *name)
{
	int i = 0;
	zassert(cfg != NULL);

	for (i = 0; i < PJ_ARRAY_SIZE(cfg->cfg.srtp_opt.crypto); i++)
	{
		if (cfg->cfg.srtp_opt.crypto[i].name.ptr && pj_strcmp2(&cfg->cfg.srtp_opt.crypto[i].name, name) == 0)
		{
			pj_str_clr(&cfg->cfg.srtp_opt.crypto[i].name);
			pj_str_clr(&cfg->cfg.srtp_opt.crypto[i].key);
			return PJ_SUCCESS;
		}
	}
	return PJ_SUCCESS;
}
int pjapp_cfg_upnp_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->cfg.enable_upnp = enable;
	return PJ_SUCCESS;
}
int pjapp_cfg_user_upnp_interface(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->cfg.upnp_if_name.slen)
	{
		pj_str_clr(&cfg->cfg.upnp_if_name);
	}
	pj_str_set(&cfg->cfg.upnp_if_name, lval);
	return PJ_SUCCESS;
}

/********************************* transport ***************************************/
/*本地IP地址和端口*/
int pjapp_transport_local_port(pjsua_app_config *cfg, pjapp_transport_proto_t type, char *address, int port, int port_range)
{
	zassert(cfg != NULL);
	pjsua_transport_config  *rcfg = NULL;
	if(type == PJAPP_PROTO_UDP)
		rcfg = &cfg->udp_cfg;
	else if(type == PJAPP_PROTO_TCP)
		rcfg = &cfg->rtp_cfg;
	rcfg->port = port;
	rcfg->port_range = port_range;
	if (rcfg->bound_addr.slen)
	{
		pj_str_clr(&rcfg->bound_addr);
	}
	pj_str_set(&rcfg->bound_addr, address);
	return PJ_SUCCESS;
}
int pjapp_transport_public(pjsua_app_config *cfg, pjapp_transport_proto_t type, char *address)
{
	zassert(cfg != NULL);
	pjsua_transport_config  *rcfg = NULL;
	if(type == PJAPP_PROTO_UDP)
		rcfg = &cfg->udp_cfg;
	else if(type == PJAPP_PROTO_TCP)
		rcfg = &cfg->rtp_cfg;
	if (rcfg->public_addr.slen)
	{
		pj_str_clr(&rcfg->public_addr);
	}
	pj_str_set(&rcfg->public_addr, address);
	return PJ_SUCCESS;
}
int pjapp_transport_randomize_port_enable(pjsua_app_config *cfg, pjapp_transport_proto_t type, pj_bool_t enable)
{
	zassert(cfg != NULL);
	pjsua_transport_config  *rcfg = NULL;
	if(type == PJAPP_PROTO_UDP)
		rcfg = &cfg->udp_cfg;
	else if(type == PJAPP_PROTO_TCP)
		rcfg = &cfg->rtp_cfg;
	rcfg->randomize_port = enable;
	return PJ_SUCCESS;
}
#if defined(PJSIP_HAS_TLS_TRANSPORT) && (PJSIP_HAS_TLS_TRANSPORT != 0)
int pjapp_transport_tls_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->use_tls = enable;
	return PJ_SUCCESS;
}

int pjapp_transport_tls_ca_file(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.ca_list_file);
	pj_str_set(&cfg->udp_cfg.tls_setting.ca_list_file, lval);
	return PJ_SUCCESS;
}

int pjapp_transport_tls_cert_file(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.cert_file);
	pj_str_set(&cfg->udp_cfg.tls_setting.cert_file, lval);
	return PJ_SUCCESS;
}

int pjapp_transport_tls_priv_file(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.privkey_file);
	pj_str_set(&cfg->udp_cfg.tls_setting.privkey_file, lval);
	return PJ_SUCCESS;
}

int pjapp_transport_tls_password(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	pj_str_clr(&cfg->udp_cfg.tls_setting.password);
	pj_str_set(&cfg->udp_cfg.tls_setting.password, lval);
	return PJ_SUCCESS;
}

int pjapp_transport_tls_verify_server(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->udp_cfg.tls_setting.verify_server = enable;
	return PJ_SUCCESS;
}

int pjapp_transport_tls_verify_client(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->udp_cfg.tls_setting.verify_client = enable;
	cfg->udp_cfg.tls_setting.require_client_cert = enable;
	return PJ_SUCCESS;
}

int pjapp_transport_tls_neg_timeout(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->udp_cfg.tls_setting.timeout.sec = value;
	return PJ_SUCCESS;
}

int pjapp_transport_tls_cipher(pjsua_app_config *cfg, char *lval)
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
			   (THIS_FILE, "Cipher \"%s\" is not supported by "
						   "TLS/SSL backend.",
				lval));
		printf("Available TLS/SSL ciphers (%d):\n", ciphers_cnt);
		for (j = 0; j < ciphers_cnt; ++j)
			printf("- 0x%06X: %s\n", ciphers[j],
				   pj_ssl_cipher_name(ciphers[j]));
		return -1;
	}
	return PJ_SUCCESS;
}
#endif
/********************************* media ***************************************/
int pjapp_media_clock_rate(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	if (lval < 8000 || lval > 192000)
	{
		PJ_LOG(1,
			   (THIS_FILE, "Error: expecting value between "
						   "8000-192000 for conference clock rate"));
		return ERROR;
	}
	cfg->media_cfg.clock_rate = (unsigned)lval;
	return PJ_SUCCESS;
}

int pjapp_media_sound_clock_rate(pjsua_app_config *cfg, int lval)
{
	zassert(cfg != NULL);
	if (lval < 8000 || lval > 192000)
	{
		PJ_LOG(1,
			   (THIS_FILE, "Error: expecting value between "
						   "8000-192000 for sound device clock rate"));
		return ERROR;
	}
	cfg->media_cfg.snd_clock_rate = (unsigned)lval;
	return PJ_SUCCESS;
}

int pjapp_media_audio_frame_ptime(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value < 10 || value > 1000)
	{
		PJ_LOG(1, (THIS_FILE, "Error: invalid --ptime option"));
		return ERROR;
	}
	cfg->media_cfg.audio_frame_ptime = value;
	return PJ_SUCCESS;
}

int pjapp_media_audio_ptime(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value < 10 || value > 1000)
	{
		PJ_LOG(1, (THIS_FILE, "Error: invalid --ptime option"));
		return ERROR;
	}
	cfg->media_cfg.ptime = value;
	return PJ_SUCCESS;
}

int pjapp_media_vad(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->media_cfg.no_vad = enable;
	return PJ_SUCCESS;
}
int pjapp_media_quality(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value > 10)
	{
		PJ_LOG(1, (THIS_FILE, "Error: invalid --quality (expecting 0-10"));
		return ERROR;
	}
	cfg->media_cfg.quality = value;
	return PJ_SUCCESS;
}

int pjapp_media_ilbc_mode(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value != 20 && value != 30)
	{
		PJ_LOG(1,
			   (THIS_FILE, "Error: invalid --ilbc-mode (expecting 20 or 30"));
		return ERROR;
	}
	cfg->media_cfg.ilbc_mode = value;
	return PJ_SUCCESS;
}

int pjapp_media_rx_drop_pct(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (cfg->media_cfg.rx_drop_pct > 100)
	{
		PJ_LOG(1,
			   (THIS_FILE, "Error: invalid --rx-drop-pct (expecting <= 100"));
		return ERROR;
	}
	cfg->media_cfg.rx_drop_pct = value;
	return PJ_SUCCESS;
}

int pjapp_media_tx_drop_pct(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (cfg->media_cfg.tx_drop_pct > 100)
	{
		PJ_LOG(1,
			   (THIS_FILE, "Error: invalid --tx-drop-pct (expecting <= 100"));
		return ERROR;
	}
	cfg->media_cfg.tx_drop_pct = value;
	return PJ_SUCCESS;
}
int pjapp_media_echo_options(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->media_cfg.ec_options = value;
	return PJ_SUCCESS;
}

int pjapp_media_echo_tial(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value > 1000)
	{
		PJ_LOG(1,
			   (THIS_FILE, "I think the ec-tail length setting "
						   "is too big"));
		return ERROR;
	}
	cfg->media_cfg.ec_tail_len = value;
	return PJ_SUCCESS;
}

int pjapp_media_record_latency(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->media_cfg.snd_rec_latency = value;
	return PJ_SUCCESS;
}
int pjapp_media_play_latency(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->media_cfg.snd_play_latency = value;
	return PJ_SUCCESS;
}

int pjapp_media_ice_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->media_cfg.enable_ice = enable;
	return PJ_SUCCESS;
}
int pjapp_media_ice_rtcp(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->media_cfg.ice_no_rtcp = !enable;
	return PJ_SUCCESS;
}

int pjapp_media_turn_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);

	cfg->media_cfg.enable_turn = enable;
	return PJ_SUCCESS;
}
int pjapp_media_turn_server(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);

	if (cfg->media_cfg.turn_server.slen)
	{
		pj_str_clr(&cfg->media_cfg.turn_server);
	}
	pj_str_set(&cfg->media_cfg.turn_server, lval);
	return PJ_SUCCESS;
}

int pjapp_media_turn_proto(pjsua_app_config *cfg, int proto)
{
	zassert(cfg != NULL);

	cfg->media_cfg.turn_conn_type = proto;

	return PJ_SUCCESS;
}

int pjapp_media_turn_auth(pjsua_app_config *cfg, char *username, char *password, char *realm)
{
	zassert(cfg != NULL);
	zassert(username != NULL);
	cfg->media_cfg.turn_auth_cred.data.static_cred.data_type = PJ_STUN_PASSWD_PLAIN;
	pj_str_set(&cfg->media_cfg.turn_auth_cred.data.static_cred.username, username);
	pj_str_set(&cfg->media_cfg.turn_auth_cred.data.static_cred.data, password);
	pj_str_set(&cfg->media_cfg.turn_auth_cred.data.static_cred.realm, realm);
	cfg->media_cfg.turn_auth_cred.type = PJ_STUN_AUTH_CRED_STATIC;

	return PJ_SUCCESS;
}
int pjapp_media_turn_auth_tlsfilstlist(pjsua_app_config *cfg, char *filstlist)
{
	zassert(cfg != NULL);

	pj_str_set(&cfg->media_cfg.turn_tls_setting.ca_list_file, filstlist);

	return PJ_SUCCESS;
}
int pjapp_media_turn_auth_cert_file(pjsua_app_config *cfg, char *filstlist)
{
	zassert(cfg != NULL);

	pj_str_set(&cfg->media_cfg.turn_tls_setting.cert_file, filstlist);

	return PJ_SUCCESS;
}
int pjapp_media_turn_auth_privkey_file(pjsua_app_config *cfg, char *filstlist)
{
	zassert(cfg != NULL);

	pj_str_set(&cfg->media_cfg.turn_tls_setting.privkey_file, filstlist);

	return PJ_SUCCESS;
}

int pjapp_media_turn_auth_keystrlist(pjsua_app_config *cfg, char *filstlist)
{
	zassert(cfg != NULL);

	pj_str_set(&cfg->media_cfg.turn_tls_setting.ca_buf, filstlist);

	return PJ_SUCCESS;
}
int pjapp_media_turn_auth_cert_key(pjsua_app_config *cfg, char *filstlist)
{
	zassert(cfg != NULL);

	pj_str_set(&cfg->media_cfg.turn_tls_setting.cert_buf, filstlist);

	return PJ_SUCCESS;
}
int pjapp_media_turn_auth_privkey(pjsua_app_config *cfg, char *filstlist)
{
	zassert(cfg != NULL);

	pj_str_set(&cfg->media_cfg.turn_tls_setting.privkey_buf, filstlist);

	return PJ_SUCCESS;
}
int pjapp_media_turn_auth_password(pjsua_app_config *cfg, char *filstlist)
{
	zassert(cfg != NULL);

	pj_str_set(&cfg->media_cfg.turn_tls_setting.password, filstlist);

	return PJ_SUCCESS;
}
int pjapp_media_auto_close_time(pjsua_app_config *cfg, int val)
{
	zassert(cfg != NULL);
	cfg->media_cfg.snd_auto_close_time = val;
	return PJ_SUCCESS;
}
int pjapp_media_video_preview_native(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->media_cfg.vid_preview_enable_native = enable;
	return PJ_SUCCESS;
}
int pjapp_media_smart_media_update(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->media_cfg.no_smart_media_update = !enable;
	return PJ_SUCCESS;
}
int pjapp_media_rtcp_sdes_bye(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->media_cfg.no_rtcp_sdes_bye = !enable;
	return PJ_SUCCESS;
}
int pjapp_media_jb_max_size(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->media_cfg.jb_max = value;
	return PJ_SUCCESS;
}
/*********************************** buddy ****************************************/
int pjapp_buddy_add(pjsua_app_config *cfg, char *uri,  pj_bool_t enable)
{
	int i = 0;
	zassert(cfg != NULL);
	zassert(uri != NULL);
	if (pjsua_verify_url(uri) != 0)
	{
		PJ_LOG(1,
			   (THIS_FILE, "Error: invalid URL '%s' in "
						   "--add-buddy option",
				uri));
		return ERROR;
	}

	if (cfg->buddy_cnt == PJ_ARRAY_SIZE(cfg->buddy_cfg))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->buddy_cfg); i++)
	{
		if (cfg->buddy_cfg[i].uri.ptr && pj_strcmp2(&cfg->buddy_cfg[i].uri, uri) == 0)
		{
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->buddy_cfg); i++)
	{
		if (cfg->buddy_cfg[i].uri.ptr == NULL)
		{
			pj_str_set(&cfg->buddy_cfg[i].uri, uri);
			cfg->buddy_cfg[i].subscribe = enable;
			cfg->buddy_cnt++;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_buddy_del(pjsua_app_config *cfg, char *uri)
{
	int i = 0;
	zassert(cfg != NULL);
	zassert(uri != NULL);
	if (pjsua_verify_url(uri) != 0)
	{
		PJ_LOG(1,
			   (THIS_FILE, "Error: invalid URL '%s' in "
						   "--add-buddy option",
				uri));
		return ERROR;
	}

	if (cfg->buddy_cnt == PJ_ARRAY_SIZE(cfg->buddy_cfg))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->buddy_cfg); i++)
	{
		if (cfg->buddy_cfg[i].uri.ptr && pj_strcmp2(&cfg->buddy_cfg[i].uri, uri) == 0)
		{
			pj_str_clr(&cfg->buddy_cfg[i].uri);
			cfg->buddy_cnt--;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}
/********************************* global ***************************************/

int pjapp_global_ipv6(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->ipv6 = enable;
	return PJ_SUCCESS;
}
int pjapp_global_refersub(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->no_refersub = !enable;
	return PJ_SUCCESS;
}

int pjapp_global_qos_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->enable_qos = enable;
	/* Set RTP traffic type to Voice */
	cfg->rtp_cfg.qos_type = PJ_QOS_TYPE_VOICE;
	/* Directly apply DSCP value to SIP traffic. Say lets
	 * set it to CS3 (DSCP 011000). Note that this will not
	 * work on all platforms.
	 */
	//cfg->udp_cfg.qos_params.flags = PJ_QOS_PARAM_HAS_DSCP;
	//cfg->udp_cfg.qos_params.dscp_val = 0x18;
	return PJ_SUCCESS;
}
int pjapp_global_tcp_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->no_tcp = !enable;	
	return PJ_SUCCESS;
}
int pjapp_global_udp_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->no_udp = !enable;	
	return PJ_SUCCESS;
}
int pjapp_global_tls_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->use_tls = enable;
	cfg->no_tcp = !enable;	
	return PJ_SUCCESS;
}

int pjapp_global_srtp_keying_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->srtp_keying = enable;	
	return PJ_SUCCESS;
}

int pjapp_global_codec_add(pjsua_app_config *cfg, char *codec)
{
	int i = 0;
	zassert(cfg != NULL);
	if (cfg->codec_cnt == PJ_ARRAY_SIZE(cfg->codec_arg))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->codec_arg); i++)
	{
		if (cfg->codec_arg[i].ptr && pj_strcmp2(&cfg->codec_arg[i], codec) == 0)
		{
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->codec_arg); i++)
	{
		if (cfg->codec_arg[i].ptr == NULL)
		{
			pj_str_set(&cfg->codec_arg[i], codec);
			cfg->codec_cnt++;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_global_codec_del(pjsua_app_config *cfg, char *codec)
{
	int i = 0;
	zassert(cfg != NULL);
	if (cfg->codec_cnt == PJ_ARRAY_SIZE(cfg->codec_arg))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->codec_arg); i++)
	{
		if (cfg->codec_arg[i].ptr && pj_strcmp2(&cfg->codec_arg[i], codec) == 0)
		{
			pj_str_clr(&cfg->buddy_cfg[i]);
			cfg->codec_cnt--;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_global_audio_null(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->null_audio = enable;
	return PJ_SUCCESS;
}


int pjapp_global_wavfile_add(pjsua_app_config *cfg, char *name)
{
	int i = 0;
	zassert(cfg != NULL);
	if (cfg->wav_count == PJ_ARRAY_SIZE(cfg->wav_files))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->wav_files); i++)
	{
		if (cfg->wav_files[i].ptr && pj_strcmp2(&cfg->wav_files[i], name) == 0)
		{
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->wav_files); i++)
	{
		if (cfg->wav_files[i].ptr == NULL)
		{
			pj_str_set(&cfg->wav_files[i], name);
			cfg->wav_count++;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_global_wavfile_del(pjsua_app_config *cfg, char *name)
{
	int i = 0;
	zassert(cfg != NULL);
	if (cfg->wav_count == PJ_ARRAY_SIZE(cfg->wav_files))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->wav_files); i++)
	{
		if (cfg->wav_files[i].ptr && pj_strcmp2(&cfg->wav_files[i], name) == 0)
		{
			pj_str_clr(&cfg->wav_files[i]);
			cfg->wav_count--;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}


int pjapp_global_tones_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->no_tones = !enable;
	return PJ_SUCCESS;
}


int pjapp_global_tones_add(pjsua_app_config *cfg, int f1, int f2, int on, int off)
{
	int i = 0;
	zassert(cfg != NULL);
	if (cfg->tone_count == PJ_ARRAY_SIZE(cfg->tones))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->tones); i++)
	{
		if (cfg->tones[i].freq1 == f1 && cfg->tones[i].freq2 == f2 && cfg->tones[i].on_msec == on  && cfg->tones[i].off_msec == off )
		{
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->tones); i++)
	{
		if (cfg->tones[i].freq1 == 0)
		{
			cfg->tones[i].freq1 = (zpl_int16)f1;
			cfg->tones[i].freq2 = (zpl_int16)f2;
			cfg->tones[i].on_msec = (zpl_int16)on;
			cfg->tones[i].off_msec = (zpl_int16)off;
			cfg->tone_count++;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_global_tones_del(pjsua_app_config *cfg, int f1, int f2, int on, int off)
{
	int i = 0;
	zassert(cfg != NULL);
	if (cfg->tone_count == PJ_ARRAY_SIZE(cfg->tones))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->tones); i++)
	{
		if (cfg->tones[i].freq1 == f1 && cfg->tones[i].freq2 == f2 && cfg->tones[i].on_msec == on  && cfg->tones[i].off_msec == off )
		{
			cfg->tones[i].freq1 = 0;
			cfg->tones[i].freq2 = 0;
			cfg->tones[i].on_msec = 0;
			cfg->tones[i].off_msec = 0;
			cfg->tone_count++;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_global_auto_play(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	// pjsua_acc_config *cur_acc;
	// cur_acc = cfg->acc_cfg;
	cfg->auto_play = enable;
	return PJ_SUCCESS;
}

int pjapp_global_auto_play_hangup(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	// pjsua_acc_config *cur_acc;
	// cur_acc = cfg->acc_cfg;
	cfg->auto_play_hangup = enable;
	return PJ_SUCCESS;
}

int pjapp_global_auto_rec(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->auto_rec = enable;
	return PJ_SUCCESS;
}

int pjapp_global_auto_loop(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->auto_loop = enable;
	return PJ_SUCCESS;
}

int pjapp_global_auto_config(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->auto_conf = enable;
	return PJ_SUCCESS;
}

int pjapp_global_rec_file(pjsua_app_config *cfg, char *lval)
{
	zassert(cfg != NULL);
	zassert(lval != NULL);
	if (cfg->rec_file.slen)
	{
		pj_str_clr(&cfg->rec_file);
	}
	pj_str_set(&cfg->rec_file, lval);
	cfg->auto_rec = PJ_TRUE;
	return PJ_SUCCESS;
}
int pjapp_global_auto_answer(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	if (value < 100 || value > 699)
	{
		PJ_LOG(1,
			   (THIS_FILE, "Error: invalid code in --auto-answer "
						   "(expecting 100-699"));
		return ERROR;
	}
	cfg->auto_answer = value;
	return PJ_SUCCESS;
}

int pjapp_global_duration(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->duration = value;
	return PJ_SUCCESS;
}

int pjapp_global_micspeaker(pjsua_app_config *cfg, float mic, float speaker)
{
	zassert(cfg != NULL);
	cfg->mic_level = mic;
	cfg->speaker_level = speaker;
	return PJ_SUCCESS;
}

int pjapp_global_avifile_add(pjsua_app_config *cfg, char *name)
{
	int i = 0;
	zassert(cfg != NULL);
	if (cfg->avi_cnt == PJ_ARRAY_SIZE(cfg->avi))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->avi); i++)
	{
		if (cfg->avi[i].path.ptr && pj_strcmp2(&cfg->avi[i].path, name) == 0)
		{
			return ERROR;
		}
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->avi); i++)
	{
		if (cfg->avi[i].path.ptr == NULL)
		{
			pj_str_set(&cfg->avi[i].path, name);
			cfg->avi_cnt++;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_global_avifile_del(pjsua_app_config *cfg, char *name)
{
	int i = 0;
	zassert(cfg != NULL);
	if (cfg->avi_cnt == PJ_ARRAY_SIZE(cfg->avi))
	{
		PJ_LOG(1, (THIS_FILE, "Error: too many buddies in buddy list."));
		return -1;
	}
	for (i = 0; i < PJ_ARRAY_SIZE(cfg->avi); i++)
	{
		if (cfg->avi[i].path.ptr && pj_strcmp2(&cfg->avi[i].path, name) == 0)
		{
			pj_str_clr(&cfg->avi[i].path);
			cfg->avi_cnt--;
			return PJ_SUCCESS;
		}
	}
	return ERROR;
}

int pjapp_global_avi_default(pjsua_app_config *cfg, int val)
{
	zassert(cfg != NULL);
	cfg->avi_def_idx = val;
	return PJ_SUCCESS;
}

int pjapp_global_aviauto_play(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->avi_auto_play = enable;
	return PJ_SUCCESS;
}

int pjapp_global_video_auto_transmit(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->vid.out_auto_transmit = enable;
	return PJ_SUCCESS;
}

int pjapp_global_video_auto_show(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->vid.in_auto_show = enable;
	return PJ_SUCCESS;
}

int pjapp_global_capture_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->capture_dev = value;
	cfg->aud_cnt = 1;
	return PJ_SUCCESS;
}

int pjapp_global_playback_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->capture_dev = value;
	cfg->aud_cnt = 1;
	return PJ_SUCCESS;
}
int pjapp_global_capture_devname(pjsua_app_config *cfg, char* name)
{
	zassert(cfg != NULL);
	memset(cfg->capture_dev_name, 0, sizeof(cfg->capture_dev_name));
	if(name)
		strcpy(cfg->capture_dev_name, name);
	cfg->aud_cnt = 1;
	return PJ_SUCCESS;
}

int pjapp_global_playback_devname(pjsua_app_config *cfg, char* name)
{
	zassert(cfg != NULL);
	memset(cfg->playback_dev_name, 0, sizeof(cfg->playback_dev_name));
	if(name)
		strcpy(cfg->playback_dev_name, name);
	cfg->aud_cnt = 1;
	return PJ_SUCCESS;
}
int pjapp_global_capture_lat(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->capture_lat = value;
	return PJ_SUCCESS;
}

int pjapp_global_playback_lat(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->playback_lat = value;
	return PJ_SUCCESS;
}

int pjapp_global_vcapture_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->vid.vcapture_dev = value;
	cfg->vid.vid_cnt = 1;
	return PJ_SUCCESS;
}

int pjapp_global_vrender_dev(pjsua_app_config *cfg, int value)
{
	zassert(cfg != NULL);
	cfg->vid.vrender_dev = value;
	cfg->vid.vid_cnt = 1;
	return PJ_SUCCESS;
}

int pjapp_global_vcapture_devname(pjsua_app_config *cfg, char* name)
{
	zassert(cfg != NULL);
	memset(cfg->vid.vcapture_dev_name, 0, sizeof(cfg->vid.vcapture_dev_name));
	if(name)
		strcpy(cfg->vid.vcapture_dev_name, name);
	cfg->vid.vid_cnt = 1;
	return PJ_SUCCESS;
}

int pjapp_global_vrender_devname(pjsua_app_config *cfg, char* name)
{
	zassert(cfg != NULL);
	memset(cfg->vid.vrender_dev_name, 0, sizeof(cfg->vid.vrender_dev_name));
	if(name)
		strcpy(cfg->vid.vrender_dev_name, name);
	cfg->vid.vid_cnt = 1;
	return PJ_SUCCESS;
}

int pjapp_global_cli_enable(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	cfg->use_cli = enable;
	return PJ_SUCCESS;
}

int pjapp_global_cli_telnet_port(pjsua_app_config *cfg, int port)
{
	zassert(cfg != NULL);
	cfg->cli_cfg.telnet_cfg.port = port;
	cfg->cli_cfg.cli_fe |= CLI_FE_TELNET;
	return PJ_SUCCESS;
}

int pjapp_global_cli_console(pjsua_app_config *cfg, pj_bool_t enable)
{
	zassert(cfg != NULL);
	if (enable)
		cfg->cli_cfg.cli_fe &= (~CLI_FE_CONSOLE);
	else
		cfg->cli_cfg.cli_fe |= (CLI_FE_CONSOLE);
	return PJ_SUCCESS;
}

/* Set default config. */
int pjapp_config_default_setting(pjsua_app_config *appcfg)
{
	char tmp[80];
	unsigned i = 0;
	assert(appcfg != NULL);

	extern pj_bool_t pjsip_include_allow_hdr_in_dlg;
	extern pj_bool_t pjmedia_add_rtpmap_for_static_pt;

	pjsua_config_default(&appcfg->cfg);

	pj_ansi_sprintf(tmp, "PJSUA V%s", pj_get_version());
	pj_strdup2_with_null(appcfg->pool, &appcfg->cfg.user_agent, tmp);

	pjsua_logging_config_default(&appcfg->log_cfg);
	appcfg->log_cfg.cb = pjapp_cfg_log_cb;
	pj_log_set_decor(PJ_LOG_HAS_NEWLINE | PJ_LOG_HAS_CR | PJ_LOG_HAS_INDENT | PJ_LOG_HAS_THREAD_SWC |
					 PJ_LOG_HAS_SENDER | PJ_LOG_HAS_THREAD_ID);

	pjsua_media_config_default(&appcfg->media_cfg);
	appcfg->media_cfg.clock_rate = 8000;
	appcfg->media_cfg.snd_clock_rate = 8000;

	pjsua_transport_config_default(&appcfg->udp_cfg);
	appcfg->udp_cfg.port = 5060;
	pjsua_transport_config_default(&appcfg->rtp_cfg);
	appcfg->rtp_cfg.port = 40000;
	appcfg->redir_op = PJSIP_REDIRECT_ACCEPT_REPLACE;
	appcfg->duration = PJSUA_APP_NO_LIMIT_DURATION;
	appcfg->wav_id = PJSUA_INVALID_ID;
	appcfg->rec_id = PJSUA_INVALID_ID;
	appcfg->wav_port = PJSUA_INVALID_ID;
	appcfg->rec_port = PJSUA_INVALID_ID;
	appcfg->speaker_level = 1.0;
	appcfg->mic_level = 1.5;
	appcfg->capture_dev = PJSUA_INVALID_ID;
	appcfg->playback_dev = PJSUA_INVALID_ID;
	appcfg->capture_lat = PJMEDIA_SND_DEFAULT_REC_LATENCY;
	appcfg->playback_lat = PJMEDIA_SND_DEFAULT_PLAY_LATENCY;
	appcfg->ringback_slot = PJSUA_INVALID_ID;
	appcfg->ring_slot = PJSUA_INVALID_ID;

	for (i = 0; i < PJ_ARRAY_SIZE(appcfg->acc_cfg); ++i)
		pjsua_acc_config_default(&appcfg->acc_cfg[i]);

	for (i = 0; i < PJ_ARRAY_SIZE(appcfg->buddy_cfg); ++i)
		pjsua_buddy_config_default(&appcfg->buddy_cfg[i]);

	appcfg->vid.vcapture_dev = PJMEDIA_VID_DEFAULT_CAPTURE_DEV;
	appcfg->vid.vrender_dev = PJMEDIA_VID_DEFAULT_RENDER_DEV;
	appcfg->vid.vid_cnt = 0;
	appcfg->aud_cnt = 0;

	appcfg->avi_def_idx = PJSUA_INVALID_ID;

	//Media Transport Options:
	appcfg->no_refersub = PJ_TRUE;//转接通话时禁止事件订阅	
	pjapp_cfg_loose_route(appcfg, 0);

	pjsip_cfg()->endpt.use_compact_form = 0;
	/* do not transmit Allow header */
	pjsip_include_allow_hdr_in_dlg = PJ_FALSE;
	/* Do not include rtpmap for static payload types (<96) */
	pjmedia_add_rtpmap_for_static_pt = PJ_FALSE;


	appcfg->use_cli = 1;
	appcfg->cli_cfg.cli_fe = CLI_FE_TELNET;
	appcfg->cli_cfg.telnet_cfg.port = 2323;
	appcfg->codec_dis_cnt = 0;
	appcfg->codec_cnt = 0;
	pjapp_global_set_api(1);
    appcfg->cfg.max_calls = 4;
    appcfg->cfg.thread_cnt = 4;
	
	pjapp_account_add(appcfg, "100");
	pjapp_account_register_server(appcfg, "100", "192.168.10.102", 5060);
	pjapp_account_stun(appcfg, "100", 0);
	pjapp_account_media_stun(appcfg, "100", 0);

	pjapp_transport_local_port(appcfg, PJAPP_PROTO_UDP, "192.168.10.100", 5060, 0);
	//pjapp_transport_public(appcfg, PJAPP_PROTO_UDP, "192.168.10.100");
	pjapp_account_100rel(appcfg, "100", zpl_true);
	pjapp_cfg_100real(appcfg, PJSUA_100REL_MANDATORY);
	pjapp_account_register_timeout(appcfg, "100", 360);
	pjapp_global_qos_enable(appcfg, zpl_true);
	//pjapp_global_tcp_enable(appcfg, zpl_true);
	pjapp_global_udp_enable(appcfg, zpl_true);
	pjapp_account_auth_username_add(appcfg, "100", "100", "100", "*", "digest");
	pjapp_account_mwi(appcfg, "100", zpl_false);
	///pjapp_account_mwi_expires(pjsua_app_config *cfg, char *user, int mwi_expires);
	pjapp_account_publish(appcfg, "100", zpl_false);
	pjapp_account_register_refresh_delay(appcfg, "100", PJSUA_REG_RETRY_INTERVAL);
	pjapp_global_auto_config(appcfg, zpl_true);
	pjapp_media_quality(appcfg, PJSUA_DEFAULT_CODEC_QUALITY);
	pjapp_media_audio_ptime(appcfg, PJSUA_DEFAULT_AUDIO_FRAME_PTIME);
	pjapp_media_echo_tial(appcfg, PJSUA_DEFAULT_EC_TAIL_LEN);
	pjapp_media_ilbc_mode(appcfg, PJSUA_DEFAULT_ILBC_MODE);

	//PJSIP_REDIRECT_ACCEPT_REPLACE
	pjapp_global_auto_answer(appcfg, 200);
	pjapp_global_duration(appcfg, PJSUA_APP_NO_LIMIT_DURATION);
	pjapp_media_record_latency(appcfg, PJMEDIA_SND_DEFAULT_REC_LATENCY);
	pjapp_media_play_latency(appcfg, PJMEDIA_SND_DEFAULT_PLAY_LATENCY);

	//pjapp_global_audio_null(appcfg, zpl_true);
	pjapp_global_codec_add(appcfg, "pcmu");
	pjapp_global_codec_add(appcfg, "pcma");
	_pjAppCfg.aud_cnt++; 
	
	return PJ_SUCCESS;
}

int pjapp_cfg_log_config(pjsua_app_config *cfg)
{
	pjapp_cfg_log_level(cfg, 6);
	pjapp_cfg_app_log_level(cfg, 6);
	pj_log_set_log_func(pjapp_cfg_log_cb);
	return PJ_SUCCESS;
}

pj_bool_t pjapp_global_isenable(void)
{
	pj_bool_t enable = 0;
	PJAPP_GLOBAL_LOCK();
	enable = _pjAppCfg.global_enable;
	PJAPP_GLOBAL_UNLOCK();
	return enable;
}
int pjapp_global_set_api(pj_bool_t enable)
{
	PJAPP_GLOBAL_LOCK();
	_pjAppCfg.global_enable = enable;
	PJAPP_GLOBAL_UNLOCK();
	return PJ_SUCCESS;
}

int pjapp_config_init(void)
{
	if(_pjAppCfg.initialization == 0)
	{
		memset(&_pjAppCfg, 0, sizeof(pjsua_app_config));
		_pjAppCfg.initialization = 1;
	}
	return PJ_SUCCESS;
}