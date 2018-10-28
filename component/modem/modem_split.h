/*
 * modem_split.h
 *
 *  Created on: Aug 3, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_SPLIT_H__
#define __MODEM_SPLIT_H__


extern const char * modem_module_name(modem_t *modem);
extern modem_cpin_en modem_usim_state(modem_t *modem);
extern const char * modem_client_module_name(modem_client_t *client);

extern int modem_operator_split(modem_client_t *client, char *buf);
extern int modem_register_state(modem_client_t *client, int code);

extern int modem_pin_state_split(modem_client_t *client, char *buf);

extern int modem_qnwinfo_split(modem_client_t *client, char *buf);

extern const char *modem_pdp_cmd(modem_t *modem);

const char *modem_operator_apn_cmd(modem_t *modem);


extern const char *modem_operator_svc_cmd(modem_t *modem);
extern const char *modem_svc_cmd(modem_t *modem);



#endif /* __MODEM_SPLIT_H__ */
