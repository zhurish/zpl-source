/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "auto_include.h"
#include "zplos_include.h"
#include "zpl_media.h"
#include "zpl_media_internal.h"
#include "zpl_media_extradata.h"
#include "zpl_media_file.h"
#include "zpl_media_api.h"

static int zpl_media_file_size(const char *name)
{
    unsigned long filesize = -1;
#ifdef ZPL_BUILD_LINUX
    struct stat statbuff;
    if (stat(name, &statbuff) < 0)
    {
        return filesize;
    }
    else
    {
        filesize = statbuff.st_size;
    }
#else
    FILE *fp = fopen(name, "r");
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        filesize = ftell(fp);
        fclose(fp);
    }
#endif
    zpl_media_debugmsg_debug(" filesize=%d", filesize);
    return (int)filesize;
}

static char *zpl_media_file_basename(const char *name)
{
    static char basename[128];
    memset(basename, 0, sizeof(basename));
    snprintf(basename, sizeof(basename), "%s%s", RTP_MEDIA_BASE_PATH, name);
    zpl_media_debugmsg_debug(" basename=%s", basename);
    return basename;
}
static char *zpl_media_filedesc_name(const char *name)
{
    static char basename[128];
    memset(basename, 0, sizeof(basename));
    snprintf(basename, sizeof(basename), "%s.desc", name);
    return basename;
}

static int zpl_media_filedesc_check(zpl_media_file_t *file)
{
    if(file->filedesc.hdrstr[0] == '$' && file->filedesc.hdrstr[1] == '$' && file->filedesc.hdrstr[2] == '$' && file->filedesc.hdrstr[3] == '$')
    {
        return 1;
    }
    return 0;
}

int zpl_media_filedesc_create(zpl_media_file_t *file)
{
    FILE *fp = NULL;
    if(file->filename[0] == '/')
        fp = fopen(zpl_media_filedesc_name(file->filename), "w+");
    else
        fp = fopen(zpl_media_filedesc_name(zpl_media_file_basename(file->filename)), "w+");
    if(fp)
    {
       fwrite(&file->filedesc, 1, sizeof(zpl_media_filedesc_t), fp);
       fclose(fp);
    }   
    return 0; 
}

static int zpl_media_filedesc_load(zpl_media_file_t *file)
{
    FILE *fp = NULL;
    if(file->filename[0] == '/')
        fp = fopen(zpl_media_filedesc_name(file->filename), "r");
    else
        fp = fopen(zpl_media_filedesc_name(zpl_media_file_basename(file->filename)), "r");
    if(fp)
    {
       fread(&file->filedesc, 1, sizeof(zpl_media_filedesc_t), fp);
       fclose(fp);
    }   
    return 0; 
}

zpl_media_file_t *zpl_media_file_create(const char *name, const char *op)
{
    zpl_media_file_t *file = (zpl_media_file_t *)malloc(sizeof(zpl_media_file_t));
    if (file)
    {
        memset(file, 0, sizeof(zpl_media_file_t));
        if(name[0] == '/')
            file->fp = fopen(name, op);
        else
            file->fp = fopen(zpl_media_file_basename(name), op);
        if (file->fp)
        {
            if (strstr(op, "r"))
            {
                file->file_size = zpl_media_file_size((name[0] == '/')?name:zpl_media_file_basename(name));
                if (file->file_size <= 0)
                {
                    free(file);
                    return NULL;
                }
            }
            else
            {
                file->filedesc.hdrstr[0] = '$';
                file->filedesc.hdrstr[1] = '$';
                file->filedesc.hdrstr[2] = '$';
                file->filedesc.hdrstr[3] = '$';
                file->file_size = 0;

                file->filedesc.begintime = file->filedesc.endtime = time(NULL);
            }
            if(name[0] == '/')
                strcpy(file->filename, name);
            else
                strcpy(file->filename, zpl_media_file_basename(name));    
#ifndef ZPL_MEDIA_FILE_TASK
            file->t_master = NULL;
            file->t_read = NULL;
#endif
            file->cnt = 0;
            file->flags = 0;
            file->offset_len = 0;
            //zpl_media_bufcache_create(&file->tmppacket, 1500);

            rewind(file->fp);

            if (strstr(op, "r"))
            {
                memset(&file->filedesc, 0, sizeof(zpl_media_filedesc_t));

                file->filedesc.video.format = ZPL_VIDEO_FORMAT_NONE;
                file->filedesc.audio.enctype = ZPL_AUDIO_CODEC_NONE;

                file->filedesc.begintime = file->filedesc.endtime = time(NULL);
                file->filedesc.endtime += 60;
                zpl_media_filedesc_load(file);
                if(zpl_media_filedesc_check(file))
                {
                    if(file->filedesc.video.format != ZPL_VIDEO_FORMAT_NONE)
                    {
                        if(file->filedesc.video.framerate)
                            file->msec = 1000U/file->filedesc.video.framerate;
                        file->b_video = zpl_true;
                        if(file->msec == 0)
                            file->msec = 1000U/25;
                    }
                    if(file->filedesc.audio.enctype != ZPL_AUDIO_CODEC_NONE)
                    {
                        if(file->filedesc.audio.framerate)
                            file->msec = 1000U/file->filedesc.audio.framerate;
                        file->b_audio = zpl_true;
                        if(file->msec == 0)
                            file->msec = 1000U/25;
                    }
                }
                else
                {
                    memset(&file->filedesc, 0, sizeof(zpl_media_filedesc_t));
                    rewind(file->fp);
                }
            }
            else
            {
                if(file->filedesc.video.format != ZPL_VIDEO_FORMAT_NONE)
                {
                    if(file->filedesc.video.framerate)
                        file->msec = 1000U/file->filedesc.video.framerate;
                    file->b_video = zpl_true;
                    if(file->msec == 0)
                        file->msec = 1000U/25;
                }
                if(file->filedesc.audio.enctype != ZPL_AUDIO_CODEC_NONE)
                {
                    if(file->filedesc.audio.framerate)
                        file->msec = 1000U/file->filedesc.audio.framerate;
                    file->b_audio = zpl_true;
                    if(file->msec == 0)
                        file->msec = 1000U/25;
                }
            }
            file->filedesc.video.enctype = ZPL_VIDEO_CODEC_H264;
            file->b_video = zpl_true;
            //file->msec -= 60;

            return file;
        }
        else
        {
            free(file);
        }
    }
    return NULL;
}

int zpl_media_file_open(zpl_media_file_t *file)
{
    if (file && file->fp == NULL)
    {
        file->fp = fopen(zpl_media_file_basename(file->filename), "r");
    }
    if (file && file->fp)
    {
        rewind(file->fp);
        return 0;
    }
    return -1;
}

int zpl_media_file_close(zpl_media_file_t *file)
{
    if (file && file->fp)
    {
        file->filedesc.endtime = time(NULL);
        zpl_media_filedesc_create(file);
        fclose(file->fp);
        file->fp = NULL;
        return 0;
    }
    return -1;
}

int zpl_media_file_destroy(zpl_media_file_t *file)
{
    if (file)
    {
        if (file->fp)
        {
            fclose(file->fp);
            file->fp = NULL;
        }
        //zpl_media_bufcache_destroy (&file->tmppacket);
        free(file);
    }
    return 0;
}

int zpl_media_file_write(zpl_media_file_t *file, zpl_skbuffer_t *bufdata)
{
    if (file)
    {
        int ret = fwrite(ZPL_SKB_DATA(bufdata), 1, ZPL_SKB_DATA_LEN(bufdata), file->fp);
        fflush(file->fp);
        return ret;
    }
    return -1;
}

int zpl_media_file_codecdata(zpl_media_file_t *file, zpl_bool video, void *codec)
{
    if (file)
    {
        if(file->b_video && video)
        {
            memcpy(codec, &file->filedesc.video, sizeof(zpl_video_codec_t));
        }
        else if(file->b_audio && !video)
        {
            memcpy(codec, &file->filedesc.audio, sizeof(zpl_audio_codec_t));
        }
    }
    return 0;
}

int zpl_media_file_interval(zpl_media_file_t *file, int interval)
{
    if (file)
    {
        file->msec = interval;
    }
    return 0;
}

int zpl_media_file_get_frame_callback(zpl_media_file_t *file, int (*func)(zpl_media_file_t*, zpl_media_bufcache_t *))
{
    if (file)
    {
        file->get_frame = func;
    }
    return 0;
}

int zpl_media_file_get_extradata_callback(zpl_media_file_t *file, int (*func)(zpl_media_file_t*, zpl_video_extradata_t *))
{
    if (file)
    {
        file->get_extradata = func;
    }
    return 0;
}

int zpl_media_file_put_frame_callback(zpl_media_file_t *file, int (*func)(zpl_media_file_t*, zpl_media_bufcache_t *))
{
    if (file)
    {
        file->put_frame = func;
    }
    return 0;
}

int zpl_media_file_put_extradata_callback(zpl_media_file_t *file, int (*func)(zpl_media_file_t*, zpl_video_extradata_t *))
{
    if (file)
    {
        file->put_extradata = func;
    }
    return 0;
}


int zpl_media_file_pdata(zpl_media_file_t *file, void *pdata)
{
    if (file)
    {
        file->pdata = pdata;
    }
    return 0;
}

int zpl_media_file_extradata(zpl_media_file_t *file, zpl_video_extradata_t *extradata)
{
    int ret = 0;
    if (!file || !extradata)
    {
        return -1;
    }
    zpl_media_bufcache_t tmp;
    int offset = ftell(file->fp);
    zpl_media_bufcache_create(&tmp, 1500);

    if (file && file->get_extradata)
    {
        ret = (file->get_extradata)(file, extradata);
        zpl_media_bufcache_destroy(&tmp);
        return ret;
    }
    else if (file && file->get_frame)
    {
        int n = 0;
        while (1)
        {
            if (n > 10)
                break;
            n++;
            tmp.len = 0;
            ret = (file->get_frame)(file, &tmp);
            fprintf(stdout, " zpl_media_file_extradata _get_frame ret=%d %d\r\n", ret, tmp.len);
            fflush(stdout);
            if (ret && tmp.len)
            {
                H264_NALU_T nalu;
                memset(&nalu, 0, sizeof(H264_NALU_T));
                if (zpl_media_channel_isnaluhdr(tmp.data, &nalu) == true)
                {
                    nalu.len = tmp.len;
                    zpl_media_channel_nalu_show(&nalu);
                    if (zpl_media_channel_nalu2extradata(&nalu, extradata))
                    {
                        // file->tmppacket.len = 0;
                        tmp.len = 0;
                        if (offset >= 0)
                            fseek(file->fp, offset, SEEK_SET);
                        zpl_media_bufcache_destroy(&tmp);
                        return 0;
                    }
                }
            }
            else
            {
                break;
            }
        }
        if (offset >= 0)
            fseek(file->fp, offset, SEEK_SET);
        zpl_media_bufcache_destroy(&tmp);
        return ret;
    }

    if (offset >= 0)
        fseek(file->fp, offset, SEEK_SET);
    zpl_media_bufcache_destroy(&tmp);
    return -1;
}

int zpl_media_file_read(zpl_media_file_t *file, zpl_skbuffer_t *pbufdata)
{
    int ret = 0;
    zpl_skbuffer_t *bufdata = NULL;
    zpl_media_channel_t *chn = NULL;
    if (!file || !file->parent)
    {
        return -1;
    }
    chn = file->parent;

    if (file && file->get_frame)
    {
        zpl_media_bufcache_t tmp;
        zpl_media_bufcache_create(&tmp, 1500);
        ret = (file->get_frame)(file, &tmp);
        uint32_t len = tmp.len;
        bufdata = zpl_skbuffer_create(ZPL_SKBUF_TYPE_MEDIA, file->buffer_queue, len);
        if (len > 0 && bufdata && bufdata->skb_data && bufdata->skb_maxsize >= len && tmp.data)
        {
            file->last_ts = zpl_media_timerstamp();
            bufdata->skb_header.media_header.ID = ZPL_MEDIA_CHANNEL_SET(chn->channel, chn->channel_index, chn->channel_type);
            bufdata->skb_header.media_header.buffer_timetick = zpl_media_timerstamp(); // 时间戳
            // bufdata->buffer_seq = file->pack_seq++;
            bufdata->skb_header.media_header.buffer_flags = ZPL_BUFFER_DATA_ENCODE;
            bufdata->skb_len = len; // 当前缓存帧的长度

            memcpy(ZPL_SKB_DATA(bufdata), tmp.data, len);

            fflush(stdout);
            zpl_skbqueue_enqueue(file->buffer_queue, bufdata);
            return ret;
        }
        return ret;
    }
    return -1;
}

int zpl_media_file_update(zpl_media_file_t *channel, bool add)
{
    if (channel)
    {
        if (add)
            channel->cnt++;
        else
            channel->cnt--;
    }
    return 0;
}

int zpl_media_file_get_frame_h264(zpl_media_file_t *file, zpl_media_bufcache_t *outpacket)
{
    int ret = 0;
    uint32_t packetsize = 1400, packet_offset = 0;
    uint32_t nalulen = 0;
    uint8_t *p = NULL;
    H264_NALU_T naluhdr;
    char buftmp[1400 + 16];
    char tmpdata[8];
    memset(&naluhdr, 0, sizeof(H264_NALU_T));
    outpacket->len = 0;
    fprintf(stdout, "============_h264_file_read_data=============\n");
    while(1)
    {
        packetsize = 1400 - packet_offset;
        ret = fread(buftmp + packet_offset, 1, packetsize, file->fp);
        if(ret > 0)
        {
            if (is_nalu4_start(buftmp))
            {
                naluhdr.hdr_len = 4;
                naluhdr.len = naluhdr.hdr_len;
                naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                naluhdr.buf = buftmp;
                file->flags |= 0x01;/* 找到开始的标志 */
                fprintf(stdout, "============_h264_file_read_data==head 4===========\n");
            }
            else if (is_nalu3_start(buftmp))
            {
                naluhdr.hdr_len = 3;
                naluhdr.len = naluhdr.hdr_len;
                naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                naluhdr.buf = buftmp;
                file->flags |= 0x01;/* 找到开始的标志 */
                fprintf(stdout, "============_h264_file_read_data==head 3===========\n");
            }
            if(file->flags & 0x01)
            {
                p = buftmp + naluhdr.len;
                nalulen = zpl_media_channel_get_nextnalu(p, (ret)-4);
                if(nalulen)
                {
                    naluhdr.len += nalulen;
                    zpl_media_bufcache_add(outpacket, buftmp, naluhdr.len);

                    file->offset_len += (naluhdr.len);
                    fseek(file->fp, file->offset_len, SEEK_SET);

                    file->flags = 0;
                    zpl_media_channel_nalu_show(&naluhdr);
                    return outpacket->len;
                }
                else
                {
                    if(ret > 8)
                    {
                        zpl_media_bufcache_add(outpacket, buftmp, ret - 8);
                        memcpy(tmpdata, buftmp+ret-8, 8);
                        memmove(buftmp, tmpdata, 8);
                        packet_offset = 8;
                    }
                    else
                    {
                        zpl_media_bufcache_add(outpacket, buftmp, ret);
                        naluhdr.len += ret;
                        file->offset_len = 0;
                        file->flags = 0;
                        fprintf(stdout, "============_h264_file_read_data==end===========\n");
                        zpl_media_channel_nalu_show(&naluhdr);
                        return outpacket->len;
                    }
                }
            }
        }
        else
            break;
    }
    file->offset_len = 0;
    return -1;
}
#ifndef ZPL_MEDIA_FILE_TASK
static int zpl_media_file_thread(struct eloop *t)
{
    zpl_media_file_t *media = THREAD_ARG(t);
    if (media && media->fp)
    {
        media->t_read = NULL;
        if (zpl_media_file_read(media, NULL) != -1)
            media->t_read = thread_add_timer_msec(media->t_master, zpl_media_file_thread, media, media->msec);
    }
    return OK;
}

int zpl_media_file_start(zpl_media_file_t *media, bool start)
{
    if (!start && media && media->t_master && media->t_read)
    {
        thread_cancel(media->t_read);
        media->t_read = NULL;
    }
    if (start && media && media->t_master && !media->t_read)
    {
        media->t_read = thread_add_timer_msec(media->t_master, zpl_media_file_thread, media, media->msec);
    }
    return 0;
}

int zpl_media_file_master(zpl_media_file_t *file, void *master, int msec)
{
    if(file)
    {
        if (file->t_master && file->t_read)
        {
            thread_cancel(file->t_read);
            file->t_read = NULL;
            file->t_master = master;
            file->msec = 1000U/30;
            //file->msec = msec;
            file->t_read = thread_add_timer_msec(file->t_master, zpl_media_file_thread, file, file->msec);
        }
        else
        {
            file->t_master = master;
            //file->msec = msec;
            file->msec = 1000U/30;
        }
    }
    return 0;
}
#else
static int zpl_media_file_thread(void *t)
{
    zpl_media_file_t *media = (zpl_media_file_t *)(t);
    if (media && media->fp)
    {
        zpl_uint32 timerstamp = 0U;
        media->msec = 1000U/60U;
        while(1)
        {
            if(media->last_ts > 0U)
            {
                timerstamp = (zpl_media_timerstamp() - media->last_ts);
                timerstamp = (media->msec - timerstamp);
            }
            if (zpl_media_file_read(media, NULL) != -1)
            {
                //timerstamp = 5;
                if(timerstamp > 0U && timerstamp <= media->msec)
                {
                    fprintf(stdout, "================timerstamp=%u\r\n", timerstamp);
                    fflush(stdout);
                    zpl_media_msleep(timerstamp);
                }
            }
            else
                break;
            if(media->run == 0)
                break;
        }
    }
    media->taskid = 0;
    return OK;
}

int zpl_media_file_start(zpl_media_file_t *media, bool start)
{
    if (start && media)
    {
        media->msec = 1000U/30 - 10;
        media->run = 1;
        if(media->taskid == 0)
            pthread_create(&media->taskid, NULL, zpl_media_file_thread, media);
        else
        {
            media->run = 1;
        }
    }
    if (!start && media)
    {
        media->msec = 1000U/30 - 10;
        media->run = 0;
        /*if(media->taskid)
        {
            pthread_cancel(media->taskid);
            pthread_join(media->taskid, NULL);

        }*/
    }
    return 0;
}

int zpl_media_file_master(zpl_media_file_t *file, void *master, int msec)
{
    if(file)
    {
        file->msec = 1000U/30 - 10;
        //file->msec = msec;
        file->run = 1;
        if(file->taskid == 0)
            pthread_create(&file->taskid, NULL, zpl_media_file_thread, file);
    }
    return 0;
}
#endif
