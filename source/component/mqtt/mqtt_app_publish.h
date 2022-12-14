/*
 * mqtt_app_publish.h
 *
 *  Created on: 2020年4月12日
 *      Author: zhurish
 */

#ifndef __MQTT_APP_PUBLISH_H__
#define __MQTT_APP_PUBLISH_H__

void mqtt_sub_publish_callback(struct mosquitto *mosq, void *obj,
		int mid, int reason_code);
void mqtt_sub_publish_v5_callback(struct mosquitto *mosq, void *obj,
		int mid, int reason_code, const mosquitto_property *properties);

int mqtt_publish(struct mosquitto *mosq, int *mid, const char *topic,
		int payloadlen, void *payload, int qos, bool retain);


#endif /* __MQTT_APP_PUBLISH_H__ */
