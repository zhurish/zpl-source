include $(MAKE_DIR)/makelinux.mk

include config.mk


#PJPROJ=$(PLBASE)/externsions/pjproject-2.8
#PJCC=$(CC)
#PJCXX=$(CXX)
#$(PJLIBS): 
#	cd $(PJPROJ); ./configure --prefix=$(PJPROJ)/_install --target=mipsel-openwrt-linux CC=$(PJCC) \
#		CXX=$(PJCXX) 
#		
#	make -C $(PJPROJ)
	
	
OBJS_DIR = $(BASE_ROOT)/$(OBJDIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(LIBDIR)

MODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(OBJS))

ifneq ($(TOBJS),)
TARGET = auddemo
TMODULE_OBJS  := $(addprefix $(OBJS_DIR)/,$(TOBJS))
endif
PLCFLAGS +=-DPJ_AUTOCONF=1 -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1
#PLCFLAGS += -Werror=format
#
#CFLAGS += -UHAVE_CONFIG_H
# It's a kind of magic...
#
#PL_LDLIBS += -lpj -lpjlib-util -lpjmedia -lpjmedia-audiodev -lpjmedia-codec\
#			 -lpjmedia-videodev -lpjnath -lpjsip -lpjsip-simple -lpjsip-ua \
#				-lpjsua -lsrtp
					
$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c 
	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 lib
	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 install
	@if test ! -d $(OBJS_DIR) ; \
		then \
		$(MKDIR) -p $(OBJS_DIR) ; \
	fi
	@$(PL_ECHO_CC)
	@$(PL_OBJ_COMPILE)
#

$(LIBS) : $(MODULE_OBJS) 
#	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 dep
#	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 lib
#	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 install
	$(PL_MAKE_LIB) $(LIBS) $(MODULE_OBJS) 

ifneq ($(TOBJS),)
LDCLFLAG = $(PLOS_LDLIBS) -L$(DSTULIBDIR) $(PL_LDLIBS) 

$(TARGET): $(TMODULE_OBJS)
	$(CC) $(TMODULE_OBJS) $(CFLAGS) $(LDCLFLAG) -o $(TARGET) 
endif		
#include $(MAKE_DIR)/make.mk

#lib: $(LIBS)
#
#
-include $(MODULE_OBJS:.o=.d)
#
#
#$(LIBS) : $(MODULE_OBJS)
#	$(PL_MAKE_LIB) $(LIBS) $(MODULE_OBJS)
#
lib: $(LIBS)


ifneq ($(TOBJS),)
obj: $(MODULE_OBJS) $(MODULE_LIBOBJS) $(TMODULE_OBJS)

install: $(LIBS) $(TARGET)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) --strip-unneeded $(LIB_DIR)/$(LIBS)
#	$(STRIP) $(LIB_DIR)/$(LIBS)
#	install -m 755 lib*.so $(LIB_DIR)
#	install -m 755 lib*.so.* $(LIB_DIR)
	@install -d ${DSTSBINDIR}
	@install -m 755 ${TARGET} ${DSTSBINDIR} 
	#$(STRIP) ${DSTSBINDIR}/${TARGET}
else
obj: $(MODULE_OBJS) $(MODULE_LIBOBJS)

install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) --strip-unneeded $(LIB_DIR)/$(LIBS)
#	$(STRIP) $(LIB_DIR)/$(LIBS)
#	install -m 755 lib*.so $(LIB_DIR)
#	install -m 755 lib*.so.* $(LIB_DIR)	
endif

clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 uninstall
	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 clean
	@$(RM) $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean
