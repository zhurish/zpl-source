/*
 * voip_uci.h
 *
 *  Created on: 2019年3月12日
 *      Author: DELL
 */

#ifndef __VOIP_UCI_H__
#define __VOIP_UCI_H__

#ifdef __cplusplus
extern "C" {
#endif

//#define ZPL_OPENWRT_UCI
#ifdef ZPL_OPENWRT_UCI

extern int voip_uci_sip_config_load(void *sip);
extern int voip_uci_sip_config_save(void *sip);
extern int voip_stream_config_save(void *voip);

extern int voip_stream_config_update_api(void *voip);
extern int voip_stream_config_load(void *voip);

extern int voip_ubus_uci_update_cb(void *p, char *buf, int len);

#endif

extern zpl_bool voip_global_enabled();

extern int voip_status_register_api(int reg);
extern int voip_status_register_main_api(zpl_bool reg);
extern int voip_status_talk_api(zpl_bool reg);
extern int voip_status_enable_api(zpl_bool reg);
extern int voip_status_clear_api();

#ifdef __cplusplus
}
#endif

#endif /* __VOIP_UCI_H__ */
