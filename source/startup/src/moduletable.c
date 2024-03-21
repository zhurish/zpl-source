#include "zplos_include.h"
#include "module.h"
#include "log.h"
 
extern struct module_list module_list_lib;
extern struct module_list module_list_osal;
extern struct module_list module_list_shell;
extern struct module_list module_list_standby;
extern struct module_list module_list_rib;
extern struct module_list module_list_nsmdhcp;
extern struct module_list module_list_nsm;
extern struct module_list module_list_bsp;
extern struct module_list module_list_txrx;
extern struct module_list module_list_hal;
extern struct module_list module_list_pal;
extern struct module_list module_list_modem;
extern struct module_list module_list_wifi;
extern struct module_list module_list_mqtt;
extern struct module_list module_list_webserver;
extern struct module_list module_list_modbus;
extern struct module_list module_list_libssh;
extern struct module_list module_list_utils;
extern struct module_list module_list_sdk;
extern struct module_list module_list_rtsp;
extern struct module_list module_list_zplmedia;
extern struct module_list module_list_medie_proxy;
extern struct module_list module_list_pjsip;
 
struct module_alllist module_lists_tbl[MODULE_MAX] = {
  &module_list_lib,
  &module_list_osal,
  &module_list_shell,
  &module_list_standby,
  &module_list_rib,
  &module_list_nsmdhcp,
  &module_list_nsm,
  &module_list_bsp,
  &module_list_txrx,
  &module_list_hal,
  &module_list_pal,
  &module_list_modem,
  &module_list_wifi,
  &module_list_mqtt,
  &module_list_webserver,
  &module_list_modbus,
  &module_list_libssh,
  &module_list_utils,
  &module_list_sdk,
  &module_list_rtsp,
  &module_list_zplmedia,
  &module_list_medie_proxy,
  &module_list_pjsip,
 NULL,
};
 
 
 
