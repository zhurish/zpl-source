/*
 * modem_proxy.h
 *
 *  Created on: Nov 13, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_PROXY_H__
#define __MODEM_PROXY_H__


extern int modem_proxy_enable(const char *name, int fd, BOOL close);
extern int modem_proxy_disable(const char *name);

#endif /* __MODEM_PROXY_H__ */
