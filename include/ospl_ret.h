/*
 * ospl_ret.h
 *
 *  Created on: Jan 8, 2018
 *      Author: zhurish
 */

#ifndef __OSPL_RET_H__
#define __OSPL_RET_H__

#ifdef __cplusplus
extern "C" {
#endif



enum
{
  OK  = 0,
  ERROR = -1,
//#ifndef HAVE_OS_TIMEOUT
  OS_TIMEOUT  = -2,
//#endif
  OS_CTRL_X  = -3,
  OS_TRY_AGAIN  = -4,
  OS_EXIST		= -100,
  OS_NOTEXIST  = -101,
};



#ifdef __cplusplus
}
#endif

#endif /* __OSPL_RET_H__ */
