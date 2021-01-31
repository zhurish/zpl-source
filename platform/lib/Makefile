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
	
DEPEND_HEADER = memtypes.h
# gitversion.h
	
	
#-include $(MODULE_OBJS:.o=.d)
	
$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)

$(LIBS) : $(DEPEND_HEADER) $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)



memtypes.h: $(SRC_DIR)/memtypes.c $(SRC_DIR)/memtypes.awk
	($(GAWK) -f $(SRC_DIR)/memtypes.awk $(SRC_DIR)/memtypes.c > $@)

files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		sh $(MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); 	\
	done

lib: $(LIBS)

lib: $(LIBS)


obj: $(DEPEND_HEADER) $(MODULE_OBJS)

install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIB_DIR)/$(LIBS)

clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean make_prepare
