/*
 * test.c
 *
 *  Created on: Jul 25, 2018
 *      Author: zhurish
 */

#include "os_include.h"
#include <zpl_include.h>
#include "lib_include.h"
#include "nsm_include.h"

#include "modem.h"
#include "modem_client.h"
#include "modem_driver.h"
#include "modem_pppd.h"


#if 0
struct tty_com ec20_attty =
{
	.devname = "/dev/ttyUSB2",
	.speed	= 115200,
	.databit = DATA_8BIT,
	.stopbit = STOP_1BIT,
	.parity = PARITY_NONE,
	.flow_control = FLOW_CTL_NONE,
};

struct tty_com ec20_pppd =
{
	.devname = "/dev/ttyUSB3",
	.speed	= 115200,
	.databit = DATA_8BIT,
	.stopbit = STOP_1BIT,
	.parity = PARITY_NONE,
	.flow_control = FLOW_CTL_NONE,
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





int modem_test_init()
{
	//zlog_warn(MODULE_MODEM, "----modem-channel profile : %s",modem_serial_channel_name(&ec20_driver));
	return modem_driver_register(&ec20_driver);
}



#endif









static modem_client_t *modem_client = NULL;

int modem_cmd_test_open()
{
	if(!modem_client)
	{
		modem_client = modem_client_lookup_api(0X2C7C, 0X0125);
		if(modem_client)
			modem_attty_open(modem_client);
	}
	return 0;
}

int modem_cmd_test_close()
{
	if(modem_client)
	{
		modem_attty_close(modem_client);
		modem_client = NULL;
	}
	return 0;
}

int modem_cmd_test(struct vty *vty)
{
	char buf[1024];
	os_memset(buf, 0, sizeof(buf));

	if(!modem_client)
	{
		modem_client = modem_client_lookup_api(0X2C7C, 0X0125);
		if(!modem_client)
			return OK;
	}
	modem_atcmd_isopen(modem_client);

	/*
	 * AT+CGMI
	 */
	modem_product_atcmd_get(modem_client);
	modem_product_id_atcmd_get(modem_client);
	modem_serial_number_atcmd_get(modem_client);
	/*
	 * AT+CGMM
	 */
	modem_signal_atcmd_get(modem_client);
	/*
	 * AT+CGMR
	 */
	modem_version_atcmd_get(modem_client);
#if 0
	/*
	 * AT+CGSN
	 */
	modem_imei_atcmd_get(modem_client);
	/*
	 * AT+CIMI
	 */
	modem_imsi_atcmd_get(modem_client);

	/*
	 * AT+CCID
	 */
	modem_simid_atcmd_get(modem_client);
	/*
	 * AT+cfun
	 */
	modem_status_atcmd_get(modem_client);
#endif
	//modem_client_attty_respone(modem_client, 60, buf, sizeof(buf), "at\r\n");
	//vty_out(vty, "--> %s%s",buf, VTY_NEWLINE);

	//modem_client_attty_respone(modem_client, 60, buf, sizeof(buf), "ati\r\n");
	//vty_out(vty, "--> %s%s",buf, VTY_NEWLINE);
	return 0;
}








