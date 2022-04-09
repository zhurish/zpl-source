
include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

UPGREATE_TARGET = app-upgrade
NVRAMENV_TARGET = nvramenv
#I2CSET_TARGET = codecreg

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)

upgrade_MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(upgrade_OBJS))
nvramenv_MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(nvramenv_OBJS))
#i2cset_MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(i2cset_OBJS))

LDCLFLAG = $(ZPLOS_LDLIBS) -L$(ZPL_INSTALL_LIB_DIR) -los 
#CFLAGS += -g


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)

-include $(upgrade_MODULE_OBJS:.o=.d)

$(UPGREATE_TARGET): $(upgrade_MODULE_OBJS)
	$(CC) $(upgrade_MODULE_OBJS) $(ZPLCFLAGS) $(LDCLFLAG) -o $(UPGREATE_TARGET) 
$(NVRAMENV_TARGET): $(nvramenv_MODULE_OBJS)
	$(CC) $(nvramenv_MODULE_OBJS) $(ZPLCFLAGS) $(LDCLFLAG) -o $(NVRAMENV_TARGET) 
 
$(I2CSET_TARGET): $(i2cset_MODULE_OBJS)
	$(CC) $(i2cset_MODULE_OBJS) $(ZPLCFLAGS) $(LDCLFLAG) -o $(I2CSET_TARGET)
	
files = $(patsubst %.o,%.c,$(upgrade_OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done

lib: $(upgrade_MODULE_OBJS) $(nvramenv_MODULE_OBJS) 
#$(i2cset_MODULE_OBJS)

obj: $(upgrade_MODULE_OBJS) $(nvramenv_MODULE_OBJS) 
#$(i2cset_MODULE_OBJS)

install: $(UPGREATE_TARGET) $(NVRAMENV_TARGET) 
#$(I2CSET_TARGET)
	@install -d ${ZPL_INSTALL_SBIN_DIR}
	@install -m 755 ${UPGREATE_TARGET} ${ZPL_INSTALL_SBIN_DIR} 
#	@$(STRIP) ${ZPL_INSTALL_SBIN_DIR}/${UPGREATE_TARGET}
	@install -m 755 ${NVRAMENV_TARGET} ${ZPL_INSTALL_SBIN_DIR} 
#	@$(STRIP) ${ZPL_INSTALL_SBIN_DIR}/${NVRAMENV_TARGET}
	
#	install -m 755 ${I2CSET_TARGET} ${ZPL_INSTALL_SBIN_DIR} 
#	$(STRIP) ${ZPL_INSTALL_SBIN_DIR}/${I2CSET_TARGET}

clean: objclean
	@$(RM) ${ZPL_INSTALL_SBIN_DIR}/${UPGREATE_TARGET}
	@$(RM) $(UPGREATE_TARGET)
	@$(RM) ${ZPL_INSTALL_SBIN_DIR}/${NVRAMENV_TARGET}
	@$(RM) $(NVRAMENV_TARGET)
	
#	@$(RM) ${ZPL_INSTALL_SBIN_DIR}/${I2CSET_TARGET}
#	@$(RM) $(I2CSET_TARGET)
	
objclean: 
	@$(RM) $(OBJS_DIR)/*

all: obj install 

.PHONY:	obj all lib install objclean clean make_prepare