
#ifndef _ZEBRA_LIB_GLOBAL_H
#define _ZEBRA_LIB_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "zpl_type.h"
#include "module.h"
#include "zmemory.h"
#include "linklist.h"
#include "prefix.h"
#include "table.h"
#include "hash.h"
#include "jhash.h"
#include "str.h"
#include "log.h"
#include "vector.h"
#include "algorithm.h"
#include "checksum.h"
#include "vector.h"

#ifdef ZPL_VRF_MODULE
#include "vrf.h"
#endif
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
