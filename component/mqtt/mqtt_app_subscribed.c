/*
 * mqtt_subscribed.c
 *
 *  Created on: 2020年4月12日
 *      Author: zhurish
 */
#include "zebra.h"
#include "memory.h"
#include "mqtt-config.h"
#include <mqtt_protocol.h>
#include <mosquitto.h>
#include <logging_mosq.h>

#include "mqtt_app_conf.h"
#include "mqtt_app_util.h"
#include "mqtt_app_publish.h"
#include "mqtt_app_subscribed.h"
#include "mqtt_app_api.h"


/*
 * 接收订阅内容
 */
void mqtt_sub_message_v5_callback(struct mosquitto *mosq, void *obj,
		const struct mosquitto_message *message,
		const mosquitto_property *properties)
{
	int i, /*msg_count = 0,*/last_mid = 0;
	bool res;
	zassert(mosq != NULL);
	zassert(obj != NULL);
	struct mqtt_app_config *cfg = (struct mqtt_app_config *) obj;
	//UNUSED(obj);
	if ((properties && cfg->mqtt_version == MQTT_PROTOCOL_V5)
			|| (properties == NULL && cfg->mqtt_version != MQTT_PROTOCOL_V5))
	{
		if (message->payloadlen)
		{
			log__printf(cfg->mosq, MOSQ_LOG_DEBUG, "topic:%s payload:%s",
						message->topic, (char * ) message->payload);
		}
		else
		{
			log__printf(cfg->mosq, MOSQ_LOG_DEBUG, "topic:%s payload:null",
						message->topic);
		}

		if (message->retain && cfg->sub.no_retain)
			return;
		if (cfg->sub.filter_outs)
		{
			for (i = 0; i < cfg->sub.filter_out_count; i++)
			{
				mosquitto_topic_matches_sub (cfg->sub.filter_outs[i],
											 message->topic, &res);
				if (res)
					return;
			}
		}

		if (cfg->sub.remove_retained && message->retain)
		{
			mosquitto_publish (mosq, &last_mid, message->topic, 0, NULL, 1,
							   true);
		}
	}
}

void mqtt_sub_message_callback(struct mosquitto *mosq, void *obj,
		const struct mosquitto_message *message)
{
	mqtt_sub_message_v5_callback(mosq, obj,  message,  NULL);
}


/*
 * 连接成功后注册订阅内容
 */
void mqtt_sub_connect_v5_callback(struct mosquitto *mosq, void *obj, int result,
		int flags, const mosquitto_property *properties)
{
	zassert(mosq != NULL);
	zassert(obj != NULL);
	struct mqtt_app_config *cfg = (struct mqtt_app_config *) obj;
	if ((properties && cfg->mqtt_version == MQTT_PROTOCOL_V5)
			|| (properties == NULL && cfg->mqtt_version != MQTT_PROTOCOL_V5))
	{
		if (!result)
		{
			//int i = 0;
			if (cfg->sub.topic_count)
				mosquitto_subscribe_multiple (mosq, NULL, cfg->sub.topic_count,
											  cfg->sub.topics, cfg->qos,
											  cfg->sub.sub_opts,
											  cfg->subscribe_props);

			if (cfg->sub.unsub_topic_count)
				mosquitto_unsubscribe_multiple (mosq, NULL,
												cfg->sub.unsub_topic_count,
												cfg->sub.unsub_topics,
												cfg->unsubscribe_props);
			/*		for (i = 0; i < cfg->sub.unsub_topic_count; i++)
			 {
			 mosquitto_unsubscribe_v5(mosq, NULL,
			 cfg->sub.unsub_topics[i],
			 cfg->unsubscribe_props);
			 }*/
		}
		else
		{
			if (cfg->mqtt_version == MQTT_PROTOCOL_V5)
			{
				if (result == MQTT_RC_UNSUPPORTED_PROTOCOL_VERSION)
				{
					log__printf(
							cfg->mosq,
							MOSQ_LOG_ERR,
							"Connection failed(%s), Try connecting to an MQTT v5 broker, or use MQTT v3.x mode.",
							mosquitto_reason_string (result));
				}
				else
				{
					log__printf(cfg->mosq, MOSQ_LOG_ERR,
								"Connection failed(%s)",
								mosquitto_reason_string (result));
				}
			}
			else
			{
				log__printf(cfg->mosq, MOSQ_LOG_ERR, "Connection failed(%s)",
							mosquitto_connack_string (result));
			}
		}
		//mosquitto_disconnect_v5(mosq, 0, mqtt_config->disconnect_props);
	}
}

void mqtt_sub_connect_callback(struct mosquitto *mosq, void *obj, int result, int flags)
{
	mqtt_sub_connect_v5_callback(mosq, obj, result, flags, NULL);
}

/*
 * 订阅消息qos
 */
void mqtt_sub_subscribe_v5_callback(struct mosquitto *mosq, void *obj, int mid,
		int qos_count, const int *granted_qos, const mosquitto_property *properties)
{
	zassert(mosq != NULL);
	zassert(obj != NULL);
	struct mqtt_app_config *cfg = (struct mqtt_app_config *) obj;
	if ((properties && cfg->mqtt_version == MQTT_PROTOCOL_V5)
			|| (properties == NULL && cfg->mqtt_version != MQTT_PROTOCOL_V5))
	{
		//mosquitto_userdata(struct mosquitto *mosq)
		if (cfg->debug)
		{
			int i = 0;

			if (!cfg->quiet)
				log__printf(cfg->mosq, MOSQ_LOG_DEBUG,
							"Subscribed (mid: %d): %d", mid, granted_qos[0]);

			for (i = 1; i < qos_count; i++)
			{
				if (!cfg->quiet)
					log__printf(cfg->mosq, MOSQ_LOG_DEBUG, ",%d",
								granted_qos[i]);
			}
		}
	}
}

void mqtt_sub_subscribe_callback(struct mosquitto *mosq, void *obj, int mid,
		int qos_count, const int *granted_qos)
{
	mqtt_sub_subscribe_v5_callback(mosq, obj, mid, qos_count, granted_qos, NULL);
}
