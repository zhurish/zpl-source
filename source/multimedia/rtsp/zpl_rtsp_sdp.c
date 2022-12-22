/*
 * Copyright © 2001-2013 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "zpl_rtsp.h"
#include "zpl_rtsp_sdp.h"



#define SPACE_CHARS "\t\r\n"

typedef struct sdp_method_tbl_s {
    const char    *name;
    rtsp_method     method;
    int    (*get_value)(const char*, struct sdp_session *, const char *);
}sdp_method_tbl_t;


static int sdp_method_get_value(const char*, struct sdp_session *, const char*);

static sdp_method_tbl_t _sdp_method_cb[] = {
    //Header               type   support   methods
    {"OPTIONS",     RTSP_METHOD_OPTIONS,        sdp_method_get_value},
    {"DESCRIBE",    RTSP_METHOD_DESCRIBE,       sdp_method_get_value},
    {"SETUP",       RTSP_METHOD_SETUP,          sdp_method_get_value},
    {"TEARDOWN",    RTSP_METHOD_TEARDOWN,       sdp_method_get_value},
    {"PLAY",        RTSP_METHOD_PLAY,           sdp_method_get_value},
    {"PAUSE",       RTSP_METHOD_PAUSE,          sdp_method_get_value},
    {"SCALE",       RTSP_METHOD_SCALE,          sdp_method_get_value},
    {"GET_PARAMETER", RTSP_METHOD_GET_PARAMETER, sdp_method_get_value},
    {"SET_PARAMETER", RTSP_METHOD_SET_PARAMETER, sdp_method_get_value},
    {"ANNOUNCE", RTSP_METHOD_SET_PARAMETER, sdp_method_get_value},
    {"RECORD", RTSP_METHOD_SET_PARAMETER, sdp_method_get_value},
    {"REDIRECT", RTSP_METHOD_SET_PARAMETER, sdp_method_get_value},
};

typedef struct sdp_code_tbl_s {
    int     code;
    const char    *name;
}sdp_code_tbl_t;

static sdp_code_tbl_t rtsp_code_tbl[] = {

    {RTSP_STATE_CODE_100,  "Continue"},
    {RTSP_STATE_CODE_200,  "OK"},
    {RTSP_STATE_CODE_201,  "Created"},
    {RTSP_STATE_CODE_250,  "Low on Storage Space"},
    {RTSP_STATE_CODE_300,  "Multiple Choices"},
    {RTSP_STATE_CODE_301,  "Moved Permanently"},
    {RTSP_STATE_CODE_302,  "Moved Temporarily"},
    {RTSP_STATE_CODE_303,  "See Other"},
    {RTSP_STATE_CODE_304,  "Not Modified"},
    {RTSP_STATE_CODE_305,  "Use Proxy"},
    {RTSP_STATE_CODE_400,  "Bad Request"},
    {RTSP_STATE_CODE_401,  "Unauthorized"},
    {RTSP_STATE_CODE_402,  "Payment Required"},
    {RTSP_STATE_CODE_403,  "Forbidden"},
    {RTSP_STATE_CODE_404,  "Not Found"},
    {RTSP_STATE_CODE_405,  "Method Not Allowed"},
    {RTSP_STATE_CODE_406,  "Not Acceptable"},
    {RTSP_STATE_CODE_407,  "Proxy Authentication Required"},
    {RTSP_STATE_CODE_408,  "Request Time-out"},
    {RTSP_STATE_CODE_410,  "Gone"},
    {RTSP_STATE_CODE_411,  "Length Required"},
    {RTSP_STATE_CODE_412,  "Precondition Failed"},
    {RTSP_STATE_CODE_413,  "Request Entity Too Large"},
    {RTSP_STATE_CODE_414,  "Request-URI Too Large"},
    {RTSP_STATE_CODE_415,  "Unsupported Media Type"},
    {RTSP_STATE_CODE_451,  "Parameter Not Understood"},
    {RTSP_STATE_CODE_452,  "Conference Not Found"},
    {RTSP_STATE_CODE_453,  "Not Enough Bandwidth"},
    {RTSP_STATE_CODE_454,  "Session Not Found"},
    {RTSP_STATE_CODE_455,  "Method Not Valid in This State"},
    {RTSP_STATE_CODE_456,  "Header Field Not Valid for Resource"},
    {RTSP_STATE_CODE_457,  "Invalid Range"},
    {RTSP_STATE_CODE_458,  "Parameter Is Read-Only"},
    {RTSP_STATE_CODE_459,  "Aggregate operation not allowed"},
    {RTSP_STATE_CODE_460,  "Only aggregate operation allowed"},
    {RTSP_STATE_CODE_461,  "Unsupported transport"},
    {RTSP_STATE_CODE_462,  "Destination unreachable"},
    {RTSP_STATE_CODE_500,  "Internal Server Error"},
    {RTSP_STATE_CODE_501,  "Not Implemented"},
    {RTSP_STATE_CODE_502,  "Bad Gateway"},
    {RTSP_STATE_CODE_503,  "Service Unavailable"},
    {RTSP_STATE_CODE_504,  "Gateway Time-out"},
    {RTSP_STATE_CODE_505,  "RTSP Version not supported"},
    {RTSP_STATE_CODE_551,  "Option not supported"},
};


const char* sdp_code_get(int code)
{
    unsigned int i = 0;
    for(i = 0; i < (sizeof(rtsp_code_tbl)/sizeof(rtsp_code_tbl[0])); i++)
    {
        if(rtsp_code_tbl[i].name && code == rtsp_code_tbl[i].code)
            return rtsp_code_tbl[i].name;
    }
    return NULL;
}

const char* sdp_method_get(rtsp_method method)
{
    unsigned int i = 0;
    for(i = 0; i < (sizeof(_sdp_method_cb)/sizeof(_sdp_method_cb[0])); i++)
    {
        if(_sdp_method_cb[i].name && method == _sdp_method_cb[i].method)
            return _sdp_method_cb[i].name;
    }
    return NULL;
}







void sdp_skip_space(struct sdp_session* sdp)
{
    char c = sdp->misc.sdp_data[sdp->misc.sdp_offset];
    sdp->misc.sdp_start = &sdp->misc.sdp_data[sdp->misc.sdp_offset];
    while(' ' == c)
    {
        c = sdp->misc.sdp_data[++sdp->misc.sdp_offset];
        sdp->misc.sdp_start++;
    }
}

int sdp_token_word(struct sdp_session* sdp, const char* escape)
{
    int n = sdp->misc.sdp_offset;
    n = strcspn(sdp->misc.sdp_data + sdp->misc.sdp_offset, escape);
    sdp->misc.sdp_offset += n;
    sdp->misc.sdp_start = &sdp->misc.sdp_data[sdp->misc.sdp_offset-n];
    return n;
}

int sdp_token_crlf(struct sdp_session* sdp)
{
    char c = sdp->misc.sdp_data[sdp->misc.sdp_offset];
    sdp->misc.sdp_start = &sdp->misc.sdp_data[sdp->misc.sdp_offset];
    while('\t' == c || '\r' == c || '\n' == c )
    {
        c = sdp->misc.sdp_data[++sdp->misc.sdp_offset];
        sdp->misc.sdp_start++;
        if(sdp->misc.sdp_offset >= sdp->misc.sdp_len)
            break;
    }
    // sdp end line
    if('\0' == sdp->misc.sdp_data[sdp->misc.sdp_offset])
        return -1;
    return 0;
}

char* sdp_get_string(const char *src, const char*brk)
{
    int len = 0,offset = 0;
    char *p = (char*)src;
    char c = *p;
    while(' ' == c || '\t' == c || '\r' == c || '\n' == c)
    {
        c = *p;
        p++;
        offset++;
    }
    p = strstr(p, brk);
    if(p)
    {
        p += len = strlen(brk);
        if(p)
        {
            fprintf(stdout, "  sdp_get_string:%s=%s(%s)\r\n", brk, p, src+offset+len);
            return (char*)(src+offset+len);
        }
    }
    return NULL;
}

int sdp_get_string_offset(const char *src, const char*brk)
{
    int len = 0,offset = 0;
    char *p = (char*)src;
    char c = *p;
    while(' ' == c || '\t' == c || '\r' == c || '\n' == c)
    {
        c = *p;
        p++;
        offset++;
    }
    p = strstr(p, brk);
    if(p)
    {
        p += len = strlen(brk);
        if(p)
        {
            return (int)(offset+len);
        }
    }
    return (int)(offset+len);
}

char * sdp_scopy(const char *src, int len)
{
    char *dst = malloc(len + 1);
    if(dst)
    {
        memset(dst, '\0', sizeof(len+1));
        memcpy(dst, src, len);
        return dst;
    }
    return NULL;
}

int sdp_get_intcode(const char *src, const char*brk)
{
    char *p = (char*)src;
    p = strstr(src, brk);
    p += strlen(brk);
    if(p)
        return atoi(p);
    return -1;
}

char* sdp_get_datetime(time_t t)
{
    //Www Mmm dd hh:mm:ss yyyy ctime
    static char data[128];
    struct tm stm;
    int len = 0;
#if defined(_WIN32)
    struct tm *ptm = &stm;
    ptm = localtime(&t);
    if(ptm)
    {
        memset(data, 0, sizeof(data));
        len = strftime(data, sizeof(data), "%d %b %Y %H:%M:%S %Z", ptm);
    }
#else
    localtime_r(&t, &stm);
    memset(data, 0, sizeof(data));
    len = strftime(data, sizeof(data), "%d %b %Y %H:%M:%S %Z", &stm);
#endif
    if(len)
        return data;
    return NULL;
}





static int sdp_method_get_value(const char*name, struct sdp_session *session, const char *cname)
{
    if(name)
    {
        if(strstr(name, "OPTIONS"))
        {
            session->method = RTSP_METHOD_OPTIONS;
        }
        else if(strstr(name, "DESCRIBE"))
        {
            session->method = RTSP_METHOD_DESCRIBE;
        }
        else if(strstr(name, "SETUP"))
        {
            session->method = RTSP_METHOD_SETUP;
        }
        else if(strstr(name, "TEARDOWN"))
        {
            session->method = RTSP_METHOD_TEARDOWN;
        }
        else if(strstr(name, "PLAY"))
        {
            session->method = RTSP_METHOD_PLAY;
        }
        else if(strstr(name, "PAUSE"))
        {
            session->method = RTSP_METHOD_PAUSE;
        }
        else if(strstr(name, "SCALE"))
        {
            session->method = RTSP_METHOD_SCALE;
        }
        else if(strstr(name, "GET_PARAMETER"))
        {
            session->method = RTSP_METHOD_GET_PARAMETER;
        }
        else if(strstr(name, "SET_PARAMETER"))
        {
            session->method = RTSP_METHOD_SET_PARAMETER;
        }
    }
    return 0;
}

static int _sdp_method_parse(char *src, uint32_t len, struct sdp_session *session)
{
    unsigned int i = 0;
    char urltmp[512];
    char version[128];
    char tmp[128];

    for(i = 0; i < (sizeof(_sdp_method_cb)/sizeof(_sdp_method_cb[0])); i++)
    {
        if(_sdp_method_cb[i].name && _sdp_method_cb[i].get_value && strstr(src, _sdp_method_cb[i].name))
            (_sdp_method_cb[i].get_value)(src, session, _sdp_method_cb[i].name);
    }
    if(session->method)
    {
        memset(urltmp, 0, sizeof(urltmp));
        memset(version, 0, sizeof(version));
        memset(tmp, 0, sizeof(tmp));
        sscanf(src, "%s %s %s", tmp, urltmp, version);
        if(session->misc.url == NULL)
        session->misc.url = strdup(urltmp);
        if(session->misc.version == NULL)
        session->misc.version = strdup(version);

        fprintf(stdout, "=========%s==%s==%s=====\r\n", tmp, urltmp, version);
        return 1;
    }
    return 0;
}

static int _sdp_method_check(const char*src, uint32_t len)
{
    unsigned int i = 0;
    for(i = 0; i < (sizeof(_sdp_method_cb)/sizeof(_sdp_method_cb[0])); i++)
    {
        if(_sdp_method_cb[i].name && strstr(src, _sdp_method_cb[i].name) && strstr(src, "rtsp://"))
            return 1;
    }
    return 0;
}

static int _sdp_check(const char*src, uint32_t len)
{
    if(strstr(src, RTSP_HDR_VER) && !strstr(src, "rtsp://"))
    {
        return 1;
    }
    return 0;
}

static int _sdp_code_parse(const char*src, uint32_t len, int *rescode, char *restring)
{
    if(strstr(src, "RTSP/1.0"))
    {
        char tmp[128];
        memset(tmp, 0, sizeof(tmp));
        sscanf(src, "%s %d %s", tmp, rescode, restring);
        //fprintf(stdout, "=====client_sdp_code_parse====%s==%d==%s=====\r\n", tmp, *rescode, restring);
        return 0;
    }
    return -1;
}


int sdp_text_init(struct sdp_session *session, const char *raw, uint32_t len)
{
    memset(session, 0, sizeof(struct sdp_session));
    session->media_start = false;
    session->header.CSeq = -1;
    session->misc.sdp_data = session->misc.sdp_start = (char *)raw;
    session->misc.sdp_offset = 0;
    session->misc.sdp_len = len;
    return 0;
}


static int _sdp_text_prase(bool srv, struct sdp_session *session)
{
    while(session->misc.sdp_offset < session->misc.sdp_len)
	{
		sdp_skip_space(session);
        int n = sdp_token_crlf(session);
        if(n == 0)
        {
            n = sdp_token_word(session, SPACE_CHARS);

            char buf[1024];

            memset(buf, '\0', sizeof(buf));
            if(n)
            {
                memcpy(buf, session->misc.sdp_start, n);

                if(srv && _sdp_method_check(buf, n))
                {
                    /* 解析客户端发出的SDP */
                    _sdp_method_parse(buf, n, session);
                }
                else if(!srv && _sdp_check(buf, n))
                {
                    /* 解析服务端返回的SDP */
                    _sdp_code_parse(buf, n, &session->code, session->misc.result_string);
                }
                else if(sdp_header_check(buf, n))
                    sdp_header_parse(buf, n, session);

                else if(sdp_attr_check(buf, n))
                {
                    sdp_attr_parse(buf, n, session, session->media_start);
                }
            }
        }
    }
    //fprintf(stdout, "==========gggggggggggggg============media_count=%d\r\n", session->media_count);
    //sdp_text_debug(session);
    return 0;
}

int sdp_text_prase(bool srv, struct sdp_session *session)
{
    return _sdp_text_prase(srv, session);
}

int sdp_text_free(struct sdp_session *session)
{
    sdp_header_free(session);
    return sdp_attr_free(session);
}

int sdp_text_debug(struct sdp_session *session)
{
    fprintf(stdout, "=========%s -> start=========\r\n", __func__);
    sdp_header_debug(session);
    sdp_attr_debug(session);
    fprintf(stdout, "=========%s -> end=========\r\n", __func__);
    return 0;
}


int sdp_build_sessionID(struct sdp_session *sdp, uint8_t *src, uint32_t sessionid)
{
    int offset = 0;
    if(sessionid > 0)
        offset += sprintf((char*)(src + offset), "Session: %08u\r\n", sessionid);
    return offset;
}
/* server */
int sdp_build_respone_header(uint8_t *src, const char *srvname, char *base,
                             int code, int CSeq, uint32_t sessionid)
{
    int offset = 0;
    offset += sprintf((char*)(src + 0), "%s %d %s\r\nServer: %s\r\nCSeq: %d\r\n", RTSP_HDR_VER,
                      code, sdp_code_get(code), srvname, CSeq);
    offset += sprintf((char*)(src + offset), "Date: %s\r\n", sdp_get_datetime(time(NULL)));
    if(sessionid > 0)
        offset += sprintf((char*)(src + offset), "Session: %08u\r\n", sessionid);
    if(base && strlen(base))
        offset += sprintf((char*)(src + offset), "Content-Base: %s\r\n", base);
    return offset;
}

/* client */
int sdp_build_request_header(struct sdp_session *sdp, uint8_t *src, const char *clientname,
                             rtsp_method method, const char *url, int cseq)
{
    //char *testreq = "SETUP rtsp://192.168.20.136:5000/xxx666/trackID=0 RTSP/1.0\r\n"
    //"CSeq: 3\r\n"
    if(RTSP_METHOD_CHECK(method))
    {
        int offset = sprintf((char*)(src + 0), "%s %s %s\r\nCSeq: %d\r\n", sdp_method_get(method),
                       url?url:"*", RTSP_HDR_VER, cseq);
        offset += sprintf((char*)(src + offset), "Date: %s\r\n", sdp_get_datetime(time(NULL)));
        offset += sprintf((char*)(src + offset), "User-Agent: %s\r\n", clientname);
        return offset;
    }
    return 0;
}

int sdp_build_respone_option(uint8_t *src, const char *srvname, int code, int CSeq)
{
    int offset = sdp_build_respone_header(src, srvname, NULL, code, CSeq, 0);
    offset += sprintf((char*)(src + offset), "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n");
    return offset;
}







char *testreq = "SETUP rtsp://192.168.20.136:5000/xxx666/trackID=0 RTSP/1.0\r\n"
"CSeq: 3\r\n"
"Transport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n"
"User-Agent: VLC media player (LIVE555 Streaming Media v2005.11.10)\r\n";


char *testrep = "RTSP/1.0 200 OK\r\n"
"CSeq: 1\r\n"
"Server: Tiny-Rain(A RTSP/SIP Media Server, V0.1, Compiled at 17:00:15 Aug  6 2019)\r\n"
"Content-Type: application/sdp\r\n"
"Content-Base: rtsp://192.168.1.19:7554/media=0/channel=0&level=0/\r\n"
"Date: Mon, 09 Aug 2021 19:18:15 GMT\r\n"
"Content-Length: 406\r\n"
"\r\n"
"v=0\r\n"
"o=- 12345678910111213 1 IN IP4 0.0.0.0\r\n"
"s=MEDIA FROM IPCamera\r\n"
"e=NONE\r\n"
"c=IN IP4 0.0.0.0\r\n"
"t=0 0\r\n"
"a=control:*\r\n"
"a=range:npt=0.0-\r\n"
"m=video 0 RTP/AVP 96\r\n"
"b=AS:4096\r\n"
"a=rtpmap:96 H264/90000\r\n"
"a=control:trackID=0\r\n"
"a=fmtp:96 packetization-mode=1;profile-level-id=64002A;sprop-parameter-sets=Z2QAKqwsaoHgCJ+WbgoCCgQ=,aO4xshs=\r\n"
"m=audio 0 RTP/AVP 8\r\n"
"b=AS:0\r\n"
"a=rtpmap:8 PCMA/8000/0\r\n"
"a=ptime:0\r\n"
"a=control:trackID=1\r\n\r\n";



int rtsp_sdp_hdr_prase_test()
{
    struct sdp_session ssssession;
    sdp_text_init(&ssssession,testreq,  strlen(testreq));

    fprintf(stdout, "request:\r\n");
    sdp_text_prase(true, &ssssession);
    fflush(stdout);
    sdp_text_free(&ssssession);

    sdp_text_init(&ssssession,testrep, strlen(testrep));
    fprintf(stdout, "response:\r\n");
    sdp_text_prase(false, &ssssession);
    fflush(stdout);
    sdp_text_free(&ssssession);
    return 0;
}
