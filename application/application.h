/*
 * application.h
 *
 *  Created on: Dec 22, 2018
 *      Author: zhurish
 */

#ifndef __APPLICATION_H__
#define __APPLICATION_H__


#ifdef APP_X5BA_MODULE
#include "X5-B/mgt/x5_b_a.h"
#include "X5-B/mgt/x5_b_ctl.h"
#include "X5-B/mgt/estate_mgt.h"
#endif

extern void cmd_app_init(void);

#endif /* __APPLICATION_H__ */
