#include "pjsipaccount.hpp"
#include "pjsipcall.hpp"
#include "pjsipbuddy.hpp"
#include "pjsipapp.hpp"
#include "pjsiptimer.hpp"
#include "pjsipobserver.hpp"
#include "pjsipsampleobserver.hpp"

using namespace pj;
using namespace std;


void pjsipSampleObserver::pjsipSampleNotifyRegisterState(pjsipAccount& account, int code, string &reason, long ex)
{
    //std::cout << "pjsipSampleObserver::pjsipAppNotifyRegisterState" << std::endl;
    if(account.account_cfg == nullptr)
    {
        if(!account.account_cfg->idUri.empty())
        {
            string idUri = account.account_cfg->idUri;
            string local_urlid = idUri.substr(idUri.find(':') + 1, idUri.find('@'));
            //std::cout << "pjsipSampleObserver::pjsipAppNotifyRegisterState:" << local_urlid << std::endl;
        }
    }
}

void pjsipSampleObserver::pjsipSampleNotifyIncomingCall(pjsipCall &call, bool isvideo)
{
    //std::cout << "pjsipSampleObserver::pjsipAppNotifyIncomingCall" << std::endl;
}

void pjsipSampleObserver::pjsipSampleNotifyCallState(pjsipCall &call, bool isvideo, int state, string& msg)
{
    //std::cout << "pjsipSampleObserver::pjsipAppNotifyCallState" << std::endl;
}

void pjsipSampleObserver::pjsipSampleNotifyCallMediaState(pjsipCall &call, bool isvideo)
{
    //std::cout << "pjsipSampleObserver::pjsipAppNotifyCallMediaState" << std::endl;
}

void pjsipSampleObserver::pjsipSampleNotifyDtmfInfo(pjsipCall &call, int type, string &msg)
{
    //std::cout << "pjsipSampleObserver::pjsipAppNotifyDtmfInfo" << std::endl;
}

void pjsipSampleObserver::pjsipSampleNotifyTimer(int ev, int timertype, int count)
{

}