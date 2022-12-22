/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "zpl_rtsp.h"
#include "zpl_rtsp_sdp.h"


typedef struct sdp_header_tbl_s {
    const char    *name;
    int     len;
    int    (*get_value)(char*, uint32_t, sdp_header_t *, const char *);
}sdp_header_tbl_t;


static int sdp_header_get_value(char*, uint32_t, sdp_header_t *, const char*);

static sdp_header_tbl_t _sdp_hdr_cb[] = {
    //Header               type   support   methods
    {"Accept:", strlen("Accept:"), sdp_header_get_value},
    {"Accept-Encoding:", strlen("Accept-Encoding:"), sdp_header_get_value},
    {"Accept-Language:", strlen("Accept-Language:"), sdp_header_get_value},
    {"Allow:", strlen("Allow:"), sdp_header_get_value},
    {"Authorization:", strlen("Authorization:"), sdp_header_get_value},
    {"Bandwidth:", strlen("Bandwidth:"), sdp_header_get_value},
    {"Blocksize:", strlen("Blocksize:"), sdp_header_get_value},
    {"Cache-Control:", strlen("Cache-Control:"), sdp_header_get_value},
    {"Conference:", strlen("Conference:"), sdp_header_get_value},
    {"Connection:", strlen("Connection:"), sdp_header_get_value},
    {"Content-Base:", strlen("Content-Base:"), sdp_header_get_value},
    {"Content-Encoding:", strlen("Content-Encoding:"), sdp_header_get_value},
    {"Content-Language:", strlen("Content-Language:"), sdp_header_get_value},
    {"Content-Length:", strlen("Content-Length:"), sdp_header_get_value},
    {"Content-Location:", strlen("Content-Location:"), sdp_header_get_value},
    {"Content-Type:", strlen("Content-Type:"), sdp_header_get_value},
    {"CSeq:", strlen("CSeq:"), sdp_header_get_value},
    {"Date:", strlen("Date:"), sdp_header_get_value},
    {"Expires:", strlen("Expires:"), sdp_header_get_value},
    {"From:", strlen("From:"), sdp_header_get_value},
    {"If-Modified-Since:", strlen("If-Modified-Since:"), sdp_header_get_value},
    {"Last-Modified:", strlen("Last-Modified:"), sdp_header_get_value},
    {"Proxy-Authenticate:", strlen("Proxy-Authenticate:"), sdp_header_get_value},
    {"Proxy-Require:", strlen("Proxy-Require:"), sdp_header_get_value},
    {"Public:", strlen("Public:"), sdp_header_get_value},
    {"Range:", strlen("Range:"), sdp_header_get_value},
    {"Referer:", strlen("Referer:"), sdp_header_get_value},
    {"Require:", strlen("Require:"), sdp_header_get_value},
    {"Retry-After:", strlen("Retry-After:"), sdp_header_get_value},
    {"RTP-Info:", strlen("RTP-Info:"), sdp_header_get_value},
    {"Scale:", strlen("Scale:"), sdp_header_get_value},
    {"Session:", strlen("Session:"), sdp_header_get_value},
    {"Server:", strlen("Server:"), sdp_header_get_value},
    {"Speed:", strlen("Speed:"), sdp_header_get_value},
    {"Transport:", strlen("Transport:"), sdp_header_get_value},
    {"Unsupported:", strlen("Unsupported:"), sdp_header_get_value},
    {"User-Agent:", strlen("User-Agent:"), sdp_header_get_value},
    {"Via:", strlen("Via:"), sdp_header_get_value},
    {"WWW-Authenticate:", strlen("WWW-Authenticate:"), sdp_header_get_value},
    {NULL, 0, NULL},
};



static int sdp_header_get_value(char*src, uint32_t len, sdp_header_t *hdr, const char *name)
{
    if(src)
    {
        if(strstr(src, "Accept:"))
        {
            if(hdr->Accept == NULL)
                hdr->Accept = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Accept-Encoding:"))
        {
            if(hdr->Accept_Encoding == NULL)
                hdr->Accept_Encoding = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Accept-Language:"))
        {
            if(hdr->Accept_Language == NULL)
                hdr->Accept_Language = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Allow:"))
        {
            if(hdr->Allow == NULL)
                hdr->Allow = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Authorization:"))
        {
            if(hdr->Authorization == NULL)
                hdr->Authorization = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Bandwidth:"))
        {
            if(hdr->Bandwidth == NULL)
                hdr->Bandwidth = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Blocksize:"))
        {
            if(hdr->Blocksize == NULL)
                hdr->Blocksize = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Cache-Control:"))
        {
            if(hdr->Cache_Control == NULL)
                hdr->Cache_Control = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Conference:"))
        {
            if(hdr->Conference == NULL)
                hdr->Conference = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Connection:"))
        {
            if(hdr->Connection == NULL)
                hdr->Connection = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Content-Base:"))
        {
            if(hdr->Content_Base == NULL)
                hdr->Content_Base = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Content-Encoding:"))
        {
            if(hdr->Content_Encoding == NULL)
                hdr->Content_Encoding = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Content-Language:"))
        {
            if(hdr->Content_Language == NULL)
                hdr->Content_Language = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Content-Length:"))
        {
            if(hdr->Content_Length == NULL)
                hdr->Content_Length = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Content-Location:"))
        {
            if(hdr->Content_Location == NULL)
                hdr->Content_Location = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Content-Type:"))
        {
            if(hdr->Content_Type == NULL)
                hdr->Content_Type = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "CSeq:"))
        {
            sscanf(src, "CSeq: %d", &hdr->CSeq);
            //hdr->CSeq = strdup(src + sdp_get_string_offset(src, name));
            //if(hdr->CSeq)
            //    fprintf(stdout, "CSeq:%s\r\n", hdr->CSeq);
        }
        else if(strstr(src, "Date:"))
        {
            if(hdr->Date == NULL)
                hdr->Date = strdup(src + sdp_get_string_offset(src, name));
            if(hdr->Date)
                fprintf(stdout, "======Date:%s\r\n", hdr->Date);
        }
        else if(strstr(src, "Expires:"))
        {
            if(hdr->Expires == NULL)
                hdr->Expires = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "From:"))
        {
            if(hdr->From == NULL)
                hdr->From = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "If-Modified-Since:"))
        {
            if(hdr->If_Modified_Since == NULL)
                hdr->If_Modified_Since = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Last-Modified:"))
        {
            if(hdr->Last_Modified == NULL)
                hdr->Last_Modified = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Proxy-Authenticate:"))
        {
            if(hdr->Proxy_Authenticate == NULL)
                hdr->Proxy_Authenticate = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Proxy-Require:"))
        {
            if(hdr->Proxy_Require == NULL)
                hdr->Proxy_Require = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Public:"))
        {
            if(hdr->Public == NULL)
                hdr->Public = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Range:"))
        {
            if(hdr->Range == NULL)
                hdr->Range = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Referer:"))
        {
            if(hdr->Referer == NULL)
                hdr->Referer = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Require:"))
        {
            if(hdr->Require == NULL)
                hdr->Require = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Retry-After:"))
        {
            if(hdr->Retry_After == NULL)
                hdr->Retry_After = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "RTP-Info:"))
        {
            if(hdr->RTP_Info == NULL)
                hdr->RTP_Info = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Scale:"))
        {
            if(hdr->Scale == NULL)
                hdr->Scale = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Session:"))
        {
            if(hdr->Session == NULL)
                hdr->Session = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Server:"))
        {
            if(hdr->Server == NULL)
                hdr->Server = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Speed:"))
        {
            if(hdr->Speed == NULL)
                hdr->Speed = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "Transport:"))
        {
            if(hdr->Transport == NULL)
                hdr->Transport = strdup(src + sdp_get_string_offset(src, name));
            //if(hdr->Transport)
            //    fprintf(stdout, "Transport:%s\r\n", hdr->Transport);
        }
        else if(strstr(src, "Unsupported:"))
        {
            if(hdr->Unsupported == NULL)
                hdr->Unsupported = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "User-Agent:"))
        {
            if(hdr->User_Agent == NULL)
                hdr->User_Agent = strdup(src + sdp_get_string_offset(src, name));
            //if(hdr->User_Agent)
            //    fprintf(stdout, "User_Agent:%s\r\n", hdr->User_Agent);
        }
        else if(strstr(src, "Via:"))
        {
            if(hdr->Via == NULL)
                hdr->Via = strdup(src + sdp_get_string_offset(src, name));
        }
        else if(strstr(src, "WWW-Authenticate:"))
        {
            if(hdr->WWW_Authenticate == NULL)
                hdr->WWW_Authenticate = strdup(src + sdp_get_string_offset(src, name));
        }
    }
    return 0;
}

int sdp_header_parse(char*src, uint32_t len, struct sdp_session *session)
{
    unsigned int i = 0;
    //fprintf(stdout, "=========%s=========\r\n", __func__);
    for(i = 0; i < (sizeof(_sdp_hdr_cb)/sizeof(_sdp_hdr_cb[0])); i++)
    {
        if(_sdp_hdr_cb[i].name && _sdp_hdr_cb[i].get_value && strstr(src, _sdp_hdr_cb[i].name))
            (_sdp_hdr_cb[i].get_value)(src, len, &session->header, _sdp_hdr_cb[i].name);
    }
    return 0;
}

int sdp_header_check(char*src, uint32_t len)
{
    unsigned int i = 0;
    for(i = 0; i < (sizeof(_sdp_hdr_cb)/sizeof(_sdp_hdr_cb[0])); i++)
    {
        if(_sdp_hdr_cb[i].name && strstr(src, _sdp_hdr_cb[i].name))
            return 1;
    }
    return 0;
}

int sdp_header_init(struct sdp_session *session)
{
    //fprintf(stdout, "=========%s=========\r\n", __func__);
    memset(session, 0, sizeof(struct sdp_session));
    return 0;
}

int sdp_header_free(struct sdp_session *session)
{
    //fprintf(stdout, "=========%s=========\r\n", __func__);
    if(session->header.Accept)
    {
        free(session->header.Accept);//               R      opt.      entity
        session->header.Accept = NULL;
    }
    if(session->header.Accept_Encoding)
    {
        free(session->header.Accept_Encoding);//               R      opt.      entity
        session->header.Accept_Encoding = NULL;
    }
    if(session->header.Accept_Language)
    {
        free(session->header.Accept_Language);//               R      opt.      entity
        session->header.Accept_Language = NULL;
    }
    if(session->header.Allow)
    {
        free(session->header.Allow);//               R      opt.      entity
        session->header.Allow = NULL;
    }
    if(session->header.Authorization)
    {
        free(session->header.Authorization);//               R      opt.      entity
        session->header.Authorization = NULL;
    }
    if(session->header.Bandwidth)
    {
        free(session->header.Bandwidth);//               R      opt.      entity
        session->header.Bandwidth = NULL;
    }
    if(session->header.Blocksize)
    {
        free(session->header.Blocksize);//               R      opt.      entity
        session->header.Blocksize = NULL;
    }
    if(session->header.Cache_Control)
    {
        free(session->header.Cache_Control);//               R      opt.      entity
        session->header.Cache_Control = NULL;
    }
    if(session->header.Conference)
    {
        free(session->header.Conference);//               R      opt.      entity
        session->header.Conference = NULL;
    }
    if(session->header.Connection)
    {
        free(session->header.Connection);//               R      opt.      entity
        session->header.Connection = NULL;
    }
    if(session->header.Content_Base)
    {
        free(session->header.Content_Base);//               R      opt.      entity
        session->header.Content_Base = NULL;
    }
    if(session->header.Content_Encoding)
    {
        free(session->header.Content_Encoding);//               R      opt.      entity
        session->header.Content_Encoding = NULL;
    }
    if(session->header.Content_Language)
    {
        free(session->header.Content_Language);//               R      opt.      entity
        session->header.Content_Language = NULL;
    }
    if(session->header.Content_Length)
    {
        free(session->header.Content_Length);//               R      opt.      entity
        session->header.Content_Length = NULL;
    }
    if(session->header.Content_Location)
    {
        free(session->header.Content_Location);//               R      opt.      entity
        session->header.Content_Location = NULL;
    }
    if(session->header.Content_Type)
    {
        free(session->header.Content_Type);//               R      opt.      entity
        session->header.Content_Type = NULL;
    }
    /*if(session->header.CSeq)
    {
        free(session->header.CSeq);//               R      opt.      entity
        session->header.CSeq = NULL;
    }*/
    if(session->header.Date)
    {
        free(session->header.Date);//               R      opt.      entity
        session->header.Date = NULL;
    }
    if(session->header.Expires)
    {
        free(session->header.Expires);//               R      opt.      entity
        session->header.Expires = NULL;
    }
    if(session->header.From)
    {
        free(session->header.From);//               R      opt.      entity
        session->header.From = NULL;
    }
    if(session->header.If_Modified_Since)
    {
        free(session->header.If_Modified_Since);//               R      opt.      entity
        session->header.If_Modified_Since = NULL;
    }
    if(session->header.Last_Modified)
    {
        free(session->header.Last_Modified);//               R      opt.      entity
        session->header.Last_Modified = NULL;
    }
    if(session->header.Proxy_Authenticate)
    {
        free(session->header.Proxy_Authenticate);//               R      opt.      entity
        session->header.Proxy_Authenticate = NULL;
    }
    if(session->header.Proxy_Require)
    {
        free(session->header.Proxy_Require);//               R      opt.      entity
        session->header.Proxy_Require = NULL;
    }
    if(session->header.Public)
    {
        free(session->header.Public);//               R      opt.      entity
        session->header.Public = NULL;
    }
    if(session->header.Range)
    {
        free(session->header.Range);//               R      opt.      entity
        session->header.Range = NULL;
    }
    if(session->header.Referer)
    {
        free(session->header.Referer);//               R      opt.      entity
        session->header.Referer = NULL;
    }
    if(session->header.Require)
    {
        free(session->header.Require);//               R      opt.      entity
        session->header.Require = NULL;
    }
    if(session->header.Retry_After)
    {
        free(session->header.Retry_After);//               R      opt.      entity
        session->header.Retry_After = NULL;
    }
    if(session->header.RTP_Info)
    {
        free(session->header.RTP_Info);//               R      opt.      entity
        session->header.RTP_Info = NULL;
    }
    if(session->header.Scale)
    {
        free(session->header.Scale);//               R      opt.      entity
        session->header.Scale = NULL;
    }
    if(session->header.Session)
    {
        free(session->header.Session);//               R      opt.      entity
        session->header.Session = NULL;
    }
    if(session->header.Server)
    {
        free(session->header.Server);//               R      opt.      entity
        session->header.Server = NULL;
    }
    if(session->header.Speed)
    {
        free(session->header.Speed);//               R      opt.      entity
        session->header.Speed = NULL;
    }
    if(session->header.Transport)
    {
        free(session->header.Transport);//               R      opt.      entity
        session->header.Transport = NULL;
    }
    if(session->header.Unsupported)
    {
        free(session->header.Unsupported);//               R      opt.      entity
        session->header.Unsupported = NULL;
    }
    if(session->header.User_Agent)
    {
        free(session->header.User_Agent);//               R      opt.      entity
        session->header.User_Agent = NULL;
    }
    if(session->header.Via)
    {
        free(session->header.Via);//               R      opt.      entity
        session->header.Via = NULL;
    }
    if(session->header.WWW_Authenticate)
    {
        free(session->header.WWW_Authenticate);//               R      opt.      entity
        session->header.WWW_Authenticate = NULL;
    }
    return 0;
}

int sdp_header_debug(struct sdp_session *session)
{
    //fprintf(stdout, "=========%s=========\r\n", __func__);
    if(session->header.Accept)
    {
        fprintf(stdout, " Accept                   : %s\r\n", session->header.Accept);//               R      opt.      entity
    }
    if(session->header.Accept_Encoding)
    {
        fprintf(stdout, " Accept-Encoding          : %s\r\n", session->header.Accept_Encoding);//               R      opt.      entity
    }
    if(session->header.Accept_Language)
    {
        fprintf(stdout, " Accept-Language          : %s\r\n", session->header.Accept_Language);//               R      opt.      entity
    }
    if(session->header.Allow)
    {
        fprintf(stdout, " Allow                    : %s\r\n", session->header.Allow);//               R      opt.      entity
    }
    if(session->header.Authorization)
    {
        fprintf(stdout, " Authorization            : %s\r\n", session->header.Authorization);//               R      opt.      entity
    }
    if(session->header.Bandwidth)
    {
        fprintf(stdout, " Bandwidth                : %s\r\n", session->header.Bandwidth);//               R      opt.      entity
    }
    if(session->header.Blocksize)
    {
        fprintf(stdout, " Blocksize                : %s\r\n", session->header.Blocksize);//               R      opt.      entity
    }

    if(session->header.Cache_Control)
    {
        fprintf(stdout, " Cache-Control            : %s\r\n", session->header.Cache_Control);//               R      opt.      entity
    }
    if(session->header.Conference)
    {
        fprintf(stdout, " Conference               : %s\r\n", session->header.Conference);//               R      opt.      entity
    }
    if(session->header.Connection)
    {
        fprintf(stdout, " Connection               : %s\r\n", session->header.Connection);//               R      opt.      entity
    }
    if(session->header.Content_Base)
    {
        fprintf(stdout, " Content-Base             : %s\r\n", session->header.Content_Base);//               R      opt.      entity
    }
    if(session->header.Content_Encoding)
    {
        fprintf(stdout, " Content-Encoding         : %s\r\n", session->header.Content_Encoding);//               R      opt.      entity
    }
    if(session->header.Content_Language)
    {
        fprintf(stdout, " Content-Language         : %s\r\n", session->header.Content_Language);//               R      opt.      entity
    }
    if(session->header.Content_Length)
    {
        fprintf(stdout, " Content-Length           : %s\r\n", session->header.Content_Length);//               R      opt.      entity
    }
    if(session->header.Content_Location)
    {
        fprintf(stdout, " Content-Location         : %s\r\n", session->header.Content_Location);//               R      opt.      entity
    }
    if(session->header.Content_Type)
    {
        fprintf(stdout, " Content-Type             : %s\r\n", session->header.Content_Type);//               R      opt.      entity
    }
    if(session->header.CSeq > -1)
    {
        fprintf(stdout, " CSeq                     : %d\r\n", session->header.CSeq);//               R      opt.      entity
    }
    if(session->header.Date)
    {
        fprintf(stdout, " Date                     : %s\r\n", session->header.Date);//               R      opt.      entity
    }
    if(session->header.Expires)
    {
        fprintf(stdout, " Expires                  : %s\r\n", session->header.Expires);//               R      opt.      entity
    }
    if(session->header.From)
    {
        fprintf(stdout, " From                     : %s\r\n", session->header.From);//               R      opt.      entity
    }
    if(session->header.If_Modified_Since)
    {
        fprintf(stdout, " If-Modified-Since        : %s\r\n", session->header.If_Modified_Since);//               R      opt.      entity
    }
    if(session->header.Last_Modified)
    {
        fprintf(stdout, " Last Modified            : %s\r\n", session->header.Last_Modified);//               R      opt.      entity
    }
    if(session->header.Proxy_Authenticate)
    {
        fprintf(stdout, " Proxy-Authenticate       : %s\r\n", session->header.Proxy_Authenticate);//               R      opt.      entity
    }
    if(session->header.Proxy_Require)
    {
        fprintf(stdout, " Proxy-Require            : %s\r\n", session->header.Proxy_Require);//               R      opt.      entity
    }
    if(session->header.Public)
    {
        fprintf(stdout, " Public                   : %s\r\n", session->header.Public);//               R      opt.      entity
    }
    if(session->header.Range)
    {
        fprintf(stdout, " Range                    : %s\r\n", session->header.Range);//               R      opt.      entity
    }
    if(session->header.Referer)
    {
        fprintf(stdout, " Referer                  : %s\r\n", session->header.Referer);//               R      opt.      entity
    }
    if(session->header.Require)
    {
        fprintf(stdout, " Require                  : %s\r\n", session->header.Require);//               R      opt.      entity
    }
    if(session->header.Retry_After)
    {
        fprintf(stdout, " Retry-After              : %s\r\n", session->header.Retry_After);//               R      opt.      entity
    }
    if(session->header.RTP_Info)
    {
        fprintf(stdout, " RTP-Info                 : %s\r\n", session->header.RTP_Info);//               R      opt.      entity
    }
    if(session->header.Scale)
    {
        fprintf(stdout, " Scale                    : %s\r\n", session->header.Scale);//               R      opt.      entity
    }
    if(session->header.Session)
    {
        fprintf(stdout, " Session                  : %s\r\n", session->header.Session);//               R      opt.      entity
    }
    if(session->header.Server)
    {
        fprintf(stdout, " Server                   : %s\r\n", session->header.Server);//               R      opt.      entity
    }
    if(session->header.Speed)
    {
        fprintf(stdout, " Speed                    : %s\r\n", session->header.Speed);//               R      opt.      entity
    }
    if(session->header.Transport)
    {
        fprintf(stdout, " Transport                : %s\r\n", session->header.Transport);//               R      opt.      entity
    }
    if(session->header.Unsupported)
    {
        fprintf(stdout, " Unsupported              : %s\r\n", session->header.Unsupported);//               R      opt.      entity
    }
    if(session->header.User_Agent)
    {
        fprintf(stdout, " User-Agent               : %s\r\n", session->header.User_Agent);//               R      opt.      entity
    }
    if(session->header.Via)
    {
        fprintf(stdout, " Via                      : %s\r\n", session->header.Via);//               R      opt.      entity
    }
    if(session->header.WWW_Authenticate)
    {
        fprintf(stdout, " WWW Authenticate         : %s\r\n", session->header.WWW_Authenticate);//               R      opt.      entity
    }
    fflush(stdout);
    return 0;
}
