/*
 * application.h
 *
 *  Created on: Dec 22, 2018
 *      Author: zhurish
 */

#ifndef __APPLICATION_H__
#define __APPLICATION_H__


#ifdef APP_X5BA_MODULE

#include "X5-B/mgt/x5_b_app.h"
#include "X5-B/mgt/x5_b_cmd.h"
#include "X5-B/mgt/x5_b_ctl.h"
#include "X5-B/mgt/x5b_dbase.h"
#include "X5-B/mgt/x5b_facecard.h"
#endif

extern void cmd_app_init(void);

#endif /* __APPLICATION_H__ */
