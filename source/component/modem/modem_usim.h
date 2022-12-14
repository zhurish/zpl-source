/*
 * modem_usim.h
 *
 *  Created on: Jul 28, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_USIM_H__
#define __MODEM_USIM_H__

#ifdef __cplusplus
extern "C" {
#endif


extern int modem_pin_state_split(modem_client_t *client, char *buf);
extern int modem_usim_detection(modem_t *modem);



#ifdef __cplusplus
}
#endif

#endif /* __MODEM_USIM_H__ */
