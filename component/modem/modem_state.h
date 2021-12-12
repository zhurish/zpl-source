/*
 * modem_state.h
 *
 *  Created on: Aug 12, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_STATE_H__
#define __MODEM_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "modem.h"
#include "modem.h"
#include "modem_client.h"

#define MODEM_SIGNAL_LOW	90
#define MODEM_BITERR_HIGH	80

#define MODEM_SIGNAL_SET(v, n)	(v) |= (1<<(n))
#define MODEM_SIGNAL_CLR(v)	(v) = 0
#define MODEM_SIGNAL_GET(v, n)	((v) & (1<<(n)))






extern const char *modem_dial_string(modem_dial_type type);

extern const char *modem_network_type_string(modem_network_type type);

extern int modem_network_type_id(modem_network_type type);

extern modem_network_type modem_network_type_get(const char * type);

extern int modem_signal_state_update(modem_client_t *client);
extern modem_signal_state modem_signal_state_get(modem_client_t *client);


extern int modem_register_state(modem_client_t *client, zpl_uint32 code);
extern modem_cpin_en modem_usim_state(modem_t *modem);


#ifdef __cplusplus
}
#endif

#endif /* __MODEM_STATE_H__ */
