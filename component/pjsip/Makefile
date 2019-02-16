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
	$(ECHO) CC '$(CFLAGS)' $@
	@$(PL_OBJ_COMPILE)
#

$(LIBS) : $(MODULE_OBJS) 
#	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 dep
#	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 lib
#	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 install
	$(PL_MAKE_LIB) $(LIBS) $(MODULE_OBJS) 

	
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



obj: $(MODULE_OBJS) $(MODULE_LIBOBJS)

install: $(LIBS)
	install -d $(LIB_DIR)
	install -m 755 $(LIBS) $(LIB_DIR)
#	$(STRIP) $(LIB_DIR)/$(LIBS)
#	install -m 755 lib*.so $(LIB_DIR)
#	install -m 755 lib*.so.* $(LIB_DIR)



clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 uninstall
	$(MAKE) -C $(PLBASE)/externsions/pjproject-2.8 clean
	@$(RM) $(OBJS_DIR)/*

all: lib install

.PHONY:	obj all lib install objclean clean
