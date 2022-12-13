/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "zpl_rtsp_def.h"
#ifdef ZPL_LIBRTSP_MODULE
#include "zpl_media.h"
#include "zpl_media_internal.h"
#endif
#include "zpl_rtsp_sdp.h"
#include "zpl_rtsp_auth.h"


int rtsp_auth_init(rtsp_auth_t* auth)
{
    memset(auth, 0, sizeof(rtsp_auth_t));
    return OK;
}


int rtsp_auth_set_scheme_mode(rtsp_auth_t* auth, rtsp_auth_mode_t mode)
{
    auth->scheme = mode;
    return OK;
}

int rtsp_auth_set_algorithm(rtsp_auth_t* auth, rtsp_algorithm_t mode)
{
    auth->algorithm = mode;
    return OK;
}

int rtsp_auth_set_realm(rtsp_auth_t* auth, char * val)
{
    if(auth->realm)
    {
        free(auth->realm);
        auth->realm = NULL;
    }
    if(val)
        auth->realm = strdup(val);
    return OK;
}

int rtsp_auth_set_domain(rtsp_auth_t* auth, char * val)
{
    if(auth->domain)
    {
        free(auth->domain);
        auth->domain = NULL;
    }
    if(val)
        auth->domain = strdup(val);
    return OK;
}

int rtsp_auth_set_nonce(rtsp_auth_t* auth, char * val)
{
    if(auth->nonce)
    {
        free(auth->nonce);
        auth->nonce = NULL;
    }
    if(val)
        auth->nonce = strdup(val);
    return OK;
}

int rtsp_auth_set_username(rtsp_auth_t* auth, char * val)
{
    if(auth->username)
    {
        free(auth->password);
        auth->password = NULL;
    }
    if(val)
        auth->username = strdup(val);
    return OK;
}
int rtsp_auth_set_password(rtsp_auth_t* auth, char * val)
{
    if(auth->password)
    {
        free(auth->password);
        auth->password = NULL;
    }
    if(val)
        auth->password = strdup(val);
    return OK;
}


int rtsp_auth_getbase(rtsp_auth_t* auth)
{
    return OK;
}

int rtsp_auth_getdigest(rtsp_auth_t* auth)
{
    return OK;
}

int rtsp_auth_destroy(rtsp_auth_t* auth)
{
    if(auth->realm)
    {
        free(auth->realm);
        auth->realm = NULL;
    }
    if(auth->domain)
    {
        free(auth->domain);
        auth->domain = NULL;
    }
    if(auth->username)
    {
        free(auth->username);
        auth->username = NULL;
    }
    if(auth->password)
    {
        free(auth->password);
        auth->password = NULL;
    }
    if(auth->nonce)
    {
        free(auth->nonce);
        auth->nonce = NULL;
    }
    return OK;
}
