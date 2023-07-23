#ifndef __JRTPLIB_API_H__
#define __JRTPLIB_API_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "rtpdefines.h"
#include "jrtp_payloadtype.h"
#include "jrtp_rtpprofile.h"

#define JRTP_PACKET_SIZE_MAX	1480

typedef struct jrtp_session_s jrtp_session_t;

extern jrtp_session_t * jrtp_session_alloc(void);
extern int jrtp_session_destroy(jrtp_session_t *);
extern int jrtp_session_create(jrtp_session_t *);
extern int jrtp_session_isactive(jrtp_session_t *jrtpsess);
extern int jrtp_session_destination_add(jrtp_session_t *jrtpsess, char *address, u_int16_t rtpport, u_int16_t rtcpport);
extern int jrtp_session_destination_del(jrtp_session_t *jrtpsess, char *address, u_int16_t rtpport, u_int16_t rtcpport);
extern int jrtp_session_multicast_add(jrtp_session_t *jrtpsess, char *address, u_int16_t rtpport, char *local);
extern int jrtp_session_multicast_del(jrtp_session_t *jrtpsess, char *address, u_int16_t rtpport, char *local);

extern int jrtp_session_local_set(jrtp_session_t *jrtpsess, char *address, u_int16_t rtpport, u_int16_t rtcpport);

extern int jrtp_session_payload_set(jrtp_session_t *jrtpsess, int pt, int clock);
extern int jrtp_session_overtcp_set(jrtp_session_t *jrtpsess, int istcp);
extern int jrtp_session_framerate_set(jrtp_session_t *jrtpsess, int framerate);

extern int jrtp_session_stop(jrtp_session_t *jrtpsess);
extern int jrtp_session_start(jrtp_session_t *jrtpsess);

extern int jrtp_session_rtpsock(jrtp_session_t *jrtpsess);
extern int jrtp_session_rtcpsock(jrtp_session_t *jrtpsess);
extern int jrtp_session_rtpdelay(jrtp_session_t *jrtpsess);
extern int jrtp_session_rtcpdelay(jrtp_session_t *jrtpsess);
extern int jrtp_session_rtprtcp_sched(jrtp_session_t *jrtpsess);

extern int jrtp_session_sendto(jrtp_session_t *jrtpsess, const void *data, size_t len,
	                u_int8_t pt, bool mark, u_int32_t timestampinc);

extern int jrtp_session_recvfrom(jrtp_session_t *);

extern int jrtp_session_event_loop(void *);
  
extern int jrtplib_api_test(void);

#ifdef __cplusplus
}
#endif


#endif /* __JRTPLIB_API_H__ */