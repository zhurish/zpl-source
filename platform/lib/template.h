/*
 * template.h
 *
 *  Created on: Dec 22, 2018
 *      Author: zhurish
 */

#ifndef __LIB_TEMPLATE_H__
#define __LIB_TEMPLATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TEMPLATE_NAME_MAX	64


typedef struct template_s
{
	//NODE	node;
	ospl_uint32 	module;
	ospl_char 	name[TEMPLATE_NAME_MAX];
	ospl_char	prompt[64];

	ospl_uint32		id;

	void	*pVoid;

	int		(*write_template)(struct vty *, void *);
	int		(*show_template)(struct vty *, void *, ospl_bool);
	int		(*show_debug)(struct vty *, void *, ospl_bool);
	ospl_bool 	service;
}template_t;

extern void nsm_template_init (void);
extern void nsm_template_exit (void);

extern template_t * nsm_template_new (ospl_bool service);
extern void nsm_template_free (template_t *template);
extern void nsm_template_install (template_t *template, ospl_uint32 module);
extern template_t* nsm_template_lookup (ospl_bool service, ospl_uint32 module);
extern template_t* nsm_template_lookup_name (ospl_bool service, ospl_char * name);

extern int nsm_template_write_config (struct vty *vty);
extern int nsm_template_show_config (struct vty *vty, ospl_bool detail);
extern int nsm_template_service_write_config (struct vty *vty);
extern int nsm_template_service_show_config (struct vty *vty, ospl_bool detail);

extern int nsm_template_debug_write_config (struct vty *vty);
extern int nsm_template_debug_show_config (struct vty *vty, ospl_bool detail);
 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_TEMPLATE_H__ */
