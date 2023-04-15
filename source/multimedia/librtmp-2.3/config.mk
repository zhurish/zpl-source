#############################################################################
# DEFINE
###########################################################################
MODULEDIR = multimedia/librtmp-2.3


OBJS += rtmp.o rtmp_log.o amf.o hashswf.o rtmp_parseurl.o 

OBJS += rtmp_thread.o rtmpgw.o rtmpsuck.o rtmpsrv.o rtmpdump.o 


ifeq ($(strip $(ZPL_OPENSSL_MODULE)),true)
ZPL_DEFINE += -DDEF_CRYPTO
ZPL_DEFINE += -DUSE_OPENSSL
ZPL_LDLIBS += -lssl -lcrypto -lz
else
ZPL_DEFINE += -DNO_CRYPTO
endif
ifeq ($(strip $(ZPL_POLARSSL_MODULE)),true)
ZPL_DEFINE += -DUSE_POLARSSL
ZPL_LDLIBS += -lpolarssl -lz
endif
ifeq ($(strip $(ZPL_GNUTLS_MODULE)),true)
ZPL_DEFINE += -DUSE_GNUTLS
ZPL_LDLIBS += -lgnutls -lgcrypt -lz
endif
VERSION=v2.3
ZPL_DEFINE += -DRTMPDUMP_VERSION=\"$(VERSION)\"
#############################################################################
# LIB
###########################################################################
LIBS = librtmp.a
