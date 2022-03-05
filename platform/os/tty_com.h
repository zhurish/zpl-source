/*
 * tty_com.h
 *
 *  Created on: Jul 22, 2018
 *      Author: zhurish
 */

#ifndef __TTY_COM_H__
#define __TTY_COM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <termios.h>

#define TTY_COM_DEVNAME_MAX 64


enum tty_hwmode
{
	TTY_HWMODE_NONE = 0,
	TTY_HWMODE_RS232 = TTY_HWMODE_NONE,	//全双工
	TTY_HWMODE_RS485,					//半双工
	TTY_HWMODE_RS422,
};

enum flow_ctl_mode
{
	TTY_FLOW_CTL_NONE = 0,
	TTY_FLOW_CTL_SW = 1,
	TTY_FLOW_CTL_HW = 2,
};

enum parity_mode
{
	TTY_PARITY_NONE = 0,
	TTY_PARITY_EVEN,
	TTY_PARITY_ODD,
	TTY_PARITY_MARK,
	TTY_PARITY_SPACE,
};


enum tty_data_bit
{
	TTY_DATA_5BIT = 5,
	TTY_DATA_6BIT,
	TTY_DATA_7BIT,
	TTY_DATA_8BIT,
};

enum tty_stop_bit
{
	TTY_STOP_NONE = 0,
	TTY_STOP_1BIT,
	TTY_STOP_2BIT,
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
	zpl_char 	devname[TTY_COM_DEVNAME_MAX];
	int		fd;
	FILE	*fp;
	zpl_uint32 		speed;		// speed bit
	enum tty_data_bit 	databit;	// data bit
	enum tty_stop_bit	stopbit;	// stop bit
	enum parity_mode 	parity;		// parity
	enum flow_ctl_mode 	flow_control;// flow control

	tty_com_mode	mode;

	enum tty_hwmode	hwmode;

	struct termios termios;
	struct termios old_termios;

	int	(*tty_rw_enable)(zpl_bool);
	int	(*encapsulation)(zpl_uchar *, zpl_uint32, zpl_uchar *, zpl_uint32);
	int	(*decapsulation)(zpl_uchar *, zpl_uint32, zpl_uchar *, zpl_uint32);
};


struct tty_slip
{
	/* These are pointers to the malloc()ed frame buffers. */
	zpl_uchar		*slipbuf;		/* receiver buffer		*/
	zpl_uint32               sliplen;         /* received chars counter       */
	zpl_uint32               buffsize;       /* Max buffers sizes            */

	zpl_uint32		flags;		/* Flag values/ mode etc	*/
#define TTY_COM_SLF_INUSE	0		/* Channel in use               */
#define TTY_COM_SLF_ESCAPE	1               /* ESC received                 */
#define TTY_COM_SLF_ERROR	2               /* Parity, etc. error           */
#define TTY_COM_SLF_KEEPTEST	3		/* Keepalive test flag		*/
#define TTY_COM_SLF_OUTWAIT	4		/* is outpacket was flag	*/
};


extern zpl_bool tty_iscom(struct tty_com *com);
extern zpl_bool tty_isopen(struct tty_com *com);
extern int tty_com_open(struct tty_com *com);
extern int tty_com_close(struct tty_com *com);
extern int tty_com_update_option(struct tty_com *com);

extern int tty_com_rw_begin(struct tty_com *com, zpl_bool enable);
extern int tty_com_rw_end(struct tty_com *com, zpl_bool enable);

extern int tty_com_write(struct tty_com *com, zpl_uchar *buf, zpl_uint32 len);
extern int tty_com_read(struct tty_com *com, zpl_uchar *buf, zpl_uint32 len);

extern int tty_com_putc(struct tty_com *com, zpl_uchar c);
extern int tty_com_getc(struct tty_com *com, zpl_uchar *c);

extern int tty_com_slip_write(struct tty_com *com, zpl_uchar *p, zpl_uint32 len);
extern int tty_com_slip_read(struct tty_com *com, zpl_uchar *p, zpl_uint32 len);

extern int tty_com_slip_encapsulation(struct tty_slip *sl, zpl_uchar *s, zpl_uint32 len);
extern int tty_com_slip_decapsulation(struct tty_slip *sl, zpl_uchar *s, zpl_uint32 len);
extern int tty_com_slip_decode_byte(struct tty_slip *sl, zpl_uchar s);

#ifdef __cplusplus
}
#endif

#endif /* __TTY_COM_H__ */
