/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include "zpl_rtsp.h"
#include "zpl_rtsp_util.h"


static rtsp_log_callback rtsp_log_cb_default = NULL;

void rtsp_log_cb(rtsp_log_callback cb)
{
    rtsp_log_cb_default = cb;
    return;
}

#ifndef ZPL_LIBRTSP_MODULE
void rtsp_vlog(const char *file, const char *func, const int line, int livel, const char *fmt,...)
{
    va_list args;
    va_start (args, fmt);
    switch(livel)
    {
    case LOG_ERR:
        fprintf(stdout, "ERROR:%s[%d]", func, line);
    break;
    case LOG_WARNING:
        fprintf(stdout, "WARNING:%s[%d]", func, line);
    break;
    case LOG_NOTICE:
        fprintf(stdout, "NOTICE:%s[%d]", func, line);
    break;
    case LOG_INFO:
        fprintf(stdout, "INFO:%s[%d]", func, line);
    break;
    case LOG_DEBUG:
        fprintf(stdout, "DEBUG:%s[%d]", func, line);
    break;
    case (LOG_DEBUG+1):
        fprintf(stdout, "TRACE:%s[%d]", func, line);
    break;
    default:
        return;
    }
    vfprintf(stdout, fmt, args);
    va_end (args);
    fprintf(stdout, "\r\n");
    fflush(stdout);
}
#endif
/*
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&rtsp
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&multcast
 * rtsp://user:pass@192.168.1.1:9988/media/channel=1&level=1&tls
 * rtsp://user:pass@192.168.1.1/media/channel=1&level=1
 * rtsp://192.168.1.1:9988/media/channel=1&level=1
 * rtsp://192.168.1.1:9988/media.h264&multcast
 * rtsp://192.168.1.1:9988/media.h264
 */
#if 0
void rtsp_url_stream_path(const char *url, rtsp_urlpath_t *urlpath)
{
    int port = 0;
    char *p = NULL, *brk = NULL;
    if(strstr(url, "channel") && strstr(url, "level"))
    {
        p = strrchr(url, '&');
        p++;
        if(strstr(p, "level"))
        {
            p += 8;
        }
        strncpy(urlpath->url, url, p-url-1);
    }
    else
    {
        p = strstr(url, "&");
        if(p)
        {
            //p--;
            strncpy(urlpath->url, url, p-url);
        }
        else
        {
            strcpy(urlpath->url, url);
        }
    }

    if(strstr(url+4, "rtsp")||strstr(url+4, "tcp"))
    {
        urlpath->mode = RTSP_TRANSPORT_RTP_TCP;
    }
    else if(strstr(url+4, "multcast"))
    {
        urlpath->mode = RTSP_TRANSPORT_RTP_MULTCAST;
    }
    else if(strstr(url+4, "tls"))
    {
        urlpath->mode = RTSP_TRANSPORT_RTP_TLS;
    }
    else
    {
        urlpath->mode = RTSP_TRANSPORT_RTP_UDP;
    }

    p = urlpath->url;
    p += 7;
    brk = strstr(p, "@");
    if(brk == NULL)
    {
        if(strstr(p, ":"))
        {
            sscanf(p, "%[^:]:%d", urlpath->hostname, &port);
        }
        else
        {
            sscanf(p, "%[^/]", urlpath->hostname);
        }
    }
    else
    {
        if(strstr(p, ":"))
            sscanf(p, "%[^:]:%[^@]", urlpath->username, urlpath->password);
        else
            sscanf(p, "%[^@]", urlpath->username);
        p = strstr(p, "@");
        p++;
        if(strstr(p, ":"))
            sscanf(p, "%[^:]:%d", urlpath->hostname, &port);
        else
            sscanf(p, "%[^/]", urlpath->hostname);
    }
    urlpath->port = port;
    p = strstr(p, "channel");
    if(p && strstr(p, "channel") && strstr(p, "level"))
        sscanf(p, "channel=%d&level=%d", &urlpath->channel, &urlpath->level);
    else
    {
        p = strstr(urlpath->url + 8, "/");
        if(p)
        {
            p++;
            if(p && strlen(p))
                strcpy(urlpath->path, p);
        }
        urlpath->channel = urlpath->level = -1;
    }
} 
#else


static void rtsp_url_stream_path_split(const char *url, os_url_t *urlpath)
{
    char *p, *brk;
    char filename[512];
    p = strstr(url, "media");
    if(p)
        p+=6;
    //if(url[0] == '/')
    //    p++;  
    if(p)
    {
        brk = strstr(p, "channel");
        if(brk && strstr(p, "level"))
        {
            sscanf(brk, "channel=%d", &urlpath->channel);
            p = brk + 8;
            brk = strstr(p, "level");
            if(brk)
            {
                sscanf(brk, "level=%d", &urlpath->level);
                p = brk + 6;
            }
        }
        else if(!strstr(p, "channel") && !strstr(p, "level"))
        {
            urlpath->channel = urlpath->level = -1;
            memset(filename, 0, sizeof(filename));
            if(strstr(p, "&"))
            {
                sscanf(p, "%[^&]", filename);
            }
            else
                strcpy(filename, p);

            if(strlen(filename))
                urlpath->filename = strdup(filename);   
        }
        if(strstr(p, "&rtsp")||strstr(p, "&tcp"))
        {
            urlpath->mode = RTSP_TRANSPORT_RTP_TCP;
        }
        else if(strstr(p, "&multcast"))
        {
            urlpath->mode = RTSP_TRANSPORT_RTP_MULTCAST;
        }
        else if(strstr(p, "&tls"))
        {
            urlpath->mode = RTSP_TRANSPORT_RTP_TLS;
        }
        else
        {
            urlpath->mode = RTSP_TRANSPORT_RTP_UDP;
        }
    }    
}

void rtsp_url_stream_path(const char *url, os_url_t *urlpath)
{
    memset(urlpath, 0, sizeof(os_url_t));
    if(os_url_split(url, urlpath) == OK)
    {
        char fullpath[1024];
        if(urlpath->filename)
        {
            memset(fullpath, 0, sizeof(fullpath));
            strcpy(fullpath, urlpath->filename);
            free(urlpath->filename);
            urlpath->filename = NULL;
            rtsp_url_stream_path_split(fullpath, urlpath);
        }
    }
}
#endif
#if defined(RTSP_DEBUG_ENABLE)
int rtsp_urlpath_test(char *url)
{
    os_url_t urlpath;
    memset(&urlpath, 0, sizeof(os_url_t));
    rtsp_url_stream_path(url,  &urlpath);

    fprintf(stdout, "===========================================\r\n");
    fprintf(stdout, "===============username   :%s\r\n", urlpath.user);
    fprintf(stdout, "===============password   :%s\r\n", urlpath.pass);
    fprintf(stdout, "===============hostname   :%s\r\n", urlpath.host);
    fprintf(stdout, "===============port       :%d\r\n", urlpath.port);
    fprintf(stdout, "===============channel    :%d\r\n", urlpath.channel);
    fprintf(stdout, "===============level      :%d\r\n", urlpath.level);
    fprintf(stdout, "===============path       :%s\r\n", urlpath.path);
    fprintf(stdout, "===============url        :%s\r\n", urlpath.url);
    fprintf(stdout, "===============mode       :%d\r\n", urlpath.mode);
    fprintf(stdout, "===========================================\r\n");
    fflush(stdout);
    return OK;
}
#endif

int rtsp_authenticate_option(const char *auth, const char *username, const char *password)
{
    char buftmp[1024];
    int length = 0;
    if(auth == NULL)
    {
        return -1;
    }
    else
    {
        if(strstr(auth, "Basic"))
        {
            char *p = strstr(auth, "Basic");
            p += strlen("Basic ");
            if(p && strlen(p))
            {
                memset(buftmp, 0, sizeof(buftmp));
                length = os_base64_decode(buftmp, p, sizeof(buftmp));
                if(length > 2 && strstr(buftmp, ":"))
                {
                    char gusername[128];
                    char gpassword[128];
                    memset(gusername, 0, sizeof(gusername));
                    memset(gpassword, 0, sizeof(gpassword));
                    sscanf(buftmp, "%s:%s", gusername, gpassword);
                    if(strcmp(username, gusername)==0 && strcmp(password, gpassword) == 0)
                    {
                        return 1;
                    }
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
    }
    return -1;
}
