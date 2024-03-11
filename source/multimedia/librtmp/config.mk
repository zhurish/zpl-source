#############################################################################
# DEFINE
###########################################################################


VERSION=v2.3
ZPL_DEFINE += -DRTMPDUMP_VERSION=\"$(VERSION)\"

OBJS += rtmp.o rtmplog.o rtmpamf.o rtmphashswf.o rtmpparseurl.o 
#rtmpsrv.o

OBJS += rtmp-chunk-header.o rtmp-chunk-read.o rtmp-chunk-write.o rtmp-client.o \
	rtmp-control-handler.o rtmp-control-message.o rtmp-event.o rtmp-handler.o rtmp-handshake.o \
	rtmp-invoke-handler.o rtmp-netconnection.o rtmp-netstream.o rtmp-server.o

#OBJS += rtmp_thread.o rtmpgw.o rtmpsuck.o rtmpsrv.o rtmpdump.o 

ifeq ($(strip $(ZPL_OPENSSL_MODULE)),true)
#ZPL_DEFINE += -DUSE_OPENSSL
#ZPL_LDLIBS += -lssl -lcrypto -lz
else
ZPL_DEFINE += -DNO_CRYPTO -DNO_SSL
endif
ifeq ($(strip $(ZPL_POLARSSL_MODULE)),true)
ZPL_DEFINE += -DUSE_POLARSSL
ZPL_LDLIBS += -lpolarssl -lz
endif
ifeq ($(strip $(ZPL_GNUTLS_MODULE)),true)
ZPL_DEFINE += -DUSE_GNUTLS
ZPL_LDLIBS += -lgnutls -lgcrypt -lz
endif

#############################################################################
# LIB
###########################################################################
LIBS = librtmp.a
