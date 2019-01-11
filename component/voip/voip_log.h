/*
 * voip_log.h
 *
 *  Created on: 2018Äê12ÔÂ18ÈÕ
 *      Author: DELL
 */

#ifndef __VOIP_LOG_H__
#define __VOIP_LOG_H__

#include "zebra.h"
#include "log.h"



#define voip_debug_log(fmt,...)		zlog_debug(ZLOG_SOUND, fmt, ##__VA_ARGS__)
#define voip_err_log(fmt,...)		zlog_err(ZLOG_SOUND, fmt, ##__VA_ARGS__)
#define voip_warn_log(fmt,...)		zlog_warn(ZLOG_SOUND, fmt, ##__VA_ARGS__)
#define voip_info_log(fmt,...)		zlog_info(ZLOG_SOUND, fmt, ##__VA_ARGS__)
#define voip_notice_log(fmt,...)		zlog_notice(ZLOG_SOUND, fmt, ##__VA_ARGS__)
#define voip_trap_log(fmt,...)		zlog_trap(ZLOG_SOUND, fmt, ##__VA_ARGS__)



#endif /* __VOIP_LOG_H__ */
