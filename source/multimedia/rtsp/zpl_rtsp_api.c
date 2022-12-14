/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "zpl_rtsp_def.h"
#include "zpl_rtsp_session.h"
#include "zpl_rtsp_media.h"
#include "zpl_rtsp_client.h"
#include "zpl_rtsp_server.h"
#ifdef ZPL_BUILD_LINUX
#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"
#include "vty_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#endif

static rtsp_srv_t *rtsp_srv = NULL;
static zpl_uint32 rtsp_srv_taskid = 0;



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
#endif
    return OK;
}

int rtsp_module_init()
{
#ifdef ZPL_WORKQUEUE
    rtsp_srv = rtsp_srv_create(NULL, 9554, 33);
#else
    rtsp_srv = rtsp_srv_create(NULL, 9554);
#endif
    if(rtsp_srv)
        rtsp_rtp_init();
    return OK;
}

int rtsp_module_exit()
{
    if(rtsp_srv)
    {
        rtsp_srv_destroy(rtsp_srv);
        ortp_exit();
    }
    return OK;
}

int rtsp_module_task_init()
{
    if(rtsp_srv)
    {
        rtsp_rtp_start();
        if(rtsp_srv_taskid == 0)
            pthread_create(&rtsp_srv_taskid, NULL, zpl_media_rtsp_task, (void *) NULL);
        return OK;
    }
    return OK;
}

int rtsp_module_task_exit()
{
    return OK;
}