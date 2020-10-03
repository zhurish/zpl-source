#ifndef PJSIPMAIN_HPP
#define PJSIPMAIN_HPP


#include "pjsipaccount.hpp"
#include "pjsipcall.hpp"
#include "pjsipbuddy.hpp"
#include "pjsipapp.hpp"
#include "pjsiptimer.hpp"
#include "pjsipobserver.hpp"
#include "pjsipsample.hpp"


using namespace pj;
using namespace std;


#define PJSIP_MAIN_CFG_FILE  "pjmaincfg-json"

class pjsipMain
{
public:
    pjsipMain();
    ~pjsipMain();
    
public:
    pjsipSample *pjsip_sample = nullptr;
    string pjsipAppDir;

    int pjsip_app_init(string &pjMainCfgFile);
    int pjsip_app_start();
    int pjsip_app_task(void *p);
};
/*
#ifdef __cplusplus
extern "C"
{
#endif

int pjsip_app_init();
int pjsip_app_start();
int pjsip_app_task(void *p);

#ifdef __cplusplus
}
#endif
*/

#endif // PJSIPMAIN_HPP
