/* $Id$ */
/*
 * Copyright (C) 2008-2013 Teluu Inc. (http://www.teluu.com)
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
#ifndef __PJSUA_MEDIA_FILE_H__
#define __PJSUA_MEDIA_FILE_H__

#include <pjlib.h>
#include <pjlib-util.h>
#include <pjmedia.h>
#include <pjmedia/vid_port.h>

PJ_BEGIN_DECL

typedef struct pjapp_media_file_data
{
    pj_pool_t *pool;
    pjmedia_clock *play_clock;
    const char *file_name;
    pjmedia_port *play_port;
    pjmedia_port *stream_port;
    void *read_buf;
    pj_size_t read_buf_size;
} pjapp_media_file_data;


PJ_DECL(pj_status_t) pjmedia_file_create_stream(pj_pool_t *pool,
                                  const char *filename,
                                  unsigned flags,
                                  pjmedia_port **p_streams);

PJ_DECL(pj_status_t) pjmedia_file_codeparam_get(
                                        pjmedia_port *port,
                                        void *info);

PJ_DECL(pj_status_t) pjmedia_file_set_eof_cb2(pjmedia_port *port,
                               void *user_data,
                               void (*cb)(pjmedia_port *port,
                                          void *usr_data));
PJ_DECL(pj_status_t) pjmedia_file_set_eof_cb( pjmedia_port *port,
                               void *user_data,
                               pj_status_t (*cb)(pjmedia_port *port,
                                                 void *usr_data));
PJ_END_DECL
#endif /* __PJSUA_MEDIA_FILE_H__ */
