# *DOCUMENTATION*
# To see a list of typical targets execute "make help"
# More info can be located in ./README
# Comments in this file are targeted only to the developer, do not
# expect to learn how to build the kernel reading this file.


# If building an external module we do not care about the all: rule
# but instead _all depend on modules
PHONY += all

_all: all


srctree		:= $(CURDIR)
objtree		:= $(CURDIR)

export srctree objtree


# SHELL used by kbuild
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

# 	Decide whether to build built-in, modular, or both.
#	Normally, just do built-in.


# Beautify output
# ---------------------------------------------------------------------------
# A simple variant is to prefix commands with $(Q) - that's useful
# for commands that shall be hidden in non-verbose mode.
#
#	$(Q)ln $@ :<
#
# If KBUILD_VERBOSE equals 0 then the above command will be hidden.
# If KBUILD_VERBOSE equals 1 then the above command is displayed.

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

# If the user is running make -s (silent mode), suppress echoing of
# commands


export quiet Q KBUILD_VERBOSE


HOSTCC  	= gcc
HOSTCXX  	= g++
HOSTCFLAGS	:=
HOSTCXXFLAGS	:=
# We need some generic definitions
include $(srctree)/scripts/Kbuild.include

HOSTCFLAGS	+= $(call hostcc-option,-Wall -Wstrict-prototypes -O2 -fomit-frame-pointer,)
HOSTCXXFLAGS	+= -O2

# For maximum performance (+ possibly random breakage, uncomment
# the following)

CHECKFLAGS     := -D__linux__ -Dlinux -D__STDC__ -Dunix -D__unix__ -Wbitwise $(CF)

export CONFIG_SHELL HOSTCC HOSTCFLAGS HOSTCXX HOSTCXXFLAGS CHECKFLAGS


# Files to ignore in find ... statements

RCS_FIND_IGNORE := \( -name SCCS -o -name BitKeeper -o -name .svn -o -name CVS -o -name .pc -o -name .hg -o -name .git -name app \) -prune -o
export RCS_TAR_IGNORE := --exclude SCCS --exclude BitKeeper --exclude .svn --exclude CVS --exclude .pc --exclude .hg --exclude .git

# ===========================================================================
# ===========================================================================
# *config targets only - make sure prerequisites are updated, and descend
# in scripts/kconfig to make the *config target

export SOURCE_ROOT=$(srctree)

#-include $(srctree)/.config
-include $(srctree)/Makefile.help
-include $(srctree)/build/board.mk


export MENUCONFIG_ZPL_BUILD=true
export MENUCONFIG_ZPL_MODULE=$(srctree)/build/module-def.mk
export MENUCONFIG_ZPL_CONFIG=$(srctree)/build/board.mk

menuconfig: 
	$(Q)mkdir -p include
	$(Q)$(MAKE) $(build)=scripts/kconfig $@

config: 
	$(Q)mkdir -p include
	$(Q)$(MAKE) $(build)=scripts/kconfig $@

%config: 
	$(Q)mkdir -p include
	$(Q)$(MAKE) $(build)=scripts/kconfig $@
#

def_make_prepare:
	if test -e ".config"; \
	then \
		cp .config build/module-def.mk; \
		sed -i 's/^CONFIG_ZPL_/ZPL_/g' build/module-def.mk; \
		sed -i 's/=y$$/=true/g' build/module-def.mk; \
		cp include/autoconf.h source/include/plautoconf.h; \
		sed -i 's/CONFIG_ZPL_/ZPL_/g' source/include/plautoconf.h; \
	fi 
#	
#diff build/aa build/module-def.mk >/dev/null 2>&1 || break \
#	echo "$? $@"
#	tmp=$? \
#	if test ${tmp} -nq 0; \
#	then \
#		mv build/module-deftmp.mk build/module-def.mk; \
#		mv source/include/plautoconftmp.h source/include/plautoconf.h; \
#	fi
#
# The all: target is the default when no target is given on the
# command line.
# This allow a user to issue only 'make' to build a kernel including modules
# Defaults Platform but it is usually overridden in the arch makefile
#all: Platform


prebuilts: def_make_prepare
	${MAKE} -C  source/ $@ 

make_prepare: def_make_prepare
	${MAKE} -C  source/ $@ 
	@if test  -f source/platform/moduletable.c ; \
	then \
		echo "-----------------------------------"; \
		mv source/platform/moduletable.c source/startup/src/ ; \
	fi

#def_make_prepare make_prepare
all: 
	@if ! test  -e source/include/plautoconf.h ; \
	then \
		make make_prepare ; \
	fi
	${MAKE} -C  source/ $@ 

lib:  
	${MAKE} -C  source/ $@ 

obj: 
	${MAKE} -C  source/ $@ 	
	

exlib:
	${MAKE} -C  source/ $@ 


exlib_clean:
	${MAKE} -C  source/ $@  

app: 
	@if ! test  -e source/include/plautoconf.h ; \
	then \
		make make_prepare ; \
	fi
	${MAKE} -C  source/ $@ 	

kernel_module:
	${MAKE} -C  source/ $@ 			
###
# Cleaning is done on three levels.
# make clean     Delete most generated files
#                Leave enough to build external modules
# make mrproper  Delete the current configuration, and all generated files
# make distclean Remove editor backup files, patch leftover files and the like

# Directories & files removed with 'make mrproper'
MRPROPER_DIRS  += include/config
MRPROPER_FILES += .config .config.old .version .old_version \
		  .kconfig.d include/autoconf.h 

# clean - Delete most, but leave enough to build external modules
#
PHONY += clean menuconfig
clean: 
	rm source/include/plautoconf.h
	${MAKE} -C  source/ clean
#
#@find . $(RCS_FIND_IGNORE) \
		\( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \) \
		-type f -print | xargs rm -f


# mrproper - Delete all generated files, including .config
#
PHONY += mrproper 

mrproper:  
	rm -rf $(MRPROPER_DIRS)
	rm -rf $(MRPROPER_FILES)

# distclean
#
PHONY += distclean

distclean: mrproper
	${MAKE} -C  source/ clean
	@find $(srctree) $(RCS_FIND_IGNORE) \
		\( -name '*.orig' -o -name '*.rej' -o -name '*~' \
		-o -name '*.bak' -o -name '#*#' -o -name '.*.orig' \
		-o -name '.*.rej' -o -name '*.tmp' -o -size 0 \
		-o -name '*%' -o -name '.*.cmd' -o -name 'core' \) \
		-type f -print | xargs rm -f



install:
	mkdir -p $(ZPL_INSTALL_PATH)
	cp -arf source/debug/usr/lib $(ZPL_INSTALL_PATH)/
	cp -arf source/debug/etc $(ZPL_INSTALL_PATH)/
	cp -arf source/debug/www $(ZPL_INSTALL_PATH)/
	cp -arf source/debug/sbin $(ZPL_INSTALL_PATH)/
	cp -arf source/debug/bin $(ZPL_INSTALL_PATH)/

#help:
#	echo "make all make_prepare prebuilts"
# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable se we can use it in if_changed and friends.
.PHONY: $(PHONY) all lib obj app exlib exlib_clean install prebuilts make_prepare help kernel_module
