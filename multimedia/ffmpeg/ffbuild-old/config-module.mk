# Automatically generated by configure - do not modify!
ifndef FFMPEG_CONFIG_MAK
FFMPEG_CONFIG_MAK=1
FFMPEG_CONFIGURATION=--prefix=$(MULTIMEDIA_ROOT)/_install --enable-libx264 --enable-gpl --enable-ffplay
prefix=$(MULTIMEDIA_ROOT)/_install
LIBDIR=$(DESTDIR)${prefix}/lib
SHLIBDIR=$(DESTDIR)${prefix}/lib
INCDIR=$(DESTDIR)${prefix}/include
BINDIR=$(DESTDIR)${prefix}/bin
DATADIR=$(DESTDIR)${prefix}/share/ffmpeg
DOCDIR=$(DESTDIR)${prefix}/share/doc/ffmpeg
MANDIR=$(DESTDIR)${prefix}/share/man
PKGCONFIGDIR=$(DESTDIR)${prefix}/lib/pkgconfig
INSTALL_NAME_DIR=
SRC_PATH=.
SRC_LINK=.
ifndef MAIN_MAKEFILE
SRC_PATH:=$(SRC_PATH:.%=..%)
endif
CC_IDENT=gcc 8 (GCC)
ARCH=x86
INTRINSICS=none
EXTERN_PREFIX=
#CC=gcc
#CXX=g++
AS=$(CC)
OBJCC=$(CC)
LD=$(CC)
DEPCC=$(CC)
DEPCCFLAGS= $(CPPFLAGS)
DEPAS=$(CC)
DEPASFLAGS= $(CPPFLAGS)
X86ASM=nasm
DEPX86ASM=nasm
DEPX86ASMFLAGS=$(X86ASMFLAGS)
#AR=$(AR)
ARFLAGS=rcD
AR_O=$@
AR_CMD=$(AR)
NM_CMD=$(NM) -g
#RANLIB=$(RANLIB) -D
#STRIP=$(STRIP)
STRIPTYPE=direct
NVCC=clang
CP=cp -p
LN_S=ln -s -f
CPPFLAGS= -D_ISOC99_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -DZLIB_CONST

CFLAGS=   -std=c11 -fomit-frame-pointer -pthread -g -Wdeclaration-after-statement -Wall \
        -Wdisabled-optimization -Wpointer-arith -Wredundant-decls -Wwrite-strings \
        -Wtype-limits -Wundef -Wmissing-prototypes -Wno-pointer-to-int-cast -Wstrict-prototypes \
        -Wempty-body -Wno-parentheses -Wno-switch -Wno-format-zero-length -Wno-pointer-sign \
        -Wno-unused-const-variable -Wno-bool-operation -Wno-char-subscripts -O3 -fno-math-errno \
        -fno-signed-zeros -fno-tree-vectorize -Werror=format-security -Werror=implicit-function-declaration \
        -Werror=missing-prototypes -Werror=return-type -Werror=vla -Wformat -fdiagnostics-color=auto -Wno-maybe-uninitialized

CXXFLAGS=  -D__STDC_CONSTANT_MACROS -std=c++11
OBJCFLAGS=  
ASFLAGS= -g
NVCCFLAGS=--cuda-gpu-arch=sm_30 -O2 -m64 -S -nocudalib -nocudainc --cuda-device-only -include ./compat/cuda/cuda_runtime.h
AS_C=-c
AS_O=-o $@
OBJCC_C=-c
OBJCC_E=-E -o $@
OBJCC_O=-o $@
CC_C=-c
CC_E=-E -o $@
CC_O=-o $@
CXX_C=-c
CXX_O=-o $@
NVCC_C=
NVCC_O=-o $@
LD_O=-o $@
X86ASM_O=-o $@
LD_LIB=-l%
LD_PATH=-L
DLLTOOL=
WINDRES=windres
DEPWINDRES=$(CC)
DOXYGEN=doxygen
LDFLAGS=  -Wl,--as-needed -Wl,-z,noexecstack -Wl,--warn-common -Wl,-rpath-link=:libpostproc:libswresample:libswscale:libavfilter:libavdevice:libavformat:libavcodec:libavutil:libavresample
LDEXEFLAGS=
LDSOFLAGS=
SHFLAGS=-shared -Wl,-soname,$$(@F) -Wl,-Bsymbolic -Wl,--version-script,$(SUBDIR)lib$(NAME).ver
ASMSTRIPFLAGS= -x
X86ASMFLAGS=-f elf64 -g -F dwarf
MSAFLAGS=
MMIFLAGS=
BUILDSUF=
PROGSSUF=
FULLNAME=$(NAME)$(BUILDSUF)
LIBPREF=lib
LIBSUF=.a
LIBNAME=$(LIBPREF)$(FULLNAME)$(LIBSUF)
SLIBPREF=lib
SLIBSUF=.so
EXESUF=
EXTRA_VERSION=
CCDEP=
CXXDEP=
CCDEP_FLAGS=
ASDEP=
ASDEP_FLAGS=
X86ASMDEP=
X86ASMDEP_FLAGS=
CC_DEPFLAGS=-MMD -MF $(@:.o=.d) -MT $@
AS_DEPFLAGS=-MMD -MF $(@:.o=.d) -MT $@
X86ASM_DEPFLAGS=-MD $(@:.o=.d)
HOSTCC=$(CC)
HOSTLD=$(CC)
HOSTCFLAGS=  -std=c99 -Wall -O3
HOSTCPPFLAGS= -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600
HOSTEXESUF=
HOSTLDFLAGS= 
HOSTEXTRALIBS=-lm
DEPHOSTCC=$(CC)
DEPHOSTCCFLAGS= $(HOSTCCFLAGS)
HOSTCCDEP=
HOSTCCDEP_FLAGS=
HOSTCC_DEPFLAGS=-MMD -MF $(@:.o=.d) -MT $@
HOSTCC_C=-c
HOSTCC_O=-o $@
HOSTLD_O=-o $@
TARGET_EXEC= 
TARGET_PATH=$(CURDIR)
TARGET_SAMPLES=$(SAMPLES)
CFLAGS-ffplay=
CFLAGS_HEADERS= -Wno-deprecated-declarations -Wno-unused-variable
LIB_INSTALL_EXTRA_CMD=$$(RANLIB) "$(LIBDIR)/$(LIBNAME)"
EXTRALIBS=
COMPAT_OBJS=
INSTALL=install
LIBTARGET=
SLIBNAME=$(SLIBPREF)$(FULLNAME)$(SLIBSUF)
SLIBNAME_WITH_VERSION=$(SLIBNAME).$(LIBVERSION)
SLIBNAME_WITH_MAJOR=$(SLIBNAME).$(LIBMAJOR)
SLIB_CREATE_DEF_CMD=
SLIB_EXTRA_CMD=
SLIB_INSTALL_NAME=$(SLIBNAME_WITH_VERSION)
SLIB_INSTALL_LINKS=$(SLIBNAME_WITH_MAJOR) $(SLIBNAME)
SLIB_INSTALL_EXTRA_LIB=
SLIB_INSTALL_EXTRA_SHLIB=
VERSION_SCRIPT_POSTPROCESS_CMD=cat
SAMPLES:=$(FATE_SAMPLES)
NOREDZONE_FLAGS=-mno-red-zone
LIBFUZZER_PATH=
IGNORE_TESTS=
avdevice_FFLIBS=avfilter swscale postproc avformat avcodec swresample avutil
avfilter_FFLIBS=swscale postproc avformat avcodec swresample avutil
swscale_FFLIBS=avutil
postproc_FFLIBS=avutil
avformat_FFLIBS=avcodec swresample avutil
avcodec_FFLIBS=swresample avutil
swresample_FFLIBS=avutil
avresample_FFLIBS=avutil
avutil_FFLIBS=
EXTRALIBS-avdevice=-lm -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lasound
EXTRALIBS-avfilter=-pthread -lm
EXTRALIBS-swscale=-lm
EXTRALIBS-postproc=-lm
EXTRALIBS-avformat=-lm -lbz2 -lz
EXTRALIBS-avcodec=-pthread -lm -llzma -lz -lx264 -pthread -lm
EXTRALIBS-swresample=-lm
EXTRALIBS-avresample=-lm
EXTRALIBS-avutil=-pthread -lm
EXTRALIBS-ffplay=
EXTRALIBS-ffprobe=
EXTRALIBS-ffmpeg=
EXTRALIBS-cpu_init=-pthread
EXTRALIBS-cws2fws=-lz


include ffbuild/config.mak

endif # FFMPEG_CONFIG_MAK
