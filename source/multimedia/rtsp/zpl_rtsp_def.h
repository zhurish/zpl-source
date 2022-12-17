/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __RTSP_DEF_H__
#define __RTSP_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"
#include "zplos_include.h"
#include "lib_include.h"

#if defined(_MSC_VER)
# if defined(DLLBUILD)
/* define DLLBUILD when building the DLL */
#  define RTSP_API __declspec(dllexport)
# else
#  define RTSP_API extern //__declspec(dllimport)
# endif
#else
# define RTSP_API extern
#endif



#ifndef max
#define max(a,b)    (a)>(b)?(a):(b)
#endif

#if defined(_WIN32)
#pragma warning(disable : 2099)
#endif



#ifdef __cplusplus
}
#endif
#endif /* __RTSP_DEF_H__ */
