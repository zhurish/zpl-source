/*
 * modem_driver.h
 *
 *  Created on: Jul 25, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_DRIVER_H__
#define __MODEM_DRIVER_H__




#define MODEM_DRIVER_NAME_MAX				64

#define THIS_MODEM_MODULE				0

struct modem_driver;


typedef struct at_cmd_s
{
	const char *atcmd;
	const char *atres;
}at_cmd_t;

typedef struct modem_cmd_s
{
	//base
	int (*md_echo_cmd)(struct modem_driver *);

	int (*md_swreset_cmd)(struct modem_driver *);
	int (*md_reboot_cmd)(struct modem_driver *);
	int (*md_save_cmd)(struct modem_driver *);

	int (*md_open_cmd)(struct modem_driver *, int);
	int (*md_factory_cmd)(struct modem_driver *, char *);
	int (*md_product_cmd)(struct modem_driver *, char *);

	int (*md_vetsion_cmd)(struct modem_driver *, char *);
	int (*md_serial_number_cmd)(struct modem_driver *, char *);

	int (*md_signal_cmd)(struct modem_driver *, int *, int *);

	/*
	 * PPPD
	 */
	char (*md_pppd_connect)(struct modem_driver *, char *, int);
	char (*md_pppd_disconnect)(struct modem_driver *, char *, int);

}modem_cmd_t;

/*
 * USB USB转串口 类型
 */
typedef enum
{
	TTY_USB  = 0,
	TTY_CAM,
	TTY_S,
}ttytype_en;

#define USB_SERIAL_BASE		"ttyUSB"
#define CAM_SERIAL_BASE		"ttyCAM"
#define UART_SERIAL_BASE	"ttyS"


/*
 * 串口功能 类型
 */
typedef enum
{
	TTY_USB_TEST = 1,
	TTY_USB_DIAL,
	TTY_USB_AT,
	TTY_USB_PPP,

	TTY_USB_MAX = 6,
}ttyseq_en;


typedef struct modem_driver
{
	int				id;
	int				bus;
	int				device;
	int				vendor;
	int				product;
	char			module_name[MODEM_DRIVER_NAME_MAX];

	ttytype_en		tty_type;
	int				ttyidmax;

	ttyseq_en		ttyseq[TTY_USB_MAX];

	struct tty_com 	*dialog;
	struct tty_com 	*attty;
	struct tty_com 	*pppd;
	struct tty_com 	*usetty;

	char			eth_name[MODEM_DRIVER_NAME_MAX];
	char			eth_name_sec[MODEM_DRIVER_NAME_MAX];

	void			*client;

	int				(*modem_driver_init)(struct modem_driver *);
	int				(*modem_driver_probe)(struct modem_driver *);
	int				(*modem_driver_exit)(struct modem_driver *);
	int				(*modem_driver_reboot)(struct modem_driver *);

	modem_cmd_t		atcmd;

}modem_driver_t;


extern int modem_driver_register(modem_driver_t *);
extern int modem_driver_unregister(modem_driver_t *);

extern modem_driver_t * modem_driver_lookup(int vendor, int product);

extern int modem_driver_remove(int vendor, int product);
extern int modem_driver_inster(int vendor, int product);

extern int modem_driver_tty_probe(modem_driver_t *, char *devname[]);

extern int modem_driver_hw_channel(int vendor, int product, u_int8 *hw_channel);

extern int modem_driver_init(modem_driver_t *);
extern int modem_driver_probe(modem_driver_t *);
extern int modem_driver_exit(modem_driver_t *);
extern int modem_driver_reboot(modem_driver_t *);




#endif /* __MODEM_DRIVER_H__ */
