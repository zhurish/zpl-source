
#ifndef _ZEBRA_LIB_GLOBAL_H
#define _ZEBRA_LIB_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_type.h"
#include "zmemory.h"
#include "module.h"
#include "memtypes.h"
#include "zebra_event.h"
#include "route_types.h"
#include "prefix.h"
#include "zassert.h"

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

#endif /* _ZEBRA_LIB_GLOBAL_H */
