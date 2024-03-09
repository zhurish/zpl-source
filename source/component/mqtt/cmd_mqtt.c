/*
 * cmd_mqtt.c
 *
 *  Created on: 2019年6月20日
 *      Author: DELL
 */

#include "zplos_include.h"
#include "zmemory.h"
#include "vty.h"
#include "buffer.h"
#include "command.h"
#include "linklist.h"
#include "log.h"
#include "template.h"

#ifdef ZPL_MQTT_MODULE
#include "zplos_include.h"
#include "zmemory.h"
#include "mqtt-config.h"
#include <mqtt_protocol.h>
#include <mosquitto.h>
#include "mqtt_app_conf.h"
#include "mqtt_app_util.h"
#include "mqtt_app_publish.h"
#include "mqtt_app_subscribed.h"
#include "mqtt_app_api.h"
#include "mqtt_app_show.h"


static int mqtt_write_config(struct vty *vty, void *pVoid);


DEFUN (cli_mqtt_service,
		cli_mqtt_service_cmd,
		"service mqtt",
		"Service Configure\n"
		"Mqtt Protocol\n")
{
	template_t * temp = lib_template_lookup_name (zpl_true, "service mqtt");
	if(temp && temp->pVoid)
	{
		if(!mqtt_isenable_api(temp->pVoid))
			mqtt_enable_api(temp->pVoid, zpl_true);
		vty->node = ALL_SERVICE_NODE;
		vty->index = temp->pVoid;
		memset(vty->prompt, 0, sizeof(vty->prompt));
		sprintf(vty->prompt, "%s", temp->prompt);
		return CMD_SUCCESS;
	}
	else
	{
		temp = lib_template_new (zpl_true);
		if(temp)
		{
			temp->module = 0;
			strcpy(temp->name, "service mqtt");
			strcpy(temp->prompt, "service-mqtt"); /* (config-app-esp)# */
			temp->write_template = mqtt_write_config;
			vty->index = temp->pVoid = mqtt_config;
			lib_template_install(temp, 0);
			if(!mqtt_isenable_api(temp->pVoid))
				mqtt_enable_api(temp->pVoid, zpl_true);
			vty->node = ALL_SERVICE_NODE;
			memset(vty->prompt, 0, sizeof(vty->prompt));
			sprintf(vty->prompt, "%s", temp->prompt);
			return CMD_SUCCESS;
		}
	}
	return CMD_WARNING;
}

DEFUN (no_cli_mqtt_service,
		no_cli_mqtt_service_cmd,
		"no service mqtt",
		NO_STR
		"Service Configure\n"
		"Mqtt Protocol\n")
{
	template_t * temp = lib_template_lookup_name (zpl_true, "service mqtt");
	if(temp)
	{
		if(!mqtt_isenable_api(temp->pVoid))
			mqtt_enable_api(temp->pVoid, zpl_false);
		vty->index = NULL;
		lib_template_free(temp);
		return CMD_SUCCESS;
	}
	return CMD_WARNING;
}

/*
 * Mqtt Local
 */
DEFUN (cli_mqtt_client_version,
	   cli_mqtt_client_version_cmd,
		"mqtt client version (3|4|5)",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Version Configure\n"
		"V3.0.0\n"
		"V3.1.1\n"
		"V5.0.0\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_version_api(vty->index, atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_version,
		no_cli_mqtt_client_version_cmd,
		"no mqtt client version",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"Version Configure\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_version_api(vty->index, MQTT_PROTOCOL_V311);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (cli_mqtt_bind_address,
		cli_mqtt_bind_address_cmd,
		"mqtt client bind "CMD_KEY_IPV4,
		"Mqtt Configure\n"
		"Client Configure\n"
		"Bind Address\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	prefix_zero(&address);
	ret = str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&address);
	if (ret <= 0)
	{
		vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(vty->index)
		ret = mqtt_bind_address_api(vty->index, argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_bind_address,
		no_cli_mqtt_bind_address_cmd,
		"no mqtt client bind",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"Bind Address\n")
{
	int ret = 0;
	if(vty->index)
		ret = mqtt_bind_address_api(vty->index, NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_port,
		cli_mqtt_port_cmd,
		"mqtt client port <256-65535>",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Server port\n"
		"port number\n")
{
	int ret = ERROR;
	zpl_uint16 value = 0;
	value = atoi(argv[0]);
	if(vty->index)
		ret = mqtt_connect_port_api(vty->index, value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_port,
		no_cli_mqtt_port_cmd,
		"no mqtt client port",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"Server port\n")
{
	int ret = ERROR;
	zpl_uint16 value = 0;
	value = atoi(argv[0]);
	if(vty->index)
		ret = mqtt_connect_port_api(vty->index, value);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (cli_mqtt_server,
		cli_mqtt_server_cmd,
		"mqtt client server "CMD_KEY_IPV4,
		"Mqtt Configure\n"
		"Client Configure\n"
		"Server Configure\n"
		CMD_KEY_IPV4_HELP)
{
	int ret = ERROR;
	struct prefix address;
	prefix_zero(&address);
	ret = str2prefix_ipv4 (argv[0], (struct prefix_ipv4 *)&address);
	if (ret <= 0)
	{
		vty_out (vty, "%% Malformed address %s", VTY_NEWLINE);
		return CMD_WARNING;
	}
	if(vty->index)
		ret = mqtt_connect_host_api(vty->index, argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_server,
		no_cli_mqtt_server_cmd,
		"no mqtt client server",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"Server Configure\n")
{
	int ret = 0;
	if(vty->index)
		ret = mqtt_connect_host_api(vty->index, NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_username,
		cli_mqtt_username_cmd,
		"mqtt client username STRING",
		"Mqtt Configure\n"
		"Client Configure\n"
		"username Configure\n"
		"username\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		ret = mqtt_username_api(vty->index, argv[0]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_username,
		no_cli_mqtt_username_cmd,
		"no mqtt client username",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"username Configure\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		ret = mqtt_username_api(vty->index, NULL);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_password,
		cli_mqtt_password_cmd,
		"mqtt client password STRING",
		"Mqtt Configure\n"
		"Client Configure\n"
		"password Configure\n"
		"password\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		ret = mqtt_password_api(vty->index, argv[0]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_password,
		no_cli_mqtt_password_cmd,
		"no mqtt client password",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"password Configure\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		ret = mqtt_password_api(vty->index, NULL);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_client_id,
		cli_mqtt_client_id_cmd,
		"mqtt client id STRING",
		"Mqtt Configure\n"
		"Client Configure\n"
		"ID Configure\n"
		"ID String\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		ret = mqtt_id_api(vty->index, argv[0]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_id,
		no_cli_mqtt_client_id_cmd,
		"no mqtt client id",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"ID Configure\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		ret = mqtt_id_api(vty->index, NULL);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_client_id_prefix,
		cli_mqtt_client_id_prefix_cmd,
		"mqtt client id-prefix STRING",
		"Mqtt Configure\n"
		"Client Configure\n"
		"ID Prefix Configure\n"
		"ID String\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		ret = mqtt_id_prefix_api(vty->index, argv[0]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_id_prefix,
		no_cli_mqtt_client_id_prefix_cmd,
		"no mqtt client id-prefix",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"ID Prefix Configure\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		ret = mqtt_id_prefix_api(vty->index, NULL);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (cli_mqtt_client_keepalive,
		cli_mqtt_client_keepalive_cmd,
		"mqtt client keepalive <1-3600>",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Keepalive Configure\n"
		"Keepalive Value\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_keepalive_api(vty->index, atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_keepalive,
		no_cli_mqtt_client_keepalive_cmd,
		"mqtt client keepalive",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"Keepalive Configure\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_keepalive_api(vty->index, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_client_max_inflight,
		cli_mqtt_client_max_inflight_cmd,
		"mqtt client max-inflight <0-3600>",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Max Inflight Configure\n"
		"Max value\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_max_inflight_api(vty->index, atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_max_inflight,
		no_cli_mqtt_client_max_inflight_cmd,
		"no mqtt client max-inflight",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"Max Inflight Configure\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_max_inflight_api(vty->index, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (cli_mqtt_client_qos,
		cli_mqtt_client_qos_cmd,
		"mqtt client qoslevel (0|1|2)",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Qos Configure\n"
		"Qos value\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_qos_api(vty->index, atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_qos,
		no_cli_mqtt_client_qos_cmd,
		"no mqtt client qoslevel",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"Qos Configure\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_qos_api(vty->index, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_client_retain,
		cli_mqtt_client_retain_cmd,
		"mqtt client retain (enable|disable)",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Retain Configure\n"
		"True value\n"
		"False value\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_retain_api(vty->index, strstr(argv[0],"enable")?zpl_true:zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_client_clean_session,
		cli_mqtt_client_clean_session_cmd,
		"mqtt client clean-session (enable|disable)",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Clean Session Configure\n"
		"True value\n"
		"False value\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_clean_session_api(vty->index, strstr(argv[0],"enable")?zpl_true:zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_client_session_expiry_interval,
		cli_mqtt_client_session_expiry_interval_cmd,
		"mqtt client session-expiry-interval <0-3600>",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Session Expiry Configure\n"
		"Interval value\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_session_expiry_interval_api(vty->index, atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (cli_mqtt_client_topic,
	   cli_mqtt_client_topic_cmd,
		"mqtt client (sub-topics|sub-untopics|sub-filter) STRING",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Sub Topics Configure\n"
		"Sub UnTopics Configure\n"
		"Sub Filter Configure\n"
		"STRING\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		if(strstr(argv[0], "untopic"))
			ret = mqtt_sub_unsubscribe_api(vty->index, argv[1]);
		else if(strstr(argv[0], "topic"))
			ret = mqtt_client_add_topic(vty->index, argv[1]);
		else if(strstr(argv[0], "filter"))
			ret = mqtt_sub_filter_out_api(vty->index, argv[1]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_topic,
		no_cli_mqtt_client_topic_cmd,
		"no mqtt client (sub-topics|sub-untopics|sub-filter) STRING",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"Sub Topics Configure\n"
		"Sub UnTopics Configure\n"
		"Sub Filter Configure\n"
		"STRING\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		if(strstr(argv[0], "untopic"))
			ret = mqtt_sub_unsubscribe_del_api(vty->index, argv[1]);
		else if(strstr(argv[0], "topic"))
			ret = mqtt_client_del_topic(vty->index, argv[1]);
		else if(strstr(argv[0], "filter"))
			ret = mqtt_sub_filter_out_del_api(vty->index, argv[1]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}



DEFUN (cli_mqtt_client_will_payload,
	   cli_mqtt_client_will_payload_cmd,
		"mqtt client will-payload STRING",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Payload Configure\n"
		"STRING\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_will_payload_api(vty->index, argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_will_payload,
		no_cli_mqtt_client_will_payload_cmd,
		"no mqtt client will-payload",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"Payload Configure\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_will_payload_api(vty->index, NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (cli_mqtt_client_will_topic,
	   cli_mqtt_client_will_topic_cmd,
		"mqtt client will-topic STRING",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Will Topic Configure\n"
		"STRING\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_will_topic_api(vty->index, argv[0]);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_will_topic,
		no_cli_mqtt_client_will_topic_cmd,
		"no mqtt client will-topic",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"Will Topic Configure\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_will_topic_api(vty->index, NULL);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_client_will_retain,
		cli_mqtt_client_will_retain_cmd,
		"mqtt client will-retain (enable|disable)",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Will Retain Configure\n"
		"True value\n"
		"False value\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_will_retain_api(vty->index, strstr(argv[0],"enable")?zpl_true:zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_client_will_qos,
		cli_mqtt_client_will_qos_cmd,
		"mqtt client will-qoslevel (0|1|2)",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Will Qos Configure\n"
		"0 value\n"
		"1 value\n"
		"2 value\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_will_qos_api(vty->index, atoi(argv[0]));
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_client_remove_retain,
		cli_mqtt_client_remove_retain_cmd,
		"mqtt client remove-retain (enable|disable)",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Remove Retain Configure\n"
		"True value\n"
		"False value\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_sub_remove_retained_api(vty->index, strstr(argv[0],"enable")?zpl_true:zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (cli_mqtt_client_option,
		cli_mqtt_client_option_cmd,
		"mqtt client option (no-local|retain-as-published|retain-always|retain-new|retain-never)",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Option Configure\n"
		"No Local Retain\n"
		"Retain As Published\n"
		"Always Retain\n"
		"Retain New\n"
		"Never Retain\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		if(strstr(argv[0],"local"))
			ret = mqtt_option_api(vty->index, MQTT_SUB_OPT_NO_LOCAL, zpl_true);
		else if(strstr(argv[0],"published"))
			ret = mqtt_option_api(vty->index, MQTT_SUB_OPT_RETAIN_AS_PUBLISHED, zpl_true);
		else if(strstr(argv[0],"always"))
			ret = mqtt_option_api(vty->index, MQTT_SUB_OPT_SEND_RETAIN_ALWAYS, zpl_true);
		else if(strstr(argv[0],"new"))
			ret = mqtt_option_api(vty->index, MQTT_SUB_OPT_SEND_RETAIN_NEW, zpl_true);
		else if(strstr(argv[0],"never"))
			ret = mqtt_option_api(vty->index, MQTT_SUB_OPT_SEND_RETAIN_NEVER, zpl_true);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_option,
		no_cli_mqtt_client_option_cmd,
		"no mqtt client option (no-local|retain-as-published|retain-always|retain-new|retain-never)",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"Option Configure\n"
		"No Local Retain\n"
		"Retain As Published\n"
		"Always Retain\n"
		"Retain New\n"
		"Never Retain\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		if(strstr(argv[0],"local"))
			ret = mqtt_option_api(vty->index, MQTT_SUB_OPT_NO_LOCAL, zpl_false);
		else if(strstr(argv[0],"published"))
			ret = mqtt_option_api(vty->index, MQTT_SUB_OPT_RETAIN_AS_PUBLISHED, zpl_false);
		else if(strstr(argv[0],"always"))
			ret = mqtt_option_api(vty->index, MQTT_SUB_OPT_SEND_RETAIN_ALWAYS, zpl_false);
		else if(strstr(argv[0],"new"))
			ret = mqtt_option_api(vty->index, MQTT_SUB_OPT_SEND_RETAIN_NEW, zpl_false);
		else if(strstr(argv[0],"never"))
			ret = mqtt_option_api(vty->index, MQTT_SUB_OPT_SEND_RETAIN_NEVER, zpl_false);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

/************************************ TLS ************************************/
#ifdef WITH_TLS
DEFUN (cli_mqtt_client_tls_insecure,
		cli_mqtt_client_tls_insecure_cmd,
		"mqtt client tls-insecure (enable|disable)",
		"Mqtt Configure\n"
		"Client Configure\n"
		"TLS Insecure Configure\n"
		"Enable value\n"
		"Disable value\n")
{
	int ret = ERROR;
	if(vty->index)
		ret = mqtt_tls_insecure_api(vty->index, strstr(argv[0],"enable")?zpl_true:zpl_false);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_client_tls_keyfile,
	   cli_mqtt_client_tls_keyfile_cmd,
		"mqtt client (tls-cafile|tls-capath|tls-certfile|tls-keyfile) FILENAME",
		"Mqtt Configure\n"
		"Client Configure\n"
		"TLS Cafile Configure\n"
		"TLS Capath Configure\n"
		"TLS Certfile Configure\n"
		"TLS Keyfile Configure\n"
		"Filename Or PathName\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		if(strstr(argv[0],"cafile"))
			ret = mqtt_tls_cafile_api(vty->index, argv[1]);
		else if(strstr(argv[0],"capath"))
			ret = mqtt_tls_capath_api(vty->index, argv[1]);
		else if(strstr(argv[0],"certfile"))
			ret = mqtt_tls_certfile_api(vty->index, argv[1]);
		else if(strstr(argv[0],"keyfile"))
			ret = mqtt_tls_keyfile_api(vty->index, argv[1]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_tls_keyfile,
		no_cli_mqtt_client_tls_keyfile_cmd,
		"no mqtt client (tls-cafile|tls-capath|tls-certfile|tls-keyfile)",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"TLS Cafile Configure\n"
		"TLS Capath Configure\n"
		"TLS Certfile Configure\n"
		"TLS Keyfile Configure\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		if(strstr(argv[0],"cafile"))
			ret = mqtt_tls_cafile_api(vty->index, NULL);
		else if(strstr(argv[0],"capath"))
			ret = mqtt_tls_capath_api(vty->index, NULL);
		else if(strstr(argv[0],"certfile"))
			ret = mqtt_tls_certfile_api(vty->index, NULL);
		else if(strstr(argv[0],"keyfile"))
			ret = mqtt_tls_keyfile_api(vty->index, NULL);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (cli_mqtt_client_tls_keywork,
	   cli_mqtt_client_tls_keywork_cmd,
		"mqtt client (tls-ciphers|tls-alpn|tls-version|tls-engine|tls-keyform|tls-engine-kpass-sha1) KEYSTRING",
		"Mqtt Configure\n"
		"Client Configure\n"
		"TLS Ciphers Configure\n"
		"TLS Alpn Configure\n"
		"TLS Version Configure\n"
		"TLS Engine Configure\n"
		"TLS Keyform Configure\n"
		"TLS Engine Kpass Sha1 Configure\n"
		"Key String\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		if(strstr(argv[0],"ciphers"))
			ret = mqtt_tls_ciphers_api(vty->index, argv[1]);
		else if(strstr(argv[0],"keyform"))
			ret = mqtt_tls_keyform_api(vty->index, argv[1]);
		else if(strstr(argv[0],"alpn"))
			ret = mqtt_tls_alpn_api(vty->index, argv[1]);
		else if(strstr(argv[0],"version"))
			ret = mqtt_tls_version_api(vty->index, argv[1]);
		else if(strstr(argv[0],"engine-kpass-sha1"))
			ret = mqtt_tls_engine_kpass_sha1_api(vty->index, argv[1]);
		else if(strstr(argv[0],"engine"))
			ret = mqtt_tls_engine_api(vty->index, argv[1]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_tls_keywork,
		no_cli_mqtt_client_tls_keywork_cmd,
		"no mqtt client (tls-ciphers|tls-alpn|tls-version|tls-engine|tls-keyform|tls-engine-kpass-sha1)",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"TLS Ciphers Configure\n"
		"TLS Alpn Configure\n"
		"TLS Version Configure\n"
		"TLS Engine Configure\n"
		"TLS Keyform Configure\n"
		"TLS Engine Kpass Sha1 Configure\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		if(strstr(argv[0],"ciphers"))
			ret = mqtt_tls_ciphers_api(vty->index, NULL);
		else if(strstr(argv[0],"keyform"))
			ret = mqtt_tls_keyform_api(vty->index, NULL);
		else if(strstr(argv[0],"alpn"))
			ret = mqtt_tls_alpn_api(vty->index, NULL);
		else if(strstr(argv[0],"version"))
			ret = mqtt_tls_version_api(vty->index, NULL);
		else if(strstr(argv[0],"engine-kpass-sha1"))
			ret = mqtt_tls_engine_kpass_sha1_api(vty->index, NULL);
		else if(strstr(argv[0],"engine"))
			ret = mqtt_tls_engine_api(vty->index, NULL);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

#endif

#ifdef FINAL_WITH_TLS_PSK
DEFUN (cli_mqtt_client_tls_psk,
	   cli_mqtt_client_tls_psk_cmd,
		"mqtt client (tls-psk|tls-psk-identity) KEYSTRING",
		"Mqtt Configure\n"
		"Client Configure\n"
		"TLS PSK Configure\n"
		"TLS PSK Identity Configure\n"
		"Key String\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		if(strstr(argv[0],"psk-identity"))
			ret = mqtt_tls_psk_identity_api(vty->index, argv[1]);
		else if(strstr(argv[0],"psk"))
			ret = mqtt_tls_psk_api(vty->index, argv[1]);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_cli_mqtt_client_tls_psk,
		no_cli_mqtt_client_tls_psk_cmd,
		"no mqtt client (tls-psk|tls-psk-identity)",
		NO_STR
		"Mqtt Configure\n"
		"Client Configure\n"
		"TLS Ciphers Configure\n"
		"TLS Alpn Configure\n"
		"TLS Version Configure\n"
		"TLS Engine Configure\n"
		"TLS Keyform Configure\n"
		"TLS Engine Kpass Sha1 Configure\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		if(strstr(argv[0],"psk-identity"))
			ret = mqtt_tls_psk_identity_api(vty->index, NULL);
		else if(strstr(argv[0],"psk"))
			ret = mqtt_tls_psk_api(vty->index, NULL);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
#endif


/************************************ commit ************************************/
DEFUN (cli_mqtt_client_commit,
	   cli_mqtt_client_commit_cmd,
		"mqtt client commit",
		"Mqtt Configure\n"
		"Client Configure\n"
		"Commit All Configure\n")
{
	int ret = ERROR;
	if(vty->index)
	{
		ret = mqtt_module_commit_api(vty->index);
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}
/************************************ debug ************************************/
/*
 * Mqtt show
 */
DEFUN (show_cli_mqtt_server,
		show_cli_mqtt_server_cmd,
		"show mqtt service",
		SHOW_STR
		"Mqtt Configure\n"
		"Service\n")
{
	mqtt_app_config_show(mqtt_config, vty, zpl_true, zpl_false);
	return CMD_SUCCESS;
}

/************************************ debug ************************************/
DEFUN (debug_cli_mqtt,
		debug_cli_mqtt_cmd,
		"debug mqtt (subscribe|unsubscribe|websockets|internal|errors|warnings|notifications|informational|debugging)",
		DEBUG_STR
		"Mqtt Configure\n"
		"Subscribe messages\n"
		"UnSubscribe messages\n"
		"WebSockets messages\n"
		"Internal messages\n"
		"Error conditions\n"
		"Warning conditions\n"
		"Normal but significant conditions\n"
		"Informational messages\n"
		"Debugging messages\n")
{
	int ret = ERROR;
	if(mqtt_config)
	{
		if(strstr(argv[0],"unsubscribe"))
			MQTT_DEBUG_ON(SUBSCRIBE)
		else if(strstr(argv[0],"subscribe"))
			MQTT_DEBUG_ON(UNSUBSCRIBE)
		if(strstr(argv[0],"websockets"))
			MQTT_DEBUG_ON(WEBSOCKETS)
		if(strstr(argv[0],"internal"))
			MQTT_DEBUG_ON(INTERNAL)

		if(strstr(argv[0],"debugging"))
			MQTT_DEBUG_ON(DEBUG)
		else if(strstr(argv[0],"informational"))
			MQTT_DEBUG_ON(INFO)
		else if(strstr(argv[0],"notifications"))
			MQTT_DEBUG_ON(NOTICE)
		else if(strstr(argv[0],"warnings"))
			MQTT_DEBUG_ON(WARNING)
		else if(strstr(argv[0],"errors"))
			MQTT_DEBUG_ON(ERR)
	}
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}

DEFUN (no_debug_cli_mqtt,
		no_debug_cli_mqtt_cmd,
		"no debug mqtt",
		NO_STR
		DEBUG_STR
		IP_STR
		"Mqtt Configure\n"
		LOG_LEVEL_DESC)
{
	int ret = ERROR;
	if(mqtt_config)
		ret = mqtt_debug_api(mqtt_config, 0);
	return  (ret == OK)? CMD_SUCCESS:CMD_WARNING;
}


DEFUN (show_debugging_mqtt,
       show_debugging_mqtt_cmd,
       "show debugging mqtt",
       SHOW_STR
       "Debugging information\n"
	   "Mqtt Configure\n")
{
	if(mqtt_config)
		mqtt_app_debug_show(mqtt_config, vty);
	return CMD_SUCCESS;
}

static int mqtt_write_config(struct vty *vty, void *pVoid)
{
	if(pVoid)
	{
		vty_out(vty, "service mqtt%s",VTY_NEWLINE);
		mqtt_app_config_show(pVoid, vty, zpl_true, zpl_true);
		return 1;
	}
	return 0;
}


static void cmd_base_mqtt_init(int node)
{
	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_version_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_version_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_bind_address_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_bind_address_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_port_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_port_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_server_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_server_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_username_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_username_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_password_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_password_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_id_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_id_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_id_prefix_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_id_prefix_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_keepalive_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_keepalive_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_max_inflight_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_max_inflight_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_qos_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_qos_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_topic_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_topic_cmd);


	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_retain_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_clean_session_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_session_expiry_interval_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_will_retain_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_will_qos_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_remove_retain_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_will_payload_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_will_payload_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_will_topic_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_will_topic_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_option_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_option_cmd);

#ifdef WITH_TLS
	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_tls_insecure_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_tls_keyfile_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_tls_keyfile_cmd);

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_tls_keywork_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_tls_keywork_cmd);
#endif

#ifdef FINAL_WITH_TLS_PSK
	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_tls_psk_cmd);
	install_element(node, CMD_CONFIG_LEVEL, &no_cli_mqtt_client_tls_psk_cmd);
#endif

	install_element(node, CMD_CONFIG_LEVEL, &cli_mqtt_client_commit_cmd);
}

static void cmd_show_mqtt_init(int node)
{
	install_element(node, CMD_VIEW_LEVEL, &show_cli_mqtt_server_cmd);
	install_element(node, CMD_VIEW_LEVEL, &show_debugging_mqtt_cmd);
}

int cmd_mqtt_init(void)
{
	template_t * temp = lib_template_new (zpl_true);
	if(temp)
	{
		temp->module = 0;
		strcpy(temp->name, "service mqtt");
		strcpy(temp->prompt, "service-mqtt");
		temp->pVoid = mqtt_config;
		temp->write_template = mqtt_write_config;

		lib_template_install(temp, 0);

		install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &cli_mqtt_service_cmd);
		install_element(CONFIG_NODE, CMD_CONFIG_LEVEL, &no_cli_mqtt_service_cmd);

		cmd_base_mqtt_init(ALL_SERVICE_NODE);

		cmd_show_mqtt_init(ENABLE_NODE);
		cmd_show_mqtt_init(CONFIG_NODE);
		cmd_show_mqtt_init(ALL_SERVICE_NODE);

		install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &debug_cli_mqtt_cmd);
		install_element(ENABLE_NODE, CMD_ENABLE_LEVEL, &no_debug_cli_mqtt_cmd);
		install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &debug_cli_mqtt_cmd);
		install_element(CONFIG_NODE, CMD_ENABLE_LEVEL, &no_debug_cli_mqtt_cmd);
	}
	return 0;
}
#endif/* ZPL_MQTT_MODULE */
