/* $Id$ */
/*
 * Copyright (C) 2010 Teluu Inc. (http://www.teluu.com)
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
#ifndef __PJLIB_UTIL_CLI_SOCKET_H__
#define __PJLIB_UTIL_CLI_SOCKET_H__

/**
 * @file cli_socket.h
 * @brief Command Line Interface Telnet Front End API
 */

#include <pjlib-util/cli_imp.h>

PJ_BEGIN_DECL

/**
 * This structure contains various options to instantiate the socket daemon.
 * Application must call pj_cli_socket_cfg_default() to initialize
 * this structure with its default values.
 */
typedef struct pj_cli_socket_cfg
{
	int wfd;
	int rfd;
	pj_bool_t tcp;
} pj_cli_socket_cfg;

/**
 * Initialize pj_cli_socket_cfg with its default values.
 *
 * @param param		The structure to be initialized.
 */
PJ_DECL(void) pj_cli_socket_cfg_default(pj_cli_socket_cfg *param);


/**
 * Create, initialize, and start a socket daemon for the application.
 *
 * @param cli		The CLI application instance.
 * @param param		Optional parameters for creating the socket daemon.
 * 			If this value is NULL, default parameters will be used.
 * @param p_fe		Optional pointer to receive the front-end instance
 * 			of the socket front-end just created.
 *
 * @return		PJ_SUCCESS on success, or the appropriate error code.
 */
PJ_DEF(pj_status_t) pj_cli_socket_create(pj_cli_t *cli,
					  const pj_cli_socket_cfg *param,
					  pj_cli_sess **p_sess,
					  pj_cli_front_end **p_fe);

PJ_DEF(pj_status_t) pj_cli_socket_get_wfd(pj_cli_front_end *fe);
/**
 * @}
 */
PJ_DEF(pj_status_t) cli_socket_destroy(void *ife);

PJ_END_DECL

#endif /* __PJLIB_UTIL_CLI_SOCKET_H__ */
