#include <fstream>
#include <iostream>
#include "pjsipapp.hpp"
#include "pjsiplogwriter.hpp"
#include "pjsipobserver.hpp"

using namespace pj;
using namespace std;

pjsipApp::pjsipApp()
{
    PJSIP_ENTER_DEBUG();
    PJSIP_LEAVE_DEBUG();
}
pjsipApp::pjsipApp(pjsipObserver *observer)
{
    PJSIP_ENTER_DEBUG();
    ep = new Endpoint();
    epConfig = new EpConfig();
    sipTpConfig = new TransportConfig();
    pjObserver = observer;
    PJSIP_LEAVE_DEBUG();
}



int pjsipApp::pjsipInit(string &app_config)
{
    string appname = "pjsip";
    return pjsipInit(appname, app_config, false);
}

int pjsipApp::pjsipInit(string &appName, string &app_config)
{
    return pjsipInit(appName, app_config, false);
}


int pjsipApp::pjsipInit(string &appName, string &app_config,
                        bool own_worker_thread)
{
    PJSIP_ENTER_DEBUG();
    if (ep == nullptr || sipTpConfig == nullptr || epConfig == nullptr)
    {
        if (ep == nullptr)
            PJSIP_ERROR_DEBUG("pjsipInit ep == nullptr");
        else if (sipTpConfig == nullptr)
            PJSIP_ERROR_DEBUG("pjsipInit sipTpConfig == nullptr");
        else if (epConfig == nullptr)
            PJSIP_ERROR_DEBUG("pjsipInit epConfig == nullptr");
        return -1;
    }
    pjsipLogWriter *logWriter = new pjsipLogWriter();
    /* Create endpoint */
    try
    {
        ep->libCreate();
    }
    catch (const string msg)
    {
        ep->utilLogWrite(0, "pjsipApp", "Exception:" + msg);
        return -1;
    }
    // 以读模式打开文件

    ifstream load_file(app_config, std::ifstream::in);
    if(load_file.is_open())
        loadConfig(app_config);

    sipTpConfig->port = LOCAL_SIP_PORT;
    /* Override log level setting */
    epConfig->logConfig.level = LOG_LEVEL;
    epConfig->logConfig.consoleLevel = LOG_LEVEL;
    epConfig->logConfig.decor = PJ_LOG_HAS_CR | PJ_LOG_HAS_NEWLINE | PJ_LOG_HAS_SENDER | PJ_LOG_HAS_LEVEL_TEXT | PJ_LOG_HAS_INDENT;
    /* Set log config. */
    LogConfig log_cfg = epConfig->logConfig;
    log_cfg.writer = (logWriter);

    /* Set ua config. */
    UaConfig ua_cfg = epConfig->uaConfig;
    ua_cfg.userAgent = appName;

    /* No worker thread */
    if (own_worker_thread)
    {
        ua_cfg.threadCnt = 0;
        ua_cfg.mainThreadOnly = true;
    }

    /* Init endpoint */
    try
    {
        ep->libInit(*epConfig);
    }
    catch (const string msg)
    {
        ep->utilLogWrite(0, "pjsipApp", "Exception:" + msg);
        return -1;
    }
    PJSIP_LEAVE_DEBUG();
    return 0;
}



int pjsipApp::pjsipTransportInit(int port, int proto)
{
    PJSIP_ENTER_DEBUG();
    if (ep == nullptr || sipTpConfig == nullptr || epConfig == nullptr)
    {
        if (ep == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit ep == nullptr");
        else if (sipTpConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit sipTpConfig == nullptr");
        else if (epConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit epConfig == nullptr");
        return -1;
    }
    LOCAL_SIP_PORT = port;
    /* Create transports. */

    sipTpConfig->port = (LOCAL_SIP_PORT);

    try
    {
        ep->transportCreate(PJSIP_TRANSPORT_UDP, *sipTpConfig);
    }
    catch (const string msg)
    {
        ep->utilLogWrite(0, "pjsipApp", "Exception:" + msg);
        return -1;
    }
    try
    {
        ep->transportCreate(PJSIP_TRANSPORT_TCP, *sipTpConfig);
    }
    catch (const string msg)
    {
        ep->utilLogWrite(0, "pjsipApp", "Exception:" + msg);
        return -1;
    }
    if (proto == PJSIP_TRANSPORT_TLS)
        sipTpConfig->port = (LOCAL_SIP_PORT + 1);

    try
    {
        ep->transportCreate(PJSIP_TRANSPORT_TLS, *sipTpConfig);
    }
    catch (const string msg)
    {
        ep->utilLogWrite(0, "pjsipApp", "Exception:" + msg);
        return -1;
    }

    sipTpConfig->port = (LOCAL_SIP_PORT);
    PJSIP_LEAVE_DEBUG();
    return 0;
}

int pjsipApp::pjsipAppStart()
{
    PJSIP_ENTER_DEBUG();
    if (ep == nullptr || sipTpConfig == nullptr || epConfig == nullptr)
    {
        if (ep == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit ep == nullptr");
        else if (sipTpConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit sipTpConfig == nullptr");
        else if (epConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit epConfig == nullptr");
        return -1;
    }
    pjsipAppLoadConfig();
    /* Start. */
    try
    {
        ep->libStart();
    }
    catch (const string msg)
    {
        ep->utilLogWrite(0, "pjsipApp", "Exception:" + msg);
        return -1;
    }
    PJSIP_LEAVE_DEBUG();
    return 0;
}

int pjsipApp::pjsipSetLogLevel(int level)
{
    PJSIP_ENTER_DEBUG();
    if (ep == nullptr || sipTpConfig == nullptr || epConfig == nullptr)
    {
        LOG_LEVEL = level;
        if (ep == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit ep == nullptr");
        else if (sipTpConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit sipTpConfig == nullptr");
        else if (epConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit epConfig == nullptr");
        return -1;
    }
    epConfig->logConfig.level = level;
    epConfig->logConfig.consoleLevel = level;
    PJSIP_LEAVE_DEBUG();
    return 0;
}

int pjsipApp::pjsipAccountAdd(AccountConfig &cfg, pjsipAccount **pjsipacc)
{
    if (ep == nullptr || sipTpConfig == nullptr || epConfig == nullptr)
    {
        if (ep == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit ep == nullptr");
        else if (sipTpConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit sipTpConfig == nullptr");
        else if (epConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit epConfig == nullptr");
        return -1;
    }
    pjsipAccount *acc = new pjsipAccount(pjObserver, cfg);
    try
    {
        acc->create(cfg);
    }
    catch (const string msg)
    {
        ep->utilLogWrite(0, "pjsipApp", "Exception:" + msg);
        acc = nullptr;
        return -1;
    }

    //accList.add(acc);
    accList.push_back(acc);
    *pjsipacc = acc;
    PJSIP_LEAVE_DEBUG();
    return 0;
}

int pjsipApp::pjsipAccountGet(int id, pjsipAccount **pjsipacc)
{
    PJSIP_ENTER_DEBUG();
    if (ep == nullptr || sipTpConfig == nullptr || epConfig == nullptr)
    {
        if (ep == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit ep == nullptr");
        else if (sipTpConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit sipTpConfig == nullptr");
        else if (epConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit epConfig == nullptr");
        return -1;
    }
    *pjsipacc = accList.at(id);
    PJSIP_LEAVE_DEBUG();
    return 0;
}

int pjsipApp::pjsipAccountDelete(pjsipAccount &acc)
{
    PJSIP_ENTER_DEBUG();
    if (ep == nullptr || sipTpConfig == nullptr || epConfig == nullptr)
    {
        if (ep == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit ep == nullptr");
        else if (sipTpConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit sipTpConfig == nullptr");
        else if (epConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit epConfig == nullptr");
        return -1;
    }
    //accList.erase(acc);
    for (std::vector<pjsipAccount *>::iterator it = accList.begin();
         it != accList.end(); ++it)
    {
        if (*it == &acc)
        {
            accList.erase(it);
            break;
        }
    }
    PJSIP_LEAVE_DEBUG();
    return 0;
}

int pjsipApp::pjsipDeinit()
{
    PJSIP_ENTER_DEBUG();
    if (ep == nullptr || sipTpConfig == nullptr || epConfig == nullptr)
    {
        if (ep == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit ep == nullptr");
        else if (sipTpConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit sipTpConfig == nullptr");
        else if (epConfig == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipInit epConfig == nullptr");
        return -1;
    }
    /* Shutdown pjsua. Note that Endpoint destructor will also invoke
     * libDestroy(), so this will be a test of double libDestroy().
     */
    try
    {
        ep->libDestroy();
    }
    catch (const string msg)
    {
       ep->utilLogWrite(0, "pjsipApp", "Exception:" + msg);
    }

    /* Force delete Endpoint here, to avoid deletion from a non-
     * registered thread (by GC?).
     */
    delete (ep);
    ep = nullptr;
    PJSIP_LEAVE_DEBUG();
    return 0;
}

int pjsipApp::pjsipAppLoadConfig()
{
    PJSIP_ENTER_DEBUG();
    /* Create accounts. */
    for (int i = 0; i < pjsipAccCfgs.size(); i++)
    {
        pjsipAccountConfig *pjsip_cfg = pjsipAccCfgs.at(i);
        if (pjsip_cfg != nullptr)
        {
            if (pjsip_cfg->account_config_cfg != nullptr)
            {
                pjsipAccount *acc = nullptr;
                pjsipAccountAdd(*pjsip_cfg->account_config_cfg, &acc);
                if (acc == nullptr)
                    continue;
                /* Add Buddies */
                for (int j = 0; j < pjsip_cfg->buddyCfgs.size(); j++)
                {
                    BuddyConfig *bud_cfg = pjsip_cfg->buddyCfgs.at(j);
                    if (bud_cfg)
                        acc->pjsipAccountBuddyAdd(*bud_cfg, nullptr);
                }
            }
        }
    }
    PJSIP_LEAVE_DEBUG();
    return 0;
}

int pjsipApp::loadConfig(string &filename)
{ 
    int i = 0;
    PJSIP_ENTER_DEBUG();
    JsonDocument *json = new JsonDocument();
    if (!json)
    {
        if (ep == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "loadConfig json == nullptr");
        return -1; 
    }
    try
    {
        //PJSIP_MSG_DEBUG(filename);
        //std::cout << "pjsipInit loadConfig:" <<  filename << std::endl;
        /* Load file */
        json->loadFile(filename);
        ContainerNode root = json->getRootContainer();

        /* Read endpoint config */
        epConfig->readObject(root);

        /* Read transport config */
        ContainerNode tp_node = root.readContainer("SipTransport");
        sipTpConfig->readObject(tp_node);

        /* Read account configs */
        pjsipAccCfgs.clear();
        ContainerNode accs_node = root.readArray("accounts");
        while (accs_node.hasUnread() && i < 1)
        { 
            i++;
            pjsipAccountConfig *acc_cfg = new pjsipAccountConfig();
            acc_cfg->readObject(accs_node);
            pjsipAccCfgs.push_back(acc_cfg);
        }
    }
    catch (const string msg)
    {
        ep->utilLogWrite(0, "pjsipApp", "Exception:" + msg);
        return -1;
    }

    /* Force delete json now, as I found that Java somehow destroys it
     * after lib has been destroyed and from non-registered thread.
     */
    delete (json);
    PJSIP_LEAVE_DEBUG();
    return 0;
}
/* Sync accCfgs from accList */
void pjsipApp::buildAccConfigs()
{
    PJSIP_ENTER_DEBUG();
    pjsipAccCfgs.clear();
    for (int i = 0; i < accList.size(); i++)
    {
        pjsipAccount *acc = accList.at(i);
        pjsipAccountConfig *pjsip_acc_cfg = new pjsipAccountConfig();
        pjsip_acc_cfg->account_config_cfg = acc->account_cfg;

        pjsip_acc_cfg->buddyCfgs.clear();
        for (int j = 0; j < acc->buddyList.size(); j++)
        {
            pjsipBuddy *bud = acc->buddyList.at(j);
            pjsip_acc_cfg->buddyCfgs.push_back(bud->buddy_cfg);
        }
        pjsipAccCfgs.push_back(pjsip_acc_cfg);
    }
    PJSIP_LEAVE_DEBUG();
}

int pjsipApp::pjsipAppSaveConfig(string &filename)
{
    PJSIP_ENTER_DEBUG();
    JsonDocument *json = new JsonDocument();
    if (!json)
    {
        if (ep == nullptr)
            ep->utilLogWrite(0, "pjsipApp", "pjsipAppSaveConfig json == nullptr");
        return -1; 
    }
    try
    {
        /* Write endpoint config */
        json->writeObject(*epConfig);

        /* Write transport config */
        ContainerNode tp_node = json->writeNewContainer("SipTransport");
        sipTpConfig->writeObject(tp_node);

        /* Write account configs */
        buildAccConfigs();
        if(pjsipAccCfgs.size() > 0)
        {
            ContainerNode accs_node = json->writeNewArray("accounts");
            for (int i = 0; i < pjsipAccCfgs.size(); i++)
            {
                pjsipAccountConfig *pjsip_acc_cfg = pjsipAccCfgs.at(i);
                if (pjsip_acc_cfg != nullptr)
                    pjsip_acc_cfg->writeObject(accs_node);
                //pjsipAccCfgs.get(i).writeObject(accs_node);
            }
        }
        /* Save file */
        json->saveFile(filename);
    }
    catch (const string msg)
    {
        ep->utilLogWrite(0, "pjsipApp", "Exception:" + msg);
        return -1;
    }

    /* Force delete json now, as I found that Java somehow destroys it
     * after lib has been destroyed and from non-registered thread.
     */
    delete (json);
    PJSIP_LEAVE_DEBUG();
    return 0;
}
