
include $(MAKE_DIR)/makelinux.mk

include config.mk

TARGET = ProcessMU

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)

LDCLFLAG = $(PL_LDLIBS) -L$(DSTLIBDIR) -los 
CFLAGS += -g

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	$(ECHO) CC '$(CFLAGS)' $@
	@$(PL_OBJ_COMPILE)


$(TARGET): $(MODULE_OBJS)
	$(CC) $(MODULE_OBJS) $(CFLAGS) $(LDCLFLAG) -o $(TARGET) 

lib: $(MODULE_OBJS)
#$(MODULE_OBJS)

obj: $(MODULE_OBJS)
#$(MODULE_OBJS)

install: $(TARGET)
	install -d ${DSTSBINDIR}
	install -m 755 ${TARGET} ${DSTSBINDIR} 
	$(STRIP) ${DSTSBINDIR}/${TARGET}

clean: objclean
	@$(RM) ${DSTSBINDIR}/${TARGET}
	@$(RM) $(TARGET)

objclean: 
	@$(RM) $(OBJS_DIR)/*.o

all: obj install 

.PHONY:	obj all lib install objclean clean