include $(MAKE_DIR)/makelinux.mk

include config.mk



OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
MODULE_BOBJS  := $(addprefix $(OBJS_DIR)/,$(BOBJS))

CFLAGS = -D_HAVE_SQLITE_CONFIG_H
LDCLFLAG += $(PL_LDLIBS) -L$(DSTLIBDIR) -lsqlite

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	$(ECHO) CC '$(CFLAGS)' $@
	@$(PL_OBJ_COMPILE)

$(LIBS) : $(MODULE_OBJS)
	$(PL_MAKE_LIB) $(LIBS) $(MODULE_OBJS)

$(LIBDS) : $(MODULE_OBJS)
	$(PL_MAKE_LIBSO) $(LIBDS) $(MODULE_OBJS)


$(SQLBIN) : $(MODULE_BOBJS)
	$(CC) $(MODULE_BOBJS) $(CFLAGS) $(LDCLFLAG) -o $(SQLBIN) 


taget: $(SQLBIN)
	install -d ${DSTSBINDIR}
	install -m 755 ${SQLBIN} ${DSTSBINDIR} 
	$(STRIP) $(DSTSBINDIR)/$(SQLBIN)

lib: $(LIBS) $(LIBDS) 
#



obj: $(MODULE_OBJS) $(MODULE_BOBJS)

install: $(LIBS) $(LIBDS) 
	install -d $(LIB_DIR)
	install -m 755 $(LIBS) $(LIB_DIR)
	$(STRIP) $(LIB_DIR)/$(LIBS)
	install -d $(LIB_DIR)
	install -m 755 $(LIBDS) $(LIB_DIR)
	$(STRIP) $(LIB_DIR)/$(LIBDS)
	
#	install -d ${DSTSBINDIR}
#	install -m 755 ${SQLBIN} ${DSTSBINDIR} 
#	$(STRIP) $(DSTSBINDIR)/$(SQLBIN)
	
clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS) $(LIB_DIR)/$(LIBDS)
	@$(RM) $(LIBS) $(LIBDS) $(SQLBIN)

objclean: 
	@$(RM) $(OBJS_DIR)/*.o

all: lib install taget

.PHONY:	obj all lib install objclean clean taget
