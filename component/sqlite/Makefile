include $(MAKE_DIR)/makelinux.mk

include config.mk


#export SQLITE_GDB_DEBUG=false



OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)
LIBD_DIR = $(BASE_ROOT)/$(ULIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
ifeq ($(strip $(PL_SQLITE_EXE_MODULE)),true)
MODULE_BOBJS  := $(addprefix $(OBJS_DIR)/,$(BOBJS))
endif

PLCFLAGS = -D_HAVE_SQLITE_CONFIG_H -MMD -MP -D__linux__ -std=gnu99 -fgnu89-inline \
				-Wfatal-errors -Wall -Wextra -Wnested-externs
		
#PLCFLAGS += -D_HAVE_SQLITE_CONFIG_H 
ifeq ($(PL_BUILD_DEBUG),YES)
PLCFLAGS += -g -ggdb 
endif
#ifeq ($(SQLITE_GDB_DEBUG),true)
#PLCFLAGS += -g -ggdb 
#endif
						
PLLDCLFLAG += $(PLOS_LDLIBS) -L$(DSTULIBDIR) -lsqlite -lz -lncurses -lreadline -lhistory

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)

-include $(MODULE_OBJS:.o=.d)
ifeq ($(strip $(PL_SQLITE_EXE_MODULE)),true)
-include $(MODULE_BOBJS:.o=.d)
endif

$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)

$(LIBDS) : $(MODULE_OBJS)
	$(CC) -shared -o  $(LIBDS) $(MODULE_OBJS)

ifeq ($(strip $(PL_SQLITE_EXE_MODULE)),true)

$(SQLBIN) : $(MODULE_BOBJS)
	$(CC) $(MODULE_BOBJS) $(PLCFLAGS) $(PLLDCLFLAG) -o $(SQLBIN) 

taget: $(SQLBIN)
	@install -d ${DSTSBINDIR}
	@install -m 755 ${SQLBIN} ${DSTSBINDIR} 
	@$(STRIP) $(DSTSBINDIR)/$(SQLBIN)

endif


lib: $(LIBS) $(LIBDS) 

#


ifeq ($(strip $(PL_SQLITE_EXE_MODULE)),true)
obj: $(MODULE_OBJS) $(MODULE_BOBJS)
else
obj: $(MODULE_OBJS)
endif

install: $(LIBS) $(LIBDS) 
	@install -d $(LIBD_DIR)
	@install -m 755 $(LIBDS) $(LIBD_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIBD_DIR)/$(LIBDS)
	#$(STRIP) $(LIBD_DIR)/$(LIBDS)
	

ifeq ($(strip $(PL_SQLITE_EXE_MODULE)),true)
clean: objclean
	@$(RM) $(LIBD_DIR)/$(LIBDS)
	@$(RM) $(DSTSBINDIR)/$(SQLBIN) $(SQLBIN)
else
clean: objclean
	@$(RM) $(LIBD_DIR)/$(LIBDS)
endif

objclean: 
	@$(RM) $(OBJS_DIR)/*

ifeq ($(strip $(PL_SQLITE_EXE_MODULE)),true)
all: lib install taget

.PHONY:	obj all lib install objclean clean taget
else
all: lib install

.PHONY:	obj all lib install objclean clean
endif