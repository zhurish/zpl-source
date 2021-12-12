SOVERSION=1
VERSION=1.6.9
###############################CLIENT###################################
#


ifeq ($(ZPL_MQTT_SSL),true)
WITH_TLS:=yes
# Comment out to disable TLS/PSK support in the broker and client. Requires
# WITH_TLS=yes.
# This must be disabled if using openssl < 1.0.
WITH_TLS_PSK:=yes
else
WITH_TLS:=no
# Comment out to disable TLS/PSK support in the broker and client. Requires
# WITH_TLS=yes.
# This must be disabled if using openssl < 1.0.
WITH_TLS_PSK:=no
endif
# Comment out to disable client threading support.
ifeq ($(ZPL_MQTT_BROKER),true)
WITH_THREADING:=yes
else
WITH_THREADING:=no
endif
# Comment out to remove bridge support from the broker. This allow the broker
# to connect to other brokers and subscribe/publish to topics. You probably
# want to leave this included unless you want to save a very small amount of
# memory size and CPU time.
ifeq ($(ZPL_MQTT_BROKER),true)
WITH_BRIDGE:=yes
else
WITH_BRIDGE:=no
endif
# Comment out to remove persistent database support from the broker. This
# allows the broker to store retained messages and durable subscriptions to a
# file periodically and on shutdown. This is usually desirable (and is
# suggested by the MQTT spec), but it can be disabled if required.
ifeq ($(ZPL_MQTT_BROKER),true)
WITH_PERSISTENCE:=yes
else
WITH_PERSISTENCE:=no
endif
# Comment out to remove memory tracking support from the broker. If disabled,
# mosquitto won't track heap memory usage nor export '$SYS/broker/heap/current
# size', but will use slightly less memory and CPU time.
WITH_MEMORY_TRACKING:=yes
# Compile with database upgrading support? If disabled, mosquitto won't
# automatically upgrade old database versions.
# Not currently supported.
#WITH_DB_UPGRADE:=yes
# Comment out to remove publishing of the $SYS topic hierarchy containing
# information about the broker state.
WITH_SYS_TREE:=yes
# Build with systemd support. If enabled, mosquitto will notify systemd after
# initialization. See README in service/systemd/ for more information.
WITH_SYSTEMD:=no
# Build with SRV lookup support.
WITH_SRV:=no
# Build with websockets support on the broker.
WITH_WEBSOCKETS:=no
# Use elliptic keys in broker
WITH_EC:=yes
# Build man page documentation by default.
WITH_DOCS:=no
# Build with client support for SOCK5 proxy.
ifeq ($(ZPL_MQTT_BROKER),true)
WITH_SOCKS:=yes
else
WITH_SOCKS:=no
endif
# Strip executables and shared libraries on install.
WITH_STRIP:=no
#ifeq ($(MQTT_SHARED_LIBRARIES),true)
# Build shared libraries
#WITH_SHARED_LIBRARIES:=yes
# Build static libraries
#WITH_STATIC_LIBRARIES:=no
#else
#WITH_SHARED_LIBRARIES:=no
# Build static libraries
WITH_STATIC_LIBRARIES:=yes
#endif
# Build with async dns lookup support for bridges (temporary). Requires glibc.
#WITH_ADNS:=yes
# Build with epoll support.
WITH_EPOLL:=yes
# Build with bundled uthash.h
ifeq ($(ZPL_MQTT_BROKER),true)
WITH_BUNDLED_DEPS:=yes
else
WITH_BUNDLED_DEPS:=no
endif
#WITH_BUNDLED_DEPS:=yes BROKER
# Build with coverage options
#WITH_COVERAGE:=no
#
ifeq ($(ZPL_MQTT_BROKER),false)
###############################CLIENT###################################
#
ifeq ($(WITH_TLS),yes)
	ZPLM_DEFINE += -DWITH_TLS
	ifeq ($(WITH_TLS_PSK),yes)
	ZPLM_DEFINE += -DWITH_TLS_PSK
	endif
endif

ifeq ($(WITH_THREADING),yes)
	ZPLM_DEFINE += -DWITH_THREADING
endif

ifeq ($(WITH_BRIDGE),yes)
	ZPLM_DEFINE += -DWITH_BRIDGE
endif

ifeq ($(WITH_PERSISTENCE),yes)
	ZPLM_DEFINE += -DWITH_PERSISTENCE
endif

ifeq ($(WITH_MEMORY_TRACKING),yes)
	ZPLM_DEFINE +=  -DWITH_MEMORY_TRACKING
endif

ifeq ($(WITH_SYS_TREE),yes)
	ZPLM_DEFINE += -DWITH_SYS_TREE
endif
ifeq ($(WITH_EC),yes)
	ZPLM_DEFINE += -DWITH_EC
endif
ifeq ($(WITH_SRV),yes)
	ZPLM_DEFINE += -DWITH_SRV
	ZPLOS_LDLIBS += -lcares
endif

ifeq ($(WITH_ADNS),yes)
	ZPLOS_LDLIBS += -lanl
	ZPLM_DEFINE += -DWITH_ADNS
endif

ifeq ($(WITH_WEBSOCKETS),yes)
	ZPLM_DEFINE += -DWITH_WEBSOCKETS
	ZPLOS_LDLIBS += -lwebsockets
endif
#
ifeq ($(WITH_SOCKS),yes)
	ZPLM_DEFINE:= -DWITH_SOCKS
endif

ifeq ($(WITH_EPOLL),yes)
	ZPLM_DEFINE += -DWITH_EPOLL
endif

ifeq ($(WITH_SYSTEMD),yes)
	ZPLM_DEFINE += -DWITH_SYSTEMD
	ZPLOS_LDLIBS += -lsystemd
endif

else
###############################BROKER###################################
BROKER_DIR = $(shell pwd)

LIB_CPPFLAGS=$(CPPFLAGS) -I. -I.. -I../mqttlib
ifeq ($(WITH_BUNDLED_DEPS),yes)
	LIB_CPPFLAGS:=$(LIB_CPPFLAGS) -I../src/deps
endif

BROKER_CPPFLAGS:=$(LIB_CPPFLAGS) 
#-I$(BROKER_DIR) -I$(BROKER_DIR)/mqttlib -I$(BROKER_DIR)/src
BROKER_CFLAGS:=${CFLAGS} -DVERSION="\"${VERSION}\"" -DWITH_BROKER
BROKER_LDFLAGS:=${LDFLAGS}
BROKER_LDADD:=
BROKER_LDADD:=$(BROKER_LDADD)  -ldl -lm

BROKER_LDADD:=$(BROKER_LDADD) -lrt
BROKER_LDFLAGS:=$(BROKER_LDFLAGS) -Wl,--dynamic-list=linker.syms
LIB_LIBADD:=$(LIB_LIBADD) -lrt
LIB_CFLAGS:=$(LIB_CFLAGS) -fPIC
LIB_CXXFLAGS:=$(LIB_CXXFLAGS) -fPIC
	
ifeq ($(WITH_TLS),yes)
BROKER_LDADD:=$(BROKER_LDADD) -lssl -lcrypto
LIB_LIBADD:=$(LIB_LIBADD) -lssl -lcrypto
BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_TLS
LIB_CPPFLAGS:=$(LIB_CPPFLAGS) -DWITH_TLS
PASSWD_LDADD:=$(PASSWD_LDADD) -lcrypto
STATIC_LIB_DEPS:=$(STATIC_LIB_DEPS) -lssl -lcrypto

ifeq ($(WITH_TLS_PSK),yes)
	BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_TLS_PSK
	LIB_CPPFLAGS:=$(LIB_CPPFLAGS) -DWITH_TLS_PSK
endif
endif


ifeq ($(WITH_THREADING),yes)
	LIB_LIBADD:=$(LIB_LIBADD) -lpthread
	LIB_CPPFLAGS:=$(LIB_CPPFLAGS) -DWITH_THREADING
	STATIC_LIB_DEPS:=$(STATIC_LIB_DEPS) -lpthread
endif

ifeq ($(WITH_SOCKS),yes)
	LIB_CPPFLAGS:=$(LIB_CPPFLAGS) -DWITH_SOCKS
endif

ifeq ($(WITH_BRIDGE),yes)
	BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_BRIDGE
endif
ifeq ($(WITH_PERSISTENCE),yes)
	BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_PERSISTENCE
endif

ifeq ($(WITH_MEMORY_TRACKING),yes)
	ifneq ($(UNAME),SunOS)
		BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_MEMORY_TRACKING
	endif
endif

ifeq ($(WITH_SYS_TREE),yes)
	BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_SYS_TREE
endif

ifeq ($(WITH_SYSTEMD),yes)
	BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_SYSTEMD
	BROKER_LDADD:=$(BROKER_LDADD) -lsystemd
endif

ifeq ($(WITH_SRV),yes)
	LIB_CPPFLAGS:=$(LIB_CPPFLAGS) -DWITH_SRV
	LIB_LIBADD:=$(LIB_LIBADD) -lcares
	STATIC_LIB_DEPS:=$(STATIC_LIB_DEPS) -lcares
endif

ifeq ($(WITH_EC),yes)
	BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_EC
endif

ifeq ($(WITH_ADNS),yes)
	BROKER_LDADD:=$(BROKER_LDADD) -lanl
	BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_ADNS
endif

ifeq ($(WITH_WEBSOCKETS),yes)
	BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_WEBSOCKETS
	BROKER_LDADD:=$(BROKER_LDADD) -lwebsockets
endif

ifeq ($(WITH_WEBSOCKETS),static)
	BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_WEBSOCKETS
	BROKER_LDADD:=$(BROKER_LDADD) -static -lwebsockets
endif

ifeq ($(WITH_EPOLL),yes)
	ifeq ($(UNAME),Linux)
		BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -DWITH_EPOLL
	endif
endif

ifeq ($(WITH_BUNDLED_DEPS),yes)
	BROKER_CPPFLAGS:=$(BROKER_CPPFLAGS) -Ideps
endif
endif