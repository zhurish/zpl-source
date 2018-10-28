/* 
 * dhcpcd - DHCP client daemon
 * Copyright (c) 2006-2009 Roy Marples <roy@marples.name>
 * All rights reserved

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef DHCPC_UTIL_H
#define DHCPC_UTIL_H

extern void sort_interfaces(void);


extern size_t get_duid(unsigned char *duid, struct dhcpc_interface *iface);


//extern int getifssid(const char *ifname, char *ssid);

extern char *hardware_platform(void);
extern int check_ipv6(const char *);

extern int closefrom(int);

extern int atoint(const char *s);
extern ssize_t parse_string_hwaddr(char *sbuf, ssize_t slen, const char *str, int clid);
extern ssize_t dhcpc_parse_string(char *sbuf, ssize_t slen, const char *str);
extern int parse_addr(struct in_addr *addr, struct in_addr *net, const char *arg);

/*extern void init_state(struct dhcpc_interface *iface);
extern void if_reboot(struct dhcpc_interface *iface);
extern void stop_interface(struct dhcpc_interface *iface);*/
extern void cleanup(void);
extern int dhcpc_hostname(char *name, int len);


extern int dhcpc_main(int argc, char **argv);


#endif
