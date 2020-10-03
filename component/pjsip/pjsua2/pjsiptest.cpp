
#include <iostream>
#include "pjsipmain.hpp"

/*
#include "pjsipaccount.hpp"
#include "pjsipcall.hpp"
#include "pjsipbuddy.hpp"
#include "pjsipapp.hpp"
#include "pjsiptimer.hpp"
#include "pjsipobserver.hpp"
#include "pjsipsample.hpp"
#include "pjsipjsonconfig.hpp"
#define TIMER_TIMEOUT (5 * 1000)
*/

//LD_LIBRARY_PATH
using namespace pj;
using namespace std;

int main()
{
    /*
    bool app_running = true;
    string appname=("pjsipQt");
    string appdir=("./");
    pjsipSample *pjsip_sample = nullptr;
   
    pjsipJsonConfig *aaa = new pjsipJsonConfig(appname);

    if (pjsip_sample == nullptr) {
        pjsip_sample = new pjsipSample();
        std::cout << "pjsipMain" <<  std::endl;
    }
    if (pjsip_sample != nullptr) {
        std::cout << "pjsipMainStart pjsipSampleInit" <<  std::endl;
        if(pjsip_sample->pjsipSampleInit( appname, appdir) != 0)
        {
            std::cout << "pjsipMainStart pjsipSampleInit error" <<  std::endl;
            pjsip_sample->pjsipSampleDestroy();
            pjsip_sample = nullptr;
            return -1;
        }
        std::cout << "pjsipSampleInit" <<  std::endl;

        std::cout << "pjsipMainConfigDefault" <<  std::endl;
        if (pjsip_sample->pjsipSampleCfgEmpty()) {
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
        if(pjsip_sample->pjsipTimerPtr != nullptr)
        {
            pjsip_sample->pjsipTimerPtr->pjsipTimerHandle();
            pjsip_sample->pjsipSampleTimerHandle();
        }

		pj_thread_sleep(1000);
	}
    }
    */
    pjsipMain *pjsipmain = new pjsipMain();
    if(pjsipmain != nullptr)
    {
        string pjMainCfgFile = "./pjmaincfg-json";
        pjsipmain->pjsip_app_init(pjMainCfgFile);
        pjsipmain->pjsip_app_start();
        pjsipmain->pjsip_app_task(nullptr);
    }
    return 0;
}
