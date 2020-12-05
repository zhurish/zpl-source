MAIN_MAKEFILE=1
include $(MAKE_DIR)/makelinux.mk
include ffbuild/config-module.mk

vpath %.c    $(SRC_PATH)
vpath %.cpp  $(SRC_PATH)
vpath %.h    $(SRC_PATH)
vpath %.inc  $(SRC_PATH)
vpath %.m    $(SRC_PATH)
vpath %.S    $(SRC_PATH)
vpath %.asm  $(SRC_PATH)
vpath %.rc   $(SRC_PATH)
vpath %.v    $(SRC_PATH)
vpath %.texi $(SRC_PATH)
vpath %.cu   $(SRC_PATH)
vpath %.ptx  $(SRC_PATH)
vpath %/fate_config.sh.template $(SRC_PATH)

##TESTTOOLS   = audiogen videogen rotozoom tiny_psnr tiny_ssim base64 audiomatch
##HOSTPROGS  := $(TESTTOOLS:%=tests/%) doc/print_options

# $(FFLIBS-yes) needs to be in linking order
FFLIBS-$(CONFIG_AVDEVICE)   += avdevice
FFLIBS-$(CONFIG_AVFILTER)   += avfilter
FFLIBS-$(CONFIG_AVFORMAT)   += avformat
FFLIBS-$(CONFIG_AVCODEC)    += avcodec
FFLIBS-$(CONFIG_AVRESAMPLE) += avresample
FFLIBS-$(CONFIG_POSTPROC)   += postproc
FFLIBS-$(CONFIG_SWRESAMPLE) += swresample
FFLIBS-$(CONFIG_SWSCALE)    += swscale

FFLIBS := avutil


SKIPHEADERS = compat/w32pthreads.h

# first so "all" becomes default target
#include configdef.mk
#defconfig:
#	./configure $()
all: all-yes

#include $(SRC_PATH)/tools/Makefile
include $(SRC_PATH)/ffbuild/common.mak

FF_EXTRALIBS := $(FFEXTRALIBS)
FF_DEP_LIBS  := $(DEP_LIBS)
FF_STATIC_DEP_LIBS := $(STATIC_DEP_LIBS)

CONFIGURABLE_COMPONENTS =                                           \
    $(wildcard $(FFLIBS:%=$(SRC_PATH)/lib%/all*.c))                 \
    $(SRC_PATH)/libavcodec/bitstream_filters.c                      \
    $(SRC_PATH)/libavcodec/parsers.c                                \
    $(SRC_PATH)/libavformat/protocols.c                             \

config.h: ffbuild/.config
ffbuild/.config: $(CONFIGURABLE_COMPONENTS)
	@-tput bold 2>/dev/null
	@-printf '\nWARNING: $(?) newer than config.h, rerun configure\n\n'
	@-tput sgr0 2>/dev/null
#
#SUBDIR_VARS := CLEANFILES FFLIBS HOSTPROGS TESTPROGS    TOOLS            \
               HEADERS ARCH_HEADERS BUILT_HEADERS SKIPHEADERS            \
               ARMV5TE-OBJS ARMV6-OBJS ARMV8-OBJS VFP-OBJS NEON-OBJS     \
               ALTIVEC-OBJS VSX-OBJS MMX-OBJS X86ASM-OBJS                \
               MIPSFPU-OBJS MIPSDSPR2-OBJS MIPSDSP-OBJS MSA-OBJS         \
               MMI-OBJS OBJS SLIBOBJS HOSTOBJS TESTOBJS

SUBDIR_VARS := CLEANFILES FFLIBS HEADERS ARCH_HEADERS BUILT_HEADERS SKIPHEADERS            \
               ARMV5TE-OBJS ARMV6-OBJS ARMV8-OBJS VFP-OBJS NEON-OBJS     \
               ALTIVEC-OBJS VSX-OBJS MMX-OBJS X86ASM-OBJS                \
               MIPSFPU-OBJS MIPSDSPR2-OBJS MIPSDSP-OBJS MSA-OBJS         \
               MMI-OBJS OBJS SLIBOBJS  

define RESET
$(1) :=
$(1)-yes :=
endef

define DOSUBDIR
$(foreach V,$(SUBDIR_VARS),$(eval $(call RESET,$(V))))
SUBDIR := $(1)/
include $(SRC_PATH)/$(1)/Makefile
-include $(SRC_PATH)/$(1)/$(ARCH)/Makefile
-include $(SRC_PATH)/$(1)/$(INTRINSICS)/Makefile
include $(SRC_PATH)/ffbuild/library.mak
endef

$(foreach D,$(FFLIBS),$(eval $(call DOSUBDIR,lib$(D))))

include $(SRC_PATH)/fftools/Makefile

libavcodec/utils.o libavformat/utils.o libavdevice/avdevice.o libavfilter/avfilter.o libavutil/utils.o libpostproc/postprocess.o libswresample/swresample.o libswscale/utils.o : libavutil/ffversion.h

$(PROGS): %$(PROGSSUF)$(EXESUF): %$(PROGSSUF)_g$(EXESUF)
ifeq ($(STRIPTYPE),direct)
	$(STRIP) -o $@ $<
else
	$(CP) $< $@
	$(STRIP) $@
endif

%$(PROGSSUF)_g$(EXESUF): $(FF_DEP_LIBS)
	$(LD) $(LDFLAGS) $(LDEXEFLAGS) $(LD_O) $(OBJS-$*) $(FF_EXTRALIBS)

VERSION_SH  = $(SRC_PATH)/ffbuild/version.sh
GIT_LOG     = $(SRC_PATH)/.git/logs/HEAD

.version: $(wildcard $(GIT_LOG)) $(VERSION_SH) ffbuild/config-module.mk
.version: M=@

libavutil/ffversion.h .version:
	$(M)$(VERSION_SH) $(SRC_PATH) libavutil/ffversion.h $(EXTRA_VERSION)
	$(Q)touch .version

# force version.sh to run whenever version might have changed
-include .version

install: install-libs install-headers

install-libs: install-libs-yes


uninstall: uninstall-headers uninstall-libs uninstall-pkgconfig

clean::
	-$(RM) $(CLEANSUFFIXES)
	-$(RM) $(addprefix compat/,$(CLEANSUFFIXES)) $(addprefix compat/*/,$(CLEANSUFFIXES)) $(addprefix compat/*/*/,$(CLEANSUFFIXES))
	-$(RM) -r coverage-html
	-$(RM) -rf coverage.info coverage.info.in lcov

distclean:: clean
	-$(RM) .version avversion.h config.asm config.h mapfile  \
		ffbuild/.config ffbuild/config.* libavutil/avconfig.h \
		version.h libavutil/ffversion.h libavcodec/codec_names.h \
		libavcodec/bsf_list.c libavformat/protocol_list.c \
		libavcodec/codec_list.c libavcodec/parser_list.c \
		libavfilter/filter_list.c libavdevice/indev_list.c libavdevice/outdev_list.c \
		libavformat/muxer_list.c libavformat/demuxer_list.c
ifeq ($(SRC_LINK),src)
	-$(RM) src
endif

config:
	$(SRC_PATH)/configure $(value FFMPEG_CONFIGURATION)

#build: all alltools examples testprogs
#check: all alltools examples testprogs fate

build: all
check: all


$(sort $(OUTDIRS)):
	$(Q)mkdir -p $@

# Dummy rule to stop make trying to rebuild removed or renamed headers
%.h:
	@:

# Disable suffix rules.  Most of the builtin rules are suffix rules,
# so this saves some time on slow systems.
.SUFFIXES:

.PHONY: all all-yes alltools build check config 
.PHONY: *clean install* uninstall*
