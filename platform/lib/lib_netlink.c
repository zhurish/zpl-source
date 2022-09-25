#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "log.h"
#include "lib_netlink.h"


static int lib_netlink_sock_create(lib_netlink_t *nsock, int proto)
{
    int ret = 0;
    struct ipstack_sockaddr_nl bindaddr;
    nsock->sock = ipstack_socket(OS_STACK, IPSTACK_AF_NETLINK, IPSTACK_SOCK_RAW, proto);
    if (ipstack_invalid(nsock->sock))
        return ERROR;
    memset(&bindaddr, 0, sizeof(struct ipstack_sockaddr_nl));
    bindaddr.nl_family = IPSTACK_AF_NETLINK;
    bindaddr.nl_pid = getpid();
    bindaddr.nl_groups = 0;
    ret = ipstack_bind(nsock->sock, &bindaddr, sizeof(bindaddr));
    if (ret < 0)
    {
        zlog_err(MODULE_LIB, "Can not bind to socket:%s", ipstack_strerror(ipstack_errno));
        ipstack_close(nsock->sock);
        return ERROR;
    }
    ipstack_set_nonblocking(nsock->sock);
    return OK;
}

static int lib_netlink_sock_close(lib_netlink_t *nsock)
{
    /* Close ipstack_socket. */
    if (!ipstack_invalid(nsock->sock))
    {
        ipstack_close(nsock->sock);
    }
    return OK;
}

static int _lib_netlink_sock_recvmsg(lib_netlink_t *nsock, struct ipstack_msghdr *msg, int flags)
{
    int status = ERROR;
    while(1)
    {
        status = ipstack_recvmsg(nsock->sock, msg, flags);
        if (status < 0)
        {
            if (ipstack_errno == IPSTACK_ERRNO_EINTR)
                continue;
            if (ipstack_errno == IPSTACK_ERRNO_EWOULDBLOCK || ipstack_errno == IPSTACK_ERRNO_EAGAIN)
                continue;
            zlog_err(MODULE_PAL, "ipstack_recvmsg from [%d] overrun: %s", ipstack_fd(nsock->sock),
                     ipstack_strerror(ipstack_errno));
            return ERROR;
        }
        if (status == 0) {
            zlog_err(MODULE_PAL, "ipstack_recvmsg from [%d] overrun: %s", ipstack_fd(nsock->sock),
                     ipstack_strerror(ipstack_errno));
            return -ENODATA;
        }  
        else
            return status;
    }
    return status;
}

static int lib_netlink_sock_recvmsg(lib_netlink_t *nsock)
{
    int ret = 0;
    struct ipstack_sockaddr_nl snl;
    struct ipstack_msghdr msg;
    struct ipstack_iovec iov;
    while (1)
    {
        iov.iov_base = NULL;
        iov.iov_len = 0;
        msg.msg_name = (void *)&snl;
        msg.msg_namelen = sizeof snl;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        ret = _lib_netlink_sock_recvmsg(nsock, &msg, MSG_PEEK | MSG_TRUNC);
        if(ret < 0)
            return ret;
        if(ret > nsock->msgmax)  
        {
            nsock->msgbuf = XREALLOC(MTYPE_NETLINK_DATA, nsock->msgbuf, ret);
            nsock->msgmax = ret;
        }

        iov.iov_base = nsock->msgbuf + nsock->msgoffset;
        iov.iov_len = nsock->msgmax - nsock->msgoffset;   
        ret = _lib_netlink_sock_recvmsg(nsock, &msg, 0);
        if (msg.msg_namelen != sizeof snl)
        {
            zlog_err(MODULE_LIB, "ipstack_recvmsg [%d] sender address length error: length %d",
                     ipstack_fd(nsock->sock), msg.msg_namelen);
            return ERROR;
        }
        if(ret > 0)
            nsock->msglen = ret;
        else
            nsock->msglen = 0;           
        return ret;
    }
    return ERROR;
}


static int lib_netlink_sock_flush(lib_netlink_t *nsock)
{
    int ret = 0;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(ipstack_fd(nsock->sock), &readfds);
    while (1)
    {
        ret = os_select_wait(ipstack_fd(nsock->sock) + 1, &readfds, NULL, 5);
        if (ret == OS_TIMEOUT)
        {
            return OS_TIMEOUT;
        }
        if (ret == ERROR)
        {
            _OS_ERROR("os_select_wait to read(%d) %s\n", ipstack_fd(nsock->sock), strerror(ipstack_errno));
            return ERROR;
        }
        if (FD_ISSET(ipstack_fd(nsock->sock), &readfds))
        {
            ret = lib_netlink_sock_recvmsg(nsock);
        }
    }
    return 0;
}

static int lib_netlink_sock_msg_parse(lib_netlink_t *nsock, int len,
                                      int (*filter)(void *, int, char *, int, void *), void *p)
{
    int ret = 0;
    int error = 0;
    struct ipstack_nlmsghdr *h = NULL;

    h = (struct ipstack_nlmsghdr *)(nsock->msgbuf + nsock->msgoffset);
    //if(nsock->proto == 29)
    {
        //nlmsg_type=4 nlmsg_len=74 nlmsg_flags=0x0 nlmsg_seq=0 nlmsg_pid=0
        //zlog_debug(MODULE_LIB, "========= nlmsg_type=%d nlmsg_len=%d nlmsg_flags=0x%x nlmsg_seq=%d nlmsg_pid=%d", h->nlmsg_type, h->nlmsg_len, h->nlmsg_flags, h->nlmsg_seq, h->nlmsg_pid);
    }
    while (IPSTACK_NLMSG_OK(h, len))
    {
        /* Finish of reading. */
        if (h->nlmsg_type == IPSTACK_NLMSG_DONE)
        {
            zlog_err(MODULE_LIB, "ipstack_recvmsg [%d] IPSTACK_NLMSG_DONE", ipstack_fd(nsock->sock));
            return OK;
        }

        /* Error handling. */
        if (h->nlmsg_type == IPSTACK_NLMSG_ERROR)
        {
            struct ipstack_nlmsgerr *err = (struct ipstack_nlmsgerr *)IPSTACK_NLMSG_DATA(h);
            int errnum = err->error;
            int msg_type = err->msg.nlmsg_type;

            /* If the error field is zero, then this is an ACK */
            if (err->error == 0)
            {
                if ((nsock->debug))
                {
                    zlog_debug(MODULE_LIB,
                               "%s: ipstack_recvmsg [%d] ACK: type=%u, seq=%u, pid=%u",
                               __FUNCTION__, ipstack_fd(nsock->sock),
                               err->msg.nlmsg_type, err->msg.nlmsg_seq,
                               err->msg.nlmsg_pid);
                }

                /* return if not a multipart message, otherwise continue */
                if (!(h->nlmsg_flags & IPSTACK_NLM_F_MULTI))
                {
                    zlog_err(MODULE_LIB, "ipstack_recvmsg [%d] IPSTACK_NLM_F_MULTI", ipstack_fd(nsock->sock));
                    return ERROR;
                }
                return ERROR;
            }

            if (h->nlmsg_len < IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_nlmsgerr)))
            {
                zlog_err(MODULE_LIB, "ipstack_recvmsg [%d] error: message truncated", ipstack_fd(nsock->sock));
                return ERROR;
            }

            zlog_err(MODULE_LIB, "ipstack_recvmsg [%d] error: %s, type=%u, seq=%u, pid=%u",
                     ipstack_fd(nsock->sock), ipstack_strerror(-errnum),
                     msg_type,
                     err->msg.nlmsg_seq, err->msg.nlmsg_pid);
            return ERROR;
        }

        if (filter)
        {
            error = (*filter)(nsock, h->nlmsg_type, IPSTACK_NLMSG_DATA(h), IPSTACK_NLMSG_PAYLOAD(h, 0), p);
            if (error < 0)
            {
                zlog_err(MODULE_LIB, "ipstack_recvmsg [%d] filter function error", ipstack_fd(nsock->sock));
                ret = error;
            }
            ret = error;
        }
        else
            ret = 0;

        if ((h->nlmsg_flags & IPSTACK_NLM_F_MULTI))
            h = IPSTACK_NLMSG_NEXT(h, len);
        else
        {
            //zlog_err(MODULE_LIB, "ipstack_recvmsg [%d] END", ipstack_fd(nsock->sock));
            return ret;
        }
    }
    return ret;
}

static int lib_netlink_sock_parse_info(lib_netlink_t *nsock,
                                       int (*filter)(void *, int, char *, int, void *), void *p)
{
    int ret = ERROR;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(ipstack_fd(nsock->sock), &readfds);
    while (1)
    {
        ret = os_select_wait(ipstack_fd(nsock->sock) + 1, &readfds, NULL, 1000);
        if (ret == OS_TIMEOUT)
        {
            return OS_TIMEOUT;
        }
        if (ret == ERROR)
        {
            _OS_ERROR("os_select_wait to read(%d) %s\n", ipstack_fd(nsock->sock), strerror(ipstack_errno));
            return ERROR;
        }
        if (!FD_ISSET(ipstack_fd(nsock->sock), &readfds))
        {
            _OS_ERROR("no events on sockfd(%d) found\n", ipstack_fd(nsock->sock));
            return ERROR;
        }
        ret = lib_netlink_sock_recvmsg(nsock);
        if(ret)
            return lib_netlink_sock_msg_parse(nsock, ret, filter, p);
        else
            return ERROR;
    }
    return ret;
}


static int lib_netlink_sock_talk(lib_netlink_t *nsock, struct ipstack_nlmsghdr *n, int (*filter)(void *, int, char *, int, void *), void *p)
{
    zpl_int32 status;
    struct ipstack_sockaddr_nl snl;
    struct ipstack_iovec iov = {.iov_base = (void *)n, .iov_len = n->nlmsg_len};
    struct ipstack_msghdr msg = {
        .msg_name = (void *)&snl,
        .msg_namelen = sizeof snl,
        .msg_iov = &iov,
        .msg_iovlen = 1,
    };
    int save_errno;

    memset(&snl, 0, sizeof snl);
    snl.nl_family = IPSTACK_AF_NETLINK;

    if ((nsock->debug))
        zlog_debug(MODULE_PAL, "netlink_talk: cfg type (%u), seq=%u", n->nlmsg_type, n->nlmsg_seq);

    lib_netlink_sock_flush(nsock);

    /* Send message to netlink interface. */
    status = ipstack_sendmsg(nsock->sock, &msg, 0);
    save_errno = ipstack_errno;
    if (status < 0)
    {
        zlog_err(MODULE_PAL, "netlink_talk ipstack_sendmsg() error: %s",
                 ipstack_strerror(save_errno));
        return -1;
    }
    if(filter)
        return lib_netlink_sock_parse_info(nsock, filter, p);
    return  status;  
}

int lib_netlink_open(lib_netlink_t *nsock, int proto)
{
    nsock->proto = proto;
    return lib_netlink_sock_create(nsock, proto);
}

int lib_netlink_close(lib_netlink_t *nsock)
{
    return lib_netlink_sock_close(nsock);
}

int lib_netlink_talk(lib_netlink_t *nsock, struct ipstack_nlmsghdr *n, int (*filter)(void *, int, char *, int, void *), void *p)
{
    return lib_netlink_sock_talk(nsock, n, filter, p);
}

int lib_netlink_send(lib_netlink_t *nsock, struct ipstack_nlmsghdr *n)
{
    return lib_netlink_sock_talk(nsock, n, NULL, NULL);
}

int lib_netlink_recv(lib_netlink_t *nsock)
{
    return lib_netlink_sock_recvmsg(nsock);
}

int lib_netlink_msg_callback(lib_netlink_t *nsock, int (*filter)(void *, int, char *, int, void *), void *p)
{
    return lib_netlink_sock_msg_parse(nsock, nsock->msglen, filter, p);
}


lib_netlink_t * lib_netlink_create(int maxsize, int msgoffset)
{
    lib_netlink_t *nsk = XMALLOC(MTYPE_NETLINK, sizeof(lib_netlink_t));
    if(nsk)
    {
        nsk->msgbuf = XMALLOC(MTYPE_NETLINK_DATA, maxsize);
        if(nsk->msgbuf)
        {
            nsk->msgmax = maxsize;
            nsk->msgoffset = msgoffset;
            return nsk;
        }
        XFREE(MTYPE_NETLINK, nsk);
        nsk = NULL;
    }
    return nsk;
}

int lib_netlink_destroy(lib_netlink_t *nsock)
{
    if(nsock)
    {
        if(nsock->msgbuf)
        {
            XFREE(MTYPE_NETLINK_DATA, nsock->msgbuf);
            nsock->msgbuf = NULL;
        }
        XFREE(MTYPE_NETLINK, nsock);
        nsock = NULL;
    }
    return OK;
}