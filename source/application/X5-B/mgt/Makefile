include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

#OBJS = $(OSOBJ)
ZPLCFLAGS += -Werror=format

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(APP_OBJ_DIR)
SRC_DIR = $(BASE_ROOT)/$(APP_OBJ_DIR)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(APP_OBJS))
LIBS=$(APP_LIBS)

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)

$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs  $(APP_LIBS) $(MODULE_OBJS)




-include $(MODULE_OBJS:.o=.d)
#
#
files = $(patsubst %.o,%.c,$(APP_OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done
#
lib: $(LIBS)



obj: $(MODULE_OBJS) $(MODULE_LIBOBJS)

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

