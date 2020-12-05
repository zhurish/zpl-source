/* $Id: alsa_dev.c 5846 2018-07-27 02:58:41Z ming $ */
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
#include <pjmedia_audiodev.h>
#include <pj/assert.h>
#include <pj/log.h>
#include <pj/os.h>
#include <pj/pool.h>
#include <pjmedia/errno.h>

#if defined(PJMEDIA_AUDIO_DEV_HAS_ALSA) && PJMEDIA_AUDIO_DEV_HAS_ALSA

#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include <errno.h>
#include <alsa/asoundlib.h>


#define THIS_FILE 			"alsa_dev.c"
#define ALSA_DEVICE_NAME 		"plughw:%d,%d"
#define ALSASOUND_PLAYBACK 		1
#define ALSASOUND_CAPTURE  		2
#define MAX_SOUND_CARDS 		5
#define MAX_SOUND_DEVICES_PER_CARD 	5
#define MAX_DEVICES			32
#define MAX_MIX_NAME_LEN                64 

/* Set to 1 to enable tracing */
#if 0
#	define TRACE_(expr)		PJ_LOG(5,expr)
#else
#	define TRACE_(expr)
#endif
//TRACE_((THIS_FILE, "add_dev (%s): Enter", dev_name));

#define ALSE_DEV_CAP_DEBUG
//#define ALSE_DEV_CAP_ARECORD
//#define ALSE_DEV_CAP_NEW_CTL
/*
 * Factory prototypes
 */
static pj_status_t alsa_factory_init(pjmedia_aud_dev_factory *f);
static pj_status_t alsa_factory_destroy(pjmedia_aud_dev_factory *f);
static pj_status_t alsa_factory_refresh(pjmedia_aud_dev_factory *f);
static unsigned    alsa_factory_get_dev_count(pjmedia_aud_dev_factory *f);
static pj_status_t alsa_factory_get_dev_info(pjmedia_aud_dev_factory *f,
					     unsigned index,
					     pjmedia_aud_dev_info *info);
static pj_status_t alsa_factory_default_param(pjmedia_aud_dev_factory *f,
					      unsigned index,
					      pjmedia_aud_param *param);
static pj_status_t alsa_factory_create_stream(pjmedia_aud_dev_factory *f,
					      const pjmedia_aud_param *param,
					      pjmedia_aud_rec_cb rec_cb,
					      pjmedia_aud_play_cb play_cb,
					      void *user_data,
					      pjmedia_aud_stream **p_strm);

/*
 * Stream prototypes
 */
static pj_status_t alsa_stream_get_param(pjmedia_aud_stream *strm,
					 pjmedia_aud_param *param);
static pj_status_t alsa_stream_get_cap(pjmedia_aud_stream *strm,
				       pjmedia_aud_dev_cap cap,
				       void *value);
static pj_status_t alsa_stream_set_cap(pjmedia_aud_stream *strm,
				       pjmedia_aud_dev_cap cap,
				       const void *value);
static pj_status_t alsa_stream_start(pjmedia_aud_stream *strm);
static pj_status_t alsa_stream_stop(pjmedia_aud_stream *strm);
static pj_status_t alsa_stream_destroy(pjmedia_aud_stream *strm);


struct alsa_factory
{
    pjmedia_aud_dev_factory	 base;
    pj_pool_factory		*pf;
    pj_pool_t			*pool;
    pj_pool_t			*base_pool;

    unsigned			 dev_cnt;
    pjmedia_aud_dev_info	 devs[MAX_DEVICES];
    char                         pb_mixer_name[MAX_MIX_NAME_LEN];
};

struct alsa_stream
{
    pjmedia_aud_stream	 base;

    /* Common */
    pj_pool_t		*pool;
    struct alsa_factory *af;
    void		*user_data;
    pjmedia_aud_param	 param;		/* Running parameter 		*/
    int                  rec_id;      	/* Capture device id		*/
    pj_bool_t            pb_quit;
    pj_bool_t            ca_quit;
    /* Playback */
    snd_pcm_t		*pb_pcm;
    snd_pcm_uframes_t    pb_frames; 	/* samples_per_frame		*/
    pjmedia_aud_play_cb  pb_cb;
    unsigned             pb_buf_size;
    char		*pb_buf;
    pj_thread_t		*pb_thread;

    /* Capture */
    snd_pcm_t		*ca_pcm;
    snd_pcm_uframes_t    ca_frames; 	/* samples_per_frame		*/
    pjmedia_aud_rec_cb   ca_cb;
    unsigned             ca_buf_size;
    char		*ca_buf;
    pj_thread_t		*ca_thread;
};

static pjmedia_aud_dev_factory_op alsa_factory_op =
{
    &alsa_factory_init,
    &alsa_factory_destroy,
    &alsa_factory_get_dev_count,
    &alsa_factory_get_dev_info,
    &alsa_factory_default_param,
    &alsa_factory_create_stream,
    &alsa_factory_refresh
};

static pjmedia_aud_stream_op alsa_stream_op =
{
    &alsa_stream_get_param,
    &alsa_stream_get_cap,
    &alsa_stream_set_cap,
    &alsa_stream_start,
    &alsa_stream_stop,
    &alsa_stream_destroy
};

static void null_alsa_error_handler (const char *file,
				int line,
				const char *function,
				int err,
				const char *fmt,
				...)
{
    PJ_UNUSED_ARG(file);
    PJ_UNUSED_ARG(line);
    PJ_UNUSED_ARG(function);
    PJ_UNUSED_ARG(err);
    PJ_UNUSED_ARG(fmt);
}

static void alsa_error_handler (const char *file,
				int line,
				const char *function,
				int err,
				const char *fmt,
				...)
{
    char err_msg[512];
    int index, len;
    va_list arg;

#ifndef NDEBUG
    index = snprintf (err_msg, sizeof(err_msg), "ALSA lib %s:%i:(%s) ",
		      file, line, function);
#else
    index = snprintf (err_msg, sizeof(err_msg), "ALSA lib: ");
#endif
    if (index < 1 || index >= (int)sizeof(err_msg)) {
	index = sizeof(err_msg)-1;
	err_msg[index] = '\0';
	goto print_msg;
    }

    va_start (arg, fmt);
    if (index < sizeof(err_msg)-1) {
	len = vsnprintf( err_msg+index, sizeof(err_msg)-index, fmt, arg);
	if (len < 1 || len >= (int)sizeof(err_msg)-index)
	    len = sizeof(err_msg)-index-1;
	index += len;
	err_msg[index] = '\0';
    }
    va_end(arg);
    if (err && index < sizeof(err_msg)-1) {
	len = snprintf( err_msg+index, sizeof(err_msg)-index, ": %s",
			snd_strerror(err));
	if (len < 1 || len >= (int)sizeof(err_msg)-index)
	    len = sizeof(err_msg)-index-1;
	index += len;
	err_msg[index] = '\0';
    }
print_msg:
    PJ_LOG (4,(THIS_FILE, "%s", err_msg));
}


static pj_status_t add_dev (struct alsa_factory *af, const char *dev_name)
{
    pjmedia_aud_dev_info *adi;
    snd_pcm_t* pcm;
    int pb_result, ca_result;
    pj_assert(af != NULL);
    pj_assert(dev_name != NULL);
    if (af->dev_cnt >= PJ_ARRAY_SIZE(af->devs))
	return PJ_ETOOMANY;

    adi = &af->devs[af->dev_cnt];

    TRACE_((THIS_FILE, "add_dev (%s): Enter", dev_name));

    /* Try to open the device in playback mode */
    pb_result = snd_pcm_open (&pcm, dev_name, SND_PCM_STREAM_PLAYBACK, 0);
    if (pb_result >= 0) {
	TRACE_((THIS_FILE, "Try to open the device for playback - success"));
	snd_pcm_close (pcm);
    } else {
    	__PJSIP_DEBUG("Try to open the device for playback - failure\r\n");
	TRACE_((THIS_FILE, "Try to open the device for playback - failure"));
    }

    /* Try to open the device in capture mode */
    ca_result = snd_pcm_open (&pcm, dev_name, SND_PCM_STREAM_CAPTURE, 0);
    if (ca_result >= 0) {
	TRACE_((THIS_FILE, "Try to open the device for capture - success"));
	snd_pcm_close (pcm);
    } else {
    	__PJSIP_DEBUG("Try to open the device for capture - failure\r\n");
	TRACE_((THIS_FILE, "Try to open the device for capture - failure"));
    }

    /* Check if the device could be opened in playback or capture mode */
    if (pb_result<0 && ca_result<0) {
    	__PJSIP_DEBUG("Unable to open sound device %s, setting "
	        	   "in/out channel count to 0\r\n", dev_name);
	TRACE_((THIS_FILE, "Unable to open sound device %s, setting "
	        	   "in/out channel count to 0", dev_name));
	/* Set I/O channel counts to 0 to indicate unavailable device */
	adi->output_count = 0;
	adi->input_count =  0;
    }

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
    strcpy(adi->driver, "ALSA");

    ++af->dev_cnt;

    PJ_LOG (5,(THIS_FILE, "Added sound device %s", adi->name));
    __PJSIP_DEBUG("Added sound device %s\r\n", adi->name);
    return PJ_SUCCESS;
}

static void get_mixer_name(struct alsa_factory *af)
{
    snd_mixer_t *handle;
    snd_mixer_elem_t *elem;
    pj_assert(af != NULL);
    if (snd_mixer_open(&handle, 0) < 0)
	return;

    if (snd_mixer_attach(handle, "default") < 0) {
	snd_mixer_close(handle);
	return;
    }

    if (snd_mixer_selem_register(handle, NULL, NULL) < 0) {
	snd_mixer_close(handle);
	return;
    }

    if (snd_mixer_load(handle) < 0) {
	snd_mixer_close(handle);
	return;
    }

    for (elem = snd_mixer_first_elem(handle); elem;
	 elem = snd_mixer_elem_next(elem))
    {
	if (snd_mixer_selem_is_active(elem) &&
	    snd_mixer_selem_has_playback_volume(elem))
	{
	    pj_ansi_strncpy(af->pb_mixer_name, snd_mixer_selem_get_name(elem),
	    		    sizeof(af->pb_mixer_name));
	    __PJSIP_DEBUG("Playback mixer name: %s\r\n", af->pb_mixer_name);
	    TRACE_((THIS_FILE, "Playback mixer name: %s", af->pb_mixer_name));
	    break;
	}
    }
    snd_mixer_close(handle);
}


/* Create ALSA audio driver. */
pjmedia_aud_dev_factory* pjmedia_alsa_factory(pj_pool_factory *pf)
{
    struct alsa_factory *af = NULL;
    pj_pool_t *pool = NULL;

    pool = pj_pool_create(pf, "alsa_aud_base", 256, 256, NULL);
    pj_assert(pool != NULL);
    af = PJ_POOL_ZALLOC_T(pool, struct alsa_factory);
    pj_assert(af != NULL);
    af->pf = pf;
    af->base_pool = pool;
    af->base.op = &alsa_factory_op;

    return &af->base;
}


/* API: init factory */
static pj_status_t alsa_factory_init(pjmedia_aud_dev_factory *f)
{
	pj_assert(f != NULL);
    pj_status_t status = alsa_factory_refresh(f);
    if (PJ_SUCCESS != status)
	return status;

    PJ_LOG(4,(THIS_FILE, "ALSA initialized"));
    return PJ_SUCCESS;
}


/* API: destroy factory */
static pj_status_t alsa_factory_destroy(pjmedia_aud_dev_factory *f)
{
    struct alsa_factory *af = (struct alsa_factory*)f;
    pj_assert(af != NULL);
    if (af->pool)
	pj_pool_release(af->pool);

    if (af->base_pool) {
	pj_pool_t *pool = af->base_pool;
	af->base_pool = NULL;
	pj_pool_release(pool);
    }

    /* Restore handler */
    snd_lib_error_set_handler(NULL);

    return PJ_SUCCESS;
}


/* API: refresh the device list */
static pj_status_t alsa_factory_refresh(pjmedia_aud_dev_factory *f)
{
    struct alsa_factory *af = (struct alsa_factory*)f;
    char **hints, **n;
    int err;
    pj_assert(af != NULL);
    TRACE_((THIS_FILE, "pjmedia_snd_init: Enumerate sound devices"));
    __PJSIP_DEBUG("pjmedia_snd_init: Enumerate sound devices\r\n");
    if (af->pool != NULL) {
	pj_pool_release(af->pool);
	af->pool = NULL;
    }

    af->pool = pj_pool_create(af->pf, "alsa_aud", 256, 256, NULL);
    af->dev_cnt = 0;

    /* Enumerate sound devices */
    err = snd_device_name_hint(-1, "pcm", (void***)&hints);
    if (err != 0)
	return PJMEDIA_EAUD_SYSERR;

    /* Set a null error handler prior to enumeration to suppress errors */
    snd_lib_error_set_handler(null_alsa_error_handler);

    n = hints;
    while (*n != NULL) {
	char *name = snd_device_name_get_hint(*n, "NAME");
	if (name != NULL) {
	    if (0 != strcmp("null", name))
	    {
		//add_dev(af, name);
	    add_dev(af, "default");
	    free(name);
	    break;
	    }
	    free(name);
	}
	n++;
    }

    /* Get the mixer name */
    get_mixer_name(af);

    /* Install error handler after enumeration, otherwise we'll get many
     * error messages about invalid card/device ID.
     */
    snd_lib_error_set_handler(alsa_error_handler);

    err = snd_device_name_free_hint((void**)hints);

    PJ_LOG(4,(THIS_FILE, "ALSA driver found %d devices", af->dev_cnt));
    __PJSIP_DEBUG("ALSA driver found %d devices\r\n", af->dev_cnt);
    return PJ_SUCCESS;
}


/* API: get device count */
static unsigned  alsa_factory_get_dev_count(pjmedia_aud_dev_factory *f)
{
    struct alsa_factory *af = (struct alsa_factory*)f;
    return af->dev_cnt;
}


/* API: get device info */
static pj_status_t alsa_factory_get_dev_info(pjmedia_aud_dev_factory *f,
					     unsigned index,
					     pjmedia_aud_dev_info *info)
{
    struct alsa_factory *af = (struct alsa_factory*)f;
    pj_assert(af != NULL);
    pj_assert(info != NULL);
    PJ_ASSERT_RETURN(index>=0 && index<af->dev_cnt, PJ_EINVAL);

    pj_memcpy(info, &af->devs[index], sizeof(*info));
    info->caps = PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY |
		 PJMEDIA_AUD_DEV_CAP_OUTPUT_LATENCY;
    return PJ_SUCCESS;
}

/* API: create default parameter */
static pj_status_t alsa_factory_default_param(pjmedia_aud_dev_factory *f,
					      unsigned index,
					      pjmedia_aud_param *param)
{
    struct alsa_factory *af = (struct alsa_factory*)f;
    pjmedia_aud_dev_info *adi;
    pj_assert(af != NULL);
    pj_assert(param != NULL);
    PJ_ASSERT_RETURN(index>=0 && index<af->dev_cnt, PJ_EINVAL);

    adi = &af->devs[index];

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

    param->clock_rate = adi->default_samples_per_sec;
    param->channel_count = 1;
    param->samples_per_frame = adi->default_samples_per_sec * 20 / 1000;
    param->bits_per_sample = 16;
    param->flags = adi->caps;
    param->input_latency_ms = PJMEDIA_SND_DEFAULT_REC_LATENCY;
    param->output_latency_ms = PJMEDIA_SND_DEFAULT_PLAY_LATENCY;

    return PJ_SUCCESS;
}


static int pb_thread_func (void *arg)
{
	usleep(5000);
    struct alsa_stream* stream = (struct alsa_stream*) arg;
    pj_assert(stream != NULL);
    snd_pcm_t* pcm             = stream->pb_pcm;
    int size                   = stream->pb_buf_size;
    snd_pcm_uframes_t nframes  = stream->pb_frames;
    void* user_data            = stream->user_data;
    char* buf 		       = stream->pb_buf;
    pj_timestamp tstamp;
    int result;

    pj_bzero (buf, size);
    tstamp.u64 = 0;

    TRACE_((THIS_FILE, "pb_thread_func(%u): Started",
	    (unsigned)syscall(SYS_gettid)));

    snd_pcm_prepare (pcm);

    while (!stream->pb_quit) {
	pjmedia_frame frame;

	frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
	frame.buf = buf;
	frame.size = size;
	frame.timestamp.u64 = tstamp.u64;
	frame.bit_info = 0;

	result = stream->pb_cb (user_data, &frame);
	if (result != PJ_SUCCESS || stream->pb_quit)
	    break;

	if (frame.type != PJMEDIA_FRAME_TYPE_AUDIO)
	    pj_bzero (buf, size);

	result = snd_pcm_writei (pcm, buf, nframes);
	if (result == -EPIPE) {
	    PJ_LOG (4,(THIS_FILE, "pb_thread_func: underrun!"));
	    snd_pcm_prepare (pcm);
	} else if (result < 0) {
	    PJ_LOG (4,(THIS_FILE, "pb_thread_func: error writing data!"));
	}
	tstamp.u64 += nframes;
    }
    __PJSIP_DEBUG( "pb_thread_func: Stopped\r\n");
    snd_pcm_drain (pcm);
    TRACE_((THIS_FILE, "pb_thread_func: Stopped"));
    return PJ_SUCCESS;
}

static int ca_audio_file(char *buf, int len)
{
	static int fpcnt = 0;
	static FILE *fp = NULL;
	if(fpcnt < 100000)
	{
		fpcnt ++;
		if(fp == NULL)
			fp = fopen("/tmp/ca.raw", "wb+");
		if(fp)
		{
			fwrite(buf, len, 1, fp);
			fflush(fp);
		}
	}
	else
	{
		if(fp)
		{
			fclose(fp);
			fp = NULL;
		}
	}
	return 0;
}

static int ca_thread_func (void *arg)
{
	usleep(5000);
    struct alsa_stream* stream = (struct alsa_stream*) arg;
    pj_assert(stream != NULL);
#ifndef ALSE_DEV_CAP_ARECORD
    snd_pcm_t* pcm             = stream->ca_pcm;
#endif
    int size                   = stream->ca_buf_size;
    snd_pcm_uframes_t nframes  = stream->ca_frames;
    void* user_data            = stream->user_data;
    char* buf 		       = stream->ca_buf;
    pj_timestamp tstamp;
    int result;
    struct sched_param param;
    pthread_t* thid;

    if(getenv("PJSIP_CA_SCHED"))
    {
    	char *value = getenv("PJSIP_CA_SCHED");
    	char *valueint = getenv("PJSIP_CA_SCHED_PRI");

    	__PJSIP_DEBUG("ca_thread_func(): set PJSIP_CA_SCHED=%s %s\r\n", value, valueint ? valueint:" ");

        thid = (pthread_t*) pj_thread_get_os_handle (pj_thread_this());
    	if(value)
    	{
    		if(strstr(value, "RR") || strstr(value, "FIFO"))
    		{
    			if(valueint)
    				param.sched_priority = atoi(valueint);
    			else
    			{
    				param.sched_priority = (sched_get_priority_max (SCHED_RR) + sched_get_priority_min (SCHED_RR))/2;
    				//param.sched_priority = sched_get_priority_min (SCHED_RR);
    				param.sched_priority = sched_get_priority_max (SCHED_RR);
    			}
    			if(strstr(value, "RR"))//SCHED_RR SCHED_FIFO
    			{
    				__PJSIP_DEBUG("ca_thread_func(): set SCHED_RR pri=%d\r\n", param.sched_priority);
    				result = pthread_setschedparam (*thid, SCHED_RR, &param);
    			}
    			else
    			{
    				__PJSIP_DEBUG("ca_thread_func(): set SCHED_FIFO pri=%d\r\n", param.sched_priority);
    				result = pthread_setschedparam (*thid, SCHED_FIFO, &param);
    			}
    			if (result)
    			{
    				if (result == EPERM)
    					PJ_LOG (5,(THIS_FILE, "Unable to increase thread priority, "
    						"root access needed."));
    				else
    					PJ_LOG (5,(THIS_FILE, "Unable to increase thread priority, "
    						"error: %d", result));
    			}
    		}
    	}
    }
    pj_bzero (buf, size);
    tstamp.u64 = 0;

    TRACE_((THIS_FILE, "ca_thread_func(%u): Started",
	    (unsigned)syscall(SYS_gettid)));
    __PJSIP_DEBUG("ca_thread_func(%u): Started\r\n",
	    (unsigned)syscall(SYS_gettid));
#ifndef ALSE_DEV_CAP_ARECORD
    snd_pcm_prepare (pcm);
#else
    int ifd = 0, ofd = 0, pfd[2];
    pipe(pfd);
    ifd = pfd[0];
    ofd = pfd[1];
	int pid = fork();
	if(pid == 0)
	{
		char *input[] = {"arecord", "-c", "1", "-f", "S16_LE", "-r", "8000", NULL};
		close(ifd);
		dup2(ofd, STDOUT_FILENO);
		//arecord -c 1 -f S16_LE -r 8000
		execvp("arecord", input);
	}
	else if(pid < 0)
	{
		return 0;
	}
	close(ofd);
#endif
    while (!stream->ca_quit) {
#ifndef ALSE_DEV_CAP_ARECORD
	pjmedia_frame frame;

	pj_bzero (buf, size);

	result = snd_pcm_readi (pcm, buf, nframes);

	if (stream->ca_quit)
	{
		__PJSIP_DEBUG( "----------------->ca_thread_func: stream->ca_quit break\r\n");
	    break;
	}
	if (result == -EPIPE) {
/*	    PJ_LOG (4,(THIS_FILE, "ca_thread_func: overrun!"));*/
	    __PJSIP_DEBUG( "ca_thread_func: overrun\r\n");
	    snd_pcm_prepare (pcm);
	    continue;
	}
	else if (result == -ESTRPIPE)
	{
		int res = 0;
	    PJ_LOG (4,(THIS_FILE, "ca_thread_func: Suspended!"));
	    __PJSIP_DEBUG( "ca_thread_func: Suspended\r\n");
		while ((res = snd_pcm_resume(pcm)) == -EAGAIN)
			sleep(1);	/* wait until suspend flag is released */
		if (res < 0)
			snd_pcm_prepare (pcm);
	    continue;
	}
	else if (result < 0) {
	    PJ_LOG (4,(THIS_FILE, "ca_thread_func: error reading data!"));
	    __PJSIP_DEBUG( "ca_thread_func: error reading data!(%s)\r\n", strerror(errno));
	    break;
	}

	frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
	frame.buf = (void*) buf;
	frame.size = size;
	frame.timestamp.u64 = tstamp.u64;
	frame.bit_info = 0;
	if(frame.bit_info)
		ca_audio_file(buf, size);

	result = stream->ca_cb (user_data, &frame);
	if (result != PJ_SUCCESS || stream->ca_quit)
	{
		__PJSIP_DEBUG( "----------------->ca_thread_func: stream->ca_quit break(%d)\r\n", result);
	    break;
	}
	tstamp.u64 += nframes;
#else
	pjmedia_frame frame;

	pj_bzero (buf, size);

	result = read (ifd, buf, size);

	if (stream->ca_quit)
	{
		 __PJSIP_DEBUG( "----------------->ca_thread_func: stream->ca_quit break\r\n");
	    break;
	}
	if(result == size)
	{
		frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
		frame.buf = (void*) buf;
		frame.size = size;
		frame.timestamp.u64 = tstamp.u64;
		frame.bit_info = 0;
		ca_audio_file(buf, size);
		result = stream->ca_cb (user_data, &frame);
		if (result != PJ_SUCCESS || stream->ca_quit)
		{
			__PJSIP_DEBUG( "----------------->ca_thread_func: stream->ca_quit break(%d)\r\n", result);
			break;
		}
		tstamp.u64 += nframes;
	}
	else
	{
		usleep(5000);
		continue;
	}
#endif
    }
#ifndef ALSE_DEV_CAP_ARECORD
    snd_pcm_drain (pcm);
#else
	kill(pid, 9);
	waitpid(pid, NULL, 0);
	close(ifd);
#endif
    __PJSIP_DEBUG( "ca_thread_func: Stopped\r\n");
    TRACE_((THIS_FILE, "ca_thread_func: Stopped"));

    return PJ_SUCCESS;
}


static pj_status_t open_playback (struct alsa_stream* stream,
			          const pjmedia_aud_param *param)
{
    snd_pcm_hw_params_t* params;
    snd_pcm_format_t format;
    int result;
    unsigned int rate;
    snd_pcm_uframes_t tmp_buf_size;
    snd_pcm_uframes_t tmp_period_size;
    pj_assert(stream != NULL);
    pj_assert(param != NULL);
    if (param->play_id < 0 || param->play_id >= stream->af->dev_cnt)
	return PJMEDIA_EAUD_INVDEV;

    /* Open PCM for playback */
    PJ_LOG (5,(THIS_FILE, "open_playback: Open playback device '%s'",
	       stream->af->devs[param->play_id].name));
    __PJSIP_DEBUG("open_playback: Open playback device '%s'\r\n",
	       stream->af->devs[param->play_id].name);
    result = snd_pcm_open (&stream->pb_pcm,
			   stream->af->devs[param->play_id].name,
			   SND_PCM_STREAM_PLAYBACK,
			   0);
    if (result < 0)
	return PJMEDIA_EAUD_SYSERR;

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca (&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any (stream->pb_pcm, params);

    /* Set interleaved mode */
    snd_pcm_hw_params_set_access (stream->pb_pcm, params,
				  SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Set format */
    switch (param->bits_per_sample) {
    case 8:
	TRACE_((THIS_FILE, "open_playback: set format SND_PCM_FORMAT_S8"));
	format = SND_PCM_FORMAT_S8;
	break;
    case 16:
	TRACE_((THIS_FILE, "open_playback: set format SND_PCM_FORMAT_S16_LE"));
	format = SND_PCM_FORMAT_S16_LE;
	break;
    case 24:
	TRACE_((THIS_FILE, "open_playback: set format SND_PCM_FORMAT_S24_LE"));
	format = SND_PCM_FORMAT_S24_LE;
	break;
    case 32:
	TRACE_((THIS_FILE, "open_playback: set format SND_PCM_FORMAT_S32_LE"));
	format = SND_PCM_FORMAT_S32_LE;
	break;
    default:
	TRACE_((THIS_FILE, "open_playback: set format SND_PCM_FORMAT_S16_LE"));
	format = SND_PCM_FORMAT_S16_LE;
	break;
    }
    snd_pcm_hw_params_set_format (stream->pb_pcm, params, format);

    /* Set number of channels */
    TRACE_((THIS_FILE, "open_playback: set channels: %d",
		       param->channel_count));
    snd_pcm_hw_params_set_channels (stream->pb_pcm, params,
				    param->channel_count);

    /* Set clock rate */
    rate = param->clock_rate;
    TRACE_((THIS_FILE, "open_playback: set clock rate: %d", rate));
    snd_pcm_hw_params_set_rate_near (stream->pb_pcm, params, &rate, NULL);
    TRACE_((THIS_FILE, "open_playback: clock rate set to: %d", rate));

    /* Set period size to samples_per_frame frames. */
    stream->pb_frames = (snd_pcm_uframes_t) param->samples_per_frame /
					    param->channel_count;
    TRACE_((THIS_FILE, "open_playback: set period size: %d",
	    stream->pb_frames));
    tmp_period_size = stream->pb_frames;
    snd_pcm_hw_params_set_period_size_near (stream->pb_pcm, params,
					    &tmp_period_size, NULL);
    TRACE_((THIS_FILE, "open_playback: period size set to: %d",
	    tmp_period_size));

    /* Set the sound device buffer size and latency */
    if (param->flags & PJMEDIA_AUD_DEV_CAP_OUTPUT_LATENCY)
	tmp_buf_size = (rate / 1000) * param->output_latency_ms;
    else
	tmp_buf_size = (rate / 1000) * PJMEDIA_SND_DEFAULT_PLAY_LATENCY;
    snd_pcm_hw_params_set_buffer_size_near (stream->pb_pcm, params,
					    &tmp_buf_size);
    stream->param.output_latency_ms = tmp_buf_size / (rate / 1000);

    /* Set our buffer */
    stream->pb_buf_size = stream->pb_frames * param->channel_count *
			  (param->bits_per_sample/8);
    stream->pb_buf = (char*) pj_pool_alloc(stream->pool, stream->pb_buf_size);

    TRACE_((THIS_FILE, "open_playback: buffer size set to: %d",
	    (int)tmp_buf_size));
    TRACE_((THIS_FILE, "open_playback: playback_latency set to: %d ms",
	    (int)stream->param.output_latency_ms));

    /* Activate the parameters */
    result = snd_pcm_hw_params (stream->pb_pcm, params);
    if (result < 0) {
	snd_pcm_close (stream->pb_pcm);
	return PJMEDIA_EAUD_SYSERR;
    }

    PJ_LOG (5,(THIS_FILE, "Opened device alsa(%s) for playing, sample rate=%d"
	       ", ch=%d, bits=%d, period size=%d frames, latency=%d ms",
	       stream->af->devs[param->play_id].name,
	       rate, param->channel_count,
	       param->bits_per_sample, stream->pb_frames,
	       (int)stream->param.output_latency_ms));

    return PJ_SUCCESS;
}

#ifndef ALSE_DEV_CAP_ARECORD
#ifdef ALSE_DEV_CAP_NEW_CTL
static pj_status_t open_capture (struct alsa_stream* stream,
			         const pjmedia_aud_param *param)
{
    snd_pcm_hw_params_t* params;
    snd_pcm_format_t format;
    int result;
    unsigned int rate, tmp1, start_threshold, stop_threshold;
    snd_pcm_uframes_t tmp_buf_size;
    snd_pcm_uframes_t tmp_period_size;
    unsigned period_time, buffer_time;
	snd_pcm_sw_params_t *swparams;
	pj_assert(stream != NULL);
	pj_assert(param != NULL);
    if (param->rec_id < 0 || param->rec_id >= stream->af->dev_cnt)
	return PJMEDIA_EAUD_INVDEV;

    /* Open PCM for capture */
    __PJSIP_DEBUG("open_capture: Open capture device '%s'\r\n",
   	       stream->af->devs[param->rec_id].name);

    result = snd_pcm_open (&stream->ca_pcm,
		            stream->af->devs[param->rec_id].name,
			   SND_PCM_STREAM_CAPTURE,
			   0);
    if (result < 0)
	return PJMEDIA_EAUD_SYSERR;

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca (&params);
	snd_pcm_sw_params_alloca(&swparams);
    /* Fill it in with default values. */
    snd_pcm_hw_params_any (stream->ca_pcm, params);

    /* Set interleaved mode */
    snd_pcm_hw_params_set_access (stream->ca_pcm, params,
				  SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Set format */
    switch (param->bits_per_sample) {
    case 8:
	TRACE_((THIS_FILE, "open_capture: set format SND_PCM_FORMAT_S8"));
	__PJSIP_DEBUG("open_capture: SND_PCM_FORMAT_S8\r\n");
	format = SND_PCM_FORMAT_S8;
	break;
    case 16:
	TRACE_((THIS_FILE, "open_capture: set format SND_PCM_FORMAT_S16_LE"));
	__PJSIP_DEBUG("open_capture: SND_PCM_FORMAT_S16_LE\r\n");
	format = SND_PCM_FORMAT_S16_LE;
	break;
    case 24:
	TRACE_((THIS_FILE, "open_capture: set format SND_PCM_FORMAT_S24_LE"));
	__PJSIP_DEBUG("open_capture: SND_PCM_FORMAT_S24_LE\r\n");
	format = SND_PCM_FORMAT_S24_LE;
	break;
    case 32:
	TRACE_((THIS_FILE, "open_capture: set format SND_PCM_FORMAT_S32_LE"));
	__PJSIP_DEBUG("open_capture: SND_PCM_FORMAT_S32_LE\r\n");
	format = SND_PCM_FORMAT_S32_LE;
	break;
    default:
	TRACE_((THIS_FILE, "open_capture: set format SND_PCM_FORMAT_S16_LE"));
	__PJSIP_DEBUG("open_capture: SND_PCM_FORMAT_S16_LE\r\n");
	format = SND_PCM_FORMAT_S16_LE;
	break;
    }
    snd_pcm_hw_params_set_format (stream->ca_pcm, params, format);

    /* Set number of channels */
    snd_pcm_hw_params_set_channels (stream->ca_pcm, params,
				    param->channel_count);
    __PJSIP_DEBUG("open_capture: set channels: %d\r\n",
	    param->channel_count);

    /* Set clock rate */
    rate = param->clock_rate;
    snd_pcm_hw_params_set_rate_near (stream->ca_pcm, params, &rate, NULL);
    __PJSIP_DEBUG("open_capture: set clock rate: %d\r\n", rate);

	if (buffer_time == 0 && tmp_buf_size == 0) {
		result = snd_pcm_hw_params_get_buffer_time_max(params,
							    &buffer_time, 0);
	    if (result < 0)
	    	return PJMEDIA_EAUD_SYSERR;
		if (buffer_time > 500000)
			buffer_time = 500000;
	}
	if (period_time == 0 && tmp_period_size == 0) {
		if (buffer_time > 0)
			period_time = buffer_time / 4;
		else
			tmp_period_size = tmp_buf_size / 4;
	}
	if (period_time > 0)
		result = snd_pcm_hw_params_set_period_time_near(stream->ca_pcm, params,
							     &period_time, 0);
	else
		result = snd_pcm_hw_params_set_period_size_near(stream->ca_pcm, params,
							     &tmp_period_size, 0);

    if (result < 0)
	return PJMEDIA_EAUD_SYSERR;

	if (buffer_time > 0) {
		result = snd_pcm_hw_params_set_buffer_time_near(stream->ca_pcm, params,
							     &buffer_time, 0);
	} else {
		result = snd_pcm_hw_params_set_buffer_size_near(stream->ca_pcm, params,
							     &tmp_buf_size);
	}

    /* Activate the parameters */
    result = snd_pcm_hw_params (stream->ca_pcm, params);
    if (result < 0) {
	snd_pcm_close (stream->ca_pcm);
	return PJMEDIA_EAUD_SYSERR;
    }
	//monotonic = snd_pcm_hw_params_is_monotonic(params);


	snd_pcm_hw_params_get_period_size(params, &tmp_period_size, 0);
	snd_pcm_hw_params_get_buffer_size(params, &tmp_buf_size);

	snd_pcm_sw_params_current(stream->ca_pcm, swparams);

	tmp1 = tmp_period_size;

	snd_pcm_sw_params_set_avail_min(stream->ca_pcm, swparams, tmp1);

	/* round up to closest transfer boundary */
	tmp1 = tmp_buf_size;
	start_threshold = (double) rate * 1 / 1000000;
	if (start_threshold < 1)
		start_threshold = 1;
	if (start_threshold > tmp1)
		start_threshold = tmp1;

	snd_pcm_sw_params_set_start_threshold(stream->ca_pcm, swparams, start_threshold);

	stop_threshold = (double) rate * 0 / 1000000;
	snd_pcm_sw_params_set_stop_threshold(stream->ca_pcm, swparams, stop_threshold);

	if (snd_pcm_sw_params(stream->ca_pcm, swparams) < 0) {
		snd_pcm_close (stream->ca_pcm);
		return PJMEDIA_EAUD_SYSERR;
	}

    /* Set period size to samples_per_frame frames. */
    stream->ca_frames = (snd_pcm_uframes_t) param->samples_per_frame /
					    param->channel_count;
    if(stream->ca_frames != tmp_period_size)
    {
        __PJSIP_DEBUG("open_capture: ca_frames set: %d period_size %d\r\n",
    	    (int)stream->ca_frames,  (int)tmp_period_size);
    	//stream->ca_frames = tmp_period_size;
    }

    stream->ca_buf_size = stream->ca_frames * param->channel_count *
			  (param->bits_per_sample/8);

    if(stream->ca_buf_size != tmp_period_size * (snd_pcm_format_physical_width(format)*param->channel_count) / 8)
    {
        __PJSIP_DEBUG("open_capture: ca_buf_size set: %d chunk_bytes %d\r\n",
    	    (int)stream->ca_buf_size,
			(int)tmp_period_size * (snd_pcm_format_physical_width(format)*param->channel_count) / 8);
    	//stream->ca_buf_size = tmp_period_size * (snd_pcm_format_physical_width(format)*param->channel_count) / 8;
    }

    stream->ca_buf = (char*) pj_pool_alloc (stream->pool, stream->ca_buf_size);

    __PJSIP_DEBUG("open_capture: buffer size set: %d period_size %d\r\n",
	    (int)tmp_buf_size,  (int)tmp_period_size);

    __PJSIP_DEBUG("Opened device alsa(%s) for capture, sample rate=%d"
	       ", ch=%d, bits=%d, period size=%d frames, latency=%d ms\r\n",
	       stream->af->devs[param->rec_id].name,
	       rate, param->channel_count,
	       param->bits_per_sample, stream->ca_frames,
	       (int)stream->param.input_latency_ms);

/*	bits_per_sample = snd_pcm_format_physical_width(params.format);
	significant_bits_per_sample = snd_pcm_format_width(params.format);
	bits_per_frame = bits_per_sample * params.channels;
	chunk_bytes = tmp_period_size * bits_per_frame / 8;
	audiobuf = realloc(audiobuf, chunk_bytes);
	if (audiobuf == NULL) {
		error(_("not enough memory"));
		prg_exit(EXIT_FAILURE);
	}*/
	// fprintf(stderr, "real chunk_size = %i, frags = %i, total = %i\n", chunk_size, setup.buf.block.frags, setup.buf.block.frags * chunk_size);

	//buffer_frames = buffer_size;	/* for position test */
    /*
    open_capture: SND_PCM_FORMAT_S16_LE
    open_capture: set channels: 1
    open_capture: set clock rate: 8000
    [  169.629570] rt5670 0-001c: AEC
    open_capture: ca_frames set: 160 period_size 1024
    open_capture: ca_buf_size set: 320 chunk_bytes 2048
    open_capture: buffer size set: 2048 period_size 1024
    Opened device alsa(default) for capture, sample rate=8000, ch=1, bits=16, period size=160 frames, latency=100 ms
	*/
    return PJ_SUCCESS;
}
#else
static pj_status_t open_capture (struct alsa_stream* stream,
			         const pjmedia_aud_param *param)
{
    snd_pcm_hw_params_t* params;
    snd_pcm_format_t format;
    int result;
    unsigned int rate;
    snd_pcm_uframes_t tmp_buf_size;
    snd_pcm_uframes_t tmp_period_size;
#ifdef ALSE_DEV_CAP_DEBUG
    unsigned period_time, buffer_time;
#endif
    pj_assert(stream != NULL);
    pj_assert(param != NULL);
    if (param->rec_id < 0 || param->rec_id >= stream->af->dev_cnt)
	return PJMEDIA_EAUD_INVDEV;

    /* Open PCM for capture */
    PJ_LOG (5,(THIS_FILE, "open_capture: Open capture device '%s'",
	       stream->af->devs[param->rec_id].name));

    __PJSIP_DEBUG("open_capture: Open capture device '%s'\r\n",
   	       stream->af->devs[param->rec_id].name);

    result = snd_pcm_open (&stream->ca_pcm,
		            stream->af->devs[param->rec_id].name,
			   SND_PCM_STREAM_CAPTURE,
			   0);
    if (result < 0)
	return PJMEDIA_EAUD_SYSERR;

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca (&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any (stream->ca_pcm, params);

    /* Set interleaved mode */
    snd_pcm_hw_params_set_access (stream->ca_pcm, params,
				  SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Set format */
    switch (param->bits_per_sample) {
    case 8:
	TRACE_((THIS_FILE, "open_capture: set format SND_PCM_FORMAT_S8"));
	__PJSIP_DEBUG("open_capture: SND_PCM_FORMAT_S8\r\n");
	format = SND_PCM_FORMAT_S8;
	break;
    case 16:
	TRACE_((THIS_FILE, "open_capture: set format SND_PCM_FORMAT_S16_LE"));
	__PJSIP_DEBUG("open_capture: SND_PCM_FORMAT_S16_LE\r\n");
	format = SND_PCM_FORMAT_S16_LE;
	break;
    case 24:
	TRACE_((THIS_FILE, "open_capture: set format SND_PCM_FORMAT_S24_LE"));
	__PJSIP_DEBUG("open_capture: SND_PCM_FORMAT_S24_LE\r\n");
	format = SND_PCM_FORMAT_S24_LE;
	break;
    case 32:
	TRACE_((THIS_FILE, "open_capture: set format SND_PCM_FORMAT_S32_LE"));
	__PJSIP_DEBUG("open_capture: SND_PCM_FORMAT_S32_LE\r\n");
	format = SND_PCM_FORMAT_S32_LE;
	break;
    default:
	TRACE_((THIS_FILE, "open_capture: set format SND_PCM_FORMAT_S16_LE"));
	__PJSIP_DEBUG("open_capture: SND_PCM_FORMAT_S16_LE\r\n");
	format = SND_PCM_FORMAT_S16_LE;
	break;
    }
    snd_pcm_hw_params_set_format (stream->ca_pcm, params, format);

    /* Set number of channels */
    TRACE_((THIS_FILE, "open_capture: set channels: %d",
	    param->channel_count));
    snd_pcm_hw_params_set_channels (stream->ca_pcm, params,
				    param->channel_count);
    __PJSIP_DEBUG("open_capture: set channels: %d\r\n",
	    param->channel_count);

    /* Set clock rate */
    rate = param->clock_rate;
    TRACE_((THIS_FILE, "open_capture: set clock rate: %d", rate));
    snd_pcm_hw_params_set_rate_near (stream->ca_pcm, params, &rate, NULL);
    __PJSIP_DEBUG("open_capture: set clock rate: %d\r\n", rate);

    /* Set period size to samples_per_frame frames. */
    stream->ca_frames = (snd_pcm_uframes_t) param->samples_per_frame /
					    param->channel_count;
    TRACE_((THIS_FILE, "open_capture: set period size: %d",
	    stream->ca_frames));
    tmp_period_size = stream->ca_frames;
#ifndef ALSE_DEV_CAP_DEBUG
    snd_pcm_hw_params_set_period_size_near (stream->ca_pcm, params,
					    &tmp_period_size, NULL);
#else// ALSE_DEV_CAP_DEBUG
    period_time = 128000;
    snd_pcm_hw_params_set_period_time_near(stream->ca_pcm, params,
    							     &period_time, 0);
#endif
    TRACE_((THIS_FILE, "open_capture: period size set to: %d",
	    tmp_period_size));
    __PJSIP_DEBUG("open_capture: set period size: %d\r\n",
    		tmp_period_size);
    /* Set the sound device buffer size and latency */
    if (param->flags & PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY)
	tmp_buf_size = (rate / 1000) * param->input_latency_ms;
    else
	tmp_buf_size = (rate / 1000) * PJMEDIA_SND_DEFAULT_REC_LATENCY;

    tmp_buf_size = 2048;//tmp_buf_size * 4;
#ifndef ALSE_DEV_CAP_DEBUG
    snd_pcm_hw_params_set_buffer_size_near (stream->ca_pcm, params,
					    &tmp_buf_size);
#else
    buffer_time = 512000;
    snd_pcm_hw_params_set_buffer_time_near(stream->ca_pcm, params,
    							     &buffer_time, 0);
#endif
    __PJSIP_DEBUG("open_capture: buffer size set to: %d\r\n",
	    (int)tmp_buf_size);
    stream->param.input_latency_ms = tmp_buf_size / (rate / 1000);

    /* Set our buffer */
    stream->ca_buf_size = stream->ca_frames * param->channel_count *
			  (param->bits_per_sample/8);
    stream->ca_buf = (char*) pj_pool_alloc (stream->pool, stream->ca_buf_size);

    TRACE_((THIS_FILE, "open_capture: buffer size set to: %d",
	    (int)tmp_buf_size));
    TRACE_((THIS_FILE, "open_capture: capture_latency set to: %d ms",
	    (int)stream->param.input_latency_ms));

    /* Activate the parameters */
    result = snd_pcm_hw_params (stream->ca_pcm, params);
    if (result < 0) {
	snd_pcm_close (stream->ca_pcm);
	return PJMEDIA_EAUD_SYSERR;
    }

	snd_pcm_hw_params_get_buffer_size(params, &tmp_buf_size);
	snd_pcm_hw_params_get_period_size(params, &tmp_period_size, 0);
    __PJSIP_DEBUG("open_capture: buffer size set: %d period_size %d\r\n",
	    (int)tmp_buf_size,  (int)tmp_period_size);

    __PJSIP_DEBUG("Opened device alsa(%s) for capture, sample rate=%d"
	       ", ch=%d, bits=%d, period size=%d frames, latency=%d ms\r\n",
	       stream->af->devs[param->rec_id].name,
	       rate, param->channel_count,
	       param->bits_per_sample, stream->ca_frames,
	       (int)stream->param.input_latency_ms);

    PJ_LOG (5,(THIS_FILE, "Opened device alsa(%s) for capture, sample rate=%d"
	       ", ch=%d, bits=%d, period size=%d frames, latency=%d ms",
	       stream->af->devs[param->rec_id].name,
	       rate, param->channel_count,
	       param->bits_per_sample, stream->ca_frames,
	       (int)stream->param.input_latency_ms));

    return PJ_SUCCESS;
}
#endif
#else
static pj_status_t open_capture (struct alsa_stream* stream,
			         const pjmedia_aud_param *param)
{
    //int result;
	pj_assert(stream != NULL);
	pj_assert(param != NULL);
    if (param->rec_id < 0 || param->rec_id >= stream->af->dev_cnt)
	return PJMEDIA_EAUD_INVDEV;

    /* Set format */
    //param->bits_per_sample


    /* Set period size to samples_per_frame frames. */
    stream->ca_frames = (snd_pcm_uframes_t) param->samples_per_frame /
					    param->channel_count;


    /* Set our buffer */
    stream->ca_buf_size = stream->ca_frames * param->channel_count *
			  (param->bits_per_sample/8);
    stream->ca_buf = (char*) pj_pool_alloc (stream->pool, stream->ca_buf_size);


    __PJSIP_DEBUG("Opened device alsa(%s) for capture, sample rate=%d"
	       ", ch=%d, bits=%d, period size=%d frames, latency=%d ms\r\n",
	       stream->af->devs[param->rec_id].name,
		   param->clock_rate, param->channel_count,
	       param->bits_per_sample, stream->ca_frames,
	       (int)stream->param.input_latency_ms);

    return PJ_SUCCESS;
}
#endif

/* API: create stream */
static pj_status_t alsa_factory_create_stream(pjmedia_aud_dev_factory *f,
					      const pjmedia_aud_param *param,
					      pjmedia_aud_rec_cb rec_cb,
					      pjmedia_aud_play_cb play_cb,
					      void *user_data,
					      pjmedia_aud_stream **p_strm)
{
    struct alsa_factory *af = (struct alsa_factory*)f;
    pj_status_t status;
    pj_pool_t* pool;
    struct alsa_stream* stream = NULL;
	pj_assert(af != NULL);
	pj_assert(param != NULL);
    pool = pj_pool_create (af->pf, "alsa%p", 1024, 1024, NULL);
    if (!pool)
	return PJ_ENOMEM;

    /* Allocate and initialize comon stream data */
    stream = PJ_POOL_ZALLOC_T (pool, struct alsa_stream);
    stream->base.op = &alsa_stream_op;
    stream->pool      = pool;
    stream->af 	      = af;
    stream->user_data = user_data;
    stream->pb_cb     = play_cb;
    stream->ca_cb     = rec_cb;
    stream->pb_quit      = PJ_FALSE;
    stream->ca_quit      = PJ_FALSE;
    pj_memcpy(&stream->param, param, sizeof(*param));

    /* Init playback */
    if (param->dir & PJMEDIA_DIR_PLAYBACK) {
	status = open_playback (stream, param);
	if (status != PJ_SUCCESS) {
	    pj_pool_release (pool);
	    return status;
	}
    }
    /* Init capture */
    if (param->dir & PJMEDIA_DIR_CAPTURE) {
	status = open_capture (stream, param);
	if (status != PJ_SUCCESS) {
	    if (param->dir & PJMEDIA_DIR_PLAYBACK)
		snd_pcm_close (stream->pb_pcm);
	    pj_pool_release (pool);
	    return status;
	}
    }
    *p_strm = &stream->base;
    return PJ_SUCCESS;
}


/* API: get running parameter */
static pj_status_t alsa_stream_get_param(pjmedia_aud_stream *s,
					 pjmedia_aud_param *pi)
{
    struct alsa_stream *stream = (struct alsa_stream*)s;

    PJ_ASSERT_RETURN(s && pi, PJ_EINVAL);

    pj_memcpy(pi, &stream->param, sizeof(*pi));

    return PJ_SUCCESS;
}


/* API: get capability */
static pj_status_t alsa_stream_get_cap(pjmedia_aud_stream *s,
				       pjmedia_aud_dev_cap cap,
				       void *pval)
{
    struct alsa_stream *stream = (struct alsa_stream*)s;
	pj_assert(stream != NULL);
    PJ_ASSERT_RETURN(s && pval, PJ_EINVAL);

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
static pj_status_t alsa_stream_set_cap(pjmedia_aud_stream *strm,
				       pjmedia_aud_dev_cap cap,
				       const void *value)
{
    struct alsa_factory *af = ((struct alsa_stream*)strm)->af;
	pj_assert(af != NULL);
    if (cap==PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING && 
	pj_ansi_strlen(af->pb_mixer_name)) 
    {
	pj_ssize_t min, max;
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	snd_mixer_elem_t* elem;
	unsigned vol = *(unsigned*)value;

	if (snd_mixer_open(&handle, 0) < 0)
	    return PJMEDIA_EAUD_SYSERR;

	if (snd_mixer_attach(handle, "default") < 0)
	    return PJMEDIA_EAUD_SYSERR;

	if (snd_mixer_selem_register(handle, NULL, NULL) < 0)
	    return PJMEDIA_EAUD_SYSERR;

	if (snd_mixer_load(handle) < 0)
	    return PJMEDIA_EAUD_SYSERR;

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, af->pb_mixer_name);
	elem = snd_mixer_find_selem(handle, sid);
	if (!elem)
	    return PJMEDIA_EAUD_SYSERR;

	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	if (snd_mixer_selem_set_playback_volume_all(elem, vol * max / 100) < 0)
	    return PJMEDIA_EAUD_SYSERR;

	snd_mixer_close(handle);
	return PJ_SUCCESS;
    }

    return PJMEDIA_EAUD_INVCAP;
}


/* API: start stream */
static pj_status_t alsa_stream_start (pjmedia_aud_stream *s)
{
    struct alsa_stream *stream = (struct alsa_stream*)s;
	pj_assert(stream != NULL);
    pj_status_t status = PJ_SUCCESS;
#if 0
    stream->ca_quit = PJ_FALSE;
    if (stream->param.dir & PJMEDIA_DIR_CAPTURE) {
	status = pj_thread_create (stream->pool,
				   "alsa_capture",
				   ca_thread_func,
				   stream,
				   0, //ZERO,
				   0,
				   &stream->ca_thread);
	if (status != PJ_SUCCESS)
	    return status;
    }
    stream->pb_quit = PJ_FALSE;
    if (stream->param.dir & PJMEDIA_DIR_PLAYBACK) {
	status = pj_thread_create (stream->pool,
				   "alsa_playback",
				   pb_thread_func,
				   stream,
				   0, //ZERO,
				   0,
				   &stream->pb_thread);
	if (status != PJ_SUCCESS) {
	    stream->ca_quit = PJ_TRUE;
	    pj_thread_kill(stream->ca_thread);
	    pj_thread_join(stream->ca_thread);
	    pj_thread_destroy(stream->ca_thread);
	    stream->ca_thread = NULL;
	}
    }
#else
    stream->pb_quit = PJ_FALSE;
    if (stream->param.dir & PJMEDIA_DIR_PLAYBACK) {
	status = pj_thread_create (stream->pool,
				   "alsa_playback",
				   pb_thread_func,
				   stream,
				   0, //ZERO,
				   0,
				   &stream->pb_thread);
	if (status != PJ_SUCCESS)
	    return status;
    }
    stream->ca_quit = PJ_FALSE;
    if (stream->param.dir & PJMEDIA_DIR_CAPTURE) {
	status = pj_thread_create (stream->pool,
				   "alsa_capture",
				   ca_thread_func,
				   stream,
				   0, //ZERO,
				   0,
				   &stream->ca_thread);
	if (status != PJ_SUCCESS) {
	    stream->pb_quit = PJ_TRUE;
	    pj_thread_join(stream->pb_thread);
	    pj_thread_destroy(stream->pb_thread);
	    stream->pb_thread = NULL;
	}
    }
#endif
    return status;
}


/* API: stop stream */
static pj_status_t alsa_stream_stop (pjmedia_aud_stream *s)
{
    struct alsa_stream *stream = (struct alsa_stream*)s;
	pj_assert(stream != NULL);
    stream->ca_quit = PJ_TRUE;
    usleep(10000);
    if (stream->ca_thread) {
    	__PJSIP_DEBUG( "alsa_stream_stop(%u): Waiting for capture to stop.\r\n",
   			   (unsigned)syscall(SYS_gettid));
	TRACE_((THIS_FILE,
		   "alsa_stream_stop(%u): Waiting for capture to stop.",
		   (unsigned)syscall(SYS_gettid)));
	//pj_thread_kill(stream->ca_thread);
	pj_thread_join (stream->ca_thread);
	__PJSIP_DEBUG( "alsa_stream_stop(%u): capture stopped.\r\n",
 			   (unsigned)syscall(SYS_gettid));
	TRACE_((THIS_FILE,
		   "alsa_stream_stop(%u): capture stopped.",
		   (unsigned)syscall(SYS_gettid)));
	pj_thread_destroy(stream->ca_thread);
	stream->ca_thread = NULL;
    }

    stream->pb_quit = PJ_TRUE;
    usleep(10000);
    if (stream->pb_thread) {
    	__PJSIP_DEBUG( "alsa_stream_stop(%u): Waiting for playback to stop.\r\n",
    			   (unsigned)syscall(SYS_gettid));
	TRACE_((THIS_FILE,
		   "alsa_stream_stop(%u): Waiting for playback to stop.",
		   (unsigned)syscall(SYS_gettid)));
	//pj_thread_kill(stream->pb_thread);
	pj_thread_join (stream->pb_thread);
	TRACE_((THIS_FILE,
		   "alsa_stream_stop(%u): playback stopped.",
		   (unsigned)syscall(SYS_gettid)));
  	 __PJSIP_DEBUG( "alsa_stream_stop(%u): playback stopped.\r\n",
  			   (unsigned)syscall(SYS_gettid));
	pj_thread_destroy(stream->pb_thread);
	stream->pb_thread = NULL;
    }

    return PJ_SUCCESS;
}



static pj_status_t alsa_stream_destroy (pjmedia_aud_stream *s)
{
    struct alsa_stream *stream = (struct alsa_stream*)s;
	pj_assert(stream != NULL);
    alsa_stream_stop (s);

    if ((stream->param.dir & PJMEDIA_DIR_PLAYBACK) && stream->pb_pcm) {
	snd_pcm_close (stream->pb_pcm);
	stream->pb_pcm = NULL;
    }
#ifndef ALSE_DEV_CAP_ARECORD
    if ((stream->param.dir & PJMEDIA_DIR_CAPTURE) && stream->ca_pcm) {
	snd_pcm_close (stream->ca_pcm);
	stream->ca_pcm = NULL;
    }
#else
#endif
    if(stream->pool)
    pj_pool_release (stream->pool);

    return PJ_SUCCESS;
}

#endif	/* PJMEDIA_AUDIO_DEV_HAS_ALSA */
