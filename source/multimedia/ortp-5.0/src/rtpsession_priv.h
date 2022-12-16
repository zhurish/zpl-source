/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of oRTP.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef rtpsession_priv_h
#define rtpsession_priv_h

#ifdef __cplusplus
extern "C"
{
#endif
#include <ortp/ortp_list.h>
#include <ortp/payloadtype.h>
#include <ortp/rtpprofile.h>
#include <ortp/sessionset.h>
#include <ortp/rtp.h>
#include <ortp/rtcp.h>
#include <ortp/str_utils.h>
#include <ortp/extremum.h>
//#include <ortp/rtpsignaltable.h>
//#include <ortp/event.h>
#include <ortp/rtp_queue.h>



#define IP_UDP_OVERHEAD (20 + 8)
#define IP6_UDP_OVERHEAD (40 + 8)

#define RTCP_XR_GMIN 16 /* Recommended value of Gmin from RFC3611, section 4.7.6 */

typedef enum {
	RTP_SESSION_RECV_SYNC=1,	/* the rtp session is synchronising in the incoming stream */
	RTP_SESSION_FIRST_PACKET_DELIVERED=1<<1,
	RTP_SESSION_SCHEDULED=1<<2,/* the scheduler controls this session*/
	RTP_SESSION_BLOCKING_MODE=1<<3, /* in blocking mode */
	RTP_SESSION_RECV_NOT_STARTED=1<<4,	/* the application has not started to try to recv */
	RTP_SESSION_SEND_NOT_STARTED=1<<5,  /* the application has not started to send something */
	RTP_SESSION_IN_SCHEDULER=1<<6,	/* the rtp session is in the scheduler list */
	RTP_SESSION_USING_EXT_SOCKETS=1<<7, /* the session is using externaly supplied sockets */
	RTP_SOCKET_CONNECTED=1<<8,
	RTCP_SOCKET_CONNECTED=1<<9,
	RTP_SESSION_USING_TRANSPORT=1<<10,
	RTCP_OVERRIDE_LOST_PACKETS=1<<11,
	RTCP_OVERRIDE_JITTER=1<<12,
	RTCP_OVERRIDE_DELAY=1<<13,
	RTP_SESSION_RECV_SEQ_INIT=1<<14,
	RTP_SESSION_FLUSH=1<<15,
    RTP_SESSION_SOCKET_REFRESH_REQUESTED=1<<16,
    RTP_SESSION_USING_OVER_RTSPTCP_SOCKETS=1<<17,
    RTP_SESSION_USING_OVER_TCP_SOCKETS=1<<18,
}RtpSessionFlags;

typedef enum {
	RTP_SESSION_RECVONLY,
	RTP_SESSION_SENDONLY,
	RTP_SESSION_SENDRECV
} RtpSessionMode;

typedef struct _RtpTransport RtpTransport;
typedef struct _RtpTransportModifier RtpTransportModifier;
typedef struct _RtpSession RtpSession;
typedef struct _RtpBundle RtpBundle;


typedef enum _OrtpJitterBufferAlgorithm {
	OrtpJitterBufferBasic,
	OrtpJitterBufferRecursiveLeastSquare,
} OrtpJitterBufferAlgorithm;

/*! Jitter buffer parameters
*/
typedef struct _JBParameters{
	int min_size; /*(adaptive=TRUE only) maximum dynamic delay to be added to incoming packets (ms) */
	int nom_size; /*(adaptive=TRUE only) initial dynamic delay to be added to incoming packets (ms) */
	int max_size; /*(adaptive=TRUE only) minimum dynamic delay to be added to incoming packets (ms) */
	bool_t adaptive; /*either a dynamic buffer should be used or not to compensate bursts */
	bool_t enabled; /*whether jitter buffer is enabled*/
	bool_t pad[2]; /*(dev only) alignment pad: insert your bool_t here*/
	int max_packets; /**max number of packets allowed to be queued in the jitter buffer */
	OrtpJitterBufferAlgorithm buffer_algorithm;
	int refresh_ms; /* (adaptive=TRUE only) dynamic buffer size update frequency (ms) */
	int ramp_threshold; /*(adaptive=TRUE, algo=RLS only) Percentage in [0;100] threshold between current jitter and previous jitter to enable smooth ramp*/
	int ramp_step_ms; /*(adaptive=TRUE, algo=RLS only) In smooth ramp, how much we should reduce jitter size on each step*/
	int ramp_refresh_ms; /*(adaptive=TRUE, algo=RLS only) In smooth ramp, frequency of step*/
} JBParameters;

typedef struct _JitterControl
{
	JBParameters params;
	unsigned int count; /* number of packets handled in jitter_control_new_packet. Used internally only. */
	int jitt_comp_ts; /* the nominal jitter buffer size converted in rtp time (same unit as timestamp) */
	int adapt_jitt_comp_ts;
	int32_t clock_offset_ts; /*offset difference between local and distant clock, in timestamp units*/
	int32_t prev_clock_offset_ts;
	int32_t olddiff;
	float jitter;
	float inter_jitter;	/* interarrival jitter as defined in the RFC */
	float jitter_buffer_mean_size; /*effective size (fullness) of jitter buffer*/
	int corrective_step;
	int corrective_slide;
	uint64_t cum_jitter_buffer_size; /*in timestamp units*/
	unsigned int cum_jitter_buffer_count; /*used for computation of jitter buffer size*/
	int clock_rate;
	uint32_t adapt_refresh_prev_ts; /*last time we refreshed the buffer*/
	OrtpExtremum max_ts_deviation; /*maximum difference between packet and expected timestamps */
	OrtpKalmanRLS kalman_rls;
	double capped_clock_ratio;
	uint32_t last_log_ts;
	uint32_t local_ts_start;
	uint32_t remote_ts_start;
	uint32_t diverged_start_ts;
	bool_t is_diverging;
	bool_t jb_size_updated;
	bool_t pad[2];
} JitterControl;

#ifndef _USER_ORTP_ADEP
typedef struct _WaitPoint
{
	ortp_mutex_t lock;
	ortp_cond_t  cond;
	uint32_t time;
	bool_t wakeup;
} WaitPoint;
void wait_point_init(WaitPoint *wp);
void wait_point_uninit(WaitPoint *wp);
#endif



struct _RtpTransportModifier
{
	void *data;
	RtpSession *session;//<back pointer to the owning session, set by oRTP
	RtpTransport *transport;//<back point to the owning transport, set by oRTP
	int  (*t_process_on_send)(RtpTransportModifier *t, mblk_t *msg);
	int  (*t_process_on_receive)(RtpTransportModifier *t, mblk_t *msg);
	void  (*t_process_on_schedule)(RtpTransportModifier *t); /*invoked each time rtp_session_recvm is called even is no message are available*/
	/**
	 * Mandatory callback responsible of freeing the #_RtpTransportModifier AND the pointer.
	 * @param[in] transport #_RtpTransportModifier object to free.
	 */
	void  (*t_destroy)(RtpTransportModifier *transport);
} ;

struct _RtpTransport
{
	void *data;
	RtpSession *session;//<back pointer to the owning session, set by oRTP
	ortp_socket_t (*t_getsocket)(RtpTransport *t);
	int  (*t_sendto)(RtpTransport *t, mblk_t *msg , int flags, const struct ipstack_sockaddr *to, socklen_t tolen);
	int  (*t_recvfrom)(RtpTransport *t, mblk_t *msg, int flags, struct ipstack_sockaddr *from, socklen_t *fromlen);
	void  (*t_close)(RtpTransport *transport);
	/**
	 * Mandatory callback responsible of freeing the #_RtpTransport object AND the pointer.
	 * @param[in] transport #_RtpTransport object to free.
	 */
	void  (*t_destroy)(RtpTransport *transport);
}  ;




typedef enum _OrtpNetworkSimulatorMode{
	OrtpNetworkSimulatorInvalid=-1,
	OrtpNetworkSimulatorInbound,/**<simulation is applied when receiving packets*/
	OrtpNetworkSimulatorOutbound, /**<simulation is applied to sent packets*/
	OrtpNetworkSimulatorOutboundControlled /**<simulation is applied to sent packets according to sent timestamps
				set in the timestamps field of mblk_t, which is defined only with -DORTP_TIMESTAMP */
}OrtpNetworkSimulatorMode;

/**
 * Structure describing the network simulator parameters
**/
typedef struct _OrtpNetworkSimulatorParams{
	int enabled; /**<Whether simulation is enabled or off.*/
	float max_bandwidth; /**<IP bandwidth, in bit/s.
						This limitation is applied after loss are simulated, so incoming bandwidth
						is NOT socket bandwidth, but after-loss-simulation bandwidth e.g with 50% loss, the bandwidth will be 50% reduced*/
	int max_buffer_size; /**<Max number of bit buffered before being discarded*/
	float loss_rate; /**<Percentage of lost packets*/
	uint32_t latency; /**<Packet transmission delay, in ms*/
	float consecutive_loss_probability;/**< a probability of having a subsequent loss after a loss occurred, in a 0-1 range. Useful to simulate burst of lost packets*/
	float jitter_burst_density; /**<density of gap/bursts events. A value of 1 means one gap/burst per second approximately*/
	float jitter_strength; /**<percentage of max_bandwidth artificially consumed during bursts events*/
	bool_t rtp_only; /**True for only RTP packet loss, False for both RTP and RTCP */
	bool_t pad[3];
	OrtpNetworkSimulatorMode mode; /**<whether simulation is applied to inbound or outbound stream.*/
}OrtpNetworkSimulatorParams;

typedef struct _OrtpNetworkSimulatorCtx{
	OrtpNetworkSimulatorParams params;
	int bit_budget;
	int qsize;
	rtp_queue_t q;/*queue used for simulating bandwidth limit*/
	rtp_queue_t latency_q;
	rtp_queue_t send_q; /*used only for OrtpNetworkSimulatorOutbound direction*/
	struct timeval last_check;
	uint64_t last_jitter_event;
	int consecutive_drops;
	int drops_to_ignore;
	int drop_by_congestion;
	int drop_by_loss;
	int total_count; /*total number of packets gone through the simulator*/
	ortp_mutex_t mutex;
	ortp_thread_t thread;
	bool_t in_jitter_event;
	bool_t thread_started;
}OrtpNetworkSimulatorCtx;

typedef struct OrtpRtcpSendAlgorithm {
	uint64_t tn; /* Time of the next scheduled RTCP RR transmission in milliseconds. */
	uint64_t tp; /* Time of the last scheduled RTCP RR transmission in milliseconds. */
	uint64_t t_rr_last; /* Time of the last regular RTCP packet sent in milliseconds. */
	uint32_t T_rr; /* Interval for the scheduling of the next regular RTCP packet. */
	uint32_t T_max_fb_delay; /* Interval within which a feeback message is considered to be useful to the sender. */
	uint32_t T_rr_interval; /* Minimal interval to be used between regular RTCP packets. */
	uint32_t T_rr_current_interval;
	uint32_t Tmin; /* Minimal interval between RTCP packets. */
	float avg_rtcp_size;
	mblk_t *fb_packets;
	bool_t initialized; /* Whether the RTCP send algorithm is fully initialized. */
	bool_t initial;
	bool_t allow_early;
	bool_t tmmbr_scheduled;
	bool_t tmmbn_scheduled;
} OrtpRtcpSendAlgorithm;

typedef struct OrtpRtcpFbConfiguration {
	bool_t generic_nack_enabled;
	bool_t tmmbr_enabled;
} OrtpRtcpFbConfiguration;

#define ORTP_RTCP_XR_UNAVAILABLE_PARAMETER 127

typedef enum {
	OrtpRtcpXrNoPlc,
	OrtpRtcpXrSilencePlc,
	OrtpRtcpXrEnhancedPlc
} OrtpRtcpXrPlcStatus;

typedef OrtpRtcpXrPlcStatus (*OrtpRtcpXrPlcCallback)(void *userdata);
typedef int (*OrtpRtcpXrSignalLevelCallback)(void *userdata);
typedef int (*OrtpRtcpXrNoiseLevelCallback)(void *userdata);
typedef float (*OrtpRtcpXrAverageQualityIndicatorCallback)(void *userdata);

typedef struct OrtpRtcpXrMediaCallbacks {
	OrtpRtcpXrPlcCallback plc;
	OrtpRtcpXrSignalLevelCallback signal_level;
	OrtpRtcpXrNoiseLevelCallback noise_level;
	OrtpRtcpXrAverageQualityIndicatorCallback average_qi;
	OrtpRtcpXrAverageQualityIndicatorCallback average_lq_qi;
	void *userdata;
} OrtpRtcpXrMediaCallbacks;

typedef enum {
	OrtpRtcpXrRcvrRttNone,
	OrtpRtcpXrRcvrRttAll,
	OrtpRtcpXrRcvrRttSender
} OrtpRtcpXrRcvrRttMode;

typedef enum {
	OrtpRtcpXrStatSummaryNone = 0,
	OrtpRtcpXrStatSummaryLoss = (1 << 7),
	OrtpRtcpXrStatSummaryDup = (1 << 6),
	OrtpRtcpXrStatSummaryJitt = (1 << 5),
	OrtpRtcpXrStatSummaryTTL = (1 << 3),
	OrtpRtcpXrStatSummaryHL = (1 << 4)
} OrtpRtcpXrStatSummaryFlag;

typedef struct OrtpRtcpXrConfiguration {
	bool_t enabled;
	bool_t stat_summary_enabled;
	bool_t voip_metrics_enabled;
	bool_t pad;
	OrtpRtcpXrRcvrRttMode rcvr_rtt_mode;
	int rcvr_rtt_max_size;
	OrtpRtcpXrStatSummaryFlag stat_summary_flags;
} OrtpRtcpXrConfiguration;

typedef struct OrtpRtcpXrStats {
	uint32_t last_rcvr_rtt_ts;	/* NTP timestamp (middle 32 bits) of last received XR rcvr-rtt */
	struct timeval last_rcvr_rtt_time;	/* Time at which last XR rcvr-rtt was received  */
	uint16_t rcv_seq_at_last_stat_summary;	/* Received sequence number at last XR stat-summary sent */
	uint32_t rcv_since_last_stat_summary;	/* The number of packets received since last XR stat-summary was sent */
	uint32_t dup_since_last_stat_summary;	/* The number of duplicate packets received since last XR stat-summary was sent */
	uint32_t min_jitter_since_last_stat_summary;	/* The minimum value of jitter since last XR stat-summary was sent */
	uint32_t max_jitter_since_last_stat_summary;	/* The maximum value of jitter since last XR stat-summary was sent */
	double olds_jitter_since_last_stat_summary;
	double oldm_jitter_since_last_stat_summary;
	double news_jitter_since_last_stat_summary;
	double newm_jitter_since_last_stat_summary;
	int64_t last_jitter_diff_since_last_stat_summary;
	double olds_ttl_or_hl_since_last_stat_summary;
	double oldm_ttl_or_hl_since_last_stat_summary;
	double news_ttl_or_hl_since_last_stat_summary;
	double newm_ttl_or_hl_since_last_stat_summary;
	uint8_t min_ttl_or_hl_since_last_stat_summary;	/* The minimum value of TTL/HL since last XR stat-summary was sent */
	uint8_t max_ttl_or_hl_since_last_stat_summary;	/* The maximum value of TTL/HL since last XR stat-summary was sent */
	uint32_t first_rcv_seq;
	uint32_t last_rcv_seq;
	uint32_t rcv_count;
	uint32_t discarded_count;
} OrtpRtcpXrStats;

typedef struct OrtpRtcpTmmbrInfo {
	mblk_t *sent;
	mblk_t *received;
} OrtpRtcpTmmbrInfo;


typedef struct _OrtpStream {
	ortp_socket_t socket;
	int sockfamily;
	int loc_port;
    OrtpAddress rem_addr;
    OrtpAddress loc_addr;
    OrtpAddress used_loc_addr; /*Address used to redirect packets from this source*/
	RtpTransport *tr;
	OrtpBwEstimator recv_bw_estimator;
	struct timeval send_bw_start; /* used for bandwidth estimation */
	struct timeval recv_bw_start; /* used for bandwidth estimation */
	unsigned int sent_bytes; /* used for bandwidth estimation */
	unsigned int recv_bytes; /* used for bandwidth estimation */
	float upload_bw;
	float download_bw;
	float average_upload_bw;
	float average_download_bw;
    ortp_list_t *aux_destinations; /*list of OrtpAddress */
	msgb_allocator_t allocator;
} OrtpStream;

typedef struct _RtpStream
{
	OrtpStream gs;
	int time_jump;
	uint32_t ts_jump;
	rtp_queue_t rq;
	rtp_queue_t tev_rq;
	void *QoSHandle;
	unsigned long QoSFlowID;
	JitterControl jittctl;
	uint32_t snd_time_offset;/*the scheduler time when the application send its first timestamp*/
	uint32_t snd_ts_offset;	/* the first application timestamp sent by the application */
	uint32_t snd_rand_offset;	/* a random number added to the user offset to make the stream timestamp*/
	uint32_t snd_last_ts;	/* the last stream timestamp sent */
	uint16_t snd_last_nack;	/* the last nack sent when in immediate mode */
	uint32_t rcv_time_offset; /*the scheduler time when the application ask for its first timestamp*/
	uint32_t rcv_ts_offset;  /* the first stream timestamp */
	uint32_t rcv_query_ts_offset;	/* the first user timestamp asked by the application */
	uint32_t rcv_last_ts;	/* the last stream timestamp got by the application */
	uint16_t rcv_last_seq;	/* the last stream sequence number got by the application*/
	uint16_t pad;
	uint32_t rcv_last_app_ts; /* the last application timestamp asked by the application */
	uint32_t rcv_last_ret_ts; /* the timestamp of the last sample returned (only for continuous audio)*/
	uint32_t hwrcv_extseq; /* last received on socket extended sequence number */
	uint32_t hwrcv_seq_at_last_SR;
	uint32_t hwrcv_since_last_SR;
	uint32_t last_rcv_SR_ts;     /* NTP timestamp (middle 32 bits) of last received SR */
	struct timeval last_rcv_SR_time;   /* time at which last SR was received  */
	uint16_t snd_seq; /* send sequence number */
	uint32_t last_rtcp_packet_count; /*the sender's octet count in the last sent RTCP SR*/
	uint32_t sent_payload_bytes; /*used for RTCP sender reports*/
	int recv_errno;
	int send_errno;
	int snd_socket_size;
	int rcv_socket_size;
	int ssrc_changed_thres;
	jitter_stats_t jitter_stats;
	void *congdetect;// struct _OrtpCongestionDetector *congdetect;
	void *video_bw_estimator;// struct _OrtpVideoBandwidthEstimator *video_bw_estimator;
#if defined(_WIN32) || defined(_WIN32_WCE)
#ifdef ORTP_WINDOWS_SAMELINUX
#else
	ortp_thread_t win_t;
	volatile bool_t is_win_thread_running;
	ortp_mutex_t winthread_lock;
	ortp_mutex_t winrq_lock;
#endif
#endif
    rtp_queue_t sockrq;
}RtpStream;

typedef struct _RtcpStream
{
	OrtpStream gs;
	OrtpRtcpSendAlgorithm send_algo;
	OrtpRtcpXrConfiguration xr_conf;
	OrtpRtcpXrMediaCallbacks xr_media_callbacks;
	OrtpRtcpTmmbrInfo tmmbr_info;
	bool_t enabled; /*tells whether we can send RTCP packets */
	bool_t rtcp_xr_dlrr_to_send;
	uint8_t rtcp_fb_fir_seq_nr;	/* The FIR command sequence number */
	uint32_t last_rtcp_fb_pli_snt;
} RtcpStream;

#define RTP_CALLBACK_TABLE_MAX_ENTRIES	5
typedef void (*RtpCallback)(RtpSession *, void *arg1, void *arg2, void *arg3);

typedef struct _RtpSignalTable
{
	RtpCallback callback[RTP_CALLBACK_TABLE_MAX_ENTRIES];
	void * user_data[RTP_CALLBACK_TABLE_MAX_ENTRIES];
	RtpSession *session;
	const char *signal_name;
	int count;
}RtpSignalTable;

#define USER_TIMER_MAX  32
typedef struct
{
    int state;
    struct timeval timer;
    unsigned int interval;
    int (*timer_func)(void *);
    void    *pdata;
}UserTimer;

typedef void (*RtpTimerFunc)(void);
	
typedef struct _RtpTimer
{
	int state;
#define RTP_TIMER_RUNNING 1
#define RTP_TIMER_STOPPED 0
	RtpTimerFunc timer_init;
	RtpTimerFunc timer_do;
	RtpTimerFunc timer_uninit;
	struct timeval interval;
}RtpTimer;



typedef struct _RtpScheduler {
 
	RtpSession *list;	/* list of scheduled sessions*/
	int sessions_cnt;
	SessionSet	all_sessions;  /* mask of scheduled sessions */
	int		all_max;		/* the highest pos in the all mask */
	SessionSet  r_sessions;		/* mask of sessions that have a recv event */
	int		r_max;
	SessionSet	w_sessions;		/* mask of sessions that have a send event */
	int 		w_max;
	SessionSet	e_sessions;	/* mask of session that have error event */
	int		e_max;
	int max_sessions;		/* the number of position in the masks */
  /* GMutex  *unblock_select_mutex; */
	ortp_cond_t   unblock_select_cond;
	ortp_mutex_t	lock;
	ortp_thread_t thread;
	int thread_running;
	RtpTimer *timer;
	uint32_t time_;       /*number of miliseconds elapsed since the start of the thread */
	uint32_t timer_inc;	/* the timer increment in milisec */
	int _UserTimerCnt;
    UserTimer   _UserTimer[USER_TIMER_MAX];
}RtpScheduler;


/**
 * An object representing a bi-directional RTP session.
 * It holds sockets, jitter buffer, various counters (timestamp, sequence numbers...)
 * Applications SHOULD NOT try to read things within the RtpSession object but use
 * instead its public API (the rtp_session_* methods) where RtpSession is used as a
 * pointer.
 * rtp_session_new() allocates and initialize a RtpSession.
**/
struct _RtpSession
{
	ortp_mutex_t main_mutex; /* To protect data that can be accessed simultaneously by a control thread and the real-time thread in charge of sending/receiving. */
	RtpSession *next;	/* next RtpSession, when the session are enqueued by the scheduler */
	int mask_pos;	/* the position in the scheduler mask of RtpSession : do not move this field: it is part of the ABI since the session_set macros use it*/
	struct {
		RtpProfile *profile;
		int pt;
		unsigned int ssrc;
#ifndef _USER_ORTP_ADEP
		WaitPoint wp;
#endif
	} snd,rcv;
	unsigned int inc_ssrc_candidate;
	int inc_same_ssrc_count;
	int hw_recv_pt; /* recv payload type before jitter buffer */
    int recv_buf_size;
    int send_buf_size;
	int target_upload_bandwidth; /* Target upload bandwidth at network layer (with IP and UDP headers) in bits/s */
	RtpSignalTable on_ssrc_changed;
	RtpSignalTable on_payload_type_changed;
	RtpSignalTable on_telephone_event_packet;
	RtpSignalTable on_telephone_event;
	RtpSignalTable on_timestamp_jump;
	RtpSignalTable on_network_error;
	RtpSignalTable on_rtcp_bye;
    ortp_list_t *signal_tables;
    ortp_list_t *eventqs;
	RtpStream rtp;
	RtcpStream rtcp;
	OrtpRtcpXrStats rtcp_xr_stats;
	RtpSessionMode mode;
#ifndef _USER_ORTP_ADEP
	RtpScheduler *sched;
#endif
	uint32_t flags;
	int dscp;
	int multicast_ttl;
	int multicast_loopback;
	float duplication_ratio; /* Number of times a packet should be duplicated */
	float duplication_left ; /* Remainder of the duplication ratio, internal use */
	void * user_data;
	/* FIXME: Should be a table for all session participants. */
	struct timeval last_recv_time; /* Time of receiving the RTP/RTCP packet. */
	mblk_t *pending;
	/* telephony events extension */
	int tev_send_pt; /*telephone event to be used for sending*/
	mblk_t *current_tev;		/* the pending telephony events */
	mblk_t *minimal_sdes;
	mblk_t *full_sdes;
	rtp_queue_t contributing_sources;
	int lost_packets_test_vector;
	unsigned int interarrival_jitter_test_vector;
	unsigned int delay_test_vector;
	float rtt;/*last round trip delay calculated*/
	int cum_loss;
	OrtpNetworkSimulatorCtx *net_sim_ctx;
	RtpSession *spliced_session; /*a RtpSession that will retransmit everything received on this session*/
	rtp_stats_t stats;
    ortp_list_t *recv_addr_map;
	uint32_t send_ts_offset; /*additional offset to add when sending packets */
	bool_t symmetric_rtp;
	bool_t permissive; /*use the permissive algorithm*/
	bool_t use_connect; /* use connect() on the socket */
	bool_t ssrc_set;

	bool_t reuseaddr; /*setsockopt SO_REUSEADDR */
	bool_t rtcp_mux;
	unsigned char avpf_features; /**< A bitmask of ORTP_AVPF_FEATURE_* macros. */
	bool_t use_pktinfo;

	bool_t is_spliced;
	bool_t congestion_detector_enabled;
	bool_t video_bandwidth_estimator_enabled;
	bool_t is_primary;  /* tells if this session is the primary of the rtp bundle */
	
	bool_t warn_non_working_pkt_info;

	/* bundle mode */
	RtpBundle *bundle; /* back pointer to the rtp bundle object */
	rtp_queue_t bundleq;
	ortp_mutex_t bundleq_lock;

    bool_t  ipv6_enable;
    int rtp_channel;
    int rtcp_channel;
};

#define ORTP_SESSION_LOCK(n) ortp_mutex_lock(&((RtpSession*)n)->main_mutex)
#define ORTP_SESSION_UNLOCK(n) ortp_mutex_unlock(&((RtpSession*)n)->main_mutex)
/**
 * Structure describing the video bandwidth estimator parameters
**/
typedef struct _OrtpVideoBandwidthEstimatorParams {
	int enabled; /**<Whether estimator is enabled or off.*/
	unsigned int packet_count_min; /** minimum number of packets with the same sent timestamp to be processed continuously before being used */
	unsigned int packets_size_max; /** number of packets needed to compute the available video bandwidth */
	unsigned int trust_percentage; /** percentage for which the chosen bandwidth value in all available will be inferior */
} OrtpVideoBandwidthEstimatorParams;



#define rtp_session_using_transport(s, stream) (((s)->flags & RTP_SESSION_USING_TRANSPORT) && (s->stream.gs.tr != 0))

int rtp_session_rtp_recv_abstract(ortp_socket_t socket, mblk_t *msg, int flags, struct ipstack_sockaddr *from, socklen_t *fromlen);

void rtp_session_update_payload_type(RtpSession * session, int pt);
int rtp_putq(rtp_queue_t *q, mblk_t *mp);
mblk_t * rtp_getq(rtp_queue_t *q, uint32_t ts, int *rejected);
int rtp_session_rtp_recv(RtpSession * session, uint32_t ts);
int rtp_session_rtcp_recv(RtpSession * session);
int rtp_session_rtp_send (RtpSession * session, mblk_t * m);

#define rtp_session_rtcp_send rtp_session_rtcp_sendm_raw

void rtp_session_rtp_parse(RtpSession *session, mblk_t *mp, uint32_t local_str_ts, struct ipstack_sockaddr *addr, socklen_t addrlen);

void rtp_session_run_rtcp_send_scheduler(RtpSession *session);
void update_avg_rtcp_size(RtpSession *session, int bytes);

mblk_t * rtp_session_network_simulate(RtpSession *session, mblk_t *input, bool_t *is_rtp_packet);
void ortp_network_simulator_destroy(OrtpNetworkSimulatorCtx *sim);

void rtcp_common_header_init(rtcp_common_header_t *ch, RtpSession *s,int type, int rc, size_t bytes_len);

mblk_t * make_xr_rcvr_rtt(RtpSession *session);
mblk_t * make_xr_dlrr(RtpSession *session);
mblk_t * make_xr_stat_summary(RtpSession *session);
mblk_t * make_xr_voip_metrics(RtpSession *session);

bool_t rtcp_is_RTPFB_internal(const mblk_t *m);
bool_t rtcp_is_PSFB_internal(const mblk_t *m);
bool_t rtp_session_has_fb_packets_to_send(RtpSession *session);
void rtp_session_send_regular_rtcp_packet_and_reschedule(RtpSession *session, uint64_t tc);
void rtp_session_send_fb_rtcp_packet_and_reschedule(RtpSession *session);

void ortp_stream_clear_aux_addresses(OrtpStream *os);
/*
 * no more public, use modifier instead
 * */
void rtp_session_set_transports(RtpSession *session, RtpTransport *rtptr, RtpTransport *rtcptr);

bool_t rtp_profile_is_telephone_event(const RtpProfile *prof, int pt);

ortp_socket_t rtp_session_get_socket(RtpSession *session, bool_t is_rtp);

void rtp_session_do_splice(RtpSession *session, mblk_t *packet, bool_t is_rtp);

/*
 * Update remote addr in the following case:
 * rtp symetric == TRUE && socket not connected && remote addr has changed && ((rtp/rtcp packet && not only at start) or (no rtp/rtcp packets received))
 * @param[in] session  on which to perform change
 * @param[in] mp packet where remote addr is retreived
 * @param[in] is_rtp true if rtp
 * @param[in] only_at_start only perform changes if no valid packets received yet
 * @return 0 if chaged was performed
 *
 */
	
int rtp_session_update_remote_sock_addr(RtpSession * session, mblk_t * mp, bool_t is_rtp,bool_t only_at_start);

void rtp_session_process_incoming(RtpSession * session, mblk_t *mp, bool_t is_rtp_packet, uint32_t ts, bool_t received_via_rtcp_mux);
void update_sent_bytes(OrtpStream *os, int nbytes);

void _rtp_session_apply_socket_sizes(RtpSession *session);

void jb_parameters_init(JBParameters *jbp);
void rtp_session_init_jitter_buffer(RtpSession *session);
void rtp_session_process (RtpSession * session, uint32_t time, RtpScheduler *sched);

//#define log_send_error(s,t,m,d,l)   _log_send_error_func(__func__, __LINE__, s, t, m, d, l)
//void _log_send_error_func(const char *, int line, RtpSession *session, const char *type, mblk_t *m, struct sockaddr *destaddr, socklen_t destlen);
#ifdef __cplusplus
}
#endif

#endif
