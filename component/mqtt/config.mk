#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = component/mqtt/mqttlib
MODULEAPPDIR = component/mqtt
#
SOVERSION=1
VERSION=1.6.3
#
#PLM_INCLUDE += -I$(MQTT_ROOT)/mqtts
PLM_INCLUDE += -I$(MQTT_ROOT)/mqtts/deps

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
WITH_THREADING:=yes
# Comment out to remove bridge support from the broker. This allow the broker
# to connect to other brokers and subscribe/publish to topics. You probably
# want to leave this included unless you want to save a very small amount of
# memory size and CPU time.
WITH_BRIDGE:=no
# Comment out to remove persistent database support from the broker. This
# allows the broker to store retained messages and durable subscriptions to a
# file periodically and on shutdown. This is usually desirable (and is
# suggested by the MQTT spec), but it can be disabled if required.
WITH_PERSISTENCE:=no
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
# Build with SRV lookup support.
WITH_SRV:=no
# Build with websockets support on the broker.
WITH_WEBSOCKETS:=no
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
#WITH_BUNDLED_DEPS:=yes
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
	PLOS_LDLIBS +=-lwebsockets
endif
#
ifeq ($(WITH_EPOLL),yes)
	PLM_DEFINE += -DWITH_EPOLL
endif
#
MOSQ_OBJS=mosquitto.o \
		  actions.o \
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
		  
MQTT_OBJS=mqtt_app_api.o



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