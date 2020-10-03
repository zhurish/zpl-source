/*
 * show_hook.h
 *
 *  Created on: 2017Äê7ÔÂ16ÈÕ
 *      Author: zhurish
 */

#ifndef __SHOW_HOOK_H__
#define __SHOW_HOOK_H__


struct show_hook
{
	  int protocol;
	  int (*show_hook) (struct vty *);
	  int (*show_debug_hook) (struct vty *);
	  int (*show_interface_hook) (struct vty *, struct interface *);
};


extern void show_hook_init (void);
extern void show_hook_exit (void);
extern void show_hook_add (int protocol, int (*func)(struct vty *));
extern void show_debug_hook_add (int protocol, int (*func)(struct vty *));
extern void show_interface_hook_add (int protocol, int (*func)(struct vty *,struct interface *));

extern void show_hook_all(struct vty *);
extern void show_debug_hook_all(struct vty *);
extern void show_interface_hook_all(struct vty *, struct interface *);


#endif /* __SHOW_HOOK_H__ */
