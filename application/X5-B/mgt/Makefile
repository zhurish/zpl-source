include $(MAKE_DIR)/makelinux.mk

include config.mk

#OBJS = $(OSOBJ)
PLCFLAGS += -Werror=format

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(APP_OBJ_DIR)
SRC_DIR = $(BASE_ROOT)/$(APP_OBJ_DIR)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(APP_OBJS))
LIBS=$(APP_LIBS)

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)

$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs  $(APP_LIBS) $(MODULE_OBJS)




-include $(MODULE_OBJS:.o=.d)
#
#
files = $(patsubst %.o,%.c,$(APP_OBJS))
make_prepare:
	@for i in $(files); do	\
		$(MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done
#
lib: $(LIBS)



obj: $(MODULE_OBJS) $(MODULE_LIBOBJS)

install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)
#	$(STRIP) $(LIB_DIR)/$(LIBS)

clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/* -rf

all: lib install

.PHONY:	obj all lib install objclean clean make_prepare

