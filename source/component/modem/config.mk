#############################################################################
# DEFINE
###########################################################################
MODULEDIR = component/modem
#OS
OBJS += modem.o
OBJS += modem_bitmap.o
OBJS += modem_event.o
OBJS += modem_attty.o
OBJS += modem_client.o

OBJS += modem_state.o
OBJS += modem_api.o
OBJS += modem_serial.o
OBJS += modem_process.o
OBJS += modem_control.o
OBJS += modem_error.o
OBJS += modem_machine.o
OBJS += modem_driver.o
OBJS += modem_message.o
OBJS += modem_mgtlayer.o

OBJS += modem_split.o
OBJS += modem_string.o

OBJS += modem_proxy.o


OBJS += modem_usim.o
OBJS += modem_dialog.o
OBJS += modem_pppd.o
OBJS += modem_dhcp.o
OBJS += modem_qmi.o

OBJS += modem_operators.o
OBJS += modem_usb_driver.o


OBJS += modem_atcmd.o
OBJS += modem_product.o
OBJS += modem_main.o

ifeq ($(strip $(ZPL_SHELL_MODULE)),true)
OBJS += cmd_modem.o
endif
#OBJS += test.o
#############################################################################
# LIB
###########################################################################
LIBS = libmodem.a
