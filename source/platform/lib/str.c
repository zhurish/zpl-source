/*
 * zebra string function
 *
 * XXX This version of snprintf does not check bounds!
 */

/*
 The implementations of strlcpy and strlcat are copied from rsync (GPL):
    Copyright (C) Andrew Tridgell 1998
    Copyright (C) 2002 by Martin Pool

 Note that these are not terribly efficient, since they make more than one
 pass over the argument strings.  At some point, they should be optimized.
 
 The implementation of strndup is copied from glibc-2.3.5:
    Copyright (C) 1996, 1997, 1998, 2001, 2002 Free Software Foundation, Inc.
*/

/* 
 * This file is part of Quagga.
 *
 * Quagga is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * Quagga is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Quagga; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include "auto_include.h"
#include "zplos_include.h"
#include "module.h"
#include "str.h"
#include "log.h"

#ifndef HAVE_SNPRINTF
/*
 * snprint() is a real basic wrapper around the standard sprintf()
 * without any bounds checking
 */
int
snprintf(zpl_char *str, zpl_size_t size, const char *format, ...)
{
  zpl_uint32 len = 0;
  va_list args;

  va_start (args, format);
  len = vsprintf (str, format, args);
  va_end(args);
  return len;
}
#endif

#ifndef HAVE_STRLCPY
/**
 * Like strncpy but does not 0 fill the buffer and always null 
 * terminates.
 *
 * @param bufsize is the size of the destination buffer.
 *
 * @return index of the terminating byte.
 **/
zpl_size_t
strlcpy(zpl_char *d, const char *s, zpl_size_t bufsize)
{
	zpl_size_t len = strlen(s);
	zpl_size_t ret = len;
	if (bufsize > 0) {
		if (len >= bufsize)
			len = bufsize-1;
		memcpy(d, s, len);
		d[len] = 0;
	}
	return ret;
}
#endif

#ifndef HAVE_STRLCAT
/**
 * Like strncat() but does not 0 fill the buffer and always null 
 * terminates.
 *
 * @param bufsize length of the buffer, which should be one more than
 * the maximum resulting string length.
 **/
zpl_size_t
strlcat(zpl_char *d, const char *s, zpl_size_t bufsize)
{
	zpl_size_t len1 = strlen(d);
	zpl_size_t len2 = strlen(s);
	zpl_size_t ret = len1 + len2;

	if (len1 < bufsize - 1) {
		if (len2 >= bufsize - len1)
			len2 = bufsize - len1 - 1;
		memcpy(d+len1, s, len2);
		d[len1+len2] = 0;
	}
	return ret;
}
#endif

#ifndef HAVE_STRNLEN
zpl_size_t
strnlen(const char *s, zpl_size_t maxlen)
{
  const char *p;
  return (p = (const char *)memchr(s, '\0', maxlen)) ? (zpl_size_t)(p-s) : maxlen;
}
#endif

#ifndef HAVE_STRNDUP
zpl_char *
strndup (const char *s, zpl_size_t maxlen)
{
    zpl_size_t len = strnlen (s, maxlen);
    zpl_char *new = (zpl_char *) malloc (len + 1);

    if (new == NULL)
      return NULL;

    new[len] = '\0';
    return (zpl_char *) memcpy (new, s, len);
}
#endif


const char *string_have_space(zpl_char* src)
{
	zpl_char *str = src;
	for (; *str != '\0'; str++)
		if (isspace ((int) *str))
			return src;
	return NULL;

/*	while (*src != '\0')
	{
		if (isspace（*src) >= 'a' && *src <= 'z')
			//在ASCII表里大写字符的值比对应小写字符的值小32.
			//*p -= 0x20; // 0x20的十进制就是32
			*src -= 32;
		src++;
	}
	return src;*/
}

int all_space (const char *str)
{
  for (; *str != '\0'; str++)
    if (!isspace ((int) *str))
      return 0;
  return 1;
}


int all_isdigit (const char *str)
{
  for (; *str != '\0'; str++)
    if (!isdigit ((int) *str))
      return 0;
  return 1;

}
/*去除头尾空格*/
const char *strrmtrim(zpl_char* src)
{
	zpl_char *start = NULL, *temp = NULL;			//定义去除空格后字符串的头尾指针和遍历指针
	temp = src;
	while (isspace(*temp))
	{
		++temp;
	}
	start = temp; //求得头指针
	temp = src + strlen(src) - 1; //得到原字符串最后一个字符的指针(不是'\0')
	//printf("%c\n", *temp);
	while (isspace(*temp))
	{
		*temp = '\0';
		--temp;
	}
	//end = temp; //求得尾指针
	//for (src = start; src <= end;)
	//{
	//	*strOut++ = *src++;
	//}
	//*strOut = '\0';
	//end++;
	//*end = '\0';
	return start;
}



/* Validate a hex character */

zpl_uint32 str_to_hex(zpl_char * room)
{
	zassert(room != NULL);
	return strtol(room, NULL, 16);
}

zpl_char * hex_to_str(zpl_uint32 hex)
{
	static zpl_char buf[64];
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%x", hex);
	return buf;
}



/* 获取某个字符数量 strccnt */
int strccnt(zpl_char *src, const char em)
{
	zpl_char *p = src;
	assert(src);
	zpl_uint32 i = 0, j = 0, count = os_strlen(src);
	for(i = 0; i < count; i++)
	{
		if(p[i] == em)
		{
			j++;
		}
	}
	return j;
}
/*获取字符的数量，返回最后一个的位置*/
int strccntlast(zpl_char *src, const char em, int step)
{
	zpl_char *p = src;
	assert(src);
	zpl_uint32 i = 0, j = 0, count = os_strlen(src);
	for(i = 0; i < count; i++)
	{
		if(p[i] == em)
		{
			j++;
			if(j == step)
				break;
		}
	}
	return (i < count)? i:0;
}
/*获取字符的偏移位置*/
int strcoffset(zpl_char *src, const char em)
{
	zpl_char *p = src;
	assert(src);
	zpl_uint32 i = 0, j = 0, count = os_strlen(src);
	for(i = 0; i < count; i++)
	{
		if(p[i] == em)
		{
			j = 1;
			if(i != 0)
				break;
		}
	}
	return j ? i:0;
}
/*获取连续两个字符的间隔*/
int strccstep(zpl_char *src, const char em)
{
	zpl_char *p = src;
	assert(src);
	zpl_uint32 i = 0, j = 0, n = 0, count = os_strlen(src);
	for(i = 0; i < count; i++)
	{
		if(p[i] == em)
		{
			if(j)
			{
				return i-n;
			}
			else
			{
				n = i;
				j = 1;
			}
		}
	}
	return 0;
}
/*获取在dest中src子串的最后位置*/
zpl_char *os_strstr_last(const char *dest,const char *src)
{
	const char *ret=NULL;
	static zpl_char *last = NULL;
	assert(dest);
	assert(src);
	if(*src == '\0')
		return (zpl_char *)dest;
	while((ret = os_strstr(dest,src)))
	{
		last=ret;
		dest=ret+1;
	}
	return (zpl_char *)last;
}

int strccreplace(zpl_char *src, char em, char r)
{
	zpl_char *p = src;
	assert(src);
	zpl_uint32 i = 0, j = 0, count = os_strlen(src);
	for(i = 0; i < count; i++)
	{
		if(p[i] == em)
		{
			p[i] = r;
		}
	}
	return j;
}

int strisempty(zpl_char *dest, zpl_uint32 len)
{
	zpl_char buf[2048];
	os_memset(buf, 0, sizeof(buf));
	if(os_memcmp(buf, dest, MIN(len, sizeof(buf))) == 0)
		return 1;
	return 0;
}



/*
int inet64_to_mac(zpl_uint64 value, zpl_uint8 *dst)
{
	unsigned i;
	zassert(dst != NULL);
	zpl_uint64 temp = value;
	for (i = 0; i < 8; i++) {
		dst[7-i] = temp & 0xff;
		temp >>= 8;
	}
	return OK;
}

zpl_uint64 mac_to_inet64(zpl_uint8 *dst)
{
	unsigned i;
	zpl_uint64 temp = 0;
	zassert(dst != NULL);
	for (i = 0; i < 8; i++) {
		temp |= dst[i] & 0xff;
		temp <<= 8;
	}
	return temp;
}
*/



/*
 *
 *  DHCPD
 */
zpl_uint32
getULong(zpl_uchar *buf)
{
	zpl_uint32 ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohl(ibuf));
}

zpl_uint16
getUShort(zpl_uchar *buf)
{
	zpl_uint16 ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohs(ibuf));
}

void
putULong(zpl_uchar *obuf, zpl_uint32 val)
{
	zpl_uint32 tmp = htonl(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
putLong(zpl_uchar *obuf, zpl_int32 val)
{
	zpl_int32 tmp = htonl(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
putUShort(zpl_uchar *obuf, zpl_int16  val)
{
	zpl_uint16 tmp = htons(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
putShort(zpl_uchar *obuf, zpl_int16 val)
{
	zpl_int16 tmp = htons(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
convert_num(zpl_uchar *buf, zpl_char *str, int base, int size)
{
	zpl_uint32 negative = 0, tval, max;
	zpl_uint32 val = 0;
	zpl_char *ptr = str;

	if (*ptr == '-') {
		negative = 1;
		ptr++;
	}

	/* If base wasn't specified, figure it out from the data. */
	if (!base) {
		if (ptr[0] == '0') {
			if (ptr[1] == 'x') {
				base = 16;
				ptr += 2;
			} else if (isascii((zpl_uchar)ptr[1]) &&
			    isdigit((zpl_uchar)ptr[1])) {
				base = 8;
				ptr += 1;
			} else
				base = 10;
		} else
			base = 10;
	}

	do {
		tval = *ptr++;
		/* XXX assumes ASCII... */
		if (tval >= 'a')
			tval = tval - 'a' + 10;
		else if (tval >= 'A')
			tval = tval - 'A' + 10;
		else if (tval >= '0')
			tval -= '0';
		else {
			//dhcpd_warning("Bogus number: %s.", str);
			break;
		}
		if (tval >= (zpl_uint32)base) {
			//dhcpd_warning("Bogus number: %s: digit %d not in base %d",
			//    str, tval, base);
			break;
		}
		val = val * base + tval;
	} while (*ptr);

	if (negative)
		max = (1 << (size - 1));
	else
		max = (1 << (size - 1)) + ((1 << (size - 1)) - 1);
	if (val > max) {
		switch (base) {
		case 8:
			//dhcpd_warning("value %s%o exceeds max (%d) for precision.",
			//    negative ? "-" : "", val, max);
			break;
		case 16:
			//dhcpd_warning("value %s%x exceeds max (%d) for precision.",
			//    negative ? "-" : "", val, max);
			break;
		default:
			//dhcpd_warning("value %s%u exceeds max (%d) for precision.",
			//    negative ? "-" : "", val, max);
			break;
		}
	}

	if (negative) {
		switch (size) {
		case 8:
			*buf = -(zpl_ulong)val;
			break;
		case 16:
			putShort(buf, -(zpl_ulong)val);
			break;
		case 32:
			putLong(buf, -(zpl_ulong)val);
			break;
		default:
			//dhcpd_warning("Unexpected integer size: %d", size);
			break;
		}
	} else {
		switch (size) {
		case 8:
			*buf = (zpl_uint8)val;
			break;
		case 16:
			putUShort(buf, (zpl_uint16)val);
			break;
		case 32:
			putULong(buf, val);
			break;
		default:
			//zlog_warning("Unexpected integer size: %d", size);
			break;
		}
	}
}




#define INVALID 	1
#define TOOSMALL 	2
#define TOOLARGE 	3

#ifndef LLONG_MIN
#define LLONG_MIN -9223372036854775808
#endif
#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807
#endif

long long
strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp)
{
	long long ll = 0;
	zpl_char *ep;
	int error = 0;
	struct errval {
		const char *errstr;
		int err;
	} ev[4] = {
		{ NULL,		0 },
		{ "invalid",	IPSTACK_ERRNO_EINVAL },
		{ "too small",	ERANGE },
		{ "too large",	ERANGE },
	};

	ev[0].err = ipstack_errno;
	ipstack_errno = 0;
	if (minval > maxval)
		error = INVALID;
	else {
		ll = strtoll(numstr, &ep, 10);
		if (numstr == ep || *ep != '\0')
			error = INVALID;
		else if ((ll == LLONG_MIN && ipstack_errno == ERANGE) || ll < minval)
			error = TOOSMALL;
		else if ((ll == LLONG_MAX && ipstack_errno == ERANGE) || ll > maxval)
			error = TOOLARGE;
	}
	if (errstrp != NULL)
		*errstrp = ev[error].errstr;
	ipstack_errno = ev[error].err;
	if (error)
		ll = 0;

	return (ll);
}
