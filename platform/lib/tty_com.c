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

struct tty_speed_table
{
	int speed;
	int p;
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
	int i = 0;
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
			zlog_debug(ZLOG_PAL, "tcsetattr %s error:%s", com->devname, safe_strerror(errno));

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
		zlog_debug(ZLOG_PAL, "open %s error:%s", com->devname, safe_strerror(errno));
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

int tty_com_write(struct tty_com *com, char *buf, int len)
{
	int ret = 0;
	tcflush(com->fd, TCIOFLUSH);
	ret = write(com->fd, buf, len);
	tcdrain(com->fd);
	return ret;
}

int tty_com_read(struct tty_com *com, char *buf, int len)
{
	int ret = read(com->fd, buf, len);
	return ret;
}

/*
int tty_com_update_option(struct tty_com *com)
{
	return _tty_com_option(com);
}*/
