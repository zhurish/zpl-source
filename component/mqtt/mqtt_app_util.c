/*
 * mqtt_app_util.c
 *
 *  Created on: 2020年4月12日
 *      Author: zhurish
 */

#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "mqtt-config.h"
#include <mqtt_protocol.h>
#include <mosquitto.h>
#include "mqtt_app_conf.h"
#include "mqtt_app_util.h"
#include "mqtt_app_publish.h"
#include "mqtt_app_subscribed.h"
#include "mqtt_app_api.h"


int mqtt_client_connect(struct mqtt_app_config *cfg)
{
#ifndef WIN32
	char *err = NULL;
#else
	char err[1024];
#endif
	int rc = 0;
	int port = 0;
	zassert(cfg != NULL);
	zassert(cfg->mosq != NULL);
	if (cfg->port < 0)
	{
#ifdef WITH_TLS
		if(cfg->cafile || cfg->capath
#  ifdef FINAL_WITH_TLS_PSK
				|| cfg->psk
#  endif
		)
		{
			port = 8883;
		}
		else
#endif
		{
			port = 1883;
		}
	}
	else
	{
		port = cfg->port;
	}

#ifdef WITH_SRV
	if(cfg->use_srv)
	{
		rc = mosquitto_connect_srv(cfg->mosq, cfg->host, cfg->keepalive, cfg->bind_address);
	}
	else
	{
		rc = mosquitto_connect_bind_v5(cfg->mosq, cfg->host, port, cfg->keepalive, cfg->bind_address, cfg->connect_props);
	}
#else
	rc = mosquitto_connect_bind_v5(cfg->mosq, cfg->host, port, cfg->keepalive,
			cfg->bind_address, cfg->connect_props);
#endif
	if (rc > 0)
	{
		if (rc == MOSQ_ERR_ERRNO)
		{
#ifndef WIN32
			err = strerror(errno);
#else
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errno, 0, (LPTSTR)&err, 1024, NULL);
#endif
			mqtt_err_printf(cfg, "Error: %s\n", err);
		}
		else
		{
			mqtt_err_printf(cfg, "Unable to connect (%s).\n",
					mosquitto_strerror(rc));
		}
		return rc;
	}
	return MOSQ_ERR_SUCCESS;
}

int mqtt_client_id_generate(struct mqtt_app_config *cfg)
{
	zassert(cfg != NULL);
	if (cfg->id_prefix)
	{
		cfg->id = XMALLOC(MTYPE_MQTT_CONF, strlen(cfg->id_prefix) + 10);
		if (!cfg->id)
		{
			mqtt_err_printf(cfg, "Error: Out of memory.");
			return -1;
		}
		snprintf(cfg->id, strlen(cfg->id_prefix) + 10, "%s%d", cfg->id_prefix,
				getpid());
	}
	return MOSQ_ERR_SUCCESS;
}


int mqtt_client_add_topic(struct mqtt_app_config *cfg, mqtt_mode_t type, char *topic,
		const char *arg)
{
	zassert(cfg != NULL);
	if (mosquitto_validate_utf8(topic, strlen(topic)))
	{
		mqtt_err_printf(cfg, "Error: Malformed UTF-8 in %s argument.\n\n", arg);
		return -1;
	}
	if (type == MQTT_MODE_PUB || type == MQTT_MODE_RR)
	{
		if (mosquitto_pub_topic_check(topic) == MOSQ_ERR_INVAL)
		{
			mqtt_err_printf(cfg,
					"Error: Invalid publish topic '%s', does it contain '+' or '#'?\n",
					topic);
			return -1;
		}
		if (type == MQTT_MODE_PUB)
			cfg->pub.topic = XSTRDUP(MTYPE_MQTT_TOPIC,topic);
		if (type == MQTT_MODE_RR)
			cfg->rr.topic = XSTRDUP(MTYPE_MQTT_TOPIC,topic);
	}
	else if (type == MQTT_MODE_RESPONSE_TOPIC)
	{
		if (mosquitto_pub_topic_check(topic) == MOSQ_ERR_INVAL)
		{
			mqtt_err_printf(cfg,
					"Error: Invalid response topic '%s', does it contain '+' or '#'?\n",
					topic);
			return -1;
		}
		cfg->rr.response_topic = XSTRDUP(MTYPE_MQTT_TOPIC,topic);
	}
	else
	{
		if (mosquitto_sub_topic_check(topic) == MOSQ_ERR_INVAL)
		{
			mqtt_err_printf(cfg,
					"Error: Invalid subscription topic '%s', are all '+' and '#' wildcards correct?\n",
					topic);
			return -1;
		}
		cfg->sub.topic_count++;
		cfg->sub.topics = XREALLOC(MTYPE_MQTT_TOPIC, cfg->sub.topics,
				cfg->sub.topic_count * sizeof(char *));
		if (!cfg->sub.topics)
		{
			mqtt_err_printf(cfg, "Error: Out of memory.");
			return -1;
		}
		cfg->sub.topics[cfg->sub.topic_count - 1] = XSTRDUP(MTYPE_MQTT_TOPIC,topic);
	}
	return 0;
}

int mqtt_client_opts_config(struct mqtt_app_config *cfg)
{
#if defined(WITH_TLS) || defined(WITH_SOCKS)
	int rc;
#endif
	zassert(cfg != NULL);
	zassert(cfg->mosq != NULL);
	mosquitto_int_option(cfg->mosq, MOSQ_OPT_PROTOCOL_VERSION,
			cfg->mqtt_version);

	if (cfg->sub.will_topic
			&& mosquitto_will_set_v5(cfg->mosq, cfg->sub.will_topic,
					cfg->sub.will_payloadlen, cfg->sub.will_payload, cfg->sub.will_qos,
					cfg->sub.will_retain, cfg->will_props))
	{
		mqtt_err_printf(cfg, "Error: Problem setting will.");
		return -1;
	}
	cfg->will_props = NULL;

	if ((cfg->username || cfg->password)
			&& mosquitto_username_pw_set(cfg->mosq, cfg->username, cfg->password))
	{
		mqtt_err_printf(cfg, "Error: Problem setting username and/or password.");
		return -1;
	}
#ifdef WITH_TLS
	if(cfg->cafile || cfg->capath)
	{
		rc = mosquitto_tls_set(cfg->mosq, cfg->cafile, cfg->capath, cfg->certfile, cfg->keyfile, NULL);
		if(rc)
		{
			if(rc == MOSQ_ERR_INVAL)
			{
				mqtt_err_printf(cfg, "Error: Problem setting TLS options: File not found.");
			}
			else
			{
				mqtt_err_printf(cfg, "Error: Problem setting TLS options: %s.\n", mosquitto_strerror(rc));
			}
			return -1;
		}
	}
	if(cfg->insecure && mosquitto_tls_insecure_set(cfg->mosq, true))
	{
		mqtt_err_printf(cfg, "Error: Problem setting TLS insecure option.");
		return -1;
	}
	if(cfg->tls_engine && mosquitto_string_option(cfg->mosq, MOSQ_OPT_TLS_ENGINE, cfg->tls_engine))
	{
		mqtt_err_printf(cfg, "Error: Problem setting TLS engine, is %s a valid engine?\n", cfg->tls_engine);
		return -1;
	}
	if(cfg->keyform && mosquitto_string_option(cfg->mosq, MOSQ_OPT_TLS_KEYFORM, cfg->keyform))
	{
		mqtt_err_printf(cfg, "Error: Problem setting key form, it must be one of 'pem' or 'engine'.");
		return -1;
	}
	if(cfg->tls_engine_kpass_sha1 && mosquitto_string_option(cfg->mosq, MOSQ_OPT_TLS_ENGINE_KPASS_SHA1, cfg->tls_engine_kpass_sha1))
	{
		mqtt_err_printf(cfg, "Error: Problem setting TLS engine key pass sha, is it a 40 character hex string?");
		return -1;
	}
	if(cfg->tls_alpn && mosquitto_string_option(cfg->mosq, MOSQ_OPT_TLS_ALPN, cfg->tls_alpn))
	{
		mqtt_err_printf(cfg, "Error: Problem setting TLS ALPN protocol.");
		return -1;
	}
#  ifdef FINAL_WITH_TLS_PSK
	if(cfg->psk && mosquitto_tls_psk_set(cfg->mosq, cfg->psk, cfg->psk_identity, NULL))
	{
		mqtt_err_printf(cfg, "Error: Problem setting TLS-PSK options.");
		return -1;
	}
#  endif
	if((cfg->tls_version || cfg->ciphers) && mosquitto_tls_opts_set(cfg->mosq, 1, cfg->tls_version, cfg->ciphers))
	{
		mqtt_err_printf(cfg, "Error: Problem setting TLS options, check the options are valid.");
		return -1;
	}
#endif
	mosquitto_max_inflight_messages_set(cfg->mosq, cfg->max_inflight);
#ifdef WITH_SOCKS
	if(cfg->socks5_host)
	{
		rc = mosquitto_socks5_set(cfg->mosq, cfg->socks5_host, cfg->socks5_port, cfg->socks5_username, cfg->socks5_password);
		if(rc)
		{
			return rc;
		}
	}
#endif
	return MOSQ_ERR_SUCCESS;
}



/* This parses property inputs. It should work for any command type, but is limited at the moment.
 *
 * Format:
 *
 * command property value
 * command property key value
 *
 * Example:
 *
 * publish message-expiry-interval 32
 * connect user-property key value
 */

int mqtt_client_property(struct mqtt_app_config *cfg, char *cmdname,
		char *propname, char *key, char *value)
{
	int cmd = 0, identifier = 0, type = 0;
	mosquitto_property **proplist = NULL;
	int rc = 0;
	zassert(cfg != NULL);
	if (mosquitto_string_to_command(cmdname, &cmd))
	{
		mqtt_err_printf(cfg,
				"Error: Invalid command given in --property argument.");
		return MOSQ_ERR_INVAL;
	}

	if (mosquitto_string_to_property_info(propname, &identifier, &type))
	{
		mqtt_err_printf(cfg,
				"Error: Invalid property name given in --property argument.");
		return MOSQ_ERR_INVAL;
	}

	if (mosquitto_property_check_command(cmd, identifier))
	{
		mqtt_err_printf(cfg,
				"Error: %s property not allow for %s in --property argument.",
				propname, cmdname);
		return MOSQ_ERR_INVAL;
	}
	/*
	 if(identifier == MQTT_PROP_USER_PROPERTY){

	 key = argv[(*idx)+2];
	 value = argv[(*idx)+3];
	 (*idx) += 3;
	 }else{
	 value = argv[(*idx)+2];
	 (*idx) += 2;
	 }
	 */
	switch (cmd)
	{
	case CMD_CONNECT:
		proplist = &cfg->connect_props;
		break;

	case CMD_PUBLISH:
		if (identifier == MQTT_PROP_TOPIC_ALIAS)
		{
			cfg->pub.have_topic_alias = true;
		}
		if (identifier == MQTT_PROP_SUBSCRIPTION_IDENTIFIER)
		{
			mqtt_err_printf(cfg,
					"Error: %s property not supported for %s in --property argument.\n\n",
					propname, cmdname);
			return MOSQ_ERR_INVAL;
		}
		proplist = &cfg->publish_props;
		break;

	case CMD_SUBSCRIBE:
		if (identifier != MQTT_PROP_SUBSCRIPTION_IDENTIFIER
				&& identifier != MQTT_PROP_USER_PROPERTY)
		{
			mqtt_err_printf(cfg,
					"Error: %s property not supported for %s in --property argument.\n\n",
					propname, cmdname);
			return MOSQ_ERR_NOT_SUPPORTED;
		}
		proplist = &cfg->subscribe_props;
		break;

	case CMD_UNSUBSCRIBE:
		proplist = &cfg->unsubscribe_props;
		break;

	case CMD_DISCONNECT:
		proplist = &cfg->disconnect_props;
		break;

	case CMD_AUTH:
		mqtt_err_printf(cfg,
				"Error: %s property not supported for %s in --property argument.\n\n",
				propname, cmdname);
		return MOSQ_ERR_NOT_SUPPORTED;

	case CMD_WILL:
		proplist = &cfg->will_props;
		break;

	case CMD_PUBACK:
	case CMD_PUBREC:
	case CMD_PUBREL:
	case CMD_PUBCOMP:
	case CMD_SUBACK:
	case CMD_UNSUBACK:
		mqtt_err_printf(cfg,
				"Error: %s property not supported for %s in --property argument.\n\n",
				propname, cmdname);
		return MOSQ_ERR_NOT_SUPPORTED;

	default:
		return MOSQ_ERR_INVAL;
	}

	switch (type)
	{
	case MQTT_PROP_TYPE_BYTE:
		rc = mosquitto_property_add_byte(proplist, identifier, atoi(value));
		break;
	case MQTT_PROP_TYPE_INT16:
		rc = mosquitto_property_add_int16(proplist, identifier, atoi(value));
		break;
	case MQTT_PROP_TYPE_INT32:
		rc = mosquitto_property_add_int32(proplist, identifier, atoi(value));
		break;
	case MQTT_PROP_TYPE_VARINT:
		rc = mosquitto_property_add_varint(proplist, identifier, atoi(value));
		break;
	case MQTT_PROP_TYPE_BINARY:
		rc = mosquitto_property_add_binary(proplist, identifier, value,
				strlen(value));
		break;
	case MQTT_PROP_TYPE_STRING:
		rc = mosquitto_property_add_string(proplist, identifier, value);
		break;
	case MQTT_PROP_TYPE_STRING_PAIR:
		rc = mosquitto_property_add_string_pair(proplist, identifier, key,
				value);
		break;
	default:
		return MOSQ_ERR_INVAL;
	}
	if (rc)
	{
		mqtt_err_printf(cfg, "Error adding property %s %d\n", propname, type);
		return rc;
	}
	return MOSQ_ERR_SUCCESS;
}

void mqtt_log_error_callback(const char *file, const char *func, const int line,
		const struct mqtt_app_config *cfg, const char *fmt, ...)
{
	va_list va;

	if (cfg->quiet)
		return;
	va_start(va, fmt);
	pl_vzlog(file, func, line, zlog_default, ZLOG_MQTT, MOSQ_LOG_ERR, fmt, va);
	//vfprintf(stderr, fmt, va);
	va_end(va);

	//extern void pl_vzlog(const char *file, const char *func, const int line, struct zlog *zl, int module, int priority, const char *format,
	//		va_list args);
	//	log__printf(mosq, MOSQ_LOG_DEBUG, "Client %s sending PUBLISH (d%d, q%d, r%d, m%d, '%s', ... (%ld bytes))",
		//mosq->id, dup, qos, retain, mid, topic, (long)payloadlen);
}


void mqtt_log_callback(const char *file, const char *func, const int line,
		struct mosquitto *mosq, void *obj, int level, const char *str)
{
	int loglevel = 0;
//	#define LOG_TRAP		(LOG_DEBUG+1)
	if(level == MOSQ_LOG_ERR)
		loglevel = LOG_ERR;
	else if(level == MOSQ_LOG_WARNING)
		loglevel = LOG_WARNING;
	else if(level == MOSQ_LOG_NOTICE)
		loglevel = LOG_NOTICE;
	else if(level == MOSQ_LOG_INFO)
		loglevel = LOG_INFO;
	else if(level == MOSQ_LOG_DEBUG)
		loglevel = LOG_DEBUG;

	if(loglevel)
		pl_zlog (file, func, line, ZLOG_MQTT, loglevel, "%s", str);

	/*	UNUSED(mosq);
		UNUSED(obj);
		UNUSED(level);
		printf("%s\n", str);*/
}