/*
 * mqtt_app_util.h
 *
 *  Created on: 2020年4月12日
 *      Author: zhurish
 */

#ifndef __MQTT_APP_UTIL_H__
#define __MQTT_APP_UTIL_H__

/*
#define MOSQ_LOG_SUBSCRIBE		(1<<5)
#define MOSQ_LOG_UNSUBSCRIBE	(1<<6)
#define MOSQ_LOG_WEBSOCKETS		(1<<7)
#define MOSQ_LOG_INTERNAL		0x80000000U
#define MOSQ_LOG_ALL			0xFFFFFFFFU
*/

int mqtt_client_connect(struct mqtt_app_config *cfg);
int mqtt_client_id_generate(struct mqtt_app_config *cfg);
int mqtt_client_opts_config(struct mqtt_app_config *cfg);
int mqtt_client_add_topic(struct mqtt_app_config *cfg, mqtt_mode_t type, char *topic, const char *arg);
int mqtt_client_property(struct mqtt_app_config *cfg, char *cmdname,
		char *propname, char *key, char *value);


void mqtt_log_callback(const char *file, const char *func, const int line,
		struct mosquitto *mosq, void *obj, int level, const char *str);

void mqtt_log_error_callback(const char *file, const char *func, const int line,
		const struct mqtt_app_config *cfg, const char *fmt, ...);

#define mqtt_err_printf(cfg, format, ...) 	mqtt_log_error_callback (__FILE__, __FUNCTION__, __LINE__, cfg, format, ##__VA_ARGS__)

#endif /* __MQTT_APP_UTIL_H__ */
