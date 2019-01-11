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

static int call_ring_max = sizeof(ring_sound_table)/sizeof(ring_sound_table[0]);

static int voip_call_ring_default = 0;


int voip_call_ring_module_init()
{
	memset(&call_ring_session, 0, sizeof(call_ring_session));
	if(voip_task.pVoid)
	{
		call_ring_session.f = voip_stream_lookup_factory_api(voip_task.pVoid);
	}
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
