include $(MAKE_DIR)/makelinux.mk

include config.mk

#OBJS = $(OSOBJ)

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
	$(ECHO) CC '$(CFLAGS)' $@
	@$(PL_OBJ_COMPILE)

$(LIBS) : $(MODULE_OBJS)
	$(PL_MAKE_LIB) $(APP_LIBS) $(MODULE_OBJS)


include $(MAKE_DIR)/make.mk

#lib: $(LIBS)
#
#
#
#obj: $(MODULE_OBJS) $(MODULE_LIBOBJS)
#
#install: $(LIBS)
#	install -d $(LIB_DIR)
#	install -m 755 $(LIBS) $(LIB_DIR)
#	$(STRIP) $(LIB_DIR)/$(LIBS)

#clean: objclean
#	@$(RM) $(LIB_DIR)/$(LIBS)
#	@$(RM) $(LIBS)

#objclean: 
#	@$(RM) $(OBJS_DIR)/*

#all: lib install

#.PHONY:	obj all lib install objclean clean

