
include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

WATCHDOG_TARGET = watchdog

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

LDCLFLAG = $(ZPLOS_LDLIBS) -L$(ZPL_INSTALL_LIB_DIR) -Xlinker "-(" -los  -lbase -lshell -Xlinker "-)" 
#-lbase -lshell -lnsm -lpal -lhal -lkernel -lproduct -lsdk -lipcbc



$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)


files = $(patsubst %.o,%.c,$(MODULE_OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done


$(WATCHDOG_TARGET): $(MODULE_OBJS)
	$(CC) -fPIC $(MODULE_OBJS) $(ZPLCFLAGS) $(LDCLFLAG) -o $(WATCHDOG_TARGET) 


lib: $(MODULE_OBJS) 


obj: $(MODULE_OBJS) 


install: $(WATCHDOG_TARGET) 
	@install -d ${ZPL_INSTALL_SBIN_DIR}
	@install -m 755 ${WATCHDOG_TARGET} ${ZPL_INSTALL_SBIN_DIR} 
#
#	@$(STRIP) ${ZPL_INSTALL_SBIN_DIR}/${WATCHDOG_TARGET}

	
clean: objclean
	@$(RM) ${ZPL_INSTALL_SBIN_DIR}/${WATCHDOG_TARGET}
	@$(RM) $(WATCHDOG_TARGET)

	

objclean: 
	@$(RM) $(OBJS_DIR)/*

all: obj install 

.PHONY:	obj all lib install objclean clean make_prepare