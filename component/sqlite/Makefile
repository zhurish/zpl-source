include $(MAKE_DIR)/makelinux.mk

include config.mk


#export SQLITE_GDB_DEBUG=false



OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)
LIBD_DIR = $(BASE_ROOT)/$(ULIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
MODULE_BOBJS  := $(addprefix $(OBJS_DIR)/,$(BOBJS))

PLCFLAGS = -D_HAVE_SQLITE_CONFIG_H -MMD -MP -D__linux__ -std=gnu99 -fgnu89-inline \
				-Wfatal-errors -Wall -Wextra -Wnested-externs
		
#PLCFLAGS += -D_HAVE_SQLITE_CONFIG_H 
ifeq ($(BUILD_DEBUG),YES)
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
-include $(MODULE_BOBJS:.o=.d)

$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)

$(LIBDS) : $(MODULE_OBJS)
	$(CC) -shared -o  $(LIBDS) $(MODULE_OBJS)


$(SQLBIN) : $(MODULE_BOBJS)
	$(CC) $(MODULE_BOBJS) $(PLCFLAGS) $(PLLDCLFLAG) -o $(SQLBIN) 


taget: $(SQLBIN)
	@install -d ${DSTSBINDIR}
	@install -m 755 ${SQLBIN} ${DSTSBINDIR} 
	@$(STRIP) $(DSTSBINDIR)/$(SQLBIN)

lib: $(LIBS) $(LIBDS) 
#



obj: $(MODULE_OBJS) $(MODULE_BOBJS)

install: $(LIBS) $(LIBDS) 
	#install -d $(LIB_DIR)
	#install -m 755 $(LIBS) $(LIB_DIR)
	#$(STRIP) --strip-unneeded $(LIB_DIR)/$(LIBS)
	#$(STRIP) $(LIB_DIR)/$(LIBS)
	@install -d $(LIBD_DIR)
	@install -m 755 $(LIBDS) $(LIBD_DIR)
	@$(STRIP) $(PLSTRIP_CFLAGS) $(LIBD_DIR)/$(LIBDS)
	#$(STRIP) $(LIBD_DIR)/$(LIBDS)
	
#	install -d ${DSTSBINDIR}
#	install -m 755 ${SQLBIN} ${DSTSBINDIR} 
#	$(STRIP) $(DSTSBINDIR)/$(SQLBIN)
	
clean: objclean
	@$(RM) $(LIBD_DIR)/$(LIBDS)
	@$(RM) $(DSTSBINDIR)/$(SQLBIN) $(SQLBIN)

objclean: 
	@$(RM) $(OBJS_DIR)/*

all: lib install taget

.PHONY:	obj all lib install objclean clean taget
