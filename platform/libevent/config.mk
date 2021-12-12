#############################################################################
# DEFINE
###########################################################################
MODULEDIR = platform/libevent
#OS

OBJS =					\
	buffer.o				\
	bufferevent.o				\
	bufferevent_filter.o			\
	bufferevent_pair.o			\
	bufferevent_ratelim.o			\
	bufferevent_sock.o			\
	event.o					\
	evmap.o					\
	evthread.o				\
	evutil.o				\
	evutil_rand.o				\
	evutil_time.o				\
	listener.o				\
	log.o		\
	evthread_pthread.o			

OBJS += strlcpy.o
OBJS += select.o
OBJS += poll.o
#OBJS += devpoll.o
#OBJS += kqueue.o
OBJS += epoll.o
#OBJS += evport.o
#ifeq ($(strip $(ZPL_LIBEVENT_SIGNAL)),true)
OBJS += \
	signal.o
#endif

OBJS +=					\
	evdns.o					\
	event_tagging.o				\
	evrpc.o					\
	http.o

ifeq ($(strip $(ZPL_OPENSSL_MODULE)),true)
OBJS += \
	bufferevent_openssl.o
endif

ifeq ($(strip $(ZPL_SYSTEM_WIN32)),true)
OBJS += win32select.o buffer_iocp.o event_iocp.o \
	bufferevent_async.o evthread_win32.o

ZPLEX_INCLUDE += -IWIN32-Code -IWIN32-Code/nmake

ZPLEX_LDLIBS += -liphlpapi
ZPLEX_LDLIBS += -lws2_32 -lshell32 -ladvapi32
endif

ifeq ($(strip $(ZPL_SYSTEM_OPENBSD)),true)
OBJS += \
	arc4random.o \
	epoll_sub.o
endif


#############################################################################
# LIB
###########################################################################
LIBS = libevent.a
