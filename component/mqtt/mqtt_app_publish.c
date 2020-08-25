/*
 * mqtt_app_publish.c
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



void mqtt_sub_publish_v5_callback(struct mosquitto *mosq, void *obj, int mid,
		int reason_code, const mosquitto_property *properties)
{
	UNUSED(obj);
	UNUSED(reason_code);
	UNUSED(properties);
	zassert(mosq != NULL);
	zassert(obj != NULL);
	UNUSED(obj);
	UNUSED(properties);
#if 0
	last_mid_sent = mid;
	if (reason_code > 127)
	{
		mqtt_err_printf(&cfg, "Warning: Publish %d failed: %s.\n", mid,
				mosquitto_reason_string(reason_code));
	}
	publish_count++;

	if (cfg.pub_mode == MSGMODE_STDIN_LINE)
	{
		if (mid == last_mid)
		{
			mosquitto_disconnect_v5(mosq, 0, cfg.disconnect_props);
			disconnect_sent = true;
		}
	}
	else if (publish_count < cfg.repeat_count)
	{
		ready_for_repeat = true;
		set_repeat_time();
	}
	else if (disconnect_sent == false)
	{
		mosquitto_disconnect_v5(mosq, 0, cfg.disconnect_props);
		disconnect_sent = true;
	}
#endif
}

void mqtt_sub_publish_callback(struct mosquitto *mosq, void *obj, int mid, int reason_code)
{
	mqtt_sub_publish_v5_callback(mosq, obj,  mid,
			 reason_code, NULL);
}


int mqtt_publish(struct mosquitto *mosq, int *mid, const char *topic,
		int payloadlen, void *payload, int qos, bool retain)
{
	struct mqtt_app_config *cfg = mosquitto_userdata(mosq);

	if (cfg->mqtt_version == MQTT_PROTOCOL_V5)
	{
		return mosquitto_publish_v5(mosq, mid, NULL, payloadlen, payload, qos,
				retain, cfg->publish_props);
	}
	else
	{
		return mosquitto_publish(mosq, mid, topic, payloadlen, payload, qos, retain);
	}
	return ERROR;
}

#if 0
void my_disconnect_callback(struct mosquitto *mosq, void *obj, int rc,
		const mosquitto_property *properties)
{
	UNUSED(mosq);
	UNUSED(obj);
	UNUSED(rc);
	UNUSED(properties);

	if (rc == 0)
	{
		status = STATUS_DISCONNECTED;
	}
}



void my_connect_callback(struct mosquitto *mosq, void *obj, int result,
		int flags, const mosquitto_property *properties)
{
	int rc = MOSQ_ERR_SUCCESS;

	UNUSED(obj);
	UNUSED(flags);
	UNUSED(properties);

	if (!result)
	{
		switch (cfg.pub_mode)
		{
		case MSGMODE_CMD:
		case MSGMODE_FILE:
		case MSGMODE_STDIN_FILE:
			rc = my_publish(mosq, &mid_sent, cfg.topic, cfg.msglen, cfg.message,
					cfg.qos, cfg.retain);
			break;
		case MSGMODE_NULL:
			rc = my_publish(mosq, &mid_sent, cfg.topic, 0, NULL, cfg.qos,
					cfg.retain);
			break;
		case MSGMODE_STDIN_LINE:
			status = STATUS_CONNACK_RECVD;
			break;
		}
		if (rc)
		{
			switch (rc)
			{
			case MOSQ_ERR_INVAL:
				mqtt_err_printf(&cfg,
						"Error: Invalid input. Does your topic contain '+' or '#'?\n");
				break;
			case MOSQ_ERR_NOMEM:
				mqtt_err_printf(&cfg,
						"Error: Out of memory when trying to publish message.\n");
				break;
			case MOSQ_ERR_NO_CONN:
				mqtt_err_printf(&cfg,
						"Error: Client not connected when trying to publish.\n");
				break;
			case MOSQ_ERR_PROTOCOL:
				mqtt_err_printf(&cfg,
						"Error: Protocol error when communicating with broker.\n");
				break;
			case MOSQ_ERR_PAYLOAD_SIZE:
				mqtt_err_printf(&cfg, "Error: Message payload is too large.\n");
				break;
			case MOSQ_ERR_QOS_NOT_SUPPORTED:
				mqtt_err_printf(&cfg,
						"Error: Message QoS not supported on broker, try a lower QoS.\n");
				break;
			}
			mosquitto_disconnect_v5(mosq, 0, cfg.disconnect_props);
		}
	}
	else
	{
		if (result)
		{
			if (cfg.mqtt_version == MQTT_PROTOCOL_V5)
			{
				if (result == MQTT_RC_UNSUPPORTED_PROTOCOL_VERSION)
				{
					mqtt_err_printf(&cfg,
							"Connection error: %s. Try connecting to an MQTT v5 broker, or use MQTT v3.x mode.\n",
							mosquitto_reason_string(result));
				}
				else
				{
					mqtt_err_printf(&cfg, "Connection error: %s\n",
							mosquitto_reason_string(result));
				}
			}
			else
			{
				mqtt_err_printf(&cfg, "Connection error: %s\n",
						mosquitto_connack_string(result));
			}
			mosquitto_disconnect_v5(mosq, 0, cfg.disconnect_props);
		}
	}
}

void my_publish_callback(struct mosquitto *mosq, void *obj, int mid,
		int reason_code, const mosquitto_property *properties)
{
	UNUSED(obj);
	UNUSED(properties);

	last_mid_sent = mid;
	if (reason_code > 127)
	{
		mqtt_err_printf(&cfg, "Warning: Publish %d failed: %s.\n", mid,
				mosquitto_reason_string(reason_code));
	}
	publish_count++;

	if (cfg.pub_mode == MSGMODE_STDIN_LINE)
	{
		if (mid == last_mid)
		{
			mosquitto_disconnect_v5(mosq, 0, cfg.disconnect_props);
			disconnect_sent = true;
		}
	}
	else if (publish_count < cfg.repeat_count)
	{
		ready_for_repeat = true;
		set_repeat_time();
	}
	else if (disconnect_sent == false)
	{
		mosquitto_disconnect_v5(mosq, 0, cfg.disconnect_props);
		disconnect_sent = true;
	}
}

int pub_shared_init(void)
{
	line_buf = malloc(line_buf_len);
	if (!line_buf)
	{
		mqtt_err_printf(&cfg, "Error: Out of memory.\n");
		return 1;
	}
	return 0;
}

int pub_stdin_line_loop(struct mosquitto *mosq)
{
	char *buf2;
	int buf_len_actual = 0;
	int pos;
	int rc = MOSQ_ERR_SUCCESS;
	int read_len;
	bool stdin_finished = false;

	mosquitto_loop_start(mosq);
	stdin_finished = false;
	do
	{
		if (status == STATUS_CONNACK_RECVD)
		{
			pos = 0;
			read_len = line_buf_len;
			while (status == STATUS_CONNACK_RECVD
					&& fgets(&line_buf[pos], read_len, stdin))
			{
				buf_len_actual = strlen(line_buf);
				if (line_buf[buf_len_actual - 1] == '\n')
				{
					line_buf[buf_len_actual - 1] = '\0';
					rc = my_publish(mosq, &mid_sent, cfg.topic,
							buf_len_actual - 1, line_buf, cfg.qos, cfg.retain);
					pos = 0;
					if (rc)
					{
						mqtt_err_printf(&cfg,
								"Error: Publish returned %d, disconnecting.\n",
								rc);
						mosquitto_disconnect_v5(mosq,
								MQTT_RC_DISCONNECT_WITH_WILL_MSG,
								cfg.disconnect_props);
					}
					break;
				}
				else
				{
					line_buf_len += 1024;
					pos += 1023;
					read_len = 1024;
					buf2 = realloc(line_buf, line_buf_len);
					if (!buf2)
					{
						mqtt_err_printf(&cfg, "Error: Out of memory.\n");
						return MOSQ_ERR_NOMEM;
					}
					line_buf = buf2;
				}
			}
			if (pos != 0)
			{
				rc = my_publish(mosq, &mid_sent, cfg.topic, buf_len_actual,
						line_buf, cfg.qos, cfg.retain);
				if (rc)
				{
					mqtt_err_printf(&cfg,
							"Error: Publish returned %d, disconnecting.\n", rc);
					mosquitto_disconnect_v5(mosq,
							MQTT_RC_DISCONNECT_WITH_WILL_MSG,
							cfg.disconnect_props);
				}
			}
			if (feof(stdin))
			{
				if (mid_sent == -1)
				{
					/* Empty file */
					mosquitto_disconnect_v5(mosq, 0, cfg.disconnect_props);
					disconnect_sent = true;
					status = STATUS_DISCONNECTING;
				}
				else
				{
					last_mid = mid_sent;
					status = STATUS_WAITING;
				}
				stdin_finished = true;
			}
			else if (status == STATUS_DISCONNECTED)
			{
				/* Not end of stdin, so we've lost our connection and must
				 * reconnect */
			}
		}

		if (status == STATUS_WAITING)
		{
			if (last_mid_sent == last_mid && disconnect_sent == false)
			{
				mosquitto_disconnect_v5(mosq, 0, cfg.disconnect_props);
				disconnect_sent = true;
			}
#ifdef WIN32
			Sleep(100);
#else
			struct timespec ts;
			ts.tv_sec = 0;
			ts.tv_nsec = 100000000;
			nanosleep(&ts, NULL);
#endif
		}
	} while (stdin_finished == false);
	mosquitto_loop_stop(mosq, false);

	if (status == STATUS_DISCONNECTED)
	{
		return MOSQ_ERR_SUCCESS;
	}
	else
	{
		return rc;
	}
}

int pub_other_loop(struct mosquitto *mosq)
{
	int rc;
	int loop_delay = 1000;

	if (cfg.repeat_count > 1
			&& (cfg.repeat_delay.tv_sec == 0 || cfg.repeat_delay.tv_usec != 0))
	{
		loop_delay = cfg.repeat_delay.tv_usec / 2000;
	}

	do
	{
		rc = mosquitto_loop(mosq, loop_delay, 1);
		if (ready_for_repeat && check_repeat_time())
		{
			rc = MOSQ_ERR_SUCCESS;
			switch (cfg.pub_mode)
			{
			case MSGMODE_CMD:
			case MSGMODE_FILE:
			case MSGMODE_STDIN_FILE:
				rc = my_publish(mosq, &mid_sent, cfg.topic, cfg.msglen,
						cfg.message, cfg.qos, cfg.retain);
				break;
			case MSGMODE_NULL:
				rc = my_publish(mosq, &mid_sent, cfg.topic, 0, NULL, cfg.qos,
						cfg.retain);
				break;
			}
			if (rc)
			{
				mqtt_err_printf(&cfg, "Error sending repeat publish: %s",
						mosquitto_strerror(rc));
			}
		}
	} while (rc == MOSQ_ERR_SUCCESS);

	if (status == STATUS_DISCONNECTED)
	{
		return MOSQ_ERR_SUCCESS;
	}
	else
	{
		return rc;
	}
}

int pub_shared_loop(struct mosquitto *mosq)
{
	if (cfg.pub_mode == MSGMODE_STDIN_LINE)
	{
		return pub_stdin_line_loop(mosq);
	}
	else
	{
		return pub_other_loop(mosq);
	}
}

void pub_shared_cleanup(void)
{
	free(line_buf);
}

int pub_main(int argc, char *argv[])
{
	struct mosquitto *mosq = NULL;
	int rc;

	mosquitto_lib_init();

	if (pub_shared_init())
		return 1;

	rc = client_config_load(&cfg, MQTT_MODE_PUB, argc, argv);
	if (rc)
	{
		if (rc == 2)
		{
			/* --help */
			print_usage();
		}
		else
		{
			fprintf(stderr, "\nUse 'mosquitto_pub --help' to see usage.\n");
		}
		goto cleanup;
	}

#ifndef WITH_THREADING
	if (cfg.pub_mode == MSGMODE_STDIN_LINE)
	{
		fprintf(stderr,
				"Error: '-l' mode not available, threading support has not been compiled in.\n");
		goto cleanup;
	}
#endif

	if (cfg.pub_mode == MSGMODE_STDIN_FILE)
	{
		if (load_stdin())
		{
			mqtt_err_printf(&cfg, "Error loading input from stdin.\n");
			goto cleanup;
		}
	}
	else if (cfg.file_input)
	{
		if (load_file(cfg.file_input))
		{
			mqtt_err_printf(&cfg, "Error loading input file \"%s\".\n",
					cfg.file_input);
			goto cleanup;
		}
	}

	if (!cfg.topic || cfg.pub_mode == MSGMODE_NONE)
	{
		fprintf(stderr, "Error: Both topic and message must be supplied.\n");
		print_usage();
		goto cleanup;
	}

	if (client_id_generate(&cfg))
	{
		goto cleanup;
	}

	mosq = mosquitto_new(cfg.id, cfg.clean_session, NULL);
	if (!mosq)
	{
		switch (errno)
		{
		case ENOMEM:
			mqtt_err_printf(&cfg, "Error: Out of memory.\n");
			break;
		case EINVAL:
			mqtt_err_printf(&cfg, "Error: Invalid id.\n");
			break;
		}
		goto cleanup;
	}
	if (cfg.debug)
	{
		mosquitto_log_callback_set(mosq, my_log_callback);
	}
	mosquitto_connect_v5_callback_set(mosq, my_connect_callback);
	mosquitto_disconnect_v5_callback_set(mosq, my_disconnect_callback);
	mosquitto_publish_v5_callback_set(mosq, my_publish_callback);

	if (client_opts_set(mosq, &cfg))
	{
		goto cleanup;
	}

	rc = client_connect(mosq, &cfg);
	if (rc)
	{
		goto cleanup;
	}

	rc = pub_shared_loop(mosq);

	if (cfg.message && cfg.pub_mode == MSGMODE_FILE)
	{
		free(cfg.message);
		cfg.message = NULL;
	}
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	client_config_cleanup(&cfg);
	pub_shared_cleanup();

	if (rc)
	{
		mqtt_err_printf(&cfg, "Error: %s\n", mosquitto_strerror(rc));
	}
	return rc;

	cleanup: mosquitto_lib_cleanup();
	client_config_cleanup(&cfg);
	pub_shared_cleanup();
	return 1;
}
#endif
