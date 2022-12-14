/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of oRTP.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ortp/ortp.h"
#include "rtptimer.h"
#include "scheduler.h"

void rtp_timer_set_interval(RtpTimer *timer, struct timeval *interval)
{
	if (timer->state==RTP_TIMER_RUNNING){
		ortp_warning("Cannot change timer interval while it is running.\n");
		return;
	}
	timer->interval.tv_sec=interval->tv_sec;
	timer->interval.tv_usec=interval->tv_usec;
}


int rtp_user_timer_add(int (*func)(void *), void *p, int interval)
{
    RtpScheduler *sche = ortp_get_scheduler();
    if(sche)
    {
        int i = 0;
        for(i = 0; i < USER_TIMER_MAX; i++)
        {
            if(sche->_UserTimer[i].state == 0)
            {
                sche->_UserTimer[i].state = 1;
                sche->_UserTimer[i].timer_func = func;
                sche->_UserTimer[i].pdata = p;
                sche->_UserTimer[i].interval = interval;
                ortp_gettimeofday(&sche->_UserTimer[i].timer, NULL);
                return 0;
            }
        }
    }
    return -1;
}


int rtp_user_timer_del(int (*func)(void *), void *p)
{
    RtpScheduler *sche = ortp_get_scheduler();
    if(sche)
    {
        int i = 0;
        for(i = 0; i < USER_TIMER_MAX; i++)
        {
            if(sche->_UserTimer[i].state != 0 &&
                    sche->_UserTimer[i].timer_func == func &&
                    sche->_UserTimer[i].pdata == p)
            {
                sche->_UserTimer[i].state = 0;
                return 0;
            }
        }
    }
    return -1;
}


int rtp_user_timer_call(UserTimer *tt)
{
    int i = 0;
    struct timeval timer;
    ortp_gettimeofday(&timer, NULL);
    for(i = 0; i < USER_TIMER_MAX; i++)
    {
        if(tt[i].state != 0 && tt[i].timer_func &&
                timer.tv_sec >= tt[i].timer.tv_sec &&
                timer.tv_usec >= tt[i].timer.tv_usec)
        {
            tt[i].timer_func(tt[i].pdata);
            tt[i].timer.tv_sec += tt[i].interval/1000;
            tt[i].timer.tv_usec += (tt[i].interval%1000)*1000;
        }
    }
}
