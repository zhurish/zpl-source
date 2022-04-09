include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk


#export SQLITE_GDB_DEBUG=false



OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)
LIBD_DIR = $(BASE_ROOT)/$(ZPL_ULIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
ifeq ($(strip $(ZPL_SQLITE_EXE_MODULE)),true)
MODULE_BOBJS  := $(addprefix $(OBJS_DIR)/,$(BOBJS))
endif

ZPLCFLAGS = -D_HAVE_SQLITE_CONFIG_H -MMD -MP -D__linux__ -std=gnu99 -fgnu89-inline \
				-Wfatal-errors -Wall -Wextra -Wnested-externs
		
#ZPLCFLAGS += -D_HAVE_SQLITE_CONFIG_H 
ifeq ($(ZPL_BUILD_DEBUG),YES)
ZPLCFLAGS += -g -ggdb 
endif
#ifeq ($(SQLITE_GDB_DEBUG),true)
#ZPLCFLAGS += -g -ggdb 
#endif
						
ZPLLDCLFLAG += $(ZPLOS_LDLIBS) -L$(ZPL_INSTALL_ULIB_DIR) -lsqlite -lz -lncurses -lreadline -lhistory

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)

-include $(MODULE_OBJS:.o=.d)
ifeq ($(strip $(ZPL_SQLITE_EXE_MODULE)),true)
-include $(MODULE_BOBJS:.o=.d)
endif

$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)

$(LIBDS) : $(MODULE_OBJS)
	$(CC) -shared -o  $(LIBDS) $(MODULE_OBJS)

ifeq ($(strip $(ZPL_SQLITE_EXE_MODULE)),true)

$(SQLBIN) : $(MODULE_BOBJS)
	$(CC) $(MODULE_BOBJS) $(ZPLCFLAGS) $(ZPLLDCLFLAG) -o $(SQLBIN) 

taget: $(SQLBIN)
	@install -d ${ZPL_INSTALL_SBIN_DIR}
	@install -m 755 ${SQLBIN} ${ZPL_INSTALL_SBIN_DIR} 
	@$(STRIP) $(ZPL_INSTALL_SBIN_DIR)/$(SQLBIN)

endif

files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done

lib: $(LIBS) $(LIBDS) 

#


ifeq ($(strip $(ZPL_SQLITE_EXE_MODULE)),true)
obj: $(MODULE_OBJS) $(MODULE_BOBJS)
else
obj: $(MODULE_OBJS)
endif

install: $(LIBS) $(LIBDS) 
	@install -d $(LIBD_DIR)
	@install -m 755 $(LIBDS) $(LIBD_DIR)

	

ifeq ($(strip $(ZPL_SQLITE_EXE_MODULE)),true)
clean: objclean
	@$(RM) $(LIBD_DIR)/$(LIBDS)
	@$(RM) $(ZPL_INSTALL_SBIN_DIR)/$(SQLBIN) $(SQLBIN)
else
clean: objclean
	@$(RM) $(LIBD_DIR)/$(LIBDS)
endif

objclean: 
	@$(RM) $(OBJS_DIR)/*

ifeq ($(strip $(ZPL_SQLITE_EXE_MODULE)),true)
all: lib install taget

.PHONY:	obj all lib install objclean clean taget make_prepare
else
all: lib install

.PHONY:	obj all lib install objclean clean make_prepare
endif