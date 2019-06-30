/*
 * voip_log.h
 *
 *  Created on: 2018��12��18��
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




extern int voip_thlog_init();
extern int voip_thlog_close();
extern int voip_thlog_reload();
extern int voip_thlog_clean();
extern int voip_thlog_log(const char *format, ...);
extern int voip_thlog_log1(u_int8 building, u_int8 unit, u_int16 room, char *id,
						   char *phone, const char *format, ...);
extern int voip_thlog_log2(const char *format, ...);
extern int voip_thlog_log3(char *type, char *result, const char *format, ...);
extern int voip_thlog_log4(const time_t ti, char *type, char *result, const char *format, ...);

#endif /* __VOIP_LOG_H__ */