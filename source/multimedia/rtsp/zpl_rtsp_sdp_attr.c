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

typedef struct sdp_attr_tbl_s {
    char    *name;
    int     len;
    int    (*get_value)(const char*, uint32_t, struct sdp_session *, const char*);
}sdp_attr_tbl_t;


static int sdp_attr_get_value(const char*, uint32_t, struct sdp_session *, const char*);
static int sdp_media_attr_get_value(const char*src, uint32_t, struct sdp_session *hdr, const char*name);

static sdp_attr_tbl_t _sdp_session_cb[] = {
    {"v=",   2, sdp_attr_get_value},
    {"o=",   2, sdp_attr_get_value},
    {"s=",   2, sdp_attr_get_value},
    {"i=",   2, sdp_attr_get_value},
    {"u=",   2, sdp_attr_get_value},
    {"e=",   2, sdp_attr_get_value},
    {"p=",   2, sdp_attr_get_value},
    {"c=",   2, sdp_attr_get_value},
    {"z=",   2, sdp_attr_get_value},
    {"k=",   2, sdp_attr_get_value},
    {"a=",   2, sdp_attr_get_value},
    {"t=",   2, sdp_attr_get_value},
    {"r=",   2, sdp_attr_get_value},
    {"m=",   2, sdp_attr_get_value},
    {"b=",   2, sdp_attr_get_value},
    {NULL, 0, NULL},
};

static sdp_attr_tbl_t _sdp_media_cb[] = {
    {"m=",   2, sdp_media_attr_get_value},
    {"i=",   2, sdp_media_attr_get_value},
    {"c=",   2, sdp_media_attr_get_value},
    {"k=",   2, sdp_media_attr_get_value},
    {"a=",   2, sdp_media_attr_get_value},
    {"b=",   2, sdp_media_attr_get_value},
    {NULL, 0, NULL},
};
/*
二、SDP格式
SDP 信息是文本信息，UTF-8 编码采用 ISO 10646 字符设置。SDP 会话描述如下（标注*符号的表示可选字段）：

v= （协议版本）
o= （所有者/创建者和会话标识符）
s= （会话名称）
i=* （会话信息）
u=* （URI 描述）
e=* （Email 地址）
p=* （电话号码）
c=* （连接信息 ― 如果包含在所有媒体中，则不需要该字段）
b=* （带宽信息）
一个或更多时间描述（如下所示）：

z=* （时间区域调整）
k=* （加密密钥）
a=* （0个或多个会话属性线路）
0个或多个媒体描述（如下所示）
时间描述

t= （会话活动时间）
r=* （0或多次重复次数）
媒体描述

m= （媒体名称和传输地址）
i=* （媒体标题）
c=* （连接信息 — 如果包含在会话层则该字段可选）
b=* （带宽信息）
k=* （加密密钥）
a=* （0个或多个会话属性线路）

V=0     ;Version 给定了SDP协议的版本
o=<username><session id> <version> <network type> <address type> <address>； Origin ,给定了会话的发起者信息
s=<sessionname> ;给定了Session Name
i=<sessiondescription> ; Information 关于Session的一些信息
u=<URI> ; URI
e=<emailaddress>    ;Email
c=<networktype> <address type> <connection address> ;Connect Data包含连接数据
t=<start time><stop time> ;Time
a=<attribute>     ; Attribute
a=<attribute>:<value>
m=<media><port> <transport> <fmt list> ; MediaAnnouncements
*/


char * sdp_attr_find(struct sdp_attr *attr, uint32_t n, const char*name, uint32_t *step)
{
    uint32_t i = 0;
    if(step)
        i = *step;
    if(attr)
    {
        for(; i < n; i++)
        {
            if(attr[i].name && strstr(attr[i].name, name))
            {
                if(step)
                    *step = i;
                return attr[i].value ? attr[i].value:attr[i].name;
            }
        }
    }
    return NULL;
}

char * sdp_attr_find_value(struct sdp_attr *attr, uint32_t n, const char*name, uint32_t *step)
{
    uint32_t i = 0;
    if(step)
        i = *step;
    if(attr)
    {
        for(; i < n; i++)
        {
            if(attr[i].name && strstr(attr[i].name, name))
            {
                if(step)
                    *step = i;
                return attr[i].value ? attr[i].value:attr[i].name;
            }
        }
    }
    return NULL;
}

struct sdp_media * sdp_media_find(struct sdp_media *media, uint32_t n, const char *name, uint32_t *step)
{
    uint32_t i = 0;
    if(step)
        i = *step;
    if(media)
    {
        for(; i < n; i++)
        {
            RTSP_TRACE(" ================== sdp_media_find :%s<->%s\r\n", media[i].desc.media, name);
            if(media[i].desc.media && strstr(media[i].desc.media, name))
            {
                if(step)
                    *step = i;
                return &media[i];
            }
        }
    }
    return NULL;
}


static int sdp_attr_repeat_parse(char *s)
{
    int tmp[2];
    if(strstr(s,"h"))
        tmp[1] = 60*60;
    else if(strstr(s,"d"))
        tmp[1] = 60*60*24;
    else if(strstr(s,"m"))
        tmp[1] = 60;
    else if(strstr(s,"s"))
        tmp[1] = 1;
    tmp[0] = atoi(s);
    return tmp[1]*tmp[0];
}

static int sdp_attr_get_value(const char*src, uint32_t len, struct sdp_session *session, const char*name)
{
    if(src)
    {
        char net_type[64];	    /**< Network type ("IN")		*/
        char addr_type[64];	    /**< Address type ("IP4", "IP6")	*/
        char addr[64];	    /**< The address.			*/
        char tmp[256];
        if(strncmp(src, "o=", 2) == 0)
        {
            char user[64];	    /**< User 				*/
            memset(user, 0, sizeof(user));
            memset(net_type, 0, sizeof(net_type));
            memset(addr_type, 0, sizeof(addr_type));
            memset(addr, 0, sizeof(addr));
            sscanf(src+2, "%s %d %d %s %s %s", user, &session->origin.id, &session->origin.version, net_type, addr_type, addr);

            if(session->origin.user == NULL)
                session->origin.user = strdup(user);	    /**< User 				*/

            if(session->origin.net_type == NULL)
                session->origin.net_type = strdup(net_type);	    /**< Network type ("IN")		*/
            if(session->origin.addr_type == NULL)
                session->origin.addr_type = strdup(addr_type);	    /**< Address type ("IP4", "IP6")	*/
            if(session->origin.addr == NULL)
                session->origin.addr = strdup(addr);	    /**< The address.*/
            //session->media_start = false;
            return 0;
        }
        else if(strncmp(src, "s=", 2) == 0)
        {
            if(session->name == NULL)
                session->name = strdup(src+2);
            //session->media_start = false;
            return 0;
        }
        else if(strncmp(src, "c=", 2) == 0)
        {
            memset(net_type, 0, sizeof(net_type));
            memset(addr_type, 0, sizeof(addr_type));
            memset(addr, 0, sizeof(addr));
            sscanf(src+2, "%s %s %s", net_type, addr_type, addr);
            if(session->conn.net_type == NULL)
                session->conn.net_type = strdup(net_type);	    /**< Network type ("IN")		*/
            if(session->conn.addr_type == NULL)
                session->conn.addr_type = strdup(addr_type);	    /**< Address type ("IP4", "IP6")	*/
            if(session->conn.addr == NULL)
                session->conn.addr = strdup(addr);	    /**< The address.*/
            return 0;
        }
        else if(strncmp(src, "i=", 2) == 0)
        {
            if(session->information == NULL)
                session->information = strdup(src+2);
            return 0;
        }
        else if(strncmp(src, "u=", 2) == 0)
        {
            if(session->uri == NULL)
                session->uri = strdup(src+2);
            //session->media_start = false;
            return 0;
        }
        else if(strncmp(src, "e=", 2) == 0)
        {
            char *token = NULL;
            /* 获取第一个子字符串 */
            token = strtok(src+2, " ");
            /* 继续获取其他的子字符串 */
            while( token != NULL ) {
                //printf( "%s\n", token );
                if(session->emails[session->emails_count] == NULL)
                    session->emails[session->emails_count++] = strdup(token);
                token = strtok(NULL, " ");
            }
            //session->media_start = false;
            return 0;
        }
        else if(strncmp(src, "p=", 2) == 0)
        {
            char *token = NULL;
            /* 获取第一个子字符串 */
            token = strtok(src+2, " ");
            /* 继续获取其他的子字符串 */
            while( token != NULL ) {
                //printf( "%s\n", token );
                if(session->phones[session->phones_count] == NULL)
                    session->phones[session->phones_count++] = strdup(token);
                token = strtok(NULL, " ");
            }
            //session->media_start = false;
            return 0;
        }
        else if(strncmp(src, "z=", 2) == 0)
        {
            // RFC4566 5.11
            // z=<adjustment time> <offset> <adjustment time> <offset> ....
            // z=2882844526 -1h 2898848070 0
            int i = 0;
            char *token = NULL;
            token = strtok(src+2, " ");
            while( token != NULL ) {
                if(i == 0)
                {
                    session->timezones[session->timezones_count].adjust = sdp_attr_repeat_parse(token);
                    i = 1;
                }
                else if(i == 1)
                {
                    session->timezones[session->timezones_count].offset = sdp_attr_repeat_parse(token);
                    i = 0;
                    session->timezones_count++;
                }
                token = strtok(NULL, " ");
            }
            //session->media_start = false;
            return 0;
        }
        else if(strncmp(src, "k=", 2) == 0)
        {
            int n = strcspn(src+2, ":");
            if(n)
            {
                if(session->encrypt_key.method == NULL)
                    session->encrypt_key.method = sdp_scopy(src+2, n);
                if(session->encrypt_key.key == NULL && len > (n+2+1))
                {
                    session->encrypt_key.key = strdup(src+2 + n +1);
                }
            }
            return 0;
        }
        else if(strncmp(src, "a=", 2) == 0)
        {
            int n = strcspn(src+2, ":");
            if(n)
            {
                if(session->attr[session->attr_count].name == NULL)
                    session->attr[session->attr_count].name = sdp_scopy(src+2, n);
                if(session->attr[session->attr_count].value == NULL && len > (n+2+1))
                {
                    session->attr[session->attr_count].value = strdup(src+2 + n +1);
                }
                session->attr_count++;
            }
            return 0;
        }
        else if(strncmp(src, "t=", 2) == 0)
        {
            sscanf(src+2, "%d %d", &session->time.start, &session->time.stop);
            session->media_start = false;
            return 0;
        }
        else if(strncmp(src, "r=", 2) == 0)
        {
            // r=604800 3600 0 90000
            // r=7d 1h 0 25h
            //sscanf(src+2, "%d %d", &session->time.start, &session->time.stop);
            int i = 0;
            char *token = NULL;
            /* 获取第一个子字符串 */
            token = strtok(src+2, " ");
            /* 继续获取其他的子字符串 */
            while( token != NULL ) {
                if(i == 0)
                {
                    session->time.repeat[session->time.repeat_count].interval = sdp_attr_repeat_parse(token);
                }
                else if(i == 1)
                {
                    session->time.repeat[session->time.repeat_count].duration = sdp_attr_repeat_parse(token);
                }
                else
                {
                    int n = session->time.repeat[session->time.repeat_count].offsets_count;
                    session->time.repeat[session->time.repeat_count].offsets[n] = sdp_attr_repeat_parse(token);
                    session->time.repeat[session->time.repeat_count].offsets_count++;
                }
                token = strtok(NULL, " ");
                i++;
            }
            session->time.repeat_count++;
            session->media_start = false;
            return 0;
        }
        else if(strncmp(src, "m=", 2) == 0)
        {
            char *token = NULL;
            memset(tmp, 0, sizeof(tmp));
            memset(net_type, 0, sizeof(net_type));
            memset(addr_type, 0, sizeof(addr_type));
            memset(addr, 0, sizeof(addr));

            sscanf(src+2, "%s %hu %s %s", net_type,
                   &session->media[session->media_count].desc.port,
                    addr_type, tmp);
            if(session->media[session->media_count].desc.media == NULL)
                session->media[session->media_count].desc.media = strdup(net_type);		/**< Media type ("audio", "video")  */
            session->media[session->media_count].desc.port_count = 1;		/**< Port count, used only when >2  */
            if(session->media[session->media_count].desc.proto == NULL)
                session->media[session->media_count].desc.proto = strdup(addr_type);		/**< Transport ("RTP/AVP")	    */


            token = strtok(tmp, " ");
            while( token != NULL ) {
                if(session->media[session->media_count].desc.fmt[session->media[session->media_count].desc.fmt_count] == NULL)
                    session->media[session->media_count].desc.fmt[session->media[session->media_count].desc.fmt_count] = strdup(token);       /**< Media formats.	    */
                session->media[session->media_count].desc.fmt_count++;
                token = strtok(NULL, " ");
            }
            session->media_count++;
            session->media_start = true;
            return 0;
        }
        else if(strncmp(src, "b=", 2) == 0)
        {
            //b=X-YZ:128
            int n = strcspn(src+2, ":");
            if(n)
            {
                if(session->bandw[session->bandw_count].bwtype == NULL)
                    session->bandw[session->bandw_count].bwtype = sdp_scopy(src+2, n);
                if(session->bandw[session->bandw_count].bandwidth == NULL && len > (n+2+1))
                {
                    session->bandw[session->bandw_count].bandwidth = strdup(src+2 + n +1);
                }
            }
            session->bandw_count++;
            return 0;
        }
    }
    return -1;
}

static int sdp_media_attr_get_value(const char*src, uint32_t len, struct sdp_session *session, const char*name)
{
    if(src && session->media_start && session->media_count >= 1)
    {
        int mindex = 0, aindex = 0;
        char net_type[64];	    /**< Network type ("IN")		*/
        char addr_type[64];	    /**< Address type ("IP4", "IP6")	*/
        char addr[64];	    /**< The address.			*/
        char tmp[256];
        if(strncmp(src, "m=", 2) == 0)
        {
            char *token = NULL;
            memset(tmp, 0, sizeof(tmp));
            memset(net_type, 0, sizeof(net_type));
            memset(addr_type, 0, sizeof(addr_type));
            memset(addr, 0, sizeof(addr));

            sscanf(src+2, "%s %hu %s %s", net_type,
                   &session->media[session->media_count].desc.port,
                    addr_type, tmp);
            if(session->media[session->media_count].desc.media == NULL)
                session->media[session->media_count].desc.media = strdup(net_type);		/**< Media type ("audio", "video")  */
            session->media[session->media_count].desc.port_count = 1;		/**< Port count, used only when >2  */
            if(session->media[session->media_count].desc.proto == NULL)
                session->media[session->media_count].desc.proto = strdup(addr_type);		/**< Transport ("RTP/AVP")	    */
            mindex = session->media[session->media_count].desc.fmt_count;
            token = strtok(tmp, " ");
            while( token != NULL ) {
                if(session->media[session->media_count].desc.fmt[mindex] == NULL)
                {
                    session->media[session->media_count].desc.fmt[mindex] = strdup(token);       /**< Media formats.	    */
                    mindex++;
                }
                token = strtok(NULL, " ");
            }
            session->media[session->media_count].desc.fmt_count = mindex;
            fprintf(stdout, "=================================media_count=%d\r\n", session->media_count);
            session->media_count++;
            session->media_start = true;
            return 0;
        }
        else if(strncmp(src, "c=", 2) == 0)
        {
            memset(net_type, 0, sizeof(net_type));
            memset(addr_type, 0, sizeof(addr_type));
            memset(addr, 0, sizeof(addr));
            sscanf(src+2, "%s %s %s", net_type, addr_type, addr);
            if(session->media[session->media_count-1].conn.net_type == NULL)
                session->media[session->media_count-1].conn.net_type = strdup(net_type);	    /**< Network type ("IN")		*/
            if(session->media[session->media_count-1].conn.addr_type == NULL)
                session->media[session->media_count-1].conn.addr_type = strdup(addr_type);	    /**< Address type ("IP4", "IP6")	*/
            if(session->media[session->media_count-1].conn.addr == NULL)
                session->media[session->media_count-1].conn.addr = strdup(addr);	    /**< The address.*/
            return 0;
        }
        else if(strncmp(src, "i=", 2) == 0)
        {
            if(session->media[session->media_count-1].title == NULL)
                session->media[session->media_count-1].title = strdup(src+2);
            return 0;
        }
        else if(strncmp(src, "k=", 2) == 0)
        {
            int n = strcspn(src+2, ":");
            if(n)
            {
                if(session->media[session->media_count-1].encrypt_key.method == NULL)
                    session->media[session->media_count-1].encrypt_key.method = sdp_scopy(src+2, n);
                if(session->media[session->media_count-1].encrypt_key.key == NULL && len > (n+2+1))
                {
                    session->media[session->media_count-1].encrypt_key.key = strdup(src+2 + n +1);
                }
            }
            return 0;
        }
        else if(strncmp(src, "a=", 2) == 0)
        {
            aindex = session->media[session->media_count-1].attr_count;
            fprintf(stdout, "============================a=====media_count-1=%d\r\n", session->media_count-1);
            int n = strcspn(src+2, ":");
            if(n)
            {
                if(session->media[session->media_count-1].attr[aindex].name == NULL)
                    session->media[session->media_count-1].attr[aindex].name = sdp_scopy(src+2, n);
                if(session->media[session->media_count-1].attr[aindex].value == NULL && len > (n+2+1))
                {
                    session->media[session->media_count-1].attr[aindex].value = strdup(src+2 + n +1);
                }
                aindex++;
                session->media[session->media_count-1].attr_count = aindex;
            }
            return 0;
        }
        else if(strncmp(src, "b=", 2) == 0)
        {
            aindex = session->media[session->media_count-1].bandw_count;
            //b=X-YZ:128
            int n = strcspn(src+2, ":");
            if(n)
            {
                if(session->media[session->media_count-1].bandw[aindex].bwtype == NULL)
                    session->media[session->media_count-1].bandw[aindex].bwtype = sdp_scopy(src+2, n);
                if(session->media[session->media_count-1].bandw[aindex].bandwidth == NULL && len > (n+2+1))
                {
                    session->media[session->media_count-1].bandw[aindex].bandwidth = strdup(src+2 + n +1);
                }
                aindex++;
            }
            session->media[session->media_count-1].bandw_count = aindex;
            return 0;
        }
        else
        {
            fprintf(stdout, "=================================%s\r\n", src);
        }
        session->media_start = false;
    }
    session->media_start = false;
    return -1;
}


int sdp_attr_check(char*src, uint32_t len)
{
    unsigned int i = 0;
    for(i = 0; i < (sizeof(_sdp_session_cb)/sizeof(_sdp_session_cb[0])); i++)
    {
        if(len > 2 && _sdp_session_cb[i].name && strncmp(src, _sdp_session_cb[i].name, 2)==0)
            return 1;
    }
    return 0;
}


static int sdp_media_attr_parse(char*src, uint32_t len, struct sdp_session *hdr)
{
    unsigned int i = 0;
    for(i = 0; i < (sizeof(_sdp_media_cb)/sizeof(_sdp_media_cb[0])); i++)
    {
        if(len > 2 && _sdp_media_cb[i].name && _sdp_media_cb[i].get_value && strncmp(src, _sdp_media_cb[i].name, 2)==0)
        {
            return (_sdp_media_cb[i].get_value)(src, len, hdr, _sdp_media_cb[i].name);
        }
    }
    return -1;
}


int sdp_attr_parse(char*src, uint32_t len, struct sdp_session *hdr, bool m)
{
    unsigned int i = 0;
    if(m)
    {
        return sdp_media_attr_parse(src, len, hdr);
    }
    for(i = 0; i < (sizeof(_sdp_session_cb)/sizeof(_sdp_session_cb[0])); i++)
    {
        if(len > 2 && _sdp_session_cb[i].name && _sdp_session_cb[i].get_value && strncmp(src, _sdp_session_cb[i].name, 2)==0)
        {
            return (_sdp_session_cb[i].get_value)(src, len, hdr, _sdp_session_cb[i].name);
        }
    }
    return -1;
}

static int sdp_media_attr_free(struct sdp_session *session)
{
    unsigned int i = 0, j = 0;
    for(i = 0; i < session->media_count; i++)
    {
        if(session->media[i].desc.media)
        {
            free(session->media[i].desc.media);
            session->media[i].desc.media = NULL;
        }
        if(session->media[i].desc.proto)
        {
            free(session->media[i].desc.proto);
            session->media[i].desc.proto = NULL;
        }
        for(j = 0; j < session->media[i].desc.fmt_count; j++)
        {
            if(session->media[i].desc.fmt[j])
            {
                free(session->media[i].desc.fmt[j]);
                session->media[i].desc.fmt[j] = NULL;
            }
        }
        if(session->media[i].title)
        {
            free(session->media[i].title);
            session->media[i].title = NULL;
        }
        if(session->media[i].conn.addr)
        {
            free(session->media[i].conn.addr);
            session->media[i].conn.addr = NULL;
        }
        if(session->media[i].conn.addr_type)
        {
            free(session->media[i].conn.addr_type);
            session->media[i].conn.addr_type = NULL;
        }
        if(session->media[i].conn.net_type)
        {
            free(session->media[i].conn.net_type);
            session->media[i].conn.net_type = NULL;
        }
        for(j = 0; j < session->media[i].bandw_count; j++)
        {
            if(session->media[i].bandw[j].bwtype)
            {
                free(session->media[i].bandw[j].bwtype);
                session->media[i].bandw[j].bwtype = NULL;
            }
            if(session->media[i].bandw[j].bandwidth)
            {
                free(session->media[i].bandw[j].bandwidth);
                session->media[i].bandw[j].bandwidth = NULL;
            }
        }
        if(session->media[i].encrypt_key.method)
        {
            free(session->media[i].encrypt_key.method);
            session->media[i].encrypt_key.method = NULL;
        }
        if(session->media[i].encrypt_key.key)
        {
            free(session->media[i].encrypt_key.key);
            session->media[i].encrypt_key.key = NULL;
        }
        for(j = 0; j < session->media[i].attr_count; j++)
        {
            if(session->media[i].attr[j].name)
            {
                free(session->media[i].attr[j].name);
                session->media[i].attr[j].name = NULL;
            }
            if(session->media[i].attr[j].value)
            {
                free(session->media[i].attr[j].value);
                session->media[i].attr[j].value = NULL;
            }
        }
    }
    return 0;
}

int sdp_attr_free(struct sdp_session *session)
{
    unsigned int j = 0;

    if(session->origin.user)
    {
        free(session->origin.user);
        session->origin.user = NULL;
    }
    if(session->origin.net_type)
    {
        free(session->origin.net_type);
        session->origin.net_type = NULL;
    }
    if(session->origin.addr_type)
    {
        free(session->origin.addr_type);
        session->origin.addr_type = NULL;
    }
    if(session->origin.addr)
    {
        free(session->origin.addr);
        session->origin.addr = NULL;
    }
    if(session->name)
    {
        free(session->name);
        session->name = NULL;
    }
    if(session->information)
    {
        free(session->information);
        session->information = NULL;
    }
    if(session->uri)
    {
        free(session->uri);
        session->uri = NULL;
    }
    for(j = 0; j < session->emails_count; j++)
    {
        if(session->emails[j])
        {
            free(session->emails[j]);
            session->emails[j] = NULL;
        }
    }
    for(j = 0; j < session->phones_count; j++)
    {
        if(session->phones[j])
        {
            free(session->phones[j]);
            session->phones[j] = NULL;
        }
    }
    if(session->conn.addr)
    {
        free(session->conn.addr);
        session->conn.addr = NULL;
    }
    if(session->conn.addr_type)
    {
        free(session->conn.addr_type);
        session->conn.addr_type = NULL;
    }
    if(session->conn.net_type)
    {
        free(session->conn.net_type);
        session->conn.net_type = NULL;
    }

    for(j = 0; j < session->bandw_count; j++)
    {
        if(session->bandw[j].bwtype)
        {
            free(session->bandw[j].bwtype);
            session->bandw[j].bwtype = NULL;
        }
        if(session->bandw[j].bandwidth)
        {
            free(session->bandw[j].bandwidth);
            session->bandw[j].bandwidth = NULL;
        }
    }
    /*if(session->time.repeat)
    {
        free(session->time.repeat);
        session->time.repeat = NULL;
    }
    if(session->zone_adjustments)
    {
        free(session->zone_adjustments);
        session->zone_adjustments = NULL;
    }*/
    if(session->encrypt_key.method)
    {
        free(session->encrypt_key.method);
        session->encrypt_key.method = NULL;
    }
    if(session->encrypt_key.key)
    {
        free(session->encrypt_key.key);
        session->encrypt_key.key = NULL;
    }
    for(j = 0; j < session->attr_count; j++)
    {
        if(session->attr[j].name)
        {
            free(session->attr[j].name);
            session->attr[j].name = NULL;
        }
        if(session->attr[j].value)
        {
            free(session->attr[j].value);
            session->attr[j].value = NULL;
        }
    }
    sdp_media_attr_free(session);
    return 0;
}



static int sdp_media_attr_debug(struct sdp_session *session)
{
    unsigned int i = 0, j = 0;
    for(i = 0; i < session->media_count; i++)
    {
        if(session->media[i].desc.media)
        {
            fprintf(stdout, " m=%s %d ", session->media[i].desc.media, session->media[i].desc.port);
        }
        if(session->media[i].desc.proto)
        {
            fprintf(stdout, "%s ", session->media[i].desc.proto);
        }
        for(j = 0; j < session->media[i].desc.fmt_count; j++)
        {
            if(session->media[i].desc.fmt[j])
            {
                fprintf(stdout, "%s ", session->media[i].desc.fmt[j]);
            }
        }
        if(session->media[i].conn.net_type)
        {
            fprintf(stdout, "%s ", session->media[i].conn.net_type);
        }
        if(session->media[i].conn.addr_type)
        {
            fprintf(stdout, "%s ", session->media[i].conn.addr_type);
        }
        if(session->media[i].conn.addr)
        {
            fprintf(stdout, "%s ", session->media[i].conn.addr);
        }
        fprintf(stdout, "\r\n");
        if(session->media[i].title)
        {
            fprintf(stdout, " i=%s\r\n", session->media[i].title);
        }

        for(j = 0; j < session->media[i].bandw_count; j++)
        {
            fprintf(stdout, " b=");
            if(session->media[i].bandw[j].bwtype && session->media[i].bandw[j].bandwidth)
            {
                fprintf(stdout, "%s:%s ", session->media[i].bandw[j].bwtype, session->media[i].bandw[j].bandwidth);
            }
            else if(session->media[i].bandw[j].bwtype)
                fprintf(stdout, "%s ", session->media[i].bandw[j].bwtype);
        }
        if(session->media[i].bandw_count)
            fprintf(stdout, "\r\n");
        if(session->media[i].encrypt_key.method)
        {
            if(session->media[i].encrypt_key.key)
                fprintf(stdout, " k=%s:%s\r\n", session->media[i].encrypt_key.method, session->media[i].encrypt_key.key);
            else
                fprintf(stdout, " k=%s\r\n", session->media[i].encrypt_key.method);
        }
        for(j = 0; j < session->media[i].attr_count; j++)
        {
            if(session->media[i].attr[j].name && session->media[i].attr[j].value)
            {
                fprintf(stdout, " a=%s:%s\r\n", session->media[i].attr[j].name,session->media[i].attr[j].value);
            }
            else if(session->media[i].attr[j].name)
            {
                fprintf(stdout, " a=%s\r\n", session->media[i].attr[j].name);
            }
        }
    }
    fflush(stdout);
    return 0;
}

int sdp_attr_debug(struct sdp_session *session)
{
    unsigned int j = 0;

    if(session->origin.user)
    {
        fprintf(stdout, " o=%s %d %d ", session->origin.user, session->origin.id, session->origin.version);
    }
    if(session->origin.net_type)
    {
        fprintf(stdout, "%s ", session->origin.net_type);
    }
    if(session->origin.addr_type)
    {
        fprintf(stdout, "%s ", session->origin.addr_type);
    }
    if(session->origin.addr)
    {
        fprintf(stdout, "%s ", session->origin.addr);
    }
    fprintf(stdout, "\r\n");
    if(session->name)
    {
        fprintf(stdout, " s=%s\r\n", session->name);
    }
    if(session->information)
    {
        fprintf(stdout, " i=%s\r\n", session->information);
    }
    if(session->uri)
    {
        fprintf(stdout, " u=%s\r\n", session->uri);
    }
    for(j = 0; j < session->emails_count; j++)
    {
        fprintf(stdout, " e=");
        if(session->emails[j])
        {
            fprintf(stdout, "%s ", session->emails[j]);
        }
    }
    if(session->emails_count)
        fprintf(stdout, "\r\n");
    for(j = 0; j < session->phones_count; j++)
    {
        fprintf(stdout, " p=");
        if(session->phones[j])
        {
            fprintf(stdout, "%s ", session->phones[j]);
        }
    }
    if(session->phones_count)
        fprintf(stdout, "\r\n");
    if(session->conn.net_type)
    {
        fprintf(stdout, " c=%s ", session->conn.net_type);
    }
    if(session->conn.addr_type)
    {
        fprintf(stdout, "%s ", session->conn.addr_type);
    }
    if(session->conn.addr)
    {
        fprintf(stdout, "%s ", session->conn.addr);
    }
    if(session->conn.net_type)
        fprintf(stdout, "\r\n");

    for(j = 0; j < session->bandw_count; j++)
    {
        fprintf(stdout, " b=");
        if(session->bandw[j].bwtype && session->bandw[j].bandwidth)
        {
            fprintf(stdout, "%s:%s ", session->bandw[j].bwtype, session->bandw[j].bandwidth);
        }
        else if(session->bandw[j].bwtype)
            fprintf(stdout, "%s ", session->bandw[j].bwtype);
    }
    if(session->bandw_count)
        fprintf(stdout, "\r\n");
    /*if(session->time.repeat)
    {
    }
    if(session->zone_adjustments)
    {
    }*/
    if(session->encrypt_key.method && session->encrypt_key.key)
    {
        fprintf(stdout, " k=%s:%s\r\n", session->encrypt_key.method, session->encrypt_key.key);
    }
    else if(session->encrypt_key.method)
    {
        fprintf(stdout, " k=%s\r\n", session->encrypt_key.method);
    }
    for(j = 0; j < session->attr_count; j++)
    {
        if(session->attr[j].name && session->attr[j].value)
        {
            fprintf(stdout, " a=%s:%s\r\n", session->attr[j].name, session->attr[j].value);
        }
        else if(session->attr[j].name)
        {
            fprintf(stdout, " a=%s\r\n", session->attr[j].name);
        }
    }
    fflush(stdout);
    sdp_media_attr_debug(session);
    return 0;
}

int sdp_attr_rtpmap_get(const char* rtpmapstr, struct sdp_rtpmap *rtpmap)
{
    const char *p1;
    const char *p = rtpmapstr;

    // payload type
    p1 = strchr(p, ' ');
    if(!p1)
        return -1;

    if(rtpmap)
    {
        rtpmap->pt = atoi(p);
    }
    p = p1 + 1;

    // encoding name
    //assert(' ' == *p1);
    p1 = strchr(p, '/');
    if(!p1)
        return -1;

    if(rtpmap)
    {
        memcpy(rtpmap->enc_name, p, p1-p);
        rtpmap->enc_name[p1-p] = '\0';
    }

    // clock rate
    //assert('/' == *p1);
    if(rtpmap)
    {
        rtpmap->clock_rate = atoi(p1+1);
    }

    // encoding parameters
    if(rtpmap)
    {
        p1 = strchr(p1+1, '/');
        if(p1)
        {
            strcpy(rtpmap->param, p1+1);
        }
        else
        {
            rtpmap->param[0] = '\0';
        }
    }

    return 0;
}
