/* soapC_nnn.c
   Generated by gSOAP 2.8.114 for onvif.h

gSOAP XML Web services tools
Copyright (C) 2000-2021, Robert van Engelen, Genivia Inc. All Rights Reserved.
The soapcpp2 tool and its generated software are released under the GPL.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
--------------------------------------------------------------------------------
A commercial use license is available from Genivia Inc., contact@genivia.com
--------------------------------------------------------------------------------
*/

#if defined(__BORLANDC__)
#pragma option push -w-8060
#pragma option push -w-8004
#endif

#include "soapH.h"

SOAP_SOURCE_STAMP("@(#) soapC_nnn.c ver 2.8.114 2021-05-24 10:18:06 GMT")


#ifndef WITH_NOGLOBAL

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_PointerToSOAP_ENV__Reason(struct soap *soap, struct SOAP_ENV__Reason *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	if (!soap_reference(soap, *a, SOAP_TYPE_SOAP_ENV__Reason))
		soap_serialize_SOAP_ENV__Reason(soap, *a);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_PointerToSOAP_ENV__Reason(struct soap *soap, const char *tag, int id, struct SOAP_ENV__Reason *const*a, const char *type)
{
	id = soap_element_id(soap, tag, id, *a, NULL, 0, type, SOAP_TYPE_SOAP_ENV__Reason, NULL);
	if (id < 0)
		return soap->error;
	return soap_out_SOAP_ENV__Reason(soap, tag, id, *a, type);
}

SOAP_FMAC3 struct SOAP_ENV__Reason ** SOAP_FMAC4 soap_in_PointerToSOAP_ENV__Reason(struct soap *soap, const char *tag, struct SOAP_ENV__Reason **a, const char *type)
{
	(void)type; /* appease -Wall -Werror */
	if (soap_element_begin_in(soap, tag, 1, NULL))
		return NULL;
	if (!a)
		if (!(a = (struct SOAP_ENV__Reason **)soap_malloc(soap, sizeof(struct SOAP_ENV__Reason *))))
			return NULL;
	*a = NULL;
	if (!soap->null && *soap->href != '#')
	{	soap_revert(soap);
		if (!(*a = soap_in_SOAP_ENV__Reason(soap, tag, *a, type)))
			return NULL;
	}
	else
	{	a = (struct SOAP_ENV__Reason **)soap_id_lookup(soap, soap->href, (void**)a, SOAP_TYPE_SOAP_ENV__Reason, sizeof(struct SOAP_ENV__Reason), 0, NULL);
		if (soap->body && soap_element_end_in(soap, tag))
			return NULL;
	}
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_PointerToSOAP_ENV__Reason(struct soap *soap, struct SOAP_ENV__Reason *const*a, const char *tag, const char *type)
{
	if (soap_out_PointerToSOAP_ENV__Reason(soap, tag ? tag : "SOAP-ENV:Reason", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 struct SOAP_ENV__Reason ** SOAP_FMAC4 soap_get_PointerToSOAP_ENV__Reason(struct soap *soap, struct SOAP_ENV__Reason **p, const char *tag, const char *type)
{
	if ((p = soap_in_PointerToSOAP_ENV__Reason(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

#endif

#ifndef WITH_NOGLOBAL

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_PointerToSOAP_ENV__Code(struct soap *soap, struct SOAP_ENV__Code *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	if (!soap_reference(soap, *a, SOAP_TYPE_SOAP_ENV__Code))
		soap_serialize_SOAP_ENV__Code(soap, *a);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_PointerToSOAP_ENV__Code(struct soap *soap, const char *tag, int id, struct SOAP_ENV__Code *const*a, const char *type)
{
	char *mark;
	id = soap_element_id(soap, tag, id, *a, NULL, 0, type, SOAP_TYPE_SOAP_ENV__Code, &mark);
	if (id < 0)
		return soap->error;
	(void)soap_out_SOAP_ENV__Code(soap, tag, id, *a, type);
	soap_unmark(soap, mark);
	return soap->error;
}

SOAP_FMAC3 struct SOAP_ENV__Code ** SOAP_FMAC4 soap_in_PointerToSOAP_ENV__Code(struct soap *soap, const char *tag, struct SOAP_ENV__Code **a, const char *type)
{
	(void)type; /* appease -Wall -Werror */
	if (soap_element_begin_in(soap, tag, 1, NULL))
		return NULL;
	if (!a)
		if (!(a = (struct SOAP_ENV__Code **)soap_malloc(soap, sizeof(struct SOAP_ENV__Code *))))
			return NULL;
	*a = NULL;
	if (!soap->null && *soap->href != '#')
	{	soap_revert(soap);
		if (!(*a = soap_in_SOAP_ENV__Code(soap, tag, *a, type)))
			return NULL;
	}
	else
	{	a = (struct SOAP_ENV__Code **)soap_id_lookup(soap, soap->href, (void**)a, SOAP_TYPE_SOAP_ENV__Code, sizeof(struct SOAP_ENV__Code), 0, NULL);
		if (soap->body && soap_element_end_in(soap, tag))
			return NULL;
	}
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_PointerToSOAP_ENV__Code(struct soap *soap, struct SOAP_ENV__Code *const*a, const char *tag, const char *type)
{
	if (soap_out_PointerToSOAP_ENV__Code(soap, tag ? tag : "SOAP-ENV:Code", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 struct SOAP_ENV__Code ** SOAP_FMAC4 soap_get_PointerToSOAP_ENV__Code(struct soap *soap, struct SOAP_ENV__Code **p, const char *tag, const char *type)
{
	if ((p = soap_in_PointerToSOAP_ENV__Code(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

#endif

#ifndef WITH_NOGLOBAL

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_PointerToSOAP_ENV__Detail(struct soap *soap, struct SOAP_ENV__Detail *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	if (!soap_reference(soap, *a, SOAP_TYPE_SOAP_ENV__Detail))
		soap_serialize_SOAP_ENV__Detail(soap, *a);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_PointerToSOAP_ENV__Detail(struct soap *soap, const char *tag, int id, struct SOAP_ENV__Detail *const*a, const char *type)
{
	id = soap_element_id(soap, tag, id, *a, NULL, 0, type, SOAP_TYPE_SOAP_ENV__Detail, NULL);
	if (id < 0)
		return soap->error;
	return soap_out_SOAP_ENV__Detail(soap, tag, id, *a, type);
}

SOAP_FMAC3 struct SOAP_ENV__Detail ** SOAP_FMAC4 soap_in_PointerToSOAP_ENV__Detail(struct soap *soap, const char *tag, struct SOAP_ENV__Detail **a, const char *type)
{
	(void)type; /* appease -Wall -Werror */
	if (soap_element_begin_in(soap, tag, 1, NULL))
		return NULL;
	if (!a)
		if (!(a = (struct SOAP_ENV__Detail **)soap_malloc(soap, sizeof(struct SOAP_ENV__Detail *))))
			return NULL;
	*a = NULL;
	if (!soap->null && *soap->href != '#')
	{	soap_revert(soap);
		if (!(*a = soap_in_SOAP_ENV__Detail(soap, tag, *a, type)))
			return NULL;
	}
	else
	{	a = (struct SOAP_ENV__Detail **)soap_id_lookup(soap, soap->href, (void**)a, SOAP_TYPE_SOAP_ENV__Detail, sizeof(struct SOAP_ENV__Detail), 0, NULL);
		if (soap->body && soap_element_end_in(soap, tag))
			return NULL;
	}
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_PointerToSOAP_ENV__Detail(struct soap *soap, struct SOAP_ENV__Detail *const*a, const char *tag, const char *type)
{
	if (soap_out_PointerToSOAP_ENV__Detail(soap, tag ? tag : "SOAP-ENV:Detail", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 struct SOAP_ENV__Detail ** SOAP_FMAC4 soap_get_PointerToSOAP_ENV__Detail(struct soap *soap, struct SOAP_ENV__Detail **p, const char *tag, const char *type)
{
	if ((p = soap_in_PointerToSOAP_ENV__Detail(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

#endif

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_PointerTo_wsa__FaultTo(struct soap *soap, struct wsa__EndpointReferenceType *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	if (!soap_reference(soap, *a, SOAP_TYPE__wsa__FaultTo))
		soap_serialize__wsa__FaultTo(soap, *a);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_PointerTo_wsa__FaultTo(struct soap *soap, const char *tag, int id, struct wsa__EndpointReferenceType *const*a, const char *type)
{
	id = soap_element_id(soap, tag, id, *a, NULL, 0, type, SOAP_TYPE__wsa__FaultTo, NULL);
	if (id < 0)
		return soap->error;
	return soap_out__wsa__FaultTo(soap, tag, id, *a, type);
}

SOAP_FMAC3 struct wsa__EndpointReferenceType ** SOAP_FMAC4 soap_in_PointerTo_wsa__FaultTo(struct soap *soap, const char *tag, struct wsa__EndpointReferenceType **a, const char *type)
{
	(void)type; /* appease -Wall -Werror */
	if (soap_element_begin_in(soap, tag, 1, NULL))
		return NULL;
	if (!a)
		if (!(a = (struct wsa__EndpointReferenceType **)soap_malloc(soap, sizeof(struct wsa__EndpointReferenceType *))))
			return NULL;
	*a = NULL;
	if (!soap->null && *soap->href != '#')
	{	soap_revert(soap);
		if (!(*a = soap_in__wsa__FaultTo(soap, tag, *a, type)))
			return NULL;
	}
	else
	{	a = (struct wsa__EndpointReferenceType **)soap_id_lookup(soap, soap->href, (void**)a, SOAP_TYPE__wsa__FaultTo, sizeof(struct wsa__EndpointReferenceType), 0, NULL);
		if (soap->body && soap_element_end_in(soap, tag))
			return NULL;
	}
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_PointerTo_wsa__FaultTo(struct soap *soap, struct wsa__EndpointReferenceType *const*a, const char *tag, const char *type)
{
	if (soap_out_PointerTo_wsa__FaultTo(soap, tag ? tag : "wsa:FaultTo", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 struct wsa__EndpointReferenceType ** SOAP_FMAC4 soap_get_PointerTo_wsa__FaultTo(struct soap *soap, struct wsa__EndpointReferenceType **p, const char *tag, const char *type)
{
	if ((p = soap_in_PointerTo_wsa__FaultTo(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_PointerTo_wsa__ReplyTo(struct soap *soap, struct wsa__EndpointReferenceType *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	if (!soap_reference(soap, *a, SOAP_TYPE__wsa__ReplyTo))
		soap_serialize__wsa__ReplyTo(soap, *a);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_PointerTo_wsa__ReplyTo(struct soap *soap, const char *tag, int id, struct wsa__EndpointReferenceType *const*a, const char *type)
{
	id = soap_element_id(soap, tag, id, *a, NULL, 0, type, SOAP_TYPE__wsa__ReplyTo, NULL);
	if (id < 0)
		return soap->error;
	return soap_out__wsa__ReplyTo(soap, tag, id, *a, type);
}

SOAP_FMAC3 struct wsa__EndpointReferenceType ** SOAP_FMAC4 soap_in_PointerTo_wsa__ReplyTo(struct soap *soap, const char *tag, struct wsa__EndpointReferenceType **a, const char *type)
{
	(void)type; /* appease -Wall -Werror */
	if (soap_element_begin_in(soap, tag, 1, NULL))
		return NULL;
	if (!a)
		if (!(a = (struct wsa__EndpointReferenceType **)soap_malloc(soap, sizeof(struct wsa__EndpointReferenceType *))))
			return NULL;
	*a = NULL;
	if (!soap->null && *soap->href != '#')
	{	soap_revert(soap);
		if (!(*a = soap_in__wsa__ReplyTo(soap, tag, *a, type)))
			return NULL;
	}
	else
	{	a = (struct wsa__EndpointReferenceType **)soap_id_lookup(soap, soap->href, (void**)a, SOAP_TYPE__wsa__ReplyTo, sizeof(struct wsa__EndpointReferenceType), 0, NULL);
		if (soap->body && soap_element_end_in(soap, tag))
			return NULL;
	}
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_PointerTo_wsa__ReplyTo(struct soap *soap, struct wsa__EndpointReferenceType *const*a, const char *tag, const char *type)
{
	if (soap_out_PointerTo_wsa__ReplyTo(soap, tag ? tag : "wsa:ReplyTo", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 struct wsa__EndpointReferenceType ** SOAP_FMAC4 soap_get_PointerTo_wsa__ReplyTo(struct soap *soap, struct wsa__EndpointReferenceType **p, const char *tag, const char *type)
{
	if ((p = soap_in_PointerTo_wsa__ReplyTo(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_PointerTo_wsa__From(struct soap *soap, struct wsa__EndpointReferenceType *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	if (!soap_reference(soap, *a, SOAP_TYPE__wsa__From))
		soap_serialize__wsa__From(soap, *a);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_PointerTo_wsa__From(struct soap *soap, const char *tag, int id, struct wsa__EndpointReferenceType *const*a, const char *type)
{
	id = soap_element_id(soap, tag, id, *a, NULL, 0, type, SOAP_TYPE__wsa__From, NULL);
	if (id < 0)
		return soap->error;
	return soap_out__wsa__From(soap, tag, id, *a, type);
}

SOAP_FMAC3 struct wsa__EndpointReferenceType ** SOAP_FMAC4 soap_in_PointerTo_wsa__From(struct soap *soap, const char *tag, struct wsa__EndpointReferenceType **a, const char *type)
{
	(void)type; /* appease -Wall -Werror */
	if (soap_element_begin_in(soap, tag, 1, NULL))
		return NULL;
	if (!a)
		if (!(a = (struct wsa__EndpointReferenceType **)soap_malloc(soap, sizeof(struct wsa__EndpointReferenceType *))))
			return NULL;
	*a = NULL;
	if (!soap->null && *soap->href != '#')
	{	soap_revert(soap);
		if (!(*a = soap_in__wsa__From(soap, tag, *a, type)))
			return NULL;
	}
	else
	{	a = (struct wsa__EndpointReferenceType **)soap_id_lookup(soap, soap->href, (void**)a, SOAP_TYPE__wsa__From, sizeof(struct wsa__EndpointReferenceType), 0, NULL);
		if (soap->body && soap_element_end_in(soap, tag))
			return NULL;
	}
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_PointerTo_wsa__From(struct soap *soap, struct wsa__EndpointReferenceType *const*a, const char *tag, const char *type)
{
	if (soap_out_PointerTo_wsa__From(soap, tag ? tag : "wsa:From", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 struct wsa__EndpointReferenceType ** SOAP_FMAC4 soap_get_PointerTo_wsa__From(struct soap *soap, struct wsa__EndpointReferenceType **p, const char *tag, const char *type)
{
	if ((p = soap_in_PointerTo_wsa__From(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_PointerTo_wsa__RelatesTo(struct soap *soap, struct wsa__Relationship *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	if (!soap_reference(soap, *a, SOAP_TYPE__wsa__RelatesTo))
		soap_serialize__wsa__RelatesTo(soap, *a);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_PointerTo_wsa__RelatesTo(struct soap *soap, const char *tag, int id, struct wsa__Relationship *const*a, const char *type)
{
	id = soap_element_id(soap, tag, id, *a, NULL, 0, type, SOAP_TYPE__wsa__RelatesTo, NULL);
	if (id < 0)
		return soap->error;
	return soap_out__wsa__RelatesTo(soap, tag, id, *a, type);
}

SOAP_FMAC3 struct wsa__Relationship ** SOAP_FMAC4 soap_in_PointerTo_wsa__RelatesTo(struct soap *soap, const char *tag, struct wsa__Relationship **a, const char *type)
{
	(void)type; /* appease -Wall -Werror */
	if (soap_element_begin_in(soap, tag, 1, NULL))
		return NULL;
	if (!a)
		if (!(a = (struct wsa__Relationship **)soap_malloc(soap, sizeof(struct wsa__Relationship *))))
			return NULL;
	*a = NULL;
	if (!soap->null && *soap->href != '#')
	{	soap_revert(soap);
		if (!(*a = soap_in__wsa__RelatesTo(soap, tag, *a, type)))
			return NULL;
	}
	else
	{	a = (struct wsa__Relationship **)soap_id_lookup(soap, soap->href, (void**)a, SOAP_TYPE__wsa__RelatesTo, sizeof(struct wsa__Relationship), 0, NULL);
		if (soap->body && soap_element_end_in(soap, tag))
			return NULL;
	}
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_PointerTo_wsa__RelatesTo(struct soap *soap, struct wsa__Relationship *const*a, const char *tag, const char *type)
{
	if (soap_out_PointerTo_wsa__RelatesTo(soap, tag ? tag : "wsa:RelatesTo", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 struct wsa__Relationship ** SOAP_FMAC4 soap_get_PointerTo_wsa__RelatesTo(struct soap *soap, struct wsa__Relationship **p, const char *tag, const char *type)
{
	if ((p = soap_in_PointerTo_wsa__RelatesTo(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put__wsa__Action(struct soap *soap, char *const*a, const char *tag, const char *type)
{
	if (soap_out__wsa__Action(soap, tag ? tag : "wsa:Action", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put__wsa__To(struct soap *soap, char *const*a, const char *tag, const char *type)
{
	if (soap_out__wsa__To(soap, tag ? tag : "wsa:To", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put__wsa__MessageID(struct soap *soap, char *const*a, const char *tag, const char *type)
{
	if (soap_out__wsa__MessageID(soap, tag ? tag : "wsa:MessageID", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_PointerTowsa__ServiceNameType(struct soap *soap, struct wsa__ServiceNameType *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	if (!soap_reference(soap, *a, SOAP_TYPE_wsa__ServiceNameType))
		soap_serialize_wsa__ServiceNameType(soap, *a);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_PointerTowsa__ServiceNameType(struct soap *soap, const char *tag, int id, struct wsa__ServiceNameType *const*a, const char *type)
{
	id = soap_element_id(soap, tag, id, *a, NULL, 0, type, SOAP_TYPE_wsa__ServiceNameType, NULL);
	if (id < 0)
		return soap->error;
	return soap_out_wsa__ServiceNameType(soap, tag, id, *a, type);
}

SOAP_FMAC3 struct wsa__ServiceNameType ** SOAP_FMAC4 soap_in_PointerTowsa__ServiceNameType(struct soap *soap, const char *tag, struct wsa__ServiceNameType **a, const char *type)
{
	(void)type; /* appease -Wall -Werror */
	if (soap_element_begin_in(soap, tag, 1, NULL))
		return NULL;
	if (!a)
		if (!(a = (struct wsa__ServiceNameType **)soap_malloc(soap, sizeof(struct wsa__ServiceNameType *))))
			return NULL;
	*a = NULL;
	if (!soap->null && *soap->href != '#')
	{	soap_revert(soap);
		if (!(*a = soap_in_wsa__ServiceNameType(soap, tag, *a, type)))
			return NULL;
	}
	else
	{	a = (struct wsa__ServiceNameType **)soap_id_lookup(soap, soap->href, (void**)a, SOAP_TYPE_wsa__ServiceNameType, sizeof(struct wsa__ServiceNameType), 0, NULL);
		if (soap->body && soap_element_end_in(soap, tag))
			return NULL;
	}
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_PointerTowsa__ServiceNameType(struct soap *soap, struct wsa__ServiceNameType *const*a, const char *tag, const char *type)
{
	if (soap_out_PointerTowsa__ServiceNameType(soap, tag ? tag : "wsa:ServiceNameType", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 struct wsa__ServiceNameType ** SOAP_FMAC4 soap_get_PointerTowsa__ServiceNameType(struct soap *soap, struct wsa__ServiceNameType **p, const char *tag, const char *type)
{
	if ((p = soap_in_PointerTowsa__ServiceNameType(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_PointerTo_QName(struct soap *soap, char **const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	if (!soap_reference(soap, *a, SOAP_TYPE__QName))
		soap_serialize__QName(soap, *a);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_PointerTo_QName(struct soap *soap, const char *tag, int id, char **const*a, const char *type)
{
	id = soap_element_id(soap, tag, id, *a, NULL, 0, type, SOAP_TYPE__QName, NULL);
	if (id < 0)
		return soap->error;
	return soap_out__QName(soap, tag, id, *a, type);
}

SOAP_FMAC3 char *** SOAP_FMAC4 soap_in_PointerTo_QName(struct soap *soap, const char *tag, char ***a, const char *type)
{
	(void)type; /* appease -Wall -Werror */
	if (soap_element_begin_in(soap, tag, 1, NULL))
		return NULL;
	if (!a)
		if (!(a = (char ***)soap_malloc(soap, sizeof(char **))))
			return NULL;
	*a = NULL;
	if (!soap->null && *soap->href != '#')
	{	soap_revert(soap);
		if (!(*a = soap_in__QName(soap, tag, *a, type)))
			return NULL;
	}
	else
	{	a = (char ***)soap_id_lookup(soap, soap->href, (void**)a, SOAP_TYPE__QName, sizeof(char *), 1, NULL);
		if (soap->body && soap_element_end_in(soap, tag))
			return NULL;
	}
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_PointerTo_QName(struct soap *soap, char **const*a, const char *tag, const char *type)
{
	if (soap_out_PointerTo_QName(soap, tag ? tag : "QName", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 char *** SOAP_FMAC4 soap_get_PointerTo_QName(struct soap *soap, char ***p, const char *tag, const char *type)
{
	if ((p = soap_in_PointerTo_QName(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_PointerTowsa__ReferenceParametersType(struct soap *soap, struct wsa__ReferenceParametersType *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	if (!soap_reference(soap, *a, SOAP_TYPE_wsa__ReferenceParametersType))
		soap_serialize_wsa__ReferenceParametersType(soap, *a);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_PointerTowsa__ReferenceParametersType(struct soap *soap, const char *tag, int id, struct wsa__ReferenceParametersType *const*a, const char *type)
{
	id = soap_element_id(soap, tag, id, *a, NULL, 0, type, SOAP_TYPE_wsa__ReferenceParametersType, NULL);
	if (id < 0)
		return soap->error;
	return soap_out_wsa__ReferenceParametersType(soap, tag, id, *a, type);
}

SOAP_FMAC3 struct wsa__ReferenceParametersType ** SOAP_FMAC4 soap_in_PointerTowsa__ReferenceParametersType(struct soap *soap, const char *tag, struct wsa__ReferenceParametersType **a, const char *type)
{
	(void)type; /* appease -Wall -Werror */
	if (soap_element_begin_in(soap, tag, 1, NULL))
		return NULL;
	if (!a)
		if (!(a = (struct wsa__ReferenceParametersType **)soap_malloc(soap, sizeof(struct wsa__ReferenceParametersType *))))
			return NULL;
	*a = NULL;
	if (!soap->null && *soap->href != '#')
	{	soap_revert(soap);
		if (!(*a = soap_in_wsa__ReferenceParametersType(soap, tag, *a, type)))
			return NULL;
	}
	else
	{	a = (struct wsa__ReferenceParametersType **)soap_id_lookup(soap, soap->href, (void**)a, SOAP_TYPE_wsa__ReferenceParametersType, sizeof(struct wsa__ReferenceParametersType), 0, NULL);
		if (soap->body && soap_element_end_in(soap, tag))
			return NULL;
	}
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_PointerTowsa__ReferenceParametersType(struct soap *soap, struct wsa__ReferenceParametersType *const*a, const char *tag, const char *type)
{
	if (soap_out_PointerTowsa__ReferenceParametersType(soap, tag ? tag : "wsa:ReferenceParametersType", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 struct wsa__ReferenceParametersType ** SOAP_FMAC4 soap_get_PointerTowsa__ReferenceParametersType(struct soap *soap, struct wsa__ReferenceParametersType **p, const char *tag, const char *type)
{
	if ((p = soap_in_PointerTowsa__ReferenceParametersType(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_PointerTowsa__ReferencePropertiesType(struct soap *soap, struct wsa__ReferencePropertiesType *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	if (!soap_reference(soap, *a, SOAP_TYPE_wsa__ReferencePropertiesType))
		soap_serialize_wsa__ReferencePropertiesType(soap, *a);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_PointerTowsa__ReferencePropertiesType(struct soap *soap, const char *tag, int id, struct wsa__ReferencePropertiesType *const*a, const char *type)
{
	id = soap_element_id(soap, tag, id, *a, NULL, 0, type, SOAP_TYPE_wsa__ReferencePropertiesType, NULL);
	if (id < 0)
		return soap->error;
	return soap_out_wsa__ReferencePropertiesType(soap, tag, id, *a, type);
}

SOAP_FMAC3 struct wsa__ReferencePropertiesType ** SOAP_FMAC4 soap_in_PointerTowsa__ReferencePropertiesType(struct soap *soap, const char *tag, struct wsa__ReferencePropertiesType **a, const char *type)
{
	(void)type; /* appease -Wall -Werror */
	if (soap_element_begin_in(soap, tag, 1, NULL))
		return NULL;
	if (!a)
		if (!(a = (struct wsa__ReferencePropertiesType **)soap_malloc(soap, sizeof(struct wsa__ReferencePropertiesType *))))
			return NULL;
	*a = NULL;
	if (!soap->null && *soap->href != '#')
	{	soap_revert(soap);
		if (!(*a = soap_in_wsa__ReferencePropertiesType(soap, tag, *a, type)))
			return NULL;
	}
	else
	{	a = (struct wsa__ReferencePropertiesType **)soap_id_lookup(soap, soap->href, (void**)a, SOAP_TYPE_wsa__ReferencePropertiesType, sizeof(struct wsa__ReferencePropertiesType), 0, NULL);
		if (soap->body && soap_element_end_in(soap, tag))
			return NULL;
	}
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_PointerTowsa__ReferencePropertiesType(struct soap *soap, struct wsa__ReferencePropertiesType *const*a, const char *tag, const char *type)
{
	if (soap_out_PointerTowsa__ReferencePropertiesType(soap, tag ? tag : "wsa:ReferencePropertiesType", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 struct wsa__ReferencePropertiesType ** SOAP_FMAC4 soap_get_PointerTowsa__ReferencePropertiesType(struct soap *soap, struct wsa__ReferencePropertiesType **p, const char *tag, const char *type)
{
	if ((p = soap_in_PointerTowsa__ReferencePropertiesType(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize__QName(struct soap *soap, char *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	(void)soap_reference(soap, *a, SOAP_TYPE__QName);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out__QName(struct soap *soap, const char *tag, int id, char *const*a, const char *type)
{
	return soap_outstring(soap, tag, id, a, type, SOAP_TYPE__QName);
}

SOAP_FMAC3 char * * SOAP_FMAC4 soap_in__QName(struct soap *soap, const char *tag, char **a, const char *type)
{
	a = soap_instring(soap, tag, a, type, SOAP_TYPE__QName, 2, 0, -1, NULL);
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put__QName(struct soap *soap, char *const*a, const char *tag, const char *type)
{
	if (soap_out__QName(soap, tag ? tag : "QName", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 char ** SOAP_FMAC4 soap_get__QName(struct soap *soap, char **p, const char *tag, const char *type)
{
	if ((p = soap_in__QName(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

SOAP_FMAC3 void SOAP_FMAC4 soap_serialize_string(struct soap *soap, char *const*a)
{
	(void)soap; (void)a; /* appease -Wall -Werror */
#ifndef WITH_NOIDREF
	(void)soap_reference(soap, *a, SOAP_TYPE_string);
#endif
}

SOAP_FMAC3 int SOAP_FMAC4 soap_out_string(struct soap *soap, const char *tag, int id, char *const*a, const char *type)
{
	return soap_outstring(soap, tag, id, a, type, SOAP_TYPE_string);
}

SOAP_FMAC3 char * * SOAP_FMAC4 soap_in_string(struct soap *soap, const char *tag, char **a, const char *type)
{
	a = soap_instring(soap, tag, a, type, SOAP_TYPE_string, 1, 0, -1, NULL);
	return a;
}

SOAP_FMAC3 char * * SOAP_FMAC4 soap_new_string(struct soap *soap, int n)
{
	char * *p;
	char * *a = (char **)soap_malloc((soap), (n = (n < 0 ? 1 : n)) * sizeof(char *));
	for (p = a; p && n--; p++)
		soap_default_string(soap, p);
	return a;
}

SOAP_FMAC3 int SOAP_FMAC4 soap_put_string(struct soap *soap, char *const*a, const char *tag, const char *type)
{
	if (soap_out_string(soap, tag ? tag : "string", -2, a, type))
		return soap->error;
	return soap_putindependent(soap);
}

SOAP_FMAC3 char ** SOAP_FMAC4 soap_get_string(struct soap *soap, char **p, const char *tag, const char *type)
{
	if ((p = soap_in_string(soap, tag, p, type)))
		if (soap_getindependent(soap))
			return NULL;
	return p;
}

#if defined(__BORLANDC__)
#pragma option pop
#pragma option pop
#endif

/* End of soapC_nnn.c */
