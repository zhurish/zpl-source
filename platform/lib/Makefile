include $(MAKE_DIR)/makelinux.mk

include config.mk

#OBJS = $(OSOBJ)

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

GAWK = gawk

#EXTRA_DIST = \
#	regex.c regex-gnu.h \
#	queue.h \
#	memtypes.awk \
#	route_types.pl route_types.txt \
#	gitversion.pl
	
BUILT_SOURCES = memtypes.h route_types.h
# gitversion.h
	
	
-include $(MODULE_OBJS:.o=.d)
	
$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(PL_ECHO_CC)
	@$(PL_OBJ_COMPILE)

$(LIBS) : $(BUILT_SOURCES) $(MODULE_OBJS)
	$(PL_MAKE_LIB) $(LIBS) $(MODULE_OBJS)



memtypes.h: $(SRC_DIR)/memtypes.c $(SRC_DIR)/memtypes.awk
	($(GAWK) -f $(SRC_DIR)/memtypes.awk $(SRC_DIR)/memtypes.c > $@)

route_types.h: $(SRC_DIR)/route_types.txt $(SRC_DIR)/route_types.pl
	/usr/bin/perl $(SRC_DIR)/route_types.pl < $(SRC_DIR)/route_types.txt > $@
	
	
	

lib: $(LIBS)


obj: $(BUILT_SOURCES) $(MODULE_OBJS)

install: $(LIBS)
	install -d $(LIB_DIR)
	install -m 755 $(LIBS) $(LIB_DIR)
	#$(STRIP) $(LIB_DIR)/$(LIBS)

clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean
