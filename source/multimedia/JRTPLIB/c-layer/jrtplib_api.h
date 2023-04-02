#ifndef __JRTPLIB_API_H__
#define __JRTPLIB_API_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct jrtp_session_s jrtp_session_t;

extern jrtp_session_t * jrtp_session_create(void);
extern int jrtp_session_destroy(jrtp_session_t *);
extern int jrtp_session_start(jrtp_session_t *);
extern int jrtp_session_stop(jrtp_session_t *);
extern int jrtp_session_destination(jrtp_session_t *jrtpsess, char *address, uint16_t rtpport, uint16_t rtcpport);
extern int jrtp_session_local_set(jrtp_session_t *jrtpsess, char *address, uint16_t rtpport, uint16_t rtcpport);
extern int jrtp_session_payload_set(jrtp_session_t *jrtpsess, int pt, int clock);
extern int jrtp_session_overtcp_set(jrtp_session_t *jrtpsess, int istcp);
extern int jrtp_session_framerate_set(jrtp_session_t *jrtpsess, int framerate);

extern int jrtp_session_rtpsock(jrtp_session_t *jrtpsess);
extern int jrtp_session_rtcpsock(jrtp_session_t *jrtpsess);
extern int jrtp_session_rtpdelay(jrtp_session_t *jrtpsess);
extern int jrtp_session_rtcpdelay(jrtp_session_t *jrtpsess);
extern int jrtp_session_rtprtcp_sched(jrtp_session_t *jrtpsess);

extern int jrtp_session_sendto(jrtp_session_t *jrtpsess, const void *data, size_t len,
	                uint8_t pt, bool mark, uint32_t timestampinc);

extern int jrtp_session_recvfrom(jrtp_session_t *);



#ifdef __cplusplus
}
#endif


#endif /* __JRTPLIB_API_H__ */