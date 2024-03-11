include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk
VERSION=v2.3
DEF=-DRTMPDUMP_VERSION=\"$(VERSION)\"

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(LIBRTMP_ROOT)
SRC_DIR = $(BASE_ROOT)/$(LIBRTMP_ROOT)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
MODULE_OBJS_DEP:=$(MODULE_OBJS:%.o=%.d)
-include $(MODULE_OBJS_DEP)
PLFLAGS += -I$(SRC_DIR)/include -I$(SRC_DIR)/source

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLFLAGS) $(ZPLDEFINE) $(DEF) $(ZPLDEBUG) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(PLFLAGS) $(ZPLDEFINE) $(DEF) $(ZPLDEBUG) -g -c  $< -o $@ $(ZPLINCLUDE) 

$(OBJS_DIR)/%.o: $(SRC_DIR)/source/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLFLAGS) $(ZPLDEFINE) $(DEF) $(ZPLDEBUG) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(PLFLAGS) $(ZPLDEFINE) $(DEF) $(ZPLDEBUG) -g -c  $< -o $@ $(ZPLINCLUDE) 


$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)



files = $(patsubst %.o,%.c,$(OBJS))
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

.PHONY:	obj all lib install objclean clean test_main