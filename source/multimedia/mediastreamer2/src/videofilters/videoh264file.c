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

#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msvideo.h"
#include "mediastreamer2/rfc3984.h"
#include "mediastreamer2/msticker.h"
#include "mediastreamer2/formats.h"
//#include "h264utils.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"

typedef struct {
	MSPlayerState state;
	MSVideoSize vsize;
	float		fps;
	int		bitrate;
	Rfc3984Context *packer;
	uint64_t framenum;
	int mode;
	//MSVideoStarter starter;
	//MSIFrameRequestsLimiterCtx iframe_limiter;
	mblk_t *sps, *pps; /*lastly generated SPS, PPS, in case we need to repeat them*/
	char *mime;
	char	*filename;
	zpl_media_file_t	*filemedia;
} MSH264Player;
/*********************************************************************************************
 * H264 Player Filter                                                                         *
 *********************************************************************************************/
static int player_close(MSFilter *f, void *arg);

static void player_init(MSFilter *f) {
	MSH264Player *obj = (MSH264Player *)ms_new0(MSH264Player, 1);
	obj->state = MSPlayerClosed;
	obj->mime = "h264";
	obj->mode = 1;	
	f->data = obj;
}

static void player_uninit(MSFilter *f) {
	MSH264Player *obj = (MSH264Player *)f->data;
	ms_filter_lock(f);
	if(obj->state != MSPlayerClosed) {
		if(obj->filemedia)
		{
		zpl_media_file_close(obj->filemedia);
		zpl_media_file_destroy(obj->filemedia);
		}
	}
	ms_free(obj);
	ms_filter_unlock(f);
}

static int player_open_file(MSFilter *f, void *arg) {
	MSH264Player *obj = (MSH264Player *)f->data;
	const char *filename = (const char *)arg;
    if (filename && zpl_media_file_lookup(filename))
    {
        obj->filemedia  = zpl_media_file_open(filename);
		if(obj->filemedia)
		{
			ms_error("MSH264Player: fail to open %s. A file is already opened", filename);
			goto fail;
		}
	}
	else
	{
		ms_error("MSH264Player: fail to open %s. A file is already opened", filename);
		goto fail;
	}
	ms_filter_lock(f);
	if(obj->state != MSPlayerClosed) {
		ms_error("MSH264Player: fail to open %s. A file is already opened", filename);
		goto fail;
	}
	ms_message("MSH264Player: opening %s", filename);

	obj->state = MSPlayerPaused;
	obj->vsize.width = obj->filemedia->filedesc.video.vidsize.width;
	obj->vsize.height = obj->filemedia->filedesc.video.vidsize.height;
	obj->fps = obj->filemedia->filedesc.video.framerate;
	obj->bitrate = obj->filemedia->filedesc.video.bitrate;
	ms_filter_notify_no_arg(f,MS_FILTER_OUTPUT_FMT_CHANGED);
	ms_filter_unlock(f);
	return 0;

fail:
	ms_filter_unlock(f);
	return -1;
}

static void set_mblk(mblk_t **packet, mblk_t *newone) {
	if (newone) {
		newone = copyb(newone);
	}

	if (*packet) {
		freemsg(*packet);
	}

	*packet = newone;
}

static void player_preprocess(MSFilter *f) {
	MSH264Player *d = (MSH264Player *)f->data;

	d->packer = rfc3984_new();
	rfc3984_set_mode(d->packer, d->mode);
	rfc3984_enable_stap_a(d->packer, FALSE);
	//ms_video_starter_init(&d->starter);
	//ms_iframe_requests_limiter_init(&d->iframe_limiter, 1000);	
}

static void player_postprocess(MSFilter *f) {
	MSH264Player *d = (MSH264Player *)f->data;

	if (d->packer){
		rfc3984_destroy(d->packer);
		d->packer = NULL;
	}
	set_mblk(&d->sps, NULL);
	set_mblk(&d->pps, NULL);
}

static void player_process(MSFilter *f) {

	MSH264Player *d = (MSH264Player *)f->data;
	//mblk_t *im;
	long long int ts = f->ticker->time * 90LL;

	size_t bufsize;
	bool_t have_seen_sps_pps = FALSE;
	ms_filter_lock(f);
	if(d->state == MSPlayerPlaying) 
	{
		zpl_media_bufcache_t *bufcache = &d->filemedia->bufcache;

        if (zpl_media_file_read(d->filemedia, bufcache) > 0)
        {
			uint8_t *buf = bufcache->data;
            if (bufcache->len)
            {
				mblk_t *m;
				MSQueue nalus;

				bufsize = bufcache->len;
				ms_queue_init(&nalus);
				ms_h264_bitstream_to_nalus(buf + 4, bufcache->len, &nalus);

				if (!ms_queue_empty(&nalus)) 
				{
					m = ms_queue_peek_first(&nalus);

					switch (ms_h264_nalu_get_type(m)) 
					{
						case MSH264NaluTypeIDR:
							if (!have_seen_sps_pps) 
							{
								ms_message("MSMediaCodecH264Enc: seeing IDR without prior SPS/PPS, so manually adding them.");

								if (d->sps && d->pps) 
								{
									ms_queue_insert(&nalus, m, copyb(d->sps));
									ms_queue_insert(&nalus, m, copyb(d->pps));
								} 
								else 
								{
									ms_error("MSMediaCodecH264Enc: SPS or PPS are not known !");
								}
							}
							break;

						case MSH264NaluTypeSPS:
							ms_message("MSMediaCodecH264Enc: seeing SPS");
							have_seen_sps_pps = TRUE;
							set_mblk(&d->sps, m);
							m = ms_queue_next(&nalus, m);

							if (!ms_queue_end(&nalus, m) && ms_h264_nalu_get_type(m) == MSH264NaluTypePPS) 
							{
								ms_message("MSMediaCodecH264Enc: seeing PPS");
								set_mblk(&d->pps, m);
							}
							break;

						case MSH264NaluTypePPS:
							ms_warning("MSMediaCodecH264Enc: unexpecting starting PPS");
							break;
						default:
							break;
					}

					rfc3984_pack(d->packer, &nalus, f->outputs[0], ts);

					/*if (d->framenum == 0) 
					{
						ms_video_starter_first_frame(&d->starter, f->ticker->time);
					}*/
					d->framenum++;
				}
				else
				{
					ms_error("MSMediaCodecH264Enc: no NALUs in buffer obtained from MediaCodec");
				}
			}
			else
			{
				ms_error("MSMediaCodecH264Enc: AMediaCodec_getOutputBuffer() returned NULL");
			}
		}
	}
	ms_filter_unlock(f);
}

static int player_close(MSFilter *f, void *arg) {
	MSH264Player *obj = (MSH264Player *)f->data;
	ms_filter_lock(f);
	if(obj->state != MSPlayerClosed) {
		obj->state = MSPlayerClosed;
	}
	if(obj->filemedia)
	{
		zpl_media_file_close(obj->filemedia);
		zpl_media_file_destroy(obj->filemedia);
	}	
	ms_filter_unlock(f);
	return 0;
}

static int player_start(MSFilter *f, void *arg) {
	MSH264Player *obj = (MSH264Player *)f->data;
	ms_filter_lock(f);
	if(obj->state == MSPlayerClosed) {
		goto fail;
	}
	obj->state = MSPlayerPlaying;
	ms_filter_unlock(f);
	return 0;

fail:
	ms_filter_unlock(f);
	return -1;
}

static int player_stop(MSFilter *f, void *arg) {
	MSH264Player *obj = (MSH264Player *)f->data;
	ms_filter_lock(f);
	if(obj->state == MSPlayerPlaying) {
		obj->state = MSPlayerPaused;
	}
	ms_filter_unlock(f);
	return 0;
}

static int player_get_output_fmt(MSFilter *f, void *arg) {
	MSH264Player *obj = (MSH264Player *)f->data;
	MSPinFormat *pinFmt = (MSPinFormat *)arg;
	ms_filter_lock(f);
	if(obj->state == MSPlayerClosed) {
		ms_error("MSH264Player: cannot get pin format when player is closed");
		goto fail;
	}
	if (pinFmt->pin == 0) 
		pinFmt->fmt = ms_factory_get_video_format(f->factory, obj->mime, obj->vsize, obj->fps, NULL);
	ms_filter_unlock(f);
	return 0;

fail:
	ms_filter_unlock(f);
	return -1;
}

static int player_get_state(MSFilter *f, void *arg) {
	MSH264Player *obj = (MSH264Player *)f->data;
	MSPlayerState *state = (MSPlayerState *)arg;
	ms_filter_lock(f);
	*state = obj->state;
	ms_filter_unlock(f);
	return 0;
}

static int player_get_br(MSFilter *f, void *arg) {
	MSH264Player *obj = (MSH264Player *)f->data;
	ms_filter_lock(f);
	*(int *)arg = obj->bitrate;
	ms_filter_unlock(f);
	return 0;
}

static int player_get_vsize(MSFilter *f, void *arg) {
	MSH264Player *obj = (MSH264Player *)f->data;
	ms_filter_lock(f);
	if(obj->state == MSPlayerClosed) {
		ms_error("MSH264Player: cannot get duration. No file is open");
		goto fail;
	}
	*(MSVideoSize *)arg = obj->vsize;
	ms_filter_unlock(f);
	return 0;

fail:
	ms_filter_unlock(f);
	return -1;
}

static int player_get_fps(MSFilter *f, void *arg) {
	MSH264Player *obj = (MSH264Player *)f->data;
	ms_filter_lock(f);
	if(obj->state == MSPlayerClosed) {
		ms_error("MSH264Player: cannot get current duration. No file is open");
		goto fail;
	}
	*(float *)arg = (float)obj->fps;
	ms_filter_unlock(f);
	return 0;

fail:
	ms_filter_unlock(f);
	return -1;
}

static MSFilterMethod player_methods[] = {
	{	MS_FILTER_GET_OUTPUT_FMT         ,	player_get_output_fmt        },
	{	MS_PLAYER_OPEN                   ,	player_open_file             },
	{	MS_PLAYER_CLOSE                  ,	player_close                 },
	{	MS_PLAYER_START                  ,	player_start                 },
	{	MS_PLAYER_PAUSE                  ,	player_stop                  },
	{	MS_PLAYER_GET_STATE              ,	player_get_state             },
	{ 	MS_FILTER_GET_BITRATE,              player_get_br                 },
	{ 	MS_FILTER_GET_FPS,                  player_get_fps                },
	{ 	MS_FILTER_GET_VIDEO_SIZE,           player_get_vsize              },
	{	0                                ,	NULL                         }
};

#ifdef _MSC_VER
MSFilterDesc ms_h264_player_desc = {
	MS_VT_H264_FILE_ID,
	"MSH264Player",
	"H264 file player",
	MS_FILTER_OTHER,
	NULL,
	0,
	2,
	player_init,
	player_preprocess,
	player_process,
	player_postprocess,
	player_uninit,
	player_methods,
	0
};
#else
MSFilterDesc ms_h264_player_desc = {
	.id = MS_VT_H264_FILE_ID,
	.name = "MSH264Player",
	.text = "H264 file player",
	.category = MS_FILTER_OTHER,
	.enc_fmt = NULL,
	.ninputs = 0,
	.noutputs = 2,
	.init = player_init,
	.preprocess = player_preprocess,
	.process = player_process,
	.postprocess = player_postprocess,
	.uninit = player_uninit,
	.methods = player_methods,
	.flags = 0
};
#endif

MS_FILTER_DESC_EXPORT(ms_h264_player_desc)
