#include "zpl_include.h"
#include "lib_include.h"
 
extern struct module_list module_list_default;
extern struct module_list module_list_lib;
extern struct module_list module_list_osal;
extern struct module_list module_list_timer;
extern struct module_list module_list_job;
extern struct module_list module_list_console;
extern struct module_list module_list_telnet;
extern struct module_list module_list_nsm;
extern struct module_list module_list_hal;
extern struct module_list module_list_pal;
extern struct module_list module_list_bsp;
extern struct module_list module_list_sdk;
 
struct module_alllist module_lists_tbl[MODULE_MAX] = {
  &module_list_default,
  &module_list_lib,
  &module_list_osal,
  &module_list_timer,
  &module_list_job,
  &module_list_console,
  &module_list_telnet,
  &module_list_nsm,
  &module_list_hal,
  &module_list_pal,
  &module_list_bsp,
  &module_list_sdk,
 NULL,
};
 
 
 
