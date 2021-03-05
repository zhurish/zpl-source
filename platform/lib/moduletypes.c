#include "zebra.h"
#include "moduletypes.h"
 
extern struct module_list module_list_pjsip;
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
extern struct module_list module_list_nsmdebug;
extern struct module_list module_list_nsmdot1x;
extern struct module_list module_list_nsmarp;
extern struct module_list module_list_nsmbridge;
extern struct module_list module_list_nsmdhcp;
extern struct module_list module_list_nsm;
extern struct module_list module_list_sntpc;
extern struct module_list module_list_sntps;
extern struct module_list module_list_pal;
extern struct module_list module_list_hal;
extern struct module_list module_list_modem;
extern struct module_list module_list_dhcp;
extern struct module_list module_list_wifi;
extern struct module_list module_list_mqtt;
extern struct module_list module_list_webserver;
extern struct module_list module_list_app;
 
struct module_alllist module_lists_tbl[MODULE_MAX] = {
  &module_list_pjsip,
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
  &module_list_nsmdebug,
  &module_list_nsmdot1x,
  &module_list_nsmarp,
  &module_list_nsmbridge,
  &module_list_nsmdhcp,
  &module_list_nsm,
  &module_list_sntpc,
  &module_list_sntps,
  &module_list_pal,
  &module_list_hal,
  &module_list_modem,
  &module_list_dhcp,
  &module_list_wifi,
  &module_list_mqtt,
  &module_list_webserver,
  &module_list_app,
 NULL,
};
