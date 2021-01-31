
include $(MAKE_DIR)/makelinux.mk

include config.mk

UPGREATE_TARGET = app-upgrade
NVRAMENV_TARGET = nvramenv
#I2CSET_TARGET = codecreg

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)

upgrade_MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(upgrade_OBJS))
nvramenv_MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(nvramenv_OBJS))
#i2cset_MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(i2cset_OBJS))

LDCLFLAG = $(PLOS_LDLIBS) -L$(DSTLIBDIR) -los 
#CFLAGS += -g


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)

-include $(upgrade_MODULE_OBJS:.o=.d)

$(UPGREATE_TARGET): $(upgrade_MODULE_OBJS)
	$(CC) $(upgrade_MODULE_OBJS) $(PLCFLAGS) $(LDCLFLAG) -o $(UPGREATE_TARGET) 
$(NVRAMENV_TARGET): $(nvramenv_MODULE_OBJS)
	$(CC) $(nvramenv_MODULE_OBJS) $(PLCFLAGS) $(LDCLFLAG) -o $(NVRAMENV_TARGET) 
 
$(I2CSET_TARGET): $(i2cset_MODULE_OBJS)
	$(CC) $(i2cset_MODULE_OBJS) $(PLCFLAGS) $(LDCLFLAG) -o $(I2CSET_TARGET)
	
files = $(patsubst %.o,%.c,$(upgrade_OBJS))
make_prepare:
	@for i in $(files); do	\
		$(MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done

lib: $(upgrade_MODULE_OBJS) $(nvramenv_MODULE_OBJS) 
#$(i2cset_MODULE_OBJS)

obj: $(upgrade_MODULE_OBJS) $(nvramenv_MODULE_OBJS) 
#$(i2cset_MODULE_OBJS)

install: $(UPGREATE_TARGET) $(NVRAMENV_TARGET) 
#$(I2CSET_TARGET)
	@install -d ${DSTSBINDIR}
	@install -m 755 ${UPGREATE_TARGET} ${DSTSBINDIR} 
	@$(STRIP) ${DSTSBINDIR}/${UPGREATE_TARGET}
	@install -m 755 ${NVRAMENV_TARGET} ${DSTSBINDIR} 
	@$(STRIP) ${DSTSBINDIR}/${NVRAMENV_TARGET}
	
#	install -m 755 ${I2CSET_TARGET} ${DSTSBINDIR} 
#	$(STRIP) ${DSTSBINDIR}/${I2CSET_TARGET}

clean: objclean
	@$(RM) ${DSTSBINDIR}/${UPGREATE_TARGET}
	@$(RM) $(UPGREATE_TARGET)
	@$(RM) ${DSTSBINDIR}/${NVRAMENV_TARGET}
	@$(RM) $(NVRAMENV_TARGET)
	
#	@$(RM) ${DSTSBINDIR}/${I2CSET_TARGET}
#	@$(RM) $(I2CSET_TARGET)
	
objclean: 
	@$(RM) $(OBJS_DIR)/*

all: obj install 

.PHONY:	obj all lib install objclean clean make_prepare