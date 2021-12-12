/*
 * modem_api.h
 *
 *  Created on: Jul 29, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_API_H__
#define __MODEM_API_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int modem_main_change_set_api(modem_t *modem, modem_event event);

extern int modem_main_apn_set_api(modem_t *modem, char *apn);
extern int modem_main_svc_set_api(modem_t *modem, char *svc);
extern int modem_main_pin_set_api(modem_t *modem, char *pin);
extern int modem_main_puk_set_api(modem_t *modem, char *puk);

extern int modem_main_ip_set_api(modem_t *modem, modem_stack_type type);
extern int modem_main_ip_get_api(modem_t *modem, modem_stack_type *type);
extern int modem_main_secondary_set_api(modem_t *modem, zpl_bool bSecondary);
extern int modem_main_secondary_get_api(modem_t *modem, zpl_bool *bSecondary);

extern int modem_main_profile_set_api(modem_t *modem, zpl_uint32 profile);
extern int modem_main_profile_get_api(modem_t *modem, zpl_uint32 *profile);
extern int modem_main_dial_set_api(modem_t *modem, modem_dial_type type);
extern int modem_main_dial_get_api(modem_t *modem, modem_dial_type *type);

extern int modem_main_network_set_api(modem_t *modem, modem_network_type profile);
extern int modem_main_network_get_api(modem_t *modem, modem_network_type *profile);


modem_t * modem_lookup_by_interface(char *name);
extern int modem_interface_add_api(char *name);

extern int modem_bind_interface_api(modem_t *modem, char *name, zpl_uint32 number);
extern int modem_unbind_interface_api(modem_t *modem, zpl_bool ppp, zpl_uint32 number);



#ifdef __cplusplus
}
#endif


#endif /* __MODEM_API_H__ */
