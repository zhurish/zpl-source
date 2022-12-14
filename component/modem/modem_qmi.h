/*
 * modem_qmi.h
 *
 *  Created on: Jul 28, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_QMI_H__
#define __MODEM_QMI_H__


//#define _MODEM_QMI_DEBUG
//#define _MODEM_QMI_DEBUG

extern int modem_qmi_start(modem_t *modem);
extern int modem_qmi_stop(modem_t *modem);
extern int modem_qmi_exit(modem_t *modem);

extern BOOL modem_qmi_isconnect(modem_t *modem);
extern BOOL modem_qmi_islinkup(modem_t *modem);


#ifdef _MODEM_QMI_DEBUG
#define MODEM_QMI_DEBUG(fmt,...)	modem_debug_printf(stderr, __func__, __LINE__,fmt, ##__VA_ARGS__)
#else
#define MODEM_QMI_DEBUG(fmt,...)
#endif


#endif /* __MODEM_QMI_H__ */
