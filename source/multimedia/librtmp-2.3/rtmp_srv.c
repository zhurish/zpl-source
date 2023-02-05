/*  Thread compatibility glue
 *  Copyright (C) 2009 Howard Chu
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RTMPDump; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include <signal.h>
#include <getopt.h>

#include <assert.h>

#include "rtmp_sys.h"
#include "rtmp_log.h"

#include "rtmp_thread.h"

#ifdef linux
#include <linux/netfilter_ipv4.h>
#endif

#ifndef WIN32
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include "rtmp_srv.h"
#include "zpl_include.h"

#ifdef WIN32
#define InitSockets()              \
  {                                \
    WORD version;                  \
    WSADATA wsaData;               \
                                   \
    version = MAKEWORD(1, 1);      \
    WSAStartup(version, &wsaData); \
  }

#define CleanupSockets() WSACleanup()
#else
#define InitSockets()
#define CleanupSockets()
#endif

#define STR2AVAL(av, str) \
  av.av_val = str;        \
  av.av_len = strlen(av.av_val)

#define SAVC(x) static const AVal _av_##x = AVC(#x)

SAVC(rtmp_app);
SAVC(rtmp_connect);
SAVC(rtmp_flashVer);
SAVC(rtmp_swfUrl);
SAVC(rtmp_pageUrl);
SAVC(rtmp_tcUrl);
SAVC(rtmp_fpad);
SAVC(rtmp_capabilities);
SAVC(rtmp_audioCodecs);
SAVC(rtmp_videoCodecs);
SAVC(rtmp_videoFunction);
SAVC(rtmp_objectEncoding);
SAVC(rtmp_result);
SAVC(rtmp_createStream);
SAVC(rtmp_getStreamLength);
SAVC(rtmp_play);
SAVC(rtmp_fmsVer);
SAVC(rtmp_mode);
SAVC(rtmp_level);
SAVC(rtmp_code);
SAVC(rtmp_description);
SAVC(rtmp_secureToken);

SAVC(rtmp_onStatus);
SAVC(rtmp_status);
static const AVal _av_rtmp_NetStream_Play_Start = AVC("NetStream.Play.Start");
static const AVal _av_rtmp_Started_playing = AVC("Started playing");
static const AVal _av_rtmp_NetStream_Play_Stop = AVC("NetStream.Play.Stop");
static const AVal _av_rtmp_Stopped_playing = AVC("Stopped playing");
SAVC(rtmp_details);
SAVC(rtmp_clientid);

static int rtmp_server_packet_handle(rtmp_server_t *server, RTMP *r, RTMPPacket *packet);
static int rtmp_client_read_thread(void *p);
static int rtmp_server_accept_thread(void *p);

int rtmp_server_create(rtmp_server_t *srv)
{
  struct sockaddr_in addr;
  int tmp;
  srv->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (srv->sock == -1)
  {
    RTMP_Log(RTMP_LOGERROR, "%s, couldn't create socket", __FUNCTION__);
    return -1;
  }

  tmp = 1;
  setsockopt(srv->sock, SOL_SOCKET, SO_REUSEADDR, (char *)&tmp, sizeof(tmp));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(srv->address); //htonl(INADDR_ANY);
  addr.sin_port = htons(srv->rtmpport);

  if (bind(srv->sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
  {
    RTMP_Log(RTMP_LOGERROR, "%s, TCP bind failed for port number: %d", __FUNCTION__, srv->rtmpport);
    return -1;
  }

  if (listen(srv->sock, RTMP_CLIENT_MAX) == -1)
  {
    RTMP_Log(RTMP_LOGERROR, "%s, listen failed", __FUNCTION__);
    closesocket(srv->sock);
    return -1;
  }
  srv->t_read = os_ansync_add(srv->t_master, OS_ANSYNC_INPUT, rtmp_server_accept_thread, srv, srv->sock);
  return 0;
}

int rtmp_server_destroy(rtmp_server_t *srv)
{
  if (srv->rtmp_client_num)
    return 0;
  if (srv->sock)
  {
    if (srv->t_master && srv->t_read)
    {
      os_ansync_cancel(srv->t_master, srv->t_read);
      srv->t_read = NULL;
    }
    closesocket(srv->sock);
    srv->sock = 0;
  }
  srv->state = RTMP_STATE_STOPPED;
  return 0;
}

static rtmp_client_t *rtmp_client_create(int sock, char *address)
{
  rtmp_client_t *client = malloc(sizeof(rtmp_client_t));
  if (client)
  {
    memset(client, 0, sizeof(rtmp_client_t));
    client->sock = sock;
    client->address = strdup(address);

    client->rtmp = RTMP_Alloc();
    if (client->rtmp)
    {
      RTMP_Init(client->rtmp);
      ((RTMP *)client->rtmp)->m_sb.sb_socket = sock;
      return client;
    }
    free(client->address);
    free(client);
  }
  return NULL;
}

static int rtmp_client_destroy(rtmp_client_t *client)
{
  if (client)
  {
    if (client->t_read && OS_ANSYNC_MASTER(client->t_read))
    {
      os_ansync_cancel(OS_ANSYNC_MASTER(client->t_read), client->t_read);
      client->t_read = NULL;
    }
    if (client->rtmp)
    {
      RTMP_Close(client->rtmp);
      RTMP_Free(client->rtmp);
      client->rtmp = NULL;
    }
    if (client->filename)
    {
      free(client->filename);
      client->filename = NULL;
    }
    if (client->rtspurl)
    {
      free(client->rtspurl);
      client->rtspurl = NULL;
    }
    if (client->httpurl)
    {
      free(client->httpurl);
      client->httpurl = NULL;
    }
    if (client->address)
    {
      free(client->address);
      client->address = NULL;
    }
    if (client->sock)
    {
      closesocket(client->sock);
      client->sock = 0;
    }
    free(client);
    return 0;
  }
  return -1;
}

rtmp_client_t *rtmp_server_lookup(rtmp_server_t *srv, int sock)
{
  u_int i = 0;
  for (i = 0; i < RTMP_CLIENT_MAX; i++)
  {
    if (srv->rtmp_client[i] && srv->rtmp_client[i]->sock == sock)
      return srv->rtmp_client[i];
  }
  return NULL;
}

int rtmp_server_add(rtmp_server_t *srv, rtmp_client_t *client)
{
  u_int i = 0;
  for (i = 0; i < RTMP_CLIENT_MAX; i++)
  {
    if (srv->rtmp_client[i] == NULL)
    {
      srv->rtmp_client[i] = client;
      srv->rtmp_client_num++;
      return 0;
    }
  }
  return -1;
}

int rtmp_server_add_bysock(rtmp_server_t *srv, int sock, char *address)
{
  u_int i = 0;
  rtmp_client_t *client = rtmp_client_create(sock, address);
  if (client)
  {
    for (i = 0; i < RTMP_CLIENT_MAX; i++)
    {
      if (srv->rtmp_client[i] == NULL)
      {
        srv->rtmp_client[i] = client;
        srv->rtmp_client_num++;
        return 0;
      }
    }
  }
  return -1;
}

int rtmp_server_del(rtmp_server_t *srv, int sock)
{
  u_int i = 0;
  for (i = 0; i < RTMP_CLIENT_MAX; i++)
  {
    if (srv->rtmp_client[i] && srv->rtmp_client[i]->sock == sock)
    {
      rtmp_client_destroy(srv->rtmp_client[i]);
      srv->rtmp_client[i] = NULL;
      srv->rtmp_client_num--;
    }
  }
  return -1;
}

static int rtmp_server_accept(rtmp_server_t *srv)
{
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  int sockfd = accept(srv->sock, (struct sockaddr *)&addr, &addrlen);
  if (sockfd > 0)
  {
    RTMP_Log(RTMP_LOGDEBUG, "%s: processed request\n", __FUNCTION__);
    rtmp_client_t *client = rtmp_server_lookup(srv, sockfd);
    if (client)
    {
      RTMP_Log(RTMP_LOGDEBUG, "%s: processed request\n", __FUNCTION__);
      return -1;
    }
    client = rtmp_client_create(sockfd, inet_ntoa(addr.sin_addr));
    if (client == NULL)
    {
      closesocket(sockfd);
      return -1;
    }
    if (rtmp_server_add(srv, client) == 0)
    //if(rtmp_server_add_bysock(srv, sockfd, inet_ntoa(addr.sin_addr)) == 0)
    {
      if (client->rtmp)
      {
        if (!RTMP_Serve(client->rtmp))
        {
          RTMP_Log(RTMP_LOGERROR, "Handshake failed");
          rtmp_server_del(srv, sockfd);
          return -1;
        }
        client->state = RTMP_STATE_IN_PROGRESS;
        client->t_read = os_ansync_add(srv->t_master, OS_ANSYNC_INPUT, rtmp_client_read_thread, client, sockfd);
        return 0;
      }
      else
      {
        rtmp_server_del(srv, sockfd);
      }
    }
  }
  else
  {
    RTMP_Log(RTMP_LOGERROR, "%s: accept failed", __FUNCTION__);
  }
  return -1;
}

static int rtmp_client_read_thread(void *p)
{
  rtmp_client_t *client = (rtmp_client_t *)p;
  if (client)
  {
    RTMPPacket packet = {0};
    client->t_read = NULL;
    memset(&packet, 0, sizeof(packet));
    while (RTMP_IsConnected(client->rtmp) && RTMP_ReadPacket(client->rtmp, &packet))
    {
      if (!RTMPPacket_IsReady(&packet))
        continue;
      rtmp_server_packet_handle(srv, client->rtmp, &packet);
      RTMPPacket_Free(&packet);
    }
    client->t_read = os_ansync_add(srv->t_master, OS_ANSYNC_INPUT, rtmp_client_read_thread, client, client->sock);
  }
  return 0;
}

static int rtmp_server_accept_thread(void *p)
{
  rtmp_server_t *srv = (rtmp_server_t *)p;
  if (srv)
  {
    srv->t_read = NULL;
    rtmp_server_accept(srv);
    srv->t_read = os_ansync_add(srv->t_master, OS_ANSYNC_INPUT, rtmp_server_accept_thread, srv, srv->sock);
  }
  return 0;
}



static int rtmp_send_connect_result(RTMP *r, double txn)
{
  RTMPPacket packet;
  char pbuf[384], *pend = pbuf + sizeof(pbuf);
  AMFObject obj;
  AMFObjectProperty p, op;
  AVal av;

  packet.m_nChannel = 0x03;   // control channel (invoke)
  packet.m_headerType = 1;    /* RTMP_PACKET_SIZE_MEDIUM; */
  packet.m_packetType = 0x14; // INVOKE
  packet.m_nTimeStamp = 0;
  packet.m_nInfoField2 = 0;
  packet.m_hasAbsTimestamp = 0;
  packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

  char *enc = packet.m_body;
  enc = AMF_EncodeString(enc, pend, &_av_rtmp_result);
  enc = AMF_EncodeNumber(enc, pend, txn);
  *enc++ = AMF_OBJECT;

  STR2AVAL(av, "FMS/3,5,1,525");
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_fmsVer, &av);
  enc = AMF_EncodeNamedNumber(enc, pend, &_av_rtmp_capabilities, 31.0);
  enc = AMF_EncodeNamedNumber(enc, pend, &_av_rtmp_mode, 1.0);
  *enc++ = 0;
  *enc++ = 0;
  *enc++ = AMF_OBJECT_END;

  *enc++ = AMF_OBJECT;

  STR2AVAL(av, "status");
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_level, &av);
  STR2AVAL(av, "NetConnection.Connect.Success");
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_code, &av);
  STR2AVAL(av, "Connection succeeded.");
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_description, &av);
  enc = AMF_EncodeNamedNumber(enc, pend, &_av_rtmp_objectEncoding, r->m_fEncoding);
#if 0
  STR2AVAL(av, "58656322c972d6cdf2d776167575045f8484ea888e31c086f7b5ffbd0baec55ce442c2fb");
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_secureToken, &av);
#endif
  STR2AVAL(p.p_name, "version");
  STR2AVAL(p.p_vu.p_aval, "3,5,1,525");
  p.p_type = AMF_STRING;
  obj.o_num = 1;
  obj.o_props = &p;
  op.p_type = AMF_OBJECT;
  STR2AVAL(op.p_name, "data");
  op.p_vu.p_object = obj;
  enc = AMFProp_Encode(&op, enc, pend);
  *enc++ = 0;
  *enc++ = 0;
  *enc++ = AMF_OBJECT_END;
  *enc++ = 0;
  *enc++ = 0;
  *enc++ = AMF_OBJECT_END;

  packet.m_nBodySize = enc - packet.m_body;

  return RTMP_SendPacket(r, &packet, FALSE);
}

static int rtmp_send_result_number(RTMP *r, double txn, double ID)
{
  RTMPPacket packet;
  char pbuf[256], *pend = pbuf + sizeof(pbuf);

  packet.m_nChannel = 0x03;   // control channel (invoke)
  packet.m_headerType = 1;    /* RTMP_PACKET_SIZE_MEDIUM; */
  packet.m_packetType = 0x14; // INVOKE
  packet.m_nTimeStamp = 0;
  packet.m_nInfoField2 = 0;
  packet.m_hasAbsTimestamp = 0;
  packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

  char *enc = packet.m_body;
  enc = AMF_EncodeString(enc, pend, &_av_rtmp_result);
  enc = AMF_EncodeNumber(enc, pend, txn);
  *enc++ = AMF_NULL;
  enc = AMF_EncodeNumber(enc, pend, ID);

  packet.m_nBodySize = enc - packet.m_body;

  return RTMP_SendPacket(r, &packet, FALSE);
}

static int rtmp_send_play_start(RTMP *r)
{
  RTMPPacket packet;
  char pbuf[384], *pend = pbuf + sizeof(pbuf);

  packet.m_nChannel = 0x03;   // control channel (invoke)
  packet.m_headerType = 1;    /* RTMP_PACKET_SIZE_MEDIUM; */
  packet.m_packetType = 0x14; // INVOKE
  packet.m_nTimeStamp = 0;
  packet.m_nInfoField2 = 0;
  packet.m_hasAbsTimestamp = 0;
  packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

  char *enc = packet.m_body;
  enc = AMF_EncodeString(enc, pend, &_av_rtmp_onStatus);
  enc = AMF_EncodeNumber(enc, pend, 0);
  *enc++ = AMF_OBJECT;

  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_level, &_av_rtmp_status);
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_code, &_av_rtmp_NetStream_Play_Start);
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_description, &_av_rtmp_Started_playing);
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_details, &r->Link.playpath);
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_clientid, &_av_rtmp_clientid);
  *enc++ = 0;
  *enc++ = 0;
  *enc++ = AMF_OBJECT_END;

  packet.m_nBodySize = enc - packet.m_body;
  return RTMP_SendPacket(r, &packet, FALSE);
}

static int rtmp_send_play_stop(RTMP *r)
{
  RTMPPacket packet;
  char pbuf[384], *pend = pbuf + sizeof(pbuf);

  packet.m_nChannel = 0x03;   // control channel (invoke)
  packet.m_headerType = 1;    /* RTMP_PACKET_SIZE_MEDIUM; */
  packet.m_packetType = 0x14; // INVOKE
  packet.m_nTimeStamp = 0;
  packet.m_nInfoField2 = 0;
  packet.m_hasAbsTimestamp = 0;
  packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

  char *enc = packet.m_body;
  enc = AMF_EncodeString(enc, pend, &_av_rtmp_onStatus);
  enc = AMF_EncodeNumber(enc, pend, 0);
  *enc++ = AMF_OBJECT;

  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_level, &_av_rtmp_status);
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_code, &_av_rtmp_NetStream_Play_Stop);
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_description, &_av_rtmp_Stopped_playing);
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_details, &r->Link.playpath);
  enc = AMF_EncodeNamedString(enc, pend, &_av_rtmp_clientid, &_av_rtmp_clientid);
  *enc++ = 0;
  *enc++ = 0;
  *enc++ = AMF_OBJECT_END;

  packet.m_nBodySize = enc - packet.m_body;
  return RTMP_SendPacket(r, &packet, FALSE);
}

static void spawn_dumper(int argc, AVal *av, char *cmd)
{
#ifdef WIN32
  STARTUPINFO si = {0};
  PROCESS_INFORMATION pi = {0};

  si.cb = sizeof(si);
  if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL,
                    &si, &pi))
  {
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }
#else
  /* reap any dead children */
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  if (fork() == 0)
  {
    char **argv = malloc((argc + 1) * sizeof(char *));
    int i;

    for (i = 0; i < argc; i++)
    {
      argv[i] = av[i].av_val;
      argv[i][av[i].av_len] = '\0';
    }
    argv[i] = NULL;
    if ((i = execvp(argv[0], argv)))
      _exit(i);
  }
#endif
}

static int rtmp_countAMF(AMFObject *obj, int *argc)
{
  int i, len;

  for (i = 0, len = 0; i < obj->o_num; i++)
  {
    AMFObjectProperty *p = &obj->o_props[i];
    len += 4;
    (*argc) += 2;
    if (p->p_name.av_val)
      len += 1;
    len += 2;
    if (p->p_name.av_val)
      len += p->p_name.av_len + 1;
    switch (p->p_type)
    {
    case AMF_BOOLEAN:
      len += 1;
      break;
    case AMF_STRING:
      len += p->p_vu.p_aval.av_len;
      break;
    case AMF_NUMBER:
      len += 40;
      break;
    case AMF_OBJECT:
      len += 9;
      len += rtmp_countAMF(&p->p_vu.p_object, argc);
      (*argc) += 2;
      break;
    case AMF_NULL:
    default:
      break;
    }
  }
  return len;
}

static char * rtmp_dumpAMF(AMFObject *obj, char *ptr, AVal *argv, int *argc)
{
  int i, len, ac = *argc;
  const char opt[] = "NBSO Z";

  for (i = 0, len = 0; i < obj->o_num; i++)
  {
    AMFObjectProperty *p = &obj->o_props[i];
    argv[ac].av_val = ptr + 1;
    argv[ac++].av_len = 2;
    ptr += sprintf(ptr, " -C ");
    argv[ac].av_val = ptr;
    if (p->p_name.av_val)
      *ptr++ = 'N';
    *ptr++ = opt[p->p_type];
    *ptr++ = ':';
    if (p->p_name.av_val)
      ptr += sprintf(ptr, "%.*s:", p->p_name.av_len, p->p_name.av_val);
    switch (p->p_type)
    {
    case AMF_BOOLEAN:
      *ptr++ = p->p_vu.p_number != 0 ? '1' : '0';
      argv[ac].av_len = ptr - argv[ac].av_val;
      break;
    case AMF_STRING:
      memcpy(ptr, p->p_vu.p_aval.av_val, p->p_vu.p_aval.av_len);
      ptr += p->p_vu.p_aval.av_len;
      argv[ac].av_len = ptr - argv[ac].av_val;
      break;
    case AMF_NUMBER:
      ptr += sprintf(ptr, "%f", p->p_vu.p_number);
      argv[ac].av_len = ptr - argv[ac].av_val;
      break;
    case AMF_OBJECT:
      *ptr++ = '1';
      argv[ac].av_len = ptr - argv[ac].av_val;
      ac++;
      *argc = ac;
      ptr = rtmp_dumpAMF(&p->p_vu.p_object, ptr, argv, argc);
      ac = *argc;
      argv[ac].av_val = ptr + 1;
      argv[ac++].av_len = 2;
      argv[ac].av_val = ptr + 4;
      argv[ac].av_len = 3;
      ptr += sprintf(ptr, " -C O:0");
      break;
    case AMF_NULL:
    default:
      argv[ac].av_len = ptr - argv[ac].av_val;
      break;
    }
    ac++;
  }
  *argc = ac;
  return ptr;
}

// Returns 0 for OK/Failed/error, 1 for 'Stop or Complete'
static int rtmp_server_invoke(rtmp_server_t *server, RTMP *r, RTMPPacket *packet, unsigned int offset)
{
  const char *body;
  unsigned int nBodySize;
  int ret = 0, nRes;

  body = packet->m_body + offset;
  nBodySize = packet->m_nBodySize - offset;

  if (body[0] != 0x02) // make sure it is a string method name we start with
  {
    RTMP_Log(RTMP_LOGWARNING, "%s, Sanity failed. no string method in invoke packet",
             __FUNCTION__);
    return 0;
  }

  AMFObject obj;
  nRes = AMF_Decode(&obj, body, nBodySize, FALSE);
  if (nRes < 0)
  {
    RTMP_Log(RTMP_LOGERROR, "%s, error decoding invoke packet", __FUNCTION__);
    return 0;
  }

  AMF_Dump(&obj);
  AVal method;
  AMFProp_GetString(AMF_GetProp(&obj, NULL, 0), &method);
  double txn = AMFProp_GetNumber(AMF_GetProp(&obj, NULL, 1));
  RTMP_Log(RTMP_LOGDEBUG, "%s, client invoking <%s>", __FUNCTION__, method.av_val);

  if (AVMATCH(&method, &_av_rtmp_connect))
  {
    AMFObject cobj;
    AVal pname, pval;
    int i;

    server->connect = packet->m_body;
    packet->m_body = NULL;

    AMFProp_GetObject(AMF_GetProp(&obj, NULL, 2), &cobj);
    for (i = 0; i < cobj.o_num; i++)
    {
      pname = cobj.o_props[i].p_name;
      pval.av_val = NULL;
      pval.av_len = 0;
      if (cobj.o_props[i].p_type == AMF_STRING)
        pval = cobj.o_props[i].p_vu.p_aval;
      if (AVMATCH(&pname, &_av_rtmp_app))
      {
        r->Link.app = pval;
        pval.av_val = NULL;
        if (!r->Link.app.av_val)
          r->Link.app.av_val = "";
        server->arglen += 6 + pval.av_len;
        server->argc += 2;
      }
      else if (AVMATCH(&pname, &_av_rtmp_flashVer))
      {
        r->Link.flashVer = pval;
        pval.av_val = NULL;
        server->arglen += 6 + pval.av_len;
        server->argc += 2;
      }
      else if (AVMATCH(&pname, &_av_rtmp_swfUrl))
      {
        r->Link.swfUrl = pval;
        pval.av_val = NULL;
        server->arglen += 6 + pval.av_len;
        server->argc += 2;
      }
      else if (AVMATCH(&pname, &_av_rtmp_tcUrl))
      {
        r->Link.tcUrl = pval;
        pval.av_val = NULL;
        server->arglen += 6 + pval.av_len;
        server->argc += 2;
      }
      else if (AVMATCH(&pname, &_av_rtmp_pageUrl))
      {
        r->Link.pageUrl = pval;
        pval.av_val = NULL;
        server->arglen += 6 + pval.av_len;
        server->argc += 2;
      }
      else if (AVMATCH(&pname, &_av_rtmp_audioCodecs))
      {
        r->m_fAudioCodecs = cobj.o_props[i].p_vu.p_number;
      }
      else if (AVMATCH(&pname, &_av_rtmp_videoCodecs))
      {
        r->m_fVideoCodecs = cobj.o_props[i].p_vu.p_number;
      }
      else if (AVMATCH(&pname, &_av_rtmp_objectEncoding))
      {
        r->m_fEncoding = cobj.o_props[i].p_vu.p_number;
      }
    }
    /* Still have more parameters? Copy them */
    if (obj.o_num > 3)
    {
      int i = obj.o_num - 3;
      r->Link.extras.o_num = i;
      r->Link.extras.o_props = malloc(i * sizeof(AMFObjectProperty));
      memcpy(r->Link.extras.o_props, obj.o_props + 3, i * sizeof(AMFObjectProperty));
      obj.o_num = 3;
      server->arglen += rtmp_countAMF(&r->Link.extras, &server->argc);
    }
    rtmp_send_connect_result(r, txn);
  }
  else if (AVMATCH(&method, &_av_rtmp_createStream))
  {
    rtmp_send_result_number(r, txn, ++server->streamID);
  }
  else if (AVMATCH(&method, &_av_rtmp_getStreamLength))
  {
    rtmp_send_result_number(r, txn, 10.0);
  }
  else if (AVMATCH(&method, &_av_rtmp_play))
  {
    char *file, *p, *q, *cmd, *ptr;
    AVal *argv, av;
    int len, argc;
    uint32_t now;
    RTMPPacket pc = {0};
    AMFProp_GetString(AMF_GetProp(&obj, NULL, 3), &r->Link.playpath);
    /*
      r->Link.seekTime = AMFProp_GetNumber(AMF_GetProp(&obj, NULL, 4));
      if (obj.o_num > 5)
	r->Link.length = AMFProp_GetNumber(AMF_GetProp(&obj, NULL, 5));
      */
    if (r->Link.tcUrl.av_len)
    {
      len = server->arglen + r->Link.playpath.av_len + 4 +
            sizeof("rtmpdump") + r->Link.playpath.av_len + 12;
      server->argc += 5;

      cmd = malloc(len + server->argc * sizeof(AVal));
      ptr = cmd;
      argv = (AVal *)(cmd + len);
      argv[0].av_val = cmd;
      argv[0].av_len = sizeof("rtmpdump") - 1;
      ptr += sprintf(ptr, "rtmpdump");
      argc = 1;

      argv[argc].av_val = ptr + 1;
      argv[argc++].av_len = 2;
      argv[argc].av_val = ptr + 5;
      ptr += sprintf(ptr, " -r \"%s\"", r->Link.tcUrl.av_val);
      argv[argc++].av_len = r->Link.tcUrl.av_len;

      if (r->Link.app.av_val)
      {
        argv[argc].av_val = ptr + 1;
        argv[argc++].av_len = 2;
        argv[argc].av_val = ptr + 5;
        ptr += sprintf(ptr, " -a \"%s\"", r->Link.app.av_val);
        argv[argc++].av_len = r->Link.app.av_len;
      }
      if (r->Link.flashVer.av_val)
      {
        argv[argc].av_val = ptr + 1;
        argv[argc++].av_len = 2;
        argv[argc].av_val = ptr + 5;
        ptr += sprintf(ptr, " -f \"%s\"", r->Link.flashVer.av_val);
        argv[argc++].av_len = r->Link.flashVer.av_len;
      }
      if (r->Link.swfUrl.av_val)
      {
        argv[argc].av_val = ptr + 1;
        argv[argc++].av_len = 2;
        argv[argc].av_val = ptr + 5;
        ptr += sprintf(ptr, " -W \"%s\"", r->Link.swfUrl.av_val);
        argv[argc++].av_len = r->Link.swfUrl.av_len;
      }
      if (r->Link.pageUrl.av_val)
      {
        argv[argc].av_val = ptr + 1;
        argv[argc++].av_len = 2;
        argv[argc].av_val = ptr + 5;
        ptr += sprintf(ptr, " -p \"%s\"", r->Link.pageUrl.av_val);
        argv[argc++].av_len = r->Link.pageUrl.av_len;
      }
      if (r->Link.extras.o_num)
      {
        ptr = rtmp_dumpAMF(&r->Link.extras, ptr, argv, &argc);
        AMF_Reset(&r->Link.extras);
      }
      argv[argc].av_val = ptr + 1;
      argv[argc++].av_len = 2;
      argv[argc].av_val = ptr + 5;
      ptr += sprintf(ptr, " -y \"%.*s\"",
                     r->Link.playpath.av_len, r->Link.playpath.av_val);
      argv[argc++].av_len = r->Link.playpath.av_len;

      av = r->Link.playpath;
      /* strip trailing URL parameters */
      q = memchr(av.av_val, '?', av.av_len);
      if (q)
      {
        if (q == av.av_val)
        {
          av.av_val++;
          av.av_len--;
        }
        else
        {
          av.av_len = q - av.av_val;
        }
      }
      /* strip leading slash components */
      for (p = av.av_val + av.av_len - 1; p >= av.av_val; p--)
        if (*p == '/')
        {
          p++;
          av.av_len -= p - av.av_val;
          av.av_val = p;
          break;
        }
      /* skip leading dot */
      if (av.av_val[0] == '.')
      {
        av.av_val++;
        av.av_len--;
      }
      file = malloc(av.av_len + 5);

      memcpy(file, av.av_val, av.av_len);
      file[av.av_len] = '\0';
      for (p = file; *p; p++)
        if (*p == ':')
          *p = '_';

      /* Add extension if none present */
      if (file[av.av_len - 4] != '.')
      {
        av.av_len += 4;
      }
      /* Always use flv extension, regardless of original */
      if (strcmp(file + av.av_len - 4, ".flv"))
      {
        strcpy(file + av.av_len - 4, ".flv");
      }
      argv[argc].av_val = ptr + 1;
      argv[argc++].av_len = 2;
      argv[argc].av_val = file;
      argv[argc].av_len = av.av_len;
      ptr += sprintf(ptr, " -o %s", file);
      now = RTMP_GetTime();
      if (now - server->filetime < DUPTIME && AVMATCH(&argv[argc], &server->filename))
      {
        printf("Duplicate request, skipping.\n");
        free(file);
      }
      else
      {
        printf("\n%s\n\n", cmd);
        fflush(stdout);
        server->filetime = now;
        free(server->filename.av_val);
        server->filename = argv[argc++];
        spawn_dumper(argc, argv, cmd);
      }

      free(cmd);
    }
    pc.m_body = server->connect;
    server->connect = NULL;
    RTMPPacket_Free(&pc);
    ret = 1;
    RTMP_SendCtrl(r, 0, 1, 0);
    rtmp_send_play_start(r);
    RTMP_SendCtrl(r, 1, 1, 0);
    rtmp_send_play_stop(r);
  }
  AMF_Reset(&obj);
  return ret;
}

static int rtmp_server_packet_handle(rtmp_server_t *server, RTMP *r, RTMPPacket *packet)
{
  int ret = 0;

  RTMP_Log(RTMP_LOGDEBUG, "%s, received packet type %02X, size %lu bytes", __FUNCTION__,
           packet->m_packetType, packet->m_nBodySize);

  switch (packet->m_packetType)
  {
  case 0x01:
    // chunk size
    //      HandleChangeChunkSize(r, packet);
    break;

  case 0x03:
    // bytes read report
    break;

  case 0x04:
    // ctrl
    //      HandleCtrl(r, packet);
    break;

  case 0x05:
    // server bw
    //      HandleServerBW(r, packet);
    break;

  case 0x06:
    // client bw
    //     HandleClientBW(r, packet);
    break;

  case 0x08:
    // audio data
    //RTMP_Log(RTMP_LOGDEBUG, "%s, received: audio %lu bytes", __FUNCTION__, packet.m_nBodySize);
    break;

  case 0x09:
    // video data
    //RTMP_Log(RTMP_LOGDEBUG, "%s, received: video %lu bytes", __FUNCTION__, packet.m_nBodySize);
    break;

  case 0x0F: // flex stream send
    break;

  case 0x10: // flex shared object
    break;

  case 0x11: // flex message
  {
    RTMP_Log(RTMP_LOGDEBUG, "%s, flex message, size %lu bytes, not fully supported",
             __FUNCTION__, packet->m_nBodySize);
    //RTMP_LogHex(packet.m_body, packet.m_nBodySize);

    // some DEBUG code
    /*RTMP_LIB_AMFObject obj;
	   int nRes = obj.Decode(packet.m_body+1, packet.m_nBodySize-1);
	   if(nRes < 0) {
	   RTMP_Log(RTMP_LOGERROR, "%s, error decoding AMF3 packet", __FUNCTION__);
	   //return;
	   }

	   obj.Dump(); */

    if (rtmp_server_invoke(server, r, packet, 1))
      RTMP_Close(r);
    break;
  }
  case 0x12:
    // metadata (notify)
    break;

  case 0x13:
    /* shared object */
    break;

  case 0x14:
    // invoke
    RTMP_Log(RTMP_LOGDEBUG, "%s, received: invoke %lu bytes", __FUNCTION__,
             packet->m_nBodySize);
    //RTMP_LogHex(packet.m_body, packet.m_nBodySize);

    if (rtmp_server_invoke(server, r, packet, 0))
      RTMP_Close(r);
    break;

  case 0x16:
    /* flv */
    break;
  default:
    RTMP_Log(RTMP_LOGDEBUG, "%s, unknown packet type received: 0x%02x", __FUNCTION__,
             packet->m_packetType);
#ifdef _DEBUG
    RTMP_LogHex(RTMP_LOGDEBUG, packet->m_body, packet->m_nBodySize);
#endif
  }
  return ret;
}