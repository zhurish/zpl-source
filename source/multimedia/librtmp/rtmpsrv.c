/*  Simple RTMP Server
 *  Copyright (C) 2009 Andrej Stepanchuk
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

/* This is just a stub for an RTMP server. It doesn't do anything
 * beyond obtaining the connection parameters from the client.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include <signal.h>
#include <getopt.h>

#include <assert.h>

#include "rtmp_sys.h"
#include "rtmplog.h"
#include "rtmpsrv.h"

#ifdef linux
#include <linux/netfilter_ipv4.h>
#endif

#ifndef WIN32
#include <sys/types.h>
#include <sys/wait.h>
#endif


#ifdef WIN32
#define InitSockets()	{\
	WORD version;			\
	WSADATA wsaData;		\
					\
	version = MAKEWORD(1,1);	\
	WSAStartup(version, &wsaData);	}

#define	CleanupSockets()	WSACleanup()
#else
#define InitSockets()
#define	CleanupSockets()
#endif


STREAMING_SERVER *rtmpServer = 0;	// server structure pointer

STREAMING_SERVER *startStreaming(const char *address, int port);
void stopStreaming(STREAMING_SERVER * server);

#define STR2AVAL(av,str)	av.av_val = str; av.av_len = strlen(av.av_val)

/* this request is formed from the parameters and used to initialize a new request,
 * thus it is a default settings list. All settings can be overriden by specifying the
 * parameters in the GET request. */
RTMP_REQUEST defaultRTMPRequest;

#ifdef _DEBUG
uint32_t debugTS = 0;

int pnum = 0;

FILE *netstackdump = NULL;
FILE *netstackdump_read = NULL;
#endif

#define SAVC(x) static const AVal av_##x = AVC(#x)

SAVC(app);
SAVC(connect);
SAVC(flashVer);
SAVC(swfUrl);
SAVC(pageUrl);
SAVC(tcUrl);
SAVC(fpad);
SAVC(capabilities);
SAVC(audioCodecs);
SAVC(videoCodecs);
SAVC(videoFunction);
SAVC(objectEncoding);
SAVC(_result);
SAVC(createStream);
SAVC(getStreamLength);
SAVC(play);
SAVC(fmsVer);
SAVC(mode);
SAVC(level);
SAVC(code);
SAVC(description);
SAVC(secureToken);

static int
SendConnectResult(RTMP *r, double txn)
{
  RTMPPacket packet;
  char pbuf[384], *pend = pbuf+sizeof(pbuf);
  AMFObject obj;
  AMFObjectProperty p, op;
  AVal av;

  packet.m_nChannel = 0x03;     // control channel (invoke)
  packet.m_headerType = 1; /* RTMP_PACKET_SIZE_MEDIUM; */
  packet.m_packetType = 0x14;   // INVOKE
  packet.m_nTimeStamp = 0;
  packet.m_nInfoField2 = 0;
  packet.m_hasAbsTimestamp = 0;
  packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

  char *enc = packet.m_body;
  enc = AMF_EncodeString(enc, pend, &av__result);
  enc = AMF_EncodeNumber(enc, pend, txn);
  *enc++ = AMF_OBJECT;

  STR2AVAL(av, "FMS/3,5,1,525");
  enc = AMF_EncodeNamedString(enc, pend, &av_fmsVer, &av);
  enc = AMF_EncodeNamedNumber(enc, pend, &av_capabilities, 31.0);
  enc = AMF_EncodeNamedNumber(enc, pend, &av_mode, 1.0);
  *enc++ = 0;
  *enc++ = 0;
  *enc++ = AMF_OBJECT_END;

  *enc++ = AMF_OBJECT;

  STR2AVAL(av, "status");
  enc = AMF_EncodeNamedString(enc, pend, &av_level, &av);
  STR2AVAL(av, "NetConnection.Connect.Success");
  enc = AMF_EncodeNamedString(enc, pend, &av_code, &av);
  STR2AVAL(av, "Connection succeeded.");
  enc = AMF_EncodeNamedString(enc, pend, &av_description, &av);
  enc = AMF_EncodeNamedNumber(enc, pend, &av_objectEncoding, r->m_fEncoding);
#if 0
  STR2AVAL(av, "58656322c972d6cdf2d776167575045f8484ea888e31c086f7b5ffbd0baec55ce442c2fb");
  enc = AMF_EncodeNamedString(enc, pend, &av_secureToken, &av);
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

static int
SendResultNumber(RTMP *r, double txn, double ID)
{
  RTMPPacket packet;
  char pbuf[256], *pend = pbuf+sizeof(pbuf);

  packet.m_nChannel = 0x03;     // control channel (invoke)
  packet.m_headerType = 1; /* RTMP_PACKET_SIZE_MEDIUM; */
  packet.m_packetType = 0x14;   // INVOKE
  packet.m_nTimeStamp = 0;
  packet.m_nInfoField2 = 0;
  packet.m_hasAbsTimestamp = 0;
  packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

  char *enc = packet.m_body;
  enc = AMF_EncodeString(enc, pend, &av__result);
  enc = AMF_EncodeNumber(enc, pend, txn);
  *enc++ = AMF_NULL;
  enc = AMF_EncodeNumber(enc, pend, ID);

  packet.m_nBodySize = enc - packet.m_body;

  return RTMP_SendPacket(r, &packet, FALSE);
}

SAVC(onStatus);
SAVC(status);
static const AVal av_NetStream_Play_Start = AVC("NetStream.Play.Start");
static const AVal av_Started_playing = AVC("Started playing");
static const AVal av_NetStream_Play_Stop = AVC("NetStream.Play.Stop");
static const AVal av_Stopped_playing = AVC("Stopped playing");
SAVC(details);
SAVC(clientid);

static int
SendPlayStart(RTMP *r)
{
  RTMPPacket packet;
  char pbuf[384], *pend = pbuf+sizeof(pbuf);

  packet.m_nChannel = 0x03;     // control channel (invoke)
  packet.m_headerType = 1; /* RTMP_PACKET_SIZE_MEDIUM; */
  packet.m_packetType = 0x14;   // INVOKE
  packet.m_nTimeStamp = 0;
  packet.m_nInfoField2 = 0;
  packet.m_hasAbsTimestamp = 0;
  packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

  char *enc = packet.m_body;
  enc = AMF_EncodeString(enc, pend, &av_onStatus);
  enc = AMF_EncodeNumber(enc, pend, 0);
  *enc++ = AMF_OBJECT;

  enc = AMF_EncodeNamedString(enc, pend, &av_level, &av_status);
  enc = AMF_EncodeNamedString(enc, pend, &av_code, &av_NetStream_Play_Start);
  enc = AMF_EncodeNamedString(enc, pend, &av_description, &av_Started_playing);
  enc = AMF_EncodeNamedString(enc, pend, &av_details, &r->Link.playpath);
  enc = AMF_EncodeNamedString(enc, pend, &av_clientid, &av_clientid);
  *enc++ = 0;
  *enc++ = 0;
  *enc++ = AMF_OBJECT_END;

  packet.m_nBodySize = enc - packet.m_body;
  return RTMP_SendPacket(r, &packet, FALSE);
}

static int
SendPlayStop(RTMP *r)
{
  RTMPPacket packet;
  char pbuf[384], *pend = pbuf+sizeof(pbuf);

  packet.m_nChannel = 0x03;     // control channel (invoke)
  packet.m_headerType = 1; /* RTMP_PACKET_SIZE_MEDIUM; */
  packet.m_packetType = 0x14;   // INVOKE
  packet.m_nTimeStamp = 0;
  packet.m_nInfoField2 = 0;
  packet.m_hasAbsTimestamp = 0;
  packet.m_body = pbuf + RTMP_MAX_HEADER_SIZE;

  char *enc = packet.m_body;
  enc = AMF_EncodeString(enc, pend, &av_onStatus);
  enc = AMF_EncodeNumber(enc, pend, 0);
  *enc++ = AMF_OBJECT;

  enc = AMF_EncodeNamedString(enc, pend, &av_level, &av_status);
  enc = AMF_EncodeNamedString(enc, pend, &av_code, &av_NetStream_Play_Stop);
  enc = AMF_EncodeNamedString(enc, pend, &av_description, &av_Stopped_playing);
  enc = AMF_EncodeNamedString(enc, pend, &av_details, &r->Link.playpath);
  enc = AMF_EncodeNamedString(enc, pend, &av_clientid, &av_clientid);
  *enc++ = 0;
  *enc++ = 0;
  *enc++ = AMF_OBJECT_END;

  packet.m_nBodySize = enc - packet.m_body;
  return RTMP_SendPacket(r, &packet, FALSE);
}

static void
spawn_dumper(int argc, AVal *av, char *cmd)
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
  while (waitpid(-1, NULL, WNOHANG) > 0);

  if (fork() == 0) {
    char **argv = malloc((argc+1) * sizeof(char *));
    int i;

    for (i=0; i<argc; i++) {
      argv[i] = av[i].av_val;
      argv[i][av[i].av_len] = '\0';
    }
    argv[i] = NULL;
    if ((i = execvp(argv[0], argv)))
      _exit(i);
  }
#endif
}

static int
countAMF(AMFObject *obj, int *argc)
{
  int i, len;

  for (i=0, len=0; i < obj->o_num; i++)
    {
      AMFObjectProperty *p = &obj->o_props[i];
      len += 4;
      (*argc)+= 2;
      if (p->p_name.av_val)
	len += 1;
      len += 2;
      if (p->p_name.av_val)
	len += p->p_name.av_len + 1;
      switch(p->p_type)
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
	  len += countAMF(&p->p_vu.p_object, argc);
	  (*argc) += 2;
	  break;
	case AMF_NULL:
	default:
	  break;
	}
    }
  return len;
}

static char *
dumpAMF(AMFObject *obj, char *ptr, AVal *argv, int *argc)
{
  int i, len, ac = *argc;
  const char opt[] = "NBSO Z";

  for (i=0, len=0; i < obj->o_num; i++)
    {
      AMFObjectProperty *p = &obj->o_props[i];
      argv[ac].av_val = ptr+1;
      argv[ac++].av_len = 2;
      ptr += sprintf(ptr, " -C ");
      argv[ac].av_val = ptr;
      if (p->p_name.av_val)
	*ptr++ = 'N';
      *ptr++ = opt[p->p_type];
      *ptr++ = ':';
      if (p->p_name.av_val)
	ptr += sprintf(ptr, "%.*s:", p->p_name.av_len, p->p_name.av_val);
      switch(p->p_type)
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
	  ptr = dumpAMF(&p->p_vu.p_object, ptr, argv, argc);
	  ac = *argc;
	  argv[ac].av_val = ptr+1;
	  argv[ac++].av_len = 2;
	  argv[ac].av_val = ptr+4;
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
int
ServeInvoke(STREAMING_SERVER *server, RTMP * r, RTMPPacket *packet, unsigned int offset)
{
  const char *body;
  unsigned int nBodySize;
  int ret = 0, nRes;

  body = packet->m_body + offset;
  nBodySize = packet->m_nBodySize - offset;

  if (body[0] != 0x02)		// make sure it is a string method name we start with
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

  if (AVMATCH(&method, &av_connect))
    {
      AMFObject cobj;
      AVal pname, pval;
      int i;

      server->connect = packet->m_body;
      packet->m_body = NULL;

      AMFProp_GetObject(AMF_GetProp(&obj, NULL, 2), &cobj);
      for (i=0; i<cobj.o_num; i++)
	{
	  pname = cobj.o_props[i].p_name;
	  pval.av_val = NULL;
	  pval.av_len = 0;
	  if (cobj.o_props[i].p_type == AMF_STRING)
	    pval = cobj.o_props[i].p_vu.p_aval;
	  if (AVMATCH(&pname, &av_app))
	    {
	      r->Link.app = pval;
	      pval.av_val = NULL;
	      if (!r->Link.app.av_val)
	        r->Link.app.av_val = "";
	      server->arglen += 6 + pval.av_len;
	      server->argc += 2;
	    }
	  else if (AVMATCH(&pname, &av_flashVer))
	    {
	      r->Link.flashVer = pval;
	      pval.av_val = NULL;
	      server->arglen += 6 + pval.av_len;
	      server->argc += 2;
	    }
	  else if (AVMATCH(&pname, &av_swfUrl))
	    {
	      r->Link.swfUrl = pval;
	      pval.av_val = NULL;
	      server->arglen += 6 + pval.av_len;
	      server->argc += 2;
	    }
	  else if (AVMATCH(&pname, &av_tcUrl))
	    {
	      r->Link.tcUrl = pval;
	      pval.av_val = NULL;
	      server->arglen += 6 + pval.av_len;
	      server->argc += 2;
	    }
	  else if (AVMATCH(&pname, &av_pageUrl))
	    {
	      r->Link.pageUrl = pval;
	      pval.av_val = NULL;
	      server->arglen += 6 + pval.av_len;
	      server->argc += 2;
	    }
	  else if (AVMATCH(&pname, &av_audioCodecs))
	    {
	      r->m_fAudioCodecs = cobj.o_props[i].p_vu.p_number;
	    }
	  else if (AVMATCH(&pname, &av_videoCodecs))
	    {
	      r->m_fVideoCodecs = cobj.o_props[i].p_vu.p_number;
	    }
	  else if (AVMATCH(&pname, &av_objectEncoding))
	    {
	      r->m_fEncoding = cobj.o_props[i].p_vu.p_number;
	    }
	}
      /* Still have more parameters? Copy them */
      if (obj.o_num > 3)
	{
	  int i = obj.o_num - 3;
	  r->Link.extras.o_num = i;
	  r->Link.extras.o_props = malloc(i*sizeof(AMFObjectProperty));
	  memcpy(r->Link.extras.o_props, obj.o_props+3, i*sizeof(AMFObjectProperty));
	  obj.o_num = 3;
	  server->arglen += countAMF(&r->Link.extras, &server->argc);
	}
      SendConnectResult(r, txn);
    }
  else if (AVMATCH(&method, &av_createStream))
    {
      SendResultNumber(r, txn, ++server->streamID);
    }
  else if (AVMATCH(&method, &av_getStreamLength))
    {
      SendResultNumber(r, txn, 10.0);
    }
  else if (AVMATCH(&method, &av_play))
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
	  argv[0].av_len = sizeof("rtmpdump")-1;
	  ptr += sprintf(ptr, "rtmpdump");
	  argc = 1;

	  argv[argc].av_val = ptr + 1;
	  argv[argc++].av_len = 2;
	  argv[argc].av_val = ptr + 5;
	  ptr += sprintf(ptr," -r \"%s\"", r->Link.tcUrl.av_val);
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
	  if (r->Link.extras.o_num) {
	    ptr = dumpAMF(&r->Link.extras, ptr, argv, &argc);
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
	  for (p=av.av_val+av.av_len-1; p>=av.av_val; p--)
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
	  file = malloc(av.av_len+5);

	  memcpy(file, av.av_val, av.av_len);
	  file[av.av_len] = '\0';
	  for (p=file; *p; p++)
	    if (*p == ':')
	      *p = '_';

	  /* Add extension if none present */
	  if (file[av.av_len - 4] != '.')
	    {
	      av.av_len += 4;
	    }
	  /* Always use flv extension, regardless of original */
	  if (strcmp(file+av.av_len-4, ".flv"))
	    {
	      strcpy(file+av.av_len-4, ".flv");
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
	  SendPlayStart(r);
	  RTMP_SendCtrl(r, 1, 1, 0);
	  SendPlayStop(r);
    }
  AMF_Reset(&obj);
  return ret;
}

int
ServePacket(STREAMING_SERVER *server, RTMP *r, RTMPPacket *packet)
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

    case 0x0F:			// flex stream send
      break;

    case 0x10:			// flex shared object
      break;

    case 0x11:			// flex message
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

	if (ServeInvoke(server, r, packet, 1))
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

      if (ServeInvoke(server, r, packet, 0))
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

int
controlServerThread(void *unused)
{
  char ich;
  while (1)
    {
      ich = getchar();
      switch (ich)
	{
	case 'q':
	  RTMP_LogPrintf("Exiting\n");
	  stopStreaming(rtmpServer);
	  exit(0);
	  break;
	default:
	  RTMP_LogPrintf("Unknown command \'%c\', ignoring\n", ich);
	}
    }
  TFRET();
}


void doServe(STREAMING_SERVER * server,	// server socket and state (our listening socket)
  int sockfd	// client connection socket
  )
{
  server->state = STREAMING_IN_PROGRESS;

  RTMP rtmp = { 0 };		/* our session with the real client */
  RTMPPacket packet = { 0 };

  // timeout for http requests
  fd_set fds;
  struct timeval tv;

  memset(&tv, 0, sizeof(struct timeval));
  tv.tv_sec = 5;

  FD_ZERO(&fds);
  FD_SET(sockfd, &fds);

  if (select(sockfd + 1, &fds, NULL, NULL, &tv) <= 0)
    {
      RTMP_Log(RTMP_LOGERROR, "Request timeout/select failed, ignoring request");
      goto quit;
    }
  else
    {
      RTMP_Init(&rtmp);
      rtmp.m_sb.sb_socket = sockfd;
      if (!RTMP_Serve(&rtmp))
	{
	  RTMP_Log(RTMP_LOGERROR, "Handshake failed");
	  goto cleanup;
	}
    }
  server->arglen = 0;
  while (RTMP_IsConnected(&rtmp) && RTMP_ReadPacket(&rtmp, &packet))
    {
      if (!RTMPPacket_IsReady(&packet))
	continue;
      ServePacket(server, &rtmp, &packet);
      RTMPPacket_Free(&packet);
    }

cleanup:
  RTMP_LogPrintf("Closing connection... ");
  RTMP_Close(&rtmp);
  /* Should probably be done by RTMP_Close() ... */
  rtmp.Link.playpath.av_val = NULL;
  rtmp.Link.tcUrl.av_val = NULL;
  rtmp.Link.swfUrl.av_val = NULL;
  rtmp.Link.pageUrl.av_val = NULL;
  rtmp.Link.app.av_val = NULL;
  rtmp.Link.flashVer.av_val = NULL;
  RTMP_LogPrintf("done!\n\n");

quit:
  if (server->state == STREAMING_IN_PROGRESS)
    server->state = STREAMING_ACCEPTING;

  return;
}

int
serverThread(void *arg)
{
  STREAMING_SERVER *server = arg;
  server->state = STREAMING_ACCEPTING;

  while (server->state == STREAMING_ACCEPTING)
    {
      struct sockaddr_in addr;
      socklen_t addrlen = sizeof(struct sockaddr_in);
      int sockfd =
	accept(server->socket, (struct sockaddr *) &addr, &addrlen);

      if (sockfd > 0)
	{
#ifdef linux
	  struct sockaddr_in dest;
	  char destch[16];
	  socklen_t destlen = sizeof(struct sockaddr_in);
	  getsockopt(sockfd, SOL_IP, SO_ORIGINAL_DST, &dest, &destlen);
	  strcpy(destch, inet_ntoa(dest.sin_addr));
	  RTMP_Log(RTMP_LOGDEBUG, "%s: accepted connection from %s to %s\n", __FUNCTION__,
	      inet_ntoa(addr.sin_addr), destch);
#else
	  RTMP_Log(RTMP_LOGDEBUG, "%s: accepted connection from %s\n", __FUNCTION__,
	      inet_ntoa(addr.sin_addr));
#endif
	  /* Create a new thread and transfer the control to that */
	  doServe(server, sockfd);
	  RTMP_Log(RTMP_LOGDEBUG, "%s: processed request\n", __FUNCTION__);
	}
      else
	{
	  RTMP_Log(RTMP_LOGERROR, "%s: accept failed", __FUNCTION__);
	}
    }
  server->state = STREAMING_STOPPED;
  TFRET();
}

STREAMING_SERVER *
startStreaming(const char *address, int port)
{
  struct sockaddr_in addr;
  int sockfd, tmp;
  STREAMING_SERVER *server;

  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == -1)
    {
      RTMP_Log(RTMP_LOGERROR, "%s, couldn't create socket", __FUNCTION__);
      return 0;
    }

  tmp = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
				(char *) &tmp, sizeof(tmp) );

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(address);	//htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) ==
      -1)
    {
      RTMP_Log(RTMP_LOGERROR, "%s, TCP bind failed for port number: %d", __FUNCTION__,
	  port);
      return 0;
    }

  if (listen(sockfd, 10) == -1)
    {
      RTMP_Log(RTMP_LOGERROR, "%s, listen failed", __FUNCTION__);
      closesocket(sockfd);
      return 0;
    }

  server = (STREAMING_SERVER *) calloc(1, sizeof(STREAMING_SERVER));
  server->socket = sockfd;

  ThreadCreate(serverThread, server);

  return server;
}

void
stopStreaming(STREAMING_SERVER * server)
{
  assert(server);

  if (server->state != STREAMING_STOPPED)
    {
      if (server->state == STREAMING_IN_PROGRESS)
	{
	  server->state = STREAMING_STOPPING;

	  // wait for streaming threads to exit
	  while (server->state != STREAMING_STOPPED)
	    msleep(1);
	}

      if (closesocket(server->socket))
	RTMP_Log(RTMP_LOGERROR, "%s: Failed to close listening socket, error %d",
	    GetSockError());

      server->state = STREAMING_STOPPED;
    }
}


void
sigIntHandler(int sig)
{
  RTMP_ctrlC = TRUE;
  RTMP_LogPrintf("Caught signal: %d, cleaning up, just a second...\n", sig);
  if (rtmpServer)
    stopStreaming(rtmpServer);
  signal(SIGINT, SIG_DFL);
}

int
main(int argc, char **argv)
{
  int nStatus = RD_SUCCESS;

  // http streaming server
  char DEFAULT_HTTP_STREAMING_DEVICE[] = "0.0.0.0";	// 0.0.0.0 is any device

  char *rtmpStreamingDevice = DEFAULT_HTTP_STREAMING_DEVICE;	// streaming device, default 0.0.0.0
  int nRtmpStreamingPort = 1935;	// port

  RTMP_LogPrintf("RTMP Server %s\n", RTMPDUMP_VERSION);
  RTMP_LogPrintf("(c) 2010 Andrej Stepanchuk, Howard Chu; license: GPL\n\n");

  RTMP_debuglevel = RTMP_LOGINFO;

  if (argc > 1 && !strcmp(argv[1], "-z"))
    RTMP_debuglevel = RTMP_LOGALL;

  // init request
  memset(&defaultRTMPRequest, 0, sizeof(RTMP_REQUEST));

  defaultRTMPRequest.rtmpport = -1;
  defaultRTMPRequest.protocol = RTMP_PROTOCOL_UNDEFINED;
  defaultRTMPRequest.bLiveStream = FALSE;	// is it a live stream? then we can't seek/resume

  defaultRTMPRequest.timeout = 300;	// timeout connection afte 300 seconds
  defaultRTMPRequest.bufferTime = 20 * 1000;


  signal(SIGINT, sigIntHandler);
#ifndef WIN32
  signal(SIGPIPE, SIG_IGN);
#endif

#ifdef _DEBUG
  netstackdump = fopen("netstackdump", "wb");
  netstackdump_read = fopen("netstackdump_read", "wb");
#endif

  InitSockets();

  // start text UI
  ThreadCreate(controlServerThread, 0);

  // start http streaming
  if ((rtmpServer =
       startStreaming(rtmpStreamingDevice, nRtmpStreamingPort)) == 0)
    {
      RTMP_Log(RTMP_LOGERROR, "Failed to start RTMP server, exiting!");
      return RD_FAILED;
    }
  RTMP_LogPrintf("Streaming on rtmp://%s:%d\n", rtmpStreamingDevice,
	    nRtmpStreamingPort);

  while (rtmpServer->state != STREAMING_STOPPED)
    {
      sleep(1);
    }
  RTMP_Log(RTMP_LOGDEBUG, "Done, exiting...");

  CleanupSockets();

#ifdef _DEBUG
  if (netstackdump != 0)
    fclose(netstackdump);
  if (netstackdump_read != 0)
    fclose(netstackdump_read);
#endif
  return nStatus;
}


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
 
#include <librtmp/rtmp.h>
 
// 大小端字节序转换
#define HTON16(x) ( (x >> 8 & 0x00FF) | (x << 8 & 0xFF00) )
#define HTON24(x) ( (x >> 16 & 0x0000FF) | (x & 0x00FF00) | (x << 16 & 0xFF0000) )
#define HTON32(x) ( (x >> 24 & 0x000000FF) | (x >> 8 & 0x0000FF00) | (x << 8 & 0x00FF0000) | (x << 24 & 0xFF000000) )
#define HTONTIME(x) ( (x >> 16 & 0x000000FF) | (x & 0x0000FF00) | (x << 16 & 0x00FF0000) | (x & 0xFF000000) )
// 从文件读取指定字节
bool ReadFP(char* pBuf, int nSize, FILE* pFile)
{
    return (fread(pBuf, 1, nSize, pFile) == nSize);
}
 
// 从文件读取1个字节整数
bool ReadU8(uint8_t* u8, FILE* fp)
{
    return ReadFP((char*)u8, 1, fp);
}
 
// 从文件读取2个字节整数
bool ReadU16(uint16_t* u16, FILE* fp)
{
    if (!ReadFP((char*)u16, 2, fp))
        return false;
 
    *u16 = HTON16(*u16);
    return true;
}
 
// 从文件读取3个字节整数
bool ReadU24(uint32_t* u24, FILE* fp)
{
    if (!ReadFP((char*)u24, 3, fp))
        return false;
 
    *u24 = HTON24(*u24);
    return true;
}
 
// 从文件读取4个字节整数
bool ReadU32(uint32_t* u32, FILE* fp)
{
    if (!ReadFP((char*)u32, 4, fp))
        return false;
 
    *u32 = HTON32(*u32);
    return true;
}
 
// 从文件读取4个字节时间戳
bool ReadTime(uint32_t* utime, FILE* fp)
{
    if (!ReadFP((char*)utime, 4, fp))
        return false;
 
    *utime = HTONTIME(*utime);
    return true;
}
 
// 从文件预读1个字节整数
bool PeekU8(uint8_t* u8, FILE* fp)
{
    if (!ReadFP((char*)u8, 1, fp))
        return false;
 
    fseek(fp, -1, SEEK_CUR);
    return true;
}
 
int main(int argc, char* argv[])
{
    FILE* pFile = fopen("1.flv", "rb");
 
    // 初使化RTMP上下文
    RTMP* pRTMP = RTMP_Alloc();
    RTMP_Init(pRTMP);
 
    // 设置推流地址
    pRTMP->Link.timeout = 10;
    RTMP_SetupURL(pRTMP, (char*)"rtmp://127.0.0.1:1935/live/a");
 
    // 开启推流标志
    RTMP_EnableWrite(pRTMP);
 
    // 连接服务器
    bool b = RTMP_Connect(pRTMP, NULL);
    if (!b)
    {
        printf("connect failed! \n");
        return -1;
    }
        
    // 连接流地址
    b = RTMP_ConnectStream(pRTMP, 0);
    if (!b)
    {
        printf("connect stream failed! \n");
        return -1;
    }
 
    // 跳过FLV文件头的13个字节
    fseek(pFile, 9, SEEK_SET);
    fseek(pFile, 4, SEEK_CUR);
 
    // 初使化RTMP报文
    RTMPPacket packet;
    RTMPPacket_Reset(&packet);
    packet.m_body = NULL;
    packet.m_chunk = NULL;
 
    packet.m_nInfoField2 = pRTMP->m_stream_id;
 
    uint32_t starttime = RTMP_GetTime();
 
    while (true)
    {
        // 读取TAG头
 
        uint8_t type = 0;
        if (!ReadU8(&type, pFile))
            break;
 
        uint32_t datalen = 0;
        if (!ReadU24(&datalen, pFile))
            break;
 
        uint32_t timestamp = 0;
        if (!ReadTime(&timestamp, pFile))
            break;
 
        uint32_t streamid = 0;
        if (!ReadU24(&streamid, pFile))
            break;
 
/*
        // 跳过0x12 Script
        if (type != 0x08 && type != 0x09)
        {
            fseek(pFile, datalen + 4, SEEK_CUR);
            continue;
        }
*/
 
        RTMPPacket_Alloc(&packet, datalen);
 
        if (fread(packet.m_body, 1, datalen, pFile) != datalen)
            break;
 
        // 组织报文并发送
        packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
        packet.m_packetType = type;
        packet.m_hasAbsTimestamp = 0;
        packet.m_nChannel = 6;
        packet.m_nTimeStamp = timestamp;
        packet.m_nBodySize = datalen;
 
        if (!RTMP_SendPacket(pRTMP, &packet, 0))
        {
            printf("Send Error! \n");
            break;
        }
 
        printf("send type:[%d] timestamp:[%d] datasize:[%d] \n", type, timestamp, datalen);
 
        // 跳过PreTag
        uint32_t pretagsize = 0;
        if (!ReadU32(&pretagsize, pFile))
            break;
 
        // 延时，避免发送太快
        uint32_t timeago = (RTMP_GetTime() - starttime);
        if (timestamp > 1000 && timeago < timestamp - 1000)
        {
            printf("sleep...\n");
            usleep(100000);
        }
 
        RTMPPacket_Free(&packet);
    }
 
    // 关闭连接，释放RTMP上下文
    RTMP_Close(pRTMP);
    RTMP_Free(pRTMP);
 
    fclose(pFile);
 
    return 0;
}