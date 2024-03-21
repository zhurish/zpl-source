
#ifndef __LIB_GLOBAL_H
#define _LIB_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif


struct lib_global
{
  zpl_uint32 module;
  zpl_char *string;

  zpl_void  *master;
  zpl_void  *vty;
};

extern void lib_global_init (void);
extern struct lib_global *lib_global_lookup (const char *);
extern struct key *lib_global_lookup_module (zpl_uint32);

 
#ifdef __cplusplus
}
#endif

#endif /* __LIB_GLOBAL_H */
