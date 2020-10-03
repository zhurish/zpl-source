include $(MAKE_DIR)/makelinux.mk

include config.mk

OBJS = $(OSOBJ)

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)

$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)



lib: $(LIBS)


obj: $(BUILT_SOURCES) $(MODULE_OBJS)

install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)

clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) -rf $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean
