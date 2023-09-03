/*
 * Copyright (C) 2009-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2007-2009 Keystream AB and Konftel AB, All rights reserved.
 *                         Author: <dan.aberg@keystream.se>
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

#if defined(ZPL_PJSIP_HISIAUDIO) && ZPL_PJSIP_HISIAUDIO!=0
#include <pjmedia_audiodev.h>
#include <pjmedia-videodev/videodev_imp.h>
#include <pj/assert.h>
#include <pj/log.h>
#include <pj/os.h>
#include <pj/pool.h>
#include <pjmedia/errno.h>

#include "zpl_media_pjdev.h"

#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_vidhal.h"
#include "zpl_vidhal_internal.h"


#define THIS_FILE                       "pjdev.c"

#define MAX_DEVICES                     4
#define MAX_MIX_NAME_LEN                64 

/* Set to 1 to enable tracing */
#define ENABLE_TRACING                  1


#if ENABLE_TRACING
#       define TRACE_(t, format, ...)             zm_msg_force_trap("%s:"format, t, ##__VA_ARGS__)
#else
#       define TRACE_(t, format, ...)
#endif



/*
 * Factory prototypes
 */
static pj_status_t pjdev_hwaudio_factory_init(pjmedia_aud_dev_factory *f);
static pj_status_t pjdev_hwaudio_factory_destroy(pjmedia_aud_dev_factory *f);
static pj_status_t pjdev_hwaudio_factory_refresh(pjmedia_aud_dev_factory *f);
static unsigned    pjdev_hwaudio_factory_get_dev_count(pjmedia_aud_dev_factory *f);
static pj_status_t pjdev_hwaudio_factory_get_dev_info(pjmedia_aud_dev_factory *f,
                                             unsigned index,
                                             pjmedia_aud_dev_info *info);
static pj_status_t pjdev_hwaudio_factory_default_param(pjmedia_aud_dev_factory *f,
                                              unsigned index,
                                              pjmedia_aud_param *param);
static pj_status_t pjdev_hwaudio_factory_create_stream(pjmedia_aud_dev_factory *f,
                                              const pjmedia_aud_param *param,
                                              pjmedia_aud_rec_cb rec_cb,
                                              pjmedia_aud_play_cb play_cb,
                                              void *user_data,
                                              pjmedia_aud_stream **p_strm);

/*
 * Stream prototypes
 */
static pj_status_t pjdev_hwaudio_stream_get_param(pjmedia_aud_stream *strm,
                                         pjmedia_aud_param *param);
static pj_status_t pjdev_hwaudio_stream_get_cap(pjmedia_aud_stream *strm,
                                       pjmedia_aud_dev_cap cap,
                                       void *value);
static pj_status_t pjdev_hwaudio_stream_set_cap(pjmedia_aud_stream *strm,
                                       pjmedia_aud_dev_cap cap,
                                       const void *value);
static pj_status_t pjdev_hwaudio_stream_start(pjmedia_aud_stream *strm);
static pj_status_t pjdev_hwaudio_stream_stop(pjmedia_aud_stream *strm);
static pj_status_t pjdev_hwaudio_stream_destroy(pjmedia_aud_stream *strm);


struct pjdev_hwaudio_factory
{
    pjmedia_aud_dev_factory      base;
    pj_pool_factory             *pf;
    pj_pool_t                   *pool;
    pj_pool_t                   *base_pool;

    unsigned                     dev_cnt;
    pjmedia_aud_dev_info         devs[MAX_DEVICES];
};

struct pjdev_hwaudio_stream
{
    pjmedia_aud_stream   base;

    /* Common */
    pj_pool_t           *pool;
    struct pjdev_hwaudio_factory *af;
    void                *user_data;
    pjmedia_aud_param    param;         /* Running parameter            */
    int quit;
    
    /* Playback */
    pjmedia_aud_play_cb  pb_cb;
    unsigned             pb_buf_size;
    char                *pb_buf;
    pj_timestamp        pb_tstamp;
    unsigned            pb_tstamp_interval;
    void                *pb_prev;
    pj_thread_t         *pb_thread;
    
    /* Capture */
    pjmedia_aud_rec_cb   ca_cb;
    pj_mutex_t          *ca_mutex;
    pj_sem_t            *ca_sem;
    unsigned             ca_buf_size;
    char                *ca_buf;
    int ca_len;
    pj_timestamp        ca_tstamp;
    unsigned            ca_tstamp_interval;
    void                *ca_prev;
    pj_thread_t         *ca_thread;
    int cbid;
};

static pjmedia_aud_dev_factory_op pjdev_hwaudio_factory_op =
{
    &pjdev_hwaudio_factory_init,
    &pjdev_hwaudio_factory_destroy,
    &pjdev_hwaudio_factory_get_dev_count,
    &pjdev_hwaudio_factory_get_dev_info,
    &pjdev_hwaudio_factory_default_param,
    &pjdev_hwaudio_factory_create_stream,
    &pjdev_hwaudio_factory_refresh
};

static pjmedia_aud_stream_op pjdev_hwaudio_stream_op =
{
    &pjdev_hwaudio_stream_get_param,
    &pjdev_hwaudio_stream_get_cap,
    &pjdev_hwaudio_stream_set_cap,
    &pjdev_hwaudio_stream_start,
    &pjdev_hwaudio_stream_stop,
    &pjdev_hwaudio_stream_destroy
};



static pj_status_t pjdev_hwaudio_add_dev (struct pjdev_hwaudio_factory *af, const char *dev_name)
{
    pjmedia_aud_dev_info *adi;

    int pb_result = 0, ca_result = 0;

    if (af->dev_cnt >= PJ_ARRAY_SIZE(af->devs))
        return PJ_ETOOMANY;

    adi = &af->devs[af->dev_cnt];

    TRACE_(THIS_FILE, "pjdev_hwaudio_add_dev (%s): Enter", dev_name);

    /* Reset device info */
    pj_bzero(adi, sizeof(*adi));

    /* Set device name */
    strncpy(adi->name, dev_name, sizeof(adi->name));

    /* Check the number of playback channels */
    adi->output_count = (pb_result>=0) ? 1 : 0;

    /* Check the number of capture channels */
    adi->input_count = (ca_result>=0) ? 1 : 0;

    /* Set the default sample rate */
    adi->default_samples_per_sec = 8000;

    /* Driver name */
    strcpy(adi->driver, "HISI");

    ++af->dev_cnt;

    TRACE_ (THIS_FILE, "Added sound device %s", adi->name);

    return PJ_SUCCESS;
}



/* Create HW audio driver. */
pjmedia_aud_dev_factory* pjmedia_pjdev_hwaudio_factory(pj_pool_factory *pf)
{
    struct pjdev_hwaudio_factory *af;
    pj_pool_t *pool;

    pool = pj_pool_create(pf, "pjdev_aud_base", 256, 256, NULL);
    af = PJ_POOL_ZALLOC_T(pool, struct pjdev_hwaudio_factory);
    af->pf = pf;
    af->base_pool = pool;
    af->base.op = &pjdev_hwaudio_factory_op;
    zm_msg_force_trap("==========pjmedia_pjdev_hwaudio_factory");
    return &af->base;
}


/* API: init factory */
static pj_status_t pjdev_hwaudio_factory_init(pjmedia_aud_dev_factory *f)
{
    pj_status_t status = pjdev_hwaudio_factory_refresh(f);
    if (PJ_SUCCESS != status)
        return status;

    TRACE_(THIS_FILE, "HW initialized");
    return PJ_SUCCESS;
}


/* API: destroy factory */
static pj_status_t pjdev_hwaudio_factory_destroy(pjmedia_aud_dev_factory *f)
{
    struct pjdev_hwaudio_factory *af = (struct pjdev_hwaudio_factory*)f;

    if (af->pool)
        pj_pool_release(af->pool);

    if (af->base_pool) {
        pj_pool_t *pool = af->base_pool;
        af->base_pool = NULL;
        pj_pool_release(pool);
    }

    return PJ_SUCCESS;
}


/* API: refresh the device list */
static pj_status_t pjdev_hwaudio_factory_refresh(pjmedia_aud_dev_factory *f)
{
    struct pjdev_hwaudio_factory *af = (struct pjdev_hwaudio_factory*)f;

    TRACE_(THIS_FILE, "pjmedia_snd_init: Enumerate sound devices");

    if (af->pool != NULL) {
        pj_pool_release(af->pool);
        af->pool = NULL;
    }

    af->pool = pj_pool_create(af->pf, "pjdev_hwaudio_aud", 256, 256, NULL);
    af->dev_cnt = 0;

    pjdev_hwaudio_add_dev(af, "hwaudio0");
    pjdev_hwaudio_add_dev(af, "hwaudio1");
    TRACE_(THIS_FILE, "HW driver found %d devices", af->dev_cnt);

    return PJ_SUCCESS;
}


/* API: get device count */
static unsigned  pjdev_hwaudio_factory_get_dev_count(pjmedia_aud_dev_factory *f)
{
    struct pjdev_hwaudio_factory *af = (struct pjdev_hwaudio_factory*)f;
    return af->dev_cnt;
}


/* API: get device info */
static pj_status_t pjdev_hwaudio_factory_get_dev_info(pjmedia_aud_dev_factory *f,
                                             unsigned index,
                                             pjmedia_aud_dev_info *info)
{
    struct pjdev_hwaudio_factory *af = (struct pjdev_hwaudio_factory*)f;

    PJ_ASSERT_RETURN(index>=0 && index<af->dev_cnt, PJ_EINVAL);

    pj_memcpy(info, &af->devs[index], sizeof(*info));
    info->caps = PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY |
                 PJMEDIA_AUD_DEV_CAP_OUTPUT_LATENCY/* |
                 PJMEDIA_AUD_DEV_CAP_INPUT_VOLUME_SETTING |
                 PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING*/;

    return PJ_SUCCESS;
}

/* API: create default parameter */
static pj_status_t pjdev_hwaudio_factory_default_param(pjmedia_aud_dev_factory *f,
                                              unsigned index,
                                              pjmedia_aud_param *param)
{
    zpl_int32 channel = 0, bitrate = 0;
    struct pjdev_hwaudio_factory *af = (struct pjdev_hwaudio_factory*)f;
    pjmedia_aud_dev_info *adi;
    zpl_media_audio_channel_t *audio;
    PJ_ASSERT_RETURN(index>=0 && index<af->dev_cnt, PJ_EINVAL);

    adi = &af->devs[index];
    if(adi)
    {
        sscanf(adi->name, "hwaudio%d", &channel);
        channel += ZPL_MEDIA_CHANNEL_AUDIO_0;
        audio = zpl_media_audio_lookup(channel);
        if(audio)
        {
            pj_bzero(param, sizeof(*param));
            if (adi->input_count && adi->output_count) {
                param->dir = PJMEDIA_DIR_CAPTURE_PLAYBACK;
                param->rec_id = index;
                param->play_id = index;
            } else if (adi->input_count) {
                param->dir = PJMEDIA_DIR_CAPTURE;
                param->rec_id = index;
                param->play_id = PJMEDIA_AUD_INVALID_DEV;
            } else if (adi->output_count) {
                param->dir = PJMEDIA_DIR_PLAYBACK;
                param->play_id = index;
                param->rec_id = PJMEDIA_AUD_INVALID_DEV;
            } else {
                return PJMEDIA_EAUD_INVDEV;
            }

            param->clock_rate = audio->input.clock_rate;
            param->channel_count = audio->input.channel_cnt;
            param->bits_per_sample = 8*(audio->input.bits_per_sample+1);
            param->samples_per_frame = audio->input.max_frame_size;  //param->clock_rate * audio->audio_param.input.encode.codec.framerate / 1000;
            param->flags = adi->caps;
            param->input_latency_ms = PJMEDIA_SND_DEFAULT_REC_LATENCY;
            param->output_latency_ms = PJMEDIA_SND_DEFAULT_PLAY_LATENCY;

            bitrate = param->clock_rate * param->channel_count * param->bits_per_sample;
            adi->ext_fmt_cnt = 2;
            pjmedia_format_init_audio(&adi->ext_fmt[0],
                                                PJMEDIA_FORMAT_PCMU,
                                                param->clock_rate,
                                                param->channel_count,
                                                param->bits_per_sample,
                                                param->samples_per_frame,
                                                bitrate,
                                                bitrate);

            pjmedia_format_init_audio(&adi->ext_fmt[1],
                                                PJMEDIA_FORMAT_PCMA,
                                                param->clock_rate,
                                                param->channel_count,
                                                param->bits_per_sample,
                                                param->samples_per_frame,
                                                bitrate,
                                                bitrate);    

            TRACE_(THIS_FILE, "=============default_param HW devices %s input_count=%d output_count=%d clock_rate=%d channel_count=%d samples_per_frame=%d bits_per_sample=%d", 
                adi->name, adi->input_count, adi->output_count, param->clock_rate, param->channel_count, 
                param->samples_per_frame, param->bits_per_sample);
            return PJ_SUCCESS;
        }
        else
        {
            param->dir = PJMEDIA_DIR_CAPTURE_PLAYBACK;
            param->rec_id = index;
            param->play_id = index;
            param->clock_rate = 8000;
            param->channel_count = 1;
            param->samples_per_frame = ((param->clock_rate*param->channel_count*param->bits_per_sample)/8)/50;
            param->bits_per_sample = 16;
            param->flags = 0;
            param->input_latency_ms = PJMEDIA_SND_DEFAULT_REC_LATENCY;
            param->output_latency_ms = PJMEDIA_SND_DEFAULT_PLAY_LATENCY;

            bitrate = param->clock_rate * param->channel_count * param->bits_per_sample;
            adi->ext_fmt_cnt = 2;
            pjmedia_format_init_audio(&adi->ext_fmt[0],
                                                PJMEDIA_FORMAT_PCMU,
                                                param->clock_rate,
                                                param->channel_count,
                                                param->bits_per_sample,
                                                param->samples_per_frame,
                                                bitrate,
                                                bitrate);

            pjmedia_format_init_audio(&adi->ext_fmt[1],
                                                PJMEDIA_FORMAT_PCMA,
                                                param->clock_rate,
                                                param->channel_count,
                                                param->bits_per_sample,
                                                param->samples_per_frame,
                                                bitrate,
                                                bitrate);    
            zm_msg_force_trap("==========pjdev_hwaudio_factory_default_param");
            return PJ_SUCCESS;
        }
    }
    return -1;
}

/* API: create stream */
static pj_status_t pjdev_hwaudio_factory_create_stream(pjmedia_aud_dev_factory *f,
                                              const pjmedia_aud_param *param,
                                              pjmedia_aud_rec_cb rec_cb,
                                              pjmedia_aud_play_cb play_cb,
                                              void *user_data,
                                              pjmedia_aud_stream **p_strm)
{
    struct pjdev_hwaudio_factory *af = (struct pjdev_hwaudio_factory*)f;
    pj_status_t status = -1;
    pj_pool_t* pool;
    struct pjdev_hwaudio_stream* stream;
    int channel = 0;
    zpl_media_audio_channel_t *audio;
    pool = pj_pool_create (af->pf, "hisi%p", 1024, 1024, NULL);
    if (!pool)
        return PJ_ENOMEM;

    /* Allocate and initialize comon stream data */
    stream = PJ_POOL_ZALLOC_T (pool, struct pjdev_hwaudio_stream);
    stream->base.op = &pjdev_hwaudio_stream_op;
    stream->pool      = pool;
    stream->af        = af;
    stream->user_data = user_data;
    stream->pb_cb     = play_cb;
    stream->ca_cb     = rec_cb;
    pj_mutex_create_simple(pool, NULL, &stream->ca_mutex);
    pj_sem_create(pool, NULL, 0, 1, &stream->ca_sem);

    pj_memcpy(&stream->param, param, sizeof(*param));

    /* Init playback */
    if (param->dir & PJMEDIA_DIR_PLAYBACK) {
        sscanf(stream->af->devs[param->play_id].name, "hwaudio%d", &channel);
        channel += ZPL_MEDIA_CHANNEL_AUDIO_0;
        audio = zpl_media_audio_lookup(channel);
        if(audio)
        {
            stream->pb_prev = audio;
            stream->param.output_latency_ms = 10;
            stream->pb_tstamp_interval = audio->output.max_frame_size;
            /* Set our buffer */
            stream->pb_buf_size = 1024;//param->clock_rate * param->channel_count *(param->bits_per_sample/8);
            stream->pb_buf = (char*) pj_pool_alloc (stream->pool, stream->pb_buf_size);
            TRACE_(THIS_FILE, "===================create_stream HW playback driver %s clock_rate=%d channel_count=%d samples_per_frame=%d bits_per_sample=%d, pb_tstamp_interval=%d", 
                stream->af->devs[param->play_id].name, param->clock_rate, param->channel_count, 
                param->samples_per_frame, param->bits_per_sample, stream->pb_tstamp_interval);
        }
        else 
        {
            zm_msg_force_trap("==========pjdev_hwaudio_factory_create_stream no playback device");
            pj_pool_release (pool);
            return status;
        }
    }

    /* Init capture */
    if (param->dir & PJMEDIA_DIR_CAPTURE) {
        sscanf(stream->af->devs[param->rec_id].name, "hwaudio%d", &channel);
        channel += ZPL_MEDIA_CHANNEL_AUDIO_0;
        audio = zpl_media_audio_lookup(channel);
        if(audio)
        {
            stream->ca_prev = audio;
            stream->param.input_latency_ms = 10;
            stream->ca_tstamp_interval = audio->input.max_frame_size;
            /* Set our buffer */
            stream->ca_buf_size = 1024;//param->clock_rate * param->channel_count *(param->bits_per_sample/8);
            stream->ca_buf = (char*) pj_pool_alloc (stream->pool, stream->ca_buf_size);
            TRACE_(THIS_FILE, "===================create_stream HW capture driver %s clock_rate=%d channel_count=%d samples_per_frame=%d bits_per_sample=%d, ca_tstamp_interval=%d", 
                stream->af->devs[param->play_id].name, param->clock_rate, param->channel_count, 
                param->samples_per_frame, param->bits_per_sample, stream->ca_tstamp_interval);
        }
        else 
        {
            zm_msg_force_trap("==========pjdev_hwaudio_factory_create_stream no capture device");
            pj_pool_release (pool);
            return status;
        }
    }

    *p_strm = &stream->base;
    return PJ_SUCCESS;
}


/* API: get running parameter */
static pj_status_t pjdev_hwaudio_stream_get_param(pjmedia_aud_stream *s,
                                         pjmedia_aud_param *pi)
{
    struct pjdev_hwaudio_stream *stream = (struct pjdev_hwaudio_stream*)s;

    PJ_ASSERT_RETURN(s && pi, PJ_EINVAL);
    zm_msg_force_trap("==========pjdev_hwaudio_stream_get_param");
    pj_memcpy(pi, &stream->param, sizeof(*pi));

    return PJ_SUCCESS;
}


/* API: get capability */
static pj_status_t pjdev_hwaudio_stream_get_cap(pjmedia_aud_stream *s,
                                       pjmedia_aud_dev_cap cap,
                                       void *pval)
{
    struct pjdev_hwaudio_stream *stream = (struct pjdev_hwaudio_stream*)s;

    PJ_ASSERT_RETURN(s && pval, PJ_EINVAL);
    zm_msg_force_trap("==========pjdev_hwaudio_stream_get_cap");
    if (cap==PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY &&
        (stream->param.dir & PJMEDIA_DIR_CAPTURE))
    {
        /* Recording latency */
        *(unsigned*)pval = stream->param.input_latency_ms;
        return PJ_SUCCESS;
    } else if (cap==PJMEDIA_AUD_DEV_CAP_OUTPUT_LATENCY &&
               (stream->param.dir & PJMEDIA_DIR_PLAYBACK))
    {
        /* Playback latency */
        *(unsigned*)pval = stream->param.output_latency_ms;
        return PJ_SUCCESS;
    } else {
        return PJMEDIA_EAUD_INVCAP;
    }
}


/* API: set capability */
static pj_status_t pjdev_hwaudio_stream_set_cap(pjmedia_aud_stream *strm,
                                       pjmedia_aud_dev_cap cap,
                                       const void *value)
{
    struct pjdev_hwaudio_factory *af = ((struct pjdev_hwaudio_stream*)strm)->af;

    return PJMEDIA_EAUD_INVCAP;
}

static int pjdev_hwaudio_stream_clone(zpl_media_channel_t *mediachn,
                                          const zpl_skbuffer_t *bufdata, void *pVoidUser)
{
    int ret = 0;
    struct pjdev_hwaudio_stream* stream = pVoidUser;
    if (bufdata && stream)
    {
        //if(stream->ca_len == 0)
        pj_mutex_lock(stream->ca_mutex);
        stream->ca_len = ZPL_SKB_DATA_LEN(bufdata);
        memcpy(stream->ca_buf, ZPL_SKB_DATA(bufdata), ZPL_SKB_DATA_LEN(bufdata));
        pj_mutex_unlock(stream->ca_mutex);
        pj_sem_post(stream->ca_sem);
        //pjdev_hwaudio_capture_frame(pVoidUser, ZPL_SKB_DATA(bufdata), ZPL_SKB_DATA_LEN(bufdata));
    }
    return 0;
}

static int pjdev_hwaudio_capture_thread (void *arg)
{
    struct pjdev_hwaudio_stream* stream = (struct pjdev_hwaudio_stream*) arg;
    pjmedia_frame frame;
    pj_timestamp tstamp;
    int result = 0, ca_tstamp_interval = 0;
    tstamp.u64 = 0;
    ca_tstamp_interval = stream->ca_tstamp_interval;
    zm_msg_force_trap("ca_thread_func: Started ca_tstamp_interval=%d",stream->ca_tstamp_interval);
    while (!stream->quit) {
        pj_sem_wait(stream->ca_sem);
        pj_mutex_lock(stream->ca_mutex);
        frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
        frame.buf = (void*) stream->ca_buf;
        frame.size = stream->ca_len;
        frame.timestamp.u64 = tstamp.u64;
        frame.bit_info = 0;

        result = stream->ca_cb (stream->user_data, &frame);
        if (result == PJ_SUCCESS)
            tstamp.u64 += ca_tstamp_interval;//nframes;

        //zm_msg_force_trap( "========HW devices capture_frame len=%d timestamp=%lu, ca_tstamp_interval=%d", 
        //        stream->ca_len, frame.timestamp.u64, ca_tstamp_interval);
        pj_mutex_unlock(stream->ca_mutex);
    }
    zm_msg_force_trap( "ca_thread_func: Stopped");
    return PJ_SUCCESS;
}

static int pjdev_hwaudio_playback_thread (void *arg)
{
    struct pjdev_hwaudio_stream* stream = (struct pjdev_hwaudio_stream*) arg;
    int size                   = 320;//stream->pb_buf_size;
    void* user_data            = stream->user_data;
    char* buf                  = stream->pb_buf;
    pj_timestamp tstamp;
    int result;
    zpl_audio_frame_t hwframe;
    pj_bzero (buf, size);
    tstamp.u64 = 0;

    zm_msg_force_trap("pb_thread_func(%u): Started pb_tstamp_interval=%d",stream->pb_thread, stream->pb_tstamp_interval);
    while (!stream->quit) {
        pjmedia_frame frame;
        #ifdef ZPL_HISIMPP_MODULE
        zpl_audio_frame_hdr_t *framehdr;
        #endif
        frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
        frame.buf = buf+4;
        frame.size = size;
        frame.timestamp.u64 = tstamp.u64;
        frame.bit_info = 0;

        result = stream->pb_cb (user_data, &frame);
        if (result != PJ_SUCCESS || stream->quit)
            break;

        if (frame.type != PJMEDIA_FRAME_TYPE_AUDIO)
            pj_bzero (buf, size);
        #ifdef ZPL_HISIMPP_MODULE
        //zm_msg_force_trap("pb_thread_func frame (%u)", frame.size);
        hwframe.frame_type = 0;
        //hwframe.codec;
        hwframe.timeStamp = tstamp.u64;
        //hwframe.seqnum;
        hwframe.len = frame.size;
        hwframe.maxlen = stream->pb_buf_size;
        hwframe.data = buf;
        if(hwframe.frame_type != 0)
        {
            hwframe.len += 4;
            framehdr = (zpl_audio_frame_hdr_t *)buf;
            //framehdr->rev;
            framehdr->frame_type = 1;
            framehdr->len = frame.size/2;
            //framehdr->seq;
        }
        #endif
        zpl_media_audio_output_frame_write(stream->pb_prev, &hwframe);

        tstamp.u64 += stream->pb_tstamp_interval;
    }
    zm_msg_force_trap( "pb_thread_func: Stopped");
    return PJ_SUCCESS;
}
/* API: start stream */
static pj_status_t pjdev_hwaudio_stream_start (pjmedia_aud_stream *s)
{
    struct pjdev_hwaudio_stream *stream = (struct pjdev_hwaudio_stream*)s;
    pj_status_t status = PJ_SUCCESS;  
    zm_msg_force_trap("==========pjdev_hwaudio_stream_start %p", stream->ca_prev);
    stream->quit = 0;
    if (stream->param.dir & PJMEDIA_DIR_CAPTURE && stream->ca_prev) {
        status = pj_thread_create (stream->pool,
                                   "capture",
                                   pjdev_hwaudio_capture_thread,
                                   stream,
                                   PJ_THREAD_DEFAULT_STACK_SIZE*4, //ZERO,
                                   0,
                                   &stream->ca_thread);
        if (status != PJ_SUCCESS)
            return status;
    }
    if (stream->param.dir & PJMEDIA_DIR_PLAYBACK && stream->pb_prev) {
        status = pj_thread_create (stream->pool,
                                   "playback",
                                   pjdev_hwaudio_playback_thread,
                                   stream,
                                   PJ_THREAD_DEFAULT_STACK_SIZE*4,
                                   0,
                                   &stream->pb_thread);
        if (status != PJ_SUCCESS)
            return status;
    }
    if (stream->ca_prev)
    {
        zpl_media_audio_channel_t *audio = stream->ca_prev;
        zpl_media_channel_t * mediachn = audio->media_channel;
        if(mediachn && stream->cbid <= 0)
        {
            zm_msg_force_trap("==========pjdev_hwaudio_stream_start zpl_media_channel_client_add %d %d", mediachn->channel, mediachn->channel_index);
            stream->cbid = zpl_media_channel_client_add(mediachn->channel, mediachn->channel_index, pjdev_hwaudio_stream_clone, stream);  
        }
        if(mediachn && stream->cbid)
        {
            zm_msg_force_trap("==========pjdev_hwaudio_stream_start zpl_media_channel_client_start %d %d", mediachn->channel, mediachn->channel_index);
            zpl_media_channel_client_start(mediachn->channel, mediachn->channel_index, stream->cbid, zpl_true);
        }
        TRACE_(THIS_FILE, "===================pjdev_hwaudio_stream_start clock_rate=%d channel_count=%d samples_per_frame=%d bits_per_sample=%d, ca_tstamp_interval=%d pb_tstamp_interval=%d", 
                stream->param.clock_rate, stream->param.channel_count, 
                stream->param.samples_per_frame, stream->param.bits_per_sample, stream->ca_tstamp_interval, stream->pb_tstamp_interval);
    }
    return status;
}


/* API: stop stream */
static pj_status_t pjdev_hwaudio_stream_stop (pjmedia_aud_stream *s)
{
    struct pjdev_hwaudio_stream *stream = (struct pjdev_hwaudio_stream*)s;
    pj_status_t status = PJ_SUCCESS;  
    zm_msg_force_trap("==========pjdev_hwaudio_stream_stop  %p", stream->ca_prev);
    stream->quit = 1;
    if (stream->ca_prev)
    {
        zpl_media_audio_channel_t *audio = stream->ca_prev;
        zpl_media_channel_t * mediachn = audio->media_channel;
        if(mediachn && stream->cbid)
        {
            zpl_media_channel_client_start(mediachn->channel, mediachn->channel_index, stream->cbid, zpl_false);
            zpl_media_channel_client_del(mediachn->channel, mediachn->channel_index, stream->cbid);
            stream->cbid = 0;
        }
    }
    if (stream->ca_thread && stream->ca_prev) {
        zm_msg_force_trap("pjdev_hwaudio_stream_stop(%u): Waiting for capture to stop.", stream->ca_thread);
        pj_thread_join (stream->ca_thread);
        zm_msg_force_trap("pjdev_hwaudio_stream_stop(%u): capture stopped.", stream->ca_thread);
        pj_thread_destroy(stream->ca_thread);
        stream->ca_thread = NULL;
    }  
    if (stream->pb_thread && stream->pb_prev) {
        zm_msg_force_trap("pjdev_hwaudio_stream_stop(%u): Waiting for playback to stop.", stream->pb_thread);
        pj_thread_join (stream->pb_thread);
        zm_msg_force_trap("pjdev_hwaudio_stream_stop(%u): playback stopped.", stream->pb_thread);
        pj_thread_destroy(stream->pb_thread);
        stream->pb_thread = NULL;
    }   
    return PJ_SUCCESS;
}



static pj_status_t pjdev_hwaudio_stream_destroy (pjmedia_aud_stream *s)
{
    struct pjdev_hwaudio_stream *stream = (struct pjdev_hwaudio_stream*)s;
    zm_msg_force_trap("==========pjdev_hwaudio_stream_destroy");
    pjdev_hwaudio_stream_stop (s);

    if (stream->ca_mutex) {
        pj_mutex_destroy(stream->ca_mutex);
        stream->ca_mutex = NULL;
    }
    if (stream->ca_sem) {
        pj_sem_destroy(stream->ca_sem);
        stream->ca_sem = NULL;
    }
    if (stream->param.dir & PJMEDIA_DIR_PLAYBACK) {

    }
    if (stream->param.dir & PJMEDIA_DIR_CAPTURE) {

    }

    pj_pool_release (stream->pool);

    return PJ_SUCCESS;
}

#endif  /* PJMEDIA_AUDIO_DEV_HAS_ALSA */
