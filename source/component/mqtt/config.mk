#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = $(MQTT_ROOT)
MODULEAPPDIR = component/mqtt
#
vpath %.c mqttlib

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
MQTT_OBJS+=mqtt_app_publish.o
MQTT_OBJS+=mqtt_app_subscribed.o
MQTT_OBJS+=mqtt_app_show.o
MQTT_OBJS+=mqtt_app_api.o
ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
MQTT_OBJS += cmd_mqtt.o
endif
#############################################################################
# LIB
###########################################################################
ifeq ($(WITH_SHARED_LIBRARIES),yes)
LIBS=libmosquitto.so
endif

ifeq ($(WITH_STATIC_LIBRARIES),yes)
LIBS=libmosquitto.a
endif

