/*
 * tty_com.c
 *
 *  Created on: Jul 22, 2018
 *      Author: zhurish
 */

#include <zebra.h>
#include "thread.h"
#include "command.h"
#include "hash.h"
#include "log.h"
#include "memory.h"
#include "pqueue.h"
#include "sigevent.h"
#include <termios.h>

#include "tty_com.h"


/* SLIP特殊字符 */
#define TTY_COM_SLIP_END 0XC0     /*标明包结束*/
#define TTY_COM_SLIP_ESC 0XDB     /*标明字节填充*/
#define TTY_COM_SLIP_ESC_END 0XDC /*ESC ESC_END用于包中数据和和END相同时的转意字符*/
#define TTY_COM_SLIP_ESC_ESC 0XDD /*ESC ESC_ESC用于包中数据和和ESC相同时的转意字符*/

/* SLIP protocol ospl_ucharacters. */
#define TTY_COM_END             0300		/* indicates end of frame	*/
#define TTY_COM_ESC             0333		/* indicates byte stuffing	*/
#define TTY_COM_ESC_END         0334		/* ESC ESC_END means END 'data'	*/
#define TTY_COM_ESC_ESC         0335		/* ESC ESC_ESC means ESC 'data'	*/


struct tty_speed_table
{
	ospl_uint32 speed;
	ospl_uint32 p;
};

struct tty_speed_table speed_table[] =
{
	{1800, B1800},
	{2400, B2400},
	{4800, B4800},
	{9600, B9600},
	{19200, B19200},
	{38400, B38400},
	{57600, B57600},
	{115200, B115200},
	{460800, B460800},
};

static int _tty_com_speed(struct tty_com *com)
{
	ospl_uint32 i = 0;
	for(i = 0; i < array_size(speed_table); i++)
	{
		if(com->speed == speed_table[i].speed)
		{
			cfsetospeed(&com->termios, speed_table[i].p);
			cfsetispeed(&com->termios, speed_table[i].p);
			return 0;
		}
	}
	cfsetospeed(&com->termios, B115200);
	cfsetispeed(&com->termios, B115200);
	return 0;
}


static int _tty_com_flow_control(struct tty_com *com)
{
	switch(com->flow_control)
	{
	case FLOW_CTL_SW:
		com->termios.c_iflag |= (IXON|IXOFF);
		break;
	case FLOW_CTL_HW:
		com->termios.c_oflag |= (IXON);
		break;
	default:
		com->termios.c_iflag &= ~(IXON);
	}
	return 0;
}

static int _tty_com_parity(struct tty_com *com)
{
	struct termios *termios;
	termios = &com->termios;
	switch(com->parity)
	{
	case PARITY_NONE:
		termios->c_cflag &= ~PARENB;
		break;
	case PARITY_EVEN:////偶校验
		termios->c_cflag |= PARENB;
		termios->c_cflag &= ~PARODD;
		termios->c_iflag |= (INPCK | ISTRIP);
		break;
	case PARITY_ODD:////奇校验
		termios->c_cflag |= PARENB;
		termios->c_cflag |= PARODD;
		termios->c_iflag |= (INPCK | ISTRIP);
		break;
	case PARITY_MARK:
		//termios->c_cflag |= CS8;
		break;
	case PARITY_SPACE:
		//termios->c_cflag |= CS8;
		break;
	default:
		termios->c_cflag &= ~PARENB;
	}
	return 0;
}

static int _tty_com_stopbit(struct tty_com *com)
{
	struct termios *termios;
	termios = &com->termios;
	switch(com->stopbit)
	{
	case STOP_2BIT:
		termios->c_cflag |= (CSTOPB);
		break;
	case STOP_1BIT:
		termios->c_cflag &= ~(CSTOPB);
		break;
	default:
		termios->c_cflag &= ~(CSTOPB);
	}
	return 0;
}

static int _tty_com_databit(struct tty_com *com)
{
	struct termios *termios;
	termios = &com->termios;
	switch(com->databit)
	{
	case DATA_5BIT:
		termios->c_cflag &= ~CSIZE;
		termios->c_cflag |= CS5;
		break;
	case DATA_6BIT:
		termios->c_cflag &= ~CSIZE;
		termios->c_cflag |= CS6;
		break;
	case DATA_7BIT:
		termios->c_cflag &= ~CSIZE;
		termios->c_cflag |= CS7;
		break;
	case DATA_8BIT:
		termios->c_cflag &= ~CSIZE;
		termios->c_cflag |= CS8;
		break;
	default:
		termios->c_cflag |= CS8;
	}
	return 0;
}

static int _tty_com_option(struct tty_com *com)
{
	if(com && com->fd)
	{
		struct termios Opt;
		struct termios *termios;
		termios = &com->termios;
		bzero( &Opt, sizeof( struct termios ));
		bzero( termios, sizeof( struct termios ));

		//tcgetattr(com->fd, &Opt);
		memcpy( termios, &Opt, sizeof( struct termios ));
		/*步骤一，设置字符大小*/

#if 1
		termios->c_cflag |= CLOCAL | CREAD;
		//cfmakeraw
		termios->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR
					| ICRNL | IXON);
		termios->c_oflag &= ~OPOST;
		termios->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
		termios->c_cflag &= ~(CSIZE | PARENB);
		termios->c_cflag |= CS8;

		cfmakeraw(termios);

		////////
		termios->c_cflag ^= (CLOCAL | HUPCL);

		//////////
		termios->c_cc[VTIME] = 0;
		termios->c_cc[VMIN] = 1;
		termios->c_cc[VEOF] = 1;
#else
	    termios->c_cflag &= ~(CBAUD | CSIZE | CSTOPB | CLOCAL | PARENB);
	    termios->c_iflag &= ~(HUPCL | IUTF8 | IUCLC | ISTRIP | IXON | IXOFF | IXANY | ICRNL);
	    termios->c_oflag &= ~(OPOST | OCRNL | ONLCR | OLCUC | ONLRET);
	    termios->c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO | ECHOE | ECHOK | ECHONL);
	    termios->c_lflag &= ~(NOFLSH | XCASE | TOSTOP | ECHOPRT | ECHOCTL | ECHOKE);
	    termios->c_cc[VMIN] = 1;
	    termios->c_cc[VTIME] = 0;
	    termios->c_cc[VEOF] = 1;
	    termios->c_cflag |= (B115200 | CS8 | CREAD | 0 | 0);
#endif
		_tty_com_speed(com);
		_tty_com_flow_control(com);
		_tty_com_parity(com);
		_tty_com_stopbit(com);
		_tty_com_databit(com);

		tcflush(com->fd, TCIOFLUSH);
		if(tcsetattr(com->fd, TCSANOW, termios) != 0)
			zlog_debug(MODULE_LIB, "tcsetattr %s error:%s", com->devname, safe_strerror(errno));

	    //int modembits = TIOCM_DTR;
	    //ioctl(com->fd, TIOCMBIS, &modembits);

		return OK;
	}
	return -1;
}



static int _tty_com_open(struct tty_com *com)
{
	return _tty_com_option(com);
}

static int _tty_com_close(struct tty_com *com)
{
	if(com && com->fd)
	{
		tcflush(com->fd, TCIOFLUSH);
		tcsetattr(com->fd, TCSANOW, &com->old_termios);
		close(com->fd);
		com->fd = -1;
	}
	return 0;
}



int tty_com_open(struct tty_com *com)
{
	if(com)
	{
		if(com->fd <= 0)
			com->fd = open( com->devname, O_RDWR|O_NOCTTY);
			//com->fd = open( com->devname, O_RDWR|O_NOCTTY|O_NDELAY);
		if(com->fd)
		{
			if(isatty(com->fd))
			{
				set_blocking(com->fd);
				return _tty_com_open(com);
			}
			close(com->fd);
			com->fd = -1;
		}
		zlog_debug(MODULE_OSAL, "open %s error:%s", com->devname, safe_strerror(errno));
	}
	return -1;
}

int tty_com_close(struct tty_com *com)
{
	return _tty_com_close(com);
}


int tty_com_update_option(struct tty_com *com)
{
	return _tty_com_option(com);
}

int tty_com_write(struct tty_com *com, ospl_uchar *buf, ospl_uint32 len)
{
	int ret = 0;
	//tcflush(com->fd, TCIOFLUSH);
	tcflush(com->fd, TCIFLUSH);
	if(com->mode == TTY_COM_MODE_SLIP)
		ret = tty_com_slip_write (com, buf,  len);
	else if(com->mode == TTY_COM_MODE_RAW)
		ret = write(com->fd, buf, len);
	tcdrain(com->fd);
	return ret;
}

int tty_com_read(struct tty_com *com, ospl_uchar *buf, ospl_uint32 len)
{
	int ret = 0;
	if(com->mode == TTY_COM_MODE_SLIP)
		ret = tty_com_slip_read (com, buf,  len);
	else if(com->mode == TTY_COM_MODE_RAW)
		ret = read(com->fd, buf, len);
	return ret;
}

int tty_com_putc(struct tty_com *com, ospl_uchar c)
{
	int ret = 0;
	ospl_uchar ic = c;
	//tcflush(com->fd, TCIOFLUSH);
	ret = write(com->fd, &ic, 1);
	tcdrain(com->fd);
	return ret;
}

int tty_com_getc(struct tty_com *com, ospl_uchar *c)
{
	ospl_uchar oc = 0;
	int ret = read(com->fd, &oc, 1);
	if(ret == 1)
	{
		if(c)
			*c = oc;
	}
	return ret;
}

ospl_bool tty_iscom(struct tty_com *com)
{
	if(strstr(com->devname, "ttyS"))
		return ospl_true;
	else if(strstr(com->devname, "ttyUSB"))
		return ospl_true;
	return ospl_false;
}

ospl_bool tty_isopen(struct tty_com *com)
{
	if(com->fd > 0)
		return ospl_true;
	return ospl_false;
}

/*
int tty_com_update_option(struct tty_com *com)
{
	return _tty_com_option(com);
}*/


int tty_com_slip_write (struct tty_com *com, ospl_uchar *p, ospl_uint32 len)
{
	int slen = len;
	ospl_uchar sc = 0;
	/*发送一个END字符*/
	tty_com_putc (com, TTY_COM_SLIP_END);

	/*发送包内的数据*/
	for(slen = 0; slen < len; slen++)
	{
		sc = p[slen];
		switch (sc)
		{
			/*如果需要转意，则进行相应的处理*/
			case TTY_COM_SLIP_END:
				tty_com_putc (com, TTY_COM_SLIP_ESC);
				tty_com_putc (com, TTY_COM_SLIP_ESC_END);
				break;
			case TTY_COM_SLIP_ESC:
				tty_com_putc (com, TTY_COM_SLIP_ESC);
				tty_com_putc (com, TTY_COM_SLIP_ESC_ESC);
				break;

				/*如果不需要转意，则直接发送*/
			default:
				tty_com_putc (com, sc);
				break;
		}
	}
	/*通知接收方发送结束*/
	tty_com_putc (com, TTY_COM_SLIP_END);
	return len;
}

/* RECV_PACKET:接收包数据，存储于P位置，如果接收到的数据大于LEN，则被截断，函数返回接收到的字节数*/
int tty_com_slip_read (struct tty_com *com, ospl_uchar *p, ospl_uint32 len)
{
	ospl_uchar c = 0;
	int received = 0;
	while (1)
	{
		/*接收字符*/
		if (tty_com_getc (com, &c) == 1)
		{
			switch (c)
			{
					/*如果接收到END，包数据结束，如果包内没有数据，直接抛弃*/
				case TTY_COM_SLIP_END:
					if (received)
					{
						if(received <= len)
							return received;
						return ERROR;
					}
					else
						break;

					/*下面的代码用于处理转意字符*/
				case TTY_COM_SLIP_ESC:
					if (tty_com_getc (com, &c) == 1)
					{
						switch (c)
						{
							case TTY_COM_SLIP_ESC_END:
								c = TTY_COM_SLIP_END;
								if (received < len)
								{
									p[received++] = c;
								}
								break;
							case TTY_COM_SLIP_ESC_ESC:
								c = TTY_COM_SLIP_ESC;
								if (received < len)
								{
									p[received++] = c;
								}
								break;
							default:
								break;
						}
					}
					break;
				default:
					if (received < len)
					{
						p[received++] = c;
					}
					else
						return ERROR;
					break;
			}
		}
	}
	return ERROR;
}



int tty_com_slip_encapsulation(struct tty_slip *sl, ospl_uchar *s, ospl_uint32 len)
{
	ospl_uchar c;
	sl->sliplen = 0;
	/*
	 * Send an initial END ospl_ucharacter to flush out any
	 * data that may have accumulated in the receiver
	 * due to line noise.
	 */
	sl->slipbuf[sl->sliplen++] = TTY_COM_END;
	/*
	 * For each byte in the packet, send the appropriate
	 * ospl_ucharacter sequence, according to the SLIP protocol.
	 */
	while (len-- > 0)
	{
		if((sl->sliplen + 2) >= sl->buffsize)
			return ERROR;
		switch (c = *s++)
		{
		case TTY_COM_END:
			sl->slipbuf[sl->sliplen++] = TTY_COM_ESC;
			sl->slipbuf[sl->sliplen++] = TTY_COM_ESC_END;
			break;
		case TTY_COM_ESC:
			sl->slipbuf[sl->sliplen++] = TTY_COM_ESC;
			sl->slipbuf[sl->sliplen++] = TTY_COM_ESC_ESC;
			break;
		default:
			sl->slipbuf[sl->sliplen++] = c;
			break;
		}
	}
	sl->slipbuf[sl->sliplen++] = TTY_COM_END;
	return sl->sliplen;
}

int tty_com_slip_decapsulation(struct tty_slip *sl, ospl_uchar *s, ospl_uint32 len)
{
	ospl_uchar c = 0, flags = 0;
	sl->sliplen = 0;
	RESET_FLAG(sl->flags);
	while (len-- > 0)
	{
		flags = 0;
		switch (c = *s++)
		{
		case TTY_COM_END:
			if (!CHECK_FLAG(sl->flags, TTY_COM_SLF_ERROR) && (sl->sliplen > 2))
			{
				UNSET_FLAG(sl->flags, TTY_COM_SLF_ESCAPE);
				return sl->sliplen;
			}
			flags = 0;
			break;

		case TTY_COM_ESC:
			SET_FLAG(sl->flags, TTY_COM_SLF_ESCAPE);
			flags = 0;
			break;

		case TTY_COM_ESC_ESC:
			if (CHECK_FLAG(sl->flags, TTY_COM_SLF_ESCAPE))
			{
				UNSET_FLAG(sl->flags, TTY_COM_SLF_ESCAPE);
				c = TTY_COM_ESC;
				flags = 1;
			}
			break;

		case TTY_COM_ESC_END:
			if (CHECK_FLAG(sl->flags, TTY_COM_SLF_ESCAPE))
			{
				UNSET_FLAG(sl->flags, TTY_COM_SLF_ESCAPE);
				c = TTY_COM_END;
				flags = 1;
			}
			break;

		default:
			flags = 1;
			//sl->slipbuf[sl->sliplen++] = c;
			break;
		}
		if (!CHECK_FLAG(sl->flags, TTY_COM_SLF_ERROR) && (flags == 1))
		{
			if (sl->sliplen < sl->buffsize)
			{
				sl->slipbuf[sl->sliplen++] = c;
			}
			else
			{
				SET_FLAG(sl->flags, TTY_COM_SLF_ERROR);
				return ERROR;
			}
		}
	}
	return ERROR;
}



int tty_com_slip_decode_byte(struct tty_slip *sl, ospl_uchar s)
{
	ospl_uchar c = s;
	switch (s)
	{
	case TTY_COM_END:
		if (!CHECK_FLAG(sl->flags, TTY_COM_SLF_ERROR) && (sl->sliplen > 2))
		{
			UNSET_FLAG(sl->flags, TTY_COM_SLF_ESCAPE);
			return sl->sliplen;
		}
		return OK;

	case TTY_COM_ESC:
		SET_FLAG(sl->flags, TTY_COM_SLF_ESCAPE);
		return OK;

	case TTY_COM_ESC_ESC:
		if (CHECK_FLAG(sl->flags, TTY_COM_SLF_ESCAPE))
		{
			UNSET_FLAG(sl->flags, TTY_COM_SLF_ESCAPE);
			c = TTY_COM_ESC;
		}
		break;
	case TTY_COM_ESC_END:
		if (CHECK_FLAG(sl->flags, TTY_COM_SLF_ESCAPE))
		{
			UNSET_FLAG(sl->flags, TTY_COM_SLF_ESCAPE);
			c = TTY_COM_END;
		}
		break;
	default:
		break;
	}
	if (!CHECK_FLAG(sl->flags, TTY_COM_SLF_ERROR))
	{
		if (sl->sliplen < sl->buffsize)
		{
			sl->slipbuf[sl->sliplen++] = c;
			return OK;
		}
		SET_FLAG(sl->flags, TTY_COM_SLF_ERROR);
		return ERROR;
	}
	return ERROR;
}

