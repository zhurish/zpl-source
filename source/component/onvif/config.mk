#############################################################################
# DEFINE
###########################################################################
#
MODULEDIR = component/onvif
#
#
#
OBJS += soapC_001.o
OBJS += soapC_002.o
OBJS += soapC_003.o
OBJS += soapC_004.o
OBJS += soapC_005.o
OBJS += soapC_006.o
OBJS += soapC_007.o
OBJS += soapC_008.o
OBJS += soapC_009.o
OBJS += soapC_010.o
OBJS += soapC_011.o
OBJS += soapC_012.o
OBJS += soapC_013.o
OBJS += soapC_014.o
OBJS += soapC_015.o
OBJS += soapC_016.o
OBJS += soapC_017.o
OBJS += soapC_018.o
OBJS += soapC_019.o
OBJS += soapC_020.o
OBJS += soapC_021.o
OBJS += soapC_022.o
OBJS += soapC_023.o
OBJS += soapC_024.o
OBJS += soapC_025.o
OBJS += soapC_026.o
OBJS += soapC_027.o
OBJS += soapC_028.o
OBJS += soapC_029.o
OBJS += soapC_030.o
OBJS += soapC_031.o
OBJS += soapC_032.o
OBJS += soapC_033.o
OBJS += soapC_034.o
OBJS += soapC_035.o
OBJS += soapC_036.o
OBJS += soapC_037.o
OBJS += soapC_038.o

OBJS += soapClient.o
OBJS += soapServer.o
OBJS += soapUtil.o

ifeq ($(strip $(ZPL_ONVIF_SSL)),true)
OBJS += stdsoap2_ssl.o
else
OBJS += stdsoap2.o
endif
#
#OBJS += stdsoap2_ck.o IPCOM

OBJS += dom.o
OBJS += threads.o
OBJS += wsaapi.o
OBJS += wsddapi.o

OBJS += httpda.o
OBJS += httpform.o
OBJS += httpget.o
OBJS += httpmd5.o
OBJS += httppipe.o
OBJS += httppost.o

#OBJS += httpposttest.o
#OBJS += httpdatest.o
#OBJS += httpmd5test.o
#OBJS += httpgettest.o

OBJS += logging.o
OBJS += mq.o
OBJS += plugin.o
OBJS += sessions.o

ifeq ($(strip $(ZPL_ONVIF_SSL)),true)
OBJS += md5evp.o
OBJS += mecevp.o
OBJS += smdevp.o

OBJS += wsse2api.o
OBJS += wsseapi.o
OBJS += wstapi.o
endif


OBJS += onvif_api.o
OBJS += onvif_server.o
OBJS += onvif_util.o
OBJS += onvif_test.o
#############################################################################
# LIB
###########################################################################
LIBS = libonvif.a

