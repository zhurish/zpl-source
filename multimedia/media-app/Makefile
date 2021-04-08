include $(MAKE_DIR)/makelinux.mk

include config.mk


OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CXX '$(PLCPPFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLINCLUDE)' $@
	@$(CXX) -fPIC $(PLCPPFLAGS) $(PLDEFINE) $(PLDEBUG) -g -c  $< -o $@ $(PLINCLUDE) 

$(OBJS_DIR)/%.o: $(SRC_DIR)/device/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLFLAGS) $(PLDEFINE) $(PLDEBUG) -g -c  $< -o $@ $(PLINCLUDE) 

$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)


$(OBJS_DIR)/testmain.o: $(SRC_DIR)/testmain.cpp
	@$(ECHO) CXX '$(PLCPPFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLINCLUDE)' $@
	@$(CXX) -fPIC $(PLCPPFLAGS) $(PLDEFINE) $(PLDEBUG) -g -c  $< -o $@ $(PLINCLUDE) 

TESTPLLDCLFLAG += -L$(LIB_DIR) -L$(DSTROOTFSDIR)/lib
ifeq ($(PL_BUILD_ARCH),X86_64)
TESTPLLDCLFLAG += -L/lib64 -L/usr/lib64 -L/usr/local/lib64
else
TESTPLLDCLFLAG += -L/lib -L/usr/lib -L/usr/local/lib
endif
TESTLIBS = -lpthread -lrt -rdynamic -lm -lcrypt -ldl -lgcc_s -lstdc++ \
		-lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lasound  \
		-lBasicUsageEnvironment  -lgroupsock  -lliveMedia  -lUsageEnvironment -lmediaapp \
		 -los

ifeq ($(strip $(PL_FFMPEG_MODULE)),true)
TESTLIBS += -lavdevice -lavcodec -lavformat -lavfilter -lavutil -lswscale -lswresample 
#-lpostproc
endif
ifeq ($(strip $(PL_OPENH264_MODULE)),true)
TESTLIBS += -lopenh264
endif
ifeq ($(strip $(PL_LIBX264_MODULE)),true)
TESTLIBS += -lx264
endif

files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done


test_main : $(OBJS_DIR)/testmain.o 
	$(CXX) $(OBJS_DIR)/testmain.o $(PLCPPFLAGS) $(TESTPLLDCLFLAG) -Xlinker "-(" $(TESTLIBS) -Xlinker "-)" -o testmain 
#
lib: $(LIBS) 

obj: $(MODULE_OBJS)

install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)


clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/* -rf

all: lib install

.PHONY:	obj all lib install objclean clean test_main