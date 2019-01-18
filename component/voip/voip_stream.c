/*
 * voip_stream.c
 *
 *  Created on: Jan 6, 2019
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
#include "vty.h"

#include "voip_def.h"
#include "voip_task.h"
#include "voip_stream.h"
#include "voip_socket.h"

voip_stream_t *voip_stream = NULL;

void *voip_lookup_stream_api()
{
	return voip_stream;
}

static voip_stream_t* voip_stream_init_default(void)
{
	voip_stream_t *tmp = NULL;
	if(!tmp)
	{
		tmp = malloc(sizeof(voip_stream_t));
		if(!tmp)
			return NULL;
		memset(tmp, 0, sizeof(voip_stream_t));
		if(voip_task.pVoid == NULL)
			voip_task.pVoid = tmp;
	}
#ifdef PL_VOIP_MEDIASTREAM
	tmp->mediastream = mediastream_init_default();
	if(!tmp->mediastream)
	{
		free(voip_task.pVoid);
		voip_task.pVoid = NULL;
		tmp = NULL;
		return NULL;
	}
#endif

	tmp->enable = TRUE;
	tmp->l_rtp_port = VOIP_RTP_PORT_DEFAULT;
	tmp->payload = VOIP_RTP_PAYLOAD_DEFAULT;


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
	memset(&voip_task, 0, sizeof(voip_task));
/*	voip_stream = malloc(sizeof(voip_stream_t));
	memset(voip_stream, 0, sizeof(voip_stream_t));*/
	voip_stream = voip_stream_init_default();
	if(!voip_stream)
		return ERROR;
	if(voip_task.pVoid == NULL)
		voip_task.pVoid = voip_stream;
	voip_streams_hw_init(voip_stream);
	return OK;
}

int voip_stream_module_exit()
{
	if(voip_task.pVoid != NULL)
	{
		voip_streams_hw_exit(voip_stream);
		free(voip_task.pVoid);
		voip_task.pVoid = NULL;
		voip_stream = NULL;
	}
	return OK;
}


BOOL voip_stream_parse_args_api(int argc, char** argv, voip_stream_t* out)
{
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_parse_args(argc, argv, out->mediastream);
#else
	return 1;
#endif
}

int voip_stream_local_port_api(voip_stream_t* out, u_int16 port)
{
#ifdef PL_VOIP_MEDIASTREAM
	out->mediastream->localport = port ? port:VOIP_RTP_PORT_DEFAULT;
#endif
	out->l_rtp_port = port ? port:VOIP_RTP_PORT_DEFAULT;
	return OK;
}

int voip_stream_remote_address_port_api(voip_stream_t* out, char *remote, u_int16 port)
{
#ifdef PL_VOIP_MEDIASTREAM
	out->mediastream->remoteport = port;
	memset(out->mediastream->ip, 0, sizeof(out->mediastream->ip));
	if(remote)
		strcpy(out->mediastream->ip, remote);
#endif
	out->r_rtp_port = port;
	memset(out->r_rtp_address, 0, sizeof(out->r_rtp_address));
	if(remote)
		strcpy(out->r_rtp_address, remote);
	return OK;
}

int voip_stream_payload_type_api(voip_stream_t* out, char *name, int payload)
{
#ifdef PL_VOIP_MEDIASTREAM
	out->mediastream->payload = (payload >= 0) ? payload:VOIP_RTP_PAYLOAD_DEFAULT;
	if(payload == -1)
	{
		out->mediastream->payload = 114;
		out->mediastream->custom_pt=ms_tools_parse_custom_payload(name);
	}
#endif
	out->payload = (payload >= 0) ? payload:VOIP_RTP_PAYLOAD_DEFAULT;
	memset(out->payload_name, 0, sizeof(out->payload_name));
	if(name)
		strcpy(out->payload_name, name);
	return OK;
}

int voip_stream_echo_canceller_api(voip_stream_t* out, BOOL enable, int ec_tail, int ec_delay, int ec_framesize)
{
#ifdef PL_VOIP_MEDIASTREAM
	out->mediastream->ec = enable;
	out->mediastream->ec_len_ms = ec_tail;
	out->mediastream->ec_delay_ms = ec_delay;
	out->mediastream->ec_framesize = ec_framesize;
#endif
	out->echo_canceller = enable;
	out->ec_tail = ec_tail;
	out->ec_delay = ec_delay;
	out->ec_framesize = ec_framesize;
	return OK;
}

int voip_stream_echo_limiter_api(voip_stream_t* out, BOOL enable, float force, float speed, int sustain, float thres, float transmit)
{
#ifdef PL_VOIP_MEDIASTREAM
	out->mediastream->el = enable;
	out->mediastream->el_speed = speed;
	out->mediastream->el_force = force;
	out->mediastream->el_sustain = sustain;

	out->mediastream->el_thres = thres;
	out->mediastream->el_transmit_thres = transmit;
#endif

	out->echo_limiter = enable;
	out->el_speed = speed;
	out->el_force = force;
	out->el_sustain = sustain;
	out->el_thres = thres;
	out->el_transmit = transmit;
	return OK;
}

int voip_stream_bitrate_api(voip_stream_t* out, int bitrate)
{
#ifdef PL_VOIP_MEDIASTREAM
	out->mediastream->bitrate = bitrate;
#endif
	out->bitrate = bitrate;
	return OK;
}

int voip_stream_jitter_api(voip_stream_t* out, int jitter)
{
#ifdef PL_VOIP_MEDIASTREAM
	out->mediastream->jitter = jitter;
#endif
	out->jitter = jitter;
	return OK;
}

int voip_stream_disable_rtcp_api(voip_stream_t* out)
{
#ifdef PL_VOIP_MEDIASTREAM
	out->mediastream->enable_rtcp = FALSE;
#endif
	out->rtcp = FALSE;
	return OK;
}

int voip_stream_disable_avpf_api(voip_stream_t* out)
{
#ifdef PL_VOIP_MEDIASTREAM
	out->mediastream->enable_avpf = FALSE;
#endif
	out->avpf = FALSE;
	return OK;
}

int voip_stream_noise_gate_api(voip_stream_t* out, BOOL enable, float ng_floorgain, float ng_threshold)
{
#ifdef PL_VOIP_MEDIASTREAM
	out->mediastream->use_ng = enable;
	out->mediastream->ng_threshold=ng_threshold;
	out->mediastream->ng_floorgain=ng_floorgain;
#endif
	out->noise_gate = enable;
	out->ng_threshold=ng_threshold;
	out->ng_floorgain=ng_floorgain;
	return OK;
}

/*
"[ --ice-local-candidate <ip:port:[host|srflx|prflx|relay]> ]\n"
"[ --ice-remote-candidate <ip:port:[host|srflx|prflx|relay]> ]\n"
*/



int voip_stream_ctlfd_get_api(void)
{
	return voip_socket.mdctl;
}

void voip_stream_setup_api(voip_stream_t* args)
{
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_setup(args->mediastream);
#else
	return;
#endif
}

void voip_stream_running_api(voip_stream_t* args)
{
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
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_clear(args->mediastream);
#else
	return;
#endif
}

void *voip_stream_lookup_factory_api(voip_stream_t *args)
{
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_lookup_factory(args->mediastream);
#else
	return args->factory;
#endif
}

int voip_streams_hw_init(voip_stream_t *args)
{
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_hw_init(args->mediastream);
#else
	return OK;
#endif
}

int voip_streams_hw_exit(voip_stream_t *args)
{
#ifdef PL_VOIP_MEDIASTREAM
	return mediastream_hw_exit(args->mediastream);
#else
	return OK;
#endif
}


int voip_stream_debug_set_api(BOOL enable, char *debug)
{
#ifdef PL_VOIP_MEDIASTREAM
	if(strstr(debug, "all"))
	{
		bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
	}
	if(strstr(debug, "trace"))
	{
		bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
	}
	if(strstr(debug, "message"))
	{
		bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
	}
	if(strstr(debug, "warning"))
	{
		bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
	}
	if(strstr(debug, "error"))
	{
		bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
	}
	if(strstr(debug, "fatal"))
	{
		bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
	}
#endif
	return OK;
}

BOOL voip_stream_is_enable()
{
	if(voip_stream && voip_stream->enable)
		return TRUE;
	else
		return FALSE;
}

int voip_stream_remote_get_api(voip_stream_remote_t* args)
{
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
}

int voip_stream_show_config(struct vty *vty)
{
	if(voip_stream)
	{

		vty_out(vty, " voip stream                        : %s%s", voip_stream->enable ? "ENABLE":"DISABLE", VTY_NEWLINE);
		vty_out(vty, " voip stream rtp-port               : %d%s", voip_stream->l_rtp_port, VTY_NEWLINE);
		vty_out(vty, " voip stream payload                : %d%s", voip_stream->payload, VTY_NEWLINE);
		if(strlen(voip_stream->payload_name))
			vty_out(vty, " voip stream payload name           : %s%s", voip_stream->payload_name, VTY_NEWLINE);

		if(strlen(voip_stream->r_rtp_address))
		{
			vty_out(vty, " voip stream remote                 : %s:%d%s",
					voip_stream->r_rtp_address, voip_stream->r_rtp_port, VTY_NEWLINE);
		}
		vty_out(vty, " voip stream bitrate                : %d%s", voip_stream->bitrate, VTY_NEWLINE);

		vty_out(vty, " voip stream jitter                 : %d%s", voip_stream->jitter, VTY_NEWLINE);

		vty_out(vty, " voip stream rtcp                   : %s%s", voip_stream->rtcp ? "TRUE":"FALSE",VTY_NEWLINE);

		vty_out(vty, " voip stream avpf                   : %s%s", voip_stream->avpf ? "TRUE":"FALSE", VTY_NEWLINE);
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
	//	voip_mediastream_show_config(voip_stream->mediastream, vty);
#endif
	}
	return OK;
}

int voip_stream_write_config(struct vty *vty)
{
	if(voip_stream && voip_stream->enable)
	{
		if(voip_stream->l_rtp_port != VOIP_RTP_PORT_DEFAULT)
			vty_out(vty, " ip voip port %d%s", voip_stream->l_rtp_port, VTY_NEWLINE);

		if(voip_stream->payload != VOIP_RTP_PAYLOAD_DEFAULT)
			vty_out(vty, " ip voip payload %d%s", voip_stream->payload, VTY_NEWLINE);
		if(strlen(voip_stream->payload_name))
		{
			vty_out(vty, " ip voip payload name %s%s", voip_stream->payload_name, VTY_NEWLINE);
		}

		if(voip_stream->bitrate)
			vty_out(vty, " ip voip bitrate %d%s", voip_stream->bitrate, VTY_NEWLINE);
		if(voip_stream->jitter != 50)
			vty_out(vty, " ip voip jitter %d%s", voip_stream->jitter, VTY_NEWLINE);
		if(!voip_stream->rtcp)
			vty_out(vty, " ip voip disable rtcp%s", VTY_NEWLINE);
		if(!voip_stream->avpf)
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

int voip_stream_start_api(voip_stream_t *args)
{
	if(voip_task.pVoid == NULL)
		voip_task.pVoid = voip_stream_init_default();
	voip_task.enable = TRUE;
	voip_task.active = TRUE;
	voip_task.stream = TRUE;
	return OK;
}

int voip_stream_shell_start_api(int argc, char * argv[])
{
	if(voip_task.pVoid == NULL)
		voip_task.pVoid = voip_stream_init_default();
	if (!voip_stream_parse_args_api(argc, argv, voip_task.pVoid))
	{
		return ERROR;
	}
	voip_task.enable = TRUE;
	voip_task.active = TRUE;
	voip_task.stream = TRUE;
	return OK;
}


int voip_stream_stop_api(void)
{
	//int n= 1;
	voip_task.active = FALSE;
	voip_task.stream = FALSE;
	voip_socket_quit(&voip_socket);
	return OK;
}

int voip_stream_stop_force_api(void)
{
	//int n= 1;
	voip_task.active = FALSE;
	voip_task.stream = FALSE;
#ifdef PL_VOIP_MEDIASTREAM
	mediastream_stop_force();
#endif
	return OK;
}


#ifdef VOIP_STREAM_DEBUG_TEST
/*
 * just for shell
 */
int _voip_stream_start_api_shell(char *address, int local, int remote)
{
	int i = 0;
	int argc = 7;
	char locals[64];
	char remotes[64];
	char payload[32];

	//char * argv[10] = {"mediastream", "--local", "5555", "--remote", "192.168.1.251:5555",
	//		"--payload", "8", "--verbose", "--interactive", "--ec", NULL, NULL, "--ec-framesize", "160", NULL};
	char * argv[20] = {"mediastream", "--local", "5555", "--remote", "192.168.1.251:5555",
			"--payload", "8", "--interactive", "--ec", "--el", NULL, "--verbose", NULL, NULL, "--ec-framesize", "160", NULL};

	/*
	"[ --ec (enable echo canceller) ]\n"
	"[ --ec-delay <echo canceller delay in ms> ]\n"
	"[ --ec-framesize <echo canceller framesize in samples> ]\n"
	"[ --ec-tail <echo canceller tail length in ms> ]\n"
	*/

	argc = 10;
	//char * argv[10] = {"mediastream", "--local", "5555", "--remote", "192.168.1.251:5555", "--payload", "8", NULL};
	//char * argv[10] = { "mediastream", "--local", NULL, "--remote", NULL, "--payload", NULL, NULL, NULL };
	if(local)
	{
		memset(locals, 0, sizeof(locals));
		snprintf(locals, sizeof(locals), "%d", local);
		argv[2] = locals;
	}

	if(address)
	{
		memset(remotes, 0, sizeof(remotes));
		snprintf(remotes, sizeof(remotes), "%s:%d", address, remote ? remote:5555);
		argv[4] = remotes;
	}
	memset(payload, 0, sizeof(payload));
	snprintf(payload, sizeof(payload), "%d", 8);
	argv[6] = payload;

	printf(" \n");
	for(i = 0; i < 20; i++)
	{
		if(argv[i] != NULL)
			printf(" %s" ,argv[i]);
	}
	printf(" \n");

	if(argv[2] && argv[4] && argv[6])
		return voip_stream_shell_start_api( argc, argv);
	return ERROR;
}
#endif
