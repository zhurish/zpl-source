#ifndef PJSIPSAMPLE_H
#define PJSIPSAMPLE_H

#include "pjsipaccount.hpp"
#include "pjsipcall.hpp"
#include "pjsipbuddy.hpp"
#include "pjsipapp.hpp"
#include "pjsiptimer.hpp"
#include "pjsipobserver.hpp"
#include "pjsipjsonconfig.hpp"
#include "pjsipsampleobserver.hpp"

using namespace pj;
using namespace std;

class pjsipSampleState
{
public:
    //pjsipSampleState(){};
    int PJSIP_STATE_ERROR = -1; //异常错误
    int PJSIP_STATE_OK = 0;     //成功

    int PJSIP_STATE_UNREGISTER = 1;      //未注册
    int PJSIP_STATE_REGISTER_FAILED = 2; //注册失败
    int PJSIP_STATE_REGISTER_SUCCESS = 3;

    int PJSIP_STATE_NO_ACC = 4; //没有注册

    int PJSIP_STATE_NO_CALL = 5; //没有呼叫

    int PJSIP_STATE_INVALUE = 6; //参数异常

    //信息响应
    int PJSIP_STATE_100 = 100; //100试呼叫（Trying）
    int PJSIP_STATE_180 = 180; //180振铃（Ringing）
    int PJSIP_STATE_181 = 181; //181呼叫正在前转（Call is Being Forwarded）
    int PJSIP_STATE_182 = 182; //182排队
    //成功响应
    int PJSIP_STATE_200 = 200; //200成功响应（OK）

    //重定向响应
    int PJSIP_STATE_300 = 300; //300多重选择
    int PJSIP_STATE_301 = 301; //永久迁移
    int PJSIP_STATE_302 = 302; //302临时迁移（Moved Temporarily）
    int PJSIP_STATE_303 = 303; //见其它
    int PJSIP_STATE_305 = 305; //使用代理
    int PJSIP_STATE_380 = 380; //代换服务

    //客户出错
    int PJSIP_STATE_400 = 400; //400错误请求（Bad Request）
    int PJSIP_STATE_401 = 401; //401未授权（Unauthorized）
    int PJSIP_STATE_402 = 402; //要求付款
    int PJSIP_STATE_403 = 403; //403禁止（Forbidden）
    int PJSIP_STATE_404 = 404; //404用户不存在（Not Found）
    int PJSIP_STATE_405 = 405; //不允许的方法
    int PJSIP_STATE_406 = 406; //不接受
    int PJSIP_STATE_407 = 407; //要求代理权
    int PJSIP_STATE_408 = 408; //408请求超时（Request Timeout）
    int PJSIP_STATE_410 = 410; //消失
    int PJSIP_STATE_413 = 413; //请求实体太大
    int PJSIP_STATE_414 = 414; //请求URI太大
    int PJSIP_STATE_415 = 414; //不支持的媒体类型
    int PJSIP_STATE_416 = 416; //不支持的URI方案
    int PJSIP_STATE_420 = 420; //分机无人接听
    int PJSIP_STATE_421 = 421; //要求转机
    int PJSIP_STATE_423 = 423; //间隔太短
    int PJSIP_STATE_480 = 480; //480暂时无人接听（Temporarily Unavailable）
    int PJSIP_STATE_481 = 481; //呼叫腿/事务不存在

    int PJSIP_STATE_482 = 482; //相环探测
    int PJSIP_STATE_483 = 483; //跳频太高
    int PJSIP_STATE_484 = 484; //地址不完整
    int PJSIP_STATE_485 = 485; //不清楚
    int PJSIP_STATE_486 = 486; //486线路忙（Busy Here）

    int PJSIP_STATE_487 = 487; //终止请求
    int PJSIP_STATE_488 = 488; //此处不接受
    int PJSIP_STATE_491 = 491; //代处理请求
    int PJSIP_STATE_493 = 493; //难以辨认

    //服务器出错
    int PJSIP_STATE_500 = 500; //内部服务器错误
    int PJSIP_STATE_501 = 501; //没实现的
    int PJSIP_STATE_502 = 502; //无效网关
    int PJSIP_STATE_503 = 503; //不提供此服务
    int PJSIP_STATE_504 = 504; //504服务器超时（Server Time-out）
    int PJSIP_STATE_505 = 505; //SIP版本不支持
    int PJSIP_STATE_513 = 513; //消息太长

    //全局故障
    int PJSIP_STATE_600 = 600; //600全忙（Busy Everywhere）
    int PJSIP_STATE_603 = 603; //拒绝
    int PJSIP_STATE_604 = 604; //都不存在
    int PJSIP_STATE_606 = 606; //不接受
};

/*
    log 日志 信息级别
 */
class pjsipSampleLogLevel
{
public:
    //pjsipSampleLogLevel(){};
    int PJSIP_SAMPLE_LOGLEVEL_FATAL = 0;
    int PJSIP_SAMPLE_LOGLEVEL_ERROR = 1;
    int PJSIP_SAMPLE_LOGLEVEL_WARN = 2;
    int PJSIP_SAMPLE_LOGLEVEL_INFO = 3;
    int PJSIP_SAMPLE_LOGLEVEL_DEBUG = 6;
    int PJSIP_SAMPLE_LOGLEVEL_TRACE = 5;
    int PJSIP_SAMPLE_LOGLEVEL_DETRC = 4;
};

#define PJSIP_APP_CFG_FILE  "pjsipcfg-json"
#define PJSIP_MAIN_CFG_FILE  "pjmaincfg-json"

class pjsipSample : public pjsipObserver
{
public:
    pjsipSample();
    ~pjsipSample();
public:

    pjsipApp *pjsip_app = nullptr;
    pjsipAccount *account = nullptr;
    pjsipCall *currentCall = nullptr;
    AccountConfig *accountCfg = nullptr;
    AccountRegConfig *accountRegCfg = nullptr;




    pjsipJsonConfig *pjsipSampleCfg = nullptr;
    pjsipTimer *pjsipTimerPtr = nullptr;
    pjsipSampleLogLevel PJSIP_LOG;

    bool is_video_call = false;
    bool is_makeing_call = false;
    string pjsipAppCfgFile = PJSIP_APP_CFG_FILE;

    bool app_running = true;
private:
    //pjsipObserver *pjsip_observer = nullptr;
    AccountNatConfig *accountNatCfg = nullptr;
    AccountVideoConfig *accountVideoCfg = nullptr;
    AccountSipConfig *accountSipCfg = nullptr;

    pjsipSampleState PJSIP_STATE;

    bool is_incomming_call = false;
    int sipRegState = 0; //pjsipSampleState.PJSIP_STATE_ERROR;
    int callState = 0;   //pjsipSampleState.PJSIP_STATE_ERROR;

    pjsipSampleObserver *pjsipSample_observer = nullptr;

private:
    bool pjsipSampleCheck(string func);
    int pjsipSampleInitDefault();
    int pjsipSampleInitDefaultExit();
    int pjsipSampleRegInit(int regtimeout, int RetryInterval);
    int pjsipSampleSetMediaParams(CallOpParam param, bool videoCall);
    int pjsipSampleIsRegister();

    //int pjsipSampleTimerHandle();
    void pjsipAppNotifyRegisterState(pjsipAccount& account, int code, string &reason, long ex);
    void pjsipAppNotifyIncomingCall(pjsipCall& call);
    void pjsipAppNotifyCallState(pjsipCall& call);
    void pjsipAppNotifyCallMediaState(pjsipCall& call);
    void pjsipAppNotifyDtmfInfo(pjsipCall& call, int type, string &msg);
    int pjsipSampleInit_hw(pjsipSampleObserver *obj,  string &app_name, string &app_dir);

public:
    bool pjsipSampleCfgEmpty();
    int pjsipSampleCfgUsername(string &username, string &password);
    int pjsipSampleCfgProxy(string &proxy);
    int pjsipSampleCfgServer(string &server);
    int pjsipSampleCfgAccountID(string &acc_id);
    int pjsipSampleCfgRealm(string &realm);
    int pjsipSampleCfgProto(int proto);
    int pjsipSampleCfgCommit();
    int pjsipSampleCfgVideoEnable(bool enable);
    bool pjsipSampleCfgIsVideoEnable();

    int pjsipSampleInit(pjsipSampleObserver *obj,  string &app_name, string &app_dir);
    int pjsipSampleInit(pjsipSampleObserver *obj);
    int pjsipSampleInit(pjsipSampleObserver *obj,  string &app_dir);
    int pjsipSampleInit(string &app_name,  string &app_dir);
    int pjsipSampleDestroy();
    int pjsipSampleMakeCall(string &num, bool videoCall);
    int pjsipSampleStopCall();
    int pjsipSampleAcceptCall();
    int pjsipSampleDtmf(int type, string &code);
    int pjsipSampleTimerStart(int delay, int period);
    int pjsipSampleTimerStop(int type);
    int pjsipSampleTimerRestart(int type);
    int pjsipSampleTimerHandle();
};

#endif // PJSIPSAMPLE_H
