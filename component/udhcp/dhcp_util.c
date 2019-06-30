/*
 * util.c
 *
 *  Created on: Apr 20, 2019
 *      Author: zhurish
 */

#include "dhcp_def.h"
#include "dhcp_util.h"

const uint8_t DHCP_MAC_BCAST_ADDR[6] ALIGN2 = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/* Parse string to IP in network order */
int udhcp_str2nip(const char *str, void *arg)
{
	u_int32 addr = 0;
	addr = inet_addr(str);
	/* arg maybe unaligned */
	move_to_unaligned32((uint32_t*)arg, addr);
	return 1;
}

/* note: ip is a pointer to an IPv6 in network order, possibly misaliged */
int FAST_FUNC sprint_nip6(char *dest, /*const char *pre,*/ const uint8_t *ip)
{
	char hexstrbuf[16 * 2];
	bin2hex(hexstrbuf, (void*)ip, 16);
	return sprintf(dest, /* "%s" */
		"%.4s:%.4s:%.4s:%.4s:%.4s:%.4s:%.4s:%.4s",
		/* pre, */
		hexstrbuf + 0 * 4,
		hexstrbuf + 1 * 4,
		hexstrbuf + 2 * 4,
		hexstrbuf + 3 * 4,
		hexstrbuf + 4 * 4,
		hexstrbuf + 5 * 4,
		hexstrbuf + 6 * 4,
		hexstrbuf + 7 * 4
	);
}




int udhcp_interface_mac(int ifindex, uint32_t *nip, uint8_t *mac)
{
	struct interface * ifp = if_lookup_by_index (ifindex);
	if(ifp)
	{
		if(nip)
		{
			struct prefix address;
			if(nsm_interface_address_get_api(ifp, &address) == OK)
			{
				*nip = address.u.prefix4.s_addr;
			}
		}
		if(mac)
			nsm_interface_mac_get_api(ifp, mac, ETHER_ADDR_LEN);
		return 0;
	}
	return -1;
}

#if 0
uint16_t FAST_FUNC inet_cksum(uint16_t *addr, int nleft)
{
	/*
	 * Our algorithm is simple, using a 32 bit accumulator,
	 * we add sequential 16 bit words to it, and at the end, fold
	 * back all the carry bits from the top 16 bits into the lower
	 * 16 bits.
	 */
	unsigned sum = 0;
	while (nleft > 1) {
		sum += *addr++;
		nleft -= 2;
	}

	/* Mop up an odd byte, if necessary */
	if (nleft == 1) {
		if (BB_LITTLE_ENDIAN)
			sum += *(uint8_t*)addr;
		else
			sum += *(uint8_t*)addr << 8;
	}

	/* Add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
	sum += (sum >> 16);                     /* add carry */

	return (uint16_t)~sum;
}
#endif
/*
void FAST_FUNC udhcp_sp_fd_set(struct pollfd pfds[], int extra_fd)
{
	pfds[0].events = POLLIN;
	pfds[0].fd = -1;
	if (extra_fd >= 0) {
		//close_on_exec_on(extra_fd);
		pfds[0].fd = extra_fd;
		pfds[0].events = POLLIN;
	}
	pfds[0].revents = 0;
}
*/
/*
void* FAST_FUNC xrealloc_vector(void *vector, unsigned sizeof_and_shift, int idx)
{
	int mask = 1 << (uint8_t)sizeof_and_shift;

	if (!(idx & (mask - 1))) {
		sizeof_and_shift >>= 8;
		vector = realloc(vector, sizeof_and_shift * (idx + mask + 1));
		memset((char*)vector + (sizeof_and_shift * idx), 0, sizeof_and_shift * (mask + 1));
	}
	return vector;
}*/

char* FAST_FUNC xasprintf(const char *format, ...)
{
	va_list p;
	int r;
	char *string_ptr;

	va_start(p, format);
	r = vasprintf(&string_ptr, format, p);
	va_end(p);

/*	if (r < 0)
		bb_error_msg_and_die(bb_msg_memory_exhausted);*/
	return string_ptr;
}



ssize_t FAST_FUNC safe_read(int fd, void *buf, size_t count)
{
	ssize_t n;

	do {
		n = ip_read(fd, buf, count);
	} while (n < 0 && errno == EINTR);

	return n;
}

#if 0
/*
 * Read all of the supplied buffer from a file.
 * This does multiple reads as necessary.
 * Returns the amount read, or -1 on an error.
 * A short read is returned on an end of file.
 */
ssize_t FAST_FUNC full_read(int fd, void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len) {
		cc = safe_read(fd, buf, len);

		if (cc < 0) {
			if (total) {
				/* we already have some! */
				/* user can do another read to know the error code */
				return total;
			}
			return cc; /* read() returns -1 on failure. */
		}
		if (cc == 0)
			break;
		buf = ((char *)buf) + cc;
		total += cc;
		len -= cc;
	}

	return total;
}


ssize_t FAST_FUNC safe_write(int fd, const void *buf, size_t count)
{
	ssize_t n;

	for (;;) {
		n = ip_write(fd, buf, count);
		if (n >= 0 || errno != EINTR)
			break;
		/* Some callers set errno=0, are upset when they see EINTR.
		 * Returning EINTR is wrong since we retry write(),
		 * the "error" was transient.
		 */
		errno = 0;
		/* repeat the write() */
	}

	return n;
}

ssize_t FAST_FUNC full_write(int fd, const void *buf, size_t len)
{
	ssize_t cc;
	ssize_t total;

	total = 0;

	while (len) {
		cc = safe_write(fd, buf, len);

		if (cc < 0) {
			if (total) {
				/* we already wrote some! */
				/* user can do another write to know the error code */
				return total;
			}
			return cc;  /* write() returns -1 on failure. */
		}

		total += cc;
		buf = ((const char *)buf) + cc;
		len -= cc;
	}

	return total;
}

/* Die with an error message if we can't read the entire buffer. */
void FAST_FUNC xread(int fd, void *buf, size_t count)
{
	if (count) {
		ssize_t size = full_read(fd, buf, count);
		if ((size_t)size != count)
			;//bb_error_msg_and_die("short read");
	}
}
#endif

/* Wrapper which restarts poll on EINTR or ENOMEM.
 * On other errors does perror("poll") and returns.
 * Warning! May take longer than timeout_ms to return! */
int FAST_FUNC safe_poll(struct pollfd *ufds, nfds_t nfds, int timeout)
{
	while (1) {
		int n = poll(ufds, nfds, timeout);
		if (n >= 0)
			return n;
		/* Make sure we inch towards completion */
		if (timeout > 0)
			timeout--;
		/* E.g. strace causes poll to return this */
		if (errno == EINTR)
			continue;
		/* Kernel is very low on memory. Retry. */
		/* I doubt many callers would handle this correctly! */
		if (errno == ENOMEM)
			continue;
		//bb_perror_msg("poll");
		return n;
	}
}


#if defined(IPV6_PKTINFO) && !defined(IPV6_RECVPKTINFO)
# define IPV6_RECVPKTINFO IPV6_PKTINFO
#endif


#if 0
int FAST_FUNC index_in_strings(const char *strings, const char *key)
{
	int idx = 0;

	while (*strings) {
		if (strcmp(strings, key) == 0) {
			return idx;
		}
		strings += strlen(strings) + 1; /* skip NUL */
		idx++;
	}
	return -1;
}
#endif

/* Emit a string of hex representation of bytes */
char* FAST_FUNC bin2hex(char *p, const char *cp, int count)
{
	const char bb_hexdigits_upcase[] ALIGN1 = "0123456789ABCDEF";
	while (count) {
		unsigned char c = *cp++;
		/* put lowercase hex digits */
		*p++ = 0x20 | bb_hexdigits_upcase[c >> 4];
		*p++ = 0x20 | bb_hexdigits_upcase[c & 0xf];
		count--;
	}
	return p;
}

/* Convert "[x]x[:][x]x[:][x]x[:][x]x" hex string to binary, no more than COUNT bytes */
char* FAST_FUNC hex2bin(char *dst, const char *str, int count)
{
	errno = EINVAL;
	while (*str && count) {
		uint8_t val;
		uint8_t c = *str++;
		if (isdigit(c))
			val = c - '0';
		else if ((c|0x20) >= 'a' && (c|0x20) <= 'f')
			val = (c|0x20) - ('a' - 10);
		else
			return NULL;
		val <<= 4;
		c = *str;
		if (isdigit(c))
			val |= c - '0';
		else if ((c|0x20) >= 'a' && (c|0x20) <= 'f')
			val |= (c|0x20) - ('a' - 10);
		else if (c == ':' || c == '\0')
			val >>= 4;
		else
			return NULL;

		*dst++ = val;
		if (c != '\0')
			str++;
		if (*str == ':')
			str++;
		count--;
	}
	errno = (*str ? ERANGE : 0);
	return dst;
}

/*
void* FAST_FUNC xmemdup(const void *s, int n)
{
	char *p = malloc(n);
	if(p)
		return memcpy(p, s, n);
	return NULL;
}
*/
