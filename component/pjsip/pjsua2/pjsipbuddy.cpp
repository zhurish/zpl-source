#include "pjsipbuddy.hpp"
#include "pjsiplogwriter.hpp"
using namespace pj;
using namespace std;
/*
 * class pjsipBuddy
 */
pjsipBuddy::pjsipBuddy(BuddyConfig *config)
    : Buddy()
{
    PJSIP_ENTER_DEBUG();
    buddy_cfg = config;
    PJSIP_LEAVE_DEBUG();
}

string pjsipBuddy::getStatusText()
{
    PJSIP_ENTER_DEBUG();
    BuddyInfo bi;

    try
    {
        bi = getInfo();
    }
    catch (const string msg)
    {
        utilLogWrite(0, "pjsipBuddy", "exception:" + msg);
        return "?";
    }

    string status = "";
    if (bi.subState == PJSIP_EVSUB_STATE_ACTIVE)
    {
        if (bi.presStatus.status == PJSUA_BUDDY_STATUS_ONLINE)
        {
            status = bi.presStatus.statusText;
            if (/*status == nullptr ||*/ status.length() == 0)
            {
                status = "Online";
            }
        }
        else if (bi.presStatus.status == PJSUA_BUDDY_STATUS_OFFLINE)
        {
            status = "Offline";
        }
        else
        {
            status = "Unknown";
        }
    }
    PJSIP_LEAVE_DEBUG();
    return status;
}

void pjsipBuddy::onBuddyState()
{
}

void pjsipBuddy::onBuddyEvSubState(OnBuddyEvSubStateParam &prm)
{
}
