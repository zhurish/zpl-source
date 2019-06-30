/*
 * voip_mediastream.c
 *
 *  Created on: 2018年12月27日
 *      Author: DELL
 */

/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <zebra.h>

#ifdef PL_VOIP_MEDIASTREAM
#include <math.h>
//#include "common.h"
#include "mediastreamer2/msequalizer.h"
#include "mediastreamer2/msvolume.h"
#ifdef VIDEO_ENABLED
#include "mediastreamer2/msv4l.h"
#endif

#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <ortp/ortp.h>
#include <ortp/b64.h>


#ifdef HAVE_CONFIG_H
//#include "mediastreamer2/mediastreamer-config.h"
#endif

#endif /* PL_VOIP_MEDIASTREAM */

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
#include "voip_util.h"
#include "voip_state.h"
#include "voip_stream.h"
#include "voip_mediastream.h"

#ifdef PL_VOIP_MEDIASTREAM


static int mediastream_running_flag = 1;

// HELPER METHODS
//static void stop_handler(int signum);

static bool_t parse_addr(const char *addr, char *ip, size_t len, int *port);
static bool_t parse_ice_addr(char* addr, char* type, size_t type_len, char* ip, size_t ip_len, int* port);
static void display_items(void *user_data, uint32_t csrc, rtcp_sdes_type_t t, const char *content, uint8_t content_len);
static void parse_rtcp(mblk_t *m);
static void parse_events(mediastream_global* args, RtpSession *session, OrtpEvQueue *q);
static bool_t parse_window_ids(const char *ids, int* video_id, int* preview_id);
static rcalgo parse_rc_algo(const char *algo);

static int mediastream_tool_iterate_flush(mediastream_global* args);

#if 0
static const char *usage="mediastream --local <port>\n"
								"--remote <ip:port> \n"
								"[--help (display this help) ]\n"
								"[--payload <payload type number or payload name like 'audio/pmcu/8000'> ]\n"
								"[ --agc (enable automatic gain control) ]\n"
								"[ --bitrate <bits per seconds> ]\n"
								"[ --camera <camera id as listed at startup> ]\n"
								"[ --capture-card <name> ]\n"
								"[ --ec (enable echo canceller) ]\n"
								"[ --ec-delay <echo canceller delay in ms> ]\n"
								"[ --ec-framesize <echo canceller framesize in samples> ]\n"
								"[ --ec-tail <echo canceller tail length in ms> ]\n"
								"[ --el (enable echo limiter) ]\n"
								"[ --el-force <(float) [0-1]> (The proportional coefficient controlling the mic attenuation) ]\n"
								"[ --el-speed <(float) [0-1]> (gain changes are smoothed with a coefficent) ]\n"
								"[ --el-sustain <(int)> (Time in milliseconds for which the attenuation is kept unchanged after) ]\n"
								"[ --el-thres <(float) [0-1]> (Threshold above which the system becomes active) ]\n"
								"[ --el-transmit-thres <(float) [0-1]> (TO BE DOCUMENTED) ]\n"
								"[ --fmtp <fmtpline> ]\n"
								"[ --recv_fmtp <fmtpline passed to decoder> ]\n"
								"[ --freeze-on-error (for video, stop upon decoding error until next valid frame) ]\n"
								"[ --height <pixels> ]\n"
		//stun:stun.l.google.com:19302
								"[ --ice-local-candidate <ip:port:[host|srflx|prflx|relay]> ]\n"
								"[ --ice-remote-candidate <ip:port:[host|srflx|prflx|relay]> ]\n"
								"[ --infile <input wav file> specify a wav file to be used for input, instead of soundcard ]\n"
								"[ --interactive (run in interactive mode) ]\n"
								"[ --jitter <miliseconds> ]\n"
								"[ --log <file> ]\n"
								"[ --mtu <mtu> (specify MTU)]\n"
								"[ --netsim-bandwidth <bandwidth limit in bits/s> (simulates a network download bandwidth limit) ]\n"
								"[ --netsim-consecutive-loss-probability <0-1> (to simulate bursts of lost packets) ]\n"
								"[ --netsim-jitter-burst-density <0-10> (density of gap/burst events, 1.0=one gap/burst per second in average) ]\n"
								"[ --netsim-jitter-strength <0-100> (strength of the jitter simulation) ]\n"
								"[ --netsim-latency <latency in ms> (simulates a network latency) ]\n"
								"[ --netsim-lossrate <0-100> (simulates a network lost rate) ]\n"
								"[ --netsim-mode inbound|outboud (whether network simulation is applied to incoming (default) or outgoing stream) ]\n"
								"[ --ng (enable noise gate)] \n"
								"[ --ng-floorgain <(float) [0-1]> (gain applied to the signal when its energy is below the threshold.) ]\n"
								"[ --ng-threshold <(float) [0-1]> (noise gate threshold) ]\n"
								"[ --no-avpf ]\n"
								"[ --no-rtcp ]\n"
								"[ --outfile <output wav file> specify a wav file to write audio into, instead of soundcard ]\n"
								"[ --playback-card <name> ]\n"
								"[ --rc <rate control algorithm> possible values are: none, simple, advanced ]\n"
								"[ --srtp <local master_key> <remote master_key> (enable srtp, master key is generated if absent from comand line) ]\n"
								"[ --verbose (most verbose messages) ]\n"
								"[ --video-display-filter <name> ]\n"
								"[ --video-windows-id <video surface:preview surface >]\n"
								"[ --width <pixels> ]\n"
								"[ --zoom zoom factor ]\n"
								"[ --zrtp (enable zrtp) ]\n"
								#if TARGET_OS_IPHONE
								"[ --speaker route audio to speaker ]\n"
								#endif
								;
#endif

mediastream_global* mediastream_init_default(void) {
	mediastream_global* args = (mediastream_global*)ms_malloc0(sizeof(mediastream_global));
	mediastream_running_flag = 1;
	args->localport=0;
	args->remoteport=0;
	args->payload=0;
	memset(args->ip, 0, sizeof(args->ip));
	args->send_fmtp=NULL;
	args->recv_fmtp=NULL;
	args->jitter=50;
	args->bitrate=0;
	args->ec=FALSE;
	args->agc=FALSE;
	args->eq=FALSE;
	args->interactive=FALSE;
	args->is_verbose=FALSE;
	//args->is_verbose=TRUE;
	args->device_rotation=-1;
	args->rc_algo = rcalgoNone;

#ifdef VIDEO_ENABLED
	args->video=NULL;
#endif
	args->capture_card=NULL;
	args->playback_card=NULL;
	args->camera=NULL;
	args->infile=args->outfile=NULL;
	args->ng_threshold=-1;
	args->use_ng=FALSE;
	args->two_windows=FALSE;
	args->el=FALSE;
	args->el_speed=-1;
	args->el_thres=-1;
	args->el_force=-1;
	args->el_sustain=-1;
	args->el_transmit_thres=-1;
	args->ng_floorgain=-1;
	args->enable_zrtp =FALSE;
	args->custom_pt=NULL;
	args->video_window_id = -1;
	args->preview_window_id = -1;
	args->enable_avpf = TRUE;
	args->enable_rtcp = TRUE;
	/* starting values echo canceller */
	args->ec_len_ms=args->ec_delay_ms=args->ec_framesize=0;

	args->enable_srtp = FALSE;
	args->srtp_local_master_key = args->srtp_remote_master_key = NULL;
	args->zoom = 1.0;
	args->zoom_cx = args->zoom_cy = 0.5;

	args->audio = NULL;
	args->session = NULL;
	args->pt = NULL;
	args->q = NULL;
	args->profile = NULL;
	args->logfile = NULL;

	args->ice_session = NULL;
	memset(args->ice_local_candidates, 0, sizeof(args->ice_local_candidates));
	memset(args->ice_remote_candidates, 0, sizeof(args->ice_remote_candidates));
	args->ice_local_candidates_nb = args->ice_remote_candidates_nb = 0;
	args->video_display_filter=NULL;
	args->vs.width=MS_VIDEO_SIZE_CIF_W;
	args->vs.height=MS_VIDEO_SIZE_CIF_H;
	args->interactive=TRUE;
	return args;
}

bool_t mediastream_parse_args(int argc, char** argv, mediastream_global* out) {
	int i;

	if (argc<4) {
		ms_error("Expected at least 3 arguments.\n");
		return FALSE;
	}

	/* default size */
	out->vs.width=MS_VIDEO_SIZE_CIF_W;
	out->vs.height=MS_VIDEO_SIZE_CIF_H;

	for (i=1;i<argc;i++){
		if (strcmp(argv[i],"--help")==0 || strcmp(argv[i],"-h")==0) {
			return FALSE;
		}else if (strcmp(argv[i],"--local")==0){
			char *is_invalid;
			i++;
			out->localport = strtol(argv[i],&is_invalid,10);
			if (*is_invalid!='\0'){
				ms_error("Failed to parse local port '%s'\n",argv[i]);
				return 0;
			}
		}else if (strcmp(argv[i],"--remote")==0){
			i++;
			if (!parse_addr(argv[i],out->ip,sizeof(out->ip),&out->remoteport)) {
				ms_error("Failed to parse remote address '%s'\n",argv[i]);
				return FALSE;
			}
			ms_message("Remote addr: ip=%s port=%i\n",out->ip,out->remoteport);
		}else if (strcmp(argv[i],"--ice-local-candidate")==0) {
			mediastreamIce_candidate *candidate;
			i++;
			if (out->ice_local_candidates_nb>=MEDIASTREAM_MAX_ICE_CANDIDATES) {
				ms_warning("Ignore ICE local candidate \"%s\" (maximum %d candidates allowed)\n",argv[i],MEDIASTREAM_MAX_ICE_CANDIDATES);
				continue;
			}
			//stun:stun.l.google.com:19302
			candidate=&out->ice_local_candidates[out->ice_local_candidates_nb];
			if (!parse_ice_addr(argv[i],candidate->type,sizeof(candidate->type),candidate->ip,sizeof(candidate->ip),&candidate->port)) {
				ms_error("Failed to parse ICE local candidates '%s'\n", argv[i]);
				return FALSE;
			}
			out->ice_local_candidates_nb++;
			ms_message("ICE local candidate: type=%s ip=%s port=%i\n",candidate->type,candidate->ip,candidate->port);
		}else if (strcmp(argv[i],"--ice-remote-candidate")==0) {
			mediastreamIce_candidate *candidate;
			i++;
			if (out->ice_remote_candidates_nb>=MEDIASTREAM_MAX_ICE_CANDIDATES) {
				ms_warning("Ignore ICE remote candidate \"%s\" (maximum %d candidates allowed)\n",argv[i],MEDIASTREAM_MAX_ICE_CANDIDATES);
				continue;
			}
			candidate=&out->ice_remote_candidates[out->ice_remote_candidates_nb];
			if (!parse_ice_addr(argv[i],candidate->type,sizeof(candidate->type),candidate->ip,sizeof(candidate->ip),&candidate->port)) {
				ms_error("Failed to parse ICE remote candidates '%s'\n", argv[i]);
				return FALSE;
			}
			out->ice_remote_candidates_nb++;
			ms_message("ICE remote candidate: type=%s ip=%s port=%i\n",candidate->type,candidate->ip,candidate->port);
		}else if (strcmp(argv[i],"--payload")==0){
			i++;
			if (isdigit(argv[i][0])){
				out->payload=atoi(argv[i]);
			}else {
				out->payload=114;
				out->custom_pt=ms_tools_parse_custom_payload(argv[i]);
			}
		}else if (strcmp(argv[i],"--fmtp")==0){
			i++;
			out->send_fmtp=argv[i];
		}else if (strcmp(argv[i],"--recv_fmtp")==0){
			i++;
			out->recv_fmtp=argv[i];
		}else if (strcmp(argv[i],"--jitter")==0){
			i++;
			out->jitter=atoi(argv[i]);
		}else if (strcmp(argv[i],"--bitrate")==0){
			i++;
			out->bitrate=atoi(argv[i]);
		}else if (strcmp(argv[i],"--width")==0){
			i++;
			out->vs.width=atoi(argv[i]);
		}else if (strcmp(argv[i],"--height")==0){
			i++;
			out->vs.height=atoi(argv[i]);
		}else if (strcmp(argv[i],"--capture-card")==0){
			i++;
			out->capture_card=argv[i];
		}else if (strcmp(argv[i],"--playback-card")==0){
			i++;
			out->playback_card=argv[i];
		}else if (strcmp(argv[i],"--ec")==0){
			out->ec=TRUE;
		}else if (strcmp(argv[i],"--ec-tail")==0){
			i++;
			out->ec_len_ms=atoi(argv[i]);
		}else if (strcmp(argv[i],"--ec-delay")==0){
			i++;
			out->ec_delay_ms=atoi(argv[i]);
		}else if (strcmp(argv[i],"--ec-framesize")==0){
			i++;
			out->ec_framesize=atoi(argv[i]);
		}else if (strcmp(argv[i],"--agc")==0){
			out->agc=TRUE;
		}else if (strcmp(argv[i],"--eq")==0){
			out->eq=TRUE;
		}else if (strcmp(argv[i],"--ng")==0){
			out->use_ng=1;
		}else if (strcmp(argv[i],"--rc")==0){
			i++;
			out->rc_algo = parse_rc_algo(argv[i]);
			if (out->rc_algo == rcalgoInvalid){
				ms_error("Invalid argument for --rc");
				return FALSE;
			}
		}else if (strcmp(argv[i],"--ng-threshold")==0){
			i++;
			out->ng_threshold=(float)atof(argv[i]);
		}else if (strcmp(argv[i],"--ng-floorgain")==0){
			i++;
			out->ng_floorgain=(float)atof(argv[i]);
		}else if (strcmp(argv[i],"--two-windows")==0){
			out->two_windows=TRUE;
		}else if (strcmp(argv[i],"--infile")==0){
			i++;
			out->infile=argv[i];
		}else if (strcmp(argv[i],"--outfile")==0){
			i++;
			out->outfile=argv[i];
		}else if (strcmp(argv[i],"--camera")==0){
			i++;
			out->camera=argv[i];
		}else if (strcmp(argv[i],"--el")==0){
			out->el=TRUE;
		}else if (strcmp(argv[i],"--el-speed")==0){
			i++;
			out->el_speed=(float)atof(argv[i]);
		}else if (strcmp(argv[i],"--el-thres")==0){
			i++;
			out->el_thres=(float)atof(argv[i]);
		}else if (strcmp(argv[i],"--el-force")==0){
			i++;
			out->el_force=(float)atof(argv[i]);
		}else if (strcmp(argv[i],"--el-sustain")==0){
			i++;
			out->el_sustain=atoi(argv[i]);
		}else if (strcmp(argv[i],"--el-transmit-thres")==0){
			i++;
			out->el_transmit_thres=(float)atof(argv[i]);
		} else if (strcmp(argv[i],"--zrtp")==0){
			out->enable_zrtp = TRUE;
		} else if (strcmp(argv[i],"--verbose")==0){
			out->is_verbose=TRUE;
		} else if (strcmp(argv[i], "--video-windows-id")==0) {
			i++;
			if (!parse_window_ids(argv[i],&out->video_window_id, &out->preview_window_id)) {
				ms_error("Failed to parse window ids '%s'\n",argv[i]);
				return FALSE;
			}
		} else if (strcmp(argv[i], "--device-rotation")==0) {
			i++;
			out->device_rotation=atoi(argv[i]);
		} else if (strcmp(argv[i], "--srtp")==0) {
			if (!ms_srtp_supported()) {
				ms_error("srtp support not enabled");
				return FALSE;
			}
			out->enable_srtp = TRUE;
			i++;
			// check if we're being given keys
			if (i + 1 < argc) {
				out->srtp_local_master_key = argv[i++];
				out->srtp_remote_master_key = argv[i++];
			}
		} else if (strcmp(argv[i],"--netsim-bandwidth")==0){
			i++;
			if (i<argc){
				out->netsim.max_bandwidth=(float)atoi(argv[i]);
				out->netsim.enabled=TRUE;
			}else{
				ms_error("Missing argument for --netsim-bandwidth");
				return FALSE;
			}
		}else if (strcmp(argv[i],"--netsim-lossrate")==0){
			i++;
			if (i<argc){
				out->netsim.loss_rate=(float)atoi(argv[i]);
				if (out->netsim.loss_rate < 0 || out->netsim.loss_rate>100) {
					ms_error("Loss rate must be between 0 and 100.");
					return FALSE;
				}
				out->netsim.enabled=TRUE;
			}else{
				ms_error("Missing argument for --netsim-lossrate");
				return FALSE;
			}
		}else if (strcmp(argv[i],"--netsim-consecutive-loss-probability")==0){
			i++;
			if (i<argc){
				sscanf(argv[i],"%f",&out->netsim.consecutive_loss_probability);
				if (out->netsim.consecutive_loss_probability < 0 || out->netsim.consecutive_loss_probability>1) {
					ms_error("The consecutive loss probability must be between 0 and 1.");
					return FALSE;
				}

				out->netsim.enabled=TRUE;
			}else{
				ms_error("Missing argument for --netsim-consecutive-loss-probability");
				return FALSE;
			}
		}else if (strcmp(argv[i], "--netsim-latency") == 0) {
			i++;
			if (i<argc){
				out->netsim.latency = atoi(argv[i]);
				out->netsim.enabled=TRUE;
			}else{
				ms_error("Missing argument for --netsim-latency");
				return FALSE;
			}
		}else if (strcmp(argv[i], "--netsim-jitter-burst-density") == 0) {
			i++;
			if (i<argc){
				sscanf(argv[i],"%f",&out->netsim.jitter_burst_density);
				if (out->netsim.jitter_burst_density<0 || out->netsim.jitter_burst_density>10){
					ms_error("The jitter burst density must be between 0 and 10");
					return FALSE;
				}
				out->netsim.enabled=TRUE;
			}else{
				ms_error("Missing argument for --netsim-jitter-burst-density");
				return FALSE;
			}
		}else if (strcmp(argv[i], "--netsim-jitter-strength") == 0) {
			i++;
			if (i<argc){
				sscanf(argv[i],"%f",&out->netsim.jitter_strength);
				if (out->netsim.jitter_strength<0 || out->netsim.jitter_strength>100){
					ms_error("The jitter strength must be between 0 and 100.");
					return FALSE;
				}
				out->netsim.enabled=TRUE;
			}else{
				ms_error("Missing argument for --netsim-jitter-strength");
				return FALSE;
			}
		}else if (strcmp(argv[i], "--netsim-mode") == 0) {
			i++;
			if (i<argc){
				if (strcmp(argv[i],"inbound")==0)
					out->netsim.mode=OrtpNetworkSimulatorInbound;
				else if (strcmp(argv[i],"outbound")==0){
					out->netsim.mode=OrtpNetworkSimulatorOutbound;
				}else{
					ms_error("Invalid value for --netsim-mode");
					return FALSE;
				}
				out->netsim.enabled=TRUE;
			}else{
				ms_error("Missing argument for --netsim-dir");
				return FALSE;
			}
		}else if (strcmp(argv[i],"--zoom")==0){
			i++;
			if (sscanf(argv[i], "%f,%f,%f", &out->zoom, &out->zoom_cx, &out->zoom_cy) != 3) {
				ms_error("Invalid zoom triplet");
				return FALSE;
			}
		} else if (strcmp(argv[i],"--mtu")==0){
			i++;
			if (sscanf(argv[i], "%i", &out->mtu) != 1) {
				ms_error("Invalid mtu value");
				return FALSE;
			}
		} else if (strcmp(argv[i],"--interactive")==0){
			out->interactive=TRUE;
		} else if (strcmp(argv[i], "--no-avpf") == 0) {
			out->enable_avpf = FALSE;
		} else if (strcmp(argv[i], "--no-rtcp") == 0) {
			out->enable_rtcp = FALSE;
		} else if (strcmp(argv[i],"--help")==0) {
			return FALSE;
		} else if (strcmp(argv[i],"--video-display-filter")==0) {
			i++;
			out->video_display_filter=argv[i];
		} else if (strcmp(argv[i], "--log") == 0) {
			i++;
			out->logfile = fopen(argv[i], "a+");
		} else if (strcmp(argv[i], "--freeze-on-error") == 0) {
			out->freeze_on_error=TRUE;
		} else if (strcmp(argv[i], "--speaker") == 0) {
			out->enable_speaker=TRUE;
		} else {
			ms_error("Unknown option '%s'\n", argv[i]);
			return FALSE;
		}
	}
	if (out->netsim.jitter_burst_density>0 && out->netsim.max_bandwidth==0){
		ms_error("Jitter probability settings requires --netsim-bandwidth to be set.");
		return FALSE;
	}
	return TRUE;
}


#ifdef VIDEO_ENABLED
static void video_stream_event_cb(void *user_pointer, const MSFilter *f, const unsigned int event_id, const void *args) {
	mediastream_global *md = (mediastream_global *)user_pointer;
	switch (event_id) {
		case MS_VIDEO_DECODER_DECODING_ERRORS:
			ms_warning("Decoding error on videostream [%p]", md->video);
			break;
		case MS_VIDEO_DECODER_FIRST_IMAGE_DECODED:
			ms_message("First video frame decoded successfully on videostream [%p]", md->video);
			break;
	}
}
#endif

static MSSndCard *get_sound_card(MSSndCardManager *manager, const char* card_name) {
	MSSndCard *play = ms_snd_card_manager_get_card(manager,card_name);
	if (play == NULL) {
		const MSList *list = ms_snd_card_manager_get_list(manager);
		char * cards = ms_strdup("");
		while (list) {
			MSSndCard *card = (MSSndCard*)list->data;
			cards = ms_strcat_printf(cards, "- %s\n", ms_snd_card_get_string_id(card));
			list = list->next;
		}
		ms_fatal("Specified card '%s' but could not find it. Available cards are:\n%s", card_name, cards);
		ms_free(cards);
	}
	return play;
}


void *mediastream_lookup_factory(mediastream_global *args)
{
	return args->factory;
}


int mediastream_hw_init(mediastream_global *args)
{
	MSFactory *factory;
	ortp_init();
	if (args->logfile)
		bctbx_set_log_file(args->logfile);

	if (args->is_verbose) {
		bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
	} else {
		bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_ERROR/*BCTBX_LOG_MESSAGE*/);
	}
	ortp_set_log_handler(md_stream_BctbxLogFunc);
	//bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
	args->factory = factory = ms_factory_new_with_voip();

	if(!args->ctlfd)
	{
		//args->ctlfd = os_pipe_create("mdctl", O_RDWR);
		args->ctlfd = voip_stream_ctlfd_get_api();
		if(args->ctlfd)
		{
			//args->ctlfp = fdopen(args->ctlfd, "r+");
			args->initialization = TRUE;
			args->running = FALSE;
			return OK;
		}
		args->running = FALSE;
		return ERROR;
	}
	args->initialization = TRUE;
	args->running = FALSE;
	return OK;
}

int mediastream_hw_exit(mediastream_global *args)
{
	if (args->bw_controller){
		ms_bandwidth_controller_destroy(args->bw_controller);
	}

	if (args->audio) {
		audio_stream_stop(args->audio);
	}
#ifdef VIDEO_ENABLED
	if (args->video) {
		if (args->video->ms.ice_check_list) ice_check_list_destroy(args->video->ms.ice_check_list);
		video_stream_stop(args->video);
		ms_factory_log_statistics(args->video->ms.factory);
	}
#endif
	if (args->ice_session) ice_session_destroy(args->ice_session);
	ortp_ev_queue_destroy(args->q);

	rtp_profile_destroy(args->profile);

	if (args->logfile)
		fclose(args->logfile);
	args->logfile = NULL;

	ms_factory_destroy(args->factory);

	args->ctlfd = 0;
	//args->ctlfp = NULL;
	args->initialization = FALSE;
	args->running = FALSE;
	//free(args);
	return OK;
}

int mediastream_stop_force()
{
	while(mediastream_running_flag)
	{
		mediastream_running_flag--;
		ms_usleep(10);
	}
	return mediastream_running_flag;
}


static int _mediastream_set_payload(mediastream_global* args)
{
	/*
	rtp_profile_set_payload(profile,0,&payload_type_pcmu8000);		OK
	rtp_profile_set_payload(profile,1,&payload_type_lpc1016);
	rtp_profile_set_payload(profile,3,&payload_type_gsm);
	rtp_profile_set_payload(profile,7,&payload_type_lpc);			OK
	rtp_profile_set_payload(profile,4,&payload_type_g7231);
	rtp_profile_set_payload(profile,8,&payload_type_pcma8000);		OK
	rtp_profile_set_payload(profile,9,&payload_type_g722);			OK
	rtp_profile_set_payload(profile,10,&payload_type_l16_stereo);	OK
	rtp_profile_set_payload(profile,11,&payload_type_l16_mono);		OK
	rtp_profile_set_payload(profile,13,&payload_type_cn);
	rtp_profile_set_payload(profile,18,&payload_type_g729);			OK 726
	rtp_profile_set_payload(profile,31,&payload_type_h261);
	rtp_profile_set_payload(profile,32,&payload_type_mpv);
	rtp_profile_set_payload(profile,34,&payload_type_h263);
	payload_type_opus												OK
	*/
	rtp_profile_set_payload(&av_profile,101,&payload_type_telephone_event);

	rtp_profile_set_payload(&av_profile,105,&payload_type_opus);
	rtp_profile_set_payload(&av_profile,110,&payload_type_speex_nb);
	rtp_profile_set_payload(&av_profile,111,&payload_type_speex_wb);
	rtp_profile_set_payload(&av_profile,112,&payload_type_ilbc);
	rtp_profile_set_payload(&av_profile,113,&payload_type_amr);
	rtp_profile_set_payload(&av_profile,114,args->custom_pt);
	rtp_profile_set_payload(&av_profile,115,&payload_type_lpc1015);
#ifdef VIDEO_ENABLED
	rtp_profile_set_payload(&av_profile,26,&payload_type_jpeg);
	rtp_profile_set_payload(&av_profile,98,&payload_type_h263_1998);
	rtp_profile_set_payload(&av_profile,97,&payload_type_theora);
	rtp_profile_set_payload(&av_profile,99,&payload_type_mp4v);
	rtp_profile_set_payload(&av_profile,100,&payload_type_x_snow);
	rtp_profile_set_payload(&av_profile,102,&payload_type_h264);
	rtp_profile_set_payload(&av_profile,103,&payload_type_vp8);
#endif
	return OK;
}

/*static void recv_tev_cb(RtpSession *session, unsigned long type, unsigned long dummy, void* user_data)
{
	printf("=============Receiving telephony event:%lu\n",type);
	//if (type<16) printf("This is dtmf %c\n",dtmf_tab[type]);
	//dtmf_count++;
}*/
#ifdef MS2_USER_DTMF_CB_ENABLE
/*static int user_recv_dtmf_cb(AudioStream *session, uint32_t type, void* user_data)
{
	int dtmf_tab[16]={'0','1','2','3','4','5','6','7','8','9','*','#','A','B','C','D'};
	if(type > 15 || type < 0)
		return -1;
	voip_app_dtmf_command_execute(dtmf_tab[type], 0);
	//printf("====%s===Receiving telephony event:%lu\n",__func__, type);
	//if (type<16) printf("This is dtmf %c\n",dtmf_tab[type]);
	//dtmf_count++;
	return 0;
}*/
#endif

void mediastream_setup(mediastream_global* args) {
	/*create the rtp session */
#ifdef VIDEO_ENABLED
	MSWebCam *cam=NULL;
#endif

	MSFactory *factory;

	if(args->running == TRUE)
		return;

	if(!args->initialization)
	{
		ortp_init();
		if (args->logfile)
			bctbx_set_log_file(args->logfile);

		if (args->is_verbose) {
			bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
		} else {
			bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_ERROR/*BCTBX_LOG_MESSAGE*/);
		}
		//bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
		args->factory = factory = ms_factory_new_with_voip();
	}
	else
		factory = args->factory;

#if TARGET_OS_IPHONE || defined(__ANDROID__)
#if TARGET_OS_IPHONE || (defined(HAVE_X264) && defined(VIDEO_ENABLED))
	libmsx264_init(); /*no plugin on IOS/Android */
#endif
#if TARGET_OS_IPHONE || (defined (HAVE_OPENH264) && defined (VIDEO_ENABLED))
	libmsopenh264_init(); /*no plugin on IOS/Android */
#endif
#if TARGET_OS_IPHONE || defined (HAVE_SILK)
	libmssilk_init(); /*no plugin on IOS/Android */
#endif
#if TARGET_OS_IPHONE || defined (HAVE_WEBRTC)
	libmswebrtc_init();
#endif

#endif /* TARGET_OS_IPHONE || defined(__ANDROID__) */

#ifdef VIDEO_ENABLED
	cam=ms_web_cam_new(ms_mire_webcam_desc_get());
	if (cam) ms_web_cam_manager_add_cam(ms_factory_get_web_cam_manager(factory), cam);
	cam=NULL;
	args->video=NULL;
#endif

	_mediastream_set_payload(args);

	args->profile=rtp_profile_clone_full(&av_profile);


	args->q=ortp_ev_queue_new();
	if (args->rc_algo == rcalgoAdvanced){
		args->bw_controller = ms_bandwidth_controller_new();
	}

	if (args->mtu) ms_factory_set_mtu(factory, args->mtu);
	ms_factory_enable_statistics(factory, TRUE);
	ms_factory_reset_statistics(factory);

	args->ice_session=ice_session_new();
	ice_session_set_remote_credentials(args->ice_session,"1234","1234567890abcdef123456");
	// ICE local credentials are assigned when creating the ICE session, but force them here to simplify testing
	ice_session_set_local_credentials(args->ice_session,"1234","1234567890abcdef123456");
	ice_dump_session(args->ice_session);

	//signal(SIGINT,stop_handler);
	args->pt=rtp_profile_get_payload(args->profile,args->payload);
	if (args->pt==NULL){
		ms_error("No payload defined with number %i.\n",args->payload);
		//exit(-1);
	}
	if (args->enable_avpf == TRUE) {
		PayloadTypeAvpfParams avpf_params;
		payload_type_set_flag(args->pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
		avpf_params.features = PAYLOAD_TYPE_AVPF_FIR | PAYLOAD_TYPE_AVPF_PLI | PAYLOAD_TYPE_AVPF_SLI | PAYLOAD_TYPE_AVPF_RPSI;
		avpf_params.trr_interval = 3000;
		payload_type_set_avpf_params(args->pt, avpf_params);
	} else {
		payload_type_unset_flag(args->pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
	}
	if (args->send_fmtp!=NULL) payload_type_set_send_fmtp(args->pt,args->send_fmtp);
	if (args->recv_fmtp!=NULL) payload_type_set_recv_fmtp(args->pt,args->recv_fmtp);
	if (args->bitrate>0) args->pt->normal_bitrate=args->bitrate;

	if (args->pt->normal_bitrate==0){
		ms_error("Default bitrate specified for codec %s/%i. "
			"Please specify a network bitrate with --bitrate option.\n",args->pt->mime_type,args->pt->clock_rate);
		//exit(-1);
	}

	// do we need to generate srtp keys ?
	if (args->enable_srtp) {
		// default profile require key-length = 30 bytes
		//  -> input : 40 b64 encoded bytes
		if (!args->srtp_local_master_key) {
			char tmp[30];
			snprintf(tmp,sizeof(tmp),"%08x%08x%08x%08x",rand(),rand(),rand(),rand());
			args->srtp_local_master_key = (char*) malloc(41);
			b64_encode((const char*)tmp, 30, args->srtp_local_master_key, 40);
			args->srtp_local_master_key[40] = '\0';
			ms_message("Generated local srtp key: '%s'", args->srtp_local_master_key);
		}
		if (!args->srtp_remote_master_key) {
			char tmp[30];
			snprintf(tmp,sizeof(tmp),"%08x%08x%08x%08x",rand(),rand(),rand(),rand());
			args->srtp_remote_master_key = (char*) malloc(41);
			b64_encode((const char*)tmp, 30, args->srtp_remote_master_key, 40);
			args->srtp_remote_master_key[40] = '\0';
			ms_message("Generated remote srtp key: '%s'", args->srtp_remote_master_key);
		}
	}

	if (args->pt->type!=PAYLOAD_VIDEO){
		MSSndCardManager *manager=ms_factory_get_snd_card_manager(factory);
		MSSndCard *capt= args->capture_card==NULL ? ms_snd_card_manager_get_default_capture_card(manager) :
				get_sound_card(manager,args->capture_card);
		MSSndCard *play= args->playback_card==NULL ? ms_snd_card_manager_get_default_capture_card(manager) :
				get_sound_card(manager,args->playback_card);
		args->audio=audio_stream_new(factory, args->localport,args->localport+1,ms_is_ipv6(args->ip));
		if (args->bw_controller){
			ms_bandwidth_controller_add_stream(args->bw_controller, (MediaStream*)args->audio);
		}
		audio_stream_enable_automatic_gain_control(args->audio,args->agc);
		audio_stream_enable_noise_gate(args->audio,args->use_ng);
		if(args->ec)
		{
/*			ELInactive,
			ELControlMic,
			ELControlFull*/
			audio_stream_enable_echo_limiter(args->audio, ELControlFull);
			//audio_stream_enable_noise_gate(args->audio, 1);
			audio_stream_enable_echo_canceller(args->audio, 1);
			if(args->ec_len_ms || args->ec_delay_ms || args->ec_framesize)
				audio_stream_set_echo_canceller_params(args->audio,args->ec_len_ms,args->ec_delay_ms,args->ec_framesize);
		}
		audio_stream_enable_echo_limiter(args->audio,args->el);
		audio_stream_enable_adaptive_bitrate_control(args->audio,args->rc_algo == rcalgoSimple);
		if (capt)
			ms_snd_card_set_preferred_sample_rate(capt,rtp_profile_get_payload(args->profile, args->payload)->clock_rate);
		if (play)
			ms_snd_card_set_preferred_sample_rate(play,rtp_profile_get_payload(args->profile, args->payload)->clock_rate);
		ms_message("Starting audio stream.\n");

		audio_stream_play_received_dtmfs(args->audio, args->enable_play_dtmf);//播放按键声音的控制
		//audio_stream_play_received_dtmfs(args->audio, true);
		//audio_stream_play_received_dtmfs(args->audio, false);
		//float output_gain_volume = audio_stream_get_sound_card_output_gain(args->audio);
		//audio_stream_set_sound_card_output_gain(args->audio, ((output_gain_volume * 2) > 1.0)? 1.0:output_gain_volume * 2);

		//rtp_session_signal_connect(session, "telephone-event",(RtpCallback)recv_tev_cb,0);
/*		{
			sip_dtmf_t dtmf = 0;
			voip_sip_dtmf_get_api(&dtmf);
			if(dtmf == VOIP_SIP_RFC2833)
			{
				uint32_t features = audio_stream_get_features(args->audio);
				features |= AUDIO_STREAM_FEATURE_DTMF | AUDIO_STREAM_FEATURE_DTMF_ECHO;
				audio_stream_set_features(args->audio, features);
			}
		}*/
#ifdef MS2_USER_DTMF_CB_ENABLE
		//zlog_debug(ZLOG_VOIP, "================%s:enable_dtmf=%d", __func__, args->enable_dtmf);
		//if(args->enable_dtmf)
		//	ms2_user_dtmf_cb_register(user_recv_dtmf_cb);
#endif


		audio_stream_start_full(args->audio,
								args->profile,
								args->ip,
								args->remoteport,
								args->ip,
								args->enable_rtcp?args->remoteport+1:-1,
								args->payload,
								args->jitter,
								args->infile,
								args->outfile,
								args->outfile==NULL ? play : NULL ,
								args->infile==NULL ? capt : NULL,
								/*args->infile!=NULL ? FALSE: */args->ec);

		if (args->ice_local_candidates_nb || args->ice_remote_candidates_nb) {
			args->audio->ms.ice_check_list = ice_check_list_new();
			rtp_session_set_pktinfo(args->audio->ms.sessions.rtp_session,TRUE);
			ice_session_add_check_list(args->ice_session, args->audio->ms.ice_check_list, 0);
		}
		if (args->ice_local_candidates_nb) {
			mediastreamIce_candidate *candidate;
			int c;
			for (c=0;c<args->ice_local_candidates_nb;c++){
				candidate=&args->ice_local_candidates[c];
				ice_add_local_candidate(args->audio->ms.ice_check_list,candidate->type,AF_INET,candidate->ip,candidate->port,1,NULL);
				ice_add_local_candidate(args->audio->ms.ice_check_list,candidate->type,AF_INET,candidate->ip,candidate->port+1,2,NULL);
			}
		}
		if (args->ice_remote_candidates_nb) {
			char foundation[4];
			mediastreamIce_candidate *candidate;
			int c;
			for (c=0;c<args->ice_remote_candidates_nb;c++){
				candidate=&args->ice_remote_candidates[c];
				memset(foundation, '\0', sizeof(foundation));
				snprintf(foundation, sizeof(foundation) - 1, "%u", c + 1);
				ice_add_remote_candidate(args->audio->ms.ice_check_list,candidate->type,AF_INET,candidate->ip,candidate->port,1,0,foundation,FALSE);
				ice_add_remote_candidate(args->audio->ms.ice_check_list,candidate->type,AF_INET,candidate->ip,candidate->port+1,2,0,foundation,FALSE);
			}
		}

		if (args->audio) {
			if (args->el) {
				if (args->el_speed!=-1)
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_EA_SPEED,&args->el_speed);
				if (args->el_force!=-1)
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_EA_FORCE,&args->el_force);
				if (args->el_thres!=-1)
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_EA_THRESHOLD,&args->el_thres);
				if (args->el_sustain!=-1)
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_EA_SUSTAIN,&args->el_sustain);
				if (args->el_transmit_thres!=-1)
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_EA_TRANSMIT_THRESHOLD,&args->el_transmit_thres);

			}
			if (args->use_ng){
				if (args->ng_threshold!=-1) {
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_NOISE_GATE_THRESHOLD,&args->ng_threshold);
					ms_filter_call_method(args->audio->volrecv,MS_VOLUME_SET_NOISE_GATE_THRESHOLD,&args->ng_threshold);
				}
				if (args->ng_floorgain != -1) {
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_NOISE_GATE_FLOORGAIN,&args->ng_floorgain);
					ms_filter_call_method(args->audio->volrecv,MS_VOLUME_SET_NOISE_GATE_FLOORGAIN,&args->ng_floorgain);
				}
			}

			if (args->enable_zrtp) {
				MSZrtpParams params = {0};
				audio_stream_enable_zrtp(args->audio,&params);
			}


			args->session=args->audio->ms.sessions.rtp_session;
		}

		if (args->enable_srtp) {
			ms_message("SRTP enabled: %d",
				audio_stream_enable_srtp(
					args->audio,
					MS_AES_128_SHA1_80,
					args->srtp_local_master_key,
					args->srtp_remote_master_key));
		}
	#if TARGET_OS_IPHONE
		if (args->enable_speaker) {
				ms_message("Setting audio route to spaker");
				UInt32 audioRouteOverride = kAudioSessionOverrideAudioRoute_Speaker;
				if (AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute, sizeof(audioRouteOverride),&audioRouteOverride) != kAudioSessionNoError) {
					ms_error("Cannot set route to speaker");
				};
		}
	#endif


	}else{
#ifdef VIDEO_ENABLED
		float zoom[] = {
			args->zoom,
			args->zoom_cx, args->zoom_cy };
		MSMediaStreamIO iodef = MS_MEDIA_STREAM_IO_INITIALIZER;

		if (args->eq){
			ms_fatal("Cannot put an audio equalizer in a video stream !");
			//exit(-1);
		}
		ms_message("Starting video stream.\n");
		args->video=video_stream_new(factory, args->localport, args->localport+1, ms_is_ipv6(args->ip));
		if (args->bw_controller){
			ms_bandwidth_controller_add_stream(args->bw_controller, (MediaStream*)args->video);
		}
		if (args->video_display_filter)
			video_stream_set_display_filter_name(args->video, args->video_display_filter);

#ifdef __ANDROID__
		if (args->device_rotation >= 0)
			video_stream_set_device_rotation(args->video, args->device_rotation);
#endif
		video_stream_set_sent_video_size(args->video,args->vs);
		video_stream_use_preview_video_window(args->video,args->two_windows);
#if TARGET_OS_IPHONE
		NSBundle* myBundle = [NSBundle mainBundle];
		const char*  nowebcam = [[myBundle pathForResource:@"nowebcamCIF"ofType:@"jpg"] cStringUsingEncoding:[NSString defaultCStringEncoding]];
		ms_static_image_set_default_image(nowebcam);
		NSUInteger cpucount = [[NSProcessInfo processInfo] processorCount];
		ms_factory_set_cpu_count(args->audio->ms.factory, cpucount);
		//ms_set_cpu_count(cpucount);
#endif
		video_stream_set_event_callback(args->video,video_stream_event_cb, args);
		video_stream_set_freeze_on_error(args->video,args->freeze_on_error);
		video_stream_enable_adaptive_bitrate_control(args->video, args->rc_algo == rcalgoSimple);
		if (args->camera)
			cam=ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(factory),args->camera);
		if (cam==NULL)
			cam=ms_web_cam_manager_get_default_cam(ms_factory_get_web_cam_manager(factory));

		if (args->infile){
			iodef.input.type = MSResourceFile;
			iodef.input.file = args->infile;
		}else{
			iodef.input.type = MSResourceCamera;
			iodef.input.camera = cam;
		}
		if (args->outfile){
			iodef.output.type = MSResourceFile;
			iodef.output.file = args->outfile;
		}else{
			iodef.output.type = MSResourceDefault;
			iodef.output.resource_arg = NULL;
		}
		rtp_session_set_jitter_compensation(args->video->ms.sessions.rtp_session, args->jitter);
		video_stream_start_from_io(args->video, args->profile,
					args->ip,args->remoteport,
					args->ip,args->enable_rtcp?args->remoteport+1:-1,
					args->payload,
					&iodef
					);
		args->session=args->video->ms.sessions.rtp_session;

		ms_filter_call_method(args->video->output,MS_VIDEO_DISPLAY_ZOOM, zoom);
		if (args->enable_srtp) {
			ms_message("SRTP enabled: %d",
				video_stream_enable_strp(
					args->video,
					MS_AES_128_SHA1_80,
					args->srtp_local_master_key,
					args->srtp_remote_master_key));
		}
#else
		ms_error("Error: video support not compiled.\n");
#endif
	}
	ice_session_set_base_for_srflx_candidates(args->ice_session);
	ice_session_compute_candidates_foundations(args->ice_session);
	ice_session_choose_default_candidates(args->ice_session);
	ice_session_choose_default_remote_candidates(args->ice_session);
	ice_session_start_connectivity_checks(args->ice_session);

	mediastream_tool_iterate_flush(args);

	mediastream_running_flag = 1;

	if (args->netsim.enabled){
		rtp_session_enable_network_simulation(args->session,&args->netsim);
	}
}


static int mediastream_tool_iterate_flush(mediastream_global* args)
{
	mediastream_hdr hdr;
	struct pollfd pfd;
	int err;
	int ctlfd = voip_stream_ctlfd_get_api();
	if(ctlfd <= 3)
		return OK;
	while (args->interactive)
	{
		if(ctlfd != args->ctlfd)
		{
			args->ctlfd = ctlfd;
		}
		pfd.fd=args->ctlfd;
		pfd.events=POLLIN;
		pfd.revents=0;
		err=poll(&pfd, 1, 1);
		if (err==1 && (pfd.revents & POLLIN))
		{
			memset(&hdr, 0, sizeof(hdr));
			read(args->ctlfd, &hdr, sizeof(hdr));
		}
		if(err <= 0)
			return OK;
	}
	return OK;
}

static void mediastream_tool_iterate(mediastream_global* args) {
#if 1
	mediastream_hdr hdr;
	struct pollfd pfd;
	int err;
	int ctlfd = voip_stream_ctlfd_get_api();
	if (args->interactive)
	{
		if(ctlfd == 0)
		{
			ms_usleep(10000);
			return;
		}
		if(ctlfd != args->ctlfd)
		{
			args->ctlfd = ctlfd;
		}
		pfd.fd=args->ctlfd;
		pfd.events=POLLIN;
		pfd.revents=0;

		err=poll(&pfd,1,10);

		if (err==1 && (pfd.revents & POLLIN) && (args->ctlfd > 0))
		{
			zlog_debug(ZLOG_VOIP, "media stream controls sock can read sock=%d", args->ctlfd);
			memset(&hdr, 0, sizeof(hdr));
			char *commands = hdr.data;
			int intarg;
			commands[127]='\0';
			ms_sleep(1);  /* ensure following text be printed after ortp messages */
			if (args->eq)
				ms_message("\nPlease enter equalizer requests, such as 'eq active 1', 'eq active 0', 'eq 1200 0.1 200'\n");

			//if (fgets(commands,sizeof(commands)-1,args->ctlfp)!=NULL)
			if (read(args->ctlfd, &hdr, sizeof(hdr)))
			{
				MSEqualizerGain d = {0};
				int active;

				if(mediastream_running_flag == 0)
				{
					return;
				}
#ifdef VOIP_MEDIASTRREM_CTL_DEBUG
				zlog_debug(ZLOG_VOIP, "GET CMD(%d byte):%s", ntohs(hdr.len), commands);
#endif
				if (sscanf(commands,"eq active %i",&active)==1)
				{
					audio_stream_enable_equalizer(args->audio, args->audio->eq_loc, active);
					ms_message("OK\n");
				}
				else if (sscanf(commands,"eq %f %f %f",&d.frequency,&d.gain,&d.width)==3)
				{
					audio_stream_equalizer_set_gain(args->audio, args->audio->eq_loc, &d);
					ms_message("OK\n");
				}
				else if (sscanf(commands,"eq %f %f",&d.frequency,&d.gain)==2)
				{
					audio_stream_equalizer_set_gain(args->audio, args->audio->eq_loc, &d);
					ms_message("OK\n");
				}
				else if (strstr(commands,"dump"))
				{
					int n=0,i;
					float *t;
					MSFilter *equalizer = NULL;
					if(args->audio->eq_loc == MSEqualizerHP)
					{
						equalizer = args->audio->spk_equalizer;
					}
					else if(args->audio->eq_loc == MSEqualizerMic)
					{
						equalizer = args->audio->mic_equalizer;
					}
					if(equalizer)
					{
						ms_filter_call_method(equalizer,MS_EQUALIZER_GET_NUM_FREQUENCIES,&n);
						t=(float*)alloca(sizeof(float)*n);
						ms_filter_call_method(equalizer,MS_EQUALIZER_DUMP_STATE,t);
						for(i=0;i<n;++i)
						{
							if (fabs(t[i]-1)>0.01)
							{
								ms_message("%i:%f:0 ",(i*args->pt->clock_rate)/(2*n),t[i]);
							}
						}
					}
					ms_message("\nOK\n");
				}
				else if (sscanf(commands,"lossrate %i",&intarg)==1)
				{
					args->netsim.enabled=TRUE;
					args->netsim.loss_rate=intarg;
					rtp_session_enable_network_simulation(args->session,&args->netsim);
				}
				else if (sscanf(commands,"bandwidth %i",&intarg)==1)
				{
					args->netsim.enabled=TRUE;
					args->netsim.max_bandwidth=intarg;
					rtp_session_enable_network_simulation(args->session,&args->netsim);
				}
				else if (strstr(commands,"quit"))
				{
					mediastream_running_flag = 0;
					zlog_debug(ZLOG_VOIP, "quit from mediastream");
				}
				else
					ms_warning("Cannot understand this.\n");
			}
		}
		else if (err==-1 && errno!=EINTR)
		{
#ifdef VOIP_MEDIASTRREM_CTL_DEBUG
			zlog_debug(ZLOG_VOIP, "mediastream's poll() returned %s",strerror(errno));
#endif
			ms_fatal("mediastream's poll() returned %s",strerror(errno));
		}
	}
	else
	{
		ms_usleep(10000);
	}
#else
	struct pollfd pfd;
	int err;
	int ctlfd = voip_stream_ctlfd_get_api();
	if (args->interactive){
		if(ctlfd == 0)
		{
			ms_usleep(10000);
			return;
		}
		if(ctlfd != args->ctlfd)
		{
			args->ctlfd = ctlfd;
		}
		pfd.fd=args->ctlfd;
		pfd.events=POLLIN;
		pfd.revents=0;

		err=poll(&pfd,1,10);
		if (err==1 && (pfd.revents & POLLIN)){
			zlog_debug(ZLOG_VOIP, "media stream controls sock can read sock=%d", args->ctlfd);
			char commands[128];
			int intarg;
			commands[127]='\0';
			ms_sleep(1);  /* ensure following text be printed after ortp messages */
			if (args->eq)
			ms_message("\nPlease enter equalizer requests, such as 'eq active 1', 'eq active 0', 'eq 1200 0.1 200'\n");
			if (fgets(commands,sizeof(commands)-1,args->ctlfp)!=NULL){
				MSEqualizerGain d = {0};
				int active;
#ifdef VOIP_MEDIASTRREM_CTL_DEBUG
				zlog_debug(ZLOG_VOIP, "GET CMD:%s", commands);
#endif
				if (sscanf(commands,"eq active %i",&active)==1){
					audio_stream_enable_equalizer(args->audio, args->audio->eq_loc, active);
					ms_message("OK\n");
				}else if (sscanf(commands,"eq %f %f %f",&d.frequency,&d.gain,&d.width)==3){
					audio_stream_equalizer_set_gain(args->audio, args->audio->eq_loc, &d);
					ms_message("OK\n");
				}else if (sscanf(commands,"eq %f %f",&d.frequency,&d.gain)==2){
					audio_stream_equalizer_set_gain(args->audio, args->audio->eq_loc, &d);
					ms_message("OK\n");
				}else if (strstr(commands,"dump")){
					int n=0,i;
					float *t;
					MSFilter *equalizer = NULL;
					if(args->audio->eq_loc == MSEqualizerHP) {
						equalizer = args->audio->spk_equalizer;
					} else if(args->audio->eq_loc == MSEqualizerMic) {
						equalizer = args->audio->mic_equalizer;
					}
					if(equalizer) {
						ms_filter_call_method(equalizer,MS_EQUALIZER_GET_NUM_FREQUENCIES,&n);
						t=(float*)alloca(sizeof(float)*n);
						ms_filter_call_method(equalizer,MS_EQUALIZER_DUMP_STATE,t);
						for(i=0;i<n;++i){
							if (fabs(t[i]-1)>0.01){
							ms_message("%i:%f:0 ",(i*args->pt->clock_rate)/(2*n),t[i]);
							}
						}
					}
					ms_message("\nOK\n");
				}else if (sscanf(commands,"lossrate %i",&intarg)==1){
					args->netsim.enabled=TRUE;
					args->netsim.loss_rate=intarg;
					rtp_session_enable_network_simulation(args->session,&args->netsim);
				}else if (sscanf(commands,"bandwidth %i",&intarg)==1){
					args->netsim.enabled=TRUE;
					args->netsim.max_bandwidth=intarg;
					rtp_session_enable_network_simulation(args->session,&args->netsim);
				}else if (strstr(commands,"quit")){
					mediastream_running_flag=0;
				}else ms_warning("Cannot understand this.\n");
			}
		}else if (err==-1 && errno!=EINTR){
#ifdef VOIP_MEDIASTRREM_CTL_DEBUG
			zlog_debug(ZLOG_VOIP, "mediastream's poll() returned %s",strerror(errno));
#endif
			ms_fatal("mediastream's poll() returned %s",strerror(errno));
		}
	}else{
		ms_usleep(10000);
	}
#endif
}

void mediastream_running(mediastream_global* args)
{
	int n;
	rtp_session_register_event_queue(args->session,args->q);

#if TARGET_OS_IPHONE
	if (args->video) ms_set_video_stream(args->video); /*for IOS*/
#endif

	while(mediastream_running_flag)
	{
		args->running = TRUE;
		//voip_sip_call_state_set_api(VOIP_SIP_TALK);
		//voip_talk_state_set(&sip_config->sip_state, SIP_STATE_TALK_RUNNING);

		for(n=0;n<50 && mediastream_running_flag;++n)
		{
			mediastream_tool_iterate(args);
#if defined(VIDEO_ENABLED)
			if (args->video) video_stream_iterate(args->video);
#endif
			if (args->audio) audio_stream_iterate(args->audio);
		}
		if(mediastream_running_flag == 0)
			break;
		rtp_stats_display(rtp_session_get_stats(args->session),"RTP stats");

		if (args->session)
		{
			float audio_load = 0;
			float video_load = 0;
			//zlog_debug(ZLOG_VOIP, "---%s: INTO mediastream", __func__);
			ms_message("Bandwidth usage: download=%f kbits/sec, upload=%f kbits/sec\n",
				rtp_session_get_recv_bandwidth(args->session)*1e-3,
				rtp_session_get_send_bandwidth(args->session)*1e-3);

			if (args->audio)
			{
				audio_load = ms_ticker_get_average_load(args->audio->ms.sessions.ticker);
			}
#if defined(VIDEO_ENABLED)
			if (args->video)
			{
				video_load = ms_ticker_get_average_load(args->video->ms.sessions.ticker);
			}
#endif
			ms_message("Thread processing load: audio=%f\tvideo=%f", audio_load, video_load);
			parse_events(args, args->session,args->q);
			ms_message("Quality indicator : %f\n",args->audio ? audio_stream_get_quality_rating(args->audio) : media_stream_get_quality_rating((MediaStream*)args->video));
		}
	}
	//zlog_debug(ZLOG_VOIP, "---%s: quit from mediastream", __func__);
}

void mediastream_clear(mediastream_global* args) {

	//zlog_debug(ZLOG_VOIP, "---%s", __func__);

	if(args->running == FALSE)
		return;
	//voip_sip_call_state_set_api(VOIP_SIP_CALL_IDLE);
	//voip_talk_state_set(&sip_config->sip_state, SIP_STATE_TALK_IDLE);
	ms_message("stopping all...\n");
	ms_message("Average quality indicator: %f",args->audio ? audio_stream_get_average_quality_rating(args->audio) : -1);

	if (args->bw_controller){
		ms_bandwidth_controller_destroy(args->bw_controller);
	}

	if (args->audio) {
		audio_stream_stop(args->audio);
	}
#ifdef VIDEO_ENABLED
	if (args->video) {
		if (args->video->ms.ice_check_list) ice_check_list_destroy(args->video->ms.ice_check_list);
		video_stream_stop(args->video);
		ms_factory_log_statistics(args->video->ms.factory);
	}
#endif
	if (args->ice_session) ice_session_destroy(args->ice_session);
	ortp_ev_queue_destroy(args->q);

	rtp_profile_destroy(args->profile);
	args->running = FALSE;
/*	if (args->logfile)
		fclose(args->logfile);
	args->logfile = NULL;

	ms_factory_destroy(args->factory);*/
	//args->initialization = TRUE;
}

// ANDROID JNI WRAPPER

static bool_t parse_addr(const char *addr, char *ip, size_t len, int *port)
{
	const char *semicolon=NULL;
	size_t iplen;
	int slen;
	const char *p;

	*port=0;

	for (p=addr+strlen(addr)-1;p>addr;p--){
		if (*p==':') {
			semicolon=p;
			break;
		}
	}
	/*if no semicolon is present, we can assume that user provided only port*/
	if (semicolon==NULL) {
		const char *localhost = "127.0.0.1";
		char * end;
		*port = strtol(addr, &end, 10);
		if (*end != '\0' || end == addr) {
			return FALSE;
		}
		strncpy(ip,localhost, MIN(len, strlen(localhost)));
		return TRUE;
	}
	iplen=semicolon-addr;
	slen=MIN(iplen,len-1);
	strncpy(ip,addr,slen);
	ip[slen]='\0';
	*port=atoi(semicolon+1);
	return TRUE;
}

static bool_t parse_ice_addr(char *addr, char *type, size_t type_len, char *ip, size_t ip_len, int *port)
{
	char *semicolon=NULL;
	size_t slen;

	semicolon=strrchr(addr,':');
	if (semicolon==NULL) return FALSE;
	slen=MIN(strlen(semicolon+1),type_len);
	strncpy(type,semicolon+1,slen);
	type[slen]='\0';
	*semicolon='\0';
	return parse_addr(addr,ip,ip_len,port);
}

static void display_items(void *user_data, uint32_t csrc, rtcp_sdes_type_t t, const char *content, uint8_t content_len){
	char str[256];
	int len=MIN(sizeof(str)-1,content_len);
	strncpy(str,content,len);
	str[len]='\0';
	switch(t){
		case RTCP_SDES_CNAME:
			ms_message("Found CNAME=%s",str);
		break;
		case RTCP_SDES_TOOL:
			ms_message("Found TOOL=%s",str);
		break;
		case RTCP_SDES_NOTE:
			ms_message("Found NOTE=%s",str);
		break;
		default:
			ms_message("Unhandled SDES item (%s)",str);
	}
}

static void parse_rtcp(mblk_t *m){
	do{
		if (rtcp_is_RR(m)){
			ms_message("Receiving RTCP RR");
		}else if (rtcp_is_SR(m)){
			ms_message("Receiving RTCP SR");
		}else if (rtcp_is_SDES(m)){
			ms_message("Receiving RTCP SDES");
			rtcp_sdes_parse(m,display_items,NULL);
		}else {
			ms_message("Receiving unhandled RTCP message");
		}
	}while(rtcp_next_packet(m));
}



static void voip_stream_telephone_event(mediastream_global* args, OrtpEventData *d)
{
	if(d)
	{
		voip_stream_t *stream = args->priv;
		//static char dtmfTab[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '#', 'A', 'B', 'C', 'D' };
		int dtmf_tab[16]={'0','1','2','3','4','5','6','7','8','9','*','#','A','B','C','D'};
		if(d->info.telephone_event > 15 || d->info.telephone_event < 0)
			return ;
		if(stream && stream->app_dtmf_cb)
			(stream->app_dtmf_cb)(dtmf_tab[d->info.telephone_event], 0);
		if(VOIP_STREAM_IS_DEBUG(EVENT))
			zlog_debug(ZLOG_VOIP, "RCF2833 recv telephone event=%d payload type=%d <- '%c'", d->info.telephone_event,
				d->info.payload_type, dtmf_tab[d->info.telephone_event]);
	}
	//mblk_t *packet;	/* most events are associated to a received packet */
}


static void parse_events(mediastream_global* args, RtpSession *session, OrtpEvQueue *q){
	OrtpEvent *ev;

	while((ev=ortp_ev_queue_get(q))!=NULL)
	{
		OrtpEventData *d=ortp_event_get_data(ev);
		switch(ortp_event_get_type(ev))
		{
			case ORTP_EVENT_RTCP_PACKET_RECEIVED:
				parse_rtcp(d->packet);
				//zlog_debug(ZLOG_VOIP, "=====%s:ORTP_EVENT_RTCP_PACKET_RECEIVED", __func__);
			break;
			case ORTP_EVENT_RTCP_PACKET_EMITTED:
				//zlog_debug(ZLOG_VOIP, "=====%s:ORTP_EVENT_RTCP_PACKET_EMITTED", __func__);
				ms_message("Jitter buffer size: %f ms",rtp_session_get_jitter_stats(session)->jitter_buffer_size_ms);
			break;
			case ORTP_EVENT_TELEPHONE_EVENT:
				//zlog_debug(ZLOG_VOIP, "=====%s:ORTP_EVENT_TELEPHONE_EVENT", __func__);
				voip_stream_telephone_event(args, d);
			break;

			default:
			break;
		}
		ortp_event_destroy(ev);
	}
}

static bool_t parse_window_ids(const char *ids, int* video_id, int* preview_id)
{
	char* copy = strdup(ids);
	char *semicolon=strchr(copy,':');
	if (semicolon==NULL) {
		free(copy);
		return FALSE;
	}
	*semicolon = '\0';

	*video_id=atoi(copy);
	*preview_id=atoi(semicolon+1);
	free(copy);
	return TRUE;
}

static rcalgo parse_rc_algo(const char *algo){
	if (algo == NULL) return rcalgoInvalid;
	if (strcasecmp(algo,"simple")==0) return rcalgoSimple;
	if (strcasecmp(algo, "advanced")==0) return rcalgoAdvanced;
	if (strcasecmp(algo, "none")==0) return rcalgoNone;
	return rcalgoInvalid;
}

#ifdef PL_VOIP_MEDIASTREAM_TEST
void clear_mediastreams(mediastream_global* args) {
	if(args->running == FALSE)
		return;
	ms_message("stopping all...\n");
	ms_message("Average quality indicator: %f",args->audio ? audio_stream_get_average_quality_rating(args->audio) : -1);

	if (args->bw_controller){
		ms_bandwidth_controller_destroy(args->bw_controller);
	}

	if (args->audio) {
		audio_stream_stop(args->audio);
	}
#ifdef VIDEO_ENABLED
	if (args->video) {
		if (args->video->ms.ice_check_list) ice_check_list_destroy(args->video->ms.ice_check_list);
		video_stream_stop(args->video);
		ms_factory_log_statistics(args->video->ms.factory);
	}
#endif
	if (args->ice_session) ice_session_destroy(args->ice_session);
	ortp_ev_queue_destroy(args->q);

	rtp_profile_destroy(args->profile);
	args->running = FALSE;
/*	if (args->logfile)
		fclose(args->logfile);
	args->logfile = NULL;

	args->ctlfd = 0;
	ms_factory_destroy(args->factory);*/
}


void setup_media_streams(mediastream_global* args) {
	/*create the rtp session */
#ifdef VIDEO_ENABLED
	MSWebCam *cam=NULL;
#endif

	MSFactory *factory;
	ortp_init();
	if (args->logfile)
		bctbx_set_log_file(args->logfile);

	if (args->is_verbose) {
		bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_DEBUG);
	} else {
		bctbx_set_log_level(BCTBX_LOG_DOMAIN, BCTBX_LOG_MESSAGE);
	}

	args->factory = factory = ms_factory_new_with_voip();

#if TARGET_OS_IPHONE || defined(__ANDROID__)
#if TARGET_OS_IPHONE || (defined(HAVE_X264) && defined(VIDEO_ENABLED))
	libmsx264_init(); /*no plugin on IOS/Android */
#endif
#if TARGET_OS_IPHONE || (defined (HAVE_OPENH264) && defined (VIDEO_ENABLED))
	libmsopenh264_init(); /*no plugin on IOS/Android */
#endif
#if TARGET_OS_IPHONE || defined (HAVE_SILK)
	libmssilk_init(); /*no plugin on IOS/Android */
#endif
#if TARGET_OS_IPHONE || defined (HAVE_WEBRTC)
	libmswebrtc_init();
#endif

#endif /* TARGET_OS_IPHONE || defined(__ANDROID__) */

	rtp_profile_set_payload(&av_profile,101,&payload_type_telephone_event);
	rtp_profile_set_payload(&av_profile,110,&payload_type_speex_nb);
	rtp_profile_set_payload(&av_profile,111,&payload_type_speex_wb);
	rtp_profile_set_payload(&av_profile,112,&payload_type_ilbc);
	rtp_profile_set_payload(&av_profile,113,&payload_type_amr);
	rtp_profile_set_payload(&av_profile,114,args->custom_pt);
	rtp_profile_set_payload(&av_profile,115,&payload_type_lpc1015);
#ifdef VIDEO_ENABLED
	cam=ms_web_cam_new(ms_mire_webcam_desc_get());
	if (cam) ms_web_cam_manager_add_cam(ms_factory_get_web_cam_manager(factory), cam);
	cam=NULL;

	rtp_profile_set_payload(&av_profile,26,&payload_type_jpeg);
	rtp_profile_set_payload(&av_profile,98,&payload_type_h263_1998);
	rtp_profile_set_payload(&av_profile,97,&payload_type_theora);
	rtp_profile_set_payload(&av_profile,99,&payload_type_mp4v);
	rtp_profile_set_payload(&av_profile,100,&payload_type_x_snow);
	rtp_profile_set_payload(&av_profile,102,&payload_type_h264);
	rtp_profile_set_payload(&av_profile,103,&payload_type_vp8);

	args->video=NULL;
#endif
	args->profile=rtp_profile_clone_full(&av_profile);


	args->q=ortp_ev_queue_new();
	if (args->rc_algo == rcalgoAdvanced){
		args->bw_controller = ms_bandwidth_controller_new();
	}

	if (args->mtu) ms_factory_set_mtu(factory, args->mtu);
	ms_factory_enable_statistics(factory, TRUE);
	ms_factory_reset_statistics(factory);

	args->ice_session=ice_session_new();
	ice_session_set_remote_credentials(args->ice_session,"1234","1234567890abcdef123456");
	// ICE local credentials are assigned when creating the ICE session, but force them here to simplify testing
	ice_session_set_local_credentials(args->ice_session,"1234","1234567890abcdef123456");
	ice_dump_session(args->ice_session);

	//signal(SIGINT,stop_handler);
	args->pt=rtp_profile_get_payload(args->profile,args->payload);
	if (args->pt==NULL){
		ms_error("No payload defined with number %i.\n",args->payload);
		//exit(-1);
	}
	if (args->enable_avpf == TRUE) {
		PayloadTypeAvpfParams avpf_params;
		payload_type_set_flag(args->pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
		avpf_params.features = PAYLOAD_TYPE_AVPF_FIR | PAYLOAD_TYPE_AVPF_PLI | PAYLOAD_TYPE_AVPF_SLI | PAYLOAD_TYPE_AVPF_RPSI;
		avpf_params.trr_interval = 3000;
		payload_type_set_avpf_params(args->pt, avpf_params);
	} else {
		payload_type_unset_flag(args->pt, PAYLOAD_TYPE_RTCP_FEEDBACK_ENABLED);
	}
	if (args->send_fmtp!=NULL) payload_type_set_send_fmtp(args->pt,args->send_fmtp);
	if (args->recv_fmtp!=NULL) payload_type_set_recv_fmtp(args->pt,args->recv_fmtp);
	if (args->bitrate>0) args->pt->normal_bitrate=args->bitrate;

	if (args->pt->normal_bitrate==0){
		ms_error("Default bitrate specified for codec %s/%i. "
			"Please specify a network bitrate with --bitrate option.\n",args->pt->mime_type,args->pt->clock_rate);
		//exit(-1);
	}

	// do we need to generate srtp keys ?
	if (args->enable_srtp) {
		// default profile require key-length = 30 bytes
		//  -> input : 40 b64 encoded bytes
		if (!args->srtp_local_master_key) {
			char tmp[30];
			snprintf(tmp,sizeof(tmp),"%08x%08x%08x%08x",rand(),rand(),rand(),rand());
			args->srtp_local_master_key = (char*) malloc(41);
			b64_encode((const char*)tmp, 30, args->srtp_local_master_key, 40);
			args->srtp_local_master_key[40] = '\0';
			ms_message("Generated local srtp key: '%s'", args->srtp_local_master_key);
		}
		if (!args->srtp_remote_master_key) {
			char tmp[30];
			snprintf(tmp,sizeof(tmp),"%08x%08x%08x%08x",rand(),rand(),rand(),rand());
			args->srtp_remote_master_key = (char*) malloc(41);
			b64_encode((const char*)tmp, 30, args->srtp_remote_master_key, 40);
			args->srtp_remote_master_key[40] = '\0';
			ms_message("Generated remote srtp key: '%s'", args->srtp_remote_master_key);
		}
	}

	if (args->pt->type!=PAYLOAD_VIDEO){
		MSSndCardManager *manager=ms_factory_get_snd_card_manager(factory);
		MSSndCard *capt= args->capture_card==NULL ? ms_snd_card_manager_get_default_capture_card(manager) :
				get_sound_card(manager,args->capture_card);
		MSSndCard *play= args->playback_card==NULL ? ms_snd_card_manager_get_default_capture_card(manager) :
				get_sound_card(manager,args->playback_card);
		args->audio=audio_stream_new(factory, args->localport,args->localport+1,ms_is_ipv6(args->ip));
		if (args->bw_controller){
			ms_bandwidth_controller_add_stream(args->bw_controller, (MediaStream*)args->audio);
		}
		audio_stream_enable_automatic_gain_control(args->audio,args->agc);
		audio_stream_enable_noise_gate(args->audio,args->use_ng);
		if(args->ec)
		{
			audio_stream_enable_echo_canceller(args->audio, 1);
			if(args->ec_len_ms || args->ec_delay_ms || args->ec_framesize)
				audio_stream_set_echo_canceller_params(args->audio,args->ec_len_ms,args->ec_delay_ms,args->ec_framesize);
		}
		audio_stream_enable_echo_limiter(args->audio,args->el);
		audio_stream_enable_adaptive_bitrate_control(args->audio,args->rc_algo == rcalgoSimple);
		if (capt)
			ms_snd_card_set_preferred_sample_rate(capt,rtp_profile_get_payload(args->profile, args->payload)->clock_rate);
		if (play)
			ms_snd_card_set_preferred_sample_rate(play,rtp_profile_get_payload(args->profile, args->payload)->clock_rate);
		ms_message("Starting audio stream.\n");

		audio_stream_start_full(args->audio,args->profile,args->ip,args->remoteport,args->ip,args->enable_rtcp?args->remoteport+1:-1, args->payload, args->jitter,args->infile,args->outfile,
								args->outfile==NULL ? play : NULL ,args->infile==NULL ? capt : NULL,args->infile!=NULL ? FALSE: args->ec);

		if (args->ice_local_candidates_nb || args->ice_remote_candidates_nb) {
			args->audio->ms.ice_check_list = ice_check_list_new();
			rtp_session_set_pktinfo(args->audio->ms.sessions.rtp_session,TRUE);
			ice_session_add_check_list(args->ice_session, args->audio->ms.ice_check_list, 0);
		}
		if (args->ice_local_candidates_nb) {
			mediastreamIce_candidate *candidate;
			int c;
			for (c=0;c<args->ice_local_candidates_nb;c++){
				candidate=&args->ice_local_candidates[c];
				ice_add_local_candidate(args->audio->ms.ice_check_list,candidate->type,AF_INET,candidate->ip,candidate->port,1,NULL);
				ice_add_local_candidate(args->audio->ms.ice_check_list,candidate->type,AF_INET,candidate->ip,candidate->port+1,2,NULL);
			}
		}
		if (args->ice_remote_candidates_nb) {
			char foundation[4];
			mediastreamIce_candidate *candidate;
			int c;
			for (c=0;c<args->ice_remote_candidates_nb;c++){
				candidate=&args->ice_remote_candidates[c];
				memset(foundation, '\0', sizeof(foundation));
				snprintf(foundation, sizeof(foundation) - 1, "%u", c + 1);
				ice_add_remote_candidate(args->audio->ms.ice_check_list,candidate->type,AF_INET,candidate->ip,candidate->port,1,0,foundation,FALSE);
				ice_add_remote_candidate(args->audio->ms.ice_check_list,candidate->type,AF_INET,candidate->ip,candidate->port+1,2,0,foundation,FALSE);
			}
		}

		if (args->audio) {
			if (args->el) {
				if (args->el_speed!=-1)
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_EA_SPEED,&args->el_speed);
				if (args->el_force!=-1)
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_EA_FORCE,&args->el_force);
				if (args->el_thres!=-1)
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_EA_THRESHOLD,&args->el_thres);
				if (args->el_sustain!=-1)
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_EA_SUSTAIN,&args->el_sustain);
				if (args->el_transmit_thres!=-1)
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_EA_TRANSMIT_THRESHOLD,&args->el_transmit_thres);

			}
			if (args->use_ng){
				if (args->ng_threshold!=-1) {
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_NOISE_GATE_THRESHOLD,&args->ng_threshold);
					ms_filter_call_method(args->audio->volrecv,MS_VOLUME_SET_NOISE_GATE_THRESHOLD,&args->ng_threshold);
				}
				if (args->ng_floorgain != -1) {
					ms_filter_call_method(args->audio->volsend,MS_VOLUME_SET_NOISE_GATE_FLOORGAIN,&args->ng_floorgain);
					ms_filter_call_method(args->audio->volrecv,MS_VOLUME_SET_NOISE_GATE_FLOORGAIN,&args->ng_floorgain);
				}
			}

			if (args->enable_zrtp) {
				MSZrtpParams params = {0};
				audio_stream_enable_zrtp(args->audio,&params);
			}


			args->session=args->audio->ms.sessions.rtp_session;
		}

		if (args->enable_srtp) {
			ms_message("SRTP enabled: %d",
				audio_stream_enable_srtp(
					args->audio,
					MS_AES_128_SHA1_80,
					args->srtp_local_master_key,
					args->srtp_remote_master_key));
		}
	#if TARGET_OS_IPHONE
		if (args->enable_speaker) {
				ms_message("Setting audio route to spaker");
				UInt32 audioRouteOverride = kAudioSessionOverrideAudioRoute_Speaker;
				if (AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute, sizeof(audioRouteOverride),&audioRouteOverride) != kAudioSessionNoError) {
					ms_error("Cannot set route to speaker");
				};
		}
	#endif


	}else{
#ifdef VIDEO_ENABLED
		float zoom[] = {
			args->zoom,
			args->zoom_cx, args->zoom_cy };
		MSMediaStreamIO iodef = MS_MEDIA_STREAM_IO_INITIALIZER;

		if (args->eq){
			ms_fatal("Cannot put an audio equalizer in a video stream !");
			//exit(-1);
		}
		ms_message("Starting video stream.\n");
		args->video=video_stream_new(factory, args->localport, args->localport+1, ms_is_ipv6(args->ip));
		if (args->bw_controller){
			ms_bandwidth_controller_add_stream(args->bw_controller, (MediaStream*)args->video);
		}
		if (args->video_display_filter)
			video_stream_set_display_filter_name(args->video, args->video_display_filter);

#ifdef __ANDROID__
		if (args->device_rotation >= 0)
			video_stream_set_device_rotation(args->video, args->device_rotation);
#endif
		video_stream_set_sent_video_size(args->video,args->vs);
		video_stream_use_preview_video_window(args->video,args->two_windows);
#if TARGET_OS_IPHONE
		NSBundle* myBundle = [NSBundle mainBundle];
		const char*  nowebcam = [[myBundle pathForResource:@"nowebcamCIF"ofType:@"jpg"] cStringUsingEncoding:[NSString defaultCStringEncoding]];
		ms_static_image_set_default_image(nowebcam);
		NSUInteger cpucount = [[NSProcessInfo processInfo] processorCount];
		ms_factory_set_cpu_count(args->audio->ms.factory, cpucount);
		//ms_set_cpu_count(cpucount);
#endif
		video_stream_set_event_callback(args->video,video_stream_event_cb, args);
		video_stream_set_freeze_on_error(args->video,args->freeze_on_error);
		video_stream_enable_adaptive_bitrate_control(args->video, args->rc_algo == rcalgoSimple);
		if (args->camera)
			cam=ms_web_cam_manager_get_cam(ms_factory_get_web_cam_manager(factory),args->camera);
		if (cam==NULL)
			cam=ms_web_cam_manager_get_default_cam(ms_factory_get_web_cam_manager(factory));

		if (args->infile){
			iodef.input.type = MSResourceFile;
			iodef.input.file = args->infile;
		}else{
			iodef.input.type = MSResourceCamera;
			iodef.input.camera = cam;
		}
		if (args->outfile){
			iodef.output.type = MSResourceFile;
			iodef.output.file = args->outfile;
		}else{
			iodef.output.type = MSResourceDefault;
			iodef.output.resource_arg = NULL;
		}
		rtp_session_set_jitter_compensation(args->video->ms.sessions.rtp_session, args->jitter);
		video_stream_start_from_io(args->video, args->profile,
					args->ip,args->remoteport,
					args->ip,args->enable_rtcp?args->remoteport+1:-1,
					args->payload,
					&iodef
					);
		args->session=args->video->ms.sessions.rtp_session;

		ms_filter_call_method(args->video->output,MS_VIDEO_DISPLAY_ZOOM, zoom);
		if (args->enable_srtp) {
			ms_message("SRTP enabled: %d",
				video_stream_enable_strp(
					args->video,
					MS_AES_128_SHA1_80,
					args->srtp_local_master_key,
					args->srtp_remote_master_key));
		}
#else
		ms_error("Error: video support not compiled.\n");
#endif
	}
	ice_session_set_base_for_srflx_candidates(args->ice_session);
	ice_session_compute_candidates_foundations(args->ice_session);
	ice_session_choose_default_candidates(args->ice_session);
	ice_session_choose_default_remote_candidates(args->ice_session);
	ice_session_start_connectivity_checks(args->ice_session);

	if (args->netsim.enabled){
		rtp_session_enable_network_simulation(args->session,&args->netsim);
	}
}
#endif


/*
 * log
 */
void md_stream_BctbxLogFunc(const char *domain, BctbxLogLevel lev, const char *fmt, va_list args)
{
	int priority = 0;
	if (lev & BCTBX_LOG_FATAL){
		priority = LOG_CRIT;
	}
	if (lev & BCTBX_LOG_ERROR){
		priority = LOG_ERR;
	}
	if (lev & BCTBX_LOG_WARNING){
		priority = LOG_WARNING;
	}
	if (lev & BCTBX_LOG_MESSAGE){
		priority = LOG_INFO;
	}
	if (lev & BCTBX_LOG_TRACE)	{
		priority = LOG_NOTICE;
	}
	if (lev & BCTBX_LOG_DEBUG){
		priority = LOG_DEBUG;
	}
	vzlog_other(NULL, ZLOG_SOUND, priority, fmt, args);
}

/*
 * mediastream
 */

static PayloadType* create_custom_payload_type(const char *type, const char *subtype, const char *rate, const char *channels, int number){
	PayloadType *pt=payload_type_new();
	if (strcasecmp(type,"audio")==0){
		pt->type=PAYLOAD_AUDIO_PACKETIZED;
	}else if (strcasecmp(type,"video")==0){
		pt->type=PAYLOAD_VIDEO;
	}else{
		fprintf(stderr,"Unsupported payload type should be audio or video, not %s\n",type);
		//exit(-1);
	}
	pt->mime_type=ms_strdup(subtype);
	pt->clock_rate=atoi(rate);
	pt->channels=atoi(channels);
	return pt;
}

PayloadType* ms_tools_parse_custom_payload(const char *name){
	char type[64]={0};
	char subtype[64]={0};
	char clockrate[64]={0};
	char nchannels[64];
	char *separator;

	if (strlen(name)>=sizeof(clockrate)-1){
		fprintf(stderr,"Cannot parse %s: too long.\n",name);
		//exit(-1);
		return NULL;
	}

	separator=strchr(name,'/');
	if (separator){
		char *separator2;

		strncpy(type,name,separator-name);
		separator2=strchr(separator+1,'/');
		if (separator2){
			char *separator3;

			strncpy(subtype,separator+1,separator2-separator-1);
			separator3=strchr(separator2+1,'/');
			if (separator3){
				strncpy(clockrate,separator2+1,separator3-separator2-1);
				strcpy(nchannels,separator3+1);
			} else {
				nchannels[0]='1';
				nchannels[1]='\0';
				strcpy(clockrate,separator2+1);
			}
			fprintf(stdout,"Found custom payload type=%s, mime=%s, clockrate=%s nchannels=%s\n", type, subtype, clockrate, nchannels);
			return create_custom_payload_type(type,subtype,clockrate,nchannels,114);
		}
	}
	fprintf(stderr,"Error parsing payload name %s.\n",name);
	//exit(-1);
	return NULL;
}



int voip_mediastream_show_config(void * p, struct vty *vty)
{
	mediastream_global *mm = (mediastream_global *)p;
	if(mm)
	{
		vty_out(vty, "----------------- voip stream hardware -----------------%s", VTY_NEWLINE);
		vty_out(vty, " voip stream rtp-port               : %d%s", mm->localport, VTY_NEWLINE);
		vty_out(vty, " voip stream payload                : %d%s", mm->payload, VTY_NEWLINE);

		if(strlen(mm->ip))
		{
			vty_out(vty, " voip stream remote                 : %s:%d%s",
					mm->ip, mm->remoteport, VTY_NEWLINE);
		}
		vty_out(vty, " voip stream bitrate                : %d%s", mm->bitrate, VTY_NEWLINE);

		vty_out(vty, " voip stream jitter                 : %d%s", mm->jitter, VTY_NEWLINE);

		vty_out(vty, " voip stream rtcp                   : %s%s", mm->enable_rtcp ? "TRUE":"FALSE",VTY_NEWLINE);

		vty_out(vty, " voip stream avpf                   : %s%s", mm->enable_avpf ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " voip stream echo-canceller         : %s%s", mm->ec ? "TRUE":"FALSE", VTY_NEWLINE);

		vty_out(vty, " voip stream ec-tail                : %d%s", mm->ec_len_ms, VTY_NEWLINE);
		vty_out(vty, " voip stream ec-delay               : %d%s", mm->ec_delay_ms, VTY_NEWLINE);
		vty_out(vty, " voip stream ec-framesize           : %d%s", mm->ec_framesize, VTY_NEWLINE);

		//echo limiter
		vty_out(vty, " voip stream echo-limiter           : %s%s", mm->el ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " voip stream el-force               : %0.2f%s", mm->el_force, VTY_NEWLINE);
		vty_out(vty, " voip stream el-thres               : %0.2f%s", mm->el_thres, VTY_NEWLINE);
		vty_out(vty, " voip stream el-transmit-thres      : %0.2f%s", mm->el_transmit_thres, VTY_NEWLINE);
		vty_out(vty, " voip stream el-sustain             : %d%s", mm->el_sustain, VTY_NEWLINE);

		vty_out(vty, " voip stream noise-gate             : %s%s", mm->use_ng ? "TRUE":"FALSE", VTY_NEWLINE);
		vty_out(vty, " voip stream ng-floorgain           : %0.2f%s", mm->ng_floorgain, VTY_NEWLINE);
		vty_out(vty, " voip stream ng-threshold           : %0.2f%s", mm->ng_threshold, VTY_NEWLINE);
		vty_out(vty, "----------------- voip stream hardware -----------------%s", VTY_NEWLINE);
	}
	return OK;
}
#endif /* PL_VOIP_MEDIASTREAM */


