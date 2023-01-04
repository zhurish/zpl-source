#ifndef _MODULE_TYPES_H
#define _MODULE_TYPES_H
 
 
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
  MODULE_BSP,
  MODULE_TXRX,
  MODULE_HAL, 
  MODULE_PAL, 
  MODULE_SNTP, 
  MODULE_SNTPS, 
  MODULE_UTILS, 
  MODULE_SDK, 
  MODULE_RTSP,
  MODULE_ZPLMEDIA,
	MODULE_MAX,
} module_t;
 
#endif /* _MODULE_TYPES_H */
