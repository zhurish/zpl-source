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
#include "pjsua_app_common.h"
#include "pjsua_app.h"
#include "pjsip_app_api.h"

#include "auto_include.h"
#include <zplos_include.h>
#include "lib_include.h"
#include "nsm_include.h"
#include "vty_include.h"

#define THIS_FILE	"pjsip_main.c"

extern zpl_bool host_waitting_loadconfig(void);


static int pjapp_task_load(void *p);

/* Called when CLI (re)started */
static void pjapp_on_started(pj_status_t status, const char *msg)
{
	//PJ_LOG(5, ("main", "--------------on_app_started-----------"));
    pj_perror(3, THIS_FILE, status, (msg)?msg:"");
}

static void pjapp_on_stopped(pj_bool_t restart, int argc, char** argv)
{
/*    if (argv) {
	_global_config.app_cfg.argc = argc;
	_global_config.app_cfg.argv = argv;
    }*/
    //PJ_LOG(5, ("main", "--------------on_app_stopped-----------"));
    //pj_perror(3, THIS_FILE, status, (msg)?msg:"","on_app_started");
    _global_config.app_running = restart;
}

static int pjapp_main_func(int argc, char *argv[])
{
	pj_status_t status = PJ_TRUE;

	pj_bzero(&_global_config.app_cfg, sizeof(_global_config.app_cfg));
	_global_config.app_cfg.on_started = &pjapp_on_started;
	_global_config.app_cfg.on_stopped = &pjapp_on_stopped;
	//_global_config.app_cfg.argc = argc;
	//_global_config.app_cfg.argv = argv;
	_global_config.app_running = PJ_TRUE;

	while (_global_config.app_running && OS_TASK_TRUE())
	{
		//__ZPL_PJSIP_DEBUG( "%s:pjsua_app_init","main_func");
		status = pjsua_app_init(&_global_config.app_cfg);
		if (status == PJ_SUCCESS)
		{
			os_task_foreach(pjapp_task_load, NULL);
			status = pjsua_app_run(PJ_TRUE);
		}
		else
		{
			_global_config.app_running = PJ_FALSE;
		}
		if (status == PJ_SUCCESS)
			break;
		//__ZPL_PJSIP_DEBUG("======================================================3\r\n");

		pjsua_app_destroy();
		//__ZPL_PJSIP_DEBUG("======================================================4\r\n");

		if(_global_config.restart)
		{
			_global_config.app_running = PJ_TRUE;
		}
	}
	return 0;
}


static int pjapp_task_load(void *p)
{
	os_task_t *task = p;
	if(task)
	{
		if(task->active && task->priv == NULL && task->td_thread)
		{
			task->priv = pj_thread_register_malloc(task->td_thread, task->td_name);
		}
	}
	return OK;
}


static void * pjapp_task_self(pthread_t td_thread)
{
	void *p = os_task_priv_get(0, os_task_pthread_self());
	//printf("===============%s======== p=%u=========\r\n", __func__, p);
	return p;
}

static int pjapp_task_add(char *name, int pri, int op, void *entry, void *arg, int stacksize,
		int td_thread, void *p)
{
	//int Priority = 0;
	os_task_add_name(name, pri, 0, entry, name, arg, stacksize, td_thread);
	//os_task_priority_get(id, &Priority);
	//__ZPL_PJSIP_DEBUG("===============%s=================:%s\r\n", __func__, name);
	if (os_task_priv_get(0, td_thread) == NULL)
		os_task_priv_set(0, td_thread, p);
	return 0;
}

static int _pjapp_task_create(void *p)
{
	os_task_t *task = p;
	if(task && strstr(task->td_name, "alsa"))
	{
		_global_config.media_quit++;
		//__ZPL_PJSIP_DEBUG( "==============%s===========%d", __func__, app_config.media_quit);
	}
	return OK;
}

static int _pjapp_task_destroy(void *p)
{
	os_task_t *task = p;
	if(task && strstr(task->td_name, "alsa"))
	{
		_global_config.media_quit--;
		//__ZPL_PJSIP_DEBUG( "==============%s===========%d", __func__, app_config.media_quit);
	}
	return OK;
}

static int pjapp_wait_quit(void)
{
	//__ZPL_PJSIP_DEBUG( "==============%s===========%d", __func__, app_config.media_quit);
	while(_global_config.media_quit > 0)
	{
		os_msleep(50);
	}
	os_msleep(500);
	return OK;
}

static int pjMainTask(void *p)
{
	int pjargc = 1;
	char *pjargv[] = {NULL, NULL};

	host_waitting_loadconfig();

	_global_config.app_running = PJ_TRUE;

	os_task_add_create_hook(_pjapp_task_create);
	os_task_add_destroy_hook(_pjapp_task_destroy);
	pj_init();
	pj_task_cb_init(pjapp_task_add, os_task_thread_del, os_task_thread_refresh_id, pjapp_task_self);

    return pj_run_app(&pjapp_main_func, pjargc, pjargv, 0);
}



static int pjapp_module_init(void)
{
	if(_pjapp_cfg == NULL)
		_pjapp_cfg = XMALLOC(MTYPE_VOIP, sizeof(pjapp_cfg_t));
	if(!_pjapp_cfg)
		return ERROR;
	os_memset(_pjapp_cfg, 0, sizeof(pjapp_cfg_t));
	_pjapp_cfg->mutex = os_mutex_name_create("_pjapp_cfg->mutex");
	pjapp_cfg_config_default(_pjapp_cfg);

	return OK;
}


static int pjapp_module_exit(void)
{
	pjsua_app_exit();
	pjapp_wait_quit();
	pjsua_app_destroy();
	memset(&_global_config, 0, sizeof(_global_config));
	if(_pjapp_cfg == NULL)
		XFREE(MTYPE_VOIP, _pjapp_cfg);
	_pjapp_cfg = NULL;
	return OK;
}

static int pjapp_module_task_init(void)
{
	return os_task_create("pjMainTask", OS_TASK_DEFAULT_PRIORITY,
	               0, pjMainTask, NULL, OS_TASK_DEFAULT_STACK*4);
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