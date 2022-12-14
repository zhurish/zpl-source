#include "pjsiptimer.hpp"

using namespace pj;
using namespace std;

pjsipTimer::pjsipTimer()
{
    timeout_period = 1;
    run = true;
}

pjsipTimer::~pjsipTimer()
{
    run = false;
}

pjsipTimer::pjsipTimer(int delay, int period)
{
    //_pjsipSampleimerPtr = new Timer();
    //if (_pjsipSampleimerPtr != null)
    if (period > 1000)
        timeout_period = period / 1000;
    else
        timeout_period = 1;
    run = true;
    //_pjsipSampleimerPtr.schedule(pjsipTimerCallTask, delay, period);
}

int pjsipTimer::pjsipTimerStop(int type)
{
    if (type == PJSIP_SAMPLE_TALKING_TIMER)
    {
        talking_timer_start = false; //呼叫计数开始
        talking_timeout = 0;         //呼叫计数开始
    }
    else if (type == PJSIP_SAMPLE_CALLING_TIMER)
    {
        calling_timer_start = false; //呼叫计数开始
        calling_timeout = 0;         //呼叫计数开始
    }
    else if (type == PJSIP_SAMPLE_REGISTER_TIMER)
    {
        register_timer_start = false; //呼叫计数开始
        register_timeout = 0;         //呼叫计数开始
    }
    else if (type == PJSIP_SAMPLE_STOP_TALKING_TIMER)
    {
        stop_talking_timer_start = false;
        stop_talking_timeout = 0;
    }
    else
    {
        register_timer_start = false; //呼叫计数开始
        register_timeout = 0;         //呼叫计数开始
        calling_timer_start = false;  //呼叫计数开始
        calling_timeout = 0;          //呼叫计数开始
        talking_timer_start = false;  //呼叫计数开始
        talking_timeout = 0;          //呼叫计数开始
        stop_talking_timer_start = false;
        stop_talking_timeout = 0;
    }
    return 0;
}

int pjsipTimer::pjsipTimerRestart(int type)
{

    if (type == PJSIP_SAMPLE_TALKING_TIMER)
    {
        calling_timer_start = false;
        talking_timer_start = true;                                                                                //呼叫计数开始
        talking_timeout = (timeout_period > 0) ? (__PJSIP_TALKING_TIMER / timeout_period) : __PJSIP_TALKING_TIMER; //呼叫计数开始
    }
    else if (type == PJSIP_SAMPLE_CALLING_TIMER)
    {
        talking_timer_start = false;
        calling_timer_start = true;                                                                                //呼叫计数开始
        calling_timeout = (timeout_period > 0) ? (__PJSIP_CALLING_TIMER / timeout_period) : __PJSIP_CALLING_TIMER; //呼叫计数开始
    }
    else if (type == PJSIP_SAMPLE_REGISTER_TIMER)
    {
        register_timer_start = true;                                                                                  //呼叫计数开始
        register_timeout = (timeout_period > 0) ? (__PJSIP_REGISTER_TIMER / timeout_period) : __PJSIP_REGISTER_TIMER; //呼叫计数开始
    }
    else if (type == PJSIP_SAMPLE_STOP_TALKING_TIMER)
    {
        stop_talking_timer_start = true;
        stop_talking_timeout = (timeout_period > 0) ? (__PJSIP_STOP_TALKING_TIMER / timeout_period) : __PJSIP_STOP_TALKING_TIMER; //呼叫计数开始
    }
    return 0;
}

bool pjsipTimer::pjsipTimerIsEnable(int type)
{
    if (type == PJSIP_SAMPLE_TALKING_TIMER && talking_timer_start)
    {
        return true;
    }
    else if (type == PJSIP_SAMPLE_CALLING_TIMER && calling_timer_start)
    {
        return true;
    }
    else if (type == PJSIP_SAMPLE_REGISTER_TIMER && register_timer_start)
    {
        return true;
    }
    else if (type == PJSIP_SAMPLE_STOP_TALKING_TIMER && stop_talking_timer_start)
    {
        return true;
    }
    return false;
}

bool pjsipTimer::pjsipTimerIsTimerout(int type)
{
    if (type == PJSIP_SAMPLE_TALKING_TIMER && talking_timer_start)
    {
        if (talking_timeout <= 0)
        {
            //talking_timer_start = false;
            return true;
        }
    }
    else if (type == PJSIP_SAMPLE_CALLING_TIMER && calling_timer_start)
    {
        if (calling_timeout <= 0)
        {
            //calling_timer_start = false;
            return true;
        }
    }
    else if (type == PJSIP_SAMPLE_REGISTER_TIMER && register_timer_start)
    {
        if (register_timeout <= 0)
        {
            //register_timer_start = false;
            return true;
        }
    }
    else if (type == PJSIP_SAMPLE_STOP_TALKING_TIMER && stop_talking_timer_start)
    {
        if (stop_talking_timeout <= 0)
        {
            //stop_talking_timer_start = false;
            return true;
        }
    }
    return false;
}

int pjsipTimer::pjsipTimerCount(int type)
{
    if (type == PJSIP_SAMPLE_TALKING_TIMER && talking_timer_start)
    {
        return talking_timeout;
    }
    else if (type == PJSIP_SAMPLE_CALLING_TIMER && calling_timer_start)
    {
        return calling_timeout;
    }
    else if (type == PJSIP_SAMPLE_REGISTER_TIMER && register_timer_start)
    {
        return register_timeout;
    }
    else if (type == PJSIP_SAMPLE_STOP_TALKING_TIMER && stop_talking_timer_start)
    {
        return stop_talking_timeout;
    }
    return -1;
}

int pjsipTimer::pjsipTimerHandle()
{

    if (talking_timer_start)
    {
        if (talking_timeout > 0)
            talking_timeout--; //呼叫计数开始
        //System.out.println("------------pjsipTimerHandle-----------talking_timeout " + talking_timeout);
    }
    else if (calling_timer_start)
    {
        if (calling_timeout > 0)
            calling_timeout--; //呼叫计数开始
        //System.out.println("------------pjsipTimerHandle-----------calling_timeout " + calling_timeout);
    }
    else if (register_timer_start)
    {
        if (register_timeout > 0)
            register_timeout--; //呼叫计数开始
        //System.out.println("------------pjsipTimerHandle-----------register_timeout " + register_timeout);
    }
    else if (stop_talking_timer_start)
    {
        if (stop_talking_timeout > 0)
            stop_talking_timeout--; //呼叫计数开始
        //System.out.println("------------pjsipTimerHandle-----------register_timeout " + register_timeout);
    }
    return 0;
}

int pjsipTimer::pjsipTimerCancel()
{
    pjsipTimerStop(0);
    //if (_pjsipSampleimerPtr != null)
    //    _pjsipSampleimerPtr.cancel();
    return 0;
}
