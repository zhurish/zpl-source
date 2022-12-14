/*
 * voip_log.h
 *
 *  Created on: 2018��12��18��
 *      Author: DELL
 */

#ifndef __VOIP_LOG_H__
#define __VOIP_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zplos_include.h"
#include "log.h"



#define voip_debug_log(fmt,...)		zlog_debug(MODULE_SOUND, fmt, ##__VA_ARGS__)
#define voip_err_log(fmt,...)		zlog_err(MODULE_SOUND, fmt, ##__VA_ARGS__)
#define voip_warn_log(fmt,...)		zlog_warn(MODULE_SOUND, fmt, ##__VA_ARGS__)
#define voip_info_log(fmt,...)		zlog_info(MODULE_SOUND, fmt, ##__VA_ARGS__)
#define voip_notice_log(fmt,...)		zlog_notice(MODULE_SOUND, fmt, ##__VA_ARGS__)
#define voip_trap_log(fmt,...)		zlog_trap(MODULE_SOUND, fmt, ##__VA_ARGS__)




extern int voip_thlog_init();
extern int voip_thlog_close();
extern int voip_thlog_reload();
extern int voip_thlog_clean();
extern int voip_thlog_log(const char *format, ...);
extern int voip_thlog_log1(zpl_uint8 building, zpl_uint8 unit, zpl_uint16 room, char *id,
						   char *phone, const char *format, ...);
extern int voip_thlog_log2(const char *format, ...);
extern int voip_thlog_log3(char *type, char *result, const char *format, ...);
extern int voip_thlog_log4(const zpl_time_t ti, char *type, char *result, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __VOIP_LOG_H__ */
