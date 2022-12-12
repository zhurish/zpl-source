/*
 * if_utsp.h
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */

#ifndef __IF_UTSP_H__
#define __IF_UTSP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "auto_include.h"

#define BMGT_DEBUG_EVENT  0x10
#define BMGT_DEBUG_DETAIL 0x08

#define IS_BMGT_DEBUG_EVENT(n) ((n)&BMGT_DEBUG_EVENT)
#define IS_BMGT_DEBUG_DETAIL(n) ((n)&BMGT_DEBUG_DETAIL)

enum UBMG_STAT
{
  /* data */
  UBMG_STAT_INIT,
  UBMG_STAT_PAND,
  UBMG_STAT_ACTIVE,
  UBMG_STAT_UNACTIVE,
};

typedef struct 
{
  NODE	node;
  if_type_t type;
  zpl_uint8 lport;
  zpl_phyport_t phyid;
}unit_port_mgt_t;

typedef struct 
{
  NODE	node;
  zpl_uint8 unit;
  zpl_uint8 slot;

  enum UBMG_STAT state;
  zpl_bool  online;
  zpl_bool  b_install;
  LIST *port_list;
  os_mutex_t *mutex;

}unit_board_mgt_t;

extern zpl_uint32 _bmgt_debug;

extern int unit_board_init(void);
extern int unit_board_exit(void);
extern int unit_board_waitting(void);
extern int unit_board_foreach(int (*func)(unit_board_mgt_t *, void *), void *p);
extern unit_board_mgt_t * unit_board_add(zpl_uint8 unit, zpl_uint8 slot);
extern int unit_board_del(zpl_uint8 unit, zpl_uint8 slot);
extern unit_board_mgt_t * unit_board_lookup(zpl_uint8 unit, zpl_uint8 slot);

extern int unit_board_port_foreach(unit_board_mgt_t*, int (*func)(unit_port_mgt_t *, void *), void *p);
extern unit_port_mgt_t *unit_board_port_add(unit_board_mgt_t *, if_type_t type, zpl_uint8 lport, zpl_phyport_t phyid);
extern int unit_board_port_del(unit_board_mgt_t *, if_type_t type, zpl_uint8 lport, zpl_phyport_t phyid);

extern int unit_board_show(void *pvoid);

/* 动态上线下线板卡 */
extern int unit_board_dynamic_install(zpl_uint8 unit, zpl_uint8 slot, zpl_bool enable);



/* 初始化 */
extern int unit_board_startup(void);


#ifdef ZPL_KERNEL_MODULE
extern int if_ktest_init(void);
#endif



#ifdef __cplusplus
}
#endif

#endif /* __IF_UTSP_H__ */
