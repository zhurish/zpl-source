/*
 * template.h
 *
 *  Created on: Dec 22, 2018
 *      Author: zhurish
 */

#ifndef __LIB_TEMPLATE_H__
#define __LIB_TEMPLATE_H__

#define TEMPLATE_NAME_MAX	64


typedef struct template_s
{
	//NODE	node;
	int 	module;
	char 	name[TEMPLATE_NAME_MAX];
	char	prompt[64];

	int		id;

	void	*pVoid;

	int		(*write_template)(struct vty *, void *);
	int		(*show_template)(struct vty *, void *, BOOL);
}template_t;

extern void nsm_template_init (void);
extern void nsm_template_exit (void);

extern template_t * nsm_template_new (void);
extern void nsm_template_free (template_t *template);
extern void nsm_template_install (template_t *template, int module);
extern template_t* nsm_template_lookup (int module);
extern template_t* nsm_template_lookup_name (char * name);

extern int nsm_template_write_config (struct vty *vty);
extern int nsm_template_show_config (struct vty *vty, BOOL detail);

#endif /* __LIB_TEMPLATE_H__ */
