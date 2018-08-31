/*
 * modem_message.h
 *
 *  Created on: Jul 26, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_MESSAGE_H__
#define __MODEM_MESSAGE_H__


typedef enum
{
	MD_SMS_PDU  = 0,
	MD_SMS_TEXT  = 1,
}modem_message_mode;


typedef struct modem_message_s
{
	modem_client_t *client;

	modem_message_mode	sms_mode;


}modem_message_t;


#endif /* __MODEM_MESSAGE_H__ */
