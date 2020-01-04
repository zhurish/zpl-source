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

typedef enum
{
	TTY_COM_MODE_NONE = 0,
	TTY_COM_MODE_RAW = TTY_COM_MODE_NONE,
	TTY_COM_MODE_SLIP,
	TTY_COM_MODE_CSLIP,
	TTY_COM_MODE_PPP,
} tty_com_mode;


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

	tty_com_mode	mode;

	struct termios termios;
	struct termios old_termios;

	int	(*encapsulation)(char *, int, char *, int);
	int	(*decapsulation)(char *, int, char *, int);
};


struct tty_slip
{
	/* These are pointers to the malloc()ed frame buffers. */
	unsigned char		*slipbuf;		/* receiver buffer		*/
	int               sliplen;         /* received chars counter       */
	int               buffsize;       /* Max buffers sizes            */

	unsigned int		flags;		/* Flag values/ mode etc	*/
#define TTY_COM_SLF_INUSE	0		/* Channel in use               */
#define TTY_COM_SLF_ESCAPE	1               /* ESC received                 */
#define TTY_COM_SLF_ERROR	2               /* Parity, etc. error           */
#define TTY_COM_SLF_KEEPTEST	3		/* Keepalive test flag		*/
#define TTY_COM_SLF_OUTWAIT	4		/* is outpacket was flag	*/
};


extern BOOL tty_iscom(struct tty_com *com);
extern BOOL tty_isopen(struct tty_com *com);
extern int tty_com_open(struct tty_com *com);
extern int tty_com_close(struct tty_com *com);
extern int tty_com_update_option(struct tty_com *com);
extern int tty_com_write(struct tty_com *com, char *buf, int len);
extern int tty_com_read(struct tty_com *com, char *buf, int len);

extern int tty_com_putc(struct tty_com *com, unsigned char c);
extern int tty_com_getc(struct tty_com *com, unsigned char *c);

extern int tty_com_slip_write(struct tty_com *com, char *p, int len);
extern int tty_com_slip_read(struct tty_com *com, char *p, int len);

extern int tty_com_slip_encapsulation(struct tty_slip *sl, unsigned char *s, int len);
extern int tty_com_slip_decapsulation(struct tty_slip *sl, unsigned char *s, int len);
extern int tty_com_slip_decode_byte(struct tty_slip *sl, unsigned char s);

#endif /* __TTY_COM_H__ */
