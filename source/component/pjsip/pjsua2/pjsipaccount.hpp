#ifndef PJSIPACCOUNT_HPP
#define PJSIPACCOUNT_HPP

#include <pjsua2.hpp>
#include <iostream>
#include "pjsipbuddy.hpp"
#include "pjsipobserver.hpp"

using namespace pj;
using namespace std;

class pjsipAccount : public Account
{
public:
    std::vector<pjsipBuddy *> buddyList; // = new ArrayList<pjsipBuddy>();
    AccountConfig *account_cfg = nullptr;
    //std::vector<Call *> call_list;

public:
    pjsipAccount(pjsipObserver *observer, AccountConfig &cfg);
    ~pjsipAccount();

    void onIncomingCall(OnIncomingCallParam &prm);
    void onRegStarted(OnRegStartedParam &prm);
    void onRegState(OnRegStateParam &prm);
    void onIncomingSubscribe(OnIncomingSubscribeParam &prm);
    void onInstantMessage(OnInstantMessageParam &prm);
    void onInstantMessageStatus(OnInstantMessageStatusParam &prm);
    void onTypingIndication(OnTypingIndicationParam &prm);
    void onMwiInfo(OnMwiInfoParam &prm);

    //void pjsipAccountRemoveCall(Call *call);
    int pjsipAccountBuddyAdd(BuddyConfig &bud_cfg, pjsipBuddy **pjsipbuddy);
    void pjsipAccountBuddyDelete(pjsipBuddy &buddy);
    void pjsipAccountBuddyDelete(int index);

private:
    pjsipObserver *pjObserver = nullptr;
};

class pjsipAccountConfig
{
public:
    pjsipAccountConfig();
    ~pjsipAccountConfig();

    AccountConfig *account_config_cfg = nullptr;
    std::vector<BuddyConfig *> buddyCfgs;

    void readObject(ContainerNode node);

    void writeObject(ContainerNode node);
};

#endif // PJSIPACCOUNT_HPP
