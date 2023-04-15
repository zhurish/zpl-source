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
ZPLPRODS_OBJ = $(ZPLPRODS)
ZPLPRODS_OBJ += $(BASE_ROOT)/empty
#
#
#
#
#ifeq ($(MKDIR_TEST),true)
#define _MODULE_OBJ
#TMP_OBJ := $$(shell basename $(strip $(1)))
#MK_OBJ := $$(shell $(ZPL_MAKE_DIR) -p  $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(TMP_OBJ) $(BASE_ROOT)/$(ZPL_LIB_DIR))
#endef
#
#$(foreach objdir,$(ZPLPRODS_OBJ), $(eval $(call _MODULE_OBJ,$(objdir))))
#endif
#
#

#
#
#
#
.PHONY:	obj all lib clean exlib exlib_clean rebuild install objclean help usage  packet debug prebuilts make_prepare kernel_module
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
	@$(ECHO) "  ZPLOBJDIR   = $(ZPLOBJDIR)"
	@$(ECHO) "  ZPLPRODOBJ  = $(ZPLPRODOBJ)"
	@$(ECHO) "  ZPLLIB      = $(ZPLLDLIBS)"
	@$(ECHO) "  ZPLLDFLAGS  = $(ZPLLDFLAGS)"
	@$(ECHO) "  ZPLCFLAGS   = $(ZPLCFLAGS)"	
	@$(ECHO) "  ZPLCPPFLAGS = $(ZPLCPPFLAGS)"	
	@$(ECHO) "  ZPLASFLAGS  = $(ZPLASFLAGS)"	
	@$(ECHO) "  ZPLDEFINE   = $(ZPLDEFINE)"	
	@$(ECHO) "  ZPLINCLUDE  = $(ZPLINCLUDE)"				
ifeq ($(ZPL_BUILD_DEBUG),YES)
	@$(ECHO) "  ZPLDEBUG    = $(ZPLDEBUG)"
endif
	@$(ECHO) ""
	@$(ECHO) "Products:"

	@$(ZPLVERB)for prod in $(ZPLPRODS); do \
		$(ECHO) "  $$prod"; \
        done
	@$(ZPLVERB)for prod in $(ZPLEX_DIR); do \
		$(ECHO) "EX : $$prod"; \
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
	./moduletypes.sh header make $(PLATFORM_BASE_ROOT)
	@for i in $(ZPLPRODS); do	\
		./moduletypes.sh def $$i $(PLATFORM_BASE_ROOT); \
	done
	./moduletypes.sh tail make $(PLATFORM_BASE_ROOT)
#	
make_prepare:
	./moduletypes.sh header make $(PLATFORM_BASE_ROOT)
	@for i in $(ZPLPRODS); do	\
		$(MAKE) -C $$i make_prepare || exit "$$?"; 	\
	done
	./moduletypes.sh tail make $(PLATFORM_BASE_ROOT)
#
objclean:
	@for i in $(ZPLPRODS); do	\
		$(MAKE) -C $$i objclean || exit "$$?"; 	\
	done
#
obj:
	@for i in $(ZPLPRODS); do	\
		$(MAKE) -C $$i obj || exit "$$?"; 	\
	done
	
install: 
	@for i in $(ZPLPRODS); do	\
		$(MAKE) -C $$i install || exit "$$?"; 	\
	done
	install -d ${ZPL_INSTALL_ETC_DIR}
	install -m 755 upgradeapp.sh ${ZPL_INSTALL_ETC_DIR} 

#
clean: 
	@for i in $(ZPLPRODS); do	\
		$(MAKE) -C $$i clean; 	\
	done

lib: 
	@for i in $(ZPLPRODS); do	\
		$(MAKE) -C $$i lib || exit "$$?"; 	\
	done
		
all: 
	@for i in $(ZPLEX_DIR); do	\
		$(MAKE) -C $$i all || exit "$$?"; 	\
		$(MAKE) -C $$i install || exit "$$?"; 	\
	done
	@for i in $(ZPLPRODS); do	\
		$(MAKE) -C $$i all || exit "$$?"; 	\
	done

#
LEXLIBDIR=${EXLIBDIR}
ifeq ($(LEXLIBDIR),)
exlib:
	@for i in $(ZPLEX_DIR); do	\
		$(MAKE) -C $$i all || exit "$$?"; 	\
		$(MAKE) -C $$i install || exit "$$?"; 	\
	done

exlib_clean:
	@for i in $(ZPLEX_DIR); do	\
		$(MAKE) -C $$i clean; 	\
	done
else
exlib:
	$(MAKE) -C $(EXTERNSION_BASE_ROOT)/${LEXLIBDIR} all || exit "$$?"; 
	$(MAKE) -C $(EXTERNSION_BASE_ROOT)/${LEXLIBDIR} install || exit "$$?"; 

exlib_clean:
	$(MAKE) -C $(EXTERNSION_BASE_ROOT)/${LEXLIBDIR} clean;
endif


debug:
	@$(ECHO) " CROSS_COMPILE: $(CROSS_COMPILE)"
	@$(ECHO) " ZPL_BUILD_ARCH: $(ZPL_BUILD_ARCH)"
	@$(ECHO) " ARCH_TYPE: $(ARCH_TYPE)"
	@$(ECHO) " GCC_TYPE: $(GCC_TYPE)"
	@$(ECHO) " ZPL_BUILD_OPENWRT: $(ZPL_BUILD_OPENWRT)"
	@$(ECHO) " ARCH_DEBUG: $(ARCH_DEBUG)"	
	@$(ECHO) " ZPL_COMPILE_OPTIONS: $(ZPL_COMPILE_OPTIONS)"


kernel_module:
	@for i in $(ZPLPRODS_KERNEL); do	\
		$(MAKE) -C $$i all || exit "$$?"; 	\
		$(MAKE) -C $$i install || exit "$$?"; 	\
	done



rebuild: clean all

packet:
	$(CD) $(ZPL_RELEASEDIR); $(MKDIR) install; $(CP) -arf sbin install/; \
		$(CP) -arf bin install/; $(CP) -arf etc install/ $(CP) -arf lib install/; \
		$(CP) -arf usr install/;$(CP) -arf www install/; \
		$(TAR) -zcvf install.tar.gz sbin bin etc lib usr