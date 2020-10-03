#include "pjsipcall.hpp"
#include "pjsiplogwriter.hpp"
#include "pjsipobserver.hpp"

using namespace pj;
using namespace std;
/*
 * class pjsipCall
 */
pjsipCall::pjsipCall(Account &acc, int call_id)
    : Call(acc, call_id)
{
    PJSIP_ENTER_DEBUG();
    incomintVideoWindow = nullptr;
    PJSIP_LEAVE_DEBUG();
}

pjsipCall::~pjsipCall()
{
}

void pjsipCall::pjsipCallObserver(pjsipObserver *observer)
{
    PJSIP_ENTER_DEBUG();
    pjObserver = observer;
    PJSIP_LEAVE_DEBUG();
}

void pjsipCall::onCallState(OnCallStateParam &prm)
{
    PJSIP_ENTER_DEBUG();
    if (pjObserver)
        pjObserver->pjsipAppNotifyCallState(*this);
    PJSIP_LEAVE_DEBUG();    
}

void pjsipCall::onCallTsxState(OnCallTsxStateParam &prm)
{
}

void pjsipCall::onCallMediaState(OnCallMediaStateParam &prm)
{
    PJSIP_ENTER_DEBUG();
    CallInfo ci;
    try
    {
        ci = getInfo();
    }
    catch (const string msg)
    {
        utilLogWrite(0, "pjsipCall", "exception:" + msg);
        return;
    }

    CallMediaInfoVector cmiv = ci.media;

    for (int i = 0; i < cmiv.size(); i++)
    {
        CallMediaInfo cmi = cmiv.at(i);
        if (cmi.type == PJMEDIA_TYPE_AUDIO &&
            (cmi.status == PJSUA_CALL_MEDIA_ACTIVE ||
             cmi.status == PJSUA_CALL_MEDIA_REMOTE_HOLD))
        {
            // connect ports
            try
            {
                AudioMedia am = getAudioMedia(i);
                //if(ep)
                {
                    //ep->audDevManager().getCaptureDevMedia().startTransmit(am);
                    //am.startTransmit(ep->audDevManager().getPlaybackDevMedia());
                    Endpoint::instance().audDevManager().getCaptureDevMedia().startTransmit(am);
                    am.startTransmit(Endpoint::instance().audDevManager().getPlaybackDevMedia());
                    am.adjustRxLevel(1);
                    am.adjustTxLevel(1);
                }
            }
            catch (const string msg)
            {
                utilLogWrite(0, "pjsipCall", "exception:" + msg);
                continue;
            }
        }
        else if (cmi.type == PJMEDIA_TYPE_VIDEO &&
                 cmi.status ==
                     PJSUA_CALL_MEDIA_ACTIVE /*&&
           cmi.getVideoIncomingWindowId() != pjsua2.INVALID_ID*/
        )
        {

            //System.out.println("==================================================pjsipCall onCallMediaState PJMEDIA_TYPE_VIDEO");
            if (cmi.videoIncomingWindowId != INVALID_ID)
                incomintVideoWindow = new VideoWindow(cmi.videoIncomingWindowId);
            outgoingVideoPreview = new VideoPreview(cmi.videoCapDev);
        }
        //System.out.println("==================================================pjsipCall onCallMediaState getType:" + cmi.getType() + " getStatus:" +
        //        cmi.status + " getVideoIncomingWindowId:" + cmi.getVideoIncomingWindowId());
    }
    if (pjObserver)
        pjObserver->pjsipAppNotifyCallMediaState(*this);
    PJSIP_LEAVE_DEBUG();
}

void pjsipCall::onCallSdpCreated(OnCallSdpCreatedParam &prm)
{
}

void pjsipCall::onStreamCreated(OnStreamCreatedParam &prm)
{
}

void pjsipCall::onStreamDestroyed(OnStreamDestroyedParam &prm)
{
}

void pjsipCall::onDtmfDigit(OnDtmfDigitParam &prm)
{
    PJSIP_ENTER_DEBUG();
    if (pjObserver)
        pjObserver->pjsipAppNotifyDtmfInfo(*this, prm.method, prm.digit);
    //pjsipApp.observer.notifyCallDtmf(this, prm.method, prm.digit);
    PJSIP_LEAVE_DEBUG();
}

void pjsipCall::onCallTransferRequest(OnCallTransferRequestParam &prm)
{
}

void pjsipCall::onCallTransferStatus(OnCallTransferStatusParam &prm)
{
}

void pjsipCall::onCallReplaceRequest(OnCallReplaceRequestParam &prm)
{
}

void pjsipCall::onCallReplaced(OnCallReplacedParam &prm)
{
}

void pjsipCall::onCallRxOffer(OnCallRxOfferParam &prm)
{
}

void pjsipCall::onCallRxReinvite(OnCallRxReinviteParam &prm)
{
}

void pjsipCall::onCallTxOffer(OnCallTxOfferParam &prm)
{
}

void pjsipCall::onInstantMessage(OnInstantMessageParam &prm)
{
}

void pjsipCall::onInstantMessageStatus(OnInstantMessageStatusParam &prm)
{
}

void pjsipCall::onTypingIndication(OnTypingIndicationParam &prm)
{
}

void pjsipCall::onCallMediaTransportState(OnCallMediaTransportStateParam &prm)
{
}

void pjsipCall::onCallMediaEvent(OnCallMediaEventParam &prm)
{
}

void pjsipCall::onCreateMediaTransport(OnCreateMediaTransportParam &prm)
{
}

void pjsipCall::onCreateMediaTransportSrtp(OnCreateMediaTransportSrtpParam &prm)
{
}
