##############################################
# OpenWrt Makefile for HelloWorld program
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to
# specify a basic description of the package,
# where to build our program, where to find
# the source files, and where to install the
# compiled program on the router.
#
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and
# there should be no trailing whitespace in
# lines that are not commented.
#
##############################################
include $(TOPDIR)/rules.mk
# Name and release number of this package
PKG_NAME:=X5-B
PKG_RELEASE:=0.0.0.11
# This specifies the directory where we're going to build the program. 
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
#
#include $(INCLUDE_DIR)/uclibc++.mk
include $(INCLUDE_DIR)/package.mk
# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/X5-B
	SECTION:=Application
	CATEGORY:=Extra packages
	TITLE:=X5-B -- X5-B Application
	DEPENDS:=+alsa-lib +libopenssl +zlib +libuuid +libuci +libstdcpp
	#+libstdc++
endef
# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/X5-B/description
	If you can't figure out what this program does, you're probably brain-dead and need immediate medical attention.
endef
# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
#
# rsync -av --exclude data/a --exclude data/b --exclude data/c data /bak
#
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cd src;./appversion.sh
	cd src/SWPlatform/make;./version.sh
#	$(CP) -arf ./src/* $(PKG_BUILD_DIR)/
#	$(CP) -R `find ./src -type d -path ./src/SWPlatform/.git -prune -o -print | sed 1d` $(PKG_BUILD_DIR)/
	rsync -av --exclude ./src/SWPlatform/.git --exclude ./src/SWPlatform/debug ./src/ $(PKG_BUILD_DIR)/
	$(RM) -rf $(PKG_BUILD_DIR)/SWPlatform/debug
	$(RM) -rf $(PKG_BUILD_DIR)/SWPlatform/.git
#	$(CP) -arf ./src/* $(PKG_BUILD_DIR)/
#	$(CP) -arf ./src/* $(PKG_BUILD_DIR)/			
endef
#
#TARGET_CFLAGS:=$(TARGET_OPTIMIZATION)$(if $(CONFIG_DEBUG), -g3) $(call qstrip,$(CONFIG_EXTRA_OPTIMIZATION))
#TARGET_CXXFLAGS = $(TARGET_CFLAGS)
#TARGET_ASFLAGS_DEFAULT = $(TARGET_CFLAGS)
#TARGET_ASFLAGS = $(TARGET_ASFLAGS_DEFAULT)
#TARGET_CPPFLAGS:=-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/include
#TARGET_LDFLAGS:=-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib
export TARGET_CFLAGS
export TARGET_CXXFLAGS
export TARGET_ASFLAGS
export TARGET_CPPFLAGS
export TARGET_LDFLAGS

export TOOLCHAIN_DIR
export BUILD_DIR_TOOLCHAIN
export TARGET_CROSS
export TARGET_CONFIGURE_OPTS

#export TARGET_LDFLAGS += -lssl -lcrypto -lasound

define Build/Compile
	$(MAKE) -C  $(PKG_BUILD_DIR) all 
	#$(TARGET_CONFIGURE_OPTS)
endef
#
# (可选)
#    软件安装之前被执行的脚本，别忘了在第一句加上#!/bin/sh，如果脚本执行完毕要取消 安装过程，直接让它返回false即可。
#
##define Package/$(PKG_NAME)/preinst
##!/bin/sh
##if [ ! -d "/home/app" ]; then
##	mkdir -p /home/app
##fi
##pkill -9 X5-B
##pkill -9 X5-B
##pkill -9 X5-B
##exit 0
##endef
#
#(可选)
#软件安装之后被执行的脚本，别忘了在第一句加上#!/bin/sh。
##define  Package/$(PKG_NAME)/postinst 
##!/bin/sh
##cd /home/app
##openssl enc -d -aes256 -k 123 -in X5-B | tar xz
##sync
##if [ -d "/home/app/lib" ]; then
##	cp -arf /home/app/lib /usr/lib/
##	rm -rf /home/app/lib
##fi
##sync
##exit 0
##endef
#

#
# 如下命令是对filename文件（test.txt）进行加密压缩，生成filename.des3加密压缩文件，123@123为加密的密码
# tar -zcf - test.txt |openssl des3 -salt -k 123@123 | dd of=test.txt.des3
#
# 解压操作
# dd if=test.txt.des3 |openssl des3 -d -k 123@123 | tar zxf -
#
# 现在要加密当前工作目录的内容（根据文件的大小，这可能需要一点时间）
# tar -czf - * | openssl enc -e -aes256 -out test.tar.gz
# 
# 要解密上述tar归档内容，需要使用以下命令
# openssl enc -d -aes256 -in test.tar.gz | tar xz -C /mnt/
#
#define Package/ $(PKG_NAME)/ prerm (可选)
#    软件删除之前被执行的脚本，别忘了在第一句加上#!/bin/sh。如果脚本执行完毕要取消 删除过程，直接让它返回false即可。
#    #!/bin/sh
#    .........
#    exit 0
#endif

#define  Package/ $(PKG_NAME)/ postrm (可选)
#    软件删除之后被执行的脚本，别忘了在第一句加上#!/bin/sh。
#    #!/bin/sh
#    .........
#    exit 0
#endif
# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one
# Specify where and how to install the program. Since we only have one file,
# the HelloWorld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/X5-B/install
	#$(MAKE) -C  $(PKG_BUILD_DIR) install
	
	$(MAKE) -C  $(PKG_BUILD_DIR) install INSTALLDSTDIR=$(1)
	
	##$(INSTALL_DIR) $(1)/usr/bin
	##$(INSTALL_BIN) $(PKG_BUILD_DIR)/appversion $(1)/usr/bin/
	##$(INSTALL_BIN) $(PKG_BUILD_DIR)/mac-tools $(1)/usr/bin/
	##$(INSTALL_BIN) $(PKG_BUILD_DIR)/lua-sync $(1)/usr/bin/
	#
	# Lib  mediastream need
	#
	##$(INSTALL_DIR) $(1)/lib
	#$(CP) $(PKG_BUILD_DIR)/lib/libstd* $(1)/lib/
	#$(CP) $(PKG_BUILD_DIR)/lib/libz* $(1)/lib/
	
	##$(INSTALL_DIR) $(1)/usr/lib
	#$(CP) $(PKG_BUILD_DIR)/lib/libbc* $(1)/usr/lib/
	#$(CP) $(PKG_BUILD_DIR)/lib/libmb* $(1)/usr/lib/	
	#$(CP) $(PKG_BUILD_DIR)/lib/libme* $(1)/usr/lib/
	#$(CP) $(PKG_BUILD_DIR)/lib/libortp* $(1)/usr/lib/	
	#$(CP) $(PKG_BUILD_DIR)/lib/libspeex* $(1)/usr/lib/	
	#$(CP) $(PKG_BUILD_DIR)/lib/libvos.so $(1)/usr/lib/	
	##$(CP) $(PKG_BUILD_DIR)/SWPlatform/debug/usr/lib/lib*.so $(1)/usr/lib/
	##$(CP) $(PKG_BUILD_DIR)/SWPlatform/debug/usr/lib/lib*.so* $(1)/usr/lib/
	#
	# factory WIFI
	#
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_BUILD_DIR)/sysconfig/7688-mtk-0816.bin $(1)/usr/share/
	#
	# default config
	#
	#$(INSTALL_DIR) $(1)/etc
	#$(CP) $(PKG_BUILD_DIR)/sysconfig/rc.local $(1)/etc/
	#$(CP) $(PKG_BUILD_DIR)/sysconfig/shadow $(1)/etc/
	#$(CP) $(PKG_BUILD_DIR)/sysconfig/fw_env.config $(1)/etc/
	#$(CP) $(PKG_BUILD_DIR)/sysconfig/vsftpd.conf $(1)/etc/
	#network	
	#$(INSTALL_DIR) $(1)/etc/config
	#$(CP) $(PKG_BUILD_DIR)/sysconfig/dropbear $(1)/etc/config/
	#$(CP) $(PKG_BUILD_DIR)/sysconfig/network $(1)/etc/config/
	#$(CP) $(PKG_BUILD_DIR)/sysconfig/uhttpd $(1)/etc/config/
	#$(CP) $(PKG_BUILD_DIR)/sysconfig/wireless $(1)/etc/config/
	#ssh
	#$(INSTALL_DIR) $(1)/etc/ssh
	#$(CP) $(PKG_BUILD_DIR)/sysconfig/ssh/* $(1)/etc/ssh/
	
	
	##$(INSTALL_DIR) $(1)/app
	##$(INSTALL_BIN) $(PKG_BUILD_DIR)/SWPlatform/debug/bin/SWP*.bin $(1)/app/
	
	##$(INSTALL_DIR) $(1)/app/etc
	##$(CP) $(PKG_BUILD_DIR)/SWPlatform/debug/etc/* $(1)/app/etc/
	##$(CP) $(PKG_BUILD_DIR)/default/ringback.wav $(1)/app/etc/
	##$(CP) $(PKG_BUILD_DIR)/default/sip.cfg $(1)/app/etc/
	##$(CP) $(PKG_BUILD_DIR)/default/x5b-app.sh $(1)/app/etc/
	##$(CP) $(PKG_BUILD_DIR)/default/phonedbtest.cfg $(1)/app/etc/
	##$(CP) $(PKG_BUILD_DIR)/default/update-app.sh $(1)/app/etc/
	##$(CP) $(PKG_BUILD_DIR)/default/remote-test.txt $(1)/app/etc/
	##$(CP) $(PKG_BUILD_DIR)/default/volume_setup.sh $(1)/app/etc/
	
	#
	# Voip lua/luci
	#
	#$(INSTALL_DIR) $(1)/usr/lib/lua
	#mkdir -p $(1)/usr/lib/lua/voip
	#$(INSTALL_DIR) $(1)/usr/lib/lua/voip
	##$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller/admin
	##$(CP) $(PKG_BUILD_DIR)/voip_luci/voip.lua $(1)/usr/lib/lua/luci/controller/admin/
	##$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/admin_voip
	#mkdir -p $(1)/usr/lib/lua/luci/model/cbi/admin_voip
	##$(CP) $(PKG_BUILD_DIR)/voip_luci/sipconfig.lua $(1)/usr/lib/lua/luci/model/cbi/admin_voip/
	##$(CP) $(PKG_BUILD_DIR)/voip_luci/sip_config.cfg $(1)/app/etc/sip_config.cfg
	
	##$(INSTALL_DIR) $(1)/etc/config
	##$(CP) $(PKG_BUILD_DIR)/voip_luci/sip_config.cfg $(1)/etc/config/sip_config
	#$(LN) /app/etc/sip_config.cfg $(1)/etc/config/sip_config
	
	##$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/admin_voip
	##$(CP) $(PKG_BUILD_DIR)/voip_luci/sip_status.htm $(1)/usr/lib/lua/luci/view/admin_voip/
endef
# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,X5-B))
