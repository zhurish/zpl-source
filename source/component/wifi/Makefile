include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

#OBJS = $(OSOBJ)
# Version of the Wireless Tools
WT_VERSION := $(shell sed -ne "/WT_VERSION/{s:\([^0-9]*\)::;p;q;}" < iwlib.h )

# Version of Wireless Extensions.
WE_VERSION := $(shell sed -ne "/WE_VERSION/{s:\([^0-9]*\)::;p;q;}" < iwlib.h )

# Always use local header for wireless extensions
WEXT_HEADER = wireless.$(WE_VERSION).h

#CFLAGS +=-DWE_ESSENTIAL #-D__USE_GNU

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
MODULE_OBJS_DEP:=$(MODULE_OBJS:%.o=%.d)

-include $(MODULE_OBJS_DEP)

# It's a kind of magic...
wireless.h:
	cp $(WEXT_HEADER) wireless.h
	
	
$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c wireless.h
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)
#

$(LIBS) : $(MODULE_OBJS) 
	@$(AR) -rs $(LIBS) $(MODULE_OBJS) 

files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done

lib: $(LIBS)


obj: $(BUILT_SOURCES) $(MODULE_OBJS)

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
