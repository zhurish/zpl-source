#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = component/osip/libosip/src/osipparser2
#
PLINCLUDE += -I$(OSIP_ROOT)/libosip/include
#
#
#OS
OBJS += osip_accept.o
OBJS += osip_accept_encoding.o
OBJS += osip_accept_language.o
OBJS += osip_alert_info.o
OBJS += osip_allow.o
OBJS += osip_authentication_info.o
OBJS += osip_authorization.o
OBJS += osip_body.o
OBJS += osip_call_id.o
OBJS += osip_call_info.o
OBJS += osip_contact.o
OBJS += osip_content_disposition.o
OBJS += osip_content_encoding.o
OBJS += osip_content_length.o
OBJS += osip_content_type.o
OBJS += osip_cseq.o
OBJS += osip_error_info.o
OBJS += osip_from.o
OBJS += osip_header.o
OBJS += osip_list.o
OBJS += osip_md5c.o
OBJS += osip_message.o
OBJS += osip_message_parse.o
OBJS += osip_message_to_str.o
OBJS += osip_mime_version.o
OBJS += osip_parser_cfg.o
OBJS += osip_port.o
OBJS += osip_proxy_authenticate.o
OBJS += osip_proxy_authentication_info.o
OBJS += osip_proxy_authorization.o
OBJS += osip_record_route.o
OBJS += osip_route.o
OBJS += osip_to.o
OBJS += osip_uri.o
OBJS += osip_via.o
OBJS += osip_www_authenticate.o
OBJS += sdp_accessor.o
OBJS += sdp_message.o
#############################################################################
# LIB
###########################################################################
LIBS = libosipparser2.a
