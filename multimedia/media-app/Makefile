include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk


OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CXX '$(ZPLCPPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLINCLUDE)' $@
	@$(CXX) -fPIC $(ZPLCPPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) -g -c  $< -o $@ $(ZPLINCLUDE) 

$(OBJS_DIR)/%.o: $(SRC_DIR)/device/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(PLFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) -g -c  $< -o $@ $(ZPLINCLUDE) 

$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)


$(OBJS_DIR)/testmain.o: $(SRC_DIR)/testmain.cpp
	@$(ECHO) CXX '$(ZPLCPPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLINCLUDE)' $@
	@$(CXX) -fPIC $(ZPLCPPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) -g -c  $< -o $@ $(ZPLINCLUDE) 

TESTPLLDCLFLAG += -L$(LIB_DIR) -L$(BASE_ROOT)/$(ZPL_ULIB_DIR)
ifeq ($(ZPL_BUILD_ARCH),X86_64)
TESTPLLDCLFLAG += -L/lib64 -L/usr/lib64 -L/usr/local/lib64
else
TESTPLLDCLFLAG += -L/lib -L/usr/lib -L/usr/local/lib
endif
TESTLIBS = -lpthread -lrt -rdynamic -lm -lcrypt -ldl -lgcc_s -lstdc++ \
		-lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lasound  \
		-lBasicUsageEnvironment  -lgroupsock  -lliveMedia  -lUsageEnvironment -lliveMediaUtil -lmediaapp\
		 -los

ifeq ($(strip $(ZPL_FFMPEG_MODULE)),true)
TESTLIBS += -lavdevice -lavcodec -lavformat -lavfilter -lavutil -lswscale -lswresample 
#-lpostproc
endif
ifeq ($(strip $(ZPL_OPENH264_MODULE)),true)
TESTLIBS += -lopenh264
endif
ifeq ($(strip $(ZPL_LIBX264_MODULE)),true)
TESTLIBS += -lx264
endif

files = $(patsubst %.o,%.cpp,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done


test_main : $(OBJS_DIR)/testmain.o 
	$(CXX) $(OBJS_DIR)/testmain.o $(ZPLCPPFLAGS) $(TESTPLLDCLFLAG) -Xlinker "-(" $(TESTLIBS) -Xlinker "-)" -o testmain 
#
#
lib: $(LIBS) 

obj: $(MODULE_OBJS)

install: $(LIBS) 
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(ZPLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)


clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/* -rf

all: lib install test_main

.PHONY:	obj all lib install objclean clean test_main