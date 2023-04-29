include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk

OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MEDIASTREAM_ROOT)
SRC_DIR = $(BASE_ROOT)/$(MEDIASTREAM_ROOT)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))
MODULE_OBJS_DEP:=$(MODULE_OBJS:%.o=%.d)

-include $(MODULE_OBJS_DEP)

$(OBJS_DIR)/%.o: %.cpp
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(ZPL_ECHO_CXX) $(ZPL_CXX_LIB_COMPILE)


$(OBJS_DIR)/%.o: %.c
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	$(call ZPL_GCC_COMPILE)


$(LIBS) : $(MODULE_OBJS)
	@$(AR) -rs $(LIBS) $(MODULE_OBJS)



files = $(patsubst %.o,%.c,$(OBJS))
make_prepare:
	@for i in $(files); do	\
		$(ZPL_MAKE_DIR)/moduletypes.sh scan $$i $(PLATFORM_ROOT); \
	done


#
lib: $(LIBS) 

obj: $(MODULE_OBJS)

install: $(LIBS) 
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)


clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/* -rf

all: lib install 

.PHONY:	obj all lib install objclean clean 