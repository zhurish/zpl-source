#############################################################################
# DEFINE
###########################################################################
MODULEDIR = multimedia/exosip

OSIP_CFLAGS = -g -O2 -pthread
OSIP_DEFINE = -DHAVE_CONFIG_H -DENABLE_TRACE
OSIP_DEBUG = 
OSIP_INCLUDE += -I$(BASE_ROOT)/$(MODULEDIR)/libosip2-5.2.0
OSIP_INCLUDE += -I$(BASE_ROOT)/$(MODULEDIR)/libosip2-5.2.0/include
OSIP_INCLUDE += -I$(BASE_ROOT)/$(MODULEDIR)/libosip2-5.2.0/src
OSIP_INCLUDE += -I$(BASE_ROOT)/$(MODULEDIR)/libosip2-5.2.0/src/osip2
OSIP_INCLUDE += -I$(BASE_ROOT)/$(MODULEDIR)/libosip2-5.2.0/src/osipparser2

OSIP_INCLUDE += -I$(BASE_ROOT)/$(MODULEDIR)/libexosip2-5.2.0
OSIP_INCLUDE += -I$(BASE_ROOT)/$(MODULEDIR)/libexosip2-5.2.0/include
OSIP_INCLUDE += -I$(BASE_ROOT)/$(MODULEDIR)/libexosip2-5.2.0/src


OSIP_CFLAGS += -lrt -lresolv -lssl -lcrypto
#EXOSIP_LIB =  -lrt -lresolv -lssl -lcrypto
#OSIP_CFLAGS = -I/home/zhurish/workspace/working/SWPlatform/source/rootfs_install/include 
#OSIP_LIBS = -L/home/zhurish/workspace/working/SWPlatform/source/rootfs_install/lib -losipparser2 -losip2 

libosip2_obj += ict_fsm.o ist_fsm.o nict_fsm.o \
	nist_fsm.o ict.o ist.o nict.o nist.o fsm_misc.o osip.o \
	osip_transaction.o osip_event.o port_fifo.o osip_dialog.o \
	osip_time.o port_sema.o port_thread.o port_condv.o

libosip2_obj += port_sema.o port_thread.o \
	port_condv.o


osip2_includedir = osip2
osip2_include_HEADERS = \
	osip.h     osip_dialog.h \
	osip_mt.h  osip_fifo.h   osip_condv.h       \
	osip_time.h


libosip2_lib=libosip2.a
libosip2_libso=libosip2.so



libosipparser2_obj += osip_accept_encoding.o osip_content_encoding.o \
	osip_authentication_info.o  osip_proxy_authentication_info.o \
	osip_accept_language.o      osip_accept.o                    \
	osip_alert_info.o           osip_error_info.o                \
	osip_allow.o                \
	sdp_accessor.o              sdp_message.o

libosipparser2_obj += osip_proxy_authorization.o \
	osip_cseq.o osip_record_route.o osip_route.o osip_to.o \
	osip_from.o osip_uri.o osip_authorization.o osip_header.o \
	osip_www_authenticate.o osip_via.o osip_body.o osip_md5c.o \
	osip_message.o osip_list.o osip_call_id.o osip_message_parse.o \
	osip_contact.o osip_message_to_str.o osip_content_length.o \
	osip_parser_cfg.o osip_content_type.o \
	osip_proxy_authenticate.o osip_mime_version.o osip_port.o \
	osip_call_info.o osip_content_disposition.o \
	osip_accept_encoding.o osip_content_encoding.o \
	osip_authentication_info.o osip_proxy_authentication_info.o \
	osip_accept_language.o osip_accept.o osip_alert_info.o \
	osip_error_info.o osip_allow.o sdp_accessor.o sdp_message.o

#EXTRA_DIST = internal.h

osipparser2_includedir=osipparser2

osipparser2_include_HEADERS=\
	osip_const.h   osip_md5.h      osip_parser.h  osip_uri.h      \
	osip_list.h    osip_message.h  osip_port.h    sdp_message.h   \
	osip_headers.h osip_body.h 

osipparser2_headers_includedir=osipparser2/headers

	osipparser2_headers_include_HEADERS= \
	osip_accept.h           osip_contact.h              osip_mime_version.h       \
	osip_accept_encoding.h  osip_content_disposition.h  osip_proxy_authenticate.h \
	osip_accept_language.h  osip_content_encoding.h     osip_proxy_authorization.h\
	osip_alert_info.h       osip_content_length.h       osip_record_route.h       \
	osip_allow.h            osip_content_type.h         osip_route.h              \
	osip_authorization.h    osip_cseq.h                 osip_to.h                 \
	osip_call_info.h        osip_error_info.h           osip_via.h                \
	osip_call_id.h          osip_from.h                 osip_www_authenticate.h   \
	osip_header.h           osip_authentication_info.h  \
	osip_proxy_authentication_info.h

libosipparser2_lib=libosipparser2.a
libosipparser2_libso=libosipparser2.so



libexosip2_obj = \
	eXsubscription_api.o    eXoptions_api.o    \
	eXinsubscription_api.o  eXpublish_api.o    \
	jnotify.o               jsubscribe.o       \
	inet_ntop.o             inet_ntop.h        \
	jpublish.o              sdp_offans.o

libexosip2_obj = eXosip.o eXconf.o eXregister_api.o \
	eXcall_api.o eXmessage_api.o eXtransport.o jrequest.o \
	jresponse.o jcallback.o jdialog.o udp.o jcall.o jreg.o \
	eXutils.o jevents.o misc.o jpipe.o jauth.o \
	eXtl_udp.o eXtl_tcp.o eXtl_dtls.o \
	eXtl_tls.o milenage.o rijndael.o  \
	eXsubscription_api.o eXoptions_api.o eXinsubscription_api.o \
	eXpublish_api.o jnotify.o jsubscribe.o inet_ntop.o \
	jpublish.o sdp_offans.o

eXosip_includedir = eXosip2
eXosip_include_HEADERS = eXosip.h eX_setup.h \
	eX_register.h eX_call.h eX_options.h \
	eX_subscribe.h eX_publish.h eX_message.h

libexosip2_lib = libeXosip2.a
libexosip2_libso = libeXosip2.so

ifeq ($(strip $(ZPL_OPENH264_MODULE)),true)

else ifeq ($(strip $(ZPL_LIBX264_MODULE)),true)

endif
ifeq ($(strip $(ZPL_LIBVPX_MODULE)),true)

endif
ifeq ($(strip $(ZPL_FFMPEG_MODULE)),true)

endif
#############################################################################
# LIB
###########################################################################

