/*
 * if_name.h
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */

#ifndef __IF_NAME_H__
#define __IF_NAME_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "if.h"

extern const char *getkernelname(if_type_t	type);

extern const char *getifpname(if_type_t	type);
extern const char *getabstractname(if_type_t	type);

extern if_type_t kernelname2type(const char *name);

extern if_type_t name2type(const char *name);
extern if_type_t abstractname2type(const char *name);

extern zpl_uint32  if_name_hash_make(const char *name);
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

extern ifindex_t if_uspv2ifindex(if_type_t, zpl_uint32, zpl_uint32, zpl_uint32, zpl_uint32);
extern const char * if_uspv2ifname(if_type_t, zpl_uint32, zpl_uint32, zpl_uint32, zpl_uint32);


extern int if_uspv_type_setting(struct interface *ifp);


extern int if_loopback_ifindex_create(if_type_t type, const char *name);

extern const char *if_mac_out_format(zpl_uchar *mac);
extern int vty_iusp_get (const char *str, zpl_uint32 *uspv, zpl_uint32 *end);
extern int vty_mac_get (const char *str, zpl_uchar *mac);

//extern int serial_kifindex_make(const char *name);

#define VTY_IUSP_GET(s,uspv,end)	vty_iusp_get(s, &uspv, &end)
#define VTY_IMAC_GET(s,m)		vty_mac_get(s, m)
 
#ifdef __cplusplus
}
#endif

#endif /* __IF_NAME_H__ */
