#############################################################################
# DEFINE
###########################################################################

ifeq ($(strip $(ZPL_APP_X5_MODULE)),true)
    APP_DIR += X5-B
endif
ifeq ($(strip $(ZPL_APP_V9_MODULE)),true)
    APP_DIR += V9
endif

#OS
#############################################################################
# LIB
###########################################################################