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
#include "mqtt_app_conf.h"
#include "mqtt_app_util.h"
#include "mqtt_app_publish.h"
#include "mqtt_app_subscribed.h"
#include "mqtt_app_api.h"


void mqtt_sub_publish_callback(struct mosquitto *mosq, void *obj,
		int mid, int reason_code, const mosquitto_property *properties)
{
	UNUSED(obj);
	UNUSED(reason_code);
	UNUSED(properties);
	zassert(mosq != NULL);
	//if (process_messages == false && (mid == last_mid || last_mid == 0))
	{
		//mosquitto_disconnect_v5(mqtt_config->mosq, 0, mqtt_config->disconnect_props);
	}
}

/*
 * 接收订阅内容
 */
void mqtt_sub_message_callback(struct mosquitto *mosq, void *obj,
		const struct mosquitto_message *message,
		const mosquitto_property *properties)
{
	int i, msg_count = 0,last_mid=0;
	bool res;
	zassert(mosq != NULL);
	UNUSED(obj);
	UNUSED(properties);
	if(message->payloadlen){
		printf("%s %s\n", message->topic, message->payload);
	}else{
		printf("%s (null)\n", message->topic);
	}
	fflush(stdout);

/*
	if (process_messages == false)
		return;
*/

/*
	if (mqtt_config->sub.retained_only && !message->retain)
	{
		//process_messages = false;
		//if (last_mid == 0)
		{
			mosquitto_disconnect_v5(mosq, 0, mqtt_config->disconnect_props);
		}
		return;
	}
*/

	if (message->retain && mqtt_config->sub.no_retain)
		return;
	if (mqtt_config->sub.filter_outs)
	{
		for (i = 0; i < mqtt_config->sub.filter_out_count; i++)
		{
			mosquitto_topic_matches_sub(mqtt_config->sub.filter_outs[i], message->topic,
					&res);
			if (res)
				return;
		}
	}

	if (mqtt_config->sub.remove_retained && message->retain)
	{
		mosquitto_publish(mosq, &last_mid, message->topic, 0, NULL, 1, true);
	}

	//print_message(&mqtt_config, message);

	//if (mqtt_config->sub.msg_count > 0)
	{
		msg_count++;
		//if (mqtt_config->sub.msg_count == msg_count)
		{
			//process_messages = false;
			//if (last_mid == 0)
			{
				//mosquitto_disconnect_v5(mosq, 0, mqtt_config->disconnect_props);
			}
		}
	}
}

/*
 * 连接成功后注册订阅内容
 */
void mqtt_sub_connect_callback(struct mosquitto *mosq, void *obj,
		int result, int flags, const mosquitto_property *properties)
{
	int i;
	zassert(mosq != NULL);
	UNUSED(obj);
	UNUSED(flags);
	UNUSED(properties);

	if (!result)
	{
		mosquitto_subscribe_multiple(mosq, NULL, mqtt_config->sub.topic_count, mqtt_config->sub.topics,
				mqtt_config->qos, mqtt_config->sub.sub_opts, mqtt_config->subscribe_props);

		for (i = 0; i < mqtt_config->sub.unsub_topic_count; i++)
		{
			mosquitto_unsubscribe_v5(mosq, NULL, mqtt_config->sub.unsub_topics[i],
					mqtt_config->unsubscribe_props);
		}
	}
	else
	{
		if (result)
		{
			if (mqtt_config->mqtt_version == MQTT_PROTOCOL_V5)
			{
				if (result == MQTT_RC_UNSUPPORTED_PROTOCOL_VERSION)
				{
					mqtt_err_printf(&mqtt_config,
							"Connection error: %s. Try connecting to an MQTT v5 broker, or use MQTT v3.x mode.\n",
							mosquitto_reason_string(result));
				}
				else
				{
					mqtt_err_printf(&mqtt_config, "Connection error: %s\n",
							mosquitto_reason_string(result));
				}
			}
			else
			{
				mqtt_err_printf(&mqtt_config, "Connection error: %s\n",
						mosquitto_connack_string(result));
			}
		}
		//mosquitto_disconnect_v5(mosq, 0, mqtt_config->disconnect_props);
	}
}

/*
 * 订阅消息qos
 */
void mqtt_sub_subscribe_callback(struct mosquitto *mosq, void *obj,
		int mid, int qos_count, const int *granted_qos)
{
	int i;
	zassert(mosq != NULL);
	UNUSED(obj);

	if (mqtt_config->debug)
	{
		if (!mqtt_config->quiet)
			printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
		for (i = 1; i < qos_count; i++)
		{
			if (!mqtt_config->quiet)
				printf(", %d", granted_qos[i]);
		}
		if (!mqtt_config->quiet)
			printf("\n");
	}

/*	if (mqtt_config->sub.exit_after_sub)
	{
		mosquitto_disconnect_v5(mosq, 0, mqtt_config->disconnect_props);
	}*/
}


/*int mqtt_subscribed_main(struct mqtt_app_config *mqttconf)
{
	int rc;
	zassert(mqttconf != NULL);
	if (mqtt_client_opts_config(mqttconf))
	{
		return ERROR;
	}

	rc = mqtt_client_connect(mqttconf);
	if (rc)
	{
		return ERROR;
	}

	rc = mosquitto_loop_forever(mqttconf->mosq, -1, 1);

	return rc;
}*/

/*
int mqtt_sub_main(int argc, char *argv[])
{
	int rc;

	mosquitto_lib_init();

	rc = client_config_load(&mqtt_config, CLIENT_SUB, argc, argv);
	if (rc)
	{
		if (rc == 2)
		{
			print_usage();
		}
		else
		{
			fprintf(stderr, "\nUse 'mosquitto_sub --help' to see usage.");
		}
		goto cleanup;
	}

	if (mqtt_config->no_retain && mqtt_config->retained_only)
	{
		fprintf(stderr,
				"\nError: Combining '-R' and '--retained-only' makes no sense.");
		goto cleanup;
	}

	if (mqtt_config_client_id_generate(&mqtt_config))
	{
		goto cleanup;
	}

	mqtt_config->mosq = mosquitto_new(mqtt_config->id, mqtt_config->clean_session, &mqtt_config);
	if (!mqtt_config->mosq)
	{
		switch (errno)
		{
		case ENOMEM:
			mqtt_err_printf(&mqtt_config, "Error: Out of memory.");
			break;
		case EINVAL:
			mqtt_err_printf(&mqtt_config, "Error: Invalid id and/or clean_session.");
			break;
		}
		goto cleanup;
	}
	if (mqtt_config_client_opts_set(mqtt_config->mosq, &mqtt_config))
	{
		goto cleanup;
	}
	if (mqtt_config->debug)
	{
		mosquitto_log_callback_set(mqtt_config->mosq, mqtt_log_callback);
	}
	mosquitto_subscribe_callback_set(mqtt_config->mosq, mqtt_sub_subscribe_callback);
	mosquitto_connect_v5_callback_set(mqtt_config->mosq, mqtt_sub_connect_callback);
	mosquitto_message_v5_callback_set(mqtt_config->mosq, mqtt_sub_message_callback);

	rc = mqtt_client_connect(mqtt_config->mosq, &mqtt_config);
	if (rc)
	{
		goto cleanup;
	}

	rc = mosquitto_loop_forever(mqtt_config->mosq, -1, 1);

	mosquitto_destroy(mqtt_config->mosq);
	mosquitto_lib_cleanup();

	if (mqtt_config->msg_count > 0 && rc == MOSQ_ERR_NO_CONN)
	{
		rc = 0;
	}
	mqtt_config_default_cleanup(&mqtt_config);
	if (rc)
	{
		mqtt_err_printf(&mqtt_config, "Error: %s\n", mosquitto_strerror(rc));
	}
	return rc;

	cleanup: mosquitto_destroy(mqtt_config->mosq);
	mosquitto_lib_cleanup();
	mqtt_config_default_cleanup(&mqtt_config);
	return 1;
}
*/
