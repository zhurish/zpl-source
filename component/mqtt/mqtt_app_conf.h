/*
 * mqtt_app_conf.h
 *
 *  Created on: 2019年12月15日
 *      Author: zhurish
 */

#ifndef __MQTT_APP_CONF_H__
#define __MQTT_APP_CONF_H__


/* pub_client.c modes */
#define MSGMODE_NONE 0
#define MSGMODE_CMD 1
#define MSGMODE_STDIN_LINE 2
#define MSGMODE_STDIN_FILE 3
#define MSGMODE_FILE 4
#define MSGMODE_NULL 5

#define CLIENT_PUB 1
#define CLIENT_SUB 2
#define CLIENT_RR 3
#define CLIENT_RESPONSE_TOPIC 4


struct mqtt_app_sub {
	char **topics; /* sub */
	int topic_count; /* sub */
	bool exit_after_sub; /* sub */
	bool no_retain; /* sub */
	bool retained_only; /* sub */
	bool remove_retained; /* sub */
	char **filter_outs; /* sub */
	int filter_out_count; /* sub */
	char **unsub_topics; /* sub */
	int unsub_topic_count; /* sub */
	bool verbose; /* sub */
	bool eol; /* sub */
	int msg_count; /* sub */
	char *format; /* sub */
	int timeout; /* sub */
	int sub_opts; /* sub */
};

struct mqtt_app_pub {

	int pub_mode; /* pub, rr */
	char *file_input; /* pub, rr */
	char *message; /* pub, rr */
	long msglen; /* pub, rr */
	char *topic; /* pub, rr */
	int repeat_count; /* pub */
	struct timeval repeat_delay; /* pub */
	bool have_topic_alias; /* pub */
};

struct mqtt_app_rr {
	int pub_mode; /* pub, rr */
	char *file_input; /* pub, rr */
	char *message; /* pub, rr */
	long msglen; /* pub, rr */
	char *topic; /* pub, rr */
	char *response_topic; /* rr */
};

struct mqtt_app_config {
	char *id;
	char *id_prefix;
	int protocol_version;
	int keepalive;
	char *host;
	int port;
	int qos;
	bool retain;
	char *bind_address;

#ifdef WITH_SRV
	bool use_srv;
#endif
	bool debug;
	bool quiet;
	unsigned int max_inflight;
	char *username;
	char *password;
	char *will_topic;
	char *will_payload;
	long will_payloadlen;
	int will_qos;
	bool will_retain;
#ifdef WITH_TLS
	char *cafile;
	char *capath;
	char *certfile;
	char *keyfile;
	char *ciphers;
	bool insecure;
	char *tls_alpn;
	char *tls_version;
	char *tls_engine;
	char *tls_engine_kpass_sha1;
	char *keyform;
#  ifdef FINAL_WITH_TLS_PSK
	char *psk;
	char *psk_identity;
#  endif
#endif
	bool clean_session;

#ifdef WITH_SOCKS
	char *socks5_host;
	int socks5_port;
	char *socks5_username;
	char *socks5_password;
#endif
	mosquitto_property *connect_props;
	mosquitto_property *publish_props;
	mosquitto_property *subscribe_props;
	mosquitto_property *unsubscribe_props;
	mosquitto_property *disconnect_props;
	mosquitto_property *will_props;


	struct mqtt_app_sub	sub;
	struct mqtt_app_pub	pub;
	struct mqtt_app_rr	rr;

};



#endif /* __MQTT_APP_CONF_H__ */
