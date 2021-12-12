#
#   goahead-vxworks-default.mk -- Makefile to build Embedthis GoAhead Community Edition for vxworks
#

NAME                  := goahead
VERSION               := 5.1.1
PROFILE               ?= default
ARCH                  ?= $(shell echo $(WIND_HOST_TYPE) | sed 's/-.*$(ME_ROOT_PREFIX)/')
CPU                   ?= $(subst X86,PENTIUM,$(shell echo $(ARCH) | tr a-z A-Z))
OS                    ?= vxworks
CC                    ?= cc$(subst x86,pentium,$(ARCH))
LD                    ?= ldundefined
AR                    ?= arundefined
CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)
ZPL_BUILD                 ?= build/$(CONFIG)
LBIN                  ?= $(ZPL_BUILD)/bin
PATH                  := $(LBIN):$(PATH)

ME_COM_COMPILER       ?= 1
ME_COM_LIB            ?= 1
ME_COM_LINK           ?= 1
ME_COM_MATRIXSSL      ?= 0
ME_COM_MBEDTLS        ?= 1
ME_COM_NANOSSL        ?= 0
ME_COM_OPENSSL        ?= 0
ME_COM_OSDEP          ?= 1
ME_COM_SSL            ?= 1
ME_COM_VXWORKS        ?= 0


ifeq ($(ME_COM_LIB),1)
    ME_COM_COMPILER := 1
endif
ifeq ($(ME_COM_LINK),1)
    ME_COM_COMPILER := 1
endif
ifeq ($(ME_COM_MBEDTLS),1)
    ME_COM_SSL := 1
endif

export PATH           := $(WIND_GNU_PATH)/$(WIND_HOST_TYPE)/bin:$(PATH)
CFLAGS                += -fno-builtin -fno-defer-pop -fvolatile -w
DFLAGS                += -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h" $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_COMPILER=$(ME_COM_COMPILER) -DME_COM_LIB=$(ME_COM_LIB) -DME_COM_LINK=$(ME_COM_LINK) -DME_COM_MATRIXSSL=$(ME_COM_MATRIXSSL) -DME_COM_MBEDTLS=$(ME_COM_MBEDTLS) -DME_COM_NANOSSL=$(ME_COM_NANOSSL) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) 
IFLAGS                += "-I$(ZPL_BUILD)/inc"
LDFLAGS               += '-Wl,-r'
LIBPATHS              += -L$(ZPL_BUILD)/bin
LIBS                  += -lgcc

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

ME_ROOT_PREFIX        ?= deploy
ME_BASE_PREFIX        ?= $(ME_ROOT_PREFIX)
ME_DATA_PREFIX        ?= $(ME_VAPP_PREFIX)
ME_STATE_PREFIX       ?= $(ME_VAPP_PREFIX)
ME_BIN_PREFIX         ?= $(ME_VAPP_PREFIX)
ME_INC_PREFIX         ?= $(ME_VAPP_PREFIX)/inc
ME_LIB_PREFIX         ?= $(ME_VAPP_PREFIX)
ME_MAN_PREFIX         ?= $(ME_VAPP_PREFIX)
ME_SBIN_PREFIX        ?= $(ME_VAPP_PREFIX)
ME_ETC_PREFIX         ?= $(ME_VAPP_PREFIX)
ME_WEB_PREFIX         ?= $(ME_VAPP_PREFIX)/web
ME_LOG_PREFIX         ?= $(ME_VAPP_PREFIX)
ME_SPOOL_PREFIX       ?= $(ME_VAPP_PREFIX)
ME_CACHE_PREFIX       ?= $(ME_VAPP_PREFIX)
ME_APP_PREFIX         ?= $(ME_BASE_PREFIX)
ME_VAPP_PREFIX        ?= $(ME_APP_PREFIX)
ME_SRC_PREFIX         ?= $(ME_ROOT_PREFIX)/usr/src/$(NAME)-$(VERSION)


TARGETS               += $(ZPL_BUILD)/bin/goahead.out
TARGETS               += $(ZPL_BUILD)/bin/goahead-test.out
TARGETS               += $(ZPL_BUILD)/bin/gopass.out

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
	@if [ "$(WIND_BASE)" = "" ] ; then echo WARNING: WIND_BASE not set. Run wrenv.sh. ; exit 255 ; fi
	@if [ "$(WIND_HOST_TYPE)" = "" ] ; then echo WARNING: WIND_HOST_TYPE not set. Run wrenv.sh. ; exit 255 ; fi
	@if [ "$(WIND_GNU_PATH)" = "" ] ; then echo WARNING: WIND_GNU_PATH not set. Run wrenv.sh. ; exit 255 ; fi
	@[ ! -x $(ZPL_BUILD)/bin ] && mkdir -p $(ZPL_BUILD)/bin; true
	@[ ! -x $(ZPL_BUILD)/inc ] && mkdir -p $(ZPL_BUILD)/inc; true
	@[ ! -x $(ZPL_BUILD)/obj ] && mkdir -p $(ZPL_BUILD)/obj; true
	@[ ! -f $(ZPL_BUILD)/inc/me.h ] && cp projects/goahead-vxworks-default-me.h $(ZPL_BUILD)/inc/me.h ; true
	@if ! diff $(ZPL_BUILD)/inc/me.h projects/goahead-vxworks-default-me.h >/dev/null ; then\
		cp projects/goahead-vxworks-default-me.h $(ZPL_BUILD)/inc/me.h  ; \
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
	rm -f "$(ZPL_BUILD)/bin/goahead.out"
	rm -f "$(ZPL_BUILD)/bin/goahead-test.out"
	rm -f "$(ZPL_BUILD)/bin/gopass.out"
	rm -f "$(ZPL_BUILD)/.install-certs-modified"
	rm -f "$(ZPL_BUILD)/bin/libgo.out"
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
	$(CC) -c -o $(ZPL_BUILD)/obj/action.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/action.c

#
#   alloc.o
#
DEPS_8 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/alloc.o: \
    src/alloc.c $(DEPS_8)
	@echo '   [Compile] $(ZPL_BUILD)/obj/alloc.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/alloc.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/alloc.c

#
#   auth.o
#
DEPS_9 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/auth.o: \
    src/auth.c $(DEPS_9)
	@echo '   [Compile] $(ZPL_BUILD)/obj/auth.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/auth.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/auth.c

#
#   cgi.o
#
DEPS_10 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/cgi.o: \
    src/cgi.c $(DEPS_10)
	@echo '   [Compile] $(ZPL_BUILD)/obj/cgi.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/cgi.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/cgi.c

#
#   cgitest.o
#

$(ZPL_BUILD)/obj/cgitest.o: \
    test/cgitest.c $(DEPS_11)
	@echo '   [Compile] $(ZPL_BUILD)/obj/cgitest.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/cgitest.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" $(IFLAGS) test/cgitest.c

#
#   crypt.o
#
DEPS_12 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/crypt.o: \
    src/crypt.c $(DEPS_12)
	@echo '   [Compile] $(ZPL_BUILD)/obj/crypt.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/crypt.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/crypt.c

#
#   file.o
#
DEPS_13 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/file.o: \
    src/file.c $(DEPS_13)
	@echo '   [Compile] $(ZPL_BUILD)/obj/file.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/file.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/file.c

#
#   fs.o
#
DEPS_14 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/fs.o: \
    src/fs.c $(DEPS_14)
	@echo '   [Compile] $(ZPL_BUILD)/obj/fs.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/fs.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/fs.c

#
#   goahead-mbedtls.o
#
DEPS_15 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/goahead-mbedtls.o: \
    src/goahead-mbedtls/goahead-mbedtls.c $(DEPS_15)
	@echo '   [Compile] $(ZPL_BUILD)/obj/goahead-mbedtls.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/goahead-mbedtls.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/goahead-mbedtls/goahead-mbedtls.c

#
#   goahead.o
#
DEPS_16 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/goahead.o: \
    src/goahead.c $(DEPS_16)
	@echo '   [Compile] $(ZPL_BUILD)/obj/goahead.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/goahead.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/goahead.c

#
#   gopass.o
#
DEPS_17 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/gopass.o: \
    src/utils/gopass.c $(DEPS_17)
	@echo '   [Compile] $(ZPL_BUILD)/obj/gopass.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/gopass.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/utils/gopass.c

#
#   http.o
#
DEPS_18 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/http.o: \
    src/http.c $(DEPS_18)
	@echo '   [Compile] $(ZPL_BUILD)/obj/http.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/http.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/http.c

#
#   js.o
#
DEPS_19 += $(ZPL_BUILD)/inc/js.h

$(ZPL_BUILD)/obj/js.o: \
    src/js.c $(DEPS_19)
	@echo '   [Compile] $(ZPL_BUILD)/obj/js.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/js.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/js.c

#
#   jst.o
#
DEPS_20 += $(ZPL_BUILD)/inc/goahead.h
DEPS_20 += $(ZPL_BUILD)/inc/js.h

$(ZPL_BUILD)/obj/jst.o: \
    src/jst.c $(DEPS_20)
	@echo '   [Compile] $(ZPL_BUILD)/obj/jst.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/jst.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/jst.c

#
#   mbedtls.h
#

src/mbedtls/mbedtls.h: $(DEPS_21)

#
#   mbedtls.o
#
DEPS_22 += src/mbedtls/mbedtls.h

$(ZPL_BUILD)/obj/mbedtls.o: \
    src/mbedtls/mbedtls.c $(DEPS_22)
	@echo '   [Compile] $(ZPL_BUILD)/obj/mbedtls.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/mbedtls.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/mbedtls/mbedtls.c

#
#   options.o
#
DEPS_23 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/options.o: \
    src/options.c $(DEPS_23)
	@echo '   [Compile] $(ZPL_BUILD)/obj/options.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/options.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/options.c

#
#   osdep.o
#
DEPS_24 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/osdep.o: \
    src/osdep.c $(DEPS_24)
	@echo '   [Compile] $(ZPL_BUILD)/obj/osdep.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/osdep.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/osdep.c

#
#   rom.o
#
DEPS_25 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/rom.o: \
    src/rom.c $(DEPS_25)
	@echo '   [Compile] $(ZPL_BUILD)/obj/rom.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/rom.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/rom.c

#
#   route.o
#
DEPS_26 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/route.o: \
    src/route.c $(DEPS_26)
	@echo '   [Compile] $(ZPL_BUILD)/obj/route.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/route.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/route.c

#
#   runtime.o
#
DEPS_27 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/runtime.o: \
    src/runtime.c $(DEPS_27)
	@echo '   [Compile] $(ZPL_BUILD)/obj/runtime.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/runtime.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/runtime.c

#
#   socket.o
#
DEPS_28 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/socket.o: \
    src/socket.c $(DEPS_28)
	@echo '   [Compile] $(ZPL_BUILD)/obj/socket.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/socket.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/socket.c

#
#   test.o
#
DEPS_29 += $(ZPL_BUILD)/inc/goahead.h
DEPS_29 += $(ZPL_BUILD)/inc/js.h

$(ZPL_BUILD)/obj/test.o: \
    test/test.c $(DEPS_29)
	@echo '   [Compile] $(ZPL_BUILD)/obj/test.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/test.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) test/test.c

#
#   time.o
#
DEPS_30 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/time.o: \
    src/time.c $(DEPS_30)
	@echo '   [Compile] $(ZPL_BUILD)/obj/time.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/time.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/time.c

#
#   upload.o
#
DEPS_31 += $(ZPL_BUILD)/inc/goahead.h

$(ZPL_BUILD)/obj/upload.o: \
    src/upload.c $(DEPS_31)
	@echo '   [Compile] $(ZPL_BUILD)/obj/upload.o'
	$(CC) -c -o $(ZPL_BUILD)/obj/upload.o $(CFLAGS) -DME_DEBUG=1 -DVXWORKS -DRW_MULTI_THREAD -DCPU=PENTIUM -DTOOL_FAMILY=gnu -DTOOL=gnu -D_GNU_TOOL -D_WRS_KERNEL_ -D_VSB_CONFIG_FILE=\"/WindRiver/vxworks-7/samples/prebuilt_projects/vsb_vxsim_linux/h/config/vsbConfig.h\" -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" $(IFLAGS) src/upload.c

ifeq ($(ME_COM_MBEDTLS),1)
#
#   libmbedtls
#
DEPS_32 += $(ZPL_BUILD)/inc/osdep.h
DEPS_32 += $(ZPL_BUILD)/inc/embedtls.h
DEPS_32 += $(ZPL_BUILD)/inc/mbedtls.h
DEPS_32 += $(ZPL_BUILD)/obj/mbedtls.o

$(ZPL_BUILD)/bin/libmbedtls.a: $(DEPS_32)
	@echo '      [Link] $(ZPL_BUILD)/bin/libmbedtls.a'
	$(AR) -cr $(ZPL_BUILD)/bin/libmbedtls.a "$(ZPL_BUILD)/obj/mbedtls.o"
endif

ifeq ($(ME_COM_MBEDTLS),1)
#
#   libgoahead-mbedtls
#
DEPS_33 += $(ZPL_BUILD)/bin/libmbedtls.a
DEPS_33 += $(ZPL_BUILD)/obj/goahead-mbedtls.o

$(ZPL_BUILD)/bin/libgoahead-mbedtls.a: $(DEPS_33)
	@echo '      [Link] $(ZPL_BUILD)/bin/libgoahead-mbedtls.a'
	$(AR) -cr $(ZPL_BUILD)/bin/libgoahead-mbedtls.a "$(ZPL_BUILD)/obj/goahead-mbedtls.o"
endif

#
#   libgo
#
DEPS_34 += $(ZPL_BUILD)/inc/osdep.h
ifeq ($(ME_COM_MBEDTLS),1)
    DEPS_34 += $(ZPL_BUILD)/bin/libgoahead-mbedtls.a
endif
DEPS_34 += $(ZPL_BUILD)/inc/goahead.h
DEPS_34 += $(ZPL_BUILD)/inc/js.h
DEPS_34 += $(ZPL_BUILD)/obj/action.o
DEPS_34 += $(ZPL_BUILD)/obj/alloc.o
DEPS_34 += $(ZPL_BUILD)/obj/auth.o
DEPS_34 += $(ZPL_BUILD)/obj/cgi.o
DEPS_34 += $(ZPL_BUILD)/obj/crypt.o
DEPS_34 += $(ZPL_BUILD)/obj/file.o
DEPS_34 += $(ZPL_BUILD)/obj/fs.o
DEPS_34 += $(ZPL_BUILD)/obj/http.o
DEPS_34 += $(ZPL_BUILD)/obj/js.o
DEPS_34 += $(ZPL_BUILD)/obj/jst.o
DEPS_34 += $(ZPL_BUILD)/obj/options.o
DEPS_34 += $(ZPL_BUILD)/obj/osdep.o
DEPS_34 += $(ZPL_BUILD)/obj/rom.o
DEPS_34 += $(ZPL_BUILD)/obj/route.o
DEPS_34 += $(ZPL_BUILD)/obj/runtime.o
DEPS_34 += $(ZPL_BUILD)/obj/socket.o
DEPS_34 += $(ZPL_BUILD)/obj/time.o
DEPS_34 += $(ZPL_BUILD)/obj/upload.o

$(ZPL_BUILD)/bin/libgo.out: $(DEPS_34)
	@echo '      [Link] $(ZPL_BUILD)/bin/libgo.out'
	$(CC) -r -o $(ZPL_BUILD)/bin/libgo.out $(LDFLAGS) $(LIBPATHS) "$(ZPL_BUILD)/obj/action.o" "$(ZPL_BUILD)/obj/alloc.o" "$(ZPL_BUILD)/obj/auth.o" "$(ZPL_BUILD)/obj/cgi.o" "$(ZPL_BUILD)/obj/crypt.o" "$(ZPL_BUILD)/obj/file.o" "$(ZPL_BUILD)/obj/fs.o" "$(ZPL_BUILD)/obj/http.o" "$(ZPL_BUILD)/obj/js.o" "$(ZPL_BUILD)/obj/jst.o" "$(ZPL_BUILD)/obj/options.o" "$(ZPL_BUILD)/obj/osdep.o" "$(ZPL_BUILD)/obj/rom.o" "$(ZPL_BUILD)/obj/route.o" "$(ZPL_BUILD)/obj/runtime.o" "$(ZPL_BUILD)/obj/socket.o" "$(ZPL_BUILD)/obj/time.o" "$(ZPL_BUILD)/obj/upload.o" -lgoahead-mbedtls -lmbedtls $(LIBS) 

#
#   install-certs
#
DEPS_35 += src/certs/samples/ca.crt
DEPS_35 += src/certs/samples/ca.key
DEPS_35 += src/certs/samples/ec.crt
DEPS_35 += src/certs/samples/ec.key
DEPS_35 += src/certs/samples/roots.crt
DEPS_35 += src/certs/samples/self.crt
DEPS_35 += src/certs/samples/self.key
DEPS_35 += src/certs/samples/test.crt
DEPS_35 += src/certs/samples/test.key

$(ZPL_BUILD)/.install-certs-modified: $(DEPS_35)
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
DEPS_36 += $(ZPL_BUILD)/bin/libgo.out
DEPS_36 += $(ZPL_BUILD)/.install-certs-modified
DEPS_36 += $(ZPL_BUILD)/inc/goahead.h
DEPS_36 += $(ZPL_BUILD)/inc/js.h
DEPS_36 += $(ZPL_BUILD)/obj/goahead.o

$(ZPL_BUILD)/bin/goahead.out: $(DEPS_36)
	@echo '      [Link] $(ZPL_BUILD)/bin/goahead.out'
	$(CC) -o $(ZPL_BUILD)/bin/goahead.out $(LDFLAGS) $(LIBPATHS) "$(ZPL_BUILD)/obj/goahead.o" $(LIBS) -lgoahead-mbedtls -lmbedtls -Wl,-r 

#
#   goahead-test
#
DEPS_37 += $(ZPL_BUILD)/bin/libgo.out
DEPS_37 += $(ZPL_BUILD)/.install-certs-modified
DEPS_37 += $(ZPL_BUILD)/obj/test.o

$(ZPL_BUILD)/bin/goahead-test.out: $(DEPS_37)
	@echo '      [Link] $(ZPL_BUILD)/bin/goahead-test.out'
	$(CC) -o $(ZPL_BUILD)/bin/goahead-test.out $(LDFLAGS) $(LIBPATHS) "$(ZPL_BUILD)/obj/test.o" $(LIBS) -lgoahead-mbedtls -lmbedtls -Wl,-r 

#
#   gopass
#
DEPS_38 += $(ZPL_BUILD)/bin/libgo.out
DEPS_38 += $(ZPL_BUILD)/inc/goahead.h
DEPS_38 += $(ZPL_BUILD)/inc/js.h
DEPS_38 += $(ZPL_BUILD)/obj/gopass.o

$(ZPL_BUILD)/bin/gopass.out: $(DEPS_38)
	@echo '      [Link] $(ZPL_BUILD)/bin/gopass.out'
	$(CC) -o $(ZPL_BUILD)/bin/gopass.out $(LDFLAGS) $(LIBPATHS) "$(ZPL_BUILD)/obj/gopass.o" $(LIBS) -lgoahead-mbedtls -lmbedtls -Wl,-r 

#
#   stop
#

stop: $(DEPS_39)

#
#   installBinary
#

installBinary: $(DEPS_40)

#
#   start
#

start: $(DEPS_41)

#
#   install
#
DEPS_42 += stop
DEPS_42 += installBinary
DEPS_42 += start

install: $(DEPS_42)

#
#   installPrep
#

installPrep: $(DEPS_43)
	if [ "`id -u`" != 0 ] ; \
	then echo "Must run as root. Rerun with sudo." ; \
	exit 255 ; \
	fi

#
#   uninstall
#
DEPS_44 += stop

uninstall: $(DEPS_44)

#
#   uninstallBinary
#

uninstallBinary: $(DEPS_45)

#
#   version
#

version: $(DEPS_46)
	echo $(VERSION)

