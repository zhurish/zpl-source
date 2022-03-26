/*
 * Main implementation file for interface to Forwarding Plane Manager.
 *
 * Copyright (C) 2012 by Open Source Routing.
 * Copyright (C) 2012 by Internet Systems Consortium, Inc. ("ISC")
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <os_include.h>

#include "log.h"
#include "stream.h"
#include "thread.h"
#include "network.h"
#include "command.h"

#include "nsm_rib.h"

#include "nsm_fpm.h"
#include "nsm_fpm_private.h"
#ifdef OS_THREAD
#include "os_list.h"
#include "os_time.h"
#endif
/*
 * Interval at which we attempt to ipstack_connect to the FPM.
 */
#define ZFPM_CONNECT_RETRY_IVL   5

/*
 * Sizes of outgoing and incoming stream buffers for writing/reading
 * FPM messages.
 */
#define ZFPM_OBUF_SIZE (2 * FPM_MAX_MSG_LEN)
#define ZFPM_IBUF_SIZE (FPM_MAX_MSG_LEN)

/*
 * The maximum number of times the FPM ipstack_socket write callback can call
 * 'write' before it yields.
 */
#define ZFPM_MAX_WRITES_PER_RUN 10

/*
 * Interval over which we collect statistics.
 */
#define ZFPM_STATS_IVL_SECS        10

/*
 * Structure that holds state for iterating over all route_node
 * structures that are candidates for being communicated to the FPM.
 */
typedef struct zfpm_rnodes_iter_t_
{
  rib_tables_iter_t tables_iter;
  route_table_iter_t iter;
} zfpm_rnodes_iter_t;

/*
 * Statistics.
 */
typedef struct zfpm_stats_t_ {
  zpl_ulong connect_calls;
  zpl_ulong connect_no_sock;

  zpl_ulong read_cb_calls;

  zpl_ulong write_cb_calls;
  zpl_ulong write_calls;
  zpl_ulong partial_writes;
  zpl_ulong max_writes_hit;
  zpl_ulong t_write_yields;

  zpl_ulong nop_deletes_skipped;
  zpl_ulong route_adds;
  zpl_ulong route_dels;

  zpl_ulong updates_triggered;
  zpl_ulong redundant_triggers;
  zpl_ulong non_fpm_table_triggers;

  zpl_ulong dests_del_after_update;

  zpl_ulong t_conn_down_starts;
  zpl_ulong t_conn_down_dests_processed;
  zpl_ulong t_conn_down_yields;
  zpl_ulong t_conn_down_finishes;

  zpl_ulong t_conn_up_starts;
  zpl_ulong t_conn_up_dests_processed;
  zpl_ulong t_conn_up_yields;
  zpl_ulong t_conn_up_aborts;
  zpl_ulong t_conn_up_finishes;

} zfpm_stats_t;

/*
 * States for the FPM state machine.
 */
typedef enum {

  /*
   * In this state we are not yet ready to ipstack_connect to the FPM. This
   * can happen when this module is disabled, or if we're cleaning up
   * after a connection has gone down.
   */
  ZFPM_STATE_IDLE,

  /*
   * Ready to talk to the FPM and periodically trying to ipstack_connect to
   * it.
   */
  ZFPM_STATE_ACTIVE,

  /*
   * In the middle of bringing up a TCP connection. Specifically,
   * waiting for a ipstack_connect() call to complete asynchronously.
   */
  ZFPM_STATE_CONNECTING,

  /*
   * TCP connection to the FPM is up.
   */
  ZFPM_STATE_ESTABLISHED

} zfpm_state_t;

/*
 * Message format to be used to communicate with the FPM.
 */
typedef enum
{
  ZFPM_MSG_FORMAT_NONE,
  ZFPM_MSG_FORMAT_NETLINK,
  ZFPM_MSG_FORMAT_PROTOBUF,
} zfpm_msg_format_e;
/*
 * Globals.
 */
typedef struct zfpm_glob_t_
{

  /*
   * True if the FPM module has been enabled.
   */
  int enabled;

  /*
   * Message format to be used to communicate with the fpm.
   */
  zfpm_msg_format_e message_format;

  struct thread_master *master;

  zfpm_state_t state;

  in_addr_t   fpm_server;
  /*
   * Port on which the FPM is running.
   */
  int fpm_port;

  /*
   * List of rib_dest_t structures to be processed
   */
  TAILQ_HEAD (zfpm_dest_q, rib_dest_t_) dest_q;

  /*
   * Stream ipstack_socket to the FPM.
   */
  zpl_socket_t sock;

  /*
   * Buffers for messages to/from the FPM.
   */
  struct stream *obuf;
  struct stream *ibuf;

  /*
   * Threads for I/O.
   */
  struct thread *t_connect;
  struct thread *t_write;
  struct thread *t_read;

  /*
   * Thread to clean up after the TCP connection to the FPM goes down
   * and the state that belongs to it.
   */
  struct thread *t_conn_down;

  struct {
    zfpm_rnodes_iter_t iter;
  } t_conn_down_state;

  /*
   * Thread to take actions once the TCP conn to the FPM comes up, and
   * the state that belongs to it.
   */
  struct thread *t_conn_up;

  struct {
    zfpm_rnodes_iter_t iter;
  } t_conn_up_state;

  zpl_ulong connect_calls;
  zpl_time_t last_connect_call_time;

  /*
   * Stats from the start of the current statistics interval up to
   * now. These are the counters we typically update in the code.
   */
  zfpm_stats_t stats;

  /*
   * Statistics that were gathered in the last collection interval.
   */
  zfpm_stats_t last_ivl_stats;

  /*
   * Cumulative stats from the last clear to the start of the current
   * statistics interval.
   */
  zfpm_stats_t cumulative_stats;

  /*
   * Stats interval timer.
   */
  struct thread *t_stats;

  /*
   * If non-zero, the last time when statistics were cleared.
   */
  zpl_time_t last_stats_clear_time;

} zfpm_glob_t;

static zfpm_glob_t zfpm_glob_space;
static zfpm_glob_t *zfpm_g = &zfpm_glob_space;

static int zfpm_read_cb (struct thread *thread);
static int zfpm_write_cb (struct thread *thread);

static void zfpm_set_state (zfpm_state_t state, const char *reason);
static void zfpm_start_connect_timer (const char *reason);
static void zfpm_start_stats_timer (void);

/*
 * zfpm_thread_should_yield
 */
static inline int
zfpm_thread_should_yield (struct thread *t)
{
  return thread_should_yield (t);
}

/*
 * zfpm_state_to_str
 */
static const char *
zfpm_state_to_str (zfpm_state_t state)
{
  switch (state)
    {

    case ZFPM_STATE_IDLE:
      return "idle";

    case ZFPM_STATE_ACTIVE:
      return "active";

    case ZFPM_STATE_CONNECTING:
      return "connecting";

    case ZFPM_STATE_ESTABLISHED:
      return "established";

    default:
      return "unknown";
    }
}

/*
 * zfpm_get_time
 */
static zpl_time_t
zfpm_get_time (void)
{
  struct timeval tv;
#ifdef OS_THREAD
  if (os_gettime (OS_CLK_MONOTONIC, &tv) < 0)
    zlog_warn (MODULE_NSM, "FPM: quagga_gettime failed!!");
#else
  if (quagga_gettime (QUAGGA_CLK_MONOTONIC, &tv) < 0)
    zlog_warn (MODULE_NSM, "FPM: quagga_gettime failed!!");
#endif
  return tv.tv_sec;
}

/*
 * zfpm_get_elapsed_time
 *
 * Returns the time elapsed (in seconds) since the given time.
 */
static zpl_time_t
zfpm_get_elapsed_time (zpl_time_t reference)
{
  zpl_time_t now;

  now = zfpm_get_time ();

  if (now < reference)
    {
      assert (0);
      return 0;
    }

  return now - reference;
}

/*
 * zfpm_is_table_for_fpm
 *
 * Returns zpl_true if the the given table is to be communicated to the
 * FPM.
 */
static inline int
zfpm_is_table_for_fpm (struct route_table *table)
{
  rib_table_info_t *info;

  info = rib_table_info (table);

  /*
   * We only ipstack_send the unicast tables in the main instance to the FPM
   * at this point.
   */
  if (info->zvrf->vrf_id != 0)
    return 0;

  if (info->safi != SAFI_UNICAST)
    return 0;

  return 1;
}

/*
 * zfpm_rnodes_iter_init
 */
static inline void
zfpm_rnodes_iter_init (zfpm_rnodes_iter_t *iter)
{
  memset (iter, 0, sizeof (*iter));
  rib_tables_iter_init (&iter->tables_iter);

  /*
   * This is a hack, but it makes implementing 'next' easier by
   * ensuring that route_table_iter_next() will return NULL the first
   * time we call it.
   */
  route_table_iter_init (&iter->iter, NULL);
  route_table_iter_cleanup (&iter->iter);
}

/*
 * zfpm_rnodes_iter_next
 */
static inline struct route_node *
zfpm_rnodes_iter_next (zfpm_rnodes_iter_t *iter)
{
  struct route_node *rn;
  struct route_table *table;

  while (1)
    {
      rn = route_table_iter_next (&iter->iter);
      if (rn)
	return rn;

      /*
       * We've made our way through this table, go to the next one.
       */
      route_table_iter_cleanup (&iter->iter);

      while ((table = rib_tables_iter_next (&iter->tables_iter)))
	{
	  if (zfpm_is_table_for_fpm (table))
	    break;
	}

      if (!table)
	return NULL;

      route_table_iter_init (&iter->iter, table);
    }

  return NULL;
}

/*
 * zfpm_rnodes_iter_pause
 */
static inline void
zfpm_rnodes_iter_pause (zfpm_rnodes_iter_t *iter)
{
  route_table_iter_pause (&iter->iter);
}

/*
 * zfpm_rnodes_iter_cleanup
 */
static inline void
zfpm_rnodes_iter_cleanup (zfpm_rnodes_iter_t *iter)
{
  route_table_iter_cleanup (&iter->iter);
  rib_tables_iter_cleanup (&iter->tables_iter);
}

/*
 * zfpm_stats_init
 *
 * Initialize a statistics block.
 */
static inline void
zfpm_stats_init (zfpm_stats_t *stats)
{
  memset (stats, 0, sizeof (*stats));
}

/*
 * zfpm_stats_reset
 */
static inline void
zfpm_stats_reset (zfpm_stats_t *stats)
{
  zfpm_stats_init (stats);
}

/*
 * zfpm_stats_copy
 */
static inline void
zfpm_stats_copy (const zfpm_stats_t *src, zfpm_stats_t *dest)
{
  memcpy (dest, src, sizeof (*dest));
}

/*
 * zfpm_stats_compose
 *
 * Total up the statistics in two stats structures ('s1 and 's2') and
 * return the result in the third argument, 'result'. Note that the
 * pointer 'result' may be the same as 's1' or 's2'.
 *
 * For simplicity, the implementation below assumes that the stats
 * structure is composed entirely of counters. This can easily be
 * changed when necessary.
 */
static void
zfpm_stats_compose (const zfpm_stats_t *s1, const zfpm_stats_t *s2,
		    zfpm_stats_t *result)
{
  const zpl_ulong *p1, *p2;
  zpl_ulong *result_p;
  zpl_uint32 i = 0, num_counters;

  p1 = (const zpl_ulong *) s1;
  p2 = (const zpl_ulong *) s2;
  result_p = (zpl_ulong *) result;

  num_counters = (sizeof (zfpm_stats_t) / sizeof (zpl_ulong));

  for (i = 0; i < num_counters; i++)
    {
      result_p[i] = p1[i] + p2[i];
    }
}

/*
 * zfpm_read_on
 */
static inline void
zfpm_read_on (void)
{
  assert (!zfpm_g->t_read);
  //assert (zfpm_g->sock >= 0);

  THREAD_READ_ON (zfpm_g->master, zfpm_g->t_read, zfpm_read_cb, 0,
		  zfpm_g->sock);
}

/*
 * zfpm_write_on
 */
static inline void
zfpm_write_on (void)
{
  assert (!zfpm_g->t_write);
  //assert (zfpm_g->sock >= 0);

  THREAD_WRITE_ON (zfpm_g->master, zfpm_g->t_write, zfpm_write_cb, 0,
		   zfpm_g->sock);
}

/*
 * zfpm_read_off
 */
static inline void
zfpm_read_off (void)
{
  THREAD_READ_OFF (zfpm_g->t_read);
}

/*
 * zfpm_write_off
 */
static inline void
zfpm_write_off (void)
{
  THREAD_WRITE_OFF (zfpm_g->t_write);
}

/*
 * zfpm_conn_up_thread_cb
 *
 * Callback for actions to be taken when the connection to the FPM
 * comes up.
 */
static int
zfpm_conn_up_thread_cb (struct thread *thread)
{
  struct route_node *rnode;
  zfpm_rnodes_iter_t *iter;
  rib_dest_t *dest;

  assert (zfpm_g->t_conn_up);
  zfpm_g->t_conn_up = NULL;

  iter = &zfpm_g->t_conn_up_state.iter;

  if (zfpm_g->state != ZFPM_STATE_ESTABLISHED)
    {
      zfpm_debug ("Connection not up anymore, conn_up thread aborting");
      zfpm_g->stats.t_conn_up_aborts++;
      goto done;
    }

  while ((rnode = zfpm_rnodes_iter_next (iter)))
    {
      dest = rib_dest_from_rnode (rnode);

      if (dest)
	{
	  zfpm_g->stats.t_conn_up_dests_processed++;
	  zfpm_trigger_update (rnode, NULL);
	}

      /*
       * Yield if need be.
       */
      if (!zfpm_thread_should_yield (thread))
	continue;

      zfpm_g->stats.t_conn_up_yields++;
      zfpm_rnodes_iter_pause (iter);
      zfpm_g->t_conn_up = thread_add_background (zfpm_g->master,
						 zfpm_conn_up_thread_cb,
						 0, 0);
      return 0;
    }

  zfpm_g->stats.t_conn_up_finishes++;

 done:
  zfpm_rnodes_iter_cleanup (iter);
  return 0;
}

/*
 * zfpm_connection_up
 *
 * Called when the connection to the FPM comes up.
 */
static void
zfpm_connection_up (const char *detail)
{
  //assert (zfpm_g->sock >= 0);
  zfpm_read_on ();
  zfpm_write_on ();
  zfpm_set_state (ZFPM_STATE_ESTABLISHED, detail);

  /*
   * Start thread to push existing routes to the FPM.
   */
  assert (!zfpm_g->t_conn_up);

  zfpm_rnodes_iter_init (&zfpm_g->t_conn_up_state.iter);

  zfpm_debug ("Starting conn_up thread");
  zfpm_g->t_conn_up = thread_add_background (zfpm_g->master,
					     zfpm_conn_up_thread_cb, 0, 0);
  zfpm_g->stats.t_conn_up_starts++;
}

/*
 * zfpm_connect_check
 *
 * Check if an asynchronous ipstack_connect() to the FPM is complete.
 */
static void
zfpm_connect_check (void)
{
  int status;
  socklen_t slen;
  int ret;

  zfpm_read_off ();
  zfpm_write_off ();

  slen = sizeof (status);
  ret = ipstack_getsockopt (zfpm_g->sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_ERROR, (void *) &status,
		    &slen);

  if (ret >= 0 && status == 0)
    {
      zfpm_connection_up ("async ipstack_connect complete");
      return;
    }

  /*
   * ipstack_getsockopt() failed or indicated an error on the ipstack_socket.
   */
  ipstack_close (zfpm_g->sock);
  //zfpm_g->sock = -1;

  zfpm_start_connect_timer ("ipstack_getsockopt() after async ipstack_connect failed");
  return;
}

/*
 * zfpm_conn_down_thread_cb
 *
 * Callback that is invoked to clean up state after the TCP connection
 * to the FPM goes down.
 */
static int
zfpm_conn_down_thread_cb (struct thread *thread)
{
  struct route_node *rnode;
  zfpm_rnodes_iter_t *iter;
  rib_dest_t *dest;

  assert (zfpm_g->state == ZFPM_STATE_IDLE);

  assert (zfpm_g->t_conn_down);
  zfpm_g->t_conn_down = NULL;

  iter = &zfpm_g->t_conn_down_state.iter;

  while ((rnode = zfpm_rnodes_iter_next (iter)))
    {
      dest = rib_dest_from_rnode (rnode);

      if (dest)
	{
	  if (CHECK_FLAG (dest->flags, RIB_DEST_UPDATE_FPM))
	    {
	      TAILQ_REMOVE (&zfpm_g->dest_q, dest, fpm_q_entries);
	    }

	  UNSET_FLAG (dest->flags, RIB_DEST_UPDATE_FPM);
	  UNSET_FLAG (dest->flags, RIB_DEST_SENT_TO_FPM);

	  zfpm_g->stats.t_conn_down_dests_processed++;

	  /*
	   * Check if the dest should be deleted.
	   */
	  rib_gc_dest(rnode);
	}

      /*
       * Yield if need be.
       */
      if (!zfpm_thread_should_yield (thread))
	continue;

      zfpm_g->stats.t_conn_down_yields++;
      zfpm_rnodes_iter_pause (iter);
      zfpm_g->t_conn_down = thread_add_background (zfpm_g->master,
						   zfpm_conn_down_thread_cb,
						   0, 0);
      return 0;
    }

  zfpm_g->stats.t_conn_down_finishes++;
  zfpm_rnodes_iter_cleanup (iter);

  /*
   * Start the process of connecting to the FPM again.
   */
  zfpm_start_connect_timer ("cleanup complete");
  return 0;
}

/*
 * zfpm_connection_down
 *
 * Called when the connection to the FPM has gone down.
 */
static void
zfpm_connection_down (const char *detail)
{
  if (!detail)
    detail = "unknown";

  assert (zfpm_g->state == ZFPM_STATE_ESTABLISHED);

  zlog_info (MODULE_NSM, "connection to the FPM has gone down: %s", detail);

  zfpm_read_off ();
  zfpm_write_off ();

  stream_reset (zfpm_g->ibuf);
  stream_reset (zfpm_g->obuf);

  if (!ipstack_invalid (zfpm_g->sock)) {
    ipstack_close (zfpm_g->sock);
    //zfpm_g->sock = -1;
  }

  /*
   * Start thread to clean up state after the connection goes down.
   */
  assert (!zfpm_g->t_conn_down);
  zfpm_debug ("Starting conn_down thread");
  zfpm_rnodes_iter_init (&zfpm_g->t_conn_down_state.iter);
  zfpm_g->t_conn_down = thread_add_background (zfpm_g->master,
					       zfpm_conn_down_thread_cb, 0, 0);
  zfpm_g->stats.t_conn_down_starts++;

  zfpm_set_state (ZFPM_STATE_IDLE, detail);
}

/*
 * zfpm_read_cb
 */
static int
zfpm_read_cb (struct thread *thread)
{
  zpl_size_t already;
  struct stream *ibuf;
  zpl_uint16 msg_len;
  fpm_msg_hdr_t *hdr;

  zfpm_g->stats.read_cb_calls++;
  assert (zfpm_g->t_read);
  zfpm_g->t_read = NULL;

  /*
   * Check if async ipstack_connect is now done.
   */
  if (zfpm_g->state == ZFPM_STATE_CONNECTING)
    {
      zfpm_connect_check();
      return 0;
    }

  assert (zfpm_g->state == ZFPM_STATE_ESTABLISHED);
  //assert (zfpm_g->sock >= 0);

  ibuf = zfpm_g->ibuf;

  already = stream_get_endp (ibuf);
  if (already < FPM_MSG_HDR_LEN)
    {
      ssize_t nbyte;

      nbyte = stream_read_try (ibuf, zfpm_g->sock, FPM_MSG_HDR_LEN - already);
      if (nbyte == 0 || nbyte == -1)
	{
	  zfpm_connection_down ("closed ipstack_socket in read");
	  return 0;
	}

      if (nbyte != (ssize_t) (FPM_MSG_HDR_LEN - already))
	goto done;

      already = FPM_MSG_HDR_LEN;
    }

  stream_set_getp (ibuf, 0);

  hdr = (fpm_msg_hdr_t *) stream_pnt (ibuf);

  if (!fpm_msg_hdr_ok (hdr))
    {
      zfpm_connection_down ("invalid message header");
      return 0;
    }

  msg_len = fpm_msg_len (hdr);

  /*
   * Read out the rest of the packet.
   */
  if (already < msg_len)
    {
      ssize_t nbyte;

      nbyte = stream_read_try (ibuf, zfpm_g->sock, msg_len - already);

      if (nbyte == 0 || nbyte == -1)
	{
	  zfpm_connection_down ("failed to read message");
	  return 0;
	}

      if (nbyte != (ssize_t) (msg_len - already))
	goto done;
    }

  zfpm_debug ("Read out a full fpm message");

  /*
   * Just throw it away for now.
   */
  stream_reset (ibuf);

 done:
  zfpm_read_on ();
  return 0;
}

/*
 * zfpm_writes_pending
 *
 * Returns zpl_true if we may have something to write to the FPM.
 */
static int
zfpm_writes_pending (void)
{

  /*
   * Check if there is any data in the outbound buffer that has not
   * been written to the ipstack_socket yet.
   */
  if (stream_get_endp (zfpm_g->obuf) - stream_get_getp (zfpm_g->obuf))
    return 1;

  /*
   * Check if there are any prefixes on the outbound queue.
   */
  if (!TAILQ_EMPTY (&zfpm_g->dest_q))
    return 1;

  return 0;
}

/*
 * zfpm_encode_route
 *
 * Encode a message to the FPM with information about the given route.
 *
 * Returns the number of bytes written to the buffer. 0 or a negative
 * value indicates an error.
 */
static inline int
zfpm_encode_route (rib_dest_t *dest, struct rib *rib, zpl_char *in_buf,
		   zpl_size_t in_buf_len, fpm_msg_type_e *msg_type)
{
  zpl_size_t len;
  zpl_uint32 cmd;
  len = 0;

  *msg_type = FPM_MSG_TYPE_NONE;

  switch (zfpm_g->message_format) {

  case ZFPM_MSG_FORMAT_PROTOBUF:
#ifdef HAVE_PROTOBUF
    len = zfpm_protobuf_encode_route (dest, rib, (zpl_uint8 *) in_buf,
				      in_buf_len);
    *msg_type = FPM_MSG_TYPE_PROTOBUF;
#endif
    break;

  case ZFPM_MSG_FORMAT_NETLINK:
#ifdef HAVE_NETLINK
    *msg_type = FPM_MSG_TYPE_NETLINK;
    cmd = rib ? IPSTACK_RTM_NEWROUTE : IPSTACK_RTM_DELROUTE;
    //len = zfpm_netlink_encode_route (cmd, dest, rib, in_buf, in_buf_len);
    assert(fpm_msg_align(len) == len);
    *msg_type = FPM_MSG_TYPE_NETLINK;
#endif /* HAVE_NETLINK */
    break;

  default:
    break;
  }

  return len;

}

/*
 * zfpm_route_for_update
 *
 * Returns the rib that is to be sent to the FPM for a given dest.
 */
struct rib *
zfpm_route_for_update (rib_dest_t *dest)
{
  struct rib *rib;

  RIB_DEST_FOREACH_ROUTE (dest, rib)
    {
      if (!CHECK_FLAG (rib->status, RIB_ENTRY_SELECTED_FIB))
	continue;

      return rib;
    }

  /*
   * We have no route for this destination.
   */
  return NULL;
}

/*
 * zfpm_build_updates
 *
 * Process the outgoing queue and write messages to the outbound
 * buffer.
 */
static void
zfpm_build_updates (void)
{
  struct stream *s;
  rib_dest_t *dest;
  zpl_uchar *buf, *data, *buf_end;
  zpl_size_t msg_len;
  zpl_size_t data_len;
  fpm_msg_hdr_t *hdr;
  struct rib *rib;
  int is_add, write_msg;
  fpm_msg_type_e msg_type;

  s = zfpm_g->obuf;

  assert (stream_empty (s));

  do {

    /*
     * Make sure there is enough space to write another message.
     */
    if (STREAM_WRITEABLE (s) < FPM_MAX_MSG_LEN)
      break;

    buf = STREAM_DATA (s) + stream_get_endp (s);
    buf_end = buf + STREAM_WRITEABLE (s);

    dest = TAILQ_FIRST (&zfpm_g->dest_q);
    if (!dest)
      break;

    assert (CHECK_FLAG (dest->flags, RIB_DEST_UPDATE_FPM));

    hdr = (fpm_msg_hdr_t *) buf;
    hdr->version = FPM_PROTO_VERSION;

    data = fpm_msg_data (hdr);

    rib = zfpm_route_for_update (dest);
    is_add = rib ? 1 : 0;

    write_msg = 1;

    /*
     * If this is a route deletion, and we have not sent the route to
     * the FPM previously, skip it.
     */
    if (!is_add && !CHECK_FLAG (dest->flags, RIB_DEST_SENT_TO_FPM))
      {
	write_msg = 0;
	zfpm_g->stats.nop_deletes_skipped++;
      }

    if (write_msg) {
      data_len = zfpm_encode_route (dest, rib, (zpl_char *) data, buf_end - data,
				    &msg_type);

      assert (data_len);
      if (data_len)
	{
	  hdr->msg_type = msg_type;
	  msg_len = fpm_data_len_to_msg_len (data_len);
	  hdr->msg_len = htons (msg_len);
	  stream_forward_endp (s, msg_len);

	  if (is_add)
	    zfpm_g->stats.route_adds++;
	  else
	    zfpm_g->stats.route_dels++;
	}
    }

    /*
     * Remove the dest from the queue, and reset the flag.
     */
    UNSET_FLAG (dest->flags, RIB_DEST_UPDATE_FPM);
    TAILQ_REMOVE (&zfpm_g->dest_q, dest, fpm_q_entries);

    if (is_add)
      {
	SET_FLAG (dest->flags, RIB_DEST_SENT_TO_FPM);
      }
    else
      {
	UNSET_FLAG (dest->flags, RIB_DEST_SENT_TO_FPM);
      }

    /*
     * Delete the destination if necessary.
     */
    if (rib_gc_dest (dest->rnode))
      zfpm_g->stats.dests_del_after_update++;

  } while (1);

}

/*
 * zfpm_write_cb
 */
static int
zfpm_write_cb (struct thread *thread)
{
  struct stream *s;
  int num_writes;

  zfpm_g->stats.write_cb_calls++;
  assert (zfpm_g->t_write);
  zfpm_g->t_write = NULL;

  /*
   * Check if async ipstack_connect is now done.
   */
  if (zfpm_g->state == ZFPM_STATE_CONNECTING)
    {
      zfpm_connect_check ();
      return 0;
    }

  assert (zfpm_g->state == ZFPM_STATE_ESTABLISHED);
  //assert (zfpm_g->sock >= 0);

  num_writes = 0;

  do
    {
      int bytes_to_write, bytes_written;

      s = zfpm_g->obuf;

      /*
       * If the stream is empty, try fill it up with data.
       */
      if (stream_empty (s))
	{
	  zfpm_build_updates ();
	}

      bytes_to_write = stream_get_endp (s) - stream_get_getp (s);
      if (!bytes_to_write)
	break;

      bytes_written = ipstack_write (zfpm_g->sock, STREAM_PNT (s), bytes_to_write);
      zfpm_g->stats.write_calls++;
      num_writes++;

      if (bytes_written < 0)
	{
	  if (IPSTACK_ERRNO_RETRY (ipstack_errno))
	    break;

	  zfpm_connection_down ("failed to write to ipstack_socket");
	  return 0;
	}

      if (bytes_written != bytes_to_write)
	{

	  /*
	   * Partial write.
	   */
	  stream_forward_getp (s, bytes_written);
	  zfpm_g->stats.partial_writes++;
	  break;
	}

      /*
       * We've written out the entire contents of the stream.
       */
      stream_reset (s);

      if (num_writes >= ZFPM_MAX_WRITES_PER_RUN)
	{
	  zfpm_g->stats.max_writes_hit++;
	  break;
	}

      if (zfpm_thread_should_yield (thread))
	{
	  zfpm_g->stats.t_write_yields++;
	  break;
	}
    } while (1);

  if (zfpm_writes_pending ())
      zfpm_write_on ();

  return 0;
}

/*
 * zfpm_connect_cb
 */
static int
zfpm_connect_cb (struct thread *t)
{
  zpl_socket_t sock;
  int ret = 0;
  struct ipstack_sockaddr_in serv;

  assert (zfpm_g->t_connect);
  zfpm_g->t_connect = NULL;
  assert (zfpm_g->state == ZFPM_STATE_ACTIVE);

  sock = ipstack_socket (OS_STACK, IPSTACK_AF_INET, IPSTACK_SOCK_STREAM, 0);
  if (ipstack_invalid(sock))
    {
      zfpm_debug ("Failed to create ipstack_socket for ipstack_connect(): %s", strerror(ipstack_errno));
      zfpm_g->stats.connect_no_sock++;
      return 0;
    }

  ipstack_set_nonblocking(sock);

  /* Make server ipstack_socket. */
  memset (&serv, 0, sizeof (serv));
  serv.sin_family = IPSTACK_AF_INET;
  serv.sin_port = htons (zfpm_g->fpm_port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  serv.sin_len = sizeof (struct ipstack_sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
  if (!zfpm_g->fpm_server)
    serv.sin_addr.s_addr = htonl (IPSTACK_INADDR_LOOPBACK);
  else 
    serv.sin_addr.s_addr = (zfpm_g->fpm_server);

  /*
   * Connect to the FPM.
   */
  zfpm_g->connect_calls++;
  zfpm_g->stats.connect_calls++;
  zfpm_g->last_connect_call_time = zfpm_get_time ();

  ret = ipstack_connect (sock, (struct ipstack_sockaddr *) &serv, sizeof (serv));
  if (ret >= 0)
    {
      zfpm_g->sock = sock;
      zfpm_connection_up ("ipstack_connect succeeded");
      return 1;
    }

  if (ipstack_errno == IPSTACK_ERRNO_EINPROGRESS)
    {
      zfpm_g->sock = sock;
      zfpm_read_on ();
      zfpm_write_on ();
      zfpm_set_state (ZFPM_STATE_CONNECTING, "async ipstack_connect in progress");
      return 0;
    }

  zlog_info (MODULE_NSM, "can't ipstack_connect to FPM %d: %s", sock, ipstack_strerror (ipstack_errno));
  ipstack_close (sock);

  /*
   * Restart timer for retrying connection.
   */
  zfpm_start_connect_timer ("ipstack_connect() failed");
  return 0;
}

/*
 * zfpm_set_state
 *
 * Move state machine into the given state.
 */
static void
zfpm_set_state (zfpm_state_t state, const char *reason)
{
  zfpm_state_t cur_state = zfpm_g->state;

  if (!reason)
    reason = "Unknown";

  if (state == cur_state)
    return;

  zfpm_debug("beginning state transition %s -> %s. Reason: %s",
	     zfpm_state_to_str (cur_state), zfpm_state_to_str (state),
	     reason);

  switch (state) {

  case ZFPM_STATE_IDLE:
    assert (cur_state == ZFPM_STATE_ESTABLISHED);
    break;

  case ZFPM_STATE_ACTIVE:
     assert (cur_state == ZFPM_STATE_IDLE ||
	     cur_state == ZFPM_STATE_CONNECTING);
    assert (zfpm_g->t_connect);
    break;

  case ZFPM_STATE_CONNECTING:
    //assert (zfpm_g->sock);
    assert (cur_state == ZFPM_STATE_ACTIVE);
    assert (zfpm_g->t_read);
    assert (zfpm_g->t_write);
    break;

  case ZFPM_STATE_ESTABLISHED:
    assert (cur_state == ZFPM_STATE_ACTIVE ||
	    cur_state == ZFPM_STATE_CONNECTING);
    //assert (zfpm_g->sock);
    assert (zfpm_g->t_read);
    assert (zfpm_g->t_write);
    break;
  }

  zfpm_g->state = state;
}

/*
 * zfpm_calc_connect_delay
 *
 * Returns the number of seconds after which we should attempt to
 * reconnect to the FPM.
 */
static long
zfpm_calc_connect_delay (void)
{
  zpl_time_t elapsed;

  /*
   * Return 0 if this is our first attempt to ipstack_connect.
   */
  if (zfpm_g->connect_calls == 0)
    {
      return 0;
    }

  elapsed = zfpm_get_elapsed_time (zfpm_g->last_connect_call_time);

  if (elapsed > ZFPM_CONNECT_RETRY_IVL) {
    return 0;
  }

  return ZFPM_CONNECT_RETRY_IVL - elapsed;
}

/*
 * zfpm_start_connect_timer
 */
static void
zfpm_start_connect_timer (const char *reason)
{
  long delay_secs;

  assert (!zfpm_g->t_connect);
  //assert (zfpm_g->sock < 0);

  assert(zfpm_g->state == ZFPM_STATE_IDLE ||
	 zfpm_g->state == ZFPM_STATE_ACTIVE ||
	 zfpm_g->state == ZFPM_STATE_CONNECTING);

  delay_secs = zfpm_calc_connect_delay();
  zfpm_debug ("scheduling ipstack_connect in %ld seconds", delay_secs);

  THREAD_TIMER_ON (zfpm_g->master, zfpm_g->t_connect, zfpm_connect_cb, 0,
		   delay_secs);
  zfpm_set_state (ZFPM_STATE_ACTIVE, reason);
}

/*
 * zfpm_is_enabled
 *
 * Returns zpl_true if the zebra FPM module has been enabled.
 */
static inline int
zfpm_is_enabled (void)
{
  return zfpm_g->enabled;
}

/*
 * zfpm_conn_is_up
 *
 * Returns zpl_true if the connection to the FPM is up.
 */
static inline int
zfpm_conn_is_up (void)
{
  if (zfpm_g->state != ZFPM_STATE_ESTABLISHED)
    return 0;

  //assert (zfpm_g->sock >= 0);

  return 1;
}

/*
 * zfpm_trigger_update
 *
 * The zebra code invokes this function to indicate that we should
 * ipstack_send an update to the FPM about the given route_node.
 */
void
zfpm_trigger_update (struct route_node *rn, const char *reason)
{
  rib_dest_t *dest;
  zpl_char buf[PREFIX_STRLEN];

  /*
   * Ignore if the connection is down. We will update the FPM about
   * all destinations once the connection comes up.
   */
/*
  if (!zfpm_conn_is_up ())
    return;
*/

  dest = rib_dest_from_rnode (rn);

  /*
   * Ignore the trigger if the dest is not in a table that we would
   * ipstack_send to the FPM.
   */
  if (!zfpm_is_table_for_fpm (rib_dest_table (dest)))
    {
      zfpm_g->stats.non_fpm_table_triggers++;
      return;
    }

  if (CHECK_FLAG (dest->flags, RIB_DEST_UPDATE_FPM)) {
    zfpm_g->stats.redundant_triggers++;
    return;
  }

  if (reason)
    {
      zfpm_debug ("%s triggering update to FPM - Reason: %s",
		  prefix2str (&rn->p, buf, sizeof(buf)), reason);
    }

  SET_FLAG (dest->flags, RIB_DEST_UPDATE_FPM);
  TAILQ_INSERT_TAIL (&zfpm_g->dest_q, dest, fpm_q_entries);
  zfpm_g->stats.updates_triggered++;

  /*
   * Make sure that writes are enabled.
   */
  if (zfpm_g->t_write)
    return;

  zfpm_write_on ();
}

/*
 * zfpm_stats_timer_cb
 */
static int
zfpm_stats_timer_cb (struct thread *t)
{
  assert (zfpm_g->t_stats);
  zfpm_g->t_stats = NULL;

  /*
   * Remember the stats collected in the last interval for display
   * purposes.
   */
  zfpm_stats_copy (&zfpm_g->stats, &zfpm_g->last_ivl_stats);

  /*
   * Add the current set of stats into the cumulative statistics.
   */
  zfpm_stats_compose (&zfpm_g->cumulative_stats, &zfpm_g->stats,
		      &zfpm_g->cumulative_stats);

  /*
   * Start collecting stats afresh over the next interval.
   */
  zfpm_stats_reset (&zfpm_g->stats);

  zfpm_start_stats_timer ();

  return 0;
}

/*
 * zfpm_stop_stats_timer
 */
static void
zfpm_stop_stats_timer (void)
{
  if (!zfpm_g->t_stats)
    return;

  zfpm_debug ("Stopping existing stats timer");
  THREAD_TIMER_OFF (zfpm_g->t_stats);
}

/*
 * zfpm_start_stats_timer
 */
void
zfpm_start_stats_timer (void)
{
  assert (!zfpm_g->t_stats);

  THREAD_TIMER_ON (zfpm_g->master, zfpm_g->t_stats, zfpm_stats_timer_cb, 0,
		   ZFPM_STATS_IVL_SECS);
}

/*
 * Helper macro for zfpm_show_stats() below.
 */
#define ZFPM_SHOW_STAT(counter)						\
  do {									\
    vty_out (vty, "%-40s %10lu %16lu%s", #counter, total_stats.counter,	\
	     zfpm_g->last_ivl_stats.counter, VTY_NEWLINE);		\
  } while (0)

/*
 * zfpm_show_stats
 */
static void
zfpm_show_stats (struct vty *vty)
{
  zfpm_stats_t total_stats;
  zpl_time_t elapsed;

  vty_out (vty, "%s%-40s %10s     Last %2d secs%s%s", VTY_NEWLINE, "Counter",
	   "Total", ZFPM_STATS_IVL_SECS, VTY_NEWLINE, VTY_NEWLINE);

  /*
   * Compute the total stats up to this instant.
   */
  zfpm_stats_compose (&zfpm_g->cumulative_stats, &zfpm_g->stats,
		      &total_stats);

  ZFPM_SHOW_STAT (connect_calls);
  ZFPM_SHOW_STAT (connect_no_sock);
  ZFPM_SHOW_STAT (read_cb_calls);
  ZFPM_SHOW_STAT (write_cb_calls);
  ZFPM_SHOW_STAT (write_calls);
  ZFPM_SHOW_STAT (partial_writes);
  ZFPM_SHOW_STAT (max_writes_hit);
  ZFPM_SHOW_STAT (t_write_yields);
  ZFPM_SHOW_STAT (nop_deletes_skipped);
  ZFPM_SHOW_STAT (route_adds);
  ZFPM_SHOW_STAT (route_dels);
  ZFPM_SHOW_STAT (updates_triggered);
  ZFPM_SHOW_STAT (non_fpm_table_triggers);
  ZFPM_SHOW_STAT (redundant_triggers);
  ZFPM_SHOW_STAT (dests_del_after_update);
  ZFPM_SHOW_STAT (t_conn_down_starts);
  ZFPM_SHOW_STAT (t_conn_down_dests_processed);
  ZFPM_SHOW_STAT (t_conn_down_yields);
  ZFPM_SHOW_STAT (t_conn_down_finishes);
  ZFPM_SHOW_STAT (t_conn_up_starts);
  ZFPM_SHOW_STAT (t_conn_up_dests_processed);
  ZFPM_SHOW_STAT (t_conn_up_yields);
  ZFPM_SHOW_STAT (t_conn_up_aborts);
  ZFPM_SHOW_STAT (t_conn_up_finishes);

  if (!zfpm_g->last_stats_clear_time)
    return;

  elapsed = zfpm_get_elapsed_time (zfpm_g->last_stats_clear_time);

  vty_out (vty, "%sStats were cleared %lu seconds ago%s", VTY_NEWLINE,
	   (zpl_ulong) elapsed, VTY_NEWLINE);
}

/*
 * zfpm_clear_stats
 */
static void
zfpm_clear_stats (struct vty *vty)
{
  if (!zfpm_is_enabled ())
    {
      vty_out (vty, "The FPM module is not enabled...%s", VTY_NEWLINE);
      return;
    }

  zfpm_stats_reset (&zfpm_g->stats);
  zfpm_stats_reset (&zfpm_g->last_ivl_stats);
  zfpm_stats_reset (&zfpm_g->cumulative_stats);

  zfpm_stop_stats_timer ();
  zfpm_start_stats_timer ();

  zfpm_g->last_stats_clear_time = zfpm_get_time();

  vty_out (vty, "Cleared FPM stats%s", VTY_NEWLINE);
}

/*
 * show_zebra_fpm_stats
 */
DEFUN (show_zebra_fpm_stats,
       show_zebra_fpm_stats_cmd,
       "show zebra fpm stats",
       SHOW_STR
       "Zebra information\n"
       "Forwarding Path Manager information\n"
       "Statistics\n")
{
  zfpm_show_stats (vty);
  return CMD_SUCCESS;
}

/*
 * clear_zebra_fpm_stats
 */
DEFUN (clear_zebra_fpm_stats,
       clear_zebra_fpm_stats_cmd,
       "clear zebra fpm stats",
       CLEAR_STR
       "Zebra information\n"
       "Clear Forwarding Path Manager information\n"
       "Statistics\n")
{
  zfpm_clear_stats (vty);
  return CMD_SUCCESS;
}

/*
 * update fpm connection information 
 */
DEFUN ( fpm_remote_ip, 
        fpm_remote_ip_cmd,
        "fpm connection ip A.B.C.D port <1-65535>",
        "fpm connection remote ip and port\n"
        "Remote fpm server ip A.B.C.D\n"
        "Enter ip ")
{

   in_addr_t fpm_server;
   zpl_uint32  port_no;

   fpm_server = ipstack_inet_addr (argv[0]);
   if (fpm_server == IPSTACK_INADDR_NONE)
     return CMD_ERR_INCOMPLETE;

   port_no = atoi (argv[1]);
   if (port_no < TCP_MIN_PORT || port_no > TCP_MAX_PORT)
     return CMD_ERR_INCOMPLETE;

   zfpm_g->fpm_server = fpm_server;
   zfpm_g->fpm_port = port_no;


   return CMD_SUCCESS;
}

DEFUN ( no_fpm_remote_ip, 
        no_fpm_remote_ip_cmd,
        "no fpm connection ip A.B.C.D port <1-65535>",
        "fpm connection remote ip and port\n"
        "Connection\n"
        "Remote fpm server ip A.B.C.D\n"
        "Enter ip ")
{
   if (zfpm_g->fpm_server != ipstack_inet_addr (argv[0]) || 
              zfpm_g->fpm_port !=  atoi (argv[1]))
       return CMD_ERR_NO_MATCH;

   zfpm_g->fpm_server = FPM_DEFAULT_IP;
   zfpm_g->fpm_port = FPM_DEFAULT_PORT;

   return CMD_SUCCESS;
}


/*
 * zfpm_init_message_format
 */
static inline void
zfpm_init_message_format (const char *format)
{
  int have_netlink, have_protobuf;

  have_netlink = have_protobuf = 0;

#ifdef HAVE_NETLINK
  have_netlink = 1;
#endif

#ifdef HAVE_PROTOBUF
  have_protobuf = 1;
#endif

  zfpm_g->message_format = ZFPM_MSG_FORMAT_NONE;

  if (!format)
    {
      if (have_netlink)
	{
	  zfpm_g->message_format = ZFPM_MSG_FORMAT_NETLINK;
	}
      else if (have_protobuf)
	{
	  zfpm_g->message_format = ZFPM_MSG_FORMAT_PROTOBUF;
	}
      return;
    }

  if (!strcmp ("netlink", format))
    {
      if (!have_netlink)
	{
	  zlog_err (MODULE_NSM, "FPM netlink message format is not available");
	  return;
	}
      zfpm_g->message_format = ZFPM_MSG_FORMAT_NETLINK;
      return;
    }

  if (!strcmp ("protobuf", format))
    {
      if (!have_protobuf)
	{
	  zlog_err (MODULE_NSM, "FPM protobuf message format is not available");
	  return;
	}
      zfpm_g->message_format = ZFPM_MSG_FORMAT_PROTOBUF;
      return;
    }

  zlog_warn (MODULE_NSM, "Unknown fpm format '%s'", format);
}

/**
 * fpm_remote_srv_write 
 *
 * Module to write remote fpm connection 
 *
 * Returns ZERO on success.
 */

int fpm_remote_srv_write (struct vty *vty )
{
   struct ipstack_in_addr in;

   in.s_addr = zfpm_g->fpm_server;

   if (zfpm_g->fpm_server != FPM_DEFAULT_IP || 
          zfpm_g->fpm_port != FPM_DEFAULT_PORT)
      vty_out (vty,"fpm connection ip %s port %d%s", ipstack_inet_ntoa (in),zfpm_g->fpm_port,VTY_NEWLINE);

   return 0;
}


/**
 * zfpm_init
 *
 * One-time initialization of the Zebra FPM module.
 *
 * @param[in] port port at which FPM is running.
 * @param[in] enable zpl_true if the zebra FPM module should be enabled
 * @param[in] format to use to talk to the FPM. Can be 'netink' or 'protobuf'.
 *
 * Returns zpl_true on success.
 */
int
zfpm_init (struct thread_master *master, int enable, zpl_uint16 port,
	   const char *format)
{
  static int initialized = 0;

  if (initialized) {
    return 1;
  }

  initialized = 1;

  memset (zfpm_g, 0, sizeof (*zfpm_g));
  zfpm_g->master = master;
  TAILQ_INIT(&zfpm_g->dest_q);
  //zfpm_g->sock = -1;
  zfpm_g->state = ZFPM_STATE_IDLE;

  zfpm_stats_init (&zfpm_g->stats);
  zfpm_stats_init (&zfpm_g->last_ivl_stats);
  zfpm_stats_init (&zfpm_g->cumulative_stats);

  install_element (ENABLE_NODE, CMD_VIEW_LEVEL, &show_zebra_fpm_stats_cmd);
  install_element (ENABLE_NODE, CMD_CONFIG_LEVEL, &clear_zebra_fpm_stats_cmd);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &fpm_remote_ip_cmd);
  install_element (CONFIG_NODE, CMD_CONFIG_LEVEL, &no_fpm_remote_ip_cmd);

  zfpm_init_message_format(format);

  /*
   * Disable FPM interface if no suitable format is available.
   */
  if (zfpm_g->message_format == ZFPM_MSG_FORMAT_NONE)
      enable = 0;

  zfpm_g->enabled = enable;

  if (!enable) {
    return 1;
  }

  if (!zfpm_g->fpm_server)
     zfpm_g->fpm_server = FPM_DEFAULT_IP;

  if (!port)
    port = FPM_DEFAULT_PORT;

  zfpm_g->fpm_port = port;

  zfpm_g->obuf = stream_new (ZFPM_OBUF_SIZE);
  zfpm_g->ibuf = stream_new (ZFPM_IBUF_SIZE);

  zfpm_start_stats_timer ();
  zfpm_start_connect_timer ("initialized");

  return 1;
}
