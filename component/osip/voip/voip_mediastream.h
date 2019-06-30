/*
 * voip_mediastream.h
 *
 *  Created on: 2018年12月27日
 *      Author: DELL
 */

#ifndef __VOIP_MEDIASTREAM_H__
#define __VOIP_MEDIASTREAM_H__

#include "zebra.h"
#include "log.h"
#include "voip_def.h"

#define PL_VOIP_MEDIASTREAM

#define VOIP_MEDIASTRREM_CTL_DEBUG

#ifdef PL_VOIP_MEDIASTREAM


#define TARGET_OS_IPHONE 0
#undef VIDEO_ENABLED
#undef __ANDROID__

#define MEDIASTREAM_MAX_ICE_CANDIDATES 3

typedef enum _rcalgo{
	rcalgoNone,
	rcalgoSimple,
	rcalgoAdvanced,
	rcalgoInvalid
}rcalgo;

typedef struct _mediastreamIce_candidate {
	char ip[64];
	char type[6];
	int port;
} mediastreamIce_candidate;

typedef struct _mediastream_global {
	MSFactory *factory;
	int localport,remoteport,payload;
	char ip[64];
	char *send_fmtp;
	char *recv_fmtp;
	int jitter;
	int bitrate;
	int mtu;
	MSVideoSize vs;
	rcalgo rc_algo;
	bool_t ec;
	bool_t agc;
	bool_t eq;
	bool_t is_verbose;
	int device_rotation;
	VideoStream *video;
	char * capture_card;
	char * playback_card;
	char * camera;
	char *infile,*outfile;
	float ng_threshold;
	bool_t use_ng;
	bool_t two_windows;
	bool_t el;
	bool_t enable_srtp;

	bool_t interactive;
	bool_t enable_avpf;
	bool_t enable_rtcp;
	bool_t freeze_on_error;

	float el_speed;
	float el_thres;
	float el_force;
	int el_sustain;
	float el_transmit_thres;
	float ng_floorgain;
	bool_t enable_zrtp;
	PayloadType *custom_pt;
	int video_window_id;
	int preview_window_id;
	/* starting values echo canceller */
	int ec_len_ms, ec_delay_ms, ec_framesize;
	char* srtp_local_master_key;
	char* srtp_remote_master_key;
	OrtpNetworkSimulatorParams netsim;
	float zoom;
	float zoom_cx, zoom_cy;

	AudioStream *audio;
	PayloadType *pt;
	RtpSession *session;
	OrtpEvQueue *q;
	RtpProfile *profile;
	MSBandwidthController *bw_controller;

	IceSession *ice_session;
	mediastreamIce_candidate ice_local_candidates[MEDIASTREAM_MAX_ICE_CANDIDATES];
	mediastreamIce_candidate ice_remote_candidates[MEDIASTREAM_MAX_ICE_CANDIDATES];
	int ice_local_candidates_nb;
	int ice_remote_candidates_nb;
	char * video_display_filter;
	FILE * logfile;
	bool_t enable_speaker;

	bool_t 	enable_play_dtmf;
	bool_t 	enable_dtmf;
	int		ctlfd;
	//FILE 	*ctlfp;
	BOOL	initialization;
	BOOL	running;
	void	*priv;
} mediastream_global;

#pragma pack(1)
typedef struct _mediastream_hdr
{
	u_int8 		type;
	u_int8 		magic;
	u_int16 	len;
	u_int8		data[256];
} mediastream_hdr;
#pragma pack(0)

extern bool_t mediastream_parse_args(int argc, char** argv, mediastream_global* out);
extern mediastream_global* mediastream_init_default(void);
extern void mediastream_setup(mediastream_global* args);

extern void mediastream_running(mediastream_global* args);
extern void mediastream_clear(mediastream_global* args);
extern void *mediastream_lookup_factory(mediastream_global *args);
extern int mediastream_stop_force();

extern int mediastream_hw_init(mediastream_global *args);
extern int mediastream_hw_exit(mediastream_global *args);


#ifdef PL_VOIP_MEDIASTREAM_TEST
extern void setup_media_streams(mediastream_global* args);
extern void clear_mediastreams(mediastream_global* args);
#endif


PayloadType* ms_tools_parse_custom_payload(const char *name);

extern void md_stream_BctbxLogFunc(const char *domain, BctbxLogLevel lev, const char *fmt, va_list args);

extern int voip_mediastream_show_config(void * p, struct vty *vty);

#endif

#endif /* __VOIP_MEDIASTREAM_H__ */
