/*
 * mqtt_app_show.c
 *
 *  Created on: 2020年5月17日
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
#include "mqtt_app_show.h"

static int __mqtt_app_config_sub_show(struct mqtt_app_config  *cfg, struct vty *vty,
		BOOL detail, BOOL wrshow)
{
	int i = 0;
	if (cfg && vty)
	{
		if (cfg->sub.topic_count)
		{
			for (i = 0; i < cfg->sub.topic_count; i++)
			{
				if (cfg->sub.topics[i])
				{
					if (wrshow)
						vty_out (vty, " mqtt client sub-topics %s%s",
								 cfg->sub.topics[i], VTY_NEWLINE);
					else
						vty_out (vty, " MQTT Sub Topics    : %s%s",
								 cfg->sub.topics[i], VTY_NEWLINE);
				}
			}
		}
		if (cfg->sub.unsub_topic_count)
		{
			for (i = 0; i < cfg->sub.unsub_topic_count; i++)
			{
				if (cfg->sub.unsub_topics[i])
				{
					if (wrshow)
						vty_out (vty, " mqtt client sub-untopics %s%s",
								 cfg->sub.unsub_topics[i], VTY_NEWLINE);
					else
						vty_out (vty, " MQTT Sub Untopics  : %s%s",
								 cfg->sub.unsub_topics[i], VTY_NEWLINE);
				}
			}
		}
		if (cfg->sub.filter_out_count)
		{
			for (i = 0; i < cfg->sub.filter_out_count; i++)
			{
				if (cfg->sub.filter_outs[i])
				{
					if (wrshow)
						vty_out (vty, " mqtt client sub-filter %s%s",
								 cfg->sub.filter_outs[i], VTY_NEWLINE);
					else
						vty_out (vty, " MQTT Sub Filter    : %s%s",
								 cfg->sub.filter_outs[i], VTY_NEWLINE);
				}
			}
		}
	}
	return OK;
}



#ifdef WITH_TLS
static int __mqtt_app_config_tls_show(struct mqtt_app_config  *cfg, struct vty *vty,
		BOOL detail, BOOL wrshow)
{
	if(wrshow)
	{
		vty_out(vty, " mqtt client tls-insecure %s%s", cfg->insecure?"enable":"disable", VTY_NEWLINE);
		if(cfg->cafile)
			vty_out(vty, " mqtt client tls-cafile %s%s", cfg->cafile, VTY_NEWLINE);
		if(cfg->capath)
			vty_out(vty, " mqtt client tls-capath %s%s", cfg->capath, VTY_NEWLINE);
		if(cfg->certfile)
			vty_out(vty, " mqtt client tls-certfile %s%s", cfg->certfile, VTY_NEWLINE);
		if(cfg->keyfile)
			vty_out(vty, " mqtt client tls-keyfile %s%s", cfg->keyfile, VTY_NEWLINE);
		if(cfg->ciphers)
			vty_out(vty, " mqtt client tls-ciphers %s%s", cfg->ciphers, VTY_NEWLINE);
		if(cfg->tls_alpn)
			vty_out(vty, " mqtt client tls-alpn %s%s", cfg->tls_alpn, VTY_NEWLINE);

		if(cfg->tls_version)
			vty_out(vty, " mqtt client tls-version %s%s", cfg->tls_version, VTY_NEWLINE);
		if(cfg->tls_engine)
			vty_out(vty, " mqtt client tls-engine %s%s", cfg->tls_engine, VTY_NEWLINE);
		if(cfg->tls_engine_kpass_sha1)
			vty_out(vty, " mqtt client tls-engine-kpass-sha1 %s%s", cfg->tls_engine_kpass_sha1, VTY_NEWLINE);
		if(cfg->keyform)
			vty_out(vty, " mqtt client tls-keyform %s%s", cfg->keyform, VTY_NEWLINE);
#  ifdef FINAL_WITH_TLS_PSK
		if(cfg->psk)
			vty_out(vty, " mqtt client tls-psk %s%s", cfg->psk, VTY_NEWLINE);
		if(cfg->psk_identity)
			vty_out(vty, " mqtt client tls-psk-identity %s%s", cfg->psk_identity, VTY_NEWLINE);
#  endif

	}
	else
	{
		vty_out(vty, " MQTT TLS Insecure  : %s%s", cfg->insecure?"enable":"disable", VTY_NEWLINE);
		vty_out(vty, " MQTT TLS Cafile    : %s%s", cfg->cafile ? cfg->cafile:" ",VTY_NEWLINE);
		vty_out(vty, " MQTT TLS Capath    : %s%s", cfg->capath ? cfg->capath:" ",VTY_NEWLINE);

		vty_out(vty, " MQTT TLS Certfile  : %s%s", cfg->certfile ? cfg->certfile:" ",VTY_NEWLINE);
		vty_out(vty, " MQTT TLS Keyfile   : %s%s", cfg->keyfile ? cfg->keyfile:" ",VTY_NEWLINE);
		vty_out(vty, " MQTT TLS Ciphers   : %s%s", cfg->ciphers ? cfg->ciphers:" ",VTY_NEWLINE);
		vty_out(vty, " MQTT TLS Tls_alpn  : %s%s", cfg->tls_alpn ? cfg->tls_alpn:" ",VTY_NEWLINE);
		vty_out(vty, " MQTT TLS Tls_version: %s%s", cfg->tls_version ? cfg->tls_version:" ",VTY_NEWLINE);
		vty_out(vty, " MQTT TLS Tls_engine: %s%s", cfg->tls_engine ? cfg->tls_engine:" ",VTY_NEWLINE);

		vty_out(vty, " MQTT TLS Tls_engine_kpass_sha1: %s%s", cfg->tls_engine_kpass_sha1 ? cfg->tls_engine_kpass_sha1:" ",VTY_NEWLINE);

		vty_out(vty, " MQTT TLS Keyform   : %s%s", cfg->keyform ? cfg->keyform:" ",VTY_NEWLINE);

#  ifdef FINAL_WITH_TLS_PSK
		vty_out(vty, " MQTT TLS Psk       : %s%s", cfg->psk ? cfg->psk:" ",VTY_NEWLINE);

		vty_out(vty, " MQTT TLS Psk ID    : %s%s", cfg->psk_identity ? cfg->psk_identity:" ",VTY_NEWLINE);

#  endif
	}
	return OK;
}
#endif

static int __mqtt_app_config_show(struct mqtt_app_config  *cfg, struct vty *vty, BOOL detail, BOOL wrshow)
{
	if(!cfg->enable)
	{
		return OK;
	}
	if(wrshow)
	{
		//vty_out(vty, " mqtt client enable%s",VTY_NEWLINE);
		vty_out(vty, " mqtt client version %d%s", cfg->mqtt_version, VTY_NEWLINE);

		if(cfg->bind_address)
			vty_out(vty, " mqtt client bind %s%s", cfg->bind_address, VTY_NEWLINE);
		if(cfg->host)
			vty_out(vty, " mqtt client server %s%s", cfg->host, VTY_NEWLINE);
		vty_out(vty, " mqtt client port %d%s", cfg->port, VTY_NEWLINE);
		vty_out(vty, " mqtt client qoslevel %d%s",cfg->qos, VTY_NEWLINE);

		if(cfg->username)
			vty_out(vty, " mqtt client username %s%s", cfg->username, VTY_NEWLINE);
		if(cfg->password)
			vty_out(vty, " mqtt client password %s%s", cfg->password, VTY_NEWLINE);

		if(cfg->id)
			vty_out(vty, " mqtt client id %s%s", cfg->id, VTY_NEWLINE);
		else if(cfg->id_prefix)
			vty_out(vty, " mqtt client id-prefix %s%s", cfg->id_prefix, VTY_NEWLINE);

		vty_out(vty, " mqtt client max-inflight %d%s", cfg->max_inflight, VTY_NEWLINE);


		vty_out(vty, " mqtt client keepalive %d%s", cfg->keepalive, VTY_NEWLINE);

		vty_out(vty, " mqtt client retain %s%s", cfg->retain ? "enable":"disable",VTY_NEWLINE);
		vty_out(vty, " mqtt client clean-session %s%s", cfg->clean_session ? "enable":"disable",VTY_NEWLINE);
		vty_out(vty, " mqtt client session-expiry-interval %d%s", cfg->session_expiry_interval,VTY_NEWLINE);

		if(cfg->sub.will_topic)
			vty_out(vty, " mqtt client will-topic %s%s", cfg->sub.will_topic, VTY_NEWLINE);
		if(cfg->sub.will_payload)
			vty_out(vty, " mqtt client will-payload %s%s", cfg->sub.will_payload, VTY_NEWLINE);

		vty_out(vty, " mqtt client will-qoslevel %d%s", cfg->sub.will_qos, VTY_NEWLINE);
		vty_out(vty, " mqtt client will-retain %s%s", cfg->sub.will_retain ? "enable":"disable", VTY_NEWLINE);

		//vty_out(vty, " mqtt client no-retain %s%s", cfg->sub.no_retain ? "enable":"disable", VTY_NEWLINE);
		vty_out(vty, " mqtt client remove-retain %s%s", cfg->sub.remove_retained ? "enable":"disable", VTY_NEWLINE);

		if(cfg->sub.sub_opts & MQTT_SUB_OPT_NO_LOCAL)
			vty_out(vty, " mqtt client option no-local%s", VTY_NEWLINE);
		else if(cfg->sub.sub_opts & MQTT_SUB_OPT_RETAIN_AS_PUBLISHED)
			vty_out(vty, " mqtt client option retain-as-published%s", VTY_NEWLINE);
		else if(cfg->sub.sub_opts & MQTT_SUB_OPT_SEND_RETAIN_ALWAYS)
			vty_out(vty, " mqtt client option retain-always%s",  VTY_NEWLINE);
		else if(cfg->sub.sub_opts & MQTT_SUB_OPT_SEND_RETAIN_NEW)
			vty_out(vty, " mqtt client option retain-new%s",  VTY_NEWLINE);
		else if(cfg->sub.sub_opts & MQTT_SUB_OPT_SEND_RETAIN_NEVER)
		{
			vty_out(vty, " mqtt client option retain-never%s",  VTY_NEWLINE);
		}
	}
	else
	{
		vty_out(vty, "MQTT Client         : enable%s",VTY_NEWLINE);
		vty_out(vty, " MQTT Version       : %d%s",cfg->mqtt_version, VTY_NEWLINE);
		vty_out(vty, " MQTT Bind          : %s%s", cfg->bind_address ? cfg->bind_address:" ",VTY_NEWLINE);
		vty_out(vty, " MQTT Server        : %s%s", cfg->host ? cfg->host:" ",VTY_NEWLINE);

		vty_out(vty, " mqtt Port          : %d%s", cfg->port, VTY_NEWLINE);
		vty_out(vty, " MQTT Username      : %s%s", cfg->username ? cfg->username:" ",VTY_NEWLINE);
		vty_out(vty, " MQTT Password      : %s%s", cfg->password ? cfg->password:" ",VTY_NEWLINE);

		vty_out(vty, " MQTT Qos Level     : %d%s",cfg->qos, VTY_NEWLINE);


		if(cfg->id)
			vty_out(vty, " MQTT ID            : %s%s", cfg->id, VTY_NEWLINE);
		else if(cfg->id_prefix)
			vty_out(vty, " MQTT ID-Prefix     : %s%s",cfg->id_prefix, VTY_NEWLINE);

		vty_out(vty, " MQTT Max-inflight  : %d%s",cfg->max_inflight, VTY_NEWLINE);
		vty_out(vty, " MQTT Keepalive     : %d%s",cfg->keepalive, VTY_NEWLINE);
		vty_out(vty, " MQTT Session Interval : %d%s",cfg->session_expiry_interval, VTY_NEWLINE);
		vty_out(vty, " MQTT Clean Session : %s%s",cfg->clean_session ? "TRUE":"FALSE", VTY_NEWLINE);

		vty_out(vty, " MQTT Max-inflight  : %d%s",cfg->max_inflight, VTY_NEWLINE);
		vty_out(vty, " MQTT Keepalive     : %d%s",cfg->keepalive, VTY_NEWLINE);
		vty_out(vty, " MQTT Session Interval : %d%s",cfg->session_expiry_interval, VTY_NEWLINE);
		vty_out(vty, " MQTT Clean Session : %s%s",cfg->clean_session ? "TRUE":"FALSE", VTY_NEWLINE);

		vty_out(vty, " MQTT Will Topic    : %s%s", cfg->sub.will_topic ? cfg->sub.will_topic:" ", VTY_NEWLINE);

		vty_out(vty, " MQTT Will Payload  : %s%s", cfg->sub.will_payload ? cfg->sub.will_payload:" ", VTY_NEWLINE);

		vty_out(vty, " MQTT Will Qos      : %d%s",cfg->sub.will_qos, VTY_NEWLINE);
		vty_out(vty, " MQTT Will Retain   : %d%s",cfg->sub.will_retain, VTY_NEWLINE);

		//vty_out(vty, " MQTT No Retain     :%s%s", cfg->sub.no_retain ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " MQTT Remove Retain :%s%s", cfg->sub.remove_retained ? "TRUE":"FALSE", VTY_NEWLINE);

		if(cfg->sub.sub_opts & MQTT_SUB_OPT_NO_LOCAL)
			vty_out(vty, " MQTT Option        : no-local%s", VTY_NEWLINE);
		else if(cfg->sub.sub_opts & MQTT_SUB_OPT_RETAIN_AS_PUBLISHED)
			vty_out(vty, " MQTT Option        : retain-as-published%s", VTY_NEWLINE);
		else if(cfg->sub.sub_opts & MQTT_SUB_OPT_SEND_RETAIN_ALWAYS)
			vty_out(vty, " MQTT Option        : retain-always%s",  VTY_NEWLINE);
		else if(cfg->sub.sub_opts & MQTT_SUB_OPT_SEND_RETAIN_NEW)
			vty_out(vty, " MQTT Option        : retain-new%s",  VTY_NEWLINE);
		else if(cfg->sub.sub_opts & MQTT_SUB_OPT_SEND_RETAIN_NEVER)
		{
			vty_out(vty, " MQTT Option        : retain-never%s",  VTY_NEWLINE);
		}
		vty_out(vty, " MQTT Debug         : %s%s",cfg->debug ? "TRUE":"FALSE", VTY_NEWLINE);

		vty_out(vty, " MQTT Connect       : %s%s", cfg->connectd ? "TRUE":"FALSE",VTY_NEWLINE);
	}
#ifdef WITH_TLS
	__mqtt_app_config_tls_show(cfg, vty, detail,  wrshow);
#endif
	__mqtt_app_config_sub_show(cfg, vty, detail,  wrshow);

	return OK;
}


int mqtt_app_config_show(struct mqtt_app_config  *cfg, struct vty *vty, BOOL detail, BOOL wrshow)
{
	if(cfg && vty)
		return __mqtt_app_config_show(cfg, vty,  detail,  wrshow);
	return OK;
}

int mqtt_app_debug_show(struct mqtt_app_config  *cfg, struct vty *vty)
{
	if(cfg && vty)
	{
		if(!cfg->enable)
		{
			return OK;
		}
		if(MQTT_IS_DEBUG(DEBUG))
			vty_out(vty, "debug mqtt debugging%s",  VTY_NEWLINE);
		else if(MQTT_IS_DEBUG(INFO))
			vty_out(vty, "debug mqtt informational%s", VTY_NEWLINE);
		else if(MQTT_IS_DEBUG(NOTICE))
			vty_out(vty, "debug mqtt notifications%s",  VTY_NEWLINE);
		else if(MQTT_IS_DEBUG(WARNING))
			vty_out(vty, "debug mqtt warnings%s",  VTY_NEWLINE);
		else if(MQTT_IS_DEBUG(ERR))
			vty_out(vty, "debug mqtt errors%s",  VTY_NEWLINE);

		if(MQTT_IS_DEBUG(SUBSCRIBE))
			vty_out(vty, "debug mqtt subscribe%s",  VTY_NEWLINE);
		if(MQTT_IS_DEBUG(UNSUBSCRIBE))
			vty_out(vty, "debug mqtt unsubscribe%s",  VTY_NEWLINE);
		if(MQTT_IS_DEBUG(WEBSOCKETS))
			vty_out(vty, "debug mqtt websockets%s",  VTY_NEWLINE);
		if(MQTT_IS_DEBUG(INTERNAL))
			vty_out(vty, "debug mqtt internal%s",  VTY_NEWLINE);
	}
	return OK;
}
