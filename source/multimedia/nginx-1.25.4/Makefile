include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

OBJS = $(OSOBJ)

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(LIBNGX_ROOT)/src
SRC_DIR = $(BASE_ROOT)/$(LIBNGX_ROOT)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
MODULE_OBJS_DEP:=$(MODULE_OBJS:%.o=%.d)

-include $(MODULE_OBJS_DEP)
vpath %.c src/core
vpath %.c src/event
vpath %.c src/event/modules
vpath %.c src/os/unix
vpath %.c src/http/modules
vpath %.c src/http
vpath %.c src/stream
vpath %.c nginx-rtmp-module
vpath %.c nginx-rtmp-module/hls
vpath %.c nginx-rtmp-module/dash
vpath %.c src

ZPL_INCLUDE += -I$(ZPLBASE)/$(LIBNGX_ROOT)/src/core \
	-I$(ZPLBASE)/$(LIBNGX_ROOT)/src/event \
	-I$(ZPLBASE)/$(LIBNGX_ROOT)/src/event/modules \
	-I$(ZPLBASE)/$(LIBNGX_ROOT)/src/event/quic \
	-I$(ZPLBASE)/$(LIBNGX_ROOT)/src/os/unix \
	-I$(ZPLBASE)/$(LIBNGX_ROOT)/src \
	-I$(ZPLBASE)/$(LIBNGX_ROOT)/src/http \
	-I$(ZPLBASE)/$(LIBNGX_ROOT)/src/http/modules \
	-I$(ZPLBASE)/$(LIBNGX_ROOT)/src/stream \
	-I$(ZPLBASE)/$(LIBNGX_ROOT)/nginx-rtmp-module
#
$(OBJS_DIR)/%.o: %.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ZPL_ECHO_CC) $(ZPL_LIB_COMPILE)
#

nginx : $(MODULE_OBJS)
	$(CC) $(MODULE_OBJS) $(ZPLCFLAGS) $(ZPLLDCLFLAG) -lpthread -lcrypt -lssl -lcrypto -lz -o nginx

# ./configure --prefix=/home/zhurish/Downloads/nginx-1.25.4/_install --with-select_module \
	--with-poll_module --with-threads --with-file-aio --without-quic_bpf_module --without-pcre \
	--without-http_rewrite_module --add-module=./nginx-rtmp-module --with-stream --with-stream_ssl_module
#
make_prepare:
	echo ""


lib: nginx


obj: $(MODULE_OBJS)

install:nginx
	@install -d ${ZPL_INSTALL_SBIN_DIR}
	@install -m 755 nginx ${ZPL_INSTALL_SBIN_DIR} 
	@$(STRIP) $(ZPL_INSTALL_SBIN_DIR)/nginx
	@install -d $(ZPL_INSTALL_ETC_DIR)/nginx
	@$(CP) conf/fastcgi.conf $(ZPL_INSTALL_ETC_DIR)/nginx/fastcgi.conf
	@$(CP) conf/nginx.conf $(ZPL_INSTALL_ETC_DIR)/nginx/nginx.conf

clean: objclean
	@if test -f $(ZPL_INSTALL_SBIN_DIR)/nginx ; \
		then \
		$(RM) -rf $(ZPL_INSTALL_SBIN_DIR)/nginx ; \
	fi
	$(RM) -rf $(ZPL_INSTALL_ETC_DIR)/nginx/* 

objclean: 
	@$(RM) -rf $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean make_prepare nginx
