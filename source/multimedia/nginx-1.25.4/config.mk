#############################################################################
# DEFINE
###########################################################################

#OS

NGX_CORE_OBJ = ngx_log.o \
	ngx_palloc.o \
	ngx_array.o \
	ngx_list.o \
	ngx_hash.o \
	ngx_buf.o \
	ngx_queue.o \
	ngx_output_chain.o \
	ngx_string.o \
	ngx_parse.o \
	ngx_parse_time.o \
	ngx_inet.o \
	ngx_file.o \
	ngx_crc32.o \
	ngx_murmurhash.o \
	ngx_md5.o \
	ngx_sha1.o \
	ngx_rbtree.o \
	ngx_radix_tree.o \
	ngx_slab.o \
	ngx_times.o \
	ngx_shmtx.o \
	ngx_connection.o \
	ngx_cycle.o \
	ngx_spinlock.o \
	ngx_rwlock.o \
	ngx_cpuinfo.o \
	ngx_conf_file.o \
	ngx_module.o \
	ngx_resolver.o \
	ngx_open_file_cache.o \
	ngx_crypt.o \
	ngx_proxy_protocol.o \
	ngx_syslog.o \
	ngx_bpf.o \
	ngx_thread_pool.o

NGX_EVENT_OBJ = ngx_event.o \
	ngx_event_timer.o \
	ngx_event_posted.o \
	ngx_event_accept.o \
	ngx_event_udp.o \
	ngx_event_connect.o \
	ngx_event_pipe.o \
	ngx_epoll_module.o \
	ngx_select_module.o \
	ngx_poll_module.o \
	ngx_event_openssl.o \
	ngx_event_openssl_stapling.o

NGX_OS_OBJ = ngx_time.o \
	ngx_errno.o \
	ngx_alloc.o \
	ngx_files.o \
	ngx_socket.o \
	ngx_recv.o \
	ngx_readv_chain.o \
	ngx_udp_recv.o \
	ngx_send.o \
	ngx_writev_chain.o \
	ngx_udp_send.o \
	ngx_udp_sendmsg_chain.o \
	ngx_channel.o \
	ngx_shmem.o \
	ngx_process.o \
	ngx_daemon.o \
	ngx_setaffinity.o \
	ngx_setproctitle.o \
	ngx_posix_init.o \
	ngx_user.o \
	ngx_dlopen.o \
	ngx_process_cycle.o \
	ngx_linux_init.o \
	ngx_linux_sendfile_chain.o \
	ngx_linux_aio_read.o \
	ngx_thread_cond.o \
	ngx_thread_mutex.o \
	ngx_thread_id.o

NGX_HTTP_OBJ = ngx_http.o \
	ngx_http_core_module.o \
	ngx_http_special_response.o \
	ngx_http_request.o \
	ngx_http_parse.o \
	ngx_http_log_module.o \
	ngx_http_request_body.o \
	ngx_http_variables.o \
	ngx_http_script.o \
	ngx_http_upstream.o \
	ngx_http_upstream_round_robin.o \
	ngx_http_file_cache.o \
	ngx_http_write_filter_module.o \
	ngx_http_header_filter_module.o \
	ngx_http_chunked_filter_module.o \
	ngx_http_range_filter_module.o \
	ngx_http_gzip_filter_module.o \
	ngx_http_postpone_filter_module.o \
	ngx_http_ssi_filter_module.o \
	ngx_http_charset_filter_module.o \
	ngx_http_userid_filter_module.o \
	ngx_http_headers_filter_module.o \
	ngx_http_copy_filter_module.o \
	ngx_http_not_modified_filter_module.o \
	ngx_http_static_module.o \
	ngx_http_autoindex_module.o \
	ngx_http_index_module.o \
	ngx_http_mirror_module.o \
	ngx_http_try_files_module.o \
	ngx_http_auth_basic_module.o \
	ngx_http_access_module.o \
	ngx_http_limit_conn_module.o \
	ngx_http_limit_req_module.o \
	ngx_http_geo_module.o \
	ngx_http_map_module.o \
	ngx_http_split_clients_module.o \
	ngx_http_referer_module.o \
	ngx_http_proxy_module.o \
	ngx_http_fastcgi_module.o \
	ngx_http_uwsgi_module.o \
	ngx_http_scgi_module.o \
	ngx_http_memcached_module.o \
	ngx_http_empty_gif_module.o \
	ngx_http_browser_module.o \
	ngx_http_upstream_hash_module.o \
	ngx_http_upstream_ip_hash_module.o \
	ngx_http_upstream_least_conn_module.o \
	ngx_http_upstream_random_module.o \
	ngx_http_upstream_keepalive_module.o \
	ngx_http_upstream_zone_module.o

NGX_STREAM_OBJ = ngx_stream.o \
	ngx_stream_variables.o \
	ngx_stream_script.o \
	ngx_stream_handler.o \
	ngx_stream_core_module.o \
	ngx_stream_log_module.o \
	ngx_stream_proxy_module.o \
	ngx_stream_upstream.o \
	ngx_stream_upstream_round_robin.o \
	ngx_stream_write_filter_module.o \
	ngx_stream_ssl_module.o \
	ngx_stream_limit_conn_module.o \
	ngx_stream_access_module.o \
	ngx_stream_geo_module.o \
	ngx_stream_map_module.o \
	ngx_stream_split_clients_module.o \
	ngx_stream_return_module.o \
	ngx_stream_set_module.o \
	ngx_stream_upstream_hash_module.o \
	ngx_stream_upstream_least_conn_module.o \
	ngx_stream_upstream_random_module.o \
	ngx_stream_upstream_zone_module.o \

NGX_RTMP_OBJ = ngx_rtmp.o \
	ngx_rtmp_init.o \
	ngx_rtmp_handshake.o \
	ngx_rtmp_handler.o \
	ngx_rtmp_amf.o \
	ngx_rtmp_send.o \
	ngx_rtmp_shared.o \
	ngx_rtmp_eval.o \
	ngx_rtmp_receive.o \
	ngx_rtmp_core_module.o \
	ngx_rtmp_cmd_module.o \
	ngx_rtmp_codec_module.o \
	ngx_rtmp_access_module.o \
	ngx_rtmp_record_module.o \
	ngx_rtmp_live_module.o \
	ngx_rtmp_play_module.o \
	ngx_rtmp_flv_module.o \
	ngx_rtmp_mp4_module.o \
	ngx_rtmp_netcall_module.o \
	ngx_rtmp_relay_module.o \
	ngx_rtmp_bandwidth.o \
	ngx_rtmp_exec_module.o \
	ngx_rtmp_auto_push_module.o \
	ngx_rtmp_notify_module.o \
	ngx_rtmp_log_module.o \
	ngx_rtmp_limit_module.o \
	ngx_rtmp_bitop.o \
	ngx_rtmp_proxy_protocol.o \
	ngx_rtmp_hls_module.o \
	ngx_rtmp_dash_module.o \
	ngx_rtmp_mpegts.o \
	ngx_rtmp_mp4.o \
	ngx_rtmp_stat_module.o \
	ngx_rtmp_control_module.o

OSOBJ = ngx_modules.o nginx.o $(NGX_CORE_OBJ) $(NGX_OS_OBJ) $(NGX_EVENT_OBJ) \
	$(NGX_HTTP_OBJ) $(NGX_STREAM_OBJ) $(NGX_RTMP_OBJ)
#############################################################################
# LIB
###########################################################################
LIBS = libngx.a
