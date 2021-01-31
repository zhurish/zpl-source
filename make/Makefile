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
.PHONY:	obj all demo lib clean rebuild install help usage objclean packet debug prebuilts make_prepare
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
ifeq ($(PL_BUILD_DEBUG),YES)
	@$(ECHO) "  PLDEBUG    = $(PLDEBUG)"
endif
	@$(ECHO) ""
	@$(ECHO) "Products:"

	@$(PLVERB)for prod in $(PLPRODS); do \
		$(ECHO) "  $$prod"; \
        done
	@$(PLVERB)for prod in $(PLEX_DIR); do \
		$(ECHO) "EX ：$$prod"; \
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
prebuilts:
	./moduletypes.sh header make $(PLATFORM_ROOT)
	@for i in $(PLPRODS); do	\
		./moduletypes.sh def $$i $(PLATFORM_ROOT); \
	done
	./moduletypes.sh tail make $(PLATFORM_ROOT)
#	
make_prepare:
	./moduletypes.sh header make $(PLATFORM_ROOT)
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i make_prepare || exit "$$?"; 	\
	done
	./moduletypes.sh tail make $(PLATFORM_ROOT)
#
objclean:
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i objclean || exit "$$?"; 	\
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
	install -d ${DSTETCDIR}
	install -m 755 upgradeapp.sh ${DSTETCDIR} 

#
clean: 
	@for i in $(PLEX_DIR); do	\
		$(MAKE) -C $$i clean; 	\
	done
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i clean; 	\
	done

demo: 	

	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i demo || exit "$$?"; 	\
	done

lib: 
	@for i in $(PLEX_DIR); do	\
		$(MAKE) -C $$i all || exit "$$?"; 	\
		$(MAKE) -C $$i install || exit "$$?"; 	\
	done
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i lib || exit "$$?"; 	\
	done
		
all: 
	@for i in $(PLEX_DIR); do	\
		$(MAKE) -C $$i all || exit "$$?"; 	\
		$(MAKE) -C $$i install || exit "$$?"; 	\
	done
	@for i in $(PLPRODS); do	\
		$(MAKE) -C $$i all || exit "$$?"; 	\
	done

debug:
	@$(ECHO) " CROSS_COMPILE: $(CROSS_COMPILE)"
	@$(ECHO) " PL_BUILD_TYPE: $(PL_BUILD_TYPE)"
	@$(ECHO) " ARCH_TYPE: $(ARCH_TYPE)"
	@$(ECHO) " GCC_TYPE: $(GCC_TYPE)"
	@$(ECHO) " PL_BUILD_OPENWRT: $(PL_BUILD_OPENWRT)"
	@$(ECHO) " ARCH_DEBUG: $(ARCH_DEBUG)"	
	@$(ECHO) " PL_COMPILE_OPTIONS: $(PL_COMPILE_OPTIONS)"


rebuild: clean all

packet:
	$(CD) $(RELEASEDIR); $(MKDIR) install; $(CP) -arf sbin install/; \
		$(CP) -arf bin install/; $(CP) -arf etc install/ $(CP) -arf lib install/; \
		$(CP) -arf usr install/;$(CP) -arf www install/; \
		$(TAR) -zcvf install.tar.gz sbin bin etc lib usr