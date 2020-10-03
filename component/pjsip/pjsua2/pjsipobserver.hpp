#ifndef PJSIPOBSERVER_HPP
#define PJSIPOBSERVER_HPP

#include <pjsua2.hpp>
#include <iostream>

using namespace pj;
using namespace std;
//#include "pjsipcall.hpp"
//#include "pjsipbuddy.hpp"
class pjsipCall;
class pjsipAccount;
/*
 * 在 pjsipApp pjsipCall pjsipAccount 使用
*/

class pjsipObserver
{
public:
    pjsipObserver();
    virtual void pjsipAppNotifyRegisterState(pjsipAccount& account, int code, string &reason, long ex);
    virtual void pjsipAppNotifyIncomingCall(pjsipCall &call);
    virtual void pjsipAppNotifyCallState(pjsipCall &call);
    virtual void pjsipAppNotifyCallMediaState(pjsipCall &call);
    virtual void pjsipAppNotifyDtmfInfo(pjsipCall &call, int type, string &msg);
    //void pjsipObserverNotifyBuddyState(pjsipBuddy buddy, string msg);
    //void pjsipObserverNotifyChangeNetwork(int state, string msg);
    //void pjsipObserverNotifyTimer(int ev);
};



#endif // PJSIPOBSERVER_HPP
