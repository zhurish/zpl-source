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
#include "pjmedia_file.h"

#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)

#define THIS_FILE "pjmeida_file.c"

#define SIGNATURE PJMEDIA_SIG_PORT_VID_AVI_PLAYER

#define VIDEO_CLOCK_RATE 90000

typedef struct pjmedia_file_fmt_info
{
    pjmedia_format_id   fmt_id;
    pjmedia_format_id   eff_fmt_id;
} pjmedia_file_fmt_info;


struct pjmedia_file_reader_port
{
    pj_bool_t        b_video;
    pjmedia_port     base;
    unsigned         stream_id;
    unsigned         options;
    pjmedia_format_id fmt_id;
    pj_uint16_t      codec;
    unsigned         usec_per_frame;
    pj_uint16_t      bits_per_sample;

    /* video */
    pj_uint16_t biWidth;
    pj_uint16_t biHeight;
    pj_uint32_t scale;
    pj_uint32_t rate;

    /* audio */
    pj_uint16_t nchannels;          /**< Number of channels.            */
    pj_uint32_t sample_rate;        /**< Sampling rate.                 */
    pj_uint32_t bytes_per_sec;      /**< Average bytes per second.      */
    pj_uint32_t frame_time_usec;      


    pj_bool_t        eof;
    pj_off_t         fsize;
    pj_off_t         start_data;
    pj_uint8_t       pad;
    pj_oshandle_t    fd;
    pj_ssize_t       size_left;
    pj_timestamp     next_ts;

    pj_status_t    (*cb)(pjmedia_port*, void*);
    pj_bool_t        subscribed;
    void           (*cb2)(pjmedia_port*, void*);

    pj_status_t    (*get_frame_cb)(pjmedia_port *this_port, 
                                 pjmedia_frame *frame);
};


static pj_status_t pjmedia_file_get_frame(pjmedia_port *this_port,
                                          pjmedia_frame *frame);
static pj_status_t pjmedia_file_on_destroy(pjmedia_port *this_port);

#if defined(PJ_IS_BIG_ENDIAN) && PJ_IS_BIG_ENDIAN != 0
static void data_to_host(void *data, pj_uint8_t bits, unsigned count)
{
    unsigned i;

    count /= (bits == 32 ? 4 : 2);

    if (bits == 32)
    {
        pj_int32_t *data32 = (pj_int32_t *)data;
        for (i = 0; i < count; ++i)
            data32[i] = pj_swap32(data32[i]);
    }
    else
    {
        pj_int16_t *data16 = (pj_int16_t *)data;
        for (i = 0; i < count; ++i)
            data16[i] = pj_swap16(data16[i]);
    }
}
static void data_to_host2(void *data, pj_uint8_t nsizes,
                          pj_uint8_t *sizes)
{
    unsigned i;
    pj_int8_t *datap = (pj_int8_t *)data;
    for (i = 0; i < nsizes; i++)
    {
        data_to_host(datap, 32, sizes[i]);
        datap += sizes[i++];
        if (i >= nsizes)
            break;
        data_to_host(datap, 16, sizes[i]);
        datap += sizes[i];
    }
}
#else
#define data_to_host(data, bits, count)
#define data_to_host2(data, nsizes, sizes)
#endif

static pj_status_t file_read3(pj_oshandle_t fd, void *data, pj_ssize_t size,
                              pj_uint16_t bits, pj_ssize_t *psz_read)
{
    pj_ssize_t size_read = size, size_to_read = size;
    pj_status_t status = pj_file_read(fd, data, &size_read);
    if (status != PJ_SUCCESS)
        return status;

    /* Normalize AVI header fields values from little-endian to host
     * byte order.
     */
    if (bits > 0)
        data_to_host(data, bits, size_read);

    if (size_read != size_to_read)
    {
        if (psz_read)
            *psz_read = size_read;
        return -1;
    }

    return status;
}

static struct pjmedia_file_reader_port *pjmedia_create_file_port(pj_pool_t *pool)
{
    const pj_str_t name = pj_str("file");
    struct pjmedia_file_reader_port *port;

    port = PJ_POOL_ZALLOC_T(pool, struct pjmedia_file_reader_port);
    if (!port)
        return NULL;

    /* Put in default values.
     * These will be overriden once the file is read.
     */
    pjmedia_port_info_init(&port->base.info, &name, SIGNATURE,
                           8000, 1, 16, 80);

    port->fd = (pj_oshandle_t)(pj_ssize_t)-1;
    port->base.get_frame = &pjmedia_file_get_frame;
    port->base.on_destroy = &pjmedia_file_on_destroy;

    return port;
}
/*
 * Create AVI player port.
 */
PJ_DEF(pj_status_t)
pjmedia_file_create_stream(pj_pool_t *pool,
                           const char *filename,
                           unsigned options,
                           pjmedia_file_stream **p_streams)
{
    struct pjmedia_file_reader_port *fport;
    pj_status_t status = PJ_SUCCESS;

    /* Check arguments. */
    PJ_ASSERT_RETURN(pool && filename && p_streams, PJ_EINVAL);

    /* Check the file really exists. */
    if (!pj_file_exists(filename))
    {
        return PJ_ENOTFOUND;
    }

    /* Create fport instance. */
    fport = pjmedia_create_file_port(pool);
    if (!fport)
    {
        return PJ_ENOMEM;
    }

    /* Get the file size. */
    fport->fsize = pj_file_size(filename);

    /* Size must be more than AVI header size */
    if (fport->fsize <= 4)
    {
        return PJMEDIA_EINVALIMEDIATYPE;
    }

    /* Open file. */
    status = pj_file_open(pool, filename, PJ_O_RDONLY, &fport->fd);
    if (status != PJ_SUCCESS)
        return status;

    // fmt_id = PJMEDIA_FORMAT_PCMU;
    if (fport->b_video)
    {
        const pjmedia_video_format_info *vfi;
        vfi = pjmedia_get_video_format_info(
            pjmedia_video_format_mgr_instance(),
            fport->codec);

        fport->bits_per_sample = (vfi ? vfi->bpp : 0);
        // fport->usec_per_frame = avi_hdr.avih_hdr.usec_per_frame;
        pjmedia_format_init_video(&fport->base.info.fmt,
                                  fport->fmt_id,
                                  fport->biWidth,
                                  fport->biHeight,
                                  fport->rate,
                                  fport->scale);
    }

    else
    {

        // fport->bits_per_sample = strf_hdr->bits_per_sample;
        // fport->usec_per_frame = avi_hdr.avih_hdr.usec_per_frame;
        pjmedia_format_init_audio(&fport->base.info.fmt,
                                  fport->fmt_id,
                                  fport->sample_rate,
                                  fport->nchannels,
                                  fport->bits_per_sample,
                                  20000 /* fport->usec_per_frame */,
                                  fport->bytes_per_sec * 8,
                                  fport->bytes_per_sec * 8);

        /* Set format to PCM (we will decode PCMA/U) */
        if (fport->fmt_id == PJMEDIA_FORMAT_PCMA ||
            fport->fmt_id == PJMEDIA_FORMAT_PCMU)
        {
            fport->base.info.fmt.id = PJMEDIA_FORMAT_PCM;
            fport->base.info.fmt.det.aud.bits_per_sample = 16;
        }
    }

    pj_strdup2(pool, &fport->base.info.name, filename);

    /* Done. */
    *p_streams = pj_pool_alloc(pool, sizeof(pjmedia_file_stream));

    (*p_streams) = &fport->base;

    return PJ_SUCCESS;

on_error:
    fport->base.on_destroy(&fport->base);
    if (status == -1)
        return PJMEDIA_EINVALIMEDIATYPE;
    return status;
}

/*
 * Get the data length, in bytes.
 */
PJ_DEF(pj_ssize_t)
pjmedia_file_stream_get_len(pjmedia_file_stream *stream)
{
    struct pjmedia_file_reader_port *fport;

    /* Sanity check */
    PJ_ASSERT_RETURN(stream, -PJ_EINVAL);

    /* Check that this is really a player port */
    PJ_ASSERT_RETURN(stream->info.signature == SIGNATURE, -PJ_EINVALIDOP);

    fport = (struct pjmedia_file_reader_port *)stream;

    return (pj_ssize_t)(fport->fsize - fport->start_data);
}

#if !DEPRECATED_FOR_TICKET_2251
/*
 * Register a callback to be called when the file reading has reached the
 * end of file.
 */
PJ_DEF(pj_status_t)
pjmedia_file_stream_set_eof_cb(pjmedia_file_stream *stream,
                               void *user_data,
                               pj_status_t (*cb)(pjmedia_file_stream *stream,
                                                 void *usr_data))
{
    struct pjmedia_file_reader_port *fport;

    /* Sanity check */
    PJ_ASSERT_RETURN(stream, -PJ_EINVAL);

    /* Check that this is really a player port */
    PJ_ASSERT_RETURN(stream->info.signature == SIGNATURE, -PJ_EINVALIDOP);

    PJ_LOG(1, (THIS_FILE, "pjmedia_file_stream_set_eof_cb() is deprecated. "
                          "Use pjmedia_file_stream_set_eof_cb2() instead."));

    fport = (struct pjmedia_file_reader_port *)stream;

    fport->base.port_data.pdata = user_data;
    fport->cb = cb;

    return PJ_SUCCESS;
}
#endif

/*
 * Register a callback to be called when the file reading has reached the
 * end of file.
 */
PJ_DEF(pj_status_t)
pjmedia_file_stream_set_eof_cb2(pjmedia_file_stream *stream,
                                void *user_data,
                                void (*cb)(pjmedia_file_stream *stream,
                                           void *usr_data))
{
    struct pjmedia_file_reader_port *fport;

    /* Sanity check */
    PJ_ASSERT_RETURN(stream, -PJ_EINVAL);

    /* Check that this is really a player port */
    PJ_ASSERT_RETURN(stream->info.signature == SIGNATURE, -PJ_EINVALIDOP);

    fport = (struct pjmedia_file_reader_port *)stream;

    fport->base.port_data.pdata = user_data;
    fport->cb2 = cb;

    return PJ_SUCCESS;
}

static pj_status_t pjmedia_file_on_event(pjmedia_event *event,
                                         void *user_data)
{
    struct pjmedia_file_reader_port *fport = (struct pjmedia_file_reader_port *)user_data;

    if (event->type == PJMEDIA_EVENT_CALLBACK)
    {
        if (fport->cb2)
            (*fport->cb2)(&fport->base, fport->base.port_data.pdata);
    }

    return PJ_SUCCESS;
}

/*
 * Get frame from file.
 */
static pj_status_t pjmedia_file_get_frame(pjmedia_port *this_port,
                                          pjmedia_frame *frame)
{
    struct pjmedia_file_reader_port *fport = (struct pjmedia_file_reader_port *)this_port;
    pj_status_t status = PJ_SUCCESS;
    pj_ssize_t size_read = 0, size_to_read = 0;

    pj_assert(fport->base.info.signature == SIGNATURE);

    /* We encountered end of file */
    if (fport->eof)
    {
        PJ_LOG(5, (THIS_FILE, "File port %.*s EOF",
                   (int)fport->base.info.name.slen,
                   fport->base.info.name.ptr));

        /* Call callback, if any */
        if (fport->cb2)
        {
            pj_bool_t no_loop = (fport->options & PJMEDIA_FILE_NO_LOOP);

            if (!fport->subscribed)
            {
                status = pjmedia_event_subscribe(NULL, &pjmedia_file_on_event,
                                                 fport, fport);
                fport->subscribed = (status == PJ_SUCCESS) ? PJ_TRUE : PJ_FALSE;
            }

            if (fport->subscribed && fport->eof != 2)
            {
                pjmedia_event event;

                if (no_loop)
                {
                    /* To prevent the callback from being called repeatedly */
                    fport->eof = 2;
                }
                else
                {
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

            return (no_loop ? PJ_EEOF : PJ_SUCCESS);
        }
        else if (fport->cb)
        {
            status = (*fport->cb)(this_port, fport->base.port_data.pdata);
        }

        /* If callback returns non PJ_SUCCESS or 'no loop' is specified,
         * return immediately (and don't try to access player port since
         * it might have been destroyed by the callback).
         */
        if ((status != PJ_SUCCESS) ||
            (fport->options & PJMEDIA_FILE_NO_LOOP))
        {
            frame->type = PJMEDIA_FRAME_TYPE_NONE;
            frame->size = 0;
            return PJ_EEOF;
        }

        /* Rewind file */
        PJ_LOG(5, (THIS_FILE, "File port %.*s rewinding..",
                   (int)fport->base.info.name.slen,
                   fport->base.info.name.ptr));
        fport->eof = PJ_FALSE;
        pj_file_setpos(fport->fd, fport->start_data, PJ_SEEK_SET);
    }

    /* For PCMU/A audio stream, reduce frame size to half (temporarily). */
    if (fport->base.info.fmt.type == PJMEDIA_TYPE_AUDIO &&
        (fport->fmt_id == PJMEDIA_FORMAT_PCMA ||
         fport->fmt_id == PJMEDIA_FORMAT_PCMU))
    {
        frame->size >>= 1;
    }

    /* Fill frame buffer. */
    size_to_read = frame->size;
    do
    {
        frame->type = (fport->base.info.fmt.type == PJMEDIA_TYPE_VIDEO ? PJMEDIA_FRAME_TYPE_VIDEO : PJMEDIA_FRAME_TYPE_AUDIO);

        if (frame->type == PJMEDIA_FRAME_TYPE_AUDIO)
        {
            if (size_to_read > fport->size_left)
                size_to_read = fport->size_left;
            status = file_read3(fport->fd, (char *)frame->buf + frame->size - size_to_read, size_to_read,
                                fport->bits_per_sample, &size_read);
            if (status != PJ_SUCCESS)
                goto on_error2;
            fport->size_left -= size_to_read;
        }
        else
        {
            int framelen = 160;
            if (fport->get_frame_cb)
                (fport->get_frame_cb)(&fport->base, frame);
            else
            {
                // pj_assert(frame->size >= framelen);
                status = file_read3(fport->fd, frame->buf, framelen,
                                    0, &size_read);
                if (status != PJ_SUCCESS)
                    goto on_error2;
                frame->size = framelen;
            }
            fport->size_left = 0;
        }

        break;

    } while (1);
    frame->timestamp.u64 = fport->next_ts.u64;
    if (frame->type == PJMEDIA_FRAME_TYPE_AUDIO)
    {

        /* Decode PCMU/A frame */
        if (fport->fmt_id == PJMEDIA_FORMAT_PCMA ||
            fport->fmt_id == PJMEDIA_FORMAT_PCMU)
        {
            unsigned i;
            pj_uint16_t *dst;
            pj_uint8_t *src;

            dst = (pj_uint16_t *)frame->buf + frame->size - 1;
            src = (pj_uint8_t *)frame->buf + frame->size - 1;

            if (fport->fmt_id == PJMEDIA_FORMAT_PCMU)
            {
                for (i = 0; i < frame->size; ++i)
                {
                    *dst-- = (pj_uint16_t)pjmedia_ulaw2linear(*src--);
                }
            }
            else
            {
                for (i = 0; i < frame->size; ++i)
                {
                    *dst-- = (pj_uint16_t)pjmedia_alaw2linear(*src--);
                }
            }

            /* Return back the frame size */
            frame->size <<= 1;
        }

        if (fport->usec_per_frame)
        {
            fport->next_ts.u64 += (fport->usec_per_frame *
                                   fport->base.info.fmt.det.aud.clock_rate /
                                   1000000);
        }
        else
        {
            fport->next_ts.u64 += (frame->size *
                                   fport->base.info.fmt.det.aud.clock_rate /
                                   (fport->base.info.fmt.det.aud.avg_bps / 8));
        }
    }
    else
    {
        if (fport->usec_per_frame)
        {
            fport->next_ts.u64 += (fport->usec_per_frame * VIDEO_CLOCK_RATE /
                                   1000000);
        }
        else
        {
            fport->next_ts.u64 += (frame->size * VIDEO_CLOCK_RATE /
                                   (fport->base.info.fmt.det.vid.avg_bps / 8));
        }
    }

    return PJ_SUCCESS;

on_error2:
    if (status == -1)
    {
        fport->eof = PJ_TRUE;

        size_to_read -= size_read;
        if (size_to_read == (pj_ssize_t)frame->size)
        {
            /* Frame is empty */
            frame->type = PJMEDIA_FRAME_TYPE_NONE;
            frame->size = 0;
            return PJ_EEOF;
        }
        pj_bzero((char *)frame->buf + frame->size - size_to_read,
                 size_to_read);

        return PJ_SUCCESS;
    }

    return status;
}

/*
 * Destroy port.
 */
static pj_status_t pjmedia_file_on_destroy(pjmedia_port *this_port)
{
    struct pjmedia_file_reader_port *fport = (struct pjmedia_file_reader_port *)this_port;

    pj_assert(this_port->info.signature == SIGNATURE);

    if (fport->subscribed)
    {
        pjmedia_event_unsubscribe(NULL, &pjmedia_file_on_event, fport, fport);
        fport->subscribed = PJ_FALSE;
    }

    if (fport->fd != (pj_oshandle_t)(pj_ssize_t)-1)
        pj_file_close(fport->fd);
    return PJ_SUCCESS;
}

#endif /* PJMEDIA_HAS_VIDEO */
