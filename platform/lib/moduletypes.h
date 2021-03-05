#ifndef _MODULE_TYPES_H
#define _MODULE_TYPES_H
 
typedef enum {
	MODULE_NONE = 0,
  MODULE_PJSIP, 
  MODULE_DEFAULT, 
  MODULE_LIB, 
  MODULE_OSAL, 
  MODULE_TIMER, 
  MODULE_JOB, 
  MODULE_CONSOLE, 
  MODULE_TELNET, 
  MODULE_UTILS, 
  MODULE_IMISH, 
  MODULE_KERNEL, 
  MODULE_NSMDEBUG, 
  MODULE_NSMDOT1X, 
  MODULE_NSMARP, 
  MODULE_NSMBRIDGE, 
  MODULE_NSMDHCP, 
  MODULE_NSM,
  MODULE_SNTP, 
  MODULE_SNTPS, 
  MODULE_PAL, 
  MODULE_HAL, 
  MODULE_MODEM, 
  MODULE_DHCP, 
  MODULE_WIFI, 
  MODULE_MQTT, 
  MODULE_WEB, 
  MODULE_APP, 
	MODULE_MAX,
} module_t;
 
#endif /* _MODULE_TYPES_H */
