/*
 * voip_osip.c
 *
 *  Created on: Jan 22, 2019
 *      Author: zhurish
 */

#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"
#include "eloop.h"
#include "network.h"

#include "voip_def.h"
#include "voip_state.h"
#include "voip_sip.h"
#include "voip_osip.h"
#include "voip_stream.h"
#include "application.h"

#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>

#ifdef _OSIP_DEBUG_ENABLE
#define _OSIP_DEBUG(fmt,...)	zlog_debug(ZLOG_VOIP, fmt, ##__VA_ARGS__)
#else
#define _OSIP_DEBUG(fmt,...) zlog_debug(ZLOG_VOIP, fmt, ##__VA_ARGS__)
#endif

#define osip_log_wrapper(pri, fmt,...)	zlog(ZLOG_VOIP, pri, fmt, ##__VA_ARGS__)
//#define osip_log_wrapper(fmt,...)

static voip_osip_t *voip_osip = NULL;
int _osip_debug = 0;

static int
voip_osip_task (void *pVoid);
static int
_voip_osip_register_event (void *event);
static int
voip_osip_state_event_report (voip_osip_t *osip);
//typedef void osip_trace_func_t (char *fi, int li, osip_trace_level_t level, char *chfr, va_list ap);
/*
 void osip_trace_initialize_func (osip_trace_level_t level, osip_trace_func_t * func);
 void osip_trace_initialize_syslog (osip_trace_level_t level, char *ident);
 int osip_trace_initialize (osip_trace_level_t level, FILE * file);
 void osip_trace_enable_until_level (osip_trace_level_t level);
 void osip_trace_enable_level (osip_trace_level_t level);
 void osip_trace_disable_level (osip_trace_level_t level);
 int osip_is_trace_level_activate (osip_trace_level_t level);
 */
static int
voip_osip_call_create_instance (voip_osip_t *osip, char *name, int cid)
{
	int i = 0;
	zassert(osip != NULL);
	zassert(name != NULL);
	//voip_osip_lock(osip);
	for (i = 0; i < OSIP_MULTI_CALL_MAX; i++)
	{
		if (osip->multicall[i].cid == 0)
		{
			osip->multicall[i].cid = cid;
			memset (osip->multicall[i].callnumber, 0,
					sizeof(osip->multicall[i].callnumber));
			strncpy (osip->multicall[i].callnumber, name,
					 MIN(sizeof(osip->multicall[i].callnumber), strlen (name)));
			osip->callparam = &osip->multicall[i];
			VOIP_SIP_DEBUG(" ===== %s:Call '%s' cid:%d(%d)", __func__, name,
						   cid, i);
			//voip_osip_unlock(osip);
			return OK;
		}
	}
	//voip_osip_unlock(osip);
	return ERROR;
}

/*static int voip_osip_call_delete_instance(voip_osip_t *osip, char *name)
 {
 int i = 0;
 char 	callnumber[SIP_NUMBER_MAX];
 zassert(osip != NULL);
 zassert(name != NULL);
 //voip_osip_lock(osip);
 memset(callnumber, 0, sizeof(callnumber));
 strncpy(callnumber, name, MIN(sizeof(callnumber), strlen(name)));
 for(i = 0; i < OSIP_MULTI_CALL_MAX; i++)
 {
 if(strlen(osip->multicall[i].callnumber) && memcmp(osip->multicall[i].callnumber, callnumber, sizeof(callnumber)) == 0)
 {
 VOIP_SIP_DEBUG(" ===== %s:Call '%s' ", __func__, name);
 if(osip->multicall[i].delete_cb)
 (osip->multicall[i].delete_cb)(osip->multicall[i].pVoid);
 memset(&osip->multicall[i], 0, sizeof(callparam_t));
 if(osip->callparam == &osip->multicall[i])
 osip->callparam = NULL;
 //voip_osip_unlock(osip);
 return OK;
 }
 }
 //voip_osip_unlock(osip);
 return ERROR;
 }*/

/*static int voip_osip_call_delete_instance_bycid(voip_osip_t *osip, int cid)
 {
 int i = 0;
 zassert(osip != NULL);
 voip_osip_lock(osip);
 for(i = 0; i < OSIP_MULTI_CALL_MAX; i++)
 {
 if(cid > 0 && osip->multicall[i].cid == cid)
 {
 //VOIP_SIP_DEBUG(" ===== %s:Call '%s' ", __func__, name);
 if(osip->multicall[i].delete_cb)
 (osip->multicall[i].delete_cb)(osip->multicall[i].pVoid);
 memset(&osip->multicall[i], 0, sizeof(callparam_t));
 if(osip->callparam == &osip->multicall[i])
 osip->callparam = NULL;
 voip_osip_unlock(osip);
 return OK;
 }
 }
 voip_osip_unlock(osip);
 return ERROR;
 }*/

static int
voip_osip_call_delete_current_instance (voip_osip_t *osip, int cid)
{
	int i = 0;
	zassert(osip != NULL);
	if (osip->callparam)
	{
		zlog_debug(ZLOG_VOIP, " ===== %s:Call CID=%d ", __func__, cid);
		if (osip->callparam->delete_cb)
			(osip->callparam->delete_cb) (osip->callparam->pVoid);
		memset (osip->callparam, 0, sizeof(callparam_t));
		osip->callparam = NULL;
		return OK;
	}
	for (i = 0; i < OSIP_MULTI_CALL_MAX; i++)
	{
		if (cid > 0 && osip->multicall[i].cid == cid)
		{
			zlog_debug(ZLOG_VOIP, " ===== %s:Call CID=%d ", __func__, cid);
			if (osip->multicall[i].delete_cb)
				(osip->multicall[i].delete_cb) (osip->multicall[i].pVoid);
			memset (&osip->multicall[i], 0, sizeof(callparam_t));
			if (osip->callparam == &osip->multicall[i])
				osip->callparam = NULL;
			return OK;
		}
	}
	return ERROR;
}

/*static int voip_osip_call_set_instance(voip_osip_t *osip, char *name, int tid, int cid, int did)
 {
 int i = 0;
 char 	callnumber[SIP_NUMBER_MAX];
 zassert(osip != NULL);
 zassert(name != NULL);
 voip_osip_lock(osip);
 memset(callnumber, 0, sizeof(callnumber));
 strncpy(callnumber, name, MIN(sizeof(callnumber), strlen(name)));
 for(i = 0; i < OSIP_MULTI_CALL_MAX; i++)
 {
 if(strlen(osip->multicall[i].callnumber) && memcmp(osip->multicall[i].callnumber, callnumber, sizeof(callnumber)) == 0)
 {
 osip->multicall[i].tid		= tid;
 osip->multicall[i].cid		= cid;
 osip->multicall[i].did		= did;
 osip->multicall[i].state	= OSIP_CALL_NONE;
 VOIP_SIP_DEBUG(" ===== %s:Call '%s' tid=%d cid=%d did=%d", __func__, name, tid, cid, did);
 voip_osip_unlock(osip);
 return OK;
 }
 }
 voip_osip_unlock(osip);
 return ERROR;
 }*/

static int
voip_osip_call_update_instance_bycid (voip_osip_t *osip, int cid, char *name,
									  int tid, int did, int state)
{
	int i = 0;
	zassert(osip != NULL);
	//voip_osip_lock(osip);
	for (i = 0; i < OSIP_MULTI_CALL_MAX; i++)
	{
		if (cid > 0 && osip->multicall[i].cid == cid)
		{
			if (name)
			{
				memset (osip->multicall[i].callnumber, 0,
						sizeof(osip->multicall[i].callnumber));
				strncpy (
						osip->multicall[i].callnumber,
						name,
						MIN(sizeof(osip->multicall[i].callnumber),
							strlen (name)));
			}
			osip->multicall[i].tid = tid;
			osip->multicall[i].cid = cid;
			osip->multicall[i].did = did;
			osip->multicall[i].state = state;
			osip->callparam = &osip->multicall[i];
			VOIP_SIP_DEBUG(" ===== %s:Call '%s' tid=%d cid=%d did=%d", __func__,
						   name ? name : "NULL", tid, cid, did);
			//voip_osip_unlock(osip);
			return OK;
		}
	}
	//voip_osip_unlock(osip);
	return ERROR;
}

/*static callparam_t *voip_osip_call_lookup_instance(voip_osip_t *osip, char *name)
 {
 int i = 0;
 char 	callnumber[SIP_NUMBER_MAX];
 zassert(osip != NULL);
 zassert(name != NULL);
 voip_osip_lock(osip);
 memset(callnumber, 0, sizeof(callnumber));
 strncpy(callnumber, name, MIN(sizeof(callnumber), strlen(name)));
 for(i = 0; i < OSIP_MULTI_CALL_MAX; i++)
 {
 if(strlen(osip->multicall[i].callnumber) && memcmp(osip->multicall[i].callnumber, callnumber, sizeof(callnumber)) == 0)
 {
 voip_osip_unlock(osip);
 return &osip->multicall[i];
 }
 }
 voip_osip_unlock(osip);
 return NULL;
 }*/

callparam_t *
voip_osip_call_lookup_instance_bycid (voip_osip_t *osip, int cid)
{
	int i = 0;
	zassert(osip != NULL);
	voip_osip_lock(osip);
	for (i = 0; i < OSIP_MULTI_CALL_MAX; i++)
	{
		if (cid > 0 && osip->multicall[i].cid == cid)
		{
			voip_osip_unlock(osip);
			return &osip->multicall[i];
		}
	}
	voip_osip_unlock(osip);
	return NULL;
}

#ifdef OSIP_SELF_CB_ENABLE
static int
voip_osip_self_callback_add (voip_osip_t *osip, int
(*cb) (void *),
							 void *pVoid)
{
	int i = 0;
	zassert(osip != NULL);
	for (i = 0; i < OSIP_CALLBACK_MAX; i++)
	{
		if (osip->cb_table[i].bReady == FALSE)
		{
			osip->cb_table[i].bReady = TRUE;
			osip->cb_table[i].cb = cb;
			osip->cb_table[i].pVoid = pVoid;
			return OK;
		}
	}
	return ERROR;
}

static int
voip_osip_self_callback_process (voip_osip_t *osip)
{
	int i = 0;
	zassert(osip != NULL);
	for (i = 0; i < OSIP_CALLBACK_MAX; i++)
	{
		if (osip->cb_table[i].bReady == TRUE)
		{
			if (osip->cb_table[i].cb)
				(osip->cb_table[i].cb) (osip->cb_table[i].pVoid);
			osip->cb_table[i].bReady = FALSE;
		}
	}
	return ERROR;
}
#endif /* OSIP_SELF_CB_ENABLE */

int
voip_osip_media_callback (voip_osip_t *osip, osip_media_callback_t *callback)
{
	zassert(osip != NULL);
	zassert(callback != NULL);
	voip_osip_lock(osip);
	osip->media_cb.media_start_cb = callback->media_start_cb;
	osip->media_cb.pVoidStart = callback->pVoidStart;
	osip->media_cb.media_stop_cb = callback->media_stop_cb;
	osip->media_cb.pVoidStop = callback->pVoidStop;
	voip_osip_unlock(osip);
	return OK;
}

static void
osip_trace_cb_func_t (char *fi, int li, osip_trace_level_t level, char *chfr,
					  va_list ap)
{
	switch (level)
	{
		case OSIP_FATAL:
			pl_vzlog (fi, NULL, li, NULL, ZLOG_VOIP, LOG_ALERT, chfr, ap);
			break;
		case OSIP_BUG:
			pl_vzlog (fi, NULL, li, NULL, ZLOG_VOIP, LOG_EMERG, chfr, ap);
			break;
		case OSIP_ERROR:
			pl_vzlog (fi, NULL, li, NULL, ZLOG_VOIP, LOG_ERR, chfr, ap);
			break;
		case OSIP_WARNING:
			pl_vzlog (fi, NULL, li, NULL, ZLOG_VOIP, LOG_WARNING, chfr, ap);
			break;
		case OSIP_INFO1:
			pl_vzlog (fi, NULL, li, NULL, ZLOG_VOIP, LOG_NOTICE, chfr, ap);
			break;
		case OSIP_INFO2:
			pl_vzlog (fi, NULL, li, NULL, ZLOG_VOIP, LOG_DEBUG, chfr, ap);
			break;
		case OSIP_INFO3:
			pl_vzlog (fi, NULL, li, NULL, ZLOG_VOIP, LOG_DEBUG, chfr, ap);
			break;
		case OSIP_INFO4:
			pl_vzlog (fi, NULL, li, NULL, ZLOG_VOIP, LOG_DEBUG, chfr, ap);
			break;
		default:
			break;
	}
}

int
voip_osip_set_log_level (int level, int detail)
{
	int inval = 0;
	switch (level)
	{
		case LOG_ALERT:
			inval = OSIP_FATAL;
			break;
		case LOG_EMERG:
			inval = OSIP_BUG;
			break;
		case LOG_ERR:
			inval = OSIP_ERROR;
			break;
		case LOG_WARNING:
			inval = OSIP_WARNING;
			break;
		case LOG_NOTICE:
			inval = OSIP_INFO1;
			break;
		case LOG_DEBUG:
			if (detail)
				inval = OSIP_INFO4;
			else
				inval = OSIP_INFO2;
			break;
		default:
			inval = OSIP_FATAL;
			break;
	}
	osip_trace_enable_until_level (inval);
	if (voip_osip)
	{
		if (level)
			voip_osip->debug |= (1 << level);
		else
			voip_osip->debug = 0;
		voip_osip->detail = detail;
	}
	return OK;
}

int
voip_osip_get_log_level (int *level, int *detail)
{
	if (voip_osip)
	{
		if (level)
			*level = voip_osip->debug;
		if (detail)
			*detail = voip_osip->detail;
	}
	return OK;
}

static char *
osip_transport_proto (sip_transport_t proto)
{
	if (proto == SIP_PROTO_UDP)
		return "UDP";
	else if (proto == SIP_PROTO_TCP)
		return "TCP";
	else if (proto == SIP_PROTO_TLS)
		return "TLS";
	else if (proto == SIP_PROTO_DTLS)
		return "DTLS";
	return "Unknow";
}

static void
_voip_osip_CbSipCallback (osip_message_t * msg, int received)
{
	char* dest = NULL;
	size_t length = 0;
	zassert(msg != NULL);
	if (msg)
	{
		osip_message_to_str (msg, &dest, &length);
		if (length && dest)
		{
			if (OSIP_IS_DEBUG(MSG))
				zlog_debug(
						ZLOG_VOIP,
						"<#######[%s] sip_method: %s, status_code: %d  %s \n%s\n#######>\n",
						(received == 1) ? "recv" : "send", msg->sip_method,
						msg->status_code, msg->reason_phrase, dest);
		}
	}
	return;
}

#ifdef _OSIP_DEBUG_ENABLE
static int _voip_osip_config_show_debug(voip_osip_t *osip)
{
	return OK;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	zlog_debug(ZLOG_VOIP, "======active====== osip enable           %d", osip->enable);
	zlog_debug(ZLOG_VOIP, "======active====== osip context          %p", osip->context);
	zlog_debug(ZLOG_VOIP, "======active====== osip address          %s", osip->address);
	zlog_debug(ZLOG_VOIP, "======active====== osip username         %s", osip->remote_param->username);
	zlog_debug(ZLOG_VOIP, "======active====== osip userid           %s", osip->remote_param->userid);
	zlog_debug(ZLOG_VOIP, "======active====== osip password         %s", osip->remote_param->password);
	zlog_debug(ZLOG_VOIP, "======active====== osip proxy            %s", osip->remote_param->proxy);
	zlog_debug(ZLOG_VOIP, "======active====== osip fromuser         %s", osip->remote_param->fromuser);
	zlog_debug(ZLOG_VOIP, "======active====== osip contact          %s", osip->remote_param->contact);
	zlog_debug(ZLOG_VOIP, "======active====== osip port             %d", osip->port);
	return OK;
}
#endif

static int
voip_osip_config_debug_detail (voip_osip_t *voiposip)
{
	zassert(voiposip != NULL);
	//zassert(voiposip->remote_param != NULL);
	if (OSIP_IS_DEBUG(MSG) && OSIP_IS_DEBUG(DETAIL))
	{
		zlog_debug(ZLOG_VOIP, "osip proto                   %s",
				   osip_transport_proto (voiposip->proto));
		zlog_debug(ZLOG_VOIP, "osip local address           %s:%d",
				   voiposip->address, voiposip->port);
		if (!voiposip->remote_param)
			return ERROR;
		zlog_debug(ZLOG_VOIP, "osip username                %s",
				   voiposip->remote_param->username);
		zlog_debug(ZLOG_VOIP, "osip password                %s",
				   voiposip->remote_param->password);
		zlog_debug(ZLOG_VOIP, "osip remote address          %s:%d",
				   voiposip->remote_param->remote_address,
				   voiposip->remote_param->remote_port);
		zlog_debug(ZLOG_VOIP, "osip proxy                   %s",
				   voiposip->remote_param->proxy);
		zlog_debug(ZLOG_VOIP, "osip fromuser                %s",
				   voiposip->remote_param->fromuser);
		zlog_debug(ZLOG_VOIP, "osip contact                 %s",
				   voiposip->remote_param->contact);
		zlog_debug(ZLOG_VOIP, "osip firewallip              %s",
				   voiposip->remote_param->firewallip);

	}
	return OK;
}

static int
voip_osip_config_default (voip_osip_t *voiposip)
{
	zassert(voiposip != NULL);
	zassert(voiposip->remote_param != NULL);
	memset (voiposip->address, 0, sizeof(voiposip->address));
	voiposip->port = SIP_PORT_DEFAULT;
	voiposip->proto = SIP_PROTO_UDP;
	voip_osip->initialization = FALSE;
	voip_osip->register_interval = REG_NO_ANSWER_INTERVAL;

	memset (voiposip->remote_param->remote_address, 0,
			sizeof(voiposip->remote_param->remote_address));
	voiposip->remote_param->remote_port = SIP_PORT_DEFAULT;
	memset (voiposip->remote_param->firewallip, 0,
			sizeof(voiposip->remote_param->firewallip));
	memset (voiposip->remote_param->username, 0,
			sizeof(voiposip->remote_param->username));
	memset (voiposip->remote_param->userid, 0,
			sizeof(voiposip->remote_param->userid));
	memset (voiposip->remote_param->password, 0,
			sizeof(voiposip->remote_param->password));
	memset (voiposip->remote_param->proxy, 0,
			sizeof(voiposip->remote_param->proxy));
	memset (voiposip->remote_param->fromuser, 0,
			sizeof(voiposip->remote_param->fromuser));
	memset (voiposip->remote_param->contact, 0,
			sizeof(voiposip->remote_param->contact));
	voiposip->remote_param->regparam.regid = 0;
	voiposip->remote_param->regparam.expiry = SIP_REGINTER_DEFAULT;
	voiposip->remote_param->regparam.auth = 0;

	return OK;
}

static int
voip_osip_config_reset (voip_osip_t *voiposip)
{
	zassert(voiposip != NULL);
	zassert(voiposip->remote_param != NULL);
	memset (voiposip->address, 0, sizeof(voiposip->address));
	/*	voiposip->port = 0;
	 voiposip->proto = OSIP_PROTO_UDP;*/
	voip_osip->initialization = FALSE;

	memset (voiposip->remote_param->remote_address, 0,
			sizeof(voiposip->remote_param->remote_address));
	memset (voiposip->remote_param->firewallip, 0,
			sizeof(voiposip->remote_param->firewallip));
	memset (voiposip->remote_param->username, 0,
			sizeof(voiposip->remote_param->username));
	memset (voiposip->remote_param->userid, 0,
			sizeof(voiposip->remote_param->userid));
	memset (voiposip->remote_param->password, 0,
			sizeof(voiposip->remote_param->password));
	memset (voiposip->remote_param->proxy, 0,
			sizeof(voiposip->remote_param->proxy));
	memset (voiposip->remote_param->fromuser, 0,
			sizeof(voiposip->remote_param->fromuser));
	memset (voiposip->remote_param->contact, 0,
			sizeof(voiposip->remote_param->contact));
	voiposip->remote_param->regparam.regid = 0;
	//voiposip->remote_param->regparam.expiry = SIP_REGINTER_DEFAULT;
	voiposip->remote_param->regparam.auth = 0;
	return OK;
}

static int
voip_osip_config_load_source_interface (voip_osip_t *osip, voip_sip_t *sip)
{
	zassert(sip != NULL);
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);

	memset (osip->address, 0, sizeof(osip->address));

	if (sip->sip_local_address)
		strcpy (osip->address, inet_address (sip->sip_local_address));
	else if (sip->sip_source_interface)
	{
		u_int32 address = voip_get_address (sip->sip_source_interface);
		if (address)
			strcpy (osip->address, inet_address (address));
	}
	osip->port = sip->sip_local_port;
	osip->proto = sip->proto;
	osip->remote_param->regparam.regid = 0;
	osip->remote_param->regparam.expiry = sip->sip_register_interval;
	osip->remote_param->regparam.auth = 0;
	return OK;
}

static int
voip_osip_config_load_username (voip_osip_t *osip, voip_sip_t *sip, BOOL sec)
{
	zassert(sip != NULL);
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);

	if (sec)
	{
		memset (osip->remote_param->username, 0,
				sizeof(osip->remote_param->username));
		memset (osip->remote_param->userid, 0,
				sizeof(osip->remote_param->userid));
		memset (osip->remote_param->password, 0,
				sizeof(osip->remote_param->password));

		if (strlen (sip->sip_user_alias))
		{
			strcpy (osip->remote_param->username, sip->sip_user_alias);
		}
		if (strlen (sip->sip_local_number_alias))
		{
			strcpy (osip->remote_param->userid, sip->sip_local_number_alias);
			if (strlen (sip->sip_user_alias) == 0)
				strcpy (osip->remote_param->username,
						sip->sip_local_number_alias);
		}
		if (strlen (sip->sip_password_alias))
		{
			strcpy (osip->remote_param->password, sip->sip_password_alias);
		}
	}
	else
	{
		memset (osip->remote_param->username, 0,
				sizeof(osip->remote_param->username));
		memset (osip->remote_param->userid, 0,
				sizeof(osip->remote_param->userid));
		memset (osip->remote_param->password, 0,
				sizeof(osip->remote_param->password));

		if (strlen (sip->sip_user))
		{
			strcpy (osip->remote_param->username, sip->sip_user);
		}
		if (strlen (sip->sip_local_number))
		{
			strcpy (osip->remote_param->userid, sip->sip_local_number);
			if (strlen (sip->sip_user) == 0)
				strcpy (osip->remote_param->username, sip->sip_local_number);
		}
		if (strlen (sip->sip_password))
		{
			strcpy (osip->remote_param->password, sip->sip_password);
		}
	}
	return OK;
}

static int
voip_osip_config_load_server (voip_osip_t *osip, voip_sip_t *sip, BOOL sec)
{
	zassert(sip != NULL);
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	if (sec)
	{
		osip->remote_param->remote_port = sip->sip_port_sec;
		if (sip->sip_server_sec)
			strcpy (osip->remote_param->remote_address,
					inet_address (sip->sip_server_sec));
	}
	else
	{
		osip->remote_param->remote_port = sip->sip_port;
		if (sip->sip_server)
			strcpy (osip->remote_param->remote_address,
					inet_address (sip->sip_server));
	}
	return OK;
}

static int
voip_osip_obtain_switch_policy (voip_osip_t *osip)
{
	zassert(osip != NULL);
	zassert(osip->sip != NULL);

	osip->osip_policy.reg_fail = 0;
	osip->osip_policy.reg_no_answer = 0;

	if (osip->sip->mutex)
		os_mutex_lock (osip->sip->mutex, OS_WAIT_FOREVER);

	if (voip_sip_multiuser_get_api ())
	{
		osip->sip->bUserMain = !osip->sip->bUserMain;
	}
	if (voip_sip_active_standby_get_api ())
	{
		osip->sip->bSipMain = !osip->sip->bSipMain;
	}
	osip->osip_policy.standby = !osip->osip_policy.standby;

	osip->osip_policy.bSwitch = TRUE;

	if (osip->sip->mutex)
		os_mutex_unlock (osip->sip->mutex);

	/*	if(osip->r_event)
	 voip_event_cancel(osip->r_event);
	 osip->r_event = NULL;

	 if(osip->r_reset)
	 voip_event_cancel(osip->r_reset);
	 osip->r_reset = NULL;

	 if(osip->r_init)
	 voip_event_cancel(osip->r_init);
	 osip->r_init = NULL;*/

	return OK;
}

static int
voip_osip_switch_resion (voip_osip_t *osip, int reason)
{
	int ret = ERROR;
	zassert(osip != NULL);
	zassert(osip->sip != NULL);
	if (reason > 0)
		return ERROR;
	switch (reason)
	{
		case OSIP_SUCCESS:
			break;
		case OSIP_UNDEFINED_ERROR:
			break;
		case OSIP_BADPARAMETER:
			osip->osip_policy.switch_reason = reason;
			osip->initialization = FALSE;
			ret = OK;
			break;
			break;
		case OSIP_WRONG_STATE:
			break;
		case OSIP_NOMEM:
			break;
		case OSIP_SYNTAXERROR:
			break;
		case OSIP_NOTFOUND:
			osip->osip_policy.switch_reason = reason;
			ret = OK;
			break;
			break;
		case OSIP_API_NOT_INITIALIZED:
			osip->osip_policy.switch_reason = reason;
			osip->initialization = FALSE;
			ret = OK;
			break;

		case OSIP_NO_NETWORK:
		case OSIP_PORT_BUSY:
		case OSIP_UNKNOWN_HOST:
			osip->osip_policy.switch_reason = reason;
			osip->initialization = FALSE;
			ret = OK;
			break;

		case OSIP_DISK_FULL:
			break;
		case OSIP_NO_RIGHTS:
			osip->osip_policy.switch_reason = reason;
			ret = OK;
			break;
		case OSIP_FILE_NOT_EXIST:
			break;
		case OSIP_TIMEOUT:
			osip->osip_policy.switch_reason = reason;
			osip->initialization = FALSE;
			ret = OK;
			break;
		case OSIP_TOOMUCHCALL:
			break;
		case OSIP_WRONG_FORMAT:
		case OSIP_NOCOMMONCODEC:
			break;
		default:
			if (osip->osip_policy.reg_fail >= 5 ||		//register fail count
					osip->osip_policy.reg_no_answer >= 3)		//no answer
			{
				ret = OK;
			}
			break;
	}
	return ret;
}

static int
voip_osip_switch_policy_applay (voip_osip_t *osip, int reason)
{
	//
	zassert(osip != NULL);
	zassert(osip->sip != NULL);
	if (!voip_sip_multiuser_get_api () || !voip_sip_active_standby_get_api ())
		return ERROR;

	if (osip_register_state_get (osip) == SIP_STATE_REGISTER_SUCCESS)
		return ERROR;

	if (voip_osip->osip_policy.bSwitching)
		return ERROR;

	if (voip_osip_switch_resion (osip, reason) == OK)
	{
		voip_osip->osip_policy.bSwitching = TRUE;
		voip_osip_obtain_switch_policy (osip);
		voip_osip_restart ();
		return OK;
	}
	return ERROR;
}

static int
voip_osip_config_load (voip_osip_t *voiposip)
{
	zassert(voiposip != NULL);
	zassert(voiposip->sip != NULL);
	zassert(voiposip->remote_param != NULL);
	memset (voiposip->remote_param, 0, sizeof(osip_remote_param_t));
	memset (&voiposip->osip_policy, 0, sizeof(osip_policy_t));
	voiposip->osip_policy.standby = FALSE;
	voip_status_register_main_api (TRUE);
	memset (&voiposip->statistics, 0, sizeof(osip_statistics_t));
	voip_osip_config_reset (voiposip);

	//sip->bUserMain		= TRUE;			//当前是否使用主号码
	//sip->bSipMain		= TRUE;			//当前是否使用主服务器
	voip_osip_config_reset (voiposip);
	if (voiposip->sip->mutex)
		os_mutex_lock (voiposip->sip->mutex, OS_WAIT_FOREVER);
	voip_osip_config_load_source_interface (voiposip, voiposip->sip);
	voip_osip_config_load_username (voiposip, voiposip->sip,
									!voiposip->sip->bUserMain);
	voip_osip_config_load_server (voiposip, voiposip->sip,
								  !voiposip->sip->bSipMain);
	if (voiposip->sip->mutex)
		os_mutex_unlock (voiposip->sip->mutex);
	return OK;
}

static int
voip_osip_config_reload (voip_osip_t *voiposip)
{
	zassert(voiposip != NULL);
	zassert(voiposip->sip != NULL);
	if (voiposip->r_event)
		voip_event_cancel(voiposip->r_event);
	voiposip->r_event = NULL;

	if (voiposip->r_reset)
		voip_event_cancel(voiposip->r_reset);
	voiposip->r_reset = NULL;

	if (voiposip->r_init)
		voip_event_cancel(voiposip->r_init);
	voiposip->r_init = NULL;

	if (voiposip->osip_policy.bSwitch)
	{
		if (voiposip->osip_policy.standby)
		{
			voiposip->remote_param = &voiposip->remote_param_standby;
			voip_status_register_main_api (FALSE);
		}
		else
		{
			voiposip->remote_param = &voiposip->remote_param_main;
			voip_status_register_main_api (TRUE);
		}
		voiposip->osip_policy.bSwitch = FALSE;
	}
	if (voiposip->osip_policy.bSwitching)
		voiposip->osip_policy.bSwitching = FALSE;

	memset (&voiposip->statistics, 0, sizeof(osip_statistics_t));
	OSIP_POLICY_REG_ZERO(voiposip, no_answer);
	OSIP_POLICY_REG_ZERO(voiposip, fail);
	zassert(voiposip->remote_param != NULL);
	memset (voiposip->remote_param, 0, sizeof(osip_remote_param_t));
	//sip->bUserMain		= TRUE;			//当前是否使用主号码
	//sip->bSipMain		= TRUE;			//当前是否使用主服务器
	voip_osip_config_reset (voiposip);
	if (voiposip->sip->mutex)
		os_mutex_lock (voiposip->sip->mutex, OS_WAIT_FOREVER);
	voip_osip_config_load_source_interface (voiposip, voiposip->sip);
	voip_osip_config_load_username (voiposip, voiposip->sip,
									!voiposip->sip->bUserMain);
	voip_osip_config_load_server (voiposip, voiposip->sip,
								  !voiposip->sip->bSipMain);
	if (voiposip->sip->mutex)
		os_mutex_unlock (voiposip->sip->mutex);
	return OK;
}

static int
voip_osip_config_active (voip_osip_t *voiposip)
{
	zassert(voiposip != NULL);
	zassert(voiposip->remote_param != NULL);

	//if(voiposip->enable > 0)
	if (voiposip->context && voiposip->remote_param
			&& strlen (voiposip->address)
			&& strlen (voiposip->remote_param->remote_address)
			&& strlen (voiposip->remote_param->username)
			&& strlen (voiposip->remote_param->password) &&
			/*			strlen(voiposip->proxy) &&
			 strlen(voiposip->fromuser) &&
			 strlen(voiposip->contact) &&*/
			voiposip->port && voiposip->remote_param->remote_port)
	{
		voiposip->remote_param->active = OSIP_ACTIVE;
		if (OSIP_IS_DEBUG(EVENT))
		{
			zlog_debug(ZLOG_VOIP, "OSIP stack is ACTIVE!");
		}
		return OSIP_ACTIVE;
	}

	voiposip->remote_param->active = OSIP_INACTIVE;
	return OSIP_INACTIVE;
}

/*
 via 消息经过192.168.50.108
 From 请求由192.168.50.108 上的分机1000发起
 Contact 用户可用此URL通信
 to 请求的目的方是192.168.50.105上的分机4000

 注册消息的起始和目地地址一样

 */
static int
voip_osip_proxy_build (voip_osip_t *osip)
{
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	if (osip->remote_param && strlen (osip->remote_param->remote_address))
	{
		memset (osip->remote_param->proxy, 0,
				sizeof(osip->remote_param->proxy));
		snprintf (osip->remote_param->proxy, sizeof(osip->remote_param->proxy),
				  "sip:%s:%d", osip->remote_param->remote_address,
				  osip->remote_param->remote_port);
		return OK;
	}
	return ERROR;
}

/*
 * local IP address and port
 */
static int
voip_osip_fromuser_build (voip_osip_t *osip)
{
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	if (osip->remote_param && strlen (osip->remote_param->username)
			&& strlen (osip->remote_param->remote_address))
	{
		memset (osip->remote_param->fromuser, 0,
				sizeof(osip->remote_param->fromuser));
		snprintf (osip->remote_param->fromuser,
				  sizeof(osip->remote_param->fromuser), "sip:%s@%s:%d",
				  osip->remote_param->username,
				  osip->remote_param->remote_address,
				  osip->remote_param->remote_port);
		return OK;
	}
	return ERROR;
}

static int
voip_osip_contact_build (voip_osip_t *osip)
{
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	if (osip->remote_param && strlen (osip->remote_param->username)
			&& strlen (osip->remote_param->remote_address))
	{
		memset (osip->remote_param->contact, 0,
				sizeof(osip->remote_param->contact));
		snprintf (osip->remote_param->contact,
				  sizeof(osip->remote_param->contact), "sip:%s@%s:%d",
				  osip->remote_param->username,
				  (osip->remote_param->remote_address),
				  osip->remote_param->remote_port);
		return OK;
	}

	return ERROR;
}

static int
voip_osip_call_source_build (voip_osip_t *osip, char *source, int len)
{
	if (!osip || !source)
		return ERROR;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	if (osip->remote_param && strlen (osip->remote_param->userid)
			&& strlen (osip->address))
	{
		snprintf (source, len, "<sip:%s@%s:%d>", osip->remote_param->userid,
				  osip->address, osip->port);
		return OK;
	}
	return ERROR;
}

static int
voip_osip_call_destination_build (voip_osip_t *osip, char *username,
								  char *phonenumber, char *dest, int len)
{
	if (!osip || !dest)
		return ERROR;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	if (osip->remote_param && strlen (osip->remote_param->remote_address)
			&& phonenumber)
	{
		snprintf (dest, len, "<sip:%s@%s:%d>", phonenumber,
				  osip->remote_param->remote_address,
				  osip->remote_param->remote_port);
		return OK;
	}

	return ERROR;
}

static int
voip_osip_call_sdp_rtpmap_make (voip_osip_t *osip, char *sdp, char *avr)
{
	char *tmp = NULL;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	zassert(sdp != NULL);
	int payload = voip_sip_get_payload_index ();
	if (voip_sip_payload_name (payload))
	{
		tmp = voip_sip_payload_rtpmap (payload);
		if (tmp)
		{
			strcat (sdp, "a=rtpmap:");
			strcat (sdp, tmp);
			strcat (sdp, "\r\n");
		}
		if (voip_sip_payload_ptime (payload) < 255
				&& voip_sip_payload_ptime (payload) > 0)
		{
			strcat (sdp, "a=ptime:");
			strcat (sdp, itoa (voip_sip_payload_ptime (payload), 10));
			strcat (sdp, "\r\n");
		}
		if (avr)
		{
			strcat (avr, " ");
			strcat (avr, itoa (payload, 10));
		}
	}
	return OK;
}

static int
voip_osip_call_sdp_build (voip_osip_t *osip, char *username, char *phonenumber,
						  char *sdp, int len)
{
	if (!osip || !sdp)
		return ERROR;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	zassert(sdp != NULL);
	if (strlen (osip->address) && phonenumber)
	{
		sip_dtmf_t dtmf = 0;
		char sdptmp[1024];
		char vartmp[512];
		u_int16 l_rtp_port = voip_stream_local_port_get_api ();

		voip_sip_dtmf_get_api (&dtmf);
		snprintf (sdp, len, "v=0\r\n"                           	// SDP版本
				  "o=X5B 0 0 IN IP4 %s\r\n"// 用户名、ID、版本、网络类型、地址类型、IP地址
				  "s=X5B-session\r\n"// 会话名称
				"c=IN IP4 %s\r\n"//c 真正流媒体使用的IP地址
				"t=0 0\r\n"// 开始时间、结束时间。此处不需要设置
				"a=sendrecv\r\n"
				"m=audio %d RTP/AVP",// 音频、传输端口、传输类型、格式列表
				osip->address, osip->address, l_rtp_port);

		memset (sdptmp, 0, sizeof(sdptmp));
		memset (vartmp, 0, sizeof(vartmp));

		voip_osip_call_sdp_rtpmap_make (osip, sdptmp, vartmp);

		if (dtmf == VOIP_SIP_RFC2833)
		{
			strcat (sdptmp, "a=rtpmap:101 telephone-event/8000\r\n");
			strcat (vartmp, " 101");
		}
		if (strlen (vartmp))
		{
			strcat (sdp, vartmp);
			strcat (sdp, "\r\n");
		}
		if (strlen (sdptmp))
		{
			strcat (sdp, sdptmp);
			//strcat(sdp, "\r\n");
		}
#if 0
		if(dtmf == VOIP_SIP_RFC2833)
		{
			snprintf(sdp, len, "v=0\r\n"                           	// SDP版本
					"o=X5B 0 0 IN IP4 %s\r\n"// 用户名、ID、版本、网络类型、地址类型、IP地址
					"s=X5B-session\r\n"// 会话名称
					"c=IN IP4 %s\r\n"//c 真正流媒体使用的IP地址
					"t=0 0\r\n"// 开始时间、结束时间。此处不需要设置
					"m=audio %d RTP/AVP 0 8 101\r\n"// 音频、传输端口、传输类型、格式列表
					"a=rtpmap:0 PCMU/8000\r\n"// 以下为具体描述格式列表中的
					"a=rtpmap:8 PCMA/8000\r\n"
					"a=rtpmap:101 telephone-event/8000\r\n"
					"a=ptime:10\r\n"
					"a=sendrecv\r\n", osip->address, osip->address, l_rtp_port);
		}
		else
		snprintf(sdp, len, "v=0\r\n"                           	// SDP版本
				"o=X5B 0 0 IN IP4 %s\r\n"// 用户名、ID、版本、网络类型、地址类型、IP地址
				"s=X5B-session\r\n"// 会话名称
				"c=IN IP4 %s\r\n"//c 真正流媒体使用的IP地址
				"t=0 0\r\n"// 开始时间、结束时间。此处不需要设置
				"m=audio %d RTP/AVP 0 8\r\n"// 音频、传输端口、传输类型、格式列表
				"a=rtpmap:0 PCMU/8000\r\n"// 以下为具体描述格式列表中的
				"a=rtpmap:8 PCMA/8000\r\n"
				"a=ptime:10\r\n"
				"a=sendrecv\r\n", osip->address, osip->address, l_rtp_port);
#endif
		if (OSIP_IS_DEBUG(MSG) && OSIP_IS_DEBUG(DETAIL))
			zlog_debug(ZLOG_VOIP, "call sdp:%s", sdp);
		return OK;
	}//						a=fmtp:101 0-11\r\n "a=rtpmap:101 telephone-event/8000\r\n"
	return ERROR;
}

static int
voip_osip_sigtran_supported (voip_osip_t *osip, char *sigtran, int len)
{
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	zassert(sigtran != NULL);
	if (osip && sigtran)
	{
		sip_dtmf_t dtmf = 0;
		voip_sip_dtmf_get_api (&dtmf);
		if (dtmf == VOIP_SIP_INFO)
			snprintf (sigtran, len,
					  "INVITE, ACK, OPTIONS, CANCEL, BYTE, INFO, MESSAGE");
		else
			snprintf (sigtran, len,
					  "INVITE, ACK, OPTIONS, CANCEL, BYTE, MESSAGE");
		/*		if(dtmf == VOIP_SIP_INFO)
		 snprintf(sigtran, len, "INVITE, ACK, OPTIONS, CANCEL, BYTE, SUBSCRIBE, NOTIFY, INFO, REFER, UPDATE, MESSAGE");
		 else
		 snprintf(sigtran, len, "INVITE, ACK, OPTIONS, CANCEL, BYTE, SUBSCRIBE, NOTIFY, REFER, UPDATE, MESSAGE");*/
		return OK;
	}
	return ERROR;
}

static int
voip_osip_context_init_api (voip_osip_t *osip)
{
	int err = 0;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	zassert(osip->context != NULL);
	if (osip->proto == SIP_PROTO_UDP)
	{
		err = eXosip_listen_addr (osip->context, IPPROTO_UDP, NULL, osip->port,
								  AF_INET, 0);
	}
	else if (osip->proto == SIP_PROTO_TCP)
	{
		err = eXosip_listen_addr (osip->context, IPPROTO_TCP, NULL, osip->port,
								  AF_INET, 0);
	}
	else if (osip->proto == SIP_PROTO_TLS)
	{
		err = eXosip_listen_addr (osip->context, IPPROTO_TCP, NULL, osip->port,
								  AF_INET, 1);
	}
	else if (osip->proto == SIP_PROTO_DTLS)
	{
		err = eXosip_listen_addr (osip->context, IPPROTO_UDP, NULL, osip->port,
								  AF_INET, 1);
	}
	if (err)
	{
		osip_log_wrapper(LOG_ERR, "eXosip_listen_addr failed");
		return ERROR;
	}
	if (strlen (osip->address))
	{
		osip_log_wrapper(LOG_INFO, "local address: %s", osip->address);
		eXosip_masquerade_contact (osip->context, osip->address, osip->port);
	}
	if (osip->remote_param && strlen (osip->remote_param->firewallip))
	{
		osip_log_wrapper(LOG_INFO, "firewall address: %s:%i",
						 osip->remote_param->firewallip, osip->port);
		eXosip_masquerade_contact (osip->context,
								   osip->remote_param->firewallip, osip->port);
	}

	eXosip_set_user_agent (osip->context, VOIP_OSIP_UA_STRING);

	osip->initialization = TRUE;
#ifdef _OSIP_DEBUG_ENABLE
	_voip_osip_config_show_debug(osip);
#endif
	return OK;
}

static int
voip_osip_context_username_api (voip_osip_t *osip)
{
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	zassert(osip->context != NULL);
	if (osip->remote_param && strlen (osip->remote_param->username)
			&& strlen (osip->remote_param->userid)
			&& strlen (osip->remote_param->password))
	{
		//osip_log_wrapper(LOG_INFO, "username: %s", osip->username);
		//osip_log_wrapper(LOG_INFO, "password: [removed]");
		if (eXosip_add_authentication_info (osip->context,
											osip->remote_param->username,
											osip->remote_param->userid,
											osip->remote_param->password, NULL,
											NULL))
		{
			osip_log_wrapper(LOG_ERR, "eXosip_add_authentication_info failed");
			return ERROR;
		}
	}
	return OK;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

/*
 *
 */

static int
voip_osip_stats_handle (voip_osip_t *osip, int *counter)
{
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	if (counter && (*counter) % 60000 == 0)
	{
		struct eXosip_stats stats;
		memset (&stats, 0, sizeof(struct eXosip_stats));
		eXosip_lock (osip->context);
		eXosip_set_option (osip->context, EXOSIP_OPT_GET_STATISTICS, &stats);
		eXosip_unlock (osip->context);
		if (OSIP_IS_DEBUG(INFO) && OSIP_IS_DEBUG(DETAIL))
			osip_log_wrapper(
					LOG_INFO,
					"eXosip stats: inmemory=(tr:%i//reg:%i) average=(tr:%f//reg:%f)",
					stats.allocated_transactions, stats.allocated_registrations,
					stats.average_transactions, stats.average_registrations);
	}
	return OK;
}
/*
 * register status callback
 */
static int
voip_osip_register_handle (voip_osip_t *osip, int res, int status_code)
{
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	zassert(osip->sip != NULL);
	//zassert(osip->callparam != NULL);
	if (res == EXOSIP_REGISTRATION_SUCCESS)
	{
		if (status_code == -1)
		{
			OSIP_STATISTICS_REG_INC(osip, no_answer);
			OSIP_POLICY_REG_INC(osip, no_answer);
			osip->register_interval = REG_NO_ANSWER_INTERVAL;
			osip_register_state_set (osip, SIP_STATE_UNREGISTER);
			if (osip->sip->bRegMain || osip->sip->bRegStanby)
			{
				osip->sip->bRegMain = FALSE;
				osip->sip->bRegStanby = FALSE;
				x5b_app_register_status_api (NULL, 0, E_CMD_TO_AUTO);
			}
			voip_osip_switch_policy_applay (osip, -1000);

			if (OSIP_IS_DEBUG(EVENT) && OSIP_IS_DEBUG(DETAIL))
				zlog_notice(ZLOG_VOIP, "%s:%s@%s:%d registrered no respone",
							osip->remote_param->username,
							osip->remote_param->userid, osip->address,
							osip->port);
			return OK;
		}
		OSIP_STATISTICS_REG_INC(osip, success);
		OSIP_POLICY_REG_ZERO(osip, no_answer);
		OSIP_POLICY_REG_ZERO(osip, fail);

		osip->register_interval = osip->remote_param->regparam.expiry >> 2;

		/*
		 if(osip->r_event)
		 voip_event_cancel(osip->r_event);
		 zlog_err(ZLOG_VOIP, "===================%s: %s", __func__, "_voip_osip_register_event");
		 osip->r_event = voip_event_timer_add(_voip_osip_register_event, voip_osip, NULL, 0, voip_osip->register_interval);
		 */

		if (OSIP_IS_DEBUG(EVENT))
			zlog_notice(ZLOG_VOIP, "%s:%s@%s:%d registrered successfully",
						osip->remote_param->username,
						osip->remote_param->userid, osip->address, osip->port);

		if (!osip->sip->bSipMain && osip->osip_policy.standby)
		{
			if (!osip->sip->bRegStanby)
			{
				osip->sip->bRegStanby = TRUE;
				x5b_app_register_status_api (NULL, 1, E_CMD_TO_AUTO);
			}
		}
		else if (osip->sip->bSipMain && !osip->osip_policy.standby)
		{
			if (!osip->sip->bRegMain)
			{
				osip->sip->bRegMain = TRUE;
				x5b_app_register_status_api (NULL, 1, E_CMD_TO_AUTO);
			}
		}
		osip_register_state_set (osip, SIP_STATE_REGISTER_SUCCESS);
	}
	if (res == EXOSIP_REGISTRATION_FAILURE)
	{
		if (status_code == -1)
		{
			OSIP_STATISTICS_REG_INC(osip, no_answer);
			OSIP_POLICY_REG_INC(osip, no_answer);
			osip->register_interval = REG_NO_ANSWER_INTERVAL;

			osip_register_state_set (osip, SIP_STATE_UNREGISTER);

			voip_osip_switch_policy_applay (osip, -1000);
			if (osip->sip->bRegMain || osip->sip->bRegStanby)
			{
				osip->sip->bRegMain = FALSE;
				osip->sip->bRegStanby = FALSE;
				x5b_app_register_status_api (NULL, 0, E_CMD_TO_AUTO);
			}
			if (OSIP_IS_DEBUG(EVENT) && OSIP_IS_DEBUG(DETAIL))
				zlog_notice(ZLOG_VOIP, "%s:%s@%s:%d registrered no respone",
							osip->remote_param->username,
							osip->remote_param->userid, osip->address,
							osip->port);
			return OK;
		}
		if (status_code == SIP_STATE_401)
		{
			//OSIP_STATISTICS_REG_INC(osip, fail);
			eXosip_clear_authentication_info (osip->context);
			voip_osip_context_username_api (osip);
		}
		if (osip->sip->bRegMain || osip->sip->bRegStanby)
		{
			osip->sip->bRegMain = FALSE;
			osip->sip->bRegStanby = FALSE;
			x5b_app_register_status_api (NULL, 0, E_CMD_TO_AUTO);
		}
		OSIP_STATISTICS_REG_INC(osip, fail);
		OSIP_POLICY_REG_INC(osip, fail);
		osip->register_interval = REG_FAIL_INTERVAL;
		voip_osip_switch_policy_applay (osip, -1000);
		if (OSIP_IS_DEBUG(EVENT))
			zlog_notice(ZLOG_VOIP, "%s:%s@%s:%d registrered fail",
						osip->remote_param->username,
						osip->remote_param->userid, osip->address, osip->port);

		osip_register_state_set (osip, SIP_STATE_REGISTER_FAILED);
	}
	return OK;
}

/*
 * incoming call callback handle
 */
static int
voip_osip_call_invite_handle (voip_osip_t *osip, eXosip_event_t *event,
							  osip_message_t **answer, int res)
{
	int ret = ERROR;
	int pos = 0, i = 0;
	char tmp[2048];
	char* audio_port = NULL;
	sdp_message_t *remote_sdp = NULL;
	sdp_media_t * audio_sdp = NULL;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	zassert(event != NULL);
	zassert(answer != NULL);
	zassert(osip->context != NULL);
	zassert(event->tid);

	ret = eXosip_call_build_answer (osip->context, event->tid, SIP_STATE_405,
									answer);

	if (ret != OSIP_SUCCESS || *answer == NULL)
	{
		zlog_err(ZLOG_VOIP, "Can not build answer for call invite");
		return ERROR;
	}
	ret = eXosip_call_send_answer (osip->context, event->tid, SIP_STATE_405,
								   *answer);
	return ERROR;
	osip_message_set_accept (*answer, "application/sdp");

	if (voip_sip_100_rel_get_api ())
		osip_message_set_supported(*answer, "100rel");

	//osip_message_set_supported (msg, "path, timer, replaces");
	osip_message_set_supported(*answer, "message-summary,ua-profile");
	//osip_message_get_allow (const osip_message_t * sip, int pos, osip_allow_t ** dest)
	{
	memset (tmp, 0, sizeof(tmp));
	voip_osip_sigtran_supported (osip, tmp, sizeof(tmp));
	zlog_err(ZLOG_VOIP, "==============osip_message_set_allow==============");
	if(osip_list_size (&(*answer)->allows))
	{
		osip_list_ofchar_free(&(*answer)->allows);
	}
	if (strlen (tmp))
		osip_message_set_allow (*answer, tmp);
	else
		osip_message_set_allow (
				*answer, "INVITE, ACK, OPTIONS, CANCEL, BYTE, INFO, MESSAGE");
	//osip_message_set_allow(*answer, "INVITE, ACK, OPTIONS, CANCEL, BYTE, SUBSCRIBE, NOTIFY, INFO, REFER, UPDATE, MESSAGE");
	}
	ret = eXosip_call_send_answer (osip->context, event->tid, SIP_STATE_180,
								   *answer);

	os_msleep (1);

	remote_sdp = eXosip_get_remote_sdp (osip->context, event->did);

	if (voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
											  event->tid, event->did,
											  OSIP_CALL_RUN) == OK)
	{
		osip->callparam->tid = event->tid;
		osip->callparam->cid = event->cid;
		osip->callparam->did = event->did;
		osip->callparam->state = OSIP_CALL_RUN;
	}
	ret = eXosip_call_build_answer (osip->context, event->tid, SIP_STATE_200,
									answer);
	if (ret != 0)
	{
		osip_log_wrapper(LOG_ERR, "failed to reject INVITE");
		return ERROR;
		//break;
	}
	memset (tmp, 0, sizeof(tmp));
	snprintf (tmp, sizeof(tmp), "v=0\r\n"
			  "o=anonymous 0 0 IN IP4 0.0.0.0\r\n"
			  "t=1 10\r\n");

	//设置回复的SDP消息体,下一步计划分析消息体
	//没有分析消息体，直接回复原来的消息，这一块做的不好。
	osip_message_set_body (*answer, tmp, strlen (tmp));
	osip_message_set_content_type (*answer, "application/sdp");

	ret = eXosip_call_send_answer (osip->context, event->tid, SIP_STATE_200,
								   *answer);

	audio_sdp = eXosip_get_audio_media (remote_sdp);

	if (!audio_sdp)
		return ERROR;

	audio_port = audio_sdp->m_port; //audio_port

	for (i = 0; i < audio_sdp->a_attributes.nb_elt; i++)
	{
		sdp_attribute_t *attr = (sdp_attribute_t*) osip_list_get (
				&audio_sdp->a_attributes, i);
		if (attr)
			printf ("%s : %s\n", attr->a_att_field, attr->a_att_value);
		else
			break;
	}

	while (!osip_list_eol (&(remote_sdp->a_attributes), pos))
	{
		sdp_attribute_t *at = NULL;
		at = (sdp_attribute_t *) osip_list_get (&remote_sdp->a_attributes, pos);
		if (at)
			printf ("%s : %s\n", at->a_att_field, at->a_att_value); //这里解释了为什么在SDP消息体中属性a里面存放必须是两列
		pos++;
	}
	while (!osip_list_eol (&(remote_sdp->m_medias), pos))
	{
		sdp_attribute_t *at = NULL;

		at = (sdp_attribute_t *) osip_list_get (&remote_sdp->m_medias, pos);
		if (at)
			printf ("%s : %s\n", at->a_att_field, at->a_att_value); //这里解释了为什么在SDP消息体中属性a里面存放必须是两列

		pos++;
	}
	if (ret != 0)
	{
		osip_log_wrapper(LOG_ERR, "failed to reject INVITE");
		return ERROR;
		//break;
	}
	//osip_log_wrapper(LOG_INFO, "INVITE rejected with 405");
	return OK;
}

/*
 * massage callback handle
 */
static int
voip_osip_massage_handle (voip_osip_t *osip, eXosip_event_t *event,
						  osip_message_t **answer, int res)
{
	int ret = 0;
	if (!osip || !event)
		return ERROR;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	//zassert(osip->callparam != NULL);
	zassert(event != NULL);
	zassert(answer != NULL);
	//zassert(event->request != NULL);
	//zassert(event->response != NULL);
	switch (event->type)
	{
		case EXOSIP_MESSAGE_NEW:
			_OSIP_DEBUG("=====%s: EXOSIP_MESSAGE_NEW", __func__);
			zassert(event->request != NULL);
			if (MSG_IS_MESSAGE(event->request)) //如果接受到的消息类型是MESSAGE
			{
				osip_body_t *body = NULL;
				osip_message_get_body (event->request, 0, &body);
				_OSIP_DEBUG( "=====%s: %s", __func__, body->body);
			}
			//按照规则，需要回复200 OK信息
			ret = eXosip_message_build_answer (osip->context, event->tid,
											   SIP_STATE_200, answer);
			if (ret != 0 || *answer == NULL)
			{
				osip_log_wrapper(LOG_ERR, "failed to reject %s",
								 event->request->sip_method);
				return ERROR;
				//break;
			}
			ret = eXosip_message_send_answer (osip->context, event->tid,
											  SIP_STATE_200, *answer);
			if (ret != 0)
			{
				osip_log_wrapper(LOG_ERR, "failed to reject %s",
								 event->request->sip_method);
				return ERROR;
				//break;
			}
			break;
		case EXOSIP_MESSAGE_PROCEEDING: /**< announce a 1xx for request. */
		case EXOSIP_MESSAGE_ANSWERED: /**< announce a 200ok  */
			zassert(event->request != NULL);
			if (event->request->sip_method)
				osip_log_wrapper(LOG_INFO, "%s rejected ",
								 event->request->sip_method);
			break;
		case EXOSIP_MESSAGE_REDIRECTED: /**< announce a failure. */
		case EXOSIP_MESSAGE_REQUESTFAILURE: /**< announce a failure. */
		case EXOSIP_MESSAGE_SERVERFAILURE: /**< announce a failure. */
		case EXOSIP_MESSAGE_GLOBALFAILURE: /**< announce a failure. */
			zassert(event->request != NULL);
			zassert(event->response != NULL);
			if (event->request->sip_method)
				osip_log_wrapper(LOG_INFO, "%s rejected ",
								 event->request->sip_method);
			osip_log_wrapper(
					LOG_ERR,
					"failure <- (%i %i) [%i %s]",
					event->cid,
					event->did,
					event->response->status_code,
					event->response->reason_phrase ?
							event->response->reason_phrase : "unknow");
			break;
		default:
			break;
	}
	return OK;
}

/*
 * insubscription callback handle
 */
static int
voip_osip_insubscription_new_handle (voip_osip_t *osip, eXosip_event_t *event,
									 osip_message_t **answer, int res)
{
	zassert(osip != NULL);
	zassert(event != NULL);
	zassert(answer != NULL);
	if (!osip || !event)
		return ERROR;
	return OK;
}

/*
 * dialing callback handle
 */
static int
voip_osip_call_dialing_handle (voip_osip_t *osip, eXosip_event_t *event,
							   osip_message_t **answer, int res)
{
	int ret = 0;
	if (!osip || !event)
		return ERROR;
	osip_message_t *ack = NULL;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	//zassert(osip->callparam != NULL);
	zassert(event != NULL);
	zassert(answer != NULL);
	//zassert(event->request != NULL);
	//zassert(event->response != NULL);

	switch (event->type)
	{
		/**< announce processing by a remote app   */
		case EXOSIP_CALL_PROCEEDING:
			//收到100 trying消息，表示请求正在处理中
			voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
												  event->tid, event->did,
												  OSIP_CALL_RUN);

			if (osip->remote_param && osip->callparam && OSIP_IS_DEBUG(EVENT))
				zlog_debug(ZLOG_VOIP, "%s:%s@%s:%d Trying(100) TO Call '%s'",
						   osip->remote_param->username,
						   osip->remote_param->userid, osip->address,
						   osip->port, osip->callparam->callnumber);

			osip_call_state_set (osip, SIP_STATE_CALL_TRYING);
			//osip_call_error_set(osip, SIP_STATE_100);
			break;
			/**< announce processing by a remote app   */
		case EXOSIP_CALL_RINGING:
			//收到180 Ringing应答，表示接收到INVITE请求的UAS正在向被叫用户振铃
			if (voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
													  event->tid, event->did,
													  OSIP_CALL_RUN) == OK
					&& osip->callparam)
			{
				osip->callparam->tid = event->tid;
				osip->callparam->cid = event->cid;
				osip->callparam->did = event->did;
				osip->callparam->state = OSIP_CALL_RUN;
				if (OSIP_IS_DEBUG(EVENT) && osip->callparam)
					zlog_debug(ZLOG_VOIP,
							   "'%s' Ringing(180), TID:%d CID:%d DID:%d",
							   osip->callparam->callnumber, event->tid,
							   event->cid, event->did);
			}
			else
			{
				if (OSIP_IS_DEBUG(EVENT) && osip->callparam)
					zlog_debug(ZLOG_VOIP, " Ringing(180), TID:%d CID:%d DID:%d",
							   event->tid, event->cid, event->did);
			}
			//osip_call_error_set(osip, SIP_STATE_180);
			osip_call_state_set (osip, SIP_STATE_CALL_RINGING);
			break;
		case EXOSIP_CALL_ANSWERED: //收到200 OK，表示请求已经被成功接受，用户应答
			zassert(event->response != NULL);
			{
				//printf("call_id is %d,dialog_id is %d \n", event->cid, event->did);
				if (voip_osip_call_update_instance_bycid (osip, event->cid,
														  NULL, event->tid,
														  event->did,
														  OSIP_CALL_TALK) == OK
						&& osip->callparam)
				{
					sdp_message_t * msg_rsp = NULL;
					sdp_connection_t * con_rsp = NULL;
					sdp_media_t * md_rsp = NULL;
					char *payload_str = NULL;

					osip->callparam->tid = event->tid;
					osip->callparam->cid = event->cid;
					osip->callparam->did = event->did;
					osip->callparam->state = OSIP_CALL_TALK;

					if (OSIP_IS_DEBUG(EVENT))
						zlog_debug(ZLOG_VOIP,
								   "'%s' Picking(200), TID:%d CID:%d DID:%d",
								   osip->callparam->callnumber,
								   osip->callparam->tid, osip->callparam->cid,
								   osip->callparam->did);
					//回送ack应答消息
					ret = eXosip_call_build_ack (osip->context, event->did,
												 &ack);
					if (ret != 0 || ack == NULL)
					{
						osip_log_wrapper(LOG_ERR, "failed to build ack %s",
										 event->request->sip_method);
						return ERROR;
					}
					eXosip_call_send_ack (osip->context, event->did, ack);

					/* 响应SIP消息中SDP分析 */
					msg_rsp = eXosip_get_sdp_info (event->response);
					if (msg_rsp)
					{
						con_rsp = eXosip_get_audio_connection (msg_rsp);
						md_rsp = eXosip_get_audio_media (msg_rsp);

						/* 取服务器支持的最优先的编码方式 */
						payload_str = (char *) osip_list_get (
								&md_rsp->m_payloads, 0);
					}
					//voip_stream_remote_address_port_api(voip_stream, con_rsp->c_addr, atoi(md_rsp->m_port));
					//voip_stream_payload_type_api(voip_stream, NULL, atoi(payload_str));
					if (msg_rsp && con_rsp && md_rsp && payload_str)
					{
						voip_stream_remote_t remote;
						if (OSIP_IS_DEBUG(EVENT))
							zlog_debug(ZLOG_VOIP, "%s@%s:%s %s",
									   osip->callparam->callnumber,
									   con_rsp->c_addr, (md_rsp->m_port),
									   (payload_str));

						memset (&remote, 0, sizeof(remote));
						remote.r_rtp_port = atoi (md_rsp->m_port);
						strcpy (remote.r_rtp_address, con_rsp->c_addr);
						remote.r_payload = atoi (payload_str);
						if (osip->media_cb.media_start_cb)
							(osip->media_cb.media_start_cb) (
									osip->media_cb.pVoidStart, &remote, 0, 0);

						//voip_event_high_add(voip_app_ev_start_stream, NULL, &remote, sizeof(voip_stream_remote_t), 0);
						//osip_call_error_set(osip, SIP_STATE_200);
						osip_call_state_set (osip, SIP_STATE_CALL_PICKING);
					}
					else
					{
						zlog_warn(ZLOG_VOIP,"msg_rsp, con_rsp, md_rsp, payload_str may be is invalid");

					}
				}
				else
				{
					if (OSIP_IS_DEBUG(EVENT))
						zlog_debug(
								ZLOG_VOIP,
								"request [%d: %s]",
								event->response->status_code,
								osip_message_get_reason (
										event->response->status_code));
				}
			}
			break;
		case EXOSIP_CALL_REDIRECTED: /**< announce a redirection                */
			zassert(event->response != NULL);
			_OSIP_DEBUG("EXOSIP_CALL_REDIRECTED\n");
			if (voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
													  event->tid, event->did,
													  OSIP_CALL_NONE) == OK
					&& osip->callparam)
			{
				//osip->callparam->callid = 0;
				/*			osip->callparam->tid = 0;
				 osip->callparam->cid = 0;
				 osip->callparam->did = 0;
				 osip->callparam->state = OSIP_CALL_NONE;
				 memset(osip->callparam->callnumber, 0, sizeof(osip->callparam->callnumber));*/
				voip_osip_call_delete_current_instance (osip,
														osip->callparam->cid);
			}
			if (OSIP_IS_DEBUG(EVENT))
				osip_log_wrapper(
						LOG_ERR, "request failure [%d: %s]",
						event->response->status_code,
						osip_message_get_reason (event->response->status_code));
			osip_call_error_set (osip, event->response->status_code);
			osip_call_state_set (osip, SIP_STATE_CALL_FAILED);
			break;

		case EXOSIP_CALL_REQUESTFAILURE: /**< announce a request failure            */
			zassert(event->response != NULL);
			_OSIP_DEBUG("EXOSIP_CALL_REQUESTFAILURE\n");
			if (voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
													  event->tid, event->did,
													  OSIP_CALL_NONE) == OK
					&& osip->callparam)
			{
				/*			osip->callparam->tid = 0;
				 osip->callparam->cid = 0;
				 osip->callparam->did = 0;
				 osip->callparam->state = OSIP_CALL_NONE;
				 memset(osip->callparam->callnumber, 0, sizeof(osip->callparam->callnumber));*/
				voip_osip_call_delete_current_instance (osip,
														osip->callparam->cid);
			}
			if (OSIP_IS_DEBUG(EVENT))
				osip_log_wrapper(
						LOG_ERR, "request failure [%d: %s]",
						event->response->status_code,
						osip_message_get_reason (event->response->status_code));
			osip_call_error_set (osip, event->response->status_code);
			osip_call_state_set (osip, SIP_STATE_CALL_FAILED);
			voip_osip_state_event_report (osip);
			break;

		case EXOSIP_CALL_SERVERFAILURE: /**< announce a server failure             */
			_OSIP_DEBUG("EXOSIP_CALL_SERVERFAILURE\n");
			zassert(event->response != NULL);
			if (voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
													  event->tid, event->did,
													  OSIP_CALL_NONE) == OK
					&& osip->callparam)
			{
				voip_osip_call_delete_current_instance (osip,
														osip->callparam->cid);
			}
			if (OSIP_IS_DEBUG(EVENT))
				osip_log_wrapper(
						LOG_ERR, "request failure [%d: %s]",
						event->response->status_code,
						osip_message_get_reason (event->response->status_code));
			osip_call_error_set (osip, event->response->status_code);
			osip_call_state_set (osip, SIP_STATE_CALL_FAILED);
			voip_osip_state_event_report (osip);
			break;

		case EXOSIP_CALL_GLOBALFAILURE: /**< announce a global failure             */
			_OSIP_DEBUG("EXOSIP_CALL_GLOBALFAILURE\n");
			zassert(event->response != NULL);
			if (voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
													  event->tid, event->did,
													  OSIP_CALL_NONE) == OK
					&& osip->callparam)
			{
				voip_osip_call_delete_current_instance (osip,
														osip->callparam->cid);
			}
			if (OSIP_IS_DEBUG(EVENT))
				osip_log_wrapper(
						LOG_ERR, "request failure [%d: %s]",
						event->response->status_code,
						osip_message_get_reason (event->response->status_code));
			osip_call_error_set (osip, event->response->status_code);
			osip_call_state_set (osip, SIP_STATE_CALL_FAILED);
			voip_osip_state_event_report (osip);
			break;
		case EXOSIP_CALL_ACK: //ACK received for 200ok to INVITE
			//zassert(event->response != NULL);
			/* 返回200后收到ack才建立媒体 */
			if (voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
													  event->tid, event->did,
													  OSIP_CALL_TALK) == OK
					&& osip->callparam)
			{
				osip->callparam->tid = event->tid;
				osip->callparam->cid = event->cid;
				osip->callparam->did = event->did;
				osip->callparam->state = OSIP_CALL_TALK;
				if (OSIP_IS_DEBUG(EVENT))
					zlog_debug(ZLOG_VOIP, "'%s' ACK(200)",
							   osip->callparam->callnumber);
			}
			//osip_call_error_set(osip, SIP_STATE_200);
			osip_call_state_set (osip, SIP_STATE_CALL_SUCCESS);
			break;
		case EXOSIP_CALL_CANCELLED: /**< announce that call has been cancelled */
			if (voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
													  event->tid, event->did,
													  OSIP_CALL_NONE) == OK
					&& osip->callparam)
			{
				if (OSIP_IS_DEBUG(EVENT))
					zlog_debug(ZLOG_VOIP, "'%s' Cancelled",
							   osip->callparam->callnumber);
				voip_osip_call_delete_current_instance (osip,
														osip->callparam->cid);
			}
			osip_call_state_set (osip, SIP_STATE_CALL_CANCELLED);
			break;
		case EXOSIP_CALL_CLOSED: //a BYE was received for this call
			if (voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
													  event->tid, event->did,
													  OSIP_CALL_NONE) == OK
					&& osip->callparam)
			{
				if (OSIP_IS_DEBUG(EVENT))
					zlog_debug(ZLOG_VOIP, "'%s' Hang UP Close",
							   osip->callparam->callnumber);

				if (eXosip_call_build_answer (osip->context, event->tid, 200,
											  answer) != 0)
				{
					if (OSIP_IS_DEBUG(EVENT))
						zlog_debug(
								ZLOG_VOIP,
								"This request msg is invalid! response '400'!");
					//printf ("This request msg is invalid!Cann't response!\n");
					eXosip_call_send_answer (osip->context, event->tid, 400,
											 NULL);
				}
				else
				{
					eXosip_call_send_answer (osip->context, event->tid, 200,
											 *answer);
				}
				if (osip->media_cb.media_stop_cb)
					(osip->media_cb.media_stop_cb) (osip->media_cb.pVoidStop,
													NULL, 0, 0);
				//voip_event_high_add(voip_app_ev_remote_stop_call, NULL, NULL, 0, 0);

				voip_osip_call_delete_current_instance (osip,
														osip->callparam->cid);
			}
			osip_call_state_set (osip, SIP_STATE_CALL_CLOSED);
			break;

			/* for both UAS & UAC events */
		case EXOSIP_CALL_RELEASED: /**< call context is cleared.            */

			if (voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
													  event->tid, event->did,
													  OSIP_CALL_NONE) == OK
					&& osip->callparam)
			{
				if (OSIP_IS_DEBUG(EVENT))
					zlog_debug(ZLOG_VOIP, "'%s' Released",
							   osip->callparam->callnumber);

				voip_osip_call_delete_current_instance (osip,
														osip->callparam->cid);
			}
			osip_call_state_set (osip, SIP_STATE_CALL_RELEASED);
			break;
		default: //收到其他应答
			if (OSIP_IS_DEBUG(EVENT))
				zlog_debug(ZLOG_VOIP, "other response, event type 0x%x",
						   event->type);
			_OSIP_DEBUG("other response!\n");
			break;
	}
	return OK;
}
/*
 * call message callback handle
 */
static int
voip_osip_call_message_handle (voip_osip_t *osip, eXosip_event_t *event,
							   osip_message_t **answer, int res)
{
	int ret = 0;
	osip_body_t *body = NULL;
	if (!osip || !event)
		return ERROR;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	//zassert(osip->callparam != NULL);
	zassert(event != NULL);
	zassert(answer != NULL);
	//zassert(event->request != NULL);
	//zassert(event->response != NULL);

	switch (event->type)
	{
		case EXOSIP_CALL_MESSAGE_NEW:
			zassert(event->request != NULL);
			voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
												  event->tid, event->did,
												  OSIP_CALL_NONE);
			if (osip->callparam)
			{
				if (MSG_IS_INFO(event->request)) //如果传输的是INFO方法
				{
					if (OSIP_IS_DEBUG(EVENT))
						zlog_debug(ZLOG_VOIP,
								   "Get Call message SIP INFO from '%s'",
								   osip->callparam->callnumber);

					ret = eXosip_call_build_answer (osip->context, event->tid,
													200, answer);
					if (ret != 0 || *answer == NULL)
					{
						zlog_err(ZLOG_VOIP, "failed to build answer %s",
								 event->request->sip_method);
						return ERROR;
					}
					eXosip_call_send_answer (osip->context, event->tid, 200,
											 *answer);
					//Signal=#
					//Duration=240
					osip_message_get_body (event->request, 0, &body);
					if (body && body->body)
					{
						if (strstr (body->body, "Signal="))
						{
							char *key = (char *) (body->body
									+ strlen ("Signal="));
							if (osip->sip && osip->sip->app_dtmf_cb)
								(osip->sip->app_dtmf_cb) (*key, 0);
							//voip_app_dtmf_command_execute(*key, 0);
						}
						if (OSIP_IS_DEBUG(MSG) && OSIP_IS_DEBUG(DETAIL))
							zlog_debug(
									ZLOG_VOIP,
									"Get Call message SIP INFO from '%s' : %s",
									osip->callparam->callnumber, body->body);
					}
				}
			}
			break;
		case EXOSIP_CALL_MESSAGE_PROCEEDING:
			_OSIP_DEBUG( "=====%s: EXOSIP_CALL_MESSAGE_PROCEEDING", __func__);
			break;
		case EXOSIP_CALL_MESSAGE_ANSWERED:
			if (voip_osip_call_update_instance_bycid (osip, event->cid, NULL,
													  event->tid, event->did,
													  OSIP_CALL_NONE) == OK
					&& osip->callparam)
			{
				if (OSIP_IS_DEBUG(EVENT))
					zlog_debug(ZLOG_VOIP, "'%s' Get Close ACK",
							   osip->callparam->callnumber);

				if (osip->media_cb.media_stop_cb)
					(osip->media_cb.media_stop_cb) (osip->media_cb.pVoidStop,
													NULL, 1, 0);
				//voip_event_high_add(voip_app_ev_stop_stream, NULL, NULL, 0, 0);

				voip_osip_call_delete_current_instance (osip,
														osip->callparam->cid);
			}

			break;
		case EXOSIP_CALL_MESSAGE_REDIRECTED:
			_OSIP_DEBUG( "=====%s: EXOSIP_CALL_MESSAGE_REDIRECTED", __func__);
			break;
		case EXOSIP_CALL_MESSAGE_REQUESTFAILURE:
			_OSIP_DEBUG( "=====%s: EXOSIP_CALL_MESSAGE_REQUESTFAILURE", __func__);
			break;
		case EXOSIP_CALL_MESSAGE_SERVERFAILURE:
			_OSIP_DEBUG( "=====%s: EXOSIP_CALL_MESSAGE_SERVERFAILURE", __func__);
			break;
		case EXOSIP_CALL_MESSAGE_GLOBALFAILURE:
			_OSIP_DEBUG( "=====%s: EXOSIP_CALL_MESSAGE_GLOBALFAILURE", __func__);
			break;
		default: //收到其他应答
			if (OSIP_IS_DEBUG(EVENT))
				zlog_debug(ZLOG_VOIP, "other response, event type 0x%x",
						   event->type);
			_OSIP_DEBUG( "other response!\n");
			break;
	}
	return OK;
}
/*
 * subscription callback handle
 */
static int
voip_osip_subscription_handle (voip_osip_t *osip, eXosip_event_t *event,
							   osip_message_t **answer, int res)
{
	if (!osip || !event)
		return ERROR;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	//zassert(osip->callparam != NULL);
	zassert(event != NULL);
	zassert(answer != NULL);
	//osip_message_t *ack = NULL;
	switch (event->type)
	{
		case EXOSIP_SUBSCRIPTION_NOANSWER:
			_OSIP_DEBUG( "=====%s: EXOSIP_SUBSCRIPTION_NOANSWER", __func__);
			break;
		case EXOSIP_SUBSCRIPTION_PROCEEDING:
			_OSIP_DEBUG( "=====%s: EXOSIP_SUBSCRIPTION_PROCEEDING", __func__);
			break;
		case EXOSIP_SUBSCRIPTION_ANSWERED:
			_OSIP_DEBUG( "=====%s: EXOSIP_SUBSCRIPTION_ANSWERED", __func__);
			break;
		case EXOSIP_SUBSCRIPTION_REDIRECTED:
			_OSIP_DEBUG( "=====%s: EXOSIP_SUBSCRIPTION_REDIRECTED", __func__);
			break;
		case EXOSIP_SUBSCRIPTION_REQUESTFAILURE:
			_OSIP_DEBUG( "=====%s: EXOSIP_SUBSCRIPTION_REQUESTFAILURE", __func__);
			break;
		case EXOSIP_SUBSCRIPTION_SERVERFAILURE:
			_OSIP_DEBUG( "=====%s: EXOSIP_SUBSCRIPTION_SERVERFAILURE", __func__);
			break;
		case EXOSIP_SUBSCRIPTION_GLOBALFAILURE:
			_OSIP_DEBUG( "=====%s: EXOSIP_SUBSCRIPTION_GLOBALFAILURE", __func__);
			break;
		case EXOSIP_SUBSCRIPTION_NOTIFY:
			_OSIP_DEBUG( "=====%s: EXOSIP_SUBSCRIPTION_NOTIFY", __func__);
			break;
		default: //收到其他应答
			if (OSIP_IS_DEBUG(EVENT))
				zlog_debug(ZLOG_VOIP, "other response, event type 0x%x",
						   event->type);
			_OSIP_DEBUG( "other response!\n");
			break;
	}
	return OK;
}
/*
 * notification callback handle
 */
static int
voip_osip_notification_handle (voip_osip_t *osip, eXosip_event_t *event,
							   osip_message_t **answer, int res)
{
	if (!osip || !event)
		return ERROR;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	//zassert(osip->callparam != NULL);
	zassert(event != NULL);
	zassert(answer != NULL);
	//osip_message_t *ack = NULL;
	switch (event->type)
	{
		case EXOSIP_NOTIFICATION_NOANSWER:
			_OSIP_DEBUG( "=====%s: EXOSIP_NOTIFICATION_NOANSWER", __func__);
			break;
		case EXOSIP_NOTIFICATION_PROCEEDING:
			_OSIP_DEBUG( "=====%s: EXOSIP_NOTIFICATION_PROCEEDING", __func__);
			break;
		case EXOSIP_NOTIFICATION_ANSWERED:
			_OSIP_DEBUG( "=====%s: EXOSIP_NOTIFICATION_ANSWERED", __func__);
			break;
		case EXOSIP_NOTIFICATION_REDIRECTED:
			_OSIP_DEBUG( "=====%s: EXOSIP_NOTIFICATION_REDIRECTED", __func__);
			break;
		case EXOSIP_NOTIFICATION_REQUESTFAILURE:
			_OSIP_DEBUG( "=====%s: EXOSIP_NOTIFICATION_REQUESTFAILURE", __func__);
			break;
		case EXOSIP_NOTIFICATION_SERVERFAILURE:
			_OSIP_DEBUG( "=====%s: EXOSIP_NOTIFICATION_SERVERFAILURE", __func__);
			break;
		case EXOSIP_NOTIFICATION_GLOBALFAILURE:
			_OSIP_DEBUG( "=====%s: EXOSIP_NOTIFICATION_GLOBALFAILURE", __func__);
			break;
		default: //收到其他应答
			if (OSIP_IS_DEBUG(EVENT))
				zlog_debug(ZLOG_VOIP, "other response, event type 0x%x",
						   event->type);
			_OSIP_DEBUG( "other response!\n");
			break;
	}
	return OK;
}

static int
voip_osip_task (void *pVoid)
{
	//int i;
	int counter = 0;
	voip_osip_t *osip = (voip_osip_t *) pVoid;
	if (!osip)
		return ERROR;
	zassert(osip != NULL);
	eXosip_event_t *event = NULL;
	osip_message_t *answer = NULL;

	while (!os_load_config_done ())
	{
		os_sleep (1);
	}
	while (osip->enable == FALSE)
	{
		os_sleep (1);
	}
	os_sleep (1);
	while (1)
	{
		voip_osip_lock(osip);
		eXosip_lock (osip->context);
		if (osip->initialization != TRUE)
		{
			eXosip_unlock (osip->context);
			voip_osip_unlock(osip);
			os_sleep (1);
			continue;
		}
		else
		{
			eXosip_unlock (osip->context);
			voip_osip_unlock(osip);
			osip_usleep (10000);
		}

		voip_osip_lock(osip);
#ifdef OSIP_SELF_CB_ENABLE
		voip_osip_self_callback_process (osip);
#endif /* OSIP_SELF_CB_ENABLE */
		voip_osip_unlock(osip);

		//_OSIP_DEBUG( "=====%s:eXosip_event_wait");
		if (!(event = eXosip_event_wait (osip->context, 1, 500)))
		{
#ifdef OSIP_MONOTHREAD
			eXosip_execute (osip->context);
#endif
			eXosip_automatic_action (osip->context);
			//voip_osip_unlock(osip);
			osip_usleep (10000);
			continue;
		}
#ifdef OSIP_MONOTHREAD
		eXosip_execute (osip->context);
#endif

		if (osip->initialization != TRUE)
		{
			os_sleep (1);
			continue;
		}
		voip_osip_lock(osip);
		eXosip_lock (osip->context);
		eXosip_automatic_action (osip->context);

		switch (event->type)
		{
			/* REGISTER related events */
			/**< user is successfully registred.  */
			/**< user is not registred.           */
			case EXOSIP_REGISTRATION_SUCCESS:
			case EXOSIP_REGISTRATION_FAILURE:
				if (event->response)
					voip_osip_register_handle (osip, event->type,
											   event->response->status_code);
				else
					voip_osip_register_handle (osip, event->type, -1);
				break;

				/* INVITE related events within calls */
				/**< announce a new call                   */
			case EXOSIP_CALL_INVITE:
				/**< announce a new INVITE within call     */
			case EXOSIP_CALL_REINVITE:
			{
				voip_osip_call_invite_handle (osip, event, &answer, 0);
				break;
			}

				/**< announce processing by a remote app   */
			case EXOSIP_CALL_NOANSWER: /**< announce no answer within the timeout */
			case EXOSIP_CALL_PROCEEDING: /**< announce processing by a remote app   */
			case EXOSIP_CALL_RINGING: /**< announce ringback                     */
			case EXOSIP_CALL_ANSWERED: /**< announce start of call                */
			case EXOSIP_CALL_REDIRECTED: /**< announce a redirection                */
			case EXOSIP_CALL_REQUESTFAILURE: /**< announce a request failure            */
			case EXOSIP_CALL_SERVERFAILURE: /**< announce a server failure             */
			case EXOSIP_CALL_GLOBALFAILURE: /**< announce a global failure             */
			case EXOSIP_CALL_ACK: /**< ACK received for 200ok to INVITE      */
			case EXOSIP_CALL_CANCELLED: /**< announce that call has been cancelled */
			case EXOSIP_CALL_CLOSED: /**< a BYE was received for this call      */
				/* for both UAS & UAC events */
			case EXOSIP_CALL_RELEASED: /**< call context is cleared.            */
			{
				voip_osip_call_dialing_handle (osip, event, &answer, 0);
				break;
			}
				/* request related events within calls (except INVITE) */
				/* request related events within calls (except INVITE) */
			case EXOSIP_CALL_MESSAGE_NEW: /**< announce new incoming request. */
			case EXOSIP_CALL_MESSAGE_PROCEEDING: /**< announce a 1xx for request. */
			case EXOSIP_CALL_MESSAGE_ANSWERED: /**< announce a 200ok  */
			case EXOSIP_CALL_MESSAGE_REDIRECTED: /**< announce a failure. */
			case EXOSIP_CALL_MESSAGE_REQUESTFAILURE: /**< announce a failure. */
			case EXOSIP_CALL_MESSAGE_SERVERFAILURE: /**< announce a failure. */
			case EXOSIP_CALL_MESSAGE_GLOBALFAILURE: /**< announce a failure. */
			{
				voip_osip_call_message_handle (osip, event, &answer, 0);
				break;
			}

				/* events received for request outside calls */
				/**< announce new incoming request. */
			case EXOSIP_MESSAGE_NEW:
			case EXOSIP_MESSAGE_PROCEEDING: /**< announce a 1xx for request. */
			case EXOSIP_MESSAGE_ANSWERED: /**< announce a 200ok  */
			case EXOSIP_MESSAGE_REDIRECTED: /**< announce a failure. */
			case EXOSIP_MESSAGE_REQUESTFAILURE: /**< announce a failure. */
			case EXOSIP_MESSAGE_SERVERFAILURE: /**< announce a failure. */
			case EXOSIP_MESSAGE_GLOBALFAILURE: /**< announce a failure. */
			{
				voip_osip_massage_handle (osip, event, &answer, 0);
				break;
			}
				break;
				/* Presence and Instant Messaging */
			case EXOSIP_SUBSCRIPTION_NOANSWER: /**< announce no answer              */
			case EXOSIP_SUBSCRIPTION_PROCEEDING: /**< announce a 1xx                  */
			case EXOSIP_SUBSCRIPTION_ANSWERED: /**< announce a 200ok                */
			case EXOSIP_SUBSCRIPTION_REDIRECTED: /**< announce a redirection          */
			case EXOSIP_SUBSCRIPTION_REQUESTFAILURE: /**< announce a request failure      */
			case EXOSIP_SUBSCRIPTION_SERVERFAILURE: /**< announce a server failure       */
			case EXOSIP_SUBSCRIPTION_GLOBALFAILURE: /**< announce a global failure       */
			case EXOSIP_SUBSCRIPTION_NOTIFY: /**< announce new NOTIFY request     */
				voip_osip_subscription_handle (osip, event, &answer, 0);
				break;
				/**< announce new incoming SUBSCRIBE/REFER.*/
			case EXOSIP_IN_SUBSCRIPTION_NEW:
			{
				voip_osip_insubscription_new_handle (osip, event, &answer, 0);
				break;
			}
			case EXOSIP_NOTIFICATION_NOANSWER: /**< announce no answer              */
			case EXOSIP_NOTIFICATION_PROCEEDING: /**< announce a 1xx                  */
			case EXOSIP_NOTIFICATION_ANSWERED: /**< announce a 200ok                */
			case EXOSIP_NOTIFICATION_REDIRECTED: /**< announce a redirection          */
			case EXOSIP_NOTIFICATION_REQUESTFAILURE: /**< announce a request failure      */
			case EXOSIP_NOTIFICATION_SERVERFAILURE: /**< announce a server failure       */
			case EXOSIP_NOTIFICATION_GLOBALFAILURE: /**< announce a global failure       */
				voip_osip_notification_handle (osip, event, &answer, 0);
				break;
			default:
				osip_log_wrapper(
						LOG_DEBUG,
						"recieved unknown eXosip event (type, did, cid) = (%d, %d, %d)",
						event->type, event->did, event->cid);

		}

		eXosip_unlock (osip->context);
		eXosip_event_free (event);

		voip_osip_stats_handle (osip, &counter);
		counter++;
		voip_osip_unlock(osip);
	}
	return OK;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/
/*
 * frist register api
 */
static int
_voip_osip_register_init_handle (voip_osip_t *osip)
{
	char sigtran[1024];
	osip_message_t *msg = NULL;
	int ret = 0;
	if (!osip)
		return ERROR;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	voip_osip_proxy_build (osip);
	voip_osip_fromuser_build (osip);
	voip_osip_contact_build (osip);

	osip->remote_param->regparam.regid =
			eXosip_register_build_initial_register (
					osip->context, osip->remote_param->fromuser,
					osip->remote_param->proxy, NULL/*osip->contact*/,
					osip->remote_param->regparam.expiry, &msg);

	if (msg == NULL || osip->remote_param->regparam.regid < 1)
	{
		osip_log_wrapper(LOG_ERR, "eXosip build initial failed(%s)",
						 osip_strerror (ret));
		return ERROR;
	}

	voip_osip_config_debug_detail (osip);

	//osip_log_wrapper(LOG_ERR,
	//				"_voip_osip_register_init_handle regid=%d",osip->remote_param->regparam.regid);

	//osip_message_set_supported (msg, "path");
	memset (sigtran, 0, sizeof(sigtran));
	voip_osip_sigtran_supported (osip, sigtran, sizeof(sigtran));
	zlog_err(ZLOG_VOIP, "==============osip_message_set_allow==============");
	if(osip_list_size (&msg->allows))
	{
		osip_list_ofchar_free(&msg->allows);
	}
	if (strlen (sigtran))
		ret = osip_message_set_allow (msg, sigtran);
	else
		ret = osip_message_set_allow (
				msg, "INVITE, ACK, OPTIONS, CANCEL, BYTE, INFO, MESSAGE");
	//ret = osip_message_set_allow(msg, "INVITE, ACK, OPTIONS, CANCEL, BYTE, SUBSCRIBE, NOTIFY, INFO, REFER, UPDATE, MESSAGE");
	if (ret != OSIP_SUCCESS)
	{
		zlog_err(ZLOG_VOIP, "eXosip message set allow failed(%s)",
				 osip_strerror (ret));
		return ERROR;
	}
	ret = eXosip_register_send_register (osip->context,
										 osip->remote_param->regparam.regid,
										 msg);
	if (ret != OSIP_SUCCESS)
	{
		zlog_err(ZLOG_VOIP, "eXosip send register message failed(%s)",
				 osip_strerror (ret));
		voip_osip_switch_policy_applay (osip, ret);
		return ERROR;
	}
	return OK;
}

/*
 * register api
 */
static int
voip_osip_register_start_handle (voip_osip_t *osip)
{
	char sigtran[1024];
	osip_message_t *msg = NULL;
	int ret = 0;
	if (!osip)
		return ERROR;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	//zassert(osip->callparam != NULL);

	if (osip->remote_param->regparam.regid < 1)
	{
		osip_log_wrapper(LOG_ERR, "eXosip register ID is error(id=%d)");
		return ERROR;
	}

	ret = eXosip_register_build_register (osip->context,
										  osip->remote_param->regparam.regid,
										  osip->remote_param->regparam.expiry,
										  &msg);

	if (ret != 0 || msg == NULL)
	{
		zlog_err(ZLOG_VOIP, "eXosip build register failed(%s)",
				 osip_strerror (ret));
		return ERROR;
	}
	//osip_message_set_supported (msg, "path");

	memset (sigtran, 0, sizeof(sigtran));
	voip_osip_sigtran_supported (osip, sigtran, sizeof(sigtran));
	zlog_err(ZLOG_VOIP, "==============osip_message_set_allow==============");
	if(osip_list_size (&msg->allows))
	{
		osip_list_ofchar_free(&msg->allows);
	}
	if (strlen (sigtran))
		ret = osip_message_set_allow (msg, sigtran);
	else
		ret = osip_message_set_allow (
				msg, "INVITE, ACK, OPTIONS, CANCEL, BYTE, INFO, MESSAGE");
	//ret = osip_message_set_allow(msg, "INVITE, ACK, OPTIONS, CANCEL, BYTE, SUBSCRIBE, NOTIFY, INFO, REFER, UPDATE, MESSAGE");
	if (ret != OSIP_SUCCESS)
	{
		zlog_err(ZLOG_VOIP, "eXosip message set allow failed(%s)",
				 osip_strerror (ret));
		return ERROR;
	}
	ret = eXosip_register_send_register (osip->context,
										 osip->remote_param->regparam.regid,
										 msg);
	if (ret != OSIP_SUCCESS)
	{
		voip_osip_switch_policy_applay (osip, ret);
		zlog_err(ZLOG_VOIP, "eXosip send register message failed(%s)",
				 osip_strerror (ret));
		return ERROR;
	}
	return OK;
}

/*
 * unregister api
 */
static int
voip_osip_unregister_handle (voip_osip_t *osip)
{
	osip_message_t *msg = NULL;
	int ret = 0;
	if (!osip)
		return ERROR;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	ret = eXosip_register_build_register (osip->context,
										  osip->remote_param->regparam.regid, 0,
										  &msg);

	if (ret != 0 || msg == NULL)
	{
		zlog_err(ZLOG_VOIP, "eXosip build register failed(%s)",
				 osip_strerror (ret));
		return ERROR;
	}
	ret = eXosip_register_send_register (osip->context,
										 osip->remote_param->regparam.regid,
										 msg);
	if (ret != 0)
	{
		osip_log_wrapper(LOG_ERR, "eXosip send register failed(%s)",
						 osip_strerror (ret));
		return ERROR;
	}
	osip->register_interval = REG_NO_ANSWER_INTERVAL;
	osip_register_state_set (osip, SIP_STATE_UNREGISTER);
	if (osip->r_event)
		voip_event_cancel(osip->r_event);
	osip->remote_param->regparam.regid = 0;
	return OK;
}

int
voip_osip_register_initialization_api (voip_osip_t *osip)
{
	int ret = 0;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	voip_osip_lock(osip);
	ret = _voip_osip_register_init_handle (osip);
	voip_osip_unlock(osip);
	return ret;
}

/*
 * register api
 */
int
voip_osip_register_api (voip_osip_t *osip)
{
	int ret = 0;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	//zassert(osip->callparam != NULL);
	voip_osip_lock(osip);
	ret = voip_osip_register_start_handle (osip);
	voip_osip_unlock(osip);
	return ret;
}

/*
 * unregister api
 */
int
voip_osip_unregister_api (voip_osip_t *osip)
{
	int ret = 0;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	voip_osip_lock(osip);
	ret = voip_osip_unregister_handle (osip);
	voip_osip_unlock(osip);
	return ret;
}

/*
 * start call api
 */
int
voip_osip_call_start_api (voip_osip_t *osip, char *username, char *phonenumber,
						  int *instance)
{
	char tmp[4096];
	osip_message_t *msg = NULL;
	int ret = 0, cid = 0;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	//zassert(osip->callparam != NULL);
	voip_osip_lock(osip);
	//char *dest_call = "<sip:to@antisip.com>"; //UAS,端口是15061
	//char *source_call = "<sip:%s@%s>"; //UAC1，端口是15060
	char dest_call[128]; //UAS,端口是15061
	char source_call[128]; //UAC1，端口是15060

	memset (source_call, 0, sizeof(source_call));
	memset (dest_call, 0, sizeof(dest_call));

	voip_osip_call_source_build (osip, source_call, sizeof(source_call));
	voip_osip_call_destination_build (osip, username, phonenumber, dest_call,
									  sizeof(dest_call));

	ret = eXosip_call_build_initial_invite (osip->context, &msg, dest_call,
											source_call, NULL, NULL);

	if (ret != OSIP_SUCCESS || msg == NULL)
	{
		zlog_err(ZLOG_VOIP, "eXosip build call initial failed(%s)",
				 osip_strerror (ret));
		voip_osip_unlock(osip);
		return ERROR;
	}
	if (voip_sip_100_rel_get_api ())
	{
		ret = osip_message_set_supported(msg, "100rel");
		if (ret != OSIP_SUCCESS)
		{
			zlog_err(ZLOG_VOIP, "eXosip message supported '100rel' failed(%s)",
					 osip_strerror (ret));
			voip_osip_unlock(osip);
			return ERROR;
		}
	}
	//osip_message_set_supported (msg, "path, timer, replaces");
	ret = osip_message_set_supported(msg, "message-summary,ua-profile");
	if (ret != OSIP_SUCCESS)
	{
		zlog_err(
				ZLOG_VOIP,
				"eXosip message supported 'message-summary,ua-profile' failed(%s)",
				osip_strerror (ret));
		voip_osip_unlock(osip);
		return ERROR;
	}
	memset (tmp, 0, sizeof(tmp));
	voip_osip_sigtran_supported (osip, tmp, sizeof(tmp));
	zlog_err(ZLOG_VOIP, "==============osip_message_set_allow==============");
	if(osip_list_size (&msg->allows))
	{
		osip_list_ofchar_free(&msg->allows);
	}
	if (strlen (tmp))
		ret = osip_message_set_allow (msg, tmp);
	else
		ret = osip_message_set_allow (
				msg, "INVITE, ACK, OPTIONS, CANCEL, BYTE, INFO, MESSAGE");
	//ret = osip_message_set_allow(msg, "INVITE, ACK, OPTIONS, CANCEL, BYTE, SUBSCRIBE, NOTIFY, INFO, REFER, UPDATE, MESSAGE");

	if (ret != OSIP_SUCCESS)
	{
		zlog_err(ZLOG_VOIP, "eXosip message allow failed(%s)",
				 osip_strerror (ret));
		voip_osip_unlock(osip);
		return ERROR;
	}
	//osip_message_set_allow(msg, "INVITE, ACK, OPTIONS, CANCEL, BYTE, SUBSCRIBE, NOTIFY, INFO, REFER, UPDATE, MESSAGE");

	// 格式化SDP信息体
	memset (tmp, 0, sizeof(tmp));
	voip_osip_call_sdp_build (osip, username, phonenumber, tmp, sizeof(tmp));

	ret = osip_message_set_body (msg, tmp, strlen (tmp));
	if (ret != OSIP_SUCCESS)
	{
		zlog_err(ZLOG_VOIP, "eXosip message body failed(%s)",
				 osip_strerror (ret));
		voip_osip_unlock(osip);
		return ERROR;
	}
	ret = osip_message_set_content_type (msg, "application/sdp");
	if (ret != OSIP_SUCCESS)
	{
		zlog_err(ZLOG_VOIP,
				 "eXosip message content 'application/sdp' failed(%s)",
				 osip_strerror (ret));
		voip_osip_unlock(osip);
		return ERROR;
	}
	ret = eXosip_call_send_initial_invite (osip->context, msg); //invite SIP INVITE message to send
	if (ret < OSIP_SUCCESS)
	{
		zlog_err(ZLOG_VOIP, "eXosip send call inital message failed(%s)",
				 osip_strerror (ret));
		voip_osip_unlock(osip);
		return ERROR;
	}
	cid = ret;
	ret = eXosip_call_set_reference (osip->context, ret, "000");
	if (ret != OSIP_SUCCESS)
	{
		zlog_err(ZLOG_VOIP, "eXosip call set reference '000' failed(%s)",
				 osip_strerror (ret));
		voip_osip_unlock(osip);
		return ERROR;
	}
	ret = voip_osip_call_create_instance (osip, phonenumber, cid);
	if (ret != OK)
	{
		zlog_err(ZLOG_VOIP, "Initial CALL Instance!");
		voip_osip_unlock(osip);
		return ERROR;
	}
	if (instance && osip->callparam)
		*instance = osip->callparam->cid;
	voip_osip_unlock(osip);
	return OK;
}

/*
 * stop call api
 */
int
voip_osip_call_stop_api (voip_osip_t *osip, int instance)
{
	if (!osip)
		return ERROR;
	zassert(osip != NULL);
	zassert(osip->remote_param != NULL);
	//zassert(osip->callparam != NULL);
	osip->callparam = voip_osip_call_lookup_instance_bycid (osip, instance);
	voip_osip_lock(osip);
	if (osip->callparam == NULL)
	{
		zlog_err(ZLOG_VOIP, "failed to find call instance(%d)", instance);
		voip_osip_unlock(osip);
		return ERROR;
	}
	if (osip->callparam->cid <= 0 || osip->callparam->did <= 0)
	{
		zlog_err(ZLOG_VOIP, "failed to terminate call");
		voip_osip_unlock(osip);
		return ERROR;
	}
	if (osip->callparam && osip->callparam->cid > 0 && osip->callparam->did > 0)
	{
		int ret = 0;
		if ((ret = eXosip_call_terminate (osip->context, osip->callparam->cid,
										  osip->callparam->did)) < 0)
		{
			zlog_err(ZLOG_VOIP, "eXosip terminate call failed(%s)",
					 osip_strerror (ret));
			voip_osip_unlock(osip);
			return ERROR;
		}
		if (OSIP_IS_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "eXosip terminate call '%s'",
					   osip->callparam->callnumber);
	}
	voip_osip_unlock(osip);
	return OK;
}

/*
 * send SIP INFO key
 */
int
voip_osip_request_info_api (voip_osip_t *osip, char key)
{
	zassert(osip != NULL);
	voip_osip_lock(osip);
	voip_osip_unlock(osip);
	return OK;
}

#ifdef OSIP_SELF_CB_ENABLE
static int
_voip_osip_register_cb (void *event)
{
	if (!event)
		return ERROR;
	zassert(event != NULL);
	voip_osip_t *voiposip = ((voip_event_t *) (event));
	zassert(voiposip != NULL);
	zassert(voiposip->remote_param != NULL);
	eXosip_lock (voiposip->context);
	voiposip->r_event = NULL;

	/*
	 if (OSIP_IS_DEBUG(EVENT))
	 zlog_debug(ZLOG_VOIP, "OSIP register event regid=%d", voiposip->remote_param->regparam.regid);
	 */

	if (voiposip->remote_param->regparam.regid >= 1)
		voip_osip_register_start_handle (voiposip);
	else
		_voip_osip_register_init_handle (voiposip);
	//voiposip->r_event = voip_event_timer_add(_voip_osip_register_event, voip_osip, NULL, 0, voip_osip->register_interval);
	eXosip_unlock (voiposip->context);
	return OK;
}
#endif /* OSIP_SELF_CB_ENABLE */

static int
_voip_osip_register_event (void *event)
{
	if (!event)
		return ERROR;
	zassert(event != NULL);
	voip_osip_t *voiposip = VOIP_EVENT_ARGV(event);
	//voip_osip_t *voiposip = ((voip_event_t *)(event))->pVoid;
	zassert(voiposip != NULL);
	voip_osip_unlock(voiposip);
	zassert(voiposip->remote_param != NULL);

#ifdef OSIP_SELF_CB_ENABLE
	eXosip_lock (voiposip->context);
	voiposip->r_event = NULL;
	voip_osip_self_callback_add (voiposip, _voip_osip_register_cb, voiposip);
	//zlog_err(ZLOG_VOIP, "===================%s: %s", __func__, "_voip_osip_register_event");
	voiposip->r_event = voip_event_timer_add(_voip_osip_register_event,
											 voip_osip, NULL, 0,
											 voip_osip->register_interval);
	eXosip_unlock (voiposip->context);
#else
	eXosip_lock(voiposip->context);
	voiposip->r_event = NULL;
	if (OSIP_IS_DEBUG(EVENT))
	zlog_debug(ZLOG_VOIP, "OSIP register event regid=%d", voiposip->remote_param->regparam.regid);

	if(voiposip->remote_param->regparam.regid >= 1)
	voip_osip_register_start_handle(voiposip);
	else
	_voip_osip_register_init_handle(voiposip);
	//zlog_err(ZLOG_VOIP, "===================%s: %s", __func__, "_voip_osip_register_event");
	voiposip->r_event = voip_event_timer_add(_voip_osip_register_event, voip_osip, NULL, 0, voip_osip->register_interval);
	eXosip_unlock(voiposip->context);
#endif /* OSIP_SELF_CB_ENABLE */
	voip_osip_unlock(voiposip);
	return OK;
}

static int
_voip_osip_reset_event (void *event)
{
	if (!event)
		return ERROR;
	zassert(event != NULL);
	voip_osip_t *voiposip = VOIP_EVENT_ARGV(event);
	//voip_osip_t *voiposip = ((voip_event_t *)(event))->pVoid;
	zassert(voiposip != NULL);
	zassert(voiposip->remote_param != NULL);

	voip_osip_lock(voiposip);

	voiposip->r_reset = NULL;

	eXosip_lock (voiposip->context);

	if (voiposip->r_event)
		voip_event_cancel(voiposip->r_event);
	if (voiposip->r_init)
		voip_event_cancel(voiposip->r_init);
	if (voiposip)
	{
		if (OSIP_IS_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "OSIP reset");
		//eXosip_lock(voiposip->context);

		voip_osip_unregister_handle (voiposip);

		voip_osip_config_reload (voiposip);

		osip_register_state_set (voiposip, SIP_STATE_UNREGISTER);

		if (voip_osip_config_active (voiposip) == OSIP_ACTIVE)
		{
			eXosip_free_transports (voiposip->context);

			voip_osip_context_init_api (voiposip);

			voiposip->initialization = TRUE;
			if (voiposip->remote_param->regparam.regid >= 1)
				voip_osip_register_start_handle (voiposip);
			else
				_voip_osip_register_init_handle (voiposip);
			if (voiposip->r_event)
				voip_event_cancel(voiposip->r_event);
			//zlog_err(ZLOG_VOIP, "===================%s: %s", __func__, "_voip_osip_register_event");
			voiposip->r_event = voip_event_timer_add(
					_voip_osip_register_event, voip_osip, NULL, 0,
					voip_osip->register_interval);

		}
		eXosip_unlock (voiposip->context);
		//_OSIP_DEBUG( "=====%s: out", __func__);
	}
	voip_osip_unlock(voiposip);
	return OK;
}

static int
_voip_osip_initialization_event (void *event)
{
	if (!event)
		return ERROR;
	zassert(event != NULL);
	voip_osip_t *voiposip = VOIP_EVENT_ARGV(event);
	//voip_osip_t *voiposip = ((voip_event_t *)(event))->pVoid;
	zassert(voiposip != NULL);
	zassert(voiposip->remote_param != NULL);
	voip_osip_lock(voiposip);
	voiposip->r_init = NULL;
	if (voiposip)
	{
		if (voiposip->initialization == TRUE)
		{
			voip_osip_unlock(voiposip);
			return OK;
		}
		if (OSIP_IS_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "OSIP register initialization");
		eXosip_lock (voiposip->context);

		if (voip_osip->r_event)
			voip_event_cancel(voip_osip->r_event);
		if (voip_osip->r_init)
			voip_event_cancel(voip_osip->r_init);
		if (voip_osip->r_reset)
			voip_event_cancel(voip_osip->r_reset);

		voip_osip_config_reload (voiposip);
		osip_register_state_set (voiposip, SIP_STATE_UNREGISTER);
		if (voip_osip_config_active (voiposip) == OSIP_ACTIVE)
		{
			voip_osip_context_init_api (voiposip);
			//voip_osip_context_username_api(voiposip);
			voiposip->enable = TRUE;

			//_OSIP_DEBUG( "=====%s: enable", __func__);
			if (voiposip->remote_param->regparam.regid >= 1)
				voip_osip_register_start_handle (voiposip);
			else
				_voip_osip_register_init_handle (voiposip);
			/*			if(voiposip->r_event)
			 voip_event_cancel(voiposip->r_event);*/
			//zlog_err(ZLOG_VOIP, "===================%s: %s", __func__, "_voip_osip_register_event");
			voiposip->r_event = voip_event_timer_add(
					_voip_osip_register_event, voip_osip, NULL, 0,
					voiposip->register_interval);

		}
		eXosip_unlock (voiposip->context);
		//_OSIP_DEBUG( "=====%s: out", __func__);
	}
	voip_osip_unlock(voiposip);
	return OK;
}

int
voip_osip_initialization (void)
{
	zassert(voip_osip != NULL);
	zassert(voip_osip->remote_param != NULL);
	voip_osip_lock(voip_osip);
	if (voip_osip)
	{
		eXosip_lock (voip_osip->context);

		if (voip_osip->r_event)
			voip_event_cancel(voip_osip->r_event);
		if (voip_osip->r_init)
			voip_event_cancel(voip_osip->r_init);
		if (voip_osip->r_reset)
			voip_event_cancel(voip_osip->r_reset);

		if (voip_osip->initialization == TRUE)
		{
			if (voip_osip->r_reset)
				voip_event_cancel(voip_osip->r_reset);
			voip_osip->r_reset = voip_event_ready_add(_voip_osip_reset_event,
													  voip_osip, NULL, 0, 0);
			eXosip_unlock (voip_osip->context);
			voip_osip_unlock(voip_osip);
			return OK;
		}
		if (voip_osip->r_init)
			voip_event_cancel(voip_osip->r_init);
		voip_osip->r_init = voip_event_ready_add(
				_voip_osip_initialization_event, voip_osip, NULL, 0, 0);
		eXosip_unlock (voip_osip->context);
	}
	voip_osip_unlock(voip_osip);
	return OK;
}

int
voip_osip_restart (void)
{
	zassert(voip_osip != NULL);
	zassert(voip_osip->remote_param != NULL);
	voip_osip_lock(voip_osip);
	if (voip_osip)
	{
		eXosip_lock (voip_osip->context);

		if (voip_osip->r_event)
			voip_event_cancel(voip_osip->r_event);
		if (voip_osip->r_init)
			voip_event_cancel(voip_osip->r_init);
		if (voip_osip->r_reset)
			voip_event_cancel(voip_osip->r_reset);

		if (voip_osip->initialization == TRUE)
		{
			if (voip_osip->r_reset)
				voip_event_cancel(voip_osip->r_reset);
			voip_osip->r_reset = voip_event_ready_add(_voip_osip_reset_event,
													  voip_osip, NULL, 0, 0);
			eXosip_unlock (voip_osip->context);
			voip_osip_unlock(voip_osip);
			return OK;
		}
		if (voip_osip->r_init)
			voip_event_cancel(voip_osip->r_init);
		voip_osip->r_init = voip_event_ready_add(
				_voip_osip_initialization_event, voip_osip, NULL, 0, 0);
		eXosip_unlock (voip_osip->context);
	}
	voip_osip_unlock(voip_osip);
	return OK;
}

static int
voip_osip_state_event_report (voip_osip_t *osip)
{
#ifdef VOIP_MULTI_CALL_MAX
	if (osip)
	{
		if (osip->call_state == SIP_STATE_CALL_FAILED
				&& osip->call_error == SIP_STATE_486)
			voip_app_multi_call_next ();
	}
#endif
	return OK;
}

int
voip_osip_module_init (voip_sip_t *gconfig)
{
	if (voip_osip == NULL)
		voip_osip = XMALLOC(MTYPE_VOIP_SIP, sizeof(voip_osip_t));
	if (!voip_osip)
		return ERROR;
	zassert(voip_osip != NULL);
	zassert(gconfig != NULL);
	memset (voip_osip, 0, sizeof(voip_osip_t));
	if (!voip_osip->context)
	{
		voip_osip->initialization = FALSE;
		voip_osip->enable = FALSE;
		voip_osip->context = eXosip_malloc ();
		voip_osip->mutex = os_mutex_init ();

		voip_osip->remote_param = &voip_osip->remote_param_main;

		voip_osip_config_default (voip_osip);

		eXosip_set_cbsip_message (voip_osip->context, _voip_osip_CbSipCallback);

		osip_trace_initialize_func (OSIP_WARNING, osip_trace_cb_func_t);
		osip_trace_enable_until_level (OSIP_WARNING);
		//osip_trace_initialize_func (OSIP_INFO4, osip_trace_cb_func_t);
		if (eXosip_init (voip_osip->context))
		{
			osip_log_wrapper(LOG_ERR, "eXosip_init failed");
			return ERROR;
		}
	}
	voip_osip->multicall = XMALLOC(MTYPE_VOIP_SIP_CALL,
								   sizeof(callparam_t) * OSIP_MULTI_CALL_MAX);
	voip_osip->sip = gconfig;
	gconfig->osip = voip_osip;
	voip_osip_config_load (voip_osip);
	//_osip_debug = OSIP_DEBUG_EVENT | OSIP_DEBUG_INFO | OSIP_DEBUG_MSG | OSIP_DEBUG_DETAIL;
	return OK;
}

int
voip_osip_module_exit ()
{
	zassert(voip_osip != NULL);
	if (voip_osip && voip_osip->context)
	{
		if (voip_osip->mutex)
			os_mutex_exit (voip_osip->mutex);
		voip_osip->mutex = NULL;
		eXosip_quit (voip_osip->context);
		osip_free(voip_osip->context);

		XFREE(MTYPE_VOIP_SIP_CALL, voip_osip->multicall);
		voip_osip->multicall = NULL;
		voip_osip->remote_param = NULL;

		voip_osip->context = NULL;
		voip_osip->initialization = FALSE;
		voip_osip->enable = FALSE;
		XFREE(MTYPE_VOIP_SIP, voip_osip);
		voip_osip = NULL;
	}
	return OK;
}

int
voip_osip_module_task_init (voip_osip_t *osip)
{
	if (!osip)
		return ERROR;
	if (osip->taskid)
		return OK;
	osip->taskid = os_task_create("osipTask", OS_TASK_DEFAULT_PRIORITY, 0,
								  voip_osip_task, osip, OS_TASK_DEFAULT_STACK);
	if (osip->taskid)
		return OK;
	return ERROR;
}

int
voip_osip_module_task_exit (voip_osip_t *osip)
{
	if (!osip)
		return ERROR;
	if (osip->taskid)
	{
		if (os_task_destroy (osip->taskid) == OK)
			osip->taskid = 0;
	}
	return OK;
}

