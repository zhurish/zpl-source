#############################################################################
# DEFINE
###########################################################################



OBJS += jaricom.o jcapimin.o jcapistd.o jcarith.o \
	jccoefct.o jccolor.o jcdctmgr.o jchuff.o jcinit.o \
	jcmainct.o jcmarker.o jcmaster.o jcomapi.o jcparam.o \
	jcprepct.o jcsample.o jctrans.o jdapimin.o jdapistd.o \
	jdarith.o jdatadst.o jdatasrc.o jdcoefct.o jdcolor.o \
	jddctmgr.o jdhuff.o jdinput.o jdmainct.o jdmarker.o \
	jdmaster.o jdmerge.o jdpostct.o jdsample.o jdtrans.o \
	jerror.o jfdctflt.o jfdctfst.o jfdctint.o jidctflt.o \
	jidctfst.o jidctint.o jquant1.o jquant2.o jutils.o \
	jmemmgr.o jmemnobs.o

OBJS += cjpeg.o djpeg.o jpegtran.o rdjpgcom.o wrjpgcom.o cdjpeg.o \
        rdcolmap.o rdswitch.o transupp.o rdppm.o wrppm.o rdgif.o wrgif.o \
        rdtarga.o wrtarga.o rdbmp.o wrbmp.o rdrle.o wrrle.o ckconfig.o

# library object files common to compression and decompression
#OBJS += jaricom.o jcomapi.o jutils.o jerror.o jmemmgr.o $(SYSDEPMEM)
# compression library object files
#OBJS += jcapimin.o jcapistd.o jcarith.o jctrans.o jcparam.o \
        jdatadst.o jcinit.o jcmaster.o jcmarker.o jcmainct.o jcprepct.o \
        jccoefct.o jccolor.o jcsample.o jchuff.o jcdctmgr.o jfdctfst.o \
        jfdctflt.o jfdctint.o
# decompression library object files
#OBJS += jdapimin.o jdapistd.o jdarith.o jdtrans.o jdatasrc.o \
        jdmaster.o jdinput.o jdmarker.o jdhuff.o jdmainct.o \
        jdcoefct.o jdpostct.o jddctmgr.o jidctfst.o jidctflt.o \
        jidctint.o jdsample.o jdcolor.o jquant1.o jquant2.o jdmerge.o

#ZPL_INCLUDE += -I$(PJSIP_ROOT)

#ZPLEX_INCLUDE += -I$(ZPL_INSTALL_ROOTFS_DIR)/include


#ZPL_DEFINE += -DUSE_GNUTLS
#ZPL_LDLIBS += -lgnutls -lgcrypt -lz

#############################################################################
# LIB
###########################################################################
LIBS = libjpeg.a
