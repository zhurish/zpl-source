/*
 *  Copyright (C) 2008-2009 Andrej Stepanchuk
 *  Copyright (C) 2009-2010 Howard Chu
 *
 *  This file is part of librtmp.
 *
 *  librtmp is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1,
 *  or (at your option) any later version.
 *
 *  librtmp is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with librtmp see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *  http://www.gnu.org/copyleft/lgpl.html
 */

#ifndef __ZPL_RTMP_SRV_H__
#define __ZPL_RTMP_SRV_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTMP_CLIENT_MAX   16
enum
{
    RTMP_STATE_ACCEPTING,
    RTMP_STATE_IN_PROGRESS,
    RTMP_STATE_STOPPING,
    RTMP_STATE_STOPPED
};


typedef struct
{
    int sock;
    int state;
    int stream_ID;
    void *rtmp;

    uint32_t lasttime;	/* time of last download we started */
    char *filename;
    char *rtspurl;
    char *httpurl;
    char *address;

    uint32_t file_offset;

    void *t_read;
    void *t_write;
    void *t_time;

} rtmp_client_t;


typedef struct
{
    int sock;
    int state;
    char *address;
    int rtmpport;
    int protocol;
    int bLiveStream;		// is it a live stream? then we can't seek/resume

    char *rtmpurl;
    int rtmp_client_num;
    rtmp_client_t *rtmp_client[RTMP_CLIENT_MAX];

    void *t_read;
    void *t_master;
} rtmp_server_t;









#ifdef __cplusplus
}
#endif

#endif /* __ZPL_RTMP_SRV_H__ */
