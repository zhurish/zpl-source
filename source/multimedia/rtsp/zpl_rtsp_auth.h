/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __RTSP_AUTH_H__
#define __RTSP_AUTH_H__

#include "zpl_rtsp_def.h"
#include "zpl_rtsp.h"

RTSP_BEGIN_DECLS

typedef enum {
    RTSP_AUTH_NONE = 0,
    RTSP_AUTH_BASIC,
    RTSP_AUTH_DIGEST,
    RTSP_AUTH_TEXT,
}rtsp_auth_mode_t;

typedef enum {
    RTSP_ALGORITHM_NONE = 0,
    RTSP_ALGORITHM_MD5,
    RTSP_ALGORITHM_SHA256,
}rtsp_algorithm_t;

typedef struct rtsp_auth_s
{
    rtsp_auth_mode_t    scheme; // HTTP_AUTHENTICATION_BASIC, HTTP_AUTHENTICATION_DIGEST
    char                *realm; // case-sensitive
    char                *domain;
    rtsp_algorithm_t    algorithm;
    char                *username;
    char                *password;
    char                *nonce;
}rtsp_auth_t;


RTSP_API int rtsp_auth_init(rtsp_auth_t*);
RTSP_API int rtsp_auth_set_scheme_mode(rtsp_auth_t* auth, rtsp_auth_mode_t mode);
RTSP_API int rtsp_auth_set_algorithm(rtsp_auth_t* auth, rtsp_algorithm_t mode);
RTSP_API int rtsp_auth_set_realm(rtsp_auth_t* auth, char * val);
RTSP_API int rtsp_auth_set_domain(rtsp_auth_t* auth, char * val);
RTSP_API int rtsp_auth_set_nonce(rtsp_auth_t* auth, char * val);
RTSP_API int rtsp_auth_set_username(rtsp_auth_t* auth, char * val);
RTSP_API int rtsp_auth_set_password(rtsp_auth_t* auth, char * val);
RTSP_API int rtsp_auth_getbase(rtsp_auth_t*);
RTSP_API int rtsp_auth_getdigest(rtsp_auth_t*);
RTSP_API int rtsp_auth_destroy(rtsp_auth_t*);

RTSP_END_DECLS

#endif /* __RTSP_AUTH_H__ */
