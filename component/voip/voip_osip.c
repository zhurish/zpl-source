/*
 * voip_osip.c
 *
 *  Created on: Jan 22, 2019
 *      Author: zhurish
 */

//#include "zebra.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <syslog.h>
#include "pthread.h"
#include "semaphore.h"

#include "os_list.h"
#include "os_task.h"


#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>

typedef enum
{
  TRUE  = 1,
  FALSE = 0,
}BOOL;

#include "log.h"
#include "voip_osip.h"

enum
{
  OK  = 0,
  ERROR = -1,
};

#define PROG_VER  "1.0"
#define UA_STRING "SipReg v" PROG_VER

#define osip_log_wrapper(pri, fmt,...)	zlog(ZLOG_VOIP, pri, fmt, ##__VA_ARGS__)
//#define osip_log_wrapper(fmt,...)


voip_osip_t voip_osip;


static int voip_osip_task(void *pVoid);

//typedef void osip_trace_func_t (char *fi, int li, osip_trace_level_t level, char *chfr, va_list ap);

static void osip_trace_cb_func_t (char *fi, int li, osip_trace_level_t level, char *chfr, va_list ap)
{
	vzlog(NULL, ZLOG_VOIP, level, chfr, ap);
}
/*
void osip_trace_initialize_func (osip_trace_level_t level, osip_trace_func_t * func);
void osip_trace_initialize_syslog (osip_trace_level_t level, char *ident);
int osip_trace_initialize (osip_trace_level_t level, FILE * file);
void osip_trace_enable_until_level (osip_trace_level_t level);
void osip_trace_enable_level (osip_trace_level_t level);
void osip_trace_disable_level (osip_trace_level_t level);
int osip_is_trace_level_activate (osip_trace_level_t level);
*/


int voip_osip_module_init()
{
	memset(&voip_osip, 0, sizeof(voip_osip));
	if (!voip_osip.context) {
		voip_osip.context = eXosip_malloc ();
		osip_trace_initialize_func (TRACE_LEVEL7, osip_trace_cb_func_t);
		if (eXosip_init (voip_osip.context)) {
		    osip_log_wrapper (LOG_ERR, "eXosip_init failed");
		    return ERROR;
		}
	}
	return OK;
}


int voip_osip_module_exit()
{
	if (voip_osip.context) {
		eXosip_quit(voip_osip.context);
		osip_free(voip_osip.context);
		voip_osip.context = NULL;
	}
	return OK;
}


int voip_osip_module_task_init(voip_osip_t *osip)
{
	if(!osip->enable)
		return ERROR;
	if(osip->taskid)
		return OK;
	osip->taskid = os_task_create("osipTask", OS_TASK_DEFAULT_PRIORITY,
	               0, voip_osip_task, osip, OS_TASK_DEFAULT_STACK);
	if(osip->taskid)
		return OK;
	return ERROR;
}


int voip_osip_module_task_exit(voip_osip_t *osip)
{
	if(osip->taskid)
	{
		if(os_task_destroy(osip->taskid)==OK)
			osip->taskid = 0;
	}
	return OK;
}

int voip_osip_context_init_api(voip_osip_t *osip)
{
	int err = 0;

	if (osip->proto == OSIP_PROTO_UDP)
	{
		err = eXosip_listen_addr(osip->context, IPPROTO_UDP, NULL, osip->port, AF_INET, 0);
	}
	else if (osip->proto == OSIP_PROTO_TCP)
	{
		err = eXosip_listen_addr(osip->context, IPPROTO_TCP, NULL, osip->port, AF_INET, 0);
	}
	else if (osip->proto == OSIP_PROTO_TLS)
	{
		err = eXosip_listen_addr(osip->context, IPPROTO_TCP, NULL, osip->port, AF_INET, 1);
	}
	else if (osip->proto == OSIP_PROTO_DTLS)
	{
		err = eXosip_listen_addr(osip->context, IPPROTO_UDP, NULL, osip->port, AF_INET, 1);
	}
	if (err)
	{
	    osip_log_wrapper (LOG_ERR, "eXosip_listen_addr failed");
	    return ERROR;
	}
	if (osip->address) {
		osip_log_wrapper(LOG_INFO, "local address: %s", osip->address);
		eXosip_masquerade_contact(osip->context, osip->address, osip->port);
	}

	if (osip->firewallip) {
		osip_log_wrapper(LOG_INFO, "firewall address: %s:%i", osip->firewallip, osip->port);
		eXosip_masquerade_contact(osip->context, osip->firewallip, osip->port);
	}
	return OK;
}

int voip_osip_context_username_api(voip_osip_t *osip)
{
	eXosip_set_user_agent(osip->context, UA_STRING);

	if (osip->username && osip->password)
	{
		osip_log_wrapper(LOG_INFO, "username: %s", osip->username);
		osip_log_wrapper(LOG_INFO, "password: [removed]");
		if (eXosip_add_authentication_info(osip->context, osip->username, osip->username,
				osip->password, NULL, NULL))
		{
			osip_log_wrapper(LOG_ERR, "eXosip_add_authentication_info failed");
			return ERROR;
		}
	}
	return OK;
}


int voip_osip_register_api(voip_osip_t *osip)
{
	osip_message_t *reg = NULL;
	int i;
	osip->regparam.regid = eXosip_register_build_initial_register(osip->context,
			osip->fromuser, osip->proxy, osip->contact, osip->regparam.expiry * 2, &reg);
	if (osip->regparam.regid < 1) {
		osip_log_wrapper(LOG_ERR,
				"eXosip_register_build_initial_register failed");
		return ERROR;
	}
	i = eXosip_register_send_register(osip->context, osip->regparam.regid, reg);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "eXosip_register_send_register failed");
		return ERROR;
	}
	return OK;
}

int voip_osip_call_start_api(voip_osip_t *osip)
{
	osip_message_t *reg = NULL;
	int i;
	osip->regparam.regid = eXosip_register_build_initial_register(osip->context,
			osip->fromuser, osip->proxy, osip->contact, osip->regparam.expiry * 2, &reg);
	if (osip->regparam.regid < 1) {
		osip_log_wrapper(LOG_ERR,
				"eXosip_register_build_initial_register failed");
		return ERROR;
	}
	i = eXosip_register_send_register(osip->context, osip->regparam.regid, reg);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "eXosip_register_send_register failed");
		return ERROR;
	}
	return OK;
}

int voip_osip_call_stop_api(voip_osip_t *osip)
{
	osip_message_t *reg = NULL;
	int i;
	osip->regparam.regid = eXosip_register_build_initial_register(osip->context,
			osip->fromuser, osip->proxy, osip->contact, osip->regparam.expiry * 2, &reg);
	if (osip->regparam.regid < 1) {
		osip_log_wrapper(LOG_ERR,
				"eXosip_register_build_initial_register failed");
		return ERROR;
	}
	i = eXosip_register_send_register(osip->context, osip->regparam.regid, reg);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "eXosip_register_send_register failed");
		return ERROR;
	}
	return OK;
}

int voip_osip_request_info_api(voip_osip_t *osip)
{
	osip_message_t *reg = NULL;
	int i;
	osip->regparam.regid = eXosip_register_build_initial_register(osip->context,
			osip->fromuser, osip->proxy, osip->contact, osip->regparam.expiry * 2, &reg);
	if (osip->regparam.regid < 1) {
		osip_log_wrapper(LOG_ERR,
				"eXosip_register_build_initial_register failed");
		return ERROR;
	}
	i = eXosip_register_send_register(osip->context, osip->regparam.regid, reg);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "eXosip_register_send_register failed");
		return ERROR;
	}
	return OK;
}








/*
 *
 */

static int voip_osip_stats_handle(voip_osip_t *osip, int *counter)
{
	struct eXosip_stats stats;
	//int counter = 0;
	(*counter)++;
	if ((*counter) % 60000 == 0) {
		memset(&stats, 0, sizeof(struct eXosip_stats));
		eXosip_lock(osip->context);
		eXosip_set_option(osip->context, EXOSIP_OPT_GET_STATISTICS,
				&stats);
		eXosip_unlock(osip->context);
		osip_log_wrapper(LOG_INFO,
				"eXosip stats: inmemory=(tr:%i//reg:%i) average=(tr:%f//reg:%f)",
				stats.allocated_transactions, stats.allocated_registrations,
				stats.average_transactions, stats.average_registrations);
	}
	return OK;
}

static int voip_osip_register_handle(voip_osip_t *osip, int res)
{
	if(res == EXOSIP_REGISTRATION_SUCCESS)
		osip_log_wrapper(LOG_INFO, "registrered successfully");
	if(res == EXOSIP_REGISTRATION_FAILURE)
		osip_log_wrapper(LOG_INFO, "registrered fail");
	return OK;
}


static int voip_osip_call_invite_handle(voip_osip_t *osip, eXosip_event_t *event,
		osip_message_t *answer, int res)
{
	int i = eXosip_call_build_answer(osip->context, event->tid, 405,
			&answer);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "failed to reject INVITE");
		return ERROR;
		//break;
	}
	osip_free(answer->reason_phrase);
	answer->reason_phrase = osip_strdup(
			"No Support for Incoming Calls");
	i = eXosip_call_send_answer(osip->context, event->tid, 405,
			answer);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "failed to reject INVITE");
		return ERROR;
		//break;
	}
	osip_log_wrapper(LOG_INFO, "INVITE rejected with 405");
	return OK;
}

static int voip_osip_call_reinvite_handle(voip_osip_t *osip, eXosip_event_t *event,
		osip_message_t *answer, int res)
{
	int i = eXosip_call_build_answer(osip->context, event->tid, 405,
			&answer);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "failed to reject INVITE");
		return ERROR;
		//break;
	}
	osip_free(answer->reason_phrase);
	answer->reason_phrase = osip_strdup(
			"No Support for Incoming Calls");
	i = eXosip_call_send_answer(osip->context, event->tid, 405,
			answer);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "failed to reject INVITE");
		return ERROR;
		//break;
	}
	osip_log_wrapper(LOG_INFO, "INVITE rejected with 405");
	return OK;
}

static int voip_osip_massage_new_handle(voip_osip_t *osip, eXosip_event_t *event,
		osip_message_t *answer, int res)
{
	int i = eXosip_message_build_answer(osip->context, event->tid, 405,
			&answer);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "failed to reject %s",
				event->request->sip_method);
		return ERROR;
		//break;
	}
	i = eXosip_message_send_answer(osip->context, event->tid, 405,
			answer);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "failed to reject %s",
				event->request->sip_method);
		return ERROR;
		//break;
	}
	osip_log_wrapper(LOG_INFO, "%s rejected with 405",
			event->request->sip_method);
	return OK;
}

static int voip_osip_insubscription_new_handle(voip_osip_t *osip, eXosip_event_t *event,
		osip_message_t *answer, int res)
{
	int i = eXosip_insubscription_build_answer(osip->context,
			event->tid, 405, &answer);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "failed to reject %s",
				event->request->sip_method);
		return ERROR;
		//break;
	}
	i = eXosip_insubscription_send_answer(osip->context, event->tid,
			405, answer);
	if (i != 0) {
		osip_log_wrapper(LOG_ERR, "failed to reject %s",
				event->request->sip_method);
		return ERROR;
		//break;
	}
	osip_log_wrapper(LOG_INFO, "%s rejected with 405",
			event->request->sip_method);
	return OK;
}


static int voip_osip_task(void *pVoid)
{
	//int i;
	int counter = 0;
	voip_osip_t *osip = (voip_osip_t *) pVoid;
	eXosip_event_t *event;
	osip_message_t *answer;

	while (!os_load_config_done())
	{
		os_sleep(1);
	}

	while (1)
	{
		voip_osip_stats_handle(osip, &counter);

		if (!(event = eXosip_event_wait(osip->context, 0, 100)))
		{
#ifdef OSIP_MONOTHREAD
			eXosip_execute (osip->context);
#endif
			eXosip_automatic_action(osip->context);
			osip_usleep(10000);
			continue;
		}
#ifdef OSIP_MONOTHREAD
		eXosip_execute (osip->context);
#endif

		eXosip_lock(osip->context);
		eXosip_automatic_action(osip->context);

		switch (event->type) {
	    /* REGISTER related events */
	    /**< user is successfully registred.  */
	    /**< user is not registred.           */
		case EXOSIP_REGISTRATION_SUCCESS:
		case EXOSIP_REGISTRATION_FAILURE:
			voip_osip_register_handle(osip, event->type);
			break;
	    /* INVITE related events within calls */
	    /**< announce a new call                   */
		case EXOSIP_CALL_INVITE: {
			voip_osip_call_invite_handle(osip, event, answer, 0);
			break;
		}
	    /**< announce a new INVITE within call     */
		case EXOSIP_CALL_REINVITE: {
			voip_osip_call_reinvite_handle(osip, event, answer, 0);
			break;
		}
#if 0
	    EXOSIP_CALL_NOANSWER,          /**< announce no answer within the timeout */
	    EXOSIP_CALL_PROCEEDING,        /**< announce processing by a remote app   */
	    EXOSIP_CALL_RINGING,           /**< announce ringback                     */
	    EXOSIP_CALL_ANSWERED,          /**< announce start of call                */
	    EXOSIP_CALL_REDIRECTED,        /**< announce a redirection                */
	    EXOSIP_CALL_REQUESTFAILURE,    /**< announce a request failure            */
	    EXOSIP_CALL_SERVERFAILURE,     /**< announce a server failure             */
	    EXOSIP_CALL_GLOBALFAILURE,     /**< announce a global failure             */
	    EXOSIP_CALL_ACK,               /**< ACK received for 200ok to INVITE      */

	    EXOSIP_CALL_CANCELLED,         /**< announce that call has been cancelled */

	    /* request related events within calls (except INVITE) */
	    EXOSIP_CALL_MESSAGE_NEW,              /**< announce new incoming request. */
	    EXOSIP_CALL_MESSAGE_PROCEEDING,       /**< announce a 1xx for request. */
	    EXOSIP_CALL_MESSAGE_ANSWERED,         /**< announce a 200ok  */
	    EXOSIP_CALL_MESSAGE_REDIRECTED,       /**< announce a failure. */
	    EXOSIP_CALL_MESSAGE_REQUESTFAILURE,   /**< announce a failure. */
	    EXOSIP_CALL_MESSAGE_SERVERFAILURE,    /**< announce a failure. */
	    EXOSIP_CALL_MESSAGE_GLOBALFAILURE,    /**< announce a failure. */

	    EXOSIP_CALL_CLOSED,            /**< a BYE was received for this call      */

	    /* for both UAS & UAC events */
	    EXOSIP_CALL_RELEASED,             /**< call context is cleared.            */
#endif
	    /* events received for request outside calls */
	    /**< announce new incoming request. */
		case EXOSIP_MESSAGE_NEW: {
			voip_osip_massage_new_handle(osip, event, answer, 0);
			break;
		}
#if 0
	    EXOSIP_MESSAGE_PROCEEDING,       /**< announce a 1xx for request. */
	    EXOSIP_MESSAGE_ANSWERED,         /**< announce a 200ok  */
	    EXOSIP_MESSAGE_REDIRECTED,       /**< announce a failure. */
	    EXOSIP_MESSAGE_REQUESTFAILURE,   /**< announce a failure. */
	    EXOSIP_MESSAGE_SERVERFAILURE,    /**< announce a failure. */
	    EXOSIP_MESSAGE_GLOBALFAILURE,    /**< announce a failure. */

	    /* Presence and Instant Messaging */
	    EXOSIP_SUBSCRIPTION_NOANSWER,          /**< announce no answer              */
	    EXOSIP_SUBSCRIPTION_PROCEEDING,        /**< announce a 1xx                  */
	    EXOSIP_SUBSCRIPTION_ANSWERED,          /**< announce a 200ok                */
	    EXOSIP_SUBSCRIPTION_REDIRECTED,        /**< announce a redirection          */
	    EXOSIP_SUBSCRIPTION_REQUESTFAILURE,    /**< announce a request failure      */
	    EXOSIP_SUBSCRIPTION_SERVERFAILURE,     /**< announce a server failure       */
	    EXOSIP_SUBSCRIPTION_GLOBALFAILURE,     /**< announce a global failure       */
	    EXOSIP_SUBSCRIPTION_NOTIFY,            /**< announce new NOTIFY request     */
#endif
	    /**< announce new incoming SUBSCRIBE/REFER.*/
		case EXOSIP_IN_SUBSCRIPTION_NEW: {
			voip_osip_insubscription_new_handle(osip, event, answer, 0);
			break;
		}
#if 0
	    EXOSIP_NOTIFICATION_NOANSWER,          /**< announce no answer              */
	    EXOSIP_NOTIFICATION_PROCEEDING,        /**< announce a 1xx                  */
	    EXOSIP_NOTIFICATION_ANSWERED,          /**< announce a 200ok                */
	    EXOSIP_NOTIFICATION_REDIRECTED,        /**< announce a redirection          */
	    EXOSIP_NOTIFICATION_REQUESTFAILURE,    /**< announce a request failure      */
	    EXOSIP_NOTIFICATION_SERVERFAILURE,     /**< announce a server failure       */
	    EXOSIP_NOTIFICATION_GLOBALFAILURE,     /**< announce a global failure       */

	    EXOSIP_EVENT_COUNT                  /**< MAX number of events              */
#endif
		default:
			osip_log_wrapper(LOG_DEBUG,
					"recieved unknown eXosip event (type, did, cid) = (%d, %d, %d)",
					event->type, event->did, event->cid);

		}
		eXosip_unlock(osip->context);
		//eXosip_event_free(event);
	}
	return OK;
}
