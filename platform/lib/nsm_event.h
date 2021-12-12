/* NSM event header.
 * Copyright (C) 1999 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
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

#ifndef __NSM_EVENT_H__
#define __NSM_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
	NSM_EVENT_NONE = 0,
	NSM_EVENT_IFP_ADD,	//增加接口
	NSM_EVENT_IFP_DEL,	//删除接口
	NSM_EVENT_IFP_UP,	//接口UP/DOWN
	NSM_EVENT_IFP_DOWN,
	NSM_EVENT_IP_ADD,	//接口添加IP
	NSM_EVENT_IP_DEL,	//接口删除IP
	NSM_EVENT_IFP_CHANGE,//接口参数变化
	NSM_EVENT_IFP_SHOW,	//show 接口信息
	NSM_EVENT_IFP_CONFIG,//write config 接口信息
	NSM_EVENT_SERVICE,	//接口删除IP
	NSM_EVENT_DEBUG,//接口参数变化
	NSM_EVENT_MAX,

}nsm_event_e;

typedef struct
{
  nsm_event_e event;
  zpl_uint32 module;
  ifindex_t ifindex;
  zpl_bool bval;
  zpl_uint8 u8val;
  zpl_uint16 u16val;
  zpl_uint32 u32val;
  union 
  {
    struct 
    {
        struct interface *ifp;
        struct connected *ifc; 
        zpl_bool second;
    } ifaddr;
    struct 
    {
        struct interface *ifp;
    } ifev;

  } evdata;// __attribute__ ((aligned (8)));

}nsm_event_data_t;
 
typedef int (*nsm_event_cb) (nsm_event_e, nsm_event_data_t *);



int nsm_event_init (void);
int nsm_event_exit (void);
int nsm_event_loop_start (void *m);
int nsm_event_loop_stop ();
int nsm_event_register_api (zpl_uint32 module, nsm_event_cb cb);

int nsm_event_post_data_api (zpl_uint32 module, nsm_event_e event, nsm_event_data_t *data);
int nsm_event_post_api (nsm_event_data_t *data);


#ifdef __cplusplus
}
#endif

#endif /* __NSM_EVENT_H__ */
