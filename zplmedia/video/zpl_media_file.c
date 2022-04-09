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

static int zpl_media_filedesc_check(zpl_media_file_t *file)
{
    if(file->filedesc.hdrstr[0] == '$' && file->filedesc.hdrstr[1] == '$' && file->filedesc.hdrstr[2] == '$' && file->filedesc.hdrstr[3] == '$')
    {
        return 1;
    }
    return 0;
}

zpl_media_file_t *zpl_media_file_create(const char *name, const char *op)
{
    zpl_media_file_t *file = (zpl_media_file_t *)malloc(sizeof(zpl_media_file_t));
    if (file)
    {
        memset(file, 0, sizeof(zpl_media_file_t));
        file->fp = fopen(zpl_media_file_basename(name), op);
        if (file->fp)
        {
            if (strstr(op, "r"))
            {
                file->file_size = zpl_media_file_size(zpl_media_file_basename(name));
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
            strcpy(file->filename, name);
#ifndef ZPL_MEDIA_FILE_TASK
            file->t_master = NULL;
            file->t_read = NULL;
#endif
            file->cnt = 0;
            file->flags = 0;
            file->tmppacket.buffer_len = 0;
            file->tmppacket.buffer_maxsize = 1500;
            file->tmppacket.buffer_data = malloc(file->tmppacket.buffer_maxsize);
            rewind(file->fp);

            if (strstr(op, "r"))
            {
                memset(&file->filedesc, 0, sizeof(zpl_media_filedesc_t));

                file->filedesc.video.format = ZPL_VIDEO_FORMAT_NONE;
                file->filedesc.audio.enctype = ZPL_AUDIO_CODEC_NONE;

                file->filedesc.begintime = file->filedesc.endtime = time(NULL);
                file->filedesc.endtime += 60;

                zpl_media_file_readhdr(file, &file->filedesc);
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
                    rewind(file->fp);
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
        zpl_media_file_writehdr(file, &file->filedesc);
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
        if (file->tmppacket.buffer_data)
        {
            free(file->tmppacket.buffer_data);
            file->tmppacket.buffer_data = NULL;
        }
        
        free(file);
    }
    return 0;
}

int zpl_media_file_write(zpl_media_file_t *file, zpl_buffer_data_t *bufdata)
{
    if (file)
    {
        if(bufdata->buffer_type == ZPL_MEDIA_VIDEO)
        {
            file->tmphead.video = 1;
            file->tmphead.audio = 0;
            file->tmphead.video_len = bufdata->buffer_len;  //缓冲区大小
            file->tmphead.audio_len = 0;  //缓冲区大小
            file->filedesc.videoframe++;
        }
        if(bufdata->buffer_type == ZPL_MEDIA_AUDIO)
        {
            file->tmphead.video = 0;
            file->tmphead.audio = 1;
            file->tmphead.video_len = 0;  //缓冲区大小
            file->tmphead.audio_len = bufdata->buffer_len;  //缓冲区大小
            file->filedesc.audioframe++;
        }
        int ret = fwrite(&file->tmphead, 1, sizeof(zpl_packet_head_t), file->fp);
        ret += fwrite(bufdata->buffer_data, 1, bufdata->buffer_len, file->fp);
        fflush(file->fp);
        //zpl_media_file_writehdr(file, &file->filedesc);
        return ret;
    }
    return -1;
}

int zpl_media_file_write_data(zpl_media_file_t *file, uint8_t *data, uint32_t len)
{
    if (file && len)
    {
        file->tmphead.video = 1;
        file->tmphead.audio = 0;
        file->tmphead.video_len = len;  //缓冲区大小
        file->tmphead.audio_len = 0;  //缓冲区大小
        if(file->tmphead.video)
            file->filedesc.videoframe++;
        if(file->tmphead.audio)
            file->filedesc.audioframe++;
        int ret = fwrite(&file->tmphead, 1, sizeof(zpl_packet_head_t), file->fp);
        ret += fwrite(data, 1, len, file->fp);
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

int zpl_media_file_get_frame_callback(zpl_media_file_t *file, int (*func)(zpl_media_file_t*, zpl_buffer_data_t *))
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

int zpl_media_file_put_frame_callback(zpl_media_file_t *file, int (*func)(zpl_media_file_t*, zpl_buffer_data_t *))
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
    zpl_buffer_data_t tmp;
    int offset = ftell(file->fp);
    memset(&tmp, 0, sizeof(zpl_buffer_data_t));
    tmp.buffer_data = NULL;
    if(zpl_media_filedesc_check(file))
    {
        while(1)
        {
            if (feof(file->fp) != 0)
            {
                if(tmp.buffer_data)
                {
                    free(tmp.buffer_data);
                }
                return -1;
            }
            memset(&file->tmphead, 0, sizeof(zpl_packet_head_t));
            ret = fread(&file->tmphead, 1, sizeof(zpl_packet_head_t), file->fp);
            if (ret == sizeof(zpl_packet_head_t))
            {
                uint32_t len = file->tmphead.audio_len + file->tmphead.video_len;
                if (tmp.buffer_maxsize < len)
                {
                    if (tmp.buffer_data)
                    {
                        tmp.buffer_data = realloc(tmp.buffer_data, len);
                    }
                    else
                    {
                        tmp.buffer_data = malloc(len);
                    }
                    if (tmp.buffer_data)
                        tmp.buffer_maxsize = len;
                }
                if (tmp.buffer_data && tmp.buffer_maxsize >= len)
                {
                    ret = fread(tmp.buffer_data, 1, len, file->fp);
                    if (ret)
                    {
                        H264_NALU_T nalu;
                        tmp.buffer_len = ret;
                        memset(&nalu, 0, sizeof(H264_NALU_T));
                        if(zpl_media_channel_isnaluhdr(tmp.buffer_data, &nalu) == true)
                        {
                            nalu.len = tmp.buffer_len;
                            zpl_media_channel_nalu_show(&nalu);
                            if (zpl_media_channel_nalu2extradata(&nalu, extradata))
                            {
                                file->tmppacket.buffer_len = 0;
                                tmp.buffer_len = 0;
                                if(offset >= 0)
                                    fseek(file->fp, offset, SEEK_SET);
                                if(tmp.buffer_data)
                                {
                                    free(tmp.buffer_data);
                                }
                                return 0;
                            }
                        }
                    }
                    else
                    {
                        if(offset >= 0)
                            fseek(file->fp, offset, SEEK_SET);
                        if(tmp.buffer_data)
                        {
                            free(tmp.buffer_data);
                        }
                        return ret;
                    }
                }
            }
            else
            {
                break;
            }

        }
        if(offset >= 0)
            fseek(file->fp, offset, SEEK_SET);
        if(tmp.buffer_data)
        {
            free(tmp.buffer_data);
        }
        return -1;
    }
    else
    {
        if (file && file->get_extradata)
        {
            ret = (file->get_extradata)(file, extradata);
            if(tmp.buffer_data)
            {
                free(tmp.buffer_data);
            }
            return ret;
        }
        else if (file && file->get_frame)
        {
            int n = 0;
            while(1)
            {
                if(n > 10)
                    break;
                n++;
                tmp.buffer_len = 0;
                ret = (file->get_frame)(file, &tmp);
                fprintf(stdout, " zpl_media_file_extradata _get_frame ret=%d %d\r\n", ret, tmp.buffer_len);
                fflush(stdout);
                if (ret && tmp.buffer_len)
                {
                    H264_NALU_T nalu;
                    memset(&nalu, 0, sizeof(H264_NALU_T));
                    if(zpl_media_channel_isnaluhdr(tmp.buffer_data, &nalu) == true)
                    {
                        nalu.len = tmp.buffer_len;
                        zpl_media_channel_nalu_show(&nalu);
                        if (zpl_media_channel_nalu2extradata(&nalu, extradata))
                        {
                            file->tmppacket.buffer_len = 0;
                            tmp.buffer_len = 0;
                            if(offset >= 0)
                                fseek(file->fp, offset, SEEK_SET);
                            if(tmp.buffer_data)
                            {
                                free(tmp.buffer_data);
                            }
                            return 0;
                        }
                    }
                }
                else
                {
                    break;
                }

            }
            if(offset >= 0)
                fseek(file->fp, offset, SEEK_SET);
            if(tmp.buffer_data)
            {
                free(tmp.buffer_data);
            }
            return ret;
        }
    }
    if(offset >= 0)
        fseek(file->fp, offset, SEEK_SET);
    if(tmp.buffer_data)
    {
        free(tmp.buffer_data);
    }
    return -1;
}

int zpl_media_file_read(zpl_media_file_t *file, zpl_buffer_data_t *pbufdata)
{
    int ret = 0;
    zpl_media_buffer_data_t *bufdata = NULL;
    if (!file)
    {
        return -1;
    }
    if(zpl_media_filedesc_check(file))
    {
        if (feof(file->fp) != 0)
            return -1;
        memset(&file->tmphead, 0, sizeof(zpl_packet_head_t));
        ret = fread(&file->tmphead, 1, sizeof(zpl_packet_head_t), file->fp);
        if (ret == sizeof(zpl_packet_head_t))
        {
            if(file->b_video && file->tmphead.video && file->tmphead.video_len)
            {
                bufdata = zpl_media_buffer_data_malloc(file->buffer_queue, ZPL_MEDIA_VIDEO, ZPL_BUFFER_DATA_ENCODE, file->tmphead.video_len);
                if (bufdata && bufdata->buffer_data && bufdata->buffer_maxsize >= file->tmphead.video_len)
                {
                    ret = fread(bufdata->buffer_data, 1, file->tmphead.video_len, file->fp);
                    if(ret > 0)
                    {
                        file->last_ts = zpl_media_timerstamp();
                        bufdata->ID = 0;                 //ID 通道号
                        bufdata->buffer_type = ZPL_MEDIA_VIDEO;        //音频视频
                        bufdata->buffer_codec = file->filedesc.video.enctype;       //编码类型
                        //bufdata->buffer_key;         //帧类型
                        //bufdata->buffer_priv;
                        bufdata->buffer_timetick = zpl_media_timerstamp();		//时间戳
                        //bufdata->buffer_seq = file->pack_seq++;
                        bufdata->buffer_len = ret; //当前缓存帧的长度
                        //zpl_media_buffer_add(file->buffer_queue, bufdata);
                        zpl_media_buffer_add_and_distribute(file->buffer_queue, bufdata);
                    }
                }
            }
            if(file->b_audio && file->tmphead.audio && file->tmphead.audio_len)
            {
                bufdata = zpl_media_buffer_data_malloc(file->buffer_queue, ZPL_MEDIA_VIDEO, ZPL_BUFFER_DATA_ENCODE, file->tmphead.audio_len);
                if (bufdata && bufdata->buffer_data && bufdata->buffer_maxsize >= file->tmphead.audio_len)
                {
                    ret = fread(bufdata->buffer_data, 1, file->tmphead.audio_len, file->fp);
                    if(ret > 0)
                    {
                        file->last_ts = zpl_media_timerstamp();
                        bufdata->ID = 0;                 //ID 通道号
                        bufdata->buffer_type = ZPL_MEDIA_AUDIO;        //音频视频
                        bufdata->buffer_codec = file->filedesc.audio.enctype;       //编码类型
                        bufdata->buffer_timetick = zpl_media_timerstamp();		//时间戳
                        //bufdata->buffer_seq = file->pack_seq++;
                        bufdata->buffer_len = ret; //当前缓存帧的长度
                        //zpl_media_buffer_add(file->buffer_queue, bufdata);
                        zpl_media_buffer_add_and_distribute(file->buffer_queue, bufdata);
                    }

                }
            }
            return ret;
        }
        return -1;
    }
    else
    {
        if (file && file->get_frame)
        {
            ret = (file->get_frame)(file, bufdata);
            uint32_t len = bufdata->buffer_len;

            bufdata = zpl_media_buffer_data_malloc(file->buffer_queue, ZPL_MEDIA_VIDEO, ZPL_BUFFER_DATA_ENCODE, len);
            if (len > 0 && bufdata && bufdata->buffer_data && bufdata->buffer_maxsize >= len && bufdata->buffer_data)
            {
                file->last_ts = zpl_media_timerstamp();
                //bufdata->ID;				//ID
                bufdata->buffer_timetick = zpl_media_timerstamp();		//时间戳
                //bufdata->buffer_seq = file->pack_seq++;
                bufdata->buffer_len = len; //当前缓存帧的长度

                memcpy(bufdata->buffer_data, bufdata->buffer_data, len);

                //fprintf(stdout, "============zpl_media_file_read=========%d (%s)\r\n", len, zpl_media_buffer_timerstring());
                fflush(stdout);
                //zpl_media_buffer_add(file->buffer_queue, bufdata);
                zpl_media_buffer_add_and_distribute(file->buffer_queue, bufdata);
                return ret;
            }
            return ret;
        }
    }
    return -1;
}

int zpl_media_file_writehdr(zpl_media_file_t *file, zpl_media_filedesc_t *hdr)
{
    int ret = 0;
    int offset = ftell(file->fp);
    if(offset > 0)
        fseek(file->fp, 0, SEEK_SET);
    hdr->hdrstr[0] = '$';
    hdr->hdrstr[1] = '$';
    hdr->hdrstr[2] = '$';
    hdr->hdrstr[3] = '$';
    if(hdr->video.format != ZPL_VIDEO_FORMAT_NONE)
        file->b_audio = zpl_true;
    if(hdr->audio.enctype != ZPL_AUDIO_CODEC_NONE)
        file->b_audio = zpl_true;
    file->filedesc.endtime = time(NULL);
    fprintf(stdout," ========= ");
    fprintf(stdout," =========enctype=%u  \r\n", hdr->video.enctype);
    fprintf(stdout," =========enctype=%u  \r\n", hdr->audio.enctype);
    fprintf(stdout," =========begintime=%u \r\n", hdr->begintime);
    fprintf(stdout," =========endtime=%u  \r\n", hdr->endtime);
    fprintf(stdout," =========videoframe=%u  \r\n", hdr->videoframe);
    fprintf(stdout," =========audioframe=%u  \r\n", hdr->audioframe);

    ret = fwrite(hdr, 1, sizeof(zpl_media_filedesc_t), file->fp);
    if(offset > 0)
        fseek(file->fp, offset, SEEK_SET);
    return ret;
}

int zpl_media_file_readhdr(zpl_media_file_t *file, zpl_media_filedesc_t *hdr)
{
    int ret = 0;
    int offset = ftell(file->fp);
    if(offset > 0)
        fseek(file->fp, 0, SEEK_SET);
    ret = fread(hdr, 1, sizeof(zpl_media_filedesc_t), file->fp);
    if(offset > 0)
        fseek(file->fp, offset, SEEK_SET);
    return ret;
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


#ifndef ZPL_MEDIA_FILE_TASK
static int zpl_media_file_thread(struct eloop *t)
{
    zpl_media_file_t *media = THREAD_ARG(t);
    if (media && media->fp)
    {
        media->t_read = NULL;
        if (zpl_media_file_read(media) != -1)
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
                    //fprintf(stdout, "================timerstamp=%u\r\n", timerstamp);
                    //fflush(stdout);
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
