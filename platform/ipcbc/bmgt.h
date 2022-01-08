/*
 * bmgt.h
 *
 *  Created on: May 1, 2017
 *      Author: zhurish
 */

#ifndef __Board_management_H__
#define __Board_management_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "os_include.h"

 
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
  zpl_uint8 type;
  zpl_uint8 port;
  zpl_uint32 phyid;
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


int unit_board_init();
int unit_board_exit();
int unit_board_waitting();
int unit_board_foreach(int (*func)(unit_board_mgt_t *, void *), void *p);
unit_board_mgt_t * unit_board_add(zpl_uint8 unit, zpl_uint8 slot);
int unit_board_del(zpl_uint8 unit, zpl_uint8 slot);
unit_board_mgt_t * unit_board_lookup(zpl_uint8 unit, zpl_uint8 slot);

int unit_board_port_foreach(unit_board_mgt_t*, int (*func)(unit_port_mgt_t *, void *), void *p);
unit_port_mgt_t * unit_board_port_add(unit_board_mgt_t*, zpl_uint8 type, zpl_uint8 port, zpl_uint32 phyid);
int unit_board_port_del(unit_board_mgt_t*, zpl_uint8 type, zpl_uint8 port, zpl_uint32 phyid);

int unit_board_show(void *pvoid);


#ifdef __cplusplus
}
#endif


#endif /* __Board_management_H__ */
