
include $(ZPL_MAKE_DIR)/makelinux.mk



TARGET = quectel-CM
ZPLCFLAGS += -lpthread


MODULEDIR = tools/quectel-CM

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)


OBJS = QmiWwanCM.o GobiNetCM.o MPQMUX.o QMIThread.o \
		util.o udhcpc.o main.o

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))


$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ECHO) CC '$(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) $(ZPLINCLUDE)' $@
	@$(CC) -fPIC $(ZPLCFLAGS) $(ZPLDEFINE) $(ZPLDEBUG) $(ZPLLDFLAGS) -c  $< -o $@ $(ZPLINCLUDE)
	
	
-include $(MODULE_OBJS:.o=.d)

$(TARGET): $(MODULE_OBJS)
	$(CC) $(MODULE_OBJS) $(ZPLCFLAGS) -o $(TARGET) 


files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done

lib: $(TARGET)
#$(MODULE_OBJS)

obj: $(TARGET)
#$(MODULE_OBJS)

install: 
	@install -d ${ZPL_INSTALL_SBIN_DIR}
	@install -m 755 ${TARGET} ${ZPL_INSTALL_SBIN_DIR} 
	@$(STRIP) ${ZPL_INSTALL_SBIN_DIR}/${TARGET}

clean: objclean
	@$(RM) ${ZPL_INSTALL_SBIN_DIR}/${TARGET}
	@$(RM) $(TARGET)

objclean: 
	@$(RM) $(OBJS_DIR)/*

all: obj install 

.PHONY:	obj all lib install objclean clean make_prepare