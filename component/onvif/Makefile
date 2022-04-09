include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk


	
OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
MODULE_OBJS_DEP:=$(MODULE_OBJS:%.o=%.d)

-include $(MODULE_OBJS_DEP)


$(OBJS_DIR)/%.o: $(SRC_DIR)/gsoap/%.c 	
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)


$(OBJS_DIR)/%.o: $(SRC_DIR)/onvifgen/%.c 
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)

#
#	
#
make_prepare:
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $(SRC_DIR)/onvifgen/onvif_api.c $(PLATFORM_ROOT); \

$(LIBS) : $(MODULE_OBJS) 
	@$(AR) -rs $(LIBS) $(MODULE_OBJS) 


#
#
-include $(MODULE_OBJS:.o=.d)

#
#
#
obj: $(MODULE_OBJS)

install-lib:
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)

	 
install: install-lib 


clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

lib: $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean install-lib make_prepare
