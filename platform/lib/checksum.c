/*
 * Checksum routine for Internet Protocol family headers (C Version).
 *
 * Refer to "Computing the Internet Checksum" by R. Braden, D. Borman and
 * C. Partridge, Computer Communication Review, Vol. 19, No. 2, April 1989,
 * pp. 86-101, for additional details on computing this checksum.
 */

#include <zebra.h>
#include "checksum.h"

#define POLYNOMIAL          0x1021
#define INITIAL_REMAINDER   0xFFFF
#define FINAL_XOR_VALUE     0x0000

typedef unsigned short width_t;
#define WIDTH (8 * sizeof(width_t))
#define TOPBIT (1 << (WIDTH - 1))

static width_t crcTable[256];
static u_int8	crc_table_init = 0;

int			/* return checksum in low-order 16 bits */
in_cksum(void *parg, int nbytes)
{
	u_short *ptr = parg;
	register long		sum;		/* assumes long == 32 bits */
	u_short			oddbyte;
	register u_short	answer;		/* assumes u_short == 16 bits */

	/*
	 * Our algorithm is simple, using a 32-bit accumulator (sum),
	 * we add sequential 16-bit words to it, and at the end, fold back
	 * all the carry bits from the top 16 bits into the lower 16 bits.
	 */

	sum = 0;
	while (nbytes > 1)  {
		sum += *ptr++;
		nbytes -= 2;
	}

				/* mop up an odd byte, if necessary */
	if (nbytes == 1) {
		oddbyte = 0;		/* make sure top half is zero */
		*((u_char *) &oddbyte) = *(u_char *)ptr;   /* one byte only */
		sum += oddbyte;
	}

	/*
	 * Add back carry outs from top 16 bits to low 16 bits.
	 */

	sum  = (sum >> 16) + (sum & 0xffff);	/* add high-16 to low-16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;		/* ones-complement, then truncate to 16 bits */
	return(answer);
}

/* Fletcher Checksum -- Refer to RFC1008. */
#define MODX                 4102   /* 5802 should be fine */

/* To be consistent, offset is 0-based index, rather than the 1-based 
   index required in the specification ISO 8473, Annex C.1 */
/* calling with offset == FLETCHER_CHECKSUM_VALIDATE will validate the checksum
   without modifying the buffer; a valid checksum returns 0 */
u_int16_t
fletcher_checksum(u_char * buffer, const size_t len, const uint16_t offset)
{
  u_int8_t *p;
  int x, y, c0, c1;
  u_int16_t checksum;
  u_int16_t *csum;
  size_t partial_len, i, left = len;
  
  checksum = 0;


  if (offset != FLETCHER_CHECKSUM_VALIDATE)
    /* Zero the csum in the packet. */
    {
      assert (offset < (len - 1)); /* account for two bytes of checksum */
      csum = (u_int16_t *) (buffer + offset);
      *(csum) = 0;
    }

  p = buffer;
  c0 = 0;
  c1 = 0;

  while (left != 0)
    {
      partial_len = MIN(left, MODX);

      for (i = 0; i < partial_len; i++)
	{
	  c0 = c0 + *(p++);
	  c1 += c0;
	}

      c0 = c0 % 255;
      c1 = c1 % 255;

      left -= partial_len;
    }

  /* The cast is important, to ensure the mod is taken as a signed value. */
  x = (int)((len - offset - 1) * c0 - c1) % 255;

  if (x <= 0)
    x += 255;
  y = 510 - c0 - x;
  if (y > 255)  
    y -= 255;

  if (offset == FLETCHER_CHECKSUM_VALIDATE)
    {
      checksum = (c1 << 8) + c0;
    }
  else
    {
      /*
       * Now we write this to the packet.
       * We could skip this step too, since the checksum returned would
       * be stored into the checksum field by the caller.
       */
      buffer[offset] = x;
      buffer[offset + 1] = y;

      /* Take care of the endian issue */
      checksum = htons((x << 8) | (y & 0xFF));
    }

  return checksum;
}


/**
 * Initialize the CRC lookup table.
 * This table is used by crcCompute() to make CRC computation faster.
 */
static void crcInit(void)
{
    width_t remainder;
    width_t dividend;
    int bit;
    /* Perform binary long division, a bit at a time. */
    for(dividend = 0; dividend < 256; dividend++)
    {
        /* Initialize the remainder.  */
        remainder = dividend << (WIDTH - 8);
        /* Shift and XOR with the polynomial.   */
        for(bit = 0; bit < 8; bit++)
        {
            /* Try to divide the current data bit.  */
            if(remainder & TOPBIT)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = remainder << 1;
            }
        }
        /* Save the result in the table. */
        crcTable[dividend] = remainder;
    }
} /* crcInit() */


/**
 * Compute the CRC checksum of a binary message block.
 * @para message, 用来计算的数据
 * @para nBytes, 数据的长度
 * @note This function expects that crcInit() has been called
 *       first to initialize the CRC lookup table.
 */
u_int16_t crc_checksum(unsigned char * message, unsigned int nBytes)
{
    unsigned int offset;
    unsigned char byte;
    width_t remainder = INITIAL_REMAINDER;
    if(crc_table_init == 0)
    {
    	crcInit();
    	crc_table_init = 1;
    }
    /* Divide the message by the polynomial, a byte at a time. */
    for( offset = 0; offset < nBytes; offset++)
    {
        byte = (remainder >> (WIDTH - 8)) ^ message[offset];
        remainder = crcTable[byte] ^ (remainder << 8);
    }
    /* The final remainder is the CRC result. */
    return (remainder ^ FINAL_XOR_VALUE);
} /* crcCompute() */


/* Table of CRC constants - implements x^16+x^12+x^5+1 */
static const uint16_t crc16_tab[] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
};

uint16_t crc16_ccitt(uint16_t crc_start, unsigned char *buf, int len)
{
	int i;
	uint16_t cksum;

	cksum = crc_start;
	for (i = 0;  i < len;  i++)
		cksum = crc16_tab[((cksum>>8) ^ *buf++) & 0xff] ^ (cksum << 8);

	return cksum;
}
/*
unsigned int crc8(const unsigned char *vptr, int len)
{
	const unsigned char *data = vptr;
	unsigned int crc = 0;
	int i, j;

	for (j = len; j; j--, data++) {
		crc ^= (*data << 8);
		for (i = 8; i; i--) {
			if (crc & 0x8000)
				crc ^= (0x1070 << 3);
			crc <<= 1;
		}
	}

	return (crc >> 8) & 0xff;
}
*/
