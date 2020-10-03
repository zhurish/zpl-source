#ifndef PJSIPAPP_HPP
#define PJSIPAPP_HPP
#include <pjsua2.hpp>
#include <iostream>
#include "pjsipbuddy.hpp"
#include "pjsipaccount.hpp"
#include "pjsipcall.hpp"
#include "pjsipobserver.hpp"

using namespace pj;
using namespace std;
/*
异常	描述
std::exception	该异常是所有标准 C++ 异常的父类。
std::bad_alloc	该异常可以通过 new 抛出。
std::bad_cast	该异常可以通过 dynamic_cast 抛出。
std::bad_exception	这在处理 C++ 程序中无法预期的异常时非常有用。
std::bad_typeid	该异常可以通过 typeid 抛出。
std::logic_error	理论上可以通过读取代码来检测到的异常。
std::domain_error	当使用了一个无效的数学域时，会抛出该异常。
std::invalid_argument	当使用了无效的参数时，会抛出该异常。
std::length_error	当创建了太长的 std::string 时，会抛出该异常。
std::out_of_range	该异常可以通过方法抛出，例如 std::vector 和 std::bitset<>::operator[]()。
std::runtime_error	理论上不可以通过读取代码来检测到的异常。
std::overflow_error	当发生数学上溢时，会抛出该异常。
std::range_error	当尝试存储超出范围的值时，会抛出该异常。
std::underflow_error	当发生数学下溢时，会抛出该异常。
*/
class pjsipApp
{
public:
    pjsipApp();
    pjsipApp(pjsipObserver *observer);

    Endpoint *ep = nullptr;
    std::vector<pjsipAccount *> accList;

    int pjsipInit(string &app_dir);
    int pjsipInit(string &appName, string &app_dir);
    int pjsipInit(string &appName, string &app_dir,
                        bool own_worker_thread);

    int pjsipTransportInit(int port, int proto);

    int pjsipAppStart();
    int pjsipSetLogLevel(int level);
    int pjsipAccountAdd(AccountConfig &cfg, pjsipAccount **pjsipacc);
    int pjsipAccountGet(int id, pjsipAccount **pjsipacc);
    int pjsipAccountDelete(pjsipAccount &acc);
    int pjsipDeinit();
    int pjsipAppSaveConfig(string &filename);

private:
    std::vector<pjsipAccountConfig *> pjsipAccCfgs;
    EpConfig *epConfig = nullptr;
    TransportConfig *sipTpConfig = nullptr;

    int LOCAL_SIP_PORT = 5060;
    int LOG_LEVEL = 6;

    pjsipObserver *pjObserver = nullptr;
    int pjsipAppLoadConfig();
    int loadConfig(string &filename);
    void buildAccConfigs();
};

#endif // PJSIPAPP_HPP
