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
#ifdef ZPL_SHELL_MODULE
#define TEMPLATE_NAME_MAX	64


typedef struct template_s
{
	//NODE	node;
	zpl_uint32 	module;
	zpl_char 	name[TEMPLATE_NAME_MAX];
	zpl_char	prompt[64];

	zpl_uint32		id;

	void	*pVoid;

	int		(*write_template)(struct vty *, void *);
	int		(*show_template)(struct vty *, void *, zpl_bool);
	int		(*show_debug)(struct vty *, void *, zpl_bool);
	zpl_bool 	service;
}template_t;

extern void nsm_template_init (void);
extern void nsm_template_exit (void);

extern template_t * nsm_template_new (zpl_bool service);
extern void nsm_template_free (template_t *template);
extern void nsm_template_install (template_t *template, zpl_uint32 module);
extern void nsm_config_list_install (template_t *template, zpl_uint32 module);
extern template_t* nsm_template_lookup (zpl_bool service, zpl_uint32 module);
extern template_t* nsm_template_lookup_name (zpl_bool service, zpl_char * name);

extern int nsm_template_write_config (struct vty *vty);
extern int nsm_template_show_config (struct vty *vty, zpl_bool detail);
extern int nsm_template_service_write_config (struct vty *vty);
extern int nsm_template_service_show_config (struct vty *vty, zpl_bool detail);

extern int nsm_template_debug_write_config (struct vty *vty);
extern int nsm_template_debug_show_config (struct vty *vty, zpl_bool detail);

extern int cmd_nsm_template_init(void);
#endif 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_TEMPLATE_H__ */
