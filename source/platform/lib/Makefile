include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

#OBJS = $(OSOBJ)

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(OS_ZPLIB_DIR)
SRC_DIR = $(BASE_ROOT)/$(OS_ZPLIB_DIR)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

GAWK = gawk

#EXTRA_DIST = \
#	regex.c regex-gnu.h \
#	queue.h \
#	memtypes.awk \
#	route_types.pl route_types.txt \
#	gitversion.pl
	
DEPEND_HEADER = memtypes.h
# gitversion.h
	
MODULE_OBJS_DEP:=$(MODULE_OBJS:%.o=%.d)

-include $(MODULE_OBJS_DEP)
#-include $(MODULE_OBJS:.o=.d)
	
$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ZPL_ECHO_CC) $(ZPL_LIB_COMPILE)

#
#@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
#@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)

$(LIBS) : $(DEPEND_HEADER) $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)



memtypes.h: $(SRC_DIR)/memtypes.c $(SRC_DIR)/memtypes.awk
	($(GAWK) -f $(SRC_DIR)/memtypes.awk $(SRC_DIR)/memtypes.c > $@)


route_types.h: $(SRC_DIR)/route_types.txt $(SRC_DIR)/route_types.pl
	/usr/bin/perl $(SRC_DIR)/route_types.pl < $(SRC_DIR)/route_types.txt > $@
	

files = $(patsubst %.o,%.c,$(OBJS))
make_prepare: route_types.h 
	@for i in $(files); do	\
		sh $(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_BASE_ROOT); 	\
	done


lib: $(LIBS)


obj: $(DEPEND_HEADER) $(MODULE_OBJS)

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
