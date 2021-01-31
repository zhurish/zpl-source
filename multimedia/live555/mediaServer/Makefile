PREFIX ?= /usr/local
INCLUDES = -I../UsageEnvironment/include -I../groupsock/include -I../liveMedia/include -I../BasicUsageEnvironment/include
# Default library filename suffixes for each library that we link with.  The "config.*" file might redefine these later.
libliveMedia_LIB_SUFFIX = $(LIB_SUFFIX)
libBasicUsageEnvironment_LIB_SUFFIX = $(LIB_SUFFIX)
libUsageEnvironment_LIB_SUFFIX = $(LIB_SUFFIX)
libgroupsock_LIB_SUFFIX = $(LIB_SUFFIX)
##### Change the following for your environment:
include $(MAKE_DIR)/makelinux.mk
export PREFIX = $(DSTROOTFSDIR)
export LIBDIR = $(PREFIX)/lib
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
#####

DEBUG_OPTS = -DDEBUG=1 -DDEBUG_LOOPBACK_CHECKING=1 -DDEBUG_SEND=1 -DDEBUG_RECEIVE=1 -DDEBUG_CONTENTS=1 -DDEBUG_ERRORS -DDEBUG_RR

COMPILE_OPTS =	$(INCLUDES) -I/usr/local/include -I. -O2 -DSOCKLEN_T=socklen_t \
                -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 $(DEBUG_OPTS)

	
ifeq ($(strip $(LIVE555_SHARED_ENABLE)),true)
COMPILE_OPTS += -fPIC
endif			
             
ifneq ($(strip $(PL_OPENSSL_MODULE)),true)
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
ifeq ($(strip $(PL_OPENSSL_MODULE)),true)
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

MEDIA_SERVER = live555MediaServer$(EXE)

ALL = $(MEDIA_SERVER)
all: $(ALL)

.$(C).$(OBJ):
	$(C_COMPILER) -c $(C_FLAGS) $<
.$(CPP).$(OBJ):
	$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

MEDIA_SERVER_OBJS = live555MediaServer.$(OBJ) DynamicRTSPServer.$(OBJ)

live555MediaServer.$(CPP):	DynamicRTSPServer.hh version.hh
DynamicRTSPServer.$(CPP):	DynamicRTSPServer.hh

USAGE_ENVIRONMENT_DIR = ../UsageEnvironment
USAGE_ENVIRONMENT_LIB = $(USAGE_ENVIRONMENT_DIR)/libUsageEnvironment.$(libUsageEnvironment_LIB_SUFFIX)
BASIC_USAGE_ENVIRONMENT_DIR = ../BasicUsageEnvironment
BASIC_USAGE_ENVIRONMENT_LIB = $(BASIC_USAGE_ENVIRONMENT_DIR)/libBasicUsageEnvironment.$(libBasicUsageEnvironment_LIB_SUFFIX)
LIVEMEDIA_DIR = ../liveMedia
LIVEMEDIA_LIB = $(LIVEMEDIA_DIR)/libliveMedia.$(libliveMedia_LIB_SUFFIX)
GROUPSOCK_DIR = ../groupsock
GROUPSOCK_LIB = $(GROUPSOCK_DIR)/libgroupsock.$(libgroupsock_LIB_SUFFIX)
LOCAL_LIBS =	$(LIVEMEDIA_LIB) $(GROUPSOCK_LIB) \
		$(BASIC_USAGE_ENVIRONMENT_LIB) $(USAGE_ENVIRONMENT_LIB)
LIBS =			$(LOCAL_LIBS) $(LIBS_FOR_CONSOLE_APPLICATION)

live555MediaServer$(EXE):	$(MEDIA_SERVER_OBJS) $(LOCAL_LIBS)
	$(LINK)$@ $(CONSOLE_LINK_OPTS) $(MEDIA_SERVER_OBJS) $(LIBS)

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

install: $(MEDIA_SERVER)
	  install -d $(DESTDIR)$(PREFIX)/bin
	  install -m 755 $(MEDIA_SERVER) $(DESTDIR)$(PREFIX)/bin

##### Any additional, platform-specific rules come here:
