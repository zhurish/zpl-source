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


#define ZPL_MEDIA_FILE_LOCK(n)  if((n) && (n)->_mutex) os_mutex_lock((n)->_mutex, OS_WAIT_FOREVER)
#define ZPL_MEDIA_FILE_UNLOCK(n)  if((n) && (n)->_mutex) os_mutex_unlock((n)->_mutex)

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

char *zpl_media_file_basename(const char *name)
{
    static char basename[128];
    memset(basename, 0, sizeof(basename));
    snprintf(basename, sizeof(basename), "%s%s", RTP_MEDIA_BASE_PATH, name);
    //zpl_media_debugmsg_debug(" basename=%s", basename);
    return basename;
}
static char *zpl_media_filedesc_name(const char *name)
{
    static char basename[128];
    memset(basename, 0, sizeof(basename));
    snprintf(basename, sizeof(basename), "%s.desc", name);
    return basename;
}

static int zpl_media_filedesc_check(zpl_media_file_t *media_file)
{
    if(media_file->filedesc.hdrstr[0] == '$' && media_file->filedesc.hdrstr[1] == '$' && media_file->filedesc.hdrstr[2] == '$' && media_file->filedesc.hdrstr[3] == '$')
    {
        return 1;
    }
    return 0;
}

int zpl_media_filedesc_create(zpl_media_file_t *media_file)
{
    FILE *fp = NULL;
    ZPL_MEDIA_FILE_LOCK(media_file);
    if (media_file->filename[0] == '/')
        fp = fopen(zpl_media_filedesc_name(media_file->filename), "w+");
    else
        fp = fopen(zpl_media_filedesc_name(zpl_media_file_basename(media_file->filename)), "w+");
    if(fp)
    {
       fwrite(&media_file->filedesc, 1, sizeof(zpl_media_filedesc_t), fp);
       fclose(fp);
    }   
    ZPL_MEDIA_FILE_UNLOCK(media_file);
    return 0; 
}

static int zpl_media_filedesc_load(zpl_media_file_t *media_file)
{
    FILE *fp = NULL;
    if(media_file->filename[0] == '/')
        fp = fopen(zpl_media_filedesc_name(media_file->filename), "r");
    else
        fp = fopen(zpl_media_filedesc_name(zpl_media_file_basename(media_file->filename)), "r");
    if(fp)
    {
       fread(&media_file->filedesc, 1, sizeof(zpl_media_filedesc_t), fp);
       fclose(fp);
    }   
    return 0; 
}

zpl_media_file_t *zpl_media_file_create(const char *name, const char *op)
{
    zpl_media_file_t *media_file = (zpl_media_file_t *)malloc(sizeof(zpl_media_file_t));
    if (media_file)
    {
        memset(media_file, 0, sizeof(zpl_media_file_t));
        if(name[0] == '/')
            media_file->fp = fopen(name, op);
        else
            media_file->fp = fopen(zpl_media_file_basename(name), op);
        if (media_file->fp)
        {
            if (strstr(op, "r"))
            {
                media_file->file_size = zpl_media_file_size((name[0] == '/')?name:zpl_media_file_basename(name));
                if (media_file->file_size <= 0)
                {
                    zpl_media_debugmsg_error("can not get media file size for media channel(%s)", name);
                    free(media_file);
                    return NULL;
                }
            }
            else
            {
                media_file->filedesc.hdrstr[0] = '$';
                media_file->filedesc.hdrstr[1] = '$';
                media_file->filedesc.hdrstr[2] = '$';
                media_file->filedesc.hdrstr[3] = '$';
                media_file->file_size = 0;

                media_file->filedesc.begintime = media_file->filedesc.endtime = time(NULL);
            }
            if(name[0] == '/')
                strcpy(media_file->filename, name);
            else
                strcpy(media_file->filename, zpl_media_file_basename(name));    

            media_file->bindcount = 0;
            media_file->flags = 0;
            media_file->offset_len = 0;
            //zpl_media_bufcache_create(&media_file->tmppacket, 1500);

            rewind(media_file->fp);

            if (strstr(op, "r"))
            {
                memset(&media_file->filedesc, 0, sizeof(zpl_media_filedesc_t));

                media_file->filedesc.video.format = ZPL_VIDEO_FORMAT_NONE;
                media_file->filedesc.audio.enctype = ZPL_AUDIO_CODEC_NONE;

                media_file->filedesc.begintime = media_file->filedesc.endtime = time(NULL);
                media_file->filedesc.endtime += 60;
                zpl_media_filedesc_load(media_file);
                if(zpl_media_filedesc_check(media_file))
                {
                    if(media_file->filedesc.video.format != ZPL_VIDEO_FORMAT_NONE)
                    {
                        if(media_file->filedesc.video.framerate)
                            media_file->delay_msec = 1000U/media_file->filedesc.video.framerate;
                        media_file->b_video = zpl_true;
                        if(media_file->delay_msec == 0)
                            media_file->delay_msec = 1000U/25;
                    }
                    if(media_file->filedesc.video.enctype == RTP_MEDIA_PAYLOAD_H264)
                    {
                        media_file->get_frame = zpl_media_file_get_frame_h264;
                        zpl_media_debugmsg_debug("media channel(%s) is h264 media file", name);
                    }
                    if(media_file->filedesc.audio.enctype != ZPL_AUDIO_CODEC_NONE)
                    {
                        if(media_file->filedesc.audio.framerate)
                            media_file->delay_msec = 1000U/media_file->filedesc.audio.framerate;
                        media_file->b_audio = zpl_true;
                        if(media_file->delay_msec == 0)
                            media_file->delay_msec = 1000U/25;
                    }
                }
                else
                {
                    memset(&media_file->filedesc, 0, sizeof(zpl_media_filedesc_t));
                    rewind(media_file->fp);
                }
            }
            else
            {
                if(media_file->filedesc.video.format != ZPL_VIDEO_FORMAT_NONE)
                {
                    if(media_file->filedesc.video.framerate)
                        media_file->delay_msec = 1000U/media_file->filedesc.video.framerate;
                    media_file->b_video = zpl_true;
                    if(media_file->delay_msec == 0)
                        media_file->delay_msec = 1000U/25;
                }
                if(media_file->filedesc.audio.enctype != ZPL_AUDIO_CODEC_NONE)
                {
                    if(media_file->filedesc.audio.framerate)
                        media_file->delay_msec = 1000U/media_file->filedesc.audio.framerate;
                    media_file->b_audio = zpl_true;
                    if(media_file->delay_msec == 0)
                        media_file->delay_msec = 1000U/25;
                }
            }
            media_file->filedesc.video.enctype = ZPL_VIDEO_CODEC_H264;
            media_file->b_video = zpl_true;
            if (media_file->_mutex == NULL)
            {
                media_file->_mutex = os_mutex_name_init("media_file_mutex");
            }
            return media_file;
        }
        else
        {
            free(media_file);
        }
    }
    zpl_media_debugmsg_error("can not create media file for media channel(%s)", name);
    return NULL;
}

int zpl_media_file_open(zpl_media_file_t *media_file)
{
    ZPL_MEDIA_FILE_LOCK(media_file);
    if (media_file && media_file->fp == NULL)
    {
        media_file->fp = fopen(zpl_media_file_basename(media_file->filename), "r");
    }
    if (media_file && media_file->fp)
    {
        rewind(media_file->fp);
        ZPL_MEDIA_FILE_UNLOCK(media_file);
        return 0;
    }
    zpl_media_debugmsg_error("can not open media file size for media channel(%s)", media_file->filename);
    ZPL_MEDIA_FILE_UNLOCK(media_file);
    return -1;
}

int zpl_media_file_close(zpl_media_file_t *media_file)
{
    ZPL_MEDIA_FILE_LOCK(media_file);
    if (media_file && media_file->fp)
    {
        media_file->filedesc.endtime = time(NULL);
        zpl_media_filedesc_create(media_file);
        fclose(media_file->fp);
        media_file->fp = NULL;
        ZPL_MEDIA_FILE_UNLOCK(media_file);
        return 0;
    }
    ZPL_MEDIA_FILE_UNLOCK(media_file);
    return -1;
}

int zpl_media_file_destroy(zpl_media_file_t *media_file)
{
    if (media_file)
    {
	    if (media_file->_mutex)
		    os_mutex_lock(media_file->_mutex, OS_WAIT_FOREVER);
        if (media_file->fp)
        {
            fclose(media_file->fp);
            media_file->fp = NULL;
        }
	    if (media_file->_mutex)
		    os_mutex_unlock(media_file->_mutex);        
        if (os_mutex_exit(media_file->_mutex) == OK)
			media_file->_mutex = NULL;
        free(media_file);
    }
    return 0;
}

int zpl_media_file_write(zpl_media_file_t *media_file, zpl_skbuffer_t *bufdata)
{
    if (media_file)
    {
        ZPL_MEDIA_FILE_LOCK(media_file);
        int ret = fwrite(ZPL_SKB_DATA(bufdata), 1, ZPL_SKB_DATA_LEN(bufdata), media_file->fp);
        fflush(media_file->fp);
        ZPL_MEDIA_FILE_UNLOCK(media_file);
        return ret;
    }
    return -1;
}

int zpl_media_file_codecdata(zpl_media_file_t *media_file, zpl_bool video, void *codec)
{
    if (media_file)
    {
        ZPL_MEDIA_FILE_LOCK(media_file);
        if(media_file->b_video && video)
        {
            memcpy(codec, &media_file->filedesc.video, sizeof(zpl_video_codec_t));
        }
        else if(media_file->b_audio && !video)
        {
            memcpy(codec, &media_file->filedesc.audio, sizeof(zpl_audio_codec_t));
        }
        ZPL_MEDIA_FILE_UNLOCK(media_file);
    }
    return 0;
}

int zpl_media_file_interval(zpl_media_file_t *media_file, int interval)
{
    if (media_file)
    {
        ZPL_MEDIA_FILE_LOCK(media_file);
        media_file->delay_msec = interval;
        ZPL_MEDIA_FILE_UNLOCK(media_file);
    }
    return 0;
}

int zpl_media_file_get_frame_callback(zpl_media_file_t *media_file, int (*func)(zpl_media_file_t*, zpl_media_bufcache_t *))
{
    if (media_file)
    {
        ZPL_MEDIA_FILE_LOCK(media_file);
        media_file->get_frame = func;
        ZPL_MEDIA_FILE_UNLOCK(media_file);
    }
    return 0;
}

int zpl_media_file_get_extradata_callback(zpl_media_file_t *media_file, int (*func)(zpl_media_file_t*, zpl_video_extradata_t *))
{
    if (media_file)
    {
        ZPL_MEDIA_FILE_LOCK(media_file);
        media_file->get_extradata = func;
        ZPL_MEDIA_FILE_UNLOCK(media_file);
    }
    return 0;
}

int zpl_media_file_put_frame_callback(zpl_media_file_t *media_file, int (*func)(zpl_media_file_t*, zpl_media_bufcache_t *))
{
    if (media_file)
    {
        ZPL_MEDIA_FILE_LOCK(media_file);
        media_file->put_frame = func;
        ZPL_MEDIA_FILE_UNLOCK(media_file);
    }
    return 0;
}

int zpl_media_file_put_extradata_callback(zpl_media_file_t *media_file, int (*func)(zpl_media_file_t*, zpl_video_extradata_t *))
{
    if (media_file)
    {
        ZPL_MEDIA_FILE_LOCK(media_file);
        media_file->put_extradata = func;
        ZPL_MEDIA_FILE_UNLOCK(media_file);
    }
    return 0;
}


int zpl_media_file_pdata(zpl_media_file_t *media_file, void *pdata)
{
    if (media_file)
    {
        ZPL_MEDIA_FILE_LOCK(media_file);
        media_file->pdata = pdata;
        ZPL_MEDIA_FILE_UNLOCK(media_file);
    }
    return 0;
}

int zpl_media_file_extradata(zpl_media_file_t *media_file, zpl_video_extradata_t *extradata)
{
    int ret = 0;
    if (!media_file || !extradata)
    {
        return -1;
    }
    zpl_media_bufcache_t tmp;
    ZPL_MEDIA_FILE_LOCK(media_file);
    int offset = ftell(media_file->fp);
    zpl_media_bufcache_create(&tmp, 1500);

    if (media_file && media_file->get_extradata)
    {
        ret = (media_file->get_extradata)(media_file, extradata);
        zpl_media_bufcache_destroy(&tmp);
        zpl_media_debugmsg_error("can not get media extradata for media channel(%s)", media_file->filename);
        ZPL_MEDIA_FILE_UNLOCK(media_file);
        return ret;
    }
    else if (media_file && media_file->get_frame)
    {
        int n = 0;
        while (1)
        {
            if (n > 10)
                break;
            n++;
            tmp.len = 0;
            ret = (media_file->get_frame)(media_file, &tmp);
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
                        // media_file->tmppacket.len = 0;
                        tmp.len = 0;
                        if (offset >= 0)
                            fseek(media_file->fp, offset, SEEK_SET);
                        zpl_media_bufcache_destroy(&tmp);
                        ZPL_MEDIA_FILE_UNLOCK(media_file);
                        return 0;
                    }
                }
            }
            else
            {
                zpl_media_debugmsg_error("can not get media frame data for media channel(%s)", media_file->filename);
                break;
            }
        }
        if (offset >= 0)
            fseek(media_file->fp, offset, SEEK_SET);
        zpl_media_bufcache_destroy(&tmp);
        ZPL_MEDIA_FILE_UNLOCK(media_file);
        return ret;
    }

    if (offset >= 0)
        fseek(media_file->fp, offset, SEEK_SET);
    zpl_media_bufcache_destroy(&tmp);
    ZPL_MEDIA_FILE_UNLOCK(media_file);
    return -1;
}

int zpl_media_file_read(zpl_media_file_t *media_file, zpl_skbuffer_t *pbufdata)
{
    int ret = 0;
    zpl_skbuffer_t *bufdata = NULL;
    zpl_media_channel_t *chn = NULL;
    if (!media_file || !media_file->parent)
    {
        return -1;
    }
    ZPL_MEDIA_FILE_LOCK(media_file);
    chn = media_file->parent;

    if (media_file && media_file->get_frame)
    {
        zpl_media_bufcache_t tmp;
        zpl_media_bufcache_create(&tmp, 1500);
        ret = (media_file->get_frame)(media_file, &tmp);
        uint32_t len = tmp.len;
        bufdata = zpl_skbuffer_create(ZPL_SKBUF_TYPE_MEDIA, media_file->frame_queue, len);
        if (len > 0 && bufdata && bufdata->skb_data && bufdata->skb_maxsize >= len && tmp.data)
        {
            zpl_media_hdr_t *media_header = bufdata->skb_hdr.other_hdr;

            media_header->ID = ZPL_MEDIA_CHANNEL_SET(chn->channel, chn->channel_index, chn->channel_type);
            media_header->sessionID = (zpl_uint32)chn;
            media_header->frame_timetick = zpl_media_timerstamp(); // 时间戳

            media_header->frame_codec = ZPL_VIDEO_CODEC_H264;
            media_header->frame_type = ZPL_MEDIA_VIDEO;
            media_header->frame_flags = ZPL_BUFFER_DATA_ENCODE;
            bufdata->skb_len = len; // 当前缓存帧的长度

            memcpy(ZPL_SKB_DATA(bufdata), tmp.data, len);
            //zpl_media_debugmsg_debug("read media frame data and add skbqueue for media channel(%s)", media_file->filename);
            zpl_skbqueue_enqueue(media_file->frame_queue, bufdata);
            ZPL_MEDIA_FILE_UNLOCK(media_file);
            return ret;
        }
        zpl_media_debugmsg_error("can not get media frame data for media channel(%s)", media_file->filename);
        ZPL_MEDIA_FILE_UNLOCK(media_file);
        return ret;
    }
    ZPL_MEDIA_FILE_UNLOCK(media_file);
    return -1;
}

int zpl_media_file_update(zpl_media_file_t *media_file, bool add)
{
    ZPL_MEDIA_FILE_LOCK(media_file);
    if (media_file)
    {
        if (add)
            media_file->bindcount++;
        else
            media_file->bindcount--;
    }
    ZPL_MEDIA_FILE_UNLOCK(media_file);
    return 0;
}

int zpl_media_file_get_frame_h264(zpl_media_file_t *media_file, zpl_media_bufcache_t *outpacket)
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
    while(media_file->fp)
    {
        packetsize = 1400 - packet_offset;
        ret = fread(buftmp + packet_offset, 1, packetsize, media_file->fp);
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
                media_file->flags |= 0x01;/* 找到开始的标志 */
            }
            else if (is_nalu3_start(buftmp))
            {
                naluhdr.hdr_len = 3;
                naluhdr.len = naluhdr.hdr_len;
                naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                naluhdr.buf = buftmp;
                media_file->flags |= 0x01;/* 找到开始的标志 */
            }
            if(media_file->flags & 0x01)
            {
                p = buftmp + naluhdr.len;
                nalulen = zpl_media_channel_get_nextnalu(p, (ret)-4);
                if(nalulen)
                {
                    naluhdr.len += nalulen;
                    zpl_media_bufcache_add(outpacket, buftmp, naluhdr.len);

                    media_file->offset_len += (naluhdr.len);
                    fseek(media_file->fp, media_file->offset_len, SEEK_SET);

                    media_file->flags = 0;
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
                        media_file->offset_len = 0;
                        media_file->flags = 0;
                        //zpl_media_channel_nalu_show(&naluhdr);
                        return outpacket->len;
                    }
                }
            }
        }
        else
            break;
    }
    media_file->offset_len = 0;
    return -1;
}

static int zpl_media_file_read_task(void *t)
{
    zpl_media_file_t *media = (zpl_media_file_t *)(t);
    if (media && media->fp)
    {
        zpl_uint32 timerstamp = media->delay_msec;
        if(media->delay_msec > 100)
            timerstamp -= 25;
        else if(media->delay_msec > 80)
            timerstamp -= 20;
        else if(media->delay_msec > 50)
            timerstamp -= 15;
        else if(media->delay_msec > 30)
            timerstamp -= 10;
        else if(media->delay_msec > 20)
            timerstamp -= 6;
        else if(media->delay_msec > 10)
            timerstamp -= 3;
        while (media->fp && media->run)
        {
            if(media->run == 2)
            {
                zpl_media_msleep(timerstamp);
                continue; 
            }  
            if(media->run == 0)
            {
                break; 
            }  
            ZPL_MEDIA_FILE_LOCK(media);
            timerstamp = media->delay_msec;
            if(media->delay_msec > 100)
                timerstamp -= 25;
            else if(media->delay_msec > 80)
                timerstamp -= 20;
            else if(media->delay_msec > 50)
                timerstamp -= 15;
            else if(media->delay_msec > 30)
                timerstamp -= 10;
            else if(media->delay_msec > 20)
                timerstamp -= 6;
            else if(media->delay_msec > 10)
                timerstamp -= 3;
            ZPL_MEDIA_FILE_UNLOCK(media);
            if (zpl_media_file_read(media, NULL) != -1)
            {
                if(timerstamp > 0U && timerstamp <= media->delay_msec)
                {
                    zpl_media_msleep(timerstamp);
                }
            }
            else
                break;
        }
    }
    media->taskid = 0;
    return OK;
}

int zpl_media_file_play_start(zpl_media_file_t *media, bool start)
{
    if (start && media)
    {
        ZPL_MEDIA_FILE_LOCK(media);
        media->run = 1;
        if(media->taskid == 0)
        	//media->taskid = os_task_create("mediaRtpFileTask", OS_TASK_DEFAULT_PRIORITY,
			//					 0, zpl_media_file_read_task, media, OS_TASK_DEFAULT_STACK);
       	    pthread_create(&media->taskid, NULL, zpl_media_file_read_task, media);
        else
        {
            media->run = 1;
        }
        ZPL_MEDIA_FILE_UNLOCK(media);
    }
    if (!start && media)
    {
        ZPL_MEDIA_FILE_LOCK(media);
        media->run = 2;
        ZPL_MEDIA_FILE_UNLOCK(media);
    }
    return 0;
}


int zpl_media_file_play_destroy(zpl_media_file_t *media_file)
{
    if(media_file)
    {
        ZPL_MEDIA_FILE_UNLOCK(media_file);
        media_file->run = 0;
        ZPL_MEDIA_FILE_UNLOCK(media_file);
    }
    return 0;
}
