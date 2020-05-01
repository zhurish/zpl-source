include ../config.mk

.PHONY: all install clean

OBJS_DIR = $(BROKER_DIR)/output

buildobj : 
	@if test ! -d $(OBJS_DIR) ; \
		then \
		mkdir -p $(OBJS_DIR) ; \
	fi
	
ifeq ($(WITH_TLS),yes)
all : buildobj mosquitto mosquitto_passwd
else
all : buildobj mosquitto
endif

OBJS=	$(OBJS_DIR)/mosquitto.o \
		$(OBJS_DIR)/alias_mosq.o \
		$(OBJS_DIR)/bridge.o \
		$(OBJS_DIR)/conf.o \
		$(OBJS_DIR)/conf_includedir.o \
		$(OBJS_DIR)/context.o \
		$(OBJS_DIR)/database.o \
		$(OBJS_DIR)/handle_auth.o \
		$(OBJS_DIR)/handle_connack.o \
		$(OBJS_DIR)/handle_connect.o \
		$(OBJS_DIR)/handle_disconnect.o \
		$(OBJS_DIR)/handle_ping.o \
		$(OBJS_DIR)/handle_pubackcomp.o \
		$(OBJS_DIR)/handle_publish.o \
		$(OBJS_DIR)/handle_pubrec.o \
		$(OBJS_DIR)/handle_pubrel.o \
		$(OBJS_DIR)/handle_suback.o \
		$(OBJS_DIR)/handle_subscribe.o \
		$(OBJS_DIR)/handle_unsuback.o \
		$(OBJS_DIR)/handle_unsubscribe.o \
		$(OBJS_DIR)/logging.o \
		$(OBJS_DIR)/loop.o \
		$(OBJS_DIR)/memory_mosq.o \
		$(OBJS_DIR)/misc_mosq.o \
		$(OBJS_DIR)/net.o \
		$(OBJS_DIR)/net_mosq.o \
		$(OBJS_DIR)/net_mosq_ocsp.o \
		$(OBJS_DIR)/packet_datatypes.o \
		$(OBJS_DIR)/packet_mosq.o \
		$(OBJS_DIR)/property_broker.o \
		$(OBJS_DIR)/property_mosq.o \
		$(OBJS_DIR)/persist_read.o \
		$(OBJS_DIR)/persist_read_v234.o \
		$(OBJS_DIR)/persist_read_v5.o \
		$(OBJS_DIR)/persist_write.o \
		$(OBJS_DIR)/persist_write_v5.o \
		$(OBJS_DIR)/plugin.o \
		$(OBJS_DIR)/read_handle.o \
		$(OBJS_DIR)/security.o \
		$(OBJS_DIR)/security_default.o \
		$(OBJS_DIR)/send_auth.o \
		$(OBJS_DIR)/send_connack.o \
		$(OBJS_DIR)/send_connect.o \
		$(OBJS_DIR)/send_disconnect.o \
		$(OBJS_DIR)/send_mosq.o \
		$(OBJS_DIR)/send_publish.o \
		$(OBJS_DIR)/send_suback.o \
		$(OBJS_DIR)/send_subscribe.o \
		$(OBJS_DIR)/send_unsuback.o \
		$(OBJS_DIR)/send_unsubscribe.o \
		$(OBJS_DIR)/service.o \
		$(OBJS_DIR)/session_expiry.o \
		$(OBJS_DIR)/signals.o \
		$(OBJS_DIR)/subs.o \
		$(OBJS_DIR)/sys_tree.o \
		$(OBJS_DIR)/time_mosq.o \
		$(OBJS_DIR)/tls_mosq.o \
		$(OBJS_DIR)/utf8_mosq.o \
		$(OBJS_DIR)/util_mosq.o \
		$(OBJS_DIR)/util_topic.o \
		$(OBJS_DIR)/websockets.o \
		$(OBJS_DIR)/will_delay.o \
		$(OBJS_DIR)/will_mosq.o

#

mosquitto : ${OBJS}
	@$(CC) ${BROKER_LDFLAGS} $^ -o $@ $(BROKER_LDADD)

$(OBJS_DIR)/mosquitto.o : mosquitto.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/alias_mosq.o : ../mqttlib/alias_mosq.c ../mqttlib/alias_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/bridge.o : bridge.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/conf.o : conf.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/conf_includedir.o : conf_includedir.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/context.o : context.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/database.o : database.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_auth.o : handle_auth.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_connack.o : handle_connack.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_connect.o : handle_connect.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_disconnect.o : handle_disconnect.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_ping.o : ../mqttlib/handle_ping.c ../mqttlib/read_handle.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_pubackcomp.o : ../mqttlib/handle_pubackcomp.c ../mqttlib/read_handle.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_publish.o : handle_publish.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_pubrec.o : ../mqttlib/handle_pubrec.c ../mqttlib/read_handle.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_pubrel.o : ../mqttlib/handle_pubrel.c ../mqttlib/read_handle.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_suback.o : ../mqttlib/handle_suback.c ../mqttlib/read_handle.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_subscribe.o : handle_subscribe.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_unsuback.o : ../mqttlib/handle_unsuback.c ../mqttlib/read_handle.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/handle_unsubscribe.o : handle_unsubscribe.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/logging.o : logging.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/loop.o : loop.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/memory_mosq.o : ../mqttlib/memory_mosq.c ../mqttlib/memory_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/misc_mosq.o : ../mqttlib/misc_mosq.c ../mqttlib/misc_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/net.o : net.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/net_mosq_ocsp.o : ../mqttlib/net_mosq_ocsp.c ../mqttlib/net_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/net_mosq.o : ../mqttlib/net_mosq.c ../mqttlib/net_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/persist_read.o : persist_read.c persist.h mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/persist_read_v234.o : persist_read_v234.c persist.h mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/persist_read_v5.o : persist_read_v5.c persist.h mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/persist_write.o : persist_write.c persist.h mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/persist_write_v5.o : persist_write_v5.c persist.h mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/packet_datatypes.o : ../mqttlib/packet_datatypes.c ../mqttlib/packet_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/packet_mosq.o : ../mqttlib/packet_mosq.c ../mqttlib/packet_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/property_broker.o : property_broker.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/property_mosq.o : ../mqttlib/property_mosq.c ../mqttlib/property_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/plugin.o : plugin.c mosquitto_plugin.h mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/read_handle.o : read_handle.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/security.o : security.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/security_default.o : security_default.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/send_auth.o : send_auth.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/send_connect.o : ../mqttlib/send_connect.c ../mqttlib/send_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/send_disconnect.o : ../mqttlib/send_disconnect.c ../mqttlib/send_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/send_connack.o : send_connack.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/send_mosq.o : ../mqttlib/send_mosq.c ../mqttlib/send_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/send_publish.o : ../mqttlib/send_publish.c ../mqttlib/send_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/send_suback.o : send_suback.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/send_subscribe.o : ../mqttlib/send_subscribe.c ../mqttlib/send_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/send_unsuback.o : send_unsuback.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/send_unsubscribe.o : ../mqttlib/send_unsubscribe.c ../mqttlib/send_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/service.o : service.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/session_expiry.o : session_expiry.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/signals.o : signals.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/subs.o : subs.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/sys_tree.o : sys_tree.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/time_mosq.o : ../mqttlib/time_mosq.c ../mqttlib/time_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/tls_mosq.o : ../mqttlib/tls_mosq.c
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/util_mosq.o : ../mqttlib/util_mosq.c ../mqttlib/util_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/util_topic.o : ../mqttlib/util_topic.c ../mqttlib/util_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/utf8_mosq.o : ../mqttlib/utf8_mosq.c
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/websockets.o : websockets.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/will_delay.o : will_delay.c mosquitto_broker_internal.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

$(OBJS_DIR)/will_mosq.o : ../mqttlib/will_mosq.c ../mqttlib/will_mosq.h
	@$(CC) $(BROKER_CPPFLAGS) $(BROKER_CFLAGS) -c $< -o $@

mosquitto_passwd : $(OBJS_DIR)/mosquitto_passwd.o $(OBJS_DIR)/misc_mosq.o
	@$(CC) ${LDFLAGS} $^ -o $@ $(PASSWD_LDADD)

$(OBJS_DIR)/mosquitto_passwd.o : mosquitto_passwd.c
	@$(CC) -I.. -I../mqttlib $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)/plugin_defer.so : plugin_defer.c mosquitto_plugin.h mosquitto_broker.h mosquitto_broker_internal.h
	@$(CC) -I. -I../mqttlib -fPIC -shared $< -o $@

$(OBJS_DIR)/plugin_debug.so : plugin_debug.c mosquitto_plugin.h mosquitto_broker.h mosquitto_broker_internal.h
	@$(CC) -I. -I../mqttlib -fPIC -shared $< -o $@


INSTALL?=install


ifneq ($(DEST_BIN_DIR),)
DESTBINDIR=$(DEST_BIN_DIR)
else
DESTBINDIR=$(BROKER_DIR)/install_sbin
endif



install : 
	@$(INSTALL) -d $(DESTBINDIR)
	@$(INSTALL) mosquitto $(DESTBINDIR)
ifeq ($(WITH_TLS),yes)
	@$(INSTALL) mosquitto_passwd $(DESTBINDIR)
endif

clean : 
	@rm -f *.o mosquitto mosquitto_passwd *.gcda *.gcno
	@rm -f $(DESTBINDIR)/mosquitto_passwd
	@rm -f $(DESTBINDIR)/mosquitto
	@rm -rf $(OBJS_DIR)
ifeq ($(DEST_BIN_DIR),)
	@rm -rf $(DESTBINDIR)	
endif




