/*
 * modem_proxy.h
 *
 *  Created on: Nov 13, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_PROXY_H__
#define __MODEM_PROXY_H__


#ifdef __cplusplus
extern "C" {
#endif

extern int modem_proxy_enable(const char *name, int fd, zpl_bool close);
extern int modem_proxy_disable(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* __MODEM_PROXY_H__ */
