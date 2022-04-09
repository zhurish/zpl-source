#ifndef _MODULE_TYPES_H
#define _MODULE_TYPES_H
 
#include "route_types.h"
#include "zebra_event.h"
 
typedef enum {
	MODULE_NONE = 0,
  MODULE_DEFAULT, 
  MODULE_LIB, 
  MODULE_OSAL, 
  MODULE_TIMER, 
  MODULE_JOB, 
  MODULE_CONSOLE,
  MODULE_TELNET,
  MODULE_RIB,
  MODULE_NSM,
  MODULE_HAL, 
  MODULE_PAL, 
  MODULE_BSP,
  MODULE_SDK, 
	MODULE_MAX,
} module_t;
 
#endif /* _MODULE_TYPES_H */
