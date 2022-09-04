#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "thread.h"
#include "if.h"
#include "vty.h"
#include "command.h"
#include "host.h"
#include "sockopt.h"
#include "hal_ipcsrv.h"
#include "hal_ipcmsg.h"
#include "hal_ipccmd.h"
#include "hal_event.h"
#include "if_utsp.h"

struct hal_ipcsrv _ipcsrv;

static int hal_ipcsrv_client_read(struct thread *thread);

#ifdef ZPL_SHELL_MODULE
static void hal_ipcsrv_cli(void);
#endif
/* Close zebra client. */
static void hal_ipcclient_client_free(struct hal_ipcclient *client)
{
    if (client->t_read)
        thread_cancel(client->t_read);
    if (!ipstack_invalid(client->sock))
    {
        ipstack_close(client->sock);
    }
}
static void hal_ipcclient_client_close(struct hal_ipcsrv *ipcsrv, struct hal_ipcclient *client)
{
    if (IS_HAL_IPCMSG_DEBUG_EVENT(ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "Client unit %d slot %d portnum %d close [%d]", client->unit, client->slot, client->portnum, ipstack_fd(client->sock));
    }
    /* Close file descriptor. */
    hal_ipcclient_client_free(client);
    /* Free client structure. */
    listnode_delete(ipcsrv->client_list, client);
    XFREE(MTYPE_HALIPCCLIENT, client);
}

/* Make new client. */
static int
hal_ipcclient_client_create(struct hal_ipcsrv *ipcsrv, struct ipstack_sockaddr_in *clientaddr, zpl_socket_t sock, enum hal_ipctype_e ev)
{
    struct hal_ipcclient *client;
    client = XCALLOC(MTYPE_HALIPCCLIENT, sizeof(struct hal_ipcclient));
    if (client == NULL)
        return ERROR;
    /* Make client input/output buffer. */
    client->sock = sock;

    memcpy(&client->clientaddr, clientaddr, sizeof(struct ipstack_sockaddr_in));

    client->ipcsrv = ipcsrv;

    client->ipctype = ev;
    client->module = -1;
    client->unit = -1;
    client->slot = -1;
    client->portnum = -1;

    /* Add this client to linked list. */

    listnode_add(ipcsrv->client_list, client);
    client->t_read = thread_add_read(ipcsrv->master, hal_ipcsrv_client_read, client, sock);

    return OK;
}

/* Accept code of hal_ipcclient server ipstack_socket. */
static int hal_ipcsrv_accept(struct thread *thread)
{
    zpl_socket_t accept_sock;
    zpl_socket_t client_sock;
    zpl_bool event = zpl_false;
    struct ipstack_sockaddr_in client;
    socklen_t len = 0;
    struct hal_ipcsrv *ipcsrv = THREAD_ARG(thread);
    accept_sock = THREAD_FD(thread);

    /* Reregister myself. */
    if (ipstack_same(ipcsrv->unixsock, accept_sock))
    {
        ipcsrv->u_accept = thread_add_read(ipcsrv->master, hal_ipcsrv_accept, ipcsrv, accept_sock);
    }
    else if (ipstack_same(ipcsrv->sock, accept_sock))
    {
        ipcsrv->t_accept = thread_add_read(ipcsrv->master, hal_ipcsrv_accept, ipcsrv, accept_sock);
    }
    len = sizeof(struct ipstack_sockaddr_in);
    client_sock = ipstack_accept(accept_sock, (struct ipstack_sockaddr *)&client, &len);

    if (ipstack_invalid(client_sock))
    {
        zlog_err(MODULE_HAL, "Can't ipstack_accept hal_ipcclient ipstack_socket: %s", ipstack_strerror(ipstack_errno));
        return -1;
    }
    zlog_force_trap(MODULE_HAL, "hal_client_start client_sock sock = %d", ipstack_fd(client_sock));
    /* Make client ipstack_socket non-blocking.  */
    ipstack_set_nonblocking(client_sock);

    /* Create new hal_ipcclient client. */
    if (hal_ipcclient_client_create(ipcsrv, &client, client_sock, event) == OK)
        return OK;
    else
    {
        ipstack_close(client_sock);
        return ERROR;
    }
}

/* hal_ipcclient server UNIX domain ipstack_socket. */
static zpl_socket_t hal_ipcsrv_un(const char *path)
{
    int ret;
    int len;
    zpl_socket_t unixsock;
    struct ipstack_sockaddr_un serv;
    unlink(path);
    /* Make UNIX domain ipstack_socket. */
    unixsock = ipstack_socket(OS_STACK, IPSTACK_AF_UNIX, IPSTACK_SOCK_STREAM, 0);
    if (ipstack_invalid(unixsock))
    {
        zlog_err(MODULE_HAL, "Can't create zserv unix ipstack_socket: %s",
                  ipstack_strerror(ipstack_errno));
        return unixsock;
    }
    /* Make server ipstack_socket. */
    memset(&serv, 0, sizeof(struct ipstack_sockaddr_un));
    serv.sun_family = IPSTACK_AF_UNIX;
    strncpy(serv.sun_path, path, strlen(path));
#ifdef HAVE_STRUCT_SOCKADDR_UN_SUN_LEN
    len = serv.sun_len = SUN_LEN(&serv);
#else
    len = sizeof(serv.sun_family) + strlen(serv.sun_path);
#endif /* HAVE_STRUCT_SOCKADDR_UN_SUN_LEN */

    ret = ipstack_bind(unixsock, (struct ipstack_sockaddr *)&serv, len);
    if (ret < 0)
    {
        _OS_ERROR("Can't ipstack_bind to unix ipstack_socket %s: %s",
                  path, ipstack_strerror(ipstack_errno));
        zlog_err(MODULE_HAL, "Can't ipstack_bind to unix ipstack_socket %s: %s",
                  path, ipstack_strerror(ipstack_errno));
        ipstack_close(unixsock);
        return unixsock;
    }

    ret = ipstack_listen(unixsock, 5);
    if (ret < 0)
    {
        zlog_err(MODULE_HAL, "Can't ipstack_listen to unix ipstack_socket %s: %s",
                  path, ipstack_strerror(ipstack_errno));
        ipstack_close(unixsock);
        return unixsock;
    }
    return unixsock;
}

static zpl_socket_t hal_ipcsrv_socket(int port)
{
    int ret;
    zpl_socket_t sock;
    struct ipstack_sockaddr_in addr;

    sock = ipstack_socket(OS_STACK, IPSTACK_AF_INET, IPSTACK_SOCK_STREAM, 0);

    if (ipstack_invalid(sock))
    {
        zlog_err(MODULE_HAL, "Can't create zserv stream ipstack_socket: %s",
                  ipstack_strerror(ipstack_errno));
        // zlog_warn (MODULE_HAL, "hal_ipcclient can't provice full functionality due to above error");
        return sock;
    }

    memset(&addr, 0, sizeof(struct ipstack_sockaddr_in));
    addr.sin_family = IPSTACK_AF_INET;
    addr.sin_port = htons(port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    addr.sin_len = sizeof(struct ipstack_sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
    addr.sin_addr.s_addr = htonl(IPSTACK_INADDR_LOOPBACK);

    sockopt_reuseaddr(sock);
    sockopt_reuseport(sock);

    ret = ipstack_bind(sock, (struct ipstack_sockaddr *)&addr,
                       sizeof(struct ipstack_sockaddr_in));
    if (ret < 0)
    {
        _OS_ERROR("Can't ipstack_bind to unix ipstack_socket %d: %s",port, ipstack_strerror(ipstack_errno));
        zlog_err(MODULE_HAL, "Can't ipstack_bind to stream ipstack_socket: %s", ipstack_strerror(ipstack_errno));
        // zlog_warn (MODULE_HAL, "hal_ipcclient can't provice full functionality due to above error");
        ipstack_close(sock); /* Avoid sd leak. */
        return sock;
    }

    ret = ipstack_listen(sock, 1);
    if (ret < 0)
    {
        zlog_err(MODULE_HAL, "Can't ipstack_listen to stream ipstack_socket: %s",ipstack_strerror(ipstack_errno));
        ipstack_close(sock); /* Avoid sd leak. */
        return sock;
    }
    return sock;
}

static void hal_ipcsrv_msg_hello(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg)
{
    struct hal_ipcmsg_hello *hello = (struct hal_ipcmsg_hello *)(ipcmsg->buf + ipcmsg->getp);
    client->ipctype = (enum hal_ipctype_e)hello->stype;
    client->module = hello->module;
    client->unit = hello->unit;
    client->slot = hello->slot;
    client->portnum = hello->portnum;
    client->board = unit_board_add(hello->unit, hello->slot);
    strcpy(client->version, hello->version);
    client->state = HAL_CLIENT_CONNECT;
    if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug))
        zlog_debug(MODULE_HAL, "client state change to connect");
    ipcmsg->getp += sizeof(struct hal_ipcmsg_hello);
    if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "Server Recv hello msg from [%d] unit %d slot %d portnum %d version %s", ipstack_fd(client->sock),
                   client->unit, client->slot, client->portnum, client->version);
    }
}

static void hal_ipcsrv_msg_register(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg)
{
    zpl_int32 value = 0;
    hal_ipcmsg_getl(ipcmsg, &value);

    if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "Server Recv init msg from [%d] unit %d slot %d", ipstack_fd(client->sock),
                   client->unit, client->slot);
    }
    client->state = HAL_CLIENT_INIT;
    if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug))
        zlog_debug(MODULE_HAL, "client state change to init");
    return;
}

static void hal_ipcsrv_msg_hwport(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg)
{
    zpl_uint32 i = 0;
    zpl_uint8 portnum = 0;
    struct hal_ipcmsg_hwport porttbl;
    hal_ipcmsg_getc(ipcmsg, &portnum);
    for (i = 0; i < portnum; i++)
    {
        hal_ipcmsg_getc(ipcmsg, &porttbl.unit);
        hal_ipcmsg_getc(ipcmsg, &porttbl.slot);
        hal_ipcmsg_getc(ipcmsg, &porttbl.type);
        hal_ipcmsg_getc(ipcmsg, &porttbl.port);
        hal_ipcmsg_getl(ipcmsg, &porttbl.phyid);
        unit_board_port_add(client->board, (if_type_t)porttbl.type, porttbl.port, porttbl.phyid);
    }
    if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "Server Recv PortTable msg from [%d] unit %d slot %d portnum %d", ipstack_fd(client->sock),
                   client->unit, client->slot, portnum);
    }
    return;
}

static void hal_ipcsrv_msg_start_done(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg)
{
    zpl_int32 value = 0;
    hal_ipcmsg_getl(ipcmsg, &value);
    if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "Server Recv done msg from [%d] unit %d slot %d portnum %d", ipstack_fd(client->sock),
                   client->unit, client->slot, client->portnum);
    }
    client->state = HAL_CLIENT_REGISTER;
    if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug))
        zlog_debug(MODULE_HAL, "client state change to register");


    host_bspinit_done();

    return OK;
}

static void hal_ipcsrv_msg_report(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg)
{
    struct hal_ipcmsg_event report;
    hal_ipcmsg_getc(ipcmsg, &report.unit);
    hal_ipcmsg_getc(ipcmsg, &report.slot);
    hal_ipcmsg_getc(ipcmsg, &report.module);
    hal_ipcmsg_getc(ipcmsg, &report.event);

    if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "Server Recv Report msg from [%d] unit %d slot %d", ipstack_fd(client->sock),
                   client->unit, client->slot);
    }
    hal_event_handler(&report, HAP_IPCMSG_PNT(ipcmsg), hal_ipcmsg_get_setp(ipcmsg)-hal_ipcmsg_get_getp(ipcmsg));
    return;
}

/* Handler of zebra service request. */
static int hal_ipcsrv_client_read(struct thread *thread)
{
    zpl_socket_t sock;
    struct hal_ipcclient *client = NULL;
    struct hal_ipcmsg_header *hdr = NULL;
    struct hal_ipcmsg *input_ipcmsg = NULL;
    /* Get thread data.  Reset reading thread because I'm running. */
    sock = THREAD_FD(thread);
    client = THREAD_ARG(thread);
    zpl_assert(client);
    zpl_assert(client->ipcsrv);
    client->t_read = NULL;
    input_ipcmsg = &client->ipcsrv->input_msg;
    hdr = (struct hal_ipcmsg_header *)input_ipcmsg->buf;

    /* Read length and command (if we don't have it already). */
    if (input_ipcmsg->setp < HAL_IPCMSG_HEADER_SIZE)
    {
        ssize_t nbyte = 0;
        nbyte = ipstack_read(sock, input_ipcmsg->buf + input_ipcmsg->setp, HAL_IPCMSG_HEADER_SIZE - input_ipcmsg->setp);
        if ((nbyte == 0) || (nbyte == -1))
        {
            if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug))
                zlog_err(MODULE_HAL, "connection closed ipstack_socket [%d]", ipstack_fd(sock));
            hal_ipcclient_client_close(client->ipcsrv, client);
            return -1;
        }
        if (nbyte != (ssize_t)(HAL_IPCMSG_HEADER_SIZE - input_ipcmsg->setp))
        {
            /* Try again later. */
            client->t_read = thread_add_read(client->ipcsrv->master, hal_ipcsrv_client_read, client, sock);
            return 0;
        }
        input_ipcmsg->setp = HAL_IPCMSG_HEADER_SIZE;
    }

    /* Fetch header values */
    hdr->length = ntohs(hdr->length);
    hdr->command = ntohl(hdr->command);

    if (hdr->marker != HAL_IPCMSG_HEADER_MARKER || hdr->version != HAL_IPCMSG_VERSION)
    {
        zlog_err(MODULE_HAL, "%s: ipstack_socket %d version mismatch, marker %d, version %d",
                 __func__, ipstack_fd(sock), hdr->marker, hdr->version);
        hal_ipcclient_client_close(client->ipcsrv, client);
        return -1;
    }
    if (hdr->length < HAL_IPCMSG_HEADER_SIZE)
    {
        zlog_warn(MODULE_HAL, "%s: ipstack_socket %d message length %u is less than header size %u",
                  __func__, ipstack_fd(sock), hdr->length, (zpl_uint32)HAL_IPCMSG_HEADER_SIZE);
        hal_ipcclient_client_close(client->ipcsrv, client);
        return -1;
    }
    if (hdr->length > input_ipcmsg->length_max)
    {
        zlog_warn(MODULE_HAL, "%s: ipstack_socket %d message length %u exceeds buffer size %lu",
                  __func__, ipstack_fd(sock), hdr->length, (u_long)(input_ipcmsg->length_max));
        hal_ipcclient_client_close(client->ipcsrv, client);
        return -1;
    }

    /* Read rest of data. */
    if (input_ipcmsg->setp < hdr->length)
    {
        ssize_t nbyte;
        if (((nbyte = ipstack_read(sock, input_ipcmsg->buf + input_ipcmsg->setp,
                                   hdr->length - input_ipcmsg->setp)) == 0) ||
            (nbyte == -1))
        {
            if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug))
                zlog_err(MODULE_HAL, "connection closed [%d] when reading zebra data", ipstack_fd(sock));
            hal_ipcclient_client_close(client->ipcsrv, client);
            return -1;
        }
        if (nbyte != (ssize_t)(hdr->length - input_ipcmsg->setp))
        {
            /* Try again later. */
            client->t_read = thread_add_read(client->ipcsrv->master, hal_ipcsrv_client_read, client, sock);
            return 0;
        }
        input_ipcmsg->setp += hdr->length;
    }

    hdr->length -= HAL_IPCMSG_HEADER_SIZE;

    /* Debug packet information. */
    // if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug))
    //     zlog_debug(MODULE_HAL, "CLIENT message comes from ipstack_socket [%d] %d bytes", sock, hdr->length);

    if (IS_HAL_IPCMSG_DEBUG_PACKET(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
        zlog_debug(MODULE_HAL, "Server Recv message comes from [%d] cmd [%s] %d bytes",
                   ipstack_fd(sock), hal_module_cmd_name(hdr->command), hdr->length);

    input_ipcmsg->getp = HAL_IPCMSG_HEADER_SIZE;

    if (IS_HAL_IPCMSG_DEBUG_PACKET(client->ipcsrv->debug) &&
        IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug) &&
        IS_HAL_IPCMSG_DEBUG_HEX(client->ipcsrv->debug))
        hal_ipcmsg_hexmsg(input_ipcmsg, HAL_IPCMSG_HEADER_SIZE + hdr->length, "SERVER Recv");

    switch (IPCCMD_MODULE_GET(hdr->command))
    {
    case HAL_MODULE_MGT:
        if (IPCCMD_CMD_GET(hdr->command) == HAL_MODULE_CMD_HELLO)
        {
            hal_ipcsrv_msg_hello(client, input_ipcmsg);
        }
        else if (IPCCMD_CMD_GET(hdr->command) == HAL_MODULE_CMD_STARTONDE) //初始化完毕
        {
            hal_ipcsrv_msg_start_done(client, input_ipcmsg);
        }
        else if (IPCCMD_CMD_GET(hdr->command) == HAL_MODULE_CMD_REGISTER) //初始化
        {
            hal_ipcsrv_msg_register(client, input_ipcmsg);
        }
        else if (IPCCMD_CMD_GET(hdr->command) == HAL_MODULE_CMD_HWPORTTBL) //板卡端口信息
        {
            hal_ipcsrv_msg_hwport(client, input_ipcmsg);
        }
        else if (IPCCMD_CMD_GET(hdr->command) == HAL_MODULE_CMD_REPORT) //上报信息
        {
            hal_ipcsrv_msg_report(client, input_ipcmsg);
        }
        
#if defined(HAL_IPCSRV_SYNC_ACK)
        else if (IPCCMD_CMD_GET(hdr->command) == HAL_MODULE_CMD_ACK) //应答信息
        {
            struct timeval current_timeval;
            os_get_monotonic(&current_timeval);
            hal_ipcmsg_msg_copy(&client->ipcsrv->ack_msg, input_ipcmsg);
            if (os_timeval_cmp(client->ipcsrv->wait_timeval, current_timeval) && !ipstack_invalid(client->ipcsrv->waitfd[1]))
                ipstack_send(client->ipcsrv->waitfd[1], &hdr->command, 4, 0);
        }
#endif
        break;
    default:
        zlog_warn(MODULE_HAL, "Server Recv unknown command %d", IPCCMD_CMD_GET(hdr->command));
        break;
    }
    hal_ipcmsg_reset(input_ipcmsg);
#if defined(HAL_IPCSRV_SYNC_ACK)
    client->t_read = thread_add_read(client->ipcsrv->master, hal_ipcsrv_client_read, client, sock);
#else
    if (client->ipctype == HAL_IPCTYPE_EVENT)
        client->t_read = thread_add_read(client->ipcsrv->master, hal_ipcsrv_client_read, client, sock);
    else
    {
        if (client->state != HAL_CLIENT_REGISTER)
        {
            client->t_read = thread_add_read(client->ipcsrv->master, hal_ipcsrv_client_read, client, sock);
        }
    }
#endif
    return 0;
}

#ifdef HAL_IPCSRV_SYNC_ACK
static int hal_ipcsrv_recv_message_client(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg, struct hal_ipcmsg_result *getvalue,
                                          struct hal_ipcmsg_callback *callback, int timeout_ms)
{
    int bytes = 0, timeoutval = timeout_ms;
    zpl_size_t already = 0;
    int current_timeval = 0;
    int start_timeval = 0;
    char cmdtmp[8];
    if (client->state != HAL_CLIENT_REGISTER)
        return ERROR;
    start_timeval = os_get_monotonic_msec();
    while (bytes < 4)
    {
        already = ipstack_read_timeout(client->ipcsrv->waitfd[0], (zpl_char *)(cmdtmp + bytes), 4 - bytes, timeoutval);
        if (already == OS_TIMEOUT)
        {
            zlog_warn(MODULE_HAL, "Server Recv msg from [%d] unit %d is timeout", ipstack_fd(client->sock), client->unit);
            return OS_TIMEOUT;
        }
        else if (already == ERROR)
        {
            zlog_err(MODULE_HAL, "Server Recv msg from [%d] unit %d is ERROR:%s", ipstack_fd(client->sock), client->unit, ipstack_strerror(ipstack_errno));
            return ERROR;
        }
        if (already != (ssize_t)(4 - bytes))
        {
            /* Try again later. */
            bytes += already;
            current_timeval = os_get_monotonic_msec();
            timeoutval -= (current_timeval - start_timeval);
            if (timeoutval <= 0)
            {
                zlog_warn(MODULE_HAL, "Server Recv msg from [%d] unit %d is timeout", ipstack_fd(client->sock), client->unit);
                return OS_TIMEOUT;
            }
            continue;
        }
        bytes = 4;
        current_timeval = os_get_monotonic_msec();
        timeoutval -= (current_timeval - start_timeval);
        break;
    }    
    if (bytes) //应答信息
    {
        int ret = OK;
        ipcmsg->getp = HAL_IPCMSG_HEADER_SIZE;
        if (ret == OK)
        {
            char *msg_result_str = NULL;
            struct hal_ipcmsg_result getvaluetmp;
            hal_ipcmsg_result_get(ipcmsg, &getvaluetmp);
            if (getvalue)
            {
                memcpy(getvalue, &getvaluetmp, sizeof(struct hal_ipcmsg_result));
            }
            msg_result_str = (ipcmsg->buf + ipcmsg->getp);
            if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
            {
                zlog_debug(MODULE_HAL, "Server Recv msg from [%d] result %d %s", ipstack_fd(client->sock), getvaluetmp.result,
                           (getvaluetmp.result != OK) ? msg_result_str : "OK");
            }
            if (getvaluetmp.result == OK)
            {
                if (callback && callback->ipcmsg_callback)
                {
                    callback->ipcmsg_callback(ipcmsg->buf + ipcmsg->getp, ipcmsg->setp - ipcmsg->getp, callback->pVoid);
                }
                return OK;
            }
            return getvaluetmp.result;
        }
    }
    return ERROR;
}
#else
static int hal_ipcsrv_recv_message_client(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg, struct hal_ipcmsg_result *getvalue,
                                          struct hal_ipcmsg_callback *callback, int timeout_ms)
{
    int bytes = 0, timeoutval = timeout_ms;
    zpl_size_t already = 0;
    int current_timeval = 0;
    int start_timeval = 0;

    if (client->state != HAL_CLIENT_REGISTER)
        return ERROR;
    start_timeval = os_get_monotonic_msec();
    hal_ipcmsg_reset(ipcmsg);
    while (ipcmsg->setp < HAL_IPCMSG_HEADER_SIZE)
    {
        already = ipstack_read_timeout(client->sock, (zpl_char *)ipcmsg->buf + ipcmsg->setp, HAL_IPCMSG_HEADER_SIZE - ipcmsg->setp, timeoutval);
        if (already == OS_TIMEOUT)
        {
            zlog_warn(MODULE_HAL, "Server Recv msg from [%d] unit %d is timeout", ipstack_fd(client->sock), client->unit);
            return OS_TIMEOUT;
        }
        else if (already == ERROR)
        {
            zlog_err(MODULE_HAL, "Server Recv msg from [%d] unit %d is ERROR:%s", ipstack_fd(client->sock), client->unit, ipstack_strerror(ipstack_errno));
            return ERROR;
        }
        if (already != (ssize_t)(HAL_IPCMSG_HEADER_SIZE - ipcmsg->setp))
        {
            /* Try again later. */
            current_timeval = os_get_monotonic_msec();
            timeoutval -= (current_timeval - start_timeval);
            if (timeoutval <= 0)
            {
                zlog_warn(MODULE_HAL, "Server Recv msg from [%d] unit %d is timeout", ipstack_fd(client->sock), client->unit);
                return OS_TIMEOUT;
            }
            continue;
        }
        ipcmsg->setp = HAL_IPCMSG_HEADER_SIZE;
        current_timeval = os_get_monotonic_msec();
        timeoutval -= (current_timeval - start_timeval);
        break;
    }
    if (ipcmsg->setp == HAL_IPCMSG_HEADER_SIZE)
    {
        struct hal_ipcmsg_header *hdr = (struct hal_ipcmsg_header *)ipcmsg->buf;
        already = 0;
        ipcmsg->getp = HAL_IPCMSG_HEADER_SIZE;
        hdr->length = ntohs(hdr->length);
        hdr->command = ntohl(hdr->command);
        while (1)
        {
            bytes = ipstack_read_timeout(client->sock, (zpl_char *)ipcmsg->buf + ipcmsg->setp,
                                         (hdr->length) - ipcmsg->setp, timeoutval);

            if (bytes == ((hdr->length) - ipcmsg->setp))
            {
                ipcmsg->setp += bytes;
                if (IPCCMD_MODULE_GET((hdr->command)) == HAL_MODULE_MGT)
                {
                    if (IPCCMD_CMD_GET((hdr->command)) == HAL_MODULE_CMD_ACK)
                    {
                        char *msg_result_str = NULL;
                        struct hal_ipcmsg_result getvaluetmp;
                        hal_ipcmsg_result_get(ipcmsg, &getvaluetmp);
                        if (getvalue)
                        {
                            memcpy(getvalue, &getvaluetmp, sizeof(struct hal_ipcmsg_result));
                        }
                        msg_result_str = (ipcmsg->buf + ipcmsg->getp);
                        if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
                        {
                            zlog_debug(MODULE_HAL, "Server Recv msg from [%d] result %d %s", ipstack_fd(client->sock), getvaluetmp.result,
                                       (getvaluetmp.result != OK) ? msg_result_str : "OK");
                        }
                        if (getvaluetmp.result == OK)
                        {
                            if (callback && callback->ipcmsg_callback)
                            {
                                callback->ipcmsg_callback(ipcmsg->buf + ipcmsg->getp, ipcmsg->setp - ipcmsg->getp, callback->pVoid);
                            }
                            return OK;
                        }
                        return getvaluetmp.result;
                    }
                }
                return ERROR;
            }
            else if (bytes > 0)
            {
                ipcmsg->setp += bytes;
                current_timeval = os_get_monotonic_msec();
                timeoutval -= (current_timeval - start_timeval);
                if (timeoutval <= 0)
                {
                    zlog_warn(MODULE_HAL, "Server Recv msg from [%d] unit %d is timeout(%d-%d)", 
                        ipstack_fd(client->sock), client->unit, current_timeval, start_timeval);
                    return OS_TIMEOUT;
                }
            }
            else
            {
                if (bytes == OS_TIMEOUT)
                    zlog_warn(MODULE_HAL, "Server Recv msg from [%d] unit %d is timeout", ipstack_fd(client->sock), client->unit);
                else
                    zlog_err(MODULE_HAL, "Server Recv msg from [%d] unit %d is ERROR:%s", ipstack_fd(client->sock), client->unit, ipstack_strerror(ipstack_errno));
                return ERROR;
            }
        }
    }
    return ERROR;
}
#endif

static int hal_ipcsrv_send_message_client(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg, 
                struct hal_ipcmsg_result *getvalue,
                struct hal_ipcmsg_callback *callback, int timeout_ms)
{
    int bytes = 0;
    zpl_size_t already = 0;
    if (client->state != HAL_CLIENT_REGISTER)
    {
        zlog_warn(MODULE_HAL, "Server Send msg to [%d] unit %d is not connect", ipstack_fd(client->sock), client->unit);
        return ERROR;
    }
    if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_SEND(client->ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "Server Send msg to [%d] unit %d slot %d portnum %d %d byte", ipstack_fd(client->sock),
                   client->unit, client->slot, client->portnum, ipcmsg->setp);
    }
    if (IS_HAL_IPCMSG_DEBUG_PACKET(client->ipcsrv->debug) &&
        IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug) &&
        IS_HAL_IPCMSG_DEBUG_HEX(client->ipcsrv->debug))
        hal_ipcmsg_hexmsg(ipcmsg, ipcmsg->setp, "SERVER Send");
#ifndef HAL_IPCSRV_SYNC_ACK
    if (client->t_read)
        thread_cancel(client->t_read);
#else
    hal_ipcmsg_reset(&client->ipcsrv->ack_msg);
#endif
    while (1)
    {
        bytes = ipstack_write_timeout(client->sock, ipcmsg->buf + already, ipcmsg->setp - already, timeout_ms);
        if (bytes == (ipcmsg->setp - already))
            break;
        else if (bytes)
        {
            already += bytes;
        }
        else
        {
            if (bytes == OS_TIMEOUT)
                zlog_warn(MODULE_HAL, "Server Send msg to [%d] unit %d is timeout", ipstack_fd(client->sock), client->unit);
            else
                zlog_err(MODULE_HAL, "Server Send msg to [%d] unit %d is ERROR:%s", ipstack_fd(client->sock), client->unit, ipstack_strerror(ipstack_errno));
            return ERROR;
        }
    }
#ifdef HAL_IPCSRV_SYNC_ACK
    os_get_monotonic(&client->ipcsrv->wait_timeval);
    client->ipcsrv->wait_timeval.tv_sec += (timeout_ms/1000);
    client->ipcsrv->wait_timeval.tv_usec += ((timeout_ms%1000)*1000);
    return hal_ipcsrv_recv_message_client(client, &client->ipcsrv->ack_msg, getvalue, callback, timeout_ms);
#else
    return hal_ipcsrv_recv_message_client(client, &client->ipcsrv->input_msg, getvalue, callback, timeout_ms);
#endif
}

static int __hal_ipcsrv_send_raw_ipcmsg(int unit, struct hal_ipcmsg *src_ipcmsg, struct hal_ipcmsg_result *getvalue,
                                        struct hal_ipcmsg_callback *callback, int timeout_ms)
{
    int ret = 0;
    struct listnode *node = NULL;
    struct hal_ipcclient *client = NULL;
    HAL_ENTER_FUNC();

    hal_ipcmsg_hdrlen_set(src_ipcmsg);

    for (ALL_LIST_ELEMENTS_RO(_ipcsrv.client_list, node, client))
    {
        if (client->ipctype == HAL_IPCTYPE_EVENT)
            continue;
        if (unit >= 0 && unit != IF_UNIT_ALL && unit == client->unit)
        {
            hal_ipcmsg_hdr_unit_set(src_ipcmsg, client->unit);
            ret = hal_ipcsrv_send_message_client(client, src_ipcmsg, getvalue, callback, timeout_ms);
            return ret;
        }
        else
        {
            hal_ipcmsg_hdr_unit_set(src_ipcmsg, client->unit);
            ret = hal_ipcsrv_send_message_client(client, src_ipcmsg, getvalue, callback, timeout_ms);
            if (ret != OK)
                break;
        }
    }
    return ret;
}

int hal_ipcsrv_send_ipcmsg(int unit, struct hal_ipcmsg *src_ipcmsg)
{
    int ret = 0;
    HAL_ENTER_FUNC();
    if (_ipcsrv.mutex)
        os_mutex_lock(_ipcsrv.mutex, OS_WAIT_FOREVER);
    ret = __hal_ipcsrv_send_raw_ipcmsg(unit, src_ipcmsg, NULL, NULL, HAL_IPCSRV_ACK_TIMEOUT);
    if (_ipcsrv.mutex)
        os_mutex_unlock(_ipcsrv.mutex);
    return ret;
}

int hal_ipcsrv_copy_send_ipcmsg(int unit, zpl_uint32 command, struct hal_ipcmsg *src_ipcmsg)
{
    int ret = 0;
    HAL_ENTER_FUNC();
    if (_ipcsrv.mutex)
        os_mutex_lock(_ipcsrv.mutex, OS_WAIT_FOREVER);
    
    hal_ipcmsg_reset(&_ipcsrv.output_msg);
    hal_ipcmsg_create_header(&_ipcsrv.output_msg, command);
    if (src_ipcmsg && src_ipcmsg->buf)
        hal_ipcmsg_msg_copy(&_ipcsrv.output_msg, src_ipcmsg);

    ret = __hal_ipcsrv_send_raw_ipcmsg(unit, &_ipcsrv.output_msg, NULL, NULL, HAL_IPCSRV_ACK_TIMEOUT);

    if (_ipcsrv.mutex)
        os_mutex_unlock(_ipcsrv.mutex);
    return ret;
}

int hal_ipcsrv_send_message(int unit, zpl_uint32 command, void *msg, int len)
{
    int ret = 0;
    HAL_ENTER_FUNC();
    if (_ipcsrv.mutex)
        os_mutex_lock(_ipcsrv.mutex, OS_WAIT_FOREVER);
    hal_ipcmsg_reset(&_ipcsrv.output_msg);
    hal_ipcmsg_create_header(&_ipcsrv.output_msg, command);
    if (len)
        hal_ipcmsg_put(&_ipcsrv.output_msg, msg, len);

    ret = __hal_ipcsrv_send_raw_ipcmsg(unit, &_ipcsrv.output_msg, NULL, NULL, HAL_IPCSRV_ACK_TIMEOUT);

    if (_ipcsrv.mutex)
        os_mutex_unlock(_ipcsrv.mutex);
    return ret;
}

int hal_ipcsrv_send_and_get_message(int unit, zpl_uint32 command, void *msg, int len, struct hal_ipcmsg_result *getvalue)
{
    int ret = 0;
    HAL_ENTER_FUNC();
    if (_ipcsrv.mutex)
        os_mutex_lock(_ipcsrv.mutex, OS_WAIT_FOREVER);
    hal_ipcmsg_reset(&_ipcsrv.output_msg);
    hal_ipcmsg_create_header(&_ipcsrv.output_msg, command);
    if (len)
        hal_ipcmsg_put(&_ipcsrv.output_msg, msg, len);

    ret = __hal_ipcsrv_send_raw_ipcmsg(unit, &_ipcsrv.output_msg, getvalue, NULL, HAL_IPCSRV_ACK_TIMEOUT);

    if (_ipcsrv.mutex)
        os_mutex_unlock(_ipcsrv.mutex);
    return ret;
}

int hal_ipcsrv_getmsg_callback(int unit, zpl_uint32 command, void *msg, int len, struct hal_ipcmsg_result *getvalue,
                               struct hal_ipcmsg_callback *callback)
{
    int ret = 0;
    HAL_ENTER_FUNC();
    if (_ipcsrv.mutex)
        os_mutex_lock(_ipcsrv.mutex, OS_WAIT_FOREVER);
    hal_ipcmsg_reset(&_ipcsrv.output_msg);
    hal_ipcmsg_create_header(&_ipcsrv.output_msg, command);
    if (len)
        hal_ipcmsg_put(&_ipcsrv.output_msg, msg, len);

    ret = __hal_ipcsrv_send_raw_ipcmsg(unit, &_ipcsrv.output_msg, getvalue, callback, HAL_IPCSRV_ACK_TIMEOUT);

    if (_ipcsrv.mutex)
        os_mutex_unlock(_ipcsrv.mutex);
    return ret;
}
/*
char *hal_ipcsrv_send_message()
{
    if(_ipcsrv.input_msg.buf_length)
    {

    }
}
*/

int hal_ipcsrv_init(void *m, int port, const char *path)
{
    memset(&_ipcsrv, 0, sizeof(struct hal_ipcsrv));
    _ipcsrv.mutex = os_mutex_name_init("ipcsrvmutex");
    zlog_force_trap(MODULE_HAL, "hal_ipcsrv_init port=%d path=%s", port, path?path:"null");
    if (_ipcsrv.mutex == NULL)
    {
        return ERROR;
    }
#ifdef HAL_IPCSRV_SYNC_ACK
    if (ipstack_socketpair (OS_STACK, IPSTACK_AF_UNIX, IPSTACK_SOCK_STREAM, 0, _ipcsrv.waitfd)<0)
    {
        os_mutex_exit(_ipcsrv.mutex);
        _ipcsrv.mutex = NULL;
        return ERROR;
    }
    ipstack_set_nonblocking(_ipcsrv.waitfd[0]);
    ipstack_set_nonblocking(_ipcsrv.waitfd[1]);
#endif
    _ipcsrv.client_list = list_new();
    if (_ipcsrv.client_list)
    {
        _ipcsrv.client_list->del = hal_ipcclient_client_free;
    }
    else
    {
        os_mutex_exit(_ipcsrv.mutex);
        _ipcsrv.mutex = NULL;
#ifdef HAL_IPCSRV_SYNC_ACK
        if (!ipstack_invalid(_ipcsrv.waitfd[0]))
            ipstack_close(_ipcsrv.waitfd[0]);
        if (!ipstack_invalid(_ipcsrv.waitfd[1]))
            ipstack_close(_ipcsrv.waitfd[1]);
#endif
        return ERROR;
    }
    _ipcsrv.unixsock = hal_ipcsrv_un(path);
    if (ipstack_invalid(_ipcsrv.unixsock))
    {
        os_mutex_exit(_ipcsrv.mutex);
        _ipcsrv.mutex = NULL;
#ifdef HAL_IPCSRV_SYNC_ACK
        if (!ipstack_invalid(_ipcsrv.waitfd[0]))
            ipstack_close(_ipcsrv.waitfd[0]);
        if (!ipstack_invalid(_ipcsrv.waitfd[1]))
            ipstack_close(_ipcsrv.waitfd[1]);
#endif
        list_delete(_ipcsrv.client_list);
        return ERROR;
    }
    if (port > 0)
    {
        _ipcsrv.sock = hal_ipcsrv_socket(port);
        if (ipstack_invalid(_ipcsrv.sock))
        {
            os_mutex_exit(_ipcsrv.mutex);
            _ipcsrv.mutex = NULL;
#ifdef HAL_IPCSRV_SYNC_ACK
        if (!ipstack_invalid(_ipcsrv.waitfd[0]))
            ipstack_close(_ipcsrv.waitfd[0]);
        if (!ipstack_invalid(_ipcsrv.waitfd[1]))
            ipstack_close(_ipcsrv.waitfd[1]);
#endif
            ipstack_close(_ipcsrv.unixsock);
            list_delete(_ipcsrv.client_list);
            return ERROR;
        }
    }
    _ipcsrv.output_msg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    _ipcsrv.input_msg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    hal_ipcmsg_create(&_ipcsrv.output_msg);
    hal_ipcmsg_create(&_ipcsrv.input_msg);
#ifdef HAL_IPCSRV_SYNC_ACK
    _ipcsrv.ack_msg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    hal_ipcmsg_create(&_ipcsrv.ack_msg);
#endif
    _ipcsrv.master = m;
    if (!ipstack_invalid(_ipcsrv.unixsock))
        _ipcsrv.u_accept = thread_add_read(_ipcsrv.master, hal_ipcsrv_accept, &_ipcsrv, _ipcsrv.unixsock);
    if (!ipstack_invalid(_ipcsrv.sock))
        _ipcsrv.t_accept = thread_add_read(_ipcsrv.master, hal_ipcsrv_accept, &_ipcsrv, _ipcsrv.sock);

#ifdef ZPL_SHELL_MODULE
    hal_ipcsrv_cli();
#endif
    _ipcsrv.debug = 0xffff;
    _ipcsrv.b_init = zpl_true;
    return OK;
}

int hal_ipcsrv_exit(void)
{
    if (!ipstack_invalid(_ipcsrv.sock) > 0)
    {
        ipstack_close(_ipcsrv.sock);
    }
    if (!ipstack_invalid(_ipcsrv.unixsock) > 0)
    {
        ipstack_close(_ipcsrv.unixsock);
    }

    list_delete(_ipcsrv.client_list);
    hal_ipcmsg_destroy(&_ipcsrv.output_msg);
    hal_ipcmsg_destroy(&_ipcsrv.input_msg);
    if (_ipcsrv.mutex)
    {
        os_mutex_exit(_ipcsrv.mutex);
        _ipcsrv.mutex = NULL;
    }
#ifdef HAL_IPCSRV_SYNC_ACK
    hal_ipcmsg_destroy(&_ipcsrv.ack_msg);
    if (!ipstack_invalid(_ipcsrv.waitfd[0]))
        ipstack_close(_ipcsrv.waitfd[0]);
    if (!ipstack_invalid(_ipcsrv.waitfd[1]))
        ipstack_close(_ipcsrv.waitfd[1]);
#endif
    return OK;
}
#ifdef ZPL_SHELL_MODULE
int hal_ipcsrv_show(void *pvoid)
{
    struct listnode *node = NULL;
    struct hal_ipcclient *client = NULL;
    struct vty *vty = (struct vty *)pvoid;
    char *ipctype_string[] = {"NONE", "EVENT", "CMD", "NULL"};

    if (_ipcsrv.mutex)
        os_mutex_lock(_ipcsrv.mutex, OS_WAIT_FOREVER);

    if (listcount(_ipcsrv.client_list))
    {
        vty_out(vty, "%-6s %-4s %-4s %-4s %-4s %-4s %-6s %-8s %-16s%s", "------", "----", "----", "----", "----", "----", "------", "--------", "--------", VTY_NEWLINE);
        vty_out(vty, "%-6s %-4s %-4s %-4s %-4s %-4s %-6s %-8s %-16s%s", "MODULE", "UNIT", "SLOT", "NUM", "DYNA", "STAT", "IPCTYPE", "VER", "IP", VTY_NEWLINE);
        vty_out(vty, "%-6s %-4s %-4s %-4s %-4s %-4s %-6s %-8s %-16s%s", "------", "----", "----", "----", "----", "----", "------", "--------", "--------", VTY_NEWLINE);
    }

    for (ALL_LIST_ELEMENTS_RO(_ipcsrv.client_list, node, client))
    {
        if (client)
        {
            vty_out(vty, "%-6d %-4d %-4d %-4d %-4d %-4d %-6s %-8s %-16s%s",
                    client->module, client->unit, client->slot,
                    client->portnum, client->dynamic,
                    client->state, ipctype_string[client->ipctype], client->version,
                    inet_ntoa(client->clientaddr.sin_addr), VTY_NEWLINE);
        }
    }

    if (_ipcsrv.mutex)
        os_mutex_unlock(_ipcsrv.mutex);
    return OK;
}

DEFUN_HIDDEN(show_ipcsrv_information,
             show_ipcsrv_information_cmd,
             "show ipcsrv info",
             SHOW_STR
             "Ipc Server Configuration\n"
             "Information\n")
{

    hal_ipcsrv_show(vty);
    return CMD_SUCCESS;
}

static void hal_ipcsrv_cli(void)
{
    install_element(VIEW_NODE, CMD_VIEW_LEVEL, &show_ipcsrv_information_cmd);
}
#endif

#if 0
int zsend_router_id_update(struct zserv *client, struct prefix *p,
                           vrf_id_t vrf_id)
{
    struct stream *s;
    zpl_uint32 blen;

    /* Check this client need interface information. */
    if (!ip_vrf_bitmap_check(client->ridinfo, vrf_id))
        return 0;

    s = client->obuf;
    stream_reset(s);

    /* Message type. */
    nsm_zserv_create_header(s, HAL_IPCMSG_ROUTER_ID_UPDATE, vrf_id);

    /* Prefix information. */
    stream_putc(s, p->family);
    blen = prefix_blen(p);
    stream_put(s, &p->u.prefix, blen);
    stream_putc(s, p->prefixlen);

    /* Write packet size. */
    stream_putw_at(s, 0, stream_get_endp(s));

    return nsm_server_send_message(client);
}
#endif
