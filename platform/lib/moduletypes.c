#include "zpl_include.h"
#include "module.h"
#include "moduletypes.h"
 
extern struct module_list module_list_default;
extern struct module_list module_list_lib;
extern struct module_list module_list_osal;
extern struct module_list module_list_timer;
extern struct module_list module_list_job;
extern struct module_list module_list_console;
extern struct module_list module_list_telnet;
extern struct module_list module_list_utils;
extern struct module_list module_list_imish;
extern struct module_list module_list_kernel;
extern struct module_list module_list_nsmdhcp;
extern struct module_list module_list_nsm;
extern struct module_list module_list_sdk;
extern struct module_list module_list_pal;
extern struct module_list module_list_hal;
 
struct module_alllist module_lists_tbl[MODULE_MAX] = {
  &module_list_default,
  &module_list_lib,
  &module_list_osal,
  &module_list_timer,
  &module_list_job,
  &module_list_console,
  &module_list_telnet,
  &module_list_utils,
  &module_list_imish,
  &module_list_kernel,
  &module_list_nsmdhcp,
  &module_list_nsm,
  &module_list_sdk,
  &module_list_pal,
  &module_list_hal,
 NULL,
};
