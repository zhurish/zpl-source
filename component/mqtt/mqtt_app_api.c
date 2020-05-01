/*
 * mqtt_app_api.c
 *
 *  Created on: 2019年7月19日
 *      Author: DELL
 */

#include "zebra.h"
#include "memory.h"
#include "mqtt-config.h"
#include <mqtt_protocol.h>
#include <mosquitto.h>
#include "mosquitto_internal.h"
#include "mqtt_app_conf.h"
#include "mqtt_app_util.h"
#include "mqtt_app_publish.h"
#include "mqtt_app_subscribed.h"
#include "mqtt_app_api.h"

static int mosquitto_new_reinit(struct mqtt_app_config *cfg)
{
	zassert(cfg != NULL);
	cfg->mosq = mosquitto_new(cfg->id, cfg->clean_session, cfg);
	if(!cfg->mosq)
	{
		printf("------------%s-------------mosquitto_new\r\n",__func__);
		return ERROR;
	}
	if (cfg->debug)
	{
		mosquitto_log_callback_set(cfg->mosq, mqtt_log_callback);
	}
	if (mqtt_client_opts_config(cfg))
	{
		printf("------------%s-------------mqtt_client_opts_config\r\n",__func__);
		return ERROR;
	}
	mosquitto_subscribe_callback_set(cfg->mosq, mqtt_sub_subscribe_callback);
	mosquitto_connect_v5_callback_set(cfg->mosq, mqtt_sub_connect_callback);
	mosquitto_message_v5_callback_set(cfg->mosq, mqtt_sub_message_callback);
	return OK;
}


int mqtt_enable_api(struct mqtt_app_config *cfg, BOOL enable)
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
	cfg->reload = TRUE;
	ret = mosquitto__set_state(cfg->mosq, mosq_cs_disconnected);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return ret;
}


int mqtt_bind_address_api(struct mqtt_app_config *cfg,
		const char *bind_address)
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

int mqtt_connect_host_api(struct mqtt_app_config *cfg, const char * host)
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

int mqtt_connect_port_api(struct mqtt_app_config *cfg, const int port)
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
		const char * username)
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
		const char * password)
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
int mqtt_tls_cafile_api(struct mqtt_app_config *cfg, const char *cafile)
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

int mqtt_tls_capath_api(struct mqtt_app_config *cfg, const char *capath)
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

int mqtt_tls_certfile_api(struct mqtt_app_config *cfg, const char *certfile)
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

int mqtt_tls_ciphers_api(struct mqtt_app_config *cfg, const char *ciphers)
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

int mqtt_tls_insecure_api(struct mqtt_app_config *cfg, const bool insecure)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->insecure = insecure;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_tls_ciphers_api(struct mqtt_app_config *cfg, const char *keyform)
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

int mqtt_tls_keyfile_api(struct mqtt_app_config *cfg, const char *keyfile)
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


int mqtt_id_api(struct mqtt_app_config *cfg, const char * format)
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
		const char * format)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if (cfg->id_prefix)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->id_prefix);
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
		const int keepalive)
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
		const int max_inflight)
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
int mqtt_tls_psk_api(struct mqtt_app_config *cfg, const char * format)
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

int mqtt_tls_psk_identity_api(struct mqtt_app_config *cfg, const char * format)
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

int mqtt_retain_api(struct mqtt_app_config *cfg, const bool retain)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->retain = retain;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}



#ifdef WITH_SRV
int mqtt_use_srv_api(struct mqtt_app_config *cfg, const bool use_srv)
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
int mqtt_tls_alpn_api(struct mqtt_app_config *cfg, const char * format)
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

int mqtt_tls_engine_api(struct mqtt_app_config *cfg, const char * format)
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

int mqtt_tls_engine_kpass_sha1_api(struct mqtt_app_config *cfg, const char * format)
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

int mqtt_tls_version_api(struct mqtt_app_config *cfg, const char * format)
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

int mqtt_version_api(struct mqtt_app_config *cfg,
		const int mqtt_version)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->mqtt_version = mqtt_version;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}


int mqtt_will_payload_api(struct mqtt_app_config *cfg,
		mqtt_mode_t type, const char * format)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(type == MQTT_MODE_SUB)
	{
		if (cfg->sub.will_payload)
		{
			XFREE(MTYPE_MQTT_DATA,cfg->sub.will_payload);
		}
		if (format)
		{
			cfg->sub.will_payload = XSTRDUP(MTYPE_MQTT_DATA,format);
			cfg->sub.will_payloadlen = strlen(cfg->sub.will_payload);
		}
	}
	else if(type == MQTT_MODE_PUB)
	{
		if (cfg->pub.will_payload)
		{
			XFREE(MTYPE_MQTT_DATA,cfg->pub.will_payload);
		}
		if (format)
		{
			cfg->pub.will_payload = XSTRDUP(MTYPE_MQTT_DATA,format);
			cfg->pub.will_payloadlen = strlen(cfg->pub.will_payload);
		}
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}


int mqtt_will_topic_api(struct mqtt_app_config *cfg,
		mqtt_mode_t type, const char * will_topic)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);

	if (mosquitto_validate_utf8(will_topic, strlen(will_topic)))
	{
		mqtt_err_printf(cfg, "Error: Malformed UTF-8 in --will-topic argument.");
		return -1;
	}
	if (mosquitto_pub_topic_check(will_topic) == MOSQ_ERR_INVAL)
	{
		mqtt_err_printf(cfg,
				"Error: Invalid will topic '%s', does it contain '+' or '#'?\n",
				will_topic);
		return -1;
	}
	if(type == MQTT_MODE_SUB)
	{
		if (cfg->sub.will_topic)
		{
			XFREE(MTYPE_MQTT_TOPIC,cfg->sub.will_topic);
		}
		if (will_topic)
		{
			cfg->sub.will_topic = XSTRDUP(MTYPE_MQTT_TOPIC,will_topic);
		}
	}
	else if(type == MQTT_MODE_PUB)
	{
		if (cfg->pub.will_topic)
		{
			XFREE(MTYPE_MQTT_TOPIC,cfg->pub.will_topic);
		}
		if (will_topic)
		{
			cfg->pub.will_topic = XSTRDUP(MTYPE_MQTT_TOPIC,will_topic);
		}
	}
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_will_qos_api(struct mqtt_app_config *cfg,
		mqtt_mode_t type, const mqtt_qos_level will_qos)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(type == MQTT_MODE_SUB)
		cfg->sub.will_qos = will_qos;
	else if(type == MQTT_MODE_PUB)
		cfg->pub.will_qos = will_qos;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_will_retain_api(struct mqtt_app_config *cfg,
		mqtt_mode_t type, const bool will_retain)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(type == MQTT_MODE_SUB)
		cfg->sub.will_retain = will_retain;
	else if(type == MQTT_MODE_PUB)
		cfg->pub.will_retain = will_retain;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}



int mqtt_sub_filter_out_api(struct mqtt_app_config *cfg,
		const char * filter)
{
	zassert(cfg != NULL);
	zassert(filter != NULL);
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
				"Error: Invalid filter topic '%s', are all '+' and '#' wildcards correct?\n",
				filter);
		return ERROR;
	}
	cfg->sub.filter_out_count++;
	cfg->sub.filter_outs = XREALLOC(MTYPE_MQTT_FILTER, cfg->sub.filter_outs,
			cfg->sub.filter_out_count * sizeof(char *));
	if (!cfg->sub.filter_outs)
	{
		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);
		mqtt_err_printf(cfg, "Error: Out of memory.");
		return ERROR;
	}
	cfg->sub.filter_outs[cfg->sub.filter_out_count - 1] = XSTRDUP(MTYPE_MQTT_FILTER,filter);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_sub_unsubscribe_api(struct mqtt_app_config *cfg,
		const char * filter)
{
	zassert(cfg != NULL);
	zassert(filter != NULL);
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
	cfg->sub.unsub_topic_count++;
	cfg->sub.unsub_topics = XREALLOC(MTYPE_MQTT_TOPIC, cfg->sub.unsub_topics,
			cfg->sub.unsub_topic_count * sizeof(char *));
	if (!cfg->sub.unsub_topics)
	{
		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);
		mqtt_err_printf(cfg, "Error: Out of memory.");
		return ERROR;
	}
	cfg->sub.unsub_topics[cfg->sub.unsub_topic_count - 1] = XSTRDUP(MTYPE_MQTT_TOPIC,filter);
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}

int mqtt_pub_repeat_count_delay_api(struct mqtt_app_config *cfg,
		const int repeat_count, const float repeat_delay)
{
	zassert(cfg != NULL);
	float f = repeat_delay;
	if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	if(repeat_delay != 0.0000000)
	{
		f *= 1.0e6;
		cfg->pub.repeat_delay.tv_sec = (int) f / 1e6;
		cfg->pub.repeat_delay.tv_usec = (int) f % 1000000;
	}
	if(repeat_count)
		cfg->pub.repeat_count = repeat_count;
	if(cfg->mutex)
		os_mutex_unlock(cfg->mutex);
	return OK;
}



/*
int mqtt_config_sub_mode(struct mqtt_app_config *cfg,
		const char * format)
{
	zassert(cfg != NULL);
	if (cfg->sub.format)
	{
		XFREE(MTYPE_MQTT_CONF,cfg->sub.format);
	}
	if (format)
	{
		cfg->sub.format = XSTRDUP(MTYPE_MQTT_CONF,format);
	}
	return MOSQ_ERR_SUCCESS;
}*/


int mqtt_sub_no_retain_api(struct mqtt_app_config *cfg,
		const bool no_retain)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->sub.no_retain = no_retain;
	cfg->sub.sub_opts |= MQTT_SUB_OPT_SEND_RETAIN_NEVER;
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
}

int mqtt_sub_remove_retained_api(struct mqtt_app_config *cfg,
		const bool remove_retained)
{
	zassert(cfg != NULL);
	if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);
	cfg->sub.remove_retained = remove_retained;
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
	mqtt_config->enable = TRUE;
	if (mqtt_client_id_generate(mqtt_config) != MOSQ_ERR_SUCCESS)
	{
		printf("------------%s-------------mqtt_client_id_generate\r\n",__func__);
		mosquitto_lib_cleanup();
		mqtt_config_default_cleanup(mqtt_config);
		return ERROR;
	}
	if (mqtt_config_cfg_check(mqtt_config,  MQTT_MODE_SUB) != MOSQ_ERR_SUCCESS)
	{
		printf("------------%s-------------mqtt_config_cfg_check\r\n",__func__);
		mosquitto_lib_cleanup();
		mqtt_config_default_cleanup(mqtt_config);
		return ERROR;
	}

	if(mosquitto_new_reinit(mqtt_config) == ERROR)
	{
		printf("------------%s-------------mosquitto_new_reinit\r\n",__func__);
		mosquitto_lib_cleanup();
		mqtt_config_default_cleanup(mqtt_config);
		return ERROR;
	}
	return OK;
}

int mqtt_module_exit(void)
{
	zassert(mqtt_config != NULL);
	if(mqtt_config->mosq)
		mosquitto_destroy(mqtt_config->mosq);
	mosquitto_lib_cleanup();
	mqtt_config_default_cleanup(mqtt_config);
	return OK;
}


static int mqtt_app_task (void *argv)
{
	int ret = 0;
	enum mosquitto_client_state state;
	struct mqtt_app_config *cfg = argv;
	zassert(argv != NULL);

	while (!os_load_config_done ())
	{
		os_sleep (1);
	}
	while (!cfg->enable)
	{
		os_sleep (1);
	}
	while (1)
	{
		if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);

		ret = mqtt_client_connect(cfg);

		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);

		if (ret != MOSQ_ERR_SUCCESS)
		{
			os_sleep(1);
			continue;
		}

		ret = mosquitto_loop_forever(cfg->mosq, -1, 1);

		if(cfg->mutex)
			os_mutex_lock(cfg->mutex, OS_WAIT_FOREVER);

		state = mosquitto__get_state(cfg->mosq);
		if(state == mosq_cs_disconnecting || state == mosq_cs_disconnected)
		{
			if(cfg->reload)
			{
				cfg->reload = FALSE;
				if(cfg->mosq)
					mosquitto_destroy(cfg->mosq);

				//mosquitto_lib_cleanup();

				if(mosquitto_new_reinit(cfg) != OK)
					break;
			}
		}
		if(cfg->mutex)
			os_mutex_unlock(cfg->mutex);
	}
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
		return OK;
	return ERROR;
}

int mqtt_module_task_exit(void)
{
	zassert(mqtt_config != NULL);
	return OK;
}
