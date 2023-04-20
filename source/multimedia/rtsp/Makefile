include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk


OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(LIBRTSP_DIR)
SRC_DIR = $(BASE_ROOT)/$(LIBRTSP_DIR)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

ZPLOS_CPPFLAGS +=-Wno-error=shadow

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ZPL_ECHO_CC) $(ZPL_LIB_COMPILE)

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ZPL_ECHO_CXX) $(ZPL_CXX_LIB_COMPILE)	

$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)



#files = $(patsubst %.o,%.cpp,$(OBJS))

make_prepare:
ifeq ($(strip $(ZPL_LIVE555_MODULE)),true)
	cd $(ZPLBASE)/$(LIVE555_ROOT);./genMakefiles pl
endif

	@for i in $(patsubst %.o,%.c,$(OBJS)); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_BASE_ROOT); \
	done


#
lib: $(LIBS) 

obj: $(MODULE_OBJS)

install: $(LIBS) 
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
#	@$(STRIP) $(ZPLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)


clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/* -rf

all: lib install 

.PHONY:	obj all lib install objclean clean 