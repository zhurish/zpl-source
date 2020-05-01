/*
 * mqtt_app_conf.h
 *
 *  Created on: 2019年12月15日
 *      Author: zhurish
 */

#ifndef __MQTT_APP_CONF_H__
#define __MQTT_APP_CONF_H__



#define MQTT_PORT_DETAULT 			1883
#define MQTT_KEEPALIVE_DETAULT 	60


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
	MQTT_MODE_RR = 3,
	MQTT_MODE_RESPONSE_TOPIC = 4
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
	int sub_opts; /* sub */

	char **topics; /* sub */ //指定消息所发布到哪个主题
	int topic_count; /* sub */

	char **unsub_topics; /* sub */
	int unsub_topic_count; /* sub */

	char **filter_outs; /* sub */
	int filter_out_count; /* sub */

	bool no_retain; /* sub */
	bool remove_retained; /* sub */
	//char *format; /* sub */

	char 		*will_topic;//指定客户端意外断开时，Will消息发送到的主题
	char 		*will_payload;//如果指定该选项，则万一客户端意外和代理服务器断开，则该消息将被保留在服务端并发送出去，
									//该选项必须同 时用--will-topic指定主题
	long 		will_payloadlen;
	int 		will_qos;//指定Will的服务质量，默认是0.必须和选项 --will-topic同时使用.
	bool 		will_retain;//如果指定该选项，则万一客户端意外断开，已被发送的消息将被当做retained消息。
								//必须和选项 --will-topic同时使 用.
};

struct mqtt_app_pub {

	//int pub_mode; /* pub*/
	char *file_input; /* pub*/
	char *message; /* pub*/ //从命令行发送一条消息，-m后面跟发送的消息内容
	long msglen; /* pub*/
	char *topic; /* pub*/ //指定消息所发布到哪个主题。
	int repeat_count; /* pub */
	struct timeval repeat_delay; /* pub */
	bool have_topic_alias; /* pub */

	char 		*will_topic;//指定客户端意外断开时，Will消息发送到的主题
	char 		*will_payload;//如果指定该选项，则万一客户端意外和代理服务器断开，则该消息将被保留在服务端并发送出去，
									//该选项必须同 时用--will-topic指定主题
	long 		will_payloadlen;
	int 		will_qos;//指定Will的服务质量，默认是0.必须和选项 --will-topic同时使用.
	bool 		will_retain;//如果指定该选项，则万一客户端意外断开，已被发送的消息将被当做retained消息。
								//必须和选项 --will-topic同时使 用.

};

struct mqtt_app_rr {
	int pub_mode; /*rr */
	char *file_input; /*rr */
	char *message; /*rr */
	long msglen; /*rr */
	char *topic; /*rr */
	char *response_topic; /* rr */
};

struct mqtt_app_config {

	struct mosquitto *mosq;

	int		taskid;
	BOOL		reload;
	BOOL		enable;
	void		*mutex;
	mqtt_mode_t mqtt_mode;

	int 		mqtt_version;
	int 		keepalive;//给代理发送PING命令（目的在于告知代理该客户端连接保持且在正常工作）的间隔时间，默认是60s

	char 		*bind_address;//服务绑定的IP地址
	char 		*host;
	int 		port;

	mqtt_qos_level qos;
	/*
	仅针对PUBLISH消息。不同值，不同含义：
	1：表示发送的消息需要一直持久保存（不受服务器重启影响），不但要发送给当前的订阅者，并且以后新来的订阅了此Topic name的订阅者会马上得到推送。
	备注：新来乍到的订阅者，只会取出最新的一个RETAIN flag = 1的消息推送。
	0：仅仅为当前订阅者推送此消息。
	假如服务器收到一个空消息体(zero-length payload)、RETAIN = 1、已存在Topic name的PUBLISH消息，服务器可以删除掉对应的已被持久化的PUBLISH消息。
	*/
	bool 		retain;//如果指定该选项，该条消息将被保留做为最后一条收到的消息。下一个订阅消息者将能至少收到该条消息。

	char 		*username;
	char 		*password;

	unsigned int max_inflight;

	char 		*id;
	char 		*id_prefix;

	bool 		debug;
	bool 		quiet;// 如果指定该选项，则不会有任何错误被打印，当然，这排除了无效的用户输入所引起的错误消息

	//仅针对SUBSCRIBED消息。不同值，不同含义：
	bool 		clean_session;//禁止'clean session'选项，即如果客户端断开连接，这个订阅仍然保留来接收随后到的QoS为1和2的
									//消息，当改客户端重新连接之后，它将接收到已排在队列中的消息。建议使用此选项时，
									//客户端id选 项设为--id
	long 		session_expiry_interval;

#ifdef WITH_SRV
	bool 		use_srv;
#endif

#ifdef WITH_TLS
	char 		*cafile;//CA证书文件
	char 		*capath;//CA证书目录
	char 		*certfile;//PEM证书文件
	char 		*keyfile;//PEM密钥文件
	char 		*ciphers;//SSL/TSL加密算法，可以使用“openssl ciphers”命令获取
	bool 		insecure;
	char 		*tls_alpn;
	char 		*tls_version;
	char 		*tls_engine;
	char 		*tls_engine_kpass_sha1;
	char 		*keyform;
#  ifdef FINAL_WITH_TLS_PSK
	char 		*psk;
	char 		*psk_identity;
#  endif
#endif

#ifdef WITH_SOCKS
	char 		*socks5_host;
	int 		ocks5_port;
	char 		*socks5_username;
	char 		*socks5_password;
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

extern struct mqtt_app_config  *mqtt_config;


void mqtt_config_default_init(struct mqtt_app_config *cfg, mqtt_mode_t pub_or_sub);
void mqtt_config_default_cleanup(struct mqtt_app_config *cfg);





int mqtt_config_cfg_check(struct mqtt_app_config *cfg, mqtt_mode_t pub_or_sub);

#endif /* __MQTT_APP_CONF_H__ */