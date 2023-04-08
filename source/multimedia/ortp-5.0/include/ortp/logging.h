/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of oRTP.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file logging.h
 * \brief Logging API.
 *
**/

#ifndef ORTP_LOGGING_H
#define ORTP_LOGGING_H


#define ORTP_LOG_DOMAIN "aa"//BCTBX_LOG_DOMAIN

//#include "bctoolbox/logging.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************/
/* logging api */
/***************/
	
#define ORTP_FATAL      0
#define	ORTP_ERROR      1
#define	ORTP_WARNING    2
#define	ORTP_MESSAGE    3
#define	ORTP_TRACE      4
#define	ORTP_DEBUG      5
#define	ORTP_END        6
#define ORTP_LOGLEV_END 7

typedef unsigned int OrtpLogLevel;
typedef int (*OrtpLogFunc)(OrtpLogLevel, char*);

	
/*#define ortp_set_log_handler ortp_set_log_handler*/
 void ortp_set_log_handler(OrtpLogFunc func);
/* This function does not have any means by now, as even ortp_set_log_handler is deprecated. use ortp_log_handler_t instead*/
 OrtpLogFunc ortp_get_log_handler(void);

 void ortp_set_log_level(int level);
 unsigned int ortp_get_log_level(void);

 void ortp_hdr_log_out(char *buf);

#ifdef ORTP_NOMESSAGE_MODE

#define ortp_log(...)
#define ortp_message(...)
#define ortp_warning(...)

#else

 void ortp_log(OrtpLogLevel lev,const char *fmt,...);
 void ortp_log_out(OrtpLogLevel lev, const char *func, int line, const char *fmt,...);

#ifdef ORTP_WINDOWS_DESKTOP
 void ortp_message(const char *fmt,...);
 void ortp_warning(const char *fmt,...);
 void ortp_error(const char *fmt,...);
 void ortp_fatal(const char *fmt,...);
 void ortp_debug(const char *fmt,...);
 void ortp_trace(format, ...);
#else

#define ortp_trace(format, ...)   ortp_log_out(ORTP_TRACE, __func__, __LINE__, format, ##__VA_ARGS__)
#define ortp_message(format, ...)   ortp_log_out(ORTP_MESSAGE, __func__, __LINE__, format, ##__VA_ARGS__)
#define ortp_warning(format, ...)   ortp_log_out(ORTP_WARNING, __func__, __LINE__, format, ##__VA_ARGS__)
#define ortp_error(format, ...)     ortp_log_out(ORTP_ERROR, __func__, __LINE__, format, ##__VA_ARGS__)
#define ortp_fatal(format, ...)     ortp_log_out(ORTP_FATAL, __func__, __LINE__, format, ##__VA_ARGS__)
#define ortp_debug(format, ...)     ortp_log_out(ORTP_DEBUG, __func__, __LINE__, format, ##__VA_ARGS__)
#endif
#endif /*ORTP_NOMESSAGE_MODE*/
	

#ifdef __cplusplus
}
#endif

#endif
