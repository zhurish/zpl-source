/*
 * mqtt_app_subscribed.h
 *
 *  Created on: 2020年4月12日
 *      Author: zhurish
 */

#ifndef __MQTT_APP_SUBSCRIBED_H__
#define __MQTT_APP_SUBSCRIBED_H__


void mqtt_sub_publish_callback(struct mosquitto *mosq, void *obj,
		int mid, int reason_code, const mosquitto_property *properties);

void mqtt_sub_message_callback(struct mosquitto *mosq, void *obj,
		const struct mosquitto_message *message,
		const mosquitto_property *properties);
void mqtt_sub_connect_callback(struct mosquitto *mosq, void *obj,
		int result, int flags, const mosquitto_property *properties);
void mqtt_sub_subscribe_callback(struct mosquitto *mosq, void *obj,
		int mid, int qos_count, const int *granted_qos);


//int mqtt_subscribed_main(struct mqtt_app_config *mqttconf);




#endif /* __MQTT_APP_SUBSCRIBED_H__ */
