/* ipcstandby's client library.
 * Copyright (C) 1999 Kunihiro Ishiguro
 * Copyright (C) 2005 Andrew J. Schorr
 *
 * This file is part of GNU ipcstandby.
 *
 * GNU ipcstandby is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU ipcstandby is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU ipcstandby; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"

#include "thread.h"
#include "stream.h"
#include "network.h"
#include "sockunion.h"
#include "log.h"
#include "linklist.h"
#include "network.h"
#include "buffer.h"
#include "vty_include.h"
#include "ipcstandby_client.h"

/* Prototype for event manager. */
static void ipcstandby_client_event(enum ipcmsg_event, struct ipcstandby_client *);
static int ipcstandby_client_failed(struct ipcstandby_client *client);
static int ipcstandby_client_hello_send(struct ipcstandby_client *client);

/* Allocate client structure. */
struct ipcstandby_client *
ipcstandby_client_new(void *m)
{
    struct ipcstandby_client *client;
    client = XCALLOC(MTYPE_STANDBY_CLIENT, sizeof(struct ipcstandby_client));

    client->ibuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);
    client->obuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);
    client->master = m;
    client->debug = 0;
    return client;
}

/* This function is only called when exiting, because
   many parts of the code do not check for I/O errors, so they could
   reference an invalid pointer if the structure was ever freed.

   Free client structure. */
void ipcstandby_client_free(struct ipcstandby_client *client)
{
    if (client->ibuf)
        stream_free(client->ibuf);
    if (client->obuf)
        stream_free(client->obuf);
    if (client->remote)
        free(client->remote);

    XFREE(MTYPE_STANDBY_CLIENT, client);
}

/* Initialize ipcstandby client.  Argument module is unwanted
   redistribute route type. */
void ipcstandby_client_start(struct ipcstandby_client *client, zpl_uint32 slot, char *remote, int port)
{
    /* Schedule first client connection. */
    if (client->debug)
        zlog_debug(MODULE_STANDBY, "ipc standby client start scheduled");
    client->slot = slot;
    client->ipcstanby.slot = slot;
    if (remote)
        client->remote = strdup(remote);
    else
        client->remote = strdup("127.0.0.1");
    client->port = port;

    ipcstandby_client_event(ZPL_IPCMSG_SCHEDULE, client);
}

/* Stop ipcstandby client services. */
void ipcstandby_client_stop(struct ipcstandby_client *client)
{
    if (client->debug)
        zlog_debug(MODULE_STANDBY, "ipc standby client stopped");

    /* Stop threads. */
    THREAD_OFF(client->t_connect);
    THREAD_OFF(client->t_write);
    THREAD_OFF(client->t_timeout);
    /* Reset streams. */
    stream_reset(client->ibuf);
    stream_reset(client->obuf);

    /* Close ipstack_socket. */
    if (!ipstack_invalid(client->sock))
    {
        ipstack_close(client->sock);
    }
    client->is_connect = zpl_false;
    client->fail = 0;
    client->recv_cnt = 0;
    client->recv_faild_cnt = 0;
    client->pkt_err_cnt = 0;
    client->send_cnt = 0;
    client->send_faild_cnt = 0;
    client->connect_cnt = 0;
}

void ipcstandby_client_reset(struct ipcstandby_client *client)
{
    ipcstandby_client_stop(client);
    ipcstandby_client_event(ZPL_IPCMSG_SCHEDULE, client);
}

static int ipcstandby_client_connect_check(struct thread *t)
{
    struct ipcstandby_client *client = NULL;
    client = THREAD_ARG(t);
    client->t_write = NULL;

    if (ipstack_sock_connect_check(client->sock) == OK)
    {
        THREAD_OFF(client->t_timeout);
        client->is_connect = zpl_true;

        if (client->t_neg_timeout)
            THREAD_OFF(client->t_neg_timeout);
        ipcstandby_client_hello_send(client);
        ipcstandby_client_event(ZPL_IPCMSG_TIMEOUT, client);
        if (client->debug)
            zlog_warn(MODULE_STANDBY, "ipc standby client connect to %s:%d sock [%d] OK", client->remote, client->port, ipstack_fd(client->sock));
        return OK;
    }
    THREAD_OFF(client->t_timeout);
    if (client->debug)
        zlog_warn(MODULE_STANDBY, "ipc standby client connect to %s:%d sock [%d] failed", client->remote, client->port, ipstack_fd(client->sock));
    return ipcstandby_client_failed(client);
}

static int ipcstandby_client_connect_timeout(struct thread *t)
{
    struct ipcstandby_client *client;
    client = THREAD_ARG(t);
    client->t_timeout = NULL;
    ipcstandby_client_failed(client);
    return OK;
}
/* Make ipstack_socket to ipcstandby daemon. Return ipcstandby ipstack_socket. */
static int ipcstandby_client_socket_connect(struct ipcstandby_client *client, char *ip, int port)
{
    int ret;
    /* We should think about IPv6 connection. */
    client->sock = ipstack_sock_create(IPSTACK_IPCOM, zpl_true);
    if (ipstack_invalid(client->sock))
    {
        return ERROR;
    }
    /* Connect to ipcstandby. */
    ret = ipstack_sock_connect_nonblock(client->sock, ip ? ip : "127.0.0.1", port);
    if (ret < 0)
    {
        ipstack_close(client->sock);
        return ERROR;
    }
    client->t_write = thread_add_write(client->master, ipcstandby_client_connect_check, client, client->sock);
    client->t_timeout = thread_add_timer(client->master, ipcstandby_client_connect_timeout, client, 5);
    return OK;
}

/**
 * Connect to ipcstandby daemon.
 * @param client a pointer to client structure
 * @return ipstack_socket fd just to make sure that connection established
 * @see ipcstandby_client_init
 * @see ipcstandby_client_new
 */
static int ipcstandby_client_failed(struct ipcstandby_client *client)
{
    client->fail++;
    ipcstandby_client_stop(client);
    ipcstandby_client_event(ZPL_IPCMSG_CONNECT, client);
    return OK;
}

/* This function is a wrapper function for calling ipcstandby_client_start from
   timer or event thread. */
static int ipcstandby_client_connect(struct thread *t)
{
    struct ipcstandby_client *client;

    client = THREAD_ARG(t);
    client->t_connect = NULL;

    if (client->debug)
        zlog_debug(MODULE_STANDBY, "ipc standby clien connect is called");
    if (ipcstandby_client_socket_connect(client, client->remote, client->port) == ERROR)
    {
        return ipcstandby_client_failed(client);
    }
    return OK;
}

static int ipcstandby_client_hello_send(struct ipcstandby_client *client)
{
    struct ipcstanby_result ack;
    ipcstandby_create_header(client->obuf, ZPL_IPCSTANBY_HELLO);
    IPCSTANDBY_LOCK();
    stream_put(client->obuf, client->version, sizeof(client->version));
    stream_putl(client->obuf, client->ipcstanby.ID);
    stream_putl(client->obuf, ++client->ipcstanby.seqnum);
    stream_putc(client->obuf, client->ipcstanby.state);
    stream_putc(client->obuf, client->ipcstanby.slot);
    stream_putc(client->obuf, client->ipcstanby.MS);
    stream_putc(client->obuf, client->ipcstanby.master);
    IPCSTANDBY_UNLOCK();
    return ipcstandby_client_send_message(client, &ack, NULL, 500);
}

static int
ipcstandby_client_timeout(struct thread *t)
{
    struct ipcstandby_client *client;
    client = THREAD_ARG(t);
    client->t_timeout = NULL;
    ipcstandby_client_hello_send(client);
    client->t_timeout = thread_add_timer(client->master, ipcstandby_client_timeout, client, ZPL_IPCSTANBY_TIMEOUT);
    return OK;
}

static int ipcstandby_client_recv_message(struct ipcstandby_client *client, struct ipcstanby_result *ack,
                                          struct ipcstandby_callback *callback, int timeout)
{
    int nbyte;
    int timeoutval = timeout;
    zpl_size_t already;
    zpl_uint16 length, command;
    zpl_uint8 marker, version;
    int current_timeval = 0;
    int start_timeval = 0;
    if (!client->is_connect)
        return ERROR;
    start_timeval = os_get_monotonic_msec();
    already = stream_get_endp(client->ibuf);
    while (already < ZPL_IPCMSG_HEADER_SIZE)
    {
        nbyte = ipstack_read_timeout(client->sock, STREAM_DATA(client->ibuf) + client->ibuf->endp,
                                     ZPL_IPCMSG_HEADER_SIZE - client->ibuf->endp, timeoutval);
        if (nbyte == OS_TIMEOUT)
        {
            stream_reset(client->ibuf);
            zlog_warn(MODULE_STANDBY, "ipc standby client Recv msg from [%d] unit %s is timeout", ipstack_fd(client->sock), client->remote);
            return OS_TIMEOUT;
        }
        else if (nbyte == ERROR)
        {
            client->recv_faild_cnt++;
            stream_reset(client->ibuf);
            zlog_err(MODULE_STANDBY, "ipc standby client Recv msg from [%d] unit %s is ERROR:%s", ipstack_fd(client->sock), client->remote, ipstack_strerror(ipstack_errno));
            return OS_CLOSE;
        }
        if (nbyte != (ssize_t)(ZPL_IPCMSG_HEADER_SIZE - client->ibuf->endp))
        {
            /* Try again later. */
            current_timeval = os_get_monotonic_msec();
            timeoutval -= (current_timeval - start_timeval);
            if (timeoutval <= 0)
            {
                stream_reset(client->ibuf);
                zlog_warn(MODULE_STANDBY, "ipc standby client Recv msg from [%d] unit %s is timeout", ipstack_fd(client->sock), client->remote);
                return OS_TIMEOUT;
            }
            continue;
        }
        client->ibuf->endp = ZPL_IPCMSG_HEADER_SIZE;
        already = ZPL_IPCMSG_HEADER_SIZE;
        current_timeval = os_get_monotonic_msec();
        timeoutval -= (current_timeval - start_timeval);
        break;
    }
    if (client->ibuf->endp == ZPL_IPCMSG_HEADER_SIZE)
    {
        stream_set_getp(client->ibuf, 0);

        /* Fetch header values. */
        length = stream_getw(client->ibuf);
        marker = stream_getc(client->ibuf);
        version = stream_getc(client->ibuf);
        command = stream_getw(client->ibuf);

        if (marker != ZPL_IPCMSG_HEADER_MARKER || version != ZPL_IPCMSG_VERSION)
        {
            client->pkt_err_cnt++;
            stream_reset(client->ibuf);
            zlog_err(MODULE_STANDBY, "%s: ipc standby client ipstack_socket %d version mismatch, marker %d, version %d",
                     __func__, ipstack_fd(client->sock), marker, version);
            return ERROR;
        }

        if (length < ZPL_IPCMSG_HEADER_SIZE)
        {
            client->pkt_err_cnt++;
            stream_reset(client->ibuf);
            zlog_err(MODULE_STANDBY, "%s: ipc standby client ipstack_socket %d message length %u is less than %d ",
                     __func__, ipstack_fd(client->sock), length, ZPL_IPCMSG_HEADER_SIZE);
            return ERROR;
        }

        /* Length check. */
        if (length > STREAM_SIZE(client->ibuf))
        {
            struct stream *ns;
            zlog_warn(MODULE_STANDBY, "%s: ipc standby client message size %u exceeds buffer size %lu, expanding...",
                      __func__, length, (u_long)STREAM_SIZE(client->ibuf));
            ns = stream_new(length);
            stream_copy(ns, client->ibuf);
            stream_free(client->ibuf);
            client->ibuf = ns;
        }
        while (1)
        {
            /* Read rest of ipcstandby packet. */
            if (already < length)
            {
                nbyte = ipstack_read_timeout(client->sock, STREAM_DATA(client->ibuf) + client->ibuf->endp,
                                             (length)-already, timeoutval);
                if (nbyte == OS_TIMEOUT)
                {
                    stream_reset(client->ibuf);
                    zlog_warn(MODULE_STANDBY, "ipc standby client Recv msg from [%d] unit %s is timeout", ipstack_fd(client->sock), client->remote);
                    return OS_TIMEOUT;
                }
                else if (nbyte == ERROR)
                {
                    stream_reset(client->ibuf);
                    client->recv_faild_cnt++;
                    zlog_err(MODULE_STANDBY, "ipc standby client Recv msg from [%d] unit %s is ERROR:%s", ipstack_fd(client->sock), client->remote, ipstack_strerror(ipstack_errno));
                    return OS_CLOSE;
                }
                if (nbyte != (ssize_t)(length - already))
                {
                    /* Try again later. */
                    current_timeval = os_get_monotonic_msec();
                    timeoutval -= (current_timeval - start_timeval);
                    if (timeoutval <= 0)
                    {
                        stream_reset(client->ibuf);
                        zlog_warn(MODULE_STANDBY, "ipc standby client Recv msg from [%d] unit %s is timeout", ipstack_fd(client->sock), client->remote);
                        return OS_TIMEOUT;
                    }
                    continue;
                }
                already += nbyte;
                client->ibuf->endp += nbyte;
                if (already == length)
                    break;
            }
        }
        if (command == ZPL_IPCSTANBY_ACK)
        {
            client->recv_cnt++;
            length -= ZPL_IPCMSG_HEADER_SIZE;
            client->last_read_time = os_time(NULL);
            if (ack)
            {
                ack->precoss = stream_getl(client->ibuf);
                ack->result = stream_getl(client->ibuf);
                ack->msglen = stream_getl(client->ibuf);
                length += sizeof(struct ipcstanby_result);
            }
            if (callback && callback->ipcstandby_callback && ack->result == OK)
            {
                (callback->ipcstandby_callback)(stream_pnt(client->ibuf), length, callback->pVoid);
            }
            stream_reset(client->ibuf);
            return OK;
        }
        stream_reset(client->ibuf);
        return ERROR;
    }
    stream_reset(client->ibuf);
    return ERROR;
}

int ipcstandby_client_switch(struct ipcstandby_client *client)
{
    struct ipcstanby_result ack;
    ipcstandby_create_header(client->obuf, ZPL_IPCSTANBY_SWITCH);
    IPCSTANDBY_LOCK();
    stream_putl(client->obuf, client->ipcstanby.ID);
    stream_putl(client->obuf, ++client->ipcstanby.seqnum);
    stream_putc(client->obuf, client->ipcstanby.state);
    stream_putc(client->obuf, client->ipcstanby.slot);
    stream_putc(client->obuf, client->ipcstanby.MS);
    stream_putc(client->obuf, client->ipcstanby.master);
    IPCSTANDBY_UNLOCK();
    return ipcstandby_client_send_message(client, &ack, NULL, 500);
}

int ipcstandby_client_send_message(struct ipcstandby_client *client, struct ipcstanby_result *ack,
                                   struct ipcstandby_callback *callback, int timeout)
{
    int ret = 0, len = stream_get_endp(client->obuf);
    char *buf = STREAM_DATA(client->obuf);
    if (!client->is_connect)
        return ERROR;
    stream_putw_at(client->obuf, 0, stream_get_endp(client->obuf));
    while (1)
    {
        ret = ipstack_write(client->sock, buf, len);
        if (ret > 0)
        {
            len -= ret;
            buf += ret;
        }
        else
        {
            if (IPSTACK_ERRNO_RETRY(ipstack_errno))
            {
                continue;
            }
            client->send_faild_cnt++;
            ipcstandby_client_failed(client);
            return ERROR;
        }
        if (len == 0)
        {
            client->send_cnt++;
            client->last_write_time = os_time(NULL);
            ret = ipcstandby_client_recv_message(client, ack, callback, timeout);
            if (ret == ERROR)
                return ERROR;
            if (ret == OS_CLOSE)
            {
                ipcstandby_client_failed(client);
            }
            return OK;
        }
    }
    return ERROR;
}

int ipcstandby_client_sendmsg(struct ipcstandby_client *client, struct ipcstanby_result *ack,
                              struct ipcstandby_callback *callback, int cmd, char *msg, int len)
{
    ipcstandby_create_header(client->obuf, cmd);
    stream_put(client->obuf, msg, len);
    // stream_putw_at(s, 0, stream_get_endp(s));
    return ipcstandby_client_send_message(client, ack, NULL, 500);
}

static void
ipcstandby_client_event(enum ipcmsg_event event, struct ipcstandby_client *client)
{
    switch (event)
    {
    case ZPL_IPCMSG_SCHEDULE:
        if (!client->t_connect)
            client->t_connect =
                thread_add_event(client->master, ipcstandby_client_connect, client, 0);
        break;
    case ZPL_IPCMSG_CONNECT:
        if (client->debug)
            zlog_debug(MODULE_STANDBY, "ipc standby client ipstack_connect schedule interval is %d",
                       client->fail < 3 ? 5 : 10);
        if (!client->t_connect)
            client->t_connect =
                thread_add_timer(client->master, ipcstandby_client_connect, client,
                                 client->fail < 3 ? 5 : 10);
        break;
    case ZPL_IPCMSG_TIMEOUT:
        if (!client->t_timeout)
            client->t_timeout = thread_add_timer(client->master, ipcstandby_client_timeout, client, ZPL_IPCSTANBY_TIMEOUT);
        break;
    default:
        break;
    }
}


