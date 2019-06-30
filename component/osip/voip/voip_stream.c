/*
 * voip_stream.c
 *
 *  Created on: Jan 6, 2019
 *      Author: zhurish
 */

#include "zebra.h"
#include "memory.h"
#include "log.h"
#include "network.h"
#include "str.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "vector.h"
#include "vty.h"
#include "eloop.h"

#include "voip_def.h"
#include "voip_task.h"
#include "voip_sip.h"
#include "voip_osip.h"
#include "voip_stream.h"
#include "voip_socket.h"
#include "voip_app.h"
#include "voip_uci.h"


voip_stream_t *voip_stream = NULL;



void *voip_lookup_stream_api()
{
	zassert(voip_stream != NULL);
	return voip_stream;
}

BOOL voip_stream_is_enable()
{
	zassert(voip_stream != NULL);
	if(voip_stream && voip_stream->enable)
		return TRUE;
	else
		return FALSE;
}


static voip_stream_t* voip_stream_init_default(void)
{
	voip_stream_t *tmp = NULL;
	if(!tmp)
	{
/*		tmp = malloc(sizeof(voip_stream_t));
		if(!tmp)
			return NULL;*/
		tmp = voip_app->voip_stream;
		memset(tmp, 0, sizeof(voip_stream_t));
		if(((voip_task_t*)(voip_app->voip_task))->pVoid == NULL)
			((voip_task_t*)(voip_app->voip_task))->pVoid = tmp;
	}
#ifdef PL_VOIP_MEDIASTREAM
	tmp->mediastream = mediastream_init_default();
	if(!tmp->mediastream)
	{
		free(((voip_task_t*)(voip_app->voip_task))->pVoid);
		((voip_task_t*)(voip_app->voip_task))->pVoid = NULL;
		tmp = NULL;
		return NULL;
	}
	tmp->mediastream->priv = tmp;
#endif

	tmp->enable = TRUE;
	tmp->l_rtp_port = VOIP_RTP_PORT_DEFAULT;
	//tmp->payload = VOIP_RTP_PAYLOAD_DEFAULT;
	tmp->play_dtmf = TRUE;

	tmp->bitrate = 0;
	tmp->jitter = 50;
	tmp->rtcp = TRUE;
	tmp->avpf = TRUE;

	//echo canceller
	tmp->echo_canceller = TRUE;
//	voip_stream->ec_tail;
//	voip_stream->ec_delay;
//	voip_stream->ec_framesize;

	//echo limiter
	tmp->echo_limiter = TRUE;
/*	voip_stream->el_force;
	voip_stream->el_speed;
	voip_stream->el_thres;
	voip_stream->el_transmit;
	voip_stream->el_sustain;*/

	//echo limiter
	tmp->noise_gate = TRUE;
/*	voip_stream->ng_floorgain;
	voip_stream->ng_threshold;*/
	return tmp;
}


int voip_stream_module_init()
{
	memset(voip_app->voip_task, 0, sizeof(voip_task_t));
/*	voip_stream = malloc(sizeof(voip_stream_t));
	memset(voip_stream, 0, sizeof(voip_stream_t));*/
	voip_stream = voip_stream_init_default();
	if(!voip_stream)
		return ERROR;
	if(((voip_task_t*)(voip_app->voip_task))->pVoid == NULL)
		((voip_task_t*)(voip_app->voip_task))->pVoid = voip_stream;
	voip_streams_hw_init(voip_stream);
#ifdef PL_OPENWRT_UCI
//	uci_ubus_cb_install(voip_stream_uci_update_cb);
	voip_stream_config_load(voip_stream);
#endif

#ifdef VOIP_STREAM_UNIT_TESTING
	x5b_unit_test_init();
#endif
	return OK;
}

int voip_stream_module_exit()
{
	if(((voip_task_t*)(voip_app->voip_task))->pVoid != NULL)
	{
		voip_streams_hw_exit(voip_stream);
		free(((voip_task_t*)(voip_app->voip_task))->pVoid);
		((voip_task_t*)(voip_app->voip_task))->pVoid = NULL;
		//voip_stream = NULL;
	}
	return OK;
}

int voip_stream_local_port_api(voip_stream_t* out, u_int16 port)
{
	zassert(out != NULL);
	if(!out)
		return ERROR;
	out->l_rtp_port = port ? port:VOIP_RTP_PORT_DEFAULT;
#ifdef PL_OPENWRT_UCI
	voip_stream_config_update_api(out);
#endif
	return OK;
}

u_int16 voip_stream_local_port_get_api()
{
	zassert(voip_stream != NULL);
	if(!voip_stream)
		return 0;
	return voip_stream->l_rtp_port;
}

int voip_stream_remote_address_port_api(voip_stream_t* out, char *remote, u_int16 port)
{
	zassert(out != NULL);
	if(!out)
		return ERROR;

	out->r_rtp_port = port;
	memset(out->r_rtp_address, 0, sizeof(out->r_rtp_address));
	if(remote)
		strcpy(out->r_rtp_address, remote);
	return OK;
}


int voip_stream_echo_canceller_api(voip_stream_t* out, BOOL enable, int ec_tail, int ec_delay, int ec_framesize)
{
	zassert(out != NULL);
	if(!out)
		return ERROR;
	out->echo_canceller = enable;
	out->ec_tail = ec_tail;
	out->ec_delay = ec_delay;
	out->ec_framesize = ec_framesize;
#ifdef PL_OPENWRT_UCI
	voip_stream_config_update_api(out);
#endif
	return OK;
}

int voip_stream_echo_limiter_api(voip_stream_t* out, BOOL enable, float force, float speed, int sustain, float thres, float transmit)
{
	zassert(out != NULL);
	if(!out)
		return ERROR;
	out->echo_limiter = enable;
	out->el_speed = speed;
	out->el_force = force;
	out->el_sustain = sustain;
	out->el_thres = thres;
	out->el_transmit = transmit;
#ifdef PL_OPENWRT_UCI
	voip_stream_config_update_api(out);
#endif
	return OK;
}

int voip_stream_bitrate_api(voip_stream_t* out, int bitrate)
{
	zassert(out != NULL);
	if(!out)
		return ERROR;
	out->bitrate = bitrate;
#ifdef PL_OPENWRT_UCI
	voip_stream_config_update_api(out);
#endif
	return OK;
}

int voip_stream_jitter_api(voip_stream_t* out, int jitter)
{
	zassert(out != NULL);
	if(!out)
		return ERROR;
	out->jitter = jitter;
#ifdef PL_OPENWRT_UCI
	voip_stream_config_update_api(out);
#endif
	return OK;
}

int voip_stream_disable_rtcp_api(voip_stream_t* out, BOOL enable)
{
	zassert(out != NULL);
	if(!out)
		return ERROR;
	if(enable)
		out->rtcp = FALSE;
	else
		out->rtcp = TRUE;
#ifdef PL_OPENWRT_UCI
	voip_stream_config_update_api(out);
#endif
	return OK;
}

int voip_stream_disable_avpf_api(voip_stream_t* out, BOOL enable)
{
	zassert(out != NULL);
	if(!out)
		return ERROR;
	if(enable)
		out->avpf = FALSE;
	else
		out->avpf = TRUE;
#ifdef PL_OPENWRT_UCI
	voip_stream_config_update_api(out);
#endif
	return OK;
}

int voip_stream_noise_gate_api(voip_stream_t* out, BOOL enable, float ng_floorgain, float ng_threshold)
{
	zassert(out != NULL);
	if(!out)
		return ERROR;
	out->noise_gate = enable;
	out->ng_threshold=ng_threshold;
	out->ng_floorgain=ng_floorgain;
#ifdef PL_OPENWRT_UCI
	voip_stream_config_update_api(out);
#endif
	return OK;
}



int voip_stream_dtmf_api(BOOL enable)
{
	zassert(voip_stream != NULL);
	if(!voip_stream)
		return ERROR;
	voip_stream->enable_dtmf = enable;
#ifdef PL_OPENWRT_UCI
	voip_stream_config_update_api(voip_stream);
#endif
	return OK;
}

int voip_stream_play_dtmf_api(BOOL enable)
{
	zassert(voip_stream != NULL);
	if(!voip_stream)
		return ERROR;
	voip_stream->play_dtmf = enable;
#ifdef PL_OPENWRT_UCI
	voip_stream_config_update_api(voip_stream);
#endif
	return OK;
}

int voip_stream_dtmf_cb_api(int (*cb)(int, int))
{
	zassert(voip_stream != NULL);
	if(!voip_stream)
		return ERROR;
	voip_stream->app_dtmf_cb = cb;
	return OK;
}
/************************************************************************/
/************************************************************************/
#ifdef PL_VOIP_MEDIASTREAM
static int voip_media_stream_config_active_api(voip_stream_t* out)
{

	out->mediastream->remoteport = out->r_rtp_port;
	memset(out->mediastream->ip, 0, sizeof(out->mediastream->ip));
	if(strlen(out->r_rtp_address))
		strcpy(out->mediastream->ip, out->r_rtp_address);
	out->mediastream->localport = out->l_rtp_port;

/*	out->mediastream->payload = (payload >= 0) ? payload:VOIP_RTP_PAYLOAD_DEFAULT;
	if(payload == -1)
	{
		out->mediastream->payload = 0;
		if(name)
			out->mediastream->custom_pt=ms_tools_parse_custom_payload(name);
	}*/

	out->mediastream->ec = out->echo_canceller;
	out->mediastream->ec_len_ms = out->ec_tail;
	out->mediastream->ec_delay_ms = out->ec_delay;
	out->mediastream->ec_framesize = out->ec_framesize;

	out->mediastream->el = out->echo_limiter;
	out->mediastream->el_speed = out->el_speed;
	out->mediastream->el_force = out->el_force;
	out->mediastream->el_sustain = out->el_sustain;

	out->mediastream->el_thres = out->el_thres;
	out->mediastream->el_transmit_thres = out->el_transmit;

	out->mediastream->bitrate = out->bitrate;
	out->mediastream->jitter = out->jitter;
	out->mediastream->enable_rtcp = out->rtcp;
	out->mediastream->enable_avpf = out->avpf;
	out->mediastream->use_ng = out->noise_gate;
	out->mediastream->ng_threshold=out->ng_threshold;
	out->mediastream->ng_floorgain=out->ng_floorgain;
	out->mediastream->enable_play_dtmf = out->play_dtmf;
	out->mediastream->enable_dtmf = out->enable_dtmf;
	return OK;
}
#endif

/*
"[ --ice-local-candidate <ip:port:[host|srflx|prflx|relay]> ]\n"
"[ --ice-remote-candidate <ip:port:[host|srflx|prflx|relay]> ]\n"
*/



int voip_stream_ctlfd_get_api(void)
{
	return voip_socket_get_writefd();
}

void voip_stream_setup_api(voip_stream_t* args)
{
	zassert(args != NULL);
	if(!args)
		return ;
	sip_dtmf_t dtmf = 0;
	voip_sip_dtmf_get_api(&dtmf);
	if(dtmf == VOIP_SIP_RFC2833)
		voip_stream_dtmf_api(TRUE);
#ifdef PL_VOIP_MEDIASTREAM
	voip_media_stream_config_active_api(args);
	return mediastream_setup(args->mediastream);
#else
	return;
#endif
}

void voip_stream_running_api(voip_stream_t* args)
{
	zassert(args != NULL);
	if(!args)
		return ;
	voip_media_state_set(args, SIP_STATE_MEDIA_CONNECTED);
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_running(args->mediastream);
#else
	while(1)
	{
		os_sleep(1);
	}
	return ;
#endif
}

void voip_stream_clear_api(voip_stream_t* args)
{
	zassert(args != NULL);
	if(!args)
		return ;
	voip_media_state_set(args, SIP_STATE_MEDIA_IDLE);
	voip_stream_stop_flag_api();
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_clear(args->mediastream);
#else
	return;
#endif
}

void *voip_stream_lookup_factory_api(voip_stream_t *args)
{
	zassert(args != NULL);
	if(!args)
		return NULL;
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_lookup_factory(args->mediastream);
#else
	return args->factory;
#endif
}

int voip_streams_hw_init(voip_stream_t *args)
{
	zassert(args != NULL);
	if(!args)
		return ERROR;
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_hw_init(args->mediastream);
#else
	return OK;
#endif
}

int voip_streams_hw_exit(voip_stream_t *args)
{
	zassert(args != NULL);
	if(!args)
		return ERROR;
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_hw_exit(args->mediastream);
#else
	return OK;
#endif
}


int voip_stream_debug_set_api(BOOL enable, char *debug)
{
#ifdef PL_VOIP_MEDIASTREAM
	int levelmask = 0;
	if(strstr(debug, "debug"))
	{
		if(enable)
			bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
		else
		{
			levelmask = bctbx_get_log_level_mask(BCTBX_LOG_DOMAIN);
			levelmask &= ~BCTBX_LOG_DEBUG;
			bctbx_set_log_level_mask(BCTBX_LOG_DOMAIN, levelmask);
		}
	}
	if(strstr(debug, "trace"))
	{
		if(enable)
			bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_TRACE);
		else
		{
			levelmask = bctbx_get_log_level_mask(BCTBX_LOG_DOMAIN);
			levelmask &= ~BCTBX_LOG_TRACE;
			bctbx_set_log_level_mask(BCTBX_LOG_DOMAIN, levelmask);
		}
	}
	if(strstr(debug, "message"))
	{
		if(enable)
			bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_MESSAGE);
		else
		{
			levelmask = bctbx_get_log_level_mask(BCTBX_LOG_DOMAIN);
			levelmask &= ~BCTBX_LOG_MESSAGE;
			bctbx_set_log_level_mask(BCTBX_LOG_DOMAIN, levelmask);
		}
	}
	if(strstr(debug, "warning"))
	{
		if(enable)
			bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_WARNING);
		else
		{
			levelmask = bctbx_get_log_level_mask(BCTBX_LOG_DOMAIN);
			levelmask &= ~BCTBX_LOG_WARNING;
			bctbx_set_log_level_mask(BCTBX_LOG_DOMAIN, levelmask);
		}
	}
	if(strstr(debug, "error"))
	{
		if(enable)
			bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_ERROR);
		else
		{
			levelmask = bctbx_get_log_level_mask(BCTBX_LOG_DOMAIN);
			levelmask &= ~BCTBX_LOG_ERROR;
			bctbx_set_log_level_mask(BCTBX_LOG_DOMAIN, levelmask);
		}
	}
	if(strstr(debug, "fatal"))
	{
		if(enable)
			bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_FATAL);
		else
		{
			levelmask = bctbx_get_log_level_mask(BCTBX_LOG_DOMAIN);
			levelmask &= ~BCTBX_LOG_FATAL;
			bctbx_set_log_level_mask(BCTBX_LOG_DOMAIN, levelmask);
		}
	}
#endif
	return OK;
}

int voip_stream_debug_get_api(struct vty *vty)
{
#ifdef PL_VOIP_MEDIASTREAM
	int levelmask = 0;

	levelmask = bctbx_get_log_level_mask(BCTBX_LOG_DOMAIN);

	if(levelmask & BCTBX_LOG_DEBUG)
		vty_out (vty, "  Voip Stream debug debugging is on%s", VTY_NEWLINE);

	if(levelmask & BCTBX_LOG_TRACE)
		vty_out (vty, "  Voip Stream trace debugging is on%s", VTY_NEWLINE);

	if(levelmask & BCTBX_LOG_MESSAGE)
		vty_out (vty, "  Voip Stream message debugging is on%s", VTY_NEWLINE);

	if(levelmask & BCTBX_LOG_WARNING)
		vty_out (vty, "  Voip Stream warning debugging is on%s", VTY_NEWLINE);

	if(levelmask & BCTBX_LOG_ERROR)
		vty_out (vty, "  Voip Stream error debugging is on%s", VTY_NEWLINE);

	if(levelmask & BCTBX_LOG_FATAL)
		vty_out (vty, "  Voip Stream fatal debugging is on%s", VTY_NEWLINE);
	return OK;
#endif
}

int voip_stream_show_config(struct vty *vty)
{
	zassert(voip_stream != NULL);
	zassert(vty != NULL);
	if(voip_stream)
	{
		vty_out(vty, " voip stream                        : %s%s", voip_stream->enable ? "ENABLE":"DISABLE", VTY_NEWLINE);
		vty_out(vty, " voip stream rtp-port               : %d%s", voip_stream->l_rtp_port, VTY_NEWLINE);
/*
		vty_out(vty, " voip stream payload                : %d%s", voip_stream->payload, VTY_NEWLINE);
		if(strlen(voip_stream->payload_name))
			vty_out(vty, " voip stream payload name           : %s%s", voip_stream->payload_name, VTY_NEWLINE);
*/

		if(strlen(voip_stream->r_rtp_address))
		{
			//if(voip_sip_call_state_get_api() >= VOIP_SIP_CALL_RINGING)
			if(voip_app_state_get(voip_app) == APP_STATE_TALK_RUNNING)
				vty_out(vty, " voip stream remote                 : %s:%d%s",
					voip_stream->r_rtp_address, voip_stream->r_rtp_port, VTY_NEWLINE);
			else
				vty_out(vty, " voip stream remote                 : %s:%d%s",
					"0.0.0.0", 0, VTY_NEWLINE);
		}
		vty_out(vty, " voip stream bitrate                : %d%s", voip_stream->bitrate, VTY_NEWLINE);

		vty_out(vty, " voip stream jitter                 : %d%s", voip_stream->jitter, VTY_NEWLINE);

		vty_out(vty, " voip stream rtcp                   : %s%s", voip_stream->rtcp ? "TRUE":"FALSE",VTY_NEWLINE);

		vty_out(vty, " voip stream avpf                   : %s%s", voip_stream->avpf ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " voip stream DTMF                   : %s%s", voip_stream->enable_dtmf ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " voip stream paly DTMF              : %s%s", voip_stream->play_dtmf ? "TRUE":"FALSE", VTY_NEWLINE);

		vty_out(vty, " voip stream echo-canceller         : %s%s", voip_stream->echo_canceller ? "TRUE":"FALSE", VTY_NEWLINE);

		vty_out(vty, " voip stream ec-tail                : %d%s", voip_stream->ec_tail, VTY_NEWLINE);
		vty_out(vty, " voip stream ec-delay               : %d%s", voip_stream->ec_delay, VTY_NEWLINE);
		vty_out(vty, " voip stream ec-framesize           : %d%s", voip_stream->ec_framesize, VTY_NEWLINE);

		//echo limiter
		vty_out(vty, " voip stream echo-limiter           : %s%s", voip_stream->echo_limiter ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " voip stream el-force               : %0.2f%s", voip_stream->el_force, VTY_NEWLINE);
		vty_out(vty, " voip stream el-thres               : %0.2f%s", voip_stream->el_thres, VTY_NEWLINE);
		vty_out(vty, " voip stream el-transmit-thres      : %0.2f%s", voip_stream->el_transmit, VTY_NEWLINE);
		vty_out(vty, " voip stream el-sustain             : %d%s", voip_stream->el_sustain, VTY_NEWLINE);
		vty_out(vty, " voip stream noise-gate             : %s%s", voip_stream->noise_gate ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " voip stream ng-floorgain           : %0.2f%s", voip_stream->ng_floorgain, VTY_NEWLINE);
		vty_out(vty, " voip stream ng-threshold           : %0.2f%s", voip_stream->ng_threshold, VTY_NEWLINE);
#ifdef PL_VOIP_MEDIASTREAM
		voip_mediastream_show_config(voip_stream->mediastream, vty);
#endif
	}
	return OK;
}

int voip_stream_write_config(struct vty *vty)
{
	zassert(vty != NULL);
	zassert(voip_stream != NULL);
	if(voip_stream && voip_stream->enable)
	{
		if(voip_stream->l_rtp_port != VOIP_RTP_PORT_DEFAULT)
			vty_out(vty, " ip voip port %d%s", voip_stream->l_rtp_port, VTY_NEWLINE);

/*		if(voip_stream->payload != VOIP_RTP_PAYLOAD_DEFAULT)
			vty_out(vty, " ip voip payload %d%s", voip_stream->payload, VTY_NEWLINE);
		if(strlen(voip_stream->payload_name))
		{
			vty_out(vty, " ip voip payload name %s%s", voip_stream->payload_name, VTY_NEWLINE);
		}*/

		if(voip_stream->bitrate)
			vty_out(vty, " ip voip bitrate %d%s", voip_stream->bitrate, VTY_NEWLINE);
		if(voip_stream->jitter != 50)
			vty_out(vty, " ip voip jitter %d%s", voip_stream->jitter, VTY_NEWLINE);
		if(voip_stream->rtcp == FALSE)
			vty_out(vty, " ip voip disable rtcp%s", VTY_NEWLINE);
		if(voip_stream->avpf == FALSE)
			vty_out(vty, " ip voip disable avpf%s", VTY_NEWLINE);

		//echo canceller
		if(voip_stream->echo_canceller)
		{
			vty_out(vty, " ip voip echo-canceller %s", VTY_NEWLINE);
			if(voip_stream->ec_tail)
				vty_out(vty, " ip voip echo-canceller tail %d%s", voip_stream->ec_tail, VTY_NEWLINE);
			if(voip_stream->ec_delay)
				vty_out(vty, " ip voip echo-canceller delay %d%s", voip_stream->ec_delay, VTY_NEWLINE);
			if(voip_stream->ec_framesize)
				vty_out(vty, " ip voip echo-canceller framesize %d%s", voip_stream->ec_framesize, VTY_NEWLINE);
		}
		//echo limiter
		if(voip_stream->echo_limiter)
		{
			vty_out(vty, " ip voip echo-limiter %s", VTY_NEWLINE);
			if(voip_stream->el_force > 0.0)
				vty_out(vty, " ip voip echo-limiter force %.2f%s", voip_stream->el_force, VTY_NEWLINE);
			if(voip_stream->el_speed > 0.0)
				vty_out(vty, " ip voip echo-limiter speed %.2f%s", voip_stream->el_speed, VTY_NEWLINE);
			if(voip_stream->el_thres > 0.0)
				vty_out(vty, " ip voip echo-limiter thres %.2f%s", voip_stream->el_thres, VTY_NEWLINE);
			if(voip_stream->el_transmit > 0.0)
				vty_out(vty, " ip voip echo-limiter transmit-thres %.2f%s", voip_stream->el_transmit, VTY_NEWLINE);
			if(voip_stream->el_sustain)
				vty_out(vty, " ip voip echo-limiter sustain %d%s", voip_stream->el_sustain, VTY_NEWLINE);
		}
		//echo limiter
		if(voip_stream->noise_gate)
		{
			vty_out(vty, " ip voip noise-gate %s", VTY_NEWLINE);
			if(voip_stream->ng_floorgain > 0.0)
				vty_out(vty, " ip voip noise-gate floorgain %.2f%s", voip_stream->ng_floorgain, VTY_NEWLINE);
			if(voip_stream->ng_threshold > 0.0)
				vty_out(vty, " ip voip noise-gate threshold %.2f%s", voip_stream->ng_threshold, VTY_NEWLINE);
		}
	}
	return OK;
}

/*
 *  ./mediastream --local 5555 --remote 192.168.1.251:5555 --payload 8 --verbose
 */

static int voip_stream_start_api(voip_stream_t *args)
{
	zassert(args != NULL);
	if(((voip_task_t*)(voip_app->voip_task))->pVoid == NULL)
		((voip_task_t*)(voip_app->voip_task))->pVoid = voip_stream_init_default();
	((voip_task_t*)(voip_app->voip_task))->enable = TRUE;
	((voip_task_t*)(voip_app->voip_task))->active = TRUE;
	((voip_task_t*)(voip_app->voip_task))->stream = TRUE;
	//VOIP_EV_DEBUG( "===================%s:", __func__);
	if(((voip_task_t*)(voip_app->voip_task))->sem)
		os_sem_give(((voip_task_t*)(voip_app->voip_task))->sem);
	return OK;
}

int voip_stream_stop_api(void)
{
	//int n= 1;
	//voip_app->voip_task->active = FALSE;
	//voip_app->voip_task->stream = FALSE;
	voip_socket_quit();
	return OK;
}

int voip_stream_stop_force_api(void)
{
	//int n= 1;
	//voip_app->voip_task->active = FALSE;
	//voip_app->voip_task->stream = FALSE;
#ifdef PL_VOIP_MEDIASTREAM
	mediastream_stop_force();
#endif
	return OK;
}

int voip_stream_stop_flag_api(void)
{
	//voip_app->voip_task->active = FALSE;
	((voip_task_t*)(voip_app->voip_task))->stream = FALSE;
	return OK;
}

/**************************************************************************************/
int voip_stream_create_and_start_api(voip_stream_remote_t *config)
{

	zassert(voip_stream != NULL);
	zassert(config != NULL);

	voip_stream_remote_address_port_api(voip_stream, config->r_rtp_address, config->r_rtp_port);
#ifdef PL_VOIP_MEDIASTREAM
	voip_stream->mediastream->payload = config->r_payload;
#endif
	//voip_stream_payload_type_api(voip_stream, NULL, config->r_payload/*voip_sip_get_payload_index()*/);

/*
	voip_stream_echo_canceller_api(voip_stream, voip_stream->echo_canceller, voip_stream->ec_tail,
			voip_stream->ec_delay, voip_stream->ec_framesize);
	voip_stream_echo_limiter_api(voip_stream, voip_stream->echo_limiter, voip_stream->el_force,
			voip_stream->el_speed, voip_stream->el_sustain, voip_stream->el_thres, voip_stream->el_transmit);

	voip_stream_noise_gate_api(voip_stream, voip_stream->noise_gate,
			voip_stream->ng_floorgain, voip_stream->ng_threshold);
*/
	//VOIP_EV_DEBUG( "===================%s:", __func__);
	return voip_stream_start_api(voip_stream);
}

/*int voip_stream_remote_get_api(voip_stream_remote_t* args)
{
	zassert(args != NULL);
	if(!args)
		return ERROR;
#if 0
	args->r_rtp_port = 5555;
	strcpy(args->r_rtp_address, "192.168.1.1");
#else
	if(voip_stream)
	{
		args->r_rtp_port = voip_stream->r_rtp_port;
		strcpy(args->r_rtp_address, voip_stream->r_rtp_address);
		return OK;
	}
#endif
	return ERROR;
}*/

#ifdef VOIP_STREAM_RAW
static BOOL voip_stream_parse_args_api(int argc, char** argv, voip_stream_t* out)
{
	zassert(out != NULL);
	if(!out)
		return 0;
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_parse_args(argc, argv, out->mediastream);
#else
	return 1;
#endif
}

int voip_stream_shell_start_api(int argc, char * argv[])
{
	if(((voip_task_t*)(voip_app->voip_task))->pVoid == NULL)
		((voip_task_t*)(voip_app->voip_task))->pVoid = voip_stream_init_default();
	if (!voip_stream_parse_args_api(argc, argv, ((voip_task_t*)(voip_app->voip_task))->pVoid))
	{
		return ERROR;
	}
	((voip_task_t*)(voip_app->voip_task))->enable = TRUE;
	((voip_task_t*)(voip_app->voip_task))->active = TRUE;
	((voip_task_t*)(voip_app->voip_task))->stream = TRUE;
	if(((voip_task_t*)(voip_app->voip_task))->sem)
		os_sem_give(((voip_task_t*)(voip_app->voip_task))->sem);
	return OK;
}

int _voip_stream_start_api_shell(char *address, int local, int remote)
{
	voip_stream_remote_t config;
	zassert(voip_stream != NULL);
	memset(&config, 0, sizeof(config));
	config.r_rtp_port = remote ? remote:5555;
	voip_stream->l_rtp_port = local ? local:5555;
	strcpy(config.r_rtp_address, address);
	return voip_stream_create_and_start_api(&config);
}
#endif

#ifdef VOIP_STREAM_UNIT_TESTING

typedef struct x5b_unit_test_s
{
	BOOL	X5CM;
	void	*master;
	int		r_fd;
	int		w_fd;
	void	*r_thread;
	void	*t_thread;

	char		*local_address;
	u_int16		local_port;
	u_int16		interval;
	u_int16 	cnt;
	BOOL		state;
	char		buf[1024];
	int			len;
}x5b_unit_test_t;

static x5b_unit_test_t x5b_unit_test;

static int x5b_unit_test_read_eloop(struct eloop *eloop);
static int x5b_unit_test_timer_eloop(struct eloop *eloop);

static int x5b_unit_test_socket_init(x5b_unit_test_t *mgt)
{
	zassert(mgt != NULL);
	if(mgt->r_fd > 0)
		return OK;
	int fd = sock_create(FALSE);
	if(fd)
	{
		//memset(mgt, 0, sizeof(x5b_unit_test_t));
		if(mgt->local_port == 0)
			mgt->local_port = 9999;
		if(sock_bind(fd, mgt->local_address, mgt->local_port) == OK)
		{
			mgt->r_fd = fd;
			mgt->w_fd = fd;

			setsockopt_so_recvbuf (fd, 8192);
			setsockopt_so_sendbuf (fd, 8192);
			setsockopt_ipv4_multicast_loop(fd, 0);
			sockopt_broadcast(fd);
			mgt->t_thread = eloop_add_timer(mgt->master, x5b_unit_test_timer_eloop, mgt, mgt->interval);
			mgt->r_thread = eloop_add_read(mgt->master, x5b_unit_test_read_eloop, mgt, fd);
			return OK;
		}
		else
		{
			zlog_err(ZLOG_APP, "Can not bind UDP socket(:%s)", strerror(errno));
		}
	}
	zlog_err(ZLOG_APP, "Can not Create UDP socket(:%s)", strerror(errno));
	return ERROR;
}

static int x5b_unit_test_socket_exit(x5b_unit_test_t *mgt)
{
	if(mgt && mgt->r_thread)
	{
		eloop_cancel(mgt->r_thread);
		mgt->r_thread = NULL;
	}
	if(mgt && mgt->t_thread)
	{
		eloop_cancel(mgt->t_thread);
		mgt->t_thread = NULL;
	}
	if(mgt)
	{
		if(mgt->r_fd)
			close(mgt->r_fd);
		mgt->r_fd = 0;
		memset(mgt->buf, 0, sizeof(mgt->buf));
		mgt->r_fd = 0;
		mgt->w_fd = 0;
/*		mgt->state = 0;*/
		return OK;
	}
	return ERROR;
}


static int x5b_unit_test_timer_eloop(struct eloop *eloop)
{
	zassert(eloop != NULL);
	char sbuf[512];
	x5b_unit_test_t *mgt = ELOOP_ARG(eloop);
	if(mgt->cnt)
		mgt->cnt--;

	if(mgt->w_fd)
	{
		memset(sbuf, 0, sizeof(sbuf));
		sprintf(sbuf, "%s", "x5b unit test keepalive");
		sock_client_write(mgt->w_fd, "192.168.2.255", mgt->local_port, sbuf, 64);
	}
	mgt->t_thread = eloop_add_timer(mgt->master, x5b_unit_test_timer_eloop, mgt, mgt->interval);

	if(mgt->cnt <= 0)
	{
		if(mgt->state)
		{
			mgt->state = FALSE;
			zlog_debug(ZLOG_VOIP, "=========== STOP ==========");
			voip_stream_stop_api();
		}
	}
	return OK;
}


static int x5b_unit_test_read_eloop(struct eloop *eloop)
{
	int sock_len, len = 0;
	struct sockaddr_in from;
	u_int32 address;
	zassert(eloop != NULL);
	x5b_unit_test_t *mgt = ELOOP_ARG(eloop);
	zassert(mgt != NULL);
	int sock = ELOOP_FD(eloop);
	mgt->r_thread = NULL;
	memset(mgt->buf, 0, sizeof(mgt->buf));
	sock_len = sizeof(struct sockaddr_in);

	x5b_app_local_address_get(&address);
	len = recvfrom(sock, mgt->buf, sizeof(mgt->buf), 0, &from, &sock_len);
	if (len <= 0)
	{
		if (len < 0)
		{
			if (ERRNO_IO_RETRY(errno))
			{
				zlog_err(ZLOG_APP, "RECV mgt on socket (%s)", strerror(errno));
				x5b_unit_test_socket_exit(mgt);
				x5b_unit_test_socket_init(mgt);
				return OK;
			}
		}
	}
	else
	{
		mgt->len = len;
		if(ntohl(from.sin_addr.s_addr) != address)
		{
			//zlog_debug(ZLOG_VOIP, "=========== recv %s from %s", mgt->buf, inet_address(ntohl(from.sin_addr.s_addr)));
			if(strstr(mgt->buf, "keepalive"))
			{
				mgt->cnt = 10;
				if(!mgt->state)
				{
					mgt->state = TRUE;
					zlog_debug(ZLOG_VOIP, "========= START ==========");
					//inet_address(ntohl(mgt->from.sin_addr.s_addr)
					_voip_stream_start_api_shell(inet_address(ntohl(from.sin_addr.s_addr)), 6666, 6666);
				}
			}
		}
	}
	mgt->r_thread = eloop_add_read(mgt->master, x5b_unit_test_read_eloop, mgt, sock);
	return OK;
}


int x5b_unit_test_init()
{
	memset(&x5b_unit_test, 0, sizeof(x5b_unit_test_t));
	if(access("/tmp/app/unit-test", 0) == 0)
	{
		if(master_eloop[MODULE_APP_START] == NULL)
			master_eloop[MODULE_APP_START] = eloop_master_module_create(MODULE_APP_START);
		x5b_unit_test.master = master_eloop[MODULE_APP_START];
		x5b_unit_test.interval = 2;
		x5b_unit_test_socket_init(&x5b_unit_test);
	}
	return OK;
}

#endif
