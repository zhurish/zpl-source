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

#include <zebra.h>

#ifndef HAVE_SNPRINTF
/*
 * snprint() is a real basic wrapper around the standard sprintf()
 * without any bounds checking
 */
int
snprintf(char *str, size_t size, const char *format, ...)
{
  int len = 0;
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
size_t
strlcpy(char *d, const char *s, size_t bufsize)
{
	size_t len = strlen(s);
	size_t ret = len;
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
size_t
strlcat(char *d, const char *s, size_t bufsize)
{
	size_t len1 = strlen(d);
	size_t len2 = strlen(s);
	size_t ret = len1 + len2;

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
size_t
strnlen(const char *s, size_t maxlen)
{
  const char *p;
  return (p = (const char *)memchr(s, '\0', maxlen)) ? (size_t)(p-s) : maxlen;
}
#endif

#ifndef HAVE_STRNDUP
char *
strndup (const char *s, size_t maxlen)
{
    size_t len = strnlen (s, maxlen);
    char *new = (char *) malloc (len + 1);

    if (new == NULL)
      return NULL;

    new[len] = '\0';
    return (char *) memcpy (new, s, len);
}
#endif

/*
C库函数 int tolower(int c)转换给定的字母为小写。
C库函数 int toupper(int c)转换给定的字母为大写。
 */
const char *strupr(char* src)
{
	char *p = src;
	/*
	 * a -> A
	 */
	while (*p != '\0')
	{
		if (*p >= 'a' && *p <= 'z')
			//在ASCII表里大写字符的值比对应小写字符的值小32.
			//*p -= 0x20; // 0x20的十进制就是32
			*p -= 32;
		p++;
	}
	return src;
}

const char *strlwr(char* src)
{
	char *p = src;
	/*
	 * A -> a
	 */
	while (*p != '\0')
	{
		if (*p >= 'A' && *p <= 'Z')
			*p += 32;
		p++;
	}
	return src;
}

const char *string_have_space(char* src)
{
	char *str = src;
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

const char *str_trim(char* src)
{
	char *start = NULL, *end = NULL, *temp = NULL;			//定义去除空格后字符串的头尾指针和遍历指针
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
	end = temp; //求得尾指针
	//for (src = start; src <= end;)
	//{
	//	*strOut++ = *src++;
	//}
	//*strOut = '\0';
	//end++;
	//*end = '\0';
	return start;
}

const char *itoa(int value, int base)
{
	static char buf[32];
	memset(buf, 0, sizeof(buf));
	if(base == 0 || base == 10)
		snprintf(buf, sizeof(buf), "%d", value);
	else if(base == 16)
		snprintf(buf, sizeof(buf), "%x", value);
	return buf;
}

const char *ftoa(float value, char *fmt)
{
	static char buf[16];
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), fmt, value);
	return buf;
}

const char *dtoa(double value, char *fmt)
{
	static char buf[32];
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), fmt, value);
	return buf;
}


/* Validate a hex character */
BOOL is_hex (char c)
{
  return (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

u_int32 str_to_hex(char * room)
{
	zassert(room != NULL);
	return strtol(room, NULL, 16);
}

char * hex_to_str(u_int32 hex)
{
	static char buf[64];
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%x", hex);
	return buf;
}

u_int8 atoascii(int a)
{
	return ((a) - 0x30);
}



int strchr_count(char *src, const char em)
{
	char *p = src;
	assert(src);
	int i = 0, j = 0, count = os_strlen(src);
	for(i = 0; i < count; i++)
	{
		if(p[i] == em)
		{
			j++;
		}
	}
	return j;
}

int strchr_step(char *src, const char em, int step)
{
	char *p = src;
	assert(src);
	int i = 0, j = 0, count = os_strlen(src);
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

int strchr_next(char *src, const char em)
{
	char *p = src;
	assert(src);
	int i = 0, j = 0, count = os_strlen(src);
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

char *os_strstr_last(const char *dest,const char *src)
{
	const char *ret=NULL;
	static char *last = NULL;
	assert(dest);
	assert(src);
	if(*src == '\0')
		return (char *)dest;
	while((ret = os_strstr(dest,src)))
	{
		last=ret;
		dest=ret+1;
	}
	return (char *)last;
}


int str_isempty(char *dest, int len)
{
	char buf[2048];
	os_memset(buf, 0, sizeof(buf));
	if(os_memcmp(buf, dest, MIN(len, sizeof(buf))) == 0)
		return 1;
	return 0;
}



/*
int inet64_to_mac(u_int64 value, u_int8 *dst)
{
	unsigned i;
	zassert(dst != NULL);
	u_int64 temp = value;
	for (i = 0; i < 8; i++) {
		dst[7-i] = temp & 0xff;
		temp >>= 8;
	}
	return OK;
}

u_int64 mac_to_inet64(u_int8 *dst)
{
	unsigned i;
	u_int64 temp = 0;
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
u_int32_t
getULong(unsigned char *buf)
{
	u_int32_t ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohl(ibuf));
}

u_int16_t
getUShort(unsigned char *buf)
{
	u_int16_t ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohs(ibuf));
}

void
putULong(unsigned char *obuf, u_int32_t val)
{
	u_int32_t tmp = htonl(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
putLong(unsigned char *obuf, int32_t val)
{
	int32_t tmp = htonl(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
putUShort(unsigned char *obuf, unsigned int val)
{
	u_int16_t tmp = htons(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
putShort(unsigned char *obuf, int val)
{
	int16_t tmp = htons(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
convert_num(unsigned char *buf, char *str, int base, int size)
{
	u_int32_t negative = 0, tval, max;
	u_int32_t val = 0;
	char *ptr = str;

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
			} else if (isascii((unsigned char)ptr[1]) &&
			    isdigit((unsigned char)ptr[1])) {
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
		if (tval >= (u_int32_t)base) {
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
			*buf = -(unsigned long)val;
			break;
		case 16:
			putShort(buf, -(unsigned long)val);
			break;
		case 32:
			putLong(buf, -(unsigned long)val);
			break;
		default:
			//dhcpd_warning("Unexpected integer size: %d", size);
			break;
		}
	} else {
		switch (size) {
		case 8:
			*buf = (u_int8_t)val;
			break;
		case 16:
			putUShort(buf, (u_int16_t)val);
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



#include "errno.h"
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
	char *ep;
	int error = 0;
	struct errval {
		const char *errstr;
		int err;
	} ev[4] = {
		{ NULL,		0 },
		{ "invalid",	EINVAL },
		{ "too small",	ERANGE },
		{ "too large",	ERANGE },
	};

	ev[0].err = errno;
	errno = 0;
	if (minval > maxval)
		error = INVALID;
	else {
		ll = strtoll(numstr, &ep, 10);
		if (numstr == ep || *ep != '\0')
			error = INVALID;
		else if ((ll == LLONG_MIN && errno == ERANGE) || ll < minval)
			error = TOOSMALL;
		else if ((ll == LLONG_MAX && errno == ERANGE) || ll > maxval)
			error = TOOLARGE;
	}
	if (errstrp != NULL)
		*errstrp = ev[error].errstr;
	errno = ev[error].err;
	if (error)
		ll = 0;

	return (ll);
}
