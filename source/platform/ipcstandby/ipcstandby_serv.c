/* ipcstandby daemon server routine.
 * Copyright (C) 1997, 98, 99 Kunihiro Ishiguro
 *
 * This file is part of GNU ipcstandby.
 *
 * GNU ipcstandby is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU ipcstandby is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU ipcstandby; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"

#include "thread.h"
#include "eloop.h"
#include "stream.h"
#include "network.h"
#include "sockunion.h"
#include "log.h"
#include "linklist.h"
#include "sockopt.h"
#include "buffer.h"
#include "vty_include.h"
#include "ipcstandby_serv.h"

/* Event list of ipcstandby. */

static void ipcstandby_serv_event(struct ipcstandby_server_t *_server, enum ipcmsg_event event, zpl_socket_t sock, struct ipcstandby_serv *client);

static void ipcstandby_serv_client_close(struct ipcstandby_serv *client);

int ipcstandby_serv_callback(struct ipcstandby_server_t *_server, struct ipcstandby_callback cli, struct ipcstandby_callback msg, struct ipcstandby_callback res)
{
    _server->cli_callback.ipcstandby_callback = cli.ipcstandby_callback;
    _server->cli_callback.pVoid = cli.pVoid;
    _server->msg_callback.ipcstandby_callback = msg.ipcstandby_callback;
    _server->msg_callback.pVoid = msg.pVoid;
    _server->res_callback.ipcstandby_callback = res.ipcstandby_callback;
    _server->res_callback.pVoid = res.pVoid;
    return OK;
}

static int ipcstandby_serv_server_send_message(struct ipcstandby_serv *client)
{
    int ret = 0, len = stream_get_endp(client->obuf);
    char *buf = STREAM_DATA(client->obuf);
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
            ipcstandby_serv_client_close(client);
            return ERROR;
        }
        if (len == 0)
        {
            client->send_cnt++;
            client->last_write_time = os_time(NULL);
            return OK;
        }
    }
    return ERROR;
}

int ipcstandby_serv_result(struct ipcstandby_serv *client, int process, int ret, char *data, int len)
{
    ipcstandby_create_header(client->obuf, ZPL_IPCSTANBY_ACK);
    stream_putl(client->obuf, process);
    stream_putl(client->obuf, ret);
    if (data)
    {
        stream_putl(client->obuf, sizeof(struct ipcstanby_result) + len);
        stream_put(client->obuf, data, len);
    }
    else
        stream_putl(client->obuf, sizeof(struct ipcstanby_result));
    return ipcstandby_serv_server_send_message(client);
}

/* Tie up route-type and client->sock */
static void ipcstandby_serv_read_hello(struct ipcstandby_serv *client)
{
    /* type of protocol (lib/zplos_include.h) */
    client->ipcstanby.slot = stream_getl(client->ibuf);
    client->ipcstanby.active = stream_getl(client->ibuf);
    client->ipcstanby.negotiate = stream_getl(client->ibuf);
    if (client->ipcserver && client->ipcserver->ipcstandby_negotiate_callback)
    {
        (client->ipcserver->ipcstandby_negotiate_callback)(&client->ipcstanby);
    }
    client->hello++;
}

static void ipcstandby_serv_read_register(struct ipcstandby_serv *client)
{
    client->ipcstanby.slot = stream_getl(client->ibuf);
    stream_get(client->version, client->ibuf, sizeof(client->version));
    _host_standby.state++;
}

/* Close ipcstandby client. */
static void
ipcstandby_serv_client_close(struct ipcstandby_serv *client)
{
    /* Close file descriptor. */
    if (!ipstack_invalid(client->sock))
    {
        ipstack_close(client->sock);
    }
    if (_host_standby.state)
        _host_standby.state--;
    /* Free stream buffers. */
    if (client->ibuf)
        stream_free(client->ibuf);
    if (client->obuf)
        stream_free(client->obuf);
    /* Release threads. */
    if (client->t_read)
        thread_cancel(client->t_read);
    listnode_delete(client->ipcserver->client_list, client);
    XFREE(MTYPE_STANDBY_SERV_CLIENT, client);
}

/* Make new client. */
static void
ipcstandby_serv_client_create(struct ipcstandby_server_t *_server, zpl_socket_t sock, struct ipstack_sockaddr_in *remote)
{
    struct ipcstandby_serv *client;
    // zpl_uint32 i = 0;

    client = XCALLOC(MTYPE_STANDBY_SERV_CLIENT, sizeof(struct ipcstandby_serv));

    /* Make client input/output buffer. */
    client->sock = sock;
    client->ibuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);
    client->obuf = stream_new(ZPL_IPCMSG_MAX_PACKET_SIZ);

    strcpy(client->remote, inet_ntoa(remote->sin_addr));
    client->port = ntohs(remote->sin_port);
    client->connect_time = os_time(NULL);
    client->ipcserver = _server;
    listnode_add(_server->client_list, client);
    if (_server->debug)
        zlog_warn(MODULE_STANDBY, "ipc standby server create client from %s:%d sock [%d] OK", client->remote, client->port, ipstack_fd(client->sock));
    /* Make new read thread. */
    ipcstandby_serv_event(_server, ZPL_IPCMSG_READ, sock, client);
}

static int
ipcstandby_serv_client_timeout(struct thread *thread)
{
    struct ipcstandby_serv *client;
    client = THREAD_ARG(thread);
    client->t_timeout = NULL;
    if (client->hello == 0)
    {
        if (client->ipcserver->debug)
            zlog_warn(MODULE_STANDBY, "ipc standby server close client %s:%d sock [%d] is timeout", client->remote, client->port, ipstack_fd(client->sock));
        client->state = zpl_false;
        ipcstandby_serv_client_close(client);
        return OK;
    }
    client->t_timeout = thread_add_timer(client->ipcserver->master, ipcstandby_serv_client_timeout, client, ZPL_IPCSTANBY_TIMEOUT);
    client->hello--;
    return OK;
}

/* Handler of ipcstandby service request. */
static int
ipcstandby_serv_client_read(struct thread *thread)
{
    zpl_socket_t sock;
    struct ipcstandby_serv *client;
    zpl_size_t already;
    zpl_uint16 length, command;
    zpl_uint8 marker, version;

    int ret = 0;
    /* Get thread data.  Reset reading thread because I'm running. */
    sock = THREAD_FD(thread);
    client = THREAD_ARG(thread);
    client->t_read = NULL;

    /* Read length and command (if we don't have it already). */
    if ((already = stream_get_endp(client->ibuf)) < ZPL_IPCMSG_HEADER_SIZE)
    {
        ssize_t nbyte;
        if (((nbyte = stream_read_try(client->ibuf, sock,
                                      ZPL_IPCMSG_HEADER_SIZE - already)) == 0) ||
            (nbyte == -1))
        {
            client->recv_faild_cnt++;
            if (IS_ZPL_IPCMSG_DEBUG_EVENT(client->ipcserver->debug))
                zlog_debug(MODULE_STANDBY, "ipc standby server connection closed [%d] from client %s:%d (%s)", ipstack_fd(sock), client->remote, client->port, strerror(errno));
            ipcstandby_serv_client_close(client);
            return -1;
        }
        if (nbyte != (ssize_t)(ZPL_IPCMSG_HEADER_SIZE - already))
        {
            /* Try again later. */
            ipcstandby_serv_event(client->ipcserver, ZPL_IPCMSG_READ, sock, client);
            return 0;
        }
        already = ZPL_IPCMSG_HEADER_SIZE;
    }

    /* Reset to read from the beginning of the incoming packet. */
    stream_set_getp(client->ibuf, 0);

    /* Fetch header values */
    length = stream_getw(client->ibuf);
    marker = stream_getc(client->ibuf);
    version = stream_getc(client->ibuf);
    command = stream_getw(client->ibuf);

    if (marker != ZPL_IPCMSG_HEADER_MARKER || version != ZPL_IPCMSG_VERSION)
    {
        client->pkt_err_cnt++;
        zlog_err(MODULE_STANDBY, "ipc standby server read socket %d version mismatch, marker %d, version %d from client %s:%d",
                 ipstack_fd(sock), marker, version, client->remote, client->port);
        ipcstandby_serv_client_close(client);
        return -1;
    }
    if (length < ZPL_IPCMSG_HEADER_SIZE)
    {
        client->pkt_err_cnt++;
        zlog_warn(MODULE_STANDBY, "ipc standby server recv socket %d message length %u is less than header size %d from client %s:%d ",
                  ipstack_fd(sock), length, ZPL_IPCMSG_HEADER_SIZE, client->remote, client->port);
        ipcstandby_serv_client_close(client);
        return -1;
    }
    if (length > STREAM_SIZE(client->ibuf))
    {
        client->pkt_err_cnt++;
        zlog_warn(MODULE_STANDBY, "ipc standby server read %d message length %u exceeds buffer size %lu from client %s:%d ",
                  ipstack_fd(sock), length, (u_long)STREAM_SIZE(client->ibuf), client->remote, client->port);
        ipcstandby_serv_client_close(client);
        return -1;
    }

    /* Read rest of data. */
    if (already < length)
    {
        ssize_t nbyte;
        if (((nbyte = stream_read_try(client->ibuf, sock,
                                      length - already)) == 0) ||
            (nbyte == -1))
        {
            client->recv_faild_cnt++;
            if (IS_ZPL_IPCMSG_DEBUG_EVENT(client->ipcserver->debug))
                zlog_debug(MODULE_STANDBY, "ipc standby server connection closed [%d] from client %s:%d when reading ipcstandby data", ipstack_fd(sock), client->remote, client->port);
            ipcstandby_serv_client_close(client);
            return -1;
        }
        if (nbyte != (ssize_t)(length - already))
        {
            /* Try again later. */
            ipcstandby_serv_event(client->ipcserver, ZPL_IPCMSG_READ, sock, client);
            return 0;
        }
    }
    client->recv_cnt++;
    length -= ZPL_IPCMSG_HEADER_SIZE;

    /* Debug packet information. */
    if (IS_ZPL_IPCMSG_DEBUG_EVENT(client->ipcserver->debug))
        zlog_debug(MODULE_STANDBY, "ipc standby server read message comes from  client %s:%d sock [%d]", client->remote, client->port, ipstack_fd(sock));

    if (IS_ZPL_IPCMSG_DEBUG_PACKET(client->ipcserver->debug) && IS_ZPL_IPCMSG_DEBUG_RECV(client->ipcserver->debug))
        zlog_debug(MODULE_STANDBY, "ipc standby server message received client %s:%d cmd [%d] len %d",
                   client->remote, client->port, (command), length);

    client->last_read_time = os_time(NULL);

    switch (command)
    {
    case ZPL_IPCSTANBY_HELLO:
        ipcstandby_serv_read_hello(client);
        break;
    case ZPL_IPCSTANBY_REGISTER:
        ipcstandby_serv_read_register(client);
        ret = OK;
        break;

    case ZPL_IPCSTANBY_CLI:
        if (client->ipcserver->cli_callback.ipcstandby_callback)
            ret = (client->ipcserver->cli_callback.ipcstandby_callback)(stream_pnt(client->ibuf), 0, client->ipcserver->cli_callback.pVoid);
        else
            ret = OS_NO_CALLBACK;
        break;
    case ZPL_IPCSTANBY_MSG:
        if (client->ipcserver->msg_callback.ipcstandby_callback)
            ret = (client->ipcserver->msg_callback.ipcstandby_callback)(stream_pnt(client->ibuf), 0, client->ipcserver->msg_callback.pVoid);
        else
            ret = OS_NO_CALLBACK;
        break;
    case ZPL_IPCSTANBY_RES:
        if (client->ipcserver->res_callback.ipcstandby_callback)
            ret = (client->ipcserver->res_callback.ipcstandby_callback)(stream_pnt(client->ibuf), 0, client);
        else
            ret = OS_NO_CALLBACK;
        break;

    default:
        zlog_info(MODULE_STANDBY, "ipc standby server received unknown command %d from client %s:%d", command, client->remote, client->port);
        ret = OS_UNKNOWN_CMD;
        break;
    }
    if (command != ZPL_IPCSTANBY_RES)
    {
        if (ret == OK)
            ipcstandby_serv_result(client, 0, ret, NULL, 0);
        else
            ipcstandby_serv_result(client, 0, ret, zpl_strerror(ret), strlen(zpl_strerror(ret)));
    }
    stream_reset(client->ibuf);
    ipcstandby_serv_event(client->ipcserver, ZPL_IPCMSG_READ, sock, client);
    return 0;
}

/* Accept code of ipcstandby server ipstack_socket. */
static int
ipcstandby_serv_accept(struct thread *thread)
{
    zpl_socket_t accept_sock;
    zpl_socket_t client_sock;
    struct ipcstandby_server_t *_server;
    struct ipstack_sockaddr_in client;
    socklen_t len;
    _server = THREAD_ARG(thread);
    accept_sock = THREAD_FD(thread);

    /* Reregister myself. */
    ipcstandby_serv_event(_server, ZPL_IPCMSG_ACCEPT, accept_sock, NULL);

    len = sizeof(struct ipstack_sockaddr_in);
    client_sock = ipstack_accept(accept_sock, (struct ipstack_sockaddr *)&client, &len);

    if (ipstack_invalid(client_sock))
    {
        zlog_warn(MODULE_STANDBY, "ipc standby server Can't ipstack_accept  ipstack_socket: %s", ipstack_strerror(ipstack_errno));
        return -1;
    }

    /* Make client ipstack_socket non-blocking.  */
    ipstack_set_nonblocking(client_sock);

    /* Create new ipcstandby client. */
    ipcstandby_serv_client_create(_server, client_sock, &client);

    return 0;
}

/* Make ipcstandby's server ipstack_socket. */
static zpl_socket_t ipcstandby_serv_tcp(char *ip, int port)
{
    int ret;
    zpl_socket_t accept_sock;
    struct ipstack_sockaddr_in addr;

    accept_sock = ipstack_socket(IPSTACK_IPCOM, IPSTACK_AF_INET, IPSTACK_SOCK_STREAM, 0);

    if (ipstack_invalid(accept_sock))
    {
        zlog_warn(MODULE_STANDBY, "ipc standby server Can't create zserv stream socket: %s",
                  ipstack_strerror(ipstack_errno));
        // zlog_warn (MODULE_STANDBY, "ipcstandby can't provice full functionality due to above error");
        return accept_sock;
    }

    memset(&addr, 0, sizeof(struct ipstack_sockaddr_in));
    addr.sin_family = IPSTACK_AF_INET;
    addr.sin_port = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    addr.sin_len = sizeof(struct ipstack_sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
    if (ip)
        addr.sin_addr.s_addr = inet_addr(ip);
    else
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    sockopt_reuseaddr(accept_sock);
    sockopt_reuseport(accept_sock);

    ret = ipstack_bind(accept_sock, (struct ipstack_sockaddr *)&addr,
                       sizeof(struct ipstack_sockaddr_in));
    if (ret < 0)
    {
        zlog_warn(MODULE_STANDBY, "ipc standby server Can't ipstack_bind to stream ipstack_socket: %s",
                  ipstack_strerror(ipstack_errno));
        ipstack_close(accept_sock); /* Avoid sd leak. */
        return accept_sock;
    }

    ret = ipstack_listen(accept_sock, 1);
    if (ret < 0)
    {
        zlog_warn(MODULE_STANDBY, "ipc standby server Can't ipstack_listen to stream ipstack_socket: %s",
                  ipstack_strerror(ipstack_errno));
        ipstack_close(accept_sock); /* Avoid sd leak. */
        return accept_sock;
    }

    // ipcstandby_serv_event(_server, ZPL_IPCMSG_ACCEPT, accept_sock, NULL);
    return accept_sock;
}

static void
ipcstandby_serv_event(struct ipcstandby_server_t *_server, enum ipcmsg_event event, zpl_socket_t sock, struct ipcstandby_serv *client)
{
    switch (event)
    {
    case ZPL_IPCMSG_ACCEPT:
        _server->t_accept = thread_add_read(_server->master, ipcstandby_serv_accept, _server, sock);
        break;
    case ZPL_IPCMSG_READ:
        client->t_read = thread_add_read(_server->master, ipcstandby_serv_client_read, client, sock);
        break;
    case ZPL_IPCMSG_TIMEOUT:
        client->t_timeout = thread_add_timer(_server->master, ipcstandby_serv_client_timeout, client, ZPL_IPCSTANBY_TIMEOUT);
        break;

    case ZPL_IPCMSG_WRITE:
        /**/
        break;
    case ZPL_IPCMSG_NONE:
    case ZPL_IPCMSG_CONNECT:
    case ZPL_IPCMSG_EVENT:
    case ZPL_IPCMSG_SCHEDULE:
        break;
    }
}

/* Make ipcstandby server ipstack_socket, wiping any existing one (see bug #403). */
struct ipcstandby_server_t *ipcstandby_serv_init(void *m, int port)
{
    struct ipcstandby_server_t *_server = XCALLOC(MTYPE_STANDBY_SERV, sizeof(struct ipcstandby_server_t));
    if (_server)
    {
        memset(_server, 0, sizeof(struct ipcstandby_server_t));
        _server->master = m;
        _server->port = port;
        _server->client_list = list_new();
        _server->serv_sock = ipcstandby_serv_tcp(NULL, port);
        _server->vty = vty_new();
        _server->vty->wfd = ipstack_init(IPSTACK_OS, STDOUT_FILENO);
        _server->vty->fd = ipstack_init(IPSTACK_OS, STDIN_FILENO);
        // ipstack_fd(_server->vty->wfd) = STDOUT_FILENO;
        // ipstack_fd(_server->vty->fd) = STDIN_FILENO;
        _server->vty->type = VTY_STABDVY;
        _server->vty->node = ENABLE_NODE;
        memset(_server->vty->buf, 0, (VTY_BUFSIZ));
        ipcstandby_serv_event(_server, ZPL_IPCMSG_ACCEPT, _server->serv_sock, NULL);
    }
    return _server;
}

void ipcstandby_serv_exit(struct ipcstandby_server_t *_server)
{
    struct listnode *node;
    struct ipcstandby_serv *client;

    for (ALL_LIST_ELEMENTS_RO(_server->client_list, node, client))
        ipcstandby_serv_client_close(client);
    if (_server->t_accept)
        thread_cancel(_server->t_accept);
    if (!ipstack_invalid(_server->serv_sock))
    {
        ipstack_close(_server->serv_sock);
    }
    XFREE(MTYPE_STANDBY_SERV, _server);
    _server = NULL;
}
