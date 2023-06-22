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

/**
 * Default file player/writer buffer size.
 */

#include <pjsua-lib/pjsua.h>
#include <pjmedia/vid_port.h>
#include <pjapp_media_file.h>
#include "zpl_media.h"
#include "zpl_media_internal.h"

#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)

#define THIS_FILE "pjmeida_file.c"

//#define SIGNATURE PJMEDIA_SIG_PORT_VID_AVI_PLAYER

#define VIDEO_CLOCK_RATE 90000

struct pjapp_vidfile_reader_port
{
    pjmedia_port base;

    pj_pool_t *pool;
    pjmedia_format_id fmt_id;

    /* video */
    pj_uint16_t biWidth;
    pj_uint16_t biHeight;
    pj_uint32_t framerate;
    pj_uint32_t profile;
    pj_int32_t packetization_mode;
    pj_off_t         fsize;
    pj_off_t         start_data;
    pj_oshandle_t    fd;
    pj_bool_t eof;
    pj_uint32_t options;
    pj_uint32_t usec_per_frame;
    pj_timestamp next_ts;

    pj_status_t (*cb)(pjmedia_port *, void *);
    pj_bool_t subscribed;
    void (*cb2)(pjmedia_port *, void *);

    zpl_media_file_t *vid_file;
    zpl_media_bufcache_t bufcache;
};

static pj_status_t pjapp_vidfile_get_frame(pjmedia_port *this_port,
                                          pjmedia_frame *frame);
static pj_status_t pjapp_vidfile_on_destroy(pjmedia_port *this_port);



static struct pjapp_vidfile_reader_port *pjmedia_create_file_port(pj_pool_t *pool, const char *filename)
{
    const pj_str_t name = pj_str("file");
    struct pjapp_vidfile_reader_port *port;

    int status;
    port = PJ_POOL_ZALLOC_T(pool, struct pjapp_vidfile_reader_port);
    if (!port)
        return NULL;

    port->vid_file = zpl_media_file_open(filename);
    if (port->vid_file)
    {
        char fmt_name[5];
        pjmedia_video_format_detail *file_vfd;
        port->biWidth = port->vid_file->filedesc.video.vidsize.width;
        port->biHeight = port->vid_file->filedesc.video.vidsize.height;
        port->framerate = port->vid_file->filedesc.video.framerate;
        port->profile = port->vid_file->filedesc.video.profile;
        port->usec_per_frame = port->framerate;
        port->fmt_id = PJMEDIA_FORMAT_H264;

        /*
            ZPL_VIDEO_FORMAT_E	format;
            zpl_video_size_t	vidsize;		//视频大小
            ZPL_VIDEO_CODEC_E 	codectype;		//编码类型
            zpl_uint32			framerate;		//帧率
            zpl_uint32			bitrate;		//码率
            zpl_uint32			profile;        //编码等级
            ZPL_BIT_RATE_E		bitrate_type;	//码率类型
            zpl_uint32		    ikey_rate;	    //I帧间隔
            ZPL_VENC_RC_E		enRcMode;
            ZPL_VENC_GOP_MODE_E	gopmode;
            zpl_uint8           packetization_mode; //封包解包模式
        */
        port->fd = (pj_oshandle_t)(port->vid_file->fp);
        port->base.get_frame = &pjapp_vidfile_get_frame;
        port->base.on_destroy = &pjapp_vidfile_on_destroy;
        
        pjmedia_format_init_video(&port->base.info.fmt,
                                  port->fmt_id,
                                  port->biWidth,
                                  port->biHeight,
                                  port->framerate,
                                  1);
        if(port->base.info.fmt.det.vid.avg_bps == port->base.info.fmt.det.vid.max_bps && port->base.info.fmt.det.vid.max_bps == 0)    
        {

        }  
        /* Collect format info */
        file_vfd = pjmedia_format_get_video_format_detail(&port->base.info.fmt,
                                                          PJ_TRUE);
        PJ_LOG(2, (THIS_FILE, "Reading video stream %dx%d %s @%.2ffps",
                   file_vfd->size.w, file_vfd->size.h,
                   pjmedia_fourcc_name(port->base.info.fmt.id, fmt_name),
                   (1.0 * file_vfd->fps.num / file_vfd->fps.denum)));
    }
    return port;
}

static int pjmedia_file_sprop_parameter_h264(zpl_media_file_t* mfile, char *src, uint32_t len)
{
    char base64sps[OS_BASE64_DECODE_SIZE(1024)];
    char base64pps[OS_BASE64_DECODE_SIZE(1024)];
    zpl_video_extradata_t extradata;
    int sdplength = 0;
    memset(base64pps, 0, sizeof(base64pps));
    memset(base64sps, 0, sizeof(base64sps));
    memset(&extradata, 0, sizeof(zpl_video_extradata_t));

    zpl_media_file_extradata(mfile, &extradata);

    if(extradata.fPPSSize)
        os_base64_encode(base64pps, sizeof(base64pps), extradata.fPPS + extradata.fPPSHdrLen, extradata.fPPSSize - extradata.fPPSHdrLen);
    if(extradata.fSPSSize)
        os_base64_encode(base64sps, sizeof(base64sps), extradata.fSPS + extradata.fSPSHdrLen, extradata.fSPSSize - extradata.fSPSHdrLen);

    if (strlen(base64sps))
    {
        if (strlen(base64pps))
        {
            sdplength += snprintf((char*)(src), len, "%s,%s",base64sps, base64pps);
        }
    }
    return sdplength;
}

PJ_DEF(pj_status_t) pjmedia_file_codeparam_get(
                                        pjmedia_port *port,
                                        void *info)
{
    struct pjapp_vidfile_reader_port *fport = (struct pjapp_vidfile_reader_port*)port;    
    pjmedia_vid_codec_param *vidcodec_param = (pjmedia_vid_codec_param*)info;
    //pjmedia_codec_param *audio_codec_param = (pjmedia_vid_codec_param*)info;
    if(port->info.fmt.type == PJMEDIA_TYPE_AUDIO)
    {

    }
    else if(port->info.fmt.type == PJMEDIA_TYPE_VIDEO)
    {
        //
        char out_str[1024];
        pj_bzero(out_str, sizeof(out_str));
        pj_ansi_snprintf(out_str, sizeof(out_str),"%06x", fport->profile);
        vidcodec_param->enc_fmtp.param[0].name = pj_str((char*)"profile-level-id");
        vidcodec_param->enc_fmtp.param[0].val = pj_strdup3(fport->pool, (char*)out_str);
        vidcodec_param->enc_fmtp.param[1].name = pj_str((char*)" packetization-mode");
        if(fport->packetization_mode >= 0)
            vidcodec_param->enc_fmtp.param[1].val = pj_strdup3(fport->pool, (char*)pj_itoa(fport->packetization_mode));//packetization_mode
        else
            vidcodec_param->enc_fmtp.param[1].val = pj_str((char*)"1");//packetization_mode

        pj_bzero(out_str, sizeof(out_str));
        if(pjmedia_file_sprop_parameter_h264(fport->vid_file, out_str, sizeof(out_str)))
        {
            vidcodec_param->enc_fmtp.param[2].name = pj_str((char*)" sprop-parameter-sets");
            vidcodec_param->enc_fmtp.param[2].val = pj_strdup3(fport->pool, (char*)out_str);
        }
        pjmedia_format_copy(&vidcodec_param->enc_fmt, &port->info.fmt);
        return PJ_SUCCESS;
    }
    return PJ_EINVAL;
}     

#if !DEPRECATED_FOR_TICKET_2251
/*
 * Register a callback to be called when the file reading has reached the
 * end of file.
 */
PJ_DEF(pj_status_t) pjmedia_file_set_eof_cb( pjmedia_port *port,
                               void *user_data,
                               pj_status_t (*cb)(pjmedia_port *port,
                                                 void *usr_data))
{
    struct pjapp_vidfile_reader_port *fport;

    /* Sanity check */
    PJ_ASSERT_RETURN(port, -PJ_EINVAL);

    /* Check that this is really a player port */
    //PJ_ASSERT_RETURN(port->info.signature == SIGNATURE, -PJ_EINVALIDOP);

    PJ_LOG(1, (THIS_FILE, "pjmedia_wav_player_set_eof_cb() is deprecated. "
               "Use pjmedia_wav_player_set_eof_cb2() instead."));

    fport = (struct pjapp_vidfile_reader_port*) port;

    fport->base.port_data.pdata = user_data;
    fport->cb = cb;

    return PJ_SUCCESS;
}
#endif


/*
 * Register a callback to be called when the file reading has reached the
 * end of file.
 */
PJ_DEF(pj_status_t) pjmedia_file_set_eof_cb2(pjmedia_port *port,
                               void *user_data,
                               void (*cb)(pjmedia_port *port,
                                          void *usr_data))
{
    struct pjapp_vidfile_reader_port *fport;

    /* Sanity check */
    PJ_ASSERT_RETURN(port, -PJ_EINVAL);

    /* Check that this is really a player port */
    //PJ_ASSERT_RETURN(port->info.signature == SIGNATURE, -PJ_EINVALIDOP);

    fport = (struct pjapp_vidfile_reader_port*) port;

    fport->base.port_data.pdata = user_data;
    fport->cb2 = cb;

    return PJ_SUCCESS;
}


/*
 * Create AVI player port.
 */
PJ_DEF(pj_status_t) pjmedia_file_create_stream(pj_pool_t *pool,
                           const char *filename,
                           unsigned options,
                           pjmedia_port **p_streams)
{
    struct pjapp_vidfile_reader_port *fport;
    pj_status_t status = PJ_SUCCESS;

    /* Check arguments. */
    PJ_ASSERT_RETURN(pool && filename && p_streams, PJ_EINVAL);

    /* Check the file really exists. */
    if (!pj_file_exists(filename))
    {
        return PJ_ENOTFOUND;
    }

    /* Create fport instance. */
    fport = pjmedia_create_file_port(pool, filename);
    if (!fport)
    {
        return PJ_ENOMEM;
    }
    fport->fsize = pj_file_size(filename);
    fport->start_data = 0;
    pj_strdup2(pool, &fport->base.info.name, filename);
    fport->pool = pool;
    /* Done. */
    *p_streams = pj_pool_alloc(pool, sizeof(pjmedia_port));
    *p_streams = &fport->base;

    PJ_LOG(4,(THIS_FILE, 
              "AVI file player '%.*s' created with ",
              (int)fport->base.info.name.slen,
              fport->base.info.name.ptr));

    //fport->cb = pjmedia_playfile_done;
    //fport->base.port_data.pdata = NULL;
    return PJ_SUCCESS;

on_error:
    fport->base.on_destroy(&fport->base);
    if (status == -1)
        return PJMEDIA_EINVALIMEDIATYPE;
    return status;
}




static pj_status_t pjapp_vidfile_on_event(pjmedia_event *event,
                                 void *user_data)
{
    struct pjapp_vidfile_reader_port *fport = (struct pjapp_vidfile_reader_port*)user_data;

    if (event->type == PJMEDIA_EVENT_CALLBACK) {
        if (fport->cb2)
            (*fport->cb2)(&fport->base, fport->base.port_data.pdata);
    }
    
    return PJ_SUCCESS;
}


/*
 * Get frame from file.
 */
static pj_status_t pjapp_vidfile_get_frame(pjmedia_port *this_port, 
                                 pjmedia_frame *frame)
{
    struct pjapp_vidfile_reader_port *fport = (struct pjapp_vidfile_reader_port*)this_port;
    pj_status_t status = PJ_SUCCESS;
    //pj_ssize_t size_read = 0, size_to_read = 0;

    //pj_assert(fport->base.info.signature == SIGNATURE);
    frame->size = 0;
    frame->type = PJMEDIA_FRAME_TYPE_NONE;
    frame->bit_info = 0;
    /* We encountered end of file */
    if (fport->eof) {
        PJ_LOG(5,(THIS_FILE, "File port %.*s EOF",
                  (int)fport->base.info.name.slen,
                  fport->base.info.name.ptr));

        /* Call callback, if any */
        if (fport->cb2) {
            pj_bool_t no_loop = (fport->options & PJMEDIA_AVI_FILE_NO_LOOP);

            if (!fport->subscribed) {
                status = pjmedia_event_subscribe(NULL, &pjapp_vidfile_on_event,
                                                 fport, fport);
                fport->subscribed = (status == PJ_SUCCESS)? PJ_TRUE:
                                    PJ_FALSE;
            }

            if (fport->subscribed && fport->eof != 2) {
                pjmedia_event event;

                if (no_loop) {
                    /* To prevent the callback from being called repeatedly */
                    fport->eof = 2;
                } else {
                    fport->eof = PJ_FALSE;
                    pj_file_setpos(fport->fd, fport->start_data, PJ_SEEK_SET);
                }

                pjmedia_event_init(&event, PJMEDIA_EVENT_CALLBACK,
                                   NULL, fport);
                pjmedia_event_publish(NULL, fport, &event,
                                      PJMEDIA_EVENT_PUBLISH_POST_EVENT);
            }
            
            /* Should not access player port after this since
             * it might have been destroyed by the callback.
             */
            frame->type = PJMEDIA_FRAME_TYPE_NONE;
            frame->size = 0;
            
            return (no_loop? PJ_EEOF: PJ_SUCCESS);

        } else if (fport->cb) {
            status = (*fport->cb)(this_port, fport->base.port_data.pdata);
        }

        /* If callback returns non PJ_SUCCESS or 'no loop' is specified,
         * return immediately (and don't try to access player port since
         * it might have been destroyed by the callback).
         */
        if ((status != PJ_SUCCESS) ||
            (fport->options & PJMEDIA_AVI_FILE_NO_LOOP)) 
        {
            frame->type = PJMEDIA_FRAME_TYPE_NONE;
            frame->size = 0;
            return PJ_EEOF;
        }

        /* Rewind file */
        PJ_LOG(5,(THIS_FILE, "File port %.*s rewinding..",
                  (int)fport->base.info.name.slen,
                  fport->base.info.name.ptr));
        fport->eof = PJ_FALSE;
        pj_file_setpos(fport->fd, fport->start_data, PJ_SEEK_SET);
    }

    status = zpl_media_file_read(fport->vid_file, &fport->bufcache);
    if(status > 0)
    {
        H264_NALU_T nalu;
        frame->size = fport->bufcache.len;
        memcpy(frame->buf, fport->bufcache.data, fport->bufcache.len);
        frame->type = PJMEDIA_FRAME_TYPE_VIDEO;
        frame->bit_info |= PJMEDIA_VID_FRM_ENCODE;
        if(zpl_media_channel_isnaluhdr(frame->buf, &nalu))
        {
            switch(nalu.nal_unit_type)
            {
                case NALU_TYPE_SLICE:    //非IDR图像中不采用数据划分的片段
                case NALU_TYPE_DPA:      //非IDR图像中A类数据划分片段
                case NALU_TYPE_DPB:      //非IDR图像中B类数据划分片段
                case NALU_TYPE_DPC:      //非IDR图像中C类数据划分片段
                break;
                case NALU_TYPE_IDR:       //IDR图像的片段
                frame->bit_info |= PJMEDIA_VID_FRM_KEYFRAME;
                break;
                case NALU_TYPE_SEI:       //补充增强信息（SEI）
                case NALU_TYPE_SPS:      //序列参数集（SPS）
                case NALU_TYPE_PPS:       //图像参数集（PPS）
                //frame->bit_info |= PJMEDIA_VID_FRM_CONFIG;
                break;
                case NALU_TYPE_AUD:       //分割符
                case NALU_TYPE_EOSEQ:    //序列结束符
                case NALU_TYPE_EOSTREAM: //流结束符
                case NALU_TYPE_FILL:    //填充数据
                break;
            }
        }
    }
    else if(status == -1)
    {
        fport->eof = 1;
        frame->type = PJMEDIA_FRAME_TYPE_NONE;
        frame->size = 0;
        return PJ_EEOF;
    }
    frame->timestamp.u64 = fport->next_ts.u64;

    if (fport->usec_per_frame) 
    {
        fport->next_ts.u64 += (fport->usec_per_frame * VIDEO_CLOCK_RATE / 1000000);
    } 
    else 
    {
        fport->next_ts.u64 += (frame->size * VIDEO_CLOCK_RATE /
                                   (fport->base.info.fmt.det.vid.avg_bps / 8));
    }
    //fport->next_ts.u64 += (frame->size * VIDEO_CLOCK_RATE /
    //                               (fport->base.info.fmt.det.vid.avg_bps / 8));

    return status?PJ_SUCCESS:PJ_EEOF;
}

/*
 * Destroy port.
 */
static pj_status_t pjapp_vidfile_on_destroy(pjmedia_port *this_port)
{
    struct pjapp_vidfile_reader_port *fport = (struct pjapp_vidfile_reader_port*) this_port;

    //pj_assert(this_port->info.signature == SIGNATURE);

    if (fport->subscribed) {
        pjmedia_event_unsubscribe(NULL, &pjapp_vidfile_on_event, fport, fport);
        fport->subscribed = PJ_FALSE;
    }

    if (fport->vid_file)
    {
        zpl_media_file_destroy(fport->vid_file);
        fport->vid_file = NULL;
    }
    return PJ_SUCCESS;
}


#endif /* PJMEDIA_HAS_VIDEO */
