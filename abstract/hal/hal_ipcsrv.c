#include "os_include.h"
#include "zpl_include.h"
#include "lib_include.h"
#include "nsm_include.h"
#include "hal_ipcsrv.h"
#include "hal_ipcmsg.h"
#include "hal_ipccmd.h"
#include "bmgt.h"

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
    if(IS_HAL_IPCMSG_DEBUG_EVENT(ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "client unit %d slot %d portnum %d close [%d]", client->unit, client->slot, client->portnum, client->sock._fd);
    }
    /* Close file descriptor. */
    hal_ipcclient_client_free(client);
    /* Free client structure. */
    listnode_delete(ipcsrv->client_list, client);
    XFREE(MTYPE_HALIPCCLIENT, client);
}

/* Make new client. */
static int
hal_ipcclient_client_create(struct hal_ipcsrv *ipcsrv, struct ipstack_sockaddr_in *clientaddr, zpl_socket_t sock, zpl_bool ev)
{
    struct hal_ipcclient *client;
    zpl_uint32 i = 0;
    client = XCALLOC(MTYPE_HALIPCCLIENT, sizeof(struct hal_ipcclient));
    if(client == NULL)
        return ERROR;
    /* Make client input/output buffer. */
    client->sock = sock;
    // client->obuf = XMALLOC(MTYPE_STREAM_DATA, HAL_IPCMSG_MAX_PACKET_SIZ);
    // client->ibuf = XMALLOC(MTYPE_STREAM_DATA, HAL_IPCMSG_MAX_PACKET_SIZ);
    memcpy(&client->clientaddr, clientaddr, sizeof(struct ipstack_sockaddr_in));

    client->ipcsrv = ipcsrv;

    client->event_client = ev;
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
        zlog_warn(MODULE_HAL, "Can't ipstack_accept hal_ipcclient ipstack_socket: %s", ipstack_strerror(ipstack_errno));
        return -1;
    }

    /* Make client ipstack_socket non-blocking.  */
    ipstack_set_nonblocking(client_sock);

    /* Create new hal_ipcclient client. */
    if(hal_ipcclient_client_create(ipcsrv, &client, client_sock, event) == OK)
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
    unixsock = ipstack_socket(OS_STACK, AF_UNIX, IPSTACK_SOCK_STREAM, 0);
    if (ipstack_invalid(unixsock))
    {
        zlog_warn(MODULE_HAL, "Can't create zserv unix ipstack_socket: %s",
                  ipstack_strerror(ipstack_errno));
        // zlog_warn (MODULE_HAL, "hal_ipcclient can't provide full functionality due to above error");
        return unixsock;
    }
    /* Make server ipstack_socket. */
    memset(&serv, 0, sizeof(struct ipstack_sockaddr_un));
    serv.sun_family = AF_UNIX;
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
        zlog_warn(MODULE_HAL, "Can't ipstack_bind to unix ipstack_socket %s: %s",
                  path, ipstack_strerror(ipstack_errno));
        // zlog_warn (MODULE_HAL, "hal_ipcclient can't provide full functionality due to above error");
        ipstack_close(unixsock);
        return unixsock;
    }

    ret = ipstack_listen(unixsock, 5);
    if (ret < 0)
    {
        zlog_warn(MODULE_HAL, "Can't ipstack_listen to unix ipstack_socket %s: %s",
                  path, ipstack_strerror(ipstack_errno));
        // zlog_warn (MODULE_HAL, "hal_ipcclient can't provide full functionality due to above error");
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
        zlog_warn(MODULE_HAL, "Can't create zserv stream ipstack_socket: %s",
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
        _OS_ERROR("Can't ipstack_bind to unix ipstack_socket %d: %s",
                  port, ipstack_strerror(ipstack_errno));
        zlog_warn(MODULE_HAL, "Can't ipstack_bind to stream ipstack_socket: %s",
                  ipstack_strerror(ipstack_errno));
        // zlog_warn (MODULE_HAL, "hal_ipcclient can't provice full functionality due to above error");
        ipstack_close(sock); /* Avoid sd leak. */
        return sock;
    }

    ret = ipstack_listen(sock, 1);
    if (ret < 0)
    {
        zlog_warn(MODULE_HAL, "Can't ipstack_listen to stream ipstack_socket: %s",
                  ipstack_strerror(ipstack_errno));
        // zlog_warn (MODULE_HAL, "hal_ipcclient can't provice full functionality due to above error");
        ipstack_close(sock); /* Avoid sd leak. */
        return sock;
    }
    // ipcsrv->t_read = thread_add_read(ipcsrv->master, hal_ipcsrv_accept, ipcsrv, ipcsrv->sock);
    return sock;
}


static void hal_ipcsrv_msg_hello(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg)
{
    struct hal_ipcmsg_hello *hello = (struct hal_ipcmsg_hello *)(ipcmsg->buf + ipcmsg->getp);
    client->event_client = hello->stype;
    client->module = hello->module;
    client->unit = hello->unit;
    client->slot = hello->slot;
    client->portnum = hello->portnum;
    client->board = unit_board_add(hello->unit, hello->slot);
    strcpy(client->version, hello->version);
    client->state = HAL_CLIENT_CONNECT;
    ipcmsg->getp += sizeof(struct hal_ipcmsg_hello);
    if(IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "RECV hello msg from [%d] unit %d slot %d portnum %d version %s", client->sock._fd, 
            client->unit, client->slot, client->portnum, client->version);
    }
}

static void hal_ipcsrv_msg_initialization(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg)
{
    zpl_int32 value = 0;
    hal_ipcmsg_getl(ipcmsg, &value);

    if(IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "RECV init msg from [%d] unit %d slot %d", client->sock._fd, 
            client->unit, client->slot);
    }
    return OK;
}

static void hal_ipcsrv_msg_porttbl(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg)
{
    zpl_uint32 i = 0;
    zpl_uint32 portnum = 0;
    struct hal_ipcmsg_porttbl porttbl;
    hal_ipcmsg_getc(ipcmsg, &portnum);
    for(i = 0; i < portnum; i++)
    {
        hal_ipcmsg_getc(ipcmsg, &porttbl.type);
        hal_ipcmsg_getc(ipcmsg, &porttbl.unit);
        hal_ipcmsg_getc(ipcmsg, &porttbl.slot);
        hal_ipcmsg_getc(ipcmsg, &porttbl.port);
        hal_ipcmsg_getl(ipcmsg, &porttbl.phyid);
        unit_board_port_add(client->board, (if_type_t)porttbl.type, porttbl.port, porttbl.phyid);
    }
    if(IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "RECV PortTable msg from [%d] unit %d slot %d portnum %d", client->sock._fd, 
            client->unit, client->slot, portnum);
    }
    return OK;
}

static void hal_ipcsrv_msg_register(struct hal_ipcclient *client, struct hal_ipcmsg *ipcmsg)
{
    zpl_int32 value = 0;
    hal_ipcmsg_getl(ipcmsg, &value);
    if(IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "RECV done msg from [%d] unit %d slot %d portnum %d", client->sock._fd, 
            client->unit, client->slot, client->portnum);
    }
    host_bspinit_done();
    return OK;
}

/* Handler of zebra service request. */
static int hal_ipcsrv_client_read(struct thread *thread)
{
    zpl_socket_t sock;
    struct hal_ipcclient *client = NULL;
    struct hal_ipcmsg_header *hdr = NULL;
    /* Get thread data.  Reset reading thread because I'm running. */
    sock = THREAD_FD(thread);
    client = THREAD_ARG(thread);
    zpl_assert(client);
    zpl_assert(client->ipcsrv);
    client->t_read = NULL;
    hdr = (struct hal_ipcmsg_header *)client->ipcsrv->input_msg.buf;
    /* Read length and command (if we don't have it already). */
    if (client->ipcsrv->input_msg.setp < HAL_IPCMSG_HEADER_SIZE)
    {
        ssize_t nbyte = 0;
        nbyte = ipstack_read(sock, client->ipcsrv->input_msg.buf, HAL_IPCMSG_HEADER_SIZE - client->ipcsrv->input_msg.setp);
        if ((nbyte == 0) || (nbyte == -1))
        {
            if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug))
                zlog_debug(MODULE_HAL, "connection closed ipstack_socket [%d]", sock);
            hal_ipcclient_client_close(client->ipcsrv, client);
            return -1;
        }
        if (nbyte != (ssize_t)(HAL_IPCMSG_HEADER_SIZE - client->ipcsrv->input_msg.setp))
        { 
            /* Try again later. */
            client->t_read = thread_add_read(client->ipcsrv->master, hal_ipcsrv_client_read, client, sock);
            return 0;
        }
        client->ipcsrv->input_msg.setp = HAL_IPCMSG_HEADER_SIZE;
    }

    /* Fetch header values */
    hdr->length = ntohs(hdr->length);
    hdr->command = ntohl(hdr->command);

    if (hdr->marker != HAL_IPCMSG_HEADER_MARKER || hdr->version != HAL_IPCMSG_VERSION)
    {
        zlog_err(MODULE_HAL, "%s: ipstack_socket %d version mismatch, marker %d, version %d",
                 __func__, sock, hdr->marker, hdr->version);
        hal_ipcclient_client_close(client->ipcsrv, client);
        return -1;
    }
    if (hdr->length < HAL_IPCMSG_HEADER_SIZE)
    {
        zlog_warn(MODULE_HAL, "%s: ipstack_socket %d message length %u is less than header size %d",
                  __func__, sock, hdr->length, HAL_IPCMSG_HEADER_SIZE);
        hal_ipcclient_client_close(client->ipcsrv, client);
        return -1;
    }
    if (hdr->length > client->ipcsrv->input_msg.length_max)
    {
        zlog_warn(MODULE_HAL, "%s: ipstack_socket %d message length %u exceeds buffer size %lu",
                  __func__, sock, hdr->length, (u_long)(client->ipcsrv->input_msg.length_max));
        hal_ipcclient_client_close(client->ipcsrv, client);
        return -1;
    }

    /* Read rest of data. */
    if (client->ipcsrv->input_msg.setp < hdr->length)
    {
        ssize_t nbyte;
        if (((nbyte = ipstack_read(sock, client->ipcsrv->input_msg.buf + client->ipcsrv->input_msg.setp, 
                                      hdr->length - client->ipcsrv->input_msg.setp)) == 0) ||
            (nbyte == -1))
        {
            if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug))
                zlog_debug(MODULE_HAL, "connection closed [%d] when reading zebra data", sock);
            hal_ipcclient_client_close(client->ipcsrv, client);
            return -1;
        }
        if (nbyte != (ssize_t)(hdr->length - client->ipcsrv->input_msg.setp))
        {
            /* Try again later. */
            client->t_read = thread_add_read(client->ipcsrv->master, hal_ipcsrv_client_read, client, sock);
            return 0;
        }
        client->ipcsrv->input_msg.setp += hdr->length;
    }

    hdr->length -= HAL_IPCMSG_HEADER_SIZE;

    /* Debug packet information. */
    //if (IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug))
    //    zlog_debug(MODULE_HAL, "CLIENT message comes from ipstack_socket [%d] %d bytes", sock, hdr->length);

    if (IS_HAL_IPCMSG_DEBUG_PACKET(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
        zlog_debug(MODULE_HAL, "CLIENT message comes from [%d] cmd [%s] %d bytes",
                   sock._fd, hal_module_cmd_name(hdr->command), hdr->length);

    client->ipcsrv->input_msg.getp = HAL_IPCMSG_HEADER_SIZE;
    
    if (IS_HAL_IPCMSG_DEBUG_PACKET(client->ipcsrv->debug) && 
        IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug) && 
        IS_HAL_IPCMSG_DEBUG_HEX(client->ipcsrv->debug))
        hal_ipcmsg_hexmsg(&client->ipcsrv->input_msg, HAL_IPCMSG_HEADER_SIZE + hdr->length, "CLIENT Recv");
    
    switch (IPCCMD_MODULE_GET(hdr->command))
    {
    case HAL_MODULE_MGT:
        if (IPCCMD_CMD_GET(hdr->command) == HAL_MODULE_CMD_HELLO)
        {
            hal_ipcsrv_msg_hello(client, &client->ipcsrv->input_msg);
        }
        else if (IPCCMD_CMD_GET(hdr->command) == HAL_MODULE_CMD_REGISTER)//初始化完毕
        {
            hal_ipcsrv_msg_register(client, &client->ipcsrv->input_msg);
        }
        else if (IPCCMD_CMD_GET(hdr->command) == HAL_MODULE_CMD_INIT)//初始化
        {
            hal_ipcsrv_msg_initialization(client, &client->ipcsrv->input_msg);
        }
        else if (IPCCMD_CMD_GET(hdr->command) == HAL_MODULE_CMD_PORTTBL)//端口信息
        {
            hal_ipcsrv_msg_porttbl(client, &client->ipcsrv->input_msg);
        }
        break;
    default:
        zlog_info(MODULE_HAL, "CLIENT received unknown command %d", IPCCMD_CMD_GET(hdr->command));
        break;
    }
    client->ipcsrv->input_msg.getp = 0;
    client->ipcsrv->input_msg.setp = 0;
    if (client->event_client)
        client->t_read = thread_add_read(client->ipcsrv->master, hal_ipcsrv_client_read, client, sock);
    else
    {
        if (client->t_read)
        {
            thread_cancel(client->t_read);
        }
        if (client->state != HAL_CLIENT_REGISTER && client->state != HAL_CLIENT_CLOSE)
        {
            client->t_read = thread_add_read(client->ipcsrv->master, hal_ipcsrv_client_read, client, sock);
        }
    }
    return 0;
}

static int hal_ipcsrv_send_message_client(struct hal_ipcclient *client, int timeout)
{
    int bytes = 0;
    zpl_size_t already = 0;
    if (client->state != HAL_CLIENT_CONNECT)
        return ERROR;
    if(IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_SEND(client->ipcsrv->debug))
    {
        zlog_debug(MODULE_HAL, "SEND msg to [%d] unit %d slot %d portnum %d %d byte", client->sock._fd, 
            client->unit, client->slot, client->portnum, client->ipcsrv->output_msg.setp);
    }
    //if (client->t_read)
    //    thread_cancel(client->t_read);
    while (1)
    {
        bytes = ipstack_write_timeout(client->sock, client->ipcsrv->output_msg.buf + already, client->ipcsrv->output_msg.setp - already, timeout);
        if (bytes == (client->ipcsrv->output_msg.setp - already))
            break;
        else if (bytes)
        {
            already += bytes;
        }
        else
            return ERROR;
    }
    client->ipcsrv->input_msg.getp = 0;
    client->ipcsrv->input_msg.setp = 0;
    already = ipstack_read_timeout(client->sock, (zpl_char *)client->ipcsrv->input_msg.buf, HAL_IPCMSG_HEADER_SIZE, timeout);
    if (already && already >= HAL_IPCMSG_HEADER_SIZE)
    {
        struct hal_ipcmsg_header *hdr = (struct hal_ipcmsg_header *)client->ipcsrv->input_msg.buf;
        client->ipcsrv->input_msg.getp += sizeof(struct hal_ipcmsg_header);
        while (1)
        {
            client->ipcsrv->input_msg.setp += already;

            bytes = ipstack_read_timeout(client->sock, (zpl_char *)client->ipcsrv->input_msg.buf + client->ipcsrv->input_msg.setp,
                                    ntohl(hdr->length) - client->ipcsrv->input_msg.setp, timeout);
            if (bytes == (ntohl(hdr->length) - already))
            {
                client->ipcsrv->input_msg.setp += bytes;
       
                if (IPCCMD_MODULE_GET(ntohl(hdr->command)) == HAL_MODULE_MGT)
                {
                    if (IPCCMD_CMD_GET(ntohl(hdr->command)) == HAL_MODULE_CMD_ACK)
                    {
                        char *msg_result_str = NULL;
                        struct hal_ipcmsg_result *msg_result = (struct hal_ipcmsg_result *)(client->ipcsrv->input_msg.buf + client->ipcsrv->input_msg.getp);
                        client->ipcsrv->input_msg.getp += sizeof(struct hal_ipcmsg_result);
                        msg_result_str = (client->ipcsrv->input_msg.buf + client->ipcsrv->input_msg.getp);
                        if(IS_HAL_IPCMSG_DEBUG_EVENT(client->ipcsrv->debug) && IS_HAL_IPCMSG_DEBUG_RECV(client->ipcsrv->debug))
                        {
                            zlog_debug(MODULE_HAL, "RECV msg from [%d] result %d %s", client->sock._fd, msg_result->result, 
                                (msg_result->result != OK)?msg_result_str:"OK");
                        }
                        if(msg_result->result == OK)
                            return OK;
                        return msg_result->result;
                    }
                }
            }
            else
            {
                if(bytes == OS_TIMEOUT)
                {
                    return ERROR;
                }
                else
                    return ERROR;
            }
        }
    }
    return ERROR;
}

int hal_ipcsrv_send_message(int unit, zpl_uint32 command, void *msg, int len, int timeout)
{
    int ret = 0;
    struct listnode *node = NULL;
    struct hal_ipcclient *client = NULL;
    HAL_ENTER_FUNC();
	if (_ipcsrv.mutex)
		os_mutex_lock(_ipcsrv.mutex, OS_WAIT_FOREVER);
    hal_ipcmsg_reset(&_ipcsrv.output_msg);
    hal_ipcmsg_create_header(&_ipcsrv.output_msg, command);
    if(len)
        hal_ipcmsg_msg_add(&_ipcsrv.output_msg, msg,  len);
    hal_ipcmsg_msglen_set(&_ipcsrv.output_msg);

    for (ALL_LIST_ELEMENTS_RO(_ipcsrv.client_list, node, client))
    {
        if (unit >= 0 && unit != IF_UNIT_ALL && unit == client->unit)
        {
            hal_ipcmsg_hdr_unit_set(&_ipcsrv.output_msg, unit);
            ret = hal_ipcsrv_send_message_client(client, timeout);
            if (_ipcsrv.mutex)
                os_mutex_unlock(_ipcsrv.mutex);
            return ret;
        }
        else
        {
            hal_ipcmsg_hdr_unit_set(&_ipcsrv.output_msg, unit);
            ret = hal_ipcsrv_send_message_client(client, timeout);
            if (ret != OK)
                break;
        }
    }
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

int hal_ipcsrv_init(void *m, int port, const char *path, int evport, const char *evpath)
{
    memset(&_ipcsrv, 0, sizeof(struct hal_ipcsrv));
    _ipcsrv.mutex = os_mutex_init();
    if(_ipcsrv.mutex == NULL)
    {
        return ERROR;
    }
    _ipcsrv.client_list = list_new();
    if(_ipcsrv.client_list)
    {
        _ipcsrv.client_list->del  = hal_ipcclient_client_free;
    }
    else
    {
        os_mutex_exit(_ipcsrv.mutex);
        _ipcsrv.mutex = NULL;
        return ERROR;
    }
    _ipcsrv.unixsock = hal_ipcsrv_un(path);
    if(ipstack_invalid(_ipcsrv.unixsock))
    {
        os_mutex_exit(_ipcsrv.mutex);
        _ipcsrv.mutex = NULL;
        list_delete (_ipcsrv.client_list);
        return ERROR;
    }
    if(port > 0)
    {
        _ipcsrv.sock = hal_ipcsrv_socket(port);
        if(ipstack_invalid(_ipcsrv.sock))
        {
            os_mutex_exit(_ipcsrv.mutex);
            _ipcsrv.mutex = NULL;
            ipstack_close(_ipcsrv.unixsock);
            list_delete (_ipcsrv.client_list);
            return ERROR;
        }
    }
    _ipcsrv.output_msg.length_max = _ipcsrv.input_msg.length_max = HAL_IPCMSG_MAX_PACKET_SIZ;
    hal_ipcmsg_create(&_ipcsrv.output_msg);
    hal_ipcmsg_create(&_ipcsrv.input_msg);
    _ipcsrv.master = m;
    if(!ipstack_invalid(_ipcsrv.unixsock))
        _ipcsrv.u_accept = thread_add_read(_ipcsrv.master, hal_ipcsrv_accept, &_ipcsrv, _ipcsrv.unixsock);
    if(!ipstack_invalid(_ipcsrv.sock))
        _ipcsrv.t_accept = thread_add_read(_ipcsrv.master, hal_ipcsrv_accept, &_ipcsrv, _ipcsrv.sock);

#ifdef ZPL_SHELL_MODULE
    hal_ipcsrv_cli();
#endif
    _ipcsrv.debug = 0xffff;
    return OK;
}

int hal_ipcsrv_exit(void)
{
    if(!ipstack_invalid(_ipcsrv.sock) > 0)
    {
        ipstack_close(_ipcsrv.sock);
    }
    if(!ipstack_invalid(_ipcsrv.unixsock) > 0)
    {
        ipstack_close(_ipcsrv.unixsock);
    }

    list_delete (_ipcsrv.client_list);
    hal_ipcmsg_destroy(&_ipcsrv.output_msg);
    hal_ipcmsg_destroy(&_ipcsrv.input_msg);
    if(_ipcsrv.mutex)
    {
        os_mutex_exit(_ipcsrv.mutex);
        _ipcsrv.mutex = NULL;
    }
    return OK;
}
#ifdef ZPL_SHELL_MODULE
int hal_ipcsrv_show(void *pvoid)
{
    struct listnode *node = NULL;
    struct hal_ipcclient *client = NULL;
	struct vty *vty = (struct vty *)pvoid;

	if (_ipcsrv.mutex)
		os_mutex_lock(_ipcsrv.mutex, OS_WAIT_FOREVER);

	if (listcount(_ipcsrv.client_list))
	{
		vty_out(vty, "%-6s %-4s %-4s %-4s %-4s %-4s %-4s %-8s %-16s%s", "------", "----", "----", "----", "----", "----", "----","--------","--------",VTY_NEWLINE);
		vty_out(vty, "%-6s %-4s %-4s %-4s %-4s %-4s %-4s %-8s %-16s%s", "MODULE", "UNIT", "SLOT", "NUM", "DYNA", "STAT", "EVEN", "VER", "IP", VTY_NEWLINE);
		vty_out(vty, "%-6s %-4s %-4s %-4s %-4s %-4s %-4s %-8s %-16s%s", "------", "----", "----", "----", "----", "----", "----","--------","--------",VTY_NEWLINE);
	}

    for (ALL_LIST_ELEMENTS_RO(_ipcsrv.client_list, node, client))
    {
		if (client)
		{
			vty_out(vty, "%-6d %-4d %-4d %-4d %-4d %-4d %-4d %-8s %-16s%s", 
                client->module, client->unit, client->slot, 
				client->portnum, client->dynamic, 
                client->state, client->event_client, client->version,
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
    if (!vrf_bitmap_check(client->ridinfo, vrf_id))
        return 0;

    s = client->obuf;
    stream_reset(s);

    /* Message type. */
    zserv_create_header(s, HAL_IPCMSG_ROUTER_ID_UPDATE, vrf_id);

    /* Prefix information. */
    stream_putc(s, p->family);
    blen = prefix_blen(p);
    stream_put(s, &p->u.prefix, blen);
    stream_putc(s, p->prefixlen);

    /* Write packet size. */
    stream_putw_at(s, 0, stream_get_endp(s));

    return zebra_server_send_message(client);
}
#endif
