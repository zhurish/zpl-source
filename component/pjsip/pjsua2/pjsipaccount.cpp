#include <pjsua2/endpoint.hpp>
#include "pjsipaccount.hpp"
#include "pjsipcall.hpp"
#include "pjsipbuddy.hpp"
#include "pjsipapp.hpp"
#include "pjsiplogwriter.hpp"
#include "pjsipobserver.hpp"

using namespace pj;
using namespace std;
/*
 * class pjsipAccount
 */
pjsipAccount::pjsipAccount(pjsipObserver *observer, AccountConfig& config)
    : Account()
{
    PJSIP_ENTER_DEBUG();
    account_cfg = &config;
    pjObserver = observer;
    PJSIP_LEAVE_DEBUG();
}

pjsipAccount::~pjsipAccount()
{
    /*
    for (std::vector<Call *>::iterator it = call_list.begin();
             it != call_list.end(); )
        {
        delete (*it);
        it = call_list.erase(it);
        }
    */
}

void pjsipAccount::onIncomingCall(OnIncomingCallParam &prm)
{
    PJSIP_ENTER_DEBUG();
    
    pjsipCall *call = new pjsipCall(*this, prm.callId);
    //pjsipCall *call = new pjsipCall(pjObserver, *this, prm.callId);
    if (call && pjObserver)
    {
        call->pjsipCallObserver(pjObserver);
        pjObserver->pjsipAppNotifyIncomingCall(*call);
    }
    PJSIP_LEAVE_DEBUG();
}

void pjsipAccount::onRegStarted(OnRegStartedParam &prm)
{
}

void pjsipAccount::onRegState(OnRegStateParam &prm)
{
    PJSIP_ENTER_DEBUG();
    if (pjObserver != nullptr)
        pjObserver->pjsipAppNotifyRegisterState(*this, prm.code, prm.reason,
                                                prm.expiration);
    PJSIP_LEAVE_DEBUG();                                            
}

void pjsipAccount::onIncomingSubscribe(OnIncomingSubscribeParam &prm)
{
}

void pjsipAccount::onInstantMessage(OnInstantMessageParam &prm)
{
    PJSIP_ENTER_DEBUG();
    utilLogWrite(4, "pjsipAccount", "======== Incoming pager ======== ");
    utilLogWrite(4, "pjsipAccount", "From     : " + prm.fromUri);
    utilLogWrite(4, "pjsipAccount", "To       : " + prm.toUri);
    utilLogWrite(4, "pjsipAccount", "Contact  : " + prm.contactUri);
    utilLogWrite(4, "pjsipAccount", "Mimetype : " + prm.contentType);
    utilLogWrite(4, "pjsipAccount", "Body     : " + prm.msgBody);
    PJSIP_LEAVE_DEBUG();
}

void pjsipAccount::onInstantMessageStatus(OnInstantMessageStatusParam &prm)
{
}

void pjsipAccount::onTypingIndication(OnTypingIndicationParam &prm)
{
}

void pjsipAccount::onMwiInfo(OnMwiInfoParam &prm)
{
}

/*
void pjsipAccount::pjsipAccountRemoveCall(Call *call)
{
    for (std::vector<Call *>::iterator it = call_list.begin();
         it != call_list.end(); ++it)
    {
        if (*it == call) {
            call_list.erase(it);
            break;
        }
    }
}
*/
int pjsipAccount::pjsipAccountBuddyAdd(BuddyConfig& bud_cfg, pjsipBuddy **pjsipbuddy)
{
    PJSIP_ENTER_DEBUG();
    pjsipBuddy *bud = new pjsipBuddy(&bud_cfg);
    try
    {
        bud->create(*this, bud_cfg);
    }
    catch (const string msg)
    {
        utilLogWrite(0, "pjsipAccount", "exception:" + msg);
        delete (bud);
        bud = nullptr;
    }

    if (bud != nullptr)
    {
        buddyList.push_back(bud);
        if (bud_cfg.subscribe)
            try
            {
                bud->subscribePresence(true);
            }
            catch (const string msg)
            {
                utilLogWrite(0, "pjsipAccount", "exception:" + msg);
            }
    }
    if(pjsipbuddy != nullptr)
        *pjsipbuddy = bud;
    PJSIP_LEAVE_DEBUG();    
    return 0;
}

void pjsipAccount::pjsipAccountBuddyDelete(pjsipBuddy& buddy)
{
    PJSIP_ENTER_DEBUG();
    for (std::vector<pjsipBuddy *>::iterator it = buddyList.begin();
         it != buddyList.end(); ++it)
    {
        if (*it == &buddy)
        {
            buddyList.erase(it);
            break;
        }
    }
    PJSIP_LEAVE_DEBUG();
}

void pjsipAccount::pjsipAccountBuddyDelete(int index)
{
    PJSIP_ENTER_DEBUG();
    pjsipBuddy *buddy = buddyList.at(index);
    //std::vector<pjsipBuddy *>::iterator buddy = buddyList.at(index);
    if (buddy != nullptr)
        ;//buddyList.erase(buddy);
    PJSIP_LEAVE_DEBUG();    
}

pjsipAccountConfig::pjsipAccountConfig()
{
    PJSIP_ENTER_DEBUG();
    account_config_cfg = new AccountConfig();
    PJSIP_LEAVE_DEBUG();
}

pjsipAccountConfig::~pjsipAccountConfig()
{
    PJSIP_ENTER_DEBUG();
    buddyCfgs.clear();
    if (account_config_cfg != nullptr)
        delete (account_config_cfg);
    PJSIP_LEAVE_DEBUG();    
}

void pjsipAccountConfig::readObject(ContainerNode node)
{
    PJSIP_ENTER_DEBUG();
    if (account_config_cfg != nullptr)
    {
        try
        {
            ContainerNode acc_node = node.readContainer("Account");
            account_config_cfg->readObject(acc_node);
            ContainerNode buddies_node = acc_node.readArray("buddies");
            buddyCfgs.clear();
            while (buddies_node.hasUnread())
            {
                BuddyConfig *bud_cfg = new BuddyConfig();
                if (bud_cfg)
                {
                    bud_cfg->readObject(buddies_node);
                    buddyCfgs.push_back(bud_cfg);
                }
            }
        }
        catch (const string msg)
        {
            utilLogWrite(0, "pjsipAccountConfig", "exception:" + msg);
        }
    }
    PJSIP_LEAVE_DEBUG();
}

void pjsipAccountConfig::writeObject(ContainerNode node)
{
    PJSIP_ENTER_DEBUG();
    if (account_config_cfg != nullptr)
    {
        try
        {
            ContainerNode acc_node = node.writeNewContainer("Account");
            account_config_cfg->writeObject(acc_node);
            //if(buddyCfgs.size() > 0) 
            {
                ContainerNode buddies_node = acc_node.writeNewArray("buddies");
            
                for (int j = 0; j < buddyCfgs.size(); j++)
                {
                    buddyCfgs.at(j)->writeObject(buddies_node);
                }
            }
        }
        catch (const string msg)
        {
           utilLogWrite(0, "pjsipAccountConfig", "exception:" + msg);
        }
    }
    PJSIP_LEAVE_DEBUG();
}
