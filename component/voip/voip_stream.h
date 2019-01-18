/*
 * voip_stream.h
 *
 *  Created on: Jan 6, 2019
 *      Author: zhurish
 */

#ifndef __VOIP_STREAM_H__
#define __VOIP_STREAM_H__



#ifdef PL_VOIP_MEDIASTREAM
#include "voip_mediastream.h"
#endif

#define VOIP_STREAM_DEBUG_TEST

#define VOIP_RTP_PORT_DEFAULT 		5550
#define VOIP_RTP_PAYLOAD_DEFAULT 	0

#define VOIP_RTP_ADDRSS_MAX 	64
#define VOIP_RTP_PAYLOAD_NAME_MAX 	64


typedef struct voip_stream_s
{
#ifdef PL_VOIP_MEDIASTREAM
	mediastream_global *mediastream;
#else
	void		*factory;
	void		*session;
#endif

	BOOL		enable;
	enum
	{
		VOIP_STREAM_NONE,
		VOIP_STREAM_STOP,
		VOIP_STREAM_RUNNING
	}	state;

	u_int16		l_rtp_port;

	char 		r_rtp_address[VOIP_RTP_ADDRSS_MAX];
	u_int16 	r_rtp_port;

	char 		payload_name[VOIP_RTP_PAYLOAD_NAME_MAX];
	u_int16 	payload;


	int 		bitrate;
	int 		jitter;
	BOOL		rtcp;
	BOOL		avpf;

	//echo canceller
	BOOL		echo_canceller;
	int 		ec_tail;
	int 		ec_delay;
	int 		ec_framesize;

	//echo limiter
	BOOL		echo_limiter;
	float 		el_force;
	float 		el_speed;
	float 		el_thres;
	float		el_transmit;
	int			el_sustain;

	//echo limiter
	BOOL		noise_gate;
	float 		ng_floorgain;
	float 		ng_threshold;
}voip_stream_t;

typedef struct voip_stream_remote_s
{
	char 		r_rtp_address[VOIP_RTP_ADDRSS_MAX];
	u_int16 	r_rtp_port;
}voip_stream_remote_t;


extern voip_stream_t *voip_stream;


extern int voip_stream_module_init();
extern int voip_stream_module_exit();
extern int voip_streams_hw_init(voip_stream_t *args);
extern int voip_streams_hw_exit(voip_stream_t *args);

extern BOOL voip_stream_parse_args_api(int argc, char** argv, voip_stream_t* out);


extern int voip_stream_local_port_api(voip_stream_t* out, u_int16 port);
extern int voip_stream_remote_address_port_api(voip_stream_t* out, char *remote, u_int16 port);
extern int voip_stream_payload_type_api(voip_stream_t* out, char *name, int payload);
extern int voip_stream_echo_canceller_api(voip_stream_t* out, BOOL enable, int ec_tail, int ec_delay, int ec_framesize);
extern int voip_stream_echo_limiter_api(voip_stream_t* out, BOOL enable, float force, float speed, int sustain, float thres, float transmit);
extern int voip_stream_bitrate_api(voip_stream_t* out, int bitrate);
extern int voip_stream_jitter_api(voip_stream_t* out, int jitter);
extern int voip_stream_disable_rtcp_api(voip_stream_t* out);
extern int voip_stream_disable_avpf_api(voip_stream_t* out);
extern int voip_stream_noise_gate_api(voip_stream_t* out, BOOL enable, float ng_floorgain, float ng_threshold);

extern int voip_stream_debug_set_api(BOOL enable, char *debug);


extern void voip_stream_setup_api(voip_stream_t* args);

extern void voip_stream_running_api(voip_stream_t* args);
extern void voip_stream_clear_api(voip_stream_t* args);
extern void *voip_stream_lookup_factory_api(voip_stream_t *args);

extern void *voip_lookup_stream_api();

extern int voip_stream_start_api(voip_stream_t *args);
extern int voip_stream_shell_start_api(int argc, char * argv[]);
extern int voip_stream_stop_api(void);
extern int voip_stream_stop_force_api(void);

extern int voip_stream_remote_get_api(voip_stream_remote_t* args);

extern int voip_stream_ctlfd_get_api(void);

extern BOOL voip_stream_is_enable();
extern int voip_stream_write_config(struct vty *vty);
extern int voip_stream_show_config(struct vty *vty);


#ifdef VOIP_STREAM_DEBUG_TEST
/*
 * just for shell
 */
extern int _voip_stream_start_api_shell(char *address, int local, int remote);
#endif;
#endif /* __VOIP_STREAM_H__ */
