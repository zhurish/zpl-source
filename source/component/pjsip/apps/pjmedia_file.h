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


typedef pjmedia_port pjmedia_file_stream;



PJ_DECL(pj_status_t) pjmedia_file_create_stream(pj_pool_t *pool,
                                  const char *filename,
                                  unsigned flags,
                                  pjmedia_file_stream **p_streams);
                                  
PJ_INLINE(pjmedia_port *) pjmedia_file_stream_get_port(pjmedia_file_stream *stream)
{
    return (pjmedia_port *)stream;
}
PJ_DECL(pj_ssize_t) pjmedia_file_stream_get_len(pjmedia_file_stream *stream);
#if !DEPRECATED_FOR_TICKET_2251
PJ_DECL(pj_status_t)  pjmedia_file_stream_set_eof_cb(pjmedia_file_stream *stream,
                              void *user_data,
                              pj_status_t (*cb)(pjmedia_file_stream *stream,
                                                void *usr_data));
#endif
PJ_DECL(pj_status_t) 
pjmedia_file_stream_set_eof_cb2(pjmedia_file_stream *stream,
                               void *user_data,
                               void (*cb)(pjmedia_file_stream *stream,
                                          void *usr_data));





#endif /* __PJSUA_MEDIA_FILE_H__ */
