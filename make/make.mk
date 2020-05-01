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

ifeq ($(BUILD_DEBUG),YES)
install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) --strip-unneeded $(LIB_DIR)/$(LIBS)
else
install: $(LIBS)
	@install -d $(LIB_DIR)
	@install -m 755 $(LIBS) $(LIB_DIR)
	@$(STRIP) --strip-unneeded $(LIB_DIR)/$(LIBS)	
endif

#install: $(LIBS)
#	install -d $(LIB_DIR)
#	install -m 755 $(LIBS) $(LIB_DIR)
#	$(STRIP) $(LIB_DIR)/$(LIBS)

clean: objclean
	@$(RM) $(LIB_DIR)/$(LIBS)
	@$(RM) $(LIBS)

objclean: 
	@$(RM) $(OBJS_DIR)/* -rf

all: lib install

.PHONY:	obj all lib install objclean clean