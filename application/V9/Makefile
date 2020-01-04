include $(MAKE_DIR)/makelinux.mk

include config.mk

#OBJS = $(OSOBJ)
PLCFLAGS += -Werror=format

OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(APP_OBJ_DIR)
SRC_DIR = $(BASE_ROOT)/$(APP_OBJ_DIR)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(APP_OBJS))
LIBS=$(APP_LIBS)

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(PL_ECHO_CC)
	#@$(PL_CXX_OBJ_COMPILE)
	@$(PL_OBJ_COMPILE)

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(PL_ECHO_CPP)
	@$(PL_CXX_OBJ_COMPILE)
	
$(LIBS) : $(MODULE_OBJS)
	$(PL_MAKE_LIB) $(APP_LIBS) $(MODULE_OBJS)


#include $(MAKE_DIR)/make.mk

-include $(MODULE_OBJS:.o=.d)
#
#
#$(LIBS) : $(MODULE_OBJS)
#	$(PL_MAKE_LIB) $(LIBS) $(MODULE_OBJS)
#
lib: $(LIBS)



obj: $(MODULE_OBJS) $(MODULE_LIBOBJS)

install: $(LIBS)
	install -d $(LIB_DIR)
	install -m 755 $(LIBS) $(LIB_DIR)
	install -m 755 $(APP_ROOT)/V9/lib/liboal_privateProtocol.a $(LIB_DIR)
#	install -d $(BASE_ROOT)/$(ULIBDIR)
#	install -m 755 $(APP_ROOT)/V9/lib/liboal_privateProtocol.so $(BASE_ROOT)/$(ULIBDIR)

#	$(STRIP) $(LIB_DIR)/$(LIBS)

clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/* -rf

all: lib install

.PHONY:	obj all lib install objclean clean

