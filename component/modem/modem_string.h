/*
 * modem_string.h
 *
 *  Created on: Aug 4, 2018
 *      Author: zhurish
 */

#ifndef __MODEM_STRING_H__
#define __MODEM_STRING_H__


/*
 * 去掉 \r\n
 */
//extern const char * atcmd_split(char *, int);

/*
 * 去掉 ""
 */
extern const char * strchr_empty(char *, const char );
extern const char * strchr_empty_step(char *src, const char em, int step);

#endif /* __MODEM_STRING_H__ */
