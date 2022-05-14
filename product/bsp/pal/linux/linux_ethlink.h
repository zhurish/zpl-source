#ifndef __LINUX_ETHLINK_H__
#define __LINUX_ETHLINK_H__

#ifdef __cplusplus
extern "C" {
#endif


struct netlink_sock
{
  vrf_id_t vrf_id;
  zpl_socket_t sock;
  zpl_uint32 seq;
  struct ipstack_sockaddr_nl snl;
  struct thread *t_netlink;
  int (*netlink_func)(int msgtype, char *msg, int len);
};

int netlink_create_socket(void);
int netlink_message_request(struct netlink_sock *nlsock, int msgtype, unsigned char *message, int len);


#ifdef __cplusplus
}
#endif


#endif /* __LINUX_ETHLINK_H__ */