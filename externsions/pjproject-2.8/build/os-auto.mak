# build/os-auto.mak.  Generated from os-auto.mak.in by configure.

export OS_CFLAGS   := $(CC_DEF)PJ_AUTOCONF=1 -O0 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1 $(PLOS_CFLAGS) $(PL_DEFINE) $(PLOS_INCLUDE) $(PL_INCLUDE) -fPIC

export OS_CXXFLAGS := $(CC_DEF)PJ_AUTOCONF=1 -g -O0
ifeq ($(PJMEDIA_SRTP_ENABLE),true)
export OS_LDFLAGS  :=  -lm -lrt -lpthread -lssl -lcrypto -lasound
else
export OS_LDFLAGS  :=  -lm -lrt -lpthread -lasound
endif

export OS_SOURCES  := 


