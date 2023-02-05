﻿/*
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
    snprintf(basename, sizeof(basename), "%s%s", ZPL_MEDIA_BASE_PATH, name);
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
int zpl_media_file_lookup(const char *name)
{
    FILE *fp = NULL;
    if(name[0] == '/')
        fp = fopen(name, "r");
    else
        fp = fopen(zpl_media_file_basename(name), "r");
    if(fp)
    {
        fclose(fp);
        return 1;
    }    
    return 0;
}

int zpl_media_file_check(zpl_media_file_t *media_file, const char *name)
{
    char filename[ZPL_MEDIA_FILE_NAME_MAX];
    if (media_file && media_file->fp && media_file->get_frame)
    {
        memset(filename, 0, sizeof(filename));
        if(name[0] == '/')
            strcpy(filename, name);
        else
            strcpy(filename, zpl_media_file_basename(name));
        if(memcmp(filename, media_file->filename, sizeof(media_file->filename)) == 0)  
            return 1;      
    }
    return 0;
}


static int zpl_media_file_nametokey(const char *name, int *isvideo, int *codec)
{
    int icodec = -1;
    char *nk = name;
    if(strstr(name, "video") || strstr(name, "VIDEO"))
    {
        nk = strstr(name, "video");
        if(nk == NULL)
            nk = strstr(name, "VIDEO");
        if(isvideo)
            *isvideo = 1;
    }
    else if(strstr(name, "audio") || strstr(name, "AUDIO"))
    {
        nk = strstr(name, "audio");
        if(nk == NULL)
            nk = strstr(name, "AUDIO");
        if(isvideo)
            *isvideo = 0;
    }
    icodec = zpl_media_codec_key(nk);
    if(codec)
        *codec = icodec;
    if(icodec != ZPL_AUDIO_CODEC_MAX && icodec != RTP_MEDIA_PAYLOAD_NONE) 
        return OK;
    return ERROR;       
}

zpl_media_file_t *zpl_media_file_create(zpl_media_channel_t *chn, const char *name)
{
    zpl_media_file_t *media_file = NULL;
    media_file = (zpl_media_file_t *)malloc(sizeof(zpl_media_file_t));
    if (media_file)
    {
        memset(media_file, 0, sizeof(zpl_media_file_t));
        if (name[0] == '/')
            media_file->fp = fopen(name, "a+");
        else
            media_file->fp = fopen(zpl_media_file_basename(name), "a+");
        if (media_file->fp)
        {
            media_file->filedesc.hdrstr[0] = '$';
            media_file->filedesc.hdrstr[1] = '$';
            media_file->filedesc.hdrstr[2] = '$';
            media_file->filedesc.hdrstr[3] = '$';
            media_file->file_size = 0;
            media_file->b_create = zpl_true;

            if (name[0] == '/')
                strcpy(media_file->filename, name);
            else
                strcpy(media_file->filename, zpl_media_file_basename(name));

            rewind(media_file->fp);
            if(chn->media_type == ZPL_MEDIA_VIDEO)
            {
                memcpy(&media_file->filedesc.video, &chn->media_param.video_media.codec, sizeof(zpl_video_codec_t));
                media_file->b_video = zpl_true;
            }
            if(chn->media_type == ZPL_MEDIA_AUDIO)
            {
                memcpy(&media_file->filedesc.audio, &chn->media_param.audio_media.codec, sizeof(zpl_audio_codec_t));
                media_file->b_audio = zpl_true;
            }
            if (media_file->_mutex == NULL)
            {
                media_file->_mutex = os_mutex_name_init("media_file_mutex");
            }
            zpl_media_debugmsg_debug("=====================Media file %p", media_file);
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

zpl_media_file_t *zpl_media_file_open(const char *name)
{
    int isvideo = 0, icodec = -1;
    zpl_media_file_t *media_file = NULL;
    media_file = (zpl_media_file_t *)malloc(sizeof(zpl_media_file_t));
    if (media_file)
    {
        memset(media_file, 0, sizeof(zpl_media_file_t));
        if (name[0] == '/')
            media_file->fp = fopen(name, "r");
        else
            media_file->fp = fopen(zpl_media_file_basename(name), "r");
        if (media_file->fp)
        {
            media_file->file_size = zpl_media_file_size((name[0] == '/') ? name : zpl_media_file_basename(name));
            if (media_file->file_size <= 0)
            {
                zpl_media_debugmsg_error("can not get media file size for media channel(%s)", name);
                zpl_media_file_close(media_file);
                free(media_file);
                return NULL;
            }
            if (name[0] == '/')
                strcpy(media_file->filename, name);
            else
                strcpy(media_file->filename, zpl_media_file_basename(name));
            rewind(media_file->fp);

            zpl_media_filedesc_load(media_file);
            if (zpl_media_filedesc_check(media_file))
            {
                if (media_file->filedesc.video.enctype != ZPL_VIDEO_CODEC_NONE)
                {
                    media_file->b_video = zpl_true;
                }
                if (media_file->filedesc.audio.enctype != ZPL_AUDIO_CODEC_NONE)
                {
                    media_file->b_audio = zpl_true;
                }
                if (media_file->filedesc.video.enctype == RTP_MEDIA_PAYLOAD_H264)
                {
                    media_file->get_frame = zpl_media_file_get_frame_h264;
                    zpl_media_debugmsg_debug("media channel(%s) is h264 media file", name);
                }
            }
            else
            {
                
                if (zpl_media_file_nametokey(name, &isvideo, &icodec) != OK)
                {
                    zpl_media_file_close(media_file);
                    return NULL;
                }
                if (isvideo)
                {
                    zpl_video_extradata_t extradata;
                    media_file->b_video = zpl_true;
                    media_file->filedesc.video.enctype = icodec;
                    if(media_file->filedesc.video.enctype == RTP_MEDIA_PAYLOAD_H264)
                    {
                        media_file->get_frame = zpl_media_file_get_frame_h264;
                        memset(&extradata, 0, sizeof(zpl_video_extradata_t));
                        
                        if (zpl_media_file_extradata(media_file, &extradata) == OK && extradata.fSPSSize)
                        {
                            media_file->filedesc.hdrstr[0] = '$';
                            media_file->filedesc.hdrstr[1] = '$';
                            media_file->filedesc.hdrstr[2] = '$';
                            media_file->filedesc.hdrstr[3] = '$';
                            zpl_media_channel_decode_sps(extradata.fSPS, extradata.fSPSSize, &media_file->filedesc.video.vidsize.width, 
                                &media_file->filedesc.video.vidsize.height, &media_file->filedesc.video.framerate);
                        }
                    }
                }
                else
                {
                    media_file->b_audio = zpl_true;
                    media_file->filedesc.audio.enctype = icodec;
                }
                rewind(media_file->fp);
            }

            if (media_file->_mutex == NULL)
            {
                media_file->_mutex = os_mutex_name_init("media_file_mutex");
            }
            zpl_media_debugmsg_debug("=====================Media file %p", media_file);
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
int zpl_media_file_reopen(zpl_media_file_t *media_file)
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
        if(media_file->b_create == zpl_true)
        {
            zpl_media_filedesc_create(media_file);
        }
        zpl_media_bufcache_destroy(&media_file->bufcache);
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
        zpl_media_bufcache_destroy(&media_file->bufcache);
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
        int ret = 0;
        ZPL_MEDIA_FILE_LOCK(media_file);
        if(media_file->put_frame)
            ret = (media_file->put_frame)(media_file, ZPL_SKB_DATA(bufdata), ZPL_SKB_DATA_LEN(bufdata));
        else    
            ret = fwrite(ZPL_SKB_DATA(bufdata), 1, ZPL_SKB_DATA_LEN(bufdata), media_file->fp);
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

int zpl_media_file_get_frame_callback(zpl_media_file_t *media_file, int (*func)(FILE*, zpl_media_bufcache_t *))
{
    if (media_file)
    {
        ZPL_MEDIA_FILE_LOCK(media_file);
        media_file->get_frame = func;
        ZPL_MEDIA_FILE_UNLOCK(media_file);
    }
    return 0;
}

int zpl_media_file_put_frame_callback(zpl_media_file_t *media_file, int (*func)(zpl_media_file_t*, char *, int))
{
    if (media_file)
    {
        ZPL_MEDIA_FILE_LOCK(media_file);
        media_file->put_frame = func;
        ZPL_MEDIA_FILE_UNLOCK(media_file);
    }
    return 0;
}


int zpl_media_file_read(zpl_media_file_t *media_file, zpl_media_bufcache_t *bufcache)
{
    int ret = 0;
    int32_t len = 0;
    if (!media_file)
    {
        return -1;
    }
    ZPL_MEDIA_FILE_LOCK(media_file);

    if (media_file && media_file->get_frame)
    {
        //ret = (media_file->get_frame)(media_file->fp, bufcache);
        ret = zpl_media_file_get_frame_h264(media_file->fp, bufcache);
        len = bufcache->len;

        if (len > 0 && ret && bufcache->data)
        {
            ZPL_MEDIA_FILE_UNLOCK(media_file);
            return len;
        }
        zpl_media_debugmsg_error("can not get media frame data for media channel(%s)", media_file->filename);
        ZPL_MEDIA_FILE_UNLOCK(media_file);
        return -1;
    }
    ZPL_MEDIA_FILE_UNLOCK(media_file);
    return -1;
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

    if (media_file && media_file->get_frame)
    {
        int n = 0;
        while (1)
        {
            if (n > 30)
                break;
            n++;
            tmp.len = 0;
            //ret = (media_file->get_frame)(media_file->fp, &tmp);
            ret = zpl_media_file_get_frame_h264(media_file->fp, &tmp);
            if (ret && tmp.len)
            {
                H264_NALU_T nalu;
                memset(&nalu, 0, sizeof(H264_NALU_T));
                if (zpl_media_channel_isnaluhdr(tmp.data, &nalu) == true)
                {
                    nalu.len = tmp.len;
                    zpl_media_channel_nalu_show(&nalu);
                    if (zpl_media_channel_nalu2extradata(&nalu, extradata) == 0)
                    {
                        tmp.len = 0;
                        if(extradata->fSPSSize && extradata->fPPSSize)
                        {
                            if (offset >= 0)
                                fseek(media_file->fp, offset, SEEK_SET);
                            zpl_media_bufcache_destroy(&tmp);
                            ZPL_MEDIA_FILE_UNLOCK(media_file);
                            return 0;
                        }
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
        return -1;
    }

    if (offset >= 0)
        fseek(media_file->fp, offset, SEEK_SET);
    zpl_media_bufcache_destroy(&tmp);
    ZPL_MEDIA_FILE_UNLOCK(media_file);
    return -1;
}
#if 0
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
#else
int zpl_media_file_get_frame_h264(FILE *fp, zpl_media_bufcache_t *outpacket)
{
    int ret = 0;
    static int tatol_len = 0;
    uint32_t packetsize = 1400, packet_offset = 0;
    uint32_t nalulen = 0, flags = 0, frist = 0;
    int offset_len = 0;
    uint8_t *p = NULL;
    H264_NALU_T naluhdr;
    char buftmp[1400 + 16];
    //char tmpdata[8];
    memset(&naluhdr, 0, sizeof(H264_NALU_T));
    outpacket->len = 0;
    while(fp)
    {
        packetsize = 1400 - packet_offset;
        ret = fread(buftmp + packet_offset, 1, packetsize, fp);
        if(ret > 0)
        {
            if(flags == 0)
            {
                if (is_nalu4_start(buftmp))
                {
                    naluhdr.hdr_len = 4;
                    naluhdr.len = naluhdr.hdr_len;
                    naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                    naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                    naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                    naluhdr.buf = buftmp;
                    flags |= 0x01;/* 找到开始的标志 */
                    frist = 1;
                }
                else if (is_nalu3_start(buftmp))
                {
                    naluhdr.hdr_len = 3;
                    naluhdr.len = naluhdr.hdr_len;
                    naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                    naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                    naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                    naluhdr.buf = buftmp;
                    flags |= 0x01;/* 找到开始的标志 */
                    frist = 1;
                }
            }
            if(flags & 0x01)
            {
                if(frist)
                    p = buftmp + naluhdr.hdr_len;
                else    
                    p = buftmp;
                nalulen = zpl_media_channel_get_nextnalu(p, (ret)-4);
                if(nalulen)
                {
                    naluhdr.len += nalulen;
                    if(frist)
                        zpl_media_bufcache_add(outpacket, buftmp, naluhdr.len);
                    else
                        zpl_media_bufcache_add(outpacket, buftmp, nalulen);
                    if(frist)
                        offset_len = naluhdr.len - ret;
                    else    
                        offset_len = nalulen - ret;
                    if(offset_len)
                        fseek(fp, offset_len, SEEK_CUR);

                    flags = 0;
                    zpl_media_channel_nalu_show(&naluhdr);
                    tatol_len += outpacket->len;
                    return outpacket->len;
                }
                else
                {
                    
                    if(ret > 8)
                    {
                        if(frist)
                            naluhdr.len = (ret-8);
                        else    
                            naluhdr.len += (ret-8);
                        zpl_media_bufcache_add(outpacket, buftmp, ret - 8);
    
                        packet_offset = 0;
                        fseek(fp, -8, SEEK_CUR);
                        frist = 0;
                    }
                    else
                    {
                        zpl_media_bufcache_add(outpacket, buftmp, ret);
                        naluhdr.len += ret;
                        offset_len = 0;
                        flags = 0;
                        //zpl_media_channel_nalu_show(&naluhdr);
                        tatol_len += outpacket->len;
                        return outpacket->len;
                    }
                }
            }
        }
        else
        {
            if(feof(fp))
            {
                zpl_media_debugmsg_error("===========can not get media frame data eof tatol_len=%d packetsize=%d, ret=%d error(%s)", tatol_len, packetsize, ret, strerror(errno));
                break;
            }
            else
            {
                if(ferror(fp))
                {
                    zpl_media_debugmsg_error("===========can not get media frame data tatol_len=%d packetsize=%d, ret=%d error(%s)", tatol_len, packetsize, ret, strerror(errno));
                    break;
                }
            }
        }
    }
    offset_len = 0;
    tatol_len = 0;
    return -1;
}
#endif

static int zpl_media_file_get_frame_h264_test(FILE *fp)
{
    int ret = 0;
    static int tatol_len = 0;
    uint32_t packetsize = 1400, packet_offset = 0;
    uint32_t nalulen = 0, flags = 0, frist = 0;
    int offset_len = 0;//, frame_len = 0;
    uint8_t *p = NULL;
    H264_NALU_T naluhdr;
    char buftmp[1400 + 16];
    char tmpdata[16];
    memset(&naluhdr, 0, sizeof(H264_NALU_T));
    while(fp)
    {
        packetsize = 1400 - packet_offset;
        ret = fread(buftmp + packet_offset, 1, packetsize, fp);
        if(ret > 0)
        {
            if(flags == 0)
            {
                if (is_nalu4_start(buftmp))
                {
                    naluhdr.hdr_len = 4;
                    naluhdr.len = naluhdr.hdr_len;
                    naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                    naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                    naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                    naluhdr.buf = buftmp;
                    flags |= 0x01;/* 找到开始的标志 */
                    frist = 1;
                }
                else if (is_nalu3_start(buftmp))
                {
                    naluhdr.hdr_len = 3;
                    naluhdr.len = naluhdr.hdr_len;
                    naluhdr.forbidden_bit = buftmp[naluhdr.hdr_len] & 0x80;	 //1 bit
                    naluhdr.nal_idc = buftmp[naluhdr.hdr_len] & 0x60;		 // 2 bit
                    naluhdr.nal_unit_type = (buftmp[naluhdr.hdr_len]) & 0x1f; // 5 bit
                    naluhdr.buf = buftmp;
                    flags |= 0x01;/* 设置开始的标志 */
                    frist = 1;
                }
            }
            if(flags & 0x01)/* 找到开始的标志 */
            {
                if(frist)
                    p = buftmp + naluhdr.hdr_len;
                else    
                    p = buftmp;
                nalulen = zpl_media_channel_get_nextnalu(p, (ret)-4);
                if(nalulen)
                {
                    naluhdr.len += nalulen;
                    tatol_len += naluhdr.len;
                    if(frist)
                        offset_len = naluhdr.len - ret;
                    else    
                        offset_len = nalulen - ret;
                    if(offset_len)
                        fseek(fp, offset_len, SEEK_CUR);
                    printf("===========get media frame size=%d offset_len=%d\r\n", naluhdr.len, offset_len);
                    packet_offset = flags = 0;
                    
                    return naluhdr.len;
                }
                else
                {
                    if(ret > 8)
                    {
                        if(frist)
                            naluhdr.len = (ret-8);
                        else    
                            naluhdr.len += (ret-8);
                        memcpy(tmpdata, buftmp+(ret-8), 8);
                        memcpy(buftmp, tmpdata, 8);
                        packet_offset = 0;
                        fseek(fp, -8, SEEK_CUR);
                        frist = 0;
                    }
                    else
                    {
                        naluhdr.len += ret;
                        offset_len = 0;
                        flags = 0;
                        frist = 0;
                        tatol_len += ret;
                        printf("===========get media last frame size=%d\r\n", naluhdr.len);
                        printf("===========get media total frame size=%d\r\n", tatol_len);
                        return naluhdr.len;
                    }
                }
            }
        }
        else
        {
            if(feof(fp))
            {
                printf("===========can not get media frame data eof tatol_len=%d packetsize=%d, ret=%d error(%s)\r\n", tatol_len, packetsize, ret, strerror(errno));
                break;
            }
            else
            {
                if(ferror(fp))
                {
                    printf("===========can not get media frame data tatol_len=%d packetsize=%d, ret=%d error(%s)\r\n", tatol_len, packetsize, ret, strerror(errno));
                    break;
                }
            }
        }
    }
    offset_len = 0;
    tatol_len = 0;
    return -1;
}


int get_frame_h264_test(void)
{
    FILE *fp = NULL;
    fp = fopen("/home/zhurish/workspace/working/zpl-source/source/multimedia/zplmedia/out-video.h264", "r");
    if(fp)
    {
        while(zpl_media_file_get_frame_h264_test(fp) > 0)
        ;
        fclose(fp);
    }
    return 0;
}