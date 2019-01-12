include $(MAKE_DIR)/makelinux.mk

include config.mk

#OBJS = $(OSOBJ)

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SSHSRC_DIR = $(BASE_ROOT)/$(MODULEDIR)/libssh
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_LIBOBJS  := $(addprefix $(OBJS_DIR)/,$(LIBSSHOBJS))

SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

#-include $(MODULE_OBJS:.o=.d)

$(OBJS_DIR)/%.o: $(SSHSRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	$(ECHO) CC '$(CFLAGS)' $@
	@$(PL_OBJ_COMPILE)
	
	
$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	$(ECHO) CC '$(CFLAGS)' $@
	@$(PL_OBJ_COMPILE)
	

$(LIBS) : $(MODULE_OBJS) $(MODULE_LIBOBJS)
	$(PL_MAKE_LIB) $(LIBS) $(MODULE_OBJS) $(MODULE_LIBOBJS)

-include $(MODULE_LIBOBJS:.o=.d)

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
