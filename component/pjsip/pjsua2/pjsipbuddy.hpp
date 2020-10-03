#ifndef PJSIPBUDDY_HPP
#define PJSIPBUDDY_HPP

#include <pjsua2.hpp>
#include <iostream>

using namespace pj;
using namespace std;

class pjsipBuddy : public Buddy
{
public:
    pjsipBuddy(BuddyConfig *config);

    void onBuddyState();
    void onBuddyEvSubState(OnBuddyEvSubStateParam &prm);

    string getStatusText();

    BuddyConfig *buddy_cfg = nullptr;
};

#endif // PJSIPBUDDY_HPP
