#include "pjsipaccount.hpp"
#include "pjsipcall.hpp"
#include "pjsipbuddy.hpp"
#include "pjsipapp.hpp"
#include "pjsiptimer.hpp"
#include "pjsipobserver.hpp"


using namespace pj;
using namespace std;

pjsipObserver::pjsipObserver()
{
}

void pjsipObserver::pjsipAppNotifyRegisterState(pjsipAccount& account, int code, string &reason, long ex)
{
    //std::cout << "pjsipObserver::pjsipAppNotifyRegisterState" << std::endl;
    if(account.account_cfg == nullptr)
    {
        if(!account.account_cfg->idUri.empty())
        {
            string idUri = account.account_cfg->idUri;
            string local_urlid = idUri.substr(idUri.find(':') + 1, idUri.find('@'));
            //std::cout << "pjsipObserver::pjsipAppNotifyRegisterState:" << local_urlid << std::endl;
        }
    }
}

void pjsipObserver::pjsipAppNotifyIncomingCall(pjsipCall &call)
{
    //std::cout << "pjsipObserver::pjsipAppNotifyIncomingCall" << std::endl;
}

void pjsipObserver::pjsipAppNotifyCallState(pjsipCall &call)
{
    //std::cout << "pjsipObserver::pjsipAppNotifyCallState" << std::endl;
}

void pjsipObserver::pjsipAppNotifyCallMediaState(pjsipCall &call)
{
    //std::cout << "pjsipObserver::pjsipAppNotifyCallMediaState" << std::endl;
}

void pjsipObserver::pjsipAppNotifyDtmfInfo(pjsipCall &call, int type, string &msg)
{
    //std::cout << "pjsipObserver::pjsipAppNotifyDtmfInfo" << std::endl;
}
