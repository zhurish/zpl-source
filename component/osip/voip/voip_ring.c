/*
 * voip_ring.c
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
 */

#include <zebra.h>
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
#include "vty.h"

#include "voip_def.h"
#include "voip_task.h"
#include "voip_app.h"
#include "voip_ring.h"
#include "application.h"

#ifdef PL_VOIP_MEDIASTREAM
#include <mediastreamer2/mscommon.h>
#include <mediastreamer2/mediastream.h>
#endif

static struct ring_session RingSession;

struct ring_sound ring_sound_table[] =
{
	{0,		"/tmp/tmp/bin/oldphone-mono.wav"},
	{1,		"/tmp/tmp/bin/oldphone-mono.wav"},
	{2,		"/tmp/tmp/bin/oldphone-mono.wav"},
};

#define VOIP_CALL_RING_DEFAULT	"/app/etc/ringback.wav"

static int call_ring_max = sizeof(ring_sound_table)/sizeof(ring_sound_table[0]);

static int voip_call_ring_default = 0;


int voip_call_ring_module_init()
{
	memset(&RingSession, 0, sizeof(RingSession));
	if(((voip_task_t*)(voip_app->voip_task))->pVoid)
	{
		RingSession.factory = voip_stream_lookup_factory_api(((voip_task_t*)(voip_app->voip_task))->pVoid);
	}
	RingSession.mutex = os_mutex_init();
	//RingSession.mutex = NULL;
	return OK;
}

int voip_call_ring_lookup_api(int id)
{
	if(id >= call_ring_max)
		return ERROR;
	if(access(ring_sound_table[id].file, F_OK) != 0)
		return ERROR;
	return OK;
}

int voip_call_ring_set_api(int id)
{
	if(id >= call_ring_max)
		return ERROR;
	if(access(ring_sound_table[id].file, F_OK) != 0)
		return ERROR;
	voip_call_ring_default = id;
	return OK;
}

int voip_call_ring_get_api(int *id)
{
	if(id)
		*id = voip_call_ring_default;
	return OK;
}


//#define PL_VOIP_MEDIASTREAM

int voip_call_ring_running(void *pVoid)
{
#ifdef PL_VOIP_MEDIASTREAM
	const char * card_id = NULL;
#endif
	if(!((voip_task_t*)(voip_app->voip_task))->pVoid)
		return OK;
	if (RingSession.mutex)
	{
		os_mutex_lock(RingSession.mutex, OS_WAIT_FOREVER);
	}
	if (((voip_task_t*)(voip_app->voip_task))->pVoid && !RingSession.factory)
	{
		RingSession.factory = voip_stream_lookup_factory_api(((voip_task_t*)(voip_app->voip_task))->pVoid);
	}
	if(!RingSession.factory)
	{
		if (RingSession.mutex)
		{
			os_mutex_unlock(RingSession.mutex);
		}
		zlog_err(ZLOG_VOIP, "RingStream Snd factory is empty");
		return ERROR;
	}
	if (!RingSession.start)
	{
		if (RingSession.mutex)
		{
			os_mutex_unlock(RingSession.mutex);
		}
		zlog_err(ZLOG_VOIP, "RingStream is not start");
		return ERROR;
	}
	RingSession.use = TRUE;
#ifdef PL_VOIP_MEDIASTREAM
	if (RingSession.sndcard == NULL)
	{
		RingSession.sndcard = ms_snd_card_manager_get_card(
				ms_factory_get_snd_card_manager(RingSession.factory), card_id);
		if (RingSession.sndcard == NULL)
			RingSession.sndcard = ms_alsa_card_new_custom(card_id, card_id);
	}
	if (RingSession.sndcard)
	{
		zlog_debug(ZLOG_VOIP, " ------- start ringing");
		RingSession.RingStream = ring_start(RingSession.factory,
				VOIP_CALL_RING_DEFAULT, 500, RingSession.sndcard);
	}
	if (RingSession.mutex)
	{
		os_mutex_unlock(RingSession.mutex);
	}

	while(RingSession.start)
	{
		ms_sleep(5);
	}

	if (RingSession.mutex)
	{
		os_mutex_lock(RingSession.mutex, OS_WAIT_FOREVER);
	}
	//RingSession.start = FALSE;
	zlog_debug(ZLOG_VOIP, " --- quit ringing");

	if (RingSession.RingStream)
		ring_stop(RingSession.RingStream);
	RingSession.use = FALSE;
	RingSession.RingStream = NULL;

	//ms_snd_card_destroy(RingSession.sndcard);
	RingSession.sndcard = NULL;
	if (RingSession.mutex)
	{
		os_mutex_unlock(RingSession.mutex);
	}
#endif
	return OK;
}

/*static int voip_call_ring_timer_thread(struct eloop *eloop)
{
	//voip_sip_t *sip = ELOOP_ARG(eloop);
	if(voip_call_ring_active_api())
	{
		voip_call_ring_stop_api();
		x5b_app_call_result_api(E_CALL_RESULT_FAIL);
	}
	return OK;
}*/


static int voip_call_ring_timer_chk_api()
{
	//struct eloop eloop;
	//eloop.arg = sip;
	//voip_sip_config_update_thread(&eloop);
	//return OK;
/*	if(voip_socket.master)
	{
		if(RingSession.t_timer)
			eloop_cancel(RingSession.t_timer);
		RingSession.t_timer = eloop_add_timer(voip_socket.master,
				voip_call_ring_timer_thread, NULL, 30);
		//voip_socket_sync_cmd();
	}*/
	return OK;
}


int voip_call_ring_start_api()
{
	if(RingSession.mutex)
	{
		os_mutex_lock(RingSession.mutex, OS_WAIT_FOREVER);
	}
	RingSession.start = TRUE;
	((voip_task_t*)(voip_app->voip_task))->enable = TRUE;
	((voip_task_t*)(voip_app->voip_task))->active = TRUE;
	((voip_task_t*)(voip_app->voip_task))->stream = FALSE;
	if(((voip_task_t*)(voip_app->voip_task))->sem)
		os_sem_give(((voip_task_t*)(voip_app->voip_task))->sem);
	zlog_debug(ZLOG_VOIP,"%s\r\n", __func__);
	if(RingSession.mutex)
	{
		os_mutex_unlock(RingSession.mutex);
	}
	voip_call_ring_timer_chk_api();
	return OK;
}

int voip_call_ring_stop_api()
{
	if(RingSession.mutex)
	{
		os_mutex_lock(RingSession.mutex, OS_WAIT_FOREVER);
	}
	RingSession.start = FALSE;
	if(RingSession.mutex)
	{
		os_mutex_unlock(RingSession.mutex);
	}
	zlog_debug(ZLOG_VOIP,"%s\r\n", __func__);
	return OK;
}

BOOL voip_call_ring_active_api()
{
	return RingSession.start;
}
