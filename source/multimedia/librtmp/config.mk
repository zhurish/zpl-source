#############################################################################
# DEFINE
###########################################################################


VERSION=v2.3
ZPL_DEFINE += -DRTMPDUMP_VERSION=\"$(VERSION)\"

OBJS += rtmplog.o rtmpamf.o rtmpparseurl.o rtmphashswf.o rtmp.o  

OBJS += rtmp-client-api.o rtmp-send-h264.o

#OBJS += rtmp_thread.o rtmpgw.o rtmpsuck.o rtmpsrv.o rtmpdump.o 

ifeq ($(strip $(ZPL_OPENSSL_MODULE)),true)
#ZPL_DEFINE += -DUSE_OPENSSL -DCRYPTO
#ZPL_LDLIBS += -lssl -lcrypto -lz
ZPL_DEFINE += -DNO_CRYPTO 
else
ZPL_DEFINE += -DNO_CRYPTO 
endif
ifeq ($(strip $(ZPL_POLARSSL_MODULE)),true)
ZPL_DEFINE += -DUSE_POLARSSL
#ZPL_LDLIBS += -lpolarssl -lz
endif
ifeq ($(strip $(ZPL_GNUTLS_MODULE)),true)
ZPL_DEFINE += -DUSE_GNUTLS
#ZPL_LDLIBS += -lgnutls -lgcrypt -lz
endif

#############################################################################
# LIB
###########################################################################
LIBS = librtmp.a
