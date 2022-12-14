
#ifndef __LIB_NETLINK_H__
#define __LIB_NETLINK_H__


typedef struct lib_netlink_s 
{
    zpl_socket_t    sock;
    int             proto;
    int             seq;
    int             debug;
    zpl_uint32      msgmax;
    zpl_uint8       *msgbuf;
    zpl_uint32      msglen;
    zpl_uint32      msgoffset;
}lib_netlink_t;

extern lib_netlink_t * lib_netlink_create(int maxsize, int msgoffset);
extern int lib_netlink_destroy(lib_netlink_t *nsock);
extern int lib_netlink_open(lib_netlink_t * nsock, int proto);
extern int lib_netlink_close(lib_netlink_t * nsock);
extern int lib_netlink_talk(lib_netlink_t * nsock, struct ipstack_nlmsghdr *n, int (*filter)(void *, int, char *, int, void *), void *p);
extern int lib_netlink_send(lib_netlink_t *nsock, struct ipstack_nlmsghdr *n);
extern int lib_netlink_recv(lib_netlink_t *nsock);
extern int lib_netlink_msg_callback(lib_netlink_t *nsock, int (*filter)(void *, int, char *, int, void *), void *p);

#endif /* __LIB_NETLINK_H__ */
