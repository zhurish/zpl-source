#ifndef pjsipTimer_HPP
#define pjsipTimer_HPP
#include <pjsua2.hpp>
#include <iostream>

using namespace pj;
using namespace std;

class pjsipTimer
{
public:
    pjsipTimer();
    pjsipTimer(int delay, int period);
    ~pjsipTimer();

    /*
    定时器类型
     */
    int PJSIP_SAMPLE_TALKING_TIMER = 1;
    int PJSIP_SAMPLE_CALLING_TIMER = 2;
    int PJSIP_SAMPLE_REGISTER_TIMER = 3;
    int PJSIP_SAMPLE_STOP_TALKING_TIMER = 4;

    int timeout_period = 0; //定时器计数
    bool run = false;
    int pjsipTimerStop(int type);
    int pjsipTimerRestart(int type);
    bool pjsipTimerIsEnable(int type);
    bool pjsipTimerIsTimerout(int type);
    int pjsipTimerCount(int type);

    int pjsipTimerCancel();

    int pjsipTimerHandle();

private:
    int __PJSIP_TALKING_TIMER = 60;     //通话时间
    int __PJSIP_CALLING_TIMER = 30;     //呼叫超时时间
    int __PJSIP_REGISTER_TIMER = 3600;  //注册时间
    int __PJSIP_STOP_TALKING_TIMER = 5; //

    int talking_timeout = 0;      //通话时间计数
    int calling_timeout = 0;      //呼叫计数
    int register_timeout = 0;     //注册计数
    int stop_talking_timeout = 0; //注册计数

    bool talking_timer_start = false;      //通话计数开始
    bool calling_timer_start = false;      //呼叫计数开始
    bool register_timer_start = false;     //注册计数开始
    bool stop_talking_timer_start = false; //注册计数开始
};

#endif // pjsipTimer_HPP
