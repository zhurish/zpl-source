#include <iostream>
#include "pjsipsample.hpp"
#include "pjsiplogwriter.hpp"
#include "pjsipsampleobserver.hpp"

#define THIS_FILE "pjsipsample.c"

using namespace pj;
using namespace std;

pjsipSample::pjsipSample()
{
    PJSIP_ENTER_DEBUG();

    sipRegState = PJSIP_STATE.PJSIP_STATE_ERROR;
    callState = PJSIP_STATE.PJSIP_STATE_ERROR;
    //pjsip_observer = new pjsipObserver();
    PJSIP_LEAVE_DEBUG();
}
pjsipSample::~pjsipSample()
{
}
bool pjsipSample::pjsipSampleCheck(string func)
{
    if (pjsip_app == nullptr)
    {
        utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_ERROR, "pjsipSample", func + ":app is nullptr ");
        return false;
    }
    if (pjsip_app->ep == nullptr)
    {
        utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_ERROR, "pjsipSample", func + ":pjsip_app->ep is nullptr ");
        return false;
    }
    if (accountCfg == nullptr)
    {
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_ERROR, "pjsipSample", func + ":accountCfg is nullptr ");
        return false;
    }
    if (accountNatCfg == nullptr)
    {
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_ERROR, "pjsipSample", func + ":accountNatCfg is nullptr ");
        return false;
    }
    if (accountVideoCfg == nullptr)
    {
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_ERROR, "pjsipSample", func + ":accountVideoCfg is nullptr ");
        return false;
    }
    if (accountSipCfg == nullptr)
    {
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_ERROR, "pjsipSample", func + ":accountSipCfg is nullptr ");
        return false;
    }
    if (accountRegCfg == nullptr)
    {
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_ERROR, "pjsipSample", func + ":accountRegCfg is nullptr ");
        return false;
    }
    if (pjsipSampleCfg == nullptr)
    {
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_ERROR, "pjsipSample", func + ":pjsipSampleCfg is nullptr ");
        return false;
    }
    return true;
}

int pjsipSample::pjsipSampleRegInit(int regtimeout, int RetryInterval)
{
    PJSIP_ENTER_DEBUG();
    if (!pjsipSampleCheck("pjsipSampleRegInit "))
    {
        return -1;
    }
    accountRegCfg->retryIntervalSec = (RetryInterval);
    accountRegCfg->timeoutSec = (regtimeout);
    try
    {
        AccountInfo accountInfo = account->getInfo();
        //if(accountInfo != nullptr)
        accountInfo.regExpiresSec = (regtimeout);
    }
    catch (string msg)
    {
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
        return PJSIP_STATE.PJSIP_STATE_ERROR;
    }
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_OK;
}

bool pjsipSample::pjsipSampleCfgEmpty()
{
    return true;
    /*        if (pjsipSampleCfg->sip_address == nullptr || pjsip_app->accList.size() == 0)
          return true;
      return false;*/
}

/*
配置用户名密码信息
 */
int pjsipSample::pjsipSampleCfgUsername(string &username, string &password)
{
    PJSIP_ENTER_DEBUG();
    if (!pjsipSampleCheck("pjsipSampleCfgUsername "))
    {
        return -1;
    }
    AuthCredInfoVector creds = accountSipCfg->authCreds;
    creds.clear();
    if (!username.empty() && username.length() != 0)
    {
        AuthCredInfo *auth = new AuthCredInfo("Digest", (!pjsipSampleCfg->pjsip_realm.empty()) ? pjsipSampleCfg->pjsip_realm : "*", username, 0,
                                              password);
        if (auth)
        {
            pjsipSampleCfg->pjsip_password = password;
            pjsipSampleCfg->pjsip_username = username;
            creds.push_back(*auth);
            return PJSIP_STATE.PJSIP_STATE_OK;
        }
    }
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_INVALUE;
}

/*
配置代理信息
 */
int pjsipSample::pjsipSampleCfgProxy(string &proxy)
{
    PJSIP_ENTER_DEBUG();
    if (!pjsipSampleCheck("pjsipSampleCfgProxy "))
    {
        return -1;
    }
    StringVector proxies = accountSipCfg->proxies;
    proxies.clear();
    if (!proxy.empty() && proxy.length() != 0)
    {
        proxies.push_back(proxy);
        return PJSIP_STATE.PJSIP_STATE_OK;
    }
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_INVALUE;
}

/*
配置SIP服务器信息
 */
int pjsipSample::pjsipSampleCfgServer(string &server)
{
    PJSIP_ENTER_DEBUG();
    if (!pjsipSampleCheck("pjsipSampleCfgServer "))
    {
        return -1;
    }
    if (!server.empty() && server.length() != 0 && accountRegCfg != nullptr)
    {
        if(pjsipSampleCfg->pjsip_proto == PJSIP_TRANSPORT_TCP)
            accountRegCfg->registrarUri = "sip:" + server + ";transport=tcp";
        else if(pjsipSampleCfg->pjsip_proto == PJSIP_TRANSPORT_TLS)
            accountRegCfg->registrarUri = "sip:" + server + ";transport=tls";
        else
            accountRegCfg->registrarUri = "sip:" + server;


        pjsipSampleCfg->pjsip_address = server;
        if (server.find(':') >= 0)
        {
            string port = server.substr(server.find(":") + 1);
            if (port.length() > 0)
            {
                char *portchar = (char *)port.data();
                if (portchar)
                    pjsipSampleCfg->pjsip_port = atoi(portchar);
            }
        }
        else
            pjsipSampleCfg->pjsip_port = 5060;
        return PJSIP_STATE.PJSIP_STATE_OK;
    }
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_INVALUE;
}

/*
配置本地用户号码信息信息
 */
int pjsipSample::pjsipSampleCfgAccountID(string &acc_id)
{
    PJSIP_ENTER_DEBUG();
    if (!pjsipSampleCheck("pjsipSampleCfgAccountID "))
    {
        return -1;
    }
    if (!acc_id.empty() && acc_id.length() != 0)
    {
        if (pjsipSampleCfg->pjsip_realm.empty() || pjsipSampleCfg->pjsip_realm == "*")
            accountCfg->idUri = ("sip:" + acc_id);
        else if (!pjsipSampleCfg->pjsip_address.empty())
            accountCfg->idUri = ("sip:" + acc_id + "@" + pjsipSampleCfg->pjsip_address);
        pjsipSampleCfg->pjsip_acc_id = acc_id;
    }
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_OK;
}

/*
配置本地用户号码信息信息
*/
int pjsipSample::pjsipSampleCfgRealm(string &realm)
{
    PJSIP_ENTER_DEBUG();
    if (!realm.empty() && realm.length() != 0)
    {
        pjsipSampleCfg->pjsip_realm = realm;
    }
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_OK;
}

int pjsipSample::pjsipSampleCfgProto(int proto)
{
    if(pjsipSampleCfg->pjsip_address.length() != 0 && !pjsipSampleCfg->pjsip_address.empty() && pjsipSampleCfg->pjsip_proto != proto) {
        if((pjsipSampleCfg->pjsip_proto == PJSIP_TRANSPORT_UDP || pjsipSampleCfg->pjsip_proto == 0) &&
                    proto != 0 && proto != PJSIP_TRANSPORT_UDP )
            {
                accountRegCfg->registrarUri = ("sip:" + pjsipSampleCfg->pjsip_address);
            }
            else
            {
                string server = pjsipSampleCfg->pjsip_address.substr(0, pjsipSampleCfg->pjsip_address.find(";"));

                if (pjsipSampleCfg->pjsip_proto == PJSIP_TRANSPORT_TCP)
                    accountRegCfg->registrarUri = ("sip:" + server + ";transport=tcp");
                else if (pjsipSampleCfg->pjsip_proto == PJSIP_TRANSPORT_TLS)
                    accountRegCfg->registrarUri = ("sip:" + server + ";transport=tls");
                else
                    accountRegCfg->registrarUri = ("sip:" + server);
                pjsipSampleCfg->pjsip_address = accountRegCfg->registrarUri.substr(4);
            }
        }
    pjsipSampleCfg->pjsip_proto = proto;
    return PJSIP_STATE.PJSIP_STATE_OK;
}
/*
提交配置信息
 */
int pjsipSample::pjsipSampleCfgCommit()
{
    PJSIP_ENTER_DEBUG();
    if (!pjsipSampleCheck("pjsipSampleCfgCommit "))
    {
        return -1;
    }
    if (!pjsipSampleCfg->pjsip_address.empty() && !pjsipSampleCfg->pjsip_acc_id.empty())
    {
        accountCfg->idUri = ("sip:" + pjsipSampleCfg->pjsip_acc_id + "@" + pjsipSampleCfg->pjsip_address);
    }
    accountNatCfg->iceEnabled = (false);
    accountVideoCfg->autoTransmitOutgoing = (true);
    accountVideoCfg->autoShowIncoming = (true);
    //accountCfg.getVideoConfig().setDefaultCaptureDevice(0);
    try
    {
        account->modify(*accountCfg);
    }
    catch (string msg)
    {
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
        return PJSIP_STATE.PJSIP_STATE_ERROR;
    }
    if (!pjsipAppCfgFile.empty())
        pjsip_app->pjsipAppSaveConfig(pjsipAppCfgFile);

    if (pjsipSampleCfg != nullptr)
        pjsipSampleCfg->pjsipJsonConfigSave();
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_OK;
}

int pjsipSample::pjsipSampleCfgVideoEnable(bool enable) {
        pjsipSampleCfg->pjsip_video_enable = enable;
        return PJSIP_STATE.PJSIP_STATE_OK;
    }
bool pjsipSample::pjsipSampleCfgIsVideoEnable() {
        return pjsipSampleCfg->pjsip_video_enable;
    }
/*
初始化
 */
int pjsipSample::pjsipSampleInitDefault()
{
    PJSIP_ENTER_DEBUG();
    accountNatCfg = &accountCfg->natConfig;
    if (accountNatCfg != nullptr)
        accountNatCfg->iceEnabled = (false);
    else
    {
        accountNatCfg = new AccountNatConfig();
        if (accountNatCfg != nullptr)
        {
            accountNatCfg->iceEnabled = (false);
            accountCfg->natConfig = (*accountNatCfg);
        }
    }
    accountVideoCfg = &accountCfg->videoConfig;
    if (accountVideoCfg != nullptr)
    {
        accountVideoCfg->autoTransmitOutgoing = (true);
        accountVideoCfg->autoShowIncoming = (true);
        //accountVideoCfg.setDefaultCaptureDevice(0);
        //accountVideoCfg.setWindowFlags(0);
    }
    else
    {
        accountVideoCfg = new AccountVideoConfig();
        if (accountVideoCfg != nullptr)
        {
            accountVideoCfg->autoTransmitOutgoing = (true);
            accountVideoCfg->autoShowIncoming = true;
            accountCfg->videoConfig = (*accountVideoCfg);
        }
    }
    accountRegCfg = &accountCfg->regConfig;
    if (accountRegCfg == nullptr)
    {
        accountRegCfg = new AccountRegConfig();
        if (accountRegCfg != nullptr)
        {
            accountCfg->regConfig = (*accountRegCfg);
        }
    }
    accountSipCfg = &accountCfg->sipConfig;
    if (accountSipCfg == nullptr)
    {
        accountSipCfg = new AccountSipConfig();
        if (accountSipCfg != nullptr)
        {
            accountCfg->sipConfig = (*accountSipCfg);
        }
    }
    PJSIP_LEAVE_DEBUG();
    return 0;
}

int pjsipSample::pjsipSampleInitDefaultExit()
{
    if (accountNatCfg != nullptr)
    {
        delete accountNatCfg;
    }
    if (accountVideoCfg != nullptr)
    {
        delete accountVideoCfg;
    }

    if (accountRegCfg != nullptr)
    {
        delete accountRegCfg;
    }

    if (accountSipCfg != nullptr)
    {
        delete accountSipCfg;
    }
    if (pjsipTimerPtr != nullptr)
    {
        pjsipTimerPtr->~pjsipTimer();
        delete pjsipTimerPtr;
    }
    if (pjsipSampleCfg != nullptr)
    {
        delete pjsipSampleCfg;
    }
    if (accountCfg != nullptr)
    {
        delete accountCfg;
    }
    /*
    if (pjsip_app != nullptr)
    {
        delete pjsip_app;
    }
    */
    return 0;
}

int pjsipSample::pjsipSampleInit_hw(pjsipSampleObserver *obj, string &app_name, string &app_dir)
{
    PJSIP_ENTER_DEBUG();
    if (pjsipAppCfgFile.empty())
    {
        pjsipAppCfgFile = app_dir + PJSIP_APP_CFG_FILE;
    }
    if (pjsipSampleCfg == nullptr)
    {
        string pjMainCfgFile = app_dir + PJSIP_MAIN_CFG_FILE;
        pjsipSampleCfg = new pjsipJsonConfig(pjMainCfgFile);
        if (pjsipSampleCfg != nullptr)
        {
            pjsipSampleCfg->pjsipJsonConfigLoad();
        }
        else
        {
            pjsipSampleInitDefaultExit();
            return -1;
        }
    }

    if (pjsip_app == nullptr)
        pjsip_app = new pjsipApp(this);

    if (pjsip_app == nullptr)
    {
        pjsipSampleInitDefaultExit();
        return -1;
    }

    if (pjsip_app != nullptr)
    {
        if (pjsip_app->pjsipInit(app_name, pjsipAppCfgFile, false) != 0)
        {
            pjsipSampleInitDefaultExit();
            return -1;
        }
        if (pjsip_app->pjsipTransportInit((pjsipSampleCfg->pjsip_port != 0) ? pjsipSampleCfg->pjsip_port : 5070,
                                          (pjsipSampleCfg->pjsip_proto != 0) ? pjsipSampleCfg->pjsip_proto : PJSIP_TRANSPORT_UDP) != 0)
        {
            pjsipSampleInitDefaultExit();
            return -1;
        }
        if (pjsip_app->pjsipAppStart() != 0)
        {
            pjsipSampleInitDefaultExit();
            return -1;
        }

        pjsipSampleCfg->pjsipAppAgentName = app_name;

        if (pjsip_app->accList.size() == 0)
        {
            accountCfg = new AccountConfig();
            if (accountCfg != nullptr)
            {
                accountCfg->idUri = ("sip:localhost");

                pjsipSampleInitDefault();

                pjsip_app->pjsipAccountAdd(*accountCfg, &account);
                pjsip_app->pjsipSetLogLevel(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_DEBUG);
                pjsipSampleRegInit(3600, 600);
            }
            else
            {
                pjsipSampleInitDefaultExit();
                return -1;
            }
        }
        else
        {
            account = pjsip_app->accList.at(0);
            if (account != nullptr)
            {
                accountCfg = account->account_cfg;
                if (accountCfg != nullptr)
                {
                    pjsipSampleInitDefault();
                }
                else
                {
                    pjsipSampleInitDefaultExit();
                    return -1;
                }
            }
            else
            {
                pjsipSampleInitDefaultExit();
                return -1;
            }
        }

        pjsip_app->pjsipSetLogLevel(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_DEBUG);

        if (!pjsipAppCfgFile.empty())
            pjsip_app->pjsipAppSaveConfig(pjsipAppCfgFile);
        PJSIP_LEAVE_DEBUG();
        return PJSIP_STATE.PJSIP_STATE_OK;
    }
    pjsipSampleInitDefaultExit();

    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_INVALUE;
}

int pjsipSample::pjsipSampleInit(pjsipSampleObserver *obj, string &app_name, string &app_dir)
{
    return pjsipSampleInit_hw(obj, app_name, app_dir);
}

int pjsipSample::pjsipSampleInit(pjsipSampleObserver *obj)
{
    string app_name = "pjsip";
    string app_dir = "./pj-json";
    return pjsipSampleInit_hw(obj, app_name, app_dir);
}

int pjsipSample::pjsipSampleInit(pjsipSampleObserver *obj, string &app_dir)
{
    string app_name = "pjsip";
    return pjsipSampleInit_hw(obj, app_name, app_dir);
}

int pjsipSample::pjsipSampleInit(string &app_name, string &app_dir)
{
    return pjsipSampleInit_hw(nullptr, app_name, app_dir);
    /*
    PJSIP_ENTER_DEBUG();
    if (pjsipSampleCfg == nullptr)
    {
        std::cout << " pjsipSampleInit pjsipSampleCfg" <<  std::endl;
        return -1;
    }
    if (!pjsipAppCfgFile.empty())
    {
        std::cout << " pjsipSampleInit pjsipAppCfgFile" <<  std::endl;
        pjsipAppCfgFile = "./config-json";
    }
    if (pjsip_app == nullptr)
    {
        std::cout << " pjsipSampleInit pjsipApp" <<  std::endl;
                //app = new pjsipApp();
        pjsip_app = new pjsipApp(this);
    }

    if (pjsip_app != nullptr)// 138 2780 6599
    {
        if (pjsip_app->pjsipInit(app_name, app_dir, false) != 0)
        {
            std::cout << " pjsipSampleInit pjsipInit" <<  std::endl;
            return -1;
        }

        if (pjsip_app->pjsipTransportInit((pjsipSampleCfg->sip_port != 0) ? pjsipSampleCfg->sip_port : 5070,
                                    (pjsipSampleCfg->sip_proto != 0) ? pjsipSampleCfg->sip_proto : PJSIP_TRANSPORT_UDP) != 0)
        {
            std::cout << " pjsipSampleInit pjsipTransportInit" <<  std::endl;
            return -1;
        }

        if (pjsip_app->pjsipAppStart() != 0)
        {
            std::cout << " pjsipSampleInit pjsipAppStart" <<  std::endl;
            return -1;
        }

        pjsipSampleCfg->appAgentName = app_name;

        if (pjsip_app->accList.size() == 0)
        {
            accountCfg = new AccountConfig();
            if (accountCfg != nullptr)
            {
                accountCfg->idUri = ("sip:localhost");

                pjsipSampleInitDefault();

                pjsip_app->pjsipAccountAdd(*accountCfg, &account);
                pjsip_app->pjsipSetLogLevel(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_DEBUG);
                pjsipSampleRegInit(3600, 600);
            }
            else
            {
                PJSIP_LEAVE_DEBUG();
                return -1;
            }
        }
        else
        {
            account = pjsip_app->accList.at(0);
            if (account != nullptr)
            {
                accountCfg = account->account_cfg;
                if (accountCfg != nullptr)
                {
                    pjsipSampleInitDefault();
                }
                else
                {
                    PJSIP_LEAVE_DEBUG();
                    return -1;
                }
            }
            else
            {
                PJSIP_LEAVE_DEBUG();
                return -1;
            }
        }

        pjsip_app->pjsipSetLogLevel(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_DEBUG);

        //sampleObserver = obj;
        if (!pjsipAppCfgFile.empty())
            pjsip_app->pjsipAppSaveConfig(pjsipAppCfgFile);
        PJSIP_LEAVE_DEBUG();    
        return PJSIP_STATE.PJSIP_STATE_OK;
    }
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_INVALUE;
    */
}

int pjsipSample::pjsipSampleDestroy()
{
    PJSIP_ENTER_DEBUG();
    if (pjsipTimerPtr != nullptr)
    {
        pjsipTimerPtr->pjsipTimerCancel();
        delete (pjsipTimerPtr);
        pjsipTimerPtr = nullptr;
    } /*
    sampleObserver = nullptr;
    msg_handler = nullptr;*/
    PJSIP_LEAVE_DEBUG();
    return pjsip_app->pjsipDeinit();
}

/*
保存配置信息
 */
/*
int pjsipSample::pjsipSampleSaveConfig() {
    app.pjsipSaveConfig();
    return PJSIP_STATE.PJSIP_STATE_OK;
}
*/

/* 设置呼叫信息 */
int pjsipSample::pjsipSampleSetMediaParams(CallOpParam param, bool videoCall)
{
    PJSIP_ENTER_DEBUG();
    if (!pjsipSampleCheck("pjsipSampleSetMediaParams "))
    {
        PJSIP_LEAVE_DEBUG();
        return -1;
    }
    CallSetting callSetting = param.opt;
    callSetting.audioCount = (1);
    callSetting.videoCount = (videoCall ? 1 : 0);
    //callSetting.setFlag(pjsua_call_flag.PJSUA_CALL_INCLUDE_DISABLED_MEDIA);
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_OK;
}

/*
发起呼叫
 */
int pjsipSample::pjsipSampleMakeCall(string &num, bool videoCall)
{
    PJSIP_ENTER_DEBUG();
    if (!pjsipSampleCheck("pjsipSampleMakeCall "))
    {
        PJSIP_LEAVE_DEBUG();
        return -1;
    }
    /* Only one call at anytime */
    if (currentCall != nullptr)
    {
        PJSIP_LEAVE_DEBUG();
        return PJSIP_STATE.PJSIP_STATE_NO_CALL;
    }
    if (account == nullptr)
    {
        PJSIP_LEAVE_DEBUG();
        return PJSIP_STATE.PJSIP_STATE_NO_ACC;
    }
    if (num.empty() || num.length() == 0)
    {
        PJSIP_LEAVE_DEBUG();
        return PJSIP_STATE.PJSIP_STATE_INVALUE;
    }
    //pjsipSampleCfg.sip_address.isEmpty() ||
    if (pjsipSampleCfg == nullptr || pjsipSampleCfg->pjsip_address.length() == 0)
    {
        PJSIP_LEAVE_DEBUG();
        return PJSIP_STATE.PJSIP_STATE_INVALUE;
    }
    string call_uri = "sip:" + num + "@" + pjsipSampleCfg->pjsip_address;

    pjsipCall *call = new pjsipCall(*account, -1);
    CallOpParam *prm = new CallOpParam(true);
    if (call == nullptr || prm == nullptr)
    {
        PJSIP_LEAVE_DEBUG();
        return PJSIP_STATE.PJSIP_STATE_INVALUE;
    }
    call->pjsipCallObserver(this);
    pjsipSampleSetMediaParams(*prm, videoCall);

    try
    {
        call->makeCall(call_uri, prm);
    }
    catch (string msg)
    {
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
        delete (prm);
        delete (call);
        is_incomming_call = false;
        is_makeing_call = false;
        is_video_call = false;
        PJSIP_LEAVE_DEBUG();
        return PJSIP_STATE.PJSIP_STATE_ERROR;
    }

    pjsipSampleTimerRestart((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER : 2);
    is_incomming_call = false;
    is_makeing_call = true;
    currentCall = call;
    is_video_call = videoCall;
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_OK;
}

/*
挂断呼叫
 */
int pjsipSample::pjsipSampleStopCall()
{
    PJSIP_ENTER_DEBUG();
    if (!pjsipSampleCheck("pjsipSampleStopCall "))
    {
        PJSIP_LEAVE_DEBUG();
        return -1;
    }
    int ret = pjsipSampleIsRegister();
    if (ret != 0)
    {
        PJSIP_LEAVE_DEBUG();
        return ret;
    }
    pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER : 1);
    pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER : 2);
    if (currentCall == nullptr)
    {
        PJSIP_LEAVE_DEBUG();
        return PJSIP_STATE.PJSIP_STATE_OK;
    }
    CallOpParam *prm = new CallOpParam();
    if (prm == nullptr)
    {
        PJSIP_LEAVE_DEBUG();
        return PJSIP_STATE.PJSIP_STATE_INVALUE;
    }
    prm->statusCode = (PJSIP_SC_DECLINE);

    try
    {
        currentCall->hangup(prm);
    }
    catch (string msg)
    {
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
        delete (currentCall);
        is_incomming_call = false;
        is_makeing_call = false;
        is_video_call = false;
        currentCall = nullptr;
        PJSIP_ERROR_DEBUG(msg);
        return PJSIP_STATE.PJSIP_STATE_ERROR;
    }
    is_incomming_call = false;
    is_makeing_call = false;
    currentCall = nullptr;
    is_video_call = false;
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_OK;
}

/*
接收呼叫
 */
int pjsipSample::pjsipSampleAcceptCall()
{
    PJSIP_ENTER_DEBUG();
    if (!pjsipSampleCheck("pjsipSampleAcceptCall "))
    {
        return -1;
    }
    int ret = pjsipSampleIsRegister();
    if (ret != 0)
    {

        return ret;
    }
    if (currentCall == nullptr)
    {
        return PJSIP_STATE.PJSIP_STATE_NO_CALL;
    }
    CallOpParam *prm = new CallOpParam();
    if (prm == nullptr)
    {
        return PJSIP_STATE.PJSIP_STATE_INVALUE;
    }
    prm->statusCode = (PJSIP_SC_OK);
    pjsipSampleSetMediaParams(prm, pjsipSampleCfgIsVideoEnable());
    try
    {
        currentCall->answer(prm);
    }
    catch (string msg)
    {
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
        delete (currentCall);
        is_incomming_call = false;
        is_makeing_call = false;
        is_video_call = false;
        PJSIP_ERROR_DEBUG(msg);
        return PJSIP_STATE.PJSIP_STATE_ERROR;
    }
    pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER : 2);
    pjsipSampleTimerRestart((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER : 1);
    is_incomming_call = true;
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_OK;
}

/*
发送DTMF信息
 */
int pjsipSample::pjsipSampleDtmf(int type, string &code)
{
    PJSIP_ENTER_DEBUG();
    if (!pjsipSampleCheck("pjsipSampleCfgUsername "))
    {
        return -1;
    }
    int ret = pjsipSampleIsRegister();
    if (ret != 0)
    {
        PJSIP_ERROR_DEBUG("ret != 0");
        return ret;
    }
    if (currentCall == nullptr)
    {
        PJSIP_ERROR_DEBUG("currentcall null");
        return PJSIP_STATE.PJSIP_STATE_NO_CALL;
    }
    if (type == PJSUA_DTMF_METHOD_RFC2833)
    {
        try
        {
            currentCall->dialDtmf(code);
        }
        catch (string msg)
        {
            pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
            delete (currentCall);
            is_incomming_call = false;
            is_makeing_call = false;
            is_video_call = false;
            PJSIP_ERROR_DEBUG(msg);
            return PJSIP_STATE.PJSIP_STATE_ERROR;
        }
    }
    else if (type == PJSUA_DTMF_METHOD_SIP_INFO)
    {
        CallSendDtmfParam *prm = new CallSendDtmfParam();
        if (prm == nullptr)
        {
            delete (currentCall);
            is_incomming_call = false;
            is_makeing_call = false;
            is_video_call = false;
            currentCall = nullptr;
            PJSIP_ERROR_DEBUG("prm null");
            return PJSIP_STATE.PJSIP_STATE_ERROR;
        }
        prm->method = (PJSUA_DTMF_METHOD_SIP_INFO);
        prm->digits = (code);
        try
        {
            currentCall->sendDtmf(*prm);
        }
        catch (string msg)
        {
            pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
            delete (currentCall);
            is_incomming_call = false;
            is_makeing_call = false;
            is_video_call = false;
            currentCall = nullptr;
            PJSIP_ERROR_DEBUG(msg);
            return PJSIP_STATE.PJSIP_STATE_ERROR;
        }
    }
    PJSIP_LEAVE_DEBUG();
    return PJSIP_STATE.PJSIP_STATE_OK;
}

/*
检测是否已经注册
 */
int pjsipSample::pjsipSampleIsRegister()
{
    if (sipRegState == PJSIP_STATE.PJSIP_STATE_REGISTER_SUCCESS)
        return PJSIP_STATE.PJSIP_STATE_OK;
    return sipRegState;
}

/*
启动定时器
 */
int pjsipSample::pjsipSampleTimerStart(int delay, int period)
{
    if (pjsipTimerPtr == nullptr)
    {

        pjsipTimerPtr = new pjsipTimer(delay, period);
        //pjsipTimerPtr->pjsipSampleTimerListenerSet(pjsipSampleTimerHandle, (void*)(this));
    }
    else
    {
        pjsipTimerPtr = new pjsipTimer();
    }

    return 0;
}

int pjsipSample::pjsipSampleTimerStop(int type)
{
    if (pjsipTimerPtr != nullptr)
    {
        pjsipTimerPtr->pjsipTimerStop(type);
    }
    return 0;
}

int pjsipSample::pjsipSampleTimerRestart(int type)
{
    PJSIP_ENTER_DEBUG();
    if (pjsipTimerPtr != nullptr)
    {
        pjsipTimerPtr->pjsipTimerRestart(type);
    }
    PJSIP_LEAVE_DEBUG();
    return 0;
}

/*
定时处理信息
 */
int pjsipSample::pjsipSampleTimerHandle()
{
    PJSIP_ENTER_DEBUG();
    if (pjsipTimerPtr != nullptr)
    {
        int count = 0, timertype = 0;
        pjsipTimerPtr->pjsipTimerHandle();
        if (pjsipTimerPtr->pjsipTimerIsTimerout(pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER))
        {
            pjsipSampleTimerStop(pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER);
            pjsipSampleStopCall();
        }
        else if (pjsipTimerPtr->pjsipTimerIsTimerout(pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER))
        {
            pjsipSampleTimerStop(pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER);
            pjsipSampleStopCall();
        }
        else if (pjsipTimerPtr->pjsipTimerIsTimerout(pjsipTimerPtr->PJSIP_SAMPLE_REGISTER_TIMER))
        {
            pjsipSampleTimerStop(pjsipTimerPtr->PJSIP_SAMPLE_REGISTER_TIMER);
            try
            {
                if (account != nullptr)
                    account->setRegistration(true);
            }
            catch (string msg)
            {
                pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
                PJSIP_ERROR_DEBUG(msg);
                return -1;
            }
        }
        else if (pjsipTimerPtr->pjsipTimerIsTimerout(pjsipTimerPtr->PJSIP_SAMPLE_STOP_TALKING_TIMER))
        {
            pjsipSampleTimerStop(pjsipTimerPtr->PJSIP_SAMPLE_STOP_TALKING_TIMER);
            pjsipSampleStopCall();
        }
        if (pjsipTimerPtr->pjsipTimerIsEnable(pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER)) {
            timertype = pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER;
            count = pjsipTimerPtr->pjsipTimerCount(pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER);
        } else if (pjsipTimerPtr->pjsipTimerIsEnable(pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER)) {
            timertype = pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER;
            count = pjsipTimerPtr->pjsipTimerCount(pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER);
        } else if (pjsipTimerPtr->pjsipTimerIsEnable(pjsipTimerPtr->PJSIP_SAMPLE_STOP_TALKING_TIMER)) {
            timertype = pjsipTimerPtr->PJSIP_SAMPLE_STOP_TALKING_TIMER;
            count = pjsipTimerPtr->pjsipTimerCount(pjsipTimerPtr->PJSIP_SAMPLE_STOP_TALKING_TIMER);
        }
        if (pjsipSample_observer != nullptr)
            pjsipSample_observer->pjsipSampleNotifyTimer(0, timertype, count);
    }
    PJSIP_LEAVE_DEBUG();
    return 0;
}

void pjsipSample::pjsipAppNotifyRegisterState(pjsipAccount &account, int code, string &reason, long ex)
{
    PJSIP_ENTER_DEBUG();
    string msg_str = "";
    if (ex == 0)
    {
        msg_str += "Unregistration";
        sipRegState = PJSIP_STATE.PJSIP_STATE_UNREGISTER;
    }
    else
    {
        msg_str += "Registration";
        sipRegState = PJSIP_STATE.PJSIP_STATE_REGISTER_FAILED;
    }

    if (code / 100 == 2)
    {
        msg_str += " successful";
        if (sipRegState == PJSIP_STATE.PJSIP_STATE_REGISTER_FAILED)
            sipRegState = PJSIP_STATE.PJSIP_STATE_REGISTER_SUCCESS;
    }
    else
    {
        msg_str += " failed: " + reason;
        if (sipRegState == PJSIP_STATE.PJSIP_STATE_REGISTER_FAILED)
            sipRegState = PJSIP_STATE.PJSIP_STATE_REGISTER_FAILED;
    }
    if(pjsipSample_observer != nullptr)
        pjsipSample_observer->pjsipSampleNotifyRegisterState(account,  sipRegState, reason,  ex);
    PJSIP_LEAVE_DEBUG();
}

void pjsipSample::pjsipAppNotifyIncomingCall(pjsipCall& call)
{
    PJSIP_ENTER_DEBUG();
    bool isVideo = false;
    CallInfo callInfo;
    CallOpParam *prm = new CallOpParam();
    if (prm == nullptr)
    {
        delete (&call);
        PJSIP_ERROR_DEBUG("prm null");
        return;
    }
    /* Only one call at anytime */
    if (currentCall != nullptr && is_makeing_call == false)
    {
        prm->statusCode = (PJSIP_SC_BUSY_HERE);
        try
        {
            call.hangup(prm);
        }
        catch (string msg)
        {
            //if(pjsipSample_observer != nullptr)
            //    pjsipSample_observer->pjsipSampleNotifyIncomingCall(call);
            pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
            delete (&call);
            delete (prm);
            is_incomming_call = false;
            is_makeing_call = false;
            is_video_call = false;
            PJSIP_ERROR_DEBUG(msg);
            return;
        }
        //if (sampleObserver != nullptr)
        //    sampleObserver.pjsipSampleNotifyIncomingCall(nullptr, "is already have call");
        delete (&call);
        delete (prm);
        PJSIP_ERROR_DEBUG("if (currentCall != nullptr && is_makeing_call == false)");
        return;
    }
    try
    {
        callInfo = call.getInfo();
    }
    catch (string msg)
    {
        //if (sampleObserver != nullptr)
        //    sampleObserver.pjsipSampleNotifyIncomingCall(nullptr, "Exception");
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
        delete (&call);
        delete (prm);
        is_incomming_call = false;
        is_makeing_call = false;
        is_video_call = false;
        PJSIP_ERROR_DEBUG(msg);
        return;
    }
    //if (callInfo) {
    isVideo = (callInfo.remOfferer && callInfo.remVideoCount > 0);
    //System.out.println("================================================== pjsipSample notifyIncomingCall getRemOfferer:" + callInfo.getRemOfferer() + " getRemVideoCount:" + callInfo.getRemVideoCount());
    //}
    if (isVideo && pjsipSampleCfgIsVideoEnable())
    {
        //System.out.println("================================================== pjsipSample notifyIncomingCall isVideo");
        pjsipSampleSetMediaParams(prm, isVideo);
    }
    else
    {
        isVideo = false;
        pjsipSampleSetMediaParams(prm, false);
        /* code */
    }
    
    /* Answer with ringing */
    prm->statusCode = (PJSIP_SC_RINGING);
    try
    {
        call.answer(prm);
    }
    catch (string msg)
    {
        //if (sampleObserver != nullptr)
        //    sampleObserver.pjsipSampleNotifyIncomingCall(nullptr, "Exception");
        delete (&call);
        delete (prm);
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
        is_incomming_call = false;
        is_makeing_call = false;
        is_video_call = false;
        PJSIP_ERROR_DEBUG(msg);
        return;
    }
    pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER : 1);
    pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER : 2);
    pjsipSampleTimerRestart((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER : 2);
    delete (prm);
    currentCall = &call;
    is_makeing_call = false;
    is_incomming_call = true;
    is_video_call = isVideo;
    if(pjsipSample_observer != nullptr)
        pjsipSample_observer->pjsipSampleNotifyIncomingCall(call, is_video_call);
    PJSIP_LEAVE_DEBUG();
}

void pjsipSample::pjsipAppNotifyCallState(pjsipCall& call)
{
    PJSIP_ENTER_DEBUG();
    //std::cout << "pjsipSample::pjsipAppNotifyCallState" << std::endl;
    int role = 0;
    string reason = "";
    CallInfo ci;
    if (currentCall == nullptr || call.getId() != currentCall->getId())
    {
        //if (pjsip_intent != "makeCall") {
        reason = "No Call";
        pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER : 1);
        pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER : 2);
        if(pjsipSample_observer != nullptr)
            pjsipSample_observer->pjsipSampleNotifyCallState(call, false, PJSIP_INV_STATE_NULL, reason);
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_ERROR, "pjsipSample", "notifyCallState : " + reason);
        //}
        PJSIP_ERROR_DEBUG(reason);
        return;
    }
    try
    {
        ci = call.getInfo();
    }
    catch (string msg)
    {
        //if (sampleObserver != nullptr)
        //    sampleObserver.pjsipSampleNotifyCallState(call, ci, pjsip_inv_state.PJSIP_INV_STATE_nullptr, nullptr, "Exception");
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_FATAL, "pjsipSample", "Exception: " + msg);
        is_incomming_call = false;
        is_makeing_call = false;
        is_video_call = false;
        currentCall = nullptr;
        PJSIP_ERROR_DEBUG(msg);
        return;
    }
    /*
    if (ci == nullptr) {
        reason = "Call disconnected";
        pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER : 1);
        pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER : 2);
        //if (sampleObserver != nullptr)
        //    sampleObserver.pjsipSampleNotifyCallState(call, ci, pjsip_inv_state.PJSIP_INV_STATE_DISCONNECTED, nullptr, reason);
        pjsip_app->ep->utilLogWrite(PJSIP_LOG.PJSIP_SAMPLE_LOGLEVEL_ERROR, "pjsipSample", "notifyCallState : " + reason);
        is_incomming_call = false;
        is_makeing_call = false;
        is_video_call = false;
        currentCall = nullptr;
        return;
    }
    */
    role = ci.role;
    callState = ci.state;
    reason = ci.stateText;
    switch (callState)
    {
    case PJSIP_INV_STATE_NULL:
        currentCall = nullptr;
        //System.out.println("PJSIP_INV_STATE_nullptr");
        break;
    case PJSIP_INV_STATE_CALLING:
        //System.out.println("PJSIP_INV_STATE_CALLING");
        break;
    case PJSIP_INV_STATE_INCOMING:
        //System.out.println("PJSIP_INV_STATE_INCOMING");
        break;
    case PJSIP_INV_STATE_EARLY:
        //System.out.println("PJSIP_INV_STATE_EARLY");
        break;
    case PJSIP_INV_STATE_CONNECTING:
        //System.out.println("PJSIP_INV_STATE_CONNECTING");
        break;
    case PJSIP_INV_STATE_CONFIRMED: //接通状态
        //System.out.println("PJSIP_INV_STATE_CONFIRMED");
        pjsipSampleTimerRestart((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER : 1);

        if(currentCall->incomintVideoWindow != nullptr && currentCall->outgoingVideoPreview != nullptr && ci.remVideoCount >= 1)
            is_video_call = true;
        else
            is_video_call = false;
        /*
        if (pjsipCallLog != null) {
            pjsipCallLog.pjsipSampleLogUpdateAccept(true);
            pjsipCallLog.pjsipSampleLogUpdateVideo(is_video_call);
        }
        */
        break;
    case PJSIP_INV_STATE_DISCONNECTED: //对端挂机状态
        //System.out.println("PJSIP_INV_STATE_DISCONNECTED");
        pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER : 1);
        pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_CALLING_TIMER : 2);
        reason = "Call disconnected: " + ci.lastReason;
        currentCall = nullptr;
        break;
    default:
        break;
    }
    if(pjsipSample_observer != nullptr)
        pjsipSample_observer->pjsipSampleNotifyCallState(call, is_video_call, callState, reason);
    PJSIP_LEAVE_DEBUG();
}

void pjsipSample::pjsipAppNotifyCallMediaState(pjsipCall& call)
{
    //std::cout << "pjsipSample::pjsipAppNotifyCallMediaState" << std::endl;
    bool isvideo = false;
    if (currentCall != nullptr && currentCall->incomintVideoWindow != nullptr)
    {
        isvideo = true;
    }
    if(pjsipSample_observer != nullptr)
        pjsipSample_observer->pjsipSampleNotifyCallMediaState(call, isvideo);
}

void pjsipSample::pjsipAppNotifyDtmfInfo(pjsipCall& call, int type, string &msg)
{
    PJSIP_ENTER_DEBUG();
    if (msg == "#")
    { //启动定时器，5秒后挂断电话
        pjsipSampleTimerStop((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_TALKING_TIMER : 1);
        pjsipSampleTimerRestart((pjsipTimerPtr != nullptr) ? pjsipTimerPtr->PJSIP_SAMPLE_STOP_TALKING_TIMER : 4);
        //pjsipSampleWriteMessage(MSG_TYPE.PJSIP_STOP_CALL, nullptr);
    }
    if(pjsipSample_observer != nullptr)
        pjsipSample_observer->pjsipSampleNotifyDtmfInfo(call, type, msg);
    //System.out.println("=================== notifyCallDtmf: " + code);
    PJSIP_LEAVE_DEBUG();
}
