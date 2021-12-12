#
#   goahead-linux-static.mk -- Makefile to build Embedthis GoAhead Core for linux
#

NAME                  := goahead
VERSION               := 5.0.0
PROFILE               ?= static
ARCH                  ?= $(shell uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/')
CC_ARCH               ?= $(shell echo $(ARCH) | sed 's/x86/i686/;s/x64/x86_64/')
OS                    ?= linux
CC                    ?= gcc
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
ZPL_BUILD                 ?= build/$(CONFIG)
LBIN                  ?= $(ZPL_BUILD)/bin
PATH                  := $(LBIN):$(PATH)

ME_COM_COMPILER       ?= 1
ME_COM_LIB            ?= 1
ME_COM_MATRIXSSL      ?= 0
ME_COM_MBEDTLS        ?= 1
ME_COM_NANOSSL        ?= 0
ME_COM_OPENSSL        ?= 0
ME_COM_OSDEP          ?= 1
ME_COM_SSL            ?= 1
ME_COM_VXWORKS        ?= 0

ME_COM_OPENSSL_PATH   ?= "/path/to/openssl"

ifeq ($(ME_COM_LIB),1)
    ME_COM_COMPILER := 1
endif
ifeq ($(ME_COM_MBEDTLS),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_OPENSSL),1)
    ME_COM_SSL := 1
endif

CFLAGS                += -fstack-protector --param=ssp-buffer-size=4 -Wformat -Wformat-security -Wl,-z,relro,-z,now -Wl,--as-needed -Wl,--no-copy-dt-needed-entries -Wl,-z,noexecstatck -Wl,-z,noexecheap -pie -fPIE -w
DFLAGS                += -DME_DEBUG=1 $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_COMPILER=$(ME_COM_COMPILER) -DME_COM_LIB=$(ME_COM_LIB) -DME_COM_MATRIXSSL=$(ME_COM_MATRIXSSL) -DME_COM_MBEDTLS=$(ME_COM_MBEDTLS) -DME_COM_NANOSSL=$(ME_COM_NANOSSL) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) 
IFLAGS                += "-I$(ZPL_BUILD)/inc"
LDFLAGS               += 
LIBPATHS              += -L$(ZPL_BUILD)/bin
LIBS                  += -lrt -ldl -lpthread -lm

DEBUG                 ?= debug
CFLAGS-debug          ?= -g
DFLAGS-debug          ?= -DME_DEBUG
LDFLAGS-debug         ?= -g
DFLAGS-release        ?= 
CFLAGS-release        ?= -O2
LDFLAGS-release       ?= 
CFLAGS                += $(CFLAGS-$(DEBUG))
DFLAGS                += $(DFLAGS-$(DEBUG))
LDFLAGS               += $(LDFLAGS-$(DEBUG))

ME_ROOT_PREFIX        ?= 
ME_BASE_PREFIX        ?= $(ME_ROOT_PREFIX)/usr/local
ME_DATA_PREFIX        ?= $(ME_ROOT_PREFIX)/
ME_STATE_PREFIX       ?= $(ME_ROOT_PREFIX)/var
ME_APP_PREFIX         ?= $(ME_BASE_PREFIX)/lib/$(NAME)
ME_VAPP_PREFIX        ?= $(ME_APP_PREFIX)/$(VERSION)
ME_BIN_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/bin
ME_INC_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/include
ME_LIB_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/lib
ME_MAN_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/local/share/man
ME_SBIN_PREFIX        ?= $(ME_ROOT_PREFIX)/usr/local/sbin
ME_ETC_PREFIX         ?= $(ME_ROOT_PREFIX)/etc/$(NAME)
ME_WEB_PREFIX         ?= $(ME_ROOT_PREFIX)/var/www/$(NAME)
ME_LOG_PREFIX         ?= $(ME_ROOT_PREFIX)/var/log/$(NAME)
ME_SPOOL_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)
ME_CACHE_PREFIX       ?= $(ME_ROOT_PREFIX)/var/spool/$(NAME)/cache
ME_SRC_PREFIX         ?= $(ME_ROOT_PREFIX)$(NAME)-$(VERSION)


TARGETS               += $(ZPL_BUILD)/bin/goahead
TARGETS               += $(ZPL_BUILD)/bin/goahead-test
TARGETS               += $(ZPL_BUILD)/bin/gopass

unexport CDPATH

ifndef SHOW
.SILENT:
endif

all build compile: prep $(TARGETS)

.PHONY: prep

prep:
	@echo "      [Info] Use "make SHOW=1" to trace executed commands."
	@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi
	@if [ "$(ME_APP_PREFIX)" = "" ] ; then echo WARNING: ME_APP_PREFIX not set ; exit 255 ; fi
	@[ ! -x $(ZPL_BUILD)/bin ] && mkdir -p $(ZPL_BUILD)/bin; true
	@[ ! -x $(ZPL_BUILD)/inc ] && mkdir -p $(ZPL_BUILD)/inc; true
	@[ ! -x $(ZPL_BUILD)/obj ] && mkdir -p $(ZPL_BUILD)/obj; true
	@[ ! -f $(ZPL_BUILD)/inc/me.h ] && cp projects/goahead-linux-static-me.h $(ZPL_BUILD)/inc/me.h ; true
	@if ! diff $(ZPL_BUILD)/inc/me.h projects/goahead-linux-static-me.h >/dev/null ; then\
		cp projects/goahead-linux-static-me.h $(ZPL_BUILD)/inc/me.h  ; \
	fi; true
	@if [ -f "$(ZPL_BUILD)/.makeflags" ] ; then \
		if [ "$(MAKEFLAGS)" != "`cat $(ZPL_BUILD)/.makeflags`" ] ; then \
			echo "   [Warning] Make flags have changed since the last build" ; \
			echo "   [Warning] Previous build command: "`cat $(ZPL_BUILD)/.makeflags`"" ; \
		fi ; \
	fi
	@echo "$(MAKEFLAGS)" >$(ZPL_BUILD)/.makeflags

clean:
	rm -f "$(ZPL_BUILD)/obj/action.o"
	rm -f "$(ZPL_BUILD)/obj/alloc.o"
	rm -f "$(ZPL_BUILD)/obj/auth.o"
	rm -f "$(ZPL_BUILD)/obj/cgi.o"
	rm -f "$(ZPL_BUILD)/obj/cgitest.o"
	rm -f "$(ZPL_BUILD)/obj/crypt.o"
	rm -f "$(ZPL_BUILD)/obj/file.o"
	rm -f "$(ZPL_BUILD)/obj/fs.o"
	rm -f "$(ZPL_BUILD)/obj/goahead-mbedtls.o"
	rm -f "$(ZPL_BUILD)/obj/goahead-openssl.o"
	rm -f "$(ZPL_BUILD)/obj/goahead.o"
	rm -f "$(ZPL_BUILD)/obj/gopass.o"
	rm -f "$(ZPL_BUILD)/obj/http.o"
	rm -f "$(ZPL_BUILD)/obj/js.o"
	rm -f "$(ZPL_BUILD)/obj/jst.o"
	rm -f "$(ZPL_BUILD)/obj/mbedtls.o"
	rm -f "$(ZPL_BUILD)/obj/options.o"
	rm -f "$(ZPL_BUILD)/obj/osdep.o"
	rm -f "$(ZPL_BUILD)/obj/rom.o"
	rm -f "$(ZPL_BUILD)/obj/route.o"
	rm -f "$(ZPL_BUILD)/obj/runtime.o"
	rm -f "$(ZPL_BUILD)/obj/socket.o"
	rm -f "$(ZPL_BUILD)/obj/test.o"
	rm -f "$(ZPL_BUILD)/obj/time.o"
	rm -f "$(ZPL_BUILD)/obj/upload.o"
	rm -f "$(ZPL_BUILD)/bin/goahead"
	rm -f "$(ZPL_BUILD)/bin/goahead-test"
	rm -f "$(ZPL_BUILD)/bin/gopass"
	rm -f "$(ZPL_BUILD)/.install-certs-modified"
	rm -f "$(ZPL_BUILD)/bin/libgo.a"
	rm -f "$(ZPL_BUILD)/bin/libgoahead-mbedtls.a"
	rm -f "$(ZPL_BUILD)/bin/libmbedtls.a"

clobber: clean
	rm -fr ./$(ZPL_BUILD)

#
#   embedtls.h
#
DEPS_1 += src/mbedtls/embedtls.h

$(ZPL_BUILD)/inc/embedtls.h: $(DEPS_1)
	@echo '      [Copy] $(ZPL_BUILD)/inc/embedtls.h'
	mkdir -p "$(ZPL_BUILD)/inc"
	cp src/mbedtls/embedtls.h $(ZPL_BUILD)/inc/embedtls.h

#
#   me.h
#

$(ZPL_BUILD)/inc/me.h: $(DEPS_2)

#
#   osdep.h
#
DEPS_3 += src/osdep/osdep.h
DEPS_3 += $(ZPL_BUILD)/inc/me.h

$(ZPL_BUILD)/inc/osdep.h: $(DEPS_3)
	@echo '      [Copy] $(ZPL_BUILD)/inc/osdep.h'
	mkdir -p "$(ZPL_BUILD)/inc"
	cp src/osdep/osdep.h $(ZPL_BUILD)/inc/osdep.h

#
#   goahead.h
#
DEPS_4 += src/goahead.h
DEPS_4 += $(ZPL_BUILD)/inc/me.h
DEPS_4 += $(ZPL_BUILD)/inc/osdep.h

$(ZPL_BUILD)/inc/goahead.h: $(DEPS_4)
	@echo '      [Copy] $(ZPL_BUILD)/inc/goahead.h'
	mkdir -p "$(ZPL_BUILD)/inc"
	cp src/goahead.h $(ZPL_BUILD)/inc/goahead.h

#
#   js.h
#
DEPS_5 += src/js.h
DEPS_5 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/inc/js.h: $(DEPS_5)
	@echo '      [Copy] $(ZPL_BUILD)/inc/js.h'
	mkdir -p "$(ZPL_BUILD)/inc"
	cp src/js.h $(ZPL_BUILD)/inc/js.h

#
#   mbedtls.h
#
DEPS_6 += src/mbedtls/mbedtls.h

$(ZPL_BUILD)/inc/mbedtls.h: $(DEPS_6)
	@echo '      [Copy] $(ZPL_BUILD)/inc/mbedtls.h'
	mkdir -p "$(ZPL_BUILD)/inc"
	cp src/mbedtls/mbedtls.h $(ZPL_BUILD)/inc/mbedtls.h

#
#   action.o
#
DEPS_7 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/action.o: \
    src/action.c $(DEPS_7)
	@echo '   [Compile] $(ZPL_BUILD)/obj/action.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/action.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/action.c

#
#   alloc.o
#
DEPS_8 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/alloc.o: \
    src/alloc.c $(DEPS_8)
	@echo '   [Compile] $(ZPL_BUILD)/obj/alloc.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/alloc.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/alloc.c

#
#   auth.o
#
DEPS_9 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/auth.o: \
    src/auth.c $(DEPS_9)
	@echo '   [Compile] $(ZPL_BUILD)/obj/auth.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/auth.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/auth.c

#
#   cgi.o
#
DEPS_10 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/cgi.o: \
    src/cgi.c $(DEPS_10)
	@echo '   [Compile] $(ZPL_BUILD)/obj/cgi.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/cgi.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/cgi.c

#
#   cgitest.o
#

$(ZPL_BUILD)/obj/cgitest.o: \
    test/cgitest.c $(DEPS_11)
	@echo '   [Compile] $(ZPL_BUILD)/obj/cgitest.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/cgitest.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) $(IFLAGS) test/cgitest.c

#
#   crypt.o
#
DEPS_12 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/crypt.o: \
    src/crypt.c $(DEPS_12)
	@echo '   [Compile] $(ZPL_BUILD)/obj/crypt.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/crypt.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/crypt.c

#
#   file.o
#
DEPS_13 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/file.o: \
    src/file.c $(DEPS_13)
	@echo '   [Compile] $(ZPL_BUILD)/obj/file.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/file.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/file.c

#
#   fs.o
#
DEPS_14 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/fs.o: \
    src/fs.c $(DEPS_14)
	@echo '   [Compile] $(ZPL_BUILD)/obj/fs.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/fs.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/fs.c

#
#   goahead-mbedtls.o
#
DEPS_15 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/goahead-mbedtls.o: \
    src/goahead-mbedtls/goahead-mbedtls.c $(DEPS_15)
	@echo '   [Compile] $(ZPL_BUILD)/obj/goahead-mbedtls.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/goahead-mbedtls.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/goahead-mbedtls/goahead-mbedtls.c

#
#   goahead-openssl.o
#
DEPS_16 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/goahead-openssl.o: \
    src/goahead-openssl/goahead-openssl.c $(DEPS_16)
	@echo '   [Compile] $(ZPL_BUILD)/obj/goahead-openssl.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/goahead-openssl.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) $(IFLAGS) "-I$(ZPL_BUILD)/inc" "-I$(ME_COM_OPENSSL_PATH)/include" src/goahead-openssl/goahead-openssl.c

#
#   goahead.o
#
DEPS_17 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/goahead.o: \
    src/goahead.c $(DEPS_17)
	@echo '   [Compile] $(ZPL_BUILD)/obj/goahead.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/goahead.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/goahead.c

#
#   gopass.o
#
DEPS_18 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/gopass.o: \
    src/utils/gopass.c $(DEPS_18)
	@echo '   [Compile] $(ZPL_BUILD)/obj/gopass.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/gopass.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/utils/gopass.c

#
#   http.o
#
DEPS_19 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/http.o: \
    src/http.c $(DEPS_19)
	@echo '   [Compile] $(ZPL_BUILD)/obj/http.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/http.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/http.c

#
#   js.o
#
DEPS_20 += $(ZPL_BUILD)/inc/js.h

$(ZPL_BUILD)/obj/js.o: \
    src/js.c $(DEPS_20)
	@echo '   [Compile] $(ZPL_BUILD)/obj/js.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/js.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/js.c

#
#   jst.o
#
DEPS_21 += $(ZPL_BUILD)/inc/goahead.h
DEPS_21 += $(ZPL_BUILD)/inc/js.h

$(ZPL_BUILD)/obj/jst.o: \
    src/jst.c $(DEPS_21)
	@echo '   [Compile] $(ZPL_BUILD)/obj/jst.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/jst.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/jst.c

#
#   mbedtls.h
#

src/mbedtls/mbedtls.h: $(DEPS_22)

#
#   mbedtls.o
#
DEPS_23 += src/mbedtls/mbedtls.h

$(ZPL_BUILD)/obj/mbedtls.o: \
    src/mbedtls/mbedtls.c $(DEPS_23)
	@echo '   [Compile] $(ZPL_BUILD)/obj/mbedtls.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/mbedtls.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/mbedtls/mbedtls.c

#
#   options.o
#
DEPS_24 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/options.o: \
    src/options.c $(DEPS_24)
	@echo '   [Compile] $(ZPL_BUILD)/obj/options.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/options.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/options.c

#
#   osdep.o
#
DEPS_25 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/osdep.o: \
    src/osdep.c $(DEPS_25)
	@echo '   [Compile] $(ZPL_BUILD)/obj/osdep.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/osdep.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/osdep.c

#
#   rom.o
#
DEPS_26 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/rom.o: \
    src/rom.c $(DEPS_26)
	@echo '   [Compile] $(ZPL_BUILD)/obj/rom.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/rom.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/rom.c

#
#   route.o
#
DEPS_27 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/route.o: \
    src/route.c $(DEPS_27)
	@echo '   [Compile] $(ZPL_BUILD)/obj/route.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/route.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/route.c

#
#   runtime.o
#
DEPS_28 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/runtime.o: \
    src/runtime.c $(DEPS_28)
	@echo '   [Compile] $(ZPL_BUILD)/obj/runtime.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/runtime.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/runtime.c

#
#   socket.o
#
DEPS_29 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/socket.o: \
    src/socket.c $(DEPS_29)
	@echo '   [Compile] $(ZPL_BUILD)/obj/socket.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/socket.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/socket.c

#
#   test.o
#
DEPS_30 += $(ZPL_BUILD)/inc/goahead.h
DEPS_30 += $(ZPL_BUILD)/inc/js.h

$(ZPL_BUILD)/obj/test.o: \
    test/test.c $(DEPS_30)
	@echo '   [Compile] $(ZPL_BUILD)/obj/test.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/test.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" test/test.c

#
#   time.o
#
DEPS_31 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/time.o: \
    src/time.c $(DEPS_31)
	@echo '   [Compile] $(ZPL_BUILD)/obj/time.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/time.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/time.c

#
#   upload.o
#
DEPS_32 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/upload.o: \
    src/upload.c $(DEPS_32)
	@echo '   [Compile] $(ZPL_BUILD)/obj/upload.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/upload.o $(LDFLAGS) $(CFLAGS) $(DFLAGS) -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" -DME_COM_OPENSSL_PATH=$(ME_COM_OPENSSL_PATH) $(IFLAGS) "-I$(ME_COM_OPENSSL_PATH)/include" src/upload.c

ifeq ($(ME_COM_MBEDTLS),1)
#
#   libmbedtls
#
DEPS_33 += $(ZPL_BUILD)/inc/osdep.h
DEPS_33 += $(ZPL_BUILD)/inc/embedtls.h
DEPS_33 += $(ZPL_BUILD)/inc/mbedtls.h
DEPS_33 += $(ZPL_BUILD)/obj/mbedtls.o

$(ZPL_BUILD)/bin/libmbedtls.a: $(DEPS_33)
	@echo '      [Link] $(ZPL_BUILD)/bin/libmbedtls.a'
	ar -cr $(ZPL_BUILD)/bin/libmbedtls.a "$(ZPL_BUILD)/obj/mbedtls.o"
endif

ifeq ($(ME_COM_MBEDTLS),1)
#
#   libgoahead-mbedtls
#
DEPS_34 += $(ZPL_BUILD)/bin/libmbedtls.a
DEPS_34 += $(ZPL_BUILD)/obj/goahead-mbedtls.o

$(ZPL_BUILD)/bin/libgoahead-mbedtls.a: $(DEPS_34)
	@echo '      [Link] $(ZPL_BUILD)/bin/libgoahead-mbedtls.a'
	ar -cr $(ZPL_BUILD)/bin/libgoahead-mbedtls.a "$(ZPL_BUILD)/obj/goahead-mbedtls.o"
endif

ifeq ($(ME_COM_OPENSSL),1)
#
#   libgoahead-openssl
#
DEPS_35 += $(ZPL_BUILD)/obj/goahead-openssl.o

$(ZPL_BUILD)/bin/libgoahead-openssl.a: $(DEPS_35)
	@echo '      [Link] $(ZPL_BUILD)/bin/libgoahead-openssl.a'
	ar -cr $(ZPL_BUILD)/bin/libgoahead-openssl.a "$(ZPL_BUILD)/obj/goahead-openssl.o"
endif

#
#   libgo
#
DEPS_36 += $(ZPL_BUILD)/inc/osdep.h
ifeq ($(ME_COM_MBEDTLS),1)
    DEPS_36 += $(ZPL_BUILD)/bin/libgoahead-mbedtls.a
endif
ifeq ($(ME_COM_OPENSSL),1)
    DEPS_36 += $(ZPL_BUILD)/bin/libgoahead-openssl.a
endif
DEPS_36 += $(ZPL_BUILD)/inc/goahead.h
DEPS_36 += $(ZPL_BUILD)/inc/js.h
DEPS_36 += $(ZPL_BUILD)/obj/action.o
DEPS_36 += $(ZPL_BUILD)/obj/alloc.o
DEPS_36 += $(ZPL_BUILD)/obj/auth.o
DEPS_36 += $(ZPL_BUILD)/obj/cgi.o
DEPS_36 += $(ZPL_BUILD)/obj/crypt.o
DEPS_36 += $(ZPL_BUILD)/obj/file.o
DEPS_36 += $(ZPL_BUILD)/obj/fs.o
DEPS_36 += $(ZPL_BUILD)/obj/http.o
DEPS_36 += $(ZPL_BUILD)/obj/js.o
DEPS_36 += $(ZPL_BUILD)/obj/jst.o
DEPS_36 += $(ZPL_BUILD)/obj/options.o
DEPS_36 += $(ZPL_BUILD)/obj/osdep.o
DEPS_36 += $(ZPL_BUILD)/obj/rom.o
DEPS_36 += $(ZPL_BUILD)/obj/route.o
DEPS_36 += $(ZPL_BUILD)/obj/runtime.o
DEPS_36 += $(ZPL_BUILD)/obj/socket.o
DEPS_36 += $(ZPL_BUILD)/obj/time.o
DEPS_36 += $(ZPL_BUILD)/obj/upload.o

$(ZPL_BUILD)/bin/libgo.a: $(DEPS_36)
	@echo '      [Link] $(ZPL_BUILD)/bin/libgo.a'
	ar -cr $(ZPL_BUILD)/bin/libgo.a "$(ZPL_BUILD)/obj/action.o" "$(ZPL_BUILD)/obj/alloc.o" "$(ZPL_BUILD)/obj/auth.o" "$(ZPL_BUILD)/obj/cgi.o" "$(ZPL_BUILD)/obj/crypt.o" "$(ZPL_BUILD)/obj/file.o" "$(ZPL_BUILD)/obj/fs.o" "$(ZPL_BUILD)/obj/http.o" "$(ZPL_BUILD)/obj/js.o" "$(ZPL_BUILD)/obj/jst.o" "$(ZPL_BUILD)/obj/options.o" "$(ZPL_BUILD)/obj/osdep.o" "$(ZPL_BUILD)/obj/rom.o" "$(ZPL_BUILD)/obj/route.o" "$(ZPL_BUILD)/obj/runtime.o" "$(ZPL_BUILD)/obj/socket.o" "$(ZPL_BUILD)/obj/time.o" "$(ZPL_BUILD)/obj/upload.o"

#
#   install-certs
#
DEPS_37 += src/certs/samples/ca.crt
DEPS_37 += src/certs/samples/ca.key
DEPS_37 += src/certs/samples/ec.crt
DEPS_37 += src/certs/samples/ec.key
DEPS_37 += src/certs/samples/roots.crt
DEPS_37 += src/certs/samples/self.crt
DEPS_37 += src/certs/samples/self.key
DEPS_37 += src/certs/samples/test.crt
DEPS_37 += src/certs/samples/test.key

$(ZPL_BUILD)/.install-certs-modified: $(DEPS_37)
	@echo '      [Copy] $(ZPL_BUILD)/bin'
	mkdir -p "$(ZPL_BUILD)/bin"
	cp src/certs/samples/ca.crt $(ZPL_BUILD)/bin/ca.crt
	cp src/certs/samples/ca.key $(ZPL_BUILD)/bin/ca.key
	cp src/certs/samples/ec.crt $(ZPL_BUILD)/bin/ec.crt
	cp src/certs/samples/ec.key $(ZPL_BUILD)/bin/ec.key
	cp src/certs/samples/roots.crt $(ZPL_BUILD)/bin/roots.crt
	cp src/certs/samples/self.crt $(ZPL_BUILD)/bin/self.crt
	cp src/certs/samples/self.key $(ZPL_BUILD)/bin/self.key
	cp src/certs/samples/test.crt $(ZPL_BUILD)/bin/test.crt
	cp src/certs/samples/test.key $(ZPL_BUILD)/bin/test.key
	touch "$(ZPL_BUILD)/.install-certs-modified"

#
#   goahead
#
DEPS_38 += $(ZPL_BUILD)/bin/libgo.a
DEPS_38 += $(ZPL_BUILD)/.install-certs-modified
DEPS_38 += $(ZPL_BUILD)/inc/goahead.h
DEPS_38 += $(ZPL_BUILD)/inc/js.h
DEPS_38 += $(ZPL_BUILD)/obj/goahead.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_38 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_38 += -lgoahead-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_38 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_38 += -lgoahead-openssl
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_38 += -lssl
    LIBPATHS_38 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_38 += -lcrypto
    LIBPATHS_38 += -L"$(ME_COM_OPENSSL_PATH)"
endif
LIBS_38 += -lgo
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_38 += -lgoahead-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_38 += -lgoahead-mbedtls
endif

$(ZPL_BUILD)/bin/goahead: $(DEPS_38)
	@echo '      [Link] $(ZPL_BUILD)/bin/goahead'
	$(CC) -o $(ZPL_BUILD)/bin/goahead $(LDFLAGS) $(LIBPATHS)  "$(ZPL_BUILD)/obj/goahead.o" $(LIBPATHS_38) $(LIBS_38) $(LIBS_38) $(LIBS) $(LIBS) 

#
#   goahead-test
#
DEPS_39 += $(ZPL_BUILD)/bin/libgo.a
DEPS_39 += $(ZPL_BUILD)/.install-certs-modified
DEPS_39 += $(ZPL_BUILD)/obj/test.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_39 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_39 += -lgoahead-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_39 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_39 += -lgoahead-openssl
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_39 += -lssl
    LIBPATHS_39 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_39 += -lcrypto
    LIBPATHS_39 += -L"$(ME_COM_OPENSSL_PATH)"
endif
LIBS_39 += -lgo
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_39 += -lgoahead-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_39 += -lgoahead-mbedtls
endif

$(ZPL_BUILD)/bin/goahead-test: $(DEPS_39)
	@echo '      [Link] $(ZPL_BUILD)/bin/goahead-test'
	$(CC) -o $(ZPL_BUILD)/bin/goahead-test $(LDFLAGS) $(LIBPATHS)  "$(ZPL_BUILD)/obj/test.o" $(LIBPATHS_39) $(LIBS_39) $(LIBS_39) $(LIBS) $(LIBS) 

#
#   gopass
#
DEPS_40 += $(ZPL_BUILD)/bin/libgo.a
DEPS_40 += $(ZPL_BUILD)/inc/goahead.h
DEPS_40 += $(ZPL_BUILD)/inc/js.h
DEPS_40 += $(ZPL_BUILD)/obj/gopass.o

ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_40 += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_40 += -lgoahead-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_40 += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_40 += -lgoahead-openssl
endif
ifeq ($(ME_COM_OPENSSL),1)
ifeq ($(ME_COM_SSL),1)
    LIBS_40 += -lssl
    LIBPATHS_40 += -L"$(ME_COM_OPENSSL_PATH)"
endif
endif
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_40 += -lcrypto
    LIBPATHS_40 += -L"$(ME_COM_OPENSSL_PATH)"
endif
LIBS_40 += -lgo
ifeq ($(ME_COM_OPENSSL),1)
    LIBS_40 += -lgoahead-openssl
endif
ifeq ($(ME_COM_MBEDTLS),1)
    LIBS_40 += -lgoahead-mbedtls
endif

$(ZPL_BUILD)/bin/gopass: $(DEPS_40)
	@echo '      [Link] $(ZPL_BUILD)/bin/gopass'
	$(CC) -o $(ZPL_BUILD)/bin/gopass $(LDFLAGS) $(LIBPATHS)  "$(ZPL_BUILD)/obj/gopass.o" $(LIBPATHS_40) $(LIBS_40) $(LIBS_40) $(LIBS) $(LIBS) 

#
#   stop
#

stop: $(DEPS_41)

#
#   installBinary
#

installBinary: $(DEPS_42)
	mkdir -p "$(ME_APP_PREFIX)" ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	ln -s "$(VERSION)" "$(ME_APP_PREFIX)/latest" ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	chmod 755 "$(ME_MAN_PREFIX)/man1" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(ZPL_BUILD)/bin/goahead $(ME_VAPP_PREFIX)/bin/goahead ; \
	chmod 755 "$(ME_VAPP_PREFIX)/bin/goahead" ; \
	mkdir -p "$(ME_BIN_PREFIX)" ; \
	rm -f "$(ME_BIN_PREFIX)/goahead" ; \
	ln -s "$(ME_VAPP_PREFIX)/bin/goahead" "$(ME_BIN_PREFIX)/goahead" ; \
	mkdir -p "$(ME_VAPP_PREFIX)/bin" ; \
	cp $(ZPL_BUILD)/bin/roots.crt $(ME_VAPP_PREFIX)/bin/roots.crt ; \
	mkdir -p "$(ME_ETC_PREFIX)" ; \
	cp $(ZPL_BUILD)/bin/self.* $(ME_ETC_PREFIX)/self.* ; \
	mkdir -p "$(ME_WEB_PREFIX)" ; \
	cp src/web/index.html $(ME_WEB_PREFIX)/index.html ; \
	cp src/web/favicon.ico $(ME_WEB_PREFIX)/favicon.ico ; \
	mkdir -p "$(ME_ETC_PREFIX)" ; \
	cp src/auth.txt $(ME_ETC_PREFIX)/auth.txt ; \
	cp src/route.txt $(ME_ETC_PREFIX)/route.txt ; \
	mkdir -p "$(ME_VAPP_PREFIX)/doc/man/man1" ; \
	cp doc/dist/man/goahead.1 $(ME_VAPP_PREFIX)/doc/man/man1/goahead.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/goahead.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/goahead.1" "$(ME_MAN_PREFIX)/man1/goahead.1" ; \
	cp doc/dist/man/gopass.1 $(ME_VAPP_PREFIX)/doc/man/man1/gopass.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/gopass.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/gopass.1" "$(ME_MAN_PREFIX)/man1/gopass.1" ; \
	cp doc/dist/man/webcomp.1 $(ME_VAPP_PREFIX)/doc/man/man1/webcomp.1 ; \
	mkdir -p "$(ME_MAN_PREFIX)/man1" ; \
	rm -f "$(ME_MAN_PREFIX)/man1/webcomp.1" ; \
	ln -s "$(ME_VAPP_PREFIX)/doc/man/man1/webcomp.1" "$(ME_MAN_PREFIX)/man1/webcomp.1"

#
#   start
#

start: $(DEPS_43)

#
#   install
#
DEPS_44 += stop
DEPS_44 += installBinary
DEPS_44 += start

install: $(DEPS_44)

#
#   installPrep
#

installPrep: $(DEPS_45)
	if [ "`id -u`" != 0 ] ; \
	then echo "Must run as root. Rerun with sudo." ; \
	exit 255 ; \
	fi

#
#   uninstall
#
DEPS_46 += stop

uninstall: $(DEPS_46)

#
#   uninstallBinary
#

uninstallBinary: $(DEPS_47)
	rm -fr "$(ME_WEB_PREFIX)" ; \
	rm -fr "$(ME_VAPP_PREFIX)" ; \
	rmdir -p "$(ME_ETC_PREFIX)" 2>/dev/null ; true ; \
	rmdir -p "$(ME_WEB_PREFIX)" 2>/dev/null ; true ; \
	rm -f "$(ME_APP_PREFIX)/latest" ; \
	rmdir -p "$(ME_APP_PREFIX)" 2>/dev/null ; true

#
#   version
#

version: $(DEPS_48)
	echo $(VERSION)

