/*
 * mqtt_app_conf.h
 *
 *  Created on: 2019年12月15日
 *      Author: zhurish
 */

#ifndef __MQTT_APP_CONF_H__
#define __MQTT_APP_CONF_H__


#include "mqtt-config.h"
#include <mqtt_protocol.h>
#include <mosquitto.h>

#define MQTT_PORT_DETAULT 			1883
#define MQTT_KEEPALIVE_DETAULT 		60
#define MQTT_TOPICS_MAX 		32


/*
#define MOSQ_LOG_NONE			0
#define MOSQ_LOG_INFO			(1<<0)
#define MOSQ_LOG_NOTICE			(1<<1)
#define MOSQ_LOG_WARNING		(1<<2)
#define MOSQ_LOG_ERR			(1<<3)
#define MOSQ_LOG_DEBUG			(1<<4)
#define MOSQ_LOG_SUBSCRIBE		(1<<5)
#define MOSQ_LOG_UNSUBSCRIBE	(1<<6)
#define MOSQ_LOG_WEBSOCKETS		(1<<7)
#define MOSQ_LOG_INTERNAL		0x80000000U
#define MOSQ_LOG_ALL			0xFFFFFFFFU
*/

#define MQTT_IS_DEBUG(n)		( (mqtt_config) && (MOSQ_LOG_ ## n & mqtt_config->loglevel) )
#define MQTT_DEBUG_ON(n)		{ if(mqtt_config) { mqtt_config->loglevel |= (MOSQ_LOG_ ## n );}}
#define MQTT_DEBUG_OFF(n)		{ if(mqtt_config) { mqtt_config->loglevel &= ~(MOSQ_LOG_ ## n );}}

/*
 名字	值	流向	描述
CONNECT	1	C->S	客户端请求与服务端建立连接
CONNACK	2	S->C	服务端确认连接建立
PUBLISH	3	CóS	发布消息
PUBACK	4	CóS	收到发布消息确认
PUBREC	5	CóS	发布消息收到
PUBREL	6	CóS	发布消息释放
PUBCOMP	7	CóS	发布消息完成
SUBSCRIBE	8	C->S	订阅请求
SUBACK	9	S->C	订阅确认
UNSUBSCRIBE	10	C->S	取消订阅
UNSUBACK	11	S->C	取消订阅确认
PING	12	C->S	客户端发送PING(连接保活)命令
PINGRSP	13	S->C	PING命令回复
DISCONNECT	14	C->S	断开连接
 */
typedef enum mqtt_mode_type
{
	MQTT_MODE_PUB = 1,
	MQTT_MODE_SUB = 2,
	//MQTT_MODE_RR = 3,
	//MQTT_MODE_RESPONSE_TOPIC = 4
}mqtt_mode_t;

enum prop_type
{
	PROP_TYPE_BYTE,
	PROP_TYPE_INT16,
	PROP_TYPE_INT32,
	PROP_TYPE_BINARY,
	PROP_TYPE_STRING,
	PROP_TYPE_STRING_PAIR
};


struct mqtt_app_sub {
	zpl_uint32 sub_opts; /* sub */

	zpl_char *topics[MQTT_TOPICS_MAX]; /* sub */ //指定消息所发布到哪个主题
	zpl_uint32 topic_count; /* sub */

	zpl_char *unsub_topics[MQTT_TOPICS_MAX]; /* sub */
	zpl_uint32 unsub_topic_count; /* sub */

	zpl_char *filter_outs[MQTT_TOPICS_MAX]; /* sub */
	zpl_uint32 filter_out_count; /* sub */

	zpl_bool no_retain; /* sub */
	zpl_bool remove_retained; /* sub */
	//char *format; /* sub */

	zpl_char 		*will_topic;//指定客户端意外断开时，Will消息发送到的主题
	zpl_char 		*will_payload;//如果指定该选项，则万一客户端意外和代理服务器断开，则该消息将被保留在服务端并发送出去，
									//该选项必须同 时用--will-topic指定主题
	zpl_long 		will_payloadlen;
	zpl_uint32 		will_qos;//指定Will的服务质量，默认是0.必须和选项 --will-topic同时使用.
	zpl_bool 		will_retain;//如果指定该选项，则万一客户端意外断开，已被发送的消息将被当做retained消息。
								//必须和选项 --will-topic同时使 用.
};
#if 0
struct mqtt_app_pub {

	//int pub_mode; /* pub*/
	zpl_char *file_input; /* pub*/
	zpl_char *message; /* pub*/ //从命令行发送一条消息，-m后面跟发送的消息内容
	zpl_long msglen; /* pub*/
	zpl_char *topic; /* pub*/ //指定消息所发布到哪个主题。
	zpl_uint32 repeat_count; /* pub */
	struct timeval repeat_delay; /* pub */
	zpl_bool have_topic_alias; /* pub */

	zpl_char 		*will_topic;//指定客户端意外断开时，Will消息发送到的主题
	zpl_char 		*will_payload;//如果指定该选项，则万一客户端意外和代理服务器断开，则该消息将被保留在服务端并发送出去，
									//该选项必须同 时用--will-topic指定主题
	zpl_long 		will_payloadlen;
	zpl_uint32 		will_qos;//指定Will的服务质量，默认是0.必须和选项 --will-topic同时使用.
	zpl_bool 		will_retain;//如果指定该选项，则万一客户端意外断开，已被发送的消息将被当做retained消息。
								//必须和选项 --will-topic同时使 用.
};
#endif

struct mqtt_app_config {

	struct mosquitto *mosq;

	zpl_taskid_t		taskid;
	zpl_bool		reload;
	zpl_bool		enable;
	zpl_bool		taskquit;
	zpl_bool		connectd;
	void		*mutex;
	mqtt_mode_t mqtt_mode;

	zpl_uint32 		mqtt_version;
	zpl_uint32 		keepalive;//给代理发送PING命令（目的在于告知代理该客户端连接保持且在正常工作）的间隔时间，默认是60s

	zpl_char 		*bind_address;//服务绑定的IP地址
	zpl_char 		*host;
	zpl_int16 		port;

	mqtt_qos_level qos;
	/*
	仅针对PUBLISH消息。不同值，不同含义：
	1：表示发送的消息需要一直持久保存（不受服务器重启影响），不但要发送给当前的订阅者，并且以后新来的订阅了此Topic name的订阅者会马上得到推送。
	备注：新来乍到的订阅者，只会取出最新的一个RETAIN flag = 1的消息推送。
	0：仅仅为当前订阅者推送此消息。
	假如服务器收到一个空消息体(zero-length payload)、RETAIN = 1、已存在Topic name的PUBLISH消息，服务器可以删除掉对应的已被持久化的PUBLISH消息。
	*/
	zpl_bool 		retain;//如果指定该选项，该条消息将被保留做为最后一条收到的消息。下一个订阅消息者将能至少收到该条消息。

	zpl_char 		*username;
	zpl_char 		*password;

	zpl_uint32 max_inflight;

	zpl_char 		*id;
	zpl_char 		*id_prefix;

	zpl_bool 		debug;
	zpl_uint32			loglevel;
	zpl_bool 		quiet;// 如果指定该选项，则不会有任何错误被打印，当然，这排除了无效的用户输入所引起的错误消息

	//仅针对SUBSCRIBED消息。不同值，不同含义：
	zpl_bool 		clean_session;//禁止'clean session'选项，即如果客户端断开连接，这个订阅仍然保留来接收随后到的QoS为1和2的
									//消息，当改客户端重新连接之后，它将接收到已排在队列中的消息。建议使用此选项时，

	zpl_uint32 session_expiry_interval;

#ifdef WITH_SRV
	zpl_bool 		use_srv;
#endif

#ifdef WITH_TLS
	zpl_char 		*cafile;//CA证书文件
	zpl_char 		*capath;//CA证书目录
	zpl_char 		*certfile;//PEM证书文件
	zpl_char 		*keyfile;//PEM密钥文件
	zpl_char 		*ciphers;//SSL/TSL加密算法，可以使用“openssl ciphers”命令获取
	zpl_bool 		insecure;
	zpl_char 		*tls_alpn;
	zpl_char 		*tls_version;
	zpl_char 		*tls_engine;
	zpl_char 		*tls_engine_kpass_sha1;
	zpl_char 		*keyform;
#  ifdef FINAL_WITH_TLS_PSK
	zpl_char 		*psk;
	zpl_char 		*psk_identity;
#  endif
#endif

#ifdef WITH_SOCKS
	zpl_char 		*socks5_host;
	zpl_uint16 		ocks5_port;
	zpl_char 		*socks5_username;
	zpl_char 		*socks5_password;
#endif

	mosquitto_property *connect_props;
	mosquitto_property *publish_props;
	mosquitto_property *subscribe_props;
	mosquitto_property *unsubscribe_props;
	mosquitto_property *disconnect_props;
	mosquitto_property *will_props;

	struct mqtt_app_sub	sub;
	//struct mqtt_app_pub	pub;
};

extern struct mqtt_app_config  *mqtt_config;


void mqtt_config_default_init(struct mqtt_app_config *cfg, mqtt_mode_t pub_or_sub);
void mqtt_config_default_cleanup(struct mqtt_app_config *cfg);





int mqtt_config_cfg_check(struct mqtt_app_config *cfg);

#endif /* __MQTT_APP_CONF_H__ */
