/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
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

#ifndef __PJSUA_APP_H__
#define __PJSUA_APP_H__

/**
 * Interface for user application to use pjsua with CLI/menu based UI. 
 */

#include "pjsua_app_common.h"
#include "pjsua_app_cfgapi.h"
PJ_BEGIN_DECL

/**
 * This will initiate the pjsua and the user interface (CLI/menu UI) based on 
 * the provided configuration.
 */
pj_status_t pjsua_app_init(const pjsua_app_cfg_t *app_cfg);

/**
 * This will run the CLI/menu based UI.
 * wait_telnet_cli is used for CLI based UI. It will tell the library to block
 * or wait until user invoke the "shutdown"/"restart" command. GUI based app
 * should define this param as PJ_FALSE.
 */
pj_status_t pjsua_app_run(pj_bool_t wait_telnet_cli);

/**
 * This will destroy/cleanup the application library.
 */
pj_status_t pjsua_app_destroy(void);
int pjsua_app_restart(void);
int pjsua_app_exit(void);

int log_refresh_proc(void *arg);

int pjapp_app_perror(const char *sender, const char *title,
                      pj_status_t status);
PJ_END_DECL
    
#endif  /* __PJSUA_APP_H__ */
