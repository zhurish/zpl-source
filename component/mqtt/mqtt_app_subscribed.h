/*
 * mqtt_app_subscribed.h
 *
 *  Created on: 2020年4月12日
 *      Author: zhurish
 */

#ifndef __MQTT_APP_SUBSCRIBED_H__
#define __MQTT_APP_SUBSCRIBED_H__



void mqtt_sub_message_callback(struct mosquitto *mosq, void *obj,
		const struct mosquitto_message *message);
void mqtt_sub_message_v5_callback(struct mosquitto *mosq, void *obj,
		const struct mosquitto_message *message,
		const mosquitto_property *properties);

void mqtt_sub_connect_callback(struct mosquitto *mosq, void *obj,
		int result, int flags);
void mqtt_sub_connect_v5_callback(struct mosquitto *mosq, void *obj,
		int result, int flags, const mosquitto_property *properties);

void mqtt_sub_subscribe_callback(struct mosquitto *mosq, void *obj,
		int mid, int qos_count, const int *granted_qos);
void mqtt_sub_subscribe_v5_callback(struct mosquitto *mosq, void *obj,
		int mid, int qos_count, const int *granted_qos, const mosquitto_property *properties);





#endif /* __MQTT_APP_SUBSCRIBED_H__ */
