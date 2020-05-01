#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = component/mqtt/mqttlib
MODULEAPPDIR = component/mqtt
#
SOVERSION=1
VERSION=1.6.9
#
#PLM_INCLUDE += -I$(MQTT_ROOT)/mqtts
#PLM_INCLUDE += -I$(MQTT_ROOT)/mqtts/deps

ifeq ($(MQTT_TLS_ENABLE),true)
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
WITH_THREADING:=no
#WITH_THREADING:=yes BROKER
# Comment out to remove bridge support from the broker. This allow the broker
# to connect to other brokers and subscribe/publish to topics. You probably
# want to leave this included unless you want to save a very small amount of
# memory size and CPU time.
WITH_BRIDGE:=no
#WITH_BRIDGE:=yes BROKER
# Comment out to remove persistent database support from the broker. This
# allows the broker to store retained messages and durable subscriptions to a
# file periodically and on shutdown. This is usually desirable (and is
# suggested by the MQTT spec), but it can be disabled if required.
WITH_PERSISTENCE:=no
#WITH_PERSISTENCE:=yes BROKER
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
WITH_SOCKS:=no
#WITH_SOCKS:=yes BROKER
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
WITH_BUNDLED_DEPS:=yes
#WITH_BUNDLED_DEPS:=yes BROKER
# Build with coverage options
#WITH_COVERAGE:=no
#
#
ifeq ($(WITH_TLS),yes)
	PLM_DEFINE += -DWITH_TLS
	ifeq ($(WITH_TLS_PSK),yes)
	PLM_DEFINE += -DWITH_TLS_PSK
	endif
endif

ifeq ($(WITH_THREADING),yes)
	PLM_DEFINE += -DWITH_THREADING
endif

ifeq ($(WITH_BRIDGE),yes)
	PLM_DEFINE += -DWITH_BRIDGE
endif

ifeq ($(WITH_PERSISTENCE),yes)
	PLM_DEFINE += -DWITH_PERSISTENCE
endif

ifeq ($(WITH_MEMORY_TRACKING),yes)
	PLM_DEFINE +=  -DWITH_MEMORY_TRACKING
endif

ifeq ($(WITH_SYS_TREE),yes)
	PLM_DEFINE += -DWITH_SYS_TREE
endif
ifeq ($(WITH_EC),yes)
	PLM_DEFINE += -DWITH_EC
endif
ifeq ($(WITH_SRV),yes)
	PLM_DEFINE += -DWITH_SRV
	PLOS_LDLIBS += -lcares
endif

ifeq ($(WITH_ADNS),yes)
	PLOS_LDLIBS += -lanl
	PLM_DEFINE += -DWITH_ADNS
endif

ifeq ($(WITH_WEBSOCKETS),yes)
	PLM_DEFINE += -DWITH_WEBSOCKETS
	PLOS_LDLIBS += -lwebsockets
endif
#
ifeq ($(WITH_SOCKS),yes)
	PLM_DEFINE:= -DWITH_SOCKS
endif

ifeq ($(WITH_EPOLL),yes)
	PLM_DEFINE += -DWITH_EPOLL
endif

ifeq ($(WITH_SYSTEMD),yes)
	PLM_DEFINE += -DWITH_SYSTEMD
	PLOS_LDLIBS += -lsystemd
endif

#
MOSQ_OBJS=mosquitto.o \
		  actions.o \
		  alias_mosq.o \
		  callbacks.o \
		  connect.o \
		  handle_auth.o \
		  handle_connack.o \
		  handle_disconnect.o \
		  handle_ping.o \
		  handle_pubackcomp.o \
		  handle_publish.o \
		  handle_pubrec.o \
		  handle_pubrel.o \
		  handle_suback.o \
		  handle_unsuback.o \
		  helpers.o \
		  logging_mosq.o \
		  loop.o \
		  memory_mosq.o \
		  messages_mosq.o \
		  misc_mosq.o \
		  net_mosq_ocsp.o \
		  net_mosq.o \
		  options.o \
		  packet_datatypes.o \
		  packet_mosq.o \
		  property_mosq.o \
		  read_handle.o \
		  send_connect.o \
		  send_disconnect.o \
		  send_mosq.o \
		  send_publish.o \
		  send_subscribe.o \
		  send_unsubscribe.o \
		  socks_mosq.o \
		  srv_mosq.o \
		  thread_mosq.o \
		  time_mosq.o \
		  tls_mosq.o \
		  utf8_mosq.o \
		  util_mosq.o \
		  util_topic.o \
		  will_mosq.o
		  
MQTT_OBJS=mqtt_app_util.o
MQTT_OBJS+=mqtt_app_conf.o
#MQTT_OBJS+=mqtt_app_publish.o
MQTT_OBJS+=mqtt_app_subscribed.o
MQTT_OBJS+=mqtt_app_api.o
#############################################################################
# LIB
###########################################################################
ifeq ($(WITH_SHARED_LIBRARIES),yes)
LIBS=libmosquitto.so
endif

ifeq ($(WITH_STATIC_LIBRARIES),yes)
LIBS=libmosquitto.a
endif

#APPLIBS=libmqtt.a
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