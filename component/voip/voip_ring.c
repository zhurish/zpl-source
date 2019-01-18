/*
 * voip_ring.c
 *
 *  Created on: 2018年12月28日
 *      Author: DELL
 */

#include <zebra.h>

#include "voip_def.h"
#include "voip_task.h"
#include "voip_ring.h"

#include "voip_stream.h"

#ifdef PL_VOIP_MEDIASTREAM
#include <mediastreamer2/mscommon.h>
#include <mediastreamer2/mediastream.h>
#endif

static struct ring_session call_ring_session;

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
	memset(&call_ring_session, 0, sizeof(call_ring_session));
	if(voip_task.pVoid)
	{
		call_ring_session.f = voip_stream_lookup_factory_api(voip_task.pVoid);
	}
	call_ring_session.mutex = os_mutex_init();
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

#if 0
static int _voip_call_ring_stop(struct ring_session *s)
{
#ifdef PL_VOIP_MEDIASTREAM
	if(s->r)
		ring_stop(s->r);
	s->sc = NULL;
#endif
	return OK;
}


static int voip_call_ring_playback(struct ring_session *s)
{
#ifdef PL_VOIP_MEDIASTREAM
	const char * card_id=NULL;
	if(!call_ring_session.f && voip_task.pVoid)
	{
		call_ring_session.f = voip_stream_lookup_factory_api(voip_task.pVoid);
	}
	s->use = TRUE;
	if(s->sc == NULL)
	{
		s->sc=ms_snd_card_manager_get_card(ms_factory_get_snd_card_manager(s->f),card_id);
		if (s->sc==NULL)
			s->sc = ms_alsa_card_new_custom(card_id, card_id);
	}
	if(s->sc)
	{
		s->r=ring_start(s->f, ring_sound_table[s->id].file, 2000, s->sc);
		ms_sleep(10);
		ring_stop(s->r);
		s->id = -1;
		s->r = NULL;
	}
	s->id = -1;
	s->sc = NULL;
	s->use = FALSE;
	ms_snd_card_destroy(s->sc);
#endif
	return OK;
}


int voip_call_ring_start(int id)
{
	int ret = 0;
	if(id >= call_ring_max)
		return ERROR;
	if(access(ring_sound_table[id].file, F_OK) != 0)
		return ERROR;
	if(call_ring_session.id == -1)
	{
		call_ring_session.id = id;
	}
	else
	{
#ifdef PL_VOIP_MEDIASTREAM
		if(call_ring_session.r)
			ring_stop(call_ring_session.r);
#endif
	}
	ret = os_job_add(voip_call_ring_playback, &call_ring_session);;
	return (ret != ERROR) ? OK:ERROR;//voip_call_ring_playback(&call_ring_session);
}


int voip_call_ring_stop(int id)
{
	if(id >= call_ring_max)
		return ERROR;
	if(access(ring_sound_table[id].file, F_OK) != 0)
		return ERROR;
	if(call_ring_session.id == -1)
		return OK;
	return _voip_call_ring_stop(&call_ring_session);
}

#endif

int voip_call_ring_running(void *pVoid)
{
#ifdef PL_VOIP_MEDIASTREAM
	const char * card_id=NULL;
#endif
	if(voip_task.pVoid)
	{
		call_ring_session.f = voip_stream_lookup_factory_api(pVoid);
	}
	if(!call_ring_session.start)
		return OK;
	call_ring_session.use = TRUE;
#ifdef PL_VOIP_MEDIASTREAM
	if(call_ring_session.sc == NULL)
	{
		call_ring_session.sc = ms_snd_card_manager_get_card(ms_factory_get_snd_card_manager(call_ring_session.f),card_id);
		if (call_ring_session.sc == NULL)
			call_ring_session.sc = ms_alsa_card_new_custom(card_id, card_id);
	}
	if(call_ring_session.sc)
	{
		while(call_ring_session.start)
		{
			if(call_ring_session.mutex)
			{
				os_mutex_lock(call_ring_session.mutex, OS_WAIT_FOREVER);
			}
			zlog_debug(ZLOG_VOIP," call remote start ringing");
			call_ring_session.r=ring_start(call_ring_session.f, VOIP_CALL_RING_DEFAULT,
					1000, call_ring_session.sc);
			//ms_sleep(20);
			if(!call_ring_session.start)
			{
				if(call_ring_session.mutex)
				{
					os_mutex_unlock(call_ring_session.mutex);
				}
				break;
			}
			if(call_ring_session.mutex)
			{
				os_mutex_unlock(call_ring_session.mutex);
			}
			zlog_debug(ZLOG_VOIP, " call remote is ringing");
			zlog_debug(ZLOG_VOIP," call remote stop ringing");
			ms_sleep(30);
/*			ring_stop(call_ring_session.r);
			call_ring_session.r = NULL;*/

		}
	}
	if(call_ring_session.mutex)
	{
		os_mutex_lock(call_ring_session.mutex, OS_WAIT_FOREVER);
	}
	if(call_ring_session.r)
		ring_stop(call_ring_session.r);
	call_ring_session.use = FALSE;
	call_ring_session.r = NULL;
	ms_snd_card_destroy(call_ring_session.sc);
	call_ring_session.sc = NULL;
	if(call_ring_session.mutex)
	{
		os_mutex_unlock(call_ring_session.mutex);
	}
#endif
	return OK;
}

int voip_call_ring_start_api()
{
	if(call_ring_session.mutex)
	{
		os_mutex_lock(call_ring_session.mutex, OS_WAIT_FOREVER);
	}
	call_ring_session.start = TRUE;
	voip_task.enable = TRUE;
	voip_task.active = TRUE;
	voip_task.stream = FALSE;
	zlog_debug(ZLOG_VOIP,"%s\r\n", __func__);
	if(call_ring_session.mutex)
	{
		os_mutex_unlock(call_ring_session.mutex);
	}
	return OK;
}

int voip_call_ring_stop_api()
{
	if(call_ring_session.mutex)
	{
		os_mutex_lock(call_ring_session.mutex, OS_WAIT_FOREVER);
	}
	call_ring_session.start = FALSE;
	voip_task.active = FALSE;
	voip_task.stream = FALSE;
	if(call_ring_session.r)
	{
		ring_stop(call_ring_session.r);
		call_ring_session.r = NULL;
	}
	if(call_ring_session.mutex)
	{
		os_mutex_unlock(call_ring_session.mutex);
	}
	zlog_debug(ZLOG_VOIP,"%s\r\n", __func__);
	return OK;
}

BOOL voip_call_ring_active_api()
{
	return call_ring_session.start;
}

#ifdef PL_VOIP_MEDIASTREAM
int ring_test()
{
	RingStream *r;
	const char *file;
	MSSndCard *sc;
	const char * card_id=NULL;
	MSFactory *factory;

/*	ortp_init();
	ortp_set_log_level_mask(ORTP_LOG_DOMAIN, ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR|ORTP_FATAL);
	*/
	//factory = ms_factory_new_with_voip();
	factory = call_ring_session.f;
/*	if (argc>1){
		file=argv[1];
	}else*/
		file = ring_sound_table[1].file;
		//file="/usr/share/sounds/linphone/rings/oldphone.wav";
/*	if (argc>2){
		card_id=argv[2];
	}*/

	sc=ms_snd_card_manager_get_card(ms_factory_get_snd_card_manager(factory),card_id);
	if (sc==NULL)
	  sc = ms_alsa_card_new_custom(card_id, card_id);

	r=ring_start(factory, file,2000,sc);
	ms_sleep(10);
	ring_stop(r);

	//ms_factory_destroy(factory);
	return 0;
}
#endif
