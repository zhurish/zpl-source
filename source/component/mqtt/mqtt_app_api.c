/*
 * mqtt_app_api.c
 *
 *  Created on: 2019年7月19日
 *      Author: DELL
 */

#include "auto_include.h"
#include <zplos_include.h>
#include "module.h"
#include "host.h"
#include "mqtt-config.h"
#include <mqtt_protocol.h>
#include <mosquitto.h>
#include "mosquitto_internal.h"
#include "mqtt_app_conf.h"
#include "mqtt_app_util.h"
#include "mqtt_app_publish.h"
#include "mqtt_app_subscribed.h"
#include "mqtt_app_api.h"
#include "util_mosq.h"

/*
//Set the connect callback.  This is called when the broker sends a CONNACK message in response to a connection.
mosquitto_connect_callback_set
//Set the connect callback.  This is called when the broker sends a CONNACK message in response to a connection.
mosquitto_connect_with_flags_callback_set
//Set the connect callback.  This is called when the broker sends a CONNACK message in response to a connection.
mosquitto_connect_v5_callback_set
//Set the disconnect callback.  This is called when the broker has received the DISCONNECT command and has disconnected the client.
mosquitto_disconnect_callback_set
//Set the disconnect callback.  This is called when the broker has received the DISCONNECT command and has disconnected the client.
mosquitto_disconnect_v5_callback_set
//Set the publish callback.  This is called when a message initiated with mosquitto_publish has been sent to the broker successfully.
mosquitto_publish_callback_set
//Set the publish callback.  This is called when a message initiated with mosquitto_publish has been sent to the broker.  This callback will be called both if the message is sent successfully,
//  or if the broker responded with an error, which will be reflected in the reason_code parameter.
mosquitto_publish_v5_callback_set
//Set the message callback.  This is called when a message is received from the broker.
mosquitto_message_callback_set
mosquitto_message_v5_callback_set
//Set the subscribe callback.  This is called when the broker responds to a subscription request.
mosquitto_subscribe_callback_set
mosquitto_subscribe_v5_callback_set
//Set the unsubscribe callback.  This is called when the broker responds to a unsubscription request.
mosquitto_unsubscribe_callback_set
mosquitto_unsubscribe_v5_callback_set
mosquitto_log_callback_set
*/

static int mosquitto_new_reinit(struct mqtt_app_config *cfg)
{
	zassert(cfg != NULL);
	cfg->mosq = mosquitto_new(cfg->id, cfg->clean_session, cfg);
	if(!cfg->mosq)
	{
		_MQTT_DBG_TRAP("------------%s-------------mosquitto_new",__func__);
		return ERROR;
	}
	if (mqtt_client_opts_config(cfg))
	{
		if(mqtt_config->mosq)
		{
			mosquitto_destroy(mqtt_config->mosq);
			mqtt_config->mosq = NULL;
		}
		_MQTT_DBG_TRAP("------------%s-------------mqtt_client_opts_config",__func__);
		return ERROR;
	}

	mosquitto_log_callback_set(cfg->mosq, mqtt_log_callback);
	mosquitto_user_data_set(cfg->mosq, cfg);

	if (cfg->mqtt_version == MQTT_PROTOCOL_V5)
	{
		mosquitto_subscribe_v5_callback_set(cfg->mosq, mqtt_sub_subscribe_v5_callback);
		mosquitto_connect_v5_callback_set(cfg->mosq, mqtt_sub_connect_v5_callback);
		mosquitto_message_v5_callback_set(cfg->mosq, mqtt_sub_message_v5_callback);
		mosquitto_publish_v5_callback_set(cfg->mosq, mqtt_sub_publish_v5_callback);
		//void mosquitto_disconnect_v5_callback_set(struct mosquitto *mosq, void (*on_disconnect)(struct mosquitto *, void *, int, const mosquitto_property *));
		//void mosquitto_unsubscribe_v5_callback_set(struct mosquitto *mosq, void (*on_unsubscribe)(struct mosquitto *, void *, int, const mosquitto_property *props));
	}
	else
	{
		mosquitto_subscribe_callback_set(cfg->mosq, mqtt_sub_subscribe_callback);
		mosquitto_connect_callback_set(cfg->mosq, mqtt_sub_connect_callback);
		mosquitto_message_callback_set(cfg->mosq, mqtt_sub_message_callback);
		mosquitto_publish_callback_set(cfg->mosq, mqtt_sub_publish_callback);
		//void mosquitto_disconnect_callback_set(struct mosquitto *mosq, void (*on_disconnect)(struct mosquitto *, void *, int));
		//void mosquitto_unsubscribe_callback_set(struct mosquitto *mosq, void (*on_unsubscribe)(struct mosquitto *, void *, int));
	}
	return OK;
}

zpl_bool mqtt_isenable_api(struct mqtt_app_config *cfg)
{
	zpl_bool enable = zpl_false;
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	enable = cfg->enable;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return enable;
}

int mqtt_enable_api(struct mqtt_app_config *cfg, zpl_bool enable)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->enable = enable;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_reload_api(struct mqtt_app_config *cfg)
{
	int ret = 0;
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->reload = zpl_true;
	ret = mosquitto__set_state(cfg->mosq, mosq_cs_disconnected);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return ret;
}


int mqtt_bind_address_api(struct mqtt_app_config *cfg,
		const zpl_char *bind_address)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if (cfg->bind_address)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->bind_address);
	}
	if(bind_address)
		cfg->bind_address = XSTRDUP(MTYPE_MQTT_CONF,bind_address);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_connect_host_api(struct mqtt_app_config *cfg, const zpl_char * host)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if (cfg->host)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->host);
	}
	if (host)
	{
		cfg->host = XSTRDUP(MTYPE_MQTT_CONF,host);
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_connect_port_api(struct mqtt_app_config *cfg, const zpl_uint16 port)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if (port)
	{
		cfg->port = port;
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_username_api(struct mqtt_app_config *cfg,
		const zpl_char * username)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if (cfg->username)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->username);
	}
	if (username)
	{
		cfg->username = XSTRDUP(MTYPE_MQTT_CONF,username);
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_password_api(struct mqtt_app_config *cfg,
		const zpl_char * password)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if (cfg->password)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->password);
	}
	if (password)
	{
		cfg->password = XSTRDUP(MTYPE_MQTT_CONF,password);
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}


#ifdef WITH_TLS
int mqtt_tls_cafile_api(struct mqtt_app_config *cfg, const zpl_char *cafile)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->cafile)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->cafile);
	}
	if(cafile)
		cfg->cafile = XSTRDUP(MTYPE_MQTT_CONF,cafile);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_tls_capath_api(struct mqtt_app_config *cfg, const zpl_char *capath)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->capath)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->capath);
	}
	if(capath)
		cfg->capath = XSTRDUP(MTYPE_MQTT_CONF,capath);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_tls_certfile_api(struct mqtt_app_config *cfg, const zpl_char *certfile)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->certfile)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->certfile);
	}
	if(certfile)
		cfg->certfile = XSTRDUP(MTYPE_MQTT_CONF,certfile);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_tls_ciphers_api(struct mqtt_app_config *cfg, const zpl_char *ciphers)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->ciphers)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->ciphers);
	}
	if(ciphers)
		cfg->ciphers = XSTRDUP(MTYPE_MQTT_CONF,ciphers);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_tls_insecure_api(struct mqtt_app_config *cfg, const zpl_bool insecure)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->insecure = insecure;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_tls_keyform_api(struct mqtt_app_config *cfg, const zpl_char *keyform)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->keyform)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->keyform);
	}
	if(keyform)
		cfg->keyform = XSTRDUP(MTYPE_MQTT_CONF,keyform);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_tls_keyfile_api(struct mqtt_app_config *cfg, const zpl_char *keyfile)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->keyfile)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->keyfile);
	}
	if(keyfile)
		cfg->keyfile = XSTRDUP(MTYPE_MQTT_CONF,keyfile);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}
#endif


int mqtt_id_api(struct mqtt_app_config *cfg, const zpl_char * format)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if (cfg->id)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->id);
	}
	if (format)
	{
		cfg->id = XSTRDUP(MTYPE_MQTT_CONF,format);
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_id_prefix_api(struct mqtt_app_config *cfg,
		const zpl_char * format)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if (cfg->id_prefix)
	{
		XFREE(MTYPE_MQTT_CONF, cfg->id_prefix);
	}
	if (format)
	{
		cfg->id_prefix = XSTRDUP(MTYPE_MQTT_CONF,format);
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_keepalive_api(struct mqtt_app_config *cfg,
		const zpl_uint32 keepalive)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->keepalive = keepalive;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_max_inflight_api(struct mqtt_app_config *cfg,
		const zpl_uint32 max_inflight)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->max_inflight = max_inflight;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}



#ifdef FINAL_WITH_TLS_PSK
int mqtt_tls_psk_api(struct mqtt_app_config *cfg, const zpl_char * format)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->psk)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->psk);
	}
	if(format)
	{
		cfg->psk = XSTRDUP(MTYPE_MQTT_CONF,format);
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_tls_psk_identity_api(struct mqtt_app_config *cfg, const zpl_char * format)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->psk_identity)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->psk_identity);
	}
	if(format)
	{
		cfg->psk_identity = XSTRDUP(MTYPE_MQTT_CONF,format);
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}
#endif

int mqtt_qos_api(struct mqtt_app_config *cfg, const mqtt_qos_level qos)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->qos = qos;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_retain_api(struct mqtt_app_config *cfg, const zpl_bool retain)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->retain = retain;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}


int mqtt_clean_session_api(struct mqtt_app_config *cfg, const zpl_bool clean_session)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->clean_session = clean_session;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

#ifdef WITH_SRV
int mqtt_use_srv_api(struct mqtt_app_config *cfg, const zpl_bool use_srv)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->use_srv = use_srv;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}
#endif

#ifdef WITH_TLS
int mqtt_tls_alpn_api(struct mqtt_app_config *cfg, const zpl_char * format)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->tls_alpn)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->tls_alpn);
	}
	if(format)
	{
		cfg->tls_alpn = XSTRDUP(MTYPE_MQTT_CONF,format);
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_tls_engine_api(struct mqtt_app_config *cfg, const zpl_char * format)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->tls_engine)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->tls_engine);
	}
	if(format)
	{
		cfg->tls_engine = XSTRDUP(MTYPE_MQTT_CONF,format);
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_tls_engine_kpass_sha1_api(struct mqtt_app_config *cfg, const zpl_char * format)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->tls_engine_kpass_sha1)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->tls_engine_kpass_sha1);
	}
	if(format)
	{
		cfg->tls_engine_kpass_sha1 = XSTRDUP(MTYPE_MQTT_CONF,format);
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_tls_version_api(struct mqtt_app_config *cfg, const zpl_char * format)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(cfg->tls_version)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->tls_version);
	}
	if(format)
	{
		cfg->tls_version = XSTRDUP(MTYPE_MQTT_CONF,format);
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}
#endif


int mqtt_debug_api(struct mqtt_app_config *cfg, zpl_uint32 loglevel)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->loglevel = loglevel;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}


int mqtt_version_api(struct mqtt_app_config *cfg,
		const zpl_uint32 mqtt_version)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->mqtt_version = mqtt_version;
	if (cfg->mqtt_version == MQTT_PROTOCOL_V5)
	{
		mosquitto_subscribe_v5_callback_set(cfg->mosq, mqtt_sub_subscribe_v5_callback);
		mosquitto_connect_v5_callback_set(cfg->mosq, mqtt_sub_connect_v5_callback);
		mosquitto_message_v5_callback_set(cfg->mosq, mqtt_sub_message_v5_callback);
		mosquitto_publish_v5_callback_set(cfg->mosq, mqtt_sub_publish_v5_callback);

		mosquitto_subscribe_callback_set(cfg->mosq, NULL);
		mosquitto_connect_callback_set(cfg->mosq, NULL);
		mosquitto_message_callback_set(cfg->mosq, NULL);
		mosquitto_publish_callback_set(cfg->mosq, NULL);
		//void mosquitto_disconnect_v5_callback_set(struct mosquitto *mosq, void (*on_disconnect)(struct mosquitto *, void *, int, const mosquitto_property *));
		//void mosquitto_unsubscribe_v5_callback_set(struct mosquitto *mosq, void (*on_unsubscribe)(struct mosquitto *, void *, int, const mosquitto_property *props));
	}
	else
	{
		mosquitto_subscribe_v5_callback_set(cfg->mosq, NULL);
		mosquitto_connect_v5_callback_set(cfg->mosq, NULL);
		mosquitto_message_v5_callback_set(cfg->mosq, NULL);
		mosquitto_publish_v5_callback_set(cfg->mosq, NULL);

		mosquitto_subscribe_callback_set(cfg->mosq, mqtt_sub_subscribe_callback);
		mosquitto_connect_callback_set(cfg->mosq, mqtt_sub_connect_callback);
		mosquitto_message_callback_set(cfg->mosq, mqtt_sub_message_callback);
		mosquitto_publish_callback_set(cfg->mosq, mqtt_sub_publish_callback);
		//void mosquitto_disconnect_callback_set(struct mosquitto *mosq, void (*on_disconnect)(struct mosquitto *, void *, int));
		//void mosquitto_unsubscribe_callback_set(struct mosquitto *mosq, void (*on_unsubscribe)(struct mosquitto *, void *, int));
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_session_expiry_interval_api(struct mqtt_app_config *cfg, zpl_uint32 interval)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->session_expiry_interval = interval;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}


int mqtt_sub_filter_out_api(struct mqtt_app_config *cfg,
		const zpl_char * filter)
{
	zassert(cfg != NULL);
	zassert(filter != NULL);
	if((cfg->sub.filter_out_count) == MQTT_TOPICS_MAX)
	{
		mqtt_err_printf(cfg, "Error: Too many Filter Topic");
		return ERROR;
	}
	if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if (mosquitto_validate_utf8(filter, strlen(filter)))
	{
		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);
		mqtt_err_printf(cfg, "Error: Malformed UTF-8 in -T argument.");
		return ERROR;
	}
	if (mosquitto_sub_topic_check(filter) == MOSQ_ERR_INVAL)
	{
		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);
		mqtt_err_printf(cfg,
				"Error: Invalid filter topic '%s', are all '+' and '#' wildcards correct?",
				filter);
		return ERROR;
	}

/*	if (!cfg->sub.filter_outs)
	{
		cfg->sub.filter_outs = XMALLOC(MTYPE_MQTT_FILTER, MQTT_TOPICS_MAX * sizeof(zpl_char *));
	}
	if (!cfg->sub.filter_outs)
	{
		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);
		mqtt_err_printf(cfg, "Error: Out of memory.");
		return ERROR;
	}*/
	cfg->sub.filter_out_count++;
	cfg->sub.filter_outs[cfg->sub.filter_out_count - 1] = XSTRDUP(MTYPE_MQTT_FILTER,filter);

	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_sub_filter_out_del_api(struct mqtt_app_config *cfg, zpl_char *filter)
{
	zpl_uint32 i = 0;
	zassert(cfg != NULL);
	for(i = 0; i < cfg->sub.filter_out_count; i++)
	{
		if(cfg->sub.filter_outs[i] && strcmp(cfg->sub.filter_outs[i], filter) == 0)
		{
			XFREE(MTYPE_MQTT_FILTER, cfg->sub.filter_outs[i]);
			cfg->sub.filter_outs[i] = NULL;
			cfg->sub.filter_out_count--;
			return OK;
		}
	}
	return ERROR;
}


int mqtt_sub_unsubscribe_api(struct mqtt_app_config *cfg,
		const zpl_char * filter)
{
	zassert(cfg != NULL);
	zassert(filter != NULL);
	if((cfg->sub.unsub_topic_count) == MQTT_TOPICS_MAX)
	{
		mqtt_err_printf(cfg, "Error: Too many Filter Topic");
		return ERROR;
	}
	if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if (mosquitto_validate_utf8(filter, strlen(filter)))
	{
		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);
		mqtt_err_printf(cfg, "Error: Malformed UTF-8 in -T argument.");
		return ERROR;
	}
	if (mosquitto_sub_topic_check(filter) == MOSQ_ERR_INVAL)
	{
		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);
		mqtt_err_printf(cfg,
				"Error: Invalid filter topic '%s', are all '+' and '#' wildcards correct?",
				filter);
		return ERROR;
	}

/*	if (!cfg->sub.unsub_topics)
	{
		cfg->sub.unsub_topics = XMALLOC(MTYPE_MQTT_TOPIC, MQTT_TOPICS_MAX * sizeof(zpl_char *));
	}

	if (!cfg->sub.unsub_topics)
	{
		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);
		mqtt_err_printf(cfg, "Error: Out of memory.");
		return ERROR;
	}*/
	cfg->sub.unsub_topic_count++;

	cfg->sub.unsub_topics[cfg->sub.unsub_topic_count - 1] = XSTRDUP(MTYPE_MQTT_TOPIC,filter);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_sub_unsubscribe_del_api(struct mqtt_app_config *cfg, zpl_char *topic)
{
	zpl_uint32 i = 0;
	zassert(cfg != NULL);
	for(i = 0; i < cfg->sub.unsub_topic_count; i++)
	{
		if(cfg->sub.unsub_topics[i] && strcmp(cfg->sub.unsub_topics[i], topic) == 0)
		{
			XFREE(MTYPE_MQTT_TOPIC, cfg->sub.unsub_topics[i]);
			cfg->sub.unsub_topics[i] = NULL;
			cfg->sub.unsub_topic_count--;
			return OK;
		}
	}
	return ERROR;
}



int mqtt_will_payload_api(struct mqtt_app_config *cfg, const zpl_char * format)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	//if(type == MQTT_MODE_SUB)
	{
		if (cfg->sub.will_payload)
		{
			XFREE(MTYPE_MQTT_DATA, cfg->sub.will_payload);
			cfg->sub.will_payload = NULL;
		}
		if (format)
		{
			cfg->sub.will_payload = XSTRDUP(MTYPE_MQTT_DATA,format);
			cfg->sub.will_payloadlen = strlen(cfg->sub.will_payload);
		}
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}


int mqtt_will_topic_api(struct mqtt_app_config *cfg, const zpl_char * will_topic)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);

	//if(type == MQTT_MODE_SUB)
	{
		if (cfg->sub.will_topic)
		{
			XFREE(MTYPE_MQTT_TOPIC,cfg->sub.will_topic);
			cfg->sub.will_payload = NULL;
		}
		if (will_topic)
		{
			if (mosquitto_validate_utf8(will_topic, strlen(will_topic)))
			{
				mqtt_err_printf(cfg, "Error: Malformed UTF-8 in --will-topic argument.");
				return -1;
			}
			if (mosquitto_pub_topic_check(will_topic) == MOSQ_ERR_INVAL)
			{
				mqtt_err_printf(cfg,
						"Error: Invalid will topic '%s', does it contain '+' or '#'?",
						will_topic);
				return -1;
			}
			cfg->sub.will_topic = XSTRDUP(MTYPE_MQTT_TOPIC,will_topic);
		}
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_will_qos_api(struct mqtt_app_config *cfg, const mqtt_qos_level will_qos)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	//if(type == MQTT_MODE_SUB)
		cfg->sub.will_qos = will_qos;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_will_retain_api(struct mqtt_app_config *cfg, const zpl_bool will_retain)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	//if(type == MQTT_MODE_SUB)
		cfg->sub.will_retain = will_retain;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}


/*int mqtt_sub_no_retain_api(struct mqtt_app_config *cfg,
		const zpl_bool no_retain)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->sub.no_retain = no_retain;
	if(no_retain)
		cfg->sub.sub_opts |= MQTT_SUB_OPT_SEND_RETAIN_NEVER;
	else
		cfg->sub.sub_opts &= ~MQTT_SUB_OPT_SEND_RETAIN_NEVER;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_sub_retain_as_published_api(struct mqtt_app_config *cfg)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->sub.sub_opts |= MQTT_SUB_OPT_RETAIN_AS_PUBLISHED;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}*/

int mqtt_sub_remove_retained_api(struct mqtt_app_config *cfg,
		const zpl_bool remove_retained)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->sub.remove_retained = remove_retained;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_option_api(struct mqtt_app_config *cfg, const zpl_uint32 option, zpl_bool set)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	//cfg->sub.no_retain = no_retain;
	//if(no_retain)
	//	cfg->sub.sub_opts |= MQTT_SUB_OPT_SEND_RETAIN_NEVER;
	//else
	//	cfg->sub.sub_opts &= ~MQTT_SUB_OPT_SEND_RETAIN_NEVER;

	//cfg->sub.sub_opts |= MQTT_SUB_OPT_RETAIN_AS_PUBLISHED;
	if(set)
	{
		cfg->sub.sub_opts |= option;
		if(option & MQTT_SUB_OPT_SEND_RETAIN_NEVER)
			cfg->sub.no_retain = zpl_true;
	}
	else
	{
		cfg->sub.sub_opts &= ~option;
		if(option & MQTT_SUB_OPT_SEND_RETAIN_NEVER)
			cfg->sub.no_retain = zpl_false;
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_module_commit_api(struct mqtt_app_config *cfg)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(host_loadconfig_done ())
	{
		if(cfg->connectd == zpl_true && cfg->mosq)
		{
			mosquitto_disconnect(cfg->mosq);
			cfg->connectd = zpl_false;
		}
		if(cfg->mosq)
		{
			mosquitto_destroy(cfg->mosq);
			cfg->mosq = NULL;
		}
		if(mosquitto_new_reinit(cfg) == ERROR)
		{
			if(cfg->mosq)
			{
				mosquitto_destroy(cfg->mosq);
				cfg->mosq = NULL;
			}
			if(cfg->mutex)
				os_mutex_unlock(cfg->mutex);
			return ERROR;
		}
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}




int mqtt_module_init(void)
{
	mqtt_config = XMALLOC(MTYPE_MQTT, sizeof(struct mqtt_app_config));
	zassert(mqtt_config != NULL);
	mosquitto_lib_init();
	mqtt_config_default_init(mqtt_config, MQTT_MODE_SUB);
	/*
	 * load config
	 */
	mqtt_config->enable = zpl_false;
	if (mqtt_client_id_generate(mqtt_config) != MOSQ_ERR_SUCCESS)
	{
		_MQTT_DBG_TRAP("------------%s-------------mqtt_client_id_generate",__func__);
		mosquitto_lib_cleanup();
		mqtt_config_default_cleanup(mqtt_config);
		return ERROR;
	}
	if (mqtt_config_cfg_check(mqtt_config) != MOSQ_ERR_SUCCESS)
	{
		_MQTT_DBG_TRAP("------------%s-------------mqtt_config_cfg_check",__func__);
		mosquitto_lib_cleanup();
		mqtt_config_default_cleanup(mqtt_config);
		return ERROR;
	}
	if(host_loadconfig_done ())
	{
		if(mosquitto_new_reinit(mqtt_config) == ERROR)
		{
			_MQTT_DBG_TRAP("------------%s-------------mosquitto_new_reinit",__func__);
			mosquitto_lib_cleanup();
			mqtt_config_default_cleanup(mqtt_config);
			return ERROR;
		}
	}
	return OK;
}

int mqtt_module_exit(void)
{
	zassert(mqtt_config != NULL);
	if(mqtt_config->mosq)
	{
		mosquitto_destroy(mqtt_config->mosq);
		mqtt_config->mosq = NULL;
	}
	mosquitto_lib_cleanup();
	mqtt_config_default_cleanup(mqtt_config);
	if(mqtt_config)
		XFREE(MTYPE_MQTT, mqtt_config);
	return OK;
}


static int mqtt_app_task (void *argv)
{
	int ret = 0;
	enum mosquitto_client_state state;
	struct mqtt_app_config *cfg = argv;
	zassert(cfg != NULL);

	host_waitting_loadconfig();
	while (!cfg->enable)
	{
		os_sleep (1);
	}
	os_sleep (1);
	if(!cfg->mosq)
	{
		if(mosquitto_new_reinit(cfg) == ERROR)
		{
			mosquitto_lib_cleanup();
			mqtt_config_default_cleanup(cfg);
			return ERROR;
		}
	}
	while (OS_TASK_TRUE())
	{
		if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);

		if(cfg->connectd != zpl_true && cfg->host)
		{
			ret = mqtt_client_connect(cfg);
		}
		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);

		if (ret != MOSQ_ERR_SUCCESS)
		{
			os_sleep(1);
			continue;
		}
		cfg->connectd = zpl_true;

#if 0
		do
		{
			ret = mosquitto_loop(cfg->mosq, 1000, 1);

			if(cfg->mutex)
				os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);

			state = mosquitto__get_state(cfg->mosq);
			if(state == mosq_cs_disconnecting || state == mosq_cs_disconnected)
			{
				cfg->connectd = zpl_false;
				if(cfg->reload)
				{
					cfg->reload = zpl_false;
					if(cfg->mosq)
						mosquitto_destroy(cfg->mosq);

					if(mosquitto_new_reinit(cfg) != OK)
					{
						if(cfg->mutex)
							os_mutex_unlock(cfg->mutex);
						continue;
					}
				}
			}
			if(cfg->taskquit)
				break;

			if(cfg->mutex)
				os_mutex_unlock(cfg->mutex);

		} while (ret == MOSQ_ERR_SUCCESS);
#else
		ret = mosquitto_loop_forever(cfg->mosq, 1000, 1);
		if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);

		state = mosquitto__get_state(cfg->mosq);
		if(state == mosq_cs_disconnecting || state == mosq_cs_disconnected)
		{
			cfg->connectd = zpl_false;
			if(cfg->reload)
			{
				cfg->reload = zpl_false;
				if(cfg->mosq)
					mosquitto_destroy(cfg->mosq);

				if(mosquitto_new_reinit(cfg) != OK)
				{
					if(cfg->mutex)
						os_mutex_unlock(cfg->mutex);
					continue;
				}
			}
		}
		if(cfg->taskquit)
			break;

		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);
#endif
	}

	cfg->taskquit = zpl_false;

	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);

	mqtt_module_exit();

	return OK;
}


int mqtt_module_task_init(void)
{
	zassert(mqtt_config != NULL);
	if (mqtt_config->taskid == 0)
		mqtt_config->taskid = os_task_create("mqttTask", OS_TASK_DEFAULT_PRIORITY, 0,
				mqtt_app_task, mqtt_config, OS_TASK_DEFAULT_STACK);
	if (mqtt_config->taskid)
	{
		//module_setup_task(MODULE_MQTT, mqtt_config->taskid);
		return OK;
	}
	return ERROR;
}

int mqtt_module_task_exit(void)
{
	zassert(mqtt_config != NULL);
	if(mqtt_config->mutex)
		os_mutex_lock(mqtt_config->mutex, OS_WAIT_FOREVER);
	if(mqtt_config->mosq)
		mosquitto_loop_stop(mqtt_config->mosq, zpl_true);
	mqtt_config->taskquit = zpl_true;
	if(mqtt_config->mutex)
		os_mutex_unlock(mqtt_config->mutex);
	return OK;
}


struct module_list module_list_mqtt = 
{ 
	.module=MODULE_MQTT, 
	.name="MQTT\0", 
	.module_init=mqtt_module_init, 
	.module_exit=mqtt_module_exit, 
	.module_task_init=mqtt_module_task_init, 
	.module_task_exit=mqtt_module_task_exit, 
	.module_cmd_init=cmd_mqtt_init, 
	.flags = 0,
	.taskid=0,
};