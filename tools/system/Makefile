
include $(MAKE_DIR)/makelinux.mk

include config.mk

TARGET = app-upgrade

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

LDCLFLAG = $(PLOS_LDLIBS) -L$(DSTLIBDIR) -los 
#CFLAGS += -g


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	$(ECHO) CC '$(CFLAGS)' $@
	@$(PL_OBJ_COMPILE)

-include $(MODULE_OBJS:.o=.d)

$(TARGET): $(MODULE_OBJS)
	$(CC) $(MODULE_OBJS) $(CFLAGS) $(LDCLFLAG) -o $(TARGET) 
#	$(CC) $(MODULE_OBJS) $(CFLAGS) -o $(TARGET) 

lib: $(MODULE_OBJS)

obj: $(MODULE_OBJS)

install: $(TARGET)
	install -d ${DSTSBINDIR}
	install -m 755 ${TARGET} ${DSTSBINDIR} 
	$(STRIP) ${DSTSBINDIR}/${TARGET}

clean: objclean
	@$(RM) ${DSTSBINDIR}/${TARGET}
	@$(RM) $(TARGET)

objclean: 
	@$(RM) $(OBJS_DIR)/*

all: obj install 

.PHONY:	obj all lib install objclean clean