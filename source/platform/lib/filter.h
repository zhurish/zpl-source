/*
 * Route filtering function.
 * Copyright (C) 1998 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 */

#ifndef __LIB_FILTER_H__
#define __LIB_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hash.h"
#ifdef ZPL_SHELL_MODULE
#include "vty.h"
#endif
#include "prefix.h"
#include "libnetpro.h"
/*
标准IP访问列表
  一个标准IP访问控制列表匹配IP包中的源地址或源地址中的一部分，可对匹配的包采取拒绝或允许两个操作。
    编号范围是从1到99的访问控制列表是标准IP访问控制列表。 [6] 
扩展IP访问
  扩展IP访问控制列表比标准IP访问控制列表具有更多的匹配项，包括协议类型、源地址、目的地址、
    源端口、目的端口、建立连接的和IP优先级等。编号范围是从100到199的访问控制列表是扩展IP访问控制列表。 [6] 
命名的IP访问
  所谓命名的IP访问控制列表是以列表名代替列表编号来定义IP访问控制列表，同样包括标准和扩展两种列表，
    定义过滤的语句与编号方式中相似。 [6] 
标准IPX访问
  标准IPX访问控制列表的编号范围是800-899，它检查IPX源网络号和目的网络号，同样可以检查源地址和目的地址的节点号部分。 [6] 
扩展IPX访问
  扩展IPX访问控制列表在标准IPX访问控制列表的基础上，增加了对IPX报头中以下几个字段的检查，它们是协议类型、
    源Socket、目标Socket。扩展IPX访问控制列表的编号范围是900-999。 [6] 
命名的IPX访问
  与命名的IP访问控制列表一样，命名的IPX访问控制列表是使用列表名取代列表编号。从而方便定义和引用列表，同样有标准和扩展之分。
*/
#define ZPL_FILTER_NORMAL_EXT
#define ZPL_FILTER_MAC

/* Filter direction.  */
#define FILTER_IN                 0
#define FILTER_OUT                1
#define FILTER_MAX                2


/* Filter type is made by `permit', `deny' and `dynamic'. */
enum filter_type 
{
  FILTER_DENY,
  FILTER_PERMIT,
  FILTER_DYNAMIC
};

enum access_type
{
  ACCESS_TYPE_STRING,
  ACCESS_TYPE_NUMBER
};

enum access_node_type
{
  FILTER_COMMON         = 0,
  FILTER_ZEBOS          = 1,
  FILTER_ZEBOS_EXT      = 2,
  FILTER_MAC            = 3,
};


struct filter_cisco
{
  /* Cisco access-list */
  int extended;
  struct ipstack_in_addr addr;
  struct ipstack_in_addr addr_mask;
  struct ipstack_in_addr mask;
  struct ipstack_in_addr mask_mask;
};

struct filter_zebra
{
  /* If this filter is "exact" match then this flag is set. */
  int exact;

  /* Prefix information. */
  struct prefix prefix;
};

#ifdef ZPL_FILTER_NORMAL_EXT
struct filter_zebos_ext
{
  int16_t protocol;
  //int16_t ip_pri_dscp;
  /* Source prefix. */
  struct prefix sprefix;

  /* Source port operator. */
  enum operation sport_op;

  /* Source port. */
  int sport;

  /* Destination prefix. */
  struct prefix dprefix;

  /* Destination port operator. */
  enum operation dport_op;

  /* Destination port. */
  int dport;
};
#endif

#ifdef ZPL_FILTER_MAC
struct filter_l2
{
  mac_t srcmac[NSM_MAC_MAX];
  mac_t dstmac[NSM_MAC_MAX];
  mac_t srcmac_mask[NSM_MAC_MAX];
  mac_t dstmac_mask[NSM_MAC_MAX];
  zpl_proto_t l2protocol;
  vlan_t vlanid;
  int16_t cos;
  vlan_t inner_vlanid;
  int16_t inner_cos;
  int16_t label;
  int16_t exp;
  int16_t inner_label;
  int16_t inner_exp;
  #define FILTER_L2MAC_SRC      0x01
  #define FILTER_L2MAC_SRCMASK  0x02
  #define FILTER_L2MAC_DST      0x04
  #define FILTER_L2MAC_DSTMASK  0x08
  #define FILTER_L2MAC_PROTO    0x10
  #define FILTER_L2MAC_VLAN     0x20
  #define FILTER_L2MAC_INNERVID 0x40
  #define FILTER_L2MAC_COS      0x80
  #define FILTER_L2MAC_INNERCOS 0x100
  #define FILTER_L2MAC_LABEL    0x200
  #define FILTER_L2MAC_INNERLABEL 0x400
  #define FILTER_L2MAC_EXP      0x800
  #define FILTER_L2MAC_INNEREXP 0x1000
  uint32_t  flag;
};
#endif
/* Filter element of access list */
struct filter_list
{
  /* For doubly linked list. */
  struct filter_list *next;
  struct filter_list *prev;

  /* Filter type information. */
  enum filter_type type;

  /* Cisco access-list */
  enum access_node_type nodetype;
  union
    {
      struct filter_cisco cfilter;
      struct filter_zebra zfilter;
      #ifdef ZPL_FILTER_NORMAL_EXT
      struct filter_zebos_ext zextfilter;
      #endif
      #ifdef ZPL_FILTER_MAC
      struct filter_l2    mac_filter
      #endif
    }u;
};

/* List of access_list. */
struct access_list_list
{
  struct access_list *head;
  struct access_list *tail;
};

/* Master structure of access_list. */
struct access_master
{
  /* List of access_list which name is number. */
  struct access_list_list num;

  /* List of access_list which name is string. */
  struct access_list_list str;

  /* Hook function which is executed when new access_list is added. */
  void (*add_hook[MODULE_MAX]) (struct access_list *);

  /* Hook function which is executed when access_list is deleted. */
  void (*delete_hook[MODULE_MAX]) (struct access_list *);
};
/* Access list */
struct access_list
{
  zpl_char *name;
  zpl_char *remark;
  /* Reference count. */
  u_int32_t ref_cnt;
  struct access_master *master;

  enum access_type type;

  struct access_list *next;
  struct access_list *prev;

  struct filter_list *head;
  struct filter_list *tail;
};

/* Prototypes for access-list. */
extern void access_list_init (void);
extern void access_list_reset (void);
extern void access_list_add_hook (zpl_uint32, void (*func)(struct access_list *));
extern void access_list_delete_hook (zpl_uint32, void (*func)(struct access_list *));
extern const char * filter_type_str(enum filter_type type);

extern int filter_compare_cisco(struct filter_cisco *filter, struct filter_cisco *new);
extern int filter_compare_zebra(struct filter_zebra *filter, struct filter_zebra *new);
#ifdef ZPL_FILTER_NORMAL_EXT
extern int filter_compare_zebos_extended(struct filter_zebos_ext *filter, struct filter_zebos_ext *new);
#endif
#ifdef ZPL_FILTER_MAC
extern int filter_compare_l2mac(struct filter_l2 *filter, struct filter_l2 *new);
#endif
extern int filter_cisco_format(struct vty *vty, const char *addr_str, const char *addr_mask_str,
                 const char *mask_str, const char *mask_mask_str,
                 int extended, struct filter_cisco *filter);

extern int filter_normal_format(struct vty *vty, afi_t afi, const char *prefix_str, int exact, struct filter_zebra *filter);
#ifdef ZPL_FILTER_NORMAL_EXT
extern int filter_zebos_ext_format(struct vty *vty, afi_t afi,
                                     const char *prot_str, const char *sprefix,
                                     const char *sport_op, const char *sport, const char *seport,
                                     const char *dprefix, const char *dport_op,
                                     const char *dport, const char *deport, struct filter_zebos_ext *filter);
#endif
#ifdef ZPL_FILTER_MAC
extern int filter_l2mac_format(struct vty *vty, 
                            char *src, char *srcmask, 
                            char *dst, char *dstmask,
                            int proto,
                            int vlan, int cos, int innervlan, int innercos,
                            int label, int exp, int innerlabel, int innerexp,
                            struct filter_l2 *l2new);
#endif

extern void access_list_write_config_cisco(struct vty *vty, struct filter_cisco *filter);
extern void access_list_write_config_zebra(struct vty *vty, struct filter_zebra *filter);
#ifdef ZPL_FILTER_NORMAL_EXT
extern void access_list_write_config_zebos_ext(struct vty *vty, struct filter_zebos_ext *filter);
#endif
#ifdef ZPL_FILTER_MAC
extern void access_list_write_config_mac(struct vty *vty, struct filter_l2 *filter);
#endif
/* If filter match to the prefix then return 1. */
extern int filter_match_cisco(struct filter_cisco *filter, struct prefix *p);
/* If filter match to the prefix then return 1. */
extern int filter_match_zebra(struct filter_zebra *filter, struct prefix *p);
#ifdef ZPL_FILTER_NORMAL_EXT
extern int filter_match_zebos_ext(struct filter_zebos_ext *filter, struct filter_zebos_ext *p);
#endif
#ifdef ZPL_FILTER_MAC
extern int filter_match_mac(struct filter_l2 *filter, struct filter_l2 *p);
#endif
extern struct access_list *access_list_lookup (afi_t, const char *);
extern enum filter_type access_list_apply (struct access_list *, void *);
 
extern int access_list_reference (afi_t, const char *, zpl_bool enable);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_FILTER_H__ */
