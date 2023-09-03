/*
 * pjsip_main.c
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <pjsua-lib/pjsua.h>
#include "pjsua_app_common.h"
#include "pjsua_app_cfgapi.h"
#include "pjsua_app_config.h"
#include "pjsua_app_cb.h"
#include "pjsua_app.h"

#include "auto_include.h"
#include "lib_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"

#define THIS_FILE	"pjsip_main.c"

extern zpl_bool host_waitting_loadconfig(void);

struct pjapp_thread_desc 
{
    pj_thread_desc       _thread_desc;
    pj_thread_t         *_thread;
};
extern int (*pj_thread_register_cb)(void);
/*
static int pjapp_task_load(void *p)
{
	struct pjapp_thread_desc *desc;
	os_task_t *task = p;
	if(task)
	{
		if(task->active && task->priv == NULL && task->td_thread)
		{
			task->priv = malloc(sizeof(struct pjapp_thread_desc));
			if(task->priv)
			{
				desc = (struct pjapp_thread_desc *)task->priv;
				pj_bzero(task->priv, sizeof(struct pjapp_thread_desc));
				pj_thread_register(task->td_name, desc->_thread_desc, &desc->_thread);
			}
		}
	}
	return OK;
}
*/

static int pj_thread_register_cbaction(void)
{
	struct pjapp_thread_desc *desc;
	os_task_t *task = os_task_self();
	if(task)
	{
		if(task->active && task->priv == NULL && task->td_thread)
		{
			task->priv = (struct pjapp_thread_desc *)malloc(sizeof(struct pjapp_thread_desc));
			if(task->priv)
			{
				desc = (struct pjapp_thread_desc *)task->priv;
				pj_bzero(task->priv, sizeof(struct pjapp_thread_desc));
				pj_thread_register(task->td_name, desc->_thread_desc, &desc->_thread);
			}
		}
	}
	return OK;
}


static int pjMainTask(void *p)
{
	pj_status_t status = PJ_TRUE;

	host_waitting_loadconfig();

	_pjAppCfg.app_running = PJ_TRUE;

	pj_init();
	pj_thread_register_cb = pj_thread_register_cbaction;

	while (_pjAppCfg.app_running && OS_TASK_TRUE())
	{
		#ifdef ZPL_HISIMPP_MODULE
		if(_pjAppCfg.app_start == PJ_FALSE)
		{
			sleep(1);
			continue;
		}
		#endif
		status = pjsua_app_init();
		if (status == PJ_SUCCESS)
		{
			status = pjsua_app_run(PJ_TRUE);
		}
		else
		{
			_pjAppCfg.app_running = PJ_FALSE;
		}
		if (status == PJ_SUCCESS)
			break;
		zm_msg_force_trap("======================================================3\r\n");

		pjsua_app_destroy();
		zm_msg_force_trap("======================================================4\r\n");

		if(_pjAppCfg.restart)
		{
			_pjAppCfg.app_running = PJ_TRUE;
		}
	}
	return 0;
}



static int pjapp_module_init(void)
{/*
	if(_pjAppCfg == NULL)
		_pjAppCfg = XMALLOC(MTYPE_VOIP, sizeof(pjapp_cfg_t));
	if(!_pjAppCfg)
		return ERROR;
	os_memset(_pjAppCfg, 0, sizeof(pjapp_cfg_t));
	_pjAppCfg->mutex = os_mutex_name_create("_pjAppCfg->mutex");
	pjapp_cfg_config_default(_pjAppCfg);
*/
	pjapp_config_init();
	return OK;
}


static int pjapp_module_exit(void)
{
	pjsua_app_exit();
	pjsua_app_destroy();
	memset(&_pjAppCfg, 0, sizeof(pjsua_app_config));

	return OK;
}

static int pjapp_module_task_init(void)
{
	return os_task_create("pjMainTask", OS_TASK_DEFAULT_PRIORITY,
	               0, pjMainTask, NULL, OS_TASK_DEFAULT_STACK*16);
}

static int pjapp_module_task_exit(void)
{
	return OK;
}



struct module_list module_list_pjsip = 
{ 
	.module=MODULE_PJAPP, 
	.name="PJSIP\0", 
	.module_init=pjapp_module_init, 
	.module_exit=pjapp_module_exit, 
	.module_task_init=pjapp_module_task_init, 
	.module_task_exit=pjapp_module_task_exit, 
	.module_cmd_init=pjapp_cmd_init, 
	.taskid=0,
};