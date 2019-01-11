/*
 * voip_dbtest.h
 *
 *  Created on: 2019年1月8日
 *      Author: DELL
 */

#ifndef __VOIP_DBTEST_H__
#define __VOIP_DBTEST_H__

#include "zebra.h"
#include "voip_app.h"

#define VOIP_ROOM_PHONE_DBTEST SYSCONF_REAL_DIR"/phonedbtest.cfg"

#define VOIP_ROOM_PHONE_DBTEST_MAX		16

typedef struct room_phone_dbtest
{
	u_int8	use;
	u_int32	room;
	char	phone[VOIP_PHONE_MAX];
	u_int8	plen;
	char	address[16];
	u_int16	port;
}room_phone_dbtest_t;



extern int voip_dbtest_enable(BOOL enable);
extern BOOL voip_dbtest_isenable();
extern int voip_dbtest_add(room_phone_dbtest_t one);
extern int voip_dbtest_del(room_phone_dbtest_t one);
extern int voip_dbtest_lookup(char *room, room_phone_dbtest_t *out);
extern int voip_dbtest_getphone(char *room, char *phone);
extern int voip_dbtest_getremote(char *room, char *address, u_int16 *port);

//extern int voip_dbtest_add_sync(char *room, char *phone);
extern int voip_dbtest_load();

extern int voip_dbtest_show(struct vty *vty, int detail);

#endif /* __VOIP_DBTEST_H__ */
