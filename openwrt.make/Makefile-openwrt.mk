# build helloworld executable when user executes "make"
#LDFLAGS	+= -lasound
#
#LDFLAGS += -L/home/zhurish/application/openwrt-lede/package/X5-B/src -lasound -lpthread
#LDFLAGS += -L/home/zhurish/application/openwrt-lede/package/X5-B/src -lasound -lpthread
#
#
#
#TARGET_CFLAGS:=$(TARGET_OPTIMIZATION)$(if $(CONFIG_DEBUG), -g3) $(call qstrip,$(CONFIG_EXTRA_OPTIMIZATION))
#TARGET_CXXFLAGS = $(TARGET_CFLAGS)
#TARGET_ASFLAGS_DEFAULT = $(TARGET_CFLAGS)
#TARGET_ASFLAGS = $(TARGET_ASFLAGS_DEFAULT)
#TARGET_CPPFLAGS:=-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include
#TARGET_LDFLAGS:=-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib
#
#TOOLCHAIN_DIR=/home/zhurish/application/openwrt-lede/staging_dir/toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24"
#TARGET_ROOTFS_DIR=/home/zhurish/application/openwrt-lede/build_dir/target-mipsel_24kc_glibc-2.24
#TOOLCHAIN_DIR_NAME=toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24
#BUILD_DIR_TOOLCHAIN=/home/zhurish/application/openwrt-lede/build_dir/toolchain-mipsel_24kc_gcc-5.4.0_glibc-2.24
#TARGET_CROSS=mipsel-openwrt-linux-gnu-
#TARGET_CONFIGURE_OPTS=AR=mipsel-openwrt-linux-gnu-gcc-ar AS=mipsel-openwrt-linux-gnu-gcc -c -Os -pipe -mno-branch-likely -mips32r2 -mtune=24kc -fno-caller-saves -fno-plt -fhonour-copts -Wno-error=unused-but-set-variable -Wno-error=unused-result -msoft-float -iremap/home/zhurish/application/openwrt-lede/build_dir/target-mipsel_24kc_glibc-2.24/X5-B:X5-B -Wformat -Werror=format-security -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro LD=mipsel-openwrt-linux-gnu-ld NM=mipsel-openwrt-linux-gnu-gcc-nm CC=mipsel-openwrt-linux-gnu-gcc GCC=mipsel-openwrt-linux-gnu-gcc CXX=mipsel-openwrt-linux-gnu-g++ RANLIB=mipsel-openwrt-linux-gnu-gcc-ranlib STRIP=mipsel-openwrt-linux-gnu-strip OBJCOPY=mipsel-openwrt-linux-gnu-objcopy OBJDUMP=mipsel-openwrt-linux-gnu-objdump SIZE=mipsel-openwrt-linux-gnu-size

#CROSS_COMPILE_ROOT = $(TOOLCHAIN_DIR)
#CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/$(TARGET_CROSS)

#CROSS_COMPILE_ROOT = /opt/toolchain/toolchain-mipsel_24kc_gcc-7.3.0_glibc
#CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/mipsel-openwrt-linux-
#
#
#
CD = cd
RM = rm
MV = mv
CP = cp
MKDIR = mkdir -p
INSTALL = install -m 755
#	install -m 755 ${TAGET} ${BINDIR}
#	$(STRIP) $(BINDIR)/$(TAGET) 
#	install -d ${DSTETCDIR} 
#
#
export APP_TOP_DIR =$(shell pwd)
#
#
#export ARCH_TYPE=MIPS
export ARCH_TYPE=MIPS
export OPENWRT=true
ifneq ($(TOOLCHAIN_DIR),)
export CROSS_COMPILE_ROOT = $(TOOLCHAIN_DIR)
export CROSS_COMPILE = $(CROSS_COMPILE_ROOT)/bin/$(TARGET_CROSS)
endif
#
#
ifneq ($(CROSS_COMPILE),)
#$(error CROSS_COMPILE is not define)
#endif
AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++
CPP=$(CC) -E
AR=$(CROSS_COMPILE)ar
NM=$(CROSS_COMPILE)nm
LDR=$(TARGET_CROSS)ldr
STRIP=$(CROSS_COMPILE)strip
OBJCOPY=$(CROSS_COMPILE)objcopy
OBJDUMP=$(CROSS_COMPILE)objdump
RANLIB=$(CROSS_COMPILE)ranlib
endif
#
#
ifneq ($(INSTALLDSTDIR),)
INSTALLDESTDIR=$(INSTALLDSTDIR)
else
INSTALLDESTDIR=$(shell pwd)
endif
#
#
#
#
#X5-B: hello mac-tools
#
.PHONY:	all clean app install
#
all: x5app appversion mac-tools lua-sync
#
#
#
#
#

app:
	$(MAKE) -C SWPlatform app ARCH_TYPE=MIPS OPENWRT=true GCC_TYPE=GCLIBC
	
appversion: appversion.o
	$(CC) -g $(TARGET_LDFLAGS) appversion.o -o appversion
		
mac-tools: mac-tools.o
	$(CC) -g $(TARGET_LDFLAGS) mac-tools.o -o mac-tools
	
lua-sync: lua-sync.o
	$(CC) -g $(TARGET_LDFLAGS) lua-sync.o -o lua-sync
		
appversion.o: appversion.c
	$(CC) -g $(TARGET_CFLAGS) -c appversion.c
	
mac-tools.o: mac-tools.c
	$(CC) -g $(TARGET_CFLAGS) -c mac-tools.c
	
lua-sync.o: lua-sync.c
	$(CC) -g $(TARGET_CFLAGS) -c lua-sync.c
		
install:
	$(MAKE) -C SWPlatform install
	$(MAKE) -C SWPlatform/web-lua install INSTALLDSTWEB=$(INSTALLDESTDIR)
	
	$(MKDIR) $(INSTALLDESTDIR)/usr/bin
	$(INSTALL) $(APP_TOP_DIR)/appversion $(INSTALLDESTDIR)/usr/bin/
	$(INSTALL) $(APP_TOP_DIR)/mac-tools $(INSTALLDESTDIR)/usr/bin/
	$(INSTALL) $(APP_TOP_DIR)/lua-sync $(INSTALLDESTDIR)/usr/bin/
	
	$(MKDIR) $(INSTALLDESTDIR)/usr/lib
	$(CP) -arf $(APP_TOP_DIR)/SWPlatform/debug/usr/lib/lib*.so* $(INSTALLDESTDIR)/usr/lib/
	$(CP) -arf $(APP_TOP_DIR)/SWPlatform/debug/usr/lib/lib*.so $(INSTALLDESTDIR)/usr/lib/
	
	$(MKDIR) $(INSTALLDESTDIR)/app
	$(INSTALL) $(APP_TOP_DIR)/SWPlatform/debug/bin/SWP*.bin $(INSTALLDESTDIR)/app/
	
	$(MKDIR) $(INSTALLDESTDIR)/app/etc
	$(CP) $(APP_TOP_DIR)/SWPlatform/debug/etc/* $(INSTALLDESTDIR)/app/etc/
	$(CP) $(APP_TOP_DIR)/default/ringback.wav $(INSTALLDESTDIR)/app/etc/
	$(INSTALL) $(APP_TOP_DIR)/default/x5b-app.sh $(INSTALLDESTDIR)/app/etc/

# remove object files and executable when user executes "make clean"
clean:
	cd SWPlatform && make clean
	rm *.o appversion mac-tools
