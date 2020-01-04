#############################################################################
# DEFINE
###########################################################################
#OS
#APP_OBJ_DIR = $(APP_ROOT)/X5-B/mgt
APP_OBJ_DIR = application/X5-B/mgt

APP_OBJS += x5_b_global.o
APP_OBJS += x5_b_app.o
APP_OBJS += x5_b_util.o
APP_OBJS += x5_b_upgrade.o
APP_OBJS += x5_b_call.o
APP_OBJS += x5_b_io.o

APP_OBJS += x5_b_json.o

APP_OBJS += x5_b_web.o

APP_OBJS += x5_b_ctl.o

APP_OBJS += x5b_dbase.o
APP_OBJS += x5b_card.o
APP_OBJS += x5b_facecard.o

APP_OBJS += x5_b_test.o
#############################################################################
# LIB
###########################################################################
APP_LIBS = libx5-b-mgt.a
