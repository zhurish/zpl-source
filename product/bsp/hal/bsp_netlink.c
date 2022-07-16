#include "zplos_include.h"
#include "module.h"
#include "zmemory.h"
#include "log.h"
#include "nsm_include.h"
#include "hal_include.h"
#include "bsp_netlink.h"

static int bsp_netlink_sock_create(bsp_netlink_t *nsock, int proto)
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
        zlog_err(MODULE_BSP, "Can not bind to socket:%s", ipstack_strerror(ipstack_errno));
        ipstack_close(nsock->sock);
        return ERROR;
    }
    ipstack_set_nonblocking(nsock->sock);
    return OK;
}

static int bsp_netlink_sock_close(bsp_netlink_t *nsock)
{
    /* Close ipstack_socket. */
    if (!ipstack_invalid(nsock->sock))
    {
        ipstack_close(nsock->sock);
    }
    return OK;
}

static int _bsp_netlink_sock_recvmsg(bsp_netlink_t *nsock, struct ipstack_msghdr *msg, int flags)
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
            zlog_err(MODULE_PAL, "ipstack_recvmsg from [%d] overrun: %s", nsock->sock._fd,
                     ipstack_strerror(ipstack_errno));
            return ERROR;
        }
        if (status == 0) {
            zlog_err(MODULE_PAL, "ipstack_recvmsg from [%d] overrun: %s", nsock->sock._fd,
                     ipstack_strerror(ipstack_errno));
            return -ENODATA;
        }  
        else
            return status;
    }
    return status;
}

static int bsp_netlink_sock_recvmsg(bsp_netlink_t *nsock)
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
        ret = _bsp_netlink_sock_recvmsg(nsock, &msg, MSG_PEEK | MSG_TRUNC);
        if(ret < 0)
            return ret;
        if(ret > nsock->msgmax)  
        {
            nsock->msgbuf = XREALLOC(MTYPE_BSP_NETLINK_DATA, nsock->msgbuf, ret);
            nsock->msgmax = ret;
        }
        iov.iov_base = nsock->msgbuf + nsock->msgoffset;
        iov.iov_len = nsock->msgmax;   
        ret = _bsp_netlink_sock_recvmsg(nsock, &msg, 0);
        if (msg.msg_namelen != sizeof snl)
        {
            zlog_err(MODULE_BSP, "ipstack_recvmsg [%d] sender address length error: length %d",
                     nsock->sock._fd, msg.msg_namelen);
            return ERROR;
        }
        return ret;
    }
    return ERROR;
}


static int bsp_netlink_sock_flush(bsp_netlink_t *nsock)
{
    int ret = 0;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(nsock->sock._fd, &readfds);
    while (1)
    {
        ret = os_select_wait(nsock->sock._fd + 1, &readfds, NULL, 5);
        if (ret == OS_TIMEOUT)
        {
            return OS_TIMEOUT;
        }
        if (ret == ERROR)
        {
            _OS_ERROR("os_select_wait to read(%d) %s\n", fd, strerror(ipstack_errno));
            return ERROR;
        }
        if (FD_ISSET(nsock->sock._fd, &readfds))
        {
            ret = bsp_netlink_sock_recvmsg(nsock);
        }
    }
    return 0;
}

static int bsp_netlink_sock_msg_parse(bsp_netlink_t *nsock,
                                      int (*filter)(void *, int, char *, int, void *), void *p)
{
    int ret = 0, status = 0;
    int error = 0;
    struct ipstack_nlmsghdr *h = NULL;

    h = (struct ipstack_nlmsghdr *)(nsock->msgbuf + nsock->msgoffset);
    while (IPSTACK_NLMSG_OK(h, status))
    {
        /* Finish of reading. */
        if (h->nlmsg_type == IPSTACK_NLMSG_DONE)
        {
            zlog_err(MODULE_BSP, "ipstack_recvmsg [%d] IPSTACK_NLMSG_DONE", nsock->sock._fd);
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
                if (IS_HAL_IPCMSG_DEBUG_EVENT(nsock->debug))
                {
                    zlog_debug(MODULE_BSP,
                               "%s: ipstack_recvmsg [%d] ACK: type=%u, seq=%u, pid=%u",
                               __FUNCTION__, nsock->sock._fd,
                               err->msg.nlmsg_type, err->msg.nlmsg_seq,
                               err->msg.nlmsg_pid);
                }

                /* return if not a multipart message, otherwise continue */
                if (!(h->nlmsg_flags & IPSTACK_NLM_F_MULTI))
                {
                    zlog_err(MODULE_BSP, "ipstack_recvmsg [%d] IPSTACK_NLM_F_MULTI", nsock->sock._fd);
                    return ERROR;
                }
                return ERROR;
            }

            if (h->nlmsg_len < IPSTACK_NLMSG_LENGTH(sizeof(struct ipstack_nlmsgerr)))
            {
                zlog_err(MODULE_BSP, "ipstack_recvmsg [%d] error: message truncated", nsock->sock._fd);
                return ERROR;
            }

            zlog_err(MODULE_BSP, "ipstack_recvmsg [%d] error: %s, type=%u, seq=%u, pid=%u",
                     nsock->sock._fd, ipstack_strerror(-errnum),
                     msg_type,
                     err->msg.nlmsg_seq, err->msg.nlmsg_pid);
            return ERROR;
        }

        if (filter)
        {
            error = (*filter)(nsock, h->nlmsg_type, IPSTACK_NLMSG_DATA(h), IPSTACK_NLMSG_PAYLOAD(h, 0), p);
            if (error < 0)
            {
                zlog_err(MODULE_BSP, "ipstack_recvmsg [%d] filter function error", nsock->sock._fd);
                ret = error;
            }
            ret = error;
        }
        else
            ret = 0;
        if ((h->nlmsg_flags & IPSTACK_NLM_F_MULTI))
            h = IPSTACK_NLMSG_NEXT(h, status);
        else
        {
            zlog_err(MODULE_BSP, "ipstack_recvmsg [%d] END", nsock->sock._fd);
            return ret;
        }
    }
    return ret;
}

static int bsp_netlink_sock_parse_info(bsp_netlink_t *nsock,
                                       int (*filter)(void *, int, char *, int, void *), void *p)
{
    int ret = 0;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(nsock->sock._fd, &readfds);
    while (1)
    {
        ret = os_select_wait(nsock->sock._fd + 1, &readfds, NULL, 1000);
        if (ret == OS_TIMEOUT)
        {
            return OS_TIMEOUT;
        }
        if (ret == ERROR)
        {
            _OS_ERROR("os_select_wait to read(%d) %s\n", fd, strerror(ipstack_errno));
            return ERROR;
        }
        if (!FD_ISSET(nsock->sock._fd, &readfds))
        {
            _OS_ERROR("no events on sockfd(%d) found\n", fd);
            return ERROR;
        }
        ret = bsp_netlink_sock_recvmsg(nsock);
        if(ret)
            return bsp_netlink_sock_msg_parse(nsock, filter, p);
        else
            return ERROR;
    }
    return ret;
}


static int bsp_netlink_sock_talk(bsp_netlink_t *nsock, struct ipstack_nlmsghdr *n, int (*filter)(void *, int, char *, int, void *), void *p)
{
    zpl_uint32 status;
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

    if (IS_HAL_IPCMSG_DEBUG_EVENT(nsock->debug))
        zlog_debug(MODULE_PAL, "netlink_talk: cfg type (%u), seq=%u", n->nlmsg_type, n->nlmsg_seq);

    bsp_netlink_sock_flush(nsock);

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
        return bsp_netlink_sock_parse_info(nsock, filter, p);
    return  status;  
}

int bsp_netlink_open(bsp_netlink_t *nsock, int proto)
{
    return bsp_netlink_sock_create(nsock, proto);
}

int bsp_netlink_close(bsp_netlink_t *nsock)
{
    return bsp_netlink_sock_close(nsock);
}

int bsp_netlink_talk(bsp_netlink_t *nsock, struct ipstack_nlmsghdr *n, int (*filter)(void *, int, char *, int, void *), void *p)
{
    return bsp_netlink_sock_talk(nsock, n, filter, p);
}

int bsp_netlink_send(bsp_netlink_t *nsock, struct ipstack_nlmsghdr *n)
{
    return bsp_netlink_sock_talk(nsock, n, NULL, NULL);
}

int bsp_netlink_recv(bsp_netlink_t *nsock)
{
    return bsp_netlink_sock_recvmsg(nsock);
}

int bsp_netlink_msg_callback(bsp_netlink_t *nsock, int (*filter)(void *, int, char *, int, void *), void *p)
{
    return bsp_netlink_sock_msg_parse(nsock, filter, p);
}


bsp_netlink_t * bsp_netlink_create(int maxsize, int msgoffset)
{
    bsp_netlink_t *nsk = XMALLOC(MTYPE_BSP_NETLINK, sizeof(bsp_netlink_t));
    if(nsk)
    {
        nsk->msgbuf = XMALLOC(MTYPE_BSP_NETLINK_DATA, maxsize);
        if(nsk->msgbuf)
        {
            nsk->msgmax = maxsize;
            nsk->msgoffset = msgoffset;
            return nsk;
        }
        XFREE(MTYPE_BSP_NETLINK, nsk);
        nsk = NULL;
    }
    return nsk;
}

int bsp_netlink_destroy(bsp_netlink_t *nsock)
{
    if(nsock)
    {
        if(nsock->msgbuf)
        {
            XFREE(MTYPE_BSP_NETLINK_DATA, nsock->msgbuf);
            nsock->msgbuf = NULL;
        }
        XFREE(MTYPE_BSP_NETLINK, nsock);
        nsock = NULL;
    }
    return OK;
}