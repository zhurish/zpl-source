#ifndef PJSIPSAMPLEOBSERVER_HPP
#define PJSIPSAMPLEOBSERVER_HPP

#include <pjsua2.hpp>
#include <iostream>

using namespace pj;
using namespace std;

class pjsipCall;
class pjsipAccount;

/*
 * 在 pjsipSample  使用, 通知上层
*/
class pjsipSampleObserver
{
public:
    pjsipSampleObserver()
    {
    }
    virtual void pjsipSampleNotifyRegisterState(pjsipAccount& account, int code, string &reason, long ex);
    virtual void pjsipSampleNotifyIncomingCall(pjsipCall &call, bool isvideo);
    virtual void pjsipSampleNotifyCallState(pjsipCall &call, bool isvideo, int state, string& msg);
    virtual void pjsipSampleNotifyCallMediaState(pjsipCall &call, bool isvideo);
    virtual void pjsipSampleNotifyDtmfInfo(pjsipCall &call, int type, string &msg);
    //void pjsipObserverNotifyBuddyState(pjsipBuddy buddy, string msg){}
    //void pjsipObserverNotifyChangeNetwork(int state, string msg){}
    virtual void pjsipSampleNotifyTimer(int ev, int timertype, int count);
};

#endif // PJSIPSAMPLEOBSERVER_HPP
