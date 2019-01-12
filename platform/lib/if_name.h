/*
 * if_usp.h
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */

#ifndef IF_USP_H_
#define IF_USP_H_

#include "if.h"
//#include "zebra.h"
//#include "linklist.h"
//#define VTY_IUSP_DEBUG

extern const char *getkernelname(if_type_t	type);

extern const char *getifpname(if_type_t	type);
extern const char *getabstractname(if_type_t	type);

extern if_type_t kernelname2type(const char *name);

extern if_type_t name2type(const char *name);
extern if_type_t abstractname2type(const char *name);

extern unsigned int if_name_hash_make(const char *name);
//two argv : ethernet 0/1/1 ->  (one argv)ethernet 0/1/1
extern const char * if_ifname_format(const char *ifname, const char *uspv);

//ethernet0/1/1 -> ethernet 0/1/1
extern const char * if_ifname_split(const char *name);

//ifindex -> ethernet 0/1/1
extern const char * if_ifname_make (ifindex_t ifindex);
//ifindex -> eth 0/1/1
extern const char * if_ifname_abstract_make(ifindex_t ifindex);
extern const char * if_ifname_alias_make(ifindex_t ifindex);

//two argv : ethernet 0/1/1 -> ifindex
extern ifindex_t if_ifindex_make(const char *ifname, const char *uspv);

extern if_type_t if_iftype_make(const char *str);



extern int if_uspv_type_setting(struct interface *ifp);


extern int if_loopback_ifindex_create(if_type_t type, const char *name);

extern char *if_mac_out_format(unsigned char *mac, int len);
extern int vty_iusp_get (const char *str, int *uspv);
extern int vty_mac_get (const char *str, unsigned char *mac);

//extern int serial_kifindex_make(const char *name);

#define VTY_IUSP_GET(s,uspv)	vty_iusp_get(s, &uspv)
#define VTY_IMAC_GET(s,m)		vty_mac_get(s, m)

#endif /* IF_USP_H_ */
