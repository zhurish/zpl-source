/*
 * tty_com.h
 *
 *  Created on: Jul 22, 2018
 *      Author: zhurish
 */

#ifndef __TTY_COM_H__
#define __TTY_COM_H__

#include <termios.h>

#define TTY_COM_DEVNAME_MAX 64

enum flow_ctl_mode
{
	FLOW_CTL_NONE = 0,
	FLOW_CTL_SW = 1,
	FLOW_CTL_HW = 2,
};

enum parity_mode
{
	PARITY_NONE = 0,
	PARITY_EVEN,
	PARITY_ODD,
	PARITY_MARK,
	PARITY_SPACE,
};


enum tty_data_bit
{
	DATA_5BIT = 5,
	DATA_6BIT,
	DATA_7BIT,
	DATA_8BIT,
};

enum tty_stop_bit
{
	STOP_NONE = 0,
	STOP_1BIT,
	STOP_2BIT,
};


struct tty_com
{
	char 	devname[TTY_COM_DEVNAME_MAX];
	int		fd;
	FILE	*fp;
	u_int32 		speed;		// speed bit
	enum tty_data_bit 	databit;	// data bit
	enum tty_stop_bit	stopbit;	// stop bit
	enum parity_mode 	parity;		// parity
	enum flow_ctl_mode 	flow_control;// flow control

	struct termios termios;
	struct termios old_termios;

	int	(*encapsulation)(char *, int, char *, int);
	int	(*decapsulation)(char *, int, char *, int);
};

extern BOOL tty_iscom(struct tty_com *com);
extern int tty_com_open(struct tty_com *com);
extern int tty_com_close(struct tty_com *com);
extern int tty_com_update_option(struct tty_com *com);
extern int tty_com_write(struct tty_com *com, char *buf, int len);
extern int tty_com_read(struct tty_com *com, char *buf, int len);

#endif /* __TTY_COM_H__ */
