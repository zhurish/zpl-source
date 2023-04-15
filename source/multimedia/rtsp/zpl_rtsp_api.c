﻿/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "host.h"
#undef ZPL_LIVE555_MODULE
#ifdef ZPL_LIVE555_MODULE
#include "livertsp_server.h"
#endif
#ifdef ZPL_LIBORTP_MODULE
#include <ortp/ortp.h>
#endif
#include "zpl_rtsp.h"
#include "zpl_rtsp_util.h"
#include "zpl_rtsp_transport.h"
#include "zpl_rtsp_sdp.h"
#include "zpl_rtsp_sdpfmtp.h"
#include "zpl_rtsp_auth.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_client.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_server.h"
#include "zpl_rtsp_api.h"




static rtsp_server_t rtsp_server;

struct module_list module_list_rtsp = {
		.module = MODULE_RTSP,
		.name = "ZPLRTSP\0",
		.module_init = rtsp_module_init,
		.module_exit = rtsp_module_exit,
		.module_task_init = rtsp_module_task_init,
		.module_task_exit = rtsp_module_task_exit,
		.module_cmd_init = NULL,
		.taskid = 0,
};

static int rtsp_main_task(void* argv)
{   
#ifdef ZPL_LIVE555_MODULE
    host_waitting_loadconfig();
    livertsp_server_loop(argv);
#endif    
#ifdef ZPL_LIBORTP_MODULE
    if(rtsp_server.rtsp_srv)
    {
		rtsp_server.rtsp_srv->t_master = eloop_master_module_create(MODULE_RTSP);
        host_waitting_loadconfig();
        eloop_mainloop(rtsp_server.rtsp_srv->t_master);
    }
#endif
    return OK;
}
#ifdef ZPL_LIVE555_MODULE
static int rtsp_logcb(const char *fmt,...)
{
    char lbuf[1024];
    int len = 0;
    va_list args;
    va_start (args, fmt);    
    memset(lbuf, 0, sizeof(lbuf));
    len += vsnprintf(lbuf+len, sizeof(lbuf)-len, fmt, args);
    va_end (args);
    zlog_debug(MODULE_RTSP,"%s\r\n", lbuf);
    //fprintf(stdout, "%s\r\n", lbuf);
    //fflush(stdout);
    return OK;
}
#endif
int rtsp_module_init(void)
{
#ifdef ZPL_LIVE555_MODULE
    rtsp_srv = malloc(sizeof(rtsp_srv_t));
    livertsp_server_init(8554, BASEUSAGEENV_BASE_DIR, rtsp_logcb);
#endif    
#ifdef ZPL_LIBORTP_MODULE
    rtsp_server.t_master = eloop_master_module_create(MODULE_RTSP);
    rtsp_server.rtsp_srv = rtsp_srv_create(rtsp_server.t_master, NULL, 554, MODULE_RTSP);
#endif    
    return OK;
}

int rtsp_module_exit(void)
{
#ifdef ZPL_LIVE555_MODULE
    livertsp_server_exit();
#endif    
#ifdef ZPL_LIBORTP_MODULE
    if(rtsp_server.rtsp_srv)
    {
        rtsp_srv_destroy(rtsp_server.rtsp_srv);
    }
#endif    
    return OK;
}

int rtsp_module_task_init(void)
{
    if(rtsp_server.rtsp_srv)
    {
		rtsp_server.t_taskid = os_task_create("rtspTask", OS_TASK_DEFAULT_PRIORITY,
								 0, rtsp_main_task, NULL, OS_TASK_DEFAULT_STACK*8);
        return OK;
    }
    return OK;
}

int rtsp_module_task_exit(void)
{
    if(rtsp_server.rtsp_srv)
    {
        if(rtsp_server.t_taskid)
		    os_task_destroy(rtsp_server.t_taskid);
	    rtsp_server.t_taskid = 0;
    }
    return OK;
}