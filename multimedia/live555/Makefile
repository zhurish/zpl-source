##### Change the following for your environment:
-include $(ZPL_MAKE_DIR)/makelinux.mk
#export PREFIX = $(DSTROOTFSDIR)
#export LIBDIR = $(PREFIX)/lib
export PREFIX = $(BASE_ROOT)/$(ZPL_RELEASEDIR)
export LIBDIR = $(BASE_ROOT)/$(ZPL_ULIB_DIR)
export INCDIR = $(BASE_ROOT)/$(ZPL_UINC_DIR)
export BINDIR = $(BASE_ROOT)/$(ZPL_RELEASEDIR)


INCLUDES = -I$(ZPL_INCLUDE) -Iinclude -I../UsageEnvironment/include -I../groupsock/include -I../liveMedia/include -I../BasicUsageEnvironment/include

LIVE555_SHARED_ENABLE=true


libliveMedia_VERSION_CURRENT=81
libliveMedia_VERSION_REVISION=7
libliveMedia_VERSION_AGE=0
libliveMedia_LIB_SUFFIX=so.$(shell expr $(libliveMedia_VERSION_CURRENT) - $(libliveMedia_VERSION_AGE)).$(libliveMedia_VERSION_AGE).$(libliveMedia_VERSION_REVISION)

libBasicUsageEnvironment_VERSION_CURRENT=1
libBasicUsageEnvironment_VERSION_REVISION=1
libBasicUsageEnvironment_VERSION_AGE=0
libBasicUsageEnvironment_LIB_SUFFIX=so.$(shell expr $(libBasicUsageEnvironment_VERSION_CURRENT) - $(libBasicUsageEnvironment_VERSION_AGE)).$(libBasicUsageEnvironment_VERSION_AGE).$(libBasicUsageEnvironment_VERSION_REVISION)

libUsageEnvironment_VERSION_CURRENT=4
libUsageEnvironment_VERSION_REVISION=0
libUsageEnvironment_VERSION_AGE=1
libUsageEnvironment_LIB_SUFFIX=so.$(shell expr $(libUsageEnvironment_VERSION_CURRENT) - $(libUsageEnvironment_VERSION_AGE)).$(libUsageEnvironment_VERSION_AGE).$(libUsageEnvironment_VERSION_REVISION)

libgroupsock_VERSION_CURRENT=10
libgroupsock_VERSION_REVISION=4
libgroupsock_VERSION_AGE=2
libgroupsock_LIB_SUFFIX=so.$(shell expr $(libgroupsock_VERSION_CURRENT) - $(libgroupsock_VERSION_AGE)).$(libgroupsock_VERSION_AGE).$(libgroupsock_VERSION_REVISION)

libliveMediaUtil_VERSION_CURRENT=1
libliveMediaUtil_VERSION_REVISION=0
libliveMediaUtil_VERSION_AGE=0
libliveMediaUtil_LIB_SUFFIX=so.$(shell expr $(libliveMediaUtil_VERSION_CURRENT) - $(libliveMediaUtil_VERSION_AGE)).$(libliveMediaUtil_VERSION_AGE).$(libliveMediaUtil_VERSION_REVISION)

##### 
DEBUG_OPTS = -DDEBUG_LOOPBACK_CHECKING=1 
DEBUG_OPTS += -DDEBUG=1 -DDEBUG_SEND=1 -DDEBUG_RECEIVE=1 -DDEBUG_CONTENTS=1 -DDEBUG_ERRORS -DDEBUG_RR 

COMPILE_OPTS = $(ZPLOS_CPPFLAGS) $(ZPLOS_DEFINE) $(ZPL_DEFINE) -g $(INCLUDES) -I/usr/local/include -I. -DSOCKLEN_T=socklen_t \
                -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DLOCALE_NOT_USED $(DEBUG_OPTS)

	
ifeq ($(strip $(LIVE555_SHARED_ENABLE)),true)
COMPILE_OPTS += -fPIC
endif			
             
ifneq ($(strip $(ZPL_OPENSSL_MODULE)),true)
COMPILE_OPTS += -DNO_OPENSSL=1
endif
C =			c
C_COMPILER =		$(CC)
C_FLAGS =		$(COMPILE_OPTS)
ifeq ($(strip $(LIVE555_SHARED_ENABLE)),true)
C_FLAGS +=		$(CPPFLAGS) $(CFLAGS)
endif
CPP =			cpp
CPLUSPLUS_COMPILER =	$(CXX)
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall -DBSD=1 
ifeq ($(strip $(LIVE555_SHARED_ENABLE)),true)
CPLUSPLUS_FLAGS += $(CPPFLAGS) $(CXXFLAGS)
endif
OBJ =			o
LINK =			$(CXX) -o
ifeq ($(strip $(LIVE555_SHARED_ENABLE)),true)
LINK_OPTS =		-L. $(LDFLAGS)
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		$(CC) -o 
SHORT_LIB_SUFFIX =	so.$(shell expr $($(NAME)_VERSION_CURRENT) - $($(NAME)_VERSION_AGE))
LIB_SUFFIX =	 	$(SHORT_LIB_SUFFIX).$($(NAME)_VERSION_AGE).$($(NAME)_VERSION_REVISION)
LIBRARY_LINK_OPTS =	-shared -Wl,-soname,$(NAME).$(SHORT_LIB_SUFFIX) $(LDFLAGS)
else
LINK_OPTS =		
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		$(AR) cr 
LIBRARY_LINK_OPTS =	$(LINK_OPTS)
LIB_SUFFIX =			a
endif
ifeq ($(strip $(ZPL_OPENSSL_MODULE)),true)
LIBS_FOR_CONSOLE_APPLICATION = -lssl -lcrypto
else
LIBS_FOR_CONSOLE_APPLICATION = 
endif
LIBS_FOR_GUI_APPLICATION =
EXE =
ifeq ($(strip $(LIVE555_SHARED_ENABLE)),true)
INSTALL2 =		install_shared_libraries
endif
##### End of variables to change

LIVEMEDIA_DIR = liveMedia
GROUPSOCK_DIR = groupsock
USAGE_ENVIRONMENT_DIR = UsageEnvironment
BASIC_USAGE_ENVIRONMENT_DIR = BasicUsageEnvironment
LIVEMEDIAUTIL_DIR = liveMediaUtil

TESTPROGS_DIR = testProgs

MEDIA_SERVER_DIR = mediaServer

PROXY_SERVER_DIR = proxyServer

HLS_PROXY_DIR = hlsProxy

all:
	cd $(LIVEMEDIA_DIR) ; $(MAKE)
	cd $(GROUPSOCK_DIR) ; $(MAKE)
	cd $(USAGE_ENVIRONMENT_DIR) ; $(MAKE)
	cd $(BASIC_USAGE_ENVIRONMENT_DIR) ; $(MAKE)
	cd $(LIVEMEDIAUTIL_DIR) ; $(MAKE)
	#cd $(TESTPROGS_DIR) ; $(MAKE)
	#cd $(MEDIA_SERVER_DIR) ; $(MAKE)
	#cd $(PROXY_SERVER_DIR) ; $(MAKE)
	#cd $(HLS_PROXY_DIR) ; $(MAKE)
	@echo
	@echo "For more information about this source code (including your obligations under the LGPL), please see our FAQ at http://live555.com/liveMedia/faq.html"

install:
	cd $(LIVEMEDIA_DIR) ; $(MAKE) install
	cd $(GROUPSOCK_DIR) ; $(MAKE) install
	cd $(USAGE_ENVIRONMENT_DIR) ; $(MAKE) install
	cd $(BASIC_USAGE_ENVIRONMENT_DIR) ; $(MAKE) install
	cd $(LIVEMEDIAUTIL_DIR) ; $(MAKE) install
	#cd $(TESTPROGS_DIR) ; $(MAKE) install
	#cd $(MEDIA_SERVER_DIR) ; $(MAKE) install
	#cd $(PROXY_SERVER_DIR) ; $(MAKE) install
	#cd $(HLS_PROXY_DIR) ; $(MAKE) install

clean:
	cd $(LIVEMEDIA_DIR) ; $(MAKE) clean
	cd $(GROUPSOCK_DIR) ; $(MAKE) clean
	cd $(USAGE_ENVIRONMENT_DIR) ; $(MAKE) clean
	cd $(BASIC_USAGE_ENVIRONMENT_DIR) ; $(MAKE) clean
	cd $(LIVEMEDIAUTIL_DIR) ; $(MAKE) clean
	#cd $(TESTPROGS_DIR) ; $(MAKE) clean
	#cd $(MEDIA_SERVER_DIR) ; $(MAKE) clean
	#cd $(PROXY_SERVER_DIR) ; $(MAKE) clean
	#cd $(HLS_PROXY_DIR) ; $(MAKE) clean

distclean: clean
	-rm -f $(LIVEMEDIA_DIR)/Makefile $(GROUPSOCK_DIR)/Makefile \
	  $(USAGE_ENVIRONMENT_DIR)/Makefile $(BASIC_USAGE_ENVIRONMENT_DIR)/Makefile \
	  $(LIVEMEDIAUTIL_DIR)/Makefile \
	  $(TESTPROGS_DIR)/Makefile $(MEDIA_SERVER_DIR)/Makefile \
	  $(PROXY_SERVER_DIR)/Makefile \
	  $(HLS_PROXY_DIR)/Makefile \
	  Makefile
