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
