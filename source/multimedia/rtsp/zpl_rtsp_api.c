/*
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
#include "zpl_rtsp.h"
#include "zpl_rtsp_util.h"
#include "zpl_rtsp_transport.h"
#include "zpl_rtsp_sdp.h"
#include "zpl_rtsp_sdpfmtp.h"
#include "zpl_rtsp_base64.h"
#include "zpl_rtsp_auth.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_client.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_adap.h"
#include "zpl_rtsp_server.h"
#include "zpl_rtsp_rtp.h"
#include "zpl_rtsp_api.h"

static rtsp_srv_t *rtsp_srv = NULL;
#ifndef ZPL_WORKQUEUE
static zpl_uint32 rtsp_srv_taskid = 0;
#endif

#ifdef ZPL_WORKQUEUE
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
#endif
static int zpl_media_rtsp_task(void* argv)
{
#ifndef ZPL_WORKQUEUE
    if(rtsp_srv)
    {
        while(1)
        {
            rtsp_srv_select(rtsp_srv);
        }
    }
#else    
    if(rtsp_srv)
    {
		rtsp_srv->t_master = eloop_master_module_create(MODULE_RTSP);
        host_waitting_loadconfig();
        eloop_mainloop(rtsp_srv->t_master);
    }

#endif
    return OK;
}

int rtsp_module_init(void)
{
#ifdef ZPL_WORKQUEUE
    rtsp_srv = rtsp_srv_create(NULL, 554, MODULE_RTSP);
#else
    rtsp_srv = rtsp_srv_create(NULL, 9554);
#endif
    if(rtsp_srv)
    {
        #ifdef ZPL_WORKQUEUE
        rtsp_srv->t_master = eloop_master_module_create(MODULE_RTSP);
        #endif
        rtsp_rtp_init();
    }
    return OK;
}

int rtsp_module_exit(void)
{
    if(rtsp_srv)
    {
        rtsp_srv_destroy(rtsp_srv);
        ortp_exit();
    }
    return OK;
}

int rtsp_module_task_init(void)
{
    if(rtsp_srv)
    {
        //rtsp_rtp_start();
#ifdef ZPL_WORKQUEUE
		rtsp_srv->t_taskid = os_task_create("rtspTask", OS_TASK_DEFAULT_PRIORITY,
								 0, zpl_media_rtsp_task, NULL, OS_TASK_DEFAULT_STACK*2);
#else
        if(rtsp_srv_taskid == 0)
            pthread_create(&rtsp_srv_taskid, NULL, zpl_media_rtsp_task, (void *) NULL);
#endif
        rtsp_rtp_start();
        return OK;
    }
    return OK;
}

int rtsp_module_task_exit(void)
{
    return OK;
}