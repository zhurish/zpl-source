
include $(MAKE_DIR)/makelinux.mk

include config.mk

TARGET = ProcessMU

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)

LDCLFLAG = $(PLOS_LDLIBS) -L$(DSTLIBDIR) -los -luci
CFLAGS += -g

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) $(PLINCLUDE)' $@
	@$(CC) -fPIC $(PLCFLAGS) $(PLDEFINE) $(PLDEBUG) $(PLLDFLAGS) -c  $< -o $@ $(PLINCLUDE)


-include $(MODULE_OBJS:.o=.d)

$(TARGET): $(MODULE_OBJS)
	$(CC) $(MODULE_OBJS) $(CFLAGS) $(LDCLFLAG) -o $(TARGET) 


files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done

lib: $(MODULE_OBJS)

obj: $(MODULE_OBJS)


install: $(TARGET)
	@install -d ${DSTSBINDIR}
	@install -m 755 ${TARGET} ${DSTSBINDIR} 
	@$(STRIP) ${DSTSBINDIR}/${TARGET}

clean: objclean
	@$(RM) ${DSTSBINDIR}/${TARGET}
	@$(RM) $(TARGET)

objclean: 
	@$(RM) $(OBJS_DIR)/*

all: obj install 

.PHONY:	obj all lib install objclean clean make_prepare