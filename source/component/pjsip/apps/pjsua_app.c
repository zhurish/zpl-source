/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
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
#include "pjsua_app.h"
#include "pjsua_app_cfgapi.h"
#include "pjsua_app_cb.h"
#include "pjapp_app_util.h"

#include "zpl_media_pjdev.h"

#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"

#define THIS_FILE       "pjsua_app.c"

//#define STEREO_DEMO
//#define TRANSPORT_ADAPTER_SAMPLE
//#define HAVE_MULTIPART_TEST

/* Ringtones                US         UK  */
#define RINGBACK_FREQ1      440     /* 400 */
#define RINGBACK_FREQ2      480     /* 450 */
#define RINGBACK_ON         2000    /* 400 */
#define RINGBACK_OFF        4000    /* 200 */
#define RINGBACK_CNT        1       /* 2   */
#define RINGBACK_INTERVAL   4000    /* 2000 */

#define RING_FREQ1          800
#define RING_FREQ2          640
#define RING_ON             200
#define RING_OFF            100
#define RING_CNT            3
#define RING_INTERVAL       3000

#define current_acc     pjsua_acc_get_default()

#ifdef STEREO_DEMO
static void stereo_demo();
#endif



static void ringback_start(pjsua_call_id call_id);
static void ring_start(pjsua_call_id call_id);
static void ring_stop(pjsua_call_id call_id);
static pj_status_t app_init(void);
static pj_status_t app_destroy(void);




/*****************************************************************************
 * Configuration manipulation
 */

/*****************************************************************************
 * Callback 
 */
static void ringback_start(pjsua_call_id call_id)
{
    if (_pjAppCfg.no_tones)
        return;

    if (_pjAppCfg.call_data[call_id].ringback_on)
        return;

    _pjAppCfg.call_data[call_id].ringback_on = PJ_TRUE;

    if (++_pjAppCfg.ringback_cnt==1 && 
        _pjAppCfg.ringback_slot!=PJSUA_INVALID_ID) 
    {
        pjsua_conf_connect(_pjAppCfg.ringback_slot, 0);
    }
}

static void ring_stop(pjsua_call_id call_id)
{
    if (_pjAppCfg.no_tones)
        return;

    if (_pjAppCfg.call_data[call_id].ringback_on) {
        _pjAppCfg.call_data[call_id].ringback_on = PJ_FALSE;

        pj_assert(_pjAppCfg.ringback_cnt>0);
        if (--_pjAppCfg.ringback_cnt == 0 && 
            _pjAppCfg.ringback_slot!=PJSUA_INVALID_ID) 
        {
            pjsua_conf_disconnect(_pjAppCfg.ringback_slot, 0);
            pjmedia_tonegen_rewind(_pjAppCfg.ringback_port);
        }
    }

    if (_pjAppCfg.call_data[call_id].ring_on) {
        _pjAppCfg.call_data[call_id].ring_on = PJ_FALSE;

        pj_assert(_pjAppCfg.ring_cnt>0);
        if (--_pjAppCfg.ring_cnt == 0 && 
            _pjAppCfg.ring_slot!=PJSUA_INVALID_ID) 
        {
            pjsua_conf_disconnect(_pjAppCfg.ring_slot, 0);
            pjmedia_tonegen_rewind(_pjAppCfg.ring_port);
        }
    }
}

static void ring_start(pjsua_call_id call_id)
{
    if (_pjAppCfg.no_tones)
        return;

    if (_pjAppCfg.call_data[call_id].ring_on)
        return;

    _pjAppCfg.call_data[call_id].ring_on = PJ_TRUE;

    if (++_pjAppCfg.ring_cnt==1 && 
        _pjAppCfg.ring_slot!=PJSUA_INVALID_ID) 
    {
        pjsua_conf_connect(_pjAppCfg.ring_slot, 0);
    }
}

/* Callback from timer when the maximum call duration has been
 * exceeded.
 */
static void call_timeout_callback(pj_timer_heap_t *timer_heap,
                                  struct pj_timer_entry *entry)
{
    pjsua_call_id call_id = entry->id;
    pjsua_msg_data msg_data_;
    pjsip_generic_string_hdr warn;
    pj_str_t hname = pj_str("Warning");
    pj_str_t hvalue = pj_str("399 pjsua \"Call duration exceeded\"");

    PJ_UNUSED_ARG(timer_heap);

    if (call_id == PJSUA_INVALID_ID) {
        PJ_LOG(1,(THIS_FILE, "Invalid call ID in timer callback"));
        return;
    }
    
    /* Add warning header */
    pjsua_msg_data_init(&msg_data_);
    pjsip_generic_string_hdr_init2(&warn, &hname, &hvalue);
    pj_list_push_back(&msg_data_.hdr_list, &warn);

    /* Call duration has been exceeded; disconnect the call */
    PJ_LOG(3,(THIS_FILE, "Duration (%d seconds) has been exceeded "
                         "for call %d, disconnecting the call",
                         _pjAppCfg.duration, call_id));
    entry->id = PJSUA_INVALID_ID;
    pjsua_call_hangup(call_id, 200, NULL, &msg_data_);
    pjapp_user_call_timeout_callback(&_pjAppCfg.cbtbl, call_id, NULL, 0);
}

/*
 * Handler when invite state has changed.
 */
static void pjapp_on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
    pjsua_call_info call_info;

    PJ_UNUSED_ARG(e);

    pjsua_call_get_info(call_id, &call_info);
	if(!pjapp_incoming_call() && (call_info.state != PJSIP_INV_STATE_DISCONNECTED))
	{
		_pjAppCfg.call_cnt++;
	}
    if (call_info.state == PJSIP_INV_STATE_DISCONNECTED) {

        /* Stop all ringback for this call */
        ring_stop(call_id);

        /* Cancel duration timer, if any */
        if (_pjAppCfg.call_data[call_id].timer.id != PJSUA_INVALID_ID) {
            pjapp_call_data *cd = &_pjAppCfg.call_data[call_id];
            pjsip_endpoint *endpt = pjsua_get_pjsip_endpt();

            cd->timer.id = PJSUA_INVALID_ID;
            pjsip_endpt_cancel_timer(endpt, &cd->timer);
        }

        /* Rewind play file when hangup automatically, 
         * since file is not looped
         */
        if (_pjAppCfg.auto_play_hangup)
            pjsua_player_set_pos(_pjAppCfg.wav_id, 0);


        PJ_LOG(3,(THIS_FILE, "Call %d is DISCONNECTED [reason=%d (%.*s)]", 
                  call_id,
                  call_info.last_status,
                  (int)call_info.last_status_text.slen,
                  call_info.last_status_text.ptr));

		pjapp_user_call_state_callback(&_pjAppCfg.cbtbl, call_id, NULL,
				call_info.state);

		PJ_LOG(3,
				(THIS_FILE, "Call %d is DISCONNECTED [reason=%d (%.*s)]", call_id, call_info.last_status, (int)call_info.last_status_text.slen, call_info.last_status_text.ptr));
        if (call_id == _pjAppCfg.current_call) {
            pjapp_find_next_call();
        }

		_pjAppCfg.incomeing = PJ_FALSE;
        /* Dump media state upon disconnected.
         * Now pjsua_media_channel_deinit() automatically log the call dump.
         */
        if (0) {
            PJ_LOG(5,(THIS_FILE, 
                      "Call %d disconnected, dumping media stats..", 
                      call_id));
            pjapp_log_call_dump(call_id);
        }

    } else {

        if (_pjAppCfg.duration != PJSUA_APP_NO_LIMIT_DURATION && 
            call_info.state == PJSIP_INV_STATE_CONFIRMED) 
        {
            /* Schedule timer to hangup call after the specified duration */
            pjapp_call_data *cd = &_pjAppCfg.call_data[call_id];
            pjsip_endpoint *endpt = pjsua_get_pjsip_endpt();
            pj_time_val delay;

            cd->timer.id = call_id;
            delay.sec = _pjAppCfg.duration;
            delay.msec = 0;
            pjsip_endpt_schedule_timer(endpt, &cd->timer, &delay);
        }

        if (call_info.state == PJSIP_INV_STATE_EARLY) {
            int code;
            pj_str_t reason;
            pjsip_msg *msg;

            /* This can only occur because of TX or RX message */
            pj_assert(e->type == PJSIP_EVENT_TSX_STATE);

            if (e->body.tsx_state.type == PJSIP_EVENT_RX_MSG) {
                msg = e->body.tsx_state.src.rdata->msg_info.msg;
            } else {
                msg = e->body.tsx_state.src.tdata->msg;
            }

            code = msg->line.status.code;
            reason = msg->line.status.reason;

            /* Start ringback for 180 for UAC unless there's SDP in 180 */
            if (call_info.role==PJSIP_ROLE_UAC && code==180 && 
                msg->body == NULL && 
                call_info.media_status==PJSUA_CALL_MEDIA_NONE) 
            {
                ringback_start(call_id);
            }

            PJ_LOG(3,(THIS_FILE, "Call %d state changed to %.*s (%d %.*s)", 
                      call_id, (int)call_info.state_text.slen, 
                      call_info.state_text.ptr, code, 
                      (int)reason.slen, reason.ptr));
        } else {
            PJ_LOG(3,(THIS_FILE, "Call %d state changed to %.*s", 
                      call_id,
                      (int)call_info.state_text.slen,
                      call_info.state_text.ptr));
        }

        if (_pjAppCfg.current_call==PJSUA_INVALID_ID)
            _pjAppCfg.current_call = call_id;

		pjapp_user_call_state_callback(&_pjAppCfg.cbtbl, call_id, NULL,
				call_info.state);
    }
}
/* Answer call */
static void delay_auto_answer_call(pj_timer_heap_t *timer_heap,
                                    struct pj_timer_entry *entry)
{
    if ( (_pjAppCfg.auto_answer > 0) &&
    		(_pjAppCfg.incomeing == PJ_TRUE) &&
			(_pjAppCfg.current_call != PJSUA_INVALID_ID) ) {
	//pjsua_call_setting opt;

	//pjsua_call_setting_default(&opt);
	//opt.aud_cnt = cfg->app_cfg.aud_cnt;
	//opt.vid_cnt = cfg->app_cfg.vid.vid_cnt;
    //__ZPL_PJSIP_DEBUG( "===========%s=======%s auto_answer\r\n",THIS_FILE, __func__);
    pjsua_call_answer(_pjAppCfg.current_call, _pjAppCfg.auto_answer, NULL, NULL);
	//pjsua_call_answer2(call_id, &cfg->app_cfg.call_opt, cfg->app_cfg.auto_answer, NULL, NULL);

	pjapp_user_call_incoming_callback(&_pjAppCfg.cbtbl, _pjAppCfg.current_call, NULL, 1);
    _pjAppCfg.auto_hangup_timer.id = 0;
    }
    return ;
}
/*
 * Handler when audio stream is destroyed.
 */
static void pjapp_on_stream_destroyed(pjsua_call_id call_id,
                                pjmedia_stream *strm,
                                unsigned stream_idx)
{
    PJ_UNUSED_ARG(strm);

    /* Now pjsua_media_channel_deinit() automatically log the call dump. */
    if (0) {
        PJ_LOG(5,(THIS_FILE, 
                  "Call %d stream %d destroyed, dumping media stats..", 
                  call_id, stream_idx));
        pjapp_log_call_dump(call_id);
    }
}

/**
 * Handler when there is incoming call.
 */
static void pjapp_on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
                             pjsip_rx_data *rdata)
{
    pjsua_call_info call_info;

    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &call_info);

    if (_pjAppCfg.current_call==PJSUA_INVALID_ID)
	{
        _pjAppCfg.current_call = call_id;
		_pjAppCfg.incomeing = PJ_TRUE;
	}

#ifdef USE_GUI
    showNotification(call_id);
#endif

	if(_pjAppCfg.call_cnt == 0)
	{
		_pjAppCfg.incomeing = PJ_FALSE;
	    pjsua_call_answer(_pjAppCfg.current_call, PJSIP_SC_NOT_ACCEPTABLE, NULL, NULL);
	    return;
	}
    /* Start ringback */
    if (call_info.rem_aud_cnt)
        ring_start(call_id);
    
	//start zhurish
    if (_pjAppCfg.auto_answer > 0) 
    {
        pj_time_val delay;
    	if(pjapp_user_call_incoming_callback(&_pjAppCfg.cbtbl, call_id, &call_info, 0) == -1)
    	{
    	    pjsua_call_answer(_pjAppCfg.current_call, PJSIP_SC_NOT_ACCEPTABLE, NULL, NULL);
    	    return;
    	}
        pj_timer_entry_init(&_pjAppCfg.auto_answer_timer, 0, NULL, 
                                    &delay_auto_answer_call);
    

        _pjAppCfg.auto_answer_timer.id = 1;
        delay.sec = 0;
        delay.msec = 10000; /* Give 200 ms before hangup */
        pjsip_endpt_schedule_timer(pjsua_get_pjsip_endpt(), 
                                &_pjAppCfg.auto_answer_timer, 
                                &delay);

        //pjsua_call_setting opt;
        //cfg->app_cfg.incomeing_call = call_id + 1;
        //os_time_create_once(delay_auto_answer_call, NULL, 10000);
        //pjsua_call_setting_default(&opt);
        //opt.aud_cnt = cfg->app_cfg.aud_cnt;
        //opt.vid_cnt = cfg->app_cfg.vid.vid_cnt;
        //pjsua_call_answer(call_id, cfg->app_cfg.auto_answer, NULL, NULL);
        //pjsua_call_answer2(call_id, &cfg->app_cfg.call_opt, cfg->app_cfg.auto_answer, NULL, NULL);
    }
    else
    {
    	if(pjapp_user_call_incoming_callback(&_pjAppCfg.cbtbl, call_id, &call_info, 0) == -1)
    	{
    	    pjsua_call_answer(_pjAppCfg.current_call, PJSIP_SC_NOT_ACCEPTABLE, NULL, NULL);
    	    return;
    	}
    }
	//end zhurish
    if (_pjAppCfg.auto_answer > 0) {
        pjsua_call_setting opt;

        pjsua_call_setting_default(&opt);
        opt.aud_cnt = _pjAppCfg.aud_cnt;
        opt.vid_cnt = _pjAppCfg.vid.vid_cnt;

        pjsua_call_answer2(call_id, &opt, _pjAppCfg.auto_answer, NULL,
                           NULL);
    }
    
    if (_pjAppCfg.auto_answer < 200) {
        char notif_st[80] = {0};

#if PJSUA_HAS_VIDEO
        if (call_info.rem_offerer && call_info.rem_vid_cnt) {
            snprintf(notif_st, sizeof(notif_st), 
                     "To %s the video, type \"vid %s\" first, "
                     "before answering the call!\n",
                     (_pjAppCfg.vid.vid_cnt? "reject":"accept"),
                     (_pjAppCfg.vid.vid_cnt? "disable":"enable"));
        }
#endif

        PJ_LOG(3,(THIS_FILE,
                  "Incoming call for account %d!\n"
                  "Media count: %d audio & %d video\n"
                  "%s"
                  "From: %.*s\n"
                  "To: %.*s\n"
                  "Press %s to answer or %s to reject call",
                  acc_id,
                  call_info.rem_aud_cnt,
                  call_info.rem_vid_cnt,
                  notif_st,
                  (int)call_info.remote_info.slen,
                  call_info.remote_info.ptr,
                  (int)call_info.local_info.slen,
                  call_info.local_info.ptr,
                  (_pjAppCfg.use_cli?"ca a":"a"),
                  (_pjAppCfg.use_cli?"g":"h")));
    }
}

/* General processing for media state. "mi" is the media index */
static void pjapp_on_call_generic_media_state(pjsua_call_info *ci, unsigned mi,
                                        pj_bool_t *has_error)
{
    const char *status_name[] = {
        "None",
        "Active",
        "Local hold",
        "Remote hold",
        "Error"
    };

    PJ_UNUSED_ARG(has_error);

    pj_assert(ci->media[mi].status <= PJ_ARRAY_SIZE(status_name));
    pj_assert(PJSUA_CALL_MEDIA_ERROR == 4);

    PJ_LOG(4,(THIS_FILE, "Call %d media %d [type=%s], status is %s",
              ci->id, mi, pjmedia_type_name(ci->media[mi].type),
              status_name[ci->media[mi].status]));
}

/* Process audio media state. "mi" is the media index. */
static void pjapp_on_call_audio_state(pjsua_call_info *ci, unsigned mi,
                                pj_bool_t *has_error)
{
    PJ_UNUSED_ARG(has_error);

    /* Stop ringback */
    ring_stop(ci->id);

    /* Connect ports appropriately when media status is ACTIVE or REMOTE HOLD,
     * otherwise we should NOT connect the ports.
     */
    if (ci->media[mi].status == PJSUA_CALL_MEDIA_ACTIVE ||
        ci->media[mi].status == PJSUA_CALL_MEDIA_REMOTE_HOLD)
    {
        pj_bool_t connect_sound = PJ_TRUE;
        pj_bool_t disconnect_mic = PJ_FALSE;
        pjsua_conf_port_id call_conf_slot;

        call_conf_slot = ci->media[mi].stream.aud.conf_slot;

        /* Make sure conf slot is valid (e.g: media dir is not "inactive") */
        if (call_conf_slot == PJSUA_INVALID_ID)
            return;

        /* Loopback sound, if desired */
        if (_pjAppCfg.auto_loop) {
            pjsua_conf_connect(call_conf_slot, call_conf_slot);
            connect_sound = PJ_FALSE;
        }

        /* Automatically record conversation, if desired */
        if (_pjAppCfg.auto_rec && _pjAppCfg.rec_port != PJSUA_INVALID_ID) {
            pjsua_conf_connect(call_conf_slot, _pjAppCfg.rec_port);
        }

        /* Stream a file, if desired */
        if ((_pjAppCfg.auto_play || _pjAppCfg.auto_play_hangup) && 
            _pjAppCfg.wav_port != PJSUA_INVALID_ID)
        {
            pjsua_conf_connect(_pjAppCfg.wav_port, call_conf_slot);
            connect_sound = PJ_FALSE;
        }

        /* Stream AVI, if desired */
        if (_pjAppCfg.avi_auto_play &&
            _pjAppCfg.avi_def_idx != PJSUA_INVALID_ID &&
            _pjAppCfg.avi[_pjAppCfg.avi_def_idx].slot != PJSUA_INVALID_ID)
        {
            pjsua_conf_connect(_pjAppCfg.avi[_pjAppCfg.avi_def_idx].slot,
                               call_conf_slot);
            disconnect_mic = PJ_TRUE;
        }

        /* Put call in conference with other calls, if desired */
        if (_pjAppCfg.auto_conf) {
            pjsua_call_id call_ids[PJSUA_MAX_CALLS];
            unsigned call_cnt=PJ_ARRAY_SIZE(call_ids);
            unsigned i;

            /* Get all calls, and establish media connection between
             * this call and other calls.
             */
            pjsua_enum_calls(call_ids, &call_cnt);

            for (i=0; i<call_cnt; ++i) {
                if (call_ids[i] == ci->id)
                    continue;
                
                if (!pjsua_call_has_media(call_ids[i]))
                    continue;

                pjsua_conf_connect(call_conf_slot,
                                   pjsua_call_get_conf_port(call_ids[i]));
                pjsua_conf_connect(pjsua_call_get_conf_port(call_ids[i]),
                                   call_conf_slot);

                /* Automatically record conversation, if desired */
                if (_pjAppCfg.auto_rec && _pjAppCfg.rec_port !=
                                           PJSUA_INVALID_ID)
                {
                    pjsua_conf_connect(pjsua_call_get_conf_port(call_ids[i]), 
                                       _pjAppCfg.rec_port);
                }

            }

            /* Also connect call to local sound device */
            connect_sound = PJ_TRUE;
        }

        /* Otherwise connect to sound device */
        if (connect_sound) {
            pjsua_conf_connect(call_conf_slot, 0);
            if (!disconnect_mic)
                pjsua_conf_connect(0, call_conf_slot);
	        pjapp_user_call_state_callback(&_pjAppCfg.cbtbl, 0, NULL, PJ_TRUE);

            /* Automatically record conversation, if desired */
            if (_pjAppCfg.auto_rec && _pjAppCfg.rec_port != PJSUA_INVALID_ID)
            {
                pjsua_conf_connect(call_conf_slot, _pjAppCfg.rec_port);
                pjsua_conf_connect(0, _pjAppCfg.rec_port);
            }
        }
    }
}

/* Process video media state. "mi" is the media index. */
static void pjapp_on_call_video_state(pjsua_call_info *ci, unsigned mi,
                                pj_bool_t *has_error)
{
    if (ci->media_status != PJSUA_CALL_MEDIA_ACTIVE)
        return;

    pjapp_arrange_window(ci->media[mi].stream.vid.win_in);

    PJ_UNUSED_ARG(has_error);
}

/*
 * Callback on media state changed event.
 * The action may connect the call to sound device, to file, or
 * to loop the call.
 */
static void pjapp_on_call_media_state(pjsua_call_id call_id)
{
    pjsua_call_info call_info;
    unsigned mi;
    pj_bool_t has_error = PJ_FALSE;

    pjsua_call_get_info(call_id, &call_info);

    for (mi=0; mi<call_info.media_cnt; ++mi) {
        pjapp_on_call_generic_media_state(&call_info, mi, &has_error);

        switch (call_info.media[mi].type) {
        case PJMEDIA_TYPE_AUDIO:
            pjapp_on_call_audio_state(&call_info, mi, &has_error);
            break;
        case PJMEDIA_TYPE_VIDEO:
            pjapp_on_call_video_state(&call_info, mi, &has_error);
            break;
        default:
            /* Make gcc happy about enum not handled by switch/case */
            break;
        }
    }

    if (has_error) {
        pj_str_t reason = pj_str("Media failed");
        pjsua_call_hangup(call_id, 500, &reason, NULL);
    }

#if PJSUA_HAS_VIDEO
    /* Check if remote has just tried to enable video */
    if (call_info.rem_offerer && call_info.rem_vid_cnt)
    {
        int vid_idx;

        /* Check if there is active video */
        vid_idx = pjsua_call_get_vid_stream_idx(call_id);
        if (vid_idx == -1 || call_info.media[vid_idx].dir == PJMEDIA_DIR_NONE) {
            PJ_LOG(3,(THIS_FILE,
                      "Just rejected incoming video offer on call %d, "
                      "use \"vid call enable %d\" or \"vid call add\" to "
                      "enable video!", call_id, vid_idx));
        }
    }
#endif
}

/*
 * DTMF callback.
 */
/*
static void call_on_dtmf_callback(pjsua_call_id call_id, int dtmf)
{
    PJ_LOG(3,(THIS_FILE, "Incoming DTMF on call %d: %c", call_id, dtmf));
}
*/

static void call_on_dtmf_callback2(pjsua_call_id call_id, 
                                   const pjsua_dtmf_info *info)
{    
    char duration[16];
    char method[16];

    duration[0] = '\0';

    switch (info->method) {
    case PJSUA_DTMF_METHOD_RFC2833:
        pj_ansi_snprintf(method, sizeof(method), "RFC2833");
	    pjapp_user_recv_tdmf_callback(&_pjAppCfg.cbtbl, call_id, NULL, info->digit);
        break;
    case PJSUA_DTMF_METHOD_SIP_INFO:
        pj_ansi_snprintf(method, sizeof(method), "SIP INFO");
        pj_ansi_snprintf(duration, sizeof(duration), ":duration(%d)", 
                         info->duration);
	    pjapp_user_recv_tdmf_callback(&_pjAppCfg.cbtbl, call_id, NULL, info->digit);
        break;
    };    
    PJ_LOG(3,(THIS_FILE, "Incoming DTMF on call %d: %c%s, using %s method", 
           call_id, info->digit, duration, method));
}

/*
 * Redirection handler.
 */
static pjsip_redirect_op call_on_redirected(pjsua_call_id call_id, 
                                            const pjsip_uri *target,
                                            const pjsip_event *e)
{
    PJ_UNUSED_ARG(e);

    if (_pjAppCfg.redir_op == PJSIP_REDIRECT_PENDING) {
        char uristr[PJSIP_MAX_URL_SIZE];
        int len;

        len = pjsip_uri_print(PJSIP_URI_IN_FROMTO_HDR, target, uristr, 
                              sizeof(uristr));
        if (len < 1) {
            pj_ansi_strcpy(uristr, "--URI too long--");
        }

        PJ_LOG(3,(THIS_FILE, "Call %d is being redirected to %.*s. "
                  "Press 'Ra' to accept+replace To header, 'RA' to accept, "
                  "'Rr' to reject, or 'Rd' to disconnect.",
                  call_id, len, uristr));
    }

    return _pjAppCfg.redir_op;
}

/*
 * Handler registration status has changed.
 */
static void pjapp_on_reg_state(pjsua_acc_id acc_id)
{
    PJ_UNUSED_ARG(acc_id);

    // Log already written.
    //pjsua_acc_set_online_status(acc_id, info->renew ? PJ_TRUE:PJ_FALSE);
    pjapp_user_register_state_callback(&_pjAppCfg.cbtbl, acc_id, NULL, 0);
}
static void pjapp_on_reg_state2(pjsua_acc_id acc_id, pjsua_reg_info *info)
{
    PJ_UNUSED_ARG(acc_id);
	//pjsua_cfg_t *cfg = &_global_pjapp_cfg;
	//printf("===================Reg state changed (on_reg_state2)%d\r\n", acc_id);
    //pjsua_acc_set_online_status(acc_id, info->renew ? PJ_TRUE:PJ_FALSE);
    pjapp_user_register_state_callback(&_pjAppCfg.cbtbl, acc_id, NULL, info->renew ? PJ_TRUE:PJ_FALSE);
    // Log already written.
}
/*
 * Handler for incoming presence subscription request
 */
static void pjapp_on_incoming_subscribe(pjsua_acc_id acc_id,
                                  pjsua_srv_pres *srv_pres,
                                  pjsua_buddy_id buddy_id,
                                  const pj_str_t *from,
                                  pjsip_rx_data *rdata,
                                  pjsip_status_code *code,
                                  pj_str_t *reason,
                                  pjsua_msg_data *msg_data_)
{
    /* Just accept the request (the default behavior) */
    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(srv_pres);
    PJ_UNUSED_ARG(buddy_id);
    PJ_UNUSED_ARG(from);
    PJ_UNUSED_ARG(rdata);
    PJ_UNUSED_ARG(code);
    PJ_UNUSED_ARG(reason);
    PJ_UNUSED_ARG(msg_data_);
}


/*
 * Handler on buddy state changed.
 */
static void pjapp_on_buddy_state(pjsua_buddy_id buddy_id)
{
    pjsua_buddy_info info;
    pjsua_buddy_get_info(buddy_id, &info);

    PJ_LOG(3,(THIS_FILE, "%.*s status is %.*s, subscription state is %s "
                         "(last termination reason code=%d %.*s)",
              (int)info.uri.slen,
              info.uri.ptr,
              (int)info.status_text.slen,
              info.status_text.ptr,
              info.sub_state_name,
              info.sub_term_code,
              (int)info.sub_term_reason.slen,
              info.sub_term_reason.ptr));
}


/*
 * Subscription state has changed.
 */
static void pjapp_on_buddy_evsub_state(pjsua_buddy_id buddy_id,
                                 pjsip_evsub *sub,
                                 pjsip_event *event)
{
    char event_info[80];

    PJ_UNUSED_ARG(sub);

    event_info[0] = '\0';

    if (event->type == PJSIP_EVENT_TSX_STATE &&
            event->body.tsx_state.type == PJSIP_EVENT_RX_MSG)
    {
        pjsip_rx_data *rdata = event->body.tsx_state.src.rdata;
        snprintf(event_info, sizeof(event_info),
                 " (RX %s)",
                 pjsip_rx_data_get_info(rdata));
    }

    PJ_LOG(4,(THIS_FILE,
              "Buddy %d: subscription state: %s (event: %s%s)",
              buddy_id, pjsip_evsub_get_state_name(sub),
              pjsip_event_str(event->type),
              event_info));

}


/**
 * Incoming IM message (i.e. MESSAGE request)!
 */
static void pjapp_on_pager(pjsua_call_id call_id, const pj_str_t *from, 
                     const pj_str_t *to, const pj_str_t *contact,
                     const pj_str_t *mime_type, const pj_str_t *text)
{
    /* Note: call index may be -1 */
    PJ_UNUSED_ARG(call_id);
    PJ_UNUSED_ARG(to);
    PJ_UNUSED_ARG(contact);
    PJ_UNUSED_ARG(mime_type);

    PJ_LOG(3,(THIS_FILE,"MESSAGE from %.*s: %.*s (%.*s)",
              (int)from->slen, from->ptr,
              (int)text->slen, text->ptr,
              (int)mime_type->slen, mime_type->ptr));
}


/**
 * Received typing indication
 */
static void pjapp_on_typing(pjsua_call_id call_id, const pj_str_t *from,
                      const pj_str_t *to, const pj_str_t *contact,
                      pj_bool_t is_typing)
{
    PJ_UNUSED_ARG(call_id);
    PJ_UNUSED_ARG(to);
    PJ_UNUSED_ARG(contact);

    PJ_LOG(3,(THIS_FILE, "IM indication: %.*s %s",
              (int)from->slen, from->ptr,
              (is_typing?"is typing..":"has stopped typing")));
}


/**
 * Call transfer request status.
 */
static void pjapp_on_call_transfer_status(pjsua_call_id call_id,
                                    int status_code,
                                    const pj_str_t *status_text,
                                    pj_bool_t final,
                                    pj_bool_t *p_cont)
{
    PJ_LOG(3,(THIS_FILE, "Call %d: transfer status=%d (%.*s) %s",
              call_id, status_code,
              (int)status_text->slen, status_text->ptr,
              (final ? "[final]" : "")));

    if (status_code/100 == 2) {
        PJ_LOG(3,(THIS_FILE, 
                  "Call %d: call transferred successfully, disconnecting call",
                  call_id));
        pjsua_call_hangup(call_id, PJSIP_SC_GONE, NULL, NULL);
        *p_cont = PJ_FALSE;
    }
}


/*
 * Notification that call is being replaced.
 */
static void pjapp_on_call_replaced(pjsua_call_id old_call_id,
                             pjsua_call_id new_call_id)
{
    pjsua_call_info old_ci, new_ci;

    pjsua_call_get_info(old_call_id, &old_ci);
    pjsua_call_get_info(new_call_id, &new_ci);

    PJ_LOG(3,(THIS_FILE, "Call %d with %.*s is being replaced by "
                         "call %d with %.*s",
                         old_call_id, 
                         (int)old_ci.remote_info.slen, old_ci.remote_info.ptr,
                         new_call_id,
                         (int)new_ci.remote_info.slen, new_ci.remote_info.ptr));
}


/*
 * NAT type detection callback.
 */
static void pjapp_on_nat_detect(const pj_stun_nat_detect_result *res)
{
    if (res->status != PJ_SUCCESS) {
        pjsua_perror(THIS_FILE, "NAT detection failed", res->status);
    } else {
        PJ_LOG(3, (THIS_FILE, "NAT detected as %s", res->nat_type_name));
    }
}


/*
 * MWI indication
 */
static void pjapp_on_mwi_info(pjsua_acc_id acc_id, pjsua_mwi_info *mwi_info)
{
    pj_str_t body;
    
    PJ_LOG(3,(THIS_FILE, "Received MWI for acc %d:", acc_id));

    if (mwi_info->rdata->msg_info.ctype) {
        const pjsip_ctype_hdr *ctype = mwi_info->rdata->msg_info.ctype;

        PJ_LOG(3,(THIS_FILE, " Content-Type: %.*s/%.*s",
                  (int)ctype->media.type.slen,
                  ctype->media.type.ptr,
                  (int)ctype->media.subtype.slen,
                  ctype->media.subtype.ptr));
    }

    if (!mwi_info->rdata->msg_info.msg->body) {
        PJ_LOG(3,(THIS_FILE, "  no message body"));
        return;
    }

    body.ptr = (char *)mwi_info->rdata->msg_info.msg->body->data;
    body.slen = mwi_info->rdata->msg_info.msg->body->len;

    PJ_LOG(3,(THIS_FILE, " Body:\n%.*s", (int)body.slen, body.ptr));
}


/*
 * Transport status notification
 */
static void pjapp_on_transport_state(pjsip_transport *tp, 
                               pjsip_transport_state state,
                               const pjsip_transport_state_info *info)
{
    char host_port[128];

    pj_addr_str_print(&tp->remote_name.host, 
                      tp->remote_name.port, host_port, sizeof(host_port), 1);
    switch (state) {
    case PJSIP_TP_STATE_CONNECTED:
        {
            PJ_LOG(3,(THIS_FILE, "SIP %s transport is connected to %s",
                     tp->type_name, host_port));
        }
        break;

    case PJSIP_TP_STATE_DISCONNECTED:
        {
            char buf[100];
            int len;

            len = pj_ansi_snprintf(buf, sizeof(buf), "SIP %s transport is "
                      "disconnected from %s", tp->type_name, host_port);
            PJ_CHECK_TRUNC_STR(len, buf, sizeof(buf));
            pjsua_perror(THIS_FILE, buf, info->status);
        }
        break;

    default:
        break;
    }

#if defined(PJSIP_HAS_TLS_TRANSPORT) && PJSIP_HAS_TLS_TRANSPORT!=0

    if (!pj_ansi_stricmp(tp->type_name, "tls") && info->ext_info &&
        (state == PJSIP_TP_STATE_CONNECTED || 
         ((pjsip_tls_state_info*)info->ext_info)->
                                 ssl_sock_info->verify_status != PJ_SUCCESS))
    {
        pjsip_tls_state_info *tls_info = (pjsip_tls_state_info*)info->ext_info;
        pj_ssl_sock_info *ssl_sock_info = tls_info->ssl_sock_info;
        char buf[2048];
        const char *verif_msgs[32];
        unsigned verif_msg_cnt;

        /* Dump server TLS cipher */
        PJ_LOG(4,(THIS_FILE, "TLS cipher used: 0x%06X/%s",
                  ssl_sock_info->cipher,
                  pj_ssl_cipher_name(ssl_sock_info->cipher) ));

        /* Dump server TLS certificate */
        pj_ssl_cert_info_dump(ssl_sock_info->remote_cert_info, "  ",
                              buf, sizeof(buf));
        PJ_LOG(4,(THIS_FILE, "TLS cert info of %s:\n%s", host_port, buf));

        /* Dump server TLS certificate verification result */
        verif_msg_cnt = PJ_ARRAY_SIZE(verif_msgs);
        pj_ssl_cert_get_verify_status_strings(ssl_sock_info->verify_status,
                                              verif_msgs, &verif_msg_cnt);
        PJ_LOG(3,(THIS_FILE, "TLS cert verification result of %s : %s",
                             host_port,
                             (verif_msg_cnt == 1? verif_msgs[0]:"")));
        if (verif_msg_cnt > 1) {
            unsigned i;
            for (i = 0; i < verif_msg_cnt; ++i)
                PJ_LOG(3,(THIS_FILE, "- %s", verif_msgs[i]));
        }

        if (ssl_sock_info->verify_status &&
            !_pjAppCfg.udp_cfg.tls_setting.verify_server) 
        {
            PJ_LOG(3,(THIS_FILE, "PJSUA is configured to ignore TLS cert "
                                 "verification errors"));
        }
    }

#endif

}

/*
 * Notification on ICE error.
 */
static void pjapp_on_ice_transport_error(int index, pj_ice_strans_op op,
                                   pj_status_t status, void *param)
{
    PJ_UNUSED_ARG(op);
    PJ_UNUSED_ARG(param);
    PJ_PERROR(1,(THIS_FILE, status,
                 "ICE keep alive failure for transport %d", index));
}

/*
 * Notification on sound device operation.
 */
static pj_status_t pjapp_on_snd_dev_operation(int operation)
{
    int cap_dev, play_dev;

    pjsua_get_snd_dev(&cap_dev, &play_dev);
    PJ_LOG(3,(THIS_FILE, "Turning sound device %d %d %s", cap_dev, play_dev,
              (operation? "ON":"OFF")));
    return PJ_SUCCESS;
}

static char *get_media_dir(pjmedia_dir dir) {
    switch (dir) {
    case PJMEDIA_DIR_ENCODING:
        return "TX";
    case PJMEDIA_DIR_DECODING:
        return "RX";
    case PJMEDIA_DIR_ENCODING+PJMEDIA_DIR_DECODING:
        return "TX+RX";
    default:
        return "unknown dir";
    }    
}

/* Callback on media events */
static void pjapp_on_call_media_event(pjsua_call_id call_id,
                                unsigned med_idx,
                                pjmedia_event *event)
{
    char event_name[5];

    PJ_LOG(5,(THIS_FILE, "Event %s",
              pjmedia_fourcc_name(event->type, event_name)));

    if (event->type == PJMEDIA_EVENT_MEDIA_TP_ERR) {
        pjmedia_event_media_tp_err_data *err_data;

        err_data = &event->data.med_tp_err;
        PJ_PERROR(3, (THIS_FILE, err_data->status, 
                  "Media transport error event (%s %s %s)",
                  (err_data->type==PJMEDIA_TYPE_AUDIO)?"Audio":"Video",
                  (err_data->is_rtp)?"RTP":"RTCP",
                  get_media_dir(err_data->dir)));
    }
#if PJSUA_HAS_VIDEO
    else if (event->type == PJMEDIA_EVENT_FMT_CHANGED) {
        /* Adjust renderer window size to original video size */
        pjsua_call_info ci;

        pjsua_call_get_info(call_id, &ci);

        if ((ci.media[med_idx].type == PJMEDIA_TYPE_VIDEO) &&
            (ci.media[med_idx].dir & PJMEDIA_DIR_DECODING))
        {
            pjsua_vid_win_id wid;
            pjmedia_rect_size size;
            pjsua_vid_win_info win_info;

            wid = ci.media[med_idx].stream.vid.win_in;
            pjsua_vid_win_get_info(wid, &win_info);

            size = event->data.fmt_changed.new_fmt.det.vid.size;
            if (size.w != win_info.size.w || size.h != win_info.size.h) {
                pjsua_vid_win_set_size(wid, &size);

                /* Re-arrange video windows */
                pjapp_arrange_window(PJSUA_INVALID_ID);
            }
        }
    }
#else
    PJ_UNUSED_ARG(call_id);
    PJ_UNUSED_ARG(med_idx);    
#endif
}

#ifdef TRANSPORT_ADAPTER_SAMPLE
/*
 * This callback is called when media transport needs to be created.
 */
static pjmedia_transport* pjapp_on_create_media_transport(pjsua_call_id call_id,
                                                    unsigned media_idx,
                                                    pjmedia_transport *base_tp,
                                                    unsigned flags)
{
    pjmedia_transport *adapter;
    pj_status_t status;

    /* Create the adapter */
    status = pjmedia_tp_adapter_create(pjsua_get_pjmedia_endpt(),
                                       NULL, base_tp,
                                       (flags & PJSUA_MED_TP_CLOSE_MEMBER),
                                       &adapter);
    if (status != PJ_SUCCESS) {
        PJ_PERROR(1,(THIS_FILE, status, "Error creating adapter"));
        return NULL;
    }

    PJ_LOG(3,(THIS_FILE, "Media transport is created for call %d media %d",
              call_id, media_idx));

    return adapter;
}
#endif

/* Playfile done notification, set timer to hangup calls */
static void pjapp_on_playfile_done(pjmedia_port *port, void *usr_data)
{
    pj_time_val delay;

    PJ_UNUSED_ARG(port);
    PJ_UNUSED_ARG(usr_data);

    /* Just rewind WAV when it is played outside of call */
    if (pjsua_call_get_count() == 0) {
        pjsua_player_set_pos(_pjAppCfg.wav_id, 0);
    }

    /* Timer is already active */
    if (_pjAppCfg.auto_hangup_timer.id == 1)
        return;

    _pjAppCfg.auto_hangup_timer.id = 1;
    delay.sec = 0;
    delay.msec = 200; /* Give 200 ms before hangup */
    pjsip_endpt_schedule_timer(pjsua_get_pjsip_endpt(), 
                               &_pjAppCfg.auto_hangup_timer, 
                               &delay);
}

/* IP change progress callback. */
static void pjapp_on_ip_change_progress(pjsua_ip_change_op op,
                           pj_status_t status,
                           const pjsua_ip_change_op_info *info)
{
    char info_str[128];
    pjsua_acc_info acc_info;
    pjsua_transport_info tp_info;

    if (status == PJ_SUCCESS) {
        switch (op) {
        case PJSUA_IP_CHANGE_OP_RESTART_LIS:
            pjsua_transport_get_info(info->lis_restart.transport_id, &tp_info);
            pj_ansi_snprintf(info_str, sizeof(info_str),
                             "restart transport %.*s",
                             (int)tp_info.info.slen, tp_info.info.ptr);
            break;
        case PJSUA_IP_CHANGE_OP_ACC_SHUTDOWN_TP:
            pjsua_acc_get_info(info->acc_shutdown_tp.acc_id, &acc_info);

            pj_ansi_snprintf(info_str, sizeof(info_str),
                             "transport shutdown for account %.*s",
                             (int)acc_info.acc_uri.slen,
                             acc_info.acc_uri.ptr);
            break;
        case PJSUA_IP_CHANGE_OP_ACC_UPDATE_CONTACT:
            pjsua_acc_get_info(info->acc_shutdown_tp.acc_id, &acc_info);
            if (info->acc_update_contact.code) {
                pj_ansi_snprintf(info_str, sizeof(info_str),
                                 "update contact for account %.*s, code[%d]",
                                 (int)acc_info.acc_uri.slen,
                                 acc_info.acc_uri.ptr,
                                 info->acc_update_contact.code);
            } else {
                pj_ansi_snprintf(info_str, sizeof(info_str),
                                 "update contact for account %.*s",
                                 (int)acc_info.acc_uri.slen,
                                 acc_info.acc_uri.ptr);
            }
            break;
        case PJSUA_IP_CHANGE_OP_ACC_HANGUP_CALLS:
            pjsua_acc_get_info(info->acc_shutdown_tp.acc_id, &acc_info);
            pj_ansi_snprintf(info_str, sizeof(info_str),
                             "hangup call for account %.*s, call_id[%d]",
                             (int)acc_info.acc_uri.slen, acc_info.acc_uri.ptr,
                             info->acc_hangup_calls.call_id);
            break;
        case PJSUA_IP_CHANGE_OP_ACC_REINVITE_CALLS:
            pjsua_acc_get_info(info->acc_shutdown_tp.acc_id, &acc_info);
            pj_ansi_snprintf(info_str, sizeof(info_str),
                             "reinvite call for account %.*s, call_id[%d]",
                             (int)acc_info.acc_uri.slen, acc_info.acc_uri.ptr,
                             info->acc_reinvite_calls.call_id);
            break;
        case PJSUA_IP_CHANGE_OP_COMPLETED:
            pj_ansi_snprintf(info_str, sizeof(info_str),
                             "done");
        default:
            break;
        }
        PJ_LOG(3,(THIS_FILE, "IP change progress report : %s", info_str));

    } else {
        PJ_PERROR(3,(THIS_FILE, status, "IP change progress fail"));
    }
}

/* Auto hangup timer callback */
static void hangup_timeout_callback(pj_timer_heap_t *timer_heap,
                                    struct pj_timer_entry *entry)
{
    PJ_UNUSED_ARG(timer_heap);
    PJ_UNUSED_ARG(entry);

    _pjAppCfg.auto_hangup_timer.id = 0;
    pjsua_call_hangup_all();
}

/*
 * A simple registrar, invoked by default_mod_on_rx_request()
 */
static void pjapp_simple_registrar(pjsip_rx_data *rdata)
{
    pjsip_tx_data *tdata;
    const pjsip_expires_hdr *exp;
    const pjsip_hdr *h;
    unsigned cnt = 0;
    pjsip_generic_string_hdr *srv;
    pj_status_t status;

    status = pjsip_endpt_create_response(pjsua_get_pjsip_endpt(),
                                         rdata, 200, NULL, &tdata);
    if (status != PJ_SUCCESS)
    return;

    exp = (pjsip_expires_hdr *)pjsip_msg_find_hdr(rdata->msg_info.msg, 
                                                  PJSIP_H_EXPIRES, NULL);

    h = rdata->msg_info.msg->hdr.next;
    while (h != &rdata->msg_info.msg->hdr) {
        if (h->type == PJSIP_H_CONTACT) {
            const pjsip_contact_hdr *c = (const pjsip_contact_hdr*)h;
            unsigned e = c->expires;

            if (e != PJSIP_EXPIRES_NOT_SPECIFIED) {
                if (exp)
                    e = exp->ivalue;
                else
                    e = 3600;
            }

            if (e > 0) {
                pjsip_contact_hdr *nc = (pjsip_contact_hdr *)pjsip_hdr_clone(
                                                                tdata->pool, h);
                nc->expires = e;
                pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr*)nc);
                ++cnt;
            }
        }
        h = h->next;
    }

    srv = pjsip_generic_string_hdr_create(tdata->pool, NULL, NULL);
    srv->name = pj_str("Server");
    srv->hvalue = pj_str("pjsua simple registrar");
    pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr*)srv);

    status = pjsip_endpt_send_response2(pjsua_get_pjsip_endpt(),
                       rdata, tdata, NULL, NULL);
        if (status != PJ_SUCCESS) {
            pjsip_tx_data_dec_ref(tdata);
        }

}

/*****************************************************************************
 * A simple module to handle otherwise unhandled request. We will register
 * this with the lowest priority.
 */

/* Notification on incoming request */
static pj_bool_t default_mod_on_rx_request(pjsip_rx_data *rdata)
{
    pjsip_tx_data *tdata;
    pjsip_status_code status_code;
    pj_status_t status;

    /* Don't respond to ACK! */
    if (pjsip_method_cmp(&rdata->msg_info.msg->line.req.method,
                         &pjsip_ack_method) == 0)
        return PJ_TRUE;

    /* Simple registrar */
    if (pjsip_method_cmp(&rdata->msg_info.msg->line.req.method,
                         &pjsip_register_method) == 0)
    {
        pjapp_simple_registrar(rdata);
        return PJ_TRUE;
    }

    /* Create basic response. */
    if (pjsip_method_cmp(&rdata->msg_info.msg->line.req.method, 
                         &pjsip_notify_method) == 0)
    {
        /* Unsolicited NOTIFY's, send with Bad Request */
        status_code = PJSIP_SC_BAD_REQUEST;
    } else {
        /* Probably unknown method */
        status_code = PJSIP_SC_METHOD_NOT_ALLOWED;
    }
    status = pjsip_endpt_create_response(pjsua_get_pjsip_endpt(), 
                                         rdata, status_code, 
                                         NULL, &tdata);
    if (status != PJ_SUCCESS) {
        pjsua_perror(THIS_FILE, "Unable to create response", status);
        return PJ_TRUE;
    }

    /* Add Allow if we're responding with 405 */
    if (status_code == PJSIP_SC_METHOD_NOT_ALLOWED) {
        const pjsip_hdr *cap_hdr;
        cap_hdr = pjsip_endpt_get_capability(pjsua_get_pjsip_endpt(), 
                                             PJSIP_H_ALLOW, NULL);
        if (cap_hdr) {
            pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr *)pjsip_hdr_clone(
                                                         tdata->pool, cap_hdr));
        }
    }

    /* Add User-Agent header */
    {
        pj_str_t user_agent;
        char tmp[80];
        const pj_str_t USER_AGENT = { "User-Agent", 10};
        pjsip_hdr *h;

        pj_ansi_snprintf(tmp, sizeof(tmp), "PJSUA v%s/%s", 
                         pj_get_version(), PJ_OS_NAME);
        pj_strdup2_with_null(tdata->pool, &user_agent, tmp);

        h = (pjsip_hdr*) pjsip_generic_string_hdr_create(tdata->pool,
                                                         &USER_AGENT,
                                                         &user_agent);
        pjsip_msg_add_hdr(tdata->msg, h);
    }

    status = pjsip_endpt_send_response2(pjsua_get_pjsip_endpt(), rdata, tdata, 
                               NULL, NULL);
            if (status != PJ_SUCCESS) pjsip_tx_data_dec_ref(tdata);

    return PJ_TRUE;
}

/* The module instance. */
static pjsip_module mod_default_handler = 
{
    NULL, NULL,                         /* prev, next.          */
    { "mod-default-handler", 19 },      /* Name.                */
    -1,                                 /* Id                   */
    PJSIP_MOD_PRIORITY_APPLICATION+99,  /* Priority             */
    NULL,                               /* load()               */
    NULL,                               /* start()              */
    NULL,                               /* stop()               */
    NULL,                               /* unload()             */
    &default_mod_on_rx_request,         /* on_rx_request()      */
    NULL,                               /* on_rx_response()     */
    NULL,                               /* on_tx_request.       */
    NULL,                               /* on_tx_response()     */
    NULL,                               /* on_tsx_state()       */

};

/** CLI callback **/

/* Called on CLI (re)started, e.g: initial start, after iOS bg */
static void pjapp_cli_started_callback(pj_status_t status)
{
    /* Notify app */
    //if (app_cfg.on_started) 
    {
        if (status == PJ_SUCCESS) {
            char info[128];
            pjapp_cli_get_info(info, sizeof(info));

        } else {
          
        }
    }
}


/*****************************************************************************
 * Public API
 */

static int cfg_refresh_proc(void *arg)
{
    PJ_UNUSED_ARG(arg);
    int cnt = 0;
    /* Set thread to lowest priority so that it doesn't clobber
     * cfg output
     */
    if(_pjAppCfg.use_cli)
    {
        pj_thread_set_prio(pj_thread_this(), 
                        pj_thread_get_prio_min(pj_thread_this()));
    }
	char cmd[512];
	memset(cmd, '\0', sizeof(cmd));
    PJ_LOG(3,(THIS_FILE, "========cfg_refresh_proc========"));
    while (1) 
    {
        PJ_LOG(3,(THIS_FILE, "========cfg_refresh_proc=%d=======", cnt));
        if(cnt < 120)
        {
            cnt++;
            sleep(1);
            continue;
        }
        snprintf(cmd, sizeof(cmd), "sip:%s@%s:%d",
					"102",
					"192.168.10.102",
					5060);
        pj_str_set(&_pjAppCfg.uri_arg, cmd);
        if (_pjAppCfg.uri_arg.slen) {
            PJ_LOG(3,(THIS_FILE, "================call  %s", _pjAppCfg.uri_arg.ptr));
            pjsua_call_setting_default(&_pjAppCfg.call_opt);
            _pjAppCfg.call_opt.aud_cnt = _pjAppCfg.aud_cnt;
            _pjAppCfg.call_opt.vid_cnt = _pjAppCfg.vid.vid_cnt;

            pjsua_call_make_call(current_acc, &_pjAppCfg.uri_arg, &_pjAppCfg.call_opt, NULL, 
                                NULL, NULL);
        }   

        pjapp_sock_read_cmd(&_pjAppCfg.sock);
        //pjapp_sock_write_result(&_pjAppCfg.sock, int ret, char *result);
        //pj_thread_sleep(1 * 1000);
    }

    return 0;
}


static pj_status_t app_init(void)
{
    pjsua_transport_id transport_id = -1;
    pjsua_transport_config tcp_cfg;
    unsigned i;
    pj_pool_t *tmp_pool;
    pj_status_t status;

    pjapp_cfg_log_config(&_pjAppCfg);

    /** Create pjsua **/
    status = pjsua_create();
    if (status != PJ_SUCCESS)
        return status;

    _pjAppCfg._g_pool = pjsua_pool_create("global-pjsua", 100, 64);
    status = pj_mutex_create_simple(_pjAppCfg._g_pool, NULL, &_pjAppCfg._g_lock);
    if (status != PJ_SUCCESS)
        return status;
    /* Create pool for application */
    _pjAppCfg.pool = pjsua_pool_create("pjsua-app", 1000, 1000);
    tmp_pool = pjsua_pool_create("tmp-pjsua", 1000, 1000);;

    /* Init CLI & its FE settings */
    if (!_pjAppCfg.app_running) {
        pj_cli_cfg_default(&_pjAppCfg.cli_cfg.cfg);
        pj_cli_telnet_cfg_default(&_pjAppCfg.cli_cfg.telnet_cfg);
        pj_cli_console_cfg_default(&_pjAppCfg.cli_cfg.console_cfg);
        _pjAppCfg.cli_cfg.telnet_cfg.on_started = pjapp_cli_started_callback;
    }

    /** Parse args **/
    pjapp_config_default_setting(&_pjAppCfg);
    if (status != PJ_SUCCESS) {
        pj_pool_release(tmp_pool);
        return status;
    }

    /* Initialize application callbacks */
    _pjAppCfg.cfg.cb.on_call_state = &pjapp_on_call_state;
    _pjAppCfg.cfg.cb.on_stream_destroyed = &pjapp_on_stream_destroyed;
    _pjAppCfg.cfg.cb.on_call_media_state = &pjapp_on_call_media_state;
    _pjAppCfg.cfg.cb.on_incoming_call = &pjapp_on_incoming_call;
    _pjAppCfg.cfg.cb.on_dtmf_digit2 = &call_on_dtmf_callback2;
    _pjAppCfg.cfg.cb.on_call_redirected = &call_on_redirected;
    _pjAppCfg.cfg.cb.on_reg_state = &pjapp_on_reg_state;
    _pjAppCfg.cfg.cb.on_reg_state2 = &pjapp_on_reg_state2;
    _pjAppCfg.cfg.cb.on_incoming_subscribe = &pjapp_on_incoming_subscribe;
    _pjAppCfg.cfg.cb.on_buddy_state = &pjapp_on_buddy_state;
    _pjAppCfg.cfg.cb.on_buddy_evsub_state = &pjapp_on_buddy_evsub_state;
    _pjAppCfg.cfg.cb.on_pager = &pjapp_on_pager;
    _pjAppCfg.cfg.cb.on_typing = &pjapp_on_typing;
    _pjAppCfg.cfg.cb.on_call_transfer_status = &pjapp_on_call_transfer_status;
    _pjAppCfg.cfg.cb.on_call_replaced = &pjapp_on_call_replaced;
    _pjAppCfg.cfg.cb.on_nat_detect = &pjapp_on_nat_detect;
    _pjAppCfg.cfg.cb.on_mwi_info = &pjapp_on_mwi_info;
    _pjAppCfg.cfg.cb.on_transport_state = &pjapp_on_transport_state;
    _pjAppCfg.cfg.cb.on_ice_transport_error = &pjapp_on_ice_transport_error;
    _pjAppCfg.cfg.cb.on_snd_dev_operation = &pjapp_on_snd_dev_operation;
    _pjAppCfg.cfg.cb.on_call_media_event = &pjapp_on_call_media_event;
    _pjAppCfg.cfg.cb.on_ip_change_progress = &pjapp_on_ip_change_progress;
#ifdef TRANSPORT_ADAPTER_SAMPLE
    _pjAppCfg.cfg.cb.on_create_media_transport = &pjapp_on_create_media_transport;
#endif

    pjapp_user_callback_init(&_pjAppCfg.cbtbl);

    /* Set sound device latency */
    if (_pjAppCfg.capture_lat > 0)
        _pjAppCfg.media_cfg.snd_rec_latency = _pjAppCfg.capture_lat;
    if (_pjAppCfg.playback_lat)
        _pjAppCfg.media_cfg.snd_play_latency = _pjAppCfg.playback_lat;

    /* Initialize pjsua */
    status = pjsua_init(&_pjAppCfg.cfg, &_pjAppCfg.log_cfg,
                        &_pjAppCfg.media_cfg);
    if (status != PJ_SUCCESS) {
        pj_pool_release(tmp_pool);
        return status;
    }

    /* Initialize our module to handle otherwise unhandled request */
    status = pjsip_endpt_register_module(pjsua_get_pjsip_endpt(),
                                         &mod_default_handler);
    if (status != PJ_SUCCESS)
        return status;

#ifdef STEREO_DEMO
    stereo_demo();
#endif
    //pjsua_get_pool_factory())
    //pjmedia_aud_dev_factory* pjmedia_pjdev_hwaudio_factory(pj_pool_factory *pf)
    #ifdef ZPL_HISIMPP_MODULE
    //pjmedia_aud_register_factory(pjmedia_pjdev_hwaudio_factory);
    #endif
    /* Initialize calls data */
    for (i=0; i<PJ_ARRAY_SIZE(_pjAppCfg.call_data); ++i) {
        _pjAppCfg.call_data[i].timer.id = PJSUA_INVALID_ID;
        _pjAppCfg.call_data[i].timer.cb = &call_timeout_callback;
    }

    /* Optionally registers WAV file */
    for (i=0; i<_pjAppCfg.wav_count; ++i) {
        pjsua_player_id wav_id;
        unsigned play_options = 0;

        if (_pjAppCfg.auto_play_hangup)
            play_options |= PJMEDIA_FILE_NO_LOOP;

        status = pjsua_player_create(&_pjAppCfg.wav_files[i], play_options, 
                                     &wav_id);
        if (status != PJ_SUCCESS)
        {
            zm_msg_force_trap("==========pjsua_player_create status=%d", status);
            goto on_error;
        }

        if (_pjAppCfg.wav_id == PJSUA_INVALID_ID) {
            _pjAppCfg.wav_id = wav_id;
            _pjAppCfg.wav_port = pjsua_player_get_conf_port(_pjAppCfg.wav_id);
            if (_pjAppCfg.auto_play_hangup) {
                pjmedia_port *port;

                pjsua_player_get_port(_pjAppCfg.wav_id, &port);
                status = pjmedia_wav_player_set_eof_cb2(port, NULL, 
                                                        &pjapp_on_playfile_done);
                if (status != PJ_SUCCESS)
                {
                    zm_msg_force_trap("==========pjmedia_wav_player_set_eof_cb2 status=%d", status);
                    goto on_error;
                }

                pj_timer_entry_init(&_pjAppCfg.auto_hangup_timer, 0, NULL, 
                                    &hangup_timeout_callback);
            }
        }
    }

    /* Optionally registers tone players */
    for (i=0; i<_pjAppCfg.tone_count; ++i) {
        pjmedia_port *tport;
        char name[80];
        pj_str_t label;
        pj_status_t status2;

        pj_ansi_snprintf(name, sizeof(name), "tone-%d,%d",
                         _pjAppCfg.tones[i].freq1, 
                         _pjAppCfg.tones[i].freq2);
        label = pj_str(name);
        status2 = pjmedia_tonegen_create2(_pjAppCfg.pool, &label,
                                          8000, 1, 160, 16, 
                                          PJMEDIA_TONEGEN_LOOP,  &tport);
        if (status2 != PJ_SUCCESS) {
            pjsua_perror(THIS_FILE, "Unable to create tone generator", status);
            goto on_error;
        }

        status2 = pjsua_conf_add_port(_pjAppCfg.pool, tport,
                                     &_pjAppCfg.tone_slots[i]);
        pj_assert(status2 == PJ_SUCCESS);

        status2 = pjmedia_tonegen_play(tport, 1, &_pjAppCfg.tones[i], 0);
        pj_assert(status2 == PJ_SUCCESS);
    }

    /* Optionally create recorder file, if any. */
    if (_pjAppCfg.rec_file.slen) {
        status = pjsua_recorder_create(&_pjAppCfg.rec_file, 0, NULL, 0, 0,
                                       &_pjAppCfg.rec_id);
        if (status != PJ_SUCCESS)
            goto on_error;

        _pjAppCfg.rec_port = pjsua_recorder_get_conf_port(_pjAppCfg.rec_id);
    }

    pj_memcpy(&tcp_cfg, &_pjAppCfg.udp_cfg, sizeof(tcp_cfg));

    /* Create ringback tones */
    if (_pjAppCfg.no_tones == PJ_FALSE) {
        unsigned samples_per_frame;
        pjmedia_tone_desc tone[RING_CNT+RINGBACK_CNT];
        pj_str_t name;

        samples_per_frame = _pjAppCfg.media_cfg.audio_frame_ptime * 
                            _pjAppCfg.media_cfg.clock_rate *
                            _pjAppCfg.media_cfg.channel_count / 1000;

        /* Ringback tone (call is ringing) */
        name = pj_str("ringback");
        status = pjmedia_tonegen_create2(_pjAppCfg.pool, &name, 
                                         _pjAppCfg.media_cfg.clock_rate,
                                         _pjAppCfg.media_cfg.channel_count, 
                                         samples_per_frame,
                                         16, PJMEDIA_TONEGEN_LOOP, 
                                         &_pjAppCfg.ringback_port);
        if (status != PJ_SUCCESS)
            goto on_error;

        pj_bzero(&tone, sizeof(tone));
        for (i=0; i<RINGBACK_CNT; ++i) {
            tone[i].freq1 = RINGBACK_FREQ1;
            tone[i].freq2 = RINGBACK_FREQ2;
            tone[i].on_msec = RINGBACK_ON;
            tone[i].off_msec = RINGBACK_OFF;
        }
        tone[RINGBACK_CNT-1].off_msec = RINGBACK_INTERVAL;

        pjmedia_tonegen_play(_pjAppCfg.ringback_port, RINGBACK_CNT, tone,
                             PJMEDIA_TONEGEN_LOOP);


        status = pjsua_conf_add_port(_pjAppCfg.pool, _pjAppCfg.ringback_port,
                                     &_pjAppCfg.ringback_slot);
        if (status != PJ_SUCCESS)
            goto on_error;

        /* Ring (to alert incoming call) */
        name = pj_str("ring");
        status = pjmedia_tonegen_create2(_pjAppCfg.pool, &name, 
                                         _pjAppCfg.media_cfg.clock_rate,
                                         _pjAppCfg.media_cfg.channel_count, 
                                         samples_per_frame,
                                         16, PJMEDIA_TONEGEN_LOOP, 
                                         &_pjAppCfg.ring_port);
        if (status != PJ_SUCCESS)
            goto on_error;

        for (i=0; i<RING_CNT; ++i) {
            tone[i].freq1 = RING_FREQ1;
            tone[i].freq2 = RING_FREQ2;
            tone[i].on_msec = RING_ON;
            tone[i].off_msec = RING_OFF;
        }
        tone[RING_CNT-1].off_msec = RING_INTERVAL;

        pjmedia_tonegen_play(_pjAppCfg.ring_port, RING_CNT, 
                             tone, PJMEDIA_TONEGEN_LOOP);

        status = pjsua_conf_add_port(_pjAppCfg.pool, _pjAppCfg.ring_port,
                                     &_pjAppCfg.ring_slot);
        if (status != PJ_SUCCESS)
            goto on_error;

    }

    /* Create AVI player virtual devices */
    if (_pjAppCfg.avi_cnt) {
#if PJMEDIA_HAS_VIDEO && PJMEDIA_VIDEO_DEV_HAS_AVI
        pjmedia_vid_dev_factory *avi_factory;

        status = pjmedia_avi_dev_create_factory(pjsua_get_pool_factory(),
                                                _pjAppCfg.avi_cnt,
                                                &avi_factory);
        if (status != PJ_SUCCESS) {
            PJ_PERROR(1,(THIS_FILE, status, "Error creating AVI factory"));
            goto on_error;
        }

        for (i=0; i<_pjAppCfg.avi_cnt; ++i) {
            pjmedia_avi_dev_param avdp;
            pjmedia_vid_dev_index avid;
            unsigned strm_idx, strm_cnt;

            _pjAppCfg.avi[i].dev_id = PJMEDIA_VID_INVALID_DEV;
            _pjAppCfg.avi[i].slot = PJSUA_INVALID_ID;

            pjmedia_avi_dev_param_default(&avdp);
            avdp.path = _pjAppCfg.avi[i].path;

            status =  pjmedia_avi_dev_alloc(avi_factory, &avdp, &avid);
            if (status != PJ_SUCCESS) {
                PJ_PERROR(1,(THIS_FILE, status,
                             "Error creating AVI player for %.*s",
                             (int)avdp.path.slen, avdp.path.ptr));
                goto on_error;
            }

            PJ_LOG(4,(THIS_FILE, "AVI player %.*s created, dev_id=%d",
                      (int)avdp.title.slen, avdp.title.ptr, avid));

            _pjAppCfg.avi[i].dev_id = avid;
            if (_pjAppCfg.avi_def_idx == PJSUA_INVALID_ID)
                _pjAppCfg.avi_def_idx = i;

            strm_cnt = pjmedia_avi_streams_get_num_streams(avdp.avi_streams);
            for (strm_idx=0; strm_idx<strm_cnt; ++strm_idx) {
                pjmedia_port *aud;
                pjmedia_format *fmt;
                pjsua_conf_port_id slot;
                char fmt_name[5];

                aud = pjmedia_avi_streams_get_stream(avdp.avi_streams,
                                                     strm_idx);
                fmt = &aud->info.fmt;

                pjmedia_fourcc_name(fmt->id, fmt_name);

                if (fmt->id == PJMEDIA_FORMAT_PCM) {
                    status = pjsua_conf_add_port(_pjAppCfg.pool, aud,
                                                 &slot);
                    if (status == PJ_SUCCESS) {
                        PJ_LOG(4,(THIS_FILE,
                                  "AVI %.*s: audio added to slot %d",
                                  (int)avdp.title.slen, avdp.title.ptr,
                                  slot));
                        _pjAppCfg.avi[i].slot = slot;
                    }
                } else {
                    PJ_LOG(4,(THIS_FILE,
                              "AVI %.*s: audio ignored, format=%s",
                              (int)avdp.title.slen, avdp.title.ptr,
                              fmt_name));
                }
            }
        }
#else
        PJ_LOG(2,(THIS_FILE,
                  "Warning: --play-avi is ignored because AVI is disabled"));
#endif  /* PJMEDIA_VIDEO_DEV_HAS_AVI */
    }

    /* Add UDP transport unless it's disabled. */
    if (!_pjAppCfg.no_udp) {
        pjsua_acc_id aid;
        pjsip_transport_type_e type = PJSIP_TRANSPORT_UDP;

        status = pjsua_transport_create(type,
                                        &_pjAppCfg.udp_cfg,
                                        &transport_id);
        if (status != PJ_SUCCESS)
            goto on_error;

        /* Add local account */
        pjsua_acc_add_local(transport_id, PJ_TRUE, &aid);

        /* Adjust local account config based on pjsua app config */
        {
            pjsua_acc_config acc_cfg;
            pjsua_acc_get_config(aid, tmp_pool, &acc_cfg);

            pjapp_config_video_init(&acc_cfg);
            acc_cfg.rtp_cfg = _pjAppCfg.rtp_cfg;
            pjsua_acc_modify(aid, &acc_cfg);
        }

        //pjsua_acc_set_transport(aid, transport_id);
        pjsua_acc_set_online_status(current_acc, PJ_TRUE);

        if (_pjAppCfg.udp_cfg.port == 0) {
            pjsua_transport_info ti;
            pj_sockaddr_in *a;

            pjsua_transport_get_info(transport_id, &ti);
            a = (pj_sockaddr_in*)&ti.local_addr;

            tcp_cfg.port = pj_ntohs(a->sin_port);
        }
    }

    /* Add UDP IPv6 transport unless it's disabled. */
    if (!_pjAppCfg.no_udp && _pjAppCfg.ipv6) {
        pjsua_acc_id aid;
        pjsip_transport_type_e type = PJSIP_TRANSPORT_UDP6;
        pjsua_transport_config udp_cfg;

        udp_cfg = _pjAppCfg.udp_cfg;
        if (udp_cfg.port == 0)
            udp_cfg.port = 5060;
        else
            udp_cfg.port += 10;
        status = pjsua_transport_create(type,
                                        &udp_cfg,
                                        &transport_id);
        if (status != PJ_SUCCESS)
            goto on_error;

        /* Add local account */
        pjsua_acc_add_local(transport_id, PJ_TRUE, &aid);

        /* Adjust local account config based on pjsua app config */
        {
            pjsua_acc_config acc_cfg;
            pjsua_acc_get_config(aid, tmp_pool, &acc_cfg);

            pjapp_config_video_init(&acc_cfg);
            acc_cfg.rtp_cfg = _pjAppCfg.rtp_cfg;
            acc_cfg.ipv6_media_use = PJSUA_IPV6_ENABLED;
            pjsua_acc_modify(aid, &acc_cfg);
        }

        //pjsua_acc_set_transport(aid, transport_id);
        pjsua_acc_set_online_status(current_acc, PJ_TRUE);

        if (_pjAppCfg.udp_cfg.port == 0) {
            pjsua_transport_info ti;

            pjsua_transport_get_info(transport_id, &ti);
            tcp_cfg.port = pj_sockaddr_get_port(&ti.local_addr);
        }
    }

    /* Add TCP transport unless it's disabled */
    if (!_pjAppCfg.no_tcp) {
        pjsua_acc_id aid;

        status = pjsua_transport_create(PJSIP_TRANSPORT_TCP,
                                        &tcp_cfg, 
                                        &transport_id);
        if (status != PJ_SUCCESS)
            goto on_error;

        /* Add local account */
        pjsua_acc_add_local(transport_id, PJ_TRUE, &aid);

        /* Adjust local account config based on pjsua app config */
        {
            pjsua_acc_config acc_cfg;
            pjsua_acc_get_config(aid, tmp_pool, &acc_cfg);

            pjapp_config_video_init(&acc_cfg);
            acc_cfg.rtp_cfg = _pjAppCfg.rtp_cfg;
            pjsua_acc_modify(aid, &acc_cfg);
        }

        pjsua_acc_set_online_status(current_acc, PJ_TRUE);

    }

    /* Add TCP IPv6 transport unless it's disabled. */
    if (!_pjAppCfg.no_tcp && _pjAppCfg.ipv6) {
        pjsua_acc_id aid;
        pjsip_transport_type_e type = PJSIP_TRANSPORT_TCP6;

        tcp_cfg.port += 10;

        status = pjsua_transport_create(type,
                                        &tcp_cfg,
                                        &transport_id);
        if (status != PJ_SUCCESS)
            goto on_error;

        /* Add local account */
        pjsua_acc_add_local(transport_id, PJ_TRUE, &aid);

        /* Adjust local account config based on pjsua app config */
        {
            pjsua_acc_config acc_cfg;
            pjsua_acc_get_config(aid, tmp_pool, &acc_cfg);

            pjapp_config_video_init(&acc_cfg);
            acc_cfg.rtp_cfg = _pjAppCfg.rtp_cfg;
            acc_cfg.ipv6_media_use = PJSUA_IPV6_ENABLED;
            pjsua_acc_modify(aid, &acc_cfg);
        }

        //pjsua_acc_set_transport(aid, transport_id);
        pjsua_acc_set_online_status(current_acc, PJ_TRUE);
    }


#if defined(PJSIP_HAS_TLS_TRANSPORT) && PJSIP_HAS_TLS_TRANSPORT!=0
    /* Add TLS transport when application wants one */
    if (_pjAppCfg.use_tls) {

        pjsua_acc_id acc_id;

        /* Copy the QoS settings */
        tcp_cfg.tls_setting.qos_type = tcp_cfg.qos_type;
        pj_memcpy(&tcp_cfg.tls_setting.qos_params, &tcp_cfg.qos_params, 
                  sizeof(tcp_cfg.qos_params));

        /* Set TLS port as TCP port+1 */
        tcp_cfg.port++;
        status = pjsua_transport_create(PJSIP_TRANSPORT_TLS,
                                        &tcp_cfg, 
                                        &transport_id);
        tcp_cfg.port--;
        if (status != PJ_SUCCESS)
            goto on_error;
        
        /* Add local account */
        pjsua_acc_add_local(transport_id, PJ_FALSE, &acc_id);

        /* Adjust local account config based on pjsua app config */
        {
            pjsua_acc_config acc_cfg;
            pjsua_acc_get_config(acc_id, tmp_pool, &acc_cfg);

            pjapp_config_video_init(&acc_cfg);
            acc_cfg.rtp_cfg = _pjAppCfg.rtp_cfg;
            pjsua_acc_modify(acc_id, &acc_cfg);
        }

        pjsua_acc_set_online_status(acc_id, PJ_TRUE);
    }

    /* Add TLS IPv6 transport unless it's disabled. */
    if (_pjAppCfg.use_tls && _pjAppCfg.ipv6) {
        pjsua_acc_id aid;
        pjsip_transport_type_e type = PJSIP_TRANSPORT_TLS6;

        tcp_cfg.port += 10;

        status = pjsua_transport_create(type,
                                        &tcp_cfg,
                                        &transport_id);
        if (status != PJ_SUCCESS)
            goto on_error;

        /* Add local account */
        pjsua_acc_add_local(transport_id, PJ_TRUE, &aid);

        /* Adjust local account config based on pjsua app config */
        {
            pjsua_acc_config acc_cfg;
            pjsua_acc_get_config(aid, tmp_pool, &acc_cfg);

            pjapp_config_video_init(&acc_cfg);
            acc_cfg.rtp_cfg = _pjAppCfg.rtp_cfg;
            acc_cfg.ipv6_media_use = PJSUA_IPV6_ENABLED;
            pjsua_acc_modify(aid, &acc_cfg);
        }

        //pjsua_acc_set_transport(aid, transport_id);
        pjsua_acc_set_online_status(current_acc, PJ_TRUE);
    }

#endif

    if (transport_id == -1) {
        PJ_LOG(1,(THIS_FILE, "Error: no transport is configured"));
        status = -1;
        goto on_error;
    }


    /* Add accounts */
    for (i=0; i<_pjAppCfg.acc_cnt; ++i) {
        _pjAppCfg.acc_cfg[i].rtp_cfg = _pjAppCfg.rtp_cfg;
        _pjAppCfg.acc_cfg[i].reg_retry_interval = 300;
        _pjAppCfg.acc_cfg[i].reg_first_retry_interval = 60;

        pjapp_config_video_init(&_pjAppCfg.acc_cfg[i]);

        status = pjsua_acc_add(&_pjAppCfg.acc_cfg[i], PJ_TRUE, NULL);
        if (status != PJ_SUCCESS)
            goto on_error;
        pjsua_acc_set_online_status(current_acc, PJ_TRUE);
    }

    /* Add buddies */
    for (i=0; i<_pjAppCfg.buddy_cnt; ++i) {
        status = pjsua_buddy_add(&_pjAppCfg.buddy_cfg[i], NULL);
        if (status != PJ_SUCCESS) {
            PJ_PERROR(1,(THIS_FILE, status, "Error adding buddy"));
            goto on_error;
        }
    }

    /* Optionally disable some codec */
    for (i=0; i<_pjAppCfg.codec_dis_cnt; ++i) {
        pjsua_codec_set_priority(&_pjAppCfg.codec_dis[i],
                                 PJMEDIA_CODEC_PRIO_DISABLED);
#if PJSUA_HAS_VIDEO
        pjsua_vid_codec_set_priority(&_pjAppCfg.codec_dis[i],
                                     PJMEDIA_CODEC_PRIO_DISABLED);
#endif
    }

    /* Optionally set codec orders */
    for (i=0; i<_pjAppCfg.codec_cnt; ++i) {
        pjsua_codec_set_priority(&_pjAppCfg.codec_arg[i],
                                 (pj_uint8_t)(PJMEDIA_CODEC_PRIO_NORMAL+i+9));
#if PJSUA_HAS_VIDEO
        pjsua_vid_codec_set_priority(&_pjAppCfg.codec_arg[i],
                                   (pj_uint8_t)(PJMEDIA_CODEC_PRIO_NORMAL+i+9));
#endif
    }

    /* Use null sound device? */
#ifndef STEREO_DEMO
    if (_pjAppCfg.null_audio) {
        status = pjsua_set_null_snd_dev();
        if (status != PJ_SUCCESS)
            goto on_error;
    }
#endif
    if (_pjAppCfg.capture_dev  == PJSUA_INVALID_ID /*&& strlen(_pjAppCfg.capture_dev_name)*/)
    {
        #if defined(ZPL_BUILD_ARCH_X86_64)||defined(ZPL_BUILD_ARCH_X86)
        #if PJMEDIA_AUDIO_DEV_HAS_PORTAUDIO
        //pjmedia_aud_dev_lookup("PA", "HDA Intel PCH: ALC3232 Analog (hw:1,0)", &_pjAppCfg.capture_dev);
        #else
        //pjmedia_aud_dev_lookup("HISI", "hwaudio0", &_pjAppCfg.capture_dev);
        #endif
        #endif
    }
    if (_pjAppCfg.playback_dev  == PJSUA_INVALID_ID /*&& strlen(_pjAppCfg.playback_dev_name)*/)
    {
        #if defined(ZPL_BUILD_ARCH_X86_64)||defined(ZPL_BUILD_ARCH_X86)
        #if PJMEDIA_AUDIO_DEV_HAS_PORTAUDIO
        //pjmedia_aud_dev_lookup("PA", "HDA Intel HDMI: 0 (hw:0,3)", &_pjAppCfg.playback_dev);
        #else
        //pjmedia_aud_dev_lookup("HISI", "hwaudio0", &_pjAppCfg.playback_dev);//1
        #endif
        #endif
    }
    if (_pjAppCfg.capture_dev  != PJSUA_INVALID_ID ||
        _pjAppCfg.playback_dev != PJSUA_INVALID_ID) 
    {
        status = pjsua_set_snd_dev(_pjAppCfg.capture_dev, 
                                   _pjAppCfg.playback_dev);
        if (status != PJ_SUCCESS)
        {
            zm_msg_force_trap("==========pjsua_set_snd_dev status=%d", status);
            goto on_error;
        }
           
    }
    pjsua_conf_adjust_rx_level(0, _pjAppCfg.mic_level);
    pjsua_conf_adjust_tx_level(0, _pjAppCfg.speaker_level);

    /* Init call setting */
    pjsua_call_setting_default(&_pjAppCfg.call_opt);
    _pjAppCfg.call_opt.aud_cnt = _pjAppCfg.aud_cnt;
    _pjAppCfg.call_opt.vid_cnt = _pjAppCfg.vid.vid_cnt;

#if defined(PJSIP_HAS_TLS_TRANSPORT) && PJSIP_HAS_TLS_TRANSPORT!=0
    /* Wipe out TLS key settings in transport configs */
    pjsip_tls_setting_wipe_keys(&_pjAppCfg.udp_cfg.tls_setting);
#endif

    pj_pool_release(tmp_pool);
    return PJ_SUCCESS;

on_error:

#if defined(PJSIP_HAS_TLS_TRANSPORT) && PJSIP_HAS_TLS_TRANSPORT!=0
    /* Wipe out TLS key settings in transport configs */
    pjsip_tls_setting_wipe_keys(&_pjAppCfg.udp_cfg.tls_setting);
#endif

    pj_pool_release(tmp_pool);
    app_destroy();
    return status;
}

pj_status_t pjsua_app_init(void)
{
    pj_status_t status;

    status = app_init();
    if (status != PJ_SUCCESS)
    {
        zm_msg_force_trap("==========app_init %d", status);
        return status;
    }

    /* Init CLI if configured */    
    if (_pjAppCfg.use_cli) {
        status = pjapp_cli_init();
        zm_msg_force_trap("==========pjapp_cli_init %d", status);
    } 
    pjapp_sock_cfg_default(&_pjAppCfg.sock);
    return status;
}

pj_status_t pjsua_app_run(pj_bool_t wait_telnet_cli)
{
    pj_thread_t *cfg_refresh_thread = NULL;
    pj_status_t status;

    /* Start console refresh thread */
    //if(_pjAppCfg.use_cli == PJ_FALSE)
    {
        if (_pjAppCfg.cfg_refresh > 0) {
            pj_thread_create(_pjAppCfg.pool, "cfg", &cfg_refresh_proc,
                            NULL, 0, 0, &cfg_refresh_thread);
        }
    }
    _pjAppCfg.current_call = PJSUA_INVALID_ID;
    status = pjsua_start();
    if (status != PJ_SUCCESS)
        goto on_return;

    if (_pjAppCfg.use_cli && (_pjAppCfg.cli_cfg.cli_fe & CLI_FE_TELNET)) {
        char info[128];
        pjapp_cli_get_info(info, sizeof(info));

    } else {

    }

    /* If user specifies URI to call, then call the URI */
/*    if (_pjAppCfg.uri_arg.slen) {
        pjsua_call_setting_default(&_pjAppCfg.call_opt);
        _pjAppCfg.call_opt.aud_cnt = _pjAppCfg.aud_cnt;
        _pjAppCfg.call_opt.vid_cnt = _pjAppCfg.vid.vid_cnt;

        pjsua_call_make_call(current_acc, &_pjAppCfg.uri_arg, &_pjAppCfg.call_opt, NULL, 
                             NULL, NULL);
    }   
*/
    _pjAppCfg.app_running = PJ_TRUE;

    if (_pjAppCfg.use_cli)
        pjapp_cli_main(wait_telnet_cli);      
    else
    {
        cfg_refresh_proc(&_pjAppCfg);
    }
        

    status = PJ_SUCCESS;

on_return:
    if (cfg_refresh_thread) {
        pj_thread_join(cfg_refresh_thread);
        pj_thread_destroy(cfg_refresh_thread);
    }
    return status;
}

static pj_status_t app_destroy(void)
{
    pj_status_t status = PJ_SUCCESS;
    unsigned i;
    pj_bool_t use_cli = PJ_FALSE;
    int cli_fe = 0;
    pj_uint16_t cli_telnet_port = 0;

#ifdef STEREO_DEMO
    if (_pjAppCfg.snd) {
        pjmedia_snd_port_destroy(_pjAppCfg.snd);
        _pjAppCfg.snd = NULL;
    }
    if (_pjAppCfg.sc_ch1) {
        pjsua_conf_remove_port(_pjAppCfg.sc_ch1_slot);
        _pjAppCfg.sc_ch1_slot = PJSUA_INVALID_ID;
        pjmedia_port_destroy(_pjAppCfg.sc_ch1);
        _pjAppCfg.sc_ch1 = NULL;
    }
    if (_pjAppCfg.sc) {
        pjmedia_port_destroy(_pjAppCfg.sc);
        _pjAppCfg.sc = NULL;
    }
#endif

    /* Close avi devs and ports */
    for (i=0; i<_pjAppCfg.avi_cnt; ++i) {
        if (_pjAppCfg.avi[i].slot != PJSUA_INVALID_ID) {
            pjsua_conf_remove_port(_pjAppCfg.avi[i].slot);
            _pjAppCfg.avi[i].slot = PJSUA_INVALID_ID;
        }
#if PJMEDIA_HAS_VIDEO && PJMEDIA_VIDEO_DEV_HAS_AVI
        if (_pjAppCfg.avi[i].dev_id != PJMEDIA_VID_INVALID_DEV) {
            pjmedia_avi_dev_free(_pjAppCfg.avi[i].dev_id);
            _pjAppCfg.avi[i].dev_id = PJMEDIA_VID_INVALID_DEV;
        }
#endif
    }

    /* Close ringback port */
    if (_pjAppCfg.ringback_port && 
        _pjAppCfg.ringback_slot != PJSUA_INVALID_ID) 
    {
        pjsua_conf_remove_port(_pjAppCfg.ringback_slot);
        _pjAppCfg.ringback_slot = PJSUA_INVALID_ID;
        pjmedia_port_destroy(_pjAppCfg.ringback_port);
        _pjAppCfg.ringback_port = NULL;
    }

    /* Close ring port */
    if (_pjAppCfg.ring_port && _pjAppCfg.ring_slot != PJSUA_INVALID_ID) {
        pjsua_conf_remove_port(_pjAppCfg.ring_slot);
        _pjAppCfg.ring_slot = PJSUA_INVALID_ID;
        pjmedia_port_destroy(_pjAppCfg.ring_port);
        _pjAppCfg.ring_port = NULL;
    }

    /* Close wav player */
    if (_pjAppCfg.wav_id != PJSUA_INVALID_ID) {
        pjsua_player_destroy(_pjAppCfg.wav_id);
        _pjAppCfg.wav_id = PJSUA_INVALID_ID;
        _pjAppCfg.wav_port = PJSUA_INVALID_ID;
    }

    /* Close wav recorder */
    if (_pjAppCfg.rec_id != PJSUA_INVALID_ID) {
        pjsua_recorder_destroy(_pjAppCfg.rec_id);
        _pjAppCfg.rec_id = PJSUA_INVALID_ID;
        _pjAppCfg.rec_port = PJSUA_INVALID_ID;
    }

    /* Close tone generators */
    for (i=0; i<_pjAppCfg.tone_count; ++i) {
        if (_pjAppCfg.tone_slots[i] != PJSUA_INVALID_ID) {
            pjsua_conf_remove_port(_pjAppCfg.tone_slots[i]);
            _pjAppCfg.tone_slots[i] = PJSUA_INVALID_ID;
        }
    }

#if defined(PJSIP_HAS_TLS_TRANSPORT) && PJSIP_HAS_TLS_TRANSPORT!=0
    /* Wipe out TLS key settings in transport configs */
    pjsip_tls_setting_wipe_keys(&_pjAppCfg.udp_cfg.tls_setting);
#endif

    pj_pool_safe_release(&_pjAppCfg.pool);
    if( _pjAppCfg._g_lock)
        pj_mutex_destroy(_pjAppCfg._g_lock);
    if( _pjAppCfg._g_pool)
        pj_pool_safe_release(&_pjAppCfg._g_pool);
    status = pjsua_destroy();

    if (_pjAppCfg.use_cli) {
        use_cli = _pjAppCfg.use_cli;
        cli_fe = _pjAppCfg.cli_cfg.cli_fe;
        cli_telnet_port = _pjAppCfg.cli_cfg.telnet_cfg.port;   
    }

    /* Reset config */
    pj_bzero(&_pjAppCfg, sizeof(_pjAppCfg));
    _pjAppCfg.wav_id = PJSUA_INVALID_ID;
    _pjAppCfg.rec_id = PJSUA_INVALID_ID;

    if (use_cli) {    
        _pjAppCfg.use_cli = use_cli;
        _pjAppCfg.cli_cfg.cli_fe = cli_fe;
        _pjAppCfg.cli_cfg.telnet_cfg.port = cli_telnet_port;
    }

    return status;
}

pj_status_t pjsua_app_destroy(void)
{
    pj_status_t status;

    status = app_destroy();

    if (_pjAppCfg.use_cli) {   
        pjapp_cli_destroy();
    }
    
    return status;
}

int pjsua_app_exit(void)
{
	_pjAppCfg.app_running = 0;
	return 0;
}
/** ======================= **/

#ifdef STEREO_DEMO
/*
 * In this stereo demo, we open the sound device in stereo mode and
 * arrange the attachment to the PJSUA-LIB conference bridge as such
 * so that channel0/left channel of the sound device corresponds to
 * slot 0 in the bridge, and channel1/right channel of the sound
 * device corresponds to slot 1 in the bridge. Then user can independently
 * feed different media to/from the speakers/microphones channels, by
 * connecting them to slot 0 or 1 respectively.
 *
 * Here's how the connection looks like:
 *
   +-----------+ stereo +-----------------+ 2x mono +-----------+
   | AUDIO DEV |<------>| SPLITCOMB   left|<------->|#0  BRIDGE |
   +-----------+        |            right|<------->|#1         |
                        +-----------------+         +-----------+
 */
static void stereo_demo()
{
    pjmedia_port *conf;
    pj_status_t status;

    /* Disable existing sound device */
    conf = pjsua_set_no_snd_dev();

    /* Create stereo-mono splitter/combiner */
    status = pjmedia_splitcomb_create(_pjAppCfg.pool, 
                                PJMEDIA_PIA_SRATE(&conf->info) /* clock rate */,
                                2           /* stereo */,
                                2 * PJMEDIA_PIA_SPF(&conf->info),
                                PJMEDIA_PIA_BITS(&conf->info),
                                0           /* options */,
                                &_pjAppCfg.sc);
    pj_assert(status == PJ_SUCCESS);

    /* Connect channel0 (left channel?) to conference port slot0 */
    status = pjmedia_splitcomb_set_channel(_pjAppCfg.sc, 0 /* ch0 */, 
                                           0 /*options*/,
                                           conf);
    pj_assert(status == PJ_SUCCESS);

    /* Create reverse channel for channel1 (right channel?)... */
    status = pjmedia_splitcomb_create_rev_channel(_pjAppCfg.pool,
                                                  _pjAppCfg.sc,
                                                  1  /* ch1 */,
                                                  0  /* options */,
                                                  &_pjAppCfg.sc_ch1);
    pj_assert(status == PJ_SUCCESS);

    /* .. and register it to conference bridge (it would be slot1
     * if there's no other devices connected to the bridge)
     */
    status = pjsua_conf_add_port(_pjAppCfg.pool, _pjAppCfg.sc_ch1, 
                                 &_pjAppCfg.sc_ch1_slot);
    pj_assert(status == PJ_SUCCESS);
    
    /* Create sound device */
    status = pjmedia_snd_port_create(_pjAppCfg.pool, -1, -1, 
                                     PJMEDIA_PIA_SRATE(&conf->info),
                                     2      /* stereo */,
                                     2 * PJMEDIA_PIA_SPF(&conf->info),
                                     PJMEDIA_PIA_BITS(&conf->info),
                                     0, &_pjAppCfg.snd);

    pj_assert(status == PJ_SUCCESS);


    /* Connect the splitter to the sound device */
    status = pjmedia_snd_port_connect(_pjAppCfg.snd, _pjAppCfg.sc);
    pj_assert(status == PJ_SUCCESS);
}
#endif
