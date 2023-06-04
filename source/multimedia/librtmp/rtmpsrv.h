#ifndef __RTMP_SRV_H__
#define __RTMP_SRV_H__

#define RD_SUCCESS		0
#define RD_FAILED		1
#define RD_INCOMPLETE		2

#define PACKET_SIZE 1024*1024


#define DUPTIME	5000	/* interval we disallow duplicate requests, in msec */

enum
{
  STREAMING_ACCEPTING,
  STREAMING_IN_PROGRESS,
  STREAMING_STOPPING,
  STREAMING_STOPPED
};

typedef struct
{
  char *hostname;
  int rtmpport;
  int protocol;
  int bLiveStream;		// is it a live stream? then we can't seek/resume

  long int timeout;		// timeout connection afte 300 seconds
  uint32_t bufferTime;

  char *rtmpurl;
  AVal playpath;
  AVal swfUrl;
  AVal tcUrl;
  AVal pageUrl;
  AVal app;
  AVal auth;
  AVal swfHash;
  AVal flashVer;
  AVal subscribepath;
  uint32_t swfSize;

  uint32_t dStartOffset;
  uint32_t dStopOffset;
  uint32_t nTimeStamp;
} RTMP_REQUEST;



typedef struct
{
  int socket;
  int state;
  int streamID;
  int arglen;
  int argc;
  uint32_t filetime;	/* time of last download we started */
  AVal filename;	/* name of last download */
  char *connect;

RTMP_REQUEST defaultRTMPRequest;
#ifdef _DEBUG
    uint32_t debugTS = 0;

    int pnum = 0;

    FILE *netstackdump = NULL;
    FILE *netstackdump_read = NULL;
#endif

#define LLSAVC(x) AVal av_##x

LLSAVC(app);
LLSAVC(connect);
LLSAVC(flashVer);
LLSAVC(swfUrl);
LLSAVC(pageUrl);
LLSAVC(tcUrl);
LLSAVC(fpad);
LLSAVC(capabilities);
LLSAVC(audioCodecs);
LLSAVC(videoCodecs);
LLSAVC(videoFunction);
LLSAVC(objectEncoding);
LLSAVC(_result);
LLSAVC(createStream);
LLSAVC(getStreamLength);
LLSAVC(play);
LLSAVC(fmsVer);
LLSAVC(mode);
LLSAVC(level);
LLSAVC(code);
LLSAVC(description);
LLSAVC(secureToken);
LLSAVC(onStatus);
LLSAVC(status);
AVal av_NetStream_Play_Start;
AVal av_Started_playing;
AVal av_NetStream_Play_Stop;
AVal av_Stopped_playing;
LLSAVC(details);
LLSAVC(clientid);
} STREAMING_SERVER;



#endif /*__RTMP_SRV_H__*/