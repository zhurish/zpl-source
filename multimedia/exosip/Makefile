include $(ZPL_MAKE_DIR)/makelinux.mk

include config.mk


OBJS_DIR = $(BASE_ROOT)/$(ZPL_OBJ_DIR)/$(MODULEDIR)
SRC_DIR = $(BASE_ROOT)/$(MODULEDIR)
LIB_DIR = $(BASE_ROOT)/$(ZPL_LIB_DIR)

osip_OBJS  := $(addprefix $(OBJS_DIR)/osip/,$(libosip2_obj))
osipparser_OBJS  := $(addprefix $(OBJS_DIR)/osipparser/,$(libosipparser2_obj))
exosip_OBJS  := $(addprefix $(OBJS_DIR)/exosip/,$(libexosip2_obj))

$(OBJS_DIR)/osip/%.o: $(SRC_DIR)/libosip2-5.2.0/src/osip2/%.c
	@if test ! -d $(OBJS_DIR)/osip ; \
		then \
		$(MKDIR) -p $(OBJS_DIR)/osip ; \
	fi
	@$(ECHO) CC '$(OSIP_CFLAGS) $(OSIP_DEFINE) $(OSIP_DEBUG) $(OSIP_INCLUDE)' $@
	@$(CC) -fPIC $(OSIP_CFLAGS) $(OSIP_DEFINE) $(OSIP_DEBUG) -c  $< -o $@ $(OSIP_INCLUDE)  

$(OBJS_DIR)/osipparser/%.o: $(SRC_DIR)/libosip2-5.2.0/src/osipparser2/%.c
	@if test ! -d $(OBJS_DIR)/osipparser ; \
		then \
		$(MKDIR) -p $(OBJS_DIR)/osipparser ; \
	fi
	@$(ECHO) CC '$(OSIP_CFLAGS) $(OSIP_DEFINE) $(OSIP_DEBUG) $(OSIP_INCLUDE)' $@
	@$(CC) -fPIC $(OSIP_CFLAGS) $(OSIP_DEFINE) $(OSIP_DEBUG) -c  $< -o $@ $(OSIP_INCLUDE) 

$(OBJS_DIR)/exosip/%.o: $(SRC_DIR)/libexosip2-5.2.0/src/%.c
	@if test ! -d $(OBJS_DIR)/exosip ; \
		then \
		$(MKDIR) -p $(OBJS_DIR)/exosip ; \
	fi
	@$(ECHO) CC '$(OSIP_CFLAGS) $(OSIP_DEFINE) $(OSIP_DEBUG) $(OSIP_INCLUDE)' $@
	@$(CC) -fPIC $(OSIP_CFLAGS) $(OSIP_DEFINE) $(OSIP_DEBUG) -c  $< -o $@ $(OSIP_INCLUDE) 


$(libosip2_lib) : $(osip_OBJS)
	@$(AR) -rs $(libosip2_lib) $(osip_OBJS)

$(libosip2_libso) : $(osip_OBJS)
	@$(CC) -shared -o -fPIC -o $(libosip2_libso) $(osip_OBJS)


$(libosipparser2_lib) : $(osipparser_OBJS)
	@$(AR) -rs $(libosipparser2_lib) $(osipparser_OBJS)

$(libosipparser2_libso) :  $(osipparser_OBJS)
	@$(CC) -shared -fPIC -o $(libosipparser2_libso) $(osipparser_OBJS)



$(libexosip2_lib) : $(exosip_OBJS)
	@$(AR) -rs $(libexosip2_lib) $(exosip_OBJS)

$(libexosip2_libso) : $(exosip_OBJS)
	@$(CC) -shared -fPIC -o $(libexosip2_libso) $(exosip_OBJS)


#
lib: $(libosip2_lib) $(libosipparser2_lib) $(libexosip2_lib) 
libso: $(libosip2_libso) $(libosipparser2_libso) $(libexosip2_libso) 


obj: $(osip_OBJS) $(osipparser_OBJS) $(exosip_OBJS)

install:  lib
	@install -d $(LIB_DIR)
	@install -m 755 $(libosip2_lib) $(LIB_DIR)
	@install -m 755 $(libosipparser2_lib) $(LIB_DIR)
	@install -m 755 $(libexosip2_lib) $(LIB_DIR)

clean: objclean
	@$(RM) $(LIB_DIR)/$(libosip2_lib)
	@$(RM) $(libosip2_lib)
	@$(RM) $(LIB_DIR)/$(libosipparser2_lib)
	@$(RM) $(libosipparser2_lib)
	@$(RM) $(LIB_DIR)/$(libexosip2_lib)
	@$(RM) $(libexosip2_lib)

objclean: 
	@$(RM) $(OBJS_DIR)/* -rf

all: lib install

.PHONY:	obj all lib install objclean clean libso