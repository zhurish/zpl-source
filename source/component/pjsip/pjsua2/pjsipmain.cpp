
#include <iostream>
#include "pjsipmain.hpp"

#include "pjsipaccount.hpp"
#include "pjsipcall.hpp"
#include "pjsipbuddy.hpp"
#include "pjsipapp.hpp"
#include "pjsiptimer.hpp"
#include "pjsipobserver.hpp"
#include "pjsipsample.hpp"
#include "pjsipjsonconfig.hpp"


//LD_LIBRARY_PATH
using namespace pj;
using namespace std;



pjsipMain::pjsipMain()
{

}

pjsipMain::~pjsipMain()
{

}

int pjsipMain::pjsip_app_init(string &pjMainCfgFile)
{
    if (pjsip_sample == nullptr)
    {
        pjsip_sample = new pjsipSample();
    }
    if (pjsip_sample != nullptr)
    {
        if (pjsip_sample->pjsipSampleCfg == nullptr)
        {
            std::cout << "pjsipMain pjsip_app_init pjsipJsonConfig" <<  std::endl;
            pjsip_sample->pjsipSampleCfg = new pjsipJsonConfig(pjMainCfgFile);
            if (pjsip_sample->pjsipSampleCfg != nullptr)
            {
                pjsip_sample->pjsipSampleCfg->pjsipJsonConfigLoad();
            }
            else
            {
                return -1;
            }
            std::cout << "pjsipMain pjsip_app_init pjsipJsonConfigLoad" <<  std::endl;
            //void libRegisterThread(const string &name) PJSUA2_THROW(Error);
            //bool libIsThreadRegistered();
        }
    }
    return -1;
}

int pjsipMain::pjsip_app_start()
{
    if (pjsip_sample != nullptr && pjsip_sample->pjsipSampleCfg != nullptr)
    {
        if (pjsip_sample->pjsipSampleInit(pjsip_sample->pjsipSampleCfg->pjsipAppAgentName, pjsipAppDir) != 0)
        {
            pjsip_sample->pjsipSampleDestroy();
            delete pjsip_sample;
            pjsip_sample = nullptr;
            return -1;
        }
        if (pjsip_sample->pjsipSampleCfgEmpty())
        {
            pjsip_sample->pjsipSampleCfgRealm(pjsip_sample->pjsipSampleCfg->pjsip_realm);
            pjsip_sample->pjsipSampleCfgAccountID(pjsip_sample->pjsipSampleCfg->pjsip_acc_id);
            pjsip_sample->pjsipSampleCfgServer(pjsip_sample->pjsipSampleCfg->pjsip_address);
            pjsip_sample->pjsipSampleCfgUsername(pjsip_sample->pjsipSampleCfg->pjsip_username, 
                                                pjsip_sample->pjsipSampleCfg->pjsip_password);
            pjsip_sample->pjsipSampleCfgCommit();
        }
        pjsip_sample->pjsipSampleTimerStart(5000, 1000);
        return 0;
    }
    return -1;
}

int pjsipMain::pjsip_app_task(void *p)
{
    while (pjsip_sample->app_running)
    {
        if (pjsip_sample->pjsipTimerPtr != nullptr)
        {
            pjsip_sample->pjsipTimerPtr->pjsipTimerHandle();
            pjsip_sample->pjsipSampleTimerHandle();
        }
        pj_thread_sleep(1000);
    }
    return 0;
}
/*
static pjsipSample *pjsip_sample = nullptr;

#ifdef __cplusplus
extern "C"
{
#endif

int pjsip_app_init()
{
    string pjsipAppDir = "./json-pjsip";
    if (pjsip_sample == nullptr)
    {
        pjsip_sample = new pjsipSample();
    }
    if (pjsip_sample != nullptr)
            return 0;
    return -1;
}

int pjsip_app_start()
{
    string pjsipAppDir;

    if (pjsip_sample != nullptr && pjsip_sample->pjsipSampleCfg != nullptr)
    {
        if (pjsip_sample->pjsipSampleInit(pjsip_sample->pjsipSampleCfg->pjsipAppAgentName, pjsipAppDir) != 0)
        {
            pjsip_sample->pjsipSampleDestroy();
            pjsip_sample = nullptr;
            return -1;
        }
        if (pjsip_sample->pjsipSampleCfgEmpty())
        {
            pjsip_sample->pjsipSampleCfgRealm(pjsip_sample->pjsipSampleCfg->pjsip_realm);
            pjsip_sample->pjsipSampleCfgAccountID(pjsip_sample->pjsipSampleCfg->pjsip_acc_id);
            pjsip_sample->pjsipSampleCfgServer(pjsip_sample->pjsipSampleCfg->pjsip_address);
            pjsip_sample->pjsipSampleCfgUsername(pjsip_sample->pjsipSampleCfg->pjsip_username, 
                                                pjsip_sample->pjsipSampleCfg->pjsip_password);
            pjsip_sample->pjsipSampleCfgCommit();
        }
        pjsip_sample->pjsipSampleTimerStart(5000, 1000);
        return 0;
    }
    return -1;
}

int pjsip_app_task(void *p)
{
    while (pjsip_sample->app_running)
    {
        if (pjsip_sample->pjsipTimerPtr != nullptr)
        {
            pjsip_sample->pjsipTimerPtr->pjsipTimerHandle();
            pjsip_sample->pjsipSampleTimerHandle();
        }
        pj_thread_sleep(1000);
    }
}


int aamain()
{
    bool app_running = true;
    string appname = ("pjsipQt");
    string appdir = ("./");
    pjsipSample *pjsip_sample = nullptr;

    pjsipJsonConfig *aaa = new pjsipJsonConfig(appname);
    if (pjsip_sample == nullptr)
    {
        pjsip_sample = new pjsipSample();
        std::cout << "pjsipMain" << std::endl;
    }
    if (pjsip_sample != nullptr)
    {
        std::cout << "pjsipMainStart pjsipSampleInit" << std::endl;
        if (pjsip_sample->pjsipSampleInit(appname, appdir) != 0)
        {
            std::cout << "pjsipMainStart pjsipSampleInit error" << std::endl;
            pjsip_sample->pjsipSampleDestroy();
            pjsip_sample = nullptr;
            return -1;
        }
        std::cout << "pjsipSampleInit" << std::endl;

        std::cout << "pjsipMainConfigDefault" << std::endl;
        if (pjsip_sample->pjsipSampleCfgEmpty())
        {
            string acc_id = "1007";
            string registrar = "192.168.2.253";
            //String proxy 	 = "";
            string username = "1007";
            string password = "1007";
            string realm = "*";
            //pjsip_sample.pjsipSampleCfgRealm("AIO100");
            //pjsip_sample.pjsipSampleCfgRealm("192.168.2.253");
            pjsip_sample->pjsipSampleCfgRealm(realm);
            pjsip_sample->pjsipSampleCfgAccountID(acc_id);
            pjsip_sample->pjsipSampleCfgServer(registrar);
            pjsip_sample->pjsipSampleCfgUsername(username, password);
            //pjsip_sample.pjsipSampleCfgProxy(proxy);
            pjsip_sample->pjsipSampleCfgCommit();
        }
        pjsip_sample->pjsipSampleTimerStart(5000, 1000);

        while (app_running)
        {
            if (pjsip_sample->pjsipTimerPtr != nullptr)
            {
                pjsip_sample->pjsipTimerPtr->pjsipTimerHandle();
                pjsip_sample->pjsipSampleTimerHandle();
            }

            pj_thread_sleep(1000);
        }
    }
    return 0;
}

#ifdef __cplusplus
}
#endif
*/