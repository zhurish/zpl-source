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
#ifndef __PJSUA_APP_CONFIG_H__
#define __PJSUA_APP_CONFIG_H__

#define pjsua_app_def_argc (PJ_ARRAY_SIZE(pjsua_app_def_argv)-1)

extern int pjapp_global_cli_write_config(void *p, pj_bool_t detail, pj_bool_t bwrt);

#endif  /* __PJSUA_APP_CONFIG_H__ */

