/*
 * modem_machine.h
 *
 *  Created on: Jul 26, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_MACHINE_H__
#define __MODEM_MACHINE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "modem_event.h"

#define MDMS(n)		MODEM_MACHINE_STATE_## n

typedef enum modem_machine
{
	MODEM_MACHINE_STATE_NONE = 0,
	MODEM_MACHINE_STATE_NO_USIM_CARD,		//USIM卡拔出
	MODEM_MACHINE_STATE_NO_SIGNAL,		//信号弱
	MODEM_MACHINE_STATE_NO_ADDR,			//没有IP地址
	MODEM_MACHINE_STATE_NO_SERVICE,
	MODEM_MACHINE_STATE_NETWORK_ACTIVE,		// network is already active

	MODEM_MACHINE_STATE_MAX,

}modem_machine;


typedef struct //modem_machine_action
{
	modem_machine	oldstate;
	modem_machine	newstate;
	modem_event		event;
	//int		(*action)(modem_t *, modem_event );
}modem_machine_action;

extern const char *modem_machine_state_string(modem_machine state);
extern int	modem_machine_state_action(modem_t *modem);
extern int modem_machine_state_set(modem_t *modem, modem_machine newstate);
extern modem_machine modem_machine_state_get(modem_t *modem);
extern int modem_machine_state(modem_t *modem);


extern int modem_machine_state_show(modem_t *modem, struct vty *vty, zpl_bool detail);



#ifdef __cplusplus
}
#endif

#endif /* __MODEM_MACHINE_H__ */
