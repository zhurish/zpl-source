include $(MAKE_DIR)/makelinux.mk

include config.mk


OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
PLCFLAGS += -DENABLE_TRACE
# It's a kind of magic...
	
$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c 
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(PL_ECHO_CC)
	@$(PL_OBJ_COMPILE)
#

$(LIBS) : $(MODULE_OBJS) 
	$(PL_MAKE_LIB) $(LIBS) $(MODULE_OBJS) 


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

