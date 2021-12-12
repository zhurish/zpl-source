#define _CRT_SECURE_NO_WARNINGS
#include "soapH.h"
/******************************************************************************\
 *                                                                            *
 * Server-Side Operations                                                     *
 *                                                                            *
\******************************************************************************/
#if defined(WIN32)
#pragma warning(disable:4505)
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif
#include "wsddapi.h"

#include <zpl_include.h>
#include <lib_include.h>

#include "onvif_util.h"
#include "onvif_test.h"




#ifdef ONVIF_CLIENT
SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__ProbeMatches(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{	struct __wsdd__ProbeMatches soap_tmp___wsdd__ProbeMatches;
	if (soap_action == NULL)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ProbeMatches";
	soap_tmp___wsdd__ProbeMatches.wsdd__ProbeMatches = wsdd__ProbeMatches;
	soap_begin(soap);
	soap->encodingStyle = NULL; /* use SOAP literal style */
	soap_serializeheader(soap);
	soap_serialize___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches);
	if (soap_begin_count(soap))
		return soap->error;
	if ((soap->mode & SOAP_IO_LENGTH))
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches, "-wsdd:ProbeMatches", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches, "-wsdd:ProbeMatches", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}
#endif /* ONVIF_CLIENT */



/** Onvif server operation SOAP_ENV__Fault */
int SOAP_ENV__Fault(struct soap *soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __wsdd__Hello */
int __wsdd__Hello(struct soap *soap, struct wsdd__HelloType *wsdd__Hello)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __wsdd__Bye */
int __wsdd__Bye(struct soap *soap, struct wsdd__ByeType *wsdd__Bye)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __wsdd__Probe */
int __wsdd__Probe(struct soap *soap, struct wsdd__ProbeType *wsdd__Probe)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);

	char _IPAddr[MAX_128_LEN] = { 0 };
	char _HwId[1024] = { 0 };
	zpl_uint8 tmp[512] = { 0 };
	zpl_uint16 port = 5000;


	wsdd__ProbeMatchesType ProbeMatches;
	ProbeMatches.ProbeMatch = (struct wsdd__ProbeMatchType *)soap_malloc(soap, sizeof(struct wsdd__ProbeMatchType));
	ProbeMatches.ProbeMatch->XAddrs = (char *)soap_malloc(soap, sizeof(char)* MAX_128_LEN);
	ProbeMatches.ProbeMatch->Types = (char *)soap_malloc(soap, sizeof(char)* MAX_128_LEN);
	ProbeMatches.ProbeMatch->Scopes = (struct wsdd__ScopesType*)soap_malloc(soap, sizeof(struct wsdd__ScopesType));
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceProperties = (struct wsa__ReferencePropertiesType*)soap_malloc(soap, sizeof(struct wsa__ReferencePropertiesType));
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceParameters = (struct wsa__ReferenceParametersType*)soap_malloc(soap, sizeof(struct wsa__ReferenceParametersType));
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName = (struct wsa__ServiceNameType*)soap_malloc(soap, sizeof(struct wsa__ServiceNameType));
	ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType = (char **)soap_malloc(soap, sizeof(char *)* MAX_128_LEN);
	ProbeMatches.ProbeMatch->wsa__EndpointReference.__any = (char **)soap_malloc(soap, sizeof(char*)* MAX_128_LEN);
	ProbeMatches.ProbeMatch->wsa__EndpointReference.__anyAttribute = (char *)soap_malloc(soap, sizeof(char)* MAX_128_LEN);
	ProbeMatches.ProbeMatch->wsa__EndpointReference.Address = (char *)soap_malloc(soap, sizeof(char)* MAX_128_LEN);

	//strcpy(_HwId, "urn:uuid:5d645de6-bc78-11eb-a90f-000c29bf00a4");
	strcpy(_HwId, "urn:uuid:");
	_hw_onvif_get_uuid(_HwId + 9);

	memset(tmp, 0, sizeof(tmp));
	_hw_onvif_get_IPport(tmp, &port);
	if(port)
		sprintf(_IPAddr, "http://%s:%d/onvif/device_service", tmp, port);
	else
		sprintf(_IPAddr, "http://%s/onvif/device_service", tmp);

	ONVIF_DEBUG_MSG("[%d] _IPAddr ==== %s\n", __LINE__, _IPAddr);
 
	ProbeMatches.__sizeProbeMatch = 1;
	ProbeMatches.ProbeMatch->Scopes->__item = (char *)soap_malloc(soap, MAX_1024_LEN);
	memset(ProbeMatches.ProbeMatch->Scopes->__item, 0, 1024);
 
	//Scopes MUST BE
	//strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/type/NetworkVideoTransmitter");
 	strcat(ProbeMatches.ProbeMatch->Scopes->__item, "onvif://www.onvif.org/type/video_encoder \
	 	onvif://www.onvif.org/type/audio_encoder \
		onvif://www.onvif.org/type/NetworkVideoTransmitter \
		onvif://www.onvif.org/type/ptz \
		onvif://www.onvif.org/type/video_analytics \
		onvif://www.onvif.org/hardware/HD-IPCAM \
		onvif://www.onvif.org/location/country/china \
		onvif://www.onvif.org/location/city/bejing \
		onvif://www.onvif.org/name/IPCAM");

	ProbeMatches.ProbeMatch->Scopes->MatchBy = NULL;
	strcpy(ProbeMatches.ProbeMatch->XAddrs, _IPAddr);
	strcpy(ProbeMatches.ProbeMatch->Types, wsdd__Probe->Types);
	ONVIF_DEBUG_MSG("wsdd__Probe->Types=%s\n", wsdd__Probe->Types);
	ProbeMatches.ProbeMatch->MetadataVersion = 1;
 
	//ws-discovery规定 为可选项
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceProperties->__size = 0;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceProperties->__any = NULL;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceParameters->__size = 0;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ReferenceParameters->__any = NULL;
 
	ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType[0] = (char *)soap_malloc(soap, sizeof(char)* MAX_8_LEN);
	//ws-discovery规定 为可选项
	strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.PortType[0], "ttl");
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName->__item = NULL;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName->PortName = NULL;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.ServiceName->__anyAttribute = NULL;
	ProbeMatches.ProbeMatch->wsa__EndpointReference.__any[0] = (char *)soap_malloc(soap, sizeof(char)* MAX_8_LEN);
	strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.__any[0], "Any");
	strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.__anyAttribute, "Attribute");
	ProbeMatches.ProbeMatch->wsa__EndpointReference.__size = 0;
	strcpy(ProbeMatches.ProbeMatch->wsa__EndpointReference.Address, _HwId);
 
	soap->header->wsa__To = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous";
	soap->header->wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches";
	soap->header->wsa__RelatesTo = (struct wsa__Relationship*)soap_malloc(soap, sizeof(struct wsa__Relationship));
	soap->header->wsa__RelatesTo->__item = soap->header->wsa__MessageID;
	ONVIF_DEBUG_MSG("__item: %p, wsa__MessageID: %p: %s\n", soap->header->wsa__RelatesTo->__item, soap->header->wsa__MessageID, soap->header->wsa__MessageID);
	soap->header->wsa__RelatesTo->RelationshipType = NULL;
	soap->header->wsa__RelatesTo->__anyAttribute = NULL;
 
	soap->header->wsa__MessageID = (char *)soap_malloc(soap, sizeof(char)* MAX_128_LEN);
	strcpy(soap->header->wsa__MessageID, _HwId + 4);
 
	if (SOAP_OK == soap_send___wsdd__ProbeMatches(soap, "http://", NULL, &ProbeMatches))
	{
		ONVIF_DEBUG_MSG("send ProbeMatches success !\n");
		return SOAP_OK;
	}
 
	ONVIF_DEBUG_MSG("[%d] soap error: %d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
 
	return soap->error;
	return SOAP_OK;
}


/** Onvif server operation __wsdd__ProbeMatches */
int __wsdd__ProbeMatches(struct soap *soap, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __wsdd__Resolve */
int __wsdd__Resolve(struct soap *soap, struct wsdd__ResolveType *wsdd__Resolve)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __wsdd__ResolveMatches */
int __wsdd__ResolveMatches(struct soap *soap, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation SOAP_ENV__Fault_alex */
int SOAP_ENV__Fault_alex(struct soap *soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tad__GetServiceCapabilities */
int __tad__GetServiceCapabilities(struct soap *soap, struct _tad__GetServiceCapabilities *tad__GetServiceCapabilities, struct _tad__GetServiceCapabilitiesResponse *tad__GetServiceCapabilitiesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tad__DeleteAnalyticsEngineControl */
int __tad__DeleteAnalyticsEngineControl(struct soap *soap, struct _tad__DeleteAnalyticsEngineControl *tad__DeleteAnalyticsEngineControl, struct _tad__DeleteAnalyticsEngineControlResponse *tad__DeleteAnalyticsEngineControlResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tad__CreateAnalyticsEngineControl */
int __tad__CreateAnalyticsEngineControl(struct soap *soap, struct _tad__CreateAnalyticsEngineControl *tad__CreateAnalyticsEngineControl, struct _tad__CreateAnalyticsEngineControlResponse *tad__CreateAnalyticsEngineControlResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tad__SetAnalyticsEngineControl */
int __tad__SetAnalyticsEngineControl(struct soap *soap, struct _tad__SetAnalyticsEngineControl *tad__SetAnalyticsEngineControl, struct _tad__SetAnalyticsEngineControlResponse *tad__SetAnalyticsEngineControlResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tad__GetAnalyticsEngineControl */
int __tad__GetAnalyticsEngineControl(struct soap *soap, struct _tad__GetAnalyticsEngineControl *tad__GetAnalyticsEngineControl, struct _tad__GetAnalyticsEngineControlResponse *tad__GetAnalyticsEngineControlResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tad__GetAnalyticsEngineControls */
int __tad__GetAnalyticsEngineControls(struct soap *soap, struct _tad__GetAnalyticsEngineControls *tad__GetAnalyticsEngineControls, struct _tad__GetAnalyticsEngineControlsResponse *tad__GetAnalyticsEngineControlsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tad__GetAnalyticsEngine */
int __tad__GetAnalyticsEngine(struct soap *soap, struct _tad__GetAnalyticsEngine *tad__GetAnalyticsEngine, struct _tad__GetAnalyticsEngineResponse *tad__GetAnalyticsEngineResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tad__GetAnalyticsEngines */
int __tad__GetAnalyticsEngines(struct soap *soap, struct _tad__GetAnalyticsEngines *tad__GetAnalyticsEngines, struct _tad__GetAnalyticsEnginesResponse *tad__GetAnalyticsEnginesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tad__SetVideoAnalyticsConfiguration */
int __tad__SetVideoAnalyticsConfiguration(struct soap *soap, struct _tad__SetVideoAnalyticsConfiguration *tad__SetVideoAnalyticsConfiguration, struct _tad__SetVideoAnalyticsConfigurationResponse *tad__SetVideoAnalyticsConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tad__SetAnalyticsEngineInput */
int __tad__SetAnalyticsEngineInput(struct soap *soap, struct _tad__SetAnalyticsEngineInput *tad__SetAnalyticsEngineInput, struct _tad__SetAnalyticsEngineInputResponse *tad__SetAnalyticsEngineInputResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tad__GetAnalyticsEngineInput */
int __tad__GetAnalyticsEngineInput(struct soap *soap, struct _tad__GetAnalyticsEngineInput *tad__GetAnalyticsEngineInput, struct _tad__GetAnalyticsEngineInputResponse *tad__GetAnalyticsEngineInputResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tad__GetAnalyticsEngineInputs */
int __tad__GetAnalyticsEngineInputs(struct soap *soap, struct _tad__GetAnalyticsEngineInputs *tad__GetAnalyticsEngineInputs, struct _tad__GetAnalyticsEngineInputsResponse *tad__GetAnalyticsEngineInputsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tad__GetAnalyticsDeviceStreamUri */
int __tad__GetAnalyticsDeviceStreamUri(struct soap *soap, struct _tad__GetAnalyticsDeviceStreamUri *tad__GetAnalyticsDeviceStreamUri, struct _tad__GetAnalyticsDeviceStreamUriResponse *tad__GetAnalyticsDeviceStreamUriResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tad__GetVideoAnalyticsConfiguration */
int __tad__GetVideoAnalyticsConfiguration(struct soap *soap, struct _tad__GetVideoAnalyticsConfiguration *tad__GetVideoAnalyticsConfiguration, struct _tad__GetVideoAnalyticsConfigurationResponse *tad__GetVideoAnalyticsConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tad__CreateAnalyticsEngineInputs */
int __tad__CreateAnalyticsEngineInputs(struct soap *soap, struct _tad__CreateAnalyticsEngineInputs *tad__CreateAnalyticsEngineInputs, struct _tad__CreateAnalyticsEngineInputsResponse *tad__CreateAnalyticsEngineInputsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tad__DeleteAnalyticsEngineInputs */
int __tad__DeleteAnalyticsEngineInputs(struct soap *soap, struct _tad__DeleteAnalyticsEngineInputs *tad__DeleteAnalyticsEngineInputs, struct _tad__DeleteAnalyticsEngineInputsResponse *tad__DeleteAnalyticsEngineInputsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tad__GetAnalyticsState */
int __tad__GetAnalyticsState(struct soap *soap, struct _tad__GetAnalyticsState *tad__GetAnalyticsState, struct _tad__GetAnalyticsStateResponse *tad__GetAnalyticsStateResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tan__GetSupportedRules */
int __tan__GetSupportedRules(struct soap *soap, struct _tan__GetSupportedRules *tan__GetSupportedRules, struct _tan__GetSupportedRulesResponse *tan__GetSupportedRulesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tan__CreateRules */
int __tan__CreateRules(struct soap *soap, struct _tan__CreateRules *tan__CreateRules, struct _tan__CreateRulesResponse *tan__CreateRulesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tan__DeleteRules */
int __tan__DeleteRules(struct soap *soap, struct _tan__DeleteRules *tan__DeleteRules, struct _tan__DeleteRulesResponse *tan__DeleteRulesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tan__GetRules */
int __tan__GetRules(struct soap *soap, struct _tan__GetRules *tan__GetRules, struct _tan__GetRulesResponse *tan__GetRulesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tan__GetRuleOptions */
int __tan__GetRuleOptions(struct soap *soap, struct _tan__GetRuleOptions *tan__GetRuleOptions, struct _tan__GetRuleOptionsResponse *tan__GetRuleOptionsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tan__ModifyRules */
int __tan__ModifyRules(struct soap *soap, struct _tan__ModifyRules *tan__ModifyRules, struct _tan__ModifyRulesResponse *tan__ModifyRulesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tan__GetServiceCapabilities */
int __tan__GetServiceCapabilities(struct soap *soap, struct _tan__GetServiceCapabilities *tan__GetServiceCapabilities, struct _tan__GetServiceCapabilitiesResponse *tan__GetServiceCapabilitiesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tan__GetSupportedAnalyticsModules */
int __tan__GetSupportedAnalyticsModules(struct soap *soap, struct _tan__GetSupportedAnalyticsModules *tan__GetSupportedAnalyticsModules, struct _tan__GetSupportedAnalyticsModulesResponse *tan__GetSupportedAnalyticsModulesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tan__CreateAnalyticsModules */
int __tan__CreateAnalyticsModules(struct soap *soap, struct _tan__CreateAnalyticsModules *tan__CreateAnalyticsModules, struct _tan__CreateAnalyticsModulesResponse *tan__CreateAnalyticsModulesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tan__DeleteAnalyticsModules */
int __tan__DeleteAnalyticsModules(struct soap *soap, struct _tan__DeleteAnalyticsModules *tan__DeleteAnalyticsModules, struct _tan__DeleteAnalyticsModulesResponse *tan__DeleteAnalyticsModulesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tan__GetAnalyticsModules */
int __tan__GetAnalyticsModules(struct soap *soap, struct _tan__GetAnalyticsModules *tan__GetAnalyticsModules, struct _tan__GetAnalyticsModulesResponse *tan__GetAnalyticsModulesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tan__GetAnalyticsModuleOptions */
int __tan__GetAnalyticsModuleOptions(struct soap *soap, struct _tan__GetAnalyticsModuleOptions *tan__GetAnalyticsModuleOptions, struct _tan__GetAnalyticsModuleOptionsResponse *tan__GetAnalyticsModuleOptionsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tan__ModifyAnalyticsModules */
int __tan__ModifyAnalyticsModules(struct soap *soap, struct _tan__ModifyAnalyticsModules *tan__ModifyAnalyticsModules, struct _tan__ModifyAnalyticsModulesResponse *tan__ModifyAnalyticsModulesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tan__GetSupportedMetadata */
int __tan__GetSupportedMetadata(struct soap *soap, struct _tan__GetSupportedMetadata *tan__GetSupportedMetadata, struct _tan__GetSupportedMetadataResponse *tan__GetSupportedMetadataResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tdn__Hello */
int __tdn__Hello(struct soap *soap, struct wsdd__HelloType tdn__Hello, struct wsdd__ResolveType *tdn__HelloResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tdn__Bye */
int __tdn__Bye(struct soap *soap, struct wsdd__ByeType tdn__Bye, struct wsdd__ResolveType *tdn__ByeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tdn__Probe */
int __tdn__Probe(struct soap *soap, struct wsdd__ProbeType tdn__Probe, struct wsdd__ProbeMatchesType *tdn__ProbeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetServices */
int __tds__GetServices(struct soap *soap, struct _tds__GetServices *tds__GetServices, struct _tds__GetServicesResponse *tds__GetServicesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	int major, minor;
	_hw_onvif_get_service_version(&major, &minor);

	tds__GetServicesResponse->__sizeService = 1;

	CHECK_FIELD(tds__GetServicesResponse->Service)
	CHECK_FIELD(tds__GetServicesResponse->Service[0].Version)

	soap_set_field_string(soap, &tds__GetServicesResponse->Service[0].XAddr, _hw_onvif_get_service_url());
	soap_set_field_string(soap, &tds__GetServicesResponse->Service[0].Namespace, _hw_onvif_get_service_namespace());
	tds__GetServicesResponse->Service[0].Version->Major = major;
	tds__GetServicesResponse->Service[0].Version->Minor = minor;

	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetServiceCapabilities */
int __tds__GetServiceCapabilities(struct soap *soap, struct _tds__GetServiceCapabilities *tds__GetServiceCapabilities, struct _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetDeviceInformation */
int __tds__GetDeviceInformation(struct soap *soap, struct _tds__GetDeviceInformation *tds__GetDeviceInformation, struct _tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	soap_set_field_string(soap, &tds__GetDeviceInformationResponse->FirmwareVersion,
			_hw_onvif_get_firmware_version());
	soap_set_field_string(soap, &tds__GetDeviceInformationResponse->HardwareId,
			_hw_onvif_get_hardware_id());
	soap_set_field_string(soap, &tds__GetDeviceInformationResponse->Manufacturer,
			_hw_onvif_get_manufacturer());
	soap_set_field_string(soap, &tds__GetDeviceInformationResponse->Model,
			_hw_onvif_get_model());
	soap_set_field_string(soap, &tds__GetDeviceInformationResponse->SerialNumber,
			_hw_onvif_get_serial_number());
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetSystemDateAndTime */
int __tds__SetSystemDateAndTime(struct soap *soap, struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime, struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	struct tm now;
	LOAD(now.tm_year , tds__SetSystemDateAndTime->UTCDateTime->Date->Year);
	LOAD(now.tm_mon , tds__SetSystemDateAndTime->UTCDateTime->Date->Month);
	LOAD(now.tm_mday , tds__SetSystemDateAndTime->UTCDateTime->Date->Day);
	LOAD(now.tm_hour , tds__SetSystemDateAndTime->UTCDateTime->Time->Hour);
	LOAD(now.tm_min , tds__SetSystemDateAndTime->UTCDateTime->Time->Minute);
	LOAD(now.tm_sec , tds__SetSystemDateAndTime->UTCDateTime->Time->Second);
	LOAD(now.tm_zone , tds__SetSystemDateAndTime->TimeZone->TZ);
	LOAD(now.tm_isdst , tds__SetSystemDateAndTime->DaylightSavings);
	_hw_onvif_set_system_date_time(tds__SetSystemDateAndTime->DateTimeType, &now);
	return SOAP_OK;
}


/** Onvif server operation __tds__GetSystemDateAndTime */
int __tds__GetSystemDateAndTime(struct soap *soap, struct _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime, struct _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	/* Return incomplete response with default data values */
	struct tm now;
	int ntp;
	_hw_onvif_get_system_date_time(&ntp, &now);
	CHECK_FIELD(tds__GetSystemDateAndTimeResponse->SystemDateAndTime);
	CHECK_FIELD(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime);
	CHECK_FIELD(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date);
	CHECK_FIELD(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType = ntp;
	STORE(now.tm_year , tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Year);
	STORE(now.tm_mon , tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Month);
	STORE(now.tm_mday , tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Day);
	STORE(now.tm_hour ,tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Hour);
	STORE(now.tm_min , tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Minute);
	STORE(now.tm_sec , tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Second);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DaylightSavings = now.tm_isdst;
	if (now.tm_zone != NULL) {
		//tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ = soap_strdup(soap, now.tm_zone);
		soap_set_field_string(soap, &tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ, now.tm_zone);
	}
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetSystemFactoryDefault */
int __tds__SetSystemFactoryDefault(struct soap *soap, struct _tds__SetSystemFactoryDefault *tds__SetSystemFactoryDefault, struct _tds__SetSystemFactoryDefaultResponse *tds__SetSystemFactoryDefaultResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	_hw_onvif_set_system_factory_default(tds__SetSystemFactoryDefault->FactoryDefault);
	#ifdef WITH_NOEMPTYSTRUCT
	tds__SetSystemFactoryDefaultResponse->dummy = 0;
	#endif
	return SOAP_OK;
}


/** Onvif server operation __tds__UpgradeSystemFirmware */
int __tds__UpgradeSystemFirmware(struct soap *soap, struct _tds__UpgradeSystemFirmware *tds__UpgradeSystemFirmware, struct _tds__UpgradeSystemFirmwareResponse *tds__UpgradeSystemFirmwareResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	soap_set_field_string(soap, &tds__UpgradeSystemFirmwareResponse->Message, "Upgrade...");
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SystemReboot */
int __tds__SystemReboot(struct soap *soap, struct _tds__SystemReboot *tds__SystemReboot, struct _tds__SystemRebootResponse *tds__SystemRebootResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	soap_set_field_string(soap, &tds__SystemRebootResponse->Message, "Reboot...");
	_hw_onvif_reboot_system();
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__RestoreSystem */
int __tds__RestoreSystem(struct soap *soap, struct _tds__RestoreSystem *tds__RestoreSystem, struct _tds__RestoreSystemResponse *tds__RestoreSystemResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	#ifdef WITH_NOEMPTYSTRUCT
	tds__RestoreSystemResponse->dummy = 0;
	#endif
	return SOAP_OK;
}


/** Onvif server operation __tds__GetSystemBackup */
int __tds__GetSystemBackup(struct soap *soap, struct _tds__GetSystemBackup *tds__GetSystemBackup, struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	tds__GetSystemBackupResponse->__sizeBackupFiles = 1024;
	//tds__GetSystemBackupResponse->Name
	//tds__GetSystemBackupResponse->Data
	//tds__GetSystemBackupResponse->Data->xop__Include
	//tds__GetSystemBackupResponse->Data->xmime__contentType
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetSystemLog */
int __tds__GetSystemLog(struct soap *soap, struct _tds__GetSystemLog *tds__GetSystemLog, struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	/*tds__GetSystemLogResponse->SystemLog
	tds__GetSystemLogResponse->SystemLog->String
	tds__GetSystemLogResponse->SystemLog->Binary
	tds__GetSystemLogResponse->SystemLog->Binary->xop__Include;
    tds__GetSystemLogResponse->SystemLog->Binary->xmime__contentType;
	*/
	return SOAP_OK;
}


/** Onvif server operation __tds__GetSystemSupportInformation */
int __tds__GetSystemSupportInformation(struct soap *soap, struct _tds__GetSystemSupportInformation *tds__GetSystemSupportInformation, struct _tds__GetSystemSupportInformationResponse *tds__GetSystemSupportInformationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetScopes */
int __tds__GetScopes(struct soap *soap, struct _tds__GetScopes *tds__GetScopes, struct _tds__GetScopesResponse *tds__GetScopesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetScopes */
int __tds__SetScopes(struct soap *soap, struct _tds__SetScopes *tds__SetScopes, struct _tds__SetScopesResponse *tds__SetScopesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__AddScopes */
int __tds__AddScopes(struct soap *soap, struct _tds__AddScopes *tds__AddScopes, struct _tds__AddScopesResponse *tds__AddScopesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__RemoveScopes */
int __tds__RemoveScopes(struct soap *soap, struct _tds__RemoveScopes *tds__RemoveScopes, struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetDiscoveryMode */
int __tds__GetDiscoveryMode(struct soap *soap, struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode, struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetDiscoveryMode */
int __tds__SetDiscoveryMode(struct soap *soap, struct _tds__SetDiscoveryMode *tds__SetDiscoveryMode, struct _tds__SetDiscoveryModeResponse *tds__SetDiscoveryModeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetRemoteDiscoveryMode */
int __tds__GetRemoteDiscoveryMode(struct soap *soap, struct _tds__GetRemoteDiscoveryMode *tds__GetRemoteDiscoveryMode, struct _tds__GetRemoteDiscoveryModeResponse *tds__GetRemoteDiscoveryModeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetRemoteDiscoveryMode */
int __tds__SetRemoteDiscoveryMode(struct soap *soap, struct _tds__SetRemoteDiscoveryMode *tds__SetRemoteDiscoveryMode, struct _tds__SetRemoteDiscoveryModeResponse *tds__SetRemoteDiscoveryModeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetDPAddresses */
int __tds__GetDPAddresses(struct soap *soap, struct _tds__GetDPAddresses *tds__GetDPAddresses, struct _tds__GetDPAddressesResponse *tds__GetDPAddressesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetEndpointReference */
int __tds__GetEndpointReference(struct soap *soap, struct _tds__GetEndpointReference *tds__GetEndpointReference, struct _tds__GetEndpointReferenceResponse *tds__GetEndpointReferenceResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetRemoteUser */
int __tds__GetRemoteUser(struct soap *soap, struct _tds__GetRemoteUser *tds__GetRemoteUser, struct _tds__GetRemoteUserResponse *tds__GetRemoteUserResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetRemoteUser */
int __tds__SetRemoteUser(struct soap *soap, struct _tds__SetRemoteUser *tds__SetRemoteUser, struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetUsers */
int __tds__GetUsers(struct soap *soap, struct _tds__GetUsers *tds__GetUsers, struct _tds__GetUsersResponse *tds__GetUsersResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__CreateUsers */
int __tds__CreateUsers(struct soap *soap, struct _tds__CreateUsers *tds__CreateUsers, struct _tds__CreateUsersResponse *tds__CreateUsersResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__DeleteUsers */
int __tds__DeleteUsers(struct soap *soap, struct _tds__DeleteUsers *tds__DeleteUsers, struct _tds__DeleteUsersResponse *tds__DeleteUsersResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetUser */
int __tds__SetUser(struct soap *soap, struct _tds__SetUser *tds__SetUser, struct _tds__SetUserResponse *tds__SetUserResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetWsdlUrl */
int __tds__GetWsdlUrl(struct soap *soap, struct _tds__GetWsdlUrl *tds__GetWsdlUrl, struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	soap_set_field_string(soap,  &tds__GetWsdlUrlResponse->WsdlUrl, _hw_onvif_get_wsdl_url());
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetCapabilities */
int __tds__GetCapabilities(struct soap *soap, struct _tds__GetCapabilities *tds__GetCapabilities, struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	CHECK_FIELD(tds__GetCapabilitiesResponse->Capabilities)
	CHECK_FIELD(tds__GetCapabilitiesResponse->Capabilities->Media)
	CHECK_FIELD(tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities)
	CHECK_FIELD(tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast)
	CHECK_FIELD(tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP)
	CHECK_FIELD(tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP)
	CHECK_FIELD(tds__GetCapabilitiesResponse->Capabilities->Extension)
	CHECK_FIELD(tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO)
	CHECK_FIELD(tds__GetCapabilitiesResponse->Capabilities->Device)
	CHECK_FIELD(tds__GetCapabilitiesResponse->Capabilities->Device->System)
	CHECK_FIELD(tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions)

	soap_set_field_string(soap, &tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, _hw_onvif_get_ipv4_address());
	*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast = xsd__boolean__false_;
	*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP = xsd__boolean__true_;
	*tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = xsd__boolean__true_;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoSources = TRUE;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging =  xsd__boolean__true_;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup =  xsd__boolean__true_;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade =  xsd__boolean__true_;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Major = 2;
	tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Minor = 0;


	// 设备IO的一些支持
    tds__GetCapabilitiesResponse->Capabilities->Device->IO = (struct tt__IOCapabilities *)soap_malloc(soap, sizeof(struct tt__IOCapabilities));
    memset(tds__GetCapabilitiesResponse->Capabilities->Device->IO, 0, sizeof(struct tt__IOCapabilities));
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors = (int *)soap_malloc(soap, sizeof(int)); 
    *(tds__GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors) = 1; 
    tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs = (int *)soap_malloc(soap, sizeof(int)); 
    *(tds__GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs) = 1; 
    
    tds__GetCapabilitiesResponse->Capabilities->Device->Security = (struct tt__SecurityCapabilities *)soap_malloc(soap, sizeof(struct tt__SecurityCapabilities));
    memset(tds__GetCapabilitiesResponse->Capabilities->Device->Security, 0, sizeof(struct tt__SecurityCapabilities));
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e1          = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e2          = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig   = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->X_x002e509Token      = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken            = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken        = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Device->Security->RELToken             = xsd__boolean__false_;

	//Imaging的一些基本信息，关于视频颜色，IRCut的一些基本信息
    tds__GetCapabilitiesResponse->Capabilities->Imaging = (struct tt__ImagingCapabilities *)soap_malloc(soap,sizeof(struct tt__ImagingCapabilities));
    memset(tds__GetCapabilitiesResponse->Capabilities->Imaging, 0, sizeof(struct tt__ImagingCapabilities));
 
    tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr = (char *)soap_malloc(soap, sizeof(char) * MAX_64_LEN );
    memset(tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr, '\0', sizeof(char) * MAX_64_LEN);
    sprintf(tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr, "%s/onvif/imaging_service","192.168.12.135:8899");
 

    tds__GetCapabilitiesResponse->Capabilities->Media = (struct tt__MediaCapabilities *)soap_malloc(soap,sizeof(struct tt__MediaCapabilities));
    memset(tds__GetCapabilitiesResponse->Capabilities->Media, 0, sizeof(struct tt__MediaCapabilities));
 
    tds__GetCapabilitiesResponse->Capabilities->Media->XAddr = (char *)soap_malloc(soap, sizeof(char) * MAX_64_LEN );
    memset(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, 0, sizeof(char) * MAX_64_LEN);
    sprintf(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, "%s/onvif/media_service", "192.168.12.135:8899");
 
    tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities = (struct tt__RealTimeStreamingCapabilities *)soap_malloc(soap, 
            sizeof(struct tt__RealTimeStreamingCapabilities));
    memset(tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities, 0, sizeof(struct tt__RealTimeStreamingCapabilities));
    tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast   = (enum xsd__boolean *)soap_malloc(soap,sizeof(int));
    *tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast  = xsd__boolean__false_;
    tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP  = (enum xsd__boolean*)soap_malloc(soap,sizeof(int));
    *tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP = xsd__boolean__true_;
    tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = (enum xsd__boolean*)soap_malloc(soap,sizeof(int));
    *tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = xsd__boolean__true_;

	return SOAP_OK;
}


/** Onvif server operation __tds__SetDPAddresses */
int __tds__SetDPAddresses(struct soap *soap, struct _tds__SetDPAddresses *tds__SetDPAddresses, struct _tds__SetDPAddressesResponse *tds__SetDPAddressesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetHostname */
int __tds__GetHostname(struct soap *soap, struct _tds__GetHostname *tds__GetHostname, struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	CHECK_FIELD(tds__GetHostnameResponse->HostnameInformation);
	tds__GetHostnameResponse->HostnameInformation->FromDHCP = xsd__boolean__false_;
	tds__GetHostnameResponse->HostnameInformation->Name = strdup(host_name_get());
	#if 0
	tds__GetHostnameResponse->HostnameInformation;
	        /** Required element 'tt:FromDHCP' of XML schema type 'xsd:boolean' */
    tds__GetHostnameResponse->HostnameInformation->FromDHCP;
        /** Optional element 'tt:Name' of XML schema type 'xsd:token' */
    tds__GetHostnameResponse->HostnameInformation->Name;
        /** Optional element 'tt:Extension' of XML schema type 'tt:HostnameInformationExtension' */
    tds__GetHostnameResponse->HostnameInformation->Extension;
        /** XML DOM attribute list */
    tds__GetHostnameResponse->HostnameInformation->__anyAttribute;
	host_name_get();
	#endif
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetHostname */
int __tds__SetHostname(struct soap *soap, struct _tds__SetHostname *tds__SetHostname, struct _tds__SetHostnameResponse *tds__SetHostnameResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	host_config_set_api (API_SET_HOSTNAME_CMD, tds__SetHostname->Name);
	return SOAP_OK;
}


/** Onvif server operation __tds__SetHostnameFromDHCP */
int __tds__SetHostnameFromDHCP(struct soap *soap, struct _tds__SetHostnameFromDHCP *tds__SetHostnameFromDHCP, struct _tds__SetHostnameFromDHCPResponse *tds__SetHostnameFromDHCPResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	if(tds__SetHostnameFromDHCP->FromDHCP)
		;//host_config_set_api (API_SET_HOSTNAME_CMD, tds__SetHostnameFromDHCP->FromDHCP);
	tds__SetHostnameFromDHCPResponse->RebootNeeded = xsd__boolean__false_;
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetDNS */
int __tds__GetDNS(struct soap *soap, struct _tds__GetDNS *tds__GetDNS, struct _tds__GetDNSResponse *tds__GetDNSResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	CHECK_FIELD(tds__GetDNSResponse->DNSInformation);
	//CHECK_FIELD(tds__GetDNSResponse->DNSInformation->DNSFromDHCP);
	CHECK_FIELD(tds__GetDNSResponse->DNSInformation->DNSManual);
	tds__GetDNSResponse->DNSInformation->FromDHCP = xsd__boolean__false_;

	tds__GetDNSResponse->DNSInformation->__sizeSearchDomain = 0;
	tds__GetDNSResponse->DNSInformation->SearchDomain = NULL;

	tds__GetDNSResponse->DNSInformation->__sizeDNSFromDHCP = 0;
	tds__GetDNSResponse->DNSInformation->DNSFromDHCP = NULL;

	tds__GetDNSResponse->DNSInformation->__sizeDNSManual = 1;
	tds__GetDNSResponse->DNSInformation->DNSManual->Type = tt__IPType__IPv4;
	tds__GetDNSResponse->DNSInformation->DNSManual->IPv4Address = strdup("19.19.19.19");
	//tds__GetDNSResponse->DNSInformation->DNSManual->IPv6Address;

	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetDNS */
int __tds__SetDNS(struct soap *soap, struct _tds__SetDNS *tds__SetDNS, struct _tds__SetDNSResponse *tds__SetDNSResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
    //tds__SetDNS->FromDHCP;
    if(tds__SetDNS->__sizeSearchDomain)
		ONVIF_DEBUG_MSG("In function:%s SearchDomain=%s\n", __FUNCTION__, tds__SetDNS->SearchDomain);
    //tds__SetDNS->SearchDomain;

    //tds__SetDNS->__sizeDNSManual;
    if(tds__SetDNS->DNSManual)
		ONVIF_DEBUG_MSG("In function: %s DNSManual.IPv4Address=%s\n", __FUNCTION__, tds__SetDNS->DNSManual->IPv4Address);

	return SOAP_OK;
}


/** Onvif server operation __tds__GetNTP */
int __tds__GetNTP(struct soap *soap, struct _tds__GetNTP *tds__GetNTP, struct _tds__GetNTPResponse *tds__GetNTPResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	CHECK_FIELD(tds__GetNTPResponse->NTPInformation);
	CHECK_FIELD(tds__GetNTPResponse->NTPInformation->NTPManual);
	tds__GetNTPResponse->NTPInformation->FromDHCP = xsd__boolean__false_;
    tds__GetNTPResponse->NTPInformation->__sizeNTPFromDHCP = 0;
	tds__GetNTPResponse->NTPInformation->NTPFromDHCP = NULL;
    tds__GetNTPResponse->NTPInformation->__sizeNTPManual = 1;
    tds__GetNTPResponse->NTPInformation->NTPManual->Type = tt__NetworkHostType__IPv4;
	//tds__GetNTPResponse->NTPInformation->NTPManual->Type = tt__NetworkHostType__IPv6;
	//tds__GetNTPResponse->NTPInformation->NTPManual->Type = tt__NetworkHostType__DNS;

    tds__GetNTPResponse->NTPInformation->NTPManual->IPv4Address = strdup("19.19.19.19");
    //tds__GetNTPResponse->NTPInformation->NTPManual->IPv6Address;
    //tds__GetNTPResponse->NTPInformation->NTPManual->DNSname;
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetNTP */
int __tds__SetNTP(struct soap *soap, struct _tds__SetNTP *tds__SetNTP, struct _tds__SetNTPResponse *tds__SetNTPResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
    if(tds__SetNTP->FromDHCP)
	{

	}
	else
	{
		if(tds__SetNTP->__sizeNTPManual && tds__SetNTP->NTPManual)
		{
			//tds__SetNTP->NTPManual->Type = tt__NetworkHostType__IPv4;
			//tds__SetNTP->NTPManual->Type = tt__NetworkHostType__IPv6;
			//tds__SetNTP->NTPManual->Type = tt__NetworkHostType__DNS;
			//tds__SetNTP->NTPManual->IPv4Address = strdup("19.19.19.19");
			//tds__SetNTP->NTPManual->IPv6Address;
			//tds__SetNTP->NTPManual->DNSname;
			ONVIF_DEBUG_MSG("In function: %s tds__SetNTP->NTPManual->IPv4Address=%s\n", __FUNCTION__, tds__SetNTP->NTPManual->IPv4Address);
		}
	}
	return SOAP_OK;
}


/** Onvif server operation __tds__GetDynamicDNS */
int __tds__GetDynamicDNS(struct soap *soap, struct _tds__GetDynamicDNS *tds__GetDynamicDNS, struct _tds__GetDynamicDNSResponse *tds__GetDynamicDNSResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	CHECK_FIELD(tds__GetDynamicDNSResponse->DynamicDNSInformation);
	tds__GetDynamicDNSResponse->DynamicDNSInformation->Type = tt__DynamicDNSType__NoUpdate;
	tds__GetDynamicDNSResponse->DynamicDNSInformation->Type = tt__DynamicDNSType__ClientUpdates;
	tds__GetDynamicDNSResponse->DynamicDNSInformation->Type = tt__DynamicDNSType__ServerUpdates;
    tds__GetDynamicDNSResponse->DynamicDNSInformation->Name = strdup("www.baidu.com");
    tds__GetDynamicDNSResponse->DynamicDNSInformation->TTL = strdup("88");
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetDynamicDNS */
int __tds__SetDynamicDNS(struct soap *soap, struct _tds__SetDynamicDNS *tds__SetDynamicDNS, struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);

	ONVIF_DEBUG_MSG("In function: %s Type=%d\n", __FUNCTION__, tds__SetDynamicDNS->Type);
	ONVIF_DEBUG_MSG("In function: %s Name=%s\n", __FUNCTION__, tds__SetDynamicDNS->Name);
	ONVIF_DEBUG_MSG("In function: %s TTL=%s\n", __FUNCTION__, tds__SetDynamicDNS->TTL);

	return SOAP_OK;
}


/** Onvif server operation __tds__GetNetworkInterfaces */
int __tds__GetNetworkInterfaces(struct soap *soap, struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces, struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces = 1;
	MEM_CHECK_FIELD(tds__GetNetworkInterfacesResponse->NetworkInterfaces, tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces);	
    //tds__GetNetworkInterfacesResponse->NetworkInterfaces->token;
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->Enabled = xsd__boolean__true_;
    CHECK_FIELD(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info);
    CHECK_FIELD(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Link);
    CHECK_FIELD(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4);
    CHECK_FIELD(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6);

	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->Name = strdup("eth0");
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress = strdup("23:43:d5:56:4d:c4");
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->MTU = malloc(4);
	*tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->MTU = 1500;


	CHECK_FIELD(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Link->AdminSettings);
    CHECK_FIELD(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Link->OperSettings);
    /** Required element 'tt:InterfaceType' of XML schema type 'tt:IANA-IfTypes' */
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->Link->InterfaceType = 1;

	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Link->AdminSettings->AutoNegotiation = xsd__boolean__true_;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Link->AdminSettings->Speed = 1000;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Link->AdminSettings->Duplex = tt__Duplex__Full;


	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Enabled = xsd__boolean__true_;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Enabled = xsd__boolean__false_;
	CHECK_FIELD(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config);
	CHECK_FIELD(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config);


	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->__sizeManual = 1;
	CHECK_FIELD(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual);
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address = strdup("1.1.1.1");
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->PrefixLength = 24;
        /** Optional element 'tt:LinkLocal' of XML schema type 'tt:PrefixedIPv4Address' */
	//tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->LinkLocal;
        /** Optional element 'tt:FromDHCP' of XML schema type 'tt:PrefixedIPv4Address' */
	//tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->FromDHCP;
        /** Required element 'tt:DHCP' of XML schema type 'xsd:boolean' */
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->DHCP = xsd__boolean__false_;
        /** Required element 'tt:Address' of XML schema type 'tt:IPv4Address' */


	//tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->AcceptRouterAdvert;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->DHCP = tt__IPv6DHCPConfiguration__Off;
	/*
	tt__IPv6DHCPConfiguration__Auto = 0,
	tt__IPv6DHCPConfiguration__Stateful = 1,
	tt__IPv6DHCPConfiguration__Stateless = 2,
	tt__IPv6DHCPConfiguration__Off = 3
	*/
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->__sizeManual = 1;
	CHECK_FIELD(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->Manual);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->Manual->Address = strdup("fe80::1e3c:3f0e:4a35:d7ca");
    tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->Manual->PrefixLength = 64;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->__sizeLinkLocal = 0;
	//tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->LinkLocal;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->__sizeFromDHCP = 0;
	//tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->FromDHCP;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->__sizeFromRA = 0;
	//tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->FromRA;
	//tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv6->Config->Extension;
	return SOAP_OK;
}


/** Onvif server operation __tds__SetNetworkInterfaces */
int __tds__SetNetworkInterfaces(struct soap *soap, struct _tds__SetNetworkInterfaces *tds__SetNetworkInterfaces, struct _tds__SetNetworkInterfacesResponse *tds__SetNetworkInterfacesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);

	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetNetworkProtocols */
int __tds__GetNetworkProtocols(struct soap *soap, struct _tds__GetNetworkProtocols *tds__GetNetworkProtocols, struct _tds__GetNetworkProtocolsResponse *tds__GetNetworkProtocolsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols = 1;
	CHECK_FIELD(tds__GetNetworkProtocolsResponse->NetworkProtocols);
	//tt__NetworkProtocolType__HTTP = 0,
	//tt__NetworkProtocolType__HTTPS = 1,
	//tt__NetworkProtocolType__RTSP = 2

    tds__GetNetworkProtocolsResponse->NetworkProtocols->Name = tt__NetworkProtocolType__HTTP;
        /** Required element 'tt:Enabled' of XML schema type 'xsd:boolean' */
    tds__GetNetworkProtocolsResponse->NetworkProtocols->Enabled = xsd__boolean__false_;
        /** Sequence of at least 1 elements 'tt:Port' of XML schema type 'xsd:int' stored in dynamic array Port of length __sizePort */
    tds__GetNetworkProtocolsResponse->NetworkProtocols->__sizePort = 1;
    tds__GetNetworkProtocolsResponse->NetworkProtocols->Port = malloc(4);
	*tds__GetNetworkProtocolsResponse->NetworkProtocols->Port = 8080;
        /** Optional element 'tt:Extension' of XML schema type 'tt:NetworkProtocolExtension' */
    //tds__GetNetworkProtocolsResponse->NetworkProtocols->Extension;
	return SOAP_OK;
}


/** Onvif server operation __tds__SetNetworkProtocols */
int __tds__SetNetworkProtocols(struct soap *soap, struct _tds__SetNetworkProtocols *tds__SetNetworkProtocols, struct _tds__SetNetworkProtocolsResponse *tds__SetNetworkProtocolsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
    if(tds__SetNetworkProtocols && tds__SetNetworkProtocols->__sizeNetworkProtocols)
	{
		if(tds__SetNetworkProtocols->NetworkProtocols->Enabled == xsd__boolean__true_ && 
			tds__SetNetworkProtocols->NetworkProtocols->__sizePort )
		{
			int i = 0; 
			for(i = 0; i < tds__SetNetworkProtocols->NetworkProtocols->__sizePort; i++)
			{
				if(tds__SetNetworkProtocols->NetworkProtocols->Name == tt__NetworkProtocolType__HTTP)
					ONVIF_DEBUG_MSG("In function: %s HTTP %d\n", __FUNCTION__, tds__SetNetworkProtocols->NetworkProtocols->Port[i]);
				if(tds__SetNetworkProtocols->NetworkProtocols->Name == tt__NetworkProtocolType__HTTPS)
					ONVIF_DEBUG_MSG("In function: %s HTTPS %d\n", __FUNCTION__, tds__SetNetworkProtocols->NetworkProtocols->Port[i]);
				if(tds__SetNetworkProtocols->NetworkProtocols->Name == tt__NetworkProtocolType__RTSP)
					ONVIF_DEBUG_MSG("In function: %s RTSP %d\n", __FUNCTION__, tds__SetNetworkProtocols->NetworkProtocols->Port[i]);
			}
		}
	}
	return SOAP_OK;
}


/** Onvif server operation __tds__GetNetworkDefaultGateway */
int __tds__GetNetworkDefaultGateway(struct soap *soap, struct _tds__GetNetworkDefaultGateway *tds__GetNetworkDefaultGateway, struct _tds__GetNetworkDefaultGatewayResponse *tds__GetNetworkDefaultGatewayResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetNetworkDefaultGateway */
int __tds__SetNetworkDefaultGateway(struct soap *soap, struct _tds__SetNetworkDefaultGateway *tds__SetNetworkDefaultGateway, struct _tds__SetNetworkDefaultGatewayResponse *tds__SetNetworkDefaultGatewayResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetZeroConfiguration */
int __tds__GetZeroConfiguration(struct soap *soap, struct _tds__GetZeroConfiguration *tds__GetZeroConfiguration, struct _tds__GetZeroConfigurationResponse *tds__GetZeroConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetZeroConfiguration */
int __tds__SetZeroConfiguration(struct soap *soap, struct _tds__SetZeroConfiguration *tds__SetZeroConfiguration, struct _tds__SetZeroConfigurationResponse *tds__SetZeroConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetIPAddressFilter */
int __tds__GetIPAddressFilter(struct soap *soap, struct _tds__GetIPAddressFilter *tds__GetIPAddressFilter, struct _tds__GetIPAddressFilterResponse *tds__GetIPAddressFilterResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetIPAddressFilter */
int __tds__SetIPAddressFilter(struct soap *soap, struct _tds__SetIPAddressFilter *tds__SetIPAddressFilter, struct _tds__SetIPAddressFilterResponse *tds__SetIPAddressFilterResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__AddIPAddressFilter */
int __tds__AddIPAddressFilter(struct soap *soap, struct _tds__AddIPAddressFilter *tds__AddIPAddressFilter, struct _tds__AddIPAddressFilterResponse *tds__AddIPAddressFilterResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__RemoveIPAddressFilter */
int __tds__RemoveIPAddressFilter(struct soap *soap, struct _tds__RemoveIPAddressFilter *tds__RemoveIPAddressFilter, struct _tds__RemoveIPAddressFilterResponse *tds__RemoveIPAddressFilterResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetAccessPolicy */
int __tds__GetAccessPolicy(struct soap *soap, struct _tds__GetAccessPolicy *tds__GetAccessPolicy, struct _tds__GetAccessPolicyResponse *tds__GetAccessPolicyResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetAccessPolicy */
int __tds__SetAccessPolicy(struct soap *soap, struct _tds__SetAccessPolicy *tds__SetAccessPolicy, struct _tds__SetAccessPolicyResponse *tds__SetAccessPolicyResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__CreateCertificate */
int __tds__CreateCertificate(struct soap *soap, struct _tds__CreateCertificate *tds__CreateCertificate, struct _tds__CreateCertificateResponse *tds__CreateCertificateResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetCertificates */
int __tds__GetCertificates(struct soap *soap, struct _tds__GetCertificates *tds__GetCertificates, struct _tds__GetCertificatesResponse *tds__GetCertificatesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetCertificatesStatus */
int __tds__GetCertificatesStatus(struct soap *soap, struct _tds__GetCertificatesStatus *tds__GetCertificatesStatus, struct _tds__GetCertificatesStatusResponse *tds__GetCertificatesStatusResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetCertificatesStatus */
int __tds__SetCertificatesStatus(struct soap *soap, struct _tds__SetCertificatesStatus *tds__SetCertificatesStatus, struct _tds__SetCertificatesStatusResponse *tds__SetCertificatesStatusResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__DeleteCertificates */
int __tds__DeleteCertificates(struct soap *soap, struct _tds__DeleteCertificates *tds__DeleteCertificates, struct _tds__DeleteCertificatesResponse *tds__DeleteCertificatesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetPkcs10Request */
int __tds__GetPkcs10Request(struct soap *soap, struct _tds__GetPkcs10Request *tds__GetPkcs10Request, struct _tds__GetPkcs10RequestResponse *tds__GetPkcs10RequestResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__LoadCertificates */
int __tds__LoadCertificates(struct soap *soap, struct _tds__LoadCertificates *tds__LoadCertificates, struct _tds__LoadCertificatesResponse *tds__LoadCertificatesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetClientCertificateMode */
int __tds__GetClientCertificateMode(struct soap *soap, struct _tds__GetClientCertificateMode *tds__GetClientCertificateMode, struct _tds__GetClientCertificateModeResponse *tds__GetClientCertificateModeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetClientCertificateMode */
int __tds__SetClientCertificateMode(struct soap *soap, struct _tds__SetClientCertificateMode *tds__SetClientCertificateMode, struct _tds__SetClientCertificateModeResponse *tds__SetClientCertificateModeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetRelayOutputs */
int __tds__GetRelayOutputs(struct soap *soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetRelayOutputSettings */
int __tds__SetRelayOutputSettings(struct soap *soap, struct _tds__SetRelayOutputSettings *tds__SetRelayOutputSettings, struct _tds__SetRelayOutputSettingsResponse *tds__SetRelayOutputSettingsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetRelayOutputState */
int __tds__SetRelayOutputState(struct soap *soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__SendAuxiliaryCommand */
int __tds__SendAuxiliaryCommand(struct soap *soap, struct _tds__SendAuxiliaryCommand *tds__SendAuxiliaryCommand, struct _tds__SendAuxiliaryCommandResponse *tds__SendAuxiliaryCommandResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetCACertificates */
int __tds__GetCACertificates(struct soap *soap, struct _tds__GetCACertificates *tds__GetCACertificates, struct _tds__GetCACertificatesResponse *tds__GetCACertificatesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__LoadCertificateWithPrivateKey */
int __tds__LoadCertificateWithPrivateKey(struct soap *soap, struct _tds__LoadCertificateWithPrivateKey *tds__LoadCertificateWithPrivateKey, struct _tds__LoadCertificateWithPrivateKeyResponse *tds__LoadCertificateWithPrivateKeyResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetCertificateInformation */
int __tds__GetCertificateInformation(struct soap *soap, struct _tds__GetCertificateInformation *tds__GetCertificateInformation, struct _tds__GetCertificateInformationResponse *tds__GetCertificateInformationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__LoadCACertificates */
int __tds__LoadCACertificates(struct soap *soap, struct _tds__LoadCACertificates *tds__LoadCACertificates, struct _tds__LoadCACertificatesResponse *tds__LoadCACertificatesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__CreateDot1XConfiguration */
int __tds__CreateDot1XConfiguration(struct soap *soap, struct _tds__CreateDot1XConfiguration *tds__CreateDot1XConfiguration, struct _tds__CreateDot1XConfigurationResponse *tds__CreateDot1XConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetDot1XConfiguration */
int __tds__SetDot1XConfiguration(struct soap *soap, struct _tds__SetDot1XConfiguration *tds__SetDot1XConfiguration, struct _tds__SetDot1XConfigurationResponse *tds__SetDot1XConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetDot1XConfiguration */
int __tds__GetDot1XConfiguration(struct soap *soap, struct _tds__GetDot1XConfiguration *tds__GetDot1XConfiguration, struct _tds__GetDot1XConfigurationResponse *tds__GetDot1XConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetDot1XConfigurations */
int __tds__GetDot1XConfigurations(struct soap *soap, struct _tds__GetDot1XConfigurations *tds__GetDot1XConfigurations, struct _tds__GetDot1XConfigurationsResponse *tds__GetDot1XConfigurationsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__DeleteDot1XConfiguration */
int __tds__DeleteDot1XConfiguration(struct soap *soap, struct _tds__DeleteDot1XConfiguration *tds__DeleteDot1XConfiguration, struct _tds__DeleteDot1XConfigurationResponse *tds__DeleteDot1XConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetDot11Capabilities */
int __tds__GetDot11Capabilities(struct soap *soap, struct _tds__GetDot11Capabilities *tds__GetDot11Capabilities, struct _tds__GetDot11CapabilitiesResponse *tds__GetDot11CapabilitiesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetDot11Status */
int __tds__GetDot11Status(struct soap *soap, struct _tds__GetDot11Status *tds__GetDot11Status, struct _tds__GetDot11StatusResponse *tds__GetDot11StatusResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__ScanAvailableDot11Networks */
int __tds__ScanAvailableDot11Networks(struct soap *soap, struct _tds__ScanAvailableDot11Networks *tds__ScanAvailableDot11Networks, struct _tds__ScanAvailableDot11NetworksResponse *tds__ScanAvailableDot11NetworksResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetSystemUris */
int __tds__GetSystemUris(struct soap *soap, struct _tds__GetSystemUris *tds__GetSystemUris, struct _tds__GetSystemUrisResponse *tds__GetSystemUrisResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__StartFirmwareUpgrade */
int __tds__StartFirmwareUpgrade(struct soap *soap, struct _tds__StartFirmwareUpgrade *tds__StartFirmwareUpgrade, struct _tds__StartFirmwareUpgradeResponse *tds__StartFirmwareUpgradeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__StartSystemRestore */
int __tds__StartSystemRestore(struct soap *soap, struct _tds__StartSystemRestore *tds__StartSystemRestore, struct _tds__StartSystemRestoreResponse *tds__StartSystemRestoreResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetStorageConfigurations */
int __tds__GetStorageConfigurations(struct soap *soap, struct _tds__GetStorageConfigurations *tds__GetStorageConfigurations, struct _tds__GetStorageConfigurationsResponse *tds__GetStorageConfigurationsResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__CreateStorageConfiguration */
int __tds__CreateStorageConfiguration(struct soap *soap, struct _tds__CreateStorageConfiguration *tds__CreateStorageConfiguration, struct _tds__CreateStorageConfigurationResponse *tds__CreateStorageConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetStorageConfiguration */
int __tds__GetStorageConfiguration(struct soap *soap, struct _tds__GetStorageConfiguration *tds__GetStorageConfiguration, struct _tds__GetStorageConfigurationResponse *tds__GetStorageConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetStorageConfiguration */
int __tds__SetStorageConfiguration(struct soap *soap, struct _tds__SetStorageConfiguration *tds__SetStorageConfiguration, struct _tds__SetStorageConfigurationResponse *tds__SetStorageConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__DeleteStorageConfiguration */
int __tds__DeleteStorageConfiguration(struct soap *soap, struct _tds__DeleteStorageConfiguration *tds__DeleteStorageConfiguration, struct _tds__DeleteStorageConfigurationResponse *tds__DeleteStorageConfigurationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__GetGeoLocation */
int __tds__GetGeoLocation(struct soap *soap, struct _tds__GetGeoLocation *tds__GetGeoLocation, struct _tds__GetGeoLocationResponse *tds__GetGeoLocationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tds__SetGeoLocation */
int __tds__SetGeoLocation(struct soap *soap, struct _tds__SetGeoLocation *tds__SetGeoLocation, struct _tds__SetGeoLocationResponse *tds__SetGeoLocationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tds__DeleteGeoLocation */
int __tds__DeleteGeoLocation(struct soap *soap, struct _tds__DeleteGeoLocation *tds__DeleteGeoLocation, struct _tds__DeleteGeoLocationResponse *tds__DeleteGeoLocationResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tev__PullMessages */
int __tev__PullMessages(struct soap *soap, struct _tev__PullMessages *tev__PullMessages, struct _tev__PullMessagesResponse *tev__PullMessagesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__Seek */
int __tev__Seek(struct soap *soap, struct _tev__Seek *tev__Seek, struct _tev__SeekResponse *tev__SeekResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tev__SetSynchronizationPoint */
int __tev__SetSynchronizationPoint(struct soap *soap, struct _tev__SetSynchronizationPoint *tev__SetSynchronizationPoint, struct _tev__SetSynchronizationPointResponse *tev__SetSynchronizationPointResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tev__Unsubscribe */
int __tev__Unsubscribe(struct soap *soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__GetServiceCapabilities */
int __tev__GetServiceCapabilities(struct soap *soap, struct _tev__GetServiceCapabilities *tev__GetServiceCapabilities, struct _tev__GetServiceCapabilitiesResponse *tev__GetServiceCapabilitiesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__CreatePullPointSubscription */
int __tev__CreatePullPointSubscription(struct soap *soap, struct _tev__CreatePullPointSubscription *tev__CreatePullPointSubscription, struct _tev__CreatePullPointSubscriptionResponse *tev__CreatePullPointSubscriptionResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__GetEventProperties */
int __tev__GetEventProperties(struct soap *soap, struct _tev__GetEventProperties *tev__GetEventProperties, struct _tev__GetEventPropertiesResponse *tev__GetEventPropertiesResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__AddEventBroker */
int __tev__AddEventBroker(struct soap *soap, struct _tev__AddEventBroker *tev__AddEventBroker, struct _tev__AddEventBrokerResponse *tev__AddEventBrokerResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tev__DeleteEventBroker */
int __tev__DeleteEventBroker(struct soap *soap, struct _tev__DeleteEventBroker *tev__DeleteEventBroker, struct _tev__DeleteEventBrokerResponse *tev__DeleteEventBrokerResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tev__GetEventBrokers */
int __tev__GetEventBrokers(struct soap *soap, struct _tev__GetEventBrokers *tev__GetEventBrokers, struct _tev__GetEventBrokersResponse *tev__GetEventBrokersResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__Renew */
int __tev__Renew(struct soap *soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__Unsubscribe_ */
int __tev__Unsubscribe_(struct soap *soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__Subscribe */
int __tev__Subscribe(struct soap *soap, struct _wsnt__Subscribe *wsnt__Subscribe, struct _wsnt__SubscribeResponse *wsnt__SubscribeResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__GetCurrentMessage */
int __tev__GetCurrentMessage(struct soap *soap, struct _wsnt__GetCurrentMessage *wsnt__GetCurrentMessage, struct _wsnt__GetCurrentMessageResponse *wsnt__GetCurrentMessageResponse)
{
	ONVIF_DEBUG_MSG("In function: %s\n", __FUNCTION__);
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__Notify */
int __tev__Notify(struct soap *soap, struct _wsnt__Notify *wsnt__Notify)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tev__GetMessages */
int __tev__GetMessages(struct soap *soap, struct _wsnt__GetMessages *wsnt__GetMessages, struct _wsnt__GetMessagesResponse *wsnt__GetMessagesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__DestroyPullPoint */
int __tev__DestroyPullPoint(struct soap *soap, struct _wsnt__DestroyPullPoint *wsnt__DestroyPullPoint, struct _wsnt__DestroyPullPointResponse *wsnt__DestroyPullPointResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__Notify_ */
int __tev__Notify_(struct soap *soap, struct _wsnt__Notify *wsnt__Notify)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tev__CreatePullPoint */
int __tev__CreatePullPoint(struct soap *soap, struct _wsnt__CreatePullPoint *wsnt__CreatePullPoint, struct _wsnt__CreatePullPointResponse *wsnt__CreatePullPointResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__Renew_ */
int __tev__Renew_(struct soap *soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__Unsubscribe__ */
int __tev__Unsubscribe__(struct soap *soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__PauseSubscription */
int __tev__PauseSubscription(struct soap *soap, struct _wsnt__PauseSubscription *wsnt__PauseSubscription, struct _wsnt__PauseSubscriptionResponse *wsnt__PauseSubscriptionResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tev__ResumeSubscription */
int __tev__ResumeSubscription(struct soap *soap, struct _wsnt__ResumeSubscription *wsnt__ResumeSubscription, struct _wsnt__ResumeSubscriptionResponse *wsnt__ResumeSubscriptionResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __timg__GetServiceCapabilities */
int __timg__GetServiceCapabilities(struct soap *soap, struct _timg__GetServiceCapabilities *timg__GetServiceCapabilities, struct _timg__GetServiceCapabilitiesResponse *timg__GetServiceCapabilitiesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __timg__GetImagingSettings */
int __timg__GetImagingSettings(struct soap *soap, struct _timg__GetImagingSettings *timg__GetImagingSettings, struct _timg__GetImagingSettingsResponse *timg__GetImagingSettingsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __timg__SetImagingSettings */
int __timg__SetImagingSettings(struct soap *soap, struct _timg__SetImagingSettings *timg__SetImagingSettings, struct _timg__SetImagingSettingsResponse *timg__SetImagingSettingsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __timg__GetOptions */
int __timg__GetOptions(struct soap *soap, struct _timg__GetOptions *timg__GetOptions, struct _timg__GetOptionsResponse *timg__GetOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __timg__Move */
int __timg__Move(struct soap *soap, struct _timg__Move *timg__Move, struct _timg__MoveResponse *timg__MoveResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __timg__Stop */
int __timg__Stop(struct soap *soap, struct _timg__Stop *timg__Stop, struct _timg__StopResponse *timg__StopResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __timg__GetStatus */
int __timg__GetStatus(struct soap *soap, struct _timg__GetStatus *timg__GetStatus, struct _timg__GetStatusResponse *timg__GetStatusResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __timg__GetMoveOptions */
int __timg__GetMoveOptions(struct soap *soap, struct _timg__GetMoveOptions *timg__GetMoveOptions, struct _timg__GetMoveOptionsResponse *timg__GetMoveOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __timg__GetPresets */
int __timg__GetPresets(struct soap *soap, struct _timg__GetPresets *timg__GetPresets, struct _timg__GetPresetsResponse *timg__GetPresetsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __timg__GetCurrentPreset */
int __timg__GetCurrentPreset(struct soap *soap, struct _timg__GetCurrentPreset *timg__GetCurrentPreset, struct _timg__GetCurrentPresetResponse *timg__GetCurrentPresetResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __timg__SetCurrentPreset */
int __timg__SetCurrentPreset(struct soap *soap, struct _timg__SetCurrentPreset *timg__SetCurrentPreset, struct _timg__SetCurrentPresetResponse *timg__SetCurrentPresetResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tls__GetServiceCapabilities */
int __tls__GetServiceCapabilities(struct soap *soap, struct _tls__GetServiceCapabilities *tls__GetServiceCapabilities, struct _tls__GetServiceCapabilitiesResponse *tls__GetServiceCapabilitiesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tls__GetLayout */
int __tls__GetLayout(struct soap *soap, struct _tls__GetLayout *tls__GetLayout, struct _tls__GetLayoutResponse *tls__GetLayoutResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tls__SetLayout */
int __tls__SetLayout(struct soap *soap, struct _tls__SetLayout *tls__SetLayout, struct _tls__SetLayoutResponse *tls__SetLayoutResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tls__GetDisplayOptions */
int __tls__GetDisplayOptions(struct soap *soap, struct _tls__GetDisplayOptions *tls__GetDisplayOptions, struct _tls__GetDisplayOptionsResponse *tls__GetDisplayOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tls__GetPaneConfigurations */
int __tls__GetPaneConfigurations(struct soap *soap, struct _tls__GetPaneConfigurations *tls__GetPaneConfigurations, struct _tls__GetPaneConfigurationsResponse *tls__GetPaneConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tls__GetPaneConfiguration */
int __tls__GetPaneConfiguration(struct soap *soap, struct _tls__GetPaneConfiguration *tls__GetPaneConfiguration, struct _tls__GetPaneConfigurationResponse *tls__GetPaneConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tls__SetPaneConfigurations */
int __tls__SetPaneConfigurations(struct soap *soap, struct _tls__SetPaneConfigurations *tls__SetPaneConfigurations, struct _tls__SetPaneConfigurationsResponse *tls__SetPaneConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tls__SetPaneConfiguration */
int __tls__SetPaneConfiguration(struct soap *soap, struct _tls__SetPaneConfiguration *tls__SetPaneConfiguration, struct _tls__SetPaneConfigurationResponse *tls__SetPaneConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tls__CreatePaneConfiguration */
int __tls__CreatePaneConfiguration(struct soap *soap, struct _tls__CreatePaneConfiguration *tls__CreatePaneConfiguration, struct _tls__CreatePaneConfigurationResponse *tls__CreatePaneConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tls__DeletePaneConfiguration */
int __tls__DeletePaneConfiguration(struct soap *soap, struct _tls__DeletePaneConfiguration *tls__DeletePaneConfiguration, struct _tls__DeletePaneConfigurationResponse *tls__DeletePaneConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetServiceCapabilities */
int __tmd__GetServiceCapabilities(struct soap *soap, struct _tmd__GetServiceCapabilities *tmd__GetServiceCapabilities, struct _tmd__GetServiceCapabilitiesResponse *tmd__GetServiceCapabilitiesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetRelayOutputOptions */
int __tmd__GetRelayOutputOptions(struct soap *soap, struct _tmd__GetRelayOutputOptions *tmd__GetRelayOutputOptions, struct _tmd__GetRelayOutputOptionsResponse *tmd__GetRelayOutputOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetAudioSources */
int __tmd__GetAudioSources(struct soap *soap, struct tmd__Get *tmd__GetAudioSources, struct tmd__GetResponse *tmd__GetAudioSourcesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetAudioOutputs */
int __tmd__GetAudioOutputs(struct soap *soap, struct tmd__Get *tmd__GetAudioOutputs, struct tmd__GetResponse *tmd__GetAudioOutputsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetVideoSources */
int __tmd__GetVideoSources(struct soap *soap, struct tmd__Get *tmd__GetVideoSources, struct tmd__GetResponse *tmd__GetVideoSourcesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetVideoOutputs */
int __tmd__GetVideoOutputs(struct soap *soap, struct _tmd__GetVideoOutputs *tmd__GetVideoOutputs, struct _tmd__GetVideoOutputsResponse *tmd__GetVideoOutputsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetVideoSourceConfiguration */
int __tmd__GetVideoSourceConfiguration(struct soap *soap, struct _tmd__GetVideoSourceConfiguration *tmd__GetVideoSourceConfiguration, struct _tmd__GetVideoSourceConfigurationResponse *tmd__GetVideoSourceConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetVideoOutputConfiguration */
int __tmd__GetVideoOutputConfiguration(struct soap *soap, struct _tmd__GetVideoOutputConfiguration *tmd__GetVideoOutputConfiguration, struct _tmd__GetVideoOutputConfigurationResponse *tmd__GetVideoOutputConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetAudioSourceConfiguration */
int __tmd__GetAudioSourceConfiguration(struct soap *soap, struct _tmd__GetAudioSourceConfiguration *tmd__GetAudioSourceConfiguration, struct _tmd__GetAudioSourceConfigurationResponse *tmd__GetAudioSourceConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetAudioOutputConfiguration */
int __tmd__GetAudioOutputConfiguration(struct soap *soap, struct _tmd__GetAudioOutputConfiguration *tmd__GetAudioOutputConfiguration, struct _tmd__GetAudioOutputConfigurationResponse *tmd__GetAudioOutputConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__SetVideoSourceConfiguration */
int __tmd__SetVideoSourceConfiguration(struct soap *soap, struct _tmd__SetVideoSourceConfiguration *tmd__SetVideoSourceConfiguration, struct _tmd__SetVideoSourceConfigurationResponse *tmd__SetVideoSourceConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__SetVideoOutputConfiguration */
int __tmd__SetVideoOutputConfiguration(struct soap *soap, struct _tmd__SetVideoOutputConfiguration *tmd__SetVideoOutputConfiguration, struct _tmd__SetVideoOutputConfigurationResponse *tmd__SetVideoOutputConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__SetAudioSourceConfiguration */
int __tmd__SetAudioSourceConfiguration(struct soap *soap, struct _tmd__SetAudioSourceConfiguration *tmd__SetAudioSourceConfiguration, struct _tmd__SetAudioSourceConfigurationResponse *tmd__SetAudioSourceConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__SetAudioOutputConfiguration */
int __tmd__SetAudioOutputConfiguration(struct soap *soap, struct _tmd__SetAudioOutputConfiguration *tmd__SetAudioOutputConfiguration, struct _tmd__SetAudioOutputConfigurationResponse *tmd__SetAudioOutputConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetVideoSourceConfigurationOptions */
int __tmd__GetVideoSourceConfigurationOptions(struct soap *soap, struct _tmd__GetVideoSourceConfigurationOptions *tmd__GetVideoSourceConfigurationOptions, struct _tmd__GetVideoSourceConfigurationOptionsResponse *tmd__GetVideoSourceConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetVideoOutputConfigurationOptions */
int __tmd__GetVideoOutputConfigurationOptions(struct soap *soap, struct _tmd__GetVideoOutputConfigurationOptions *tmd__GetVideoOutputConfigurationOptions, struct _tmd__GetVideoOutputConfigurationOptionsResponse *tmd__GetVideoOutputConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetAudioSourceConfigurationOptions */
int __tmd__GetAudioSourceConfigurationOptions(struct soap *soap, struct _tmd__GetAudioSourceConfigurationOptions *tmd__GetAudioSourceConfigurationOptions, struct _tmd__GetAudioSourceConfigurationOptionsResponse *tmd__GetAudioSourceConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetAudioOutputConfigurationOptions */
int __tmd__GetAudioOutputConfigurationOptions(struct soap *soap, struct _tmd__GetAudioOutputConfigurationOptions *tmd__GetAudioOutputConfigurationOptions, struct _tmd__GetAudioOutputConfigurationOptionsResponse *tmd__GetAudioOutputConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetRelayOutputs */
int __tmd__GetRelayOutputs(struct soap *soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__SetRelayOutputSettings */
int __tmd__SetRelayOutputSettings(struct soap *soap, struct _tmd__SetRelayOutputSettings *tmd__SetRelayOutputSettings, struct _tmd__SetRelayOutputSettingsResponse *tmd__SetRelayOutputSettingsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tmd__SetRelayOutputState */
int __tmd__SetRelayOutputState(struct soap *soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetDigitalInputs */
int __tmd__GetDigitalInputs(struct soap *soap, struct _tmd__GetDigitalInputs *tmd__GetDigitalInputs, struct _tmd__GetDigitalInputsResponse *tmd__GetDigitalInputsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetDigitalInputConfigurationOptions */
int __tmd__GetDigitalInputConfigurationOptions(struct soap *soap, struct _tmd__GetDigitalInputConfigurationOptions *tmd__GetDigitalInputConfigurationOptions, struct _tmd__GetDigitalInputConfigurationOptionsResponse *tmd__GetDigitalInputConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__SetDigitalInputConfigurations */
int __tmd__SetDigitalInputConfigurations(struct soap *soap, struct _tmd__SetDigitalInputConfigurations *tmd__SetDigitalInputConfigurations, struct _tmd__SetDigitalInputConfigurationsResponse *tmd__SetDigitalInputConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetSerialPorts */
int __tmd__GetSerialPorts(struct soap *soap, struct _tmd__GetSerialPorts *tmd__GetSerialPorts, struct _tmd__GetSerialPortsResponse *tmd__GetSerialPortsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetSerialPortConfiguration */
int __tmd__GetSerialPortConfiguration(struct soap *soap, struct _tmd__GetSerialPortConfiguration *tmd__GetSerialPortConfiguration, struct _tmd__GetSerialPortConfigurationResponse *tmd__GetSerialPortConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__SetSerialPortConfiguration */
int __tmd__SetSerialPortConfiguration(struct soap *soap, struct _tmd__SetSerialPortConfiguration *tmd__SetSerialPortConfiguration, struct _tmd__SetSerialPortConfigurationResponse *tmd__SetSerialPortConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tmd__GetSerialPortConfigurationOptions */
int __tmd__GetSerialPortConfigurationOptions(struct soap *soap, struct _tmd__GetSerialPortConfigurationOptions *tmd__GetSerialPortConfigurationOptions, struct _tmd__GetSerialPortConfigurationOptionsResponse *tmd__GetSerialPortConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tmd__SendReceiveSerialCommand */
int __tmd__SendReceiveSerialCommand(struct soap *soap, struct _tmd__SendReceiveSerialCommand *tmd__SendReceiveSerialCommand, struct _tmd__SendReceiveSerialCommandResponse *tmd__SendReceiveSerialCommandResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetServiceCapabilities */
int __tptz__GetServiceCapabilities(struct soap *soap, struct _tptz__GetServiceCapabilities *tptz__GetServiceCapabilities, struct _tptz__GetServiceCapabilitiesResponse *tptz__GetServiceCapabilitiesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetConfigurations */
int __tptz__GetConfigurations(struct soap *soap, struct _tptz__GetConfigurations *tptz__GetConfigurations, struct _tptz__GetConfigurationsResponse *tptz__GetConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetPresets */
int __tptz__GetPresets(struct soap *soap, struct _tptz__GetPresets *tptz__GetPresets, struct _tptz__GetPresetsResponse *tptz__GetPresetsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__SetPreset */
int __tptz__SetPreset(struct soap *soap, struct _tptz__SetPreset *tptz__SetPreset, struct _tptz__SetPresetResponse *tptz__SetPresetResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__RemovePreset */
int __tptz__RemovePreset(struct soap *soap, struct _tptz__RemovePreset *tptz__RemovePreset, struct _tptz__RemovePresetResponse *tptz__RemovePresetResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GotoPreset */
int __tptz__GotoPreset(struct soap *soap, struct _tptz__GotoPreset *tptz__GotoPreset, struct _tptz__GotoPresetResponse *tptz__GotoPresetResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetStatus */
int __tptz__GetStatus(struct soap *soap, struct _tptz__GetStatus *tptz__GetStatus, struct _tptz__GetStatusResponse *tptz__GetStatusResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetConfiguration */
int __tptz__GetConfiguration(struct soap *soap, struct _tptz__GetConfiguration *tptz__GetConfiguration, struct _tptz__GetConfigurationResponse *tptz__GetConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetNodes */
int __tptz__GetNodes(struct soap *soap, struct _tptz__GetNodes *tptz__GetNodes, struct _tptz__GetNodesResponse *tptz__GetNodesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetNode */
int __tptz__GetNode(struct soap *soap, struct _tptz__GetNode *tptz__GetNode, struct _tptz__GetNodeResponse *tptz__GetNodeResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__SetConfiguration */
int __tptz__SetConfiguration(struct soap *soap, struct _tptz__SetConfiguration *tptz__SetConfiguration, struct _tptz__SetConfigurationResponse *tptz__SetConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetConfigurationOptions */
int __tptz__GetConfigurationOptions(struct soap *soap, struct _tptz__GetConfigurationOptions *tptz__GetConfigurationOptions, struct _tptz__GetConfigurationOptionsResponse *tptz__GetConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GotoHomePosition */
int __tptz__GotoHomePosition(struct soap *soap, struct _tptz__GotoHomePosition *tptz__GotoHomePosition, struct _tptz__GotoHomePositionResponse *tptz__GotoHomePositionResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__SetHomePosition */
int __tptz__SetHomePosition(struct soap *soap, struct _tptz__SetHomePosition *tptz__SetHomePosition, struct _tptz__SetHomePositionResponse *tptz__SetHomePositionResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__ContinuousMove (沿某方向持续移动)命令*/
int __tptz__ContinuousMove(struct soap *soap, struct _tptz__ContinuousMove *tptz__ContinuousMove, struct _tptz__ContinuousMoveResponse *tptz__ContinuousMoveResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__RelativeMove （相对移动）命令*/
int __tptz__RelativeMove(struct soap *soap, struct _tptz__RelativeMove *tptz__RelativeMove, struct _tptz__RelativeMoveResponse *tptz__RelativeMoveResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__SendAuxiliaryCommand */
int __tptz__SendAuxiliaryCommand(struct soap *soap, struct _tptz__SendAuxiliaryCommand *tptz__SendAuxiliaryCommand, struct _tptz__SendAuxiliaryCommandResponse *tptz__SendAuxiliaryCommandResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__AbsoluteMove 控球用AbsoluteMove（绝对移动）命令 */
int __tptz__AbsoluteMove(struct soap *soap, struct _tptz__AbsoluteMove *tptz__AbsoluteMove, struct _tptz__AbsoluteMoveResponse *tptz__AbsoluteMoveResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GeoMove */
int __tptz__GeoMove(struct soap *soap, struct _tptz__GeoMove *tptz__GeoMove, struct _tptz__GeoMoveResponse *tptz__GeoMoveResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__Stop */
int __tptz__Stop(struct soap *soap, struct _tptz__Stop *tptz__Stop, struct _tptz__StopResponse *tptz__StopResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetPresetTours */
int __tptz__GetPresetTours(struct soap *soap, struct _tptz__GetPresetTours *tptz__GetPresetTours, struct _tptz__GetPresetToursResponse *tptz__GetPresetToursResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetPresetTour */
int __tptz__GetPresetTour(struct soap *soap, struct _tptz__GetPresetTour *tptz__GetPresetTour, struct _tptz__GetPresetTourResponse *tptz__GetPresetTourResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetPresetTourOptions */
int __tptz__GetPresetTourOptions(struct soap *soap, struct _tptz__GetPresetTourOptions *tptz__GetPresetTourOptions, struct _tptz__GetPresetTourOptionsResponse *tptz__GetPresetTourOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__CreatePresetTour */
int __tptz__CreatePresetTour(struct soap *soap, struct _tptz__CreatePresetTour *tptz__CreatePresetTour, struct _tptz__CreatePresetTourResponse *tptz__CreatePresetTourResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__ModifyPresetTour */
int __tptz__ModifyPresetTour(struct soap *soap, struct _tptz__ModifyPresetTour *tptz__ModifyPresetTour, struct _tptz__ModifyPresetTourResponse *tptz__ModifyPresetTourResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__OperatePresetTour */
int __tptz__OperatePresetTour(struct soap *soap, struct _tptz__OperatePresetTour *tptz__OperatePresetTour, struct _tptz__OperatePresetTourResponse *tptz__OperatePresetTourResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__RemovePresetTour */
int __tptz__RemovePresetTour(struct soap *soap, struct _tptz__RemovePresetTour *tptz__RemovePresetTour, struct _tptz__RemovePresetTourResponse *tptz__RemovePresetTourResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __tptz__GetCompatibleConfigurations */
int __tptz__GetCompatibleConfigurations(struct soap *soap, struct _tptz__GetCompatibleConfigurations *tptz__GetCompatibleConfigurations, struct _tptz__GetCompatibleConfigurationsResponse *tptz__GetCompatibleConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tptz__MoveAndStartTracking */
int __tptz__MoveAndStartTracking(struct soap *soap, struct _tptz__MoveAndStartTracking *tptz__MoveAndStartTracking, struct _tptz__MoveAndStartTrackingResponse *tptz__MoveAndStartTrackingResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trc__GetServiceCapabilities */
int __trc__GetServiceCapabilities(struct soap *soap, struct _trc__GetServiceCapabilities *trc__GetServiceCapabilities, struct _trc__GetServiceCapabilitiesResponse *trc__GetServiceCapabilitiesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__CreateRecording */
int __trc__CreateRecording(struct soap *soap, struct _trc__CreateRecording *trc__CreateRecording, struct _trc__CreateRecordingResponse *trc__CreateRecordingResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__DeleteRecording */
int __trc__DeleteRecording(struct soap *soap, struct _trc__DeleteRecording *trc__DeleteRecording, struct _trc__DeleteRecordingResponse *trc__DeleteRecordingResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trc__GetRecordings */
int __trc__GetRecordings(struct soap *soap, struct _trc__GetRecordings *trc__GetRecordings, struct _trc__GetRecordingsResponse *trc__GetRecordingsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__SetRecordingConfiguration */
int __trc__SetRecordingConfiguration(struct soap *soap, struct _trc__SetRecordingConfiguration *trc__SetRecordingConfiguration, struct _trc__SetRecordingConfigurationResponse *trc__SetRecordingConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trc__GetRecordingConfiguration */
int __trc__GetRecordingConfiguration(struct soap *soap, struct _trc__GetRecordingConfiguration *trc__GetRecordingConfiguration, struct _trc__GetRecordingConfigurationResponse *trc__GetRecordingConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__GetRecordingOptions */
int __trc__GetRecordingOptions(struct soap *soap, struct _trc__GetRecordingOptions *trc__GetRecordingOptions, struct _trc__GetRecordingOptionsResponse *trc__GetRecordingOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__CreateTrack */
int __trc__CreateTrack(struct soap *soap, struct _trc__CreateTrack *trc__CreateTrack, struct _trc__CreateTrackResponse *trc__CreateTrackResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__DeleteTrack */
int __trc__DeleteTrack(struct soap *soap, struct _trc__DeleteTrack *trc__DeleteTrack, struct _trc__DeleteTrackResponse *trc__DeleteTrackResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trc__GetTrackConfiguration */
int __trc__GetTrackConfiguration(struct soap *soap, struct _trc__GetTrackConfiguration *trc__GetTrackConfiguration, struct _trc__GetTrackConfigurationResponse *trc__GetTrackConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__SetTrackConfiguration */
int __trc__SetTrackConfiguration(struct soap *soap, struct _trc__SetTrackConfiguration *trc__SetTrackConfiguration, struct _trc__SetTrackConfigurationResponse *trc__SetTrackConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trc__CreateRecordingJob */
int __trc__CreateRecordingJob(struct soap *soap, struct _trc__CreateRecordingJob *trc__CreateRecordingJob, struct _trc__CreateRecordingJobResponse *trc__CreateRecordingJobResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__DeleteRecordingJob */
int __trc__DeleteRecordingJob(struct soap *soap, struct _trc__DeleteRecordingJob *trc__DeleteRecordingJob, struct _trc__DeleteRecordingJobResponse *trc__DeleteRecordingJobResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trc__GetRecordingJobs */
int __trc__GetRecordingJobs(struct soap *soap, struct _trc__GetRecordingJobs *trc__GetRecordingJobs, struct _trc__GetRecordingJobsResponse *trc__GetRecordingJobsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__SetRecordingJobConfiguration */
int __trc__SetRecordingJobConfiguration(struct soap *soap, struct _trc__SetRecordingJobConfiguration *trc__SetRecordingJobConfiguration, struct _trc__SetRecordingJobConfigurationResponse *trc__SetRecordingJobConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__GetRecordingJobConfiguration */
int __trc__GetRecordingJobConfiguration(struct soap *soap, struct _trc__GetRecordingJobConfiguration *trc__GetRecordingJobConfiguration, struct _trc__GetRecordingJobConfigurationResponse *trc__GetRecordingJobConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__SetRecordingJobMode */
int __trc__SetRecordingJobMode(struct soap *soap, struct _trc__SetRecordingJobMode *trc__SetRecordingJobMode, struct _trc__SetRecordingJobModeResponse *trc__SetRecordingJobModeResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trc__GetRecordingJobState */
int __trc__GetRecordingJobState(struct soap *soap, struct _trc__GetRecordingJobState *trc__GetRecordingJobState, struct _trc__GetRecordingJobStateResponse *trc__GetRecordingJobStateResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__ExportRecordedData */
int __trc__ExportRecordedData(struct soap *soap, struct _trc__ExportRecordedData *trc__ExportRecordedData, struct _trc__ExportRecordedDataResponse *trc__ExportRecordedDataResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__StopExportRecordedData */
int __trc__StopExportRecordedData(struct soap *soap, struct _trc__StopExportRecordedData *trc__StopExportRecordedData, struct _trc__StopExportRecordedDataResponse *trc__StopExportRecordedDataResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trc__GetExportRecordedDataState */
int __trc__GetExportRecordedDataState(struct soap *soap, struct _trc__GetExportRecordedDataState *trc__GetExportRecordedDataState, struct _trc__GetExportRecordedDataStateResponse *trc__GetExportRecordedDataStateResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trp__GetServiceCapabilities */
int __trp__GetServiceCapabilities(struct soap *soap, struct _trp__GetServiceCapabilities *trp__GetServiceCapabilities, struct _trp__GetServiceCapabilitiesResponse *trp__GetServiceCapabilitiesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trp__GetReplayUri */
int __trp__GetReplayUri(struct soap *soap, struct _trp__GetReplayUri *trp__GetReplayUri, struct _trp__GetReplayUriResponse *trp__GetReplayUriResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trp__GetReplayConfiguration */
int __trp__GetReplayConfiguration(struct soap *soap, struct _trp__GetReplayConfiguration *trp__GetReplayConfiguration, struct _trp__GetReplayConfigurationResponse *trp__GetReplayConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trp__SetReplayConfiguration */
int __trp__SetReplayConfiguration(struct soap *soap, struct _trp__SetReplayConfiguration *trp__SetReplayConfiguration, struct _trp__SetReplayConfigurationResponse *trp__SetReplayConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetServiceCapabilities */
int __trt__GetServiceCapabilities(struct soap *soap, struct _trt__GetServiceCapabilities *trt__GetServiceCapabilities, struct _trt__GetServiceCapabilitiesResponse *trt__GetServiceCapabilitiesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetVideoSources */
int __trt__GetVideoSources(struct soap *soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioSources */
int __trt__GetAudioSources(struct soap *soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioOutputs */
int __trt__GetAudioOutputs(struct soap *soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__CreateProfile */
int __trt__CreateProfile(struct soap *soap, struct _trt__CreateProfile *trt__CreateProfile, struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetProfile */
int __trt__GetProfile(struct soap *soap, struct _trt__GetProfile *trt__GetProfile, struct _trt__GetProfileResponse *trt__GetProfileResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetProfiles */
int __trt__GetProfiles(struct soap *soap, struct _trt__GetProfiles *trt__GetProfiles, struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
#if 0
        // VideoSourceConfiguration 
        trt__GetProfilesResponse->Profiles[i].Name = (char *)soap_malloc(soap, sizeof(char)*MAX_PROF_TOKEN);
        memset(trt__GetProfilesResponse->Profiles[i].Name, '\0', sizeof(char)*MAX_PROF_TOKEN);
        //profiles的名字，和token不同，实际请求的时候都是需要对应的token值来获取的
        strcpy(trt__GetProfilesResponse->Profiles[i].Name, "test_profile");
 
        trt__GetProfilesResponse->Profiles[i].token = (char *)soap_malloc(soap, sizeof(char)*MAX_PROF_TOKEN);
        memset(trt__GetProfilesResponse->Profiles[i].token, '\0', sizeof(char)*MAX_PROF_TOKEN);
        //此token也就是每次需要获取对应profiles的一些信息的时候，就需要在请求的时候填写此对应的token来，来进行验证判断 
        strcpy(trt__GetProfilesResponse->Profiles[i].token, "test_token");
 
        trt__GetProfilesResponse->Profiles[i].fixed = (enum xsd__boolean *)soap_malloc(soap, sizeof(int));
        memset(trt__GetProfilesResponse->Profiles[i].fixed, 0, sizeof(int));
        *(trt__GetProfilesResponse->Profiles[i].fixed) = (enum xsd__boolean )0;
        trt__GetProfilesResponse->Profiles[i].__anyAttribute = NULL;
 
        trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration = (struct tt__VideoSourceConfiguration *)soap_malloc(soap,sizeof(struct tt__VideoSourceConfiguration));
        memset(trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration, 0, sizeof(struct tt__VideoSourceConfiguration));
 
        trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Name = (char *)soap_malloc(soap,sizeof(char) * MAX_PROF_TOKEN);
        memset(trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Name, '\0', sizeof(char) * MAX_PROF_TOKEN);
        // 类似与上面,VideoSource Name,
        strcpy(trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Name, "test_vsname");
 
        trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->token = (char *)soap_malloc(soap,sizeof(char) * MAX_PROF_TOKEN);
        memset(trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->token, '\0', sizeof(char) * MAX_PROF_TOKEN);
        //求不同码流的视频源信息需要此token值匹配
        strcpy(trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->token, "test_vsoken");
 
        trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->SourceToken = (char *)soap_malloc(soap,sizeof(char) * MAX_PROF_TOKEN);
        memset(trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->SourceToken, '\0', sizeof(char) * MAX_PROF_TOKEN);
   
        strcpy(trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->SourceToken, "test_vstoken"); 
        trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->UseCount = 1; 
        trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle)); 
        memset(trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Bounds, 0, sizeof(struct tt__IntRectangle)); 
        trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Bounds->x = 0; 
        trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Bounds->y = 0; 
        trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Bounds->width = 1280; 
        trt__GetProfilesResponse->Profiles[i].VideoSourceConfiguration->Bounds->height = 720; 
        // VideoEncoderConfiguration 
        trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration = (struct tt__VideoEncoderConfiguration *)soap_malloc(soap, sizeof(struct tt__VideoEncoderConfiguration)) ;
        memset(trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration, '\0', sizeof(struct tt__VideoEncoderConfiguration)); 
        trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Name = (char *)soap_malloc(soap, sizeof(char)*MAX_PROF_TOKEN); 
        memset(trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Name, '\0', sizeof(char)*MAX_PROF_TOKEN); 
        strcpy(trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->Name, "test_vename"); 
        trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->token = (char *)soap_malloc(soap, sizeof(char)*MAX_PROF_TOKEN); 
        memset(trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->token, '\0', sizeof(char)*MAX_PROF_TOKEN); 
        strcpy(trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->token, "test_vstoken"); 
        trt__GetProfilesResponse->Profiles[i].VideoEncoderConfiguration->UseCount = 1; 
       //当然在实际开发的过程中最好把单个也实现了，也就是__trt__GetProfile函数接口，此接口是根据对应的请求的token来获取对应的信息的！
#endif
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__AddVideoEncoderConfiguration */
int __trt__AddVideoEncoderConfiguration(struct soap *soap, struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration, struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__AddVideoSourceConfiguration */
int __trt__AddVideoSourceConfiguration(struct soap *soap, struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration, struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__AddAudioEncoderConfiguration */
int __trt__AddAudioEncoderConfiguration(struct soap *soap, struct _trt__AddAudioEncoderConfiguration *trt__AddAudioEncoderConfiguration, struct _trt__AddAudioEncoderConfigurationResponse *trt__AddAudioEncoderConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__AddAudioSourceConfiguration */
int __trt__AddAudioSourceConfiguration(struct soap *soap, struct _trt__AddAudioSourceConfiguration *trt__AddAudioSourceConfiguration, struct _trt__AddAudioSourceConfigurationResponse *trt__AddAudioSourceConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__AddPTZConfiguration */
int __trt__AddPTZConfiguration(struct soap *soap, struct _trt__AddPTZConfiguration *trt__AddPTZConfiguration, struct _trt__AddPTZConfigurationResponse *trt__AddPTZConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__AddVideoAnalyticsConfiguration */
int __trt__AddVideoAnalyticsConfiguration(struct soap *soap, struct _trt__AddVideoAnalyticsConfiguration *trt__AddVideoAnalyticsConfiguration, struct _trt__AddVideoAnalyticsConfigurationResponse *trt__AddVideoAnalyticsConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__AddMetadataConfiguration */
int __trt__AddMetadataConfiguration(struct soap *soap, struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration, struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__AddAudioOutputConfiguration */
int __trt__AddAudioOutputConfiguration(struct soap *soap, struct _trt__AddAudioOutputConfiguration *trt__AddAudioOutputConfiguration, struct _trt__AddAudioOutputConfigurationResponse *trt__AddAudioOutputConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__AddAudioDecoderConfiguration */
int __trt__AddAudioDecoderConfiguration(struct soap *soap, struct _trt__AddAudioDecoderConfiguration *trt__AddAudioDecoderConfiguration, struct _trt__AddAudioDecoderConfigurationResponse *trt__AddAudioDecoderConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__RemoveVideoEncoderConfiguration */
int __trt__RemoveVideoEncoderConfiguration(struct soap *soap, struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration, struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__RemoveVideoSourceConfiguration */
int __trt__RemoveVideoSourceConfiguration(struct soap *soap, struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration, struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__RemoveAudioEncoderConfiguration */
int __trt__RemoveAudioEncoderConfiguration(struct soap *soap, struct _trt__RemoveAudioEncoderConfiguration *trt__RemoveAudioEncoderConfiguration, struct _trt__RemoveAudioEncoderConfigurationResponse *trt__RemoveAudioEncoderConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__RemoveAudioSourceConfiguration */
int __trt__RemoveAudioSourceConfiguration(struct soap *soap, struct _trt__RemoveAudioSourceConfiguration *trt__RemoveAudioSourceConfiguration, struct _trt__RemoveAudioSourceConfigurationResponse *trt__RemoveAudioSourceConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__RemovePTZConfiguration */
int __trt__RemovePTZConfiguration(struct soap *soap, struct _trt__RemovePTZConfiguration *trt__RemovePTZConfiguration, struct _trt__RemovePTZConfigurationResponse *trt__RemovePTZConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__RemoveVideoAnalyticsConfiguration */
int __trt__RemoveVideoAnalyticsConfiguration(struct soap *soap, struct _trt__RemoveVideoAnalyticsConfiguration *trt__RemoveVideoAnalyticsConfiguration, struct _trt__RemoveVideoAnalyticsConfigurationResponse *trt__RemoveVideoAnalyticsConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__RemoveMetadataConfiguration */
int __trt__RemoveMetadataConfiguration(struct soap *soap, struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration, struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__RemoveAudioOutputConfiguration */
int __trt__RemoveAudioOutputConfiguration(struct soap *soap, struct _trt__RemoveAudioOutputConfiguration *trt__RemoveAudioOutputConfiguration, struct _trt__RemoveAudioOutputConfigurationResponse *trt__RemoveAudioOutputConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__RemoveAudioDecoderConfiguration */
int __trt__RemoveAudioDecoderConfiguration(struct soap *soap, struct _trt__RemoveAudioDecoderConfiguration *trt__RemoveAudioDecoderConfiguration, struct _trt__RemoveAudioDecoderConfigurationResponse *trt__RemoveAudioDecoderConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__DeleteProfile */
int __trt__DeleteProfile(struct soap *soap, struct _trt__DeleteProfile *trt__DeleteProfile, struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetVideoSourceConfigurations */
int __trt__GetVideoSourceConfigurations(struct soap *soap, struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations, struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse)
{
#if 0
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].UseCount = 1;
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Name = (char*)soap_malloc(soap, sizeof(char) * MAX_PROF_TOKEN);
        memset(trt__GetVideoSourceConfigurationsResponse->Configurations[0].Name, '\0', sizeof(char) * MAX_PROF_TOKEN);
        strcpy(trt__GetVideoSourceConfigurationsResponse->Configurations[0].Name, "test_vsname");
 
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].token = (char*)soap_malloc(soap, sizeof(char) * MAX_PROF_TOKEN);
        memset(trt__GetVideoSourceConfigurationsResponse->Configurations[0].token, '\0', sizeof(char) * MAX_PROF_TOKEN);
        strcpy(trt__GetVideoSourceConfigurationsResponse->Configurations[0].token, "test_vstoken");
 
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].SourceToken = (char*)soap_malloc(soap, sizeof(char) * MAX_PROF_TOKEN);
        memset(trt__GetVideoSourceConfigurationsResponse->Configurations[0].SourceToken, '\0', sizeof(char) * MAX_PROF_TOKEN);
        strcpy(trt__GetVideoSourceConfigurationsResponse->Configurations[0].SourceToken, "test_vstoken");
 
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
        memset(trt__GetVideoSourceConfigurationsResponse->Configurations[0].Bounds, 0, sizeof(struct tt__IntRectangle));
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Bounds->x      = 0;
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Bounds->y      = 0;
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Bounds->width  = 1280;
        trt__GetVideoSourceConfigurationsResponse->Configurations[0].Bounds->height = 720;
#endif
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetVideoEncoderConfigurations */
int __trt__GetVideoEncoderConfigurations(struct soap *soap, struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations, struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse)
{
#if 0
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].Name = (char *)soap_malloc(soap, sizeof(char)*MAX_PROF_TOKEN);         
        memset(trt__GetVideoEncoderConfigurationsResponse->Configurations[0].Name, '\0', sizeof(char)*MAX_PROF_TOKEN);
        strcpy(trt__GetVideoEncoderConfigurationsResponse->Configurations[0].Name, "test_vsname");
            
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].token = (char *)soap_malloc(soap, sizeof(char)*MAX_PROF_TOKEN);
        memset(trt__GetVideoEncoderConfigurationsResponse->Configurations[0].token, '\0', sizeof(char)*MAX_PROF_TOKEN); 
        //请求的token值
        strcpy(trt__GetVideoEncoderConfigurationsResponse->Configurations[0].token, "test_vstoken");
                                 
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].UseCount = 1;
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].Quality = 100;
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].Encoding = (enum tt__VideoEncoding) 2;   // JPEG = 0 , MPEG = 1, H264 = 2;      
 
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].Resolution = (struct tt__VideoResolution *)soap_malloc(soap,sizeof(struct tt__VideoResolution));
        memset(trt__GetVideoEncoderConfigurationsResponse->Configurations[0].Resolution, 0 , sizeof(struct tt__VideoResolution));
        // 请求的视频的分辨率，对应前端设备填写对应的值，我这是1280 * 720
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].Resolution->Width  = 1280;
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].Resolution->Height = 720;
 
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
        memset(trt__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl, 0, sizeof(struct tt__VideoRateControl));
        //请求的视频数据的一些编码信息
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->FrameRateLimit   = 30;
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->EncodingInterval = 1;
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].RateControl->BitrateLimit     = 2048;
 
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
        memset(trt__GetVideoEncoderConfigurationsResponse->Configurations[0].H264, 0, sizeof(struct tt__H264Configuration));
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].H264->GovLength  = 30;
        trt__GetVideoEncoderConfigurationsResponse->Configurations[0].H264->H264Profile = (enum tt__H264Profile)3;
#endif
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioSourceConfigurations */
int __trt__GetAudioSourceConfigurations(struct soap *soap, struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations, struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioEncoderConfigurations */
int __trt__GetAudioEncoderConfigurations(struct soap *soap, struct _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations, struct _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetVideoAnalyticsConfigurations */
int __trt__GetVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetVideoAnalyticsConfigurations *trt__GetVideoAnalyticsConfigurations, struct _trt__GetVideoAnalyticsConfigurationsResponse *trt__GetVideoAnalyticsConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetMetadataConfigurations */
int __trt__GetMetadataConfigurations(struct soap *soap, struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations, struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioOutputConfigurations */
int __trt__GetAudioOutputConfigurations(struct soap *soap, struct _trt__GetAudioOutputConfigurations *trt__GetAudioOutputConfigurations, struct _trt__GetAudioOutputConfigurationsResponse *trt__GetAudioOutputConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioDecoderConfigurations */
int __trt__GetAudioDecoderConfigurations(struct soap *soap, struct _trt__GetAudioDecoderConfigurations *trt__GetAudioDecoderConfigurations, struct _trt__GetAudioDecoderConfigurationsResponse *trt__GetAudioDecoderConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetVideoSourceConfiguration */
int __trt__GetVideoSourceConfiguration(struct soap *soap, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration, struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
{
#if 0
        trt__GetVideoSourceConfigurationResponse->Configuration->UseCount = 1;
        trt__GetVideoSourceConfigurationResponse->Configuration->Name = (char*)soap_malloc(soap, sizeof(char) * MAX_PROF_TOKEN);
        memset(trt__GetVideoSourceConfigurationResponse->Configuration->Name, '\0', sizeof(char) * MAX_PROF_TOKEN);
        strcpy(trt__GetVideoSourceConfigurationResponse->Configuration->Name, "test_vsname");
 
        trt__GetVideoSourceConfigurationResponse->Configuration->token = (char*)soap_malloc(soap, sizeof(char) * MAX_PROF_TOKEN);
        memset(trt__GetVideoSourceConfigurationResponse->Configuration->token, '\0', sizeof(char) * MAX_PROF_TOKEN);
        strcpy(trt__GetVideoSourceConfigurationResponse->Configuration->token, "test_vstoken");
 
        trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken = (char*)soap_malloc(soap, sizeof(char) * MAX_PROF_TOKEN);
        memset(trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken, '\0', sizeof(char) * MAX_PROF_TOKEN);
        strcpy(trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken, "test_vstoken");
 
        trt__GetVideoSourceConfigurationResponse->Configuration->Bounds = (struct tt__IntRectangle *)soap_malloc(soap, sizeof(struct tt__IntRectangle));
        memset(trt__GetVideoSourceConfigurationResponse->Configuration->Bounds, 0, sizeof(struct tt__IntRectangle));
        trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->x      = 0;
        trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->y      = 0;
        trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->width  = 1280;
        trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->height = 720;
#endif
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetVideoEncoderConfiguration */
int __trt__GetVideoEncoderConfiguration(struct soap *soap, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration, struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
{
#if 0
        //请求的时候需要匹配的一些基本信息
        trt__GetVideoEncoderConfigurationResponse->Configuration->Name = (char *)soap_malloc(soap, sizeof(char)*MAX_PROF_TOKEN);
        memset(trt__GetVideoEncoderConfigurationResponse->Configuration->Name, '\0', sizeof(char)*MAX_PROF_TOKEN);
        strcpy(trt__GetVideoEncoderConfigurationResponse->Configuration->Name, "test_vsname");
 
        trt__GetVideoEncoderConfigurationResponse->Configuration->token = (char *)soap_malloc(soap, sizeof(char)*MAX_PROF_TOKEN);
        memset(trt__GetVideoEncoderConfigurationResponse->Configuration->token, '\0', sizeof(char)*MAX_PROF_TOKEN);
        strcpy(trt__GetVideoEncoderConfigurationResponse->Configuration->token, "test_vstoken");
 
        trt__GetVideoEncoderConfigurationResponse->Configuration->UseCount = 1;
        trt__GetVideoEncoderConfigurationResponse->Configuration->Quality = 100;
        //根据前端设备时间支持的编码格式选择对应的值，因为我测试的是设备只支持H264 ，所以选了2
        trt__GetVideoEncoderConfigurationResponse->Configuration->Encoding = (enum tt__VideoEncoding) 2;   // JPEG = 0 , MPEG = 1, H264 = 2; 
 
        trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution = (struct tt__VideoResolution *)soap_malloc(soap,sizeof(struct tt__VideoResolution));
        memset(trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution, 0 , sizeof(struct tt__VideoResolution));
        // 请求的视频的分辨率，对应前端设备填写对应的值，我这是1280 * 720
        trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Width  = 1280;
        trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Height = 720;
 
        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl = (struct tt__VideoRateControl *)soap_malloc(soap, sizeof(struct tt__VideoRateControl));
        memset(trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl, 0, sizeof(struct tt__VideoRateControl));
        //请求的对应的编码信息.各个意思参考上面说明
        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->FrameRateLimit   = 30;
        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->EncodingInterval = 1;
        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit     =2048;
 
        trt__GetVideoEncoderConfigurationResponse->Configuration->H264 = (struct tt__H264Configuration *)soap_malloc(soap, sizeof(struct tt__H264Configuration));
        memset(trt__GetVideoEncoderConfigurationResponse->Configuration->H264, 0, sizeof(struct tt__H264Configuration));
        trt__GetVideoEncoderConfigurationResponse->Configuration->H264->GovLength  = 30;
        trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = (enum tt__H264Profile)3;
#endif
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioSourceConfiguration */
int __trt__GetAudioSourceConfiguration(struct soap *soap, struct _trt__GetAudioSourceConfiguration *trt__GetAudioSourceConfiguration, struct _trt__GetAudioSourceConfigurationResponse *trt__GetAudioSourceConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioEncoderConfiguration */
int __trt__GetAudioEncoderConfiguration(struct soap *soap, struct _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration, struct _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetVideoAnalyticsConfiguration */
int __trt__GetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__GetVideoAnalyticsConfiguration *trt__GetVideoAnalyticsConfiguration, struct _trt__GetVideoAnalyticsConfigurationResponse *trt__GetVideoAnalyticsConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetMetadataConfiguration */
int __trt__GetMetadataConfiguration(struct soap *soap, struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration, struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioOutputConfiguration */
int __trt__GetAudioOutputConfiguration(struct soap *soap, struct _trt__GetAudioOutputConfiguration *trt__GetAudioOutputConfiguration, struct _trt__GetAudioOutputConfigurationResponse *trt__GetAudioOutputConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioDecoderConfiguration */
int __trt__GetAudioDecoderConfiguration(struct soap *soap, struct _trt__GetAudioDecoderConfiguration *trt__GetAudioDecoderConfiguration, struct _trt__GetAudioDecoderConfigurationResponse *trt__GetAudioDecoderConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetCompatibleVideoEncoderConfigurations */
int __trt__GetCompatibleVideoEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations, struct _trt__GetCompatibleVideoEncoderConfigurationsResponse *trt__GetCompatibleVideoEncoderConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetCompatibleVideoSourceConfigurations */
int __trt__GetCompatibleVideoSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations, struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetCompatibleAudioEncoderConfigurations */
int __trt__GetCompatibleAudioEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioEncoderConfigurations *trt__GetCompatibleAudioEncoderConfigurations, struct _trt__GetCompatibleAudioEncoderConfigurationsResponse *trt__GetCompatibleAudioEncoderConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetCompatibleAudioSourceConfigurations */
int __trt__GetCompatibleAudioSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioSourceConfigurations *trt__GetCompatibleAudioSourceConfigurations, struct _trt__GetCompatibleAudioSourceConfigurationsResponse *trt__GetCompatibleAudioSourceConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetCompatibleVideoAnalyticsConfigurations */
int __trt__GetCompatibleVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoAnalyticsConfigurations *trt__GetCompatibleVideoAnalyticsConfigurations, struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse *trt__GetCompatibleVideoAnalyticsConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetCompatibleMetadataConfigurations */
int __trt__GetCompatibleMetadataConfigurations(struct soap *soap, struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations, struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetCompatibleAudioOutputConfigurations */
int __trt__GetCompatibleAudioOutputConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioOutputConfigurations *trt__GetCompatibleAudioOutputConfigurations, struct _trt__GetCompatibleAudioOutputConfigurationsResponse *trt__GetCompatibleAudioOutputConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetCompatibleAudioDecoderConfigurations */
int __trt__GetCompatibleAudioDecoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioDecoderConfigurations *trt__GetCompatibleAudioDecoderConfigurations, struct _trt__GetCompatibleAudioDecoderConfigurationsResponse *trt__GetCompatibleAudioDecoderConfigurationsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__SetVideoSourceConfiguration */
int __trt__SetVideoSourceConfiguration(struct soap *soap, struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration, struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__SetVideoEncoderConfiguration */
int __trt__SetVideoEncoderConfiguration(struct soap *soap, struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration, struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__SetAudioSourceConfiguration */
int __trt__SetAudioSourceConfiguration(struct soap *soap, struct _trt__SetAudioSourceConfiguration *trt__SetAudioSourceConfiguration, struct _trt__SetAudioSourceConfigurationResponse *trt__SetAudioSourceConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__SetAudioEncoderConfiguration */
int __trt__SetAudioEncoderConfiguration(struct soap *soap, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration, struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__SetVideoAnalyticsConfiguration */
int __trt__SetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__SetVideoAnalyticsConfiguration *trt__SetVideoAnalyticsConfiguration, struct _trt__SetVideoAnalyticsConfigurationResponse *trt__SetVideoAnalyticsConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__SetMetadataConfiguration */
int __trt__SetMetadataConfiguration(struct soap *soap, struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration, struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__SetAudioOutputConfiguration */
int __trt__SetAudioOutputConfiguration(struct soap *soap, struct _trt__SetAudioOutputConfiguration *trt__SetAudioOutputConfiguration, struct _trt__SetAudioOutputConfigurationResponse *trt__SetAudioOutputConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__SetAudioDecoderConfiguration */
int __trt__SetAudioDecoderConfiguration(struct soap *soap, struct _trt__SetAudioDecoderConfiguration *trt__SetAudioDecoderConfiguration, struct _trt__SetAudioDecoderConfigurationResponse *trt__SetAudioDecoderConfigurationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetVideoSourceConfigurationOptions */
int __trt__GetVideoSourceConfigurationOptions(struct soap *soap, struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions, struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetVideoEncoderConfigurationOptions */
int __trt__GetVideoEncoderConfigurationOptions(struct soap *soap, struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions, struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioSourceConfigurationOptions */
int __trt__GetAudioSourceConfigurationOptions(struct soap *soap, struct _trt__GetAudioSourceConfigurationOptions *trt__GetAudioSourceConfigurationOptions, struct _trt__GetAudioSourceConfigurationOptionsResponse *trt__GetAudioSourceConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioEncoderConfigurationOptions */
int __trt__GetAudioEncoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions, struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetMetadataConfigurationOptions */
int __trt__GetMetadataConfigurationOptions(struct soap *soap, struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions, struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioOutputConfigurationOptions */
int __trt__GetAudioOutputConfigurationOptions(struct soap *soap, struct _trt__GetAudioOutputConfigurationOptions *trt__GetAudioOutputConfigurationOptions, struct _trt__GetAudioOutputConfigurationOptionsResponse *trt__GetAudioOutputConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetAudioDecoderConfigurationOptions */
int __trt__GetAudioDecoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioDecoderConfigurationOptions *trt__GetAudioDecoderConfigurationOptions, struct _trt__GetAudioDecoderConfigurationOptionsResponse *trt__GetAudioDecoderConfigurationOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetGuaranteedNumberOfVideoEncoderInstances */
int __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap *soap, struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances, struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetStreamUri */
int __trt__GetStreamUri(struct soap *soap, struct _trt__GetStreamUri *trt__GetStreamUri, struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
{
    trt__GetStreamUriResponse->MediaUri = (struct tt__MediaUri *)soap_malloc(soap, sizeof(struct tt__MediaUri));
    memset(trt__GetStreamUriResponse->MediaUri, 0, sizeof(struct tt__MediaUri));
        
    trt__GetStreamUriResponse->MediaUri->Uri = (char *)soap_malloc(soap, sizeof(char) * MAX_64_LEN);
    memset(trt__GetStreamUriResponse->MediaUri->Uri, '\0', sizeof(char) * MAX_64_LEN);
    //根据各个设备的rtsp协议的uri不同填写对应的值
    strncpy(trt__GetStreamUriResponse->MediaUri->Uri, "rtsp://192.168.182.129:554/livestream", MAX_64_LEN);
    trt__GetStreamUriResponse->MediaUri->InvalidAfterConnect = (enum xsd__boolean)0;
    trt__GetStreamUriResponse->MediaUri->InvalidAfterReboot  = (enum xsd__boolean)0;
    //超时时间
    trt__GetStreamUriResponse->MediaUri->Timeout = 200;
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__StartMulticastStreaming */
int __trt__StartMulticastStreaming(struct soap *soap, struct _trt__StartMulticastStreaming *trt__StartMulticastStreaming, struct _trt__StartMulticastStreamingResponse *trt__StartMulticastStreamingResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__StopMulticastStreaming */
int __trt__StopMulticastStreaming(struct soap *soap, struct _trt__StopMulticastStreaming *trt__StopMulticastStreaming, struct _trt__StopMulticastStreamingResponse *trt__StopMulticastStreamingResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__SetSynchronizationPoint */
int __trt__SetSynchronizationPoint(struct soap *soap, struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint, struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetSnapshotUri */
int __trt__GetSnapshotUri(struct soap *soap, struct _trt__GetSnapshotUri *trt__GetSnapshotUri, struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetVideoSourceModes */
int __trt__GetVideoSourceModes(struct soap *soap, struct _trt__GetVideoSourceModes *trt__GetVideoSourceModes, struct _trt__GetVideoSourceModesResponse *trt__GetVideoSourceModesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__SetVideoSourceMode */
int __trt__SetVideoSourceMode(struct soap *soap, struct _trt__SetVideoSourceMode *trt__SetVideoSourceMode, struct _trt__SetVideoSourceModeResponse *trt__SetVideoSourceModeResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetOSDs */
int __trt__GetOSDs(struct soap *soap, struct _trt__GetOSDs *trt__GetOSDs, struct _trt__GetOSDsResponse *trt__GetOSDsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetOSD */
int __trt__GetOSD(struct soap *soap, struct _trt__GetOSD *trt__GetOSD, struct _trt__GetOSDResponse *trt__GetOSDResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__GetOSDOptions */
int __trt__GetOSDOptions(struct soap *soap, struct _trt__GetOSDOptions *trt__GetOSDOptions, struct _trt__GetOSDOptionsResponse *trt__GetOSDOptionsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__SetOSD */
int __trt__SetOSD(struct soap *soap, struct _trt__SetOSD *trt__SetOSD, struct _trt__SetOSDResponse *trt__SetOSDResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__CreateOSD */
int __trt__CreateOSD(struct soap *soap, struct _trt__CreateOSD *trt__CreateOSD, struct _trt__CreateOSDResponse *trt__CreateOSDResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trt__DeleteOSD */
int __trt__DeleteOSD(struct soap *soap, struct _trt__DeleteOSD *trt__DeleteOSD, struct _trt__DeleteOSDResponse *trt__DeleteOSDResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trv__GetServiceCapabilities */
int __trv__GetServiceCapabilities(struct soap *soap, struct _trv__GetServiceCapabilities *trv__GetServiceCapabilities, struct _trv__GetServiceCapabilitiesResponse *trv__GetServiceCapabilitiesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trv__GetReceivers */
int __trv__GetReceivers(struct soap *soap, struct _trv__GetReceivers *trv__GetReceivers, struct _trv__GetReceiversResponse *trv__GetReceiversResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trv__GetReceiver */
int __trv__GetReceiver(struct soap *soap, struct _trv__GetReceiver *trv__GetReceiver, struct _trv__GetReceiverResponse *trv__GetReceiverResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trv__CreateReceiver */
int __trv__CreateReceiver(struct soap *soap, struct _trv__CreateReceiver *trv__CreateReceiver, struct _trv__CreateReceiverResponse *trv__CreateReceiverResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __trv__DeleteReceiver */
int __trv__DeleteReceiver(struct soap *soap, struct _trv__DeleteReceiver *trv__DeleteReceiver, struct _trv__DeleteReceiverResponse *trv__DeleteReceiverResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trv__ConfigureReceiver */
int __trv__ConfigureReceiver(struct soap *soap, struct _trv__ConfigureReceiver *trv__ConfigureReceiver, struct _trv__ConfigureReceiverResponse *trv__ConfigureReceiverResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trv__SetReceiverMode */
int __trv__SetReceiverMode(struct soap *soap, struct _trv__SetReceiverMode *trv__SetReceiverMode, struct _trv__SetReceiverModeResponse *trv__SetReceiverModeResponse)
{
	(void)soap; /* appease -Wall -Werror */
	return SOAP_OK;
}


/** Onvif server operation __trv__GetReceiverState */
int __trv__GetReceiverState(struct soap *soap, struct _trv__GetReceiverState *trv__GetReceiverState, struct _trv__GetReceiverStateResponse *trv__GetReceiverStateResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__GetServiceCapabilities */
int __tse__GetServiceCapabilities(struct soap *soap, struct _tse__GetServiceCapabilities *tse__GetServiceCapabilities, struct _tse__GetServiceCapabilitiesResponse *tse__GetServiceCapabilitiesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__GetRecordingSummary */
int __tse__GetRecordingSummary(struct soap *soap, struct _tse__GetRecordingSummary *tse__GetRecordingSummary, struct _tse__GetRecordingSummaryResponse *tse__GetRecordingSummaryResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__GetRecordingInformation */
int __tse__GetRecordingInformation(struct soap *soap, struct _tse__GetRecordingInformation *tse__GetRecordingInformation, struct _tse__GetRecordingInformationResponse *tse__GetRecordingInformationResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__GetMediaAttributes */
int __tse__GetMediaAttributes(struct soap *soap, struct _tse__GetMediaAttributes *tse__GetMediaAttributes, struct _tse__GetMediaAttributesResponse *tse__GetMediaAttributesResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__FindRecordings */
int __tse__FindRecordings(struct soap *soap, struct _tse__FindRecordings *tse__FindRecordings, struct _tse__FindRecordingsResponse *tse__FindRecordingsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__GetRecordingSearchResults */
int __tse__GetRecordingSearchResults(struct soap *soap, struct _tse__GetRecordingSearchResults *tse__GetRecordingSearchResults, struct _tse__GetRecordingSearchResultsResponse *tse__GetRecordingSearchResultsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__FindEvents */
int __tse__FindEvents(struct soap *soap, struct _tse__FindEvents *tse__FindEvents, struct _tse__FindEventsResponse *tse__FindEventsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__GetEventSearchResults */
int __tse__GetEventSearchResults(struct soap *soap, struct _tse__GetEventSearchResults *tse__GetEventSearchResults, struct _tse__GetEventSearchResultsResponse *tse__GetEventSearchResultsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__FindPTZPosition */
int __tse__FindPTZPosition(struct soap *soap, struct _tse__FindPTZPosition *tse__FindPTZPosition, struct _tse__FindPTZPositionResponse *tse__FindPTZPositionResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__GetPTZPositionSearchResults */
int __tse__GetPTZPositionSearchResults(struct soap *soap, struct _tse__GetPTZPositionSearchResults *tse__GetPTZPositionSearchResults, struct _tse__GetPTZPositionSearchResultsResponse *tse__GetPTZPositionSearchResultsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__GetSearchState */
int __tse__GetSearchState(struct soap *soap, struct _tse__GetSearchState *tse__GetSearchState, struct _tse__GetSearchStateResponse *tse__GetSearchStateResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__EndSearch */
int __tse__EndSearch(struct soap *soap, struct _tse__EndSearch *tse__EndSearch, struct _tse__EndSearchResponse *tse__EndSearchResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__FindMetadata */
int __tse__FindMetadata(struct soap *soap, struct _tse__FindMetadata *tse__FindMetadata, struct _tse__FindMetadataResponse *tse__FindMetadataResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}


/** Onvif server operation __tse__GetMetadataSearchResults */
int __tse__GetMetadataSearchResults(struct soap *soap, struct _tse__GetMetadataSearchResults *tse__GetMetadataSearchResults, struct _tse__GetMetadataSearchResultsResponse *tse__GetMetadataSearchResultsResponse)
{
	(void)soap; /* appease -Wall -Werror */
	/* Return response with default data and some values copied from the request */
	return SOAP_OK;
}
