#ifndef PJSIPCALL_HPP
#define PJSIPCALL_HPP

#include <pjsua2.hpp>
#include <iostream>
#include "pjsipobserver.hpp"
using namespace pj;
using namespace std;

class pjsipCall : public Call
{
public:
    pjsipCall(Account &account, int call_id = PJSUA_INVALID_ID);
    //pjsipCall(pjsipObserver *observer, Account &acc, int call_id = PJSUA_INVALID_ID);
    ~pjsipCall();

    void pjsipCallObserver(pjsipObserver *observer);
    void onCallState(OnCallStateParam &prm);
    void onCallTsxState(OnCallTsxStateParam &prm);
    void onCallMediaState(OnCallMediaStateParam &prm);
    void onCallSdpCreated(OnCallSdpCreatedParam &prm);
    void onStreamCreated(OnStreamCreatedParam &prm);
    void onStreamDestroyed(OnStreamDestroyedParam &prm);
    void onDtmfDigit(OnDtmfDigitParam &prm);
    void onCallTransferRequest(OnCallTransferRequestParam &prm);
    void onCallTransferStatus(OnCallTransferStatusParam &prm);
    void onCallReplaceRequest(OnCallReplaceRequestParam &prm);
    void onCallReplaced(OnCallReplacedParam &prm);
    void onCallRxOffer(OnCallRxOfferParam &prm);
    void onCallRxReinvite(OnCallRxReinviteParam &prm);
    void onCallTxOffer(OnCallTxOfferParam &prm);
    void onInstantMessage(OnInstantMessageParam &prm);
    void onInstantMessageStatus(OnInstantMessageStatusParam &prm);
    void onTypingIndication(OnTypingIndicationParam &prm);
    pjsip_redirect_op onCallRedirected(OnCallRedirectedParam &prm)
    {
        return PJSIP_REDIRECT_REJECT;
    }
    void onCallMediaTransportState(OnCallMediaTransportStateParam &prm);
    void onCallMediaEvent(OnCallMediaEventParam &prm);
    void onCreateMediaTransport(OnCreateMediaTransportParam &prm);
    void onCreateMediaTransportSrtp(OnCreateMediaTransportSrtpParam &prm);

    VideoWindow *incomintVideoWindow = nullptr;
    VideoPreview *outgoingVideoPreview = nullptr;

private:
    //Endpoint *ep = nullptr;
    std::vector<Call *> call_list;
    pjsipObserver *pjObserver = nullptr;
};

#endif // PJSIPCALL_HPP
