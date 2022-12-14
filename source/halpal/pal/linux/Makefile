include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

#OBJS = $(OSOBJ)

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)


MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
MODULE_OBJS_DEP:=$(MODULE_OBJS:%.o=%.d)

ifeq ($(strip $(LIB_IPROUTE_ENABLE)),true)
IPFLAGS = -I$(SRC_DIR)/libiproute/include
MODULE_IPOBJS  := $(addprefix $(OBJS_DIR)/,$(IPOBJS))
MODULE_IPOBJS_DEP:=$(MODULE_IPOBJS:%.o=%.d)
else
MODULE_IPOBJS = 
endif

-include $(MODULE_OBJS_DEP)

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ZPL_ECHO_CC) $(ZPL_LIB_COMPILE)
#
ifeq ($(strip $(LIB_IPROUTE_ENABLE)),true)
$(OBJS_DIR)/%.o: $(SRC_DIR)/libiproute/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	$(CC) -fPIC $(IPFLAGS) $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE) 
#
endif
#@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
#@$(CC) -fPIC $(ZPLCFLAGS) $(IPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)

$(LIBS) : $(MODULE_OBJS) $(MODULE_IPOBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS) $(MODULE_IPOBJS)

files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done

lib: $(LIBS)


obj: $(MODULE_OBJS) $(MODULE_IPOBJS)

install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)


clean: objclean
	@if test -f $(LIB_DIR)/$(LIBS) ; \
		then \
		$(RM) $(LIB_DIR)/$(LIBS) ; \
	fi
	@if test -f $(LIBS) ; \
		then \
		$(RM) $(LIBS) ; \
	fi

objclean: 
	@$(RM) -rf $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean make_prepare
