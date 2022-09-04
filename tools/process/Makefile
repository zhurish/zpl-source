
include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

TARGET = ProcessMU

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)

LDCLFLAG = $(ZPLOS_LDLIBS) -L$(ZPL_INSTALL_LIB_DIR) -los -luci
CFLAGS += -g

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)


-include $(MODULE_OBJS:.o=.d)

$(TARGET): $(MODULE_OBJS)
	$(CC) $(MODULE_OBJS) $(CFLAGS) $(LDCLFLAG) -o $(TARGET) 


files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done

lib: $(MODULE_OBJS)

obj: $(MODULE_OBJS)


install: $(TARGET)
	@install -d ${ZPL_INSTALL_SBIN_DIR}
	@install -m 755 ${TARGET} ${ZPL_INSTALL_SBIN_DIR} 
#
#	@$(STRIP) ${ZPL_INSTALL_SBIN_DIR}/${TARGET}

clean: objclean
	@if test -f ${ZPL_INSTALL_SBIN_DIR}/${TARGET} ; \
		then \
		@$(RM) ${ZPL_INSTALL_SBIN_DIR}/${TARGET} ; \
	fi
	@if test -f $(TARGET) ; \
		then \
		@$(RM) $(TARGET) ; \
	fi

objclean: 
	@$(RM) -rf $(OBJS_DIR)/*

all: obj install 

.PHONY:	obj all lib install objclean clean make_prepare