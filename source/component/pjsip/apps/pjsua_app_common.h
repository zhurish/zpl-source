/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#ifndef __PJSUA_APP_COMMON_H__
#define __PJSUA_APP_COMMON_H__

#include <pjsua-lib/pjsua.h>
#include "pjsua_app_cb.h"
PJ_BEGIN_DECL

#define current_acc     pjsua_acc_get_default()

#define PJSUA_APP_NO_LIMIT_DURATION     (int)0x7FFFFFFF
#define PJSUA_APP_MAX_AVI               4
#define PJSUA_APP_NO_NB                 -2

#define PJAPP_CODEC_MAX		PJMEDIA_CODEC_MGR_MAX_CODECS
#define PJAPP_TONES_MAX		32
#define PJAPP_DEV_NAME_MAX		32

#define PJAPP_NUMBER_MAX		32
#define PJAPP_USERNAME_MAX		32
#define PJAPP_PASSWORD_MAX		32
#define PJAPP_ADDRESS_MAX		32
#define PJAPP_FILE_MAX			64
#define PJAPP_DATA_MAX			128


typedef enum pjsua_app_register_state_s
{
	PJAPP_STATE_UNKNOW,
	PJAPP_STATE_UNREGISTER,
	PJAPP_STATE_REGISTER_FAILED,
	PJAPP_STATE_REGISTER_SUCCESS,
}pjapp_register_state_t;


typedef enum pjsua_app_connect_state_s
{
	PJAPP_STATE_UNCONNECT,
	PJAPP_STATE_CONNECT_FAILED,
	PJAPP_STATE_CONNECT_SUCCESS,
	PJAPP_STATE_CONNECT_LOCAL,
}pjapp_connect_state_t;

typedef enum pjsua_app_call_state_s
{
	//SIP呼叫状态		SIP层面
	PJAPP_STATE_CALL_IDLE,			//呼叫空闲
	PJAPP_STATE_CALL_TRYING,
	PJAPP_STATE_CALL_RINGING,
	PJAPP_STATE_CALL_PICKING,
	PJAPP_STATE_CALL_FAILED,			//呼叫失败
	PJAPP_STATE_CALL_SUCCESS,			//呼叫建立
	PJAPP_STATE_CALL_CANCELLED,			//
	PJAPP_STATE_CALL_CLOSED,				//
	PJAPP_STATE_CALL_RELEASED,

}pjapp_call_state_t;


typedef struct pjsua_app_server_s
{
	pj_uint8_t		sip_address[PJAPP_ADDRESS_MAX];
	pj_uint16_t		sip_port;
	pjapp_connect_state_t state;
}pjapp_register_server_t;

typedef struct pjsua_app_username_s
{
	char				    sip_phone[PJAPP_NUMBER_MAX];
    pjapp_register_server_t		    register_svr;

	pjapp_register_state_t	sip_state;
	pjapp_call_state_t	call_state;
	
	pj_bool_t				is_default;
	pj_uint8_t				id;
	pj_bool_t				is_current;
}pjapp_username_t;

typedef enum pjapp_transport_proto_s
{
	PJAPP_PROTO_UDP,
	PJAPP_PROTO_TCP,
	PJAPP_PROTO_TLS,
	PJAPP_PROTO_DTLS,
}pjapp_transport_proto_t;


typedef struct pjapp_input_result
{
    int   nb_result;
    char *uri_result;
} pjapp_input_result;

/* Call specific data */
typedef struct pjapp_call_data
{
    pj_timer_entry          timer;
    pj_bool_t               ringback_on;
    pj_bool_t               ring_on;
} pjapp_call_data;

/* Video settings */
typedef struct pjapp_video_dev
{
    unsigned                vid_cnt;
    int                     vcapture_dev;
    int                     vrender_dev;
    pj_bool_t               in_auto_show;
    pj_bool_t               out_auto_transmit;
    char                    vcapture_dev_name[PJAPP_DEV_NAME_MAX];
    char                    vrender_dev_name[PJAPP_DEV_NAME_MAX];    
} pjapp_video_dev;

/* Enumeration of CLI frontends */
typedef enum {
    CLI_FE_CONSOLE          = 1,
    CLI_FE_TELNET           = 2,
} CLI_FE;

/** CLI config **/
typedef struct pjapp_cli_cfg_s
{
    /** Bitmask of PJAPP_CLI_FE **/
    int                     cli_fe;
    pj_cli_cfg              cfg;
    pj_cli_telnet_cfg       telnet_cfg;
    pj_cli_console_cfg      console_cfg;
} pjapp_cli_cfg_t;

struct pjapp_sock_cfg
{
	int fd[2];
	pj_uint8_t	ibuf[1024];
	pj_uint32_t ilen;
	pj_uint8_t	obuf[1024];
	pj_uint32_t olen;
	int (*cmd_hander)(struct pjapp_sock_cfg*, int, char*, int);
};


/* Pjsua application data */
typedef struct pjsua_app_config_s
{
    pj_bool_t               initialization;    
    pjsua_config            cfg;
    pjsua_logging_config    log_cfg;
    pjsua_media_config      media_cfg;
    pj_bool_t               no_refersub;
    pj_bool_t               ipv6;
    pj_bool_t               enable_qos;
    pj_bool_t               no_tcp;
    pj_bool_t               no_udp;
    pj_bool_t               use_tls;
    pjsua_transport_config  udp_cfg;
    pjsua_transport_config  rtp_cfg;
    pjsip_redirect_op       redir_op;
    int                     srtp_keying;

    unsigned                acc_cnt;
    pjsua_acc_config        acc_cfg[PJSUA_MAX_ACC];

    unsigned                buddy_cnt;
    pjsua_buddy_config      buddy_cfg[PJSUA_MAX_BUDDIES];

    pjapp_call_data         call_data[PJAPP_DATA_MAX];

    pj_pool_t              *pool;
    /* Compatibility with older pjsua */

    unsigned                codec_cnt;
    pj_str_t                codec_arg[PJAPP_CODEC_MAX];
    unsigned                codec_dis_cnt;
    pj_str_t                codec_dis[PJAPP_CODEC_MAX];
    pj_bool_t               null_audio;
    unsigned                wav_count;
    pj_str_t                wav_files[PJAPP_FILE_MAX];
    unsigned                tone_count;
    pjmedia_tone_desc       tones[PJAPP_TONES_MAX];
    pjsua_conf_port_id      tone_slots[PJAPP_TONES_MAX];
    pjsua_player_id         wav_id;
    pjsua_conf_port_id      wav_port;
    pj_bool_t               auto_play;
    pj_bool_t               auto_play_hangup;
    pj_timer_entry          auto_hangup_timer;
    pj_timer_entry          auto_answer_timer;
    pj_bool_t               auto_loop;
    pj_bool_t               auto_conf;
    pj_str_t                rec_file;
    pj_bool_t               auto_rec;
    pjsua_recorder_id       rec_id;
    pjsua_conf_port_id      rec_port;
    unsigned                auto_answer;
    unsigned                duration;

#ifdef STEREO_DEMO
    pjmedia_snd_port       *snd;
    pjmedia_port           *sc, *sc_ch1;
    pjsua_conf_port_id      sc_ch1_slot;
#endif

    float                   mic_level,
                            speaker_level;

    int                     capture_dev, playback_dev;
    unsigned                capture_lat, playback_lat;
    char                    capture_dev_name[PJAPP_DEV_NAME_MAX];
    char                    playback_dev_name[PJAPP_DEV_NAME_MAX];
    pj_bool_t               no_tones;
    int                     ringback_slot;
    int                     ringback_cnt;
    pjmedia_port           *ringback_port;
    int                     ring_slot;
    int                     ring_cnt;
    pjmedia_port           *ring_port;

    pjapp_video_dev                 vid;
    unsigned                aud_cnt;

    /* AVI to play */
    unsigned                avi_cnt;
    struct {
        pj_str_t                path;
        pjmedia_vid_dev_index   dev_id;
        pjsua_conf_port_id      slot;
    } avi[PJSUA_APP_MAX_AVI];
    pj_bool_t               avi_auto_play;
    int                     avi_def_idx;

    /* CLI setting */
    pj_bool_t               use_cli;
    pjapp_cli_cfg_t               cli_cfg;

    pj_bool_t           pj_inited;
    pj_caching_pool     cli_cp;
    pj_bool_t           cli_cp_inited;
    pj_cli_t            *cli;
    pj_cli_sess         *cli_cons_sess;
    pj_cli_front_end    *telnet_front_end;


    pjsua_call_id        current_call;
    pj_str_t             uri_arg;
    pjsua_call_setting   call_opt;
    pjsua_msg_data       msg_data;

    pjapp_user_callback_tbl	 cbtbl;
    pj_mutex_t          *_g_lock;
    pj_pool_t           *_g_pool;
    pj_bool_t            global_enable;
    pj_bool_t            app_running;
    pj_bool_t            app_start;
    pj_bool_t            restart;
    pj_bool_t            media_quit;
    int					 call_cnt;
    pj_bool_t            incomeing;
    int                  cfg_refresh;
    pj_bool_t            cfg_refresh_quit;

    struct pjapp_sock_cfg   sock;

} pjsua_app_config;

#define PJAPP_GLOBAL_LOCK() if(_pjAppCfg._g_lock) pj_mutex_lock(_pjAppCfg._g_lock)
#define PJAPP_GLOBAL_UNLOCK() if(_pjAppCfg._g_lock) pj_mutex_unlock(_pjAppCfg._g_lock)


/** Extern variable declaration **/

extern pjsua_app_config     _pjAppCfg;



int my_atoi(const char *cs);
int my_atoi2(const pj_str_t *s);


pj_bool_t pjapp_find_next_call(void);
pj_bool_t pjapp_find_prev_call(void);
pj_bool_t pjapp_incoming_call(void);
pjsua_call_id pjapp_current_call(void);
void pjapp_send_request(char *cstr_method, const pj_str_t *dst_uri);
void pjapp_log_call_dump(int call_id);
void pjapp_config_video_init(pjsua_acc_config *acc_cfg);
void pjapp_arrange_window(pjsua_vid_win_id wid);

/** Pjsua cli method **/
void pj_cli_out(pj_cli_cmd_val *cval, const char *fmt,...);
void pj_cli_error_out(pj_cli_cmd_val *cval, int status, const char *fmt,...);
pj_status_t pjapp_cli_init(void);
pj_status_t pjapp_cli_main(pj_bool_t wait_telnet_cli);
void pjapp_cli_destroy(void);
void pjapp_cli_get_info(char *info, pj_size_t size); 

/** Legacy method **/
void pjapp_legacy_main(void);
void pjapp_aud_list_devs(pj_cli_cmd_val *cval);
#if PJSUA_HAS_VIDEO
void pjapp_vid_print_dev(pj_cli_cmd_val *cval, int id, const pjmedia_vid_dev_info *vdi, const char *title);
void pjapp_vid_list_devs(pj_cli_cmd_val *cval);
void pjapp_config_show_video(pj_cli_cmd_val *cval, int acc_id, const pjsua_acc_config *acc_cfg);
#endif

#ifdef HAVE_MULTIPART_TEST
    /*
    * Enable multipart in msg_data and add a dummy body into the
    * multipart bodies.
    */
    void add_multipart(pjsua_msg_data *msg_data);
#  define TEST_MULTIPART(msg_data)      add_multipart(msg_data)
#else
#  define TEST_MULTIPART(msg_data)
#endif


PJ_END_DECL
    
#endif  /* __PJSUA_APP_COMMON_H__ */

