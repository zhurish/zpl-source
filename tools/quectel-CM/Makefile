
include $(MAKE_DIR)/makelinux.mk



TARGET = quectel-CM
PLCFLAGS += -lpthread


MODULEDIR = tools/quectel-CM

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)


OBJS = QmiWwanCM.o GobiNetCM.o MPQMUX.o QMIThread.o \
		util.o udhcpc.o main.o

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
	$(CC) $(MODULE_OBJS) $(PLCFLAGS) -o $(TARGET) 


files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done

lib: $(TARGET)
#$(MODULE_OBJS)

obj: $(TARGET)
#$(MODULE_OBJS)

install: 
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