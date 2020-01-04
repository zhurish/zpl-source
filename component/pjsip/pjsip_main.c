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
#include "pjsua_app.h"
#include "pjsip_main.h"
#include "pjsip_app_api.h"
#ifdef PL_APP_MODULE
#include "voip_app.h"
#endif
#define THIS_FILE	"pjsip_main.c"


static pjsua_app_cfg_t	    cfg;

static int pl_pjsip_task_load(void *p);

/* Called when CLI (re)started */
static void on_app_started(pj_status_t status, const char *msg)
{
	//PJ_LOG(5, ("main", "--------------on_app_started-----------"));
    pj_perror(3, THIS_FILE, status, (msg)?msg:"");
}

static void on_app_stopped(pj_bool_t restart, int argc, char** argv)
{
/*    if (argv) {
	cfg.argc = argc;
	cfg.argv = argv;
    }*/
    //PJ_LOG(5, ("main", "--------------on_app_stopped-----------"));
    //pj_perror(3, THIS_FILE, status, (msg)?msg:"","on_app_started");
    cfg.running = restart;
}

static int main_func(int argc, char *argv[])
{
	pj_status_t status = PJ_TRUE;

	pj_bzero(&cfg, sizeof(cfg));
	cfg.on_started = &on_app_started;
	cfg.on_stopped = &on_app_stopped;
	//cfg.argc = argc;
	//cfg.argv = argv;
	cfg.running = PJ_TRUE;
	//app_config.app_cli_running = PJ_TRUE;

	while (cfg.running)
	{
		//__PL_PJSIP_DEBUG( "%s:pjsua_app_init","main_func");
		status = pjsua_app_init(&cfg);
		if (status == PJ_SUCCESS)
		{
			os_task_foreach(pl_pjsip_task_load, NULL);
			status = pjsua_app_run(PJ_TRUE);
		}
		else
		{
			cfg.running = PJ_FALSE;
		}
		//__PL_PJSIP_DEBUG("======================================================3\r\n");

		pjsua_app_destroy();
		//__PL_PJSIP_DEBUG("======================================================4\r\n");

		if(cfg.restart)
		{
			cfg.running = PJ_TRUE;
		}
	}
	return 0;
}


static int pl_pjsip_task_load(void *p)
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


static void * pl_pjsip_task_self(pthread_t td_thread)
{
	void *p = os_task_priv_get(0, os_task_pthread_self());
	//printf("===============%s======== p=%u=========\r\n", __func__, p);
	return p;
}

static int pl_pjsip_task_add(char *name, int pri, int op, void *entry, void *arg, int stacksize,
		int td_thread, void *p)
{
	//int Priority = 0;
	os_task_add_name(name, pri, 0, entry, name, arg, stacksize, td_thread);
	//os_task_priority_get(id, &Priority);
	//__PL_PJSIP_DEBUG("===============%s=================:%s\r\n", __func__, name);
	if (os_task_priv_get(0, td_thread) == NULL)
		os_task_priv_set(0, td_thread, p);
	return 0;
}

static int _pl_pjsip_task_create(void *p)
{
	os_task_t *task = p;
	if(task && strstr(task->td_name, "alsa"))
	{
		cfg.media_quit++;
		//__PL_PJSIP_DEBUG( "==============%s===========%d", __func__, cfg.media_quit);
	}
	return OK;
}

static int _pl_pjsip_task_destroy(void *p)
{
	os_task_t *task = p;
	if(task && strstr(task->td_name, "alsa"))
	{
		cfg.media_quit--;
		//__PL_PJSIP_DEBUG( "==============%s===========%d", __func__, cfg.media_quit);
	}
	return OK;
}

int pjsip_media_wait_quit(void)
{
	//__PL_PJSIP_DEBUG( "==============%s===========%d", __func__, cfg.media_quit);
	while(cfg.media_quit > 0)
	{
		os_msleep(50);
	}
	os_msleep(500);
	return OK;
}

static int pjmain(void *p)
{
	int pjargc = 1;
	char *pjargv[] = {NULL, NULL};

/*	os_task_cb_tbl_t cb;

	cb.cb_start = pl_pjsip_task_load;
	os_task_cb_install(&cb);
*/
//	pj_task_cb_init(pl_pjsip_task_add, os_task_del, os_task_refresh_id, pl_pjsip_task_self);
//	cli_callback_init(0, pl_pjsip_account_set_api);

	while(!os_load_config_done())
	{
		os_sleep(1);
	}
#ifdef PL_OPENWRT_UCI
	pl_pjsip_module_reload();
#endif
	cfg.running = PJ_TRUE;

	os_task_add_create_hook(&_pl_pjsip_task_create);
	os_task_add_destroy_hook(&_pl_pjsip_task_destroy);

	pj_task_cb_init(pl_pjsip_task_add, os_task_del, os_task_refresh_id, pl_pjsip_task_self);

    return pj_run_app(&main_func, pjargc, pjargv, 0);
}



int pjsip_module_init()
{
	memset(&app_config, 0, sizeof(app_config));
	return OK;
}

int pjsip_module_exit()
{
	memset(&app_config, 0, sizeof(app_config));
	return OK;
}

int pjsip_module_task_init()
{
	return os_task_create("pjMainTask", OS_TASK_DEFAULT_PRIORITY,
	               0, pjmain, NULL, OS_TASK_DEFAULT_STACK*4);

	pjmain(NULL);
	return OK;
}

int pjsip_module_task_exit()
{
	return OK;
}
#if 0
/*
 * main()
 *
 * argv[1] may contain URL to call.
 */
int main(int argc, char *argv[])
{
    pjsua_acc_id acc_id;
    pj_status_t status;

    /* Create pjsua first! */
    status = pjsua_create();
    if (status != PJ_SUCCESS) error_exit("Error in pjsua_create()", status);

    /* If argument is specified, it's got to be a valid SIP URL */
    if (argc > 1) {
        status = pjsua_verify_url(argv[1]);
        if (status != PJ_SUCCESS) error_exit("Invalid URL in argv", status);
    }

    /* Init pjsua */
    {
        pjsua_config cfg;
        pjsua_logging_config log_cfg;

        pjsua_config_default(&cfg);
        cfg.cb.on_incoming_call = &on_incoming_call;
        cfg.cb.on_call_media_state = &on_call_media_state;
        cfg.cb.on_call_state = &on_call_state;

        pjsua_logging_config_default(&log_cfg);
        log_cfg.console_level = 4;

        status = pjsua_init(&cfg, &log_cfg, NULL);
        if (status != PJ_SUCCESS) error_exit("Error in pjsua_init()", status);
    }

    /* Add UDP transport. */
    {
        pjsua_transport_config cfg;

        pjsua_transport_config_default(&cfg);
        cfg.port = 5060;
        status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &cfg, NULL);
        if (status != PJ_SUCCESS) error_exit("Error creating transport", status);
    }

    /* Initialization is done, now start pjsua */
    status = pjsua_start();
    if (status != PJ_SUCCESS) error_exit("Error starting pjsua", status);

    /* Register to SIP server by creating SIP account. */
    {
        pjsua_acc_config cfg;

        pjsua_acc_config_default(&cfg);
        cfg.id = pj_str("sip:" SIP_USER "@" SIP_DOMAIN);
        cfg.reg_uri = pj_str("sip:" SIP_DOMAIN);
        cfg.cred_count = 1;
        cfg.cred_info[0].realm = pj_str(SIP_DOMAIN);
        cfg.cred_info[0].scheme = pj_str("digest");
        cfg.cred_info[0].username = pj_str(SIP_USER);
        cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
        cfg.cred_info[0].data = pj_str(SIP_PASSWD);

        status = pjsua_acc_add(&cfg, PJ_TRUE, &acc_id);
        if (status != PJ_SUCCESS) error_exit("Error adding account", status);
    }

    /* If URL is specified, make call to the URL. */
    if (argc > 1) {
        pj_str_t uri = pj_str(argv[1]);
        status = pjsua_call_make_call(acc_id, &uri, 0, NULL, NULL, NULL);
        if (status != PJ_SUCCESS) error_exit("Error making call", status);
    }

    /* Wait until user press "q" to quit. */
    for (;;) {
        char option[10];

        puts("Press 'h' to hangup all calls, 'q' to quit");
        if (fgets(option, sizeof(option), stdin) == NULL) {
            puts("EOF while reading stdin, will quit now..");
            break;
        }

        if (option[0] == 'q')
            break;

        if (option[0] == 'h')
            pjsua_call_hangup_all();
    }

    /* Destroy pjsua */
    pjsua_destroy();

    return 0;
}

//有来电时的通知回调函数：
/* Callback called by the library upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
                             pjsip_rx_data *rdata)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &ci);

    PJ_LOG(3,(THIS_FILE, "Incoming call from %.*s!!",
                         (int)ci.remote_info.slen,
                         ci.remote_info.ptr));

    /* Automatically answer incoming calls with 200/OK */
    pjsua_call_answer(call_id, 200, NULL, NULL);
}
//电话状态通知回调函数：
/* Callback called by the library when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
    pjsua_call_info ci;

    PJ_UNUSED_ARG(e);

    pjsua_call_get_info(call_id, &ci);
    PJ_LOG(3,(THIS_FILE, "Call %d state=%.*s", call_id,
                         (int)ci.state_text.slen,
                         ci.state_text.ptr));
}
//通话建立连接时，语音、视频等的相关状态通知回调函数：
/* Callback called by the library when call's media state has changed */
static void on_call_media_state(pjsua_call_id call_id)
{
    pjsua_call_info ci;

    pjsua_call_get_info(call_id, &ci);

    if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
        // When media is active, connect call to sound device.
        pjsua_conf_connect(ci.conf_slot, 0);
        pjsua_conf_connect(0, ci.conf_slot);
    }
}
#endif
