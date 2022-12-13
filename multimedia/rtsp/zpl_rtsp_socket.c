
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "zpl_rtsp_socket.h"




#ifdef _WIN32
static int _win32_socket_init = 0;
static int _rtsp_srv_init_win32(void)
{
    /* Initialise Windows Socket API */
    WSADATA wsaData;
    if(_win32_socket_init)
        return 0;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup() returned error code %d\n",
                (unsigned int)GetLastError());
        errno = EIO;
        return -1;
    }
    _win32_socket_init = 1;
    return 0;
}
#endif



static int _rtsp_set_ipv4_options(zpl_socket_t s)
{
    int rc;
    int option;

    /* Set the TCP no delay flag */
    /* SOL_TCP = IPPROTO_TCP */
    option = 1;
    rc = ipstack_setsockopt(s, IPSTACK_IPPROTO_TCP, IPSTACK_TCP_NODELAY,
                    (const void *)&option, sizeof(int));
    if (rc == -1) {
        fprintf(stdout,"%s [%d]  setsockopt(%s)\r\n", __func__, __LINE__, strerror(errno));
        fflush(stdout);
        return -1;
    }

    /* If the OS does not offer SOCK_NONBLOCK, fall back to setting FIONBIO to
     * make sockets non-blocking */
    /* Do not care about the return value, this is optional */
#if !defined(SOCK_NONBLOCK) && defined(FIONBIO)
#ifdef _WIN32
    {
        /* Setting FIONBIO expects an unsigned long according to MSDN */
        u_long loption = 1;
        ioctlsocket(s, IPSTACK_FIONBIO, &loption);
    }
#else
    option = 1;
    ioctl(s, IPSTACK_FIONBIO, &option);
#endif
#endif

#ifndef _WIN32
    /**
     * Cygwin defines IPTOS_LOWDELAY but can't handle that flag so it's
     * necessary to workaround that problem.
     **/
    /* Set the IP low delay option */
    option = IPSTACK_IPTOS_LOWDELAY;
    rc = ipstack_setsockopt(s, IPSTACK_IPPROTO_IP, IPSTACK_IP_TOS,
                    (const void *)&option, sizeof(int));
    if (rc == -1) {
        fprintf(stdout,"%s [%d]  setsockopt(%s)\r\n", __func__, __LINE__, strerror(errno));
        fflush(stdout);
        return -1;
    }
#endif

    return 0;
}

static int _rtsp_set_ipv4_keepalive(zpl_socket_t sock)
{
#if defined(__WIN32__) || defined(_WIN32)
  // How do we do this in Windows?  For now, just make this a no-op in Windows:
    int const keepalive_enabled = 1;
    if (ipstack_setsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_KEEPALIVE, (void*)&keepalive_enabled, sizeof keepalive_enabled) < 0) {
        fprintf(stdout,"%s [%d]  setsockopt(%s)\r\n", __func__, __LINE__, strerror(errno));
        fflush(stdout);
        return -1;
    }
#else
  int const keepalive_enabled = 1;
  if (ipstack_setsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_KEEPALIVE, (void*)&keepalive_enabled, sizeof keepalive_enabled) < 0) {
      fprintf(stdout,"%s [%d]  setsockopt(%s)\r\n", __func__, __LINE__, strerror(errno));
      fflush(stdout);
      return -1;
  }

#ifdef TCP_KEEPIDLE
  int const keepalive_time = 180;
  if (ipstack_setsockopt(sock, IPSTACK_IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keepalive_time, sizeof keepalive_time) < 0) {
      fprintf(stdout,"%s [%d]  setsockopt(%s)\r\n", __func__, __LINE__, strerror(errno));
      fflush(stdout);
      return -1;
  }
#endif

#ifdef TCP_KEEPCNT
  int const keepalive_count = 5;
  if (ipstack_setsockopt(sock, IPSTACK_IPPROTO_TCP, TCP_KEEPCNT, (void*)&keepalive_count, sizeof keepalive_count) < 0) {
      fprintf(stdout,"%s [%d]  setsockopt(%s)\r\n", __func__, __LINE__, strerror(errno));
      fflush(stdout);
      return -1;
  }
#endif

#ifdef TCP_KEEPINTVL
  int const keepalive_interval = 20;
  if (ipstack_setsockopt(sock, IPSTACK_IPPROTO_TCP, TCP_KEEPINTVL, (void*)&keepalive_interval, sizeof keepalive_interval) < 0) {
      fprintf(stdout,"%s [%d]  setsockopt(%s)\r\n", __func__, __LINE__, strerror(errno));
      fflush(stdout);
      return -1;
  }
#endif
#endif

  return 0;
}


static void _rtsp_set_ipv4_ignore_sig_pipe(zpl_socket_t socketNum) {
  #ifdef USE_SIGNALS
  #ifdef SO_NOSIGPIPE
  int set_option = 1;
  ipstack_setsockopt(socketNum, IPSTACK_SOL_SOCKET, SO_NOSIGPIPE, &set_option, sizeof set_option);
  #else
  signal(SIGPIPE, SIG_IGN);
  #endif
  #endif
}

static unsigned _rtsp_get_buffer_size(int bufOptName,zpl_socket_t sock) {
  unsigned curSize;
  socklen_t sizeSize = sizeof curSize;
  if (ipstack_getsockopt(sock, IPSTACK_SOL_SOCKET, bufOptName,
         (char*)&curSize, &sizeSize) < 0) {
      fprintf(stdout,"%s [%d]  getsockopt(%s)\r\n", __func__, __LINE__, strerror(errno));
      fflush(stdout);
    return 0;
  }
  return curSize;
}


static unsigned _rtsp_set_buffer_size(int bufOptName,
                zpl_socket_t socket, unsigned requestedSize) {
  socklen_t sizeSize = sizeof requestedSize;
  ipstack_setsockopt(socket, IPSTACK_SOL_SOCKET, bufOptName, (char*)&requestedSize, sizeSize);

  // Get and return the actual, resulting buffer size:
  return _rtsp_get_buffer_size(bufOptName, socket);
}

static unsigned _rtsp_set_send_buffer_size(zpl_socket_t sock, unsigned requestedSize) {
    return _rtsp_set_buffer_size(IPSTACK_SO_SNDBUF, sock, requestedSize);
}
static unsigned _rtsp_set_recv_buffer_size(zpl_socket_t sock, unsigned requestedSize) {
    return _rtsp_set_buffer_size(IPSTACK_SO_RCVBUF, sock, requestedSize);
}


/* Closes the network connection and socket in TCP mode */
static int rtsp_socket_close(zpl_socket_t s)
{
    if (s != -1) {
        ipstack_shutdown(s, IPSTACK_SHUT_RDWR);
#ifdef _WIN32
        ipstack_closesocket(s);
#else
        ipstack_close(s);
#endif
    }
}

static int rtsp_socket_listen(zpl_socket_t s, int nb_connection)
{
    if (ipstack_listen(s, nb_connection) == -1) {
        fprintf(stdout,"%s [%d]  listen(%s)\r\n", __func__, __LINE__, strerror(errno));
        fflush(stdout);
        return -1;
    }
    return 0;
}

/* Listens for any request from one or many modbus masters in TCP */
static int rtsp_socket_bind(zpl_socket_t s, char *address, uint16_t port)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = IPSTACK_AF_INET;
    /* If the modbus port is < to 1024, we need the setuid root. */
    addr.sin_port = htons(port);
    if (address == NULL) {
        fprintf(stdout, " socket bind 0.0.0.0:%d\r\n", port);
        fflush(stdout);
        /* Listen any addresses */
        addr.sin_addr.s_addr = htonl(IPSTACK_INADDR_ANY);
    } else {
        fprintf(stdout, " socket bind %s:%d\r\n", address, port);
        fflush(stdout);
        /* Listen only specified IP address */
        addr.sin_addr.s_addr = inet_addr(address);
    }
    if (ipstack_bind(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        fprintf(stdout, " socket bind %s:%d(%s)\r\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), strerror(errno));
        fflush(stdout);
        return -1;
    }
    return 0;
}


zpl_socket_t rtsp_socket_accept(zpl_socket_t s, char *address, uint16_t *port)
{
    struct sockaddr_in addr;
    socklen_t addrlen;
    zpl_socket_t sc;

    addrlen = sizeof(addr);
#ifdef HAVE_ACCEPT4
    /* Inherit socket flags and use accept4 call */
    sc = accept4(s, (struct sockaddr *)&addr, &addrlen, SOCK_CLOEXEC);
#else
    sc = ipstack_accept(s, (struct sockaddr *)&addr, &addrlen);
#endif

    if (ipstack_invalid(sc)) {
        fprintf(stdout,"%s [%d]  accept(%s)\r\n", __func__, __LINE__, strerror(errno));
        fflush(stdout);
        return sc;
    }

    _rtsp_set_ipv4_options(sc);
    _rtsp_set_ipv4_keepalive(sc);
    _rtsp_set_send_buffer_size(sc, 4096);
    _rtsp_set_recv_buffer_size(sc, 4096);

    if(address)
        strcpy(address, inet_ntoa(addr.sin_addr));
    if(port)
        *port = ntohs(addr.sin_port);
    fprintf(stdout, " socket accept %s:%d\r\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    fflush(stdout);
    return sc;
}



static int rtsp_socket_sendto(zpl_socket_t s,  uint8_t *req, uint32_t req_length)
{
    /* MSG_NOSIGNAL
       Requests not to send SIGPIPE on errors on stream oriented
       sockets when the other end breaks the connection.  The EPIPE
       error is still returned. */
    return ipstack_send(s, (const char *)req, req_length, IPSTACK_MSG_NOSIGNAL);
}

static int rtsp_socket_recvfrom(zpl_socket_t s, uint8_t *rsp, uint32_t rsp_length) {
    return ipstack_recv(s, (char *)rsp, rsp_length, 0);
}


/* Establishes a modbus TCP connection with a Modbus server. */
static int rtsp_socket_connect(zpl_socket_t s,  char *address, uint16_t port)
{
    int rc = 0;
    /* Specialized version of sockaddr for Internet socket address (same size) */
    struct sockaddr_in addr;
    addr.sin_family = IPSTACK_AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address);
    rc = ipstack_connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if (rc == -1) {
        fprintf(stdout,"%s [%d]  _connect(%s)\r\n", __func__, __LINE__, strerror(errno));
        fflush(stdout);
        return -1;
    }
    return 0;
}

static int rtsp_socket_select(zpl_ipstack st, int s, ipstack_fd_set *rset, ipstack_fd_set *wset, struct timeval *tv)
{
    int s_rc = -1;
    while(1)
    {
        s_rc = ipstack_select(st, s, rset, wset, NULL, tv);
        if(s_rc <= -1)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EPIPE || errno == ESPIPE) {
                /* Necessary after an error */
                fprintf(stdout,"%s [%d]  select(%s)\r\n", __func__, __LINE__, strerror(errno));
                fflush(stdout);
                continue;
            } else {
                fprintf(stdout,"%s [%d]  select(%s)\r\n", __func__, __LINE__, strerror(errno));
                fflush(stdout);
                return -1;
            }
        }
        else if (s_rc == 0) {
            //errno = ETIMEDOUT;
            //fprintf(stdout,"%s [%d]  select(%s)\r\n", __func__, __LINE__, strerror(errno));
            //fflush(stdout);
            return 0;//-ETIMEDOUT;
        }
        else
            return s_rc;
    }
    return s_rc;
}

static zpl_socket_t rtsp_socket_create(zpl_ipstack s, int ai_family, int pro, int flag)
{
#ifdef _WIN32
    _rtsp_srv_init_win32();
#endif
    zpl_socket_t sock = ipstack_socket(s, ai_family, pro, flag);
    if (ipstack_invalid(sock)) {
        fprintf(stdout,"%s [%d]  socket(%s)\r\n", __func__, __LINE__, strerror(errno));
        fflush(stdout);
        return sock;
    } else {
        int enable = 1;
        int rc = ipstack_setsockopt(sock, IPSTACK_SOL_SOCKET, IPSTACK_SO_REUSEADDR,
                        (void *)&enable, sizeof (enable));
        if (rc != 0) {
            fprintf(stdout,"%s [%d]  setsockopt(%s)\r\n", __func__, __LINE__, strerror(errno));
            fflush(stdout);
#ifdef _WIN32
            closesocket(sock);
#else
            ipstack_close(sock);
#endif
            return sock;
        }
        if(pro == IPSTACK_SOCK_STREAM)
        {
            _rtsp_set_ipv4_options(sock);
            _rtsp_set_ipv4_keepalive(sock);
        }
        _rtsp_set_send_buffer_size(sock, 4096);
        _rtsp_set_recv_buffer_size(sock, 4096);
    }
    return sock;
}
#ifndef _WIN32
static int rtsp_socket_sendmsg(zpl_socket_t s,  struct msghdr *msg, uint32_t flags)
{
    return ipstack_sendmsg(s, msg, flags);
}

static int rtsp_socket_recvmsg(zpl_socket_t s,  struct msghdr *msg, uint32_t flags) {
    return ipstack_recvmsg(s, msg, flags);
}
#endif

rtsp_socket_t  rtsp_socket =
{
    rtsp_socket_create,
    rtsp_socket_bind,
    rtsp_socket_accept,
    rtsp_socket_listen,
    rtsp_socket_connect,
    rtsp_socket_close,
    rtsp_socket_select,
    rtsp_socket_sendto,
    rtsp_socket_recvfrom,
    #ifdef _WIN32
    NULL,
    NULL,
    #else
    rtsp_socket_sendmsg,
    rtsp_socket_recvmsg,
    #endif
};
