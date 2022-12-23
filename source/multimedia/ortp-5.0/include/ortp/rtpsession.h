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

/**
 * \file rtpsession.h
 * \brief The RtpSession api
 *
 * The RtpSession objects represent a RTP session: once it is configured with
 * local and remote network addresses and a payload type is given, it let you send
 * and recv a media stream.
**/


#ifndef RTPSESSION_H
#define RTPSESSION_H

#ifdef __cplusplus
extern "C"
{
#endif



#define ORTP_AVPF_FEATURE_NONE 0
#define ORTP_AVPF_FEATURE_TMMBR (1 << 0)
#define ORTP_AVPF_FEATURE_GENERIC_NACK (1 << 1)
#define ORTP_AVPF_FEATURE_IMMEDIATE_NACK (1 << 2)




ORTP_PUBLIC const char *ortp_network_simulator_mode_to_string(OrtpNetworkSimulatorMode mode);
ORTP_PUBLIC OrtpNetworkSimulatorMode ortp_network_simulator_mode_from_string(const char *str);


/* public API */
ORTP_PUBLIC RtpSession *rtp_session_new(int mode);
ORTP_PUBLIC void rtp_session_set_scheduling_mode(RtpSession *session, int yesno);
ORTP_PUBLIC void rtp_session_set_blocking_mode(RtpSession *session, int yesno);
ORTP_PUBLIC void rtp_session_set_profile(RtpSession *session, RtpProfile *profile);
ORTP_PUBLIC void rtp_session_set_send_profile(RtpSession *session,RtpProfile *profile);
ORTP_PUBLIC void rtp_session_set_recv_profile(RtpSession *session,RtpProfile *profile);
ORTP_PUBLIC RtpProfile *rtp_session_get_profile(RtpSession *session);
ORTP_PUBLIC RtpProfile *rtp_session_get_send_profile(RtpSession *session);
ORTP_PUBLIC RtpProfile *rtp_session_get_recv_profile(RtpSession *session);
ORTP_PUBLIC int rtp_session_signal_connect(RtpSession *session,const char *signal_name, RtpCallback cb, void *user_data);
ORTP_PUBLIC int rtp_session_signal_disconnect_by_callback(RtpSession *session,const char *signal_name, RtpCallback cb);
ORTP_PUBLIC void rtp_session_set_ssrc(RtpSession *session, uint32_t ssrc);
ORTP_PUBLIC uint32_t rtp_session_get_send_ssrc(RtpSession* session);
ORTP_PUBLIC uint32_t rtp_session_get_recv_ssrc(RtpSession *session);
ORTP_PUBLIC void rtp_session_set_seq_number(RtpSession *session, uint16_t seq);
ORTP_PUBLIC uint16_t rtp_session_get_seq_number(RtpSession *session);
ORTP_PUBLIC uint32_t rtp_session_get_rcv_ext_seq_number(RtpSession *session);
ORTP_PUBLIC int rtp_session_get_cum_loss(RtpSession *session);
ORTP_PUBLIC void rtp_session_set_duplication_ratio(RtpSession *session, float ratio);

ORTP_PUBLIC void rtp_session_enable_jitter_buffer(RtpSession *session , bool_t enabled);
ORTP_PUBLIC bool_t rtp_session_jitter_buffer_enabled(const RtpSession *session);
ORTP_PUBLIC void rtp_session_set_jitter_buffer_params(RtpSession *session, const JBParameters *par);
ORTP_PUBLIC void rtp_session_get_jitter_buffer_params(RtpSession *session, JBParameters *par);

/**
 * Set an additional timestamps offset for outgoing stream..
 * @param s		a rtp session freshly created.
 * @param offset		a timestamp offset value
 *
**/
ORTP_PUBLIC void rtp_session_set_send_ts_offset(RtpSession *s, uint32_t offset);
ORTP_PUBLIC uint32_t rtp_session_get_send_ts_offset(RtpSession *s);


/*deprecated jitter control functions*/
ORTP_PUBLIC void rtp_session_set_jitter_compensation(RtpSession *session, int milisec);
ORTP_PUBLIC void rtp_session_enable_adaptive_jitter_compensation(RtpSession *session, bool_t val);
ORTP_PUBLIC bool_t rtp_session_adaptive_jitter_compensation_enabled(RtpSession *session);

ORTP_PUBLIC void rtp_session_set_time_jump_limit(RtpSession *session, int miliseconds);
ORTP_PUBLIC int rtp_session_join_multicast_group(RtpSession *session, const char *ip);
ORTP_PUBLIC int rtp_session_set_local_addr(RtpSession *session,const char *addr, int rtp_port, int rtcp_port);
ORTP_PUBLIC int rtp_session_get_local_port(const RtpSession *session);
ORTP_PUBLIC int rtp_session_get_local_rtcp_port(const RtpSession *session);

ORTP_PUBLIC int
rtp_session_set_remote_addr_full (RtpSession * session, const char * rtp_addr, int rtp_port, const char * rtcp_addr, int rtcp_port);
/*same as previous function, old name:*/
ORTP_PUBLIC int rtp_session_set_remote_addr_and_port (RtpSession * session, const char * addr, int rtp_port, int rtcp_port);
ORTP_PUBLIC int rtp_session_set_remote_addr(RtpSession *session,const char *addr, int port);
ORTP_PUBLIC int rtp_session_add_aux_remote_addr_full(RtpSession * session, const char * rtp_addr, int rtp_port, const char * rtcp_addr, int rtcp_port);
ORTP_PUBLIC int rtp_session_del_aux_remote_addr_full(RtpSession * session, const char * rtp_addr, int rtp_port, const char * rtcp_addr, int rtcp_port);
ORTP_PUBLIC void rtp_session_clear_aux_remote_addr(RtpSession * session);
/* alternatively to the set_remote_addr() and set_local_addr(), an application can give
a valid socket (potentially connect()ed )to be used by the RtpSession */
ORTP_PUBLIC void rtp_session_set_sockets(RtpSession *session, ortp_socket_t rtpfd, ortp_socket_t rtcpfd);
ORTP_PUBLIC void rtp_session_set_overtcp(RtpSession *session, bool_t rtsp, int rtp_channel, int rtcp_channel);

ORTP_PUBLIC void rtp_session_get_transports(const RtpSession *session, RtpTransport **rtptr, RtpTransport **rtcptr);
/*those methods are provided for people who wants to send non-RTP messages using the RTP/RTCP sockets */
ORTP_PUBLIC ortp_socket_t rtp_session_get_rtp_socket(const RtpSession *session);
ORTP_PUBLIC ortp_socket_t rtp_session_get_rtcp_socket(const RtpSession *session);
ORTP_PUBLIC void rtp_session_refresh_sockets(RtpSession *session);


/* QOS / DSCP */
ORTP_PUBLIC int rtp_session_set_dscp(RtpSession *session, int dscp);
ORTP_PUBLIC int rtp_session_get_dscp(const RtpSession *session);


/* Packet info */
ORTP_PUBLIC int rtp_session_set_pktinfo(RtpSession *session, int activate);

/* Multicast methods */
ORTP_PUBLIC int rtp_session_set_multicast_ttl(RtpSession *session, int ttl);
ORTP_PUBLIC int rtp_session_get_multicast_ttl(RtpSession *session);

ORTP_PUBLIC int rtp_session_set_multicast_loopback(RtpSession *session, int yesno);
ORTP_PUBLIC int rtp_session_get_multicast_loopback(RtpSession *session);



ORTP_PUBLIC int rtp_session_set_send_payload_type(RtpSession *session, int paytype);
ORTP_PUBLIC int rtp_session_get_send_payload_type(const RtpSession *session);

ORTP_PUBLIC int rtp_session_get_recv_payload_type(const RtpSession *session);
ORTP_PUBLIC int rtp_session_set_recv_payload_type(RtpSession *session, int pt);

ORTP_PUBLIC int rtp_session_set_send_telephone_event_payload_type(RtpSession *session, int paytype);

ORTP_PUBLIC int rtp_session_set_payload_type(RtpSession *session, int pt);

ORTP_PUBLIC void rtp_session_set_symmetric_rtp (RtpSession * session, bool_t yesno);

ORTP_PUBLIC bool_t rtp_session_get_symmetric_rtp (const RtpSession * session);

ORTP_PUBLIC void rtp_session_enable_rtcp_mux(RtpSession *session, bool_t yesno);

ORTP_PUBLIC bool_t rtp_session_rtcp_mux_enabled(RtpSession *session);

ORTP_PUBLIC void rtp_session_set_connected_mode(RtpSession *session, bool_t yesno);

ORTP_PUBLIC void rtp_session_enable_rtcp(RtpSession *session, bool_t yesno);
/*
 * rtcp status
 * @return TRUE if rtcp is enabled for this session
 */
ORTP_PUBLIC bool_t rtp_session_rtcp_enabled(const RtpSession *session);

ORTP_PUBLIC void rtp_session_set_rtcp_report_interval(RtpSession *session, int value_ms);

/**
 * Define the bandwidth available for RTCP streams based on the upload bandwidth
 * targeted by the application (in bits/s). RTCP streams would not take more than
 * a few percents of the limit bandwidth (around 5%).
 *
 * @param session a rtp session
 * @param target_bandwidth bandwidth limit in bits/s
 */
ORTP_PUBLIC void rtp_session_set_target_upload_bandwidth(RtpSession *session, int target_bandwidth);
ORTP_PUBLIC int rtp_session_get_target_upload_bandwidth(RtpSession *session);

ORTP_PUBLIC void rtp_session_configure_rtcp_xr(RtpSession *session, const OrtpRtcpXrConfiguration *config);
ORTP_PUBLIC void rtp_session_set_rtcp_xr_media_callbacks(RtpSession *session, const OrtpRtcpXrMediaCallbacks *cbs);

ORTP_PUBLIC void rtp_session_set_ssrc_changed_threshold(RtpSession *session, int numpackets);

/*low level recv and send functions */
ORTP_PUBLIC mblk_t * rtp_session_recvm_with_ts (RtpSession * session, uint32_t user_ts);
ORTP_PUBLIC mblk_t * rtp_session_create_packet(RtpSession *session, size_t header_size, const uint8_t *payload, size_t payload_size, uint8_t m);
ORTP_PUBLIC mblk_t * rtp_session_create_packet_raw(const uint8_t *packet, size_t packet_size);
ORTP_PUBLIC mblk_t * rtp_session_create_packet_with_data(RtpSession *session, uint8_t *payload, size_t payload_size, uint8_t m, void (*freefn)(void*));
ORTP_PUBLIC mblk_t * rtp_session_create_packet_in_place(RtpSession *session,uint8_t *buffer, size_t size, uint8_t m, void (*freefn)(void*) );
ORTP_PUBLIC int rtp_session_sendm_with_ts (RtpSession * session, mblk_t *mp, uint32_t userts);
ORTP_PUBLIC int rtp_session_sendto(RtpSession *session, bool_t is_rtp, mblk_t *m, int flags, const struct sockaddr *destaddr, socklen_t destlen);
ORTP_PUBLIC int rtp_session_recvfrom(RtpSession *session, bool_t is_rtp, mblk_t *m, int flags, struct sockaddr *from, socklen_t *fromlen);
/* high level recv and send functions */
ORTP_PUBLIC int rtp_session_recv_with_ts(RtpSession *session, uint8_t *buffer, int len, uint32_t ts, int *have_more);
ORTP_PUBLIC int rtp_session_send_with_ts(RtpSession *session, const uint8_t *buffer, int len, uint32_t userts, uint8_t m);
ORTP_PUBLIC int rtp_session_tcp_forward(ortp_socket_t sock, const uint8_t *msg , int tolen);

/* Specific function called to reset the winrq queue and if called on windows to stop the async reception thread */
ORTP_PUBLIC void rtp_session_reset_recvfrom(RtpSession *session);

/* event API*/
ORTP_PUBLIC void rtp_session_register_event_queue(RtpSession *session, void *q);
ORTP_PUBLIC void rtp_session_unregister_event_queue(RtpSession *session, void *q);


/* IP bandwidth usage estimation functions, returning bits/s*/
ORTP_PUBLIC float rtp_session_compute_send_bandwidth(RtpSession *session);
ORTP_PUBLIC float rtp_session_compute_recv_bandwidth(RtpSession *session);
ORTP_PUBLIC float rtp_session_get_send_bandwidth(RtpSession *session);
ORTP_PUBLIC float rtp_session_get_recv_bandwidth(RtpSession *session);
ORTP_PUBLIC float rtp_session_get_rtp_send_bandwidth(RtpSession *session);
ORTP_PUBLIC float rtp_session_get_rtp_recv_bandwidth(RtpSession *session);
ORTP_PUBLIC float rtp_session_get_rtcp_send_bandwidth(RtpSession *session);
ORTP_PUBLIC float rtp_session_get_rtcp_recv_bandwidth(RtpSession *session);

ORTP_PUBLIC float rtp_session_get_send_bandwidth_smooth(RtpSession *session);
ORTP_PUBLIC float rtp_session_get_recv_bandwidth_smooth(RtpSession *session);

ORTP_PUBLIC void rtp_session_send_rtcp_APP(RtpSession *session, uint8_t subtype, const char *name, const uint8_t *data, int datalen);
/**
 *	Send the rtcp datagram \a packet to the destination set by rtp_session_set_remote_addr()
 *  The packet (\a packet) is freed once it is sent.
 *
 * @param session a rtp session.
 * @param m a rtcp packet presented as a mblk_t.
 * @return the number of bytes sent over the network.
 **/

ORTP_PUBLIC	int rtp_session_rtcp_sendm_raw(RtpSession * session, mblk_t * m);


ORTP_PUBLIC uint32_t rtp_session_get_current_send_ts(RtpSession *session);
ORTP_PUBLIC uint32_t rtp_session_get_current_recv_ts(RtpSession *session);
ORTP_PUBLIC void rtp_session_flush_sockets(RtpSession *session);
ORTP_PUBLIC void rtp_session_release_sockets(RtpSession *session);
ORTP_PUBLIC void rtp_session_resync(RtpSession *session);
ORTP_PUBLIC void rtp_session_reset(RtpSession *session);
ORTP_PUBLIC void rtp_session_destroy(RtpSession *session);

ORTP_PUBLIC const rtp_stats_t * rtp_session_get_stats(const RtpSession *session);
ORTP_PUBLIC const jitter_stats_t * rtp_session_get_jitter_stats( const RtpSession *session );
ORTP_PUBLIC void rtp_session_reset_stats(RtpSession *session);

ORTP_PUBLIC void rtp_session_set_data(RtpSession *session, void *data);
ORTP_PUBLIC void *rtp_session_get_data(const RtpSession *session);

ORTP_PUBLIC void rtp_session_set_recv_buf_size(RtpSession *session, int bufsize);
ORTP_PUBLIC void rtp_session_set_send_buf_size(RtpSession *session, int bufsize);
ORTP_PUBLIC void rtp_session_set_rtp_socket_send_buffer_size(RtpSession * session, unsigned int size);
ORTP_PUBLIC void rtp_session_set_rtp_socket_recv_buffer_size(RtpSession * session, unsigned int size);

/* in use with the scheduler to convert a timestamp in scheduler time unit (ms) */
ORTP_PUBLIC uint32_t rtp_session_ts_to_time(RtpSession *session,uint32_t timestamp);
ORTP_PUBLIC uint32_t rtp_session_time_to_ts(RtpSession *session, int millisecs);
ORTP_PUBLIC uint32_t rtp_session_payload_clock_rate(RtpSession *session);
/* this function aims at simulating senders with "imprecise" clocks, resulting in
rtp packets sent with timestamp uncorrelated with the system clock .
This is only availlable to sessions working with the oRTP scheduler */
ORTP_PUBLIC void rtp_session_make_time_distorsion(RtpSession *session, int milisec);

/*RTCP functions */
ORTP_PUBLIC void rtp_session_set_source_description(RtpSession *session, const char *cname,
	const char *name, const char *email, const char *phone,
    const char *loc, const char *tool, const char *note);
ORTP_PUBLIC void rtp_session_add_contributing_source(RtpSession *session, uint32_t csrc,
    const char *cname, const char *name, const char *email, const char *phone,
    const char *loc, const char *tool, const char *note);
/* DEPRECATED: Use rtp_session_remove_contributing_source instead of rtp_session_remove_contributing_sources */
#define rtp_session_remove_contributing_sources rtp_session_remove_contributing_source
ORTP_PUBLIC void rtp_session_remove_contributing_source(RtpSession *session, uint32_t csrc);
ORTP_PUBLIC mblk_t* rtp_session_create_rtcp_sdes_packet(RtpSession *session, bool_t full);

ORTP_PUBLIC void rtp_session_get_last_recv_time(RtpSession *session, struct timeval *tv);
ORTP_PUBLIC int rtp_session_bye(RtpSession *session, const char *reason);

ORTP_PUBLIC int rtp_session_get_last_send_error_code(RtpSession *session);
ORTP_PUBLIC void rtp_session_clear_send_error_code(RtpSession *session);
ORTP_PUBLIC int rtp_session_get_last_recv_error_code(RtpSession *session);
ORTP_PUBLIC void rtp_session_clear_recv_error_code(RtpSession *session);


ORTP_PUBLIC float rtp_session_get_round_trip_propagation(RtpSession *session);


ORTP_PUBLIC void rtp_session_enable_network_simulation(RtpSession *session, const OrtpNetworkSimulatorParams *params);
ORTP_PUBLIC void rtp_session_enable_congestion_detection(RtpSession *session, bool_t enabled);
ORTP_PUBLIC void rtp_session_enable_video_bandwidth_estimator(RtpSession *session, const OrtpVideoBandwidthEstimatorParams *params);

ORTP_PUBLIC void rtp_session_rtcp_set_lost_packet_value( RtpSession *session, const int value );
ORTP_PUBLIC void rtp_session_rtcp_set_jitter_value(RtpSession *session, const unsigned int value );
ORTP_PUBLIC void rtp_session_rtcp_set_delay_value(RtpSession *session, const unsigned int value );
ORTP_PUBLIC mblk_t * rtp_session_pick_with_cseq (RtpSession * session, const uint16_t sequence_number);


ORTP_PUBLIC void rtp_session_send_rtcp_xr_rcvr_rtt(RtpSession *session);
ORTP_PUBLIC void rtp_session_send_rtcp_xr_dlrr(RtpSession *session);
ORTP_PUBLIC void rtp_session_send_rtcp_xr_stat_summary(RtpSession *session);
ORTP_PUBLIC void rtp_session_send_rtcp_xr_voip_metrics(RtpSession *session);


ORTP_PUBLIC bool_t rtp_session_avpf_enabled(RtpSession *session);
ORTP_PUBLIC bool_t rtp_session_avpf_payload_type_feature_enabled(RtpSession *session, unsigned char feature);
ORTP_PUBLIC bool_t rtp_session_avpf_feature_enabled(RtpSession *session, unsigned char feature);
ORTP_PUBLIC void rtp_session_enable_avpf_feature(RtpSession *session, unsigned char feature, bool_t enable);
ORTP_PUBLIC uint16_t rtp_session_get_avpf_rr_interval(RtpSession *session);
ORTP_PUBLIC bool_t rtp_session_rtcp_psfb_scheduled(RtpSession *session, rtcp_psfb_type_t type);
ORTP_PUBLIC bool_t rtp_session_rtcp_rtpfb_scheduled(RtpSession *session, rtcp_rtpfb_type_t type);
ORTP_PUBLIC void rtp_session_send_rtcp_fb_generic_nack(RtpSession *session, uint16_t pid, uint16_t blp);
ORTP_PUBLIC void rtp_session_send_rtcp_fb_pli(RtpSession *session);
ORTP_PUBLIC void rtp_session_send_rtcp_fb_fir(RtpSession *session);
ORTP_PUBLIC void rtp_session_send_rtcp_fb_sli(RtpSession *session, uint16_t first, uint16_t number, uint8_t picture_id);
ORTP_PUBLIC void rtp_session_send_rtcp_fb_rpsi(RtpSession *session, uint8_t *bit_string, uint16_t bit_string_len);
ORTP_PUBLIC void rtp_session_send_rtcp_fb_tmmbr(RtpSession *session, uint64_t mxtbr);
ORTP_PUBLIC void rtp_session_send_rtcp_fb_tmmbn(RtpSession *session, uint32_t ssrc);


/*private */
ORTP_PUBLIC uint16_t rtp_session_mblk_get_seq(mblk_t *mp);
ORTP_PUBLIC void rtp_session_init(RtpSession *session, int mode);
#define rtp_session_set_flag(session,flag) (session)->flags|=(flag)
#define rtp_session_unset_flag(session,flag) (session)->flags&=~(flag)
ORTP_PUBLIC void rtp_session_uninit(RtpSession *session);
ORTP_PUBLIC void rtp_session_dispatch_event(RtpSession *session, void *ev);


ORTP_PUBLIC void rtp_session_set_reuseaddr(RtpSession *session, bool_t yes);

ORTP_PUBLIC int meta_rtp_transport_sendto(RtpTransport *t, mblk_t *msg , int flags, const struct ipstack_sockaddr *to, socklen_t tolen);

ORTP_PUBLIC int meta_rtp_transport_modifier_inject_packet_to_send(RtpTransport *t, RtpTransportModifier *tpm, mblk_t *msg, int flags);
ORTP_PUBLIC int meta_rtp_transport_modifier_inject_packet_to_send_to(RtpTransport *t, RtpTransportModifier *tpm, mblk_t *msg, int flags, const struct sockaddr *to, socklen_t tolen);
ORTP_PUBLIC int meta_rtp_transport_modifier_inject_packet_to_recv(RtpTransport *t, RtpTransportModifier *tpm, mblk_t *msg, int flags);

/**
 * get endpoint if any
 * @param[in] transport RtpTransport object.
 * @return #_RtpTransport
 *
 * */
ORTP_PUBLIC RtpTransport* meta_rtp_transport_get_endpoint(const RtpTransport *transport);
/**
 * set endpoint
 * @param[in] transport RtpTransport object.
 * @param[in] endpoint RtpEndpoint.
 *
 * */
ORTP_PUBLIC void meta_rtp_transport_set_endpoint(RtpTransport *transport,RtpTransport *endpoint);

ORTP_PUBLIC void meta_rtp_transport_destroy(RtpTransport *tp);
ORTP_PUBLIC void meta_rtp_transport_append_modifier(RtpTransport *tp,RtpTransportModifier *tpm);
ORTP_PUBLIC void meta_rtp_transport_prepend_modifier(RtpTransport *tp,RtpTransportModifier *tpm);
ORTP_PUBLIC void meta_rtp_transport_remove_modifier(RtpTransport *tp, RtpTransportModifier *tpm);

ORTP_PUBLIC int rtp_session_splice(RtpSession *session, RtpSession *to_session);
ORTP_PUBLIC int rtp_session_unsplice(RtpSession *session, RtpSession *to_session);

ORTP_PUBLIC bool_t ortp_stream_is_ipv6(OrtpStream *os);

/* RtpBundle api */


ORTP_PUBLIC RtpBundle* rtp_bundle_new(void);
ORTP_PUBLIC void rtp_bundle_delete(RtpBundle *bundle);

ORTP_PUBLIC int rtp_bundle_get_mid_extension_id(RtpBundle *bundle);
ORTP_PUBLIC void rtp_bundle_set_mid_extension_id(RtpBundle *bundle, int id);

ORTP_PUBLIC void rtp_bundle_add_session(RtpBundle *bundle, const char *mid, RtpSession *session);
ORTP_PUBLIC void rtp_bundle_remove_session_by_id(RtpBundle *bundle, const char *mid);
ORTP_PUBLIC void rtp_bundle_remove_session(RtpBundle *bundle, RtpSession *session);
ORTP_PUBLIC void rtp_bundle_clear(RtpBundle *bundle);

ORTP_PUBLIC RtpSession* rtp_bundle_get_primary_session(RtpBundle *bundle);
ORTP_PUBLIC void rtp_bundle_set_primary_session(RtpBundle *bundle, const char * mid);

ORTP_PUBLIC const char *rtp_bundle_get_session_mid(RtpBundle *bundle, RtpSession *session);

ORTP_PUBLIC int rtp_bundle_send_through_primary(RtpBundle *bundle, bool_t is_rtp, mblk_t *m, int flags, const struct ipstack_sockaddr *destaddr, socklen_t destlen);
/* Returns FALSE if the rtp packet or at least one of the RTCP packet (compound) was for the primary */
ORTP_PUBLIC bool_t rtp_bundle_dispatch(RtpBundle *bundle, bool_t is_rtp, mblk_t *m);
ORTP_PUBLIC void rtp_session_use_local_addr(RtpSession * session, const char * rtp_local_addr, const char * rtcp_local_addr);

#ifdef __cplusplus
}
#endif

#endif
