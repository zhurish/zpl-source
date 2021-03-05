/*
 * mqtt_app_util.h
 *
 *  Created on: 2020年4月12日
 *      Author: zhurish
 */

#ifndef __MQTT_APP_UTIL_H__
#define __MQTT_APP_UTIL_H__

#define _MQTT_DEBUG_ENABLE 1

#if defined(_MQTT_DEBUG_ENABLE)
#define _MQTT_DBG_ERR(format, ...) 			zlog_err (MODULE_MQTT, format, ##__VA_ARGS__)
#define _MQTT_DBG_WARN(format, ...) 		zlog_warn (MODULE_MQTT, format, ##__VA_ARGS__)
#define _MQTT_DBG_INFO(format, ...) 		zlog_info (MODULE_MQTT, format, ##__VA_ARGS__)
#define _MQTT_DBG_DEBUG(format, ...) 		zlog_debug (MODULE_MQTT, format, ##__VA_ARGS__)
#define _MQTT_DBG_TRAP(format, ...) 		zlog_trap (MODULE_MQTT, format, ##__VA_ARGS__)
#else
#define _MQTT_DBG_ERR(format, ...)
#define _MQTT_DBG_WARN(format, ...)
#define _MQTT_DBG_INFO(format, ...)
#define _MQTT_DBG_DEBUG(format, ...)
#define _MQTT_DBG_TRAP(format, ...)
#endif


int mqtt_client_connect(struct mqtt_app_config *cfg);
int mqtt_client_id_generate(struct mqtt_app_config *cfg);
int mqtt_client_opts_config(struct mqtt_app_config *cfg);
int mqtt_client_add_topic(struct mqtt_app_config *cfg, ospl_char *topic);
int mqtt_client_del_topic(struct mqtt_app_config *cfg, ospl_char *topic);

int mqtt_client_property(struct mqtt_app_config *cfg, ospl_char *cmdname,
		ospl_char *propname, ospl_char *key, ospl_char *value);


void mqtt_log_callback(const ospl_char *file, const ospl_char *func, const ospl_uint32 line,
		struct mosquitto *mosq, void *obj, ospl_uint32 level, const ospl_char *str);

void mqtt_log_error_callback(const ospl_char *file, const ospl_char *func, const ospl_uint32 line,
		const struct mqtt_app_config *cfg, const ospl_char *fmt, ...);

#define mqtt_err_printf(cfg, format, ...) 	mqtt_log_error_callback (__FILE__, __FUNCTION__, __LINE__, cfg, format, ##__VA_ARGS__)

#endif /* __MQTT_APP_UTIL_H__ */
