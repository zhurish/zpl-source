/*
 * modem_product.c
 *
 *  Created on: Oct 23, 2018
 *      Author: zhurish
 */


#include "zebra.h"
#include "log.h"
#include "memory.h"
#include "str.h"
#include "vty.h"
#include "os_util.h"
#include "tty_com.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_driver.h"
#include "modem_usb_driver.h"
#include "modem_pppd.h"
#include "modem_product.h"


static struct tty_com ec20_attty =
{
	.devname = "/dev/ttyUSB2",
	.speed	= 115200,
	.databit = DATA_8BIT,
	.stopbit = STOP_1BIT,
	.parity = PARITY_NONE,
	.flow_control = FLOW_CTL_NONE,
	.mode = TTY_COM_MODE_RAW,
};

static struct tty_com ec20_pppd =
{
	.devname = "/dev/ttyUSB3",
	.speed	= 115200,
	.databit = DATA_8BIT,
	.stopbit = STOP_1BIT,
	.parity = PARITY_NONE,
	.flow_control = FLOW_CTL_NONE,
	.mode = TTY_COM_MODE_RAW,
};

static modem_driver_t ec20_driver =
{
	.id				= THIS_MODEM_MODULE,
	.vendor 		= 0X2C7C,
	.product 		= 0X0125,
	.module_name 	= "Quectel EC20",
	.tty_type		= TTY_USB,
	.ttyidmax		= 4,
	.ttyseq			= {TTY_USB_TEST, TTY_USB_DIAL, TTY_USB_AT, TTY_USB_PPP, 0, 0},
	.attty 			= &ec20_attty,
	.pppd 			= &ec20_pppd,
/*	.md_probe = NULL,
	.md_init = NULL,
	.md_reboot = NULL,*/
};




int modem_product_init()
{
	//zlog_warn(MODULE_MODEM, "----modem-channel profile : %s",modem_serial_channel_name(&ec20_driver));
	modem_driver_register(&ec20_driver);
	return OK;
}
