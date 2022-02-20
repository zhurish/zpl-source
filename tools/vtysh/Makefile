
include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

VTYSH_TARGET = vtysh

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

LDCLFLAG = $(ZPLOS_LDLIBS) -L$(ZPL_INSTALL_LIB_DIR) -lreadline -Xlinker "-(" -los -lbase -lshell -lsyslog -Xlinker "-)" 

vtysh_cmd_FILES = $(BASE_ROOT)/platform/lib/cmd_host.c $(BASE_ROOT)/platform/lib/cmd_log.c \
		  $(BASE_ROOT)/platform/lib/cmd_memory.c $(BASE_ROOT)/platform/lib/eloop.c \
		  $(BASE_ROOT)/platform/lib/distribute.c $(BASE_ROOT)/platform/lib/filter.c \
		  $(BASE_ROOT)/platform/lib/keychain.c \
		  $(BASE_ROOT)/platform/lib/nsm_vrf.c $(BASE_ROOT)/platform/lib/plist.c \
		  $(BASE_ROOT)/platform/nsm/cli/cmd_interface.c \
		  $(BASE_ROOT)/platform/nsm/cli/cmd_arp.c \
		  $(BASE_ROOT)/platform/nsm/cli/cmd_debug.c $(BASE_ROOT)/platform/nsm/cli/cmd_dhcpc.c \
		  $(BASE_ROOT)/platform/nsm/cli/cmd_dhcps.c $(BASE_ROOT)/platform/nsm/cli/cmd_dns.c \
		  $(BASE_ROOT)/platform/nsm/cli/cmd_dos.c \
	      $(BASE_ROOT)/platform/nsm/cli/cmd_dot1x.c

vtysh_cmd.c: 
	./extract.pl $(vtysh_cmd_FILES) > vtysh_cmd.c

perldepends:
	rm perlinc.txt
	echo $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLINCLUDE) > perlinc.txt
	sed -i ':a;N;$!ba;s/\n/;/g' perlinc.txt

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

$(VTYSH_TARGET): $(MODULE_OBJS)
	$(CC) -fPIC $(MODULE_OBJS) $(ZPLCFLAGS) $(LDCLFLAG) -o $(VTYSH_TARGET) 

lib: perldepends vtysh_cmd.c $(MODULE_OBJS) 


obj: perldepends vtysh_cmd.c $(MODULE_OBJS) 


install: $(VTYSH_TARGET) 
	@install -d ${ZPL_INSTALL_SBIN_DIR}
	@install -m 755 ${VTYSH_TARGET} ${ZPL_INSTALL_SBIN_DIR} 
	@$(STRIP) ${ZPL_INSTALL_SBIN_DIR}/${VTYSH_TARGET}

	
clean: objclean
	@$(RM) ${ZPL_INSTALL_SBIN_DIR}/${VTYSH_TARGET}
	@$(RM) $(VTYSH_TARGET)

	

objclean: 
	@$(RM) $(OBJS_DIR)/*

all: obj install 

.PHONY:	obj all lib install objclean clean make_prepare perldepends