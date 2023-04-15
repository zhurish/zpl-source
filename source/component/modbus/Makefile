include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

MODBUSBIN = modbus-test

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODBUS_ROOT)
SRC_DIR = $(BASE_ROOT)/$(MODBUS_ROOT)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
MODULE_BOBJS  := $(addprefix $(OBJS_DIR)/,$(BOBJS))
MODULE_OBJS_DEP:=$(MODULE_OBJS:%.o=%.d)
MODULE_BOBJS_DEP:=$(MODULE_BOBJS:%.o=%.d)

-include $(MODULE_OBJS_DEP)
-include $(MODULE_BOBJS_DEP)

ZPLLDCLFLAG += $(ZPLOS_LDLIBS) -L$(BASE_ROOT)/$(ZPL_ULIB_DIR) -L$(LIB_DIR) -L$(ZPL_INSTALL_ROOTFS_DIR)/lib


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ZPL_ECHO_CC) $(ZPL_LIB_COMPILE)
#
#@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
#@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)
#

$(LIBS) : $(MODULE_OBJS) 
	@$(AR) -rs $(LIBS) $(MODULE_OBJS) 

$(MODBUSBIN) : $(MODULE_BOBJS)
	$(CC) $(MODULE_BOBJS) $(MODULE_OBJS) $(ZPLCFLAGS) $(ZPLLDCLFLAG) $(ZPLOS_LDLIBS) -Xlinker "-(" -los -luci -lbase -lshell -lmodbus -Xlinker "-)" -o $(MODBUSBIN) 
 


taget: $(MODBUSBIN)
	@install -d ${ZPL_INSTALL_SBIN_DIR}
	@install -m 755 ${MODBUSBIN} ${ZPL_INSTALL_SBIN_DIR} 
	@$(STRIP) $(ZPL_INSTALL_SBIN_DIR)/$(MODBUSBIN)


files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_BASE_ROOT); \
	done

lib: $(LIBS)


obj: $(MODULE_OBJS)

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

.PHONY:	obj all lib install objclean clean make_prepare taget
