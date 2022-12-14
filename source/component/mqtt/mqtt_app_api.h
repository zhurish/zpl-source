/*
 * mqtt_app_api.h
 *
 *  Created on: 2019年7月19日
 *      Author: DELL
 */

#ifndef __MQTT_APP_API_H__
#define __MQTT_APP_API_H__



#include "mqtt_app_conf.h"

zpl_bool mqtt_isenable_api(struct mqtt_app_config *cfg);
int mqtt_enable_api(struct mqtt_app_config *cfg, zpl_bool enable);

int mqtt_reload_api(struct mqtt_app_config *cfg);

int mqtt_bind_address_api(struct mqtt_app_config *cfg,
		const zpl_char *bind_address);
/*
 * mqtt 服务器地址
 */
int mqtt_connect_host_api(struct mqtt_app_config *cfg, const zpl_char * host);
/*
 * mqtt 服务器连接端口
 */
int mqtt_connect_port_api(struct mqtt_app_config *cfg, const zpl_uint16 port);
/*
 * mqtt 代理服务器用户名
 */
int mqtt_username_api(struct mqtt_app_config *cfg, const zpl_char * username);
/*
 * mqtt 代理服务器用户密码
 */
int mqtt_password_api(struct mqtt_app_config *cfg, const zpl_char * password);

#ifdef WITH_TLS
int mqtt_tls_cafile_api(struct mqtt_app_config *cfg, const zpl_char *cafile);
int mqtt_tls_capath_api(struct mqtt_app_config *cfg, const zpl_char *capath);
int mqtt_tls_certfile_api(struct mqtt_app_config *cfg, const zpl_char *certfile);
int mqtt_tls_ciphers_api(struct mqtt_app_config *cfg, const zpl_char *ciphers);
int mqtt_tls_insecure_api(struct mqtt_app_config *cfg, const zpl_bool insecure);
int mqtt_tls_keyform_api(struct mqtt_app_config *cfg, const zpl_char *keyform);
int mqtt_tls_keyfile_api(struct mqtt_app_config *cfg, const zpl_char *keyfile);
#endif

int mqtt_id_api(struct mqtt_app_config *cfg, const zpl_char * format);
int mqtt_id_prefix_api(struct mqtt_app_config *cfg,
		const char * format);

int mqtt_keepalive_api(struct mqtt_app_config *cfg,
		const zpl_uint32 keepalive);
int mqtt_max_inflight_api(struct mqtt_app_config *cfg,
		const zpl_uint32 max_inflight);

#ifdef FINAL_WITH_TLS_PSK
int mqtt_tls_psk_api(struct mqtt_app_config *cfg, const zpl_char * format);
int mqtt_tls_psk_identity_api(struct mqtt_app_config *cfg, const zpl_char * format);
#endif

int mqtt_qos_api(struct mqtt_app_config *cfg, const mqtt_qos_level qos);
int mqtt_retain_api(struct mqtt_app_config *cfg, const zpl_bool retain);
int mqtt_clean_session_api(struct mqtt_app_config *cfg, const zpl_bool clean_session);
int mqtt_session_expiry_interval_api(struct mqtt_app_config *cfg, zpl_uint32 interval);

#ifdef WITH_SRV
int mqtt_use_srv_api(struct mqtt_app_config *cfg, const zpl_bool use_srv);
#endif

#ifdef WITH_TLS
int mqtt_tls_alpn_api(struct mqtt_app_config *cfg, const zpl_char * format);
int mqtt_tls_engine_api(struct mqtt_app_config *cfg, const zpl_char * format);
int mqtt_tls_engine_kpass_sha1_api(struct mqtt_app_config *cfg, const zpl_char * format);
int mqtt_tls_version_api(struct mqtt_app_config *cfg, const zpl_char * format);
#endif


int mqtt_debug_api(struct mqtt_app_config *cfg, zpl_uint32 loglevel);

int mqtt_version_api(struct mqtt_app_config *cfg,
		const zpl_uint32 protocol_version);


int mqtt_sub_filter_out_api(struct mqtt_app_config *cfg,
		const zpl_char * filter);
int mqtt_sub_filter_out_del_api(struct mqtt_app_config *cfg, zpl_char *filter);

int mqtt_sub_unsubscribe_api(struct mqtt_app_config *cfg,
		const zpl_char * filter);
int mqtt_sub_unsubscribe_del_api(struct mqtt_app_config *cfg, zpl_char *topic);

/*
int mqtt_pub_repeat_count_delay_api(struct mqtt_app_config *cfg,
		const zpl_uint32 repeat_count, const zpl_float repeat_delay);

*/
int mqtt_will_payload_api(struct mqtt_app_config *cfg, const zpl_char * format);

int mqtt_will_topic_api(struct mqtt_app_config *cfg, const zpl_char * will_topic);

int mqtt_will_qos_api(struct mqtt_app_config *cfg, const mqtt_qos_level will_qos);

int mqtt_will_retain_api(struct mqtt_app_config *cfg, const zpl_bool will_retain);


/*
int mqtt_sub_no_retain_api(struct mqtt_app_config *cfg,
		const bool no_retain);
int mqtt_sub_retain_as_published_api(struct mqtt_app_config *cfg);
*/

int mqtt_sub_remove_retained_api(struct mqtt_app_config *cfg,
		const zpl_bool remove_retained);

int mqtt_option_api(struct mqtt_app_config *cfg, const zpl_uint32 option, zpl_bool set);


int mqtt_module_commit_api(struct mqtt_app_config *cfg);
int mqtt_module_init(void);
int mqtt_module_exit(void);
int mqtt_module_task_init(void);
int mqtt_module_task_exit(void);
void cmd_mqtt_init(void);

#endif /* __MQTT_APP_API_H__ */
