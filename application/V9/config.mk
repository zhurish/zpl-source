#############################################################################
# DEFINE
###########################################################################
#OS
#APP_OBJ_DIR = $(APP_ROOT)/X5-B/mgt
APP_OBJ_DIR = application/V9

APP_OBJS += v9_serial.o
APP_OBJS += v9_slipnet.o
APP_OBJS += v9_cmd.o
APP_OBJS += v9_board.o
APP_OBJS += v9_util.o

APP_OBJS += v9_video_api.o
APP_OBJS += v9_video_board.o
APP_OBJS += v9_video.o

APP_OBJS += v9_video_sdk.o
APP_OBJS += v9_video_user.o

#APP_OBJS += x5_b_ctl.o

#APP_OBJS += x5b_dbase.o
#APP_OBJS += x5b_card.o
#APP_OBJS += x5b_facecard.o

APP_OBJS += application.o
#############################################################################
# LIB
###########################################################################
APP_LIBS = libv9ctl.a
