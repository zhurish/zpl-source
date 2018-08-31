/*
 * if_hook.h
 *
 *  Created on: 2017Äê7ÔÂ8ÈÕ
 *      Author: zhurish
 */

#ifndef PLATFORM_LIB_IF_HOOK_H_
#define PLATFORM_LIB_IF_HOOK_H_

/* Interface hook sort. */
#define IF_NEW_HOOK   0
#define IF_DELETE_HOOK 1
#define IF_ADDR_NEW_HOOK   2
#define IF_ADDR_DELETE_HOOK 3
#define IF_STATUS_UP_HOOK   4
#define IF_STATUS_DOWN_HOOK 5
//#define IF_SHOW_HOOK 6


struct if_hook
{
	  int protocol;
	  int (*if_new_hook) (struct interface *);
	  int (*if_delete_hook) (struct interface *);
	  int (*if_address_add_hook) (struct interface *, struct connected *);
	  int (*if_address_del_hook) (struct interface *, struct connected *);
	  int (*if_status_up_hook) (struct interface *);
	  int (*if_status_down_hook) (struct interface *);
//	  int (*if_show_hook) (struct interface *);
};


extern void if_hook_init (void);
extern void if_hook_exit (void);

extern struct if_hook * if_hook_lookup(int protocol);

extern void if_new_hook_call(int protocol, struct interface *ifp);
extern void if_delete_hook_call(int protocol, struct interface *ifp);
extern void if_address_add_hook_call(int protocol, struct interface *ifp, struct connected *p);
extern void if_address_delete_hook_call(int protocol, struct interface *ifp, struct connected *p);
extern void if_status_up_hook_call(int protocol, struct interface *ifp);
extern void if_status_down_hook_call(int protocol, struct interface *ifp);
//extern void if_show_hook_call(int protocol, struct interface *ifp);

extern void if_hook_all(int cmd, struct interface *ifp);
extern void if_address_hook_all(int cmd, struct interface *ifp, struct connected *p);
extern void if_status_hook_all(int cmd, struct interface *ifp);
//extern void if_show_hook_all(struct interface *ifp);

extern void if_add_hook (int cmd, int protocol, int (*func)(struct interface *));
extern void if_status_hook (int cmd, int protocol, int (*func)(struct interface *));
extern void if_address_hook (int cmd, int protocol, int (*func)(struct interface *,struct connected *));
//extern void if_show_hook (int protocol, int (*func)(struct interface *));


#endif /* PLATFORM_LIB_IF_HOOK_H_ */
