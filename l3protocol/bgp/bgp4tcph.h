
#include "bgp4_api.h"
#include "bgp4main.h"
#include "bgp4peer.h"
#include "bgp4path.h"

#ifdef NEW_BGP_WANTED
#ifndef BGP4TCPH_H
#define BGP4TCPH_H
#ifdef __cplusplus
 extern "C" {
#endif
	 
int bgp4_peer_sock_send(tBGP4_PEER *p_peer, u_char *p_msg, u_int len);
int bgp4_peer_sock_connect(tBGP4_PEER *p_peer);
int bgp4_set_sock_noblock(int s,int on);
int bgp4_set_sock_tcpnodelay(int s,int on);
int bgp4_set_sock_rxbuf(int s,int on);
int bgp4_set_sock_txbuf(int s,int on);
int bgp4_set_sock_reuseaddr(int s,int on);
int bgp4_set_sock_md5_support(int s,int on);
int bgp4_max_peer_socket(int exclude);
void bgp4_set_sock_md5_key(int s, int on, tBGP4_PEER * p_peer);
void bgp4_tcp_set_peer_ttl(int s,tBGP4_PEER *p_peer);
void bgp4_peer_tcp_addr_fill(tBGP4_PEER *p_peer);
void bgp4_server_sock_open(u_int uiVrf);
void bgp4_server_sock_close(u_int uiVrf);
void bgp4_tcp_socket_recv(fd_set * p_rfd, fd_set * p_wfd);

#ifdef __cplusplus
}
#endif 
#endif

#else
#ifndef BGP4TCPH_H
#define BGP4TCPH_H
#ifdef __cplusplus
      extern "C" {
 #endif
     
int bgp4_server_open();
void bgp4_server_close();
void bgp4_check_socket();
int bgp4_sock_send(tBGP4_PEER *p_peer, u_char *p_msg, u_int len);
int bgp4_sock_open(tBGP4_PEER *p_peer);
int bgp4_set_sock_noblock(int s,int on);
int bgp4_set_sock_tcpnodelay(int s,int on);
int bgp4_set_sock_rxbuf(int s,int on);
int bgp4_set_sock_txbuf(int s,int on);
int bgp4_set_sock_reuseaddr(int s,int on);
int bgp4_set_sock_md5(int s,int on);
int bgp4_set_sock_md5_key(int s, int on,u_char *p_key,u_char len,u_char pf,tBGP4_ADDR *p_addr);
void bgp4_tcp_set_peer_md5(int s,tBGP4_PEER *p_peer);
void bgp4_tcp_set_peer_ttl(int s,tBGP4_PEER *p_peer);
void bgp4_fill_tcp_address(tBGP4_PEER *p_peer);

#ifdef __cplusplus
     }
     #endif 
#endif

#endif
