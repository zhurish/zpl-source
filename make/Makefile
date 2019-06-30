#
#
#
#
include makelinux.mk
#
#
#
# mkdir OBJ dir
#
PLPRODS_OBJ = $(PLPRODS)
PLPRODS_OBJ += $(BASE_ROOT)/empty
#
#
#
#
ifeq ($(MKDIR_TEST),true)
define _MODULE_OBJ
TMP_OBJ := $$(shell basename $(strip $(1)))
MK_OBJ := $$(shell $(MAKE_DIR) -p  $(BASE_ROOT)/$(OBJDIR)/$(TMP_OBJ) $(BASE_ROOT)/$(LIBDIR))
endef
#
$(foreach objdir,$(PLPRODS_OBJ), $(eval $(call _MODULE_OBJ,$(objdir))))
endif
#
#
#
#
#
#
.PHONY:	obj all demo lib clean rebuild install help usage objclean objlibclean
#
#
help:usage
#
usage:
	@$(ECHO) ""
	@$(ECHO) "This is an Interpeak demo GNU makefile."
	@$(ECHO) "Please check board.cfg for configuration options!"
	@$(ECHO) "Please check module.mk for configuration options!"
	@$(ECHO) ""	
	@$(ECHO) "MAKE tools:"
	@$(ECHO) "  Board Information     = board.cfg"
	@$(ECHO) "  Module Information    = module.mk"
	@$(ECHO) "  Make options          = make.linux"
	@$(ECHO) "  Make Command          = mk.make"					
	@$(ECHO) ""
	@$(ECHO) "Configuration:"
	@$(ECHO) "  PLOBJDIR   = $(PLOBJDIR)"
	@$(ECHO) "  PLPRODOBJ  = $(PLPRODOBJ)"
	@$(ECHO) "  PLLIB      = $(PLLDLIBS)"
	@$(ECHO) "  PLLDFLAGS  = $(PLLDFLAGS)"
	@$(ECHO) "  PLCFLAGS   = $(PLCFLAGS)"	
	@$(ECHO) "  PLCPPFLAGS = $(PLCPPFLAGS)"	
	@$(ECHO) "  PLASFLAGS  = $(PLASFLAGS)"	
	@$(ECHO) "  PLDEFINE   = $(PLDEFINE)"	
	@$(ECHO) "  PLINCLUDE  = $(PLINCLUDE)"				
ifeq ($(BUILD_DEBUG),YES)
	@$(ECHO) "  PLDEBUG    = $(PLDEBUG)"
endif
	@$(ECHO) ""
	@$(ECHO) "Products:"

	@$(PLVERB)for prod in $(PLPRODS); do \
		$(ECHO) "  $$prod"; \
        done
	@$(ECHO) "Targets:"
	@$(ECHO) "  all        build all"
	@$(ECHO) "  clean      remove all"
	@$(ECHO) "  obj        build objects"
	@$(ECHO) "  lib        build library"
	@$(ECHO) "  demo       build demo app"
	@$(ECHO) "  rebuild    clean + all"
	@$(ECHO) ""

#
#rebuild:clean all
#
#
objlibclean:
	@for i in $(PLPRODS); do	\
		$(RM) $$i/*.a; 	\
	done
#
objclean: 
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i objclean; 	\
	done
#
obj:
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i obj || exit "$$?"; 	\
	done
	
install: 
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i install || exit "$$?"; 	\
	done
#
clean: 
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i clean; 	\
	done

demo: 	
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i demo || exit "$$?"; 	\
	done

lib: 
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i lib || exit "$$?"; 	\
	done
		
all: 
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i all || exit "$$?"; 	\
	done
	
rebuild: clean all