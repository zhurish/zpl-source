/*
 * xyz_modem.c
 *
 *  Created on: Apr 18, 2019
 *      Author: zhurish
 */




/*
 *==========================================================================
 *
 *      xyzModem.c
 *
 *      RedBoot stream handler for xyzModem protocol
 *
 *==========================================================================
 * SPDX-License-Identifier:	eCos-2.0
 *==========================================================================
 *#####DESCRIPTIONBEGIN####
 *
 * Author(s):    gthomas
 * Contributors: gthomas, tsmith, Yoshinori Sato
 * Date:         2000-07-14
 * Purpose:
 * Description:
 *
 * This code is part of RedBoot (tm).
 *
 *####DESCRIPTIONEND####
 *
 *==========================================================================
 */
#include "os_include.h"
#include "zpl_include.h"
#include "xyz_modem.h"
#include "tty_com.h"
#include "checksum.h"

int xyz_modem_build_hdr(xyz_modem_t*xyz, xyz_modem_hdr_t *hdr, zpl_char *filename, zpl_uint32 filesize)
{
	zpl_uint16 crc = 0, offset = 0;

	memset(hdr, 0, sizeof(xyz_modem_hdr_t));
	xyz->total_SOH++;
	hdr->code = SOH;
	hdr->sequm = xyz->sequm++;
	hdr->sequm_inv = ~hdr->sequm;
	//hdr->data[128];
	if(filename)
	{
		strcpy(hdr->data, filename);
		offset = strlen(filename) + 1;
	}
	sprintf(hdr->data + offset, "%d", filesize);
	//crc = crc_checksum(hdr->data, sizeof(hdr->data));
	crc = crc16_ccitt(0, hdr->data, sizeof(hdr->data));
	hdr->crc_h = crc >> 8;
	hdr->crc_l = crc & 0xff;
	return OK;
}

int xyz_modem_build_data(xyz_modem_t*xyz, xyz_modem_data_t *hdr, zpl_uchar *data, zpl_uint32 len)
{
	zpl_uint16 crc = 0;
	memset(hdr, 0, sizeof(xyz_modem_data_t));
	xyz->total_STX++;
	xyz->len += len;
	hdr->code = STX ;
	if(xyz->sequm == 0XFF)
		xyz->sequm = 1;
	if(xyz->sequm == 0)
		xyz->sequm = 1;
	hdr->sequm = xyz->sequm++;
	hdr->sequm_inv = ~hdr->sequm;
	if(len < XYZ_MAX_SIZE)
	{
		memset(hdr->data, 0x1A, XYZ_MAX_SIZE);
	}
	memcpy(hdr->data, data, len);
	//crc = crc_checksum(hdr->data, sizeof(hdr->data));
	crc = crc16_ccitt(0, hdr->data, sizeof(hdr->data));
	hdr->crc_h = crc >> 8;
	hdr->crc_l = crc & 0xff;
	return OK;
}

int xyz_modem_build_data_last(xyz_modem_t*xyz, xyz_modem_data_last_t *hdr, zpl_uchar *data, zpl_uint32 len)
{
	zpl_uint16 crc = 0;
	memset(hdr, 0, sizeof(xyz_modem_data_last_t));
	xyz->total_SOH++;
	xyz->len += len;
	hdr->code = SOH ;
	if(xyz->sequm == 0XFF)
		xyz->sequm = 1;
	if(xyz->sequm == 0)
		xyz->sequm = 1;
	hdr->sequm = xyz->sequm++;
	hdr->sequm_inv = ~hdr->sequm;
	if(len < XYZ_MIN_SIZE)
	{
		memset(hdr->data, 0x1A, XYZ_MIN_SIZE);
	}
	memcpy(hdr->data, data, len);
	//crc = crc_checksum(hdr->data, sizeof(hdr->data));
	crc = crc16_ccitt(0, hdr->data, sizeof(hdr->data));
	hdr->crc_h = crc >> 8;
	hdr->crc_l = crc & 0xff;
	return OK;
}

int xyz_modem_build_finsh_empty(xyz_modem_t*xyz, xyz_modem_data_last_t *hdr)
{
	zpl_uint16 crc = 0;
	memset(hdr, 0, sizeof(xyz_modem_data_last_t));
	hdr->code = EOT ;
	xyz->sequm = 0;
	hdr->sequm = 0;
	hdr->sequm_inv = ~hdr->sequm;
	//crc = crc_checksum(hdr->data, sizeof(hdr->data));
	crc = crc16_ccitt(0, hdr->data, sizeof(hdr->data));
	hdr->crc_h = crc >> 8;
	hdr->crc_l = crc & 0xff;
	return OK;
}

int xyz_modem_build_finsh_eot(xyz_modem_t*xyz, zpl_uint32 seq)
{
	xyz->s_eof = seq;
	//hdr->code = EOT ;
	return OK;
}
/************************************************************************/
/************************************************************************/
/************************************************************************/
static int
CYGACC_COMM_IF_GETC_TIMEOUT (int chan, zpl_uchar *c)
{
	int ret = os_read_timeout(chan, c, 1, xyzModem_CHAR_TIMEOUT);
	if(ret == 1)
		return 1;
	return 0;
/*  ulong now = get_timer(0);
  while (!tstc ())
    {
      if (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        break;
    }
  if (tstc ())
    {
      *c = getc ();
      return 1;
    }
  return 0;*/
}

static void
CYGACC_COMM_IF_PUTC (int x, zpl_uchar y)
{
  //putc (y);
	zpl_uchar val = y;
	os_write_timeout(x, &val, 1, xyzModem_CHAR_TIMEOUT);
}


/* Convert a single hex nibble */
__inline__ static int
_from_hex (zpl_char c)
{
	int ret = 0;

	if ((c >= '0') && (c <= '9'))
	{
		ret = (c - '0');
	}
	else if ((c >= 'a') && (c <= 'f'))
	{
		ret = (c - 'a' + 0x0a);
	}
	else if ((c >= 'A') && (c <= 'F'))
	{
		ret = (c - 'A' + 0x0A);
	}
	return ret;
}

/* Convert a character to lower case */
__inline__ static zpl_char
xyz__tolower (zpl_char c)
{
	if ((c >= 'A') && (c <= 'Z'))
	{
		c = (c - 'A') + 'a';
	}
	return c;
}

/* Parse (scan) a number */
static zpl_bool
parse_num (zpl_char *s, zpl_uint32 *val, zpl_char **es, zpl_char *delim)
{
	zpl_bool first = zpl_true;
	zpl_uint32 radix = 10;
	zpl_char c;
	zpl_uint32 result = 0;
	zpl_uint32 digit;

	while (*s == ' ')
		s++;
	while (*s)
	{
		if (first && (s[0] == '0') && (xyz__tolower (s[1]) == 'x'))
		{
			radix = 16;
			s += 2;
		}
		first = zpl_false;
		c = *s++;
		if (is_hex (c) && ((digit = _from_hex (c)) < radix))
		{
			/* Valid digit */
			result = (result * radix) + digit;
		}
		else
		{
			if (delim != (zpl_char *) 0)
			{
				/* See if this character is one of the delimiters */
				zpl_char *dp = delim;
				while (*dp && (c != *dp))
					dp++;
				if (*dp)
					break; /* Found a good delimiter */
			}
			return zpl_false; /* Malformatted number */
		}
	}
	*val = result;
	if (es != (zpl_char **) 0)
	{
		*es = s;
	}
	return zpl_true;
}

static int zm_dprintf (xyz_modem_t*xyz, zpl_char *fmt, ...)
{
	zpl_uint32 len;
	va_list args;
	zpl_char p[1024];
	memset(p, 0, sizeof(p));
	va_start(args, fmt);
	len = vsnprintf(p, sizeof(p), fmt, args);
	va_end(args);
	if(xyz->vty && xyz->show_debug)
	{
		(xyz->show_debug)(xyz->vty, "%s\r\n", p);
	}
	return len;
}

#define ZM_DEBUG(x)

/* Wait for the line to go idle */
static void
xyz_modem_flush (xyz_modem_t *xyz)
{
	int res;
	zpl_char c;
	while (zpl_true)
	{
		res = CYGACC_COMM_IF_GETC_TIMEOUT(xyz->fd, &c);
		if (!res)
			return;
	}
}

static int
xyz_modem_get_hdr (xyz_modem_t *xyz)
{
	zpl_uchar c;
	zpl_uint32 res;
	zpl_bool hdr_found = zpl_false;
	zpl_uint32 i, can_total, hdr_chars;
	zpl_int16 cksum;

	ZM_DEBUG (zm_new ());
	/* Find the start of a header */
	can_total = 0;
	hdr_chars = 0;

	if (xyz->tx_ack)
	{
		CYGACC_COMM_IF_PUTC (xyz->fd, ACK);
		xyz->tx_ack = zpl_false;
	}
	while (!hdr_found)
	{
		res = CYGACC_COMM_IF_GETC_TIMEOUT (xyz->fd, &c);
		ZM_DEBUG (zm_save (c));
		if (res)
		{
			hdr_chars++;
			switch (c)
			{
				case SOH:
					xyz->total_SOH++;
				case STX:
					if (c == STX)
						xyz->total_STX++;
					hdr_found = zpl_true;
					break;
				case CAN:
					xyz->total_CAN++;
					ZM_DEBUG (zm_dump (__LINE__));
					if (++can_total == xyzModem_CAN_COUNT)
					{
						return xyzModem_cancel;
					}
					else
					{
						/* Wait for multiple CAN to avoid early quits */
						break;
					}
				case EOT:
					/* EOT only supported if no noise */
					if (hdr_chars == 1)
					{
						CYGACC_COMM_IF_PUTC (xyz->fd, ACK);
						zm_dprintf (xyz, "ACK on EOT #%d\n", __LINE__);
						ZM_DEBUG (zm_dump (__LINE__));
						return xyzModem_eof;
					}
				default:
					/* Ignore, waiting for start of header */
					;
			}
		}
		else
		{
			/* Data stream timed out */
			xyz_modem_flush (xyz); /* Toss any current input */
			ZM_DEBUG (zm_dump (__LINE__));
			CYGACC_CALL_IF_DELAY_US((int ) 250000);
			return xyzModem_timeout;
		}
	}

	/* Header found, now read the data */
	res = CYGACC_COMM_IF_GETC_TIMEOUT (xyz->fd, (zpl_uchar *) &xyz->blk);
	ZM_DEBUG (zm_save (xyz->blk));
	if (!res)
	{
		ZM_DEBUG (zm_dump (__LINE__));
		return xyzModem_timeout;
	}
	res = CYGACC_COMM_IF_GETC_TIMEOUT (xyz->fd, (zpl_uchar *) &xyz->cblk);
	ZM_DEBUG (zm_save (xyz->cblk));
	if (!res)
	{
		ZM_DEBUG (zm_dump (__LINE__));
		return xyzModem_timeout;
	}
	xyz->len = (c == SOH) ? XYZ_MIN_SIZE : XYZ_MAX_SIZE;
	xyz->bufp = xyz->pkt;
	for (i = 0; i < xyz->len; i++)
	{
		res = CYGACC_COMM_IF_GETC_TIMEOUT (xyz->fd, &c);
		ZM_DEBUG (zm_save (c));
		if (res)
		{
			xyz->pkt[i] = c;
		}
		else
		{
			ZM_DEBUG (zm_dump (__LINE__));
			return xyzModem_timeout;
		}
	}
	res = CYGACC_COMM_IF_GETC_TIMEOUT (xyz->fd, (zpl_uchar *) &xyz->crc1);
	ZM_DEBUG (zm_save (xyz->crc1));
	if (!res)
	{
		ZM_DEBUG (zm_dump (__LINE__));
		return xyzModem_timeout;
	}
	if (xyz->crc_mode)
	{
		res = CYGACC_COMM_IF_GETC_TIMEOUT (xyz->fd, (zpl_uchar *) &xyz->crc2);
		ZM_DEBUG (zm_save (xyz->crc2));
		if (!res)
		{
			ZM_DEBUG (zm_dump (__LINE__));
			return xyzModem_timeout;
		}
	}ZM_DEBUG (zm_dump (__LINE__));
	/* Validate the message */
	if ((xyz->blk ^ xyz->cblk) != (zpl_uchar) 0xFF)
	{
		zm_dprintf (xyz, "Framing error - blk: %x/%x/%x\n", xyz->blk, xyz->cblk,
					(xyz->blk ^ xyz->cblk));
		ZM_DEBUG (zm_dump_buf (xyz->pkt, xyz->len));
		xyz_modem_flush (xyz);
		return xyzModem_frame;
	}
	/* Verify checksum/CRC */
	if (xyz->crc_mode)
	{
		cksum = crc16_ccitt (0, xyz->pkt, xyz->len);
		if (cksum != ((xyz->crc1 << 8) | xyz->crc2))
		{
			zm_dprintf (xyz, "CRC error - recvd: %02x%02x, computed: %x\n",
						xyz->crc1, xyz->crc2, cksum & 0xFFFF);
			return xyzModem_cksum;
		}
	}
	else
	{
		cksum = 0;
		for (i = 0; i < xyz->len; i++)
		{
			cksum += xyz->pkt[i];
		}
		if (xyz->crc1 != (cksum & 0xFF))
		{
			zm_dprintf (xyz, "Checksum error - recvd: %x, computed: %x\n",
						xyz->crc1, cksum & 0xFF);
			return xyzModem_cksum;
		}
	}
	/* If we get here, the message passes [structural] muster */
	return 0;
}

static int
xyz_modem_stream_open (xyz_modem_t *xyz, zpl_uint32 *err)
{
	zpl_uint32 stat = 0;
	zpl_uint32 retries = xyzModem_MAX_RETRIES;
	zpl_uint32 crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

	/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
	if (info->mode == xyzModem_zmodem)
	{
		*err = xyzModem_noZmodem;
		return -1;
	}
#endif

	/* TODO: CHECK ! */
	//int dummy = 0;
	//xyz->__chan = &dummy;
	xyz->len = 0;
	xyz->crc_mode = zpl_true;
	xyz->at_eof = zpl_false;
	xyz->tx_ack = zpl_false;
	// xyz->mode = info->mode;
	xyz->total_retries = 0;
	xyz->total_SOH = 0;
	xyz->total_STX = 0;
	xyz->total_CAN = 0;
	xyz->read_length = 0;
	xyz->file_length = 0;

	CYGACC_COMM_IF_PUTC (xyz->fd, (xyz->crc_mode ? 'C' : NAK));

	if (xyz->mode == xyzModem_xmodem)
	{
		/* X-modem doesn't have an information header - exit here */
		xyz->next_blk = 1;
		return 0;
	}

	while (retries-- > 0)
	{
		stat = xyz_modem_get_hdr (xyz);
		if (stat == 0)
		{
			/* Y-modem file information header */
			if (xyz->blk == 0)
			{
				/* skip filename */
				while (*xyz->bufp++)
					;
				/* get the length */
				parse_num ((zpl_char *) xyz->bufp, &xyz->file_length, NULL, " ");
				/* The rest of the file name data block quietly discarded */
				xyz->tx_ack = zpl_true;
			}
			xyz->next_blk = 1;
			xyz->len = 0;
			return 0;
		}
		else if (stat == xyzModem_timeout)
		{
			if (--crc_retries <= 0)
				xyz->crc_mode = zpl_false;
			CYGACC_CALL_IF_DELAY_US(5 * 100000); /* Extra delay for startup */
			CYGACC_COMM_IF_PUTC (xyz->fd, (xyz->crc_mode ? 'C' : NAK));
			xyz->total_retries++;
			zm_dprintf (xyz, "NAK (%d)\n", __LINE__);
		}
		if (stat == xyzModem_cancel)
		{
			break;
		}
	}
	*err = stat;
	ZM_DEBUG (zm_flush ());
	return -1;
}

static int
xyz_modem_stream_read (xyz_modem_t *xyz, zpl_uchar *buf, zpl_uint32 size, zpl_uint32 *err)
{
	zpl_uint32 stat, total, len;
	zpl_uint32 retries;

	total = 0;
	stat = xyzModem_cancel;
	/* Try and get 'size' bytes into the buffer */
	while (!xyz->at_eof && (size > 0))
	{
		if (xyz->len == 0)
		{
			retries = xyzModem_MAX_RETRIES;
			while (retries-- > 0)
			{
				stat = xyz_modem_get_hdr (xyz);
				if (stat == 0)
				{
					if (xyz->blk == xyz->next_blk)
					{
						xyz->tx_ack = zpl_true;
						zm_dprintf (xyz, "ACK block %d (%d)\n", xyz->blk,
						__LINE__);
						xyz->next_blk = (xyz->next_blk + 1) & 0xFF;

						if (xyz->mode == xyzModem_xmodem
								|| xyz->file_length == 0)
						{
							/* Data blocks can be padded with ^Z (EOF) characters */
							/* This code tries to detect and remove them */
							if ((xyz->bufp[xyz->len - 1] == EOF)
									&& (xyz->bufp[xyz->len - 2] == EOF)
									&& (xyz->bufp[xyz->len - 3] == EOF))
							{
								while (xyz->len
										&& (xyz->bufp[xyz->len - 1] == EOF))
								{
									xyz->len--;
								}
							}
						}

						/*
						 * See if accumulated length exceeds that of the file.
						 * If so, reduce size (i.e., cut out pad bytes)
						 * Only do this for Y-modem (and Z-modem should it ever
						 * be supported since it can fall back to Y-modem mode).
						 */
						if (xyz->mode != xyzModem_xmodem
								&& 0 != xyz->file_length)
						{
							xyz->read_length += xyz->len;
							if (xyz->read_length > xyz->file_length)
							{
								xyz->len -=
										(xyz->read_length - xyz->file_length);
							}
						}
						break;
					}
					else if (xyz->blk == ((xyz->next_blk - 1) & 0xFF))
					{
						/* Just re-ACK this so sender will get on with it */
						CYGACC_COMM_IF_PUTC (xyz->fd, ACK);
						continue; /* Need new header */
					}
					else
					{
						stat = xyzModem_sequence;
					}
				}
				if (stat == xyzModem_cancel)
				{
					break;
				}
				if (stat == xyzModem_eof)
				{
					CYGACC_COMM_IF_PUTC (xyz->fd, ACK);
					zm_dprintf (xyz, "ACK (%d)\n", __LINE__);
					if (xyz->mode == xyzModem_ymodem)
					{
						CYGACC_COMM_IF_PUTC (xyz->fd,
											 (xyz->crc_mode ? 'C' : NAK));
						xyz->total_retries++;
						zm_dprintf (xyz, "Reading Final Header\n");
						stat = xyz_modem_get_hdr (xyz);
						CYGACC_COMM_IF_PUTC (xyz->fd, ACK);
						zm_dprintf (xyz, "FINAL ACK (%d)\n", __LINE__);
					}
					xyz->at_eof = zpl_true;
					break;
				}
				CYGACC_COMM_IF_PUTC (xyz->fd, (xyz->crc_mode ? 'C' : NAK));
				xyz->total_retries++;
				zm_dprintf (xyz, "NAK (%d)\n", __LINE__);
			}
			if (stat < 0)
			{
				*err = stat;
				xyz->len = -1;
				return total;
			}
		}
		/* Don't "read" data from the EOF protocol package */
		if (!xyz->at_eof)
		{
			len = xyz->len;
			if (size < len)
				len = size;
			memcpy (buf, xyz->bufp, len);
			size -= len;
			buf += len;
			total += len;
			xyz->len -= len;
			xyz->bufp += len;
		}
	}
	return total;
}

static void
xyz_modem_stream_close (xyz_modem_t *xyz, zpl_uint32 *err)
{
	zm_dprintf(xyz,
			"xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
			xyz->crc_mode ? "CRC" : "Cksum", xyz->total_SOH, xyz->total_STX,
			xyz->total_CAN, xyz->total_retries);
	ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
static void
xyz_modem_stream_terminate (xyz_modem_t *xyz, zpl_bool abort, int (*xyz_getc) (xyz_modem_t *))
{
	zpl_uchar c;

	if (abort)
	{
		zm_dprintf (xyz, "!!!! TRANSFER ABORT !!!!\n");
		switch (xyz->mode)
		{
			case xyzModem_xmodem:
			case xyzModem_ymodem:
				/* The X/YMODEM Spec seems to suggest that multiple CAN followed by an equal */
				/* number of Backspaces is a friendly way to get the other end to abort. */
				CYGACC_COMM_IF_PUTC (xyz->fd, CAN);
				CYGACC_COMM_IF_PUTC (xyz->fd, CAN);
				CYGACC_COMM_IF_PUTC (xyz->fd, CAN);
				CYGACC_COMM_IF_PUTC (xyz->fd, CAN);
				CYGACC_COMM_IF_PUTC (xyz->fd, BSP);
				CYGACC_COMM_IF_PUTC (xyz->fd, BSP);
				CYGACC_COMM_IF_PUTC (xyz->fd, BSP);
				CYGACC_COMM_IF_PUTC (xyz->fd, BSP);
				/* Now consume the rest of what's waiting on the line. */
				zm_dprintf (xyz, "Flushing serial line.\n");
				xyz_modem_flush (xyz);
				xyz->at_eof = zpl_true;
				break;
#ifdef xyzModem_zmodem
				case xyzModem_zmodem:
				/* Might support it some day I suppose. */
#endif
				break;
		}
	}
	else
	{
		zm_dprintf (xyz, "Engaging cleanup mode...\n");
		/*
		 * Consume any trailing crap left in the inbuffer from
		 * previous received blocks. Since very few files are an exact multiple
		 * of the transfer block size, there will almost always be some gunk here.
		 * If we don't eat it now, RedBoot will think the user typed it.
		 */
		zm_dprintf (xyz, "Trailing gunk:\n");
		while ((c = (*xyz_getc) (xyz)) > -1)
			;
		zm_dprintf (xyz, "\n");
		/*
		 * Make a small delay to give terminal programs like minicom
		 * time to get control again after their file transfer program
		 * exits.
		 */
		CYGACC_CALL_IF_DELAY_US((zpl_uint32 ) 250000);
	}
	zm_dprintf (xyz, "----------------------------\n");
}

zpl_char *
xyz_modem_error (zpl_uint32 err)
{
	switch (err) {
	case xyzModem_access:
		return "Can't access file";
		break;
	case xyzModem_noZmodem:
		return "Sorry, zModem not available yet";
		break;
	case xyzModem_timeout:
		return "Timed out";
		break;
	case xyzModem_eof:
		return "End of file";
		break;
	case xyzModem_cancel:
		return "Cancelled";
		break;
	case xyzModem_frame:
		return "Invalid framing";
		break;
	case xyzModem_cksum:
		return "CRC/checksum error";
		break;
	case xyzModem_sequence:
		return "Block sequence error";
		break;
	default:
		return "Unknown error";
		break;
	}
}


static int xyz_getcxmodem(xyz_modem_t *xyz) {

	zpl_uchar c = 0;
	int ret = os_read_timeout(xyz->fd, &c, 1, xyzModem_CHAR_TIMEOUT);
	if(ret == 1)
		return 1;
/*	return 0;
	if (tstc())
		return (getc());*/
	return -1;
}


static int xyz_putc_cb(int fd, zpl_uchar c)
{
	zpl_uchar i = c & 0xff;
	return os_write_timeout(fd, &i, 1, xyzModem_CHAR_TIMEOUT);
}

static int xyz_getc_cb(int fd, zpl_uchar *c, zpl_uint32 ms)
{
	//return read(fd, c, 1);
	//return xyz_getc_timeout(fd, c, 1, ms);
	return os_read_timeout(fd, c, 1, xyzModem_CHAR_TIMEOUT);

}

static int write_wait_cb(int fd, zpl_uchar *c, zpl_uint32 len, zpl_uint32 ms)
{
	//return write(fd, c, len);
	return os_write_timeout(fd, c, len, xyzModem_CHAR_TIMEOUT);
}

static int xyz_modem_cb_default(xyz_modem_t *xyz)
{
	if(xyz->write_cb == NULL)
		xyz->write_cb = write;

	if(xyz->read_cb == NULL)
		xyz->read_cb = read;

	if(xyz->write_wait_cb == NULL)
		xyz->write_wait_cb = write_wait_cb;

	if(xyz->putc_cb == NULL)
		xyz->putc_cb = xyz_putc_cb;

	if(xyz->getc_cb == NULL)
		xyz->getc_cb = xyz_getc_cb;
	return OK;
}


int xyz_modem_load(xyz_modem_t *xyz, zpl_uint32 mode, zpl_char *devi)
{
	zpl_uint32 size = 0;
	zpl_uint32 err = 0;
	zpl_uint32 res = 0;
	struct tty_com	ttycom_tmp;
	zpl_uchar ymodemBuf[XYZ_MAX_SIZE];

	memset(&ttycom_tmp, 0, sizeof(ttycom_tmp));
	xyz_modem_cb_default (xyz);
	if(xyz->mode == 0)
		xyz->mode = mode;
	if(xyz->mode == 0)
		xyz->mode = xyzModem_ymodem;
	if(devi)
	{
		memset(&ttycom_tmp, 0, sizeof(ttycom_tmp));
		sprintf(ttycom_tmp.devname, "/dev/%s", devi);
		ttycom_tmp.speed	= 115200;
		ttycom_tmp.databit = TTY_DATA_8BIT;
		ttycom_tmp.stopbit = TTY_STOP_1BIT;
		ttycom_tmp.parity = TTY_PARITY_NONE;
		ttycom_tmp.flow_control = TTY_FLOW_CTL_NONE;
		if(tty_com_open(&ttycom_tmp) != OK)
		{
			return ERROR;
		}
		xyz->fd = ttycom_tmp.fd;
	}
	size = 0;
	res = xyz_modem_stream_open (xyz, &err);
	if (!res)
	{
		while ((res = xyz_modem_stream_read (xyz, ymodemBuf, XYZ_MAX_SIZE, &err))
				> 0)
		{
			size += res;
			if(xyz->data_cb)
			{
				res = (xyz->data_cb)(xyz->pravi, ymodemBuf, res);
				//memcpy((char *)(store_addr), ymodemBuf, res);
			}
		}
	}
	else
	{
		zm_dprintf (xyz, "%s\n", xyz_modem_error (err));
		size = res;
	}

	xyz_modem_stream_close (xyz, &err);
	xyz_modem_stream_terminate (xyz, zpl_false, &xyz_getcxmodem);
	if(devi)
	{
		tty_com_close(&ttycom_tmp);
	}
	zm_dprintf (xyz, "## Total Size      = 0x%08x = %d Bytes\n", size, size);
	return size;
}

/*
 * RedBoot interface
 */
