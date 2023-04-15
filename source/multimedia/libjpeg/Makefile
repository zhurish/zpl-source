include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk


OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(LIBJPEG_ROOT)
SRC_DIR = $(BASE_ROOT)/$(LIBJPEG_ROOT)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CXX '$(ZPLCPPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLINCLUDE)' $@
	@$(CXX) -fPIC $(ZPLCPPFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) -g -c  $< -o $@ $(ZPLINCLUDE) 

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(PLFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) -g -c  $< -o $@ $(ZPLINCLUDE) 

$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)



files = $(patsubst %.o,%.cpp,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_BASE_ROOT); \
	done


#
lib: $(LIBS) 

obj: $(MODULE_OBJS)

install: $(LIBS) 
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(ZPLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)


clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/* -rf

all: lib install 

.PHONY:	obj all lib install objclean clean 