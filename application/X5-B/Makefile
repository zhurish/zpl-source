
#EXTRA_DEFINE += -DX5_B_A_DEBUG

include $(MAKE_DIR)/makelinux.mk

include config.mk

#PL_DEBUG += -DX5_B_A_DEBUG

.PHONY:	obj all demo lib clean rebuild install objclean


objclean:
	@for i in $(X5_B_DIR); do	\
		$(MAKE) -C $$i objclean || exit "$$?"; 	\
	done
#
obj:
	@for i in $(X5_B_DIR); do	\
		$(MAKE) -C $$i obj || exit "$$?"; 	\
	done
	
install: 
	@for i in $(X5_B_DIR); do	\
		$(MAKE) -C $$i install || exit "$$?"; 	\
	done
#
clean: 
	@for i in $(X5_B_DIR); do	\
		$(MAKE) -C $$i clean || exit "$$?"; 	\
	done

demo: 	
	@for i in $(X5_B_DIR); do	\
		$(MAKE) -C $$i demo || exit "$$?"; 	\
	done

lib: 
	@for i in $(X5_B_DIR); do	\
		$(MAKE) -C $$i lib || exit "$$?"; 	\
	done
		
all: 
	@for i in $(X5_B_DIR); do	\
		$(MAKE) -C $$i all || exit "$$?"; 	\
	done
	
rebuild: clean all

