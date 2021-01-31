include $(MAKE_DIR)/makelinux.mk

include config.mk




.PHONY:	obj all demo lib clean rebuild install objclean make_prepare

make_prepare:
	@for i in $(APP_DIR); do	\
		$(MAKE) -C $$i make_prepare || exit "$$?"; 	\
	done

objclean:
	@for i in $(APP_DIR); do	\
		$(MAKE) -C $$i objclean || exit "$$?"; 	\
	done
#
obj:
	@for i in $(APP_DIR); do	\
		$(MAKE) -C $$i obj || exit "$$?"; 	\
	done
	
install: 
	@for i in $(APP_DIR); do	\
		$(MAKE) -C $$i install || exit "$$?"; 	\
	done
#
clean: 
	@for i in $(APP_DIR); do	\
		$(MAKE) -C $$i clean || exit "$$?"; 	\
	done

demo: 	
	@for i in $(APP_DIR); do	\
		$(MAKE) -C $$i demo || exit "$$?"; 	\
	done

lib: 
	@for i in $(APP_DIR); do	\
		$(MAKE) -C $$i lib || exit "$$?"; 	\
	done
		
all: 
	@for i in $(APP_DIR); do	\
		$(MAKE) -C $$i all || exit "$$?"; 	\
	done
	
rebuild: clean all


#taget:
#	$(MAKE) -c $(MODULEDIR) taget
#	
#lib:
#	$(MAKE) -c $(MODULEDIR) lib
#		
#install:
#	$(MAKE) -c $(MODULEDIR) install
#
#clean: objclean
#	$(MAKE) -c $(MODULEDIR) clean
#
#objclean: 
#	$(MAKE) -c $(MODULEDIR) objclean

#all: lib install taget

#.PHONY:	obj all lib install objclean clean taget
