/*
 * mqtt_app_conf.c
 *
 *  Created on: 2019年12月15日
 *      Author: zhurish
 */
#include "auto_include.h"
#include "zplos_include.h"

#include "mqtt-config.h"
#include <mqtt_protocol.h>
#include <mosquitto.h>
#include "mqtt_app_conf.h"
#include "mqtt_app_util.h"
#include "mqtt_app_publish.h"
#include "mqtt_app_subscribed.h"
#include "mqtt_app_api.h"


struct mqtt_app_config *mqtt_config = NULL;

/*
 * 1: mosquitto
 * 2: mosquitto_sub -t rimelink
 * 3: mosquitto_pub -t rimelink -h localhost -m "hello, mosquitto"
 */


void mqtt_config_default_init(struct mqtt_app_config *cfg, mqtt_mode_t pub_or_sub)
{
	zassert(cfg != NULL);
	memset(cfg, 0, sizeof(struct mqtt_app_config));

	cfg->mutex = os_mutex_name_create("mqtt->mutex");

	cfg->mqtt_mode = pub_or_sub;

	//if (pub_or_sub == MQTT_MODE_RR)
	//{
	//	cfg->mqtt_version = MQTT_PROTOCOL_V5;
	//}
	//else
	{
		cfg->mqtt_version = MQTT_PROTOCOL_V311;
	}
	cfg->keepalive = MQTT_KEEPALIVE_DETAULT;//给代理发送PING命令（目的在于告知代理该客户端连接保持且在正常工作）的间隔时间，默认是60s

	//cfg->bind_address;//服务绑定的IP地址

	//mqtt_connect_host_api(cfg, "192.168.3.222");
	mqtt_connect_port_api(cfg, os_netservice_port_get("mqtt_port")/*MQTT_PORT_DETAULT*/);

	cfg->qos = 0;
	cfg->retain = zpl_true;//如果指定该选项，该条消息将被保留做为最后一条收到的消息。下一个订阅消息者将能至少收到该条消息。

	mqtt_id_prefix_api(cfg, "aabbcc");//客户端ID/ID前缀

	cfg->debug = zpl_true;
	cfg->quiet = zpl_false;// 如果指定该选项，则不会有任何错误被打印，当然，这排除了无效的用户输入所引起的错误消息

	cfg->clean_session = zpl_true;//禁止'clean session'选项，即如果客户端断开连接，这个订阅仍然保留来接收随后到的QoS为1和2的
									//消息，当改客户端重新连接之后，它将接收到已排在队列中的消息。建议使用此选项时，
									//客户端id选 项设为--id
	/*
	cfg->session_expiry_interval;

	cfg->will_topic;//指定客户端意外断开时，Will消息发送到的主题
	cfg->will_payload;//如果指定该选项，则万一客户端意外和代理服务器断开，则该消息将被保留在服务端并发送出去，
									//该选项必须同 时用--will-topic指定主题
	cfg->will_payloadlen;
	cfg->will_qos;//指定Will的服务质量，默认是0.必须和选项 --will-topic同时使用.
	cfg->will_retain;//如果指定该选项，则万一客户端意外断开，已被发送的消息将被当做retained消息。
								//必须和选项 --will-topic同时使 用.
	 */
	/* sub */
	//cfg->sub.topic_count = 1;
	//mqtt_client_add_topic(cfg, "aa");//设置订阅主题
	/*
 	cfg->sub.topics;  //指定消息所发布到哪个主题
	cfg->sub.no_retain;
	cfg->sub.remove_retained;
	cfg->sub.filter_outs;
	cfg->sub.filter_out_count;
	cfg->sub.unsub_topics;
	cfg->sub.unsub_topic_count;
	cfg->sub.format;
	cfg->sub.sub_opts;
	*/

	/* pub*/
	/*
	cfg->pub.pub_mode;
	cfg->pub.file_input;
	cfg->pub.message; //从命令行发送一条消息，-m后面跟发送的消息内容
	cfg->pub.msglen;
	cfg->pub.topic; //指定消息所发布到哪个主题。
	cfg->pub.repeat_count;
	cfg->pub.repeat_delay;
	cfg->pub.have_topic_alias;
	 */
/*	cfg->pub.repeat_count = 1;
	cfg->pub.repeat_delay.tv_sec = 0;
	cfg->pub.repeat_delay.tv_usec = 0;*/
}

void mqtt_config_default_cleanup(struct mqtt_app_config *cfg)
{
	zpl_uint32 i;
	zassert(cfg != NULL);
	if(cfg->mutex)
		os_mutex_destroy(cfg->mutex);
	if (cfg->id)
		XFREE(MTYPE_MQTT_CONF,cfg->id);
	if (cfg->id_prefix)
		XFREE(MTYPE_MQTT_CONF,cfg->id_prefix);
	if (cfg->host)
		XFREE(MTYPE_MQTT_CONF,cfg->host);
/*	if (cfg->pub.file_input)
		XFREE(MTYPE_MQTT_CONF,cfg->pub.file_input);

	if (cfg->pub.message)
		XFREE(MTYPE_MQTT_MESSAGE,cfg->pub.message);

	if (cfg->pub.topic)
		XFREE(MTYPE_MQTT_TOPIC,cfg->pub.topic);*/

	if (cfg->bind_address)
		XFREE(MTYPE_MQTT_CONF,cfg->bind_address);
	if (cfg->username)
		XFREE(MTYPE_MQTT_CONF,cfg->username);
	if (cfg->password)
		XFREE(MTYPE_MQTT_CONF,cfg->password);
	if (cfg->sub.will_topic)
		XFREE(MTYPE_MQTT_TOPIC,cfg->sub.will_topic);
	if (cfg->sub.will_payload)
		XFREE(MTYPE_MQTT_DATA,cfg->sub.will_payload);
/*	if (cfg->pub.will_topic)
		XFREE(MTYPE_MQTT_TOPIC,cfg->pub.will_topic);
	if (cfg->pub.will_payload)
		XFREE(MTYPE_MQTT_DATA,cfg->pub.will_payload);*/
/*	if (cfg->sub.format)
		XFREE(MTYPE_MQTT_CONF,cfg->sub.format);*/


#ifdef WITH_TLS
	if(cfg->cafile)
		XFREE(MTYPE_MQTT_CONF,cfg->cafile);
	if(cfg->capath)
		XFREE(MTYPE_MQTT_CONF,cfg->capath);
	if(cfg->certfile)
		XFREE(MTYPE_MQTT_CONF,cfg->certfile);
	if(cfg->keyfile)
		XFREE(MTYPE_MQTT_CONF,cfg->keyfile);
	if(cfg->ciphers)
		XFREE(MTYPE_MQTT_CONF,cfg->ciphers);
	if(cfg->tls_alpn)
		XFREE(MTYPE_MQTT_CONF,cfg->tls_alpn);
	if(cfg->tls_version)
		XFREE(MTYPE_MQTT_CONF,cfg->tls_version);
	if(cfg->tls_engine)
		XFREE(MTYPE_MQTT_CONF,cfg->tls_engine);
	if(cfg->tls_engine_kpass_sha1)
		XFREE(MTYPE_MQTT_CONF,cfg->tls_engine_kpass_sha1);
	if(cfg->keyform)
		XFREE(MTYPE_MQTT_CONF,cfg->keyform);

#  ifdef FINAL_WITH_TLS_PSK
	if(cfg->psk)
		XFREE(MTYPE_MQTT_CONF,cfg->psk);
	if(cfg->psk_identity)
		XFREE(MTYPE_MQTT_CONF,cfg->psk_identity);
#  endif
#endif
	if (cfg->sub.topics)
	{
		for (i = 0; i < cfg->sub.topic_count; i++)
		{
			if (cfg->sub.topics[i])
				XFREE(MTYPE_MQTT_TOPIC,cfg->sub.topics[i]);
		}
		//XFREE(MTYPE_MQTT_TOPIC,cfg->sub.topics);
	}
	if (cfg->sub.filter_outs)
	{
		for (i = 0; i < cfg->sub.filter_out_count; i++)
		{
			if (cfg->sub.filter_outs[i])
				XFREE(MTYPE_MQTT_FILTER,cfg->sub.filter_outs[i]);
		}
		//XFREE(MTYPE_MQTT_FILTER,cfg->sub.filter_outs);
	}
	if (cfg->sub.unsub_topics)
	{
		for (i = 0; i < cfg->sub.unsub_topic_count; i++)
		{
			if (cfg->sub.unsub_topics[i])
				XFREE(MTYPE_MQTT_TOPIC,cfg->sub.unsub_topics[i]);
		}
		//XFREE(MTYPE_MQTT_TOPIC,cfg->sub.unsub_topics);
	}
#ifdef WITH_SOCKS
	if(cfg->socks5_host)
		XFREE(MTYPE_MQTT_CONF,cfg->socks5_host);
	if(cfg->socks5_username)
		XFREE(MTYPE_MQTT_CONF,cfg->socks5_username);
	if(cfg->socks5_password)
		XFREE(MTYPE_MQTT_CONF,cfg->socks5_password);
#endif
	if (cfg->connect_props)
		mosquitto_property_free_all(&cfg->connect_props);
	if (cfg->publish_props)
		mosquitto_property_free_all(&cfg->publish_props);
	if (cfg->subscribe_props)
		mosquitto_property_free_all(&cfg->subscribe_props);
	if (cfg->unsubscribe_props)
		mosquitto_property_free_all(&cfg->unsubscribe_props);
	if (cfg->disconnect_props)
		mosquitto_property_free_all(&cfg->disconnect_props);
	if (cfg->will_props)
		mosquitto_property_free_all(&cfg->will_props);
}






int mqtt_config_cfg_check(struct mqtt_app_config *cfg)
{
	int rc = 0;
	zassert(cfg != NULL);
	//if(pub_or_sub == MQTT_MODE_SUB)
	{
		if (cfg->sub.will_payload && !cfg->sub.will_topic)
		{
			mqtt_err_printf(cfg,
					"Error: Will payload given, but no will topic given.");
			return -1;
		}
		if (cfg->sub.will_retain && !cfg->sub.will_topic)
		{
			mqtt_err_printf(cfg, "Error: Will retain given, but no will topic given.");
			return -1;
		}
	}
/*	else if(pub_or_sub == MQTT_MODE_PUB)
	{
		if (cfg->pub.will_payload && !cfg->pub.will_topic)
		{
			mqtt_err_printf(cfg,
					"Error: Will payload given, but no will topic given.");
			return -1;
		}
		if (cfg->pub.will_retain && !cfg->pub.will_topic)
		{
			mqtt_err_printf(cfg, "Error: Will retain given, but no will topic given.");
			return -1;
		}
	}*/
#ifdef WITH_TLS
	if((cfg->certfile && !cfg->keyfile) || (cfg->keyfile && !cfg->certfile))
	{
		mqtt_err_printf(cfg, "Error: Both certfile and keyfile must be provided if one of them is set.");
		return -1;
	}
	if((cfg->keyform && !cfg->keyfile))
	{
		mqtt_err_printf(cfg, "Error: If keyform is set, keyfile must be also specified.");
		return -1;
	}
	if((cfg->tls_engine_kpass_sha1 && (!cfg->keyform || !cfg->tls_engine)))
	{
		mqtt_err_printf(cfg, "Error: when using tls-engine-kpass-sha1, both tls-engine and keyform must also be provided.");
		return -1;
	}
#endif
#ifdef FINAL_WITH_TLS_PSK
	if((cfg->cafile || cfg->capath) && cfg->psk)
	{
		mqtt_err_printf(cfg, "Error: Only one of --psk or --cafile/--capath may be used at once.");
		return -1;
	}
	if(cfg->psk && !cfg->psk_identity)
	{
		mqtt_err_printf(cfg, "Error: --psk-identity required if --psk used.");
		return -1;
	}
#endif

	if (cfg->mqtt_version == MQTT_PROTOCOL_V5)
	{
		if (cfg->clean_session == false && cfg->session_expiry_interval == -1)
		{
			/* User hasn't set session-expiry-interval, but has cleared clean
			 * session so default to persistent session. */
			cfg->session_expiry_interval = UINT32_MAX;
		}
		if (cfg->session_expiry_interval > 0)
		{
			if (cfg->session_expiry_interval == UINT32_MAX
					&& (cfg->id_prefix || !cfg->id))
			{
				mqtt_err_printf(cfg,
						"Error: You must provide a client id if you are using an infinite session expiry interval.");
				return -1;
			}
			rc = mosquitto_property_add_int32(&cfg->connect_props,
					MQTT_PROP_SESSION_EXPIRY_INTERVAL,
					cfg->session_expiry_interval);
			if (rc)
			{
				mqtt_err_printf(cfg,
						"Error adding property session-expiry-interval");
			}
		}
	}
	else
	{
		if (cfg->clean_session == false && (cfg->id_prefix || !cfg->id))
		{
			mqtt_err_printf(cfg,
					"Error: You must provide a client id if you are using the -c option.");
			return -1;
		}
	}

/*	if (pub_or_sub == MQTT_MODE_SUB)
	{
		if (cfg->sub.topic_count == 0)
		{
			mqtt_err_printf(cfg,
					"Error: You must specify a topic to subscribe to.");
			return -1;
		}
	}*/

	if (!cfg->host)
	{
		cfg->host = XSTRDUP(MTYPE_MQTT_CONF, "localhost");
		if (!cfg->host)
		{
			mqtt_err_printf(cfg, "Error: Out of memory.");
			return -1;
		}
	}
	rc = mosquitto_property_check_all(CMD_CONNECT, cfg->connect_props);
	if (rc)
	{
		mqtt_err_printf(cfg, "Error in CONNECT properties: %s",
				mosquitto_strerror(rc));
		return -1;
	}
	rc = mosquitto_property_check_all(CMD_PUBLISH, cfg->publish_props);
	if (rc)
	{
		mqtt_err_printf(cfg, "Error in PUBLISH properties: %s",
				mosquitto_strerror(rc));
		return -1;
	}
	rc = mosquitto_property_check_all(CMD_SUBSCRIBE, cfg->subscribe_props);
	if (rc)
	{
		mqtt_err_printf(cfg, "Error in SUBSCRIBE properties: %s",
				mosquitto_strerror(rc));
		return -1;
	}
	rc = mosquitto_property_check_all(CMD_UNSUBSCRIBE, cfg->unsubscribe_props);
	if (rc)
	{
		mqtt_err_printf(cfg, "Error in UNSUBSCRIBE properties: %s",
				mosquitto_strerror(rc));
		return -1;
	}
	rc = mosquitto_property_check_all(CMD_DISCONNECT, cfg->disconnect_props);
	if (rc)
	{
		mqtt_err_printf(cfg, "Error in DISCONNECT properties: %s",
				mosquitto_strerror(rc));
		return -1;
	}
	rc = mosquitto_property_check_all(CMD_WILL, cfg->will_props);
	if (rc)
	{
		mqtt_err_printf(cfg, "Error in Will properties: %s",
				mosquitto_strerror(rc));
		return -1;
	}
	return MOSQ_ERR_SUCCESS;
}


